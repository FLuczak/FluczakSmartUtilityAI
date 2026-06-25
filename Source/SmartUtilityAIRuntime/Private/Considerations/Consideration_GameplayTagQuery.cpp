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

#include "Considerations/Consideration_GameplayTagQuery.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagAssetInterface.h"
#if SMARTUTILITYAI_WITH_GAS
#include "AbilitySystemComponent.h"
#endif

#include "Core/UtilityBrainComponent.h"

UConsideration_GameplayTagQuery::UConsideration_GameplayTagQuery()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_GameplayTagQuery::GetExtraDescription() const
{
	return TagQuery.IsEmpty() ? INVTEXT("(no query)") : INVTEXT("query set");
}
#endif

float UConsideration_GameplayTagQuery::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const AActor *BrainOwner = OwnerComp.GetOwner();
	if (BrainOwner == nullptr || TagQuery.IsEmpty())
	{
		return 0.f;
	}

	FGameplayTagContainer OwnerTags;
	bool bFoundAnyTags = false;

	auto GatherTagsFromActor = [&OwnerTags, &bFoundAnyTags](const AActor *CandidateActor)
	{
		if (CandidateActor == nullptr)
		{
			return;
		}

		if (const IGameplayTagAssetInterface *TagInterface = Cast<IGameplayTagAssetInterface>(CandidateActor))
		{
			FGameplayTagContainer ActorTags;
			TagInterface->GetOwnedGameplayTags(ActorTags);
			OwnerTags.AppendTags(ActorTags);
			bFoundAnyTags = true;
		}

#if SMARTUTILITYAI_WITH_GAS
		if (const UAbilitySystemComponent *AbilitySystemComp = CandidateActor->FindComponentByClass<UAbilitySystemComponent>())
		{
			AbilitySystemComp->GetOwnedGameplayTags(OwnerTags);
			bFoundAnyTags = true;
		}
#endif
	};

	GatherTagsFromActor(BrainOwner);

	if (ControlledPawn && ControlledPawn != BrainOwner)
	{
		GatherTagsFromActor(ControlledPawn);
	}
	else if (const APawn *PawnOwner = Cast<APawn>(BrainOwner))
	{
		GatherTagsFromActor(PawnOwner->GetController());
	}

	if (!bFoundAnyTags)
	{
		return 0.f;
	}

	return TagQuery.Matches(OwnerTags) ? 1.f : 0.f;
}