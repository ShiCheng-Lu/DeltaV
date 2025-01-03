// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/FuelComponent.h"

#include "Common/Craft/EngineComponent.h"
#include "Common/Part.h"

FuelState::FuelState() : TMap<FuelType, double>({
	{ FuelType::LiquidFuel, 5 },
	{ FuelType::Oxidizer, 5 },
	{ FuelType::SolidFuel, 5 },
	{ FuelType::Monopropellant, 5 },
}) {}

double FuelState::Drain(FuelState Fuel) {
	double Total = 0;
	for (auto& KVP : *this) {
		KVP.Value -= Fuel.FindChecked(KVP.Key);
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

FuelState FuelState::operator*(double Mult) {
	FuelState State;
	for (auto& KVP : *this) {
		State.FindChecked(KVP.Key) = KVP.Value * Mult;
	}
	return State;
}


FuelState FuelState::operator/(FuelState Other) {
	FuelState State;
	for (auto& KVP : *this) {
		double Div = Other.FindChecked(KVP.Key);
		State.FindChecked(KVP.Key) = Div != 0 ? KVP.Value / Div : 0;
	}
	return State;
}

double FuelState::CanDrain(FuelState Fuel) {
	double MaxDrain = 1;
	for (auto& KVP : *this) {
		MaxDrain = FMath::Min(MaxDrain, KVP.Value / Fuel.FindChecked(KVP.Key));
	}
	return MaxDrain;
}

void FuelState::FromJson(TSharedPtr<FJsonObject> Json) {

	if (Json->HasTypedField(TEXT("liquid_fuel"), EJson::Number)) {
		this->FindChecked(FuelType::LiquidFuel) = Json->GetNumberField(TEXT("liquid_fuel"));
	}
	if (Json->HasTypedField(TEXT("oxidizer"), EJson::Number)) {
		this->FindChecked(FuelType::Oxidizer) = Json->GetNumberField(TEXT("oxidizer"));
	}
	if (Json->HasTypedField(TEXT("solid_fuel"), EJson::Number)) {
		this->FindChecked(FuelType::SolidFuel) = Json->GetNumberField(TEXT("solid_fuel"));
	}
	if (Json->HasTypedField(TEXT("monopropellant"), EJson::Number)) {
		this->FindChecked(FuelType::Monopropellant) = Json->GetNumberField(TEXT("monopropellant"));
	}
}

TSharedPtr<FJsonObject> FuelState::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	double Fuel;
	Fuel = this->FindChecked(FuelType::LiquidFuel);
	if (Fuel > 0) {
		Json->SetNumberField(TEXT("liquid_fuel"), Fuel);
	}
	Fuel = this->FindChecked(FuelType::Oxidizer);
	if (Fuel > 0) {
		Json->SetNumberField(TEXT("oxidizer"), Fuel);
	}
	Fuel = this->FindChecked(FuelType::SolidFuel);
	if (Fuel > 0) {
		Json->SetNumberField(TEXT("solid_fuel"), Fuel);
	}
	Fuel = this->FindChecked(FuelType::Monopropellant);
	if (Fuel > 0) {
		Json->SetNumberField(TEXT("monopropellant"), Fuel);
	}
	return Json;
}

// Sets default values for this component's properties
UFuelComponent::UFuelComponent() : Super()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_DuringPhysics; // can be during physics or post physics
	// ...
}

void UFuelComponent::FromJson(TSharedPtr<FJsonObject> Json) {
	Current.FromJson(Json->GetObjectField(TEXT("current")));
	Max.FromJson(Json->GetObjectField(TEXT("max")));
}

TSharedPtr<FJsonObject> UFuelComponent::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	Json->SetObjectField(TEXT("current"), Current.ToJson());
	Json->SetObjectField(TEXT("max"), Max.ToJson());

	return Json;
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

	if (!Part->PhysicsEnabled) {
		return;
	}
	// only tick for fuel components at the root of each fuel group, root if parent is null or a decoupler
	if (Part->Parent != nullptr && Part->Parent->Type != "decoupler") {
		return;
	}
	// gather fuel's for this group, we have to iterate through all fuels every tick anyway
	// so it's ok to just recursively find all children (probably)
	TArray<UPart*> Parts = { Part };
	TArray<UEngineComponent*> Engines;
	FuelState TotalCurrent;
	FuelState TotalMax;

	for (int i = 0; i < Parts.Num(); ++i) {
		for (auto Child : Parts[i]->Children) {
			if (Child->Type != "decoupler") {
				Parts.Add(Child);
			}
		}
		UFuelComponent* Fuel = Cast<UFuelComponent>(Parts[i]->GetComponent("fuel"));
		if (Fuel) {
			TotalCurrent.Fill(Fuel->Current);
			TotalMax.Fill(Fuel->Max);
		}
		UEngineComponent* Engine = Cast<UEngineComponent>(Parts[i]->GetComponent("engine"));
		if (Engine) {
			Engines.Add(Engine);
		}
	}
}

void UFuelComponent::TickPostPhysics(float DeltaTime) {
	// 

}

FuelState UFuelComponent::Total() {
	UPart* Top = Part;
	while (Top->Parent && Top->Parent->Type != "decopuler") {
		Top = Top->Parent;
	}

	return FuelState();
}

FuelState GetTotalFuel(UPart* Part) {
	FuelState TotalFuel;
	UPartComponent** PartComponent = Part->AdditionalComponents.Find("fuel");
	if (PartComponent) {
		UFuelComponent* FuelComponent = Cast<UFuelComponent>(*PartComponent);
		if (FuelComponent) {
			TotalFuel.Fill(FuelComponent->Current);
		}
	}
	for (UPart* Child : Part->Children) {
		if (Child->Type == "decoupler") {
			continue;
		}
		TotalFuel.Fill(GetTotalFuel(Child));
	}
	return TotalFuel;
}

UPart* GetTop(UPart* Part) {
	UPart* Top = Part;
	while (Top->Parent && Top->Parent->Type != "decoupler") {
		Top = Top->Parent;
	}
	return Top;
}

void temp() {
	TArray<UPart*> ActiveEngines;
	TMap<UPart*, TArray<UPart*>> FuelGroup;
	for (UPart* Engine : ActiveEngines) {
		UPart* Top = GetTop(Engine);
		if (!FuelGroup.Contains(Top)) {
			FuelGroup.Add(Top, TArray<UPart*>());
		}
		FuelGroup.FindChecked(Top).Add(Engine);
	}
	for (auto Group : FuelGroup) {
		Group.Key;
		
	}
}

