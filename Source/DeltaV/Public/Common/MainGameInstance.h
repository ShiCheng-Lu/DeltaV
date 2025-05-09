// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "MainGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UMainGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	FString CraftPath;

	void SaveSettings();
	void LoadSettings();

	TObjectPtr<USaveGame> Settings;

	static void GetSettings();

	bool AeroArrows = false;
};
