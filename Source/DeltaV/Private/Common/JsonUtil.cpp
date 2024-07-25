// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/JsonUtil.h"

JsonUtil::JsonUtil()
{
}

JsonUtil::~JsonUtil()
{
}

TSharedPtr<FJsonObject> JsonUtil::ReadFile(FString FilePath, FStringFormatNamedArguments TemplateArguments) {
	FString JsonString;
	FFileHelper::LoadFileToString(JsonString, *FilePath);

	return FromString(JsonString, TemplateArguments);
}

bool JsonUtil::WriteFile(FString FilePath, TSharedPtr<FJsonObject> JsonObject) {
	FString JsonString;

	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString))) {
		return false;
	}

	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

TSharedPtr<FJsonObject> JsonUtil::FromString(FString JsonString, FStringFormatNamedArguments TemplateArguments) {
	if (!TemplateArguments.IsEmpty()) {
		JsonString = FString::Format(*JsonString, TemplateArguments);
	}

	TSharedPtr<FJsonObject> Result;
	FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), Result);

	return Result;
}

FVector JsonUtil::Vector(const TSharedPtr<FJsonObject>& Json, const FString& FieldName) {
	TArray<TSharedPtr<FJsonValue>> Values = Json->GetArrayField(FieldName);

	if (Values.Num() != 3) {
		return FVector();
	}
	
	return FVector(
		Values[0]->AsNumber(),
		Values[1]->AsNumber(),
		Values[2]->AsNumber()
	);
}

void JsonUtil::Vector(TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FVector& Vector) {
	TArray<TSharedPtr<FJsonValue>> Array;
	
	Array.Add(MakeShareable(new FJsonValueNumber(Vector.X)));
	Array.Add(MakeShareable(new FJsonValueNumber(Vector.Y)));
	Array.Add(MakeShareable(new FJsonValueNumber(Vector.Z)));

	Json->SetArrayField(FieldName, Array);
}

FQuat JsonUtil::Quat(const TSharedPtr<FJsonObject>& Json, const FString& FieldName) {
	TArray<TSharedPtr<FJsonValue>> Values = Json->GetArrayField(FieldName);

	if (Values.Num() != 3) {
		return FQuat();
	}

	return FQuat(
		Values[0]->AsNumber(),
		Values[1]->AsNumber(),
		Values[2]->AsNumber(),
		Values[3]->AsNumber()
	);
}

void JsonUtil::Quat(TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FQuat& Quat) {
	TArray<TSharedPtr<FJsonValue>> Array;

	Array.Add(MakeShareable(new FJsonValueNumber(Quat.X)));
	Array.Add(MakeShareable(new FJsonValueNumber(Quat.Y)));
	Array.Add(MakeShareable(new FJsonValueNumber(Quat.Z)));
	Array.Add(MakeShareable(new FJsonValueNumber(Quat.W)));

	Json->SetArrayField(FieldName, Array);
}