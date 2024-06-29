// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UDynamicMesh.h"
#include "PartShapeEditor.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UPartShapeEditor : public UDynamicMeshGenerator
{
	GENERATED_BODY()
	
public:

	virtual void Generate(FDynamicMesh3& MeshInOut);

	FDynamicMesh3 Initialize(UStaticMesh* StaticMesh);

	void SetTargetBounds(FVector in_target_bounds[8]);
public:
	TObjectPtr<class UStaticMesh> BaseMesh;

	FVector BoundPosition; // negative extrema of the bound
	FVector BoundSize; // size of the bound
	FVector TargetBound[8];
};
