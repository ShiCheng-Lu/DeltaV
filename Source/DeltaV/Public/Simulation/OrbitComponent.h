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

	UPROPERTY()
	double Mass;

	UPROPERTY()
	FVector AngularMomentum;

	UPROPERTY()
	double AngularMomentumSquared; // (cm)^2

	UPROPERTY()
	FVector AxisOfRotation;

	UPROPERTY()
	double Eccentricity; // unitless

	UPROPERTY()
	FVector EccentricityVector;

	UPROPERTY()
	FVector PeriapsisDirection;

	UPROPERTY()
	double OrbitDuration;

	UPROPERTY()
	double TimeAtPeriapsis;

	UPROPERTY()
	TObjectPtr<class ACelestialBody> CentralBody;

	void UpdateOrbit(FVector RelativeLocation, FVector RelativeVelocity, double Time);

	inline static TObjectPtr<class UStaticMesh> SplineMesh;

	UPROPERTY()
	TArray<TObjectPtr<class USplineMeshComponent>> Spline;

	void UpdateSplineWithOrbit();

	virtual void UpdateSpline() override;

	virtual void OnVisibilityChanged() override;

	double Periapsis(); // cm

	double Apoapsis(); // cm

	/**
	 Get relative position and velocity to the parent body at true anomaly
	*/
	void GetPositionAndVelocity(FVector* Position, FVector* Velocity, double TrueAnomaly) const;

	double GetTime(double TrueAnomaly);

	double GetTrueAnomaly(double Time) const;
};
