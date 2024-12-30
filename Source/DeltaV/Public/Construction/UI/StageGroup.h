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
	UStageGroup(const FObjectInitializer& ObjectInitializer);

	inline static TSubclassOf<UUserWidget> BlueprintClass;

	UFUNCTION(BlueprintCallable)
	void Init(UObject* Object);

	UObject* Payload;
	int Type;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* Name;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWidgetSwitcher* WidgetIndex;

	UFUNCTION(BlueprintCallable)
	void AddStage(int Offset);
};
