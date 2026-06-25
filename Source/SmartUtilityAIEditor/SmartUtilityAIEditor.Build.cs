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
using System.IO;

public class SmartUtilityAIEditor : ModuleRules
{
	public SmartUtilityAIEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Public"),
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Private"),
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"SmartUtilityAIRuntime",
		});

		// Editor-only dependencies
		PrivateDependencyModuleNames.AddRange(new string[] {
			"UnrealEd",
			"GraphEditor",
			"Slate",
			"SlateCore",
			"EditorStyle",
			"GameplayTags",
			"AssetTools",
			"AssetRegistry",
			"PropertyEditor",
			"ToolMenus",
			"ContentBrowser",
			"EditorWidgets",
			"CurveEditor",
			"GameplayDebugger",
			"MessageLog",
			"AIModule",
		});
	}
}
