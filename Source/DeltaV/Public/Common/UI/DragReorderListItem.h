// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Construction/UI/PartItem.h"
#include "DragReorderListItem.generated.h"


/**
 * 
 */
UCLASS()
class DELTAV_API UDragReorderListItem : public UUserWidget
{
	GENERATED_BODY()

public:

	UDragReorderListItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
		ConstructorHelpers::FClassFinder<UDragReorderListItem> Widget(TEXT("WidgetBlueprint'/Game/Common/UI/WBP_DragReorderListItem'"));
		if (Widget.Succeeded()) {
			UDragReorderListItem::BlueprintClass = Widget.Class;
		}
	}

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* PartLabel;

	inline static TSubclassOf<UUserWidget> BlueprintClass;

	UFUNCTION(BlueprintCallable)
	void Init(UObject* Object) {
		UPartItemData* Data = Cast<UPartItemData>(Object);
		if (Data) {
			PartLabel->SetText(FText::FromString(Data->Name));
		}
	}
};
