// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/AssetLibrary.h"
#include "Common/JsonUtil.h"

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

