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
#include "Components/ActorComponent.h"
#include "Core/UtilityBrainDebugTypes.h"
#include "UtilityBrainDebugComponent.generated.h"

class UUtilityBrainComponent;
class UUtilityActionBase;

// Add this component to any actor that owns a UUtilityBrainComponent to enable live
// debug data capture. The component self-registers with the brain on BeginPlay and
// is also created dynamically by the editor asset editor and Gameplay Debugger.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SMARTUTILITYAIRUNTIME_API UUtilityBrainDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUtilityBrainDebugComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetCaptureEnabled(bool bEnabled);
	bool IsCaptureEnabled() const { return bCaptureEnabled; }
	const FBrainDebugSnapshot &GetSnapshot() const { return Snapshot; }

	// Reset the ActionInfos array for a new evaluation pass.
	void BeginEvaluationCapture(int32 ActionCount);

	// Scratch buffer written into by ScoreAction() per consideration.
	TArray<FConsiderationDebugInfo> &GetConsiderationScratch() { return ConsiderationScratch; }

	// Store completed per-action data after all modifiers have been applied.
	void CommitActionInfo(int32 Index, const FActionDebugInfo &Info);

	// Seal the snapshot after all actions have been committed. Bumping
	// EvaluationCount last is intentional: panels use it as a change signal.
	void FinalizeCapture(TWeakObjectPtr<UUtilityActionBase> ActiveAction, int32 EvalCount);

private:
	// Whether the component is actively recording evaluation data.
	bool bCaptureEnabled = false;
	// The most recently finalized evaluation snapshot.
	FBrainDebugSnapshot Snapshot;
	// Temporary per-consideration data accumulated during a single ScoreAction call.
	TArray<FConsiderationDebugInfo> ConsiderationScratch;
};
