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

#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityActionBase.h"

void UUtilityBrainAsset::EnsureUniqueActionIds()
{
    TSet<FName> UsedIds;
    int32 Suffix = 0;

    for (UUtilityActionBase* Action : Actions)
    {
        if (Action == nullptr)
        {
            continue;
        }

        if (!Action->ActionId.IsNone() && !UsedIds.Contains(Action->ActionId))
        {
            UsedIds.Add(Action->ActionId);
            continue;
        }

        const FString BaseName = Action->GetClass() ? Action->GetClass()->GetName() : TEXT("Action");
        FName Candidate;
        do
        {
            Candidate = FName(*FString::Printf(TEXT("%s_%d"), *BaseName, Suffix++));
        } while (UsedIds.Contains(Candidate));

        Action->ActionId = Candidate;
        UsedIds.Add(Candidate);
    }
}

UUtilityBrainAsset::UUtilityBrainAsset()
{
}

const FBucket* UUtilityBrainAsset::GetBucketForAction(const UUtilityActionBase* Action) const
{
    return Buckets.FindByPredicate([Action](const FBucket& Bucket)
    {
        return Action->Tags.HasTag(Bucket.BucketTag);
	});
}

void UUtilityBrainAsset::PostLoad()
{
  Super::PostLoad();
    EnsureUniqueActionIds();
    SortBuckets();
}

#if WITH_EDITOR
void UUtilityBrainAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    EnsureUniqueActionIds();
    SortBuckets();
}
#endif

void UUtilityBrainAsset::SortBuckets()
{
    Buckets.Sort();
}
