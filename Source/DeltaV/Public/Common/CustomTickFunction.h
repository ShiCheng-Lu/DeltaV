// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "ComponentTickFunction.generated.h"

template<class T>
struct FCustomComponentTick : public FActorComponentTickFunction {
	T* Target;

	void (T::* TickFunction)(float);

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override {
		(Target->*TickFunction)(DeltaTime);
	}
	virtual FString DiagnosticMessage() override {
		return FString("CustomComponentTickFunction");
	}
	virtual FName DiagnosticContext(bool bDetailed) override {
		return FName("CustomComponentTickFunction");
	}

	FCustomComponentTick() {}

	FCustomComponentTick(ETickingGroup TickingGroup, void (T::*Function)(float)) {
		TickGroup = TickingGroup;
		bCanEverTick = true;
		bStartWithTickEnabled = true;
		TickFunction = Function;
	}

	void RegisterTickFunctions(T* Actor, bool bRegister) {
		if (bRegister)
		{
			if (bCanEverTick) {
				Target = Actor;
				SetTickFunctionEnable(bStartWithTickEnabled || IsTickFunctionEnabled());
				RegisterTickFunction(Actor->GetLevel());
			}
		}
		else
		{
			if (IsTickFunctionEnabled()) {
				UnRegisterTickFunction();
			}
		}
	}
};
/*
template<class T>
struct TStructOpsTypeTraits<FCustomComponentTick<T>> : public TStructOpsTypeTraitsBase2<FCustomComponentTick>
{
	enum
	{
		WithCopy = false
	};
};
*/
template<class T>
struct FCustomActorTick : public FActorTickFunction {
	T* Target;

	void (T::* TickFunction)(float);

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override {
		(Target->*TickFunction)(DeltaTime);
	}
	virtual FString DiagnosticMessage() override {
		return FString("CustomComponentTickFunction");
	}
	virtual FName DiagnosticContext(bool bDetailed) override {
		return FName("CustomComponentTickFunction");
	}

	FCustomActorTick() {}

	FCustomActorTick(ETickingGroup TickingGroup, void (T::* Function)(float)) {
		TickGroup = TickingGroup;
		bCanEverTick = true;
		bStartWithTickEnabled = true;
		// SetTickFunctionEnable(true);
		TickFunction = Function;
	}

	void RegisterTickFunctions(T* Actor, bool bRegister) {
		if (bRegister)
		{
			if (bCanEverTick) {
				Target = Actor;
				SetTickFunctionEnable(bStartWithTickEnabled || IsTickFunctionEnabled());
				RegisterTickFunction(Actor->GetLevel());
			}
		}
		else
		{
			if (IsTickFunctionEnabled()) {
				UnRegisterTickFunction();
			}
		}
	}
};


