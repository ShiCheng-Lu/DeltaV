// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/MeshGeneration.h"
#include "Generation/SimplexNoise.h"
#include "DrawDebugHelpers.h"

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
	GenerateIsoSphere(MeshInOut, 5);

	for (int VertexId : MeshInOut.VertexIndicesItr()) {
		FVector Vertex = MeshInOut.GetVertex(VertexId);
		double Noise = SimplexNoise::At(Vertex);
		double Scale = Noise / 2 + 1;
		UpliftMap.Add(VertexId, Noise);
		HeightMap.Add(VertexId, Radius * Scale);
		MeshInOut.SetVertex(VertexId, Vertex.GetSafeNormal() * Radius * Scale);
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
	TMap<int, int> Path;
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

		if (Path.Find(High) == nullptr) {
			Path.Add(High, Low);
			DrainageMap.Add(High, 0);
		} else if (HeightMap[Low] < HeightMap[Path[High]]) {
			Path[High] = Low;
		}
	}
	
	// construct a connected tree
	// start from edges and work our way up
	TMap<int, TArray<int>> Tree;
	TSet<int> MinimumsSet;
	for (auto& KVP : Path) {
		Tree.FindOrAdd(KVP.Value).Add(KVP.Key);
		if (Path.Find(KVP.Value) == nullptr && HeightMap[KVP.Value] > Radius) { // the downstream doesn't have a downstream and doens't lead to the ocean, so it must be a minimum
			MinimumsSet.Add(KVP.Value);
		}
	}
	TArray<int> Minimums = MinimumsSet.Array();

	// connect up lakes
	TMap<int, int> Lake;
	for (int LakeId = 0; LakeId < Minimums.Num(); ++LakeId) {
		// traverse the tree from the minimum to mark as this lake
		TArray<int> Vertices = { Minimums[LakeId]};
		while (!Vertices.IsEmpty()) {
			int Vertex = Vertices.Pop();
			Lake.Add(Vertex, LakeId);
			if (Tree.Find(Vertex) != nullptr) {
				Vertices.Append(Tree[Vertex]);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("lake base %s"), *MeshInOut.GetVertex(Minimums[LakeId]).ToString());
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
			else if (ContainA) {
				Tree.FindOrAdd(Pass.Pass.A).Add(Minimums[Pass.Lakes.B]);
				Connected.Add(Pass.Lakes.B);

				FVector A = MeshInOut.GetVertex(Minimums[Pass.Lakes.B]);
				FVector B = MeshInOut.GetVertex(Pass.Pass.A);
				DrawDebugDirectionalArrow(GetWorld(), A, B, 1, FColor::Purple, true);
			}
			else if (ContainB) {
				Tree.FindOrAdd(Pass.Pass.B).Add(Minimums[Pass.Lakes.A]);
				Connected.Add(Pass.Lakes.A);

				FVector A = MeshInOut.GetVertex(Minimums[Pass.Lakes.A]);
				FVector B = MeshInOut.GetVertex(Pass.Pass.B);
				DrawDebugDirectionalArrow(GetWorld(), A, B, 1, FColor::Purple, true);
			}
		}
		OverflowPasses = NextSetOverflowPasses;
	}

	for (auto& KVP : Tree) {
		for (auto& Vertices : KVP.Value) {
			FVector A = MeshInOut.GetVertex(KVP.Key);
			FVector B = MeshInOut.GetVertex(Vertices);
			DrawDebugDirectionalArrow(GetWorld(), A, B, 1, FColor::Red, true);
		}
	}
	/*
	for (auto& KVP : Path) {
		FVector A = MeshInOut.GetVertex(KVP.Key);
		FVector B = MeshInOut.GetVertex(KVP.Value);
		DrawDebugDirectionalArrow(GetWorld(), A, B, 1, FColor::Red, true);
	}
	*/
	for (auto Vertex : Roots) {
		FVector A = MeshInOut.GetVertex(Vertex);
		DrawDebugPoint(GetWorld(), A, 10, FColor::Orange, true);
	}
	for (auto Vertex : Minimums) {
		FVector A = MeshInOut.GetVertex(Vertex);
		DrawDebugPoint(GetWorld(), A, 20, FColor::Green, true);
	}

	TArray<FColor> Colors = { FColor::Yellow, FColor::Blue, FColor::Purple, FColor::Cyan };
	for (auto& KVP : Lake) {
		FVector A = MeshInOut.GetVertex(KVP.Key);
		if (KVP.Value < 4) {
			DrawDebugPoint(GetWorld(), A, 10, Colors[KVP.Value], true);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Path size %d - lake num %d"), Path.Num(), Minimums.Num());

	// DFS traverse the tree to generate the drainage area

}

void UMeshGeneration::Generate(FDynamicMesh3& MeshInOut) {
	for (int VertexId : MeshInOut.VertexIndicesItr()) {
		FVector Vertex = MeshInOut.GetVertex(VertexId);
		MeshInOut.SetVertex(VertexId, Vertex.GetSafeNormal() * HeightMap[VertexId]);
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
		MeshInOut.AppendTriangle(Face);
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
