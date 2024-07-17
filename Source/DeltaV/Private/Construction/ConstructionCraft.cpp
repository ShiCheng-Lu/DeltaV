// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionCraft.h"

#include "Common/JsonUtil.h"
#include "Common/AttachmentNode.h"

AConstructionCraft::AConstructionCraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{

}

void AConstructionCraft::Initialize(TSharedPtr<FJsonObject> CraftJson) {
	Super::Initialize(CraftJson);

	for (auto& PartKVP : Parts) {
		auto Part = PartKVP.Value;

		Part->AttachToComponent(Part == RootPart ? nullptr : RootPart, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));

		Part->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
		Part->SetSimulatePhysics(false);

		for (auto& node : Part->definition->GetArrayField(L"attachment")) {
			auto location = JsonUtil::Vector(node->AsObject()->GetArrayField(L"location"));

			auto attachment_node = NewObject<UAttachmentNode>(Part);
			attachment_node->SetRelativeLocation(location);
			attachment_node->RegisterComponent();

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

	auto AttachemtnRule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);
	for (auto& PartKVP : NewCraft->Parts) {
		auto& Part = PartKVP.Value;
		Part->AttachToComponent(NewCraft->RootComponent, AttachemtnRule);
	}
}
