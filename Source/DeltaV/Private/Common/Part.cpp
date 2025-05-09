// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Part.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"

// 
#include "MeshDescriptionToDynamicMesh.h"
#include "DynamicMeshToMeshDescription.h" // can always use this

#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/Craft.h"

#include "Common/Craft/FuelComponent.h"
#include "Common/Craft/EngineComponent.h"
#include "Common/Craft/AeroCompoenent.h"
#include "Common/Craft/WheelComponent.h"


static auto DetachmentRule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
static auto AttachmentRule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);

static TMap<FString, TSubclassOf<UPartComponent>> AdditionalFields;

UPart::UPart(const FObjectInitializer& ObjectInitializer) : UActorComponent(ObjectInitializer) {
	if (AdditionalFields.Num() == 0) {
		AdditionalFields.Add("fuel", UFuelComponent::StaticClass());
		AdditionalFields.Add("aero", UAeroCompoenent::StaticClass());
		AdditionalFields.Add("engine", UEngineComponent::StaticClass());
		AdditionalFields.Add("wheel", UWheelComponent::StaticClass());
	}

	Parent = nullptr;
	Children = TArray<UPart*>();
	FCollisionResponseContainer();

	Craft = Cast<ACraft>(GetOwner());
	
	// TODO: this should be at the position of the attachment node, maybe then the limit can be smaller
	// and also add a drive to push the connection back towards to default position
	Physics = CreateDefaultSubobject<UPhysicsConstraintComponent>("Link");
	Physics->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0);
	Physics->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0);
	Physics->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0);

	// controls how noodly the craft is
	Physics->SetLinearDriveParams(1e2, 1e2, 1e6);
	Physics->SetLinearPositionTarget(FVector(0));
	Physics->SetLinearPositionDrive(true, true, true);
	//Physics->SetLinearVelocityTarget(FVector(0));
	//Physics->SetLinearVelocityDrive(true, true, true);

	PhysicsEnabled = false;

	// static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> PhysMaterial(TEXT("/Game/Simulation/Rubber"));
	// UPhysicalMaterial* PhysMaterial = NewObject<UPhysicalMaterial>();
	// PhysMaterial->Friction = 1;

	// SetPhysMaterialOverride(PhysMaterial);
	Id = GetName();
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

void UPart::BeginPlay() {
	Super::BeginPlay();

	for (auto& FieldKVP : AdditionalComponents) {
		FieldKVP.Value->RegisterComponent();
	}
	Mesh->RegisterComponent();
	Physics->RegisterComponent();
}

void UPart::Detach() {
	if (PhysicsEnabled) {
		Physics->BreakConstraint();
	}
	else {
		Mesh->DetachFromComponent(DetachmentRule);
	}
}

void UPart::Attach() {
	if (Parent == nullptr) {
		return;
	}

	if (PhysicsEnabled) {
		Physics->SetConstrainedComponents(Mesh, Bone, Parent->Mesh, Parent->Bone);
	}
	else {
		Mesh->AttachToComponent(Parent->Mesh, AttachmentRule);
	}
}

void UPart::SetPhysicsEnabled(bool bSimulate) {
	if (PhysicsEnabled == bSimulate) {
		return;
	}
	Detach();
	PhysicsEnabled = bSimulate;
	Attach();
	
	for (auto& FieldKVP : AdditionalComponents) {
		FieldKVP.Value->SetPhysicsEnabled(bSimulate);
	}

	Mesh->SetSimulatePhysics(PhysicsEnabled);
}

void UPart::FromJson(TSharedPtr<FJsonObject> Json) {
	Type = Json->GetStringField(TEXT("type"));

	Physics->SetRelativeLocation(JsonUtil::Vector(Json, "attach_location"));

	TSharedPtr<FJsonObject> PartDefinition = UAssetLibrary::PartDefinition(Type);
	FString MeshPath = PartDefinition->GetStringField(TEXT("mesh"));
	FString MeshTypeString = PartDefinition->GetStringField(TEXT("type"));

	if (MeshTypeString == "static") {
		TObjectPtr<UStaticMesh> StaticMesh = UAssetLibrary::LoadAsset<UStaticMesh>(*MeshPath);
		TObjectPtr<UStaticMeshComponent> MeshObj = NewObject<UStaticMeshComponent>(this);
		MeshObj->SetStaticMesh(StaticMesh);
		Mesh = MeshObj;
		Bone = FName("");
		MeshType = STATIC_MESH;
	}
	else if (MeshTypeString == "skeletal") {
		TObjectPtr<USkeletalMesh> SkeletalMesh = UAssetLibrary::LoadAsset<USkeletalMesh>(*MeshPath);
		TObjectPtr<USkeletalMeshComponent> MeshObj = NewObject<USkeletalMeshComponent>(this);
		MeshObj->SetSkeletalMesh(SkeletalMesh);
		Mesh = MeshObj;
		Bone = MeshObj->GetBoneName(0);
		MeshType = SKELETAL_MESH;
	}

	Physics->AttachToComponent(Mesh, AttachmentRule);
	// TODO: add custom collision channel for each craft, no intervessel collision
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetLinearDamping(0);
	Mesh->SetAngularDamping(0);
	Mesh->SetAbsolute(false, false, true);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel11);

	Mesh->SetRelativeLocation(JsonUtil::Vector(Json, "location"));
	Mesh->SetWorldRotation(JsonUtil::Rotator(Json, "rotation"));
	Mesh->SetWorldScale3D(JsonUtil::Vector(Json, "scale"));

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
	JsonUtil::Vector(Json, "location", Mesh->GetComponentLocation() - Owner->GetActorLocation());
	JsonUtil::Rotator(Json, "rotation", Mesh->GetComponentRotation());
	JsonUtil::Vector(Json, "scale", Mesh->GetRelativeScale3D());
	JsonUtil::Vector(Json, "attach_location", Physics->GetRelativeLocation());

	for (auto& FieldKVP : AdditionalComponents) {
		Json->SetObjectField(FieldKVP.Key, FieldKVP.Value->ToJson());
	}

	return Json;
}

/*
UPartComponent* UPart::GetComponent(FString Name) {
	UPartComponent** Component = AdditionalComponents.Find(Name);
	if (Component == nullptr) {
		return nullptr;
	}
	else {
		return *Component;
	}
}*/


FMeshDescription* UPart::CopyMeshToDynamicMesh(TObjectPtr<UDynamicMesh> DynamicMesh, int LOD) {
	FGeometryScriptCopyMeshFromAssetOptions AssetOptions;
	FGeometryScriptMeshReadLOD TargetLOD;
	TargetLOD.LODIndex = LOD;
	EGeometryScriptOutcomePins OutResult;

	switch (MeshType)
	{
	case STATIC_MESH: {
		TObjectPtr<UStaticMesh> StaticMesh = Cast<UStaticMeshComponent>(Mesh)->GetStaticMesh();
		if (DynamicMesh) {
			UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(
				StaticMesh, DynamicMesh, AssetOptions, TargetLOD, OutResult);
		}
		return StaticMesh->GetMeshDescription(LOD);
	} break;
	case SKELETAL_MESH: {
		TObjectPtr<USkeletalMesh> SkeletalMesh = Cast<USkeletalMeshComponent>(Mesh)->GetSkeletalMeshAsset();
		if (DynamicMesh) {
			UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh(
				SkeletalMesh, DynamicMesh, AssetOptions, TargetLOD, OutResult);
		}
		return SkeletalMesh->GetMeshDescription(LOD);
	} break;
	default:
		break;
	}
	return nullptr;
}
