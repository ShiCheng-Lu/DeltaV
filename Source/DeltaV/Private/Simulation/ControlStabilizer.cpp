// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/ControlStabilizer.h"

#include "Common/Craft.h"
#include "Simulation/SimulationController.h"

// Sets default values for this component's properties
UControlStabilizer::UControlStabilizer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TimeSinceLastInput = 0;
}


// Called when the game starts
void UControlStabilizer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UControlStabilizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TimeSinceLastInput > TimeSinceLastInputThreshold) {
		// try to stabilize craft
		// Controller->craft->Rotate();




	}
	else {
		TargetOrientation = Controller->craft->GetActorRotation().Quaternion();
	}

	TimeSinceLastInput += DeltaTime;
}
