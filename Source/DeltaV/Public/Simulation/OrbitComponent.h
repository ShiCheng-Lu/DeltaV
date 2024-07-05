// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OrbitComponent.generated.h"

class ACelestialBody;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UOrbitComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOrbitComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	double mass;

	double angular_momentum;
	FVector axis_of_rotation;
	double eccentricity;
	FVector periapsis_direction;

	double orbit_time;
	double time_at_periapsis;
	ACelestialBody* central_body;

	void UpdateOrbit(FVector relative_position, FVector relative_velocity, ACelestialBody* central_body);

	FVector GetPosition(float Time);

	FVector GetVelocity(float Time);
};
