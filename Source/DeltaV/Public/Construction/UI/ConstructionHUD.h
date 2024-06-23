// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConstructionHUD.generated.h"

class UButton;
class UTextBlock;
class AConstructionController;

/**
 * 
 */
UCLASS()
class DELTAV_API UConstructionHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UConstructionHUD(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

public:
	inline static TSubclassOf<UUserWidget> BlueprintClass;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* LaunchButton;

	UFUNCTION(BlueprintCallable)
	void LaunchButtonClicked();

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* CraftName;

	AConstructionController* Controller;
};
