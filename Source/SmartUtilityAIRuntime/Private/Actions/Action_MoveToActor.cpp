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

#include "Actions/Action_MoveToActor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/UtilityBrainComponent.h"
#include "Navigation/PathFollowingComponent.h"

AActor *UAction_MoveToActor::ResolveTarget(const UUtilityBrainComponent &OwnerComp) const
{
	if (TargetActor)
	{
		return TargetActor;
	}

	if (!TargetActorBlackboardKey.IsNone())
	{
		if (const UBlackboardComponent *Blackboard = OwnerComp.GetBlackboardComponent())
		{
			return Cast<AActor>(Blackboard->GetValueAsObject(TargetActorBlackboardKey));
		}
	}

	return nullptr;
}

EActionStatus UAction_MoveToActor::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	AAIController *Controller = OwnerComp.GetAIOwner();
	AActor *MoveTarget = ResolveTarget(OwnerComp);
	if (!Controller || !MoveTarget)
	{
		return EActionStatus::Failed;
	}

	const EPathFollowingRequestResult::Type Result = Controller->MoveToActor(
		MoveTarget,
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

EActionStatus UAction_MoveToActor::OnActionTick(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, float DeltaTime, uint8 *ActionMemory)
{
	AAIController *Controller = OwnerComp.GetAIOwner();
	AActor *MoveTarget = ResolveTarget(OwnerComp);
	if (!Controller || !ControlledPawn || !MoveTarget)
	{
		return EActionStatus::Failed;
	}

	const float DistSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), MoveTarget->GetActorLocation());
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

void UAction_MoveToActor::OnActionExited(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, EActionStatus Status, uint8 *ActionMemory)
{
	if (Status == EActionStatus::Aborted)
	{
		if (AAIController *Controller = OwnerComp.GetAIOwner())
		{
			Controller->StopMovement();
		}
	}
}
