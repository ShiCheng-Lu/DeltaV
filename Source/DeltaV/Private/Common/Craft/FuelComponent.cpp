// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/FuelComponent.h"

FuelState::FuelState() : TMap<FuelType, double>({
	{ FuelType::LiquidFuel, 0 },
	{ FuelType::Monopropellant, 0 },
	{ FuelType::Oxidizer, 0 },
	{ FuelType::SolidFuel, 0 },
}) {}

double FuelState::Drain(FuelState Fuel) {
	double Total = 0;
	for (auto& KVP : *this) {
		KVP.Value += Fuel.FindChecked(KVP.Key);
		Total += KVP.Value;
	}
	return Total;
}

double FuelState::Fill(FuelState Fuel) {
	double Total = 0;
	for (auto& KVP : *this) {
		KVP.Value += Fuel.FindChecked(KVP.Key);
		Total += KVP.Value;
	}
	return Total;
}

// Sets default values for this component's properties
UFuelComponent::UFuelComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFuelComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFuelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

