// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "OrbitLine.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UOrbitLine : public USplineComponent
{
	GENERATED_BODY()
	
public:
	TArray<TObjectPtr<class USplineMeshComponent>> Body;

	void Generate();
};
