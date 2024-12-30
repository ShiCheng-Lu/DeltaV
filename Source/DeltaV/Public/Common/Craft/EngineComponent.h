// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Craft/PartComponent.h"
#include "Common/Craft/FuelComponent.h"
#include "Common/CustomTickFunction.h"
#include "EngineComponent.generated.h"

class UPart;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DELTAV_API UEngineComponent : public UPartComponent
{
	GENERATED_BODY()

	FuelState Drain;
	FVector PivotOffset;
	double PivotLimit;
	double Thrust;

	TArray<UPart*> Fuels;

public:
	// Sets default values for this component's properties
	UEngineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*
	{
		"pivot_offset": FVector,
		"pivot_limit": double,
		"thrust", double,
		"fuel_drain": FuelState,
	}
	*/
	virtual void FromJson(TSharedPtr<FJsonObject> Json) override;
	virtual TSharedPtr<FJsonObject> ToJson() override;


	void PostPhysics(float a);
	void DuringPhysics(float a);

	FCustomComponentTick<UEngineComponent> PostPhysicsTick;
	FCustomComponentTick<UEngineComponent> DuringPhysicsTick;
};
