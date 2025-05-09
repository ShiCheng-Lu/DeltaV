// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/WheelComponent.h"

#include "Common/Part.h"
#include "Common/JsonUtil.h"
#include "Common/Craft.h"
#include "Simulation/SimulationController.h"
#include "Common/AssetLibrary.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UWheelComponent::UWheelComponent() {
	// create a physics contraint


	UE_LOG(LogTemp, Warning, TEXT("Wheel created"));
}

// Called when the game starts
void UWheelComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	UE_LOG(LogTemp, Warning, TEXT("Wheel begin play"));
}


// Called every frame
void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWheelComponent::SetPhysicsEnabled(bool bSimulate) {
	if (bSimulate) {
		//Mesh->DetachFromComponent(DetachmentRule);
		//Physics->SetConstrainedComponents(Part, "", Mesh, "");
		//Mesh->SetSimulatePhysics(true);
	}
	else {

	}
}

void UWheelComponent::FromJson(TSharedPtr<FJsonObject> Json) {
	Offset = JsonUtil::Vector(Json, TEXT("offset"));
}

TSharedPtr<FJsonObject> UWheelComponent::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	JsonUtil::Vector(Json, TEXT("offset"), Offset);

	return Json;
}
