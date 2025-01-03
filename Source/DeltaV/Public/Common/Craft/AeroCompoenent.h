// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Craft/PartComponent.h"
#include "AeroCompoenent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UAeroCompoenent : public UPartComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAeroCompoenent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*
	{
	}
	*/
	virtual void FromJson(TSharedPtr<FJsonObject> Json) override;
	virtual TSharedPtr<FJsonObject> ToJson() override;
};
