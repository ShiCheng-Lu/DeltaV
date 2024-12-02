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

	TArray<UPart*> Parts;

	int StageNumber;

	/*
	Json serialization
	{
		"parts": [<part-id>]
	}
	*/
	void FromJson(TSharedPtr<FJsonValue> Json);
	TSharedPtr<FJsonValue> ToJson();
};
