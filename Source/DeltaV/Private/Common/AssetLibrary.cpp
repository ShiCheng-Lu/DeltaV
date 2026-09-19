// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/AssetLibrary.h"
#include "Common/JsonUtil.h"
#include "ImageUtils.h"

TMap<FString, UTexture2D*> FAssetLibrary::TextureCache;

FAssetLibrary::FAssetLibrary()
{
	
}

FAssetLibrary::~FAssetLibrary()
{

}

TSharedPtr<FJsonObject> FAssetLibrary::PartDefinition(FString PartName) {
	FString Path = FPaths::Combine(FPaths::ProjectContentDir(), "Parts", PartName + ".json");
	return JsonUtil::ReadFile(Path);
}

UTexture2D* FAssetLibrary::LoadTexture(const FString& Path) {
	UTexture2D* Texture;
	if (TextureCache.Contains(Path)) {
		Texture = TextureCache.FindChecked(Path);
		if (Texture) {
			return Texture;
		}
	}
	Texture = FImageUtils::ImportFileAsTexture2D(Path);
	TextureCache.Add({ Path, Texture });
	return Texture;
}

void FAssetLibrary::ClearTextureCache(const FString& Path) {
	TextureCache.Remove(Path);
}

void FAssetLibrary::ClearTextureCache() {
	TextureCache.Reset();
}