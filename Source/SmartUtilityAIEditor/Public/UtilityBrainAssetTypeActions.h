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

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FUtilityBrainAssetTypeActions : public FAssetTypeActions_Base
{
public:
	explicit FUtilityBrainAssetTypeActions(EAssetTypeCategories::Type InCategory)
		: MyCategory(InCategory)
	{
	}

	virtual UClass *GetSupportedClass() const override;
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	virtual void OpenAssetEditor(
		const TArray<UObject *> &InObjects,
		TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

private:
	EAssetTypeCategories::Type MyCategory;
};
