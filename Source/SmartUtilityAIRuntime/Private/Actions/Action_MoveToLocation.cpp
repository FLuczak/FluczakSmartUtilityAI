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

#include "Actions/Action_MoveToLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/UtilityBrainComponent.h"
#include "Navigation/PathFollowingComponent.h"

bool UAction_MoveToLocation::ResolveLocation(const UUtilityBrainComponent &OwnerComp, FVector &OutLocation) const
{
	if (!TargetLocationBlackboardKey.IsNone())
	{
		if (const UBlackboardComponent *Blackboard = OwnerComp.GetBlackboardComponent())
		{
			OutLocation = Blackboard->GetValueAsVector(TargetLocationBlackboardKey);
			return true;
		}
	}

	OutLocation = TargetLocation;
	return true;
}

EActionStatus UAction_MoveToLocation::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	AAIController *Controller = OwnerComp.GetAIOwner();
	FVector MoveLocation = FVector::ZeroVector;
	if (!Controller || !ResolveLocation(OwnerComp, MoveLocation))
	{
		return EActionStatus::Failed;
	}

	const EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(
		MoveLocation,
		AcceptableRadius,
		bStopOnOverlap,
		bUsePathfinding,
		bAllowPartialPath);

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EActionStatus::Succeeded;
	}

	return Result == EPathFollowingRequestResult::RequestSuccessful ? EActionStatus::Running : EActionStatus::Failed;
}

EActionStatus UAction_MoveToLocation::OnActionTick(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, float DeltaTime, uint8 *ActionMemory)
{
	AAIController *Controller = OwnerComp.GetAIOwner();
	FVector MoveLocation = FVector::ZeroVector;
	if (!Controller || !ControlledPawn || !ResolveLocation(OwnerComp, MoveLocation))
	{
		return EActionStatus::Failed;
	}

	const float DistSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), MoveLocation);
	if (DistSq <= FMath::Square(FMath::Max(0.f, AcceptableRadius)))
	{
		return EActionStatus::Succeeded;
	}

	if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		return EActionStatus::Failed;
	}

	return EActionStatus::Running;
}

void UAction_MoveToLocation::OnActionExited(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, EActionStatus Status, uint8 *ActionMemory)
{
	if (Status == EActionStatus::Aborted)
	{
		if (AAIController *Controller = OwnerComp.GetAIOwner())
		{
			Controller->StopMovement();
		}
	}
}
