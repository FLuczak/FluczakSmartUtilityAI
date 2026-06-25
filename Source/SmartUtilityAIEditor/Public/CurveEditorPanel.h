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
#include "CurveEditorTypes.h"

class FCurveEditor;
class FUtilityBrainEditorToolkit;
class UUtilityConsiderationBase;
class UCurveFloat;

struct FCurvePresetDef
{
	FText Name;
	struct FKey
	{
		float Time = 0.f;
		float Value = 0.f;
		ERichCurveInterpMode Interp = RCIM_Linear;
		ERichCurveTangentMode TangentMode = RCTM_Auto;
		float ArriveTangent = 0.f;
		float LeaveTangent = 0.f;
	};
	TArray<FKey> Keys;
};

// Wrapper panel hosting FCurveEditor + preset dropdown.
// Renamed to avoid collision with the engine's SCurveEditorPanel.
class SUtilityCurveEditorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUtilityCurveEditorPanel) {}
	SLATE_ARGUMENT(TWeakPtr<FUtilityBrainEditorToolkit>, Toolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs);
	void BindConsideration(UUtilityConsiderationBase *Consideration);

private:
	TWeakPtr<FUtilityBrainEditorToolkit> ToolkitPtr;

	TSharedPtr<FCurveEditor> CurveEditor;
	TSharedPtr<SWidget> CurveEditorWidget; // SCurveEditorPanel from CurveEditor module

	FCurveModelID BoundCurveID;
	bool bHasBoundCurve = false;

	TArray<TSharedPtr<FCurvePresetDef>> Presets;
	TSharedPtr<SBox> ContentBox; // swapped between editor layout and no-curve message
	void UnbindCurve();
	void RebuildContent(UCurveFloat *Curve);

	TSharedRef<SWidget> MakePresetCombo(UCurveFloat *Curve);
	TSharedRef<SWidget> MakeNoCurveMessage();
	void ApplyPreset(TSharedPtr<FCurvePresetDef> Preset, UCurveFloat *TargetCurve);
	FReply OnCreateNewCurveClicked();

	static TArray<TSharedPtr<FCurvePresetDef>> BuildPresets();
};
