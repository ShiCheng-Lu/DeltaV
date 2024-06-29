// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Navball.generated.h"

class ACraft;

UCLASS()
class DELTAV_API ANavball : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANavball();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USceneCaptureComponent2D> Camera;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextureRenderTarget2D> RenderTarget;

	TObjectPtr<class ACraft> Target;
	FVector Origin;

	void SetTarget(ACraft* Craft, FVector PlanetCenter);
};
