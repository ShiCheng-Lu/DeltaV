// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DragReorderList.generated.h"

class UListView;

/**
 * 
 */
UCLASS()
class DELTAV_API UDragReorderList : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

public:
	UDragReorderList(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UListView* ListView;

	UFUNCTION(BlueprintCallable)
	void OnItemDropped(UDragDropOperation* Operation);

	// Should be overriden by child class
	virtual UDragDropOperation* DragItem(UUserWidget* Widget);

	// Should be overriden by child class
	virtual void DropItem(UUserWidget* After, UDragDropOperation* Operation);

	// Should be overriden by child class
	virtual void ClickItem(UUserWidget* Widget);
};
