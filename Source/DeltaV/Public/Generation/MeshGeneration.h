// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UDynamicMesh.h"
#include "MeshGeneration.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UMeshGeneration : public UDynamicMeshGenerator
{
	GENERATED_BODY()

	using FIndex3i = UE::Geometry::FIndex3i;

public:
	TMap<TPair<int, int>, int> MidpointMap;
	
	double Radius = 100;

	UPROPERTY()
	TMap<int, double> UpliftMap;

	UPROPERTY()
	TMap<int, double> HeightMap;

public:
	UMeshGeneration();
	~UMeshGeneration();

	int GetMidpoint(FDynamicMesh3& MeshInOut, int a, int b);

	void Subdivide(FDynamicMesh3& MeshInOut, FIndex3i Face, int Depth = 0);

	void Initialize(FDynamicMesh3& MeshInOut);

	void Iterate(FDynamicMesh3& MeshInOut);

	void Generate(FDynamicMesh3& MeshInOut);

	void GenerateIsoSphere(FDynamicMesh3& MeshInOut, int Subdivisions);
};
