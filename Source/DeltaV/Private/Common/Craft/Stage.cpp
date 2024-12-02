// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/Stage.h"

#include "Common/Craft.h"
#include "Common/Part.h"

void UStage::FromJson(TSharedPtr<FJsonValue> Json) {
	ACraft* Craft = GetTypedOuter<ACraft>();
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

