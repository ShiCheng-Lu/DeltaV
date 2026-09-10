// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ThumbnailGenerator.h"

#include "Common/AssetLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "ImageCore.h"

#include "Engine/SceneCapture2D.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/DirectionalLightComponent.h"


ThumbnailGenerator::ThumbnailGenerator()
{
}

ThumbnailGenerator::~ThumbnailGenerator()
{
}

void ThumbnailGenerator::GenerateThumbnail(const FString& Path, TObjectPtr<AActor> Actor) {
	return GenerateThumbnails({ {Path, Actor} });
}

static void LogObjectFlags(const UObject* Object, const TCHAR* Label)
{
	if (!Object)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: nullptr"), Label);
		return;
	}

	const EObjectFlags Flags = Object->GetFlags();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("%s: %s | Flags=0x%08X"),
		Label,
		*Object->GetFullName(),
		static_cast<uint32>(Flags)
	);

	UE_LOG(LogTemp, Warning, TEXT("  RF_Public:             %s"),
		EnumHasAnyFlags(Flags, RF_Public) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_Standalone:         %s"),
		EnumHasAnyFlags(Flags, RF_Standalone) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_Transient:          %s"),
		EnumHasAnyFlags(Flags, RF_Transient) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_MarkAsRootSet:      %s"),
		EnumHasAnyFlags(Flags, RF_MarkAsRootSet) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_BeginDestroyed:     %s"),
		EnumHasAnyFlags(Flags, RF_BeginDestroyed) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_FinishDestroyed:    %s"),
		EnumHasAnyFlags(Flags, RF_FinishDestroyed) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_NeedInitialization: %s"),
		EnumHasAnyFlags(Flags, RF_NeedInitialization) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_NeedLoad:            %s"),
		EnumHasAnyFlags(Flags, RF_NeedLoad) ? TEXT("YES") : TEXT("NO"));

	UE_LOG(LogTemp, Warning, TEXT("  RF_NeedPostLoad:        %s"),
		EnumHasAnyFlags(Flags, RF_NeedPostLoad) ? TEXT("YES") : TEXT("NO"));
}

void ThumbnailGenerator::GenerateThumbnails(TMap<FString, TObjectPtr<AActor>> FilePathToActors) {
	if (!Initialize()) {
		return;
	}

	for (auto& [Path, Actor] : FilePathToActors) {
		Render(Path, Actor);
	}

	Cleanup();
}

bool ThumbnailGenerator::Initialize() {
	// create world
	if (!World) {
		UPackage* Package = NewObject<UPackage>(
			GetTransientPackage(),
			NAME_None,
			RF_Transient
		);
		UWorld* Template = FAssetLibrary::LoadAsset<UWorld>(
			TEXT("/Game/Construction/Thumbnail/ThumbnailWorld")
		);
		FObjectDuplicationParameters DuplicationsParams(
			Template,
			Package
		);
		DuplicationsParams.DestName = TEXT("ThumbnailRuntimeWorld");
		DuplicationsParams.DuplicateMode = EDuplicateMode::World;

		World = Cast<UWorld>(StaticDuplicateObjectEx(DuplicationsParams));

		LogObjectFlags(World, TEXT("After Load"));

		if (!World) {
			return false;
		}
		
		World->WorldType = EWorldType::Game;
		if (!World->IsInitialized()) {
			World->InitWorld(FWorldInitializationValues()
				.AllowAudioPlayback(false)
				.RequiresHitProxies(false)
				.CreatePhysicsScene(false)
				.CreateNavigation(false)
				.CreateAISystem(false)
				.ShouldSimulatePhysics(false)
			);
		}
		LogObjectFlags(World, TEXT("After InitWorld"));

		if (!World->AreActorsInitialized()) {
			World->InitializeActorsForPlay(FURL());
		}
		if (!World->HasBegunPlay()) {
			World->BeginPlay();
		}
		UE_LOG(LogTemp, Warning,
			TEXT("Runtime World: %s"),
			World ? *World->GetFullName() : TEXT("NULL"));

		UE_LOG(LogTemp, Warning,
			TEXT("Runtime PersistentLevel: %s"),
			World && World->PersistentLevel
			? *World->PersistentLevel->GetFullName()
			: TEXT("NULL"));

		if (World && World->PersistentLevel)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Runtime actor count: %d"),
				World->PersistentLevel->Actors.Num()
			);

			for (AActor* Actor : World->PersistentLevel->Actors)
			{
				if (!Actor)
				{
					continue;
				}

				if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
				{
					UStaticMeshComponent* Mesh = MeshActor->GetStaticMeshComponent();

					UE_LOG(LogTemp, Warning,
						TEXT("StaticMeshActor: %s"), *GetNameSafe(MeshActor));

					UE_LOG(LogTemp, Warning,
						TEXT("  Mesh: %s"), *GetNameSafe(Mesh->GetStaticMesh()));

					UE_LOG(LogTemp, Warning,
						TEXT("  Registered: %d"), Mesh->IsRegistered());

					UE_LOG(LogTemp, Warning,
						TEXT("  Visible: %d"), Mesh->IsVisible());

					UE_LOG(LogTemp, Warning,
						TEXT("  HiddenInGame: %d"), Mesh->bHiddenInGame);

					UE_LOG(LogTemp, Warning,
						TEXT("  Bounds Origin: %s"),
						*Mesh->Bounds.Origin.ToString());

					UE_LOG(LogTemp, Warning,
						TEXT("  Bounds Extent: %s"),
						*Mesh->Bounds.BoxExtent.ToString());

					UE_LOG(LogTemp, Warning,
						TEXT("  CastShadow: %d"), Mesh->CastShadow);
				}
			}
		}
		LogObjectFlags(World, TEXT("After BeginPlay"));
	}
	if (!RenderTarget) {
		RenderTarget = FAssetLibrary::LoadAsset<UTextureRenderTarget2D>(
			TEXT("/Game/Construction/Thumbnail/ThumbnailRenderTarget")
		);
		if (!RenderTarget) {
			return false;
		}
	}
	return true;
}

void ThumbnailGenerator::Cleanup() {
	

	if (RenderTarget) {
		RenderTarget = nullptr;
	}
	if (World->HasBegunPlay()) {
		World->EndPlay(EEndPlayReason::Destroyed);
	}
	if (World->AreActorsInitialized()) {
		World->BeginTearingDown();
		World->CleanupWorld();
	}
	World->ClearFlags(RF_Standalone);

	LogObjectFlags(World, TEXT("After CleanupWorld"));

	World = nullptr;
}

bool ThumbnailGenerator::Render(const FString& Path, TObjectPtr<AActor> Actor) {
	// Spawn actor

	// Render and save image
	FImage Image;
	TArray64<uint8> PNGData;
	if (FImageUtils::GetRenderTargetImage(RenderTarget, Image)) {
		// Invert the alpha channel because SceneCapture captures in RGB with inv opacity in A
		if (Image.Format == ERawImageFormat::RGBA16F)
		{
			FFloat16Color* Pixels =
				reinterpret_cast<FFloat16Color*>(Image.RawData.GetData());

			const int64 PixelCount =
				int64(Image.SizeX) * int64(Image.SizeY);

			for (int64 i = 0; i < PixelCount; ++i)
			{
				Pixels[i].A = 1.0f - Pixels[i].A;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Image format: %d"), Image.Format);
		if (FImageUtils::CompressImage(PNGData, TEXT("png"), Image)) {
			FString FilePath = FPaths::ProjectSavedDir() + TEXT("Temp/thumbnail.png");
			FFileHelper::SaveArrayToFile(PNGData, *FilePath);
		}
	}

	// Cleanup actor

	return true;
}
