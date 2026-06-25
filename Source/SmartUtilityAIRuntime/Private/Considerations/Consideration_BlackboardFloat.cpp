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

#include "Considerations/Consideration_BlackboardFloat.h"

#include "Core/SmartUtilityAIRuntime.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Core/UtilityBrainComponent.h"
#include "AIController.h"

UConsideration_BlackboardFloat::UConsideration_BlackboardFloat()
{
}

#if WITH_EDITOR
FText UConsideration_BlackboardFloat::GetExtraDescription() const
{
	if (BlackboardKey.SelectedKeyName.IsNone())
		return FText::GetEmpty();
	return FText::Format(INVTEXT("{0} [{1}, {2}]"),
						 FText::FromName(BlackboardKey.SelectedKeyName),
						 FText::AsNumber(MinValue),
						 FText::AsNumber(MaxValue));
}
#endif

float UConsideration_BlackboardFloat::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr)
	{
		UE_LOG(LogUtilityAI, Warning, TEXT("Consideration_BlackboardFloat: OwnerComp has no BlackboardComponent"));
		return 0.f;
	}

	float Value = BlackboardComp->GetValueAsFloat(BlackboardKey.SelectedKeyName);
	return Value;
}