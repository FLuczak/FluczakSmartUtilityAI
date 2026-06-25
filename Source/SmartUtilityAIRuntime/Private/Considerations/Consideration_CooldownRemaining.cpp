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

#include "Considerations/Consideration_CooldownRemaining.h"
#include "Core/UtilityBrainComponent.h"
#include "Core/UtilityActionBase.h"
#include "Core/UtilityBrainAsset.h"

UConsideration_CooldownRemaining::UConsideration_CooldownRemaining()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_CooldownRemaining::GetExtraDescription() const
{
	return ActionId.IsNone() ? INVTEXT("(no action)") : FText::FromName(ActionId);
}
#endif

float UConsideration_CooldownRemaining::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	if (ActionId.IsNone())
	{
		return 0.f;
	}

	const float CooldownRemaining = OwnerComp.GetCooldownRemaining(ActionId);
	float MaxCooldown = 0.f;

	if (const UUtilityBrainAsset *BrainAsset = OwnerComp.BrainAsset)
	{
		for (const UUtilityActionBase *Action : BrainAsset->GetActions())
		{
			if (Action && Action->ActionId == ActionId)
			{
				MaxCooldown = Action->Cooldown;
				break;
			}
		}
	}

	if (MaxCooldown > 0.f)
	{
		return FMath::Clamp(CooldownRemaining / MaxCooldown, 0.f, 1.f);
	}

	return 0.f;
}