// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DragReorderList.h"
#include "StagesList.generated.h"


class UTreeView;
class UDragDropOperation;

class ACraft;
class UStage;
class UStageManager;

/**
 * 
 */
UCLASS()
class DELTAV_API UStagesList : public UDragReorderList
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

public:
	UStageManager* Manager;

	UTreeView* TreeView;

	TSet<UStage*> HiddenStages;

	virtual UDragDropOperation* DragItem(UUserWidget* Widget) override;

	virtual void DropItem(UUserWidget* After, UDragDropOperation* Operation) override;

	virtual void ClickItem(UUserWidget* Widget) override;

	UFUNCTION(BlueprintCallable)
	TArray<UObject*> GetStageParts(UObject* Object);

	void Reload();

	void AddStage(UStage* At, int Offset);
};
