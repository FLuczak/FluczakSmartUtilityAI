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
#include "Action_Wait.generated.h"

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Wait"))
class SMARTUTILITYAIRUNTIME_API UAction_Wait : public UUtilityActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wait", meta = (ClampMin = "0.0"))
	float Duration = 1.0f;

	virtual uint16 GetInstanceMemorySize() const override;
	virtual EActionStatus OnActionEntered(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, uint8 *ActionMemory) override;
	virtual EActionStatus OnActionTick(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, float DeltaTime, uint8 *ActionMemory) override;
	virtual void OnActionExited(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, EActionStatus Status, uint8 *ActionMemory) override;

private:
	struct FWaitActionMemory
	{
		float ElapsedTime = 0.f;
	};
};
