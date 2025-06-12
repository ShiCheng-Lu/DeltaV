// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Craft/StageManager.h"

#include "Common/Craft.h"
#include "Common/Part.h"

// Sets default values for this component's properties
UStageManager::UStageManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	Active = CreateDefaultSubobject<UStage>("ActiveStage");
}


// Called when the game starts
void UStageManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	Craft = Cast<ACraft>(GetOwner());
}
/*
void UStageManager::UpdateStages() {

}
*/
// Called every frame
void UStageManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...



}

void UStageManager::FromJson(TArray<TSharedPtr<FJsonValue>> Json) {
	for (TSharedPtr<FJsonValue> StageJson : Json) {
		UStage* Stage = NewObject<UStage>();
		for (auto& PartJson : StageJson->AsObject()->GetArrayField(TEXT("parts"))) {
			FString PartId = PartJson->AsString();
			UPart** Part = Craft->Parts.Find(PartId);
			if (Part) {
				Stage->Parts.Add(*Part);
			}
		}
		Stages.Push(Stage);
	}

}

TArray<TSharedPtr<FJsonValue>> UStageManager::ToJson() {
	TArray<TSharedPtr<FJsonValue>> Json;
	for (UStage* Stage : Stages) {
		TSharedPtr<FJsonObject> StageJson = MakeShareable(new FJsonObject());

		TArray<TSharedPtr<FJsonValue>> Array;
		for (UPart* Part : Stage->Parts) {
			Array.Add(MakeShareable(new FJsonValueString(Part->Id)));
		}
		StageJson->SetArrayField("parts", Array);
		Json.Push(MakeShareable(new FJsonValueObject(StageJson)));
	}
	return Json;
}

TArray<ACraft*> UStageManager::Stage() {
	if (Stages.Num() == 0) {
		return {}; // empty list
	}
	UStage* Stage = Stages.Last();

	TArray<ACraft*> Detached;
	for (UPart* Part : Stage->Parts) {
		if (Part->Type == "decoupler") {
			ACraft* NewCraft = GetWorld()->SpawnActor<ACraft>();
			Craft->DetachPart(Part, NewCraft);
			Detached.Add(NewCraft);
		}
		if (Part->Type == "engine") {
			Active->Parts.Add(Part);
		}
	}
	return Detached;
}
