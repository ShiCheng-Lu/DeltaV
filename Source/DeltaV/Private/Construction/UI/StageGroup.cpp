// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/StageGroup.h"

#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Common/UI/StagesList.h"
#include "Common/Craft/StageManager.h"
#include "Common/Part.h"

UStageGroup::UStageGroup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


TSubclassOf<UUserWidget> UStageGroup::BlueprintClass() {
	auto Widget = TSoftClassPtr<UUserWidget>(FSoftObjectPath("WidgetBlueprint'/Game/Common/UI/WBP_StageGroup'"));
	if (Widget.IsValid()) {
		return Widget.Get();
	}
	auto Loaded = Widget.LoadSynchronous();
	if (Loaded) {
		return Loaded;
	}
	throw "Blueprint class not found";
}

void UStageGroup::Init(UObject* Object) {
	UPart* Part = Cast<UPart>(Object);
	if (Part) {
		Name->SetText(FText::FromString(Part->Id));

		WidgetIndex->SetActiveWidgetIndex(1);

		Payload = Part;
		Type = 0;

		return;
	}
	UStage* Stage = Cast<UStage>(Object);
	if (Stage) {

		Payload = Stage;
		Type = 1;

		WidgetIndex->SetActiveWidgetIndex(0);
		return;
	}
}

void UStageGroup::AddStage(int Offset) {
	UStagesList* StagesList = GetTypedOuter<UStagesList>();
	UStage* Stage = Cast<UStage>(Payload);
	if (StagesList && Stage) {
		StagesList->AddStage(Stage, Offset);
	}
}
