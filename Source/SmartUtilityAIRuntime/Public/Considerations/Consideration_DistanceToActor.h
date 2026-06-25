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
#include "Consideration_DistanceToActor.generated.h"

// Distance from the owner to a blackboard actor key. Configurable 2D/3D. 
// Uses max distance for normalization. The most-used consideration in any game.
// With a bell curve: prefer medium range. With exponential decay: urgency when close. 
// With inverted linear: urgency when far.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Distance To Actor"))
class SMARTUTILITYAIRUNTIME_API UConsideration_DistanceToActor : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_DistanceToActor();

	UPROPERTY(EditAnywhere, Category = "Spatial")
	FUtilityBlackboardKeySelector BlackboardKey;

	UPROPERTY(EditAnywhere, Category = "Spatial")
	bool bUse2DDistance = false;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};