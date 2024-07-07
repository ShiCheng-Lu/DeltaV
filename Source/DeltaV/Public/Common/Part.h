// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Part.generated.h"

class UAttachmentNode;
class UPhysicsConstraintComponent;

/**
 * 
 */
UCLASS()
class UPart : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	virtual void SetSimulatePhysics(bool bSimulate) override;

	TSharedPtr<FJsonObject> Json;
	TSharedPtr<FJsonObject> Structure;

	TSharedPtr<FJsonObject> definition;

	FString Id;
	TArray<UPart*> Children;
	UPart* Parent;

	TArray<UAttachmentNode*> AttachmentNodes;

	UPhysicsConstraintComponent* Physics;
	
	UPart(const FObjectInitializer &ObjectInitializer);

	void Initialize(FString Id, TSharedPtr<FJsonObject> Structure, TSharedPtr<FJsonObject> Json);
	
	void SetAttachmentNodeVisibility(bool visibility);

	void SetParent(UPart* Part);
};
