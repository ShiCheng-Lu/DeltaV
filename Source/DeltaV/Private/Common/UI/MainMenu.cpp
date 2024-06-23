// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UI/MainMenu.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

#include "Common/MainGameInstance.h"

void UMainMenu::NativeOnInitialized() {
	Super::NativeOnInitialized();

	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
	GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeUIOnly());

	FScriptDelegate PlayButtonClicked;
	PlayButtonClicked.BindUFunction(this, "PlayButtonClicked");
	PlayButton->OnClicked.Add(PlayButtonClicked);

	FScriptDelegate SettingsButtonClicked;
	SettingsButtonClicked.BindUFunction(this, "SettingsButtonClicked");
	SettingsButton->OnClicked.Add(SettingsButtonClicked);

	FScriptDelegate QuitButtonClicked;
	QuitButtonClicked.BindUFunction(this, "QuitButtonClicked");
	QuitButton->OnClicked.Add(QuitButtonClicked);
}

void UMainMenu::PlayButtonClicked() {
	Cast<UMainGameInstance>(GetGameInstance())->CraftPath = "";
	UGameplayStatics::OpenLevel(GetWorld(), "Construction");
}

void UMainMenu::SettingsButtonClicked() {
	UE_LOG(LogTemp, Warning, TEXT("Settings button pressed"));
	UGameplayStatics::OpenLevel(GetWorld(), "Simulation");
}

void UMainMenu::QuitButtonClicked() {
	UE_LOG(LogTemp, Warning, TEXT("Quit button pressed"));
	RequestEngineExit("Quit Button Clicked");
}
