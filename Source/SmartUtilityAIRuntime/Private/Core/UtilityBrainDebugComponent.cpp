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

#include "Core/UtilityBrainDebugComponent.h"
#include "Core/UtilityBrainComponent.h"

UUtilityBrainDebugComponent::UUtilityBrainDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUtilityBrainDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor *Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (UUtilityBrainComponent *Brain = Owner->FindComponentByClass<UUtilityBrainComponent>())
	{
		Brain->RegisterDebugComponent(this);
	}
}

void UUtilityBrainDebugComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AActor *Owner = GetOwner();
	if (Owner)
	{
		if (UUtilityBrainComponent *Brain = Owner->FindComponentByClass<UUtilityBrainComponent>())
		{
			Brain->UnregisterDebugComponent(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UUtilityBrainDebugComponent::SetCaptureEnabled(bool bEnabled)
{
	bCaptureEnabled = bEnabled;
	if (!bEnabled)
	{
		Snapshot = FBrainDebugSnapshot{};
		ConsiderationScratch.Reset();
	}
}

void UUtilityBrainDebugComponent::BeginEvaluationCapture(int32 ActionCount)
{
	Snapshot.ActionInfos.SetNum(ActionCount);
}

void UUtilityBrainDebugComponent::CommitActionInfo(int32 Index, const FActionDebugInfo &Info)
{
	if (Snapshot.ActionInfos.IsValidIndex(Index))
	{
		Snapshot.ActionInfos[Index] = Info;
	}
}

void UUtilityBrainDebugComponent::FinalizeCapture(TWeakObjectPtr<UUtilityActionBase> ActiveAction, int32 EvalCount)
{
	Snapshot.ActiveAction = ActiveAction;
	Snapshot.EvaluationCount = EvalCount;
}
