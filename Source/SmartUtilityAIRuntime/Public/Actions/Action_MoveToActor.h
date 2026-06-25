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
#include "Action_MoveToActor.generated.h"

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Move To Actor"))
class SMARTUTILITYAIRUNTIME_API UAction_MoveToActor : public UUtilityActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	FName TargetActorBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (ClampMin = "0.0"))
	float AcceptableRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	bool bStopOnOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	bool bUsePathfinding = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	bool bAllowPartialPath = true;

	virtual EActionStatus OnActionEntered(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, uint8 *ActionMemory) override;
	virtual EActionStatus OnActionTick(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, float DeltaTime, uint8 *ActionMemory) override;
	virtual void OnActionExited(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, EActionStatus Status, uint8 *ActionMemory) override;

private:
	AActor *ResolveTarget(const class UUtilityBrainComponent &OwnerComp) const;
};
