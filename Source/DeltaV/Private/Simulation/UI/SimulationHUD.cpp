// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/UI/SimulationHUD.h"

#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Common/Craft.h"
#include "Simulation/UI/Navball.h"
#include "Simulation/SimulationController.h"
#include "Simulation/ControlStabilizer.h"

USimulationHUD::USimulationHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!USimulationHUD::BlueprintClass) {
		ConstructorHelpers::FClassFinder<USimulationHUD> Widget(TEXT("WidgetBlueprint'/Game/Simulation/UI/WBP_SimulationHUD'"));
		if (Widget.Succeeded()) {
			USimulationHUD::BlueprintClass = Widget.Class;
		}
	}
}

void USimulationHUD::NativeOnInitialized() {
	Super::NativeOnInitialized();

	Controller = Cast<ASimulationController>(GetOwningPlayer());

	NavballActor = GetWorld()->SpawnActor<ANavball>();

	FScriptDelegate delegate;
	delegate.BindUFunction(Controller, "VelChanged");
	Velocity->OnValueChanged.Add(delegate);

	FScriptDelegate delegate1;
	delegate1.BindUFunction(Controller, "GravChanged");
	Gravity->OnValueChanged.Add(delegate1);
}

void USimulationHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	
}

void USimulationHUD::SetNavballTarget(ACraft* Craft, FVector PlanetCenter) const {
	NavballActor->SetTarget(Craft, PlanetCenter);
}

void USimulationHUD::SetStabilizationMode(EStabilizationMode Mode) {
	Controller->ControlStabilizer->Mode = Mode;

	switch (Mode)
	{
	case EStabilizationMode::NONE:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization NONE"));
		break;
	case EStabilizationMode::HOLD_ATTITUDE:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization HOLD_ATTITUDE"));
		break;
	case EStabilizationMode::MANEUVER:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization MANEUVER"));
		break;
	case EStabilizationMode::PROGRADE:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization PROGRADE"));
		break;
	case EStabilizationMode::RETROGRADE:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization RETROGRADE"));
		break;
	case EStabilizationMode::RADIAL_IN:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization RADIAL_IN"));
		break;
	case EStabilizationMode::RADIAL_OUT:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization RADIAL_OUT"));
		break;
	case EStabilizationMode::NORMAL:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization NORMAL"));
		break;
	case EStabilizationMode::ANTI_NORMAL:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization ANTI_NORMAL"));
		break;
	case EStabilizationMode::TARGET:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization TARGET"));
		break;
	case EStabilizationMode::ANTI_TARGET:
		UE_LOG(LogTemp, Warning, TEXT("Stabilization ANTI_TARGET"));
		break;
	default:
		break;
	}

}
