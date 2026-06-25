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

#include "BucketEditProxy.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityAITypes.h"

void UBucketEditProxy::PopulateFrom(const FBucket &Bucket)
{
	BucketTag = Bucket.BucketTag;
	Priority = Bucket.Priority;
	CutoffThreshold = Bucket.CutoffThreshold;
}

void UBucketEditProxy::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	WriteBack();
}

void UBucketEditProxy::WriteBack()
{
	UUtilityBrainAsset *Asset = AssetPtr.Get();
	if (!Asset || !Asset->Buckets.IsValidIndex(BucketIndex))
	{
		return;
	}

	// The property editor has already opened a transaction — just mark as modified.
	Asset->Modify();

	FBucket &Bucket = Asset->Buckets[BucketIndex];
	Bucket.BucketTag = BucketTag;
	Bucket.Priority = Priority;
	Bucket.CutoffThreshold = CutoffThreshold;

	Asset->MarkPackageDirty();

	if (OnBucketModified)
	{
		OnBucketModified();
	}
}
