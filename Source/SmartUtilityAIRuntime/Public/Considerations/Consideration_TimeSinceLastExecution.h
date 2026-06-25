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
#include "Core/UtilityConsiderationBase.h"
#include "Consideration_TimeSinceLastExecution.generated.h"

// Time since this action last completed successfully. Normalized against a configurable window. 
// Uses a linear curve where actions that haven't run in a while score higher, 
// creating variety and preventing the AI from never using certain routines or looping.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Time Since Last Execution"))
class SMARTUTILITYAIRUNTIME_API UConsideration_TimeSinceLastExecution : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_TimeSinceLastExecution();

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
};