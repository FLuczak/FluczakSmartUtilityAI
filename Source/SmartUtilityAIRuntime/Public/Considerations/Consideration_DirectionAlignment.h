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
#include "Core/UtilityBlackboardKeySelector.h"
#include "Consideration_DirectionAlignment.generated.h"

// Dot product between the owner's forward vector and the direction to a target. 
// Returns 1.0 when facing the target, 0.0 when perpendicular, -1.0 when facing away 
// (remapped to 0-1 through input range). Useful for "prefer targets I'm already facing" 
// and "don't attack things behind me."
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Direction Alignment"))
class SMARTUTILITYAIRUNTIME_API UConsideration_DirectionAlignment : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_DirectionAlignment();

	UPROPERTY(EditAnywhere, Category = "Spatial")
	FUtilityBlackboardKeySelector TargetKey;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};