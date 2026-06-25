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

#include "ConsiderationPipelinePanel.h"
#include "ConsiderationCard.h"
#include "UtilityBrainEditorToolkit.h"
#include "Core/UtilityActionBase.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityBrainComponent.h"
#include "Core/UtilityBrainDebugComponent.h"
#include "Core/UtilityConsiderationBase.h"
#include "Core/UtilityBrainDebugTypes.h"
#include "Widgets/Layout/SScrollBox.h"
#include "InputCoreTypes.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Images/SImage.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "ScopedTransaction.h"
#include "UObject/UObjectIterator.h"

#define LOCTEXT_NAMESPACE "ConsiderationPipelinePanel"

void SConsiderationPipelinePanel::Construct(const FArguments &InArgs)
{
	ToolkitPtr = InArgs._Toolkit;

	SAssignNew(CardScroll, SScrollBox)
		.Orientation(Orient_Vertical);

	SetCanTick(true);

	ChildSlot
		[SNew(SBorder)
			 .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			 .Padding(4.f)
				 [SNew(SVerticalBox)

				  // Toolbar: label + add button
				  + SVerticalBox::Slot()
						.AutoHeight()
						.Padding(2.f, 0.f, 2.f, 4.f)
							[SNew(SHorizontalBox)

							 + SHorizontalBox::Slot()
								   .FillWidth(1.f)
								   .VAlign(VAlign_Center)
									   [SNew(STextBlock)
											.Text(LOCTEXT("Header", "Considerations"))
											.Font(FAppStyle::GetFontStyle("SmallFont"))
											.ColorAndOpacity(FSlateColor::UseSubduedForeground())]

							 + SHorizontalBox::Slot()
								   .AutoWidth()
									   [SNew(SComboButton)
											.ButtonStyle(FAppStyle::Get(), "SimpleButton")
											.ToolTipText(LOCTEXT("AddTip", "Add a new consideration to this action"))
											.OnGetMenuContent(this, &SConsiderationPipelinePanel::MakeAddConsiderationMenu)
											.HasDownArrow(true)
											.IsEnabled_Lambda([WeakToolkit = ToolkitPtr]()
															  {
						auto Toolkit = WeakToolkit.Pin();
						return Toolkit.IsValid() && Toolkit->GetSelectedAction() != nullptr; })
											.ButtonContent()
												[SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SImage).Image(FAppStyle::GetBrush("Icons.Plus"))] + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(3.f, 0.f, 0.f, 0.f)[SNew(STextBlock).Text(LOCTEXT("AddLabel", "Add")).Font(FAppStyle::GetFontStyle("SmallFont"))]]]

							 + SHorizontalBox::Slot()
								   .AutoWidth()
								   .Padding(2.f, 0.f, 0.f, 0.f)
									   [SNew(SButton)
											.ButtonStyle(FAppStyle::Get(), "SimpleButton")
											.ToolTipText(LOCTEXT("RemoveTip", "Remove the selected consideration"))
											.OnClicked(this, &SConsiderationPipelinePanel::OnRemoveConsiderationClicked)
											.IsEnabled_Lambda([WeakToolkit = ToolkitPtr]()
															  {
						auto Toolkit = WeakToolkit.Pin();
						return Toolkit.IsValid()
							&& Toolkit->GetSelectedAction() != nullptr
							&& Toolkit->GetSelectedConsideration() != nullptr; })
												[SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SImage).Image(FAppStyle::GetBrush("Icons.Delete"))] + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(3.f, 0.f, 0.f, 0.f)[SNew(STextBlock).Text(LOCTEXT("RemoveLabel", "Remove")).Font(FAppStyle::GetFontStyle("SmallFont"))]]]]

				  // Card strip (horizontal scroll)
				  + SVerticalBox::Slot()
						.FillHeight(1.f)
							[CardScroll.ToSharedRef()]]];

	// Subscribe to action selection changes so we rebuild when the user
	// clicks a different action in the list panel.
	if (auto Toolkit = ToolkitPtr.Pin())
	{
		TWeakPtr<SConsiderationPipelinePanel> WeakSelf = SharedThis(this);

		Toolkit->OnSelectedActionChanged.AddLambda([WeakSelf]()
												   {
			if (auto Pinned = WeakSelf.Pin())
			{
				Pinned->RebuildCards();
			} });

		// Also rebuild when a consideration is selected so the selected card
		// highlights itself via its border attribute.
		Toolkit->OnSelectedConsiderationChanged.AddLambda([WeakSelf]()
														  {
			if (auto Pinned = WeakSelf.Pin())
			{
				// Invalidate the paint so borders refresh (no full rebuild needed)
				Pinned->Invalidate(EInvalidateWidget::Paint);
			} });
	}

	RebuildCards();
}

FReply SConsiderationPipelinePanel::OnMouseButtonDown(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Unhandled();
	}

	Toolkit->SetSelectedConsideration(nullptr);
	return FReply::Handled();
}

void SConsiderationPipelinePanel::RebuildCards()
{
	if (!CardScroll.IsValid())
	{
		return;
	}
	CardScroll->ClearChildren();
	CardWidgets.Reset();
	LastDebugEvalCount = -1;

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return;
	}

	UUtilityActionBase *Action = Toolkit->GetSelectedAction();
	if (!Action)
	{
		CardScroll->AddSlot()
			[SNew(SBox)
				 .HAlign(HAlign_Center)
				 .VAlign(VAlign_Center)
				 .Padding(16.f)
					 [SNew(STextBlock)
						  .Text(LOCTEXT("NoAction", "Select an action to view its considerations"))
						  .ColorAndOpacity(FSlateColor::UseSubduedForeground())]];
		return;
	}

	const TArray<TObjectPtr<UUtilityConsiderationBase>> &Considerations =
		Action->GetConsiderations();

	if (Considerations.IsEmpty())
	{
		CardScroll->AddSlot()
			[SNew(SBox)
				 .HAlign(HAlign_Center)
				 .VAlign(VAlign_Center)
				 .Padding(16.f)
					 [SNew(STextBlock)
						  .Text(LOCTEXT("NoneYet", "No considerations — use Add to create one"))
						  .ColorAndOpacity(FSlateColor::UseSubduedForeground())]];
		return;
	}

	for (UUtilityConsiderationBase *Consideration : Considerations)
	{
		if (!Consideration)
		{
			CardWidgets.Add(nullptr);
			continue;
		}

		TSharedRef<SConsiderationCard> Card =
			SNew(SConsiderationCard)
				.Toolkit(ToolkitPtr)
				.Consideration(Consideration);

		CardWidgets.Add(Card);

		CardScroll->AddSlot()
			.Padding(4.f, 2.f)
				[Card];
	}
}

void SConsiderationPipelinePanel::Tick(const FGeometry &AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	UpdateLiveScores();
}

void SConsiderationPipelinePanel::UpdateLiveScores()
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit)
		return;

	UUtilityBrainComponent *Comp = Toolkit->GetLiveComponent();
	UUtilityBrainDebugComponent *DbgComp = IsValid(Comp) ? Comp->GetDebugComponent() : nullptr;

	if (!DbgComp)
	{
		// Not in PIE, no component, or debug component not yet created — clear overlays
		if (LastDebugEvalCount != -1)
		{
			LastDebugEvalCount = -1;
			for (TSharedPtr<SConsiderationCard> &Card : CardWidgets)
			{
				if (Card.IsValid())
					Card->ClearDebugScore();
			}
		}
		return;
	}

	const FBrainDebugSnapshot &Snapshot = DbgComp->GetSnapshot();
	if (Snapshot.EvaluationCount == LastDebugEvalCount)
		return;
	LastDebugEvalCount = Snapshot.EvaluationCount;

	// Find the asset index of the selected action (snapshot is parallel to asset actions)
	UUtilityActionBase *SelectedAction = Toolkit->GetSelectedAction();
	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!SelectedAction || !Asset)
		return;

	int32 ActionIndex = INDEX_NONE;
	const TArray<UUtilityActionBase *> &AssetActions = Asset->GetActions();
	for (int32 i = 0; i < AssetActions.Num(); ++i)
	{
		if (AssetActions[i] == SelectedAction)
		{
			ActionIndex = i;
			break;
		}
	}

	if (ActionIndex == INDEX_NONE || !Snapshot.ActionInfos.IsValidIndex(ActionIndex))
	{
		for (TSharedPtr<SConsiderationCard> &Card : CardWidgets)
		{
			if (Card.IsValid())
				Card->ClearDebugScore();
		}
		return;
	}

	const FActionDebugInfo &ActionInfo = Snapshot.ActionInfos[ActionIndex];

	for (int32 k = 0; k < CardWidgets.Num(); ++k)
	{
		if (!CardWidgets[k].IsValid())
			continue;

		if (ActionInfo.ConsiderationScores.IsValidIndex(k))
		{
			CardWidgets[k]->ApplyDebugScore(ActionInfo.ConsiderationScores[k]);
		}
		else
		{
			CardWidgets[k]->ClearDebugScore();
		}
	}
}

// ---------------------------------------------------------------------------
// Add consideration
// ---------------------------------------------------------------------------

TSharedRef<SWidget> SConsiderationPipelinePanel::MakeAddConsiderationMenu()
{
	FMenuBuilder MenuBuilder(/*bShouldCloseAfterSelection=*/true, nullptr);

	TArray<UClass *> Classes;
	GetDerivedClasses(UUtilityConsiderationBase::StaticClass(), Classes, /*bRecursive=*/true);
	Classes.RemoveAll([](UClass *C)
					  { return C->HasAnyClassFlags(CLASS_Abstract); });
	Classes.Sort([](const UClass &A, const UClass &B)
				 { return A.GetFName().LexicalLess(B.GetFName()); });

	for (UClass *Class : Classes)
	{
		FString DisplayName = Class->GetName();
		DisplayName.RemoveFromStart(TEXT("Consideration_"));

		MenuBuilder.AddMenuEntry(
			FText::FromString(DisplayName),
			FText::FromString(Class->GetPathName()),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(
				this, &SConsiderationPipelinePanel::AddConsiderationOfClass, Class)));
	}

	return MenuBuilder.MakeWidget();
}

void SConsiderationPipelinePanel::AddConsiderationOfClass(UClass *ConsiderationClass)
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid() || !ConsiderationClass)
	{
		return;
	}

	UUtilityActionBase *Action = Toolkit->GetSelectedAction();
	if (!Action)
	{
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("AddConsideration", "Add Consideration"));
	Action->Modify();

	UUtilityConsiderationBase *NewConsideration = NewObject<UUtilityConsiderationBase>(
		Action, ConsiderationClass, NAME_None, RF_Transactional);
	Action->AddConsideration(NewConsideration);

	RebuildCards();
	Toolkit->SetSelectedConsideration(NewConsideration);
}

FReply SConsiderationPipelinePanel::OnRemoveConsiderationClicked()
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Handled();
	}

	UUtilityActionBase *Action = Toolkit->GetSelectedAction();
	UUtilityConsiderationBase *Consideration = Toolkit->GetSelectedConsideration();
	if (!Action || !Consideration)
	{
		return FReply::Handled();
	}

	FScopedTransaction Transaction(LOCTEXT("RemoveConsideration", "Remove Consideration"));
	Action->Modify();
	Action->RemoveConsideration(Consideration);

	Toolkit->SetSelectedConsideration(nullptr);
	RebuildCards();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
