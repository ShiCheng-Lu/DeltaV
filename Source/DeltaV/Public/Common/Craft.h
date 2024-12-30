// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Common/Part.h"
#include "Common/Craft/Stage.h"
#include "Components/SphereComponent.h"
#include "Common/CustomTickFunction.h"

#include "Craft.generated.h"

UCLASS()
class DELTAV_API ACraft : public APawn
{
	GENERATED_BODY()

public:
	// TSharedPtr<FJsonObject> Json;
	TMap<FString, UPart*> Parts;
	bool PhysicsEnabled;
	TMap<FString, TArray<UPart*>> SymmetryGroups;

	TObjectPtr<class UOrbitComponent> Orbit;
	FVector TargetVelocity; // Absolute velocity target for physics simulation (from orbit)
	FVector TargetPosition;

	TArray<UStage*> Stages;
	UStage* Active; // stage that is active

	void FromJson(TSharedPtr<FJsonObject> Json);
	TSharedPtr<FJsonObject> ToJson();
	ACraft* Clone();

public:
	// Sets default values for this pawn's properties
	ACraft(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void AttachPart(ACraft* SourceCraft, UPart* AttachToPart);

	void DetachPart(UPart* Part, ACraft* NewCraft);

	// control
	void Rotate(FRotator rotator, float strength);

	void SetPhysicsEnabled(bool enabled);

	TArray<ACraft*> StageCraft();

	FVector GetAngularVelocity();

	FVector CalculateCoM();

	FVector GetWorldCoM();

	UPart* RootPart() { return Cast<UPart>(GetRootComponent()); }


	FCustomActorTick<ACraft> PostPhysics;
	void TickPostPhysics(float DeltaTime);
	virtual void RegisterActorTickFunctions(bool bRegister) override {
		Super::RegisterActorTickFunctions(bRegister);
		PostPhysics.RegisterTickFunctions(this, bRegister);
	}
};
