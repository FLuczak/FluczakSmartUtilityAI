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

// TCommands<> subclass. Registers keyboard shortcuts: Ctrl+S (save), F7
// (compile), Delete (remove selected), Ctrl+D (duplicate selected), Ctrl+Z/Y
// (undo/redo bindings). Maps each command to a FUIAction that the toolkit
// dispatches.
class FUtilityAIEditorCommands /* : public TCommands<FUtilityAIEditorCommands> */
{
public:
    // Intentionally left empty; command registration belongs in implementation.
};
