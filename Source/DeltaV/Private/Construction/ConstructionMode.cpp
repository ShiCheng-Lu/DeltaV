// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionMode.h"

#include "Construction/ConstructionPawn.h"
#include "Construction/ConstructionController.h"

AConstructionMode::AConstructionMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	DefaultPawnClass = AConstructionPawn::StaticClass();
	PlayerControllerClass = AConstructionController::StaticClass();
}
