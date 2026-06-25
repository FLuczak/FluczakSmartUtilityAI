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
#include "Factories/Factory.h"
#include "UtilityBrainAssetFactory.generated.h"

UCLASS()
class UUtilityBrainAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UUtilityBrainAssetFactory();

	virtual UObject *FactoryCreateNew(
		UClass *InClass, UObject *InParent,
		FName InName, EObjectFlags Flags,
		UObject *Context, FFeedbackContext *Warn) override;
};
