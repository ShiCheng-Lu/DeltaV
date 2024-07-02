// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/OrbitComponent.h"

// Sets default values for this component's properties
UOrbitComponent::UOrbitComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UOrbitComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UOrbitComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

}


void UOrbitComponent::UpdateOrbit(FVector orbiting_body, FVector velocity, FVector in_central_body) {
	periapsis = orbiting_body;
	central_body = in_central_body;

	mass = 10;

	orbital_plane = FPlane(central_body, periapsis, periapsis + velocity);

	// constant for an orbit
	double radius = FVector::Distance(central_body, periapsis);
	FVector angular_component = velocity.ProjectOnTo(orbital_plane.GetNormal().Cross(orbiting_body - central_body));
	double angular_velocity = angular_component.Length() / radius;
	angular_momentum = mass * radius * radius * angular_velocity;

	double radial_velocity = (velocity - angular_component).Length();

	// double energy = mass * gravitational_parameter + mass * velocity.SquaredLength();
	eccentricity = angular_momentum * angular_momentum / (mass * mass * gravitational_parameter * radius) - 1;

	UE_LOG(LogTemp, Warning, TEXT("setup: %f, %f"), angular_momentum, eccentricity);
}

FVector UOrbitComponent::GetPosition(float Time) {
	double angle = FMath::DegreesToRadians(Time);
	double radius = angular_momentum * angular_momentum / (mass * mass * gravitational_parameter * (1 + eccentricity * FMath::Cos(angle)));

	FVector result = (periapsis - central_body).RotateAngleAxisRad(angle, orbital_plane.GetNormal());
	result.Normalize();

	return result * radius + central_body;
}

FVector UOrbitComponent::GetVelocity(float Time) {
	return FVector();
}
