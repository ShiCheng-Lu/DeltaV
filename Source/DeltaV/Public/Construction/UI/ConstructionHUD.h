// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConstructionHUD.generated.h"

class AConstructionController;
class UButton;
class UTextBlock;
class UUniformGridPanel;

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

	AConstructionController* Controller;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* LaunchButton;

	UFUNCTION(BlueprintCallable)
	void LaunchButtonClicked();

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* CraftName;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UUniformGridPanel* PartsList;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* SymmetryText;

	UFUNCTION(BlueprintCallable)
	void ClearPart();
};
