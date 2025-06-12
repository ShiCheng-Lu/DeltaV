// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/UI/SimulationHUD.h"

#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ProgressBar.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Common/Craft.h"
#include "Common/UI/StagesList.h"
#include "Simulation/UI/Navball.h"
#include "Simulation/SimulationController.h"
#include "Simulation/ControlStabilizer.h"
#include "Simulation/OrbitComponent.h"
#include "Common/Craft/FuelManager.h"

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
	if (Controller == nullptr || Controller->Craft == nullptr) {
		return;
	}

	VelocityValue = VelocityValue * 0.95 + Controller->Craft->GetVelocity() * 0.05;

	Info->SetText(FText::Format(FTextFormat::FromString("Periapsis: {0}\nApoapsis: {1}\nAltitude: {2}\nVelocity: {3}"), 
		Controller->Craft->Orbit->Periapsis() * 0.01,
		Controller->Craft->Orbit->Apoapsis() * 0.01,
		Controller->Craft->GetActorLocation().Length() * 0.01,
		Controller->Craft->GetVelocity().Length() * 0.01));

	Throttle->SetPercent(Controller->ThrottleValue);

	{
		UFuelManager* FuelManager = Controller->Craft->FuelManager;
		FuelState FuelPercent = FuelManager->TotalFuel / FuelManager->TotalMax;

		Fuel->SetPercent(FuelPercent.FindChecked(FuelType::LiquidFuel));
	}
}

void USimulationHUD::SetNavballTarget(ACraft* Craft, FVector PlanetCenter) const {
	NavballActor->SetTarget(Craft, PlanetCenter);
	StagesList->Manager = Craft->StageManager;
	StagesList->Reload();
}

void USimulationHUD::SetStabilizationMode(bool Checked, EStabilizationMode Mode) {
	FString ModeString;
	switch (Mode)
	{
	case EStabilizationMode::None:
		ModeString = "None";
		break;
	case EStabilizationMode::HoldAttitude:
		ModeString = "HoldAttitude";
		break;
	case EStabilizationMode::Maneuver:
		ModeString = "Maneuver";
		break;
	case EStabilizationMode::Prograde:
		ModeString = "Prograde";
		break;
	case EStabilizationMode::Retrograde:
		ModeString = "Retrograde";
		break;
	case EStabilizationMode::RadialIn:
		ModeString = "RadialIn";
		break;
	case EStabilizationMode::RadialOut:
		ModeString = "RadialOut";
		break;
	case EStabilizationMode::Normal:
		ModeString = "Normal";
		break;
	case EStabilizationMode::AntiNormal:
		ModeString = "AntiNormal";
		break;
	case EStabilizationMode::Target:
		ModeString = "Target";
		break;
	case EStabilizationMode::AntiTarget:
		ModeString = "AntiTarget";
		break;
	default:
		break;
	}

	if (Checked) {
		// deselect all others
		for (UWidget* Child : StabilizationPanel->GetAllChildren()) {
			UCheckBox* Checkbox = Cast<UCheckBox>(Child);
			if (Checkbox->GetName() != ModeString && Checkbox->GetCheckedState() == ECheckBoxState::Checked) {
				Checkbox->SetCheckedState(ECheckBoxState::Unchecked);
			}
		}
		NavballActor->StabilizationMode = Mode;
	}
	else {
		// if any other checkbox is checked, this was the result of a change
		for (UWidget* Child : StabilizationPanel->GetAllChildren()) {
			UCheckBox* Checkbox = Cast<UCheckBox>(Child);
			if (Checkbox->GetName() != ModeString && Checkbox->GetCheckedState() == ECheckBoxState::Checked) {
				return;
			}
		}
		// no other stabilization is checked, disable stabilization
		NavballActor->StabilizationMode = EStabilizationMode::None;
	}

	UE_LOG(LogTemp, Warning, TEXT("Stabilization %s"), *ModeString);
}

void USimulationHUD::ButtonClicked(UCheckBox* Checkbox) {
	UE_LOG(LogTemp, Warning, TEXT("Button clicked %s"), *Checkbox->GetName());

	if (Checkbox->GetName() == "Stabilization") {
		if (Checkbox->GetCheckedState() == ECheckBoxState::Checked) {
			StabilizationPanel->SetVisibility(ESlateVisibility::Visible);
			SetStabilizationMode(true, EStabilizationMode::HoldAttitude);
		} else {
			StabilizationPanel->SetVisibility(ESlateVisibility::Hidden);
			SetStabilizationMode(true, EStabilizationMode::None);
		}
	}
	else if (Checkbox->GetName() == "RCS") {

	}
	else if (Checkbox->GetName() == "Gear") {

	}
	else if (Checkbox->GetName() == "Lights") {

	}
}


