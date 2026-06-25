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

#include "ConsiderationCard.h"
#include "UtilityBrainEditorToolkit.h"
#include "Core/UtilityConsiderationBase.h"
#include "Curves/CurveFloat.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "Styling/StyleColors.h"

#define LOCTEXT_NAMESPACE "ConsiderationCard"

namespace
{
	// Strip common prefixes to get a friendly class display name
	FString GetFriendlyClassName(const UClass *Class)
	{
		if (!Class)
		{
			return TEXT("?");
		}
		FString Name = Class->GetName();
		Name.RemoveFromStart(TEXT("Consideration_"));
		Name.RemoveFromStart(TEXT("Utility"));
		// Insert spaces before uppercase letters for BP-style names
		FString Result;
		for (int32 i = 0; i < Name.Len(); ++i)
		{
			if (i > 0 && FChar::IsUpper(Name[i]) && !FChar::IsUpper(Name[i - 1]))
			{
				Result += ' ';
			}
			Result += Name[i];
		}
		return Result;
	}

	FText GetCurveLabel(const UUtilityConsiderationBase *C)
	{
		if (!C)
		{
			return FText::GetEmpty();
		}
		if (C->ResponseCurve)
		{
			return FText::FromString(C->ResponseCurve->GetName());
		}
		return LOCTEXT("Linear", "Linear");
	}
}

void SConsiderationCard::Construct(const FArguments &InArgs)
{
	ToolkitPtr = InArgs._Toolkit;
	ConsiderationPtr = InArgs._Consideration;

	const UUtilityConsiderationBase *C = ConsiderationPtr.Get();
	const FText ClassName = FText::FromString(GetFriendlyClassName(C ? C->GetClass() : nullptr));
	const FText RangeText = C
								? FText::Format(LOCTEXT("Range", "[{0}, {1}]"),
												FText::AsNumber(C->InputMin), FText::AsNumber(C->InputMax))
								: FText::GetEmpty();

	// Captured by value for live attribute reads
	TWeakObjectPtr<UUtilityConsiderationBase> WeakC = ConsiderationPtr;

	// Debug section — hidden when not in PIE
	TSharedPtr<STextBlock> RawText, NormText, ScoreText;
	TSharedPtr<SBox> BarContainer, BarFill;

	TSharedRef<SWidget> DebugSectionWidget =
		SNew(SVerticalBox)
			.Visibility(EVisibility::Collapsed) // shown by SetDebugInfo()

		+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0.f, 4.f, 0.f, 2.f)
				  [
					  // Score fill bar
					  SAssignNew(BarContainer, SBox)
						  .HeightOverride(6.f)
							  [SNew(SOverlay) + SOverlay::Slot()[SNew(SBorder).BorderImage(FAppStyle::GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.f))] + SOverlay::Slot().HAlign(HAlign_Left)[SAssignNew(BarFill, SBox).WidthOverride(0.f)[SNew(SBorder).BorderImage(FAppStyle::GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.1f, 0.8f, 0.2f, 1.f))]]]]

		+ SVerticalBox::Slot()
			  .AutoHeight()
				  [SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.f)[SAssignNew(RawText, STextBlock).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(FSlateColor::UseSubduedForeground())] + SHorizontalBox::Slot().FillWidth(1.f)[SAssignNew(NormText, STextBlock).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(FSlateColor::UseSubduedForeground())] + SHorizontalBox::Slot().FillWidth(1.f)[SAssignNew(ScoreText, STextBlock).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(FSlateColor(FLinearColor(0.1f, 0.9f, 0.2f)))]];

	DebugSection = DebugSectionWidget;
	DebugRawText = RawText;
	DebugNormText = NormText;
	DebugScoreText = ScoreText;
	DebugBarContainer = BarContainer;
	DebugBarFill = BarFill;

	ChildSlot
		[SNew(SButton)
			 .ButtonStyle(FAppStyle::Get(), "NoBorder")
			 .ContentPadding(0.f)
			 .OnClicked(this, &SConsiderationCard::OnCardClicked)
				 [SNew(SBorder)
					  .BorderImage(this, &SConsiderationCard::GetBorderBrush)
					  .Padding(FMargin(8.f, 6.f))
						  [SNew(SVerticalBox)

						   // Class name
						   + SVerticalBox::Slot()
								 .AutoHeight()
								 .VAlign(VAlign_Center)
									 [SNew(STextBlock)
										  .Text(ClassName)
										  .Font(FAppStyle::GetFontStyle("NormalFont"))]

						   + SVerticalBox::Slot()
								 .AutoHeight()
								 .VAlign(VAlign_Center)
									 [SNew(STextBlock)
										  .Text_Lambda([WeakC]()
													   { return GetCurveLabel(WeakC.Get()); })
										  .Font(FAppStyle::GetFontStyle("SmallFont"))
										  .ColorAndOpacity(FSlateColor::UseSubduedForeground())]

						   // Input range
						   + SVerticalBox::Slot()
								 .AutoHeight()
								 .VAlign(VAlign_Center)
									 [SNew(STextBlock)
										  .Text(RangeText)
										  .Font(FAppStyle::GetFontStyle("SmallFont"))
										  .ColorAndOpacity(FSlateColor::UseSubduedForeground())]

						   // Extra description — key names, expected values, etc.
						   + SVerticalBox::Slot()
								 .AutoHeight()
								 .VAlign(VAlign_Center)
									 [SNew(STextBlock)
										  .Text_Lambda([WeakC]() -> FText
													   {
											  const UUtilityConsiderationBase* C = WeakC.Get();
											  return C ? C->GetExtraDescription() : FText::GetEmpty(); })
										  .Font(FAppStyle::GetFontStyle("SmallFont"))
										  .ColorAndOpacity(FSlateColor::UseSubduedForeground())
										  .Visibility_Lambda([WeakC]() -> EVisibility
															 {
											  const UUtilityConsiderationBase* C = WeakC.Get();
											  return (C && !C->GetExtraDescription().IsEmpty()) ? EVisibility::Visible : EVisibility::Collapsed; })]

						   // Live debug section (shown during PIE)
						   + SVerticalBox::Slot()
								 .AutoHeight()
									 [DebugSectionWidget]]]];
}

void SConsiderationCard::ApplyDebugScore(const FConsiderationDebugInfo &Info)
{
	CurrentDebugInfo = Info;
	bHasDebugInfo = true;

	if (DebugSection.IsValid())
		DebugSection->SetVisibility(EVisibility::Visible);

	if (DebugRawText.IsValid())
		DebugRawText->SetText(FText::FromString(FString::Printf(TEXT("Raw %.3f"), Info.RawValue)));
	if (DebugNormText.IsValid())
		DebugNormText->SetText(FText::FromString(FString::Printf(TEXT("Norm %.3f"), Info.NormalizedScore)));
	if (DebugScoreText.IsValid())
		DebugScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score %.3f"), Info.FinalScore)));

	if (DebugBarFill.IsValid() && DebugBarContainer.IsValid())
	{
		const float TrackWidth = DebugBarContainer->GetCachedGeometry().GetLocalSize().X;
		DebugBarFill->SetWidthOverride(TrackWidth * FMath::Clamp(Info.FinalScore, 0.f, 1.f));
	}
}

void SConsiderationCard::ClearDebugScore()
{
	if (!bHasDebugInfo)
		return;
	bHasDebugInfo = false;

	if (DebugSection.IsValid())
		DebugSection->SetVisibility(EVisibility::Collapsed);
}

const FSlateBrush *SConsiderationCard::GetBorderBrush() const
{
	auto Toolkit = ToolkitPtr.Pin();
	const bool bSelected = Toolkit.IsValid() && Toolkit->GetSelectedConsideration() == ConsiderationPtr.Get();

	return bSelected
			   ? FAppStyle::GetBrush("DetailsView.CategoryTop")
			   : FAppStyle::GetBrush("Menu.Background");
}

FReply SConsiderationCard::OnCardClicked()
{
	auto Toolkit = ToolkitPtr.Pin();
	if (Toolkit.IsValid())
	{
		Toolkit->SetSelectedConsideration(ConsiderationPtr.Get());
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
