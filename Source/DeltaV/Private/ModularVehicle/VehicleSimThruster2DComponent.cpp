// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularVehicle/VehicleSimThruster2DComponent.h"

#include "SimModule/SimModulesInclude.h"
#include "ModularVehicle/Thruster2DSimModule.h"

UVehicleSimThruster2DComponent::UVehicleSimThruster2DComponent() : Super() {
	
}

Chaos::ISimulationModuleBase* UVehicleSimThruster2DComponent::CreateNewCoreModule() const {
	Chaos::FThruster2DSettings Settings;

	Settings.MaxThrustForce = MaxThrustForce;
	Settings.ForceAxis = ForceAxis;
	Settings.ForceOffset = ForceOffset;
	Settings.SteeringEnabled = bSteeringEnabled;
	Settings.SteeringAxis = SteeringAxis;
	Settings.MaxSteeringAngle = MaxSteeringAngle;
	Settings.SteeringForceEffect = SteeringForceEffect;
	Settings.BoostMultiplier = BoostMultiplierEffect;

	Chaos::ISimulationModuleBase* Thruster = new Chaos::FThruster2DSimModule(Settings);
	Thruster->SetAnimationEnabled(bAnimationEnabled);

	return Thruster;
}
