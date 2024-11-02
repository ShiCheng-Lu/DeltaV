// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/GenerationMode.h"
#include "Generation/GenerationController.h"

AGenerationMode::AGenerationMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	PlayerControllerClass = AGenerationController::StaticClass();
}
