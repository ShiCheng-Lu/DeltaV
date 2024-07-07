// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationController.h"

#include "GameFramework/PlayerInput.h"
#include "GameFramework/HUD.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Kismet/GameplayStatics.h"

#include "Common/MainGameInstance.h"
#include "Common/Craft.h"
#include "Common/JsonUtil.h"
#include "Simulation/SimulationCamera.h"
#include "Simulation/UI/SimulationHUD.h"
#include "Simulation/MapViewPawn.h"
#include "Simulation/OrbitComponent.h"
#include "Simulation/CelestialBody.h"


ASimulationController::ASimulationController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ASimulationCamera::StaticClass();
}

void ASimulationController::BeginPlay() {
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());

	// get references to the celestial bodies placed in the world
	TArray<AActor*> celestial_bodies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACelestialBody::StaticClass(), celestial_bodies);

	if (celestial_bodies.Num() > 0) {
		earth = (ACelestialBody*)celestial_bodies[0];
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No celestial body"));
		return;
	}

	FString Path = Cast<UMainGameInstance>(GetGameInstance())->CraftPath;


	UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *Path);

	if (Path.IsEmpty()) {
		Path = FPaths::ProjectSavedDir() + "ship2.json";
		UE_LOG(LogTemp, Warning, TEXT("No craft, using default"));
	}

	FActorSpawnParameters spawn_parameters;
	craft = GetWorld()->SpawnActor<ACraft>();
	craft->Initialize(JsonUtil::ReadFile(Path));

	FVector origin, extent;
	craft->GetActorBounds(true, origin, extent);
	UE_LOG(LogTemp, Warning, TEXT("craft bounds %s %s"), *origin.ToString(), *extent.ToString());

	craft->SetActorLocation(FVector(-(earth->radius * 100 + extent.Z), 0, 0));
	craft->SetActorRotation(FRotator(90, 0, 0));
	craft->SetPhysicsEnabled(true);

	Possess(craft);
	SetControlRotation(FRotator(0));

	PlayerCameraManager->SetViewTarget(craft);
	PlayerCameraManager->FreeCamDistance = 1000;

	HUD = CreateWidget<USimulationHUD>(this, USimulationHUD::BlueprintClass);
	HUD->AddToPlayerScreen();

	HUD->SetNavballTarget(craft, FVector(0, 0, 0));

	UE_LOG(LogTemp, Warning, TEXT("craft location %s %f"), *craft->GetActorLocation().ToString(), earth->radius);
}

void ASimulationController::SetupInputComponent() {
	Super::SetupInputComponent();

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Pitch", EKeys::W, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Pitch", EKeys::S, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::Q, -1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::E, 1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Yaw", EKeys::A, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Yaw", EKeys::D, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("LookX", EKeys::MouseX, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("LookY", EKeys::MouseY, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::LeftShift, 0.01f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::LeftControl, -0.01f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::X, -1));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::Z, 1));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("CameraZoom", EKeys::MouseWheelAxis, 0.05f));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("LeftClick", EKeys::LeftMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("RightClick", EKeys::RightMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("MiddleClick", EKeys::MiddleMouseButton));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("Stage", EKeys::SpaceBar));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("ToggleMap", EKeys::M));

	InputComponent->BindAxis("LookX", this, &ASimulationController::AddYawInput);
	InputComponent->BindAxis("LookY", this, &ASimulationController::AddPitchInput);

	InputComponent->BindAxis("Throttle", this, &ASimulationController::Throttle);

	InputComponent->BindAxis("CameraZoom", this, &ASimulationController::Zoom);

	InputComponent->BindAxis("Pitch", this, &ASimulationController::Pitch);
	InputComponent->BindAxis("Roll", this, &ASimulationController::Roll);
	InputComponent->BindAxis("Yaw", this, &ASimulationController::Yaw);

	InputComponent->BindAction("Stage", EInputEvent::IE_Pressed, this, &ASimulationController::Stage);
	InputComponent->BindAction("ToggleMap", EInputEvent::IE_Pressed, this, &ASimulationController::ToggleMap);
}


void ASimulationController::Throttle(float value) {
	if (value != 0 && craft) {
		ThrottleValue = FMath::Clamp(ThrottleValue + value, 0, 1);
		HUD->Throttle->SetPercent(ThrottleValue);
	}
}

void ASimulationController::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	craft->Throttle(ThrottleValue);
}

void ASimulationController::Zoom(float value) {
	if (value != 0 && GetPawn() == craft) {
		PlayerCameraManager->FreeCamDistance *= (1 - value);
	}
}

void ASimulationController::Pitch(float value) {
	if (value != 0 && GetPawn() == craft) {
		craft->Rotate(FRotator(90, 0, 0), 100000000 * value);
	}
}
void ASimulationController::Roll(float value) {
	if (value != 0 && GetPawn() == craft) {
		craft->Rotate(FRotator(0, 0, 90), 100000000 * value);
	}
}
void ASimulationController::Yaw(float value) {
	if (value != 0 && GetPawn() == craft) {
		craft->Rotate(FRotator(0, 90, 0), 100000000 * value);
	}
}
void ASimulationController::Stage() {
	auto NewCraft = GetWorld()->SpawnActor<ACraft>();
	// craft->DetachPart(, NewCraft);
}



void ASimulationController::VelChange(float value) {
	vel = value;

	UE_LOG(LogTemp, Warning, TEXT("vel changed %f"), value);
}
void ASimulationController::GravChange(float value) {
	grav = value;
	UE_LOG(LogTemp, Warning, TEXT("grav changed %f"), value);
}

void ASimulationController::ToggleMap() {
	/*
	if (PlayerCameraManager->FreeCamDistance == 1000) {
		PlayerCameraManager->FreeCamDistance = 100000000;
	}
	else {
		PlayerCameraManager->FreeCamDistance = 1000;
		return;
	}
	*/
	// Debug orbit;

	UE_LOG(LogTemp, Warning, TEXT("values %f %f"), HUD->Gravity->Value, HUD->Velocity->Value);
	UOrbitComponent* orbit = NewObject<UOrbitComponent>(this);

	ACelestialBody* body = NewObject<ACelestialBody>(this);
	body->mu = HUD->Gravity->Value;

	orbit->UpdateOrbit(FVector(100, 0, 0), FVector(20, HUD->Velocity->Value, 40), body);
	
	TArray<FVector> array;
	for (int i = 0; i < 360; i++) {
		array.Add(orbit->GetPosition(i));
	}

	for (int i = 0; i < 359; i++) {
		DrawDebugLine(GetWorld(), array[i], array[i + 1], FColor(200, 0, 200), false, 5, 0, 2);
		// UE_LOG(LogTemp, Warning, TEXT("p %d: %s"), i, *array[i].ToString());
	}
	// GetHUD()->Draw3DLine(array[0], array[35], FColor(230, 230, 0));
	DrawDebugSphere(GetWorld(), FVector(0, 0, 0), 10, 40, FColor(0, 250, 0), false, 5, 0, 0);
	DrawDebugSphere(GetWorld(), FVector(100, 0, 0), 1, 40, FColor(0, 250, 0), false, 5, 0, 0);
}
