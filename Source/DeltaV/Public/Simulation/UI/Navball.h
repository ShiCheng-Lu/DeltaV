// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Navball.generated.h"

class ACraft;
class UStaticMeshComponent;

UENUM()
enum EStabilizationMode : uint8 {
	HoldAttitude,
	Maneuver,
	Prograde,
	Retrograde,
	RadialIn,
	RadialOut,
	Normal,
	AntiNormal,
	Target,
	AntiTarget,

	None,
	Num = None UMETA(Hidden),
};


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
	TObjectPtr<UStaticMeshComponent> ProgradeTexture;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RetrogradeTexture;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> NormalTexture;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> AntiNormalTexture;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RadialInTexture;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RadialOutTexture;

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "EStabilizationMode"))
	TObjectPtr<UStaticMeshComponent> TargetTextures[EStabilizationMode::Num];

	EStabilizationMode StabilizationMode;
	float TimeSinceLastInput;
	float TimeSinceLastInputThreshold;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextureRenderTarget2D> RenderTarget;

	TObjectPtr<class ACraft> Target;
	FVector Origin;

	void SetTarget(ACraft* Craft, FVector PlanetCenter);
};
