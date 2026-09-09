// Fill out your copyright notice in the Description page of Project Settings.


#include "Construction/ThumbnailGenerator.h"

#include "Common/AssetLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"

ThumbnailGenerator::ThumbnailGenerator()
{
}

ThumbnailGenerator::~ThumbnailGenerator()
{
}

void ThumbnailGenerator::GenerateThumbnail(const FString& Path, TObjectPtr<AActor> Actor) {
	TObjectPtr<UWorld> World = CreateWorld();


	World->DestroyWorld(false);
}

void ThumbnailGenerator::GenerateThumbnails(TMap<FString, TObjectPtr<AActor>> FilePathToActors) {
	TObjectPtr<UWorld> World = CreateWorld();
	if (!World) {
		return;
	}


}

TObjectPtr<UWorld> ThumbnailGenerator::CreateWorld() {
	UWorld* World = FAssetLibrary::LoadAsset<UWorld>(
		TEXT("/Game/Construction/Thumbnail/ThumbnailWorld")
	);
	if (World) {
		World->WorldType = EWorldType::Game;
		World->InitWorld(FWorldInitializationValues()
			.AllowAudioPlayback(false)
			.RequiresHitProxies(false)
			.CreatePhysicsScene(false)
			.CreateNavigation(false)
			.CreateAISystem(false)
			.ShouldSimulatePhysics(false)
		);
	}
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	return World;
}

void ThumbnailGenerator::DestroyWorld(TObjectPtr<UWorld> World) {
	World->BeginTearingDown();
	World->CleanupWorld();
}

void ThumbnailGenerator::SpawnActors(TObjectPtr<UWorld> World) {
	
}

void ThumbnailGenerator::Render(TObjectPtr<UWorld> World) {
	UTextureRenderTarget2D* RenderTarget = FAssetLibrary::LoadAsset<UTextureRenderTarget2D>(
		TEXT("/Game/Construction/Thumbnail/ThumbnailRenderTarget")
	);
	if (!RenderTarget) {
		return;
	}
	
	FImage Image;
	if (!FImageUtils::GetRenderTargetImage(RenderTarget, Image)) {
		return;
	}

	TArray64<uint8> PNGData;
	if (!FImageUtils::CompressImage(PNGData, TEXT("png"), Image)) {
		return;
	}

	FFileHelper::SaveArrayToFile(PNGData, TEXT("/Game/Temp/thumbnail.png"));
}

void ThumbnailGenerator::SaveImage(TObjectPtr<UWorld> World) {

}
