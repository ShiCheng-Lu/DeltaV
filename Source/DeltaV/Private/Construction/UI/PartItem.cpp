// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/PartItem.h"

#include "IImageWrapper.h" 
#include "IImageWrapperModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Common/Craft.h"
#include "Common/Part.h"
#include "Common/JsonUtil.h"
#include "Common/AssetLibrary.h"
#include "Construction/ConstructionController.h"
#include "Construction/Constructor.h"

UPartItem::UPartItem(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer) 
{
	if (!UPartItem::BlueprintClass) {
		ConstructorHelpers::FClassFinder<UPartItem> Widget(TEXT("WidgetBlueprint'/Game/Construction/UI/WBP_PartItem'"));
		if (Widget.Succeeded()) {
			UPartItem::BlueprintClass = Widget.Class;
		}
	}

	PartName = GetName();

	// ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("asd"));
	// Mesh.Succeeded();

	// USceneCaptureComponent2D* Camera = CreateDefaultSubobject<USceneCaptureComponent2D>("Camera");

	PartJson = UAssetLibrary::PartDefinition(PartName);
}

void UPartItem::NativeOnInitialized() {

	// main button
	FScriptDelegate MainButtonClicked;
	MainButtonClicked.BindUFunction(this, "MainButtonClicked");
	MainButton->OnClicked.Add(MainButtonClicked);

	// main text
	PartLabel->SetText(FText::FromString(PartName));

	Controller = GetOwningPlayer<AConstructionController>();
}

void UPartItem::MainButtonClicked() {
	UE_LOG(LogTemp, Warning, TEXT("part item clicked"));

	auto Craft = Controller->Constructor.CreateCraft(PartJson->GetObjectField(TEXT("craft")));
	UE_LOG(LogTemp, Warning, TEXT("stages %d"), Craft->Stages.Num());

	if (Controller->Constructor.Selected) {
		Controller->Constructor.Selected->GetOwner()->Destroy();
		Controller->Constructor.Selected = nullptr;
	}
	Controller->Constructor.Select(Craft->RootPart());
	Controller->Constructor.Distance = 500;
}
