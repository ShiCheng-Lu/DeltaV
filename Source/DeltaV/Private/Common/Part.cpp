// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Part.h"

#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"

UPart::UPart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

	Parent = nullptr;
	Children = TArray<UPart*>();

	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetSimulatePhysics(true);

	Physics = CreateDefaultSubobject<UPhysicsConstraintComponent>("Link");
	Physics->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0);
	Physics->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0);
}

// Sets default values
void UPart::Initialize(FString InId, TSharedPtr<FJsonObject> InStructure, TSharedPtr<FJsonObject> InJson)
{
	Id = InId;
	Structure = InStructure;
	Json = InJson;
	
	// Load mesh
	FString DefinitionName = FPaths::ProjectDir() + FString(L"Content/Parts/") + Json->GetStringField(L"type") + FString(".json");
	definition = JsonUtil::ReadFile(DefinitionName);
	FString MeshPath = definition->GetStringField(L"mesh");

	FSoftObjectPath MethObjectPath(*MeshPath);
	UStaticMesh* Mesh = UAssetLibrary::LoadAsset<UStaticMesh>(*MeshPath);
	if (Mesh != nullptr) {
		SetStaticMesh(Mesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("failed to load mesh for: %s"), *Id);
	}

	// set locaiton and scale
	SetRelativeLocation(JsonUtil::Vector(Json->GetArrayField(L"location")));
	SetRelativeScale3D(JsonUtil::Vector(Json->GetArrayField(L"scale")));
	
	RegisterComponent();
}

void UPart::SetAttachmentNodeVisibility(bool visibility) {
	for (auto node : AttachmentNodes) {
		node->SetVisibility(visibility);
	}
}

void UPart::SetParent(UPart* NewParent) {
	if (Parent != nullptr) {
		Parent->Children.Remove(this);
		Parent->Structure->RemoveField(Id);
	}
	Parent = NewParent;
	if (Parent != nullptr) {
		Parent->Children.Add(this);
		if (!Parent->Structure->HasField(Id)) {
			Parent->Structure->SetObjectField(Id, Structure);
		}
		Physics->SetConstrainedComponents(this, "", Parent, "");
	}
	else {
		Physics->BreakConstraint();
	}
}
