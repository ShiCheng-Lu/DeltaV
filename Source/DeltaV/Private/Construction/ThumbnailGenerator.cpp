// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ThumbnailGenerator.h"

#include "Common/AssetLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "ImageCore.h"

#include "Engine/SceneCapture2D.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Components/SceneCaptureComponent2D.h"
#include "RenderingThread.h"

ThumbnailGenerator::ThumbnailGenerator()
{
}

ThumbnailGenerator::~ThumbnailGenerator()
{
}

void ThumbnailGenerator::GenerateThumbnail(const FString& Path, FSpawnActor SpawnActor) {
	return GenerateThumbnails({ Path }, SpawnActor);
}

void ThumbnailGenerator::GenerateThumbnails(TArray<FString> Paths, FSpawnActor SpawnActor) {
	if (!SpawnActor.IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("Spawn function not bound"));
		return;
	}

	if (!Initialize()) {
		UE_LOG(LogTemp, Warning, TEXT("Initialize failed"));
		return;
	}

	for (auto& Path : Paths) {
		Render(Path, SpawnActor);
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

		if (!World->AreActorsInitialized()) {
			World->InitializeActorsForPlay(FURL());
		}
		if (!World->HasBegunPlay()) {
			World->BeginPlay();
		}
	}
	if (!CaptureComponent) {
		ASceneCapture2D* SceneCapture = nullptr;
		for (TActorIterator<ASceneCapture2D> It(World); It; ++It)
		{
			CaptureComponent = It->GetCaptureComponent2D();
		}
		if (!CaptureComponent) {
			UE_LOG(LogTemp, Warning, TEXT("No capture component found"));
			return false;
		}
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

	World = nullptr;
}

bool ThumbnailGenerator::Render(const FString& Path, FSpawnActor SpawnActor) {
	// Spawn actor
	if (!SpawnActor.IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("Actor spawning method is unbound"));
		return false;
	}
	TObjectPtr<AActor> Actor = SpawnActor.Execute(World, Path);

	// Fit actor into the camera
	FQuat CameraRotation = CaptureComponent->GetComponentQuat();
	FQuat CameraInverse = CameraRotation.Inverse();

	double MinX = INFINITY, MinY = INFINITY, MinZ = INFINITY;
	double MaxY = -INFINITY, MaxZ = -INFINITY;
	TArray<UPrimitiveComponent*> Components;
	Actor->GetComponents<UPrimitiveComponent>(Components);

	FVector ActorCenter = Actor->GetActorLocation();
	for (UPrimitiveComponent* Component : Components)
	{
		FBox Box = Component->Bounds.GetBox();
		FVector Vertices[8];
		Box.GetVertices(Vertices);

		for (FVector& Vertex : Vertices) {
			FVector Projection = CameraInverse.RotateVector(Vertex - ActorCenter);
			MinY = FMath::Min(MinY, Projection.Y);
			MaxY = FMath::Max(MaxY, Projection.Y);
			MinZ = FMath::Min(MinZ, Projection.Z);
			MaxZ = FMath::Max(MaxZ, Projection.Z);
			MinX = FMath::Min(MinX, Projection.X);
		}
	}
	FVector CenterOffset = FVector(-MinX, -(MinY + MaxY) / 2, -(MinZ + MaxZ) / 2);
	double Scale = 1000 / FMath::Max(MaxY - MinY, MaxZ - MinZ);

	Actor->SetActorLocation(CameraRotation.RotateVector(CenterOffset) * Scale);
	Actor->SetActorScale3D(FVector(Scale));

	// Capture the scene
	CaptureComponent->CaptureScene();
	FlushRenderingCommands();

	// Render and save image
	FImage Image;
	TArray64<uint8> PNGData;
	if (FImageUtils::GetRenderTargetImage(RenderTarget, Image)) {
		// Invert the alpha channel because SceneCapture captures in RGB with inv opacity in A
		if (Image.Format == ERawImageFormat::RGBA16F) {
			FFloat16Color* Pixels = reinterpret_cast<FFloat16Color*>(Image.RawData.GetData());
			const int64 PixelCount = int64(Image.SizeX) * int64(Image.SizeY);

			for (int64 i = 0; i < PixelCount; ++i) {
				Pixels[i].A = 1.0f - Pixels[i].A;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Image format: %d"), Image.Format);
		if (FImageUtils::CompressImage(PNGData, TEXT("png"), Image)) {
			FString FilePath = FPaths::ProjectSavedDir() + TEXT("Temp/thumbnail.png");
			FFileHelper::SaveArrayToFile(PNGData, *FilePath);
			UE_LOG(LogTemp, Warning, TEXT("Saved image"));
		}
	}

	// Cleanup actor
	Actor->Destroy();

	return true;
}
