// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimulationController.generated.h"

class ACraft;
class USimulationHUD;

/**
 * 
 */
UCLASS()
class DELTAV_API ASimulationController : public APlayerController
{
	GENERATED_BODY()

public:

	ASimulationController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	void SetupInputComponent();

	ACraft* craft;

	UPROPERTY()
	USimulationHUD* HUD;

	void Throttle(float value);

	void Zoom(float value);

	void Pitch(float value);
	void Roll(float value);
	void Yaw(float value);
};
