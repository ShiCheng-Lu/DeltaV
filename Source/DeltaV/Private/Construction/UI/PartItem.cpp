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
#include "UObject/Interface.h"


UPartItemData* UPartItemData::Create(FString Name) {
	UPartItemData* Data = NewObject<UPartItemData>();

	Data->Name = Name;
	Data->PartJson = UAssetLibrary::PartDefinition(Name);
	if (Data->PartJson.IsValid()) {
		Data->CraftJson = Data->PartJson->GetObjectField(TEXT("craft"));
	}

	return Data;
}


UPartItem::UPartItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
}

TSubclassOf<UUserWidget> UPartItem::BlueprintClass() {
	auto Widget = TSoftClassPtr<UUserWidget>(FSoftObjectPath("WidgetBlueprint'/Game/Construction/UI/WBP_PartItem'"));
	if (Widget.IsValid()) {
		return Widget.Get();
	}
	auto Loaded = Widget.LoadSynchronous();
	if (Loaded) {
		return Loaded;
	}
	throw "Blueprint class not found";
}

void UPartItem::Init(UObject* ListItemObject) {
	UPartItemData* Data = Cast<UPartItemData>(ListItemObject);
	if (!Data) {
		UE_LOG(LogTemp, Warning, TEXT("UPartItem: invalid init param"));
		return;
	}

	PartLabel->SetText(FText::FromString(Data->Name));
}
