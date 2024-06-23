// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "AttachmentNode.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UAttachmentNode : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this component's properties
	UAttachmentNode(const FObjectInitializer& ObjectInitializer);

	void Initialize(FVector& RelativeLocation);
};
