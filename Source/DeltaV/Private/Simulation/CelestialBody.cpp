 // Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/CelestialBody.h"

#include "GameFramework/GameStateBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"

#include "Simulation/OrbitComponent.h"

// Sets default values
ACelestialBody::ACelestialBody(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer),
	PostPhysics(ETickingGroup::TG_PostPhysics, &ACelestialBody::TickPostPhysics)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("StaticMesh'/Game/Shapes/Shape_Sphere.Shape_Sphere'"));
	if (MeshFinder.Succeeded()) {
		Mesh->SetStaticMesh(MeshFinder.Object);
	}

	SetRootComponent(Mesh);

	// Mesh->SetSimulatePhysics(true);
	Mesh->Mobility = EComponentMobility::Movable;
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	Mu = Mesh->CalculateMass() * 6.6743E-5; // G: 6.6743E-5 (cN)(cm^2)(kg^-2), Mu: (cN)(cm^2)(kg^-1) = (cm^3)(s^-2)

	Orbit = CreateDefaultSubobject<UOrbitComponent>("Orbit");
}

// Called when the game starts or when spawned
void ACelestialBody::BeginPlay()
{
	Super::BeginPlay();
	
	Orbit->CentralBody = Cast<ACelestialBody>(GetAttachParentActor());
	if (Orbit->CentralBody) {
		PrimaryActorTick.AddPrerequisite(Orbit->CentralBody, Orbit->CentralBody->PrimaryActorTick);

		Mesh->SetSimulatePhysics(true);

		Orbit->UpdateOrbit(GetActorLocation() - Orbit->CentralBody->GetActorLocation(), InitialVelocity, GetGameTimeSinceCreation());
	}

	TargetPosition = GetActorLocation();

	// // Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>("Atmosphere");
	// // Atmosphere->TransformMode = ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform;

	// // Atmosphere->SetupAttachment(Mesh);
	// Orbit->SetVisibility(true);

	// UE_LOG(LogTemp, Warning, TEXT("CelestInit: %s - %s"), *InitialVelocity.ToString(), *GetActorLocation().ToString());
	// UE_LOG(LogTemp, Warning, TEXT("CelestInit: %f - %f"), Orbit->CentralBody, Orbit->Eccentricity);
}

// Called every frame
void ACelestialBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// GetWorld()->GetGameState();
	double Time = GetGameTimeSinceCreation();
	Mesh->SetPhysicsAngularVelocityInRadians(FVector(0, 0, -2 * PI / RotationPeriod));
	
	if (Orbit->CentralBody) {
		double TrueAnomaly = Orbit->GetTrueAnomaly(Time + DeltaTime);
		Orbit->GetPositionAndVelocity(&TargetPosition, nullptr, TrueAnomaly);
		TargetPosition += Orbit->CentralBody->TargetPosition;
		FVector VelocityChange = (TargetPosition - GetActorLocation()) / DeltaTime - GetVelocity();

		Mesh->AddImpulse(VelocityChange, NAME_None, true);
		
		// UE_LOG(LogTemp, Warning, TEXT("%s pre phys tick pos: %s, expect next %s"), *Name, *GetActorLocation().ToString(), *TargetPosition.ToString());
	}
}

void ACelestialBody::TickPostPhysics(float DeltaTime) {
	if (Orbit->CentralBody) {
		// UE_LOG(LogTemp, Warning, TEXT("%s post tick pos: %s, vel %s"), *Name, *GetActorLocation().ToString(), *GetVelocity().ToString());

		Orbit->UpdateSpline();
	}
}


#if WITH_EDITOR
void ACelestialBody::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();
	UE_LOG(LogTemp, Warning, TEXT("Changed: %s - %s"), *PropertyName.ToString(), *PropertyChangedEvent.GetPropertyName().ToString());

	if (PropertyName == "RelativeScale3D" || PropertyName == "Mesh") {
		Radius = FMath::Max(Mesh->GetRelativeScale3D().X, 0); // the sphere is 1m, so radius in meters is the scale
		Mesh->SetRelativeScale3D(FVector(Radius));
		Mu = Mesh->CalculateMass() * 6.6743E-5;
		// Atmosphere->SetBottomRadius(Radius / 1000); // in km
	}

	if (PropertyName == "InitialVelocity" || PropertyName == "RelativeLocation") {
		// Draw Debug
		Orbit->CentralBody = Cast<ACelestialBody>(GetAttachParentActor());
		Orbit->SetVisibility(true);
		if (Orbit->CentralBody) {
			Orbit->UpdateOrbit(GetActorLocation() - Orbit->CentralBody->GetActorLocation(), InitialVelocity, GetGameTimeSinceCreation());
			Orbit->UpdateSplineWithOrbit();

			if (Orbit->Eccentricity >= 1) {
				UE_LOG(LogTemp, Warning, TEXT("WARNING: %s is on Hyperbolic orbit"), *Name);
			}
		}
	}

	if (PropertyName == "ActorLabel") {
		Name = GetActorLabel();
	}
}

void ACelestialBody::PostEditMove(bool bFinished) {
	Super::PostEditMove(bFinished);

	UE_LOG(LogTemp, Warning, TEXT("MOVE %d"), bFinished);
	if (bFinished) {
	}
}
#endif
