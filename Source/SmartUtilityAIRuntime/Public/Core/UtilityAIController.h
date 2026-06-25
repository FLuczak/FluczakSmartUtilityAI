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
#include "AIController.h"
#include "UtilityAIController.generated.h"

UCLASS()
class SMARTUTILITYAIRUNTIME_API AUtilityAIController : public AAIController
{
	GENERATED_BODY()

public:
	AUtilityAIController(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

	// Initializes the brain component with the given asset and starts logic; returns false if the asset is invalid.
	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	bool RunUtilityBrain(class UUtilityBrainAsset *InBrainAsset);

	// Stops and cleans up the active brain logic.
	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	void StopUtilityBrain(const FString &Reason = TEXT("StopUtilityBrain"));

	// Returns the brain component owned by this controller.
	UFUNCTION(BlueprintPure, Category = "Utility AI")
	class UUtilityBrainComponent *GetUtilityBrainComponent() const { return UtilityBrainComponent; }

protected:
	// The brain component created in the constructor and driven by RunUtilityBrain.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Utility AI")
	TObjectPtr<class UUtilityBrainComponent> UtilityBrainComponent;
};
