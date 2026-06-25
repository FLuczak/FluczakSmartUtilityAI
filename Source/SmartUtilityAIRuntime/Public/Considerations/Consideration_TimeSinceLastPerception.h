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
#include "Consideration_TimeSinceLastPerception.generated.h"

class UAISense;

// Time in seconds since the last stimulus of a given sense. 
// With an exponential decay curve, this creates natural "investigation" behavior — 
// urgency is high right after hearing a sound, fades over time.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Time Since Last Perception"))
class SMARTUTILITYAIRUNTIME_API UConsideration_TimeSinceLastPerception : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_TimeSinceLastPerception();

	UPROPERTY(EditAnywhere, Category = "Perception")
	TSubclassOf<UAISense> SenseClass;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};