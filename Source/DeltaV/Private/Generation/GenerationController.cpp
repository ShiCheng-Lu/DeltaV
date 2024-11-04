// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/GenerationController.h"
#include "Generation/MeshGeneration.h"
#include "DynamicMeshActor.h"
#include "GameFramework/PlayerInput.h"

void AGenerationController::BeginPlay() {
	ShapeEditor = NewObject<UMeshGeneration>(this);

	Mesh = FDynamicMesh3();
	Mesh.Clear();

	DynamicMesh = NewObject<UDynamicMesh>(this);

	NewActor = GetWorld()->SpawnActor<ADynamicMeshActor>();
	NewActor->SetActorLocation(FVector(0, 0, 0));
	NewActor->GetDynamicMeshComponent()->SetDynamicMesh(DynamicMesh);
	

	// DynamicMesh->bEnableMeshGenerator = true;
	// DynamicMesh->SetMeshGenerator(ShapeEditor);
	ShapeEditor->Initialize(Mesh);
	ShapeEditor->Iterate(Mesh);
	DynamicMesh->SetMesh(Mesh);

	UMaterialInterface* Water = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, 
		TEXT("Material'/Game/Materials/Water.Water'")));
	UMaterialInterface* Grass = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		TEXT("Material'/Game/Materials/Grass.Grass'")));
	UMaterialInterface* Dirt = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		TEXT("Material'/Game/Materials/Dirt.Dirt'")));
	UMaterialInterface* Snow = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		TEXT("Material'/Game/Materials/Snow.Snow'")));
	UMaterialInterface* VertexColor = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		TEXT("Material'/Game/Materials/VertexColor.VertexColor'")));
	TArray<UMaterialInterface*> MaterialList = { Water, Grass, Dirt, Dirt, Dirt, Snow };
	NewActor->GetDynamicMeshComponent()->ConfigureMaterialSet(MaterialList);
	int i = NewActor->GetDynamicMeshComponent()->GetNumMaterials();
	UE_LOG(LogTemp, Warning, TEXT("%d number of materials"), i);
}

void AGenerationController::Tick(float DeltaSeconds) {
	if (Progress) {
		ShapeEditor->Iterate(Mesh);
		ShapeEditor->Generate(Mesh);
		DynamicMesh->SetMesh(Mesh);
		NewActor->GetDynamicMeshComponent()->NotifyMeshUpdated();
	}
	
	// DrawDebugDirectionalArrow(GetWorld(), FVector(0, 0, 0), FVector(0.0001, 0, 0), 3, FColor::Red);
}

void AGenerationController::SetupInputComponent() {
	Super::SetupInputComponent();

	PlayerInput->AddActionMapping(FInputActionKeyMapping("Stage", EKeys::N));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Con", EKeys::M));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Clear", EKeys::B));
	InputComponent->BindAction("Stage", EInputEvent::IE_Pressed, this, &AGenerationController::Start);
	InputComponent->BindAction("Stage", EInputEvent::IE_Released, this, &AGenerationController::Stop);
	InputComponent->BindAction("Con", EInputEvent::IE_Pressed, this, &AGenerationController::Toggle);
	InputComponent->BindAction("Clear", EInputEvent::IE_Pressed, this, &AGenerationController::Clear);
}

void AGenerationController::Start() {
	Progress = true;
}
void AGenerationController::Stop() {
	Progress = false;
}
void AGenerationController::Toggle() {
	Progress = !Progress;
}
void AGenerationController::Clear() {
	FlushPersistentDebugLines(GetWorld());
}