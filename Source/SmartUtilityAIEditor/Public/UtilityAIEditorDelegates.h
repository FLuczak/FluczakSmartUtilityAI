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
#include "Delegates/Delegate.h"

// Header-only. Declares delegate types shared across panels: FOnActionSelected,
// FOnConsiderationSelected, FOnBrainCompiled. Kept in one file so panels don't
// need to include each other's headers.

// Broadcast when an action is selected. Parameter: the selected action ID or pointer.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnActionSelected, UObject *);

// Broadcast when a consideration is selected. Parameter: the selected consideration.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnConsiderationSelected, UObject *);

// Broadcast when the brain has been compiled. No parameters.
DECLARE_MULTICAST_DELEGATE(FOnBrainCompiled);
