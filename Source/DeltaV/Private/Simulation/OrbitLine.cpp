// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/OrbitLine.h"

#include "Components/SplineMeshComponent.h"

void UOrbitLine::Generate() {


	for (int i = 0; i < GetNumberOfSplinePoints(); ++i) {
		Body.Add(NewObject<USplineMeshComponent>());
		
		GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);

		// Body[i]->SetStaticMesh();
		// Body[i]->SetForwardAxis();
		Body[i]->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	}

	// GetSplinePointsPosition()
}