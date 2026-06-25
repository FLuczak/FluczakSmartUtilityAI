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
#include "UtilityAITypes.h"
#include "UtilityActionBase.h"
#include "UtilityActionBPBase.generated.h"

/**
 * Base class for Blueprint-defined utility actions.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class SMARTUTILITYAIRUNTIME_API UUtilityActionBPBase : public UUtilityActionBase
{
	GENERATED_BODY()

public:
	UUtilityActionBPBase();

	// Calls ReceiveOnActionEntered and returns the pending status set by SubmitActionResult.
	virtual EActionStatus OnActionEntered(class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn, uint8 *Memory) override;
	// Calls ReceiveOnActionTick and returns the pending status set by SubmitActionResult.
	virtual EActionStatus OnActionTick(class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn, float DeltaTime, uint8 *Memory) override;
	// Calls ReceiveOnActionExited to notify Blueprint of the terminal status.
	virtual void OnActionExited(class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn, EActionStatus Status, uint8 *Memory) override;

	/** Signal the action result from Blueprint. Call Succeeded or Failed to finish; Running keeps the action alive. */
	UFUNCTION(BlueprintCallable, Category = "Utility Action")
	void SubmitActionResult(EActionStatus Status);

	/** Called when the action starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Utility Action", meta = (DisplayName = "On Action Entered"))
	void ReceiveOnActionEntered(class UUtilityBrainComponent *OwnerComp, class APawn *ControlledPawn);

	/** Called each tick while the action is running */
	UFUNCTION(BlueprintImplementableEvent, Category = "Utility Action", meta = (DisplayName = "On Action Ticked"))
	void ReceiveOnActionTick(class UUtilityBrainComponent *OwnerComp, class APawn *ControlledPawn, float DeltaTime);

	/** Called when the action finishes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Utility Action", meta = (DisplayName = "On Action Finished"))
	void ReceiveOnActionExited(class UUtilityBrainComponent *OwnerComp, class APawn *ControlledPawn, EActionStatus Status);

	// Blueprint actions always require a runtime instance so per-component state is isolated.
	virtual bool ShouldCreateInstance() const override { return true; };

private:
	// Status set by SubmitActionResult and consumed on the next OnActionEntered/OnActionTick call.
	EActionStatus PendingStatus;
};
