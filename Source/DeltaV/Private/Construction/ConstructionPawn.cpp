// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionPawn.h"

#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerInput.h"
#include "Construction/ConstructionController.h"

// Sets default values
AConstructionPawn::AConstructionPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetActorEnableCollision(false);
	SetCanBeDamaged(true);

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	NetPriority = 3.0f;

	BaseEyeHeight = 0.0f;
	bCollideWhenPlacing = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("AConstructionPawn::CollisionComponentName");
	CollisionComponent->InitSphereRadius(35.0f);
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	CollisionComponent->SetShouldUpdatePhysicsVolume(true);
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->bDynamicObstacle = true;

	RootComponent = CollisionComponent;

	MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>("AConstructionPawn::MovementComponentName");
	MovementComponent->UpdatedComponent = CollisionComponent;
}

// Called when the game starts or when spawned
void AConstructionPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AConstructionPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AConstructionPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForwardBackward", this, &AConstructionPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveLeftRight", this, &AConstructionPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUpDown", this, &AConstructionPawn::MoveUp);
	PlayerInputComponent->BindAxis("ZoomIn", this, &AConstructionPawn::ZoomIn);

	// PlayerInputComponent->BindAxis("TurnRate", this, &AConstructionPawn::TurnAtRate);
	// PlayerInputComponent->BindAxis("LookUpRate", this, &AConstructionPawn::LookUpAtRate);

	UE_LOG(LogTemp, Warning, TEXT("PlayerInput Setup"));

}

void AConstructionPawn::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		if (Controller)
		{
			FRotator const ControlSpaceRot = Controller->GetControlRotation();

			// transform to world space and add it
			AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Y), Val);
		}
	}
}

void AConstructionPawn::MoveForward(float Val)
{
	if (Val != 0.f)
	{
		if (Controller)
		{
			FRotator const ControlSpaceRot = Controller->GetControlRotation();

			FVector Direction = FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X);
			Direction.Z = 0;
			AddMovementInput(Direction, Val);
		}
	}
}

void AConstructionPawn::MoveUp(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(FVector::UpVector, Val);
	}
}

void AConstructionPawn::ZoomIn(float Val) {
	if (Val != 0.f) {
		if (Controller)
		{
			UE_LOG(LogTemp, Warning, TEXT("Zoom in %f"), Val);
			FRotator const ControlSpaceRot = Controller->GetControlRotation();

			FVector NewLocation = GetActorLocation();
			NewLocation += FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X) * Val;
			SetActorLocation(NewLocation);
			((AConstructionController*)Controller)->PlaceDistance -= Val;
		}
	}
}

void AConstructionPawn::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AConstructionPawn::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AConstructionPawn::HandleClick(FKey Key) {

	if (Key == EKeys::LeftMouseButton) {
		UE_LOG(LogTemp, Warning, TEXT("AA Left clicked"));
	}
	else if (Key == EKeys::RightMouseButton) {
		UE_LOG(LogTemp, Warning, TEXT("AA Right clicked"));
	}
	// AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController());
	// PlayerController->HandleClick(key);
}

UPawnMovementComponent* AConstructionPawn::GetMovementComponent() const
{
	return MovementComponent;
}
