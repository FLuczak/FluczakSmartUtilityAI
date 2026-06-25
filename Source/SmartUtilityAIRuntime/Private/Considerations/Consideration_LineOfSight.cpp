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

#include "Considerations/Consideration_LineOfSight.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_LineOfSight::UConsideration_LineOfSight()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_LineOfSight::GetExtraDescription() const
{
	if (BlackboardKey.SelectedKeyName.IsNone())
		return FText::GetEmpty();
	return FText::Format(INVTEXT("to {0}"), FText::FromName(BlackboardKey.SelectedKeyName));
}
#endif

float UConsideration_LineOfSight::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr)
	{
		return 0.f;
	}

	const AActor *SourceActor = ControlledPawn ? ControlledPawn : OwnerComp.GetOwner();
	if (SourceActor == nullptr)
	{
		return 0.f;
	}

	const AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(BlackboardKey.SelectedKeyName));
	if (TargetActor == nullptr)
	{
		return 0.f;
	}

	UWorld *World = SourceActor->GetWorld();
	if (World == nullptr)
	{
		return 0.f;
	}

	const FVector TraceStart = SourceActor->GetActorLocation();
	const FVector TraceEnd = TargetActor->GetActorLocation();

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(UtilityAI_LineOfSight), false, SourceActor);
	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, TraceParams);

	if (!bHit)
	{
		return 1.f;
	}

	return HitResult.GetActor() == TargetActor ? 1.f : 0.f;
}