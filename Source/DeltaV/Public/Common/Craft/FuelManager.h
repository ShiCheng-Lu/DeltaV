// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Craft/FuelComponent.h"
#include "FuelManager.generated.h"

class FuelGroup {
public:
	TArray<UPart*> Fuels;
	TArray<UPart*> Engines;

	FuelState TotalFuel();
	FuelState TotalMax();
	FuelState TotalDrain();
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UFuelManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFuelManager();

	TObjectPtr<class ACraft> Craft;
	double Throttle;

	FuelState TotalFuel;
	FuelState TotalMax;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TArray<FuelGroup> FuelGroups;

	void TickDuringPhysics(float DeltaTime);

	void TickPostPhysics(float DeltaTime);

	void FromJson();
};
