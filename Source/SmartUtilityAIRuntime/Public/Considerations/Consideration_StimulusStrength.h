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
#include "Perception/AIPerceptionTypes.h"
#include "Consideration_StimulusStrength.generated.h"

class UAISense;

// Reads the strength value from the most recent perception stimulus. 
// Useful for hearing (loud sound vs quiet footstep) and custom senses (threat pheromone intensity).
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Stimulus Strength"))
class SMARTUTILITYAIRUNTIME_API UConsideration_StimulusStrength : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_StimulusStrength();

	UPROPERTY(EditAnywhere, Category = "Perception")
	TSubclassOf<UAISense> SenseClass;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};