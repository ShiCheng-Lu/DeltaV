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

	Mode = EStabilizationMode::NONE;
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

	if (Controller->craft == nullptr) {
		return;
	}

	if (TimeSinceLastInput > TimeSinceLastInputThreshold) {
		if (Mode == EStabilizationMode::NONE) {
			return;
		}
		// try to stabilize craft
		// Controller->craft->Rotate();
		FQuat target = Controller->craft->GetActorRotation().Quaternion();
		FVector WorldAngularVelocity = Controller->craft->GetAngularVelocity();
		// rotate velocity into relative to craft pitch/yaw/roll
		FVector LocalAngularVelocity = target.Inverse().RotateVector(WorldAngularVelocity);

		FVector DeltaVector = target.Inverse().RotateVector(TargetOrientation.Vector());
		FRotator DeltaRotator = DeltaVector.ToOrientationRotator();
		
		FRotator Rotation = FRotator(0, 0, 0);
		Rotation.Pitch = FMath::RadiansToDegrees(LocalAngularVelocity.Y);
		Rotation.Yaw = -FMath::RadiansToDegrees(LocalAngularVelocity.Z);
		Rotation.Roll = FMath::RadiansToDegrees(LocalAngularVelocity.X);

		if (FMath::Abs(Rotation.Pitch) <= 0.1) {
			Rotation.Pitch = 0;
		}
		if (FMath::Abs(Rotation.Yaw) <= 0.1) {
			Rotation.Yaw = 0;
		}
		if (FMath::Abs(Rotation.Roll) <= 0.1) {
			Rotation.Roll = 0;
		}

		UE_LOG(LogTemp, Warning, TEXT("Target: %s, V: %s,  R: %s"),  *TargetOrientation.ToString(), *LocalAngularVelocity.ToString(), *Rotation.ToString());

		Controller->craft->Rotate(Rotation, 50000000);
	}
	else { // update target orientation
		switch (Mode)
		{
		case EStabilizationMode::NONE:
			break;
		case EStabilizationMode::HOLD_ATTITUDE:
			TargetOrientation = Controller->craft->GetActorRotation().Quaternion();
			break;
		case EStabilizationMode::MANEUVER:
			break;
		case EStabilizationMode::PROGRADE:
			break;
		case EStabilizationMode::RETROGRADE:
			break;
		case EStabilizationMode::RADIAL_IN:
			break;
		case EStabilizationMode::RADIAL_OUT:
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

	TimeSinceLastInput += DeltaTime;
}
