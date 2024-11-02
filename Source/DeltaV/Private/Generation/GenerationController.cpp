// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/GenerationController.h"
#include "Generation/MeshGeneration.h"
#include "DynamicMeshActor.h"

void AGenerationController::BeginPlay() {
	
}

void AGenerationController::Tick(float DeltaSeconds) {
	if (!started) {
		started = true;

		ShapeEditor = NewObject<UMeshGeneration>(this);

		Mesh = FDynamicMesh3();
		Mesh.Clear();

		UDynamicMesh* DynamicMesh = NewObject<UDynamicMesh>();

		ADynamicMeshActor* NewActor = GetWorld()->SpawnActor<ADynamicMeshActor>();
		NewActor->SetActorLocation(FVector(0, 0, 0));
		NewActor->GetDynamicMeshComponent()->SetDynamicMesh(DynamicMesh);

		// DynamicMesh->bEnableMeshGenerator = true;
		// DynamicMesh->SetMeshGenerator(ShapeEditor);
		ShapeEditor->Initialize(Mesh);
		ShapeEditor->Iterate(Mesh);
		DynamicMesh->SetMesh(Mesh);
	}
	// DrawDebugDirectionalArrow(GetWorld(), FVector(0, 0, 0), FVector(0.0001, 0, 0), 3, FColor::Red);
}
