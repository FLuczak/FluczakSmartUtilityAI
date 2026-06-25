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
#include "Widgets/Views/STreeView.h"

class FUtilityBrainEditorToolkit;
class UUtilityActionBase;

struct FActionTreeItem
{
	bool bIsBucket = false;
	int32 BucketIndex = -1;	  // valid (>= 0) when bIsBucket == true
	bool bHasValidTag = true; // false when bucket has no tag assigned
	FText Label;
	TWeakObjectPtr<UUtilityActionBase> Action; // null for bucket rows
	TArray<TSharedPtr<FActionTreeItem>> Children;
};

class SActionListPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SActionListPanel) {}
	SLATE_ARGUMENT(TWeakPtr<FUtilityBrainEditorToolkit>, Toolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs);
	void RefreshTree();

	virtual void Tick(const FGeometry &AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnMouseButtonDown(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) override;

	void RefreshActions(class UUtilityBrainAsset *Asset, const TArray<struct FBucket> &Buckets, TArray<TSharedPtr<FActionTreeItem>> &BucketItems);
	void RefreshBuckets(const TArray<struct FBucket> &Buckets, TArray<TSharedPtr<FActionTreeItem>> &BucketItems);

private:
	TWeakPtr<FUtilityBrainEditorToolkit> ToolkitPtr;
	TArray<TSharedPtr<FActionTreeItem>> RootItems;
	TSharedPtr<STreeView<TSharedPtr<FActionTreeItem>>> TreeView;

	int32 LastEvalCount = -1;

	TSharedRef<SWidget> MakeToolbar();
	TSharedRef<ITableRow> MakeBucketRow(TSharedPtr<FActionTreeItem> Item, const TSharedRef<STableViewBase> &OwnerTable);
	TSharedRef<ITableRow> MakeActionRow(TSharedPtr<FActionTreeItem> Item, const TSharedRef<STableViewBase> &OwnerTable);
	void SelectBucketAfterRefresh(int32 NewBucketIdx, TSharedPtr<FUtilityBrainEditorToolkit> Toolkit);

	void GetItemChildren(TSharedPtr<FActionTreeItem> Item, TArray<TSharedPtr<FActionTreeItem>> &OutChildren);
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FActionTreeItem> Item, const TSharedRef<STableViewBase> &OwnerTable);
	void OnSelectionChanged(TSharedPtr<FActionTreeItem> Item, ESelectInfo::Type SelectInfo);

	TSharedRef<SWidget> MakeAddActionMenu();
	void AddActionOfClass(UClass *ActionClass);
	FReply OnRemoveClicked();
	FReply OnMoveUpClicked();
	FReply OnMoveDownClicked();
	FReply OnAddBucketClicked();

	FReply OnActionDragDetected(const FGeometry &Geom, const FPointerEvent &Event, TSharedPtr<FActionTreeItem> Item);
	TOptional<EItemDropZone> OnBucketCanAcceptDrop(const FDragDropEvent &Event, EItemDropZone Zone, TSharedPtr<FActionTreeItem> BucketItem);
	FReply OnBucketAcceptDrop(const FDragDropEvent &Event, EItemDropZone Zone, TSharedPtr<FActionTreeItem> BucketItem);
	bool OnAllowDropOnBackground(TSharedPtr<FDragDropOperation> Op);
	FReply OnDroppedOnBackground(const FGeometry &Geom, const FDragDropEvent &Event);
};
