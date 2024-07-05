// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/OrbitComponent.h"
#include "Simulation/CelestialBody.h"

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

void UOrbitComponent::UpdateOrbit(FVector relative_position, FVector relative_velocity, ACelestialBody* in_central_body) {
	central_body = in_central_body;

	FVector angular_momentum_vector = relative_position.Cross(relative_velocity);
	axis_of_rotation = angular_momentum_vector.GetSafeNormal();
	angular_momentum = angular_momentum_vector.SquaredLength();

	double energy = relative_velocity.SquaredLength() / 2 - central_body->mu / relative_position.Length();
	eccentricity = sqrt(1 + 2 * energy * angular_momentum / (central_body->mu * central_body->mu));

	// find the periapsis point
	double angle = FMath::Acos((angular_momentum / (central_body->mu * relative_position.Length()) - 1) / eccentricity);

	periapsis_direction = relative_position.RotateAngleAxisRad(angle, axis_of_rotation);
	periapsis_direction.Normalize();

	UE_LOG(LogTemp, Warning, TEXT("values %f %f %f"), angular_momentum, eccentricity, angle);
}

FVector UOrbitComponent::GetPosition(float Time) {
	double angle = FMath::DegreesToRadians(Time);
	double radius = angular_momentum / (central_body->mu + central_body->mu * eccentricity * FMath::Cos(angle));

	FVector result = periapsis_direction.RotateAngleAxisRad(angle, axis_of_rotation);

	return result * radius;
}

FVector UOrbitComponent::GetVelocity(float Time) {
	return FVector();
}
