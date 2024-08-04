 // Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/CelestialBody.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"

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

	Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>("Atmosphere");
	Atmosphere->TransformMode = ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform;

	SetRootComponent(Mesh);

	Atmosphere->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void ACelestialBody::BeginPlay()
{
	Super::BeginPlay();

	Mu = Mesh->CalculateMass() * 6.6743E-11;
}

// Called every frame
void ACelestialBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


#if WITH_EDITOR
void ACelestialBody::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("Changed: %s"), *PropertyChangedEvent.GetPropertyName().ToString());
	if (PropertyChangedEvent.GetPropertyName() == "RelativeScale3D" || PropertyChangedEvent.GetPropertyName() == "Mesh") {
		Radius = FMath::Max(Mesh->GetRelativeScale3D().X, 0);
		Mesh->SetRelativeScale3D(FVector(Radius));
		Atmosphere->SetBottomRadius(Radius / 1000); // in km
	}

}
#endif
