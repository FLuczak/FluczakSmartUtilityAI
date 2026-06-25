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
#include "Consideration_HasPerceptionStimulus.generated.h"

class UAISense;

// Checks if the AI perception component currently has an active stimulus of a given sense 
// (sight, hearing, damage, custom). Returns 1.0 if yes, 0.0 if no. 
// Functionally acts as the gate question: "do I know about a threat?"
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Has Perception Stimulus"))
class SMARTUTILITYAIRUNTIME_API UConsideration_HasPerceptionStimulus : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_HasPerceptionStimulus();

	UPROPERTY(EditAnywhere, Category = "Perception")
	TSubclassOf<UAISense> SenseClass;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};