// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UI/DragReorderList.h"

#include "Construction/UI/PartItem.h"
#include "Components/ListView.h"
#include "Common/UI/DragReorderListItem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UDragReorderList::UDragReorderList(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDragReorderList::NativeOnInitialized() {
	Super::NativeOnInitialized();

	ListView->ClearListItems();
	ListView->AddItem(UPartItemData::Create("cone"));
	ListView->AddItem(UPartItemData::Create("cylinder"));
	ListView->AddItem(UPartItemData::Create("decoupler"));
	ListView->AddItem(UPartItemData::Create("engine"));
	ListView->AddItem(UPartItemData::Create("wing"));
	ListView->AddItem(UPartItemData::Create("cockpit"));
	ListView->AddItem(UPartItemData::Create("leg"));
}

FReply UDragReorderList::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	return FReply::Unhandled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
}

void UDragReorderList::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) {
	// Grab the Item in the array
	FVector2f MousePosition = InMouseEvent.GetScreenSpacePosition();

	for (UUserWidget* Widget : ListView->GetDisplayedEntryWidgets()) {
		if (!Widget->GetTickSpaceGeometry().IsUnderLocation(MousePosition)) {
			continue;
		}

		// Widget is under the mouse;
		OutOperation = DragItem(Widget);

		FVector2d RenderOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		OutOperation->DefaultDragVisual->SetRenderTranslation(RenderOffset);
		OutOperation->Pivot = EDragPivot::MouseDown;
		OutOperation->Offset = FVector2d(-0.5, -0.5);

		FScriptDelegate OnDropDelegate;
		OnDropDelegate.BindUFunction(this, "OnItemDropped");
		// Because item doesn't implement on drag/drop completed, it actually triggers OnDragCancelled instead of OnDrop
		OutOperation->OnDragCancelled.Add(OnDropDelegate);

		return;
	}
}

void UDragReorderList::OnItemDropped(UDragDropOperation* Operation) {
	FGeometry DroppedGeometry = Operation->DefaultDragVisual->GetTickSpaceGeometry();
	float DroppedY = DroppedGeometry.GetAbsolutePosition().Y;

	UUserWidget* Previous = nullptr;
	for (UUserWidget* Widget : ListView->GetDisplayedEntryWidgets()) {
		if (Widget->GetTickSpaceGeometry().GetAbsolutePosition().Y > DroppedY) {
			DropItem(Previous, Operation);
			return;
		}
		Previous = Widget;
	}
}


UDragDropOperation* UDragReorderList::DragItem(UUserWidget* Widget) {
	UDragDropOperation* Operation = NewObject<UDragDropOperation>();

	UDragReorderListItem* NewWidget = CreateWidget<UDragReorderListItem>(this, UDragReorderListItem::BlueprintClass);
	Operation->DefaultDragVisual = NewWidget;
	Operation->Payload = ListView->GetListObjectFromEntry(*Widget);
	NewWidget->Init(Operation->Payload);

	ListView->RemoveItem(Operation->Payload);

	return Operation;
}

void UDragReorderList::DropItem(UUserWidget* After, UDragDropOperation* Operation) {
	TArray<UObject*> List = ListView->GetListItems();
	if (!After) { // insert into first position
		ListView->ClearListItems();
		ListView->AddItem(Operation->Payload);
		for (UObject* Item : List) {
			ListView->AddItem(Item);
		}
	}
	else { 
		UObject* AfterObject = ListView->GetListObjectFromEntry(*After);
		ListView->ClearListItems();
		for (UObject* Item : List) {
			ListView->AddItem(Item);
			if (Item == AfterObject) {
				ListView->AddItem(Operation->Payload);
			}
		}
	}
}
