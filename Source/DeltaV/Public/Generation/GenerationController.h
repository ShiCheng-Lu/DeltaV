// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DynamicMeshActor.h"
#include "GenerationController.generated.h"


class UMeshGeneration;

/**
 * 
 */
UCLASS()
class DELTAV_API AGenerationController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	bool started = false;

	virtual void Tick(float DeltaSeconds) override;

	UMeshGeneration* ShapeEditor;
	FDynamicMesh3 Mesh;

};
