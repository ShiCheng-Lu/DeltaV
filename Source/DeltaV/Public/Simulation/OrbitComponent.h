// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OrbitComponent.generated.h"


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
	double gravitational_parameter;
	double angular_momentum;
	double eccentricity;

	double orbit_time;
	double time_at_periapsis;

	FVector periapsis;
	FVector central_body;
	FPlane orbital_plane;

	void UpdateOrbit(FVector orbiting_body, FVector velocity, FVector in_central_body);

	FVector GetPosition(float Time);

	FVector GetVelocity(float Time);

};
