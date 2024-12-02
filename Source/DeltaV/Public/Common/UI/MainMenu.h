// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

public:

	UFUNCTION(BlueprintCallable)
	void PlayButtonClicked();

	UFUNCTION(BlueprintCallable)
	void SettingsButtonClicked();

	UFUNCTION(BlueprintCallable)
	void QuitButtonClicked();
};
