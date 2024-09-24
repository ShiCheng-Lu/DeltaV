// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/MultiTickActor.h"


void CustomTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) {
	(Target->*TickFunction)(DeltaTime);
}

FString CustomTickFunction::DiagnosticMessage() {
	return FString("CustomTickFunction");
}

FName DiagnosticContext(bool bDetailed) {
	return FName("CustomTickFunction");
}

// Sets default values
MultiTickActor::MultiTickActor()
{
	PostPhysicsTick.TickGroup = ETickingGroup::TG_PostPhysics;
	PostPhysicsTick.bCanEverTick = false;
	PostPhysicsTick.bStartWithTickEnabled = true;
	PostPhysicsTick.SetTickFunctionEnable(false);
	PostPhysicsTick.TickFunction = &MultiTickActor::TickPostPhysics;

	DuringPhysicsTick.TickGroup = ETickingGroup::TG_PostPhysics;
	DuringPhysicsTick.bCanEverTick = false;
	DuringPhysicsTick.bStartWithTickEnabled = true;
	DuringPhysicsTick.SetTickFunctionEnable(false);
	DuringPhysicsTick.TickFunction = &MultiTickActor::TickDuringPhysics;
}

void MultiTickActor::RegisterMultiTickActorTickFunctions(AActor* Actor, bool bRegister)
{
	if (bRegister)
	{
		if (PostPhysicsTick.bCanEverTick)
		{
			PostPhysicsTick.Target = this;
			PostPhysicsTick.SetTickFunctionEnable(PostPhysicsTick.bStartWithTickEnabled || PostPhysicsTick.IsTickFunctionEnabled());
			PostPhysicsTick.RegisterTickFunction(Actor->GetLevel());
		}
		if (DuringPhysicsTick.bCanEverTick) {
			DuringPhysicsTick.Target = this;
			DuringPhysicsTick.SetTickFunctionEnable(PostPhysicsTick.bStartWithTickEnabled || PostPhysicsTick.IsTickFunctionEnabled());
			DuringPhysicsTick.RegisterTickFunction(Actor->GetLevel());
		}
	}
	else
	{
		if (PostPhysicsTick.IsTickFunctionRegistered())
		{
			PostPhysicsTick.UnRegisterTickFunction();
		}
		if (DuringPhysicsTick.IsTickFunctionEnabled()) {
			DuringPhysicsTick.UnRegisterTickFunction();
		}
	}
}
