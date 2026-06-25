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
#include "Consideration_CooldownRemaining.generated.h"

// Reads the remaining cooldown time for a specific action
// from the brain component's runtime state. Normalized against the full cooldown duration. 
// Enables scoring dependencies like "prefer ranged attack when melee is on cooldown."
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Cooldown Remaining"))
class SMARTUTILITYAIRUNTIME_API UConsideration_CooldownRemaining : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_CooldownRemaining();

    UPROPERTY(EditAnywhere, Category = "Self-state")
	FName ActionId;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};