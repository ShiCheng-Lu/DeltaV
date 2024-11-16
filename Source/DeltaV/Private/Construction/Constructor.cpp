// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/Constructor.h"

#include "Common/Craft.h"
#include "Common/Part.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Construction/ConstructionController.h"

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

ACraft* Constructor::CreateCraft(TSharedPtr<FJsonObject> CraftJson) {
	ACraft* Craft = World->SpawnActor<ACraft>(SpawnParamsAlwaysSpawn);
	Craft->FromJson(CraftJson);

	// add attachment nodes
	for (auto& PartKVP : Craft->Parts) {
		UPart* Part = PartKVP.Value;

		Part->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
		Part->SetCollisionResponseToAllChannels(ECR_Ignore);
		Part->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Block);
		Part->SetSimulatePhysics(false);

		TSharedPtr<FJsonObject> PartDefinition = UAssetLibrary::PartDefinition(Part->Type);
		for (auto& Node : PartDefinition->GetArrayField(TEXT("attachment"))) {
			auto location = JsonUtil::Vector(Node->AsObject(), "location");

			auto AttachmentNode = NewObject<UAttachmentNode>(Part);
			AttachmentNode->SetRelativeLocation(location);
			AttachmentNode->RegisterComponent();
			AttachmentNode->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
			AttachmentNode->SetCollisionResponseToAllChannels(ECR_Ignore);
			AttachmentNode->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Block);

			Part->AttachmentNodes.Add(AttachmentNode);

			AttachmentNode->SetAbsolute(false, true, true);
		}
	}

	return Craft;
}

void Constructor::Select(UPart* Part) {
	bool SameOwner = (Part && Selected && (Part->GetOwner() == Selected->GetOwner()));

	// both null, same part, or same owner, we don't need to update ray-trace settings
	if (Part == Selected || SameOwner) {
		Selected = Part;
		return;
	}

	// Set previously selected part to respond to ECC_Construct
	if (Selected != nullptr && !SameOwner) {
		for (auto* Component : Selected->GetOwner()->GetComponents()) {
			UMeshComponent* SceneComponent = Cast<UMeshComponent>(Component);
			if (Component->GetClass() == UPart::StaticClass()) {
				SceneComponent->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Block);
			}
			if (Component->GetClass() == UAttachmentNode::StaticClass()) {
				SceneComponent->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Block);
			}
		}
	}

	// Set newly selected part to respond ignore to ECC_Construct
	if (Part != nullptr && !SameOwner) {
		for (auto* Component : Part->GetOwner()->GetComponents()) {
			UMeshComponent* SceneComponent = Cast<UMeshComponent>(Component);
			if (Component->GetClass() == UPart::StaticClass()) {
				SceneComponent->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Ignore);
			}
			if (Component->GetClass() == UAttachmentNode::StaticClass()) {
				SceneComponent->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Ignore);
			}
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
		return Cast<UPart>(Result.GetComponent());
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

	Distance = FVector::Distance(CameraLocation, Part->GetComponentLocation());
}

UPart* Constructor::Update() {
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
	FVector SelectedLocaction = Selected->GetComponentLocation();
	for (auto& AttachmentNode : Selected->AttachmentNodes) {
		FVector RelativeLocation = AttachmentNode->GetComponentLocation() - SelectedLocaction;

		End = (Direction * Distance + RelativeLocation) * 2 + Start;

		World->LineTraceMultiByChannel(Results, Start, End, ECC_AttachmentNodes);

		for (auto& HitResult : Results) {
			UAttachmentNode* TracedNode = Cast<UAttachmentNode>(HitResult.GetComponent());
			if (TracedNode == nullptr) {
				continue;
			}

			PartLocation = TracedNode->GetComponentLocation() - RelativeLocation;
			Selected->GetOwner()->SetActorLocation(PartLocation);

			// node attachment, destroy any symmetry parts
			if (SymmetryCrafts.Num() > 0) {
				for (ACraft* Craft : SymmetryCrafts) {
					Craft->Destroy();
				}
				SymmetryCrafts.Empty();
			}

			return TracedNode->GetTypedOuter<UPart>();
		}
	}

	// side attachment
	End = Start + Direction * Distance;
	World->LineTraceMultiByChannel(Results, Start, End, ECC_NoneHeldParts);

	for (auto& HitResult : Results) {
		UPart* Part = Cast<UPart>(HitResult.GetComponent());

		PartLocation = HitResult.Location;
		Selected->GetOwner()->SetActorLocation(PartLocation);
		

		// update each symmetry parts location and rotation
		UpdateSymmetry(Symmetry);
		if (SymmetryCrafts.Num() > 0) {
			FQuat BaseRotation = Selected->GetComponentQuat();
			FVector Axis = Part->GetComponentRotation().RotateVector(FVector(0, 0, 1));
			double Angle = 2 * PI / Symmetry;

			FVector BaseLocation = Part->GetComponentLocation();
			FVector Offset = Selected->GetComponentLocation() - BaseLocation;
			
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
	Selected->GetOwner()->SetActorLocation(PartLocation);
	
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
		AActor* Actor = Selected->GetOwner();
		Actor->SetActorRotation(Rotation * Actor->GetActorQuat());
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
