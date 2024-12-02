// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AttachmentNodes.generated.h"

class UPart;
class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELTAV_API UAttachmentNodes : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttachmentNodes();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPart* Part;

	TArray<UStaticMeshComponent*> AttachmentNodes;
	FVector SideAttachment;

	static UAttachmentNodes* Get(UPart* Part);

	void SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse NewResponse);

	inline FVector SideAttachmentLocation();


};
