// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ConstructionController.h"

#include "GameFramework/PlayerInput.h"

#include "Common/JsonUtil.h"
#include "Common/Part.h"
#include "Common/AttachmentNode.h"
#include "Construction/ConstructionCraft.h"
#include "Construction/UI/ConstructionHUD.h"
#include "Construction/PartShapeEditor.h"

#include "DynamicMeshActor.h"
#include "Components/DynamicMeshComponent.h"

AConstructionController::AConstructionController() {

	bEnableClickEvents = true;

	// add right mouse button to click event as well
	ClickEventKeys.Add(EKeys::RightMouseButton);

	Selected = nullptr;
}

void AConstructionController::BeginPlay() {
	Super::BeginPlay();
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());

	HUD = CreateWidget<UConstructionHUD>(this, UConstructionHUD::BlueprintClass);
	HUD->AddToPlayerScreen();
}

void AConstructionController::SetupInputComponent() {
	Super::SetupInputComponent();

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveForwardBackward", EKeys::W, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveForwardBackward", EKeys::S, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveLeftRight", EKeys::A, -1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveLeftRight", EKeys::D, 1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveUpDown", EKeys::SpaceBar, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("MoveUpDown", EKeys::LeftShift, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("LookX", EKeys::MouseX, 1.f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("LookY", EKeys::MouseY, -1.f));

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("ZoomIn", EKeys::MouseWheelAxis, 100.f));

	PlayerInput->AddActionMapping(FInputActionKeyMapping("LeftClick", EKeys::LeftMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("RightClick", EKeys::RightMouseButton));
	PlayerInput->AddActionMapping(FInputActionKeyMapping("MiddleClick", EKeys::MiddleMouseButton));

	InputComponent->BindAxis("LookX", this, &AConstructionController::AddYawInput);
	InputComponent->BindAxis("LookY", this, &AConstructionController::AddPitchInput);

	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AConstructionController::HandleClick);
	InputComponent->BindAction("RightClick", IE_Pressed, this, &AConstructionController::HandleClick);
	InputComponent->BindAction("MiddleClick", IE_Pressed, this, &AConstructionController::HandleClick);

	PlayerInput->AddActionMapping(FInputActionKeyMapping("Save", EKeys::M));
	InputComponent->BindAction("Save", IE_Pressed, this, &AConstructionController::Save);
	PlayerInput->AddActionMapping(FInputActionKeyMapping("Load", EKeys::N));
	InputComponent->BindAction("Load", IE_Pressed, this, &AConstructionController::Load);

	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::I, 0.1f));
	PlayerInput->AddAxisMapping(FInputAxisKeyMapping("Throttle", EKeys::K, -0.1f));
	InputComponent->BindAxis("Throttle", this, &AConstructionController::Throttle);

	PlayerInput->AddActionMapping(FInputActionKeyMapping("DebugAction", EKeys::L));
	InputComponent->BindAction("DebugAction", IE_Pressed, this, &AConstructionController::DebugAction);
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

UPart* AConstructionController::PlaceHeldPart() {
	if (Craft == nullptr || SelectedPart == nullptr || Selected == nullptr) {
		return nullptr;
	}

	FVector location;
	FVector direction;
	if (!DeprojectMousePositionToWorld(location, direction)) {
		return nullptr;
	}

	FVector part_location = location + direction * PlaceDistance;
	UPart* AttachTo = nullptr;

	for (auto& node : SelectedPart->AttachmentNodes) {
		TArray<FHitResult> hit_results;
		FVector start = location;
		FVector end = (part_location + node->GetRelativeLocation() - location) * 2 + location;

		GetWorld()->LineTraceMultiByObjectType(hit_results, start, end, FCollisionObjectQueryParams::AllObjects);

		for (auto& hit_result : hit_results) {
			UAttachmentNode* component = Cast<UAttachmentNode>(hit_result.GetComponent());
			if (component == nullptr) {
				continue;
			}

			// don't check for the closer attachment node for now
			AttachTo = Cast<UPart>(component->GetOuter());
			// Actor filter don't work for some reason, maybe to do with changing component ownership with .Rename()
			if (AttachTo && component->GetOwner() != Selected) {
				part_location = AttachTo->GetComponentLocation() + component->GetRelativeLocation() - node->GetRelativeLocation();

				Selected->SetActorLocationAndRotation(part_location, FQuat::Identity);

				return AttachTo;
			}
			else {
				AttachTo = nullptr;
			}
		}
	}

	Selected->SetActorLocationAndRotation(part_location, FQuat::Identity);

	return AttachTo;
}

void AConstructionController::HandleClick(FKey Key) {
	if (ConstructionMode == AConstructionController::EditMode) {
		if (Key == EKeys::LeftMouseButton) {
			if (Selected != nullptr) {
				// Place
				UPart* AttachToPart = PlaceHeldPart();
				if (AttachToPart != nullptr) {
					Craft = Cast<AConstructionCraft>(AttachToPart->GetOwner());
					Craft->AttachPart(Selected, AttachToPart);
				}
				Selected = nullptr;
				// Craft->SetAttachmentNodeVisibility(true);
			}
			else {
				// Select
				FHitResult result;
				if (GetHitResultUnderCursor(ECC_WorldStatic, true, result)) {
					SelectedPart = Cast<UPart>(result.GetComponent());
					if (!SelectedPart) {
						return;
					}

					FVector ActorLocation = SelectedPart->GetRelativeLocation();
					FVector PawnLocation = GetPawn()->GetActorLocation();
					PlaceDistance = FVector::Distance(ActorLocation, PawnLocation);

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
				UE_LOG(LogTemp, Warning, TEXT("Nothing Selected"));
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
	JsonUtil::WriteFile(FPaths::Combine(FPaths::ProjectSavedDir(), "ship2.json"), Craft->Json);
}

void AConstructionController::Load() {
	Craft = GetWorld()->SpawnActor<AConstructionCraft>();
	Craft->Initialize(JsonUtil::ReadFile(FPaths::ProjectDir() + "Content/Crafts/ship.json"));
}

void AConstructionController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("tick"));

	switch (ConstructionMode)
	{
	case AConstructionController::EditMode:
		PlaceHeldPart();
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
		Craft->Throttle(Val);
	}
}
