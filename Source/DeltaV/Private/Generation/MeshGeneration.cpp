// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/MeshGeneration.h"
#include "Generation/SimplexNoise.h"
#include "DrawDebugHelpers.h"
#include "DynamicMesh/MeshAttributeUtil.h"

using UE::Geometry::FIndex2i;

UMeshGeneration::UMeshGeneration()
{
}

UMeshGeneration::~UMeshGeneration()
{
}

void StreamTree(FDynamicMesh3& MeshInOut) {
	TMap<int, int> Tree;
	for (auto Edge : MeshInOut.EdgesItr()) {
		Edge.Vert.A;
		Edge.Vert.B;

	}

}

void UMeshGeneration::Initialize(FDynamicMesh3& MeshInOut) {
	MeshInOut.EnableAttributes();
	MeshInOut.Attributes()->EnableMaterialID();

	GenerateIsoSphere(MeshInOut, 6);

	for (int VertexId : MeshInOut.VertexIndicesItr()) {
		FVector Vertex = MeshInOut.GetVertex(VertexId);
		double Noise = SimplexNoise::At(Vertex) + 0.02;
		UpliftMap.Add(VertexId, Noise);
		double Noise2 = SimplexNoise::At(Vertex * 2);
		double Noise3 = SimplexNoise::At(Vertex * 4);
		HeightMap.Add(VertexId, Radius * (1 + Noise / 2 + Noise2 / 8 + Noise3 / 32));
		MeshInOut.SetVertex(VertexId, Vertex.GetSafeNormal() * HeightMap[VertexId]);
	}
}

struct OverflowPass {
	FIndex2i Lakes;
	FIndex2i Pass;
	double PassHeight = INFINITY;

	inline bool operator<(const OverflowPass& Other) const {
		return PassHeight < Other.PassHeight;
	}
};

void UMeshGeneration::Iterate(FDynamicMesh3& MeshInOut) {
	// which vertex is the lowest from the current node
	TMap<int, int> DownstreamMap;
	TMap<int, int> DrainageMap;
	TArray<int> Roots;

	for (auto Edge : MeshInOut.EdgesItr()) {
		int High, Low;
		if (HeightMap[Edge.Vert.A] > HeightMap[Edge.Vert.B]) {
			High = Edge.Vert.A; Low = Edge.Vert.B;
		}
		else {
			High = Edge.Vert.B; Low = Edge.Vert.A;
		}

		if (HeightMap[High] <= Radius) {
			continue;
		}
		if (HeightMap[Low] <= Radius) {
			Roots.Add(Low);
		}

		if (DownstreamMap.Find(High) == nullptr) {
			DownstreamMap.Add(High, Low);
			DrainageMap.Add(High, 0);
		}
		else if (HeightMap[Low] < HeightMap[DownstreamMap[High]]) {
			DownstreamMap[High] = Low;
		}
	}

	// construct a connected tree
	// start from edges and work our way up
	TMap<int, TArray<int>> Tree;
	TSet<int> MinimumsSet;
	for (auto& KVP : DownstreamMap) {
		Tree.FindOrAdd(KVP.Value).Add(KVP.Key);
		if (!DownstreamMap.Contains(KVP.Value) && HeightMap[KVP.Value] > Radius) { // the downstream doesn't have a downstream and doens't lead to the ocean, so it must be a minimum
			MinimumsSet.Add(KVP.Value);
		}
	}
	TArray<int> Minimums = MinimumsSet.Array();

	// connect up lakes
	TMap<int, int> Lake;
	for (int LakeId = 0; LakeId < Minimums.Num(); ++LakeId) {
		// traverse the tree from the minimum to mark as this lake
		TArray<int> Vertices = { Minimums[LakeId] };
		while (!Vertices.IsEmpty()) {
			int Vertex = Vertices.Pop();
			Lake.Add(Vertex, LakeId);
			if (Tree.Find(Vertex) != nullptr) {
				Vertices.Append(Tree[Vertex]);
			}
		}
	}

	TMap<FIndex2i, OverflowPass> OverflowPassesMap;
	for (auto Edge : MeshInOut.EdgesItr()) {
		int* LakeAPtr = Lake.Find(Edge.Vert.A);
		int* LakeBPtr = Lake.Find(Edge.Vert.B);
		int LakeA = (LakeAPtr == nullptr) ? -1 : *LakeAPtr;
		int LakeB = (LakeBPtr == nullptr) ? -1 : *LakeBPtr;
		if (LakeA == LakeB) {
			continue;
		}
		// check if a overflow pass already exist between the lakes, if not, add it
		// if it does exist, replace if this overflow pass is more optimal
		FIndex2i Index = FIndex2i(LakeA, LakeB);
		Index.Sort();
		OverflowPass& Pass = OverflowPassesMap.FindOrAdd(Index);
		double PassHeight = FMath::Max(HeightMap[Edge.Vert.A], HeightMap[Edge.Vert.B]);
		if (PassHeight < Pass.PassHeight) {
			Pass.Lakes = FIndex2i(LakeA, LakeB);
			Pass.Pass = Edge.Vert;
			Pass.PassHeight = PassHeight;
		}
	}

	TArray<OverflowPass> OverflowPasses;
	OverflowPassesMap.GenerateValueArray(OverflowPasses);
	OverflowPasses.Sort();

	TSet<int> Connected = { -1 };
	while (!OverflowPasses.IsEmpty()) {
		TArray<OverflowPass> NextSetOverflowPasses;
		for (OverflowPass& Pass : OverflowPasses) {
			bool ContainA = Connected.Contains(Pass.Lakes.A);
			bool ContainB = Connected.Contains(Pass.Lakes.B);
			if (!ContainA && !ContainB) {
				NextSetOverflowPasses.Add(Pass);
			}
			else if (ContainA && ContainB) {
			}
			else if (ContainA) {
				Tree.FindOrAdd(Pass.Pass.A).Add(Minimums[Pass.Lakes.B]);
				DownstreamMap.Add(Minimums[Pass.Lakes.B], Pass.Pass.A);
				Connected.Add(Pass.Lakes.B);
			}
			else if (ContainB) {
				Tree.FindOrAdd(Pass.Pass.B).Add(Minimums[Pass.Lakes.A]);
				DownstreamMap.Add(Minimums[Pass.Lakes.A], Pass.Pass.B);
				Connected.Add(Pass.Lakes.A);
			}
		}
		OverflowPasses = NextSetOverflowPasses;
	}

	// since our graph is an iso sphere, most of the drainage area will be the same size
	// we can reuse the same for approximation. Hexagon with side length of ~ Radius / (5 * (2 ^ SubdivideLevel))
	double DrainageAreaConstant = 3 * FMath::Sqrt(3.0) / 8; // area of single drainage
	double ErosionRate = 0.01408361 * 3;

	// using Roots as the stack for DFS to traverse the entire stream map
	TArray<int> TreeInBFS = TArray(Roots);
	for (int i = 0; i < TreeInBFS.Num(); ++i) {
		TArray<int>* Upstreams = Tree.Find(TreeInBFS[i]);
		if (Upstreams != nullptr) {
			TreeInBFS.Append(*Upstreams);
		}
	}

	// traverse in reverse order and calculate drainage area and height change
	TMap<int, int> DrainageAreaMap;
	TMap<int, double> Deltas;

	for (int i = TreeInBFS.Num() - 1; i >= Roots.Num(); --i) {
		int Vertex = TreeInBFS[i];
		int DrainageArea = 1;
		TArray<int>* Upstreams = Tree.Find(Vertex);
		if (Upstreams != nullptr) {
			for (int Upstream : *Upstreams) {
				DrainageArea += DrainageAreaMap[Upstream];
			}
		}
		DrainageAreaMap.Add(Vertex, DrainageArea);

		// try using the current height gradient
		// maybe this needs to be done separately afterwards to solve with next step gradient
		double Slope = (HeightMap[Vertex] - HeightMap[DownstreamMap[Vertex]]) / 3;

		double Delta = UpliftMap[Vertex] / 8 - ErosionRate * FMath::Sqrt(DrainageArea * DrainageAreaConstant) * Slope;
		if (Delta + HeightMap[Vertex] > Radius) {
			HeightMap[Vertex] += Delta / 32;
		}
		Deltas.Add(Vertex, Delta);
	}

	FlushPersistentDebugLines(GetWorld());
	for (auto& KVP : DownstreamMap) {
		FVector A = MeshInOut.GetVertex(KVP.Key);
		FVector B = MeshInOut.GetVertex(KVP.Value);

		/*
		if (Deltas[KVP.Key] > 0) {
			DrawDebugPoint(GetWorld(), A, 10, FColor::Green, true);
		}*/

		if (Lake.Find(KVP.Key) != nullptr) {
			DrawDebugDirectionalArrow(GetWorld(), A, B, 1, FColor::Purple, true);
		} else {
			DrawDebugDirectionalArrow(GetWorld(), A, B, 1, FColor::Red, true);
		}
	}
	for (auto Vertex : Roots) {
		FVector A = MeshInOut.GetVertex(Vertex);
		DrawDebugPoint(GetWorld(), A, 10, FColor::Orange, true);
	}
	for (auto Vertex : Minimums) {
		FVector A = MeshInOut.GetVertex(Vertex);
		DrawDebugPoint(GetWorld(), A, 20, FColor::Green, true);
	}

	double maxUplift = 0;
	for (auto& KVP : UpliftMap) {
		maxUplift = FMath::Max(KVP.Value, maxUplift);
	}
}

void UMeshGeneration::Generate(FDynamicMesh3& MeshInOut) {
	double MaxHeight = 0;
	for (int VertexId : MeshInOut.VertexIndicesItr()) {
		FVector Vertex = MeshInOut.GetVertex(VertexId);
		double Height = HeightMap[VertexId];
		MeshInOut.SetVertex(VertexId, Vertex.GetSafeNormal() * Height);
		MeshInOut.SetVertexNormal(VertexId, FVector3f(Vertex.GetSafeNormal()));
		MeshInOut.SetVertexColor(VertexId, FVector3f(0, 0, 0));
		MaxHeight = FMath::Max(MaxHeight, Height);
	}
	MaxHeight -= Radius;
	int id = 0;
	for (int TriangleId : MeshInOut.TriangleIndicesItr()) {
		FIndex3i Triangle = MeshInOut.GetTriangle(TriangleId);
		double TotalHeight = HeightMap[Triangle.A] + HeightMap[Triangle.B] + HeightMap[Triangle.C];
		int GroupId = (TotalHeight / 3 - Radius) / MaxHeight * 5 + 1; // 0 for negative

		// Any material id or vertex color needs to be set on attributes (haven't tried vertex color)
		MeshInOut.Attributes()->GetMaterialID()->SetValue(TriangleId, FMath::Clamp(GroupId, 0, 5));
	}
}

void UMeshGeneration::GenerateIsoSphere(FDynamicMesh3 & MeshInOut, int Subdivisions) {
	double t = (1.0 + FMath::Sqrt(5.0)) / 2.0;

	double InvRoot5 = 1 / FMath::Sqrt(5.0);
	double Half1MInvRoot5 = (1 - InvRoot5) / 2;
	double Half1PInvRoot5 = (1 + InvRoot5) / 2;

	FVector Top = FVector(0, 0, 1);

	TArray<FVector> TopLayer = {
		FVector( 2 * InvRoot5,	  0,				            InvRoot5),
		FVector( Half1MInvRoot5,  FMath::Sqrt(Half1PInvRoot5),  InvRoot5),
		FVector(-Half1PInvRoot5,  FMath::Sqrt(Half1MInvRoot5),  InvRoot5),
		FVector(-Half1PInvRoot5, -FMath::Sqrt(Half1MInvRoot5),  InvRoot5),
		FVector( Half1MInvRoot5, -FMath::Sqrt(Half1PInvRoot5),  InvRoot5),
	};

	TArray<FVector> BotLayer = {
		FVector(-2 * InvRoot5,    0,        				   -InvRoot5),
		FVector(-Half1MInvRoot5, -FMath::Sqrt(Half1PInvRoot5), -InvRoot5),
		FVector( Half1PInvRoot5, -FMath::Sqrt(Half1MInvRoot5), -InvRoot5),
		FVector( Half1PInvRoot5,  FMath::Sqrt(Half1MInvRoot5), -InvRoot5),
		FVector(-Half1MInvRoot5,  FMath::Sqrt(Half1PInvRoot5), -InvRoot5),
	};

	FVector Bot = FVector(0, 0, -1);

	int TopId = MeshInOut.AppendVertex(Top);
	TArray<int> TopLayerId;
	for (FVector& Vertex : TopLayer) {
		TopLayerId.Add(MeshInOut.AppendVertex(Vertex));
	}
	TArray<int> BotLayerId;
	for (FVector& Vertex : BotLayer) {
		BotLayerId.Add(MeshInOut.AppendVertex(Vertex));
	}
	int BotId = MeshInOut.AppendVertex(Bot);

	// Generate Faces
	for (int i = 0; i < 5; ++i) {
		// Faces index are wrapped around, and faces direction are clockwise to have the normal pointing outwards
		// Create face with Top, TopLayer[i], TopLayer[i + 1]
		Subdivide(MeshInOut, { TopId, TopLayerId[(i + 1) % 5], TopLayerId[i] }, Subdivisions);
		// Create face with Bot, BotLayer[i], BotLayer[i + 1]
		Subdivide(MeshInOut, { BotId, BotLayerId[i], BotLayerId[(i + 1) % 5] }, Subdivisions);
		// Create face with TopLayer[i], BotLayer[i + 2], BotLayer[i + 3]
		Subdivide(MeshInOut, { TopLayerId[i], BotLayerId[(i + 3) % 5], BotLayerId[(i + 2) % 5] }, Subdivisions);
		// Create face with BotLayer[i], TopLayer[i], TopLayer[(i + 1) % 5]
		Subdivide(MeshInOut, { BotLayerId[i], TopLayerId[(i + 2) % 5], TopLayerId[(i + 3) % 5] }, Subdivisions);
	}
}

int UMeshGeneration::GetMidpoint(FDynamicMesh3& MeshInOut, int a, int b) {
	TPair<int, int> Key = (a < b) ? TPair<int, int>(a, b) : TPair<int, int>(b, a);
	int* Midpoint = MidpointMap.Find(Key);
	if (Midpoint) {
		return *Midpoint;
	}
	// Add to the dynamic mesh if the vertex doesn't exist
	FVector MidpointVertex = (MeshInOut.GetVertex(a) + MeshInOut.GetVertex(b)) / 2;
	MidpointVertex.Normalize();
	int MidpointId = MeshInOut.AppendVertex(MidpointVertex);
	MidpointMap.Add(Key, MidpointId);
	
	return MidpointId;
}

void UMeshGeneration::Subdivide(FDynamicMesh3& MeshInOut, FIndex3i Face, int Depth) {
	if (Depth <= 0) { // Last depth, Add face to dynamic mesh
		MeshInOut.AppendTriangle(Face, 2);
		return;
	}
	int AB = GetMidpoint(MeshInOut, Face.A, Face.B);
	int BC = GetMidpoint(MeshInOut, Face.B, Face.C);
	int CA = GetMidpoint(MeshInOut, Face.C, Face.A);

	Subdivide(MeshInOut, { CA, Face.A, AB }, Depth - 1);
	Subdivide(MeshInOut, { AB, Face.B, BC }, Depth - 1);
	Subdivide(MeshInOut, { BC, Face.C, CA }, Depth - 1);
	Subdivide(MeshInOut, { AB, BC, CA }, Depth - 1);
}
