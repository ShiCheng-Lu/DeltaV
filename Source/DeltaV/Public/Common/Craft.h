// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Common/Part.h"
#include "Components/SphereComponent.h"

#include "Craft.generated.h"

UCLASS()
class DELTAV_API ACraft : public APawn
{
	GENERATED_BODY()

public:
	TSharedPtr<FJsonObject> Json;
	TMap<FString, UPart*> Parts;
	UPart* RootPart;
	UPart* Engine;
	bool PhysicsEnabled;

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

	void Initialize(TSharedPtr<FJsonObject> CraftJson);

	void AddPart(UPart* Part);

	void RemovePart(UPart* Part);

	static void TransferPart(UPart* Part, ACraft* FromCraft, ACraft* ToCraft);

	void SetAttachmentNodeVisibility(bool visibility);

	void AttachPart(ACraft* SourceCraft, UPart* AttachToPart);

	void DetachPart(UPart* Part, ACraft* NewCraft);

	// control
	void Rotate(FRotator rotator, float strength);

	void Throttle(float throttle);

	void SetPhysicsEnabled(bool enabled);
};
