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

#include "UtilityAITypes.h"
#include "UtilityConsiderationBase.h"
#include "UtilityConsiderationBPBase.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class SMARTUTILITYAIRUNTIME_API UUtilityConsiderationBPBase : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UUtilityConsiderationBPBase();

protected:
	// Delegates raw value computation to the Blueprint implementable BP_ComputeRawValue.
	virtual float ComputeRawValue(const class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn) const override;

	// Override in Blueprint to return the unnormalized input value for this consideration.
	UFUNCTION(BlueprintImplementableEvent, Category = "Consideration", meta = (DisplayName = "Compute Raw Value"))
	float BP_ComputeRawValue(const class UUtilityBrainComponent *OwnerComp, class APawn *ControlledPawn) const;
};
