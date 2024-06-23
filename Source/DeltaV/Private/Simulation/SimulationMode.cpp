// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationMode.h"

#include "Simulation/SimulationController.h"
#include "Simulation/SimulationCamera.h"


ASimulationMode::ASimulationMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ASimulationController::StaticClass();
	DefaultPawnClass = nullptr;
}
