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

#include "UtilityBrainAssetFactory.h"
#include "Core/UtilityBrainAsset.h"

UUtilityBrainAssetFactory::UUtilityBrainAssetFactory()
{
	SupportedClass = UUtilityBrainAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject *UUtilityBrainAssetFactory::FactoryCreateNew(
	UClass *InClass, UObject *InParent,
	FName InName, EObjectFlags Flags,
	UObject *Context, FFeedbackContext *Warn)
{
	return NewObject<UUtilityBrainAsset>(InParent, InClass, InName, Flags);
}
