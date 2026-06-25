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

#include "Considerations/Consideration_Random.h"

#include "GameFramework/Actor.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_Random::UConsideration_Random()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_Random::GetExtraDescription() const
{
	return bSeedPerAgent ? INVTEXT("seeded per agent") : FText::GetEmpty();
}
#endif

float UConsideration_Random::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	if (!bSeedPerAgent)
	{
		return FMath::FRandRange(0.f, 1.f);
	}

	uint32 Seed = GetTypeHash(GetFName());
	if (const AActor *OwnerActor = OwnerComp.GetOwner())
	{
		Seed = HashCombineFast(Seed, GetTypeHash(OwnerActor->GetFName()));
	}

	FRandomStream Stream(static_cast<int32>(Seed));
	return Stream.FRandRange(0.f, 1.f);
}