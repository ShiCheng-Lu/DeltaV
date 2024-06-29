// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimulationHUD.generated.h"

class ASimulationController;
class ACraft;

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
};
