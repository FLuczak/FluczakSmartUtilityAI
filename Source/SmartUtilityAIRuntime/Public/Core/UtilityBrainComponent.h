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
#include "BrainComponent.h"
#include "GameplayTagContainer.h"
#include "UtilityAITypes.h"
#include "UtilityBrainComponent.generated.h"

class UUtilityBrainDebugComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnActionChanged,
	class UUtilityActionBase *, OldAction,
	class UUtilityActionBase *, NewAction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnActionStatusChanged,
	class UUtilityActionBase *, Action,
	EActionStatus, NewStatus);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SMARTUTILITYAIRUNTIME_API UUtilityBrainComponent : public UBrainComponent
{
	GENERATED_BODY()

public:
	UUtilityBrainComponent();

	// Per-component tick rate override; 0 means use the BrainAsset's DefaultTickRate.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI",
			  meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float TickRateOverride = 0.f;

	// Duration of score penalty after failure (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility AI")
	float FailurePenaltyDuration = 2.f;

	// Penalty multiplier applied to score during failure penalty window
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility AI")
	float FailurePenaltyMultiplier = 0.1f;

	// Bonus applied to the active action's score to reduce thrashing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility AI",
			  meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InertiaBonus = 0.1f;

	// Seconds to ramp inertia bonus from 0 to full
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility AI",
			  meta = (ClampMin = "0.0"))
	float InertiaRampSeconds = 0.25f;

	// The brain asset driving this component's action scoring and selection.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	TObjectPtr<class UUtilityBrainAsset> BrainAsset = nullptr;

	// Fired when the active action changes.
	UPROPERTY(BlueprintAssignable, Category = "Utility AI|Events")
	FOnActionChanged OnActionChanged;

	// Fired when an action's status changes.
	UPROPERTY(BlueprintAssignable, Category = "Utility AI|Events")
	FOnActionStatusChanged OnActionStatusChanged;

	// Cleans up the debug component on end play.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// Advances evaluation timer and ticks the active action.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction *ThisTickFunction) override;

	// Initializes the blackboard and action instances, then begins evaluation.
	virtual void StartLogic() override;
	// Resets all runtime state and restarts brain logic.
	virtual void RestartLogic() override;
	// Stops evaluation and aborts the active action.
	virtual void StopLogic(const FString &Reason) override;
	// Pauses evaluation without aborting the current action.
	virtual void PauseLogic(const FString &Reason) override;
	// Resumes a previously paused evaluation.
	virtual EAILogicResuming::Type ResumeLogic(const FString &Reason) override;
	// Returns true if brain logic has been started and not stopped.
	virtual bool IsRunning() const override;
	// Returns true if evaluation is currently paused.
	virtual bool IsPaused() const override;

	// Triggers an immediate evaluation pass, bypassing the tick interval.
	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	void ForceEvaluation();

	// Aborts the active action and forces a new evaluation.
	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	void AbortCurrentAction();

	// Pauses or resumes evaluation without affecting the active action.
	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	void SetEvaluationPaused(bool bPaused);

	// Returns true if evaluation is currently paused.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	bool IsEvaluationPaused() const { return bEvaluationPaused; }

	// Returns the action currently being executed.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	class UUtilityActionBase *GetActiveAction() const { return ActiveAction; }

	// Returns the status of the currently active action.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	EActionStatus GetActiveActionStatus() const;

	// Returns the blackboard component used by this brain.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	UBlackboardComponent *GetBlackboardComponent() const { return BlackboardComp; }

	// Returns the pawn controlled by the owning AI controller.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	APawn *GetControlledPawn() const;

	// Returns how long the active action has been running, in seconds.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	float GetActiveActionElapsedTime() const { return ActionElapsedTime; }

	// Returns remaining cooldown in seconds for a specific action id, or 0 if not on cooldown.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	float GetCooldownRemaining(FName ActionId) const;

	// Returns the world time at which a specific action id last completed successfully,
	// or -1.0 if it has never succeeded.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	double GetLastSuccessfulExecutionTime(FName ActionId) const;

	// Returns the debug component if one is registered, otherwise null.
	UUtilityBrainDebugComponent *GetDebugComponent() const { return DebugComp; }
	// Registers a debug component to receive evaluation snapshots.
	void RegisterDebugComponent(UUtilityBrainDebugComponent *Comp);
	// Unregisters a previously registered debug component.
	void UnregisterDebugComponent(UUtilityBrainDebugComponent *Comp);

protected:
	// Creates or links the blackboard component from the brain asset.
	void InitializeBlackboard();
	// Duplicates asset actions into runtime instances and precomputes bucket indices.
	void InitializeActions();

	// Scores all actions, selects the best, and transitions if the interrupt policy allows.
	void EvaluateActions(float DeltaTime);
	// Clears all runtime maps and resets the active action.
	void ResetRuntimeState();
	// Fills FinalScores for each action, applying failure penalties and cooldown filtering.
	void ScoreActions(const TArray<TObjectPtr<class UUtilityActionBase>> &Actions, float MinimumScoreThreshold);
	// Multiplies score by the penalty multiplier if the action is within its failure window.
	float ApplyFailurePenalty(class UUtilityActionBase *Action, float Score);
	// Returns true if the interrupt policy permits switching to the given action.
	bool ShouldTransitionToAction(class UUtilityActionBase *BestAction) const;
	// Computes the weighted consideration product for a single action.
	float ScoreAction(const class UUtilityActionBase *Action) const;
	// Dave Mark's IAUS formula: prevents score deflation with many considerations.
	static float ApplyCompensation(float Score, int32 ConsiderationCount);
	// Chooses the highest-scoring action, respecting bucket priorities and cutoffs.
	class UUtilityActionBase *SelectBestAction(const TArray<float> &Scores) const;
	// Returns TickRateOverride if non-zero, otherwise the BrainAsset's DefaultTickRate.
	float GetEffectiveTickInterval() const;

	// Returns the inertia bonus for the active action, ramping up over InertiaRampSeconds.
	float CalculateInertiaBonus(class UUtilityActionBase *Action) const;
	// Returns true if the action's cooldown has not yet expired.
	bool IsOnCooldown(const class UUtilityActionBase *Action) const;
	// Records cooldown expiry time for an action after successful completion.
	void StartCooldown(class UUtilityActionBase *Action);
	// Records a failure penalty expiry for an action.
	void ApplyScorePenalty(class UUtilityActionBase *Action);

	// Destroys runtime action instances and clears associated state maps.
	void CleanupRuntimeActions();
	// Exits the current action and enters a new one, broadcasting OnActionChanged.
	void TransitionToAction(class UUtilityActionBase *NewAction);
	// Calls OnActionTick on the active action and handles the returned status.
	void ExecuteActiveAction(float DeltaTime);
	// Processes terminal action states: starts cooldown, applies penalty, forces re-evaluation.
	void HandleActionCompletion(EActionStatus Result);

#if ENABLE_VISUAL_LOG
	// Writes the current action scores to the visual log.
	void RecordVisualLogForCurrentAction(const UUtilityActionBase *BestAction) const;
#endif

private:
	// Updates the action status map and broadcasts OnActionStatusChanged.
	void SetActionStatus(class UUtilityActionBase *Action, EActionStatus NewStatus);
	// Returns a pointer into the flat ActionMemory buffer for the given action.
	uint8 *GetActionMemory(UUtilityActionBase *Action);

	// The action currently being executed.
	UPROPERTY()
	TObjectPtr<class UUtilityActionBase> ActiveAction = nullptr;
	// Per-action status map, keyed by runtime action pointer.
	TMap<TObjectPtr<UUtilityActionBase>, EActionStatus> ActionStatuses{};

	// World time at which each action's cooldown expires.
	TMap<TObjectPtr<class UUtilityActionBase>, double> CooldownExpiries{};
	// World time until which each action's failure penalty is active.
	TMap<TObjectPtr<class UUtilityActionBase>, double> ScorePenalties{};
	// World time of each action's most recent successful completion.
	TMap<TObjectPtr<class UUtilityActionBase>, double> LastSuccessfulExecutionTimes{};

	// Accumulates DeltaTime between evaluation ticks.
	float TimeSinceLastEvaluation = 0.f;
	// Time in seconds the active action has been running.
	float ActionElapsedTime = 0.f;
	// When true, EvaluateActions is skipped each tick.
	bool bEvaluationPaused = false;
	// True between StartLogic and StopLogic calls.
	bool bIsLogicRunning = false;

	// UPROPERTY keeps DuplicateObject instances alive through the GC
	UPROPERTY()
	TArray<TObjectPtr<UUtilityActionBase>> RuntimeActions;
	// Fast lookup from action pointer to its index in RuntimeActions.
	TMap<TObjectPtr<UUtilityActionBase>, int32> ActionIndexByPtr;

	// Per-action bucket index (index into BrainAsset->GetBuckets(), or INDEX_NONE if unbucketed).
	// Precomputed in InitializeActions so SelectBestAction runs in O(N+M) instead of O(N*M).
	TArray<int32> ActionBucketIndices;

	// Byte offset into ActionMemory for each action's instance data.
	TArray<int32> ActionMemoryOffsets;
	// Flat byte buffer for all action instance memory, indexed via ActionMemoryOffsets.
	TArray<uint8> ActionMemory;

	// Scratch buffer — preallocated to avoid per-frame allocation
	TArray<float> FinalScores;

	// Total number of brain evaluations since StartLogic.
	int32 EvaluationCount = 0;

	// Optional debug component for live score capture.
	UPROPERTY()
	TObjectPtr<UUtilityBrainDebugComponent> DebugComp = nullptr;
};
