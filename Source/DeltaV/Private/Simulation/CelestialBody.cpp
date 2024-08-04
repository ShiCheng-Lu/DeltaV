 // Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/CelestialBody.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"

// Sets default values
ACelestialBody::ACelestialBody(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("StaticMesh'/Game/Shapes/Shape_Sphere.Shape_Sphere'"));
	if (MeshFinder.Succeeded()) {
		mesh->SetStaticMesh(MeshFinder.Object);
	}
	mesh->SetRelativeScale3D(FVector(radius));

	atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>("Atmosphere");
	atmosphere->TransformMode = ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform;
	atmosphere->SetBottomRadius(radius / 1000); // in km
	atmosphere->SetAtmosphereHeight(atmosphere_height / 1000); // in km

	SetRootComponent(mesh);

	atmosphere->SetupAttachment(mesh);
}

// Called when the game starts or when spawned
void ACelestialBody::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACelestialBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


#if WITH_EDITOR
void ACelestialBody::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == "radius") {
		radius = FMath::Max(0, radius);
		mesh->SetRelativeScale3D(FVector(radius));
		atmosphere->SetBottomRadius(radius / 1000); // in km
	}
	if (PropertyChangedEvent.GetPropertyName() == "atmosphere_height") {
		atmosphere_height = FMath::Max(100, atmosphere_height); // atmosphere height 
		atmosphere->SetAtmosphereHeight(atmosphere_height / 1000);
	}
}
#endif
