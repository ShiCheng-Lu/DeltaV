// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Craft.h"
#include "ConstructionCraft.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API AConstructionCraft : public ACraft
{
	GENERATED_BODY()

public:
	AConstructionCraft(const FObjectInitializer& ObjectInitializer);

	void FromJson(TSharedPtr<FJsonObject> CraftJson);

	void SetAttachmentNodeVisibility(bool visibility);

	void AttachPart(AConstructionCraft* SourceCraft, UPart* AttachToPart);

	void DetachPart(UPart* Part, AConstructionCraft* NewCraft);
};
