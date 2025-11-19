// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SphereComponent.h"

#include "Common/JsonUtil.h"
#include "Simulation/CelestialBody.h"
#include "Simulation/OrbitComponent.h"
#include "Simulation/SimulationController.h"
#include "Common/Craft/FuelComponent.h"
#include "Common/Craft/StageManager.h"
#include "Common/Craft/FuelManager.h"
#include "Common/AssetLibrary.h"

#include "GeometryCollection/GeometryCollection.h"
#include "GeometryCollection/GeometryCollectionClusteringUtility.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

#include "ChaosModularVehicle/ClusterUnionVehicleComponent.h"
#include "ChaosModularVehicle/VehicleSimComponentsInclude.h"
#include "ModularVehicle/VehicleSimThruster2DComponent.h"

#include "GameFramework/PawnMovementComponent.h"

#include "ChaosModularVehicle/ModularVehicleBaseComponent.h"


class Part {
	UGeometryCollectionComponent* Component;

	TArray<Part> Children;
};

// Sets default values
ACraft::ACraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	PostPhysics(ETickingGroup::TG_PostPhysics, &ACraft::TickPostPhysics)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseEyeHeight = 0;
	PhysicsEnabled = false;

	Orbit = CreateDefaultSubobject<UOrbitComponent>("OrbitComponent");

	FuelManager = CreateDefaultSubobject<UFuelManager>("FuelManager");
	StageManager = CreateDefaultSubobject<UStageManager>("StageManager");



	static const FName Default__Craft(TEXT("Default__Craft"));
	if (GetFName() == Default__Craft) {

	}
	// Name = name.random-int
	// name is the saved craft file name, appended .random-int is an instance of the craft

	// Add inputs
	if (auto* BaseSim = GetVehicleSimulationComponent()) {
		BaseSim->InputConfig.Add(FModuleInputSetup(FName("Steering"), EModuleInputValueType::MAxis1D));
		BaseSim->InputConfig.Add(FModuleInputSetup(FName("Throttle"), EModuleInputValueType::MAxis1D));
		BaseSim->InputConfig.Add(FModuleInputSetup(FName("Thrust"), EModuleInputValueType::MAxis1D));
		BaseSim->InputConfig.Add(FModuleInputSetup(FName("Pitch"), EModuleInputValueType::MAxis1D));
		BaseSim->InputConfig.Add(FModuleInputSetup(FName("Roll"), EModuleInputValueType::MAxis1D));
		BaseSim->InputConfig.Add(FModuleInputSetup(FName("Yaw"), EModuleInputValueType::MAxis1D));
		UE_LOG(LogTemp, Warning, TEXT("Setup inputs"));
	}

}

void ACraft::OnConstruction(const FTransform& Transform) {
}

void ACraft::FromJson(TSharedPtr<FJsonObject> Json) {
	// structure + parts

	// Array of (Parent, ChildJson[])

	for (auto& [Name, Definition] : Json->GetObjectField(TEXT("parts"))->Values) {
		FTransform Transform;
		// auto* Part = Cast<UPart>(AddComponentByClass(UPart::StaticClass(), true, Transform, false));

		auto* Part = NewObject<UPart>(this, FName(Name));
		Part->FromJson(Definition->AsObject());
		Parts.Add({ Name, Part });
	}

	// depth first traversal through the tree via array (avoid recursion)
	TArray<TPair<USceneComponent*, TSharedPtr<FJsonObject>>> Structures = {
		{ GetClusterUnionComponent(), Json->GetObjectField(TEXT("structure")) }
	};
	while (Structures.Num() > 0) {
		auto [Parent, ChildJson] = Structures.Pop();
		for (auto& [Name, GrandChildrenJson] : ChildJson->Values) {
			auto Part = Parts.FindChecked(Name);
			Part->Mesh->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
			Structures.Add({ Part->Mesh, GrandChildrenJson->AsObject() });
		}
	}
}

TSharedPtr<FJsonObject> ACraft::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> JsonStructure = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> JsonParts = MakeShared<FJsonObject>();

	for (auto& [Name, Part] : Parts) {
		auto PartJson = Part->ToJson();
		JsonParts->SetObjectField(Name, PartJson);
	}

	// depth first traversal through 
	TArray<TPair<USceneComponent*, TSharedPtr<FJsonObject>>> Structure = {
		{ GetClusterUnionComponent(), JsonStructure }
	};
	while (Structure.Num() > 0) {
		auto [Parent, ChildJson] = Structure.Pop();
		for (auto& Child : Parent->GetAttachChildren()) {
			// check the owner of this component is a UPart, if it is, it's the base primitive
			// for the part, otherwise, it is be a subobject definition (sim/partial part)
			if (UPart* Part = Cast<UPart>(Child->GetOuter())) {
				TSharedPtr<FJsonObject> GrandChildrenJson = MakeShared<FJsonObject>();
				ChildJson->SetObjectField(Part->GetName(), GrandChildrenJson);
				Structure.Add({ Child, GrandChildrenJson });
			}
		}
	}

	JsonUtil::Vector(Json, TEXT("location"), GetActorLocation());
	JsonUtil::Quat(Json, TEXT("rotation"), GetActorQuat());
	Json->SetObjectField(TEXT("structure"), JsonStructure);
	Json->SetObjectField(TEXT("parts"), JsonParts);

	/**
	// structure + parts
	TArray<TPair<UPart*, TSharedPtr<FJsonObject>>> PartStructures;
	if (RootPart()) {
		TSharedPtr<FJsonObject> RootPartJson = MakeShareable(new FJsonObject());
		Structure->SetObjectField(RootPart()->Id, RootPartJson);
		PartStructures.Add({ RootPart(), RootPartJson});
	}

	for (int i = 0; i < PartStructures.Num(); ++i) {
		UPart* Part = PartStructures[i].Key;
		PartsJson->SetObjectField(Part->Id, Part->ToJson());

		for (auto Child : Part->Children) {
			UE_LOG(LogTemp, Warning, TEXT("Part: %s"), *Child->Id);
			
			TSharedPtr<FJsonObject> ChildJson = MakeShareable(new FJsonObject());
			PartStructures[i].Value->SetObjectField(Child->Id, ChildJson);
			PartStructures.Add({ Child, ChildJson });
		}
	}

	Json->SetObjectField(TEXT("structure"), Structure);
	Json->SetObjectField(TEXT("parts"), PartsJson);

	// stages
	Json->SetArrayField(TEXT("stages"), StageManager->ToJson());
	*/
	return Json;
}

ACraft* ACraft::Clone() {
	// TODO: make this implementation direct? maybe
	TSharedPtr<FJsonObject> Json = ToJson();
	FActorSpawnParameters SpawnParamsAlwaysSpawn = FActorSpawnParameters();
	SpawnParamsAlwaysSpawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACraft* NewCraft = GetWorld()->SpawnActor<ACraft>(SpawnParamsAlwaysSpawn);
	NewCraft->FromJson(Json);

	return NewCraft;
}

// Called when the game starts or when spawned
void ACraft::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACraft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*
	if (!PhysicsEnabled) {
		if (Orbit->CentralBody != nullptr) { // simulation, but no physics
			double Time = GetGameTimeSinceCreation();
			FVector Position;
			double TrueAnomaly = Orbit->GetTrueAnomaly(Time);
			Orbit->GetPositionAndVelocity(&Position, nullptr, TrueAnomaly);
			SetActorLocation(Position + Orbit->CentralBody->GetActorLocation());

			Orbit->UpdateSpline();
		}
		return;
	}

	if (Orbit->CentralBody != nullptr) {
		FVector RelativeLocation = Orbit->CentralBody->GetActorLocation() - GetWorldCoM();
		FVector Gravity = RelativeLocation.GetSafeNormal() * Orbit->CentralBody->Mu / RelativeLocation.SquaredLength();
		for (auto& PartKVP : Parts) {
			UPart* Part = PartKVP.Value;
			Part->Mesh->AddForce(Gravity, Part->Bone, true);
		}
	}
	*/
	
	/*
	FVector Position, Velocity;
	double TrueAnomaly = Orbit->GetTrueAnomaly(GetGameTimeSinceCreation() + DeltaTime);
	Orbit->GetPositionAndVelocity(&Position, &Velocity, TrueAnomaly);

	// Gravitation (via Orbit component)
	TargetPosition = (Position + Orbit->CentralBody->TargetPosition);
	TargetVelocity = (TargetPosition - CalculateCoM()) / DeltaTime;
	FVector VelocityChange = TargetVelocity - GetVelocity();

	UE_LOG(LogTemp, Warning, TEXT("Vel Change: %s -- %s -- %s -- %s -- %s"), *TargetPosition.ToString(), *Orbit->CentralBody->TargetPosition.ToString(), *TargetVelocity.ToString(), *GetVelocity().ToString(), *RootPart()->GetComponentVelocity().ToString());

	for (auto& PartKVP : Parts) {
		UPart* Part = PartKVP.Value;
		// UE_LOG(LogTemp, Warning, TEXT("grav %f"), CentralBody->Mu / SquareDistance);
		Part->AddImpulse(VelocityChange, NAME_None, true);
	}
	*/
	// Throttle
	/*
	ASimulationController* SimulationController = Cast<ASimulationController>(Controller);
	if (SimulationController && SimulationController->ThrottleValue > 0) 
	{
		double FuelDrain = 0;
		for (auto& Engine : Active->Engines) {
			// FuelDrain += Engine.FuelDrain;
		}

		FuelState FuelTotal;
		for (auto& FuelTank : Active->Fuels) {

			// FuelTotal += FuelTank.Fuel;
		}

		double ThrustPercent = FMath::Min(FuelTotal.FindChecked(FuelType::LiquidFuel) / FuelDrain, SimulationController->ThrottleValue);

		FVector thrust = FVector(0, 0, 700000 * SimulationController->ThrottleValue);
		thrust = RootPart()->GetComponentRotation().RotateVector(thrust);
		RootPart()->AddForce(thrust);
	}
	*/
}

void ACraft::TickPostPhysics(float DeltaTime) {
	return;
	/*
	if (!PhysicsEnabled && Orbit->CentralBody == nullptr) {
		return; // In build mode
	}

	if (Orbit->CentralBody == nullptr) {
		return;
	}

	FVector Position = GetWorldCoM();
	FVector Velocity = Root->Mesh->GetComponentVelocity();

	SetActorLocationAndRotation(Position, Root->Mesh->GetComponentQuat());

	// TODO: Optimize, call Orbit->GetTrueAnomaly less as it's a loop

	// Updating orbit
	FVector PositionChange = Position - TargetPosition;
	FVector VelocityChange = Velocity - TargetVelocity;
	if (!VelocityChange.IsNearlyZero()) {
		FVector OrbitVelocity;
		double Time = GetGameTimeSinceCreation() + DeltaTime;
		double TrueAnomaly = Orbit->GetTrueAnomaly(Time);
		Orbit->GetPositionAndVelocity(nullptr, &OrbitVelocity, TrueAnomaly);

		//UE_LOG(LogTemp, Warning, TEXT("Velocity didn't get there - %f"), VelocityChange.Length());

		Orbit->UpdateOrbit(GetWorldCoM() - Orbit->CentralBody->GetActorLocation(), Velocity, Time);
		// Orbit->UpdateOrbit(GetActorLocation() - Orbit->CentralBody->GetActorLocation(), VelocityChange + Velocity, Time);
	}
	 
	if (!PositionChange.IsNearlyZero()) {
		//UE_LOG(LogTemp, Warning, TEXT("Position didn't get there - %f"), PositionChange.Length());

	}
	return;
	/*
	// Updating frame of reference if outside SOI
	if (FVector::DistSquared(GetActorLocation(), Orbit->CentralBody->GetActorLocation()) > FMath::Square(Orbit->CentralBody->RadiusOfInfluence)) {
		FVector Position, Velocity;
		FVector CentralBodyPosition, CentralBodyVelocity;
		double Time = GetGameTimeSinceCreation() + DeltaTime;
		// 
		Orbit->CentralBody = Orbit->CentralBody->Orbit->CentralBody;
		Orbit->UpdateOrbit(Position + CentralBodyPosition, Velocity + CentralBodyVelocity, Time);
	}

	// Updating frame of reference if going inside a moon of this planet
	Orbit->UpdateSpline();
	*/
}

// Called to bind functionality to input
void ACraft::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// transfer ownership of part and any children to another craft
// detaches everything, must be re-attached afterwards
// also doesn't remove from the part list of the previous craft, so additional processing can be done
static void TransferPart(UPart* Part, ACraft* FromCraft, ACraft* ToCraft) {
	if (FromCraft == ToCraft) {
		return;
	}

	Part->UnregisterComponent();

	// All attached childrens are children in the part hierchy
	

	Part->Mesh->GetAttachChildren();

	UE_LOG(LogTemp, Warning, TEXT("Transfer %s"), *Part->Id);
	if (!FromCraft->Parts.Contains(Part->Id)) {
		// Part has already been moved???
		UE_LOG(LogTemp, Warning, TEXT("Transfer Failed %s"), *Part->Id);
		return;
	}

	// Part->Detach();

	for (auto& Child : Part->Children) {
		TransferPart(Child, FromCraft, ToCraft);
	}
	// every part that can be connected to this has been detached, we can safely rename (change ownership) now
	// avoid name collisions
	FString OriginalName = Part->Id;
	int i = 0;
	while (ToCraft->Parts.Contains(Part->Id)) {
		Part->Id = FString::Printf(TEXT("%s-%d"), *Part->Type, i); // increment and set new id
		++i;
	}
	if (OriginalName != Part->Id) {
		FromCraft->Parts.Remove(OriginalName);
		FromCraft->Parts.Add(Part->Id, Part);
	}
	Part->Rename(*Part->Id, ToCraft);
	ToCraft->Parts.Add(Part->Id, Part);
}

void Transfer(UPart* SourcePart, UPrimitiveComponent* DestPart) {
	if (SourcePart == nullptr || DestPart == nullptr) {
		return;
	}
	ACraft* SourceCraft = SourcePart->GetOwner<ACraft>();
	ACraft* DestCraft = DestPart->GetOwner<ACraft>();


	if (SourceCraft == nullptr || DestCraft == nullptr) {
		return;
	}
	
	// go through all children of the part, unregister them, 

	TArray<TPair<USceneComponent*, USceneComponent*>> Components;
	TArray<UPart*> Parts;

	// detach and unregister everything
	while (Components.Num() > 0) {
		auto [AttachedTo, Component] = Components.Pop();

		Component->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Component->UnregisterComponent();

		Component->Rename(nullptr, AttachedTo);

		for (auto& Child : Component->GetAttachChildren()) {
			Components.Add({ Component, Child });
		}

		// if the direct outer is a UPart, it's the base mesh transform
		if (UPart* Part = Cast<UPart>(Component->GetOuter())) {
			Parts.Add(Part);
		}
	}

	// rename parts, which are the top owners of their own physics/etc
	for (auto* Part : Parts) {
		Part->Rename(nullptr, DestCraft);
	}

	// attach everything again
	for (auto [AttachedTo, Component] : Components) {
		Component->AttachToComponent(AttachedTo, FAttachmentTransformRules::KeepRelativeTransform);
		Component->RegisterComponent();
	}
}

void ACraft::DetachPart(UPart* Part, ACraft* NewCraft) {
	
	TSharedPtr<FJsonObject> CraftJson = JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/empty.json");
	if (!CraftJson.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("invalid new craft"));
		return;
	}
	NewCraft->FromJson(CraftJson);
	NewCraft->SetActorLocation(Part->Mesh->GetComponentLocation());

	// transfer
	TransferPart(Part, this, NewCraft);

	Part->SetParent(nullptr);

	// complete
	for (auto& PartKVP : NewCraft->Parts) {
		Parts.Remove(PartKVP.Key);
		// PartKVP.Value->Attach();
	}

	// NewCraft->SetRootComponent(Part->Mesh);
	NewCraft->PhysicsEnabled = PhysicsEnabled;
}

void ACraft::AttachPart(ACraft* SourceCraft, UPart* AttachToPart) {
	auto& RootMeshes = SourceCraft->GetRootComponent()->GetAttachChildren();
	if (RootMeshes.Num() <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("Source craft has no root part!"));
		return;
	}
	UPart* Part = RootMeshes.Last()->GetTypedOuter<UPart>();

	// transfer
	TransferPart(Part, SourceCraft, this);

	// Part->SetParent(AttachToPart);

	for (auto& PartKVP : SourceCraft->Parts) {
		// PartKVP.Value->Attach();
	}

	// TODO: transfer stages
	//for (auto& Stage : SourceCraft->Stages) {
	//	Stages.Add(Stage);
	//}

	SourceCraft->Parts.Empty();
	SourceCraft->SetRootComponent(nullptr);
	SourceCraft->Destroy();
}

void ACraft::Rotate(FRotator Rotator, float Strength) {
	
}

void ACraft::SetPhysicsEnabled(bool enabled) {
	for (UActorComponent* Component : GetComponents())
	{
		if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
		{
			PrimComp->SetSimulatePhysics(enabled);
		}
	}
	PhysicsEnabled = enabled;
}

FVector ACraft::CalculateCoM() {
	// center of mass relative to root
	double Mass = 0;
	FVector CenterOfMass = FVector(0);
	for (auto PartKVP : Parts) {
		auto Part = PartKVP.Value;
		CenterOfMass += Part->Mesh->GetRelativeLocation() * Part->Mesh->CalculateMass();
		Mass += Part->Mesh->CalculateMass();
	}
	return CenterOfMass / Mass;
}


FVector ACraft::GetWorldCoM() {
	// center of mass relative to root
	double Mass = 0;
	FVector CenterOfMass = FVector(0);
	for (auto PartKVP : Parts) {
		auto Part = PartKVP.Value;
		CenterOfMass += Part->Mesh->GetComponentLocation() * Part->Mesh->CalculateMass();
		Mass += Part->Mesh->CalculateMass();
	}
	return CenterOfMass / Mass;
}

TArray<ACraft*> ACraft::StageCraft() {
	// Temp try to lock the wheels
	GetVehicleSimulationComponent()->RecreatePhysicsState();
	/** Code for getting the active chaos simulation component from the cluster union sim component 
	int WheelGuid = 0;
	UPart* Part = Parts.FindChecked(FString("tire_fr"));
	for (auto& Child : Part->Mesh->GetAttachChildren()) {
		if (auto* Suspension = Cast<UVehicleSimSuspensionComponent>(Child)) {
			auto& SuspensionChildren = Suspension->GetAttachChildren();
			UE_LOG(LogTemp, Warning, TEXT("Found wheel has %d children"), SuspensionChildren.Num());
			if (SuspensionChildren.Num() > 0) {
				if (auto* Wheel = Cast<UVehicleSimWheelComponent>(SuspensionChildren[0])) {
					Wheel->bSteeringEnabled = false;

					const auto& Data = GetVehicleSimulationComponent()->ComponentToPhysicsObjects.Find(Wheel);

					WheelGuid = Data->Guid;
					UE_LOG(LogTemp, Warning, TEXT("Found wheel sim, tree index is %d"), WheelGuid);
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Tire children is "));
	}
	// Probably can only update the sim component, then in place replace it at the tree index, and hope there is no other reference
	for (auto& Node : GetVehicleSimulationComponent()->VehicleSimulationPT->AccessSimComponentTree()->GetSimulationModuleTree()) {
		if (Node.SimModule->GetGuid() == WheelGuid) {
			if (Chaos::FWheelSimModule* Wheel = static_cast<Chaos::FWheelSimModule*>(Node.SimModule)) {
				float MaxSteering = Wheel->AccessSetup().MaxSteeringAngle;
				UE_LOG(LogTemp, Warning, TEXT("Wow it's a wheel, steering %f"), MaxSteering);
				Wheel->AccessSetup().MaxSteeringAngle = 0;
			}
		}
	}
	*/
	
	UE_LOG(LogTemp, Warning, TEXT("---"));

	// TODO: 
	return TArray<ACraft*>();
}

FVector ACraft::GetAngularVelocity() {
	return FVector();// RootPart()->Mesh->GetPhysicsAngularVelocityInRadians();
}

void ACraft::SetLocation(FVector Location) {
	if (PhysicsEnabled) { // need to set every part
		ForEachComponent<UMeshComponent>(true, [](UMeshComponent* Mesh) {

		});
	}
	else { // all parts attached to root, so only need to set root
		// RootPart()->Mesh->SetWorldLocation(Location);
	}

	// SetActorLocation(Location);

	return;
}

void ACraft::SetRotation(FQuat Rotation) {
	if (PhysicsEnabled) {
		ForEachComponent<UMeshComponent>(true, [](UMeshComponent* Mesh) {

		});
	}
	else {
		// RootPart()->Mesh->SetWorldRotation(Rotation);
	}

	return;
}
