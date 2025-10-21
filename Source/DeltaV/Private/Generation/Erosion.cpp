// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/Erosion.h"

Erosion::Erosion()
{



}

Erosion::~Erosion()
{
}

void Erosion::CalculateGradient() {
	Gradient.Reset();
	for (const auto& [Edge, VertexA, VertexB] : Edges) {
	 	FVector EdgeGradient = EdgeDirection[Edge] * (Height[VertexA] - Height[VertexB]);

		Gradient[VertexA] += EdgeGradient;
		Gradient[VertexB] += EdgeGradient;
	}
}

void Erosion::Step() {
	CalculateGradient();

	/*
	for (const auto& [Edge, VertexA, VertexB] : Edges) {
		Gradient[VertexA];
		EdgeDirection[Edge];

		Gradient[VertexB];
		EdgeDirection[Edge];
	}

	for (const int Vertex : Vertices) {
		
	}

	for (const auto& [Vertex, VertexNeighbours] : Neighbours) {

		// compute gradient

		// compute errosion

	}
	*/

}
