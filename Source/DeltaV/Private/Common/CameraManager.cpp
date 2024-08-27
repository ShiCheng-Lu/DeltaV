// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/CameraManager.h"

#include "Kismet/KismetMathLibrary.h"

ACameraManager::ACameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void ACameraManager::GetCameraViewPoint(FVector& OutCamLoc, FRotator& OutCamRot) const {
	const FMinimalViewInfo& CurrentPOV = GetCameraCacheView();
	OutCamRot = UKismetMathLibrary::ComposeRotators(CurrentPOV.Rotation, BaseRotation);
	OutCamLoc = CurrentPOV.Location - OutCamRot.Vector() * FreeCamDistance;
}