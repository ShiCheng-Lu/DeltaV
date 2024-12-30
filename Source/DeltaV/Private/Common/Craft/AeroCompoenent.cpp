// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/AeroCompoenent.h"

// Sets default values for this component's properties
UAeroCompoenent::UAeroCompoenent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAeroCompoenent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAeroCompoenent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// aerodynamics???

	// find air deflection based on velocity
	
	AActor* Actor = Cast<AActor>(GetOutermostObject());
	if (!Actor) {
		return;
	}
	FVector Velocity = Actor->GetVelocity();


}


void UAeroCompoenent::FromJson(TSharedPtr<FJsonObject> Json) {
}

TSharedPtr<FJsonObject> UAeroCompoenent::ToJson() {
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	return Json;
}
