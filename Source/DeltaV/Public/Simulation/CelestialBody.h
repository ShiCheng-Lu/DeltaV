// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/MultiTickActor.h"
#include "CelestialBody.generated.h"

UCLASS()
class DELTAV_API ACelestialBody : public AActor, public MultiTickActor
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
	TObjectPtr<class USkyAtmosphereComponent> Atmosphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TObjectPtr<class UOrbitComponent> Orbit;

	// UPROPERTY()
	TObjectPtr<ACelestialBody> Parent;

	FVector TargetPosition;

	double angle;
	FVector axis_of_rotation;

	double Mass; // kg

	UPROPERTY()
	double Mu; // (cm^3)(s^-2) gravitational parameter = Mass * G


	double Radius; // m

	// condition
	double RadiusOfInfluence = INFINITY;

	UPROPERTY(EditAnywhere)
	FVector InitialVelocity;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

	virtual void TickPostPhysics(float DeltaTime) override;

	virtual void TickDuringPhysics(float DeltaTime) override {};

	virtual void RegisterActorTickFunctions(bool bRegister) override {
		Super::RegisterActorTickFunctions(bRegister);
		RegisterMultiTickActorTickFunctions(this, bRegister);
	}
};
