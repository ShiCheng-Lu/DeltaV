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

	// ConstructorHelpers::FObjectFinder<UMaterial> MeshFinder(TEXT("/Game/Simulation/UI/navball_Mat"));
	UMaterial* Material = UAssetLibrary::LoadAsset<UMaterial>(L"/Game/Simulation/UI/navball_Mat");
	if (Material == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("texture not found"));
	}
	Mesh->SetMaterial(0, Material);
	
	SetRootComponent(Mesh);

	RenderTarget = UAssetLibrary::LoadAsset<UTextureRenderTarget2D>(L"/Game/Simulation/UI/Navballrender");
	// RenderTarget->InitAutoFormat(512, 512);
	// RenderTarget->UpdateResourceImmediate();

	double CameraDistance = 1000;

	Camera = CreateDefaultSubobject<USceneCaptureComponent2D>("Camera");
	Camera->SetRelativeLocation(FVector(CameraDistance, 0, 0));
	Camera->SetRelativeRotation(FQuat(FVector(0, 0, 1), PI));
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

// Called every frame
void ANavball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector dir = Target->GetActorLocation() - Origin;
	FVector side = dir.Cross(FVector(0, 0, 1));
	FVector up = dir.Cross(side);

	dir.Normalize();
	side.Normalize();
	up.Normalize();

	FMatrix rotation;
	rotation.SetColumn(0, dir);
	rotation.SetColumn(1, side);
	rotation.SetColumn(2, up);

	// FVector Start = Camera->GetComponentLocation();
	// FVector End = Start + Camera->GetComponentRotation().RotateVector(FVector(100, 0, 0));
	// DrawDebugDirectionalArrow(GetWorld(), Start, End, 10, FColor(130, 0, 0), false, -1, 0, 3);
	FQuat target_rotation = Target->GetActorQuat().Inverse();

	Mesh->SetRelativeRotation(FQuat(rotation) * target_rotation);
}

void ANavball::SetTarget(ACraft* Craft, FVector PlanetCenter) {
	Target = Craft;
	Origin = PlanetCenter;
}
