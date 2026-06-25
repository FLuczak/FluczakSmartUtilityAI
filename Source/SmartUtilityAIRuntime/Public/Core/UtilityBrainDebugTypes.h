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

class UUtilityActionBase;

// Per-consideration intermediate values captured during a single evaluation pass.
struct FConsiderationDebugInfo
{
	float RawValue = 0.f;		 // value returned by ComputeRawValue before normalization
	float NormalizedScore = 0.f; // after InputMin/Max clamp, before curve
	float FinalScore = 0.f;		 // after response curve
};

// Per-action data captured after all modifiers are applied for a single evaluation.
struct FActionDebugInfo
{
	TWeakObjectPtr<UUtilityActionBase> Action; // weak ref to the runtime action instance
	FString DisplayName;					   // cached at capture time so it survives after PIE ends

	TArray<FConsiderationDebugInfo> ConsiderationScores; // one entry per consideration, in order

	float PreModifierScore = 0.f;		  // raw consideration score before inertia/penalty
	float FinalScore = 0.f;				  // after all modifiers
	float InertiaBonusApplied = 1.f;	  // multiplier (1.0 = no bonus)
	float PenaltyMultiplierApplied = 1.f; // < 1.0 when a failure penalty is active
	bool bOnCooldown = false;			  // true if the action was skipped due to an active cooldown
};

// Snapshot of the full brain evaluation, updated every tick when capture is enabled.
struct FBrainDebugSnapshot
{
	TArray<FActionDebugInfo> ActionInfos;			 // parallel to UUtilityBrainComponent::RuntimeActions
	TWeakObjectPtr<UUtilityActionBase> ActiveAction; // action selected at the end of the last evaluation
	int32 EvaluationCount = 0;						 // incremented each evaluation; used as a change signal by UI panels
};
