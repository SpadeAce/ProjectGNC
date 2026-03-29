// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Dev_GNC : ModuleRules
{
	public Dev_GNC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "SlateCore" });

		PublicIncludePaths.AddRange(new string[] {
			"Dev_GNC",
			"Dev_GNC/CardGame",
			"Dev_GNC/CardGame/Subsystem",
			"Dev_GNC/CardGame/Data",
			"Dev_GNC/CardGame/Grid",
			"Dev_GNC/CardGame/Entity",
			"Dev_GNC/CardGame/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
