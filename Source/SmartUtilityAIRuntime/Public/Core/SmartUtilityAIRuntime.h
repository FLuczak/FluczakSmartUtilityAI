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
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUtilityAI, Log, All);

class FSmartUtilityAIRuntimeModule : public IModuleInterface
{
public:
	// Called when the module is loaded; registers any required systems.
	virtual void StartupModule() override;
	// Called when the module is unloaded; performs cleanup.
	virtual void ShutdownModule() override;
};