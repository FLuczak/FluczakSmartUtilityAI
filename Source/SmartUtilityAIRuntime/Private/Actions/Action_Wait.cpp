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

#include "Actions/Action_Wait.h"

#include "Core/UtilityBrainComponent.h"

uint16 UAction_Wait::GetInstanceMemorySize() const
{
	return static_cast<uint16>(sizeof(FWaitActionMemory));
}

EActionStatus UAction_Wait::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	FWaitActionMemory *Memory = CastInstanceActionMemory<FWaitActionMemory>(ActionMemory);
	Memory->ElapsedTime = 0.f;
	return Duration <= 0.f ? EActionStatus::Succeeded : EActionStatus::Running;
}

EActionStatus UAction_Wait::OnActionTick(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, float DeltaTime, uint8 *ActionMemory)
{
	FWaitActionMemory *Memory = CastInstanceActionMemory<FWaitActionMemory>(ActionMemory);
	Memory->ElapsedTime += DeltaTime;
	return Memory->ElapsedTime >= Duration ? EActionStatus::Succeeded : EActionStatus::Running;
}

void UAction_Wait::OnActionExited(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, EActionStatus Status, uint8 *ActionMemory)
{
}
