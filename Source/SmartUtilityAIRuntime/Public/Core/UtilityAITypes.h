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
#include "Logging/LogMacros.h"
#include "UtilityAITypes.generated.h"

/**
 * Defines the interrupt policy for a utility action.
 */
UENUM(BlueprintType)
enum class EInterruptPolicy : uint8
{
    NeverInterrupt,    // The action will never be interrupted once it starts, even if a better action becomes available.
    InterruptIfBetter, // The action will be interrupted if a better action becomes available, allows specifying a minimum utility difference to prevent interrupting for negligible improvements.
    AlwaysReevaluate   // The action will be continuously reevaluated and will be interrupted immediately if a better action becomes available.
};

UENUM(BlueprintType)
enum class EActionStatus : uint8
{
    Inactive,  // The task is not active and has not been started.
    Running,   // The task is currently active and running.
    Succeeded, // The task has completed successfully.
    Failed,    // The task has completed with a failure.
    Aborted    // The task was aborted before completion.
};

USTRUCT(BlueprintType)
struct SMARTUTILITYAIRUNTIME_API FBucket
{
    GENERATED_BODY()

    // Tag that identifies which actions belong to this bucket
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bucket")
    FGameplayTag BucketTag{};

    // Priority — higher values evaluated first
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bucket",
              meta = (ClampMin = "0"))
    int32 Priority = 0;

    // Minimum score for a bucket winner to block lower buckets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bucket", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CutoffThreshold = 0.3f;

    // Sort predicate for TArray::Sort
    bool operator<(const FBucket &Other) const
    {
        return Priority > Other.Priority; // Descending — highest first
    }
};