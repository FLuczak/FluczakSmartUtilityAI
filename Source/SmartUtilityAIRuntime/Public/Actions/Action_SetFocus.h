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
#include "Core/UtilityActionBase.h"
#include "Action_SetFocus.generated.h"

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Set Focus"))
class SMARTUTILITYAIRUNTIME_API UAction_SetFocus : public UUtilityActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus")
	FName TargetActorBlackboardKey;

	virtual EActionStatus OnActionEntered(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, uint8 *ActionMemory) override;
};
