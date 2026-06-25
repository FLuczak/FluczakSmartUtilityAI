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
#include "UObject/NoExportTypes.h"
#include <type_traits>
#include "UtilityActionBase.generated.h"

UCLASS(Abstract)
class SMARTUTILITYAIRUNTIME_API UUtilityActionBase : public UObject
{
	GENERATED_BODY()

public:
	UUtilityActionBase();

	// Gameplay tags used to assign this action to priority buckets.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility Action")
	FGameplayTagContainer Tags;

	// Seconds to suppress this action after successful completion; 0 disables cooldown.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility Action")
	float Cooldown = 0.f;

	// Maximum seconds the action may run before being force-aborted; 0 disables timeout.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility Action")
	float Timeout = 0.f;

	// Multiplier applied to the consideration product before compensation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility Action")
	float BaseWeight = 1.f;

	// Unique identifier used to track cooldowns and execution times across evaluations.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility Action")
	FName ActionId;

	// Determines when this action may be interrupted by a higher-scoring candidate.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility Action")
	EInterruptPolicy InterruptPolicy;

	// Fraction above the active score a challenger must exceed to interrupt (InterruptIfBetter only).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility Action")
	float InterruptThreshold;

	/** size of instance memory */
	virtual uint16 GetInstanceMemorySize() const;

	// Placement-constructs instance memory for this action; override to use a typed struct.
	virtual void InitializeActionMemory(class UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const;
	// Destroys instance memory; override to call the destructor of a custom struct.
	virtual void CleanupActionMemory(class UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const;

	// Placement-constructs T in the action's memory block and returns a typed pointer.
	template <typename T>
	T *InitializeActionMemory(class UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const;

	// Calls ~T() if T is not trivially destructible.
	template <typename T>
	void CleanupActionMemory(class UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const;

	// Reinterprets the action memory pointer as T*, with a size-check assertion.
	template <typename T>
	T *CastInstanceActionMemory(uint8 *ActionMemory) const;

	// Called once when this action is selected; return Running to begin, or a terminal status to finish immediately.
	virtual EActionStatus OnActionEntered(class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn, uint8 *ActionMemory);
	// Called each frame while active; return a terminal status to signal completion.
	virtual EActionStatus OnActionTick(class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn, float DeltaTime, uint8 *ActionMemory);
	// Called on any terminal status to allow cleanup.
	virtual void OnActionExited(class UUtilityBrainComponent &OwnerComp, class APawn *ControlledPawn, EActionStatus Status, uint8 *ActionMemory);

	// Returns the considerations array used for scoring.
	const TArray<TObjectPtr<class UUtilityConsiderationBase>> &GetConsiderations() const { return Considerations; }

	// Returns true if the brain should duplicate this action into a per-component runtime instance.
	virtual bool ShouldCreateInstance() const { return false; }

#if WITH_EDITOR
	// Appends a consideration to this action's consideration list.
	void AddConsideration(class UUtilityConsiderationBase *Consideration);
	// Removes a consideration from this action's consideration list.
	void RemoveConsideration(class UUtilityConsiderationBase *Consideration);
#endif

protected:
	// Instanced consideration objects that compute scores contributing to this action's total.
	UPROPERTY(EditAnywhere, Instanced, Category = "Utility Action", meta = (ShowInnerProperties))
	TArray<TObjectPtr<class UUtilityConsiderationBase>> Considerations;
};

template <typename T>
T *UUtilityActionBase::InitializeActionMemory(UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const
{
	if (!ActionMemory)
	{
		return nullptr;
	}

	new (ActionMemory) T();
	return CastInstanceActionMemory<T>(ActionMemory);
}

template <typename T>
void UUtilityActionBase::CleanupActionMemory(UUtilityBrainComponent &OwnerComp, uint8 *ActionMemory) const
{
	if (!ActionMemory)
	{
		return;
	}

	if constexpr (!std::is_trivially_destructible_v<T>)
	{
		CastInstanceActionMemory<T>(ActionMemory)->~T();
	}
}

template <typename T>
T *UUtilityActionBase::CastInstanceActionMemory(uint8 *ActionMemory) const
{
	check(ActionMemory);
	checkf(sizeof(T) <= GetInstanceMemorySize(), TEXT("Requesting type of %zu bytes but GetInstanceMemorySize returns %u. Make sure GetInstanceMemorySize is implemented properly in %s class hierarchy."), sizeof(T), static_cast<uint32>(GetInstanceMemorySize()), *GetFName().ToString());
	return reinterpret_cast<T *>(ActionMemory);
}
