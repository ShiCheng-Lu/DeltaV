// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionController.h"

#include "GameFramework/PlayerInput.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"

#include "Common/JsonUtil.h"
#include "Common/Part.h"
#include "Common/AttachmentNode.h"
#include "Construction/ConstructionCraft.h"
#include "Construction/UI/ConstructionHUD.h"
#include "Construction/PartShapeEditor.h"
#include "Construction/ConstructionPawn.h"

#include "DynamicMeshActor.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/TextBlock.h"

AConstructionController::AConstructionController() {

	bEnableClickEvents = true;

	// add right mouse button to click event as well
	ClickEventKeys.Add(EKeys::RightMouseButton);

	Selected = nullptr;
	Craft = nullptr;

	Symmetry = 1;

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
	InputComponent->BindAxis("Throttle", this, &AConstructionController::Throttle);

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
	if (Selected && !GetPawn()->InputEnabled()) {
		UE_LOG(LogTemp, Warning, TEXT("Rotated %s"), *Rotation.ToString());
		Selected->SetActorRotation(Rotation.Quaternion() * Selected->GetActorQuat());
	}
}


void AConstructionController::DebugAction() {
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
}

std::pair<UPart*, bool> AConstructionController::UpdateHeldPart() {
	if (Craft == nullptr || SelectedPart == nullptr || Selected == nullptr) {
		return { nullptr, false };
	}

	FVector CameraLocation;
	FVector direction;
	if (!DeprojectMousePositionToWorld(CameraLocation, direction)) {
		return { nullptr, false };
	}

	FRotator CameraRot;
	PlayerCameraManager->GetCameraViewPoint(CameraLocation, CameraRot);

	FVector part_location = CameraLocation + direction * PlaceDistance;
	UPart* AttachTo = nullptr;

	// node attachment
	for (auto& node : SelectedPart->AttachmentNodes) {
		TArray<FHitResult> hit_results;
		FVector start = CameraLocation;
		FVector RelativeNodeLocation = node->GetComponentLocation() - SelectedPart->GetComponentLocation();
		FVector end = ((part_location + RelativeNodeLocation) - start) * 2 + start;

		GetWorld()->LineTraceMultiByObjectType(hit_results, start, end, FCollisionObjectQueryParams::AllObjects);

		for (auto& HitResult : hit_results) {
			UAttachmentNode* component = Cast<UAttachmentNode>(HitResult.GetComponent());
			if (component == nullptr || component->GetOwner() == Selected) {
				continue;
			}
			UE_LOG(LogTemp, Warning, TEXT("Project Collide %s.%s, %s"), *component->GetOwner()->GetName(), *component->GetName(), *Selected->GetName());
			// don't check for the closer attachment node for now
			AttachTo = Cast<UPart>(component->GetOuter());
			// Actor filter don't work for some reason, maybe to do with changing component ownership with .Rename()
			if (AttachTo) {
				part_location = component->GetComponentLocation() - RelativeNodeLocation;

				Selected->SetActorLocation(part_location);

				return { AttachTo, false };
			}
			else {
				AttachTo = nullptr;
			}
		}
	}

	// side attachment
	if (AttachTo == nullptr) {
		TArray<FHitResult> HitResults;
		FVector Start = CameraLocation;
		FVector End = Start + direction * PlaceDistance;

		GetWorld()->LineTraceMultiByObjectType(HitResults, Start, End, FCollisionObjectQueryParams::AllObjects);

		for (auto& HitResult : HitResults) {
			UPart* Part = Cast<UPart>(HitResult.GetComponent());
			if (Part == nullptr || Part == SelectedPart || HitResult.GetActor() == Selected) {
				continue;
			}
			AttachTo = Part;
			part_location = HitResult.Location;
		}
	}

	Selected->SetActorLocation(part_location);

	return { AttachTo, true };
}

void AConstructionController::HandleClick(FKey Key) {
	if (Key == EKeys::MiddleMouseButton && Selected == nullptr) {
		FHitResult Result;
		if (GetHitResultUnderCursor(ECC_WorldStatic, true, Result)) {
			FVector Location = Result.GetComponent()->GetComponentLocation();
			GetPawn()->SetActorLocation(Location);
		}
	}

	if (ConstructionMode == AConstructionController::EditMode) {
		if (Key == EKeys::LeftMouseButton) {
			if (Selected != nullptr) {
				// Place
				auto [ AttachToPart, SideAttachment ] = UpdateHeldPart();
				
				// Craft->SetAttachmentNodeVisibility(true);
				if (AttachToPart == nullptr) {
					UE_LOG(LogTemp, Warning, TEXT("Simple place"));
					Selected = nullptr;
					return;
				}
				UE_LOG(LogTemp, Warning, TEXT("Attach place to %s.%s"), *AttachToPart->GetOwner()->GetName(), *AttachToPart->GetName());
				//if (AttachToPart->SymmetryGroup) {

				// }

				Craft = Cast<AConstructionCraft>(AttachToPart->GetOwner());
				if (SideAttachment && Symmetry == 0) {
					// mirror attachment, TODO: implement
					Craft->AttachPart(Selected, AttachToPart);
				}
				else {
					FVector Center = AttachToPart->GetComponentLocation();
					FVector Offset = SelectedPart->GetComponentLocation() - Center;
					if (SideAttachment) {
						// rotational symmetry attachment, place at location and additional positions
						for (int i = 1; i < Symmetry * SideAttachment; ++i) {
							FQuat RotationQuat = FQuat(FVector(0, 0, 1), 2 * PI * i / Symmetry);
							FVector NewOffset = RotationQuat.RotateVector(Offset);
							FQuat NewRotation = RotationQuat * SelectedPart->GetComponentQuat();

							// clone by converting to json and spawning another instance from json
							AConstructionCraft* NewCraft = Selected->Clone();
							NewCraft->SetActorLocationAndRotation(Center + NewOffset, NewRotation);
							Craft->AttachPart(NewCraft, AttachToPart);
						}
					}
					// place base
					Craft->AttachPart(Selected, AttachToPart);
				}
				Selected = nullptr;
				SelectedPart = nullptr;
			}
			else {
				// Select
				FHitResult result;
				if (GetHitResultUnderCursor(ECC_WorldStatic, true, result)) {
					SelectedPart = Cast<UPart>(result.GetComponent());
					if (!SelectedPart) {
						return;
					}

					FVector CameraLocation; FRotator Rot;
					PlayerCameraManager->GetCameraViewPoint(CameraLocation, Rot);
					PlaceDistance = FVector::Distance(SelectedPart->GetComponentLocation(), CameraLocation);

					Selected = Cast<AConstructionCraft>(SelectedPart->GetOwner());

					if (Selected->RootPart() != SelectedPart) {
						FActorSpawnParameters SpawnParamsAlwaysSpawn = FActorSpawnParameters();
						SpawnParamsAlwaysSpawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						auto NewCraft = GetWorld()->SpawnActor<AConstructionCraft>(SpawnParamsAlwaysSpawn);
						if (NewCraft) {
							Selected->DetachPart(SelectedPart, NewCraft);
							Selected = NewCraft;
						}
						else {
							UE_LOG(LogTemp, Warning, TEXT("Craft not spawned"));
						}
					}

					UE_LOG(LogTemp, Warning, TEXT("Selected part %s - %d"), *SelectedPart->Id, Selected->GetUniqueID());

					// Craft->SetAttachmentNodeVisibility(false);
				}
				if (Selected) {
					UE_LOG(LogTemp, Warning, TEXT("Selected Craft: %s"), *Selected->GetName());
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Nothing Selected"));
				}
			}
		}
		else if (Key == EKeys::RightMouseButton) {
			UE_LOG(LogTemp, Warning, TEXT("Right clicked"));
			// ignore if holding a part
			if (SelectedPart == NULL) {
				FHitResult result;
				if (GetHitResultUnderCursor(ECC_Visibility, true, result)) {
					// TSharedPtr<FJsonObject> Part = ((APart*)result.GetActor())->Json;
					// ((AConstructionHUD*)MyHUD)->MyWidget->ShowPart(Part);
				}
			}

			// DEBUG:
			if (GEngine) {
				FHitResult result;
				if (GetHitResultUnderCursor(ECC_WorldStatic, true, result)) {
					int actorGuid = result.GetComponent()->GetOwner()->GetUniqueID();
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt(result.GetComponent()->GetOwner()->GetUniqueID()));
				}
			}
		}
	}
	else if (ConstructionMode == AConstructionController::RotateMode) {

	}
	else { // ConstructionMode == AConstructionController::TranslationMode

	}
}

void AConstructionController::Save() {
	if (!Craft) {
		return;
	}
	JsonUtil::WriteFile(FPaths::Combine(FPaths::ProjectSavedDir(), "ship2.json"), Craft->ToJson());
}

void AConstructionController::Load() {
	Craft = GetWorld()->SpawnActor<AConstructionCraft>();
	Craft->FromJson(JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/ship.json"));
}

void AConstructionController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("tick"));

	switch (ConstructionMode)
	{
	case AConstructionController::EditMode:
		UpdateHeldPart();
		break;
	case AConstructionController::RotateMode:
		break;
	case AConstructionController::TranslateMode:
		break;
	default:
		break;
	}
}

void AConstructionController::Throttle(float Val) {
	if (Val != 0 && Craft != nullptr) {
		// Craft->Throttle(Val);
	}
}

void AConstructionController::SymmetryAdd() {
	Symmetry += 1;
	if (Symmetry) {
		HUD->SymmetryText->SetText(FText::Format(FTextFormat::FromString("{0}"), Symmetry));
	}
	else {
		HUD->SymmetryText->SetText(FText::FromString("M"));
	}
}

void AConstructionController::SymmetrySub() {
	Symmetry = FMath::Max(0, Symmetry - 1);
	if (Symmetry) {
		HUD->SymmetryText->SetText(FText::Format(FTextFormat::FromString("{0}"), Symmetry));
	}
	else {
		HUD->SymmetryText->SetText(FText::FromString("M"));
	}
}
