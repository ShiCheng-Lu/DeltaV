// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "InputTriggerInvChordAction.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Inv Chorded Action", NotInputConfigurable = "true"))
class DELTAV_API UInputTriggerInvChordAction : public UInputTriggerChordAction
{
	GENERATED_BODY()

protected:
	// Implicit, so action cannot fire unless this is firing.
	virtual ETriggerType GetTriggerType_Implementation() const override { return ETriggerType::Implicit; }

	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;
};
