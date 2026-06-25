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

#include "UtilityAIEditorStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/AppStyle.h"

TSharedPtr<FSlateStyleSet> FUtilityAIEditorStyle::StyleInstance = nullptr;

void FUtilityAIEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUtilityAIEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

const ISlateStyle &FUtilityAIEditorStyle::Get()
{
	return *StyleInstance;
}

FName FUtilityAIEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UtilityAIEditorStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FUtilityAIEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	// No custom icons yet; panels use FAppStyle engine icons.
	return Style;
}
