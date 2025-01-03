// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ControlStabilizer.generated.h"

class ASimulationController;

UENUM()
enum EStabilizationMode : uint8 {
	NONE,
	HOLD_ATTITUDE,
	MANEUVER,
	PROGRADE,
	RETROGRADE,
	RADIAL_IN,
	RADIAL_OUT,
	NORMAL,
	ANTI_NORMAL,
	TARGET,
	ANTI_TARGET,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UControlStabilizer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UControlStabilizer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	float TimeSinceLastInput;
	float TimeSinceLastInputThreshold;

	ASimulationController* Controller;
	FVector TargetOrientation;
	EStabilizationMode Mode;
};
