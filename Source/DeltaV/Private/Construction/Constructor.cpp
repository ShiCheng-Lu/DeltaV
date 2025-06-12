// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/Constructor.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Common/Craft.h"
#include "Common/Part.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Construction/ConstructionController.h"
#include "Construction/AttachmentNodes.h"

Constructor::Constructor()
{
	SpawnParamsAlwaysSpawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

Constructor::~Constructor()
{
}

void Constructor::SetController(AConstructionController* InController) {
	Controller = InController;
	World = Controller->GetWorld();
}

TObjectPtr<ACraft> Constructor::CreateCraft(TSharedPtr<FJsonObject> CraftJson) {
	TObjectPtr<ACraft> Craft = World->SpawnActor<ACraft>(SpawnParamsAlwaysSpawn);
	Craft->FromJson(CraftJson);

	// add attachment nodes
	for (auto& PartKVP : Craft->Parts) {
		UPart* Part = PartKVP.Value;

		Part->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
		Part->Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Part->Mesh->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Block);
		Part->Mesh->SetSimulatePhysics(false);

		UAttachmentNodes* AttachmentNodes = NewObject<UAttachmentNodes>(Part);
		AttachmentNodes->RegisterComponent();
	}

	Craft->SetActorRotation(DefaultOrientation);

	return Craft;
}

void Constructor::Select(UPart* Part) {
	UE_LOG(LogTemp, Warning, TEXT("Constructor: Selecting %x"), Part);

	bool SameOwner = (Part && Selected && (Part->GetOwner() == Selected->GetOwner()));

	// both null, same part, or same owner, we don't need to update ray-trace settings
	if (Part == Selected || SameOwner) {
		Selected = Part;
		return;
	}

	// Set previously selected part to respond to ECC_Construct
	if (Selected != nullptr && !SameOwner) {
		ACraft* Craft = Cast<ACraft>(Selected->GetOwner());
		for (auto& PartKVP : Craft->Parts) {
			PartKVP.Value->Mesh->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Block);
			UAttachmentNodes::Get(PartKVP.Value)->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Block);
		}
	}

	// Set newly selected part to respond ignore to ECC_Construct
	if (Part != nullptr && !SameOwner) {
		ACraft* Craft = Cast<ACraft>(Part->GetOwner());
		for (auto& PartKVP : Craft->Parts) {
			PartKVP.Value->Mesh->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Ignore);
			UAttachmentNodes::Get(PartKVP.Value)->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Ignore);
		}
	}

	Selected = Part;
}

UPart* Constructor::Trace(FVector Position, FVector Direction) {
	return nullptr;
}

UPart* Constructor::TraceMouse() {
	FHitResult Result;
	if (Controller->GetHitResultUnderCursor(ECC_NoneHeldParts, true, Result)) {
		return Result.GetComponent()->GetTypedOuter<UPart>();
	}
	return nullptr;
}

void Constructor::Grab() {
	UPart* Part = TraceMouse();
	if (Part == nullptr) {
		return;
	}
	ACraft* Craft = Cast<ACraft>(Part->GetOwner());
	if (Craft == nullptr) {
		return;
	}
	if (Craft->RootPart() != Part) {
		ACraft* NewCraft = World->SpawnActor<ACraft>(SpawnParamsAlwaysSpawn);
		Craft->DetachPart(Part, NewCraft);
	}
	Select(Part);
	UpdateSymmetry(Symmetry);

	FVector CameraLocation; FRotator _;
	Controller->PlayerCameraManager->GetCameraViewPoint(CameraLocation, _);

	Distance = FVector::Distance(CameraLocation, Part->Mesh->GetComponentLocation());
}

UPart* Constructor::Update() {
	if (Selected == nullptr) {
		return nullptr;
	}

	// update location of selected part
	FVector CameraLocation;
	FVector Direction;
	if (!Controller->DeprojectMousePositionToWorld(CameraLocation, Direction)) {
		return nullptr;
	}
	FVector CameraLocation2;
	FRotator _;
	Controller->PlayerCameraManager->GetCameraViewPoint(CameraLocation2, _);

	FVector PartLocation = CameraLocation + Direction * Distance;

	TArray<FHitResult> Results;
	FVector Start = CameraLocation;
	FVector End;
	// node attachment
	FVector SelectedLocaction = Selected->Mesh->GetComponentLocation();

	UAttachmentNodes* Attachment = UAttachmentNodes::Get(Selected);
	for (auto& Node : Attachment->AttachmentNodes) {

		FVector RelativeLocation = Node->GetComponentLocation() - SelectedLocaction;

		End = (Direction * Distance + RelativeLocation) * 2 + Start;

		World->LineTraceMultiByChannel(Results, Start, End, ECC_AttachmentNodes);

		for (auto& HitResult : Results) {
			UStaticMeshComponent* TracedNode = Cast<UStaticMeshComponent>(HitResult.GetComponent());
			if (TracedNode == nullptr) {
				continue;
			}

			PartLocation = TracedNode->GetComponentLocation() - RelativeLocation;
			Selected->Mesh->SetWorldLocation(PartLocation);

			// node attachment, destroy any symmetry parts
			if (SymmetryCrafts.Num() > 0) {
				for (ACraft* Craft : SymmetryCrafts) {
					Craft->Destroy();
				}
				SymmetryCrafts.Empty();
			}
			if (Selected) {
				Selected->Physics->SetWorldLocation(TracedNode->GetComponentLocation());
			}
			return TracedNode->GetTypedOuter<UPart>();
		}
	}

	// side attachment
	End = Start + Direction * Distance;
	World->LineTraceMultiByChannel(Results, Start, End, ECC_NoneHeldParts);

	for (auto& Result : Results) {
		UPart* Part = Result.GetComponent()->GetTypedOuter<UPart>();
		if (!Part) {
			continue;
		}

		PartLocation = Result.Location + Attachment->GetComponentRotation().RotateVector(Attachment->SideAttachment);
		if (Selected) {
			Selected->Physics->SetWorldLocation(Result.Location);
			Selected->Mesh->SetWorldLocation(PartLocation);
		}

		// update each symmetry parts location and rotation
		UpdateSymmetry(Symmetry);
		if (SymmetryCrafts.Num() > 0) {
			FQuat BaseRotation = Selected->Mesh->GetComponentQuat();
			FVector Axis = Part->Mesh->GetComponentRotation().RotateVector(FVector(1, 0, 0));
			double Angle = 2 * PI / Symmetry;

			FVector BaseLocation = Part->Mesh->GetComponentLocation();
			FVector Offset = Selected->Mesh->GetComponentLocation() - BaseLocation;
			
			for (int i = 0; i < SymmetryCrafts.Num(); ++i) {
				FQuat Rotation = FQuat(Axis, Angle * (i + 1));
				FVector Location = Rotation.RotateVector(Offset);
				SymmetryCrafts[i]->SetActorRotation(Rotation * BaseRotation);
				SymmetryCrafts[i]->SetActorLocation(Location + BaseLocation);
				SymmetryCrafts[i]->SetHidden(false);
			}
		}

		return Part;
	}
	Selected->Mesh->SetWorldLocation(PartLocation);
	
	if (SymmetryCrafts.Num() > 0) {
		for (ACraft* Craft : SymmetryCrafts) {
			Craft->Destroy();
		}
		SymmetryCrafts.Empty();
	}

	return nullptr;
}

void Constructor::Place() {
	UPart* Part = Update(); // part to attach to
	if (Part == nullptr || Selected == nullptr) {
		Select(nullptr);
		return;
	}
	ACraft* Craft = Cast<ACraft>(Part->GetOwner());

	ACraft* Source = Cast<ACraft>(Selected->GetOwner());
	Craft->AttachPart(Source, Part);
	Select(nullptr);

	for (ACraft* SymmetrySource : SymmetryCrafts) {
		Craft->AttachPart(SymmetrySource, Part);
	}
	UpdateSymmetry(Symmetry);
	return;
}

void Constructor::Tick() {
	if (Selected == nullptr) {
		return;
	}
	// update location of selected part
	Update();
}

void Constructor::UpdateSymmetry(int InSymmetry) {
	Symmetry = InSymmetry;
	if (Selected) {
		int NumOfCrafts = FMath::Max(0, Symmetry - 1);
		// remove old
		while (SymmetryCrafts.Num() > NumOfCrafts) {
			ACraft* Craft = SymmetryCrafts.Pop();
			Craft->Destroy();
		}
		// create new
		TSharedPtr<FJsonObject> CraftJson = Cast<ACraft>(Selected->GetOwner())->ToJson();
		while (SymmetryCrafts.Num() < NumOfCrafts) {
			ACraft* Craft = CreateCraft(CraftJson);

			for (auto* Component : Craft->GetComponents()) {
				UMeshComponent* SceneComponent = Cast<UMeshComponent>(Component);
				if (Component->GetClass() == UPart::StaticClass()) {
					SceneComponent->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Ignore);
				}
				if (Component->GetClass() == UAttachmentNode::StaticClass()) {
					SceneComponent->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Ignore);
				}
			}

			SymmetryCrafts.Push(Craft);
		}
	}
	else if (SymmetryCrafts.Num() > 0) {
		for (ACraft* Craft : SymmetryCrafts) {
			Craft->Destroy();
		}
		SymmetryCrafts.Empty();
	}
}

void Constructor::RotatePart(FQuat Rotation) {
	if (Selected) {
		FQuat Original = Selected->Mesh->GetComponentQuat();
		Selected->Mesh->SetWorldRotation(Rotation * Original);
	}
}

void Constructor::Delete() {
	if (Selected) {
		AActor* Actor = Selected->GetOwner();
		Actor->Destroy();

		for (ACraft* Craft : SymmetryCrafts) {
			Craft->Destroy();
		}
		SymmetryCrafts.Empty();

		Selected = nullptr;
	}
}
