// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosModularVehicle/VehicleSimThrusterComponent.h"
#include "VehicleSimThruster2DComponent.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UVehicleSimThruster2DComponent : public UVehicleSimThrusterComponent
{
	GENERATED_BODY()

public:
	UVehicleSimThruster2DComponent();
	virtual ~UVehicleSimThruster2DComponent() = default;
	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	float MaxThrustForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FVector ForceAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FVector ForceOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	bool bSteeringEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FVector SteeringAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	float MaxSteeringAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	float SteeringForceEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	float BoostMultiplierEffect;
	*/
	virtual Chaos::ISimulationModuleBase* CreateNewCoreModule() const override;
};
