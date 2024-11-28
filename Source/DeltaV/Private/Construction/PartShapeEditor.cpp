// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/PartShapeEditor.h"

#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "MeshTypes.h"

FDynamicMesh3 UPartShapeEditor::Initialize(UStaticMesh* StaticMesh) {
	BaseMesh = StaticMesh;

	FBoxSphereBounds bounds = BaseMesh->GetBounds();
	BoundPosition = bounds.GetBoxExtrema(0);
	BoundSize = bounds.GetBox().GetSize();

	FDynamicMesh3 DynamicMesh;

	DynamicMesh.Clear();

	/*
	FMeshDescription* MeshDescription = BaseMesh->GetMeshDescription(0);
	FVertexArray vertices = MeshDescription->Vertices();
	for (FVertexID vertexId : vertices.GetElementIDs()) {
		FVector position = FVector(MeshDescription->GetVertexPosition(vertexId));
		DynamicMesh.InsertVertex(vertexId, position);
	}
	FTriangleArray triangles = MeshDescription->Triangles();
	for (FTriangleID triangleId : triangles.GetElementIDs()) {
		TArrayView<const FVertexID> vertexIds = MeshDescription->GetTriangleVertices(triangleId);
		UE::Geometry::FIndex3i s = UE::Geometry::FIndex3i(vertexIds[0], vertexIds[1], vertexIds[2]);
		DynamicMesh.InsertTriangle(triangleId, s);
	}
	*/
	return DynamicMesh;
}

void UPartShapeEditor::SetTargetBounds(FVector InTargetBound[8]) {
	memcpy(TargetBound, InTargetBound, sizeof(TargetBound));
}

void UPartShapeEditor::Generate(FDynamicMesh3& MeshInOut) {
	// Vertices[0] = TVector<T>(Min);
	// Vertices[1] = TVector<T>(Min.X, Min.Y, Max.Z);
	// Vertices[2] = TVector<T>(Min.X, Max.Y, Min.Z);
	// Vertices[3] = TVector<T>(Max.X, Min.Y, Min.Z);
	// Vertices[4] = TVector<T>(Max.X, Max.Y, Min.Z);
	// Vertices[5] = TVector<T>(Max.X, Min.Y, Max.Z);
	// Vertices[6] = TVector<T>(Min.X, Max.Y, Max.Z);
	// Vertices[7] = TVector<T>(Max);

	/*
	FMeshDescription* MeshDescription = BaseMesh->GetMeshDescription(0);
	FVertexArray vertices = MeshDescription->Vertices();
	for (auto vertexId : vertices.GetElementIDs()) {	
		FVector position = FVector(MeshDescription->GetVertexPosition(vertexId));

		position.X *= 0.9;

		FVector relative = (position - BoundPosition) / BoundSize;

		// position after linearly interpolating Z position
		FVector min_x_min_y = TargetBound[0] * (1 - relative.Z) + TargetBound[1] * relative.Z;
		FVector min_x_max_y = TargetBound[2] * (1 - relative.Z) + TargetBound[6] * relative.Z;
		FVector max_x_min_y = TargetBound[3] * (1 - relative.Z) + TargetBound[5] * relative.Z;
		FVector max_x_max_y = TargetBound[4] * (1 - relative.Z) + TargetBound[7] * relative.Z;
		// position after linearly interpolating Y position
		FVector min_x = min_x_min_y * (1 - relative.Y) + min_x_max_y * relative.Y;
		FVector max_x = max_x_min_y * (1 - relative.Y) + max_x_max_y * relative.Y;
		// position after linearly interpolating X position
		FVector transformed_position = min_x * (1 - relative.X) + max_x * relative.X;

		MeshInOut.SetVertex(vertexId, transformed_position);
	}
	*/
}
