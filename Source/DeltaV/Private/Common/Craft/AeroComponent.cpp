// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/AeroComponent.h"

#include "Common/MainGameInstance.h"
#include "Common/Part.h"
#include "Common/Craft.h"

// Sets default values for this component's properties
UAeroComponent::UAeroComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAeroComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Part->PhysicsEnabled) {
		return;
	}
	// aerodynamics???

	// find air deflection based on velocity
	FVector AirVelocity = Part->Mesh->GetComponentVelocity();
	double Airspeed = AirVelocity.Length();
	if (Airspeed < 1e-8) {
		return;
	}
	FVector VelocityDirection = AirVelocity / Airspeed;
	FQuat PartRotation = Part->Mesh->GetComponentQuat();
	// approximate drag and lift coefficients, 
	// https://aviation.stackexchange.com/questions/64490/is-there-a-simple-relationship-between-angle-of-attack-and-lift-coefficient
	double RelativeVelocityZ = PartRotation.Inverse().RotateVector(VelocityDirection).Z;
	double AoA = FMath::Asin(FMath::Abs(RelativeVelocityZ));
	bool Stall = (AoA > (PI / 8)) && (AoA < (PI / 8 * 7));
	double LiftCoefficient = Stall ? FMath::Sin(2 * AoA) : FMath::Sin(6 * AoA);
	double DragCoefficient = 1 - FMath::Cos(2 * AoA);
	// multiply by air speed, invert lift if direction is backward
	LiftCoefficient *= (RelativeVelocityZ < 0) ? Airspeed * Airspeed : -Airspeed * Airspeed;
	DragCoefficient *= Airspeed * Airspeed;
	
	FVector LiftDirection = PartRotation.RotateVector(FVector::UpVector);
	LiftDirection -= LiftDirection.ProjectOnToNormal(VelocityDirection);
	LiftDirection.Normalize();

	FVector AeroForce = (Lift * LiftCoefficient * LiftDirection) - (Drag * DragCoefficient * VelocityDirection);
	Part->Mesh->AddForce(AeroForce * DeltaTime);

	if (true) { // Enable show aero arrows
		UWorld* World = GetWorld();
		UMainGameInstance* GameInstance = Cast<UMainGameInstance>(World->GetGameInstance());
		if (GameInstance && GameInstance->AeroArrows) {
			UE_LOG(LogTemp, Warning, TEXT("aero arrows %d"), GameInstance->AeroArrows);
			FVector Start = Part->Mesh->GetComponentLocation() + PartRotation.RotateVector(FVector(100, 0, 0));
			DrawDebugDirectionalArrow(World, Start, Start + 0.01 * Lift * LiftCoefficient * LiftDirection, 10, FColor::Green, false, -1, 1);
			DrawDebugDirectionalArrow(World, Start, Start - 0.01 * Drag * DragCoefficient * VelocityDirection, 10, FColor::Red, false, -1, 1);
		}
	}
}

void UAeroComponent::FromJson(TSharedPtr<FJsonObject> Json) {
	Lift = Json->GetNumberField(TEXT("lift"));
	Drag = Json->GetNumberField(TEXT("drag"));
}

TSharedPtr<FJsonObject> UAeroComponent::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	Json->SetNumberField(TEXT("lift"), Lift);
	Json->SetNumberField(TEXT("drag"), Drag);

	return Json;
}
