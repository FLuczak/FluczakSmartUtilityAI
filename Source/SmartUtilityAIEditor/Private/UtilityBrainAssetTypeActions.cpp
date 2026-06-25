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

#include "UtilityBrainAssetTypeActions.h"
#include "Core/UtilityBrainAsset.h"
#include "UtilityBrainEditorToolkit.h"

UClass *FUtilityBrainAssetTypeActions::GetSupportedClass() const
{
	return UUtilityBrainAsset::StaticClass();
}

FText FUtilityBrainAssetTypeActions::GetName() const
{
	return NSLOCTEXT("UtilityAI", "BrainAsset", "Utility Brain");
}

FColor FUtilityBrainAssetTypeActions::GetTypeColor() const
{
	return FColor(0, 188, 150);
}

uint32 FUtilityBrainAssetTypeActions::GetCategories()
{
	return MyCategory;
}

void FUtilityBrainAssetTypeActions::OpenAssetEditor(
	const TArray<UObject *> &InObjects,
	TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject *Obj : InObjects)
	{
		UUtilityBrainAsset *BrainAsset = Cast<UUtilityBrainAsset>(Obj);
		if (!BrainAsset)
		{
			continue;
		}

		TSharedRef<FUtilityBrainEditorToolkit> Toolkit = MakeShared<FUtilityBrainEditorToolkit>();
		Toolkit->InitEditor(EToolkitMode::Standalone, EditWithinLevelEditor, BrainAsset);
	}
}
