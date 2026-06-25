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
#include "Consideration_BlackboardComparison.generated.h"

UENUM(BlueprintType)
enum class EBBComparisonOp : uint8
{
	GreaterThan,
	LessThan,
	EqualTo,
	NotEqualTo,
	GreaterThanOrEqualTo,
	LessThanOrEqualTo,
};

// Compares two float values (blackboard key vs key, or key vs constant).
// Returns 1.0 when the comparison is true, 0.0 when false. Covers cases like
// "is my health lower than target's health" or "is distance below threshold."
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (DisplayName = "Blackboard Comparison"))
class SMARTUTILITYAIRUNTIME_API UConsideration_BlackboardComparison : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_BlackboardComparison();

	UPROPERTY(EditAnywhere, Category = "Comparison")
	FUtilityBlackboardKeySelector KeyA;

	// When true, KeyB is used for the right-hand side. When false, ConstantB is used.
	UPROPERTY(EditAnywhere, Category = "Comparison")
	bool bCompareToBlackboardKey = false;

	UPROPERTY(EditAnywhere, Category = "Comparison", meta = (EditCondition = "bCompareToBlackboardKey"))
	FUtilityBlackboardKeySelector KeyB;

	UPROPERTY(EditAnywhere, Category = "Comparison", meta = (EditCondition = "!bCompareToBlackboardKey"))
	float ConstantB = 0.f;

	UPROPERTY(EditAnywhere, Category = "Comparison")
	EBBComparisonOp Operator = EBBComparisonOp::GreaterThan;

	// Tolerance used for EqualTo and NotEqualTo comparisons.
	UPROPERTY(EditAnywhere, Category = "Comparison")
	float EqualityTolerance = 0.01f;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif

private: 

	bool Compare(float ValueA, float ValueB) const;
};
