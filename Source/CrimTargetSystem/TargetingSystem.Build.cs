// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TargetingSystem : ModuleRules
{
	public TargetingSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags",
				"NetCore",
				"DeveloperSettings"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"GameplayTags", 
				"CrimBlueprintStatics",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
