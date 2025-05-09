// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/MainGameInstance.h"

#include "Kismet/GameplayStatics.h"

void UMainGameInstance::SaveSettings() {

	// Settings null check is done in SaveGameToSlot
	UGameplayStatics::SaveGameToSlot(Settings, FString("Settings"), 0);
}

void UMainGameInstance::LoadSettings() {

	Settings = UGameplayStatics::LoadGameFromSlot(FString("Settings"), 0);
}


void UMainGameInstance::GetSettings() {
	
}
