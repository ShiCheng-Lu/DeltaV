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

	FString Id;
	TArray<UPart*> Children;
	UPart* Parent;

	TArray<UAttachmentNode*> AttachmentNodes;

	UPhysicsConstraintComponent* Physics;
	
	TObjectPtr<class UActorComponent> Component;

	int AttachedAt = -1; // parent location
	// AttachLocation = Physics.RelativeLocation

	UPart(const FObjectInitializer &ObjectInitializer);

	void Initialize(FString Id, TSharedPtr<FJsonObject> Structure, TSharedPtr<FJsonObject> Json);
	
	void SetAttachmentNodeVisibility(bool visibility);

	void SetParent(UPart* Part);

	/*
	Json serialization
	{
	  "type": <part type>,
	  "location": [x, y, z],
	  "rotation": [x, y, z, w],
	  "scale": [x, y, z],
	  "attached_at": <int>
	  "attach_location": [rel_x, rel_y, rel_z]

	  // optional?
	  "dynamic_warp": [matrix of some sort]
	}
	*/
	void FromJson(TSharedPtr<FJsonObject> Json);
	TSharedPtr<FJsonObject> ToJson();

	FString Type;

	template<class T>
	TObjectPtr<T> GetComponent() {
		return Cast<T>(Component);
	}

	bool PhysicsEnabled;
	void SetPhysicsEnabled(bool bSimulate);

	void Attach();
	void Detach();
};
