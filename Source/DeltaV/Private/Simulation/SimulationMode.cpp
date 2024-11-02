// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationMode.h"

#include "Simulation/SimulationController.h"
#include "Generation/SimplexNoise.h"

ASimulationMode::ASimulationMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ASimulationController::StaticClass();
	DefaultPawnClass = nullptr;

	SimplexNoise::At(FVector(0.1, -1.1, 3.6));
}
