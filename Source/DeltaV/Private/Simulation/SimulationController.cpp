// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationController.h"

#include "GameFramework/PlayerInput.h"
#include "GameFramework/HUD.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Common/MainGameInstance.h"
#include "Common/Craft.h"
#include "Common/JsonUtil.h"
#include "Simulation/UI/SimulationHUD.h"
#include "Simulation/MapViewPawn.h"
#include "Simulation/OrbitComponent.h"
#include "Simulation/CelestialBody.h"
#include "Simulation/ControlStabilizer.h"


ASimulationController::ASimulationController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ASimulationController::BeginPlay() {
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));

	// get references to the celestial bodies placed in the world
	TArray<AActor*> celestial_bodies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACelestialBody::StaticClass(), celestial_bodies);

	if (celestial_bodies.Num() > 0) {
		Earth = (ACelestialBody*)celestial_bodies[0];
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

	Craft = GetWorld()->SpawnActor<ACraft>();
	Craft->SetActorRotation(FRotator(90, 0, 0));
	Craft->FromJson(JsonUtil::ReadFile(Path));

	FVector origin, extent;
	Craft->GetActorBounds(true, origin, extent);
	Craft->SetActorLocation(FVector(-(Earth->GetActorScale3D().Z * 100 + extent.Z * 2), 0, 0));
	Craft->SetActorRotation(FRotator(180, 0, 0));
	Craft->SetPhysicsEnabled(true);

	Possess(Craft);
	SetControlRotation(FRotator(0));
	Craft->OrbitComponent->CentralBody = Earth;

	PlayerCameraManager->CameraStyle = FName(TEXT("FreeCam"));

	HUD = CreateWidget<USimulationHUD>(this, USimulationHUD::BlueprintClass);
	HUD->AddToPlayerScreen();

	HUD->SetNavballTarget(Craft, FVector(0, 0, 0));

	UE_LOG(LogTemp, Warning, TEXT("craft location %s %f"), *Craft->GetActorLocation().ToString(), Earth->GetActorScale3D().Z);

	
	ControlStabilizer = NewObject<UControlStabilizer>(this); // CreateDefaultSubobject<UControlStabilizer>("Stabilizer");
	ControlStabilizer->Controller = this;

	ControlStabilizer->RegisterComponent();

}


void ASimulationController::UpdateRotation(float DeltaTime)
{
	// Calculate Delta to be applied on ViewRotation
	FRotator DeltaRot(RotationInput);

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
	}

	FRotator Rotation = GetPawn()->GetActorLocation().ToOrientationRotator();
	Rotation = UKismetMathLibrary::ComposeRotators(FRotator(90, -180, 0), Rotation);
	Rotation = UKismetMathLibrary::ComposeRotators(ViewRotation, Rotation);

	SetControlRotation(Rotation);

	APawn* const P = GetPawnOrSpectator();
	if (P)
	{
		P->FaceRotation(Rotation, DeltaTime);
	}
}

void ASimulationController::SetupInputComponent() {
	Super::SetupInputComponent();

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Pitch", EKeys::W, -1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Pitch", EKeys::S, 1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::Q, -1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::E, 1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Yaw", EKeys::A, -1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Yaw", EKeys::D, 1.f));

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

	PlayerInput->AddActionMapping(FInputActionKeyMapping("TimeWarpAdd", EKeys::Period));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("TimeWarpSub", EKeys::Comma));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("TimeWarpReset", EKeys::Slash));


	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action", EKeys::One));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action", EKeys::Two));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action", EKeys::Three));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action", EKeys::Four));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action", EKeys::Five));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action", EKeys::Six));

	InputComponent->BindAxis("LookX", this, &ASimulationController::AddYawInput).bExecuteWhenPaused = true;
	InputComponent->BindAxis("LookY", this, &ASimulationController::AddPitchInput).bExecuteWhenPaused = true;

	InputComponent->BindAxis("Throttle", this, &ASimulationController::Throttle);

	InputComponent->BindAxis("CameraZoom", this, &ASimulationController::Zoom).bExecuteWhenPaused = true;

	InputComponent->BindAxis("Pitch", this, &ASimulationController::Pitch);
	InputComponent->BindAxis("Roll", this, &ASimulationController::Roll);
	InputComponent->BindAxis("Yaw", this, &ASimulationController::Yaw);

	InputComponent->BindAction("Stage", EInputEvent::IE_Pressed, this, &ASimulationController::Stage);
	InputComponent->BindAction("ToggleMap", EInputEvent::IE_Pressed, this, &ASimulationController::ToggleMap);

	InputComponent->BindAction("TimeWarpAdd", EInputEvent::IE_Pressed, this, &ASimulationController::TimeWarpAdd).bExecuteWhenPaused = true;
	InputComponent->BindAction("TimeWarpSub", EInputEvent::IE_Pressed, this, &ASimulationController::TimeWarpSub).bExecuteWhenPaused = true;
	InputComponent->BindAction("TimeWarpReset", EInputEvent::IE_Pressed, this, &ASimulationController::TimeWarpReset).bExecuteWhenPaused = true;

	InputComponent->BindAction("Action", IE_Pressed, this, &ASimulationController::Action);
}

void ASimulationController::Throttle(float value) {
	if (value != 0 && Craft) {
		ThrottleValue = FMath::Clamp(ThrottleValue + value, 0, 1);
		HUD->Throttle->SetPercent(ThrottleValue);
	}
}

void ASimulationController::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	Craft->Throttle(ThrottleValue);
	// UE_LOG(LogTemp, Warning, TEXT("craft ore %s"), *craft->GetActorRotation().ToString());
}

void ASimulationController::Zoom(float value) {
	if (value != 0 && GetPawn() == Craft) {
		PlayerCameraManager->FreeCamDistance *= (1 - value);
	}
}

void ASimulationController::Pitch(float value) {
	if (value != 0 && GetPawn() == Craft) {
		Craft->Rotate(FRotator(45, 0, 0), 100000000 * value);
		ControlStabilizer->TimeSinceLastInput = 0;
	}
}
void ASimulationController::Roll(float value) {
	if (value != 0 && GetPawn() == Craft) {
		Craft->Rotate(FRotator(0, 0, 45), 100000000 * value);
		ControlStabilizer->TimeSinceLastInput = 0;
	}
}
void ASimulationController::Yaw(float value) {
	if (value != 0 && GetPawn() == Craft) {
		Craft->Rotate(FRotator(0, 45, 0), 100000000 * value);
		ControlStabilizer->TimeSinceLastInput = 0;
	}
}
void ASimulationController::Stage() {
	if (GetPawn() == Craft) {
		Craft->StageCraft();
	}
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
}

void ASimulationController::SetTimeWarp(int TimeWarpLevel) {
	TimeWarp = TimeWarpLevel;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeWarpMapping[TimeWarp]);
	HUD->TimeWarp->SetPercent(float(TimeWarp) / 6);
}

void ASimulationController::TimeWarpAdd() {
	if (TimeWarp < 6) {
		SetTimeWarp(TimeWarp + 1);
	}
}
void ASimulationController::TimeWarpSub() {
	if (TimeWarp > 0) {
		SetTimeWarp(TimeWarp - 1);
	}
}
void ASimulationController::TimeWarpReset() {
	SetTimeWarp(1);
}

void ASimulationController::Action(FKey Key) {
	if (Key == EKeys::One) {
		Craft->SetPhysicsEnabled(false);
	}
	else if (Key == EKeys::Two) {
		Craft->SetPhysicsEnabled(true);
	}
	else if (Key == EKeys::Three) {
		Craft->SetActorLocation(FVector(-501000, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else if (Key == EKeys::Four) {
		if (Craft->OrbitComponent->CentralBody == nullptr) {
			Craft->OrbitComponent->CentralBody = Earth;
			UE_LOG(LogTemp, Warning, TEXT("Craft orbit didn't have a central body"))
		}
	}
	else if (Key == EKeys::Five) {
		UOrbitComponent* orbit = NewObject<UOrbitComponent>(this);
		orbit->CentralBody = Earth;
		orbit->UpdateOrbit(Craft->GetActorLocation(), Craft->GetVelocity());

		/*
		TArray<FVector> Array;
		for (int i = 0; i < 360; i++) {
			Array.Add(orbit->GetPosition(i));
		}

		for (int i = 0; i < 359; i++) {
			DrawDebugDirectionalArrow(GetWorld(), Array[i], Array[i + 1], 20, FColor(200, 0, 200), true, -1, 0, 2);
		}*/
	}
	else if (Key == EKeys::Six) {
		UE_LOG(LogTemp, Warning, TEXT("craft loc %s"), *Craft->GetActorLocation().ToString());
	}

	UE_LOG(LogTemp, Warning, TEXT("action %s"), *Key.GetFName().ToString());
}
