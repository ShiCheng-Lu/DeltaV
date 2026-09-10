// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"

/**
 * 
 */
class DELTAV_API ThumbnailGenerator
{
	TObjectPtr<UWorld> World;
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

public:
	ThumbnailGenerator();
	~ThumbnailGenerator();

	void GenerateThumbnail(const FString& Path, TObjectPtr<AActor> Actor);
	void GenerateThumbnails(TMap<FString, TObjectPtr<AActor>> FilePathToActors);

private:
	bool Initialize();
	bool Render(const FString& Path, TObjectPtr<AActor> Actor);
	void Cleanup();
};
