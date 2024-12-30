// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UI/StagesList.h"

#include "Common/Craft.h"
#include "Construction/UI/StageGroup.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/ListView.h"
#include "Components/TreeView.h"

void UStagesList::NativeOnInitialized() {
	Super::NativeOnInitialized();

	TreeView = Cast<UTreeView>(ListView);
}

TPair<UStage*, int> FindStageOfPart(TArray<UStage*>& Stages, UPart* Part) {
	for (UStage* Stage : Stages) {
		int Index = Stage->Parts.Find(Part);
		if (Index != INDEX_NONE) {
			return { Stage, Index };
		}
	}
	return { nullptr, INDEX_NONE };
}

UDragDropOperation* UStagesList::DragItem(UUserWidget* Widget) {
	UDragDropOperation* Operation = NewObject<UDragDropOperation>();

	UStageGroup* NewWidget = CreateWidget<UStageGroup>(this, UStageGroup::BlueprintClass);

	UStageGroup* StageItem = Cast<UStageGroup>(Widget);


	UObject* Object = TreeView->GetListObjectFromEntry(*Widget);

	Operation->DefaultDragVisual = NewWidget;

	Operation->Payload = Object;
	NewWidget->Init(Operation->Payload);

	UPart* Part = Cast<UPart>(Object);
	if (Part) {
		for (UStage* Stage : Craft->Stages) {
			if (Stage->Parts.Remove(Part) > 0) {
				break;
			}
		}
	}
	UStage* Stage = Cast<UStage>(Object);
	if (Stage) {
		ListView->RemoveItem(Object);
	}
	Reload();

	return Operation;
}

void UStagesList::DropItem(UUserWidget* After, UDragDropOperation* Operation) {
	UStageGroup* StageGroup = Cast<UStageGroup>(After);
	if (StageGroup == nullptr) {
		// dropped above the first stage
		return;
	}

	UStage* AfterStage = Cast<UStage>(StageGroup->Payload);
	UPart* AfterPart = Cast<UPart>(StageGroup->Payload);

	UStage* Stage = Cast<UStage>(Operation->Payload);
	UPart* Part = Cast<UPart>(Operation->Payload);

	int Index = INDEX_NONE;

	if (Part) {
		if (AfterPart) {
			Tie(AfterStage, Index) = FindStageOfPart(Craft->Stages, AfterPart);
			UE_LOG(LogTemp, Warning, TEXT("Dropped after part %d"), Index);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Dropped after Stage"));
		}
		AfterStage->Parts.Insert(Part, Index + 1);
	}
	else if (Stage) {
		/*
		if (AfterPart) {
			Tie(AfterStage, Index) = FindStageOfPart(Craft->Stages, AfterPart);
		}
		Index = Craft->Stages.Find(AfterStage);
		Craft->Stages.Insert(Stage, Index);
		*/
	}
	Reload();
}

TArray<UObject*> UStagesList::GetStageParts(UObject* Object) {
	UStage* Stage = Cast<UStage>(Object);
	TArray<UObject*> Parts;
	if (Stage == nullptr) {
		return Parts;
	}
	for (UPart* Part : Stage->Parts) {	
		Parts.Add(Part);
	}
	return Parts;
}

void UStagesList::Reload() {
	TreeView->ClearListItems();
	TArray<UObject*> Stages;
	for (UStage* Stage : Craft->Stages) {
		Stages.Add(Stage);
	}
	TreeView->SetListItems(Stages);
	
	for (UStage* Stage : Craft->Stages) {
		TreeView->SetItemExpansion(Stage, !HiddenStages.Contains(Stage));
	}
}

void UStagesList::ClickItem(UUserWidget* Widget) {
	UObject* Object = TreeView->GetListObjectFromEntry(*Widget);
	UStage* Stage = Cast<UStage>(Object);
	if (Stage) {
		if (HiddenStages.Remove(Stage) == 0 && Stage->Parts.Num() > 0) {
			// Hide stage
			TreeView->SetItemExpansion(Object, false);
			HiddenStages.Add(Stage);
		}
		else {
			TreeView->SetItemExpansion(Object, true);
		}
	}
}

void UStagesList::AddStage(UStage* At, int Offset) {
	int Index = Craft->Stages.Find(At);
	if (Index == INDEX_NONE) {
		return;
	}
	UStage* NewStage = NewObject<UStage>();
	Craft->Stages.Insert(NewStage, Index + Offset);
	Reload();
}
