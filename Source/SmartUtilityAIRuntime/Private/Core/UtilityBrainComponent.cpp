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

#include "Core/UtilityBrainComponent.h"

#include "Core/SmartUtilityAIRuntime.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/UtilityActionBase.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityBrainDebugComponent.h"
#include "Core/UtilityConsiderationBase.h"

#include "BehaviorTree/BlackboardData.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "VisualLogger/VisualLogger.h"

DECLARE_CYCLE_STAT(TEXT("UtilityAI EvaluateActions"),
				   STAT_UtilityAI_Evaluate, STATGROUP_AI);
DECLARE_CYCLE_STAT(TEXT("UtilityAI ExecuteTask"),
				   STAT_UtilityAI_ExecuteTask, STATGROUP_AI);
DECLARE_CYCLE_STAT(TEXT("UtilityAI ScoreAction"),
				   STAT_UtilityAI_ScoreAction, STATGROUP_AI);

// =============================================================================
// Constructor
// =============================================================================

UUtilityBrainComponent::UUtilityBrainComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Default to ticking in the AI group — keeps it ordered with other AI
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

// =============================================================================
// UActorComponent overrides
// =============================================================================

void UUtilityBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopLogic(TEXT("EndPlay"));

	Super::EndPlay(EndPlayReason);
}

void UUtilityBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsRunning())
	{
		return;
	}

	if (!BrainAsset || bEvaluationPaused)
	{
		if (ActiveAction)
		{
			ExecuteActiveAction(DeltaTime);
		}
		return;
	}

	TimeSinceLastEvaluation += DeltaTime;
	const float TickInterval = GetEffectiveTickInterval();
	if (TimeSinceLastEvaluation >= TickInterval)
	{
		EvaluateActions(DeltaTime);
		TimeSinceLastEvaluation = 0.f;
	}

	// Always tick the active action every frame (independent of eval rate)
	if (ActiveAction)
	{
		ExecuteActiveAction(DeltaTime);
	}
}

// =============================================================================
// UBrainComponent overrides
// =============================================================================

void UUtilityBrainComponent::StartLogic()
{
	if (bIsLogicRunning)
	{
		return;
	}

	InitializeBlackboard();

	bIsLogicRunning = true;
	bEvaluationPaused = false;
	TimeSinceLastEvaluation = 0.f;

	ResetRuntimeState();
	RuntimeActions.Reset();
	ActionMemoryOffsets.Reset();
	ActionMemory.Reset();
	ActionIndexByPtr.Reset();
	ActionBucketIndices.Reset();
	if (!BrainAsset)
	{
		SetComponentTickEnabled(true);
		Super::StartLogic();
		return;
	}

	InitializeActions();
	FinalScores.SetNumZeroed(RuntimeActions.Num());

	SetComponentTickEnabled(true);
	Super::StartLogic();
}

void UUtilityBrainComponent::RestartLogic()
{
	StopLogic(TEXT("RestartLogic"));
	StartLogic();
}

void UUtilityBrainComponent::StopLogic(const FString &Reason)
{
	if (!bIsLogicRunning)
	{
		return;
	}

	// Set false before AbortCurrentAction so that the ForceEvaluation triggered by
	// HandleActionCompletion is a no-op and doesn't start a new action during teardown.
	bIsLogicRunning = false;

	AbortCurrentAction();

	ResetRuntimeState();
	TimeSinceLastEvaluation = 0.f;

	CleanupRuntimeActions();

	// Release references so DuplicateObject'd actions can be GC'd after stop.
	RuntimeActions.Reset();
	ActionMemoryOffsets.Reset();
	ActionMemory.Reset();
	ActionIndexByPtr.Reset();
	ActionBucketIndices.Reset();

	SetComponentTickEnabled(false);
	Super::StopLogic(Reason);
}

void UUtilityBrainComponent::PauseLogic(const FString &Reason)
{
	bEvaluationPaused = true;
}

EAILogicResuming::Type UUtilityBrainComponent::ResumeLogic(const FString &Reason)
{
	const EAILogicResuming::Type ResumeResult = Super::ResumeLogic(Reason);
	if (ResumeResult == EAILogicResuming::Continue)
	{
		bEvaluationPaused = false;
	}

	return ResumeResult;
}

bool UUtilityBrainComponent::IsRunning() const
{
	return bIsLogicRunning;
}

bool UUtilityBrainComponent::IsPaused() const
{
	return bEvaluationPaused;
}

// =============================================================================
// Blueprint callable
// =============================================================================

void UUtilityBrainComponent::ForceEvaluation()
{
	if (!IsRunning() || bEvaluationPaused || !BrainAsset)
	{
		return;
	}

	EvaluateActions(0.f);
	TimeSinceLastEvaluation = 0.f;
}

void UUtilityBrainComponent::AbortCurrentAction()
{
	if (!ActiveAction)
	{
		return;
	}

	uint8 *Memory = GetActionMemory(ActiveAction);
	ActiveAction->OnActionExited(*this, GetControlledPawn(), EActionStatus::Aborted, Memory);
	SetActionStatus(ActiveAction, EActionStatus::Aborted);
	HandleActionCompletion(EActionStatus::Aborted);
}

void UUtilityBrainComponent::SetEvaluationPaused(bool bPaused)
{
	bEvaluationPaused = bPaused;
}

// =============================================================================
// Getters
// =============================================================================

EActionStatus UUtilityBrainComponent::GetActiveActionStatus() const
{
	return ActiveAction ? ActionStatuses.FindChecked(ActiveAction) : EActionStatus::Inactive;
}

APawn *UUtilityBrainComponent::GetControlledPawn() const
{
	if (AAIController *AIC = GetAIOwner())
	{
		return AIC->GetPawn();
	}
	return Cast<APawn>(GetOwner());
}

float UUtilityBrainComponent::GetCooldownRemaining(FName ActionId) const
{
	if (!BrainAsset || ActionId.IsNone())
	{
		return 0.f;
	}

	for (UUtilityActionBase *ItAction : RuntimeActions)
	{
		if (ItAction && ItAction->ActionId == ActionId)
		{
			if (const double *Expiry = CooldownExpiries.Find(ItAction))
			{
				return FMath::Max(0.f, static_cast<float>(*Expiry - GetWorld()->GetTimeSeconds()));
			}
			return 0.f;
		}
	}
	return 0.f;
}

double UUtilityBrainComponent::GetLastSuccessfulExecutionTime(FName ActionId) const
{
	if (!BrainAsset || ActionId.IsNone())
	{
		return -1.0;
	}

	for (UUtilityActionBase *Action : RuntimeActions)
	{
		if (Action && Action->ActionId == ActionId)
		{
			if (const double *Time = LastSuccessfulExecutionTimes.Find(Action))
			{
				return *Time;
			}
			return -1.0;
		}
	}
	return -1.0;
}

void UUtilityBrainComponent::RegisterDebugComponent(UUtilityBrainDebugComponent *Comp)
{
	DebugComp = Comp;
}

void UUtilityBrainComponent::UnregisterDebugComponent(UUtilityBrainDebugComponent *Comp)
{
	if (DebugComp == Comp)
	{
		DebugComp = nullptr;
	}
}

// =============================================================================
// Initialization
// =============================================================================

void UUtilityBrainComponent::InitializeBlackboard()
{
	AActor *Owner = GetOwner();

	if (!Owner)
		return;

	BlackboardComp = Owner->FindComponentByClass<UBlackboardComponent>();

	if (!BlackboardComp)
	{
		if (APawn *Pawn = Cast<APawn>(Owner))
		{
			if (AAIController *AIC = Cast<AAIController>(Pawn->GetController()))
			{
				BlackboardComp = AIC->GetBlackboardComponent();
			}
		}
		else if (AAIController *AIC = Cast<AAIController>(Owner))
		{
			BlackboardComp = AIC->GetBlackboardComponent();
		}
	}

	// If still not found and we have a brain asset with BB data, create one
	if (!BlackboardComp && BrainAsset && BrainAsset->GetBlackboardAsset())
	{
		BlackboardComp = NewObject<UBlackboardComponent>(Owner);
		BlackboardComp->RegisterComponent();

		if (AAIController *AIC = Cast<AAIController>(Owner))
		{
			UBlackboardComponent *BlackboardCompRaw = BlackboardComp.Get();
			AIC->UseBlackboard(BrainAsset->GetBlackboardAsset(), BlackboardCompRaw);
			BlackboardComp = BlackboardCompRaw;
		}
		else
		{
			BlackboardComp->InitializeBlackboard(*BrainAsset->GetBlackboardAsset());
		}
	}
}

void UUtilityBrainComponent::InitializeActions()
{
	const TArray<UUtilityActionBase *> &AssetActions = BrainAsset->GetActions();
	for (UUtilityActionBase *Action : AssetActions)
	{
		if (!Action)
		{
			RuntimeActions.Add(nullptr);
			continue;
		}

		UUtilityActionBase *Runtime = Action->ShouldCreateInstance()
										  ? DuplicateObject<UUtilityActionBase>(Action, this)
										  : Action;

		ActionIndexByPtr.Add(Runtime, RuntimeActions.Num());
		RuntimeActions.Add(Runtime);
	}

	// Precompute per-action bucket membership so SelectBestAction runs O(N+M) per tick.
	// Buckets are already sorted descending by priority; the first match is the highest-priority one.
	const TArray<FBucket> &Buckets = BrainAsset->GetBuckets();
	ActionBucketIndices.SetNum(RuntimeActions.Num());
	for (int32 i = 0; i < RuntimeActions.Num(); ++i)
	{
		ActionBucketIndices[i] = INDEX_NONE;
		if (!RuntimeActions[i])
			continue;
		for (int32 j = 0; j < Buckets.Num(); ++j)
		{
			if (RuntimeActions[i]->Tags.HasTag(Buckets[j].BucketTag))
			{
				ActionBucketIndices[i] = j;
				break;
			}
		}
	}

	int32 TotalSize = 0;

	for (UUtilityActionBase *Action : RuntimeActions)
	{
		TotalSize = Align(TotalSize, 4);
		ActionMemoryOffsets.Add(TotalSize);

		const uint16 Size = Action ? Action->GetInstanceMemorySize() : 0;
		TotalSize += Size;
	}

	ActionMemory.SetNumZeroed(TotalSize);

	for (int32 Index = 0; Index < RuntimeActions.Num(); ++Index)
	{
		UUtilityActionBase *Action = RuntimeActions[Index];
		if (!Action)
		{
			continue;
		}

		const uint16 Size = Action->GetInstanceMemorySize();
		if (Size == 0)
		{
			continue;
		}

		uint8 *Memory = ActionMemory.GetData() + ActionMemoryOffsets[Index];
		Action->InitializeActionMemory(*this, Memory);
	}
}

// =============================================================================
// Evaluation
// =============================================================================

void UUtilityBrainComponent::ResetRuntimeState()
{
	ActiveAction = nullptr;
	ActionStatuses.Reset();
	CooldownExpiries.Reset();
	ScorePenalties.Reset();
	LastSuccessfulExecutionTimes.Reset();
	ActionElapsedTime = 0.f;
	FinalScores.Reset();
	EvaluationCount = 0;
}

void UUtilityBrainComponent::EvaluateActions(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UtilityAI_Evaluate);

	if (!BrainAsset)
	{
		UE_LOG(LogUtilityAI, Warning, TEXT("UtilityBrainComponent on %s has no BrainAsset assigned."), *GetNameSafe(GetOwner()));
		return;
	}

	const int32 ActionCount = RuntimeActions.Num();
	if (ActionCount == 0)
		return;

	FinalScores.SetNumZeroed(ActionCount);

	const float MinimumScoreThreshold = BrainAsset->GetMinimumScoreThreshold();
	ScoreActions(RuntimeActions, MinimumScoreThreshold);

	EvaluationCount++;

	if (DebugComp && DebugComp->IsCaptureEnabled())
	{
		DebugComp->FinalizeCapture(ActiveAction, EvaluationCount);
	}

	UUtilityActionBase *BestAction = SelectBestAction(FinalScores);

#if ENABLE_VISUAL_LOG
	RecordVisualLogForCurrentAction(BestAction);
#endif

	if (!ShouldTransitionToAction(BestAction))
		return;

	TransitionToAction(BestAction);
}

#if ENABLE_VISUAL_LOG
void UUtilityBrainComponent::RecordVisualLogForCurrentAction(const UUtilityActionBase *BestAction) const
{
	if (FVisualLogger::IsRecording())
	{
		FString Snapshot = FString::Printf(TEXT("Eval #%d  best: %s"), EvaluationCount,
										   BestAction ? *(BestAction->ActionId.IsNone() ? BestAction->GetClass()->GetName() : BestAction->ActionId.ToString()) : TEXT("none"));
		for (int32 i = 0; i < RuntimeActions.Num(); ++i)
		{
			const UUtilityActionBase *Act = RuntimeActions[i];
			const FString Name = Act
									 ? (Act->ActionId.IsNone() ? Act->GetClass()->GetName() : Act->ActionId.ToString())
									 : TEXT("(null)");
			const float Score = FinalScores.IsValidIndex(i) ? FinalScores[i] : 0.f;
			Snapshot += FString::Printf(TEXT("\n  [%.3f] %s%s%s"),
										Score, *Name,
										Act == BestAction ? TEXT(" *") : TEXT(""),
										Act && IsOnCooldown(Act) ? TEXT(" [cd]") : TEXT(""));
		}
		UE_VLOG(GetOwner(), LogUtilityAI, Verbose, TEXT("%s"), *Snapshot);
	}
}
#endif

void UUtilityBrainComponent::ScoreActions(const TArray<TObjectPtr<UUtilityActionBase>> &Actions, float MinimumScoreThreshold)
{
	const bool bCapturing = DebugComp && DebugComp->IsCaptureEnabled();

	if (bCapturing)
	{
		DebugComp->BeginEvaluationCapture(Actions.Num());
	}

	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		UUtilityActionBase *Action = Actions[i];

		const bool bCooldown = IsValid(Action) && IsOnCooldown(Action);
		if (!IsValid(Action) || bCooldown)
		{
			if (bCapturing)
			{
				FActionDebugInfo Info{};
				Info.Action = Action;
				Info.bOnCooldown = bCooldown;
				Info.DisplayName = IsValid(Action)
									   ? (Action->ActionId.IsNone() ? Action->GetClass()->GetName() : Action->ActionId.ToString())
									   : TEXT("(null)");
				DebugComp->CommitActionInfo(i, Info);
			}
			FinalScores[i] = 0.f;
			continue;
		}

		float Score = ScoreAction(Action);
		const float RawConsiderationScore = Score;

		float InertiaMultiplier = 1.f;
		if (Action == ActiveAction)
		{
			InertiaMultiplier = CalculateInertiaBonus(Action);
			Score *= InertiaMultiplier;
		}

		const float PrePenaltyScore = Score;
		Score = ApplyFailurePenalty(Action, Score);

		if (MinimumScoreThreshold > 0.f && Score < MinimumScoreThreshold)
		{
			Score = 0.f;
		}

		FinalScores[i] = Score;

		if (bCapturing)
		{
			FActionDebugInfo Info{};
			Info.Action = Action;
			Info.DisplayName = Action->ActionId.IsNone() ? Action->GetClass()->GetName() : Action->ActionId.ToString();
			Info.ConsiderationScores = DebugComp->GetConsiderationScratch();
			Info.PreModifierScore = RawConsiderationScore;
			Info.InertiaBonusApplied = InertiaMultiplier;
			Info.PenaltyMultiplierApplied = (PrePenaltyScore > KINDA_SMALL_NUMBER) ? Score / PrePenaltyScore : 1.f;
			Info.FinalScore = Score;
			DebugComp->CommitActionInfo(i, Info);
		}
	}
}

float UUtilityBrainComponent::ApplyFailurePenalty(UUtilityActionBase *Action, float Score)
{
	if (!Action)
		return Score;

	if (const double *PenaltyExpiry = ScorePenalties.Find(Action))
	{
		if (GetWorld()->GetTimeSeconds() < *PenaltyExpiry)
		{
			return Score * FailurePenaltyMultiplier;
		}

		ScorePenalties.Remove(Action);
	}

	return Score;
}

bool UUtilityBrainComponent::ShouldTransitionToAction(UUtilityActionBase *BestAction) const
{
	if (!BestAction || BestAction == ActiveAction)
		return false;

	if (!ActiveAction)
		return true;

	const EActionStatus *CurrentStatus = ActionStatuses.Find(ActiveAction);
	if (!CurrentStatus || *CurrentStatus != EActionStatus::Running)
		return true;

	const EInterruptPolicy Policy = ActiveAction->InterruptPolicy;
	switch (Policy)
	{
	case EInterruptPolicy::NeverInterrupt:
		return false;

	case EInterruptPolicy::InterruptIfBetter:
	{
		const int32 ActiveIdx = RuntimeActions.IndexOfByKey(ActiveAction);
		const int32 BestIdx = RuntimeActions.IndexOfByKey(BestAction);
		if (ActiveIdx == INDEX_NONE || BestIdx == INDEX_NONE)
		{
			return true;
		}

		const float ActiveScore = FinalScores.IsValidIndex(ActiveIdx) ? FinalScores[ActiveIdx] : 0.f;
		const float BestScore = FinalScores.IsValidIndex(BestIdx) ? FinalScores[BestIdx] : 0.f;

		if (BestScore < ActiveScore * ActiveAction->InterruptThreshold)
		{
			return false;
		}
		return true;
	}

	case EInterruptPolicy::AlwaysReevaluate:
		return true;
	}

	return true;
}

float UUtilityBrainComponent::ScoreAction(const UUtilityActionBase *Action) const
{
	SCOPE_CYCLE_COUNTER(STAT_UtilityAI_ScoreAction);

	if (!IsValid(Action))
		return 0.f;

	const TArray<TObjectPtr<UUtilityConsiderationBase>> &Considerations = Action->GetConsiderations();

	if (Considerations.IsEmpty())
		return 0.f;

	float Score = Action->BaseWeight;
	const int32 NumConsiderations = Considerations.Num();
	APawn *ControlledPawn = GetControlledPawn();

	const bool bCapturing = DebugComp && DebugComp->IsCaptureEnabled();
	if (bCapturing)
	{
		DebugComp->GetConsiderationScratch().SetNum(NumConsiderations);
	}

	for (int32 ConsiderationIndex = 0; ConsiderationIndex < NumConsiderations; ++ConsiderationIndex)
	{
		const UUtilityConsiderationBase *Consideration = Considerations[ConsiderationIndex];
		if (!IsValid(Consideration))
		{
			if (bCapturing)
			{
				DebugComp->GetConsiderationScratch()[ConsiderationIndex] = FConsiderationDebugInfo{};
			}
			continue;
		}

		float RawScore;
		if (bCapturing)
		{
			RawScore = Consideration->EvaluateWithDebug(*this, ControlledPawn, DebugComp->GetConsiderationScratch()[ConsiderationIndex]);
		}
		else
		{
			RawScore = Consideration->Evaluate(*this, ControlledPawn);
		}

		const float AdjustedScore = ApplyCompensation(RawScore, NumConsiderations);
		Score *= AdjustedScore;

		// Early-out: score collapsed to zero, no recovery possible
		if (Score < KINDA_SMALL_NUMBER)
		{
			if (bCapturing)
			{
				for (int32 j = ConsiderationIndex + 1; j < NumConsiderations; ++j)
				{
					DebugComp->GetConsiderationScratch()[j] = FConsiderationDebugInfo{};
				}
			}
			return 0.f;
		}
	}

	return Score;
}

float UUtilityBrainComponent::ApplyCompensation(float Score, int32 ConsiderationCount)
{
	// Without compensation, multiplying N scores in [0,1] naturally shrinks
	// the product — actions with more considerations always lose.
	// The compensation factor counteracts this by boosting each score
	// proportionally to how many considerations are being multiplied.
	//
	// Formula from Dave Mark's Infinite Axis Utility System:
	//   modFactor = 1 - (1 / N)
	//   compensation = (1 - score) * modFactor
	//   adjustedScore = score + (compensation * score)

	if (ConsiderationCount <= 1)
		return Score;

	const float ModFactor = 1.f - (1.f / static_cast<float>(ConsiderationCount));
	const float Compensation = (1.f - Score) * ModFactor;

	return FMath::Clamp(Score + (Compensation * Score), 0.f, 1.f);
}

UUtilityActionBase *UUtilityBrainComponent::SelectBestAction(const TArray<float> &Scores) const
{
	if (!BrainAsset)
		return nullptr;

	const TArray<FBucket> &Buckets = BrainAsset->GetBuckets();

	UUtilityActionBase *BestAction = nullptr;
	float BestScore = 0.0f;

	if (Buckets.IsEmpty())
	{
		for (int32 i = 0; i < Scores.Num(); ++i)
		{
			if (Scores[i] > BestScore)
			{
				BestScore = Scores[i];
				BestAction = RuntimeActions[i];
			}
		}
		return BestAction;
	}

	// Single O(N) pass: assign each action's score to its precomputed bucket slot,
	// or to the unbucketed best if it has no bucket (ActionBucketIndices[i] == INDEX_NONE).
	TArray<float> BucketBestScores;
	TArray<UUtilityActionBase *> BucketWinners;
	BucketBestScores.SetNumZeroed(Buckets.Num());
	BucketWinners.SetNum(Buckets.Num());

	for (int32 i = 0; i < RuntimeActions.Num(); ++i)
	{
		if (!RuntimeActions[i])
			continue;

		const float Score = Scores[i];
		const int32 BucketIdx = ActionBucketIndices[i];

		if (BucketIdx == INDEX_NONE)
		{
			if (Score > BestScore)
			{
				BestScore = Score;
				BestAction = RuntimeActions[i];
			}
		}
		else if (Score > BucketBestScores[BucketIdx])
		{
			BucketBestScores[BucketIdx] = Score;
			BucketWinners[BucketIdx] = RuntimeActions[i];
		}
	}

	// Evaluate buckets in priority order (already sorted descending at asset load).
	// Return immediately if a bucket winner clears its cutoff threshold.
	for (int32 j = 0; j < Buckets.Num(); ++j)
	{
		if (!BucketWinners[j])
			continue;

		if (BucketBestScores[j] >= Buckets[j].CutoffThreshold)
			return BucketWinners[j];

		if (BucketBestScores[j] > BestScore)
		{
			BestScore = BucketBestScores[j];
			BestAction = BucketWinners[j];
		}
	}

	return BestAction;
}

float UUtilityBrainComponent::GetEffectiveTickInterval() const
{
	float BaseRate = (TickRateOverride > 0.f)
						 ? TickRateOverride
						 : (BrainAsset ? BrainAsset->GetDefaultTickRate() : 0.2f);

	return BaseRate > 0.f ? BaseRate : 0.2f;
}

// =============================================================================
// Inertia, Cooldowns, and Penalties
// =============================================================================

float UUtilityBrainComponent::CalculateInertiaBonus(UUtilityActionBase *Action) const
{
	if (!Action || !BrainAsset)
		return 1.f;
	if (Action != ActiveAction)
		return 1.f;

	const float Bonus = FMath::Max(0.f, InertiaBonus);
	if (Bonus <= 0.f)
		return 1.f;

	const float RampSeconds = FMath::Max(InertiaRampSeconds, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(ActionElapsedTime / RampSeconds, 0.f, 1.f);
	return 1.f + (Bonus * Alpha);
}

bool UUtilityBrainComponent::IsOnCooldown(const UUtilityActionBase *Action) const
{
	if (!Action || Action->Cooldown <= 0.f)
		return false;

	if (const double *Expiry = CooldownExpiries.Find(Action))
	{
		return GetWorld()->GetTimeSeconds() < *Expiry;
	}

	return false;
}

void UUtilityBrainComponent::StartCooldown(UUtilityActionBase *Action)
{
	if (!Action || Action->Cooldown <= 0.f)
		return;

	CooldownExpiries.Add(Action,
						 GetWorld()->GetTimeSeconds() + Action->Cooldown);
}

void UUtilityBrainComponent::ApplyScorePenalty(UUtilityActionBase *Action)
{
	if (!Action)
		return;

	ScorePenalties.Add(Action, GetWorld()->GetTimeSeconds() + FailurePenaltyDuration);
}

// =============================================================================
// Action Lifecycle
// =============================================================================

void UUtilityBrainComponent::CleanupRuntimeActions()
{
	for (int32 Index = 0; Index < RuntimeActions.Num(); ++Index)
	{
		UUtilityActionBase *Action = RuntimeActions[Index];
		if (!IsValid(Action))
		{
			continue;
		}

		const uint16 Size = Action->GetInstanceMemorySize();
		if (Size == 0)
		{
			continue;
		}

		uint8 *Memory = ActionMemory.GetData() + ActionMemoryOffsets[Index];
		Action->CleanupActionMemory(*this, Memory);
	}
}

void UUtilityBrainComponent::TransitionToAction(UUtilityActionBase *NewAction)
{
	UUtilityActionBase *OldAction = ActiveAction;

	if (OldAction)
	{
		const EActionStatus *OldStatus = ActionStatuses.Find(OldAction);
		if (OldStatus && *OldStatus == EActionStatus::Running)
		{
			uint8 *OldMemory = GetActionMemory(OldAction);
			OldAction->OnActionExited(*this, GetControlledPawn(), EActionStatus::Aborted, OldMemory);
			SetActionStatus(OldAction, EActionStatus::Aborted);
			ApplyScorePenalty(OldAction);
		}
		else if (OldStatus)
		{
			SetActionStatus(OldAction, EActionStatus::Inactive);
		}
	}

	ActiveAction = NewAction;
	if (ActiveAction)
	{
		// Explicitly add the new action with Inactive status
		SetActionStatus(ActiveAction, EActionStatus::Inactive);
	}
	ActionElapsedTime = 0.f;

#if ENABLE_VISUAL_LOG
	UE_VLOG(GetOwner(), LogUtilityAI, Log, TEXT("Transition: %s -> %s"),
			OldAction ? *(OldAction->ActionId.IsNone() ? OldAction->GetClass()->GetName() : OldAction->ActionId.ToString()) : TEXT("none"),
			NewAction ? *(NewAction->ActionId.IsNone() ? NewAction->GetClass()->GetName() : NewAction->ActionId.ToString()) : TEXT("none"));
#endif

	OnActionChanged.Broadcast(OldAction, NewAction);
}

void UUtilityBrainComponent::ExecuteActiveAction(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UtilityAI_ExecuteTask);

	// Cache locally: virtual calls below (OnActionEntered/OnActionTick/OnActionExited) may
	// invoke BP code that calls AbortCurrentAction(), which sets ActiveAction = nullptr via
	// HandleActionCompletion. Operating on the cached pointer keeps all map lookups correct.
	UUtilityActionBase *CurrentAction = ActiveAction;
	if (!CurrentAction)
		return;

	const EActionStatus *CurrentStatus = ActionStatuses.Find(CurrentAction);
	if (!CurrentStatus)
	{
		return; // action not properly initialized in map
	}

	uint8 *Memory = GetActionMemory(CurrentAction);
	const EActionStatus Status = *CurrentStatus;
	APawn *ControlledPawn = GetControlledPawn();

	switch (Status)
	{
	case EActionStatus::Inactive:
	{
		// Fresh task — enter it
		const EActionStatus EnterResult = CurrentAction->OnActionEntered(*this, ControlledPawn, Memory);

		// Guard: BP may have re-entrantly transitioned away from this action
		if (ActiveAction != CurrentAction)
			break;

		ActionStatuses[CurrentAction] = EnterResult;
		OnActionStatusChanged.Broadcast(CurrentAction, EnterResult);

		if (EnterResult != EActionStatus::Running)
		{
			// Task completed instantly (succeeded or failed from OnEnter)
			CurrentAction->OnActionExited(*this, ControlledPawn, EnterResult, Memory);
			if (ActiveAction == CurrentAction)
				HandleActionCompletion(EnterResult);
		}
		break;
	}

	case EActionStatus::Running:
	{
		ActionElapsedTime += DeltaTime;

		if (CurrentAction->Timeout > 0.f && ActionElapsedTime >= CurrentAction->Timeout)
		{
			ActionStatuses[CurrentAction] = EActionStatus::Aborted;
			OnActionStatusChanged.Broadcast(CurrentAction, EActionStatus::Aborted);
			CurrentAction->OnActionExited(*this, ControlledPawn, EActionStatus::Aborted, Memory);
			if (ActiveAction == CurrentAction)
				HandleActionCompletion(EActionStatus::Aborted);
			break;
		}

		// Normal tick
		const EActionStatus TickResult = CurrentAction->OnActionTick(*this, ControlledPawn, DeltaTime, Memory);

		if (TickResult != EActionStatus::Running)
		{
			// Guard: BP may have re-entrantly transitioned away
			if (ActiveAction != CurrentAction)
				break;

			ActionStatuses[CurrentAction] = TickResult;
			OnActionStatusChanged.Broadcast(CurrentAction, TickResult);
			CurrentAction->OnActionExited(*this, ControlledPawn, TickResult, Memory);
			if (ActiveAction == CurrentAction)
				HandleActionCompletion(TickResult);
		}
		break;
	}

	case EActionStatus::Succeeded:
	case EActionStatus::Failed:
	case EActionStatus::Aborted:
	{
		// Terminal state — shouldn't be ticked, force cleanup
		HandleActionCompletion(Status);
		break;
	}
	}
}

void UUtilityBrainComponent::HandleActionCompletion(EActionStatus Result)
{
	if (!ActiveAction)
		return;

#if ENABLE_VISUAL_LOG
	UE_VLOG(GetOwner(), LogUtilityAI, Log, TEXT("Completed: %s [%s]"),
			*(ActiveAction->ActionId.IsNone() ? ActiveAction->GetClass()->GetName() : ActiveAction->ActionId.ToString()),
			Result == EActionStatus::Succeeded ? TEXT("Succeeded") : Result == EActionStatus::Failed ? TEXT("Failed")
																									 : TEXT("Aborted"));
#endif

	switch (Result)
	{
	case EActionStatus::Succeeded:
		StartCooldown(ActiveAction);
		LastSuccessfulExecutionTimes.Add(ActiveAction, GetWorld()->GetTimeSeconds());
		break;

	case EActionStatus::Failed:
	case EActionStatus::Aborted:
		ApplyScorePenalty(ActiveAction);
		break;

	default:
		break;
	}

	if (ActiveAction)
	{
		SetActionStatus(ActiveAction, EActionStatus::Inactive);
	}

	ActiveAction = nullptr;
	ActionElapsedTime = 0.f;

	// Force immediate re-evaluation so the brain doesn't idle for a tick
	ForceEvaluation();
}

// =============================================================================
// Private
// =============================================================================

void UUtilityBrainComponent::SetActionStatus(UUtilityActionBase *Action, EActionStatus NewStatus)
{
	if (!Action)
	{
		return;
	}

	ActionStatuses.Add(Action, NewStatus);
	OnActionStatusChanged.Broadcast(Action, NewStatus);
}

uint8 *UUtilityBrainComponent::GetActionMemory(UUtilityActionBase *Action)
{
	const int32 *Index = ActionIndexByPtr.Find(Action);
	if (!Index || !ActionMemoryOffsets.IsValidIndex(*Index))
	{
		return nullptr;
	}

	// If size == 0, returning nullptr is fine (check in caller if desired)
	return ActionMemory.GetData() + ActionMemoryOffsets[*Index];
}
