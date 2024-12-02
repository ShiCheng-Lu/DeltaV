// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "PartItem.generated.h"

class UImage;
class UPart;
class UTextBlock;
class UButton;

class AConstructionController;


UCLASS()
class DELTAV_API UPartItemData : public UObject {
	GENERATED_BODY()

public:
	FString Name;
	TSharedPtr<FJsonObject> PartJson;
	TSharedPtr<FJsonObject> CraftJson;

	static UPartItemData* Create(FString Name);
};

/**
 * 
 */
UCLASS()
class DELTAV_API UPartItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPartItem(const FObjectInitializer& ObjectInitializer);

public:
	inline static TSubclassOf<UUserWidget> BlueprintClass;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* PartLabel;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UImage* Thumbnail;

	UFUNCTION(BlueprintCallable)
	void Init(UObject* ListItemObject);
};
