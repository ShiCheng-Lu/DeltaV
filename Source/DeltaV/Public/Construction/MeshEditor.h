// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UDynamicMesh.h"
#include "DynamicMesh/DynamicMesh3.h"

/**
 * 
 */
class DELTAV_API MeshEditor
{
public:
	MeshEditor();
	~MeshEditor();

	void Transform(UE::Geometry::FDynamicMesh3& input, UE::Geometry::FDynamicMesh3& output, TArray<FVector>& input_bound, TArray<FVector>& output_bound);
};
