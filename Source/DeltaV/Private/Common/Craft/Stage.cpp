// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/Stage.h"

#include "Common/Craft.h"
#include "Common/Part.h"

UStage::UStage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Craft = GetTypedOuter<ACraft>();
}

void UStage::FromJson(TSharedPtr<FJsonValue> Json) {
	if (!Craft) {
		return;
	}
	for (auto& PartJson : Json->AsObject()->GetArrayField("parts")) {
		FString PartId = PartJson->AsString();
		UPart** Part = Craft->Parts.Find(PartId);
		if (Part) {
			Parts.Add(*Part);
		}
	}
}

TSharedPtr<FJsonValue> UStage::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	TArray<TSharedPtr<FJsonValue>> Array;
	for (UPart* Part : Parts) {
		Array.Add(MakeShareable(new FJsonValueString(Part->Id)));
	}
	Json->SetArrayField("parts", Array);
	return MakeShareable(new FJsonValueObject(Json));
}

TArray<ACraft*>UStage::Activate() {
	TArray<ACraft*> Detached;
	for (UPart* Part : Parts) {
		if (Part->Type == "decoupler") {
			ACraft* NewCraft = GetWorld()->SpawnActor<ACraft>();
			Craft->DetachPart(Part, NewCraft);
			Detached.Add(NewCraft);
		}
		if (Part->Type == "engine") {
			Craft->Active->Parts.Add(Part);
		}
	}
	return Detached;
}
