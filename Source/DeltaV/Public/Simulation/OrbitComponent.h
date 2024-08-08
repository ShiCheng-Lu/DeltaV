// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "OrbitComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UOrbitComponent : public USplineComponent
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

	double Mass;

	double AngularMomentum; // (cm)^2
	FVector AxisOfRotation;
	double Eccentricity; // unitless
	FVector PeriapsisDirection;

	double OrbitDuration;
	double TimeAtPeriapsis;
	TObjectPtr<class ACelestialBody> CentralBody;

	void UpdateOrbit(FVector RelativeLocation, FVector RelativeVelocity);

	FVector GetPosition(float Time);

	FVector GetVelocity(float Time);

	inline static TObjectPtr<class UStaticMesh> SplineMesh;

	TArray<TObjectPtr<class USplineMeshComponent>> Spline;

	void UpdateSplineWithOrbit();

	virtual void UpdateSpline() override;

	virtual void OnVisibilityChanged() override;

	double Periapsis();

	double Apoapsis();
};
