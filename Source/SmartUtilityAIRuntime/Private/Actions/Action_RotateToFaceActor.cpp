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

#include "Actions/Action_RotateToFaceActor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/UtilityBrainComponent.h"
#include "GameFramework/Pawn.h"

AActor *UAction_RotateToFaceActor::ResolveTarget(const UUtilityBrainComponent &OwnerComp) const
{
	if (TargetActor)
	{
		return TargetActor;
	}

	if (!TargetActorBlackboardKey.IsNone())
	{
		if (const UBlackboardComponent *Blackboard = OwnerComp.GetBlackboardComponent())
		{
			return Cast<AActor>(Blackboard->GetValueAsObject(TargetActorBlackboardKey));
		}
	}

	return nullptr;
}

EActionStatus UAction_RotateToFaceActor::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	return OnActionTick(OwnerComp, ControlledPawn, 0.f, ActionMemory);
}

EActionStatus UAction_RotateToFaceActor::OnActionTick(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, float DeltaTime, uint8 *ActionMemory)
{
	AActor *Target = ResolveTarget(OwnerComp);
	if (!ControlledPawn || !Target)
	{
		return EActionStatus::Failed;
	}

	const FVector ToTarget = Target->GetActorLocation() - ControlledPawn->GetActorLocation();
	if (ToTarget.IsNearlyZero())
	{
		return EActionStatus::Succeeded;
	}

	const FRotator DesiredRot = ToTarget.Rotation();
	const FRotator CurrentRot = ControlledPawn->GetActorRotation();
	const float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw));

	if (YawDelta <= AcceptanceAngleDegrees)
	{
		return EActionStatus::Succeeded;
	}

	const float RotationSpeed = FMath::Max(0.f, RotationSpeedDegreesPerSecond);
	const FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, DesiredRot, DeltaTime, RotationSpeed);
	ControlledPawn->SetActorRotation(NewRot);

	return EActionStatus::Running;
}
