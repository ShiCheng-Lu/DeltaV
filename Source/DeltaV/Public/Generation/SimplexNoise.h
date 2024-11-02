// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DELTAV_API SimplexNoise
{
public:
	static const int BitPatterns[8];
	
	
	static double ContributionOf(FVector Vertex, FVector Point);

	static double At(FVector Point);

};
