// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/MapViewPawn.h"

// Sets default values
AMapViewPawn::AMapViewPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMapViewPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMapViewPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMapViewPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

