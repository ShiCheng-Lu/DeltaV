// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/AttachmentNode.h"

#include "Common/Part.h"

// Sets default values for this component's properties
UAttachmentNode::UAttachmentNode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	// PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("/Game/Shapes/AttachmentNode"));

	SetStaticMesh(SphereMeshAsset.Object);
	SetRelativeScale3D(FVector(0.1f));

	SetupAttachment(Cast<UPart>(GetOuter()));
}

void UAttachmentNode::Initialize(FVector& InRelativeLocation) {
	SetRelativeLocation(InRelativeLocation);
	RegisterComponent();
}
