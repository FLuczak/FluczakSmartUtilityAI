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
#include "Core/UtilityBlackboardKeySelector.h"
#include "Action_SetBlackboardValue.generated.h"

UENUM(BlueprintType)
enum class EUtilityBlackboardValueType : uint8
{
	Bool,
	Int,
	Float,
	Vector,
	Object
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Set Blackboard Value"))
class SMARTUTILITYAIRUNTIME_API UAction_SetBlackboardValue : public UUtilityActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FUtilityBlackboardKeySelector KeyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	EUtilityBlackboardValueType ValueType = EUtilityBlackboardValueType::Bool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	float FloatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FVector VectorValue = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	TObjectPtr<UObject> ObjectValue = nullptr;

	virtual EActionStatus OnActionEntered(class UUtilityBrainComponent &OwnerComp, class APawn* ControlledPawn, uint8 *ActionMemory) override;
};
