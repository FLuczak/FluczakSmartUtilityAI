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
#include "Consideration_BlackboardBool.generated.h"

// Reads a bool key from the blackboard. Returns 1.0 when true, 0.0 when false
// (inverted if bInvert is set). The response curve still applies, so a designer
// can return 0.8 instead of 1.0 to make the gate influence rather than dictate.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (DisplayName = "Blackboard Bool"))
class SMARTUTILITYAIRUNTIME_API UConsideration_BlackboardBool : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_BlackboardBool();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FUtilityBlackboardKeySelector BlackboardKey;

	// When true, returns 0.0 for a true key and 1.0 for a false key.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	bool bInvert = false;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};
