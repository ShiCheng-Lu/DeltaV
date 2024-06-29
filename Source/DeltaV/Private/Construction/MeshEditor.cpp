// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/MeshEditor.h"

#include "Components/DynamicMeshComponent.h"
#include "DynamicMeshToMeshDescription.h"

using namespace UE::Geometry;

MeshEditor::MeshEditor()
{
}

MeshEditor::~MeshEditor()
{
}

void MeshEditor::Transform(FDynamicMesh3& input, FDynamicMesh3& output, TArray<FVector>& input_bound, TArray<FVector>& output_bound) {
	FDynamicMesh3 DynamicMesh;
	
	UDynamicMeshComponent component;
	FDynamicMeshToMeshDescription Convertor;
	

	// FPositionVertexBuffer& PositionVertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
	// FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResources.VertexBuffers.StaticMeshVertexBuffer;
	// FRawStaticIndexBuffer& IndexBuffer = LODResources.IndexBuffer;
	// 
	// // Add vertices
	// for (uint32 VertexIndex = 0; VertexIndex < PositionVertexBuffer.GetNumVertices(); ++VertexIndex)
	// {
	// 	FVector Position = FVector(PositionVertexBuffer.VertexPosition(VertexIndex));
	// 	DynamicMesh.AppendVertex(Transform.TransformPosition(Position));
	// }
	// 
	// // Add triangles
	// const int32 NumIndices = IndexBuffer.GetNumIndices();
	// for (int32 Index = 0; Index < NumIndices; Index += 3)
	// {
	// 	int32 Index0 = IndexBuffer.GetIndex(Index);
	// 	int32 Index1 = IndexBuffer.GetIndex(Index + 1);
	// 	int32 Index2 = IndexBuffer.GetIndex(Index + 2);
	// 
	// 	DynamicMesh.AppendTriangle(Index0, Index1, Index2);
	// }
	// 
	// DynamicMesh.
}
