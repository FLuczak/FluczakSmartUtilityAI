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

#include "Considerations/Consideration_DistanceToLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

#include "Core/UtilityBrainComponent.h"

UConsideration_DistanceToLocation::UConsideration_DistanceToLocation()
{
	InputMin = 0.f;
	InputMax = 2000.f;
}

#if WITH_EDITOR
FText UConsideration_DistanceToLocation::GetExtraDescription() const
{
	if (BlackboardKey.SelectedKeyName.IsNone())
		return FText::GetEmpty();
	return bUse2DDistance
			   ? FText::Format(INVTEXT("to {0} (2D)"), FText::FromName(BlackboardKey.SelectedKeyName))
			   : FText::Format(INVTEXT("to {0}"), FText::FromName(BlackboardKey.SelectedKeyName));
}
#endif

float UConsideration_DistanceToLocation::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
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

	const FVector SourceLocation = SourceActor->GetActorLocation();
	const FVector TargetLocation = BlackboardComp->GetValueAsVector(BlackboardKey.SelectedKeyName);

	return bUse2DDistance
			   ? FVector::Dist2D(SourceLocation, TargetLocation)
			   : FVector::Dist(SourceLocation, TargetLocation);
}