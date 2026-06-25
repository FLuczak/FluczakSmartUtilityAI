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

#pragma once

#include "CoreMinimal.h"
#include "Core/UtilityConsiderationBase.h"
#include "GameplayTagContainer.h"
#include "Consideration_GameplayTagQuery.generated.h"

// Evaluates a FGameplayTagQuery against the owner's tag container. 
// Returns 1.0 if the query matches, 0.0 if not. 
// Covers state checks like "am I stunned," "do I have the Burning status," etc.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Gameplay Tag Query"))
class SMARTUTILITYAIRUNTIME_API UConsideration_GameplayTagQuery : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_GameplayTagQuery();

	UPROPERTY(EditAnywhere, Category = "Self-state")
	FGameplayTagQuery TagQuery;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};