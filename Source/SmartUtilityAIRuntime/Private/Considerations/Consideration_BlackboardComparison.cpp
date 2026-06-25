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

#include "Considerations/Consideration_BlackboardComparison.h"
#include "Core/SmartUtilityAIRuntime.h"
#include "Core/UtilityBrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UConsideration_BlackboardComparison::UConsideration_BlackboardComparison()
{
	InputMin = 0.f;
	InputMax = 1.f;
}

#if WITH_EDITOR
FText UConsideration_BlackboardComparison::GetExtraDescription() const
{
	if (KeyA.SelectedKeyName.IsNone())
		return FText::GetEmpty();

	const TCHAR *Op = TEXT("?");
	switch (Operator)
	{
	case EBBComparisonOp::GreaterThan:
		Op = TEXT(">");
		break;
	case EBBComparisonOp::LessThan:
		Op = TEXT("<");
		break;
	case EBBComparisonOp::EqualTo:
		Op = TEXT("==");
		break;
	case EBBComparisonOp::NotEqualTo:
		Op = TEXT("!=");
		break;
	case EBBComparisonOp::GreaterThanOrEqualTo:
		Op = TEXT(">=");
		break;
	case EBBComparisonOp::LessThanOrEqualTo:
		Op = TEXT("<=");
		break;
	}

	const FText RHS = bCompareToBlackboardKey
						  ? FText::FromName(KeyB.SelectedKeyName)
						  : FText::AsNumber(ConstantB);

	return FText::Format(INVTEXT("{0} {1} {2}"),
						 FText::FromName(KeyA.SelectedKeyName),
						 FText::FromString(Op),
						 RHS);
}
#endif

float UConsideration_BlackboardComparison::ComputeRawValue(const UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn) const
{
	const UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr)
	{
		UE_LOG(LogUtilityAI, Warning, TEXT("Consideration_BlackboardComparison: OwnerComp has no BlackboardComponent"));
		return 0.f;
	}

	if (bCompareToBlackboardKey)
	{
		float ValueA = BlackboardComp->GetValueAsFloat(KeyA.SelectedKeyName);
		float ValueB = BlackboardComp->GetValueAsFloat(KeyB.SelectedKeyName);
		return Compare(ValueA, ValueB) ? 1.f : 0.f;
	}
	else
	{
		float ValueA = BlackboardComp->GetValueAsFloat(KeyA.SelectedKeyName);
		return Compare(ValueA, ConstantB) ? 1.f : 0.f;
	}
}

bool UConsideration_BlackboardComparison::Compare(float ValueA, float ValueB) const
{
	switch (Operator)
	{
	case EBBComparisonOp::GreaterThan:
		return ValueA > ValueB;
	case EBBComparisonOp::LessThan:
		return ValueA < ValueB;
	case EBBComparisonOp::EqualTo:
		return FMath::Abs(ValueA - ValueB) <= EqualityTolerance;
	case EBBComparisonOp::NotEqualTo:
		return FMath::Abs(ValueA - ValueB) > EqualityTolerance;
	case EBBComparisonOp::GreaterThanOrEqualTo:
		return ValueA >= ValueB;
	case EBBComparisonOp::LessThanOrEqualTo:
		return ValueA <= ValueB;
	default:
		return false;
	}
}
