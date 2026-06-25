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

#include "Considerations/Consideration_BlackboardBool.h"

#include "Core/SmartUtilityAIRuntime.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Core/UtilityBrainComponent.h"
#include "AIController.h"

UConsideration_BlackboardBool::UConsideration_BlackboardBool()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_BlackboardBool::GetExtraDescription() const
{
	if (BlackboardKey.SelectedKeyName.IsNone())
		return FText::GetEmpty();
	return FText::Format(INVTEXT("{0} == {1}"),
						 FText::FromName(BlackboardKey.SelectedKeyName),
						 bInvert ? INVTEXT("false") : INVTEXT("true"));
}
#endif

float UConsideration_BlackboardBool::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr)
	{
		UE_LOG(LogUtilityAI, Warning, TEXT("Consideration_BlackboardBool: OwnerComp has no BlackboardComponent"));
		return 0.f;
	}

	return BlackboardComp->GetValueAsBool(BlackboardKey.SelectedKeyName) != bInvert ? 1.f : 0.f;
}