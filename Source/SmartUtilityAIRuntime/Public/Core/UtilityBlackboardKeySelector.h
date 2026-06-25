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
#include "UtilityBlackboardKeySelector.generated.h"

// Blackboard key reference for UtilityAI considerations and actions.
// In the editor, any UPROPERTY of this type automatically shows a key
// dropdown populated from the owning UUtilityBrainAsset's BlackboardAsset.
USTRUCT(BlueprintType)
struct SMARTUTILITYAIRUNTIME_API FUtilityBlackboardKeySelector
{
	GENERATED_BODY()

	// The name of the selected blackboard key.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FName SelectedKeyName;
};
