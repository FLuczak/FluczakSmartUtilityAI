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
#include "Consideration_InvertScore.generated.h"

// Wraps another consideration instance and returns 1 MINUS its score. 
// A decorator pattern that saves designers from flipping every curve manually 
// (e.g. "not in line of sight" without duplicating the LOS consideration).
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Invert Score"))
class SMARTUTILITYAIRUNTIME_API UConsideration_InvertScore : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_InvertScore();

	UPROPERTY(EditAnywhere, Instanced, Category = "Utility")
	TObjectPtr<UUtilityConsiderationBase> ChildConsideration;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};