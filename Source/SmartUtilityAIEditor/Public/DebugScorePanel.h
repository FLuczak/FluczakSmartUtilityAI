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
#include "Widgets/SCompoundWidget.h"

class FUtilityBrainEditorToolkit;
class SVerticalBox;

class SDebugScorePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDebugScorePanel) {}
	SLATE_ARGUMENT(TWeakPtr<FUtilityBrainEditorToolkit>, Toolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs);

	virtual void Tick(const FGeometry &AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TWeakPtr<FUtilityBrainEditorToolkit> ToolkitPtr;

	TSharedPtr<SVerticalBox> BarBox;
	TSharedPtr<SWidget> PlaceholderWidget;
	TSharedPtr<SWidget> LiveWidget;

	int32 LastEvalCount = -1;
	bool bShowingLive = false;

	void ShowPlaceholder();
	void RebuildBars();

	TSharedRef<SWidget> MakePlaceholder() const;
};
