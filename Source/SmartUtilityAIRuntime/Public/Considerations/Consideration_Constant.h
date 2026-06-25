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
#include "Consideration_Constant.generated.h"

// Always returns a fixed value. Used as a baseline for default actions 
// (e.g. setting an idle action to 0.5 means "do this when nothing else scores > 0.5"). 
// Also useful for debugging to isolate whether scoring issues are from considerations or curves.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Constant"))
class SMARTUTILITYAIRUNTIME_API UConsideration_Constant : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_Constant();

	UPROPERTY(EditAnywhere, Category = "Utility")
	float ConstantValue = 0.5f;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};