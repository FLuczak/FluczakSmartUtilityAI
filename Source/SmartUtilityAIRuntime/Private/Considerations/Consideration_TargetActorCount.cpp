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

#include "Considerations/Consideration_TargetActorCount.h"

#include "AIController.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagAssetInterface.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_TargetActorCount::UConsideration_TargetActorCount()
{
	InputMin = 0.f;
	InputMax = 10.f;
}

#if WITH_EDITOR
FText UConsideration_TargetActorCount::GetExtraDescription() const
{
	FText Subject;
	if (TargetClass)
	{
		FString Name = TargetClass->GetName();
		Name.RemoveFromEnd(TEXT("_C"));
		Subject = FText::FromString(Name);
	}
	else if (TargetTag.IsValid())
	{
		Subject = FText::FromName(TargetTag.GetTagName());
	}
	else
	{
		Subject = INVTEXT("any");
	}
	return FText::Format(INVTEXT("{0} within {1}"), Subject, FText::AsNumber(Radius));
}
#endif

float UConsideration_TargetActorCount::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const AActor *SourceActor = ControlledPawn ? ControlledPawn : OwnerComp.GetOwner();
	if (SourceActor == nullptr)
	{
		return 0.f;
	}

	UWorld *World = SourceActor->GetWorld();
	if (World == nullptr)
	{
		return 0.f;
	}

	const float RadiusSq = FMath::Square(FMath::Max(0.f, Radius));
	int32 MatchingCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor *Candidate = *It;
		if (Candidate == nullptr || Candidate == SourceActor)
		{
			continue;
		}

		if (TargetClass && !Candidate->IsA(TargetClass))
		{
			continue;
		}

		if (FVector::DistSquared(SourceActor->GetActorLocation(), Candidate->GetActorLocation()) > RadiusSq)
		{
			continue;
		}

		if (TargetTag.IsValid())
		{
			const IGameplayTagAssetInterface *TagInterface = Cast<IGameplayTagAssetInterface>(Candidate);
			if (TagInterface == nullptr)
			{
				continue;
			}

			FGameplayTagContainer CandidateTags;
			TagInterface->GetOwnedGameplayTags(CandidateTags);
			if (!CandidateTags.HasTag(TargetTag))
			{
				continue;
			}
		}

		++MatchingCount;
	}

	return static_cast<float>(MatchingCount);
}