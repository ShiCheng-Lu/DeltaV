// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PartDetails.generated.h"

class UPart;
class USlider;
class USpinBox;
class UEditableTextBox;
class AConstructionController;

UENUM()
enum PartField {
	Name,
	SizeX,
	SizeY,
	SizeZ,
	LiquidFuel,
	Oxidizer,
};

/**
 * 
 */
UCLASS()
class DELTAV_API UPartDetails : public UUserWidget
{
	GENERATED_BODY()

	PartField InteractingField;
	USlider* InteractingSlider;

public:

	UPROPERTY(EditAnywhere)
	UPart* Part;

	UPartDetails(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	AConstructionController* Controller;

	inline static TSubclassOf<UUserWidget> BlueprintClass;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UEditableTextBox* PartName;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	USpinBox* SizeX;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	USpinBox* SizeY;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	USpinBox* SizeZ;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	USlider* LiquidFuel;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	USlider* Oxidizer;

	UFUNCTION(BlueprintCallable)
	void Update(PartField Field);

	UFUNCTION(BlueprintCallable)
	void MakeDynamic();

	void SetPart(UPart* InPart);
};
