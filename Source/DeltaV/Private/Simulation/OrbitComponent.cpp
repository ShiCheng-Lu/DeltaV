// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/OrbitComponent.h"
#include "Simulation/CelestialBody.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"

// Sets default values for this component's properties
UOrbitComponent::UOrbitComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Game/Shapes/OrbitLine"));
	if (MeshFinder.Succeeded()) {
		UOrbitComponent::SplineMesh = MeshFinder.Object;
	}

	SetUsingAbsoluteLocation(true);
	SetUsingAbsoluteRotation(true);
	SetUsingAbsoluteScale(true);
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

	if (IsVisible()) {
		UpdateSplineWithOrbit();
	}
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

void UOrbitComponent::UpdateSpline() {
	Super::UpdateSpline();

	UE_LOG(LogTemp, Warning, TEXT("Spline updated"));
	
	// update spline meshes
	
	// remove extra spline mesh components
	for (int i = Spline.Num() - GetNumberOfSplineSegments(); i > 0; --i) {
		USplineMeshComponent* SplineComponent = Spline.Pop();
		SplineComponent->UnregisterComponent();
		SplineComponent->DestroyComponent();
	}
	// add missing spline mesh components
	for (int i = GetNumberOfSplineSegments() - Spline.Num(); i > 0; --i) {
		USplineMeshComponent* SplineComponent = NewObject<USplineMeshComponent>(this);
		SplineComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SplineComponent->SetStaticMesh(SplineMesh);
		SplineComponent->SetForwardAxis(ESplineMeshAxis::Z);
		SplineComponent->SetupAttachment(this);
		SplineComponent->RegisterComponent();
		Spline.Add(SplineComponent);
	}

	if (GetNumberOfSplineSegments() != Spline.Num()) {
		UE_LOG(LogTemp, Warning, TEXT("Something went terribly wrong, spline component length mismatch"));
	}

	// update spline mesh components
	FInterpCurveVector SplinePoints = GetSplinePointsPosition();

	for (int i = 0; i < Spline.Num(); ++i) {
		Spline[i]->SetStartAndEnd(
			SplinePoints.Points[i].OutVal,
			SplinePoints.Points[i].LeaveTangent,
			SplinePoints.Points[(i + 1) % Spline.Num()].OutVal,
			SplinePoints.Points[(i + 1) % Spline.Num()].ArriveTangent
		);
	}
}

void UOrbitComponent::UpdateSplineWithOrbit() {
	
	FVector Periapsis;
	FVector Apoapsis;

	FVector Midpoint = (Periapsis + Apoapsis) / 2;

	FVector FocalPoint;

	FVector HighFocalPoint = Periapsis - FocalPoint + Apoapsis;

	// construct until midpoint
	ClearSplinePoints(false);
	// while point doesn't cross midpoint axis
	//   add point

	
	// mirror into high half until apoapsis
	for (int i = GetNumberOfSplineSegments(); i > 0; --i) {
		FSplinePoint Point = GetSplinePointAt(i, ESplineCoordinateSpace::Local);
		FSplinePoint NewPoint = FSplinePoint(
			0, // key
			Point.Position, // TODO: reflect this across midpoint axis
			Point.LeaveTangent, // TODO: reflect this across midpont axis
			Point.ArriveTangent // TODO: reflect this across midpont axis
		);
		AddPoint(NewPoint, false);
	}

	// mirror into second half, from apoapsis back to periapsis
	for (int i = GetNumberOfSplineSegments() - 1; i > 1; --i) {
		FSplinePoint Point = GetSplinePointAt(i, ESplineCoordinateSpace::Local);
		FSplinePoint NewPoint = FSplinePoint(
			0, // key
			Point.Position, // TODO: reflect this across apsis axis
			Point.LeaveTangent, // TODO: reflect this across apsis axis
			Point.ArriveTangent // TODO: reflect this across apsis axis
		);
		AddPoint(NewPoint, false);
	}

	// call update spline once everything has updated to update the mesh
	UpdateSpline();
}

void UOrbitComponent::OnVisibilityChanged() {
	UpdateSplineWithOrbit();
}
