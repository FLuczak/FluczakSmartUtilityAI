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

#include "Considerations/Consideration_StimulusStrength.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_StimulusStrength::UConsideration_StimulusStrength()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_StimulusStrength::GetExtraDescription() const
{
	if (!SenseClass)
		return INVTEXT("(no sense)");
	FString Name = SenseClass->GetName();
	Name.RemoveFromStart(TEXT("AISense_"));
	return FText::FromString(Name);
}
#endif

float UConsideration_StimulusStrength::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const AActor *BrainOwner = OwnerComp.GetOwner();
	if (BrainOwner == nullptr || !SenseClass)
	{
		return 0.f;
	}

	UAIPerceptionComponent *PerceptionComp = BrainOwner->FindComponentByClass<UAIPerceptionComponent>();
	if (PerceptionComp == nullptr && ControlledPawn && ControlledPawn != BrainOwner)
	{
		PerceptionComp = ControlledPawn->FindComponentByClass<UAIPerceptionComponent>();
	}

	if (PerceptionComp == nullptr)
	{
		return 0.f;
	}

	const FAISenseID SenseID = UAISense::GetSenseID(SenseClass);
	if (!SenseID.IsValid())
	{
		return 0.f;
	}

	float MostRecentAge = TNumericLimits<float>::Max();
	float MostRecentStrength = 0.f;

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

			const float StimulusAge = Stimulus.GetAge();
			if (StimulusAge < MostRecentAge)
			{
				MostRecentAge = StimulusAge;
				MostRecentStrength = Stimulus.Strength;
			}
		}
	}

	return MostRecentStrength;
}