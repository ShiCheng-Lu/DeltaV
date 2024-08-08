// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/UI/SimulationHUD.h"

#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
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
	// Info->SetText(FText::Format(FTextFormat::FromString("Periapsis: %.2f\nApoapsis: %.2f"), ));


}

void USimulationHUD::SetNavballTarget(ACraft* Craft, FVector PlanetCenter) const {
	NavballActor->SetTarget(Craft, PlanetCenter);
}

void USimulationHUD::SetStabilizationMode(bool Checked, EStabilizationMode Mode) {
	FString ModeString;
	switch (Mode)
	{
	case NONE:
		ModeString = "None";
		break;
	case HOLD_ATTITUDE:
		ModeString = "HoldAttitude";
		break;
	case MANEUVER:
		ModeString = "Maneuver";
		break;
	case PROGRADE:
		ModeString = "Prograde";
		break;
	case RETROGRADE:
		ModeString = "Retrograde";
		break;
	case RADIAL_IN:
		ModeString = "RadialIn";
		break;
	case RADIAL_OUT:
		ModeString = "RadialOut";
		break;
	case NORMAL:
		ModeString = "Normal";
		break;
	case ANTI_NORMAL:
		ModeString = "AntiNormal";
		break;
	case TARGET:
		ModeString = "Target";
		break;
	case ANTI_TARGET:
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
		Controller->ControlStabilizer->Mode = Mode;
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
		Controller->ControlStabilizer->Mode = EStabilizationMode::NONE;
	}

	UE_LOG(LogTemp, Warning, TEXT("Stabilization %s"), *ModeString);
}

void USimulationHUD::ButtonClicked(UCheckBox* Checkbox) {
	UE_LOG(LogTemp, Warning, TEXT("Button clicked %s"), *Checkbox->GetName());

	if (Checkbox->GetName() == "Stabilization") {
		if (Checkbox->GetCheckedState() == ECheckBoxState::Checked) {
			StabilizationPanel->SetVisibility(ESlateVisibility::Visible);
			SetStabilizationMode(true, EStabilizationMode::HOLD_ATTITUDE);
		} else {
			StabilizationPanel->SetVisibility(ESlateVisibility::Hidden);
			SetStabilizationMode(true, EStabilizationMode::NONE);
		}
	}
	else if (Checkbox->GetName() == "RCS") {

	}
	else if (Checkbox->GetName() == "Gear") {

	}
	else if (Checkbox->GetName() == "Lights") {

	}
}
