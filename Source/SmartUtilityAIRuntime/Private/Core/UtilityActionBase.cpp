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

#include "Core/UtilityActionBase.h"
#include "GameFramework/Pawn.h"

UUtilityActionBase::UUtilityActionBase()
{
}

uint16 UUtilityActionBase::GetInstanceMemorySize() const
{
	return 0;
}

void UUtilityActionBase::InitializeActionMemory(UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const
{
}

void UUtilityActionBase::CleanupActionMemory(UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const
{
}

EActionStatus UUtilityActionBase::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	// By default, actions start in the Running state, but this can be overridden by child classes to allow for actions that execute entirely within a single tick or that have conditions for starting that may not be met when the action is selected.
	return EActionStatus::Running;
}

EActionStatus UUtilityActionBase::OnActionTick(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, float DeltaTime, uint8 *ActionMemory)
{
	return EActionStatus::Running;
}

void UUtilityActionBase::OnActionExited(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, EActionStatus Status, uint8 *ActionMemory)
{
}

#if WITH_EDITOR
void UUtilityActionBase::AddConsideration(UUtilityConsiderationBase *Consideration)
{
	if (Consideration)
	{
		Considerations.Add(Consideration);
	}
}

void UUtilityActionBase::RemoveConsideration(UUtilityConsiderationBase *Consideration)
{
	Considerations.Remove(Consideration);
}
#endif
