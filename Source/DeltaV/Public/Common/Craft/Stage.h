// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Stage.generated.h"

class UPart;
class ACraft;

/**
 * 
 */
UCLASS()
class DELTAV_API UStage : public UObject
{
	GENERATED_BODY()
	

public:
	ACraft* Craft;

	TArray<UPart*> Parts;

	TArray<UPart*> Decouplers;
	TArray<UPart*> Fuels;
	TArray<UPart*> Engines;

	int StageNumber;

	UStage(const FObjectInitializer& ObjectInitializer);

	/*
	Json serialization
	{
		"parts": [<part-id>]
		"decouplers": [<part-id>]
		"fuels": [<part-id>]
		"engines": [<part-id>]
	}
	*/
	void FromJson(TSharedPtr<FJsonValue> Json);
	TSharedPtr<FJsonValue> ToJson();

	TArray<ACraft*> Activate();
};
