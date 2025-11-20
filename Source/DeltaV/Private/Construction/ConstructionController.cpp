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

#include "Common/AssetLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosModularVehicle/ModularVehicleBaseComponent.h"

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

	HUD = CreateWidget<UConstructionHUD>(this, UAssetLibrary::LoadClass<UUserWidget>(UConstructionHUDClass));
	HUD->AddToPlayerScreen();
	/**
	TransformGadget = GetWorld()->SpawnActor<ATransformGadget>();
	TransformGadget->Controller = this;

	PartShapeEditor = GetWorld()->SpawnActor<APartShapeEditor>();
	PartShapeEditor->SetController(this);
	*/
	Load();
}

void AConstructionController::SetupInputComponent() {
	Super::SetupInputComponent();

	
	if (auto* LocalPlayer = GetLocalPlayer()) {
		if (auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) {
			//auto* InputMappingContext = UAssetLibrary::LoadAsset<UInputMappingContext>("/Game/Construction/IMC_Construction");
			//Subsystem->AddMappingContext(InputMappingContext, 1);

			auto* IMC_Common = UAssetLibrary::LoadAsset<UInputMappingContext>("/Game/Inputs/IMC_Common");
			Subsystem->AddMappingContext(IMC_Common, 2);

			auto* IMC_Simulation = UAssetLibrary::LoadAsset<UInputMappingContext>("/Game/Inputs/IMC_Simulation");
			Subsystem->AddMappingContext(IMC_Simulation, 0);
		}
	}
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent)) {
		auto* Move = UAssetLibrary::LoadAsset<UInputAction>("/Game/Inputs/IA_Move");
		EnhancedInput->BindAction(Move, ETriggerEvent::Triggered, this, &AConstructionController::Move);

		UE_LOG(LogTemp, Warning, TEXT("Added input"));
		if (OwnedCraft != nullptr) {

		}

		auto SetupCraftInput = [this, EnhancedInput](const FString Name) {
			auto* Input = UAssetLibrary::LoadAsset<UInputAction>("/Game/Inputs/IA_" + Name);
			EnhancedInput->BindActionValueLambda(Input, ETriggerEvent::Triggered, [this, Name](const FInputActionValue& Input) {
				if (OwnedCraft != nullptr) {
					OwnedCraft->GetVehicleSimulationComponent()->SetInputAxis1D(FName(Name), Input.Get<float>());
				}
			});
		};

		SetupCraftInput("Steering");
		SetupCraftInput("Thrust");
		SetupCraftInput("Throttle");
		SetupCraftInput("Pitch");
		SetupCraftInput("Roll");
		SetupCraftInput("Yaw");

		auto* Look = UAssetLibrary::LoadAsset<UInputAction>("/Game/Inputs/IA_Look");
		EnhancedInput->BindActionValueLambda(Look, ETriggerEvent::Triggered, [this](const FInputActionValue& Input) {
			AddPitchInput(Input.Get<FVector2D>().Y);
			AddYawInput(Input.Get<FVector2D>().X);
		});

		auto* Stage = UAssetLibrary::LoadAsset<UInputAction>("/Game/Inputs/IA_Stage");
		EnhancedInput->BindActionValueLambda(Stage, ETriggerEvent::Triggered, [this](const FInputActionValue& Input) {
			if (OwnedCraft != nullptr) {
				OwnedCraft->StageCraft();
			}
		});
	}

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
	case WarpMode:
		HUD->ModeText->SetText(FText::FromString("W"));
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

void AConstructionController::Move(const FInputActionValue& Movement) {
	UE_LOG(LogTemp, Warning, TEXT("Moved: %s"), *Movement.ToString());

	FRotator ControlSpaceRot = GetControlRotation();
	ControlSpaceRot.Pitch = 0;
	FVector Input = Movement.Get<FVector>();
	FVector Move = FVector(Input.Y, Input.X, 0);
	FVector Direction = ControlSpaceRot.RotateVector(Move);
	Direction.Z = Input.Z;
	GetPawn()->AddMovementInput(Direction);
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
	GetMousePosition(PressedPosition.X, PressedPosition.Y);

	if (Key == EKeys::LeftMouseButton) {
		switch (ConstructionMode)
		{
		case AConstructionController::EditMode:
			if (Constructor.Selected) {
				UPart* Part = Constructor.Selected;
				Constructor.Place();
				/*
				ACraft* Craft = Cast<ACraft>(Part->GetOwner());
				if (Craft) {
					HUD->SetCraft(Craft);
				}
				*/
				UE_LOG(LogTemp, Warning, TEXT("LeftMouseButton EditMode Place"));
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("LeftMouseButton EditMode Grab"));
				Constructor.Grab();
			}
			break;
		case AConstructionController::RotateMode:
			UE_LOG(LogTemp, Warning, TEXT("LeftMouseButton RotateMode"));
			break;
		case AConstructionController::TranslateMode:
			UE_LOG(LogTemp, Warning, TEXT("LeftMouseButton TranslateMode"));
			TransformGadget->StartTracking();
			break;
		case AConstructionController::ScaleMode:
			UE_LOG(LogTemp, Warning, TEXT("LeftMouseButton ScaleMode"));
			break;
		case AConstructionController::WarpMode:
			UE_LOG(LogTemp, Warning, TEXT("LeftMouseButton WarpMode"));
			PartShapeEditor->Pressed(Key);
			break;
		default:
			break;
		}
	}
	else if (Key == EKeys::RightMouseButton) {
		// ignore if holding a part
		GetPawn()->EnableInput(this);
		ResetIgnoreLookInput();
	}
	else if (Key == EKeys::MiddleMouseButton) {
	}
}

void AConstructionController::Released(FKey Key) {
	GetMousePosition(ReleasedPosition.X, ReleasedPosition.Y);

	if (Key == EKeys::LeftMouseButton) {
		switch (ConstructionMode)
		{
		case AConstructionController::EditMode:
			break;
		case AConstructionController::RotateMode:
			break;
		case AConstructionController::TranslateMode:
			TransformGadget->StopTracking();
			if (ReleasedPosition.Equals(PressedPosition)) {
				TransformGadget->Select(Constructor.TraceMouse()->Mesh);
			}
			break;
		case AConstructionController::ScaleMode:
			break;
		case AConstructionController::WarpMode:
			PartShapeEditor->Released(Key);
			break;
		default:
			break;
		}
	}
	else if (Key == EKeys::RightMouseButton) {
		GetPawn()->DisableInput(this);
		SetIgnoreLookInput(true);

		if (ReleasedPosition.Equals(PressedPosition)) {
			// mouse haven't moved, consider this a click
			HUD->PartDetails->SetPart(Constructor.TraceMouse());
		}
	}
	else if (Key == EKeys::MiddleMouseButton) {
		UPart* Part = Constructor.TraceMouse();
		if (Part) {
			GetPawn()->SetActorLocation(Part->Mesh->GetComponentLocation());
		}
	}
}

void AConstructionController::Save() {
	/*
	TSharedPtr<FJsonObject> CraftJson = OwnedCraft->ToJson();

	FString Path = FPaths::Combine(FPaths::ProjectContentDir(), "Crafts/saved.json");
	JsonUtil::WriteFile(Path, CraftJson);
	OwnedCraft->Destroy();

	OwnedCraft = Constructor.CreateCraft(CraftJson);
	// OwnedCraft->SetActorLocation(FVector(0, 100, 0));
	OwnedCraft->SetPhysicsEnabled(true);

	HUD->SetCraft(OwnedCraft);
	Possess(OwnedCraft);
	*/

	FString Path = FPaths::Combine(FPaths::ProjectContentDir(), "Crafts/car.json");
	TSharedPtr<FJsonObject> CraftJson = JsonUtil::ReadFile(Path);
	OwnedCraft->FromJson(CraftJson);
	/*
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACraft::StaticClass(), Actors);
	for (AActor* Actor : Actors) {
		ACraft* Craft = Cast<ACraft>(Actor);
		FString Path = FPaths::Combine(FPaths::ProjectSavedDir(), "ship2.json");
		JsonUtil::WriteFile(Path, Craft->ToJson());
	}
	*/
}

void AConstructionController::Load() {
	// FString Path = FPaths::Combine(FPaths::ProjectSavedDir(), "ship2.json");
	FString Path = FPaths::Combine(FPaths::ProjectContentDir(), "Crafts/car.json");
	TSharedPtr<FJsonObject> CraftJson = JsonUtil::ReadFile(Path);
	OwnedCraft = Constructor.CreateCraft(CraftJson);
	// OwnedCraft->SetActorLocation(FVector(0, 0, 100));
	OwnedCraft->SetPhysicsEnabled(false);

	HUD->SetCraft(OwnedCraft);
	Possess(OwnedCraft);
}

void AConstructionController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	/*
	if (OwnedCraft != nullptr) {
		if (auto* Sim = OwnedCraft->GetVehicleSimulationComponent()) {
			Sim->SetInputAxis1D(FName("Throttle"), 1);
		}
	}
	*/

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
