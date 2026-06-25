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

#include "UtilityBrainDebugger.h"
#include "Core/UtilityBrainComponent.h"
#include "Core/UtilityBrainDebugComponent.h"
#include "Core/UtilityActionBase.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"

FUtilityBrainDebugger::FUtilityBrainDebugger()
{
	bShowOnlyWithDebugActor = true;
}

TSharedRef<FGameplayDebuggerCategory> FUtilityBrainDebugger::MakeInstance()
{
	return MakeShared<FUtilityBrainDebugger>();
}

UUtilityBrainComponent *FUtilityBrainDebugger::GetBrainComponent(AActor *Actor)
{
	if (!Actor)
		return nullptr;

	// Debug actor is typically a pawn — check its controller first
	if (const APawn *Pawn = Cast<APawn>(Actor))
	{
		if (AController *Controller = Pawn->GetController())
		{
			if (UUtilityBrainComponent *Comp = Controller->FindComponentByClass<UUtilityBrainComponent>())
			{
				return Comp;
			}
		}
	}

	return Actor->FindComponentByClass<UUtilityBrainComponent>();
}

static UUtilityBrainDebugComponent *EnsureDebugComponent(UUtilityBrainComponent *Brain)
{
	UUtilityBrainDebugComponent *DbgComp = Brain->GetDebugComponent();
	if (DbgComp)
	{
		return DbgComp;
	}

	AActor *Owner = Brain->GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	DbgComp = NewObject<UUtilityBrainDebugComponent>(Owner, NAME_None, RF_Transient);
	DbgComp->RegisterComponent();
	return DbgComp;
}

void FUtilityBrainDebugger::CollectData(APlayerController *OwnerPC, AActor *DebugActor)
{
	CachedActions.Reset();
	ActiveActionName = TEXT("None");
	CachedEvalCount = 0;

	UUtilityBrainComponent *Comp = GetBrainComponent(DebugActor);
	if (!IsValid(Comp))
		return;

	UUtilityBrainDebugComponent *DbgComp = EnsureDebugComponent(Comp);
	if (!DbgComp)
		return;

	DbgComp->SetCaptureEnabled(true);

	const FBrainDebugSnapshot &Snapshot = DbgComp->GetSnapshot();
	CachedEvalCount = Snapshot.EvaluationCount;

	const UUtilityActionBase *ActiveAction = Snapshot.ActiveAction.Get();
	if (ActiveAction)
	{
		ActiveActionName = ActiveAction->ActionId.IsNone()
							   ? ActiveAction->GetClass()->GetName()
							   : ActiveAction->ActionId.ToString();
	}

	for (const FActionDebugInfo &Info : Snapshot.ActionInfos)
	{
		FActionDisplayData Data;
		Data.Name = Info.DisplayName.IsEmpty() ? TEXT("?") : Info.DisplayName;
		Data.FinalScore = Info.FinalScore;
		Data.PreModifierScore = Info.PreModifierScore;
		Data.InertiaBonusApplied = Info.InertiaBonusApplied;
		Data.PenaltyMultiplierApplied = Info.PenaltyMultiplierApplied;
		Data.bIsActive = Info.Action.IsValid() && Info.Action.Get() == ActiveAction;
		Data.bOnCooldown = Info.bOnCooldown;

		if (Data.bIsActive)
		{
			Data.ConsiderationScores = Info.ConsiderationScores;
		}

		CachedActions.Add(MoveTemp(Data));
	}
}

void FUtilityBrainDebugger::DrawData(APlayerController *OwnerPC, FGameplayDebuggerCanvasContext &CanvasContext)
{
	CanvasContext.Print(FColor::White, FString::Printf(TEXT("[Utility AI]  Active: %s  Evals: %d"),
													   *ActiveActionName, CachedEvalCount));

	CanvasContext.MoveToNewLine();

	if (CachedActions.IsEmpty())
	{
		CanvasContext.Print(FColor::Silver, TEXT("No brain data — brain may not be running."));
		return;
	}

	// Sort by final score descending
	TArray<const FActionDisplayData *> Sorted;
	for (const FActionDisplayData &A : CachedActions)
		Sorted.Add(&A);
	Sorted.Sort([](const FActionDisplayData &A, const FActionDisplayData &B)
				{ return A.FinalScore > B.FinalScore; });

	// Bar chart: 30 characters wide
	constexpr int32 BarWidth = 30;

	for (const FActionDisplayData *Action : Sorted)
	{
		const int32 Filled = FMath::RoundToInt(Action->FinalScore * BarWidth);
		const FString Bar = FString::ChrN(Filled, TEXT('#')) + FString::ChrN(BarWidth - Filled, TEXT('-'));

		FString Suffix;
		if (Action->bIsActive)
			Suffix = TEXT(" [ACTIVE]");
		else if (Action->bOnCooldown)
			Suffix = TEXT(" [COOLDOWN]");
		else if (Action->PenaltyMultiplierApplied < 0.99f)
			Suffix = TEXT(" [PENALTY]");

		const FColor BarColor = Action->bIsActive						   ? FColor::Green
								: Action->bOnCooldown					   ? FColor::Silver
								: Action->PenaltyMultiplierApplied < 0.99f ? FColor::Orange
																		   : FColor::Cyan;

		CanvasContext.Print(BarColor, FString::Printf(TEXT("[%s] %.2f  %s%s"),
													  *Bar, Action->FinalScore, *Action->Name, *Suffix));
	}

	// Consideration breakdown for the active action
	for (const FActionDisplayData &Action : CachedActions)
	{
		if (!Action.bIsActive || Action.ConsiderationScores.IsEmpty())
			continue;

		CanvasContext.MoveToNewLine();
		CanvasContext.Print(FColor::Yellow,
							FString::Printf(TEXT("[%s] Considerations:"), *Action.Name));

		for (int32 k = 0; k < Action.ConsiderationScores.Num(); ++k)
		{
			const FConsiderationDebugInfo &C = Action.ConsiderationScores[k];
			CanvasContext.Print(FColor::White,
								FString::Printf(TEXT("  [%d]  Raw=%.3f  Norm=%.3f  Score=%.3f"),
												k, C.RawValue, C.NormalizedScore, C.FinalScore));
		}
		break;
	}
}
