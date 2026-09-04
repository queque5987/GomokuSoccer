// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class gosoccer : ModuleRules
{
	public gosoccer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                "gosoccer"
            }
        );
        PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine", 
			"InputCore", 
			"Niagara", 
			"EnhancedInput", 
			"Slate", 
			"SlateCore", 
			"OnlineSubsystem", 
			"OnlineSubsystemUtils",
			"OnlineSubsystemSteam",
            "OnlineSubsystemNull",
            "Steamworks",
            "GeometryCollectionEngine",
            "Chaos",
            "PhysicsCore",
            "FieldSystemEngine"
        });
        //PrivateDependencyModuleNames.AddRange(new string[] {  });
        //AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
