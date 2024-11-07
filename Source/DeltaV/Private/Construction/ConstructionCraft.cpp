// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionCraft.h"

#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Common/AttachmentNode.h"

AConstructionCraft::AConstructionCraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{

}

void AConstructionCraft::FromJson(TSharedPtr<FJsonObject> CraftJson) {
	Super::FromJson(CraftJson);

	for (auto& PartKVP : Parts) {
		auto Part = PartKVP.Value;

		if (Part != RootPart) {
			Part->AttachToComponent(RootPart, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
		}

		Part->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
		Part->SetSimulatePhysics(false);

		TSharedPtr<FJsonObject> PartDefinition = UAssetLibrary::PartDefinition(Part->Type);
		for (auto& Node : PartDefinition->GetArrayField(TEXT("attachment"))) {
			auto location = JsonUtil::Vector(Node->AsObject(), "location");

			auto attachment_node = NewObject<UAttachmentNode>(Part);
			attachment_node->SetRelativeLocation(location);
			attachment_node->RegisterComponent();
			attachment_node->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
			attachment_node->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

			Part->AttachmentNodes.Add(attachment_node);
		}
	}
}

void AConstructionCraft::SetAttachmentNodeVisibility(bool visibility) {

}

void AConstructionCraft::AttachPart(AConstructionCraft* SourceCraft, UPart* AttachToPart) {
	
	TArray<UPart*> SourceParts;
	for (auto& PartKVP : SourceCraft->Parts) {
		SourceParts.Add(PartKVP.Value);
	}

	Super::AttachPart(SourceCraft, AttachToPart);
	
	for (auto& Part : SourceParts) {
		Part->AttachToComponent(RootPart, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	}
}

void AConstructionCraft::DetachPart(UPart* DetachPart, AConstructionCraft* NewCraft) {
	Super::DetachPart(DetachPart, NewCraft);
	
	auto AttachmentRule2 = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);
	for (auto& PartKVP : NewCraft->Parts) {
		auto& Part = PartKVP.Value;
		Part->AttachToComponent(NewCraft->RootComponent, AttachmentRule2);

		UE_LOG(LogTemp, Warning, TEXT("Detached part %s"), *PartKVP.Key);
	}
}

AConstructionCraft* AConstructionCraft::Clone() {
	TSharedPtr<FJsonObject> Json = ToJson();
	FActorSpawnParameters SpawnParamsAlwaysSpawn = FActorSpawnParameters();
	SpawnParamsAlwaysSpawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AConstructionCraft* NewCraft = GetWorld()->SpawnActor<AConstructionCraft>(SpawnParamsAlwaysSpawn);
	NewCraft->FromJson(Json);
	return NewCraft;
}