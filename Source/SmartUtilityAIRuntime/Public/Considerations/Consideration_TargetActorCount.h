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
#include "Consideration_TargetActorCount.generated.h"

// Counts actors of a given class or with a given tag within a radius around the owner. 
// Normalized against a configurable max count. Valid for swarm tactics triggering 
// with logistic curves or cooperative behavior with linear scaled curves.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Target Actor Count"))
class SMARTUTILITYAIRUNTIME_API UConsideration_TargetActorCount : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_TargetActorCount();

	UPROPERTY(EditAnywhere, Category = "Context")
	TSubclassOf<AActor> TargetClass;

	UPROPERTY(EditAnywhere, Category = "Context")
	FGameplayTag TargetTag;

	UPROPERTY(EditAnywhere, Category = "Context")
	float Radius = 1000.f;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};