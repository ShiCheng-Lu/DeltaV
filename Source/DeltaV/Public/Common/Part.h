// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Part.generated.h"

class UAttachmentNode;
class UPhysicsConstraintComponent;
class UPartComponent;
class ACraft;
class UDynamicMesh;
struct FMeshDescription;

/**
 * 
 */
UCLASS()
class UPart : public UActorComponent
{
	GENERATED_BODY()
	
public:
	enum EMeshType {
		STATIC_MESH,
		SKELETAL_MESH,
	};

	EMeshType MeshType;

	FString Id;
	ACraft* Craft;
	// probably don't need these two, handled by component hierachy
	TArray<UPart*> Children;
	UPart* Parent;

	TArray<UAttachmentNode*> AttachmentNodes;
	TMap<FString, UPartComponent*> AdditionalComponents;

	UPhysicsConstraintComponent* Physics;

	TObjectPtr<UMeshComponent> Mesh;
	FName Bone;

	int AttachedAt = -1; // parent location
	// AttachLocation = Physics.RelativeLocation

	UPart(const FObjectInitializer &ObjectInitializer);

	virtual void BeginPlay() override;

	void SetAttachmentNodeVisibility(bool visibility);

	void SetParent(UPart* Part);

	/*
	Json serialization
	{
	  "type": <part type>, 
	  "location": [x, y, z],
	  "rotation": [x, y, z, w],
	  "scale": [x, y, z],
	  "attached_at": <string>,
	  "attach_location": [rel_x, rel_y, rel_z],

	  // maybe optional?
	  "dynamic_warp": [matrix of some sort]
	}
	*/
	void FromJson(TSharedPtr<FJsonObject> Json);
	TSharedPtr<FJsonObject> ToJson();

	FString Type;

	template<typename T>
	T* GetComponent(FString Name) {
		for (auto Component : AdditionalComponents) {
			T* CastComponent = Cast<T>(Component.Value);
			if (CastComponent) {
				return CastComponent;
			}
		}
		return nullptr;
	}

	bool PhysicsEnabled;
	void SetPhysicsEnabled(bool bSimulate);

	void Attach();
	void Detach();

	FMeshDescription* CopyMeshToDynamicMesh(TObjectPtr<UDynamicMesh> DynamicMesh, int LOD = 0);
};
