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

#include "Core/SmartUtilityAIRuntime.h"

DEFINE_LOG_CATEGORY(LogUtilityAI);

#define LOCTEXT_NAMESPACE "FSmartUtilityAIRuntimeModule"

void FSmartUtilityAIRuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uproject file per-module
}

void FSmartUtilityAIRuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSmartUtilityAIRuntimeModule, SmartUtilityAIRuntime)