// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SphereComponent.h"

#include "Common/JsonUtil.h"
#include "Simulation/CelestialBody.h"
#include "Simulation/OrbitComponent.h"
#include "Common/Craft/FuelComponent.h"

static auto DetachmentRule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
static auto AttachmentRule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);

// Sets default values
ACraft::ACraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USphereComponent* Root = CreateDefaultSubobject<USphereComponent>("Root");
	
	Root->SetLinearDamping(0);
	Root->SetAngularDamping(0);
	Root->SetMassOverrideInKg(NAME_None, 0);
	Root->SetMassScale(NAME_None, 0);
	Root->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Root->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	UE_LOG(LogTemp, Warning, TEXT("Sphere root mass %f"), Root->CalculateMass());
	SetRootComponent(Root);

	BaseEyeHeight = 0;
	PhysicsEnabled = false;
	RootPart = nullptr;

	JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/empty.json");

	OrbitComponent = CreateDefaultSubobject<UOrbitComponent>("OrbitComponent");
}

void ACraft::FromJson(TSharedPtr<FJsonObject> Json) {
	// structure + parts

	TArray<TPair<UPart*, TSharedPtr<FJsonObject>>> PartStructures = { { nullptr, Json->GetObjectField(TEXT("structure")) } };
	for (int i = 0; i < PartStructures.Num(); ++i) {
		for (auto& PartKVP : PartStructures[i].Value->Values) {
			UPart* Part = NewObject<UPart>(this, FName(PartKVP.Key));

			auto& PartListJson = Json->GetObjectField(TEXT("parts"));
			auto& PartJson = PartListJson->GetObjectField(PartKVP.Key);
			Part->Initialize(PartKVP.Key, PartKVP.Value->AsObject(), PartJson);
			Part->SetParent(PartStructures[i].Key);

			Parts.Add(PartKVP.Key, Part);

			PartStructures.Add({ Part, PartKVP.Value->AsObject() });
			UE_LOG(LogTemp, Warning, TEXT("created: %s"), *PartKVP.Key);

			Part->AttachToComponent(RootComponent, AttachmentRule);
		}
	}
	for (auto& PartKVP : Json->GetObjectField(TEXT("structure"))->Values) {
		RootPart = *Parts.Find(PartKVP.Key);
		break;
	}
	// stages
	for (auto& StageJson : Json->GetArrayField(TEXT("stages"))) {
		TArray<UPart*> Stage;
		for (auto& PartId : StageJson->AsArray()) {
			UPart* Part = *Parts.Find(PartId->AsString()); // TODO: error handling here
			Stage.Add(Part);
		}
		Stages.Add(Stage);
	}
}

TSharedPtr<FJsonObject> ACraft::ToJson() {
	TSharedPtr<FJsonObject> ResJson = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> Structure = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> PartsJson = MakeShareable(new FJsonObject());

	// structure + parts
	TArray<TPair<UPart*, TSharedPtr<FJsonObject>>> PartStructures;
	if (RootPart) {
		TSharedPtr<FJsonObject> RootPartJson = MakeShareable(new FJsonObject());
		Structure->SetObjectField(RootPart->Id, RootPartJson);
		PartStructures.Add({ RootPart, RootPartJson });
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

	ResJson->SetObjectField(TEXT("structure"), Structure);
	ResJson->SetObjectField(TEXT("parts"), PartsJson);

	// stages
	TArray<TSharedPtr<FJsonValue>> StagesJson;
	for (auto& Stage : Stages) {
		TArray<TSharedPtr<FJsonValue>> StageParts;
		for (auto& Part : Stage) {
			StageParts.Add(MakeShareable(new FJsonValueString(Part->Id)));
		}
		StagesJson.Add(MakeShareable(new FJsonValueArray(StageParts)));
		// StagesJson.Emplace(Json); // ? is this ok ?
	}
	ResJson->SetArrayField(TEXT("stages"), StagesJson);
	return ResJson;
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

	if (!PhysicsEnabled || OrbitComponent->CentralBody == nullptr) {
		// UE_LOG(LogTemp, Warning, TEXT("no grav"));
		return;
	}

	// Simple gravity force
	FVector GravityDirection = (GetActorLocation() - GetVelocity() * DeltaTime / 2) - OrbitComponent->CentralBody->GetActorLocation();
	double SquareDistance = GravityDirection.SquaredLength();
	GravityDirection.Normalize();
	for (auto& PartKVP : Parts) {
		UPart* Part = PartKVP.Value;
		// UE_LOG(LogTemp, Warning, TEXT("grav %f"), CentralBody->Mu / SquareDistance);
		Part->AddForce(GravityDirection * -OrbitComponent->CentralBody->Mu / SquareDistance, NAME_None, true);
	}
	Cast<USphereComponent>(RootComponent)->AddForce(GravityDirection * -OrbitComponent->CentralBody->Mu / SquareDistance, NAME_None, true);
	
	// calculate orbit
	OrbitComponent->UpdateOrbit(GetActorLocation(), GetVelocity());

	// Engine
	// use DeltaTime and AddImpulse, because we want to calculate fuel drain correctly, so the force must be applied based on the last frame
}

// Called to bind functionality to input
void ACraft::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACraft::SetAttachmentNodeVisibility(bool visibility) {
	for (auto& part : Parts) {
		part.Value->SetAttachmentNodeVisibility(visibility);
	}
}

void ACraft::AddPart(UPart* Part) {
	Parts.Add(Part->Id, Part);
	// Json->GetObjectField(TEXT("parts"))->SetObjectField(Part->Id, Part->Json);
	// change ownership
	Part->Rename(*Part->Id, this);
}

void ACraft::RemovePart(UPart* Part) {
	Parts.Remove(Part->Id);

	ActiveEngines.Remove(Part);
	ActiveFuelTanks.Remove(Part);

	// Json->GetObjectField(TEXT("parts"))->RemoveField(Part->Id);
}

// transfer ownership of part and any children to another craft
void ACraft::TransferPart(UPart* Part, ACraft* FromCraft, ACraft* ToCraft) {
	UE_LOG(LogTemp, Warning, TEXT("Transfer %s"), *Part->Id);
	if (!FromCraft->Parts.Contains(Part->Id)) {
		// Part has already been moved???
		UE_LOG(LogTemp, Warning, TEXT("Transfer Failed %s"), *Part->Id);
		return;
	}
	FromCraft->RemovePart(Part);
	ToCraft->AddPart(Part);

	// JsonUtil::Vector(Part->Json, TEXT("location"), (Part->GetComponentLocation() - ToCraft->RootPart->GetComponentLocation()));

	for (auto& Child : Part->Children) {
		TransferPart(Child, FromCraft, ToCraft);
	}
}

void ACraft::DetachPart(UPart* Part, ACraft* NewCraft) {
	TSharedPtr<FJsonObject> CraftJson = JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/empty.json");
	if (!CraftJson.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("invalid new craft"));
		return;
	}
	
	NewCraft->FromJson(CraftJson);
	// NewCraft->Json->SetStringField(L"name", "sub craft");
	// NewCraft->Json->GetObjectField(L"structure")->SetObjectField(Part->Id, Part->Structure);

	NewCraft->SetActorLocation(Part->GetComponentLocation());

	TransferPart(Part, this, NewCraft);
	NewCraft->RootPart = Part;

	Part->SetParent(nullptr);

	NewCraft->PhysicsEnabled = PhysicsEnabled;
}

void ACraft::AttachPart(ACraft* SourceCraft, UPart* AttachToPart) {
	if (!SourceCraft->RootPart) {
		UE_LOG(LogTemp, Warning, TEXT("Source craft has no root part!"));
		return;
	}

	// avoid name collisions
	for (auto& PartKVP : SourceCraft->Parts) {
		FString OriginalKey = PartKVP.Key;
		UPart* Part = PartKVP.Value;
		int i = 0;
		while (Parts.Contains(Part->Id)) {
			Part->Id = FString::Printf(TEXT("%s-%d"), *Part->Type, i); // increment and set new id
			++i;
		}
		if (OriginalKey != Part->Id) {
			SourceCraft->Parts.Remove(OriginalKey);
			SourceCraft->Parts.Add(Part->Id, Part);
		}
	}

	UPart* SourceRoot = SourceCraft->RootPart;
	TransferPart(SourceRoot, SourceCraft, this);
	SourceRoot->SetParent(AttachToPart);

	// transfer stages
	for (auto& Stage : SourceCraft->Stages) {
		Stages.Add(Stage);
	}

	if (SourceCraft->Parts.IsEmpty()) {
		SourceCraft->Destroy();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Source craft should be empty but is not!"));
	}
}

void ACraft::Throttle(float throttle) {
	if (!PhysicsEnabled) {
		return;
	}

	double FuelDrain = 0;
	for (auto& Engine : ActiveEngines) {
		// FuelDrain += Engine.FuelDrain;
	}

	FuelState FuelTotal;
	for (auto& FuelTank : ActiveFuelTanks) {
		
		// FuelTotal += FuelTank.Fuel;
	}

	double ThrustPercent = FMath::Min(FuelTotal.FindChecked(FuelType::LiquidFuel) / FuelDrain, throttle);


	FVector thrust = FVector(0, 0, 700000 * throttle);
	thrust = RootPart->GetComponentRotation().RotateVector(thrust);
	RootPart->AddForce(thrust);

	// update orbit
	OrbitComponent->UpdateOrbit(GetActorLocation(), GetVelocity());
}

void ACraft::Rotate(FRotator rotator, float strength) {
	UPart* Engine = RootPart;
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

	if (enabled) {
		for (auto& PartKVP : Parts) {
			auto Part = PartKVP.Value;
			Part->DetachFromComponent(DetachmentRule);
			Part->SetSimulatePhysics(true);
		}
		UPrimitiveComponent* RC = Cast<UPrimitiveComponent>(RootComponent);
		RC->SetSimulatePhysics(true);
		RootPart->Physics->SetConstrainedComponents(RootPart, "", RC, "");
	}
	else {
		RootPart->Physics->BreakConstraint();
		for (auto& PartKVP : Parts) {
			auto Part = PartKVP.Value;
			Part->SetSimulatePhysics(false);
			Part->AttachToComponent(RootComponent, AttachmentRule);
		}
		UPrimitiveComponent* RC = Cast<UPrimitiveComponent>(RootComponent);
		RC->SetSimulatePhysics(false);
	}
	UE_LOG(LogTemp, Warning, TEXT("root loc %s"), *GetRootComponent()->GetComponentLocation().ToString());
}


TArray<ACraft*> ACraft::StageCraft() {
	TArray<ACraft*> Detached;
	for (auto& Part : Stages[0]) {
		if (Part->Type == "decoupler") {
			ACraft* Craft = NewObject<ACraft>();
			Detached.Add(Craft);
			DetachPart(Part, Craft);
		}
		else if (Part->Type == "engine") {
			ActiveEngines.Add(Part);
		}
		else if (Part->Type == "fuel_tank") {
			ActiveFuelTanks.Add(Part);
		}
	}
	Stages.RemoveAt(0);
	return Detached;
}

FVector ACraft::GetAngularVelocity() {
	return RootPart->GetPhysicsAngularVelocityInRadians();
}
