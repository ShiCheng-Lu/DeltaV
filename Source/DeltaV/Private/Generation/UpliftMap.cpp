// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/UpliftMap.h"

UpliftMap::UpliftMap()
{

}

UpliftMap::~UpliftMap()
{
}

void Generate() {
	


}

double UpliftMap::SimplexNoise(FVector Point) {
	// Coordinate skewing
	double Skew = (Point.X + Point.Y + Point.Z) / 3;
	Point = Point + Skew;

	// Simplicial subdivision (finding which simplex the point belongs to)
	FVector SkewedUnitHypercude = FVector(FMath::Floor(Point.X), FMath::Floor(Point.X), FMath::Floor(Point.X));
	FVector InternalCoordinates = Point - SkewedUnitHypercude;

	int minIndex = 0, midIndex = 1, maxIndex = 2;
	for (int index = 0; index < 3; ++index) {
		if (InternalCoordinates[index] > InternalCoordinates[maxIndex]) {
			maxIndex = index;
		}
		if (InternalCoordinates[index] < InternalCoordinates[minIndex]) {
			minIndex = index;
		}
	}

	midIndex = 3 - minIndex - maxIndex;
	InternalCoordinates;

	TArray<FVector> SimplexVertices;
	FVector SimplexVertex = FVector(0);
	SimplexVertices.Add(FVector(SimplexVertex));
	SimplexVertex[maxIndex] = 1;
	SimplexVertices.Add(FVector(SimplexVertex));
	SimplexVertex[midIndex] = 1;
	SimplexVertices.Add(FVector(SimplexVertex));
	SimplexVertex[minIndex] = 1;
	SimplexVertices.Add(FVector(SimplexVertex));

	// Gradient selection



	// Kernel summation


	return 4;
}
