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
#include "Engine/DataAsset.h"

#include "BehaviorTree/BlackboardData.h"
#include "UtilityAITypes.h"

#include "UtilityBrainAsset.generated.h"

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class SMARTUTILITYAIRUNTIME_API UUtilityBrainAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UUtilityBrainAsset();

	// Blackboard data asset — defines the keys available to considerations.
	// If null, the brain component will attempt to use an existing BB on the
	// owning actor/controller, but considerations won't be validated against
	// a known key set.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	TObjectPtr<UBlackboardData> BlackboardAsset;

	// All actions this brain can choose from.
	// Each action is an instanced subobject owned by this asset.
	// The array order has no effect on scoring — bucket priority and scores
	// determine selection. Order only affects editor display.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Actions")
	TArray<TObjectPtr<class UUtilityActionBase>> Actions;

	// Priority buckets. Actions are assigned to buckets via GameplayTags.
	// Higher-priority buckets are evaluated first. If any action in a bucket
	// scores above the bucket's cutoff threshold, lower buckets are skipped.
	//
	// Example: Survival(100) > Combat(50) > Idle(10)
	// If a Survival action scores 0.6 (above its 0.3 cutoff), Combat and
	// Idle actions are never evaluated — saving performance and ensuring
	// the agent always prioritizes survival over combat.
	//
	// If empty, all actions compete in a flat pool (highest score wins).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buckets",
			  meta = (TitleProperty = "BucketTag"))
	TArray<FBucket> Buckets;

	// Global score threshold — actions scoring below this are ignored.
	// Prevents near-zero actions from winning in sparse scenarios.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults",
			  meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumScoreThreshold = 0.01f;

	// Returns all actions registered in this asset.
	UFUNCTION(BlueprintPure, Category = "Brain")
	const TArray<class UUtilityActionBase *> &GetActions() const { return Actions; }

	// Returns all priority buckets defined in this asset.
	UFUNCTION(BlueprintPure, Category = "Brain")
	const TArray<FBucket> &GetBuckets() const { return Buckets; }

	// Returns the blackboard data asset associated with this brain.
	UFUNCTION(BlueprintPure, Category = "Brain")
	UBlackboardData *GetBlackboardAsset() const { return BlackboardAsset; }

	// Returns the minimum score threshold below which actions are ignored.
	UFUNCTION(BlueprintPure, Category = "Brain")
	float GetMinimumScoreThreshold() const { return MinimumScoreThreshold; }

	// Returns the bucket an action belongs to, or null if the action is unbucketed.
	const FBucket *GetBucketForAction(const class UUtilityActionBase *Action) const;

	// Returns the brain evaluation tick rate in seconds.
	float GetDefaultTickRate() const { return DefaultTickRate; }

	// Ensures action IDs are unique after asset load.
	virtual void PostLoad() override;

#if WITH_EDITOR
	// Re-sorts buckets and validates the asset after a property change in the editor.
	virtual void PostEditChangeProperty(
		FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

private:
	// How often the brain evaluates actions (seconds between ticks).
	UPROPERTY(EditDefaultsOnly, Category = "Brain", meta = (ClampMin = "0.01", ClampMax = "60.0"))
	float DefaultTickRate = 0.1f;

	// Ensures every action in the asset has a unique ActionId.
	void EnsureUniqueActionIds();

	// Sorts buckets by priority descending so evaluation visits highest-priority first.
	void SortBuckets();
};
