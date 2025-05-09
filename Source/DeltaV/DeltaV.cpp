// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeltaV.h"
#include "Modules/ModuleManager.h"

#include "Modules/ModuleInterface.h"
#include "Editor/UnrealEdEngine.h"
#include "ClassIconFinder.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define SLATE_IMAGE_BRUSH( ImagePath, ImageSize ) new FSlateImageBrush( StyleSetInstance->RootToContentDir( TEXT(ImagePath), TEXT(".png") ), FVector2D(ImageSize, ImageSize ) )


class DeltaV : public FDefaultGameModuleImpl
{
	typedef DeltaV ThisClass;

	static inline TSharedPtr<FSlateStyleSet> StyleSetInstance = nullptr;

	virtual void StartupModule() override
	{
		// Create the new style set
		StyleSetInstance = MakeShareable(new FSlateStyleSet("QuodGameplayFrameworkEditorStyle"));
		// Assign the content root of this style set
		StyleSetInstance->SetContentRoot(FPaths::ProjectContentDir());
		// Modify the class icons to use our new awesome icons
		StyleSetInstance->Set("ClassIcon.Part", SLATE_IMAGE_BRUSH("/Icons/part", 20.0f));
		// Finally register the style set so it is actually used
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSetInstance);
	}

	virtual void ShutdownModule() override
	{
		FModuleManager::Get().OnModulesChanged().RemoveAll(this);

		// Unregister the style set and reset the pointer
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSetInstance.Get());
		StyleSetInstance.Reset();
	}
};


IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, DeltaV, "DeltaV" );
