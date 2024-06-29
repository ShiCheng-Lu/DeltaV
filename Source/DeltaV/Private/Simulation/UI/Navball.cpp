// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/UI/Navball.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"

#include "Common/Craft.h"
#include "Common/AssetLibrary.h"

// Sets default values
ANavball::ANavball()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");

	UStaticMesh* StaticMesh = UAssetLibrary::LoadAsset<UStaticMesh>(L"/Game/Shapes/Shape_Sphere.Shape_Sphere");
	if (StaticMesh != nullptr) {
		Mesh->SetStaticMesh(StaticMesh);
	}
	Mesh->SetVisibleInSceneCaptureOnly(true);
	Mesh->SetCastShadow(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(Mesh);

	RenderTarget = UAssetLibrary::LoadAsset<UTextureRenderTarget2D>(L"/Game/Simulation/UI/Navballrender");
	// RenderTarget->InitAutoFormat(512, 512);
	// RenderTarget->UpdateResourceImmediate();

	double CameraDistance = 1000;

	Camera = CreateDefaultSubobject<USceneCaptureComponent2D>("Camera");
	Camera->SetRelativeLocation(FVector(-CameraDistance, 0, 0));
	Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	Camera->ShowOnlyActorComponents(this);
	Camera->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	Camera->bCaptureEveryFrame = true;
	Camera->TextureTarget = RenderTarget;

	UE_LOG(LogTemp, Warning, TEXT("Mesh size %f %f"), StaticMesh->GetBounds().GetSphere().W, FMath::Atan2(StaticMesh->GetBounds().GetSphere().W, CameraDistance));

	Camera->FOVAngle = FMath::RadiansToDegrees(FMath::Atan2(StaticMesh->GetBounds().GetSphere().W, CameraDistance)) * 2;
	UE_LOG(LogTemp, Warning, TEXT("Mesh size %f"), Camera->FOVAngle);
}

// Called when the game starts or when spawned
void ANavball::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANavball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// FVector Start = Camera->GetComponentLocation();
	// FVector End = Start + Camera->GetComponentRotation().RotateVector(FVector(100, 0, 0));
	// DrawDebugDirectionalArrow(GetWorld(), Start, End, 10, FColor(130, 0, 0), false, -1, 0, 3);
	FRotator direction_from_origin = (Target->GetActorLocation() - Origin).Rotation();
	FRotator target_rotation = Target->GetActorRotation();
	Mesh->SetRelativeRotation(direction_from_origin - target_rotation);
}

void ANavball::SetTarget(ACraft* Craft, FVector PlanetCenter) {
	Target = Craft;
	Origin = PlanetCenter;
}
