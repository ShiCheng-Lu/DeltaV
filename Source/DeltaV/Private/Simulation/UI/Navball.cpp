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

	UStaticMesh* StaticMesh = UAssetLibrary::LoadAsset<UStaticMesh>(TEXT("StaticMesh'/Game/Shapes/Navball2'"));
	if (StaticMesh == nullptr) {
		return;
	}
	Mesh->SetStaticMesh(StaticMesh);
	Mesh->SetVisibleInSceneCaptureOnly(true);
	Mesh->SetCastShadow(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ConstructorHelpers::FObjectFinder<UMaterial> MeshFinder(TEXT("/Game/Simulation/UI/navball_Mat"));
	// UMaterial* Material = UAssetLibrary::LoadAsset<UMaterial>(L"/Game/Simulation/UI/navball_Mat");
	// if (Material == nullptr) {
	// 	UE_LOG(LogTemp, Warning, TEXT("texture not found"));
	// }
	// Mesh->SetMaterial(0, Material);
	
	SetRootComponent(Mesh);

	RenderTarget = UAssetLibrary::LoadAsset<UTextureRenderTarget2D>(TEXT("/Game/Simulation/UI/Navballrender"));
	// RenderTarget->InitAutoFormat(512, 512);
	// RenderTarget->UpdateResourceImmediate();

	double CameraDistance = 1000;

	Camera = CreateDefaultSubobject<USceneCaptureComponent2D>("Camera");
	FVector CameraPosition = FVector(-CameraDistance, 0, 0);
	Camera->SetRelativeLocation(CameraPosition);
	// Camera->SetRelativeRotation((- CameraPosition).ToOrientationQuat());
	Camera->SetRelativeRotation(FQuat(FVector(1, 0, 0), PI));
	// Camera->SetRelativeRotation(FMatrix(FVector(0, 0, 1), FVector(0, -1, 0), FVector(1, 0, 0), FVector(0)).ToQuat());
	Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	Camera->ShowOnlyActorComponents(this);
	Camera->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	Camera->bCaptureEveryFrame = true;
	Camera->TextureTarget = RenderTarget;

	UE_LOG(LogTemp, Warning, TEXT("Mesh size %f %f"), StaticMesh->GetBounds().GetSphere().W, FMath::Atan2(StaticMesh->GetBounds().GetSphere().W, CameraDistance));

	double SphereRadius = StaticMesh->GetBounds().GetSphere().W;
	Camera->FOVAngle = FMath::RadiansToDegrees(FMath::Atan2(SphereRadius, CameraDistance)) * 2;
	UE_LOG(LogTemp, Warning, TEXT("Mesh size %f"), Camera->FOVAngle);
}

// Called when the game starts or when spawned
void ANavball::BeginPlay()
{
	Super::BeginPlay();
}

void DebugQuat(UWorld* world, FVector location, FQuat quat, int length) {
	FVector X = quat.RotateVector(FVector(1, 0, 0));
	FVector Y = quat.RotateVector(FVector(0, 1, 0));
	FVector Z = quat.RotateVector(FVector(0, 0, 1));
	DrawDebugDirectionalArrow(world, location, location + X * length, 10, FColor(200, 0, 0), false, -1, 0, 5);
	DrawDebugDirectionalArrow(world, location, location + Y * length, 10, FColor(0, 200, 0), false, -1, 0, 5);
	DrawDebugDirectionalArrow(world, location, location + Z * length, 10, FColor(0, 0, 200), false, -1, 0, 5);
}

// Called every frame
void ANavball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Navball direction is based on what rotation is needed to align the craft from current orientation to a due north with up vector away from the center of the central body.

	FVector newZ = Target->GetActorLocation() - Origin;
	FVector newY = newZ.Cross(FVector(0, 0, 1));
	FVector newX = newY.Cross(newZ);

	newX.Normalize();
	newY.Normalize();
	newZ.Normalize();

	FMatrix rotation = FMatrix(newX, newY, newZ, FVector(0));
	// DebugQuat(GetWorld(), Target->GetActorLocation(), FQuat(0), 300);
	DebugQuat(GetWorld(), Target->GetActorLocation(), rotation.ToQuat(), 300);
	DebugQuat(GetWorld(), Target->GetActorLocation(), Target->GetActorQuat(), 500);
	
	FQuat rot = (FQuat(rotation).Inverse() * Target->GetActorQuat()).Inverse(); // rotation
	Mesh->SetRelativeRotation(rot);
	UE_LOG(LogTemp, Warning, TEXT("ROT: %s"), *rot.Rotator().ToString());
}

void ANavball::SetTarget(ACraft* Craft, FVector PlanetCenter) {
	Target = Craft;
	Origin = PlanetCenter;
}
