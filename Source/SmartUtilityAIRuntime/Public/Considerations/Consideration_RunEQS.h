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
#include "EnvironmentQuery/EnvQueryTypes.h"

#include "Consideration_RunEQS.generated.h"

class UEnvQuery;

// Optional EQS compatibility wrapper. Safely delegates consideration scoring 
// to a generated Environment Query on the AI blackboard.
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Run EQS"))
class SMARTUTILITYAIRUNTIME_API UConsideration_RunEQS : public UUtilityConsiderationBase
{
	GENERATED_BODY()

public:
	UConsideration_RunEQS();

	UPROPERTY(EditAnywhere, Category = "EQS")
	TObjectPtr<UEnvQuery> QueryTemplate;

	UPROPERTY(EditAnywhere, Category = "EQS")
	TArray<FEnvNamedValue> QueryConfig;

	UPROPERTY(EditAnywhere, Category = "EQS")
	TEnumAsByte<EEnvQueryRunMode::Type> RunMode;

protected:
	virtual float ComputeRawValue(const class UUtilityBrainComponent& OwnerComp, class APawn* ControlledPawn) const override;
#if WITH_EDITOR
	virtual FText GetExtraDescription() const override;
#endif
};