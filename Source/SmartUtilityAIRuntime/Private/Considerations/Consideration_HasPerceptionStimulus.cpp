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

#include "Considerations/Consideration_HasPerceptionStimulus.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_HasPerceptionStimulus::UConsideration_HasPerceptionStimulus()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_HasPerceptionStimulus::GetExtraDescription() const
{
	if (!SenseClass)
		return INVTEXT("(no sense)");
	FString Name = SenseClass->GetName();
	Name.RemoveFromStart(TEXT("AISense_"));
	return FText::FromString(Name);
}
#endif

float UConsideration_HasPerceptionStimulus::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const AActor *BrainOwner = OwnerComp.GetOwner();
	if (BrainOwner == nullptr || !SenseClass)
	{
		return 0.f;
	}

	const UAIPerceptionComponent *PerceptionComp = BrainOwner->FindComponentByClass<UAIPerceptionComponent>();
	if (PerceptionComp == nullptr && ControlledPawn && ControlledPawn != BrainOwner)
	{
		PerceptionComp = ControlledPawn->FindComponentByClass<UAIPerceptionComponent>();
	}

	if (PerceptionComp == nullptr)
	{
		return 0.f;
	}

	TArray<AActor *> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(SenseClass, PerceivedActors);
	return PerceivedActors.IsEmpty() ? 0.f : 1.f;
}