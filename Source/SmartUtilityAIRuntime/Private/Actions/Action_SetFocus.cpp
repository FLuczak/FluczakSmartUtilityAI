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

#include "Actions/Action_SetFocus.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/UtilityBrainComponent.h"

EActionStatus UAction_SetFocus::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	AAIController *Controller = OwnerComp.GetAIOwner();
	if (!Controller)
	{
		return EActionStatus::Failed;
	}

	AActor *ResolvedTarget = TargetActor;
	if (!ResolvedTarget && !TargetActorBlackboardKey.IsNone())
	{
		if (UBlackboardComponent *Blackboard = OwnerComp.GetBlackboardComponent())
		{
			ResolvedTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorBlackboardKey));
		}
	}

	if (!ResolvedTarget)
	{
		return EActionStatus::Failed;
	}

	Controller->SetFocus(ResolvedTarget);
	return EActionStatus::Succeeded;
}
