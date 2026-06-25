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

#include "Considerations/Consideration_TimeSinceLastExecution.h"

#include "Core/UtilityActionBase.h"
#include "Engine/World.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_TimeSinceLastExecution::UConsideration_TimeSinceLastExecution()
{
	InputMin = 0.f;
	InputMax = 30.f;
}

float UConsideration_TimeSinceLastExecution::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const UUtilityActionBase *OwningAction = GetTypedOuter<UUtilityActionBase>();
	if (OwningAction == nullptr)
	{
		return 0.f;
	}

	if (OwningAction->ActionId.IsNone())
	{
		return 0.f;
	}

	const double LastExecutionTime = OwnerComp.GetLastSuccessfulExecutionTime(OwningAction->ActionId);
	if (LastExecutionTime < 0.0)
	{
		return InputMax;
	}

	const UWorld *World = OwnerComp.GetWorld();
	if (World == nullptr)
	{
		return 0.f;
	}

	return FMath::Max(0.f, static_cast<float>(World->GetTimeSeconds() - LastExecutionTime));
}