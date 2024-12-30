// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PartComponent.generated.h"

class UPart;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UPartComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPartComponent();

protected:
	UPart* Part;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	virtual void FromJson(TSharedPtr<FJsonObject> Json) {};
	virtual TSharedPtr<FJsonObject> ToJson() { return MakeShareable(new FJsonObject()); };
};
