// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DELTAV_API JsonUtil
{
public:
	JsonUtil();
	~JsonUtil();

	static bool WriteFile(FString FilePath, TSharedPtr<FJsonObject> JsonObject);

	static TSharedPtr<FJsonObject> ReadFile(FString FilePath, FStringFormatNamedArguments TemplateArguments = FStringFormatNamedArguments());
	static TSharedPtr<FJsonObject> FromString(FString Json, FStringFormatNamedArguments TemplateArguments = FStringFormatNamedArguments());

	static FVector Vector(const TSharedPtr<FJsonObject>& Json, const FString& FieldName);
	static void Vector(TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FVector& Vector);

	static FQuat Quat(const TSharedPtr<FJsonObject>& Json, const FString& FieldName);
	static void Quat(TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FQuat& Quat);

	static FRotator Rotator(const TSharedPtr<FJsonObject>& Json, const FString& FieldName);
	static void Rotator(TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FRotator& Rotator);
};
