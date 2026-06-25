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
class SConsiderationCard;
class SHorizontalBox;
class SScrollBox;

class SConsiderationPipelinePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConsiderationPipelinePanel) {}
	SLATE_ARGUMENT(TWeakPtr<FUtilityBrainEditorToolkit>, Toolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs);
	void RebuildCards();

	virtual void Tick(const FGeometry &AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnMouseButtonDown(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) override;

private:
	TWeakPtr<FUtilityBrainEditorToolkit> ToolkitPtr;
	TSharedPtr<SScrollBox> CardScroll;

	// Kept in sync with CardScroll children for direct score updates
	TArray<TSharedPtr<SConsiderationCard>> CardWidgets;

	int32 LastDebugEvalCount = -1;

	void UpdateLiveScores();

	TSharedRef<SWidget> MakeAddConsiderationMenu();
	void AddConsiderationOfClass(UClass *ConsiderationClass);
	FReply OnRemoveConsiderationClicked();
};
