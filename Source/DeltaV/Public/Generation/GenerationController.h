// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DynamicMeshActor.h"
#include "GenerationController.generated.h"


class UMeshGeneration;
class UDynamicMesh;

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
	bool Progress = false;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY()
	UMeshGeneration* ShapeEditor;

	FDynamicMesh3 Mesh;

	UPROPERTY()
	UDynamicMesh* DynamicMesh;
	
	UPROPERTY()
	ADynamicMeshActor* NewActor;

	void SetupInputComponent();
	void Start();
	void Stop();
	void Toggle();
	void Clear();
};
