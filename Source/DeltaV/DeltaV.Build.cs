 // Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DeltaV : ModuleRules
{
	public DeltaV(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "GeometryFramework", 
			"GeometryCore", "DynamicMesh", "ImageWrapper", "ProceduralMeshComponent", "GeometryCollectionEngine",
            "PhysicsCore",
            "Chaos", "ChaosVehicles", "ChaosModularVehicle", "ChaosModularVehicleEngine", "ChaosVehiclesCore", "ChaosVehiclesEngine",
			"Niagara",
        });

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Slate", "SlateCore", "UMG", "Json", "PhysicsCore",
            "GeometryFramework", "GeometryCore", "GeometryScriptingCore",
            "Chaos", "ChaosVehicles", "ChaosModularVehicle", "ChaosModularVehicleEngine", "ChaosVehiclesCore", "ChaosVehiclesEngine",
            "ImageWrapper", "RenderCore",
        });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
