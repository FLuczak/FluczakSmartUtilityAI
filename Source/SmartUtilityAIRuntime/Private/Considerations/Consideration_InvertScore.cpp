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

#include "Considerations/Consideration_InvertScore.h"
#include "Core/UtilityBrainComponent.h"

UConsideration_InvertScore::UConsideration_InvertScore()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_InvertScore::GetExtraDescription() const
{
	if (!ChildConsideration)
		return INVTEXT("(no child)");
	FString Name = ChildConsideration->GetClass()->GetName();
	Name.RemoveFromStart(TEXT("Consideration_"));
	return FText::Format(INVTEXT("NOT {0}"), FText::FromString(Name));
}
#endif

float UConsideration_InvertScore::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	if (ChildConsideration == nullptr)
	{
		return 0.f;
	}

	return 1.f - FMath::Clamp(ChildConsideration->Evaluate(OwnerComp, ControlledPawn), 0.f, 1.f);
}