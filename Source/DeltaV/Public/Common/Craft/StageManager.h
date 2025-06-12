// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StageManager.generated.h"

class UPart;
class ACraft;

/**
 *
 */
class DELTAV_API UStage : public UObject
{
public:
	TArray<UPart*> Parts;

	TArray<UPart*> Decouplers;
	TArray<UPart*> Fuels;
	TArray<UPart*> Engines;

	int StageNumber;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UStageManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStageManager();

	TArray<UStage*> Stages;

	UStage* Active;

	ACraft* Craft;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	TArray<ACraft*> Stage();

	/*
	Json serialization
	{
		"parts": [<part-id>]
		"decouplers": [<part-id>]
		"fuels": [<part-id>]
		"engines": [<part-id>]
	}
	*/
	void FromJson(TArray<TSharedPtr<FJsonValue>> Json);
	TArray<TSharedPtr<FJsonValue>> ToJson();
};
