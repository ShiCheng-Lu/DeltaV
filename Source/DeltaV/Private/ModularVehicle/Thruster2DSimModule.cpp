// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularVehicle/Thruster2DSimModule.h"

namespace Chaos {
	FThruster2DSimModule::FThruster2DSimModule(const FThruster2DSettings& Settings)
		: TSimModuleSettings<FThruster2DSettings>(Settings)
	{

	}

	void FThruster2DSimModule::Simulate(float DeltaTime, const FAllInputs& Inputs, FSimModuleTree& VehicleModuleSystem)
	{
		double Pitch = Inputs.GetControls().GetMagnitude(PitchControlName);
		double Roll = Inputs.GetControls().GetMagnitude(RollControlName);
		double Yaw = Inputs.GetControls().GetMagnitude(YawControlName);

		SteerAngle = Pitch * Setup().Pitch + Roll * Setup().Roll + Yaw * Setup().Yaw;
		if (SteerAngle.SizeSquared() > 1) {
			SteerAngle.Normalize();
		}
		// = Response x Vector(0, 0, 1) (thrust direction)
		FVector RotationAxis = FVector(0, SteerAngle.Y, -SteerAngle.X);
	
		if (RotationAxis.SizeSquared() > UE_SMALL_NUMBER) {
			Steer = FQuat(RotationAxis, SteerAngle.Size() * FMath::DegreesToRadians(Setup().MaxSteeringAngle));
			Steer.Normalize();
		}
		else {
			Steer = FQuat(FVector(0, 0, 1), 0);
		}

		// applies continuous force
		FVector Force = Setup().ForceAxis * Setup().MaxThrustForce * Inputs.GetControls().GetMagnitude("Thrust");
		AddLocalForceAtPosition(Steer.RotateVector(Force), Setup().ForceOffset, true, false, false, FColor::Magenta);
	}

	void FThruster2DSimModule::Animate()
	{
		AnimationData.AnimFlags = EAnimationFlags::AnimateRotation;
		AnimationData.AnimationRotOffset.Pitch = SteerAngle.X;
		AnimationData.AnimationRotOffset.Yaw = SteerAngle.Y;
		// TODO: Animate rotation like wheel steering

		AnimationData.CombinedRotation = Steer;
	}
}
