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
#include "UObject/NoExportTypes.h"
#include "UtilityAITypes.h"
#include "Curves/CurveFloat.h"
#include "UtilityConsiderationBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class SMARTUTILITYAIRUNTIME_API UUtilityConsiderationBase : public UObject
{
	GENERATED_BODY()

public:
	UUtilityConsiderationBase();

	// Raw input range used to normalize ComputeRawValue into [0, 1].
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	float InputMin = 0.f;

	// Upper bound of the raw input range; values at or above this map to 1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	float InputMax = 1.f;

	// Optional response curve applied after normalization. X-axis is the normalized
	// [0,1] value; Y-axis is the final score. Leave null for a linear passthrough.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consideration")
	TObjectPtr<UCurveFloat> ResponseCurve;

	// Normalizes ComputeRawValue() via InputMin/InputMax, then applies ResponseCurve.
	float Evaluate(const class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn) const;

	// Like Evaluate(), but also captures the intermediate raw/normalized/final values
	// into OutDebug for live display. Used when a UUtilityBrainDebugComponent is active.
	float EvaluateWithDebug(const class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn,
							struct FConsiderationDebugInfo &OutDebug) const;

#if WITH_EDITOR
	// Returns a short summary of this consideration's key parameters for display
	// in the consideration card. Override in subclasses to surface dynamic values
	// (e.g. which blackboard key is selected, expected comparison value, etc.).
	// Return an empty FText to hide the row.
	virtual FText GetExtraDescription() const { return FText::GetEmpty(); }
#endif

protected:
	// Override to return the raw, unnormalized value for this consideration.
	// The base Evaluate() handles normalization and curve application.
	virtual float ComputeRawValue(const class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn) const;

	// Clamps RawValue to [Min, Max], normalizes it to [0,1], then applies the optional response curve.
	static float NormalizeAndApplyCurve(float RawValue, float Min, float Max, const UCurveFloat *Curve);
};
