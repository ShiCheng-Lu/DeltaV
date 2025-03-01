// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransformGadget.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class AConstructionController;

UCLASS()
class DELTAV_API ATransformGadget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATransformGadget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	enum TransformMode {
		Translate,
		Rotate,
	};

	UStaticMeshComponent* TranslateX;
	UStaticMeshComponent* TranslateY;
	UStaticMeshComponent* TranslateZ;

	UStaticMeshComponent* RotateX;
	UStaticMeshComponent* RotateY;
	UStaticMeshComponent* RotateZ;

	UStaticMeshComponent* ScaleX;
	UStaticMeshComponent* ScaleY;
	UStaticMeshComponent* ScaleZ;

	USceneComponent* Selected = nullptr;

	AConstructionController* Controller;

	void SetupStaticMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh, UMaterialInterface* Material, FVector Rotation);

	void StartTracking();

	void StopTracking();

	void Select(USceneComponent* Object);

	FRay TranslateRay;
	FVector Offset;
	bool Active = false;

	FPlane RotationPlane;

};
