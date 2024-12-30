// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Part.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"

#include "Common/Craft/FuelComponent.h"
#include "Common/Craft/EngineComponent.h"
#include "Common/Craft/AeroCompoenent.h"


static auto DetachmentRule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
static auto AttachmentRule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);

static TMap<FString, TSubclassOf<UPartComponent>> AdditionalFields;

UPart::UPart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	if (AdditionalFields.Num() == 0) {
		AdditionalFields.Add("fuel", UFuelComponent::StaticClass());
		AdditionalFields.Add("aero", UAeroCompoenent::StaticClass());
		AdditionalFields.Add("engine", UEngineComponent::StaticClass());
	}

	Parent = nullptr;
	Children = TArray<UPart*>();
	FCollisionResponseContainer();
	
	// TODO: add custom collision channel for each craft, no intervessel collision

	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// TODO: this should be at the position of the attachment node, maybe then the limit can be smaller
	// and also add a drive to push the connection back towards to default position
	Physics = CreateDefaultSubobject<UPhysicsConstraintComponent>("Link");
	Physics->SetupAttachment(this);
	
	Physics->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0);
	Physics->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0);
	Physics->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0);
	
	SetLinearDamping(0);
	SetAngularDamping(0);

	SetAbsolute(false, false, true);

	PhysicsEnabled = false;

	// static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> PhysMaterial(TEXT("/Game/Simulation/Rubber"));
	// UPhysicalMaterial* PhysMaterial = NewObject<UPhysicalMaterial>();
	// PhysMaterial->Friction = 1;

	// SetPhysMaterialOverride(PhysMaterial);
}

// Sets default values
void UPart::Initialize(FString InId, TSharedPtr<FJsonObject> InStructure, TSharedPtr<FJsonObject> Json)
{
	Id = InId;
	
	// set locaiton and scale
	FromJson(Json);

	SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore);
	SetCollisionObjectType(ECC_GameTraceChannel11);

	SetSimulatePhysics(true);
}

void UPart::SetAttachmentNodeVisibility(bool visibility) {
	for (auto node : AttachmentNodes) {
		node->SetVisibility(visibility);
	}
}

void UPart::SetParent(UPart* NewParent) {
	if (NewParent == this) {
		UE_LOG(LogTemp, Warning, TEXT("Parenting self, not allowed"));
		return;
	}

	if (Parent != nullptr) {
		Parent->Children.Remove(this);

	}

Detach();

Parent = NewParent;
if (Parent != nullptr) {
	Parent->Children.Add(this);
}

Attach();
}

void UPart::SetSimulatePhysics(bool bSimulate) {
	// UE_LOG(LogTemp, Warning, TEXT("phys constraint: %s, %s"), *Id, *Physics->GetComponentLocation().ToString());
	/*
	if (Parent == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("No parent"));
		DetachFromComponent(DetachmentRule);
		Physics->BreakConstraint();
	}
	else if (bSimulate) {
		DetachFromComponent(DetachmentRule);
		Physics->SetConstrainedComponents(this, "", Parent, "");
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Yes parent"));
		UE_LOG(LogTemp, Warning, TEXT("asdas"));
		Physics->BreakConstraint();
		AttachToComponent(Parent, AttachmentRule);
	}*/
	Super::SetSimulatePhysics(bSimulate);
}

void UPart::BeginPlay() {
	Super::BeginPlay();

	for (auto& FieldKVP : AdditionalComponents) {
		FieldKVP.Value->RegisterComponent();
	}
	Physics->RegisterComponent();
}

void UPart::Detach() {
	if (PhysicsEnabled) {
		Physics->BreakConstraint();
	}
	else {
		DetachFromComponent(DetachmentRule);
	}
}

void UPart::Attach() {
	if (Parent == nullptr) {
		return;
	}

	if (PhysicsEnabled) {
		Physics->SetConstrainedComponents(this, "", Parent, "");
	}
	else {
		AttachToComponent(Parent, AttachmentRule);
	}
}

void UPart::SetPhysicsEnabled(bool bSimulate) {
	if (PhysicsEnabled == bSimulate) {
		return;
	}
	Detach();
	PhysicsEnabled = bSimulate;
	Attach();
	SetSimulatePhysics(PhysicsEnabled);
}

void UPart::FromJson(TSharedPtr<FJsonObject> Json) {
	Type = Json->GetStringField(TEXT("type"));

	SetRelativeLocation(JsonUtil::Vector(Json, "location"));
	SetWorldRotation(JsonUtil::Rotator(Json, "rotation"));
	SetWorldScale3D(JsonUtil::Vector(Json, "scale"));

	Physics->SetRelativeLocation(JsonUtil::Vector(Json, "attach_location"));

	FString MeshPath = UAssetLibrary::PartDefinition(Type)->GetStringField(TEXT("mesh"));

	UStaticMesh* Mesh = UAssetLibrary::LoadAsset<UStaticMesh>(*MeshPath);
	if (Mesh != nullptr) {
		SetStaticMesh(Mesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("failed to load mesh for: %s"), *Id);
	}

	for (auto& FieldKVP : AdditionalFields) {
		if (Json->HasTypedField(FieldKVP.Key, EJson::Object)) {
			UPartComponent* AdditionalComponent = NewObject<UPartComponent>(this, FieldKVP.Value, FName(FieldKVP.Key));
			AdditionalComponent->FromJson(Json->GetObjectField(FieldKVP.Key));
			AdditionalComponents.Add(FieldKVP.Key, AdditionalComponent);
		}
	}
}

TSharedPtr<FJsonObject> UPart::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	AActor* Owner = GetOwner();

	Json->SetStringField(TEXT("type"), Type);
	JsonUtil::Vector(Json, "location", GetComponentLocation() - Owner->GetActorLocation());
	JsonUtil::Rotator(Json, "rotation", GetComponentRotation());
	JsonUtil::Vector(Json, "scale", GetRelativeScale3D());
	JsonUtil::Vector(Json, "attach_location", Physics->GetRelativeLocation());

	for (auto& FieldKVP : AdditionalComponents) {
		Json->SetObjectField(FieldKVP.Key, FieldKVP.Value->ToJson());
	}

	return Json;
}

UPartComponent* UPart::GetComponent(FString Name) {
	UPartComponent** Component = AdditionalComponents.Find(Name);
	if (Component == nullptr) {
		return nullptr;
	}
	else {
		return *Component;
	}
}