// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SphereComponent.h"

#include "Common/JsonUtil.h"

static auto DetachmentRule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
static auto AttachmentRule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);

// Sets default values
ACraft::ACraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USphereComponent* Root = CreateDefaultSubobject<USphereComponent>("Root");
	SetRootComponent(Root);

	BaseEyeHeight = 0;
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
}

// Called to bind functionality to input
void ACraft::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Sets default values
void ACraft::Initialize(TSharedPtr<FJsonObject> InJson)
{
	Json = InJson;

	TArray<TPair<UPart*, TSharedPtr<FJsonObject>>> PartStructures = { { nullptr, Json->GetObjectField(TEXT("structure")) } };
	for (int i = 0; i < PartStructures.Num(); ++i) {
		for (auto& PartKVP : PartStructures[i].Value->Values) {
			auto Part = NewObject<UPart>(this);

			auto& PartListJson = Json->GetObjectField(L"parts");
			auto& PartJson = PartListJson->GetObjectField(PartKVP.Key);
			Part->Initialize(PartKVP.Key, PartKVP.Value->AsObject(), PartJson);
			Part->SetParent(PartStructures[i].Key);

			Parts.Add(PartKVP.Key, Part);

			PartStructures.Add({ Part, PartKVP.Value->AsObject() });
			UE_LOG(LogTemp, Warning, TEXT("created: %s"), *PartKVP.Key);

			Part->AttachToComponent(RootComponent, AttachmentRule);
		}
	}

	RootPart = *Parts.Find(PartStructures[0].Value->Values.begin().Key());

	if (Parts.Find("engine-1")) {
		Engine = *Parts.Find("engine-1");
	}
}

void ACraft::SetAttachmentNodeVisibility(bool visibility) {
	for (auto& part : Parts) {
		part.Value->SetAttachmentNodeVisibility(visibility);
	}
}

void ACraft::AddPart(UPart* Part) {
	Parts.Add(Part->Id, Part);
	Json->GetObjectField(L"parts")->SetObjectField(Part->Id, Part->Json);
	// change ownership
	Part->Rename(*Part->GetName(), this);
}

void ACraft::RemovePart(UPart* Part) {
	Parts.Remove(Part->Id);
	Json->GetObjectField(L"parts")->RemoveField(Part->Id);
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

	Part->Json->SetArrayField(L"location", JsonUtil::Vector(Part->GetComponentLocation() - ToCraft->RootPart->GetComponentLocation()));

	for (auto& Child : Part->Children) {
		TransferPart(Child, FromCraft, ToCraft);
	}
}

void ACraft::DetachPart(UPart* Part, ACraft* NewCraft) {
	NewCraft->RootPart = Part;
	NewCraft->Json = MakeShareable(new FJsonObject());
	NewCraft->Json->SetStringField(L"name", "sub craft");
	TSharedPtr<FJsonObject> structure = MakeShareable(new FJsonObject());
	structure->SetObjectField(Part->Id, Part->Structure);
	NewCraft->Json->SetObjectField(L"structure", structure);
	NewCraft->Json->SetObjectField(L"parts", MakeShareable(new FJsonObject()));

	NewCraft->SetActorLocation(Part->GetComponentLocation());

	TransferPart(Part, this, NewCraft);
	
	Part->SetParent(nullptr);
	NewCraft->SetRootComponent(Part);
}

void ACraft::AttachPart(ACraft* SourceCraft, UPart* AttachToPart) {
	TransferPart(SourceCraft->RootPart, SourceCraft, this);

	SourceCraft->RootPart->SetParent(AttachToPart);

	if (SourceCraft->Parts.IsEmpty()) {
		SourceCraft->Destroy();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Source craft should be empty but is not!"));
	}
}

void ACraft::Throttle(float throttle) {
	if (Engine) {
		FVector thrust = FVector(0, 0, 50000 * throttle);
		thrust = Engine->GetComponentRotation().RotateVector(thrust);
		Engine->AddForce(thrust);
	}
}

void ACraft::Rotate(FRotator rotator, float strength) {
	if (Engine) {
		FRotator shortest_rotation = rotator;
		FQuat quaternion = shortest_rotation.Quaternion();

		FVector rotation_axis = GetActorRotation().RotateVector(quaternion.GetRotationAxis());

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
