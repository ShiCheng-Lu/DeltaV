// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimulationController.generated.h"

class ACraft;
class USimulationHUD;
class UControlStabilizer;

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

	virtual void Tick(float DeltaSeconds) override;

	ACraft* craft;
	TObjectPtr<class ACelestialBody> earth;

	UPROPERTY()
	USimulationHUD* HUD;

	float ThrottleValue = 0;

	void Throttle(float value);

	void Zoom(float value);

	void Pitch(float value);
	void Roll(float value);
	void Yaw(float value);
	void Stage();
	void ToggleMap();

	float vel = 0;
	float grav = 0;

	UFUNCTION()
	void VelChange(float value);

	UFUNCTION()
	void GravChange(float value);

	UControlStabilizer* ControlStabilizer;
};
