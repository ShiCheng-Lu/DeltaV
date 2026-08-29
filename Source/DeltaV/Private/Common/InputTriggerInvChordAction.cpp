// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/InputTriggerInvChordAction.h"

#include "EnhancedInputModule.h"
#include "EnhancedPlayerInput.h"
#include "HAL/IConsoleManager.h"
#include "Misc/DataValidation.h"

ETriggerState UInputTriggerInvChordAction::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
	ETriggerState EventDataState = Super::UpdateState_Implementation(PlayerInput, ModifiedValue, DeltaTime);
	switch (EventDataState)
	{
	case ETriggerState::None:
		return ETriggerState::Triggered;
	case ETriggerState::Ongoing:
		return ETriggerState::Ongoing;
	case ETriggerState::Triggered:
		return ETriggerState::None;
	default:
		return ETriggerState::None;
	}
}
