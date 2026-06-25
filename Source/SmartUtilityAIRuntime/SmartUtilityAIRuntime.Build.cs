// Copyright (c) 2026 Franciszek Łuczak
//
// This file is part of SmartUtilityAI plugin.
//
// Licensed under a custom source-available license.
// See LICENSE file in the project root for full terms.
//
// You may use this software in game development, learning and research,
// including commercial games, but you may not redistribute,
// sublicense, or sell this source code or derived plugin products
// without explicit permission.

using UnrealBuildTool;

public class SmartUtilityAIRuntime : ModuleRules
{
	public SmartUtilityAIRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "AIModule", "GameplayTasks", "GameplayTags", "NavigationSystem" });


        PrivateDependencyModuleNames.AddRange(new string[] {  });

        // Optional Gameplay Abilities System (GAS) support.
		// Set bWithGAS = false to exclude GAS dependency if the project doesn't use it.
		bool bWithGAS = true;
		if (bWithGAS)
		{
			PrivateDependencyModuleNames.Add("GameplayAbilities");
			PublicDefinitions.Add("SMARTUTILITYAI_WITH_GAS=1");
		}
		else
		{
			PublicDefinitions.Add("SMARTUTILITYAI_WITH_GAS=0");
		}
    }
}
