// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimulationController.generated.h"

class AStaticMeshActor;
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

	virtual void UpdateRotation(float DeltaTime) override;

	FRotator ViewRotation;

	TObjectPtr<class ACraft> Craft;
	TObjectPtr<class UObjectComponent> CraftOrbit;

	TObjectPtr<class ACelestialBody> Earth;

	int TimeWarp = 0;
	TArray<float> TimeWarpMapping = { 0, 1, 2, 4, 8, 14, 20 };

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

	UFUNCTION(BlueprintCallable)
	void VelChange(float value);

	UFUNCTION(BlueprintCallable)
	void GravChange(float value);

	void SetTimeWarp(int TimeWarpLevel);
	void TimeWarpAdd();
	void TimeWarpSub();
	void TimeWarpReset();

	void Action(FKey Key);

	void Exit();

	AStaticMeshActor* TestActor;
};
