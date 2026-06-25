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

#include "ActionListPanel.h"
#include "UtilityBrainEditorToolkit.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityActionBase.h"
#include "Core/UtilityBrainComponent.h"
#include "Core/UtilityBrainDebugComponent.h"
#include "Core/UtilityBrainDebugTypes.h"
#include "Core/UtilityAITypes.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "ScopedTransaction.h"
#include "UObject/UObjectIterator.h"
#include "Input/DragAndDrop.h"
#include "InputCoreTypes.h"
#include "SDropTarget.h"

#define LOCTEXT_NAMESPACE "ActionListPanel"

class FActionDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FActionDragDropOp, FDragDropOperation)

	TWeakObjectPtr<UUtilityActionBase> ActionPtr;

	static TSharedRef<FActionDragDropOp> New(UUtilityActionBase *Action)
	{
		TSharedRef<FActionDragDropOp> Op = MakeShareable(new FActionDragDropOp());
		Op->ActionPtr = Action;
		Op->Construct();
		return Op;
	}

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		FString Name = ActionPtr.IsValid() ? ActionPtr->GetClass()->GetName() : TEXT("Action");
		Name.RemoveFromStart(TEXT("Action_"));

		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Menu.Background"))
			.Padding(FMargin(6.f, 3.f))
				[SNew(STextBlock)
					 .Text(FText::FromString(Name))
					 .Font(FAppStyle::GetFontStyle("SmallFont"))];
	}
};

void SActionListPanel::Construct(const FArguments &InArgs)
{
	ToolkitPtr = InArgs._Toolkit;

	SetCanTick(true);

	SAssignNew(TreeView, STreeView<TSharedPtr<FActionTreeItem>>)
		.TreeItemsSource(&RootItems)
		.OnGetChildren(this, &SActionListPanel::GetItemChildren)
		.OnGenerateRow(this, &SActionListPanel::GenerateRow)
		.OnSelectionChanged(this, &SActionListPanel::OnSelectionChanged)
		.SelectionMode(ESelectionMode::Single);

	ChildSlot
		[SNew(SVerticalBox)

		 // Toolbar
		 + SVerticalBox::Slot()
			   .AutoHeight()
			   .Padding(2.f, 2.f, 2.f, 4.f)
				   [SNew(SHorizontalBox)

					// Add Action
					+ SHorizontalBox::Slot()
						  .AutoWidth()
							  [SNew(SComboButton)
								   .ButtonStyle(FAppStyle::Get(), "SimpleButton")
								   .ToolTipText(LOCTEXT("AddActionTip", "Add a new action to the brain"))
								   .OnGetMenuContent(this, &SActionListPanel::MakeAddActionMenu)
								   .HasDownArrow(true)
								   .ButtonContent()
									   [SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SImage).Image(FAppStyle::GetBrush("Icons.Plus"))] + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(3.f, 0.f, 0.f, 0.f)[SNew(STextBlock).Text(LOCTEXT("AddActionLabel", "Add Action"))]]]

					// Add Bucket
					+ SHorizontalBox::Slot()
						  .AutoWidth()
						  .Padding(4.f, 0.f, 0.f, 0.f)
							  [SNew(SButton)
								   .ButtonStyle(FAppStyle::Get(), "SimpleButton")
								   .ToolTipText(LOCTEXT("AddBucketTip", "Add a new priority bucket"))
								   .OnClicked(this, &SActionListPanel::OnAddBucketClicked)
									   [SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SImage).Image(FAppStyle::GetBrush("Icons.PlusCircle"))] + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(3.f, 0.f, 0.f, 0.f)[SNew(STextBlock).Text(LOCTEXT("AddBucketLabel", "Add Bucket"))]]]

					// Remove (action or bucket)
					+ SHorizontalBox::Slot()
						  .AutoWidth()
						  .Padding(4.f, 0.f, 0.f, 0.f)
							  [SNew(SButton)
								   .ButtonStyle(FAppStyle::Get(), "SimpleButton")
								   .ToolTipText(LOCTEXT("RemoveTip", "Remove selected action or bucket"))
								   .IsEnabled_Lambda([this]()
													 { return TreeView.IsValid() && TreeView->GetSelectedItems().Num() > 0; })
								   .OnClicked(this, &SActionListPanel::OnRemoveClicked)
									   [SNew(SImage).Image(FAppStyle::GetBrush("Icons.Delete"))]]

					// Spacer
					+ SHorizontalBox::Slot().FillWidth(1.f)

					// Move Up (actions and buckets)
					+ SHorizontalBox::Slot()
						  .AutoWidth()
						  .Padding(0.f, 0.f, 2.f, 0.f)
							  [SNew(SButton)
								   .ButtonStyle(FAppStyle::Get(), "SimpleButton")
								   .ToolTipText(LOCTEXT("UpTip", "Move selected item up"))
								   .IsEnabled_Lambda([this]()
													 { return TreeView.IsValid() && TreeView->GetSelectedItems().Num() > 0; })
								   .OnClicked(this, &SActionListPanel::OnMoveUpClicked)
									   [SNew(SImage).Image(FAppStyle::GetBrush("Icons.ChevronUp"))]]

					// Move Down (actions and buckets)
					+ SHorizontalBox::Slot()
						  .AutoWidth()
							  [SNew(SButton)
								   .ButtonStyle(FAppStyle::Get(), "SimpleButton")
								   .ToolTipText(LOCTEXT("DownTip", "Move selected item down"))
								   .IsEnabled_Lambda([this]()
													 { return TreeView.IsValid() && TreeView->GetSelectedItems().Num() > 0; })
								   .OnClicked(this, &SActionListPanel::OnMoveDownClicked)
									   [SNew(SImage).Image(FAppStyle::GetBrush("Icons.ChevronDown"))]]]

		 // Tree wrapped in a SDropTarget so empty-space drops unbucket the action.
		 // The background is set to transparent so only the narrow dashed edge lines
		 // are visible during a drag — avoiding the bright full-area fill.
		 + SVerticalBox::Slot()
			   .FillHeight(1.f)
				   [SNew(SDropTarget)
						.OnAllowDrop(this, &SActionListPanel::OnAllowDropOnBackground)
						.OnIsRecognized(this, &SActionListPanel::OnAllowDropOnBackground)
						.OnDropped(this, &SActionListPanel::OnDroppedOnBackground)
						.ValidColor(FSlateColor(FLinearColor(0.3f, 0.6f, 1.f, 0.45f)))
						.BackgroundImage(FAppStyle::GetBrush("NoBrush"))
							[TreeView.ToSharedRef()]]];

	RefreshTree();
}

FReply SActionListPanel::OnMouseButtonDown(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent)
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

	Toolkit->ClearSelection();

	if (TreeView.IsValid())
	{
		TreeView->ClearSelection();
	}

	return FReply::Handled();
}

void SActionListPanel::Tick(const FGeometry &AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit || !Toolkit->bIsInPIE)
	{
		if (LastEvalCount != -1)
		{
			LastEvalCount = -1;
			Invalidate(EInvalidateWidgetReason::Paint);
		}
		return;
	}

	UUtilityBrainComponent *Comp = Toolkit->GetLiveComponent();
	if (!IsValid(Comp))
		return;

	UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent();
	if (!DbgComp)
		return;

	const int32 NewCount = DbgComp->GetSnapshot().EvaluationCount;
	if (NewCount != LastEvalCount)
	{
		LastEvalCount = NewCount;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SActionListPanel::RefreshTree()
{
	RootItems.Empty();

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return;
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return;
	}

	const TArray<FBucket> &Buckets = Asset->GetBuckets();

	TArray<TSharedPtr<FActionTreeItem>> BucketItems;
	BucketItems.SetNum(Buckets.Num());

	RefreshBuckets(Buckets, BucketItems);
	RefreshActions(Asset, Buckets, BucketItems);

	if (TreeView.IsValid())
	{
		for (const TSharedPtr<FActionTreeItem> &Item : RootItems)
		{
			if (Item->bIsBucket)
			{
				TreeView->SetItemExpansion(Item, true);
			}
		}
		TreeView->RequestTreeRefresh();
	}
}

void SActionListPanel::RefreshActions(UUtilityBrainAsset *Asset, const TArray<FBucket> &Buckets, TArray<TSharedPtr<FActionTreeItem>> &BucketItems)
{
	for (UUtilityActionBase *Action : Asset->GetActions())
	{
		if (!Action)
		{
			continue;
		}

		TSharedPtr<FActionTreeItem> ActionItem = MakeShared<FActionTreeItem>();
		ActionItem->bIsBucket = false;
		ActionItem->BucketIndex = -1;
		ActionItem->Action = Action;

		FString ClassName = Action->GetClass()->GetName();
		ClassName.RemoveFromStart(TEXT("Action_"));
		ActionItem->Label = Action->ActionId.IsNone()
								? FText::FromString(ClassName)
								: FText::FromName(Action->ActionId);

		const FBucket *OwnerBucket = Asset->GetBucketForAction(Action);
		if (OwnerBucket)
		{
			const int32 Idx = static_cast<int32>(OwnerBucket - Buckets.GetData());
			if (BucketItems.IsValidIndex(Idx))
			{
				BucketItems[Idx]->Children.Add(ActionItem);
				continue;
			}
		}
		RootItems.Add(ActionItem);
	}
}

void SActionListPanel::RefreshBuckets(const TArray<FBucket> &Buckets, TArray<TSharedPtr<FActionTreeItem>> &BucketItems)
{
	for (int32 i = 0; i < Buckets.Num(); ++i)
	{
		const FBucket &B = Buckets[i];
		FString TagStr = B.BucketTag.ToString();
		int32 DotIdx;
		FString ShortName = TagStr.FindLastChar('.', DotIdx) ? TagStr.Mid(DotIdx + 1) : TagStr;
		if (ShortName.IsEmpty())
		{
			ShortName = TEXT("(untagged)");
		}

		const bool bTagValid = B.BucketTag.IsValid();

		TSharedPtr<FActionTreeItem> BucketItem = MakeShared<FActionTreeItem>();
		BucketItem->bIsBucket = true;
		BucketItem->BucketIndex = i;
		BucketItem->bHasValidTag = bTagValid;
		BucketItem->Label = bTagValid
								? FText::Format(LOCTEXT("BucketFmt", "{0}  [Priority {1}]"),
												FText::FromString(ShortName), FText::AsNumber(B.Priority))
								: FText::Format(LOCTEXT("BucketFmtNoTag", "[No Tag Assigned]  [Priority {0}]"),
												FText::AsNumber(B.Priority));

		BucketItems[i] = BucketItem;
		RootItems.Add(BucketItem);
	}
}

void SActionListPanel::GetItemChildren(
	TSharedPtr<FActionTreeItem> Item,
	TArray<TSharedPtr<FActionTreeItem>> &OutChildren)
{
	OutChildren = Item->Children;
}

TSharedRef<ITableRow> SActionListPanel::GenerateRow(
	TSharedPtr<FActionTreeItem> Item,
	const TSharedRef<STableViewBase> &OwnerTable)
{
	if (Item->bIsBucket)
	{
		const FLinearColor BucketColor = Item->bHasValidTag
											 ? FLinearColor(1.f, 1.f, 1.f, 1.f)			 // white — valid bucket
											 : FLinearColor(0.85f, 0.25f, 0.25f, 0.65f); // faint red — no tag

		return SNew(STableRow<TSharedPtr<FActionTreeItem>>, OwnerTable)
			.Style(FAppStyle::Get(), "TableView.Row")
			.Padding(FMargin(0.f))
			.OnCanAcceptDrop(this, &SActionListPanel::OnBucketCanAcceptDrop)
			.OnAcceptDrop(this, &SActionListPanel::OnBucketAcceptDrop)
				[SNew(SBorder)
					 .BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					 .BorderBackgroundColor(FLinearColor(0.025f, 0.025f, 0.025f))
					 .Padding(FMargin(0.f, 3.f))
						 [SNew(SHorizontalBox) + SHorizontalBox::Slot()
													 .FillWidth(1.f)
														 [SNew(STextBlock)
															  .Text(Item->Label)
															  .Font(FAppStyle::GetFontStyle("BoldFont"))
															  .ColorAndOpacity(BucketColor)]]];
	}

	UUtilityActionBase *Action = Item->Action.Get();
	FText ClassText = Action
						  ? FText::FromString(Action->GetClass()->GetName())
						  : LOCTEXT("Null", "(null)");

	TSharedPtr<FActionTreeItem> ItemCopy = Item;
	TWeakPtr<FUtilityBrainEditorToolkit> WeakToolkit = ToolkitPtr;
	TWeakObjectPtr<UUtilityActionBase> WeakAction = Action;

	// Returns the debug info index for this action in the snapshot, or INDEX_NONE.
	auto FindActionIndex = [WeakToolkit, WeakAction]() -> int32
	{
		auto Toolkit = WeakToolkit.Pin();
		if (!Toolkit || !Toolkit->bIsInPIE)
			return INDEX_NONE;
		UUtilityActionBase *A = WeakAction.Get();
		UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
		if (!A || !Asset)
			return INDEX_NONE;
		return Asset->GetActions().IndexOfByKey(A);
	};

	auto GetScore = [WeakToolkit, FindActionIndex]() -> FText
	{
		auto Toolkit = WeakToolkit.Pin();
		if (!Toolkit || !Toolkit->bIsInPIE)
			return FText::GetEmpty();
		UUtilityBrainComponent *Comp = Toolkit->GetLiveComponent();
		if (!IsValid(Comp))
			return FText::GetEmpty();
		UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent();
		if (!DbgComp)
			return FText::GetEmpty();
		const int32 Idx = FindActionIndex();
		if (Idx == INDEX_NONE)
			return FText::GetEmpty();
		const FBrainDebugSnapshot &Snap = DbgComp->GetSnapshot();
		if (!Snap.ActionInfos.IsValidIndex(Idx))
			return FText::GetEmpty();
		return FText::FromString(FString::Printf(TEXT("%.2f"), Snap.ActionInfos[Idx].FinalScore));
	};

	auto GetRowColor = [WeakToolkit, FindActionIndex]() -> FSlateColor
	{
		auto Toolkit = WeakToolkit.Pin();
		if (!Toolkit || !Toolkit->bIsInPIE)
			return FSlateColor::UseForeground();
		UUtilityBrainComponent *Comp = Toolkit->GetLiveComponent();
		if (!IsValid(Comp))
			return FSlateColor::UseForeground();
		UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent();
		if (!DbgComp)
			return FSlateColor::UseForeground();
		const int32 Idx = FindActionIndex();
		if (Idx == INDEX_NONE)
			return FSlateColor::UseForeground();
		const FBrainDebugSnapshot &Snap = DbgComp->GetSnapshot();
		if (!Snap.ActionInfos.IsValidIndex(Idx))
			return FSlateColor::UseForeground();
		const FActionDebugInfo &Info = Snap.ActionInfos[Idx];
		if (Info.Action.IsValid() && Info.Action == Snap.ActiveAction)
			return FSlateColor(FLinearColor(0.1f, 0.9f, 0.2f));
		return FSlateColor::UseForeground();
	};

	return SNew(STableRow<TSharedPtr<FActionTreeItem>>, OwnerTable)
		.Padding(FMargin(4.f, 2.f))
		.OnDragDetected_Lambda([ItemCopy, this](const FGeometry &Geom, const FPointerEvent &Event) -> FReply
							   { return OnActionDragDetected(Geom, Event, ItemCopy); })
			[SNew(SHorizontalBox)

			 // Action label — green when this is the active action
			 + SHorizontalBox::Slot()
				   .FillWidth(1.f)
				   .VAlign(VAlign_Center)
					   [SNew(STextBlock)
							.Text(Item->Label)
							.ColorAndOpacity_Lambda(GetRowColor)]

			 // Class name (subdued, always shown)
			 + SHorizontalBox::Slot()
				   .AutoWidth()
				   .VAlign(VAlign_Center)
				   .Padding(4.f, 0.f, 0.f, 0.f)
					   [SNew(STextBlock)
							.Text(ClassText)
							.Font(FAppStyle::GetFontStyle("SmallFont"))
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())]

			 // Utility score — only visible in PIE
			 + SHorizontalBox::Slot()
				   .AutoWidth()
				   .VAlign(VAlign_Center)
				   .Padding(8.f, 0.f, 0.f, 0.f)
					   [SNew(STextBlock)
							.Text_Lambda(GetScore)
							.Font(FAppStyle::GetFontStyle("SmallFont"))
							.ColorAndOpacity_Lambda(GetRowColor)]];
}

void SActionListPanel::OnSelectionChanged(
	TSharedPtr<FActionTreeItem> Item,
	ESelectInfo::Type SelectInfo)
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return;
	}

	if (!Item.IsValid())
	{
		return;
	}

	if (Item->bIsBucket)
	{
		Toolkit->SetSelectedBucket(Item->BucketIndex);
	}
	else
	{
		Toolkit->SetSelectedBucket(-1);
		Toolkit->SetSelectedAction(Item->Action.Get());
	}
}

TSharedRef<SWidget> SActionListPanel::MakeAddActionMenu()
{
	FMenuBuilder MenuBuilder(/*bShouldCloseWindowAfterMenuSelection=*/true, nullptr);

	TArray<UClass *> Classes;
	GetDerivedClasses(UUtilityActionBase::StaticClass(), Classes, /*bRecursive=*/true);
	Classes.RemoveAll([](UClass *C)
					  { return C->HasAnyClassFlags(CLASS_Abstract); });
	Classes.Sort([](const UClass &A, const UClass &B)
				 { return A.GetFName().LexicalLess(B.GetFName()); });

	for (UClass *Class : Classes)
	{
		FString DisplayName = Class->GetName();
		DisplayName.RemoveFromStart(TEXT("Action_"));

		MenuBuilder.AddMenuEntry(
			FText::FromString(DisplayName),
			FText::FromString(Class->GetPathName()),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SActionListPanel::AddActionOfClass, Class)));
	}

	return MenuBuilder.MakeWidget();
}

void SActionListPanel::AddActionOfClass(UClass *ActionClass)
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid() || !ActionClass)
	{
		return;
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("AddAction", "Add Utility Action"));
	Asset->Modify();

	UUtilityActionBase *NewAction = NewObject<UUtilityActionBase>(
		Asset, ActionClass, NAME_None, RF_Transactional);
	Asset->Actions.Add(NewAction);

	FPropertyChangedEvent Evt(nullptr);
	Asset->PostEditChangeProperty(Evt);

	RefreshTree();
	Toolkit->SetSelectedAction(NewAction);
}

FReply SActionListPanel::OnRemoveClicked()
{
	if (!TreeView.IsValid())
	{
		return FReply::Handled();
	}
	auto Selected = TreeView->GetSelectedItems();
	if (Selected.IsEmpty())
	{
		return FReply::Handled();
	}

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Handled();
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return FReply::Handled();
	}

	TSharedPtr<FActionTreeItem> Item = Selected[0];

	if (Item->bIsBucket)
	{
		const int32 Idx = Item->BucketIndex;
		if (!Asset->Buckets.IsValidIndex(Idx))
		{
			return FReply::Handled();
		}

		FScopedTransaction Transaction(LOCTEXT("RemoveBucket", "Remove Bucket"));
		Asset->Modify();

		// Remove the bucket's tag from all actions that belonged to it
		const FGameplayTag BucketTag = Asset->Buckets[Idx].BucketTag;
		if (BucketTag.IsValid())
		{
			for (UUtilityActionBase *Action : Asset->Actions)
			{
				if (Action && Action->Tags.HasTag(BucketTag))
				{
					Action->Modify();
					Action->Tags.RemoveTag(BucketTag);
				}
			}
		}

		Asset->Buckets.RemoveAt(Idx);
		Asset->MarkPackageDirty();

		Toolkit->SetSelectedBucket(-1);
		RefreshTree();
	}
	else
	{
		UUtilityActionBase *Action = Item->Action.Get();
		if (!Action)
		{
			return FReply::Handled();
		}

		FScopedTransaction Transaction(LOCTEXT("RemoveAction", "Remove Utility Action"));
		Asset->Modify();
		Asset->Actions.Remove(Action);

		FPropertyChangedEvent Evt(nullptr);
		Asset->PostEditChangeProperty(Evt);

		Toolkit->SetSelectedAction(nullptr);
		RefreshTree();
	}

	return FReply::Handled();
}

FReply SActionListPanel::OnMoveUpClicked()
{
	if (!TreeView.IsValid())
	{
		return FReply::Handled();
	}
	auto SelItems = TreeView->GetSelectedItems();
	if (SelItems.IsEmpty())
	{
		return FReply::Handled();
	}

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Handled();
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return FReply::Handled();
	}

	TSharedPtr<FActionTreeItem> Item = SelItems[0];

	if (Item->bIsBucket)
	{
		const int32 Idx = Item->BucketIndex;
		if (Idx <= 0)
		{
			return FReply::Handled();
		}

		FScopedTransaction Transaction(LOCTEXT("MoveBucketUp", "Move Bucket Up"));
		Asset->Modify();
		Asset->Buckets.Swap(Idx, Idx - 1);
		Asset->MarkPackageDirty();

		RefreshTree();

		const int32 NewIdx = Idx - 1;
		Toolkit->SetSelectedBucket(NewIdx);
		for (const TSharedPtr<FActionTreeItem> &Root : RootItems)
		{
			if (Root->bIsBucket && Root->BucketIndex == NewIdx)
			{
				TreeView->SetSelection(Root);
				break;
			}
		}
	}
	else
	{
		UUtilityActionBase *Selected = Toolkit->GetSelectedAction();
		if (!Selected)
		{
			return FReply::Handled();
		}

		const int32 Idx = Asset->Actions.Find(Selected);
		if (Idx <= 0)
		{
			return FReply::Handled();
		}

		FScopedTransaction Transaction(LOCTEXT("MoveUp", "Move Action Up"));
		Asset->Modify();
		Asset->Actions.Swap(Idx, Idx - 1);

		FPropertyChangedEvent Evt(nullptr);
		Asset->PostEditChangeProperty(Evt);

		RefreshTree();
	}

	return FReply::Handled();
}

FReply SActionListPanel::OnMoveDownClicked()
{
	if (!TreeView.IsValid())
	{
		return FReply::Handled();
	}
	auto SelItems = TreeView->GetSelectedItems();
	if (SelItems.IsEmpty())
	{
		return FReply::Handled();
	}

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Handled();
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return FReply::Handled();
	}

	TSharedPtr<FActionTreeItem> Item = SelItems[0];

	if (Item->bIsBucket)
	{
		const int32 Idx = Item->BucketIndex;
		if (Idx >= Asset->Buckets.Num() - 1)
		{
			return FReply::Handled();
		}

		FScopedTransaction Transaction(LOCTEXT("MoveBucketDown", "Move Bucket Down"));
		Asset->Modify();
		Asset->Buckets.Swap(Idx, Idx + 1);
		Asset->MarkPackageDirty();

		RefreshTree();

		const int32 NewIdx = Idx + 1;
		Toolkit->SetSelectedBucket(NewIdx);
		for (const TSharedPtr<FActionTreeItem> &Root : RootItems)
		{
			if (Root->bIsBucket && Root->BucketIndex == NewIdx)
			{
				TreeView->SetSelection(Root);
				break;
			}
		}
	}
	else
	{
		UUtilityActionBase *Selected = Toolkit->GetSelectedAction();
		if (!Selected)
		{
			return FReply::Handled();
		}

		const int32 Idx = Asset->Actions.Find(Selected);
		if (Idx == INDEX_NONE || Idx >= Asset->Actions.Num() - 1)
		{
			return FReply::Handled();
		}

		FScopedTransaction Transaction(LOCTEXT("MoveDown", "Move Action Down"));
		Asset->Modify();
		Asset->Actions.Swap(Idx, Idx + 1);

		FPropertyChangedEvent Evt(nullptr);
		Asset->PostEditChangeProperty(Evt);

		RefreshTree();
	}

	return FReply::Handled();
}

FReply SActionListPanel::OnAddBucketClicked()
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Handled();
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return FReply::Handled();
	}

	FScopedTransaction Transaction(LOCTEXT("AddBucket", "Add Bucket"));
	Asset->Modify();

	FBucket NewBucket;
	Asset->Buckets.Add(NewBucket);
	Asset->MarkPackageDirty();

	const int32 NewIdx = Asset->Buckets.Num() - 1;
	RefreshTree();

	// Select the new bucket in the tree and the details panel
	for (const TSharedPtr<FActionTreeItem> &RootItem : RootItems)
	{
		if (RootItem->bIsBucket && RootItem->BucketIndex == NewIdx)
		{
			TreeView->SetSelection(RootItem);
			break;
		}
	}

	Toolkit->SetSelectedBucket(NewIdx);
	return FReply::Handled();
}

FReply SActionListPanel::OnActionDragDetected(
	const FGeometry &Geom,
	const FPointerEvent &Event,
	TSharedPtr<FActionTreeItem> Item)
{
	if (!Item.IsValid() || Item->bIsBucket)
	{
		return FReply::Unhandled();
	}
	UUtilityActionBase *Action = Item->Action.Get();
	if (!Action)
	{
		return FReply::Unhandled();
	}

	return FReply::Handled().BeginDragDrop(FActionDragDropOp::New(Action));
}

namespace
{
	void RemoveAllBucketTags(UUtilityActionBase *Action, const UUtilityBrainAsset *Asset)
	{
		for (const FBucket &B : Asset->GetBuckets())
		{
			if (B.BucketTag.IsValid())
			{
				Action->Tags.RemoveTag(B.BucketTag);
			}
		}
	}
}

TOptional<EItemDropZone> SActionListPanel::OnBucketCanAcceptDrop(
	const FDragDropEvent &Event,
	EItemDropZone Zone,
	TSharedPtr<FActionTreeItem> BucketItem)
{
	if (!Event.GetOperationAs<FActionDragDropOp>().IsValid())
	{
		return TOptional<EItemDropZone>();
	}

	if (Zone == EItemDropZone::OntoItem && BucketItem.IsValid() && !BucketItem->bHasValidTag)
	{
		return TOptional<EItemDropZone>();
	}

	return Zone;
}

FReply SActionListPanel::OnBucketAcceptDrop(
	const FDragDropEvent &Event,
	EItemDropZone Zone,
	TSharedPtr<FActionTreeItem> BucketItem)
{
	auto DragOp = Event.GetOperationAs<FActionDragDropOp>();
	if (!DragOp.IsValid())
	{
		return FReply::Unhandled();
	}

	UUtilityActionBase *Action = DragOp->ActionPtr.Get();
	if (!Action)
	{
		return FReply::Unhandled();
	}

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Unhandled();
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return FReply::Unhandled();
	}

	if (Zone == EItemDropZone::OntoItem)
	{
		const int32 TargetBucketIdx = BucketItem->BucketIndex;
		if (!Asset->Buckets.IsValidIndex(TargetBucketIdx))
		{
			return FReply::Unhandled();
		}

		FScopedTransaction Transaction(LOCTEXT("MoveActionToBucket", "Move Action to Bucket"));
		Action->Modify();
		RemoveAllBucketTags(Action, Asset);

		const FGameplayTag &TargetTag = Asset->Buckets[TargetBucketIdx].BucketTag;
		if (TargetTag.IsValid())
		{
			Action->Tags.AddTag(TargetTag);
		}
	}
	else
	{
		FScopedTransaction Transaction(LOCTEXT("UnbucketAction", "Remove Action from Bucket"));
		Action->Modify();
		RemoveAllBucketTags(Action, Asset);
	}

	RefreshTree();
	return FReply::Handled();
}

bool SActionListPanel::OnAllowDropOnBackground(TSharedPtr<FDragDropOperation> Op)
{
	return Op.IsValid() && Op->IsOfType<FActionDragDropOp>();
}

FReply SActionListPanel::OnDroppedOnBackground(const FGeometry &Geom, const FDragDropEvent &Event)
{
	auto DragOp = Event.GetOperationAs<FActionDragDropOp>();
	if (!DragOp.IsValid())
	{
		return FReply::Unhandled();
	}

	UUtilityActionBase *Action = DragOp->ActionPtr.Get();
	if (!Action)
	{
		return FReply::Unhandled();
	}

	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Unhandled();
	}

	UUtilityBrainAsset *Asset = Toolkit->GetBrainAsset();
	if (!Asset)
	{
		return FReply::Unhandled();
	}

	FScopedTransaction Transaction(LOCTEXT("UnbucketActionBg", "Remove Action from Bucket"));
	Action->Modify();
	RemoveAllBucketTags(Action, Asset);

	RefreshTree();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
