// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/PartItem.h"

#include "IImageWrapper.h" 
#include "IImageWrapperModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Common/Part.h"
#include "Common/JsonUtil.h"
#include "Construction/ConstructionController.h"
#include "Construction/ConstructionCraft.h"


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
	FActorSpawnParameters SpawnParamsAlwaysSpawn = FActorSpawnParameters();
	SpawnParamsAlwaysSpawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto Craft = GetWorld()->SpawnActor<AConstructionCraft>(SpawnParamsAlwaysSpawn);
	// generate an ID for the part
	FString PartId = PartName; // TODO: id needs to be unique Controller->GetUniquePartId(PartName);
	// set part at root part of the new craft, and add the part description
	TSharedPtr<FJsonObject> CraftJson = JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/single_part.json", {
		{ "part_id", PartId },
		{ "part_type", PartName }
	});

	if (!CraftJson.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("CraftJson was invalid"));
		return;
	}
	if (!Controller) {
		UE_LOG(LogTemp, Warning, TEXT("Controller not found"));
	}

	Craft->Initialize(CraftJson);

	if (!Controller->Craft) {
		Controller->Craft = Craft;
	}
	Controller->Selected = Craft;
	Controller->SelectedPart = Craft->RootPart();
}
