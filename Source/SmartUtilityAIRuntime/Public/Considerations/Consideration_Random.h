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
#include "Consideration_Random.generated.h"

// Returns a random float between 0 and 1, optionally seeded per-agent for determinism. 
// Adds controlled noise to scoring with flat curves, preventing identical 
// agents from making identical decisions and making groups feel organic.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Random"))
class SMARTUTILITYAIRUNTIME_API UConsideration_Random : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_Random();

	UPROPERTY(EditAnywhere, Category = "Context")
	bool bSeedPerAgent = false;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};