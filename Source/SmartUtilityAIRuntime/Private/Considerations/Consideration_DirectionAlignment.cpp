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

#include "Considerations/Consideration_DirectionAlignment.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_DirectionAlignment::UConsideration_DirectionAlignment()
{
	InputMin = -1.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_DirectionAlignment::GetExtraDescription() const
{
	if (TargetKey.SelectedKeyName.IsNone())
		return FText::GetEmpty();
	return FText::Format(INVTEXT("→ {0}"), FText::FromName(TargetKey.SelectedKeyName));
}
#endif

float UConsideration_DirectionAlignment::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
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

	const AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	if (TargetActor == nullptr)
	{
		return 0.f;
	}

	const FVector ToTarget = TargetActor->GetActorLocation() - SourceActor->GetActorLocation();
	if (ToTarget.IsNearlyZero())
	{
		return 1.f;
	}

	const FVector DirectionToTarget = ToTarget.GetSafeNormal();
	const FVector Forward = SourceActor->GetActorForwardVector();
	return FVector::DotProduct(Forward, DirectionToTarget);
}