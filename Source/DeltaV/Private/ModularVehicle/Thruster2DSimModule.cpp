// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularVehicle/Thruster2DSimModule.h"

namespace Chaos {
	FThruster2DSimModule::FThruster2DSimModule(const FThruster2DSettings& Settings)
		: TSimModuleSettings<FThruster2DSettings>(Settings)
	{

	}

	void FThruster2DSimModule::Simulate(float DeltaTime, const FAllInputs& Inputs, FSimModuleTree& VehicleModuleSystem)
	{
		SteerAngleDegrees = 0.0f;
		if (Setup().SteeringEnabled)
		{
			SteerAngleDegrees = Setup().SteeringEnabled ? Inputs.GetControls().GetMagnitude(TEXT("Steering")) * Setup().MaxSteeringAngle : 0.0f;
		}

		// applies continuous force
		float BoostEffect = Inputs.GetControls().GetMagnitude(BoostControlName) * Setup().BoostMultiplier;
		FVector Force = Setup().ForceAxis * Setup().MaxThrustForce * Inputs.GetControls().GetMagnitude(ThrottleControlName) * (1.0f + BoostEffect);
		FQuat Steer = FQuat(Setup().SteeringAxis, FMath::DegreesToRadians(SteerAngleDegrees) * Setup().SteeringForceEffect);
		AddLocalForceAtPosition(Steer.RotateVector(Force), Setup().ForceOffset, true, false, false, FColor::Magenta);
	}

	void FThruster2DSimModule::Animate()
	{
		AnimationData.AnimFlags = EAnimationFlags::AnimateRotation;
		AnimationData.AnimationRotOffset.Yaw = SteerAngleDegrees;
		// TODO: Animate rotation like wheel steering
	}
}