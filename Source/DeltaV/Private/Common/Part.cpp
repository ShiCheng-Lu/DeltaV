// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Part.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"

UPart::UPart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

	Parent = nullptr;
	Children = TArray<UPart*>();
	FCollisionResponseContainer();
		// TODO: add custom collision channel for each craft, no intervessel collision

	SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
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
}

// Sets default values
void UPart::Initialize(FString InId, TSharedPtr<FJsonObject> InStructure, TSharedPtr<FJsonObject> InJson)
{
	Id = InId;
	Structure = InStructure;
	Json = InJson;
	

	// set locaiton and scale
	SetRelativeLocation(JsonUtil::Vector(Json, "location"));
	SetRelativeScale3D(JsonUtil::Vector(Json, "scale"));

	FString DefinitionName = FPaths::Combine(FPaths::ProjectContentDir(), "Parts", Json->GetStringField(TEXT("type")) + ".json");
	definition = JsonUtil::ReadFile(DefinitionName);
	FString MeshPath = definition->GetStringField(TEXT("mesh"));

	UStaticMesh* Mesh = UAssetLibrary::LoadAsset<UStaticMesh>(*MeshPath);
	if (Mesh != nullptr) {
		SetStaticMesh(Mesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("failed to load mesh for: %s"), *Id);
	}
	
	RegisterComponent();
	Physics->RegisterComponent();
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

void UPart::SetSimulatePhysics(bool bSimulate) {
	// UE_LOG(LogTemp, Warning, TEXT("phys constraint: %s, %s"), *Id, *Physics->GetComponentLocation().ToString());
	if (Parent && bSimulate) {
		Physics->SetConstrainedComponents(this, "", Parent, "");
	}
	else {
		Physics->BreakConstraint();
	}
	Super::SetSimulatePhysics(bSimulate);
}


void UPart::FromJson(TSharedPtr<FJsonObject> InJson) {
	type = InJson->GetStringField(TEXT("type"));

	SetRelativeLocation(JsonUtil::Vector(InJson, "location"));
	SetRelativeRotation(JsonUtil::Quat(InJson, "rotation"));
	SetRelativeScale3D(JsonUtil::Vector(InJson, "scale"));
}
TSharedPtr<FJsonObject> UPart::ToJson() {
	TSharedPtr<FJsonObject> OutJson = MakeShareable(new FJsonObject());

	OutJson->SetStringField(TEXT("type"), type);
	JsonUtil::Vector(OutJson, "location", GetRelativeLocation());
	JsonUtil::Quat(OutJson, "rotation", GetRelativeRotation().Quaternion());
	JsonUtil::Vector(OutJson, "scale", GetRelativeScale3D());

	return OutJson;
}