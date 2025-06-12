// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/EngineComponent.h"

#include "Common/Part.h"
#include "Common/JsonUtil.h"
#include "Common/Craft.h"
#include "Simulation/SimulationController.h"

// Sets default values for this component's properties
UEngineComponent::UEngineComponent() : Super(),
	PostPhysicsTick(ETickingGroup::TG_PostPhysics, &UEngineComponent::PostPhysics),
	DuringPhysicsTick(ETickingGroup::TG_DuringPhysics, &UEngineComponent::DuringPhysics)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	
}

void UEngineComponent::PostPhysics(float a) {
	UE_LOG(LogTemp, Warning, TEXT("Post Physics Tick"));
}
void UEngineComponent::DuringPhysics(float a) {
	UE_LOG(LogTemp, Warning, TEXT("During Physics Tick"));
}

// Called when the game starts
void UEngineComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	UPart* SearchRoot = Part;
	while (SearchRoot->Parent && SearchRoot->Parent->Type != "decoupler") {
		SearchRoot = SearchRoot->Parent;
	}

}


// Called every frame
void UEngineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	// do engine stuff, get input, this component should also handle particles from the engine
	
	if (!Part->PhysicsEnabled) {
		return;
	}

	// FVector ThrustVector = Part->GetComponentQuat().RotateVector();
	Part->Mesh->AddForceAtLocationLocal(FVector(1000000 * Thrust, 0, 0), PivotOffset);
	if (Thrust > 0.01) {
		UWorld* World = GetWorld();
		FVector Start = Part->Mesh->GetComponentLocation();
		FVector Throttle = Part->Mesh->GetComponentQuat().RotateVector(FVector(1000000 * Thrust, 0, 0));
		DrawDebugDirectionalArrow(World, Start, Start + Throttle, 10, FColor::Yellow, false, -1, 1);
	}
	return;

	// temp
	if (Part->Parent && Part->Parent->Type == "cylinder") {
		// ACraft* Craft = Cast<ACraft>(GetOwner());
		// ASimulationController* Controller = Cast<ASimulationController>(Craft->Controller);

		//UFuelComponent* Fuel = Part->Parent->GetComponent<UFuelComponent>("fuel");
		//FuelState TickDrain = Drain * DeltaTime;
		
		// double Throttle = FMath::Min(Controller->ThrottleValue, Fuel->Current.CanDrain(TickDrain));
		//double ResThrust = Throttle * DeltaTime * (1 << 25);

		// FVector ThrustVector = Part->GetComponentQuat().RotateVector();
		//Part->Mesh->AddForceAtLocationLocal(FVector(Throttle, 0, 0), PivotOffset);
		//Fuel->Current.Drain(TickDrain);
	}
}

void UEngineComponent::FromJson(TSharedPtr<FJsonObject> Json) {
	PivotOffset = JsonUtil::Vector(Json, TEXT("pivot_offset"));

	PivotLimit = Json->GetNumberField(TEXT("pivot_limit"));
	Thrust = Json->GetNumberField(TEXT("pivot_limit"));
	Drain.FromJson(Json->GetObjectField(TEXT("fuel_drain")));
}

TSharedPtr<FJsonObject> UEngineComponent::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	JsonUtil::Vector(Json, TEXT("pivot_offset"), PivotOffset);
	Json->SetNumberField(TEXT("pivot_limit"), PivotLimit);
	Json->SetNumberField(TEXT("thrust"), Thrust);
	Json->SetObjectField(TEXT("fuel_drain"), Drain.ToJson());

	return Json;
}
