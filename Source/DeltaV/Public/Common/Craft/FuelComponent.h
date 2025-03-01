// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Craft/PartComponent.h"
#include "FuelComponent.generated.h"

UENUM()
enum class FuelType {
	LiquidFuel,
	Oxidizer,
	SolidFuel,
	Monopropellant,
};

class FuelState : public TMap<FuelType, double> {
public:
	FuelState();

	double Drain(FuelState FuelDrain);
	double Fill(FuelState FuelFill);

	void FromJson(TSharedPtr<FJsonObject> Json);
	TSharedPtr<FJsonObject> ToJson();

	FuelState operator*(double Mult);
	FuelState operator/(FuelState Other);
	
	double CanDrain(FuelState FuelDrain);
};

class FuelGroup {
public:
	TArray<UPart*> Fuels;
	TArray<UPart*> Engines;

	FuelState TotalFuel();
	FuelState TotalDrain();

};


/*
Fuel can hold any combination of fuel types
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UFuelComponent : public UPartComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFuelComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TickPostPhysics(float DeltaTime);

	double TotalCapacity;
	
	FuelState Current;
	FuelState Max;


	

	/*
	{
		"current": {
			
		}
	}
	*/
	virtual void FromJson(TSharedPtr<FJsonObject> Json) override;
	virtual TSharedPtr<FJsonObject> ToJson() override;

	FuelState Total();
};
