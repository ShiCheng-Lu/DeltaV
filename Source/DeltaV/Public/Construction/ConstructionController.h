// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Constructor.h"
#include "ConstructionController.generated.h"

class UPart;
class ACraft;
class UConstructionHUD;
class ATransformGadget;

/**
 * 
 */
UCLASS()
class DELTAV_API AConstructionController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AConstructionController();

	void Pressed(FKey Key);
	void Released(FKey Key);

	virtual void SetupInputComponent() override;

	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	Constructor Constructor;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UConstructionHUD> HUDClass;

	UPROPERTY()
	UConstructionHUD* HUD;

	enum Mode {
		EditMode,
		RotateMode,
		TranslateMode,
		ScaleMode,
		WarpMode,
	};

	Mode ConstructionMode = Mode::EditMode;

	void SwitchMode(Mode NewMode);

	ATransformGadget* TransformGadget;

	const static int SYMMETRY_NONE = 1;
	const static int SYMMETRY_MIRROR = 0;

	void DebugAction();

	void Save();

	void Load();

	void SymmetryAdd();
	void SymmetrySub();

	void EnableMovement();
	void DisableMovement();

	void Zoom(float value);

	void RotatePart(FRotator Rotation);

	FRay GetMouseRay();
};
