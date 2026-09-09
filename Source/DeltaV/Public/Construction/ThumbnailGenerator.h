// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"

/**
 * 
 */
class DELTAV_API ThumbnailGenerator
{
public:
	ThumbnailGenerator();
	~ThumbnailGenerator();

	void GenerateThumbnail(const FString& Path, TObjectPtr<AActor> Actor);
	void GenerateThumbnails(TMap<FString, TObjectPtr<AActor>> FilePathToActors);

private:
	TObjectPtr<UWorld> CreateWorld();
	void SpawnActors(TObjectPtr<UWorld> World);
	void Render(TObjectPtr<UWorld> World);
	void SaveImage(TObjectPtr<UWorld> World);
	void DestroyWorld(TObjectPtr<UWorld> World);
};
