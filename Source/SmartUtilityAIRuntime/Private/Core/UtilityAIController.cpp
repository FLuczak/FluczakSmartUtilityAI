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

#include "Core/UtilityAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityBrainComponent.h"

AUtilityAIController::AUtilityAIController(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	UtilityBrainComponent = CreateDefaultSubobject<UUtilityBrainComponent>(TEXT("UtilityBrainComponent"));
	BrainComponent = UtilityBrainComponent;
}

bool AUtilityAIController::RunUtilityBrain(UUtilityBrainAsset *InBrainAsset)
{
	if (!UtilityBrainComponent || !InBrainAsset)
	{
		return false;
	}

	if (InBrainAsset->GetBlackboardAsset())
	{
		UBlackboardComponent *CreatedBlackboard = nullptr;
		UseBlackboard(InBrainAsset->GetBlackboardAsset(), CreatedBlackboard);
	}

	UtilityBrainComponent->BrainAsset = InBrainAsset;
	UtilityBrainComponent->StartLogic();
	return UtilityBrainComponent->IsRunning();
}

void AUtilityAIController::StopUtilityBrain(const FString &Reason)
{
	if (UtilityBrainComponent)
	{
		UtilityBrainComponent->StopLogic(Reason);
	}
}
