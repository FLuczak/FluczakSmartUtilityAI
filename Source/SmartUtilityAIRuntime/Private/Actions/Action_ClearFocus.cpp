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

#include "Actions/Action_ClearFocus.h"

#include "AIController.h"
#include "Core/UtilityBrainComponent.h"

EActionStatus UAction_ClearFocus::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	if (AAIController *Controller = OwnerComp.GetAIOwner())
	{
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
		return EActionStatus::Succeeded;
	}

	return EActionStatus::Failed;
}
