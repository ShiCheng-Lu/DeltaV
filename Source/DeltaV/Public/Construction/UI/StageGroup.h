// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageGroup.generated.h"

class UWidgetSwitcher;
class UTextBlock;

/**
 * 
 */
UCLASS()
class DELTAV_API UStageGroup : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void Init(UObject* Object);

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* Name;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWidgetSwitcher* WidgetIndex;

	static TArray<UObject*> GetStagingList(UObject* Object);
};
