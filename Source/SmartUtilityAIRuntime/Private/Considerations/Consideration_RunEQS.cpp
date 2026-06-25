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

#include "Considerations/Consideration_RunEQS.h"
#include "Core/UtilityBrainComponent.h"
#include "EnvironmentQuery/EnvQuery.h"

UConsideration_RunEQS::UConsideration_RunEQS()
{
	RunMode = EEnvQueryRunMode::SingleResult;
}

#if WITH_EDITOR
FText UConsideration_RunEQS::GetExtraDescription() const
{
	if (!QueryTemplate)
		return INVTEXT("(no query)");
	return FText::FromName(QueryTemplate->GetQueryName());
}
#endif

float UConsideration_RunEQS::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	return 0.f;
}