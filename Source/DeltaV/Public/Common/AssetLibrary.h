// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/ObjectLibrary.h"
#include "AssetLibrary.generated.h"

/**
 * 
 */
UCLASS()
class DELTAV_API UAssetLibrary : public UObject
{
	GENERATED_BODY()

public:
	UAssetLibrary();
	~UAssetLibrary();

	template <typename T>
	static T* LoadAsset(FWideStringView Path) {
		FSoftObjectPath ObjectPath(Path);
		T* Object = Cast<T>(ObjectPath.ResolveObject());
		if (Object == nullptr)
		{
			Object = Cast<T>(ObjectPath.TryLoad());
		}
		if (Object == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("Failed to load asset"));
		}
		return Object;
	}

	template <typename T>
	static T* LoadAsset(FAnsiStringView Path)
	{
		TStringBuilder<256> Wide;
		Wide << Path;
		return LoadAsset<T>(Wide);
	}

	template <typename T>
	static T* LoadAsset(FUtf8StringView Path)
	{
		TStringBuilder<256> Wide;
		Wide << Path;
		return LoadAsset<T>(Wide);
	}

	template <typename T>
	static T* LoadAsset(const FString& Path) { return LoadAsset<T>(FStringView(Path)); }

	template <typename T>
	static T* LoadAsset(const WIDECHAR* Path) { return LoadAsset<T>(FWideStringView(Path)); }

	template <typename T>
	static T* LoadAsset(const ANSICHAR* Path) { return LoadAsset<T>(FAnsiStringView(Path)); }
};
