// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/ConstructionHUD.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/TileView.h"
#include "Components/TreeView.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"

#include "Common/UI/DragReorderList.h"
#include "Construction/ConstructionController.h"
#include "Construction/UI/PartItem.h"
#include "Construction/UI/StageGroup.h"
#include "Common/MainGameInstance.h"
#include "Common/Craft.h"
#include "Common/Craft/StageManager.h"
#include "Common/UI/StagesList.h"

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

	// craft name display
	CraftName->SetText(FText::FromString("Craft Name"));

	// part list
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FileNames;
	FileManager.FindFiles(FileNames, *(FPaths::Combine(FPaths::ProjectContentDir(), "Parts")), TEXT("json"));
	for (FString& FileName : FileNames) {
		FString PartName = FPaths::GetBaseFilename(FileName);
		PartsList->AddItem(UPartItemData::Create(PartName));
	}
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


void UConstructionHUD::PartClicked(UObject* Object) {
	UPartItemData* Data = Cast<UPartItemData>(Object);
	if (!Data) {
		return;
	}

	auto Craft = Controller->Constructor.CreateCraft(Data->CraftJson);
	UE_LOG(LogTemp, Warning, TEXT("stages %d"), Craft->StageManager->Stages.Num());

	if (Controller->Constructor.Selected) {
		Controller->Constructor.Selected->GetOwner()->Destroy();
		Controller->Constructor.Selected = nullptr;
	}
	Controller->Constructor.Select(Craft->RootPart());
	Controller->Constructor.Distance = 500;
}

void UConstructionHUD::SetCraft(ACraft* Craft) const {
	UStagesList* StagesList = Cast<UStagesList>(StagesListWidget);
	StagesList->Manager = Craft->StageManager;
	StagesList->Reload();
}
