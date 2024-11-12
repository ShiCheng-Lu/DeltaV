// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionCraft.h"

#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"

static auto DetachmentRule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
static auto AttachmentRule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);

AConstructionCraft::AConstructionCraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{

}

void AConstructionCraft::FromJson(TSharedPtr<FJsonObject> CraftJson) {
	Super::FromJson(CraftJson);

	for (auto& PartKVP : Parts) {
		auto Part = PartKVP.Value;

		Part->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
		Part->SetSimulatePhysics(false);

		TSharedPtr<FJsonObject> PartDefinition = UAssetLibrary::PartDefinition(Part->Type);
		for (auto& Node : PartDefinition->GetArrayField(TEXT("attachment"))) {
			auto location = JsonUtil::Vector(Node->AsObject(), "location");

			auto AttachmentNode = NewObject<UAttachmentNode>(Part);
			AttachmentNode->SetRelativeLocation(location);
			AttachmentNode->RegisterComponent();
			AttachmentNode->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
			AttachmentNode->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

			Part->AttachmentNodes.Add(AttachmentNode);
		}
	}
}

void AConstructionCraft::SetAttachmentNodeVisibility(bool visibility) {

}

void AConstructionCraft::AttachPart(AConstructionCraft* SourceCraft, UPart* AttachToPart) {
	Super::AttachPart(SourceCraft, AttachToPart);
	/*
	for (auto& PartKVP : Parts) {
		auto Part = PartKVP.Value;
		Part->AttachToComponent(Part->Parent, AttachmentRule);
	}
	*/
}

void AConstructionCraft::DetachPart(UPart* DetachPart, AConstructionCraft* NewCraft) {
	Super::DetachPart(DetachPart, NewCraft);
	/*
	auto AttachmentRule2 = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);
	for (auto& PartKVP : NewCraft->Parts) {
		auto& Part = PartKVP.Value;
		Part->AttachToComponent(NewCraft->RootComponent, AttachmentRule2);

		UE_LOG(LogTemp, Warning, TEXT("Detached part %s"), *PartKVP.Key);
	}
	*/
}

AConstructionCraft* AConstructionCraft::Clone() {
	TSharedPtr<FJsonObject> Json = ToJson();
	FActorSpawnParameters SpawnParamsAlwaysSpawn = FActorSpawnParameters();
	SpawnParamsAlwaysSpawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AConstructionCraft* NewCraft = GetWorld()->SpawnActor<AConstructionCraft>(SpawnParamsAlwaysSpawn);
	NewCraft->FromJson(Json);
	return NewCraft;
}