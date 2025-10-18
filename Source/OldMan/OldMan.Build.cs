// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OldMan : ModuleRules
{
	public OldMan(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore", 
				"EnhancedInput",
				"XyFrame",
				"OldManItem",
			});

        PublicIncludePaths.AddRange(
			new string[] { "XyFrame/Public",
            "OldManItem/Public"
            });
    }
}
