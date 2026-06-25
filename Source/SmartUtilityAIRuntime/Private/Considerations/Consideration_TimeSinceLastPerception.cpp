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

#include "Considerations/Consideration_TimeSinceLastPerception.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_TimeSinceLastPerception::UConsideration_TimeSinceLastPerception()
{
	InputMin = 0.f;
	InputMax = 15.f;
}

#if WITH_EDITOR
FText UConsideration_TimeSinceLastPerception::GetExtraDescription() const
{
	if (!SenseClass)
		return INVTEXT("(no sense)");
	FString Name = SenseClass->GetName();
	Name.RemoveFromStart(TEXT("AISense_"));
	return FText::FromString(Name);
}
#endif

float UConsideration_TimeSinceLastPerception::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const AActor *BrainOwner = OwnerComp.GetOwner();
	if (BrainOwner == nullptr || !SenseClass)
	{
		return InputMax;
	}

	UAIPerceptionComponent *PerceptionComp = BrainOwner->FindComponentByClass<UAIPerceptionComponent>();
	if (PerceptionComp == nullptr && ControlledPawn && ControlledPawn != BrainOwner)
	{
		PerceptionComp = ControlledPawn->FindComponentByClass<UAIPerceptionComponent>();
	}

	if (PerceptionComp == nullptr)
	{
		return InputMax;
	}

	const FAISenseID SenseID = UAISense::GetSenseID(SenseClass);
	if (!SenseID.IsValid())
	{
		return InputMax;
	}

	float YoungestAge = TNumericLimits<float>::Max();
	TArray<AActor *> KnownActors;
	PerceptionComp->GetKnownPerceivedActors(SenseClass, KnownActors);

	for (AActor *Actor : KnownActors)
	{
		if (Actor == nullptr)
		{
			continue;
		}

		FActorPerceptionBlueprintInfo PerceptionInfo;
		if (!PerceptionComp->GetActorsPerception(Actor, PerceptionInfo))
		{
			continue;
		}

		for (const FAIStimulus &Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if (Stimulus.Type != SenseID || !Stimulus.IsValid())
			{
				continue;
			}

			YoungestAge = FMath::Min(YoungestAge, Stimulus.GetAge());
		}
	}

	return YoungestAge == TNumericLimits<float>::Max() ? InputMax : YoungestAge;
}