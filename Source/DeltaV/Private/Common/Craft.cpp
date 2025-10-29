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
	if (auto* Cluster = GetClusterUnionComponent()) {
		if (auto* GC_Chassis = UAssetLibrary::LoadAsset<UGeometryCollection>("/Game/Shapes/cockpit_cockpit/GC_cockpit")) {
			auto* Chassis = NewObject<UGeometryCollectionComponent>(this, "chassis");
			if (Chassis) {
				Chassis->SetRestCollection(GC_Chassis);
				Chassis->SetRelativeLocation(FVector(0, 0, 0));
				Chassis->DamageThreshold = { 1e8 };
				Chassis->SetupAttachment(Cluster);
				Chassis->RegisterComponent();
			}

			auto* ChassisSim = NewObject<UVehicleSimChassisComponent>(this, "chassis_sim");
			ChassisSim->SetupAttachment(Chassis);
			ChassisSim->RegisterComponent();

			auto* EngineSim = NewObject<UVehicleSimEngineComponent>(this, "engine_sim");
			EngineSim->MaxTorque = 2000;
			EngineSim->EngineBrakeEffect = 750;
			EngineSim->SetupAttachment(ChassisSim);
			EngineSim->RegisterComponent();

			auto* ClutchSim = NewObject<UVehicleSimClutchComponent>(this, "clutch_sim");
			ClutchSim->SetupAttachment(EngineSim);
			ClutchSim->RegisterComponent();

			auto* TransSim = NewObject<UVehicleSimTransmissionComponent>(this, "trans_sim");
			TransSim->SetupAttachment(ClutchSim);
			TransSim->RegisterComponent();
		}

		auto* TireMesh = UAssetLibrary::LoadAsset<UGeometryCollection>("/Game/Shapes/tire_tire/GC_tire");
		auto* TireStaticMesh = UAssetLibrary::LoadAsset<UStaticMesh>("/Game/Shapes/tire");
		auto CreateWheel = [this, TireMesh, Cluster](FString Name, FVector Location, bool Steering) {
			auto* Wheel = NewObject<UGeometryCollectionComponent>(this, FName(Name + "w"));
			Wheel->SetRestCollection(TireMesh);
			Wheel->SetRelativeLocation(Location);
			Wheel->DamageThreshold = { 1e8 };
			Wheel->AttachToComponent(Cluster, FAttachmentTransformRules::KeepRelativeTransform);
			Wheel->RegisterComponent();

			auto* Suspension = NewObject<UVehicleSimSuspensionComponent>(this, FName(Name + "sus"));
			Suspension->SuspensionMaxDrop = 100;
			Suspension->SuspensionMaxRaise = 100;
			Suspension->SpringRate = 200;
			Suspension->SpringPreload = 100;
			Suspension->AttachToComponent(Wheel, FAttachmentTransformRules::KeepRelativeTransform);
			Suspension->RegisterComponent();

			auto* WheelSim = NewObject<UVehicleSimWheelComponent>(this, FName(Name + "sim"));
			WheelSim->MaxSteeringAngle = 30;
			WheelSim->WheelRadius = 100;
			WheelSim->AxisType = EWheelAxisType::X;
			WheelSim->MaxSteeringAngle *= -1;
			WheelSim->bSteeringEnabled = Steering;
			WheelSim->AttachToComponent(Suspension, FAttachmentTransformRules::KeepRelativeTransform);
			WheelSim->RegisterComponent();

			return Wheel;
		};

		CreateWheel("wheel_fl", FVector(200, -150, -50), true);
		CreateWheel("wheel_fr", FVector(200, 150, -50), true);
		CreateWheel("wheel_rl", FVector(-200, -150, -50), false);
		CreateWheel("wheel_rr", FVector(-200, 150, -50), false);

		if (auto* ThrusterMesh = UAssetLibrary::LoadAsset<UGeometryCollection>("/Game/Shapes/engine_engine/GC_engine")) {
			auto* Thruster = NewObject<UGeometryCollectionComponent>(this, FName("thruster"));
			Thruster->SetRestCollection(ThrusterMesh);
			Thruster->SetRelativeLocation(FVector(-150, 0, 0));
			Thruster->AttachToComponent(Cluster, FAttachmentTransformRules::KeepRelativeTransform);
			Thruster->RegisterComponent();

			auto* ThrusterSim = NewObject<UVehicleSimThruster2DComponent>(this, FName("thruster_sim"));
			ThrusterSim->bSteeringEnabled = true;
			ThrusterSim->MaxThrustForce = 2000000.0f;
			ThrusterSim->MaxSteeringAngle = 20;
			ThrusterSim->AttachToComponent(Thruster, FAttachmentTransformRules::KeepRelativeTransform);
			ThrusterSim->RegisterComponent();
		}
	}
}

void ACraft::FromJson(TSharedPtr<FJsonObject> Json) {
	// structure + parts

	// Array of (Parent, ChildJson[])

	// temp just make a craft lmao

	/**
	auto& PartListJson = Json->GetObjectField(TEXT("parts"));
	TArray<TPair<TObjectPtr<UPart>, TSharedPtr<FJsonObject>>> Structures = { 
		{ nullptr, Json->GetObjectField(TEXT("structure")) } 
	};
	for (int i = 0; i < Structures.Num(); ++i) {
		for (auto& PartKVP : Structures[i].Value->Values) {
			UPart* Part = NewObject<UPart>(this, FName(PartKVP.Key));

			auto& PartJson = PartListJson->GetObjectField(PartKVP.Key);
			Part->FromJson(PartJson);
			Part->SetParent(Structures[i].Key);

			Parts.Add(PartKVP.Key, Part);

			Structures.Add({ Part, PartKVP.Value->AsObject() });
			UE_LOG(LogTemp, Warning, TEXT("created: %s"), *PartKVP.Key);
		}
	}
	if (Structures.Num() > 1) {
		Root = Structures[1].Key;
	}

	// stages
	StageManager->FromJson(Json->GetArrayField(TEXT("stages")));
	FuelManager->FromJson();


	for (auto& PartKVP : Parts) {
		PartKVP.Value->RegisterComponent();
	}
	UE_LOG(LogTemp, Warning, TEXT("Finished loading craft"));
	*/
}

TSharedPtr<FJsonObject> ACraft::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> Structure = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> PartsJson = MakeShareable(new FJsonObject());

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
	UE_LOG(LogTemp, Warning, TEXT("Cluster children before begin play %d"), GetClusterUnionComponent()->GetAttachChildren().Num());

	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("ClusterUnionComponent HasBegun status: %d"), GetClusterUnionComponent()->HasBegunPlay());

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
	UE_LOG(LogTemp, Warning, TEXT("Transfer %s"), *Part->Id);
	if (!FromCraft->Parts.Contains(Part->Id)) {
		// Part has already been moved???
		UE_LOG(LogTemp, Warning, TEXT("Transfer Failed %s"), *Part->Id);
		return;
	}

	Part->Detach();

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
		PartKVP.Value->Attach();
	}

	// NewCraft->SetRootComponent(Part->Mesh);
	NewCraft->PhysicsEnabled = PhysicsEnabled;
}

void ACraft::AttachPart(ACraft* SourceCraft, UPart* AttachToPart) {
	if (!SourceCraft->RootPart()) {
		UE_LOG(LogTemp, Warning, TEXT("Source craft has no root part!"));
		return;
	}

	// transfer
	UPart* Part = SourceCraft->RootPart();
	TransferPart(Part, SourceCraft, this);

	Part->SetParent(AttachToPart);

	for (auto& PartKVP : SourceCraft->Parts) {
		PartKVP.Value->Attach();
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
	UPart* Engine = RootPart();
	if (PhysicsEnabled && Engine && !Rotator.IsZero()) {
		FVector Axis = GetActorRotation().RotateVector(Rotator.Quaternion().GetRotationAxis());

		Engine->Mesh->AddTorqueInDegrees(Axis * Strength);
	}
}

void ACraft::SetPhysicsEnabled(bool enabled) {
	if (enabled == PhysicsEnabled) {
		return;
	}
	PhysicsEnabled = enabled;

	for (auto& PartKVP : Parts) {
		auto Part = PartKVP.Value;
		Part->SetPhysicsEnabled(PhysicsEnabled);
		// Part->SetSimulatePhysics(PhysicsEnabled);
	}
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
