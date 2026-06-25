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

#include "Core/UtilityActionBPBase.h"
#include "GameFramework/Pawn.h"

UUtilityActionBPBase::UUtilityActionBPBase()
	: PendingStatus(EActionStatus::Running)
{
}

void UUtilityActionBPBase::SubmitActionResult(EActionStatus Status)
{
	PendingStatus = Status;
}

EActionStatus UUtilityActionBPBase::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *Memory)
{
	PendingStatus = EActionStatus::Running;
	ReceiveOnActionEntered(&OwnerComp, ControlledPawn);
	return PendingStatus;
}

EActionStatus UUtilityActionBPBase::OnActionTick(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, float DeltaTime, uint8 *Memory)
{
	if (PendingStatus != EActionStatus::Running)
	{
		return PendingStatus;
	}
	ReceiveOnActionTick(&OwnerComp, ControlledPawn, DeltaTime);
	return PendingStatus;
}

void UUtilityActionBPBase::OnActionExited(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, EActionStatus Status, uint8 *Memory)
{
	PendingStatus = EActionStatus::Running;
	ReceiveOnActionExited(&OwnerComp, ControlledPawn, Status);
}
