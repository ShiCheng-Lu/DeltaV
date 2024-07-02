// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/UI/SimulationHUD.h"

#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Common/Craft.h"
#include "Simulation/SimulationController.h"
#include "Simulation/UI/Navball.h"

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
