// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CelestialBody.generated.h"

UCLASS()
class DELTAV_API ACelestialBody : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACelestialBody(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USkyAtmosphereComponent> atmosphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UStaticMeshComponent> mesh;

	double angle;
	FVector axis_of_rotation;
	double angle_to_parent;

	TObjectPtr<ACelestialBody> parent;
	double mass;
	double mu; // gravitational parameter

	// condition
	double radius_of_influence;

	UPROPERTY(EditAnywhere)
	double atmosphere_height = 100;

	UPROPERTY(EditAnywhere)
	double radius = 100;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
