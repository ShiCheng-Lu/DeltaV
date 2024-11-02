// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/SimplexNoise.h"

const int SimplexNoise::BitPatterns[] = {0x15, 0x38, 0x32, 0x2c, 0x0d, 0x13, 0x07, 0x2a};

double SimplexNoise::ContributionOf(FVector Vertex, FVector Point)
{
	FVector Distance = Point - Vertex;
	Distance = Distance - (Distance.X + Distance.Y + Distance.Z) / 6;

	double Contribution = 0.5 - Distance.SquaredLength();
	if (Contribution <= 0) {
		return 0.0;
	}

	FIntVector3 IntVertex = FIntVector3(Vertex);
	int BitPattern = 0, PatternIndex;
	for (int i = 0; i < 8; ++i) {
		PatternIndex = ((IntVertex.X >> i) & 1) << 2;
		PatternIndex += ((IntVertex.Y >> i) & 1) << 1;
		PatternIndex += ((IntVertex.Z >> i) & 1);
		BitPattern += SimplexNoise::BitPatterns[PatternIndex];
	}
	int b5 = (BitPattern >> 5) & 1;
	int b4 = (BitPattern >> 4) & 1;
	int b3 = (BitPattern >> 3) & 1;

	int b2 = (BitPattern >> 2) & 1; // 0 if [1] = 1, 1 if [2] = 0
	int Shift = BitPattern & 0b11;

	FVector ShiftedDistance;
	if (Shift == 1) {
		ShiftedDistance = FVector(Distance.Y, Distance.Z, Distance.X);
	}
	else if (Shift == 2) {
		ShiftedDistance = FVector(Distance.Z, Distance.X, Distance.Y);
	}
	else {
		ShiftedDistance = Distance;
	}

	double Result = (b5 == b3) ? ShiftedDistance.X : -ShiftedDistance.X;
	if (Shift == 0) {
		Result += (b5 == b4) ? ShiftedDistance.Y : -ShiftedDistance.Y;
		Result += (b5 != (b4 ^ b3)) ? ShiftedDistance.Z : -ShiftedDistance.Z;
	}
	else if (b2) {
		Result += (b5 == b4) ? ShiftedDistance.Y : -ShiftedDistance.Y;
	}
	else {
		Result += (b5 != (b4 ^ b3)) ? ShiftedDistance.Z : -ShiftedDistance.Z;
	}

	Contribution *= Contribution;
	return 8 * Contribution * Contribution * Result;
}

double SimplexNoise::At(FVector Point)
{
	FVector SkewedPoint = Point + (Point.X + Point.Y + Point.Z) / 3;
	// starts off at one corner of the simplex
	FVector SkewedSimplexCorner = FVector(
		FMath::Floor(SkewedPoint.X),
		FMath::Floor(SkewedPoint.Y),
		FMath::Floor(SkewedPoint.Z)
	);
	FVector InternalCoordinates = SkewedPoint - SkewedSimplexCorner;
	// find out which simplex of the cube the Point belongs to
	int MaxIndex, MidIndex, MinIndex;
	if (InternalCoordinates.X > InternalCoordinates.Y) {
		if (InternalCoordinates.Y > InternalCoordinates.Z) {
			MaxIndex = 0; MidIndex = 1; MinIndex = 2;
		}
		else if (InternalCoordinates.X > InternalCoordinates.Z) {
			MaxIndex = 0; MidIndex = 2; MinIndex = 1;
		}
		else { // InternalCoordinates.Z > InternalCoordinates.X
			MaxIndex = 2; MidIndex = 0; MinIndex = 1;
		}
	}
	else { // InternalCoordinates.Y > InternalCoordinates.X
		if (InternalCoordinates.X > InternalCoordinates.Z) {
			MaxIndex = 1; MidIndex = 0; MinIndex = 2;
		}
		else if (InternalCoordinates.Y > InternalCoordinates.Z) {
			MaxIndex = 1; MidIndex = 2; MinIndex = 0;
		}
		else {
			MaxIndex = 2; MidIndex = 1; MinIndex = 0;
		}
	}

	double Result = ContributionOf(SkewedSimplexCorner, SkewedPoint);
	SkewedSimplexCorner[MaxIndex] += 1; // move to next corner of the simplex
	Result += ContributionOf(SkewedSimplexCorner, SkewedPoint);
	SkewedSimplexCorner[MidIndex] += 1; // move to next corner of the simplex
	Result += ContributionOf(SkewedSimplexCorner, SkewedPoint);
	SkewedSimplexCorner[MinIndex] += 1; // move to next corner of the simplex
	Result += ContributionOf(SkewedSimplexCorner, SkewedPoint);

	return Result;
}
