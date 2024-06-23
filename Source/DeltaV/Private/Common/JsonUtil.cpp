// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/JsonUtil.h"

JsonUtil::JsonUtil()
{
}

JsonUtil::~JsonUtil()
{
}

TSharedPtr<FJsonObject> JsonUtil::ReadFile(FString FilePath) {
	FString JsonString;
	FFileHelper::LoadFileToString(JsonString, *FilePath);

	TSharedPtr<FJsonObject> Result;
	FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), Result);

	return Result;
}

bool JsonUtil::WriteFile(FString FilePath, TSharedPtr<FJsonObject> JsonObject) {
	FString JsonString;

	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString))) {
		return false;
	}

	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}


FVector JsonUtil::Vector(const TArray<TSharedPtr<FJsonValue>>& Values) {
	if (Values.Num() != 3) {
		return FVector();
	}
	
	return FVector(
		Values[0]->AsNumber(),
		Values[1]->AsNumber(),
		Values[2]->AsNumber()
	);
}

TArray<TSharedPtr<FJsonValue>> JsonUtil::Vector(const FVector& Vector) {
	TArray<TSharedPtr<FJsonValue>> Array;
	
	Array.Add(MakeShareable(new FJsonValueNumber(Vector.X)));
	Array.Add(MakeShareable(new FJsonValueNumber(Vector.Y)));
	Array.Add(MakeShareable(new FJsonValueNumber(Vector.Z)));

	return Array;
}

