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

#include "Core/UtilityConsiderationBase.h"
#include "Core/UtilityBrainComponent.h"
#include "Core/UtilityBrainDebugTypes.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Pawn.h"

UUtilityConsiderationBase::UUtilityConsiderationBase()
{
}

float UUtilityConsiderationBase::Evaluate(const UUtilityBrainComponent& OwnerComp, APawn* ControlledPawn) const
{
	return NormalizeAndApplyCurve(ComputeRawValue(OwnerComp, ControlledPawn), InputMin, InputMax, ResponseCurve);
}

float UUtilityConsiderationBase::ComputeRawValue(const UUtilityBrainComponent& OwnerComp, APawn* ControlledPawn) const
{
	return 0.0f;
}

float UUtilityConsiderationBase::EvaluateWithDebug(const UUtilityBrainComponent& OwnerComp, APawn* ControlledPawn,
                                                   FConsiderationDebugInfo& OutDebug) const
{
	OutDebug.RawValue = ComputeRawValue(OwnerComp, ControlledPawn);
	const float Normalized = FMath::IsNearlyEqual(InputMin, InputMax)
		? 0.f
		: FMath::Clamp((OutDebug.RawValue - InputMin) / (InputMax - InputMin), 0.f, 1.f);
	OutDebug.NormalizedScore = Normalized;
	OutDebug.FinalScore = ResponseCurve ? ResponseCurve->GetFloatValue(Normalized) : Normalized;
	return OutDebug.FinalScore;
}

float UUtilityConsiderationBase::NormalizeAndApplyCurve(float RawValue, float Min, float Max, const UCurveFloat* Curve)
{
	const float Normalized = FMath::IsNearlyEqual(Min, Max)
		? 0.f
		: FMath::Clamp((RawValue - Min) / (Max - Min), 0.f, 1.f);
	return Curve ? Curve->GetFloatValue(Normalized) : Normalized;
}
