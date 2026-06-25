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
#include "Modules/ModuleInterface.h"

// The IModuleInterface implementation. StartupModule registers the asset type
// actions, the asset factory, the asset category in the "Add New" menu, and
// the custom Slate style set. ShutdownModule unregisters everything. This is
// the entry point that makes the entire editor exist.
class FUtilityAIEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
