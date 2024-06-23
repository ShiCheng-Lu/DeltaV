// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/ConstructionHUD.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

#include "Construction/ConstructionController.h"
#include "Construction/ConstructionCraft.h"
#include "Common/MainGameInstance.h"

UConstructionHUD::UConstructionHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!UConstructionHUD::BlueprintClass) {
		ConstructorHelpers::FClassFinder<UConstructionHUD> Widget(TEXT("WidgetBlueprint'/Game/Construction/UI/WBP_ConstructionHUD'"));
		if (Widget.Succeeded()) {
			UConstructionHUD::BlueprintClass = Widget.Class;
		}
	}
}

void UConstructionHUD::NativeOnInitialized() {
	Super::NativeOnInitialized();

	FScriptDelegate LaunchButtonClicked;
	LaunchButtonClicked.BindUFunction(this, "LaunchButtonClicked");
	LaunchButton->OnClicked.Add(LaunchButtonClicked);

	CraftName->SetText(FText::FromString("Hello"));

	Controller = Cast<AConstructionController>(GetOwningPlayer());
}

void UConstructionHUD::LaunchButtonClicked() {
	Controller->Save();

	if (Controller->Craft) {
		Controller->Craft->Destroy();
	}

	Cast<UMainGameInstance>(GetGameInstance())->CraftPath = FPaths::ProjectDir() + "Content/Crafts/ship2.json";

	UGameplayStatics::OpenLevel(GetWorld(), "Simulation");
}
