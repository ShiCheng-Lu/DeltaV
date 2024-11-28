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

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	SetClosedLoop(true, false);
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


}

void UOrbitComponent::UpdateOrbit(FVector OrbitRelativeLocation, FVector RelativeVelocity, double Time) {
	if (CentralBody == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("No CentralBody"));
		return;
	}

	AngularMomentum = OrbitRelativeLocation.Cross(RelativeVelocity);
	AxisOfRotation = AngularMomentum.GetSafeNormal();
	AngularMomentumSquared = AngularMomentum.SquaredLength(); // (cm)^2

	double Energy = RelativeVelocity.SquaredLength() / 2 - CentralBody->Mu / OrbitRelativeLocation.Length(); // (cm/s)^2 - (cm)^3(s^-2) / (cm) = (cm^2)(s^-2)
	Eccentricity = sqrt(1 + 2 * Energy * AngularMomentumSquared / (CentralBody->Mu * CentralBody->Mu)); // (cm^2)(s^-2)(cm)^2 / (cm)^3(s^-2)(cm)^3(s^-2) = 1
	EccentricityVector = ((RelativeVelocity.SquaredLength() - CentralBody->Mu / OrbitRelativeLocation.Length()) * OrbitRelativeLocation
		- (OrbitRelativeLocation.Dot(RelativeVelocity) * RelativeVelocity)) / CentralBody->Mu;

	// find the periapsis point
	double Angle = FMath::Acos((AngularMomentumSquared / (CentralBody->Mu * OrbitRelativeLocation.Length()) - 1) / Eccentricity);
	if (RelativeVelocity.ProjectOnTo(OrbitRelativeLocation).GetSafeNormal().Equals(OrbitRelativeLocation.GetSafeNormal())) {
		Angle *= -1;
	}

	PeriapsisDirection = OrbitRelativeLocation.RotateAngleAxisRad(Angle, AxisOfRotation);
	PeriapsisDirection.Normalize();

	double SemiMajorAxis = AngularMomentum.SquaredLength() / (CentralBody->Mu * (1 - Eccentricity * Eccentricity));
	double SemiMinorAxis = SemiMajorAxis * FMath::Sqrt(1 - Eccentricity * Eccentricity);
	OrbitDuration = 2 * PI * SemiMajorAxis * SemiMinorAxis / AngularMomentum.Length();

	TimeAtPeriapsis = Time - GetTime(Angle);

	UE_LOG(LogTemp, Warning, TEXT("values %f %s %f %f %f %f"), AngularMomentum.Length(), *RelativeVelocity.ToString(), Eccentricity, Angle, OrbitDuration, SemiMajorAxis * SemiMinorAxis);

	if (IsVisible()) {
		UpdateSplineWithOrbit();
	}
}

double UOrbitComponent::GetTime(double TrueAnomaly) {
	double MeanAnomaly = TrueAnomaly;
	if (Eccentricity < 1) {
		double SinAnomaly, CosAnomaly;
		FMath::SinCos(&SinAnomaly, &CosAnomaly, TrueAnomaly);
		MeanAnomaly = 2 * FMath::Atan(FMath::Sqrt((1 - Eccentricity) / (1 + Eccentricity)) * FMath::Tan(TrueAnomaly / 2))
			- (Eccentricity * FMath::Sqrt(1 - Eccentricity * Eccentricity) * SinAnomaly) / (1 + Eccentricity * CosAnomaly);

		return MeanAnomaly * OrbitDuration / (PI * 2);
	}
	else if (Eccentricity == 1) {
		double TanAnomaly = FMath::Tan(TrueAnomaly / 2);
		MeanAnomaly = TanAnomaly / 2 + TanAnomaly * TanAnomaly * TanAnomaly / 6;

		return MeanAnomaly * AngularMomentumSquared * FMath::Sqrt(AngularMomentumSquared) / (CentralBody->Mu * CentralBody->Mu);
	}
	else {
		return 0;
	}
}

double UOrbitComponent::GetTrueAnomaly(double Time) const {
	if (OrbitDuration == 0) {
		UE_LOG(LogTemp, Warning, TEXT("Orbit duration is 0"), Eccentricity);
		return 0;
	}
	// get angle from time after periapsis
	// if e < 1:
	// u^2 / h^3 * t = (2 tan-1 ( sqrt((1 - e) / (1 + e)) tan (a / 2) ) - (e sqrt(1 - e^2) sin a) / (1 + e cos a)) / (e^2 - 1)^(3/2)
	// 
	// if e = 1:
	// u^2 / h^3 * t = (tan(v / 2) / 2 + tan^3(v / 2) / 6)
	// 
	// if e > 1
	// u^2 / h^3 * t = ((e sqrt(e^2 - 1) sin a) / (1 + e cos a) - ln((sqrt(e + 1) + sqrt(e - 1) tan (a / 2)) / (sqrt(e + 1) - sqrt(e - 1) tan (a / 2)))) / (e^2 - 1)^(3/2)

	static double LastEccentricAnomalyGuess = 0;
	if (Eccentricity < 1) {
		double MeanAnomaly = FMath::Modulo(Time - TimeAtPeriapsis, OrbitDuration) * (2 * PI) / OrbitDuration;
		// solve EccentricAnomaly - Eccentricity * sin(EccentricAnomaly) - MeanAnomaly = f(EccentricAnomaly) = 0
		//   with derivative f'(EccentricAnomaly) = 1 - Eccentricity * cos(EccentricAnomaly)
		double EccentricAnomaly = 0;
		double SinEccentricityAnomaly, CosEccentricityAnomaly, FunctionValue, DerivativeValue;

		int i = 0;

		do { // Newton's method to find the root (FunctionValue == 0)
			FMath::SinCos(&SinEccentricityAnomaly, &CosEccentricityAnomaly, EccentricAnomaly);
			FunctionValue = EccentricAnomaly - Eccentricity * SinEccentricityAnomaly - MeanAnomaly;
			DerivativeValue = 1 - Eccentricity * CosEccentricityAnomaly;
			EccentricAnomaly = EccentricAnomaly - FunctionValue / DerivativeValue;
		} while (FMath::Abs(FunctionValue) > 1e-13 && i++ < 10);

		LastEccentricAnomalyGuess = EccentricAnomaly;

		double TrueAnomaly = FMath::Atan(FMath::Tan(EccentricAnomaly / 2) / FMath::Sqrt((1 - Eccentricity) / (1 + Eccentricity))) * 2;

		return TrueAnomaly;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Eccentricity %f does not apply"), Eccentricity);
		return 0; // todo: 
	}
}

void UOrbitComponent::GetPositionAndVelocity(FVector* Position, FVector* Velocity, double TrueAnomaly) const {
	FVector RightApsisDirection = AngularMomentum.GetSafeNormal().Cross(PeriapsisDirection);

	// Position and Velocity
	double SinAnomaly, CosAnomaly; // no sin/cos approximation for doubles, Sin Cos is evaluated individually. maybe we can just use float (depending on desired precision)
	FMath::SinCos(&SinAnomaly, &CosAnomaly, TrueAnomaly);

	if (Position) {
		*Position = CosAnomaly * PeriapsisDirection + SinAnomaly * RightApsisDirection; // Position direction
		*Position *= AngularMomentum.SquaredLength() / (CentralBody->Mu + CentralBody->Mu * Eccentricity * CosAnomaly); // Position Radius
	}
	if (Velocity) {
		*Velocity = -SinAnomaly * PeriapsisDirection + (Eccentricity + CosAnomaly) * RightApsisDirection; // Velocity direction
		*Velocity *= CentralBody->Mu / AngularMomentum.Length(); // Velocity magnitude
	}
}

void UOrbitComponent::UpdateSpline() {
	Super::UpdateSpline();

	if (GetNumberOfSplineSegments() > 10) {
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("Spline updated"));
	
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
		SplineComponent->SetMobility(EComponentMobility::Movable);
		SplineComponent->RegisterComponent();
		Spline.Add(SplineComponent);
	}

	if (GetNumberOfSplineSegments() != Spline.Num()) {
		UE_LOG(LogTemp, Warning, TEXT("Something went terribly wrong, spline component length mismatch"));
	}

	// update spline mesh components
	FInterpCurveVector SplinePoints = GetSplinePointsPosition();

	for (int i = 0; i < GetNumberOfSplinePoints() - 1; ++i) {
		Spline[i]->SetStartAndEnd(
			SplinePoints.Points[i].OutVal + CentralBody->GetActorLocation(),
			SplinePoints.Points[i].LeaveTangent,
			SplinePoints.Points[i + 1].OutVal + CentralBody->GetActorLocation(),
			SplinePoints.Points[i + 1].ArriveTangent
		);
	}
	if (IsClosedLoop()) {
		int i = GetNumberOfSplinePoints() - 1;
		Spline[i]->SetStartAndEnd(
			SplinePoints.Points[i].OutVal + CentralBody->GetActorLocation(),
			SplinePoints.Points[i].LeaveTangent,
			SplinePoints.Points[0].OutVal + CentralBody->GetActorLocation(),
			SplinePoints.Points[0].ArriveTangent
		);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Update Spline itself"));
}

double UOrbitComponent::Periapsis() {
	if (CentralBody == nullptr) {
		return 0;
	}
	return AngularMomentum.SquaredLength() / (CentralBody->Mu + CentralBody->Mu * Eccentricity);
}

double UOrbitComponent::Apoapsis() {
	if (CentralBody == nullptr) {
		return 0;
	}
	return AngularMomentum.SquaredLength() / (CentralBody->Mu - CentralBody->Mu * Eccentricity);
}

void UOrbitComponent::UpdateSplineWithOrbit() {

	if (Eccentricity > 0.95) {
		return; // Cannot draw Eclipse with eccentricity over 0.95 for now, needs a different strategy
	}

	UE_LOG(LogTemp, Warning, TEXT("UpdateSplineWithOrbit"));

	ClearSplinePoints(false);
	// SetClosedLoop(true, false);
	double SemiMajorAxis = AngularMomentum.SquaredLength() / (CentralBody->Mu * (1 - Eccentricity * Eccentricity));
	double SemiMinorAxis = SemiMajorAxis * FMath::Sqrt(1 - Eccentricity * Eccentricity);

	SetWorldLocation(FVector(0, 0, 0));
	// transform circle into orbit?

	FTransform Transform; // scale, rotate, and translate
	Transform.SetScale3D(FVector(SemiMinorAxis, SemiMajorAxis, 1));
	Transform.SetTranslation(-PeriapsisDirection * SemiMajorAxis * Eccentricity);

	// while point doesn't cross midpoint axis
	//   add point
	int TotalPoints = 0;
	
	for (int i = 0; i < 4; ++i) {
		// https://spencermortensen.com/articles/bezier-circle/

		FVector PointLocation = Transform.TransformPosition(FVector(0, -1.00005519, 0).RotateAngleAxis(i * 90, FVector(0, 0, 1)));
		FVector PointTangent = Transform.TransformVector(FVector(1.66028058, 0.00379245, 0).RotateAngleAxis(i * 90, FVector(0, 0, 1)));
		// PointTangent.Normalize();
		// PointTangent *= 1000;
		FSplinePoint NewPoint = FSplinePoint(TotalPoints, PointLocation, PointTangent, PointTangent);
		AddPoint(NewPoint, false);
		TotalPoints += 1;
	}

	/*
	
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
	*/

	// call update spline once everything has updated to update the mesh
	UpdateSpline();

	// SetWorldLocation(- PeriapsisDirection * SemiMajorAxis * Eccentricity);
}

void UOrbitComponent::OnVisibilityChanged() {
	if (IsVisible()) {
		UpdateSplineWithOrbit();
	}
}
