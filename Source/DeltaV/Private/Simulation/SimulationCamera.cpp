// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationCamera.h"
#include "Kismet/KismetMathLibrary.h"

ASimulationCamera::ASimulationCamera(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer) 
{

}

void ASimulationCamera::GetCameraViewPoint(FVector& OutCamLoc, FRotator& OutCamRot) const {
	const FMinimalViewInfo& CurrentPOV = GetCameraCacheView();
	OutCamRot = UKismetMathLibrary::ComposeRotators(CurrentPOV.Rotation, FRotator(90, 0, 0));
	OutCamLoc = CurrentPOV.Location - OutCamRot.Vector() * FreeCamDistance;
}
