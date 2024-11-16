// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionController.h"

#include "GameFramework/PlayerInput.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"

#include "Common/JsonUtil.h"
#include "Common/Part.h"
#include "Common/Craft.h"
#include "Common/AttachmentNode.h"
#include "Construction/UI/ConstructionHUD.h"
#include "Construction/PartShapeEditor.h"
#include "Construction/ConstructionPawn.h"
#include "Construction/UI/PartDetails.h"

#include "DynamicMeshActor.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

AConstructionController::AConstructionController() {

	bEnableClickEvents = true;

	// add right mouse button to click event as well
	ClickEventKeys.Add(EKeys::RightMouseButton);

	Constructor.SetController(this);
	// PlayerCameraManagerClass = ACameraManager::StaticClass();
}

void AConstructionController::BeginPlay() {
	Super::BeginPlay();
	
	PlayerCameraManager->CameraStyle = FName(TEXT("FreeCam"));
	PlayerCameraManager->SetActorEnableCollision(false);

	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));

	HUD = CreateWidget<UConstructionHUD>(this, UConstructionHUD::BlueprintClass);
	HUD->AddToPlayerScreen();
}

void AConstructionController::SetupInputComponent() {
	Super::SetupInputComponent();


	// PlayerCameraManager->SetupInput(PlayerInput, InputComponent);

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveForwardBackward", EKeys::W, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveForwardBackward", EKeys::S, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveLeftRight", EKeys::A, -1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveLeftRight", EKeys::D, 1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveUpDown", EKeys::SpaceBar, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveUpDown", EKeys::LeftShift, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("LookX", EKeys::MouseX, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("LookY", EKeys::MouseY, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("CameraZoom", EKeys::MouseWheelAxis, 0.05f));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("LeftClick", EKeys::LeftMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("RightClick", EKeys::RightMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("MiddleClick", EKeys::MiddleMouseButton));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("Undo", EKeys::Z, false, true));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Redo", EKeys::Y, false, true));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Redo", EKeys::Z, true, true));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Cut", EKeys::X, false, true));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Copy", EKeys::C, false, true));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Paste", EKeys::V, false, true));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("SymmetryAdd", EKeys::X, false));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("SymmetrySub", EKeys::X, true));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("RightMouseButton", EKeys::RightMouseButton));

	// Rotate part actions
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate+X", EKeys::W));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate-X", EKeys::S));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate+Y", EKeys::A));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate-Y", EKeys::D));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate+Z", EKeys::Q));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate-Z", EKeys::E));


	InputComponent->BindAxis("CameraZoom", this, &AConstructionController::Zoom);

	InputComponent->BindAction("RightMouseButton", EInputEvent::IE_Pressed, this, &AConstructionController::EnableMovement);
	InputComponent->BindAction("RightMouseButton", EInputEvent::IE_Released, this, &AConstructionController::DisableMovement);

	InputComponent->BindAxis("LookX", this, &AConstructionController::AddYawInput);
	InputComponent->BindAxis("LookY", this, &AConstructionController::AddPitchInput);

	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AConstructionController::HandleClick);
	InputComponent->BindAction("RightClick", IE_Pressed, this, &AConstructionController::HandleClick);
	InputComponent->BindAction("MiddleClick", IE_Pressed, this, &AConstructionController::HandleClick);

	InputComponent->BindAction("SymmetryAdd", EInputEvent::IE_Pressed, this, &AConstructionController::SymmetryAdd);
	InputComponent->BindAction("SymmetrySub", EInputEvent::IE_Pressed, this, &AConstructionController::SymmetrySub);

	PlayerInput->AddActionMapping(FInputActionKeyMapping("Save", EKeys::M));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Load", EKeys::N));
	InputComponent->BindAction("Save", IE_Pressed, this, &AConstructionController::Save);
	InputComponent->BindAction("Load", IE_Pressed, this, &AConstructionController::Load);

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::I, 0.1f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::K, -0.1f));
	// InputComponent->BindAxis("Throttle", this, &AConstructionController::Throttle);

	PlayerInput->AddActionMapping(FInputActionKeyMapping("DebugAction", EKeys::L));
	InputComponent->BindAction("DebugAction", IE_Pressed, this, &AConstructionController::DebugAction);
	
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("CameraZoom", EKeys::MouseWheelAxis, 0.05f));

	InputComponent->BindAction<TDelegate<void(FRotator)>, AConstructionController>("Rotate+X", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(90, 0, 0));
	InputComponent->BindAction<TDelegate<void(FRotator)>, AConstructionController>("Rotate-X", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(-90, 0, 0));
	InputComponent->BindAction<TDelegate<void(FRotator)>, AConstructionController>("Rotate+Y", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, 90, 0));
	InputComponent->BindAction<TDelegate<void(FRotator)>, AConstructionController>("Rotate-Y", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, -90, 0));
	InputComponent->BindAction<TDelegate<void(FRotator)>, AConstructionController>("Rotate+Z", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, 0, 90));
	InputComponent->BindAction<TDelegate<void(FRotator)>, AConstructionController>("Rotate-Z", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, 0, -90));

	/*
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	if (EnhancedInputComponent) {
		UE_LOG(LogTemp, Warning, TEXT("ENAHNED INPUT !!! WE CAN USE"));
	}
	UInputAction* Action;
	EnhancedInputComponent->BindActionValueLambda(Action, ETriggerEvent::Triggered, [this]() { RotatePart(FRotator(90, 0, 0)); });
	// EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered, );
	*/
}

void AConstructionController::EnableMovement() {
	GetPawn()->EnableInput(this);
	ResetIgnoreLookInput();
}

void AConstructionController::DisableMovement() {
	GetPawn()->DisableInput(this);
	SetIgnoreLookInput(true);
}


void AConstructionController::Zoom(float value) {
	if (value != 0) {
		PlayerCameraManager->FreeCamDistance *= (1 - value);
	}
}

void AConstructionController::RotatePart(FRotator Rotation) {
	if (!GetPawn()->InputEnabled()) {
		Constructor.RotatePart(Rotation.Quaternion());
	}
}

void AConstructionController::DebugAction() {
	/*
	UPartShapeEditor* ShapeEditor = NewObject<UPartShapeEditor>();
	SelectedPart->GetStaticMesh()->GetBounds().GetBox().GetVertices(ShapeEditor->TargetBound);
	
	double size = ShapeEditor->TargetBound[1].Z - ShapeEditor->TargetBound[0].Z;
	ShapeEditor->TargetBound[0].Z += size * 0.1;
	ShapeEditor->TargetBound[1].Z -= size * 0.1;
	ShapeEditor->TargetBound[3].Z += size * 0.1;
	ShapeEditor->TargetBound[5].Z -= size * 0.1;
	
	UE::Geometry::FDynamicMesh3 Mesh = ShapeEditor->Initialize(SelectedPart->GetStaticMesh());


	UE_LOG(LogTemp, Warning, TEXT("Base Bound %s %s"), *ShapeEditor->BoundPosition.ToString(), *ShapeEditor->BoundSize.ToString());
	for (int i = 0; i < 8; ++i) {
		UE_LOG(LogTemp, Warning, TEXT("Target Bound %s"), *ShapeEditor->TargetBound[i].ToString());
	}

	UDynamicMesh* DynamicMesh = NewObject<UDynamicMesh>();

	ADynamicMeshActor* NewActor = GetWorld()->SpawnActor<ADynamicMeshActor>();
	NewActor->SetActorLocation(FVector(-200, -200, 0));
	NewActor->GetDynamicMeshComponent()->SetDynamicMesh(DynamicMesh);
	//DynamicMesh->bEnableMeshGenerator = true;
	//DynamicMesh->SetMeshGenerator(ShapeEditor);
	ShapeEditor->Generate(Mesh);
	DynamicMesh->SetMesh(Mesh);
	*/
}

void AConstructionController::HandleClick(FKey Key) {
	if (ConstructionMode == AConstructionController::EditMode) {
		if (Key == EKeys::LeftMouseButton) {
			if (Constructor.Selected) {
				Constructor.Place();
			}
			else {
				Constructor.Grab();
			}
		}
		else if (Key == EKeys::RightMouseButton) {
			// ignore if holding a part
			if (Constructor.Selected == NULL) {
				UPart* Part = Constructor.TraceMouse();
				HUD->PartDetails->SetPart(Part);

				UE_LOG(LogTemp, Warning, TEXT("HERE %d"), Part != nullptr);

				if (GEngine && Part) {
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Part->GetOwner()->GetName());
				}
			}
		}
		else if (Key == EKeys::MiddleMouseButton) {
			UPart* Part = Constructor.TraceMouse();
			if (Part) {
				GetPawn()->SetActorLocation(Part->GetComponentLocation());
			}
		}
	}
	else if (ConstructionMode == AConstructionController::RotateMode) {

	}
	else if (ConstructionMode == AConstructionController::TranslateMode) {

	}
}

void AConstructionController::Save() {
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACraft::StaticClass(), Actors);
	for (AActor* Actor : Actors) {
		ACraft* Craft = Cast<ACraft>(Actor);
		FString Path = FPaths::Combine(FPaths::ProjectSavedDir(), "ship2.json");
		JsonUtil::WriteFile(Path, Craft->ToJson());
	}
}

void AConstructionController::Load() {
	/*
	Craft = GetWorld()->SpawnActor<ACraft>();
	Craft->FromJson(JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/ship.json"));
	*/
}

void AConstructionController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("tick"));

	switch (ConstructionMode)
	{
	case AConstructionController::EditMode:
		Constructor.Tick();
		break;
	case AConstructionController::RotateMode:
		break;
	case AConstructionController::TranslateMode:
		break;
	default:
		break;
	}
}

void AConstructionController::SymmetryAdd() {
	int Symmetry = Constructor.Symmetry + 1;
	Constructor.UpdateSymmetry(Symmetry);
	if (Symmetry) {
		HUD->SymmetryText->SetText(FText::Format(FTextFormat::FromString("{0}"), Symmetry));
	}
	else {
		HUD->SymmetryText->SetText(FText::FromString("M"));
	}
}

void AConstructionController::SymmetrySub() {
	int Symmetry = FMath::Max(0, Constructor.Symmetry - 1);
	Constructor.UpdateSymmetry(Symmetry);
	if (Symmetry) {
		HUD->SymmetryText->SetText(FText::Format(FTextFormat::FromString("{0}"), Symmetry));
	}
	else {
		HUD->SymmetryText->SetText(FText::FromString("M"));
	}
}
