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

#include "Actions/Action_SetBlackboardValue.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Core/UtilityBrainComponent.h"

EActionStatus UAction_SetBlackboardValue::OnActionEntered(UUtilityBrainComponent &OwnerComp, APawn *ControlledPawn, uint8 *ActionMemory)
{
	UBlackboardComponent *Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard || KeyName.SelectedKeyName.IsNone())
	{
		return EActionStatus::Failed;
	}

	switch (ValueType)
	{
	case EUtilityBlackboardValueType::Bool:
		Blackboard->SetValueAsBool(KeyName.SelectedKeyName, BoolValue);
		break;
	case EUtilityBlackboardValueType::Int:
		Blackboard->SetValueAsInt(KeyName.SelectedKeyName, IntValue);
		break;
	case EUtilityBlackboardValueType::Float:
		Blackboard->SetValueAsFloat(KeyName.SelectedKeyName, FloatValue);
		break;
	case EUtilityBlackboardValueType::Vector:
		Blackboard->SetValueAsVector(KeyName.SelectedKeyName, VectorValue);
		break;
	case EUtilityBlackboardValueType::Object:
		Blackboard->SetValueAsObject(KeyName.SelectedKeyName, ObjectValue);
		break;
	default:
		return EActionStatus::Failed;
	}

	return EActionStatus::Succeeded;
}
