// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/AttachmentNodes.h"

#include "Components/StaticMeshComponent.h"
#include "Common/Part.h"
#include "Common/AssetLibrary.h"
#include "Common/JsonUtil.h"
#include "Construction/Constructor.h"

// Sets default values for this component's properties
UAttachmentNodes::UAttachmentNodes()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Part = GetTypedOuter<UPart>();
	if (Part == nullptr) {
		return;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("/Game/Shapes/AttachmentNode"));

	TSharedPtr<FJsonObject> PartDefinition = UAssetLibrary::PartDefinition(Part->Type);
	for (auto& LocationJson : PartDefinition->GetArrayField(TEXT("attachment"))) {
		FVector Location = JsonUtil::Vector(LocationJson->AsObject(), "location");

		auto Node = CreateDefaultSubobject<UStaticMeshComponent>(*Location.ToString());
		Node->SetStaticMesh(SphereMeshAsset.Object);
		Node->SetRelativeScale3D(FVector(0.1f));

		Node->SetupAttachment(this);
		Node->SetRelativeLocation(Location);
		Node->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
		Node->SetCollisionResponseToAllChannels(ECR_Ignore);
		Node->SetCollisionResponseToChannel(ECC_AttachmentNodes, ECR_Block);
		Node->SetAbsolute(false, false, true);

		AttachmentNodes.Add(Node);
	}

	SideAttachment = JsonUtil::Vector(PartDefinition, "side_attachment");

	SetupAttachment(Part);
}


// Called when the game starts
void UAttachmentNodes::BeginPlay()
{
	Super::BeginPlay();

	// ...
	for (auto& Node : AttachmentNodes) {
		Node->RegisterComponent();
	}

	UE_LOG(LogTemp, Warning, TEXT("Location %s"), *GetComponentLocation().ToString());

}


// Called every frame
void UAttachmentNodes::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UAttachmentNodes* UAttachmentNodes::Get(UPart* Part) {
	TArray<USceneComponent*> Components;
	Part->GetChildrenComponents(false, Components);
	for (USceneComponent* Component : Components) {
		UAttachmentNodes* Node = Cast<UAttachmentNodes>(Component);
		if (Node) {
			return Node;
		}
	}
	return nullptr;
}

void UAttachmentNodes::SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse NewResponse) {
	for (auto* Node : AttachmentNodes) {
		Node->SetCollisionResponseToChannel(Channel, NewResponse);
	}
}

FVector UAttachmentNodes::SideAttachmentLocation() {
	return GetComponentQuat().RotateVector(SideAttachment);
}
