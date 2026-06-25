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
#include "Core/UtilityBlackboardKeySelector.h"
#include "Engine/EngineTypes.h"
#include "Consideration_LineOfSight.generated.h"

// Traces from the owner to a blackboard actor. Returns 1.0 if clear, 0.0 if blocked. 
// Uses a configurable trace channel and is not thread-safe (uses physics scene). 
// The most basic perception check that doesn't require the full AI perception system.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Line Of Sight"))
class SMARTUTILITYAIRUNTIME_API UConsideration_LineOfSight : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_LineOfSight();

	UPROPERTY(EditAnywhere, Category = "Context")
	FUtilityBlackboardKeySelector BlackboardKey;

	UPROPERTY(EditAnywhere, Category = "Context")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};