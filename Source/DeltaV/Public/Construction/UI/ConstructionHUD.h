// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConstructionHUD.generated.h"

class UButton;
class UTextBlock;
class UUniformGridPanel;
class UPartDetails;
class UTileView;
class UTreeView;

class UDragReorderList;

class AConstructionController;
class ACraft;

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
	UTileView* PartsList;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* SymmetryText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* ModeText;

	UFUNCTION(BlueprintCallable)
	void ClearPart();

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UPartDetails* PartDetails;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UUserWidget* StagesListWidget;

	UFUNCTION(BlueprintCallable)
	void SaveClicked();

	UFUNCTION(BlueprintCallable)
	void PartClicked(UObject* Object);

	void SetCraft(ACraft* Craft) const;
};
