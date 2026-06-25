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

#include "Considerations/Consideration_Constant.h"
#include "Core/UtilityBrainComponent.h"

UConsideration_Constant::UConsideration_Constant()
{
}

#if WITH_EDITOR
FText UConsideration_Constant::GetExtraDescription() const
{
	return FText::Format(INVTEXT("= {0}"), FText::AsNumber(ConstantValue));
}
#endif

float UConsideration_Constant::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	return ConstantValue;
}