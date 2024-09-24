// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"


struct CustomTickFunction : public FActorTickFunction {

	class MultiTickActor* Target;

	void (MultiTickActor::*TickFunction)(float);

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override {
		(Target->*TickFunction)(DeltaTime);
	}
	virtual FString DiagnosticMessage() override {
		return FString("CustomTickFunction");
	}
	virtual FName DiagnosticContext(bool bDetailed) override {
		return FName("CustomTickFunction");
	}
};



/*
Adds Post Physics and During Physics ticks to an actor
*/
class DELTAV_API MultiTickActor
{
public:
	CustomTickFunction PostPhysicsTick;
	CustomTickFunction DuringPhysicsTick;

	MultiTickActor();

	virtual void TickPostPhysics(float DeltaTime) = 0;

	virtual void TickDuringPhysics(float DeltaTime) = 0;


	void RegisterMultiTickActorTickFunctions(AActor* Actor, bool bRegister);

	/* Override the Actor's RegisterActorTickFunction like this
	virtual void RegisterActorTickFunctions(bool bRegister) override {
		Super::RegisterActorTickFunctions(bRegister);
		RegisterMultiTickActorTickFunctions(this, bRegister);
	}
	*/
};
