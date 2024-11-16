// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UPart;
class UWorld;
class ACraft;
class AConstructionController;

#define ECC_NoneHeldParts ECC_GameTraceChannel1
#define ECC_AttachmentNodes ECC_GameTraceChannel2

/**
 * 
 */
class DELTAV_API Constructor
{
	AConstructionController* Controller;

	FActorSpawnParameters SpawnParamsAlwaysSpawn;

	TArray<ACraft*> SymmetryCrafts;

	UWorld* World;

public:
	UPart* Selected;
	double Distance;
	int Symmetry = 1;

	Constructor();
	~Constructor();

	void SetController(AConstructionController* InController);

	ACraft* CreateCraft(TSharedPtr<FJsonObject> CraftJson);

	UPart* Update();

	void Tick();

	void Place();
	void Grab();

	// start holding a part, the part (and it's craft) will be ignored from trace
	void Select(UPart* Part = nullptr);
	void Deselect() { Select(nullptr); }


	UPart* Trace(FVector Position, FVector Direction);

	// Get the part under the mouse
	UPart* TraceMouse();

	void UpdateSymmetry(int InSymmetry);

	void RotatePart(FQuat Rotation);

	// delete currently held part
	void Delete();
};
