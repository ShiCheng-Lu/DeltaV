// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/UI/Navball.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Components/BillboardComponent.h"

#include "Common/Craft.h"
#include "Common/AssetLibrary.h"

UStaticMeshComponent* SetupTargetTexture(ANavball* Navball, FString Target, FVector Colour) {
	UStaticMesh* Plane = UAssetLibrary::LoadAsset<UStaticMesh>(TEXT("/Game/Shapes/plane"));

	UMaterial* PlaneMaterial = UAssetLibrary::LoadAsset<UMaterial>(TEXT("/Game/Simulation/UI/PlaneMaterial"));
	UStaticMeshComponent* Mesh = Navball->CreateDefaultSubobject<UStaticMeshComponent>(FName(Target));

	UTexture2D* Texture = UAssetLibrary::LoadAsset<UTexture2D>(TEXT("/Game/Simulation/UI/Stabilization/") + Target);
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(PlaneMaterial, Navball);
	Material->SetTextureParameterValue("Texture", Texture);
	Material->SetVectorParameterValue("Colour", Colour);

	Mesh->SetRelativeRotation(FQuat(FVector::ZAxisVector, PI));
	Mesh->SetRelativeLocation(FVector(-110, 0, 0));

	Mesh->SetStaticMesh(Plane);
	Mesh->SetMaterial(0, Material);
	Mesh->SetVisibleInSceneCaptureOnly(true);
	Mesh->SetCastShadow(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Navball->Camera->ShowOnlyComponent(Mesh);

	return Mesh;
}

// Sets default values
ANavball::ANavball()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");

	UStaticMesh* StaticMesh = UAssetLibrary::LoadAsset<UStaticMesh>(TEXT("/Game/Shapes/Navball2"));
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
	Camera = CreateDefaultSubobject<USceneCaptureComponent2D>("Camera");
	FVector CameraPosition = FVector(-120, 0, 0);
	Camera->SetRelativeLocation(CameraPosition);

	Camera->SetRelativeRotation(FQuat(FVector::XAxisVector, PI));
	// Camera->SetRelativeRotation(FMatrix(FVector(0, 0, 1), FVector(0, -1, 0), FVector(1, 0, 0), FVector(0)).ToQuat());
	Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	//Camera->ShowOnlyActorComponents(this);
	Camera->ShowOnlyComponent(Mesh);
	Camera->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	Camera->bCaptureEveryFrame = true;
	Camera->TextureTarget = RenderTarget;
	Camera->ProjectionType = ECameraProjectionMode::Orthographic;
	Camera->OrthoWidth = 200;


	ProgradeTexture		= SetupTargetTexture(this, "Prograde", FVector(0, 0, 0));
	RetrogradeTexture	= SetupTargetTexture(this, "Retrograde", FVector(0, 0, 0));
	NormalTexture		= SetupTargetTexture(this, "Normal", FVector(0, 0, 0));
	AntiNormalTexture	= SetupTargetTexture(this, "AntiNormal", FVector(0, 0, 0));
	RadialInTexture		= SetupTargetTexture(this, "RadialIn", FVector(0, 0, 0));
	RadialOutTexture	= SetupTargetTexture(this, "RadialOut", FVector(0, 0, 0));
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

void UpdateTexture(UStaticMeshComponent* Mesh, FVector Orientation) {
	Mesh->SetVisibility(Orientation.X < 0);
	Mesh->SetWorldLocation(FVector(-100, Orientation.Y * 100, Orientation.Z * 100));
}

// Called every frame
void ANavball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Navball direction is based on what rotation is needed to align the craft from current orientation to a due north with up vector away from the center of the central body.

	FVector newZ = (Target->GetActorLocation() - Origin).GetSafeNormal();
	FVector newY = newZ.Cross(FVector::UpVector);
	FVector newX = newY.Cross(newZ);

	FMatrix rotation = FMatrix(newX, newY, newZ, FVector::ZeroVector);
	
	// DebugQuat(GetWorld(), Target->GetActorLocation(), rotation.ToQuat(), 300);
	// DebugQuat(GetWorld(), Target->GetActorLocation(), Target->GetActorQuat(), 500);
	
	FQuat rot = (FQuat(rotation).Inverse() * Target->GetActorQuat()).Inverse();
	Mesh->SetRelativeRotation(rot * FQuat(FVector(0, 1, 0), PI));

	FVector Prograde = Target->GetVelocity().GetSafeNormal();
	FVector Retrograde = -Prograde;
	FVector Normal = Prograde.Cross(Target->GetActorLocation() - Origin).GetSafeNormal();
	FVector AntiNormal = -Normal;
	FVector RadialIn = Prograde.Cross(Normal);
	FVector RadialOut = -RadialIn;

	FQuat TargetInverse = Target->GetActorQuat().Inverse() * FQuat(FVector::ZAxisVector, PI);
	UpdateTexture(ProgradeTexture, TargetInverse.RotateVector(Prograde));
	UpdateTexture(RetrogradeTexture, TargetInverse.RotateVector(Retrograde));
	UpdateTexture(NormalTexture, TargetInverse.RotateVector(Normal));
	UpdateTexture(AntiNormalTexture, TargetInverse.RotateVector(AntiNormal));
	UpdateTexture(RadialInTexture, TargetInverse.RotateVector(RadialIn));
	UpdateTexture(RadialOutTexture, TargetInverse.RotateVector(RadialOut));

	// Stabilization

	FVector TargetOrientation;
	switch (StabilizationMode)
	{
	case EStabilizationMode::NONE:
		return;
	case EStabilizationMode::HOLD_ATTITUDE:
		break;
	case EStabilizationMode::MANEUVER:
		break;
	case EStabilizationMode::PROGRADE:
		TargetOrientation = Prograde;
		break;
	case EStabilizationMode::RETROGRADE:
		TargetOrientation = Retrograde;
		break;
	case EStabilizationMode::RADIAL_IN:
		TargetOrientation = RadialIn;
		break;
	case EStabilizationMode::RADIAL_OUT:
		TargetOrientation = RadialOut;
		break;
	case EStabilizationMode::NORMAL:
		TargetOrientation = Normal;
		break;
	case EStabilizationMode::ANTI_NORMAL:
		TargetOrientation = AntiNormal;
		break;
	case EStabilizationMode::TARGET:
		break;
	case EStabilizationMode::ANTI_TARGET:
		break;
	default:
		break;
	}

	FQuat CurrentRotation = Target->GetActorQuat();
	FVector TargetAngularVel = CurrentRotation.Inverse().RotateVector(TargetOrientation);
	FVector CurrentAngularVel = CurrentRotation.Inverse().RotateVector(Target->GetAngularVelocity());

	// stablizing via PD controller, with coefficients of 1
	// proportional component
	FRotator Rotation = TargetAngularVel.ToOrientationRotator();
	// derivative component
	Rotation.Pitch += FMath::RadiansToDegrees(CurrentAngularVel.Y);
	Rotation.Yaw -= FMath::RadiansToDegrees(CurrentAngularVel.Z);
	Rotation.Roll += FMath::RadiansToDegrees(CurrentAngularVel.X);

	if (FMath::Abs(Rotation.Pitch) <= 0.1) {
		Rotation.Pitch = 0;
	}
	if (FMath::Abs(Rotation.Yaw) <= 0.1) {
		Rotation.Yaw = 0;
	}
	if (FMath::Abs(Rotation.Roll) <= 0.1) {
		Rotation.Roll = 0;
	}

	Target->Rotate(Rotation, 100000000);

}

void ANavball::SetTarget(ACraft* Craft, FVector PlanetCenter) {
	Target = Craft;
	Origin = PlanetCenter;
}
