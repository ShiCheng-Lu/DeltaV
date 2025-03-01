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

	TimeSinceLastInputThreshold = 0.2;
	Controller = nullptr;
}


// Called when the game starts
void UControlStabilizer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	UE_LOG(LogTemp, Warning, TEXT("Stab begin play"));
}

// Called every frame
void UControlStabilizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	return;
	/*
	ACraft* Craft = Controller->Craft;
	if (Craft == nullptr) {
		return;
	}
	if (Mode == EStabilizationMode::NONE) {
		return;
	}

	if (Mode == EStabilizationMode::HOLD_ATTITUDE) {
		TimeSinceLastInput += DeltaTime;
		if (TimeSinceLastInput < TimeSinceLastInputThreshold) {
			TargetOrientation = Craft->GetActorRotation().Vector();
			return; // for normal stabiliztion mode, only update target while there is user input
		}
	}
	else {
		// TODO: some of these would depend on reference frame
		switch (Mode)
		{
		case EStabilizationMode::NONE:
			break;
		case EStabilizationMode::HOLD_ATTITUDE: // handled in previous case
			break;
		case EStabilizationMode::MANEUVER:
			break;
		case EStabilizationMode::PROGRADE:
			TargetOrientation = Craft->GetVelocity();
			break;
		case EStabilizationMode::RETROGRADE:
			TargetOrientation = -Craft->GetVelocity();
			break;
		case EStabilizationMode::RADIAL_IN:
			TargetOrientation = -Craft->GetActorLocation();
			break;
		case EStabilizationMode::RADIAL_OUT:
			TargetOrientation = Craft->GetActorLocation();
			break;
		case EStabilizationMode::NORMAL:
			break;
		case EStabilizationMode::ANTI_NORMAL:
			break;
		case EStabilizationMode::TARGET:
			break;
		case EStabilizationMode::ANTI_TARGET:
			break;
		default:
			break;
		}
	}

	FQuat CurrentRotation = Craft->GetActorQuat();
	FVector TargetAngularVel = CurrentRotation.Inverse().RotateVector(TargetOrientation);
	FVector CurrentAngularVel = CurrentRotation.Inverse().RotateVector(Craft->GetAngularVelocity());

	// stablizing via PD controller, with coefficients of 1
	// proportional component
	FRotator Rotation = TargetAngularVel.ToOrientationRotator();
	// derivative component
	Rotation.Pitch += FMath::RadiansToDegrees(CurrentAngularVel.Y);
	Rotation.Yaw -= FMath::RadiansToDegrees(CurrentAngularVel.Z);
	Rotation.Roll += FMath::RadiansToDegrees(CurrentAngularVel.X);

	if (FMath::Abs(Rotation.Pitch) <= 0.1) {
		Rotation.Pitch = 0;
	}
	if (FMath::Abs(Rotation.Yaw) <= 0.1) {
		Rotation.Yaw = 0;
	}
	if (FMath::Abs(Rotation.Roll) <= 0.1) {
		Rotation.Roll = 0;
	}

	Controller->Craft->Rotate(Rotation, 100000000);
	*/
}
