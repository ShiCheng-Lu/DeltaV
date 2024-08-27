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


	int Symmetry = 1;
	const static int SYMMETRY_NONE = 1;
	const static int SYMMETRY_MIRROR = 0;

	void DebugAction();

	void Save();

	void Load();

	// return the part that the held part is placed against, if it exists
	UPart* PlaceHeldPart();

	// return the part that the help part will be placed against, if it exists
	std::pair<UPart*, bool> UpdateHeldPart();

	void Throttle(float Val);

	void SymmetryAdd();
	void SymmetrySub();
};
