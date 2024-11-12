// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Part.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"


static auto DetachmentRule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
static auto AttachmentRule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);

UPart::UPart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

	Parent = nullptr;
	Children = TArray<UPart*>();
	FCollisionResponseContainer();
		// TODO: add custom collision channel for each craft, no intervessel collision

	SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// TODO: this should be at the position of the attachment node, maybe then the limit can be smaller
	// and also add a drive to push the connection back towards to default position
	Physics = CreateDefaultSubobject<UPhysicsConstraintComponent>("Link");
	Physics->SetRelativeLocation(FVector(0));
	Physics->SetupAttachment(this);
	Physics->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.1);
	Physics->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.1);
	Physics->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.1);
	Physics->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.015);
	Physics->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.015);
	Physics->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.015);

	SetLinearDamping(0);
	SetAngularDamping(0);

	// static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> PhysMaterial(TEXT("/Game/Simulation/Rubber"));
	// UPhysicalMaterial* PhysMaterial = NewObject<UPhysicalMaterial>();
	// PhysMaterial->Friction = 1;

	// SetPhysMaterialOverride(PhysMaterial);
}

// Sets default values
void UPart::Initialize(FString InId, TSharedPtr<FJsonObject> InStructure, TSharedPtr<FJsonObject> InJson)
{
	Id = InId;
	
	// set locaiton and scale
	FromJson(InJson);
	
	RegisterComponent();
	Physics->RegisterComponent();
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
	
	// Detach();

	Parent = NewParent;
	if (Parent != nullptr) {
		Parent->Children.Add(this);
	}

	// Attach();
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

void UPart::Detach() {
	if (PhysicsEnabled) {
		Physics->BreakConstraint();
	}
	else {
		DetachFromComponent(DetachmentRule);
	}
}

void UPart::Attach() {
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
}


void UPart::FromJson(TSharedPtr<FJsonObject> InJson) {
	Type = InJson->GetStringField(TEXT("type"));

	SetRelativeLocation(JsonUtil::Vector(InJson, "location"));
	SetRelativeRotation(JsonUtil::Rotator(InJson, "rotation"));
	SetRelativeScale3D(JsonUtil::Vector(InJson, "scale"));

	FString MeshPath = UAssetLibrary::PartDefinition(Type)->GetStringField(TEXT("mesh"));

	UStaticMesh* Mesh = UAssetLibrary::LoadAsset<UStaticMesh>(*MeshPath);
	if (Mesh != nullptr) {
		SetStaticMesh(Mesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("failed to load mesh for: %s"), *Id);
	}
}

TSharedPtr<FJsonObject> UPart::ToJson() {
	TSharedPtr<FJsonObject> OutJson = MakeShareable(new FJsonObject());

	AActor* Owner = GetOwner();

	OutJson->SetStringField(TEXT("type"), Type);
	JsonUtil::Vector(OutJson, "location", GetComponentLocation() - Owner->GetActorLocation());
	JsonUtil::Rotator(OutJson, "rotation", GetComponentRotation() - Owner->GetActorRotation());
	JsonUtil::Vector(OutJson, "scale", GetRelativeScale3D());

	return OutJson;
}