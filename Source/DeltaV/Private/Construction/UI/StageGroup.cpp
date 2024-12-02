// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/StageGroup.h"

#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Common/Craft/Stage.h"
#include "Common/Part.h"

void UStageGroup::Init(UObject* Object) {
	UPart* Part = Cast<UPart>(Object);
	if (Part) {
		Name->SetText(FText::FromString(Part->Id));

		WidgetIndex->SetActiveWidgetIndex(1);

		return;
	}
	UStage* Stage = Cast<UStage>(Object);
	if (Stage) {

		WidgetIndex->SetActiveWidgetIndex(0);
		return;
	}
}

TArray<UObject*> UStageGroup::GetStagingList(UObject* Object) {
	UStage* Stage = Cast<UStage>(Object);
	if (!Stage) {
		return TArray<UObject*>();
	}
	TArray<UObject*> Array;
	for (UPart* Part : Stage->Parts) {
		Array.Add(Part);
	}
	return Array;
}
