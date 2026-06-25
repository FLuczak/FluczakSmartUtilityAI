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
#include "Toolkits/AssetEditorToolkit.h"
#include "Misc/NotifyHook.h"
#include "UObject/GCObject.h"
#include "EditorUndoClient.h"
#include "Widgets/Input/SComboBox.h"

class UUtilityBrainAsset;
class UUtilityActionBase;
class UUtilityConsiderationBase;
class UBucketEditProxy;
class UUtilityBrainComponent;
class SActionListPanel;
class SConsiderationPipelinePanel;
class SUtilityCurveEditorPanel;
class SDebugScorePanel;
class IDetailsView;

class FUtilityBrainEditorToolkit
	: public FAssetEditorToolkit,
	  public FNotifyHook,
	  public FGCObject,
	  public FSelfRegisteringEditorUndoClient
{
public:
	virtual ~FUtilityBrainEditorToolkit();

	void InitEditor(EToolkitMode::Type Mode,
					const TSharedPtr<IToolkitHost> &Host,
					UUtilityBrainAsset *InBrainAsset);

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager> &InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager> &InTabManager) override;

	// FGCObject
	virtual void AddReferencedObjects(FReferenceCollector &Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FUtilityBrainEditorToolkit"); }

	UUtilityBrainAsset *GetBrainAsset() const { return BrainAsset; }
	UUtilityActionBase *GetSelectedAction() const { return SelectedAction; }
	UUtilityConsiderationBase *GetSelectedConsideration() const { return SelectedConsideration; }
	int32 GetSelectedBucketIndex() const { return SelectedBucketIndex; }

	void SetSelectedAction(UUtilityActionBase *Action);
	void SetSelectedConsideration(UUtilityConsiderationBase *Consideration);
	void SetSelectedBucket(int32 BucketIndex);

	// FNotifyHook — called by the details panel when a property changes
	virtual void NotifyPostChange(const FPropertyChangedEvent &PropertyChangedEvent, FProperty *PropertyThatChanged) override;

	// FSelfRegisteringEditorUndoClient
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

	// Clears all selection state and broadcasts delegates so the details panel
	// falls back to showing the root asset.
	void ClearSelection();

	FSimpleMulticastDelegate OnSelectedActionChanged;
	FSimpleMulticastDelegate OnSelectedConsiderationChanged;
	FSimpleMulticastDelegate OnSelectedBucketChanged;

	// Fired when the PIE state changes or the selected instance changes.
	// Panels subscribe to this to enable/disable live display.
	FSimpleMulticastDelegate OnPIEStateChanged;

	// Returns the currently selected live brain component in PIE, or nullptr.
	// Lazily populates the instance list on the first call after PIE starts.
	UUtilityBrainComponent *GetLiveComponent();

	bool bIsInPIE = false;

private:
	TObjectPtr<UUtilityBrainAsset> BrainAsset = nullptr;
	TObjectPtr<UUtilityActionBase> SelectedAction = nullptr;
	TObjectPtr<UUtilityConsiderationBase> SelectedConsideration = nullptr;
	int32 SelectedBucketIndex = -1;
	TObjectPtr<UBucketEditProxy> BucketProxy = nullptr;

	TSharedPtr<SActionListPanel> ActionListPanel;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SConsiderationPipelinePanel> PipelinePanel;
	TSharedPtr<SUtilityCurveEditorPanel> CurveEditorPanel;
	TSharedPtr<SDebugScorePanel> DebugPanel;

	TSharedRef<SDockTab> SpawnActionListTab(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab> SpawnDetailsTab(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab> SpawnConsiderationPipelineTab(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab> SpawnCurveEditorTab(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab> SpawnBlackboardTab(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab> SpawnDebugTab(const FSpawnTabArgs &Args);

	void ExtendToolbar();
	void FillToolbar(FToolBarBuilder &Builder);
	void Compile();

	// -----------------------------------------------------------------------
	// PIE instance tracking
	// -----------------------------------------------------------------------

	// Scans the PIE world for all UUtilityBrainComponent instances using BrainAsset
	// and rebuilds InstanceDisplayNames / AllLiveComponents. Enables debug capture
	// on the selected instance only.
	void RefreshLiveComponents();

	// Called when a new instance is chosen in the combo box.
	void OnInstanceSelected(TSharedPtr<FString> Item, ESelectInfo::Type SelectType);

	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);

	// Parallel arrays — AllLiveComponents[i] corresponds to InstanceDisplayNames[i].
	TArray<TWeakObjectPtr<UUtilityBrainComponent>> AllLiveComponents;
	TArray<TSharedPtr<FString>> InstanceDisplayNames;

	// Currently selected item; points into InstanceDisplayNames.
	TSharedPtr<FString> SelectedInstanceItem;

	// The combo widget in the toolbar.
	TSharedPtr<SComboBox<TSharedPtr<FString>>> InstanceCombo;

	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	static const FName ActionListTabId;
	static const FName DetailsTabId;
	static const FName PipelineTabId;
	static const FName CurveEditorTabId;
	static const FName BlackboardTabId;
	static const FName DebugTabId;
};
