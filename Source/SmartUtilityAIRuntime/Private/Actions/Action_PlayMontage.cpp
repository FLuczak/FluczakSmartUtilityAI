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

#include "Actions/Action_PlayMontage.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/UtilityBrainComponent.h"
#include "GameFramework/Pawn.h"

UAnimInstance *UAction_PlayMontage::ResolveAnimInstance(const APawn *Pawn) const
{
	USkeletalMeshComponent *MeshComp = Pawn ? Pawn->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	return MeshComp ? MeshComp->GetAnimInstance() : nullptr;
}

EActionStatus UAction_PlayMontage::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	UAnimInstance *AnimInstance = ResolveAnimInstance(ControlledPawn);
	if (!AnimInstance || !Montage)
	{
		return EActionStatus::Failed;
	}

	const float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
	return Duration > 0.f ? EActionStatus::Running : EActionStatus::Failed;
}

EActionStatus UAction_PlayMontage::OnActionTick(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, float DeltaTime, uint8 *ActionMemory)
{
	UAnimInstance *AnimInstance = ResolveAnimInstance(ControlledPawn);
	if (!AnimInstance || !Montage)
	{
		return EActionStatus::Failed;
	}

	return AnimInstance->Montage_IsPlaying(Montage) ? EActionStatus::Running : EActionStatus::Succeeded;
}

void UAction_PlayMontage::OnActionExited(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, EActionStatus Status, uint8 *ActionMemory)
{
	if (!bStopOnExit)
	{
		return;
	}

	if (UAnimInstance *AnimInstance = ResolveAnimInstance(ControlledPawn))
	{
		if (Montage && AnimInstance->Montage_IsPlaying(Montage))
		{
			AnimInstance->Montage_Stop(0.2f, Montage);
		}
	}
}
