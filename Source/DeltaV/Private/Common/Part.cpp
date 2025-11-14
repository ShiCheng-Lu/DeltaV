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

#include "GeometryCollection/GeometryCollection.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/Craft.h"

#include "Common/Craft/FuelComponent.h"
#include "Common/Craft/EngineComponent.h"
#include "Common/Craft/AeroComponent.h"
#include "Common/Craft/WheelComponent.h"

#include "ChaosModularVehicle/VehicleSimComponentsInclude.h"
#include "ModularVehicle/VehicleSimThruster2DComponent.h"

static TMap<FString, TSubclassOf<UPartComponent>> AdditionalFields = {
	{"fuel", UFuelComponent::StaticClass()},
	{"aero", UAeroComponent::StaticClass()},
	{"engine", UEngineComponent::StaticClass()},
	{"wheel", UWheelComponent::StaticClass()},
};

UPart::UPart(const FObjectInitializer& ObjectInitializer) : UActorComponent(ObjectInitializer) {
	Parent = nullptr;
	Children = TArray<UPart*>();

	Craft = GetOwner<ACraft>();
	
	Id = GetName();
}

void UPart::SetAttachmentNodeVisibility(bool visibility) {
	for (auto node : AttachmentNodes) {
		node->SetVisibility(visibility);
	}
}

void UPart::SetParent(UPrimitiveComponent* NewParent) {
	
	UPart* PartParent = nullptr;// Cast<UPart>(NewParent);
	if (PartParent == this) {
		UE_LOG(LogTemp, Warning, TEXT("Parenting self, not allowed"));
		return;
	}

	if (Parent != nullptr) {
		Parent->Children.Remove(this);

		Mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	}

	Parent = PartParent;
	if (Parent != nullptr) {
		Parent->Children.Add(this);

		Mesh->AttachToComponent(Parent->Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	}

}

void UPart::BeginPlay() {
	Super::BeginPlay();

	for (auto& FieldKVP : AdditionalComponents) {
		FieldKVP.Value->RegisterComponent();
	}
	Mesh->RegisterComponent();
	Physics->RegisterComponent();
}

void UPart::FromJson(TSharedPtr<FJsonObject> Json) {
	Type = Json->GetStringField(TEXT("type"));

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
		Bone = MeshObj->GetBoneName(1);

		FString BoneName = Bone.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Bone: %s"), *BoneName);
		MeshType = SKELETAL_MESH;
	}
	else if (MeshTypeString == "geometry_collection") {
		auto* GeometryCollection = UAssetLibrary::LoadAsset<UGeometryCollection>(*MeshPath);
		auto* Component = NewObject<UGeometryCollectionComponent>(this);
		Component->SetRestCollection(GeometryCollection);
		Component->DamageThreshold = { 1e8 };

		UE_LOG(LogTemp, Warning, TEXT("Creating geometry collection"));

		Mesh = Component;
		Bone = FName("");

		MeshType = GEOMETRY_COLLECTION;
	}

	// Physics->AttachToComponent(Mesh, AttachmentRule);
	// TODO: add custom collision channel for each craft, no intervessel collision
	/*
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetLinearDamping(0);
	Mesh->SetAngularDamping(0);
	*/

	Mesh->SetAbsolute(false, false, true);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel11);
	
	Mesh->SetRelativeLocation(JsonUtil::Vector(Json, "location"));
	Mesh->SetWorldRotation(JsonUtil::Rotator(Json, "rotation"));
	Mesh->SetWorldScale3D(JsonUtil::Vector(Json, "scale"));

	/*
	for (auto& FieldKVP : AdditionalFields) {
		if (Json->HasTypedField(FieldKVP.Key, EJson::Object)) {
			UPartComponent* AdditionalComponent = NewObject<UPartComponent>(this, FieldKVP.Value, FName(FieldKVP.Key));
			AdditionalComponent->FromJson(Json->GetObjectField(FieldKVP.Key));
			AdditionalComponents.Add(FieldKVP.Key, AdditionalComponent);
		}
	}
	*/

	const TSharedPtr<FJsonObject>* SpecialJson;
	if (Json->TryGetObjectField(TEXT("wheel"), SpecialJson)) {
		auto& WheelJson = *SpecialJson;

		auto* Suspension = NewObject<UVehicleSimSuspensionComponent>(GetOwner());
		Suspension->SuspensionMaxDrop = 100;
		Suspension->SuspensionMaxRaise = 100;
		Suspension->SpringRate = 200;
		Suspension->SpringPreload = 100;
		// Suspension->SuspensionForceEffect = 0;
		Suspension->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		Suspension->RegisterComponent();

		auto* Wheel = NewObject<UVehicleSimWheelComponent>(GetOwner());
		Wheel->MaxSteeringAngle = WheelJson->GetNumberField(TEXT("max_steering"));
		Wheel->WheelRadius = 100;
		Wheel->AxisType = EWheelAxisType::X;
		Wheel->MaxSteeringAngle *= -1;
		Wheel->bSteeringEnabled = true;
		Wheel->AttachToComponent(Suspension, FAttachmentTransformRules::KeepRelativeTransform);
		Wheel->RegisterComponent();
	} else if (Json->TryGetObjectField(TEXT("thruster"), SpecialJson)) {
		auto& ThrusterJson = *SpecialJson;

		auto* Thruster = NewObject<UVehicleSimThruster2DComponent>(GetOwner());
		Thruster->bSteeringEnabled = true;
		Thruster->MaxThrustForce = 2000000.0f;
		Thruster->MaxSteeringAngle = 20;
		Thruster->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		Thruster->RegisterComponent();
	}
	else {
		auto* Chassis = NewObject<UVehicleSimChassisComponent>(GetOwner());
		Chassis->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		Chassis->RegisterComponent();
	}

	// extract to an overriden RegisterComponent that registers the mesh
	Mesh->RegisterComponent();
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

/*
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
*/