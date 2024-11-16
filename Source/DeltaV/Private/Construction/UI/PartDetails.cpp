// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/UI/PartDetails.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/SpinBox.h"

#include "Common/Part.h"
#include "Construction/ConstructionController.h"

UPartDetails::UPartDetails(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!UPartDetails::BlueprintClass) {
		ConstructorHelpers::FClassFinder<UPartDetails> Widget(TEXT("WidgetBlueprint'/Game/Construction/UI/WBP_PartDetails'"));
		if (Widget.Succeeded()) {
			UPartDetails::BlueprintClass = Widget.Class;
		}
	}
}

void UPartDetails::NativeOnInitialized() {
	Controller = GetOwningPlayer<AConstructionController>();
}

void UPartDetails::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	
	if (Part) {
	}

}

void UPartDetails::Update(PartField Field) {
	if (Part == nullptr) {
		return;
	}
	FVector Scale = Part->GetRelativeScale3D();

	switch (Field)
	{
	case PartField::Name:
		break;
	case PartField::SizeX:
		Scale.X = SizeX->GetValue();
		return Part->SetRelativeScale3D(Scale);
	case PartField::SizeY:
		Scale.Y = SizeY->GetValue();
		return Part->SetRelativeScale3D(Scale);
	case PartField::SizeZ:
		Scale.Z = SizeZ->GetValue();
		return Part->SetRelativeScale3D(Scale);
	case PartField::LiquidFuel:
		break;
	case PartField::Oxidizer:
		break;
	default:
		break;
	}
	
}

void UPartDetails::SetPart(UPart* InPart) {
	Part = InPart;
	if (Part) {
		PartName->SetText(FText::FromString(Part->GetName()));

		SizeX->SetValue(Part->GetRelativeScale3D().X);
		SizeY->SetValue(Part->GetRelativeScale3D().Y);
		SizeZ->SetValue(Part->GetRelativeScale3D().Z);
	}
}
