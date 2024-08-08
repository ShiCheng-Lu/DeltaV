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
#include "Simulation/ControlStabilizer.h"


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
	Craft->Initialize(JsonUtil::ReadFile(Path));

	FVector origin, extent;
	Craft->GetActorBounds(true, origin, extent);
	Craft->SetActorLocation(FVector(-(Earth->GetActorScale3D().Z * 100 + extent.Z * 2), 0, 0));
	Craft->SetActorRotation(FRotator(180, 0, 0));
	Craft->SetPhysicsEnabled(true);

	Possess(Craft);
	SetControlRotation(FRotator(0));
	Craft->CentralBody = Earth;

	PlayerCameraManager->FreeCamDistance = 1000;

	HUD = CreateWidget<USimulationHUD>(this, USimulationHUD::BlueprintClass);
	HUD->AddToPlayerScreen();

	HUD->SetNavballTarget(Craft, FVector(0, 0, 0));

	UE_LOG(LogTemp, Warning, TEXT("craft location %s %f"), *Craft->GetActorLocation().ToString(), Earth->GetActorScale3D().Z);

	
	ControlStabilizer = NewObject<UControlStabilizer>(this); // CreateDefaultSubobject<UControlStabilizer>("Stabilizer");
	ControlStabilizer->Controller = this;

	ControlStabilizer->RegisterComponent();
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


	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action1", EKeys::One));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action2", EKeys::Two));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action3", EKeys::Three));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action4", EKeys::Four));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action5", EKeys::Five));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Action6", EKeys::Six));

	InputComponent->BindAxis("LookX", this, &ASimulationController::AddYawInput);
	InputComponent->BindAxis("LookY", this, &ASimulationController::AddPitchInput);

	InputComponent->BindAxis("Throttle", this, &ASimulationController::Throttle);

	InputComponent->BindAxis("CameraZoom", this, &ASimulationController::Zoom);

	InputComponent->BindAxis("Pitch", this, &ASimulationController::Pitch);
	InputComponent->BindAxis("Roll", this, &ASimulationController::Roll);
	InputComponent->BindAxis("Yaw", this, &ASimulationController::Yaw);

	InputComponent->BindAction("Stage", EInputEvent::IE_Pressed, this, &ASimulationController::Stage);
	InputComponent->BindAction("ToggleMap", EInputEvent::IE_Pressed, this, &ASimulationController::ToggleMap);

	InputComponent->BindAction("TimeWarpAdd", EInputEvent::IE_Pressed, this, &ASimulationController::TimeWarpAdd);
	InputComponent->BindAction("TimeWarpSub", EInputEvent::IE_Pressed, this, &ASimulationController::TimeWarpSub);
	InputComponent->BindAction("TimeWarpReset", EInputEvent::IE_Pressed, this, &ASimulationController::TimeWarpReset);

	InputComponent->BindAction<TDelegate<void(int)>, ASimulationController>("Action1", EInputEvent::IE_Pressed, this, &ASimulationController::Action, 1);
	InputComponent->BindAction<TDelegate<void(int)>, ASimulationController>("Action2", EInputEvent::IE_Pressed, this, &ASimulationController::Action, 2);
	InputComponent->BindAction<TDelegate<void(int)>, ASimulationController>("Action3", EInputEvent::IE_Pressed, this, &ASimulationController::Action, 3);
	InputComponent->BindAction<TDelegate<void(int)>, ASimulationController>("Action4", EInputEvent::IE_Pressed, this, &ASimulationController::Action, 4);
	InputComponent->BindAction<TDelegate<void(int)>, ASimulationController>("Action5", EInputEvent::IE_Pressed, this, &ASimulationController::Action, 5);
	InputComponent->BindAction<TDelegate<void(int)>, ASimulationController>("Action6", EInputEvent::IE_Pressed, this, &ASimulationController::Action, 6);

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

	DrawDebugDirectionalArrow(GetWorld(), Craft->GetActorLocation(), Craft->GetActorLocation() + Craft->GetActorRotation().Vector() * 10, 2, FColor(255, 0, 0));

	DrawDebugSphere(GetWorld(), FVector(-(Earth->Radius * 100 + 400), 0, 0), 20, 10, FColor(0, 255, 0));
	DrawDebugDirectionalArrow(GetWorld(), FVector(-(Earth->Radius * 100 + 400), 0, 0), FVector(-(Earth->Radius * 100 + 400), 0, 100), 2, FColor(0, 255, 0));
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
	UE_LOG(LogTemp, Warning, TEXT("grav, vel - %f, %f"), HUD->Gravity->GetValue(), HUD->Velocity->GetValue());

	Craft->SetPhysicsEnabled(false);
	Craft->SetActorLocation(FVector(-(Earth->Radius * 100 + 400), 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
	
	UE_LOG(LogTemp, Warning, TEXT("craft loc %s"), *Craft->GetActorLocation().ToString());

	FVector Velocity = FVector(0, HUD->Velocity->GetValue(), 0);
	
	for (auto& PartKVP : Craft->Parts) {
		auto& Part = PartKVP.Value;
		Part->ComponentVelocity = Velocity;
	}
	Craft->GetRootComponent()->ComponentVelocity = Velocity;
	
	
	UOrbitComponent* orbit = NewObject<UOrbitComponent>(this);

	orbit->UpdateOrbit(Craft->GetActorLocation(), Velocity, Earth);
	
	TArray<FVector> array;
	for (int i = 0; i < 360; i++) {
		array.Add(orbit->GetPosition(i));
	}

	for (int i = 0; i < 359; i++) {
		DrawDebugLine(GetWorld(), array[i], array[i + 1], FColor(200, 0, 200), false, 5, 0, 2);
		// UE_LOG(LogTemp, Warning, TEXT("p %d: %s"), i, *array[i].ToString());
	}
	// GetHUD()->Draw3DLine(array[0], array[35], FColor(230, 230, 0));
	// DrawDebugSphere(GetWorld(), FVector(0, 0, 0), 10, 40, FColor(0, 250, 0), false, 5, 0, 0);
	// DrawDebugSphere(GetWorld(), FVector(100, 0, 0), 1, 40, FColor(0, 250, 0), false, 5, 0, 0);
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

void ASimulationController::Action(int Action) {
	switch (Action)
	{
	case 1:
		Craft->SetPhysicsEnabled(false);
		break;
	case 2:
		Craft->SetPhysicsEnabled(true);
		break;
	case 3:
		Craft->SetActorLocation(FVector(-501000, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
		break;
	case 4:
		break;
	case 5:
	{
		UOrbitComponent* orbit = NewObject<UOrbitComponent>(this);

		orbit->UpdateOrbit(Craft->GetActorLocation(), Craft->GetVelocity(), Earth);

		TArray<FVector> Array;
		for (int i = 0; i < 360; i++) {
			Array.Add(orbit->GetPosition(i));
		}

		for (int i = 0; i < 359; i++) {
			DrawDebugDirectionalArrow(GetWorld(), Array[i], Array[i + 1], 20, FColor(200, 0, 200), true, -1, 0, 2);
		}
	}
		break;
	case 6:
		UE_LOG(LogTemp, Warning, TEXT("craft loc %s, vel %s (%f)"), *Craft->GetActorLocation().ToString(), *Craft->GetVelocity().ToString(), Craft->GetVelocity().Length());
		break;
	default:
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("action %d"), Action);
}
