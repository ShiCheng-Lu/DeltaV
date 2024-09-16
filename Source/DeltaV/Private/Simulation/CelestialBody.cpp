 // Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/CelestialBody.h"

#include "GameFramework/GameStateBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"

#include "Simulation/OrbitComponent.h"

// Sets default values
ACelestialBody::ACelestialBody(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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
		Orbit->CentralBody->Mu = Orbit->CentralBody->Mesh->CalculateMass() * 6.6743E-5;
		UE_LOG(LogTemp, Warning, TEXT("Mass %f"), Orbit->CentralBody->Mu);
	}
	Mesh->SetSimulatePhysics(true);

	// // Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>("Atmosphere");
	// // Atmosphere->TransformMode = ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform;

	// // Atmosphere->SetupAttachment(Mesh);
	Orbit->UpdateOrbit(GetActorLocation(), InitialVelocity);
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

	FVector Position, Velocity;
	// Orbit->GetPositionAndVelocity(&Position, &Velocity, Time);

	// FVector Pos2 = Orbit->GetPosition(Time);

	// SetActorLocation(Position);

	double RotationPeriod = 60; // 60 seconds for a full rotation

	UE_LOG(LogTemp, Warning, TEXT("Tick Time: %f"), Time);

	SetActorRotation(FRotator(0, Time / RotationPeriod, 0));
	if (Orbit->CentralBody) {
		FVector Postion;
		double TrueAnomaly = Orbit->GetTrueAnomaly(Time);
		Orbit->GetPositionAndVelocity(&Position, nullptr, TrueAnomaly);
		SetActorLocation(Position);


		UE_LOG(LogTemp, Warning, TEXT("Pos %s   %f"), *Position.ToString(), TrueAnomaly);
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
		UE_LOG(LogTemp, Warning, TEXT("Mu %f"), Mu);
	}

	if (PropertyName == "InitialVelocity" || PropertyName == "RelativeLocation") {
		// Draw Debug
		Orbit->CentralBody = Cast<ACelestialBody>(GetAttachParentActor());
		Orbit->UpdateOrbit(GetActorLocation(), InitialVelocity);
	}
}

void ACelestialBody::PostEditMove(bool bFinished) {
	Super::PostEditMove(bFinished);

	UE_LOG(LogTemp, Warning, TEXT("MOVE %d"), bFinished);
	if (bFinished) {
	}
}
#endif
