// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/PartComponent.h"

#include "Common/Part.h"

// Sets default values for this component's properties
UPartComponent::UPartComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	Part = GetTypedOuter<UPart>();
}


// Called when the game starts
void UPartComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	UE_LOG(LogTemp, Warning, TEXT("Begin Play"));
}


// Called every frame
void UPartComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
