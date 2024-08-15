// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PartItem.generated.h"

class UImage;
class UPart;
class UTextBlock;
class UButton;

class AConstructionController;

/**
 * 
 */
UCLASS()
class DELTAV_API UPartItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPartItem(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

public:
	AConstructionController* Controller;
	FString PartName;
	TSharedPtr<FJsonObject> PartJson;

	inline static TSubclassOf<UUserWidget> BlueprintClass;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* MainButton;

	UFUNCTION(BlueprintCallable)
	void MainButtonClicked();

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* PartLabel;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UImage* Thumbnail;

};
