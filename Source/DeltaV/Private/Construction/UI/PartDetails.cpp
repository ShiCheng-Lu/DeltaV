// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/PartDetails.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/SpinBox.h"

#include "Common/Part.h"
#include "Construction/ConstructionController.h"
#include "Construction/PartShapeEditor.h"

UPartDetails::UPartDetails(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!UPartDetails::BlueprintClass) {
		ConstructorHelpers::FClassFinder<UPartDetails> Widget(TEXT("WidgetBlueprint'/Game/Construction/UI/WBP_PartDetails'"));
		if (Widget.Succeeded()) {
			UPartDetails::BlueprintClass = Widget.Class;
		}
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UPartDetails::NativeOnInitialized() {
	Controller = GetOwningPlayer<AConstructionController>();

	if (Part == nullptr) {
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPartDetails::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	
	if (Part) {
	}

}

void UPartDetails::Update(PartField Field) {
	if (Part == nullptr) {
		return;
	}
	FVector Scale = Part->Mesh->GetRelativeScale3D();

	switch (Field)
	{
	case PartField::Name:
		break;
	case PartField::SizeX:
		Scale.X = SizeX->GetValue();
		return Part->Mesh->SetRelativeScale3D(Scale);
	case PartField::SizeY:
		Scale.Y = SizeY->GetValue();
		return Part->Mesh->SetRelativeScale3D(Scale);
	case PartField::SizeZ:
		Scale.Z = SizeZ->GetValue();
		return Part->Mesh->SetRelativeScale3D(Scale);
	case PartField::LiquidFuel:
		break;
	case PartField::Oxidizer:
		break;
	default:
		break;
	}
	
}

void UPartDetails::SetPart(UPart* InPart) {
	if (Part) {
		Part->Mesh->SetRenderCustomDepth(false);
		Part->Mesh->SetCustomDepthStencilValue(0);
	}

	Part = InPart;
	if (Part) {
		PartName->SetText(FText::FromString(Part->GetName()));

		SizeX->SetValue(Part->Mesh->GetRelativeScale3D().X);
		SizeY->SetValue(Part->Mesh->GetRelativeScale3D().Y);
		SizeZ->SetValue(Part->Mesh->GetRelativeScale3D().Z);
	
		SetVisibility(ESlateVisibility::Visible);

		Part->Mesh->SetRenderCustomDepth(false);
		Part->Mesh->SetCustomDepthStencilValue(4);
	}
	else {
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPartDetails::MakeDynamic() {
	Controller->PartShapeEditor->SetPart(Part);
	Controller->SwitchMode(AConstructionController::WarpMode);
}
