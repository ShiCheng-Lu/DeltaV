// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SphereComponent.h"

#include "Common/JsonUtil.h"
#include "Simulation/CelestialBody.h"
#include "Simulation/OrbitComponent.h"
#include "Simulation/SimulationController.h"
#include "Common/Craft/FuelComponent.h"

// Sets default values
ACraft::ACraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	PostPhysics(ETickingGroup::TG_PostPhysics, &ACraft::TickPostPhysics)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseEyeHeight = 0;
	PhysicsEnabled = false;

	JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/empty.json");

	Orbit = CreateDefaultSubobject<UOrbitComponent>("OrbitComponent");
}

void ACraft::FromJson(TSharedPtr<FJsonObject> Json) {
	// structure + parts

	// Array of (Parent, ChildJson[])
	auto& PartListJson = Json->GetObjectField(TEXT("parts"));
	TArray<TPair<UPart*, TSharedPtr<FJsonObject>>> PartStructures = { { nullptr, Json->GetObjectField(TEXT("structure")) } };
	for (int i = 0; i < PartStructures.Num(); ++i) {
		for (auto& PartKVP : PartStructures[i].Value->Values) {
			UPart* Part = NewObject<UPart>(this, FName(PartKVP.Key));

			auto& PartJson = PartListJson->GetObjectField(PartKVP.Key);
			Part->Initialize(PartKVP.Key, PartKVP.Value->AsObject(), PartJson);
			Part->SetParent(PartStructures[i].Key);

			Parts.Add(PartKVP.Key, Part);

			PartStructures.Add({ Part, PartKVP.Value->AsObject() });
			UE_LOG(LogTemp, Warning, TEXT("created: %s"), *PartKVP.Key);
		}
	}

	for (auto& PartKVP : PartStructures[0].Value->Values) {
		SetRootComponent(*Parts.Find(PartKVP.Key));
		break;
	}
	// stages
	for (auto& StageJson : Json->GetArrayField(TEXT("stages"))) {
		UStage* Stage = NewObject<UStage>(this);
		Stage->FromJson(StageJson);
		Stages.Add(Stage);
	}

	for (auto& PartKVP : Parts) {
		PartKVP.Value->RegisterComponent();
	}
	UE_LOG(LogTemp, Warning, TEXT("Finished loading craft"));
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
	TArray<TSharedPtr<FJsonValue>> StagesJson;
	for (UStage* Stage : Stages) {
		StagesJson.Add(Stage->ToJson());
	}
	Json->SetArrayField(TEXT("stages"), StagesJson);

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
			Part->AddForce(Gravity, NAME_None, true);
		}
	}
	
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

	if (!PhysicsEnabled && Orbit->CentralBody == nullptr) {
		return; // In build mode
	}

	if (Orbit->CentralBody == nullptr) {
		return;
	}

	// TODO: Optimize, call Orbit->GetTrueAnomaly less as it's a loop

	// Updating orbit
	FVector PositionChange = GetWorldCoM() - TargetPosition;
	FVector VelocityChange = GetVelocity() - TargetVelocity;
	if (!VelocityChange.IsNearlyZero()) {
		FVector Velocity;
		double Time = GetGameTimeSinceCreation() + DeltaTime;
		double TrueAnomaly = Orbit->GetTrueAnomaly(Time);
		Orbit->GetPositionAndVelocity(nullptr, &Velocity, TrueAnomaly);

		UE_LOG(LogTemp, Warning, TEXT("Velocity didn't get there - %f"), VelocityChange.Length());

		Orbit->UpdateOrbit(GetWorldCoM() - Orbit->CentralBody->GetActorLocation(), GetVelocity(), Time);
		// Orbit->UpdateOrbit(GetActorLocation() - Orbit->CentralBody->GetActorLocation(), VelocityChange + Velocity, Time);
	}
	 
	if (!PositionChange.IsNearlyZero()) {
		UE_LOG(LogTemp, Warning, TEXT("Position didn't get there - %f"), PositionChange.Length());

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
	NewCraft->SetActorLocation(Part->GetComponentLocation());

	// transfer
	TransferPart(Part, this, NewCraft);

	Part->SetParent(nullptr);

	// complete
	for (auto& PartKVP : NewCraft->Parts) {
		Parts.Remove(PartKVP.Key);
		PartKVP.Value->Attach();
	}

	NewCraft->SetRootComponent(Part);
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

	// transfer stages
	for (auto& Stage : SourceCraft->Stages) {
		Stages.Add(Stage);
	}

	SourceCraft->Parts.Empty();
	SourceCraft->SetRootComponent(nullptr);
	SourceCraft->Destroy();
}

void ACraft::Rotate(FRotator rotator, float strength) {
	UPart* Engine = RootPart();
	if (PhysicsEnabled && Engine && !rotator.IsZero()) {
		FVector rotation_axis = GetActorRotation().RotateVector(rotator.Quaternion().GetRotationAxis());

		Engine->AddTorqueInDegrees(rotation_axis * strength);
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
		CenterOfMass += Part->GetRelativeLocation() * Part->CalculateMass();
		Mass += Part->CalculateMass();
	}
	return CenterOfMass / Mass;
}


FVector ACraft::GetWorldCoM() {
	// center of mass relative to root
	double Mass = 0;
	FVector CenterOfMass = FVector(0);
	for (auto PartKVP : Parts) {
		auto Part = PartKVP.Value;
		CenterOfMass += Part->GetComponentLocation() * Part->CalculateMass();
		Mass += Part->CalculateMass();
	}
	return CenterOfMass / Mass;
}

TArray<ACraft*> ACraft::StageCraft() {
	UStage* Stage = Stages.Pop();
	if (!Stage) {
		return TArray<ACraft*>();
	}
	return Stage->Activate();
}

FVector ACraft::GetAngularVelocity() {
	return RootPart()->GetPhysicsAngularVelocityInRadians();
}
