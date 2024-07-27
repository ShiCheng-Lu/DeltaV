// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimulationHUD.generated.h"


class ASimulationController;
class ACraft;
class UButton;
class UCheckBox;
enum EStabilizationMode : uint8;


/**
 * 
 */
UCLASS()
class DELTAV_API USimulationHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	USimulationHUD(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	inline static TSubclassOf<UUserWidget> BlueprintClass;

	ASimulationController* Controller;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UImage> Navball;

	TObjectPtr<class ANavball> NavballActor;

	void SetNavballTarget(ACraft* Craft, FVector PlanetCenter) const;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UProgressBar> Throttle;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class USlider> Velocity;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class USlider> Gravity;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UUniformGridPanel> StabilizationPanel;

	UFUNCTION(BlueprintCallable)
	void SetStabilizationMode(bool Checked, EStabilizationMode Mode);

	UFUNCTION(BlueprintCallable)
	void ButtonClicked(UCheckBox* Button);
};
