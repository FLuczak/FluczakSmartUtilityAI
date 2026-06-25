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
#include "Core/UtilityBrainDebugTypes.h"

class FUtilityBrainEditorToolkit;
class UUtilityConsiderationBase;
class STextBlock;

class SConsiderationCard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConsiderationCard) {}
	SLATE_ARGUMENT(TWeakPtr<FUtilityBrainEditorToolkit>, Toolkit)
	SLATE_ARGUMENT(UUtilityConsiderationBase *, Consideration)
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs);

	// Called by SConsiderationPipelinePanel each tick to push live evaluation data.
	void ApplyDebugScore(const FConsiderationDebugInfo &Info);
	void ClearDebugScore();

private:
	TWeakPtr<FUtilityBrainEditorToolkit> ToolkitPtr;
	TWeakObjectPtr<UUtilityConsiderationBase> ConsiderationPtr;

	TSharedPtr<SWidget> DebugSection;
	TSharedPtr<STextBlock> DebugRawText;
	TSharedPtr<STextBlock> DebugNormText;
	TSharedPtr<STextBlock> DebugScoreText;
	TSharedPtr<SBox> DebugBarContainer; // full-width bar track
	TSharedPtr<SBox> DebugBarFill;		// resized each update

	bool bHasDebugInfo = false;
	FConsiderationDebugInfo CurrentDebugInfo;

	const FSlateBrush *GetBorderBrush() const;
	FReply OnCardClicked();
};
