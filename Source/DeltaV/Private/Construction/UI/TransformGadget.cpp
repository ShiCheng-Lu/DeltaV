// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/TransformGadget.h"
#include "Construction/Constructor.h"
#include "Construction/ConstructionController.h"

#include "Common/Part.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "Kismet/GameplayStatics.h"

void ATransformGadget::SetupStaticMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh, UMaterialInterface* Material, FVector Rotation) {
	Component->SetStaticMesh(Mesh);
	Component->SetMaterial(0, Material);
	Component->SetDepthPriorityGroup(ESceneDepthPriorityGroup::SDPG_Foreground);

	Component->SetRelativeRotation(Rotation.ToOrientationRotator());

	Component->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	Component->SetCollisionResponseToAllChannels(ECR_Ignore);
	Component->SetCollisionResponseToChannel(ECC_TransformGadget, ECR_Block);

	Component->SetupAttachment(GetRootComponent());
}

// Sets default values
ATransformGadget::ATransformGadget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<UStaticMesh> TranslateMesh(TEXT("StaticMesh'/Game/Shapes/arrow'"));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/Materials/xyz'"));

	USphereComponent* Center = CreateDefaultSubobject<USphereComponent>("Center");
	Center->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(Center);

	TranslateX = CreateDefaultSubobject<UStaticMeshComponent>("TranslateX");
	TranslateY = CreateDefaultSubobject<UStaticMeshComponent>("TranslateY");
	TranslateZ = CreateDefaultSubobject<UStaticMeshComponent>("TranslateZ");

	if (TranslateMesh.Succeeded() && Material.Succeeded()) {
		SetupStaticMesh(TranslateX, TranslateMesh.Object, Material.Object, FVector(1, 0, 0));
		SetupStaticMesh(TranslateY, TranslateMesh.Object, Material.Object, FVector(0, 1, 0));
		SetupStaticMesh(TranslateZ, TranslateMesh.Object, Material.Object, FVector(0, 0, 1));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No Mesh / Material"))
	}
}

double RayIntersection1(const FRay& A, const FRay& B) {
	FVector C = B.Origin - A.Origin;

	double AdotB = A.Direction.Dot(B.Direction);
	double BdotB = B.Direction.Dot(B.Direction);

	double denom = A.Direction.Dot(A.Direction) * BdotB - AdotB * AdotB;

	if (FMath::IsNearlyZero(denom)) {
		return 0;
	}
	
	double t = (BdotB * A.Direction.Dot(C) - AdotB * B.Direction.Dot(C)) / denom;

	return t;
}

// Called when the game starts or when spawned
void ATransformGadget::BeginPlay()
{
	Super::BeginPlay();

	TranslateX->RegisterComponent();
	TranslateY->RegisterComponent();
	TranslateZ->RegisterComponent();

	GetRootComponent()->SetHiddenInGame(true, true);
}

// Called every frame
void ATransformGadget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Active) {
		return;
	}
	// do things

	FRay MouseRay = Controller->GetMouseRay();

	FVector Distance = TranslateRay.Direction * RayIntersection1(TranslateRay, MouseRay);
	FVector Location = Distance + TranslateRay.Origin - Offset;
	SetActorLocation(Location);
	if (Selected) {
		Selected->SetWorldLocation(Location);
	}
}

void ATransformGadget::StartTracking() {
	UE_LOG(LogTemp, Warning, TEXT("Started Tracking"));
	// plane such that it's perpendicular to the mouse projection and parallel to the clicked axis
	FRay MouseRay = Controller->GetMouseRay();
	
	FHitResult Result;
	if (Selected != nullptr && GetWorld()->LineTraceSingleByChannel(Result, MouseRay.Origin, MouseRay.PointAt(10000), ECC_TransformGadget)) {
		// clicked on a arrow
		TranslateRay.Origin = GetActorLocation();
		TranslateRay.Direction = Result.GetComponent()->GetComponentRotation().Vector();

		Offset = TranslateRay.Direction * RayIntersection1(TranslateRay, MouseRay);

		Active = true;
		return;
	}

	if (GetWorld()->LineTraceSingleByChannel(Result, MouseRay.Origin, MouseRay.PointAt(10000), ECC_NoneHeldParts)) {
		// 
		UE_LOG(LogTemp, Warning, TEXT("Shown"));
		Selected = Cast<UPart>(Result.GetComponent());
		if (Selected) {
			SetActorLocation(Selected->GetComponentLocation());
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Hidden"));
		Selected = nullptr;
	}
	GetRootComponent()->SetHiddenInGame(!Selected, true);
}

void ATransformGadget::StopTracking() {
	Active = false;
}
