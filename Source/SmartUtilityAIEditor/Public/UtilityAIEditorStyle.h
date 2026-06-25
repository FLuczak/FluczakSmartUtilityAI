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
#include "Styling/ISlateStyle.h"

class FSlateStyleSet;

class FUtilityAIEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static const ISlateStyle &Get();
	static FName GetStyleSetName();

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;

	static TSharedRef<FSlateStyleSet> Create();
};
