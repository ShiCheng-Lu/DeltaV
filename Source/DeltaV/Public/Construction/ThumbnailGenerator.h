// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"

DECLARE_DELEGATE_RetVal_TwoParams(AActor*, FSpawnActor, UWorld*, FString)

/**
 * 
 */
class DELTAV_API ThumbnailGenerator
{
	TObjectPtr<UWorld> World;
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

public:
	ThumbnailGenerator();
	~ThumbnailGenerator();

	void GenerateThumbnail(const FString& Path, FSpawnActor SpawnActor);
	void GenerateThumbnails(TArray<FString> Paths, FSpawnActor SpawnActor);

private:
	bool Initialize();
	bool Render(const FString& Path, FSpawnActor SpawnActor);
	void Cleanup();
};
