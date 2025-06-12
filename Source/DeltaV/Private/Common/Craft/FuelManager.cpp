// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/FuelManager.h"
#include "Common/Craft/FuelComponent.h"
#include "Common/Craft/EngineComponent.h"

#include "Common/Part.h"
#include "Common/Craft.h"

#include "Simulation/SimulationController.h"

// ========================
// FuelGroup
// ========================

FuelState FuelGroup::TotalFuel() {
	FuelState State;
	for (UPart* Part : Fuels) {
		auto* Fuel = Part->GetComponent<UFuelComponent>("fuel");
		if (Fuel == nullptr) {
			continue;
		}
		State.Fill(Fuel->Current);
	}
	return State;
}

FuelState FuelGroup::TotalMax() {
	FuelState State;
	for (UPart* Part : Fuels) {
		auto* Fuel = Part->GetComponent<UFuelComponent>("fuel");
		if (Fuel == nullptr) {
			continue;
		}
		State.Fill(Fuel->Max);
	}
	return State;
}

FuelState FuelGroup::TotalDrain() {
	FuelState State;
	for (UPart* Part : Engines) {
		auto* Engine = Part->GetComponent<UEngineComponent>("engine");
		if (Engine == nullptr) {
			continue;
		}
		State.Fill(Engine->Drain);
	}
	return State;
}


// Sets default values for this component's properties
UFuelManager::UFuelManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_DuringPhysics;

	// ...
	Craft = Cast<ACraft>(GetOwner());
}


// Called when the game starts
void UFuelManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// Add this ticking to pre-requisite to all engine ticking
	for (FuelGroup& FuelGroup : FuelGroups) {
		for (UPart* Engine : FuelGroup.Engines) {
			UEngineComponent* EngineComponent = Engine->GetComponent<UEngineComponent>("engine");
			EngineComponent->PrimaryComponentTick.AddPrerequisite(this, PrimaryComponentTick);
		}
	}
}


// Called every frame
void UFuelManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ASimulationController* SimulationController = Cast<ASimulationController>(Craft->Controller);
	if (!SimulationController) {
		return;
	}

	double SavedThrottle = SimulationController->ThrottleValue;

	TotalFuel = {};
	TotalMax = {};
	// ...
	for (FuelGroup& FuelGroup : FuelGroups) {
		// Total fuel in this group and total drained in this group
		FuelState FuelAmount = FuelGroup.TotalFuel();
		FuelState DrainAmount = FuelGroup.TotalDrain() * DeltaTime;

		TotalFuel.Fill(FuelAmount);
		TotalMax.Fill(FuelGroup.TotalMax());

		double FuelGroupThrottle = FMath::Min(SavedThrottle, FuelAmount.CanDrain(DrainAmount));

		for (UPart* Engine : FuelGroup.Engines) {
			UEngineComponent* EngineComponent = Engine->GetComponent<UEngineComponent>("engine");
			EngineComponent->Thrust = FuelGroupThrottle;
		}

		FuelState DrainRatio = DrainAmount / FuelAmount * FuelGroupThrottle;
		for (UPart* Fuel : FuelGroup.Fuels) {
			UFuelComponent* FuelComponent = Fuel->GetComponent<UFuelComponent>("fuel");
			FuelComponent->Current.Drain(FuelComponent->Current * DrainRatio);
		}

		UE_LOG(LogTemp, Warning, TEXT("Fuel: %f, Drain: %f"), TotalFuel[FuelType::LiquidFuel], DrainRatio[FuelType::LiquidFuel]);
	}
}

void UFuelManager::FromJson() {
	FuelGroups.Add({});

	for (auto PartKVP : Craft->Parts) {
		UPart* Part = PartKVP.Value;

		if (Part->GetComponent<UFuelComponent>("fuel")) {
			FuelGroups.Last().Fuels.Add(Part);
		}
		if (Part->GetComponent<UEngineComponent>("engine")) {
			FuelGroups.Last().Engines.Add(Part);
		}
	}
}

/*
void UFuelManager::ToJson() {

}
*/
