// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExtractionShooter14 : ModuleRules
{
	public ExtractionShooter14(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			//Basic
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			//UI
			"UMG", "Slate", "SlateCore",
			"CinematicCamera",
			//AI
			"AIModule", "NavigationSystem",

            "AnimGraphRuntime",

            "Niagara"
        });

		PublicIncludePaths.AddRange(new string[]
		{
			"ExtractionShooter14"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OnlineSubsystem", "OnlineSubsystemUtils",
			"EngineSettings"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		//PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
