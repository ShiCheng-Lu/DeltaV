// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/ConstructionHUD.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Kismet/GameplayStatics.h"

#include "Construction/ConstructionController.h"
#include "Construction/UI/PartItem.h"
#include "Common/MainGameInstance.h"
#include "Common/Craft.h"

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
	Controller = GetOwningPlayer<AConstructionController>();

	// launch button
	FScriptDelegate LaunchButtonClicked;
	LaunchButtonClicked.BindUFunction(this, "LaunchButtonClicked");
	LaunchButton->OnClicked.Add(LaunchButtonClicked);

	// craft name display
	CraftName->SetText(FText::FromString("Craft Name"));

	// part list
	PartsList->AddChildToUniformGrid(CreateWidget<UPartItem>(this, UPartItem::BlueprintClass, "cone"), 0, 0);
	PartsList->AddChildToUniformGrid(CreateWidget<UPartItem>(this, UPartItem::BlueprintClass, "cylinder"), 0, 1);
	PartsList->AddChildToUniformGrid(CreateWidget<UPartItem>(this, UPartItem::BlueprintClass, "decoupler"), 1, 0);
	PartsList->AddChildToUniformGrid(CreateWidget<UPartItem>(this, UPartItem::BlueprintClass, "engine"), 1, 1);
	PartsList->AddChildToUniformGrid(CreateWidget<UPartItem>(this, UPartItem::BlueprintClass, "wing"), 2, 0);
	PartsList->AddChildToUniformGrid(CreateWidget<UPartItem>(this, UPartItem::BlueprintClass, "cockpit"), 2, 1);
	PartsList->AddChildToUniformGrid(CreateWidget<UPartItem>(this, UPartItem::BlueprintClass, "leg"), 3, 0);



}

void UConstructionHUD::LaunchButtonClicked() {
	Controller->Save();

	Cast<UMainGameInstance>(GetGameInstance())->CraftPath = FPaths::Combine(FPaths::ProjectSavedDir(), "ship2.json");

	UGameplayStatics::OpenLevel(GetWorld(), "Simulation");
}

void UConstructionHUD::SaveClicked() {
	Controller->Save();



	Controller->Load();
}

void UConstructionHUD::ClearPart() {
	Controller->Constructor.Delete();
}
