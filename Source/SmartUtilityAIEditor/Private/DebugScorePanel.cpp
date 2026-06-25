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

#include "DebugScorePanel.h"
#include "UtilityBrainEditorToolkit.h"
#include "Core/UtilityBrainComponent.h"
#include "Core/UtilityBrainDebugComponent.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityActionBase.h"
#include "Core/UtilityBrainDebugTypes.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/AppStyle.h"
#include "Styling/StyleColors.h"

#define LOCTEXT_NAMESPACE "DebugScorePanel"

void SDebugScorePanel::Construct(const FArguments &InArgs)
{
	ToolkitPtr = InArgs._Toolkit;

	PlaceholderWidget = MakePlaceholder();

	SAssignNew(BarBox, SVerticalBox);
	LiveWidget = SNew(SScrollBox)
					 .Orientation(Orient_Vertical) +
				 SScrollBox::Slot()
					 [BarBox.ToSharedRef()];

	ChildSlot
		[SNew(SBorder)
			 .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			 .Padding(8.f)
				 [PlaceholderWidget.ToSharedRef()]];

	SetCanTick(true);
}

TSharedRef<SWidget> SDebugScorePanel::MakePlaceholder() const
{
	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
			[SNew(STextBlock)
				 .Text(LOCTEXT("Placeholder", "Debug Scores — enter PIE to see live scores"))
				 .ColorAndOpacity(FSlateColor::UseSubduedForeground())];
}

void SDebugScorePanel::ShowPlaceholder()
{
	if (bShowingLive)
	{
		bShowingLive = false;
		LastEvalCount = -1;
		ChildSlot
			[SNew(SBorder)
				 .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				 .Padding(8.f)
					 [PlaceholderWidget.ToSharedRef()]];
	}
}

void SDebugScorePanel::Tick(const FGeometry &AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit)
	{
		ShowPlaceholder();
		return;
	}

	UUtilityBrainComponent *Comp = Toolkit->GetLiveComponent();
	if (!IsValid(Comp))
	{
		ShowPlaceholder();
		return;
	}

	UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent();
	if (!DbgComp)
	{
		ShowPlaceholder();
		return;
	}

	const FBrainDebugSnapshot &Snapshot = DbgComp->GetSnapshot();

	if (!bShowingLive)
	{
		bShowingLive = true;
		ChildSlot
			[SNew(SBorder)
				 .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				 .Padding(8.f)
					 [LiveWidget.ToSharedRef()]];
	}

	if (Snapshot.EvaluationCount != LastEvalCount)
	{
		LastEvalCount = Snapshot.EvaluationCount;
		RebuildBars();
	}
}

void SDebugScorePanel::RebuildBars()
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit || !BarBox.IsValid())
		return;

	UUtilityBrainComponent *Comp = Toolkit->GetLiveComponent();
	if (!IsValid(Comp))
		return;

	UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent();
	if (!DbgComp)
		return;

	const FBrainDebugSnapshot &Snapshot = DbgComp->GetSnapshot();
	const UUtilityActionBase *ActiveAction = Snapshot.ActiveAction.Get();

	// Sort indices by final score descending
	TArray<int32> SortedIndices;
	for (int32 i = 0; i < Snapshot.ActionInfos.Num(); ++i)
		SortedIndices.Add(i);
	SortedIndices.Sort([&Snapshot](int32 A, int32 B)
					   { return Snapshot.ActionInfos[A].FinalScore > Snapshot.ActionInfos[B].FinalScore; });

	BarBox->ClearChildren();

	// Header row
	BarBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
			[SNew(STextBlock)
				 .Text(FText::Format(LOCTEXT("Header", "Evaluation #{0}  —  {1} actions"),
									 FText::AsNumber(Snapshot.EvaluationCount),
									 FText::AsNumber(Snapshot.ActionInfos.Num())))
				 .Font(FAppStyle::GetFontStyle("SmallFont"))
				 .ColorAndOpacity(FSlateColor::UseSubduedForeground())];

	constexpr float MaxBarWidth = 240.f;
	constexpr float BarHeight = 14.f;

	for (int32 Idx : SortedIndices)
	{
		const FActionDebugInfo &Info = Snapshot.ActionInfos[Idx];
		const bool bIsActive = Info.Action.IsValid() && Info.Action.Get() == ActiveAction;
		const bool bHasPenalty = Info.PenaltyMultiplierApplied < 0.99f;

		const FLinearColor BarColor = bIsActive			 ? FLinearColor(0.1f, 0.8f, 0.2f, 1.f)
									  : Info.bOnCooldown ? FLinearColor(0.4f, 0.4f, 0.4f, 1.f)
									  : bHasPenalty		 ? FLinearColor(0.9f, 0.5f, 0.1f, 1.f)
														 : FLinearColor(0.2f, 0.6f, 0.9f, 1.f);

		const FString NameLabel = Info.DisplayName.IsEmpty() ? TEXT("?") : Info.DisplayName;

		FString TagLabel;
		if (bIsActive)
			TagLabel = TEXT(" [ACTIVE]");
		else if (Info.bOnCooldown)
			TagLabel = TEXT(" [CD]");
		else if (bHasPenalty)
			TagLabel = FString::Printf(TEXT(" [x%.2f]"), Info.PenaltyMultiplierApplied);

		BarBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 1.f)
				[SNew(SHorizontalBox)

				 // Colored fill bar
				 + SHorizontalBox::Slot()
					   .AutoWidth()
					   .VAlign(VAlign_Center)
						   [SNew(SBox)
								.WidthOverride(MaxBarWidth)
								.HeightOverride(BarHeight)
									[SNew(SOverlay) + SOverlay::Slot()[SNew(SBorder).BorderImage(FAppStyle::GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.f))] + SOverlay::Slot().HAlign(HAlign_Left)[SNew(SBox).WidthOverride(MaxBarWidth * FMath::Clamp(Info.FinalScore, 0.f, 1.f))[SNew(SBorder).BorderImage(FAppStyle::GetBrush("WhiteBrush")).BorderBackgroundColor(BarColor)]]]]

				 // Score value
				 + SHorizontalBox::Slot()
					   .AutoWidth()
					   .VAlign(VAlign_Center)
					   .Padding(6.f, 0.f)
						   [SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("%.2f"), Info.FinalScore)))
								.Font(FAppStyle::GetFontStyle("SmallFont"))
								.MinDesiredWidth(36.f)]

				 // Action name + tag
				 + SHorizontalBox::Slot()
					   .FillWidth(1.f)
					   .VAlign(VAlign_Center)
						   [SNew(STextBlock)
								.Text(FText::FromString(NameLabel + TagLabel))
								.Font(FAppStyle::GetFontStyle("SmallFont"))
								.ColorAndOpacity(bIsActive
													 ? FSlateColor(FLinearColor(0.1f, 0.9f, 0.2f))
													 : FSlateColor::UseForeground())]];
	}
}

#undef LOCTEXT_NAMESPACE
