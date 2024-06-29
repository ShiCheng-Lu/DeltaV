// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ConstructionController.generated.h"

class UPart;
class AConstructionCraft;
class UConstructionHUD;

/**
 * 
 */
UCLASS()
class DELTAV_API AConstructionController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AConstructionController();

	void HandleClick(FKey Key);

	virtual void SetupInputComponent() override;

	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY()
	UPart* SelectedPart;

	UPROPERTY()
	AConstructionCraft* Craft;

	UPROPERTY()
	AConstructionCraft* Selected;

	UPROPERTY()
	float PlaceDistance;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UConstructionHUD> HUDClass;

	UPROPERTY()
	UConstructionHUD* HUD;

	enum Mode {
		EditMode,
		RotateMode,
		TranslateMode,
	};

	Mode ConstructionMode = Mode::EditMode;

	void DebugAction();

	void Save();

	void Load();

	// return the part that the held part is placed against, if it exists
	UPart* PlaceHeldPart();

	void Throttle(float Val);
};
