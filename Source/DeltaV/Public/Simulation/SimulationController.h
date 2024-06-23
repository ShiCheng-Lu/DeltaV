// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimulationController.generated.h"

class ACraft;

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

	void Throttle(float value);

	// void StageCraft();
};
