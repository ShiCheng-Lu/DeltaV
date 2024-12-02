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
	Data->CraftJson = Data->PartJson->GetObjectField(TEXT("craft"));

	return Data;
}


UPartItem::UPartItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	if (!UPartItem::BlueprintClass) {
		ConstructorHelpers::FClassFinder<UPartItem> Widget(TEXT("WidgetBlueprint'/Game/Construction/UI/WBP_PartItem'"));
		if (Widget.Succeeded()) {
			UPartItem::BlueprintClass = Widget.Class;
		}
	}
}

void UPartItem::Init(UObject* ListItemObject) {
	UPartItemData* Data = Cast<UPartItemData>(ListItemObject);
	if (!Data) {
		UE_LOG(LogTemp, Warning, TEXT("UPartItem: invalid init param"));
		return;
	}

	PartLabel->SetText(FText::FromString(Data->Name));
	UE_LOG(LogTemp, Warning, TEXT("init part name: %s"), *Data->Name);


	

}
