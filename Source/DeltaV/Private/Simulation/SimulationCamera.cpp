// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationCamera.h"


ASimulationCamera::ASimulationCamera(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer) 
{

}

void ASimulationCamera::GetCameraViewPoint(FVector& OutCamLoc, FRotator& OutCamRot) const {
	const FMinimalViewInfo& CurrentPOV = GetCameraCacheView();
	OutCamLoc = CurrentPOV.Location - CurrentPOV.Rotation.Vector() * FreeCamDistance;
	OutCamRot = CurrentPOV.Rotation;
}
