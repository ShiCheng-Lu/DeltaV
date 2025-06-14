// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Simulation/UI/Navball.h"
#include "SimulationHUD.generated.h"


class ASimulationController;
class ACraft;
class UButton;
class UCheckBox;

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

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum="EStabilizationMode"))
	FColor TargetColour[(int)EStabilizationMode::Num];

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

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UStagesList> StagesList;

	UFUNCTION(BlueprintCallable)
	void SetStabilizationMode(bool Checked, EStabilizationMode Mode);

	UFUNCTION(BlueprintCallable)
	void ButtonClicked(UCheckBox* Button);

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UProgressBar> TimeWarp;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Info;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UProgressBar> Fuel;

	FVector VelocityValue = FVector(0);
};
