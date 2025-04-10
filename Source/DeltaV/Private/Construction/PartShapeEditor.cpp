// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/PartShapeEditor.h"

#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "MeshTypes.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"

#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"

// 
#include "MeshDescriptionToDynamicMesh.h"
#include "DynamicMeshToMeshDescription.h" // can always use this

// runtime conversion from render data to dynamic mesh
// #include "Runtime/MeshConversionEngineTypes/Public/StaticMeshLODResourcesToDynamicMesh.h"

// for procedural mesh from mesh description if I want to use procedural
#include "ProceduralMeshConversion.h"

#include "Components/DynamicMeshComponent.h"

#include "DynamicMesh/DynamicMesh3.h"
#include "Construction/ConstructionController.h"
#include "Common/Part.h"
#include "Construction/UI/TransformGadget.h"

APartShapeEditor::APartShapeEditor(const FObjectInitializer& ObjectInitializer) {
	
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SquareMesh(TEXT("/Game/Shapes/square"));

	Mesh = CreateDefaultSubobject<UDynamicMeshComponent>("Mesh");
	SetRootComponent(Mesh);

	// corners are in binary order, a 1 bit represent the upper bound, a 0 bit represent the lower bound,
	// 0bXYZ 
	for (int i = 0; i < 8; ++i) {
		FName Name = FName(*FString::Printf(TEXT("Corner%i"), i));
		auto* Corner = CreateDefaultSubobject<UStaticMeshComponent>(Name);

		Corner->SetStaticMesh(SquareMesh.Object);
		Corner->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
		Corner->SetCollisionResponseToAllChannels(ECR_Ignore);
		Corner->SetCollisionResponseToChannel(ECC_TransformGadget, ECR_Block);
		Corner->SetRelativeScale3D(FVector(0.1));
		Corner->SetAbsolute(false, false, true);

		Corner->SetupAttachment(Mesh);

		Corners.Push(Corner);
	}
}

void APartShapeEditor::SetPart(UPart* Part) {
	SelectedPart = Part;

	FVector Min, Max;
	Max = Part->Mesh->GetLocalBounds().GetBoxExtrema(1);
	Min = Part->Mesh->GetLocalBounds().GetBoxExtrema(0);
	for (int i = 0; i < Corners.Num(); ++i) {
		FVector Loc = FVector(
			i & 0b100 ? Max.X : Min.X,
			i & 0b010 ? Max.Y : Min.Y,
			i & 0b001 ? Max.Z : Min.Z
		);
		Corners[i]->SetRelativeLocation(Loc);
	}

	SetActorLocation(Part->Mesh->GetComponentLocation());

	// Make a dynamic mesh from the part
	FBox Box;
	Box.GetCenter();
	Box.GetExtent();

	Part->CopyMeshToDynamicMesh(Mesh->GetDynamicMesh());

	Mesh->EditMesh([this](FDynamicMesh3& MeshInOut) {
		UpdateMesh(MeshInOut);
	});
	UE_LOG(LogTemp, Warning, TEXT("Set Part"));
}

void APartShapeEditor::SetController(AConstructionController* InController) {
	Controller = InController;
}


void APartShapeEditor::Pressed(FKey Key) {
	if (Selected) {
		Controller->TransformGadget->StartTracking();
		if (Controller->TransformGadget->Active) {
			Active = true;
			UE_LOG(LogTemp, Warning, TEXT("active = true"));
		}
	}
}

void APartShapeEditor::Released(FKey Key) {
	if (Controller->ReleasedPosition.Equals(Controller->PressedPosition)) {
		FHitResult Result;
		if (Controller->GetHitResultUnderCursor(ECC_TransformGadget, true, Result)) {
			Selected = Cast<UStaticMeshComponent>(Result.GetComponent());
			Controller->TransformGadget->Select(Selected);
		}
	}
	else {
		Controller->TransformGadget->StopTracking();
		if (Active) {
			Active = false;
			UE_LOG(LogTemp, Warning, TEXT("active = false"));
		}
	}
}

void APartShapeEditor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (Active) {
		Mesh->EditMesh([this](FDynamicMesh3& MeshInOut) {
			UpdateMesh(MeshInOut);
		});
	}
}

void APartShapeEditor::UpdateMesh(FDynamicMesh3& MeshInOut) {
	FMeshDescription* MeshDescription = SelectedPart->CopyMeshToDynamicMesh(nullptr);
	FVertexArray vertices = MeshDescription->Vertices();

	FVector TargetBound[8];
	for (int i = 0; i < Corners.Num(); ++i) {
		TargetBound[i] = Corners[i]->GetRelativeLocation();
	}
	FVector BoundPosition, BoundSize;
	FBox BoundBox = SelectedPart->Mesh->GetLocalBounds().GetBox();
	BoundSize = BoundBox.GetSize();
	BoundPosition = BoundBox.GetCenter() - BoundSize / 2;

	for (auto Id : vertices.GetElementIDs()) {
	//for (auto Id : MeshInOut.VertexIndicesItr()) {
		FVector P = FVector(MeshDescription->GetVertexPosition(Id));
		//FVector P = MeshInOut.GetVertex(Id);

		// point normalized to be from 0 to 1
		FVector PN = (P - BoundPosition) / BoundSize;

		// position after linearly interpolating Z position
		FVector NX_NY = TargetBound[0] * (1 - PN.Z) + TargetBound[1] * PN.Z;
		FVector NX_PY = TargetBound[2] * (1 - PN.Z) + TargetBound[3] * PN.Z;
		FVector PX_NY = TargetBound[4] * (1 - PN.Z) + TargetBound[5] * PN.Z;
		FVector PX_PY = TargetBound[6] * (1 - PN.Z) + TargetBound[7] * PN.Z;
		// position after linearly interpolating Y position
		FVector NX = NX_NY * (1 - PN.Y) + NX_PY * PN.Y;
		FVector PX = PX_NY * (1 - PN.Y) + PX_PY * PN.Y;
		// position after linearly interpolating X position
		FVector transformed_position = NX * (1 - PN.X) + PX * PN.X;

		MeshInOut.SetVertex(Id, transformed_position);
	}
}

