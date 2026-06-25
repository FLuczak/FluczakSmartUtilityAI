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
#include "Consideration_BlackboardFloat.generated.h"

// Reads a float key from the blackboard, normalizes it through InputMin/InputMax,
// and applies the response curve.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (DisplayName = "Blackboard Float"))
class SMARTUTILITYAIRUNTIME_API UConsideration_BlackboardFloat : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_BlackboardFloat();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FUtilityBlackboardKeySelector BlackboardKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	float MinValue = 0.f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	float MaxValue = 1.f;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};
