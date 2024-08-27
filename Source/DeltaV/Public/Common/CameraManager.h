// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraManager.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API ACameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACameraManager(const FObjectInitializer& ObjectInitializer);

	virtual void GetCameraViewPoint(FVector& OutCamLoc, FRotator& OutCamRot) const override;

	FRotator BaseRotation;
};
