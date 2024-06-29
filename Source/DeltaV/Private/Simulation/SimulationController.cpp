// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationController.h"

#include "GameFramework/PlayerInput.h"

#include "Common/MainGameInstance.h"
#include "Common/Craft.h"
#include "Common/JsonUtil.h"
#include "Simulation/SimulationCamera.h"
#include "Simulation/UI/SimulationHUD.h"

ASimulationController::ASimulationController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ASimulationCamera::StaticClass();
}

void ASimulationController::BeginPlay() {
	FString Path = Cast<UMainGameInstance>(GetGameInstance())->CraftPath;

	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());

	UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *Path);

	if (Path.IsEmpty()) {
		Path = FPaths::ProjectSavedDir() + "ship2.json";
		UE_LOG(LogTemp, Warning, TEXT("No craft, using default"));
	}

	craft = GetWorld()->SpawnActor<ACraft>();
	craft->Initialize(JsonUtil::ReadFile(Path));

	Possess(craft);

	PlayerCameraManager->SetViewTarget(craft);
	PlayerCameraManager->FreeCamDistance = 1000;

	HUD = CreateWidget<USimulationHUD>(this, USimulationHUD::BlueprintClass);
	HUD->AddToPlayerScreen();

	HUD->SetNavballTarget(craft, FVector(0, 0, 0));
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

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::LeftShift, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::LeftControl, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("CameraZoom", EKeys::MouseWheelAxis, 0.05f));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("LeftClick", EKeys::LeftMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("RightClick", EKeys::RightMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("MiddleClick", EKeys::MiddleMouseButton));

	InputComponent->BindAxis("LookX", this, &ASimulationController::AddYawInput);
	InputComponent->BindAxis("LookY", this, &ASimulationController::AddPitchInput);

	InputComponent->BindAxis("Throttle", this, &ASimulationController::Throttle);
	InputComponent->BindAxis("CameraZoom", this, &ASimulationController::Zoom);

	InputComponent->BindAxis("Pitch", this, &ASimulationController::Pitch);
	InputComponent->BindAxis("Roll", this, &ASimulationController::Roll);
	InputComponent->BindAxis("Yaw", this, &ASimulationController::Yaw);
}


void ASimulationController::Throttle(float value) {
	if (value != 0 && craft) {
		craft->Throttle(value);
	}
}

void ASimulationController::Zoom(float value) {
	if (value != 0) {
		PlayerCameraManager->FreeCamDistance *= (1 - value);
	}
}

void ASimulationController::Pitch(float value) {
	if (value != 0) {
		craft->Rotate(FRotator(90, 0, 0), 100000000 * value);
	}
}
void ASimulationController::Roll(float value) {
	if (value != 0) {
		craft->Rotate(FRotator(0, 0, 90), 100000000 * value);
	}
}
void ASimulationController::Yaw(float value) {
	if (value != 0) {
		craft->Rotate(FRotator(0, 90, 0), 100000000 * value);
	}
}