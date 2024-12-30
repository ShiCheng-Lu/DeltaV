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
#include "Construction/UI/TransformGadget.h"

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

	TransformGadget = GetWorld()->SpawnActor<ATransformGadget>();
	TransformGadget->Controller = this;

	Load();
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

	// Rotate part actions
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate-X", EKeys::W));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate+X", EKeys::S));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate-Y", EKeys::A));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate+Y", EKeys::D));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate-Z", EKeys::Q));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Rotate+Z", EKeys::E));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("TranslateMode", EKeys::T));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("RotateMode", EKeys::R));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("ScaleMode", EKeys::F));


	InputComponent->BindAxis("CameraZoom", this, &AConstructionController::Zoom);

	InputComponent->BindAction("RightClick", EInputEvent::IE_Pressed, this, &AConstructionController::EnableMovement);
	InputComponent->BindAction("RightClick", EInputEvent::IE_Released, this, &AConstructionController::DisableMovement);

	InputComponent->BindAxis("LookX", this, &AConstructionController::AddYawInput);
	InputComponent->BindAxis("LookY", this, &AConstructionController::AddPitchInput);

	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AConstructionController::Pressed);
	InputComponent->BindAction("RightClick", IE_Pressed, this, &AConstructionController::Pressed);
	InputComponent->BindAction("MiddleClick", IE_Pressed, this, &AConstructionController::Pressed);
	InputComponent->BindAction("LeftClick", IE_Released, this, &AConstructionController::Released);
	InputComponent->BindAction("RightClick", IE_Released, this, &AConstructionController::Released);
	InputComponent->BindAction("MiddleClick", IE_Released, this, &AConstructionController::Released);

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

	DECLARE_DELEGATE_OneParam(SwitchMode, Mode);
	InputComponent->BindAction<SwitchMode>("TranslateMode", IE_Pressed, this, &AConstructionController::SwitchMode, Mode::TranslateMode);
	InputComponent->BindAction<SwitchMode>("RotateMode", IE_Pressed, this, &AConstructionController::SwitchMode, Mode::RotateMode);
	InputComponent->BindAction<SwitchMode>("ScaleMode", IE_Pressed, this, &AConstructionController::SwitchMode, Mode::ScaleMode);

	DECLARE_DELEGATE_OneParam(RotatePart, FRotator);
	InputComponent->BindAction<RotatePart>("Rotate+X", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(90, 0, 0));
	InputComponent->BindAction<RotatePart>("Rotate-X", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(-90, 0, 0));
	InputComponent->BindAction<RotatePart>("Rotate+Y", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, 90, 0));
	InputComponent->BindAction<RotatePart>("Rotate-Y", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, -90, 0));
	InputComponent->BindAction<RotatePart>("Rotate+Z", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, 0, 90));
	InputComponent->BindAction<RotatePart>("Rotate-Z", IE_Pressed, this, &AConstructionController::RotatePart, FRotator(0, 0, -90));

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

void AConstructionController::SwitchMode(Mode NewMode) {
	if (NewMode == ConstructionMode) {
		NewMode = Mode::EditMode;
	}
	ConstructionMode = NewMode;

	switch (ConstructionMode) {
	case EditMode:
		HUD->ModeText->SetText(FText::FromString("E"));
		break;
	case TranslateMode:
		HUD->ModeText->SetText(FText::FromString("T"));
		break;
	case RotateMode:
		HUD->ModeText->SetText(FText::FromString("R"));
		break;
	}
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

void AConstructionController::Pressed(FKey Key) {
	if (Key == EKeys::LeftMouseButton) {
		switch (ConstructionMode)
		{
		case AConstructionController::EditMode:
			if (Constructor.Selected) {
				UPart* Part = Constructor.Selected;
				Constructor.Place();
				ACraft* Craft = Cast<ACraft>(Part->GetOwner());
				if (Craft) {
					HUD->SetCraft(Craft);
				}
			}
			else {
				Constructor.Grab();
			}
			break;
		case AConstructionController::RotateMode:
			break;
		case AConstructionController::TranslateMode:
			TransformGadget->StartTracking();
			break;
		case AConstructionController::ScaleMode:
			break;
		case AConstructionController::WarpMode:
			break;
		default:
			break;
		}
	}
	else if (Key == EKeys::RightMouseButton) {
		// ignore if holding a part
		GetPawn()->EnableInput(this);
		ResetIgnoreLookInput();

		GetMousePosition(MousePosition.X, MousePosition.Y);
	}
	else if (Key == EKeys::MiddleMouseButton) {
	}
}

void AConstructionController::Released(FKey Key) {
	if (Key == EKeys::LeftMouseButton) {
		switch (ConstructionMode)
		{
		case AConstructionController::EditMode:
			break;
		case AConstructionController::RotateMode:
			break;
		case AConstructionController::TranslateMode:
			TransformGadget->StopTracking();
			break;
		case AConstructionController::ScaleMode:
			break;
		case AConstructionController::WarpMode:
			break;
		default:
			break;
		}
	}
	else if (Key == EKeys::RightMouseButton) {
		GetPawn()->DisableInput(this);
		SetIgnoreLookInput(true);

		FVector2f ReleasePosition;
		GetMousePosition(ReleasePosition.X, ReleasePosition.Y);
		if (ReleasePosition.Equals(MousePosition)) {
			// mouse haven't moved, consider this a click
			HUD->PartDetails->SetPart(Constructor.TraceMouse());
		}
	}
	else if (Key == EKeys::MiddleMouseButton) {
		UPart* Part = Constructor.TraceMouse();
		if (Part) {
			GetPawn()->SetActorLocation(Part->GetComponentLocation());
		}
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
	FString Path = FPaths::Combine(FPaths::ProjectSavedDir(), "ship2.json");
	TSharedPtr<FJsonObject> CraftJson = JsonUtil::ReadFile(Path);
	ACraft* Craft = Constructor.CreateCraft(CraftJson);
	HUD->SetCraft(Craft);
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


FRay AConstructionController::GetMouseRay() {
	FRay MouseRay = FRay();

	if (!DeprojectMousePositionToWorld(MouseRay.Origin, MouseRay.Direction)) {
		return FRay();
	}
	FRotator _;
	PlayerCameraManager->GetCameraViewPoint(MouseRay.Origin, _);
	
	return MouseRay;
}
