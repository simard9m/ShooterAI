// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Devoir3Shooter : ModuleRules
{
	public Devoir3Shooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Devoir3Shooter",
			"Devoir3Shooter/Variant_Platforming",
			"Devoir3Shooter/Variant_Platforming/Animation",
			"Devoir3Shooter/Variant_Combat",
			"Devoir3Shooter/Variant_Combat/AI",
			"Devoir3Shooter/Variant_Combat/Animation",
			"Devoir3Shooter/Variant_Combat/Gameplay",
			"Devoir3Shooter/Variant_Combat/Interfaces",
			"Devoir3Shooter/Variant_Combat/UI",
			"Devoir3Shooter/Variant_SideScrolling",
			"Devoir3Shooter/Variant_SideScrolling/AI",
			"Devoir3Shooter/Variant_SideScrolling/Gameplay",
			"Devoir3Shooter/Variant_SideScrolling/Interfaces",
			"Devoir3Shooter/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
