// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SimModule/ThrusterModule.h"

namespace Chaos
{
	struct FAllInputs;
	class FSimModuleTree;

	const FName ThrustControl("Thrust");

	/// <summary>
	/// Thruster2D settings
	/// </summary>
	struct FThruster2DSettings : FThrusterSettings
	{
		FThruster2DSettings()
			: FThrusterSettings()
		{
			Pitch = FVector2D(0, -1);
			Roll = FVector2D(1, 0);
			Yaw = FVector2D(1, 0);
		}

		FVector2D Pitch;
		FVector2D Roll;
		FVector2D Yaw;
	};

	/// <summary>
	/// A vehicle component that transmits torque from one source to another, i.e. from an engine or differential to wheels
	///
	/// </summary>
	class FThruster2DSimModule : public ISimulationModuleBase, public TSimModuleSettings<FThruster2DSettings>, public TSimulationModuleTypeable<FThruster2DSimModule>
	{
	public:
		DEFINE_CHAOSSIMTYPENAME(FThruster2DSimModule);
		FThruster2DSimModule(const FThruster2DSettings& Settings);

		virtual TSharedPtr<FModuleNetData> GenerateNetData(const int32 NodeArrayIndex) const override { return nullptr; }

		virtual const FString GetDebugName() const { return TEXT("Thruster2D"); }

		virtual bool IsBehaviourType(eSimModuleTypeFlags InType) const override { return (InType & NonFunctional); }

		virtual void Simulate(float DeltaTime, const FAllInputs& Inputs, FSimModuleTree& VehicleModuleSystem) override;

		virtual void Animate() override;

	private:
		FVector2D SteerAngle;
		FQuat Steer;
	};


} // namespace Chaos
