// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/ConstructionHUD.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/TileView.h"
#include "Components/TreeView.h"
#include "Kismet/GameplayStatics.h"

#include "Construction/ConstructionController.h"
#include "Construction/UI/PartItem.h"
#include "Construction/UI/StageGroup.h"
#include "Common/MainGameInstance.h"
#include "Common/Craft.h"
#include "Common/Craft/Stage.h"

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
	PartsList->AddItem(UPartItemData::Create("cone"));
	PartsList->AddItem(UPartItemData::Create("cylinder"));
	PartsList->AddItem(UPartItemData::Create("decoupler"));
	PartsList->AddItem(UPartItemData::Create("engine"));
	PartsList->AddItem(UPartItemData::Create("wing"));
	PartsList->AddItem(UPartItemData::Create("cockpit"));
	PartsList->AddItem(UPartItemData::Create("leg"));

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
	UE_LOG(LogTemp, Warning, TEXT("stages %d"), Craft->Stages.Num());

	if (Controller->Constructor.Selected) {
		Controller->Constructor.Selected->GetOwner()->Destroy();
		Controller->Constructor.Selected = nullptr;
	}
	Controller->Constructor.Select(Craft->RootPart());
	Controller->Constructor.Distance = 500;
}

TArray<UObject*> UConstructionHUD::GetStagingList(UObject* Object) {
	UStage* Data = Cast<UStage>(Object);
	if (!Data) {
		UE_LOG(LogTemp, Warning, TEXT("GetStagingList, not UStageGroupData"));
		return TArray<UObject*>();
	}
	UE_LOG(LogTemp, Warning, TEXT("GetStagingList, %d items"), Data->Parts.Num());
	TArray<UObject*> Children;
	for (UPart* Part : Data->Parts) {
		Children.Add(Part);
	}
	return Children;
}

void UConstructionHUD::SetCraft(ACraft* Craft) {
	StagesList->ClearListItems();
	for (UStage* Stage : Craft->Stages) {
		StagesList->AddItem(Stage);
		StagesList->SetItemExpansion(Stage, true);
	}
	UE_LOG(LogTemp, Warning, TEXT("Stages has %d items"), StagesList->GetListItems().Num());
	UE_LOG(LogTemp, Warning, TEXT("Stages has %d renders"), StagesList->GetDisplayedEntryWidgets().Num());



}
