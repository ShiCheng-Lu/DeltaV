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
#include "ChaosModularVehicle/ClusterUnionVehicleComponent.h"
#include "ChaosModularVehicle/ModularVehicleBaseComponent.h"

Constructor::Constructor()
{
}

Constructor::~Constructor()
{
}

void Constructor::SetController(AConstructionController* InController) {
	Controller = InController;
	World = Controller->GetWorld();
}

TObjectPtr<ACraft> Constructor::CreateCraft(TSharedPtr<FJsonObject> CraftJson, FVector Location) {
	// TObjectPtr<ACraft> Craft = World->SpawnActor<ACraft>(SpawnParamsAlwaysSpawn);
	FActorSpawnParameters Params = FActorSpawnParameters();
	// Params.Name = "custom-craft-name";
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TObjectPtr<ACraft> Craft = World->SpawnActor<ACraft>(Params);
	Craft->FromJson(CraftJson);
	// Craft->SetPhysicsEnabled(false);
	// add attachment nodes
	for (auto& [Name, Part] : Craft->Parts) {
		auto* AttachmentNodes = NewObject<UAttachmentNodes>(Part->Mesh);
		AttachmentNodes->RegisterComponent();

		// Part->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// Part->Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		// Craft->ClusterUnionVehicleComponent()->AddComponent(Part->Mesh);

		// part must be (re)registered after collision response type set, otherwise collision don't work ?!
	}
	// Craft->SetActorRotation(DefaultOrientation);

	// Craft->FinishSpawning(Transform);
	Craft->SetActorLocation(Location);

	return Craft;
}

void Constructor::Select(UPart* Part) {
	UE_LOG(LogTemp, Warning, TEXT("Constructor: Selecting %x"), Part);

	bool SameOwner = (Part && Selected && (Part->GetOwner() == Selected->GetOwner()));

	// both null, same part, or same owner, we don't need to update ray-trace settings
	if ((Part == Selected) || SameOwner) {
		Selected = Part;
		return;
	}

	// Set previously selected part to respond to ECC_Construct
	if (Selected != nullptr) {
		/*
		ACraft* Craft = Cast<ACraft>(Selected->GetOwner());
		for (auto& PartKVP : Craft->Parts) {
			PartKVP.Value->Mesh->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Block);
			UAttachmentNodes::Get(PartKVP.Value)->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Block);
		}*/
	}

	// Set newly selected part to respond ignore to ECC_Construct
	if (Part != nullptr) {
		/*
		ACraft* Craft = Cast<ACraft>(Part->GetOwner());
		for (auto& PartKVP : Craft->Parts) {
			PartKVP.Value->Mesh->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Ignore);
			UAttachmentNodes::Get(PartKVP.Value)->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Ignore);
		}
		*/
		UE_LOG(LogTemp, Warning, TEXT("Select: part not null"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Set selected"));
	Selected = Part;
}

UPart* Constructor::TraceMouse() {
	FHitResult Result;
	if (Controller->GetHitResultUnderCursor(ECC_WorldDynamic, false, Result)) {
		UE_LOG(LogTemp, Warning, TEXT("TraceMouse, has part under mouse %s"), *Result.GetComponent()->GetName());
		UE_LOG(LogTemp, Warning, TEXT("TraceMouse, Outer is %s"), *Result.GetComponent()->GetOuter()->GetName());
		return Result.GetComponent()->GetTypedOuter<UPart>();
	}
	if (Controller->GetHitResultUnderCursor(ECC_WorldDynamic, true, Result)) {
		UE_LOG(LogTemp, Warning, TEXT("TraceMouse Complex, has part under mouse %s"), *Result.GetComponent()->GetName());
		UE_LOG(LogTemp, Warning, TEXT("TraceMouse Complex, Outer is %s"), *Result.GetComponent()->GetOuter()->GetName());
		return Result.GetComponent()->GetTypedOuter<UPart>();
	}
	return nullptr;
}

void Constructor::Grab() {
	UPart* Part = TraceMouse();
	if (Part == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Grab part is null"));
		return;
	}
	ACraft* Craft = Part->GetOwner<ACraft>();
	if (Craft == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Grab craft is null"));
		return;
	}
	FActorSpawnParameters Params = FActorSpawnParameters();
	// Params.Name = "custom-craft-name";
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACraft* Other = World->SpawnActor<ACraft>(Params);
	Other->SetActorLocation(Part->Mesh->GetComponentLocation());
	ACraft::Transfer(Part, Other->GetClusterUnionComponent());
	Other->SetPhysicsEnabled(false);

	/**
	if (Craft->RootPart() != Part) {
		ACraft* NewCraft = World->SpawnActor<ACraft>();
		Craft->DetachPart(Part, NewCraft);
	}*/
	/*
	Select(Part);
	UpdateSymmetry(Symmetry);

	*/
	FVector Start; FRotator Rotation;
	Controller->PlayerCameraManager->GetCameraViewPoint(Start, Rotation);
	FVector End = Start + Rotation.RotateVector(FVector(1000000, 0, 0));
	FHitResult Result;
	World->LineTraceSingleByChannel(Result, Start, End, ECC_WorldStatic);



	Distance = FVector::Distance(Start, Part->Mesh->GetComponentLocation());

	// UPart* Part = Cast<UPart>(Other->GetClusterUnionComponent()->GetAttachChildren().Last().GetOuter());
	Select(Part);
}

UPart* Constructor::Update() {
	if (Selected == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Update called but no selected"));
		return nullptr;
	}

	// update location of selected part
	FVector CameraLocation;
	FVector Direction;
	if (!Controller->DeprojectMousePositionToWorld(CameraLocation, Direction)) {
		UE_LOG(LogTemp, Warning, TEXT("Update called camera deproject failed"));
		return nullptr;
	}
	FVector CameraLocation2;
	FRotator _;
	Controller->PlayerCameraManager->GetCameraViewPoint(CameraLocation2, _);


	FHitResult Result;
	FVector Start = CameraLocation; // start of the trace ray
	FVector End; // end of the trace ray, goes through every attachment node
	// node attachment
	FVector SelectedLocaction = Selected->Mesh->GetComponentLocation();

	if (UAttachmentNodes* Attachment = UAttachmentNodes::Get(Selected)) {
		for (auto& Node : Attachment->AttachmentNodes) {

			FVector RelativeLocation = Node->GetComponentLocation() - SelectedLocaction;

			End = (Direction * Distance + RelativeLocation) * 2 + Start;

			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Selected->GetOwner());
			World->LineTraceSingleByChannel(Result, Start, End, ECC_AttachmentNodes, Params);

			if (!Result.bBlockingHit) {
				continue;
			}
		
			// Handle symmetry attachment

			// Put the part so that the node traced at is at the same location as the target node
			auto NodeLocation = Result.Component->GetComponentLocation();
			auto* Part = Result.Component->GetTypedOuter<UPart>();
			if (Part == nullptr) {
				continue;
			}
			Selected->GetOwner()->SetActorLocation(NodeLocation - RelativeLocation);

			UE_LOG(LogTemp, Warning, TEXT("Update part is attached to %s (part owner: %s)"), *Result.Component->GetOwner()->GetName(), *Selected->GetOwner()->GetName());
			return Part;
		}
	}

	/*
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
	*/

	// No attachment node or side attachment, place the craft/part at where it is
	Selected->GetOwner<ACraft>()->SetActorLocation(CameraLocation + Direction * Distance);

	// UE_LOG(LogTemp, Warning, TEXT("Update set location to %s - %s"), *(Selected->GetOwner<ACraft>()->GetActorLocation()).ToString(), *Selected->Mesh->GetComponentLocation().ToString());
	/*
	if (SymmetryCrafts.Num() > 0) {
		for (ACraft* Craft : SymmetryCrafts) {
			Craft->Destroy();
		}
		SymmetryCrafts.Empty();
	}
	*/
	return nullptr;
}

void Constructor::Place() {
	UE_LOG(LogTemp, Warning, TEXT("Part placed"));
	UPart* Part = Update(); // part to attach to
	if (Part == nullptr || Selected == nullptr) {
		Select(nullptr);
		return;
	}
	ACraft* Craft = Part->GetOwner<ACraft>();
	ACraft* Source = Selected->GetOwner<ACraft>();
	if (Craft == nullptr || Source == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("for some reason part owners are not ACraft"));
		return;
	}
	Craft->AttachPart(Source, Part);
	Select(nullptr);

	/*
	for (ACraft* SymmetrySource : SymmetryCrafts) {
		Craft->AttachPart(SymmetrySource, Part);
	}
	UpdateSymmetry(Symmetry);
	*/
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
					// SceneComponent->SetCollisionResponseToChannel(ECC_NoneHeldParts, ECR_Ignore);
				}
				if (Component->GetClass() == UAttachmentNode::StaticClass()) {
					// SceneComponent->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Ignore);
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
