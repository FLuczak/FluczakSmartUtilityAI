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
#include "GameplayDebuggerCategory.h"
#include "Core/UtilityBrainDebugTypes.h"

class UUtilityBrainComponent;

class FUtilityBrainDebugger : public FGameplayDebuggerCategory
{
public:
	FUtilityBrainDebugger();

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	virtual void CollectData(APlayerController *OwnerPC, AActor *DebugActor) override;
	virtual void DrawData(APlayerController *OwnerPC, FGameplayDebuggerCanvasContext &CanvasContext) override;

private:
	// Display data populated in CollectData() and read in DrawData().
	// Using plain types so this works trivially in single-player PIE without
	// formal data-pack replication.
	struct FActionDisplayData
	{
		FString Name;
		float FinalScore = 0.f;
		float PreModifierScore = 0.f;
		float InertiaBonusApplied = 1.f;
		float PenaltyMultiplierApplied = 1.f;
		bool bIsActive = false;
		bool bOnCooldown = false;
		TArray<FConsiderationDebugInfo> ConsiderationScores;
	};

	TArray<FActionDisplayData> CachedActions;
	FString ActiveActionName;
	int32 CachedEvalCount = 0;

	static UUtilityBrainComponent *GetBrainComponent(AActor *Actor);
};
