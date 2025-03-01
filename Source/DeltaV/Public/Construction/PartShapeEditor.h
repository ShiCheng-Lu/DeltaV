// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UDynamicMesh.h"
#include "DynamicMeshActor.h"
#include "PartShapeEditor.generated.h"

class UPart;
class UStaticMeshComponent;
class AConstructionController;

UCLASS()
class DELTAV_API APartShapeEditor : public ADynamicMeshActor {
	GENERATED_BODY()

public:
	APartShapeEditor(const FObjectInitializer& ObjectInitializer);

	void SetPart(UPart* Part);

	void SetController(AConstructionController* Controller);

	AConstructionController* Controller;

	TArray<TObjectPtr<UStaticMeshComponent>> Corners;
	// the corner that has been selected
	UStaticMeshComponent* Selected;

	UPart* SelectedPart;

	void Pressed(FKey Key);

	void Released(FKey Key);

	virtual void Tick(float DeltaTime) override;

	void UpdateMesh(FDynamicMesh3& MeshInOut);

	UDynamicMeshComponent* Mesh;

	bool Active;
};
