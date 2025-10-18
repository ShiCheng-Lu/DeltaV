// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DELTAV_API Erosion
{
public:
	Erosion();
	~Erosion();

	TMap<int, double> Height;
	TMap<int, FVector> Position;
	
	TMap<int, double> Drainage;
	TMap<int, FVector> Velocity;

	TMap<int, double> NextDrainage;
	TMap<int, FVector> NextVelocity;

	TArray<int> Vertices;
	TArray<TTuple<int, int, int>> Edges;
	TMap<int, FVector> EdgeDirection;
	TMap<int, TArray<int>> Neighbours;

	// iteration variables, should be reset for each step
	TMap<int, FVector> Gradient;


	void CalculateGradient();

	void Step();
};
