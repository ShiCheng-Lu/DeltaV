// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Craft/PartComponent.h"
#include "WheelComponent.generated.h"

class UStaticMeshComponent;
class UPhysicsConstraintComponent;

/**
 * 
 */
UCLASS()
class DELTAV_API UWheelComponent : public UPartComponent
{
	GENERATED_BODY()

public:
	UWheelComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetPhysicsEnabled(bool bSimulate) override;

	FVector Offset;
	UStaticMeshComponent* Mesh;
	UPhysicsConstraintComponent* Physics;

	/*
	{
		"mesh": <string>,
		"offset": <vec3>,

	}
	*/
	virtual void FromJson(TSharedPtr<FJsonObject> Json) override;
	virtual TSharedPtr<FJsonObject> ToJson() override;
};
