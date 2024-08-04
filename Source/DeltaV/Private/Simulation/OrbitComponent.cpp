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

void UOrbitComponent::UpdateOrbit(FVector OrbitRelativeLocation, FVector RelativeVelocity, TObjectPtr<ACelestialBody> CelestialBody) {
	CentralBody = CelestialBody;

	FVector AngularMomentumVector = OrbitRelativeLocation.Cross(RelativeVelocity);
	AxisOfRotation = AngularMomentumVector.GetSafeNormal();
	AngularMomentum = AngularMomentumVector.SquaredLength();

	double Energy = RelativeVelocity.SquaredLength() / 2 - CentralBody->Mu / OrbitRelativeLocation.Length();
	Eccentricity = sqrt(1 + 2 * Energy * AngularMomentum / (CentralBody->Mu * CentralBody->Mu));

	// find the periapsis point
	double Angle = FMath::Acos((AngularMomentum / (CentralBody->Mu * OrbitRelativeLocation.Length()) - 1) / Eccentricity);

	PeriapsisDirection = OrbitRelativeLocation.RotateAngleAxisRad(Angle, AxisOfRotation);
	PeriapsisDirection.Normalize();

	UE_LOG(LogTemp, Warning, TEXT("values %f %f %f"), AngularMomentum, Eccentricity, Angle);

	if (IsVisible()) {
		UpdateSplineWithOrbit();
	}
}

FVector UOrbitComponent::GetPosition(float Time) {
	// Current implmemntation evaluate time as degrees after periapsis
	double Angle = FMath::DegreesToRadians(Time);
	double Radius = AngularMomentum / (CentralBody->Mu + CentralBody->Mu * Eccentricity * FMath::Cos(Angle));

	FVector Result = PeriapsisDirection.RotateAngleAxisRad(Angle, AxisOfRotation);

	return Result * Radius;
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
	int TotalPoints = 0;
	while (true) {
		FVector PointLocation = GetPosition(TotalPoints); // 
		FVector PointTangent;
		FSplinePoint Point(TotalPoints, PointLocation, PointTangent, PointTangent);
		break;
	}

	
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

	for (int i = 0; i < GetNumberOfSplineSegments(); ++i) {
		
	}

	// call update spline once everything has updated to update the mesh
	UpdateSpline();
}

void UOrbitComponent::OnVisibilityChanged() {
	if (IsVisible()) {
		UpdateSplineWithOrbit();
	}
}
