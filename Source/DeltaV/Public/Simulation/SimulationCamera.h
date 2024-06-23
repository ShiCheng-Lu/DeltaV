// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SimulationCamera.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API ASimulationCamera : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ASimulationCamera(const FObjectInitializer& ObjectInitializer);

	virtual void GetCameraViewPoint(FVector& OutCamLoc, FRotator& OutCamRot) const override;
};
