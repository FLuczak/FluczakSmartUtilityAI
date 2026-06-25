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
#include "GameplayTagContainer.h"
#include "BucketEditProxy.generated.h"

/**
 * Transient editor-only UObject that mirrors a single FBucket's properties
 * so the standard IDetailsView can display and edit them. Changes are written
 * back to the owning UUtilityBrainAsset on every PostEditChangeProperty.
 */
UCLASS(Transient, MinimalAPI)
class UBucketEditProxy : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Bucket")
	FGameplayTag BucketTag;

	UPROPERTY(EditAnywhere, Category = "Bucket", meta = (ClampMin = "0"))
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, Category = "Bucket", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CutoffThreshold = 0.3f;

	TWeakObjectPtr<class UUtilityBrainAsset> AssetPtr;
	int32 BucketIndex = -1;

	// Called after every successful write-back (used to refresh the action tree).
	TFunction<void()> OnBucketModified;

	void PopulateFrom(const struct FBucket &Bucket);

	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

private:
	void WriteBack();
};
