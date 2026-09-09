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
#include "ChaosModularVehicle/ClusterUnionVehicleComponent.h"

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
}

void UPart::FromJson(TSharedPtr<FJsonObject> Json) {
	Type = Json->GetStringField(TEXT("type"));

	TSharedPtr<FJsonObject> PartDefinition = FAssetLibrary::PartDefinition(Type);
	FString MeshPath = PartDefinition->GetStringField(TEXT("mesh"));
	FString MeshTypeString = PartDefinition->GetStringField(TEXT("type"));

	if (MeshTypeString == "static") {
		TObjectPtr<UStaticMesh> StaticMesh = FAssetLibrary::LoadAsset<UStaticMesh>(*MeshPath);
		TObjectPtr<UStaticMeshComponent> MeshObj = NewObject<UStaticMeshComponent>(this);
		MeshObj->SetStaticMesh(StaticMesh);
		Mesh = MeshObj;
		Bone = FName("");
		MeshType = STATIC_MESH;
	}
	else if (MeshTypeString == "skeletal") {
		TObjectPtr<USkeletalMesh> SkeletalMesh = FAssetLibrary::LoadAsset<USkeletalMesh>(*MeshPath);
		TObjectPtr<USkeletalMeshComponent> MeshObj = NewObject<USkeletalMeshComponent>(this);
		MeshObj->SetSkeletalMesh(SkeletalMesh);
		Mesh = MeshObj;
		Bone = MeshObj->GetBoneName(1);

		FString BoneName = Bone.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Bone: %s"), *BoneName);
		MeshType = SKELETAL_MESH;
	}
	else if (MeshTypeString == "geometry_collection") {
		auto* GeometryCollection = FAssetLibrary::LoadAsset<UGeometryCollection>(*MeshPath);
		auto* Component = NewObject<UGeometryCollectionComponent>(this);
		Component->SetRestCollection(GeometryCollection);
		Component->DamageThreshold = { 1e8 };
		Component->bForceUpdateActiveTransforms = true;

		UE_LOG(LogTemp, Warning, TEXT("Creating geometry collection"));

		Mesh = Component;
		Bone = FName("");

		MeshType = GEOMETRY_COLLECTION;
	}

	Mesh->SetAbsolute(false, false, false);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// ignore self collision
	// Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore);
	// Mesh->SetCollisionObjectType(ECC_GameTraceChannel11);
	
	Mesh->SetWorldLocation(JsonUtil::Vector(Json, "location"));
	Mesh->SetWorldRotation(JsonUtil::Rotator(Json, "rotation"));
	Mesh->SetWorldScale3D(JsonUtil::Vector(Json, "scale"));

	

	// Mesh->SetAbsolute(false, false, true);

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

		auto* Suspension = NewObject<UVehicleSimSuspensionComponent>(Mesh);
		Suspension->SuspensionMaxDrop = 100;
		Suspension->SuspensionMaxRaise = 100;
		Suspension->SpringRate = 200;
		Suspension->SpringPreload = 100;
		// Suspension->SuspensionForceEffect = 0;
		Suspension->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		Suspension->RegisterComponent();
		Craft->AddOwnedComponent(Suspension);

		auto* Wheel = NewObject<UVehicleSimWheelComponent>(Mesh);
		Wheel->MaxSteeringAngle = WheelJson->GetNumberField(TEXT("max_steering"));
		Wheel->WheelRadius = 100;
		Wheel->AxisType = EWheelAxisType::X;
		Wheel->MaxSteeringAngle *= -1;
		Wheel->bSteeringEnabled = true;
		Wheel->AttachToComponent(Suspension, FAttachmentTransformRules::KeepRelativeTransform);
		Wheel->RegisterComponent();
		Craft->AddOwnedComponent(Wheel);
	} else if (Json->TryGetObjectField(TEXT("thruster"), SpecialJson)) {
		auto& ThrusterJson = *SpecialJson;

		auto* Thruster = NewObject<UVehicleSimThruster2DComponent>(Mesh);
		Thruster->bSteeringEnabled = true;
		Thruster->MaxThrustForce = 2000000.0f;
		Thruster->MaxSteeringAngle = 20;
		Thruster->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		Thruster->RegisterComponent();
		Craft->AddOwnedComponent(Thruster);
	}
	else {
		auto* Chassis = NewObject<UVehicleSimChassisComponent>(Mesh);
		Chassis->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		Chassis->RegisterComponent();
		Craft->AddOwnedComponent(Chassis);
	}
	// extract to an overriden RegisterComponent that registers the mesh
	
	UE_LOG(LogTemp, Warning, TEXT("%s has %s physics state"), *GetName(), Mesh->HasValidPhysicsState() ? TEXT("valid") : TEXT("invalid"));
	// ! important, super important
	Craft->GetClusterUnionComponent()->AddComponentToCluster(Mesh, {});
	// Craft->GetClusterUnionComponent()->RemoveComponentFromCluster(Mesh);
	Mesh->RegisterComponent();

}

TSharedPtr<FJsonObject> UPart::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	AActor* Owner = GetOwner();

	Json->SetStringField(TEXT("type"), Type);
	JsonUtil::Vector(Json, "location", Mesh->GetRelativeLocation());// -Owner->GetActorLocation());
	JsonUtil::Rotator(Json, "rotation", Mesh->GetRelativeRotation());// -Owner->GetActorRotation());
	JsonUtil::Vector(Json, "scale", Mesh->GetRelativeScale3D());
	// JsonUtil::Vector(Json, "attach_location", Physics->GetRelativeLocation());

	for (USceneComponent* Component : Mesh->GetAttachChildren()) {
		if (auto* Suspension = Cast<UVehicleSimSuspensionComponent>(Component)) {
			
			auto& SuspensionChildren = Suspension->GetAttachChildren();
			if (auto* Wheel = Cast<UVehicleSimWheelComponent>(SuspensionChildren.Last())) {
				auto WheelJson = MakeShared<FJsonObject>();
				WheelJson->SetNumberField(TEXT("max_steering"), -Wheel->MaxSteeringAngle);
				Json->SetObjectField(TEXT("wheel"), WheelJson);
			}
		} else if (auto* Thruster = Cast<UVehicleSimThruster2DComponent>(Component)) {
			auto ThrusterJson = MakeShared<FJsonObject>();
			Json->SetObjectField(TEXT("thruster"), ThrusterJson);
		} else if (auto* Chassis = Cast< UVehicleSimChassisComponent>(Component)) {

		}
	}
	// if there is a suspension component
	


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