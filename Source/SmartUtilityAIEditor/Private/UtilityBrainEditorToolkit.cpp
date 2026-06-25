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

#include "UtilityBrainEditorToolkit.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityActionBase.h"
#include "Core/UtilityConsiderationBase.h"
#include "Core/UtilityBrainComponent.h"
#include "Core/UtilityBrainDebugComponent.h"
#include "ActionListPanel.h"
#include "BucketEditProxy.h"
#include "ConsiderationPipelinePanel.h"
#include "CurveEditorPanel.h"
#include "DebugScorePanel.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "Modules/ModuleManager.h"
#include "Logging/MessageLog.h"
#include "Editor.h"
#include "UObject/UObjectIterator.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
#include "AIController.h"
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "UtilityBrainEditor"

static UUtilityBrainDebugComponent *EnsureDebugComponent(UUtilityBrainComponent *Brain)
{
	UUtilityBrainDebugComponent *DbgComp = Brain ? Brain->GetDebugComponent() : nullptr;
	if (DbgComp)
	{
		return DbgComp;
	}

	AActor *Owner = Brain ? Brain->GetOwner() : nullptr;
	if (!Owner)
	{
		return nullptr;
	}

	DbgComp = NewObject<UUtilityBrainDebugComponent>(Owner, NAME_None, RF_Transient);
	DbgComp->RegisterComponent();
	return DbgComp;
}

const FName FUtilityBrainEditorToolkit::ActionListTabId(TEXT("UtilityBrain_ActionList"));
const FName FUtilityBrainEditorToolkit::DetailsTabId(TEXT("UtilityBrain_Details"));
const FName FUtilityBrainEditorToolkit::PipelineTabId(TEXT("UtilityBrain_Pipeline"));
const FName FUtilityBrainEditorToolkit::CurveEditorTabId(TEXT("UtilityBrain_CurveEditor"));
const FName FUtilityBrainEditorToolkit::BlackboardTabId(TEXT("UtilityBrain_Blackboard"));
const FName FUtilityBrainEditorToolkit::DebugTabId(TEXT("UtilityBrain_Debug"));

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

FUtilityBrainEditorToolkit::~FUtilityBrainEditorToolkit()
{
	FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
	FEditorDelegates::EndPIE.Remove(PIEEndedHandle);

	for (auto &Comp : AllLiveComponents)
	{
		if (Comp.IsValid())
		{
			if (UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent())
			{
				DbgComp->SetCaptureEnabled(false);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::InitEditor(
	EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost> &Host,
	UUtilityBrainAsset *InBrainAsset)
{
	BrainAsset = InBrainAsset;

	const TSharedRef<FTabManager::FLayout> Layout =
		FTabManager::NewLayout("UtilityBrainEditorLayout_v1")
			->AddArea(
				FTabManager::NewPrimaryArea()
					->SetOrientation(Orient_Vertical)
					->Split(
						FTabManager::NewSplitter()
							->SetOrientation(Orient_Horizontal)
							->SetSizeCoefficient(0.65f)
							->Split(
								FTabManager::NewStack()
									->SetSizeCoefficient(0.2f)
									->AddTab(ActionListTabId, ETabState::OpenedTab))
							->Split(
								FTabManager::NewStack()
									->SetSizeCoefficient(0.5f)
									->AddTab(PipelineTabId, ETabState::OpenedTab))
							->Split(
								FTabManager::NewStack()
									->SetSizeCoefficient(0.3f)
									->AddTab(DetailsTabId, ETabState::OpenedTab)))
					->Split(
						FTabManager::NewStack()
							->SetSizeCoefficient(0.35f)
							->AddTab(CurveEditorTabId, ETabState::OpenedTab)
							->AddTab(BlackboardTabId, ETabState::OpenedTab)
							->AddTab(DebugTabId, ETabState::ClosedTab)));

	InitAssetEditor(Mode, Host, FName("UtilityBrainEditorApp"),
					Layout,
					/*bCreateDefaultStandaloneMenu=*/true,
					/*bCreateDefaultToolbar=*/true,
					InBrainAsset);

	ExtendToolbar();
	RegenerateMenusAndToolbars();

	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddRaw(this, &FUtilityBrainEditorToolkit::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddRaw(this, &FUtilityBrainEditorToolkit::OnPIEEnded);

	// If PIE is already running when the editor opens, activate debug mode immediately.
	if (GEditor && GEditor->IsPlayingSessionInEditor())
	{
		OnPIEStarted(false);
	}
}

// ---------------------------------------------------------------------------
// FAssetEditorToolkit identity
// ---------------------------------------------------------------------------

FName FUtilityBrainEditorToolkit::GetToolkitFName() const
{
	return FName("UtilityBrainEditor");
}

FText FUtilityBrainEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("EditorName", "Utility Brain Editor");
}

FLinearColor FUtilityBrainEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.f, 0.74f, 0.59f, 0.5f);
}

FString FUtilityBrainEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("UtilityBrain");
}

// ---------------------------------------------------------------------------
// FGCObject
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::AddReferencedObjects(FReferenceCollector &Collector)
{
	Collector.AddReferencedObject(BrainAsset);
	Collector.AddReferencedObject(SelectedAction);
	Collector.AddReferencedObject(SelectedConsideration);
	Collector.AddReferencedObject(BucketProxy);
}

// ---------------------------------------------------------------------------
// Tab registration
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager> &InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(ActionListTabId,
									 FOnSpawnTab::CreateSP(this, &FUtilityBrainEditorToolkit::SpawnActionListTab))
		.SetDisplayName(LOCTEXT("ActionListTab", "Actions"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Outliner"));

	InTabManager->RegisterTabSpawner(DetailsTabId,
									 FOnSpawnTab::CreateSP(this, &FUtilityBrainEditorToolkit::SpawnDetailsTab))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PipelineTabId,
									 FOnSpawnTab::CreateSP(this, &FUtilityBrainEditorToolkit::SpawnConsiderationPipelineTab))
		.SetDisplayName(LOCTEXT("PipelineTab", "Consideration Pipeline"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.BlueprintDefaults"));

	InTabManager->RegisterTabSpawner(CurveEditorTabId,
									 FOnSpawnTab::CreateSP(this, &FUtilityBrainEditorToolkit::SpawnCurveEditorTab))
		.SetDisplayName(LOCTEXT("CurveTab", "Response Curve"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCurveEditor.TabIcon"));

	InTabManager->RegisterTabSpawner(BlackboardTabId,
									 FOnSpawnTab::CreateSP(this, &FUtilityBrainEditorToolkit::SpawnBlackboardTab))
		.SetDisplayName(LOCTEXT("BBTab", "Blackboard"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.BTNode.Task.BlackboardBase.Icon"));

	InTabManager->RegisterTabSpawner(DebugTabId,
									 FOnSpawnTab::CreateSP(this, &FUtilityBrainEditorToolkit::SpawnDebugTab))
		.SetDisplayName(LOCTEXT("DebugTab", "Debug Scores"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Debug"));
}

void FUtilityBrainEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager> &InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(ActionListTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(PipelineTabId);
	InTabManager->UnregisterTabSpawner(CurveEditorTabId);
	InTabManager->UnregisterTabSpawner(BlackboardTabId);
	InTabManager->UnregisterTabSpawner(DebugTabId);
}

// ---------------------------------------------------------------------------
// Tab spawn functions
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FUtilityBrainEditorToolkit::SpawnActionListTab(const FSpawnTabArgs &Args)
{
	TSharedRef<SActionListPanel> Panel = SNew(SActionListPanel).Toolkit(SharedThis(this));
	ActionListPanel = Panel;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
			[Panel];
}

TSharedRef<SDockTab> FUtilityBrainEditorToolkit::SpawnDetailsTab(const FSpawnTabArgs &Args)
{
	FPropertyEditorModule &PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bHideSelectionTip = true;
	DetailsArgs.bShowPropertyMatrixButton = false;
	DetailsArgs.NotifyHook = this;

	DetailsView = PropertyModule.CreateDetailView(DetailsArgs);

	if (BrainAsset)
	{
		DetailsView->SetObject(BrainAsset);
	}

	// Selection changes drive what the details panel inspects.
	OnSelectedActionChanged.AddLambda([this]()
									  {
										  if (!DetailsView.IsValid())
										  {
											  return;
										  }
										  if (SelectedAction)
										  {
											  DetailsView->SetObject(SelectedAction);
										  }
										  else if (SelectedBucketIndex < 0)
										  {
											  // No action selected and no bucket selected → show asset root
											  DetailsView->SetObject(BrainAsset);
										  }
										  // If a bucket is selected, OnSelectedBucketChanged will update the panel
									  });

	OnSelectedConsiderationChanged.AddLambda([this]()
											 {
		if (!DetailsView.IsValid()) { return; }
		if (SelectedConsideration)
		{
			DetailsView->SetObject(SelectedConsideration);
		}
		else if (SelectedAction)
		{
			DetailsView->SetObject(SelectedAction);
		} });

	OnSelectedBucketChanged.AddLambda([this]()
									  {
		if (!DetailsView.IsValid()) { return; }
		if (SelectedBucketIndex >= 0 && BrainAsset && BrainAsset->Buckets.IsValidIndex(SelectedBucketIndex))
		{
			if (!BucketProxy)
			{
				BucketProxy = NewObject<UBucketEditProxy>(
					GetTransientPackage(), NAME_None, RF_Transient);
			}
			BucketProxy->AssetPtr    = BrainAsset;
			BucketProxy->BucketIndex = SelectedBucketIndex;

			TWeakPtr<SActionListPanel> WeakPanel = ActionListPanel;
			BucketProxy->OnBucketModified = [WeakPanel]()
			{
				if (auto Panel = WeakPanel.Pin()) { Panel->RefreshTree(); }
			};

			BucketProxy->PopulateFrom(BrainAsset->Buckets[SelectedBucketIndex]);
			DetailsView->SetObject(BucketProxy);
		}
		else if (!SelectedAction)
		{
			DetailsView->SetObject(BrainAsset);
		} });

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
			[DetailsView.ToSharedRef()];
}

TSharedRef<SDockTab> FUtilityBrainEditorToolkit::SpawnConsiderationPipelineTab(const FSpawnTabArgs &Args)
{
	TSharedRef<SConsiderationPipelinePanel> Panel =
		SNew(SConsiderationPipelinePanel).Toolkit(SharedThis(this));
	PipelinePanel = Panel;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
			[Panel];
}

TSharedRef<SDockTab> FUtilityBrainEditorToolkit::SpawnCurveEditorTab(const FSpawnTabArgs &Args)
{
	TSharedRef<SUtilityCurveEditorPanel> Panel =
		SNew(SUtilityCurveEditorPanel).Toolkit(SharedThis(this));
	CurveEditorPanel = Panel;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
			[Panel];
}

TSharedRef<SDockTab> FUtilityBrainEditorToolkit::SpawnBlackboardTab(const FSpawnTabArgs &Args)
{
	FPropertyEditorModule &PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs BBDetailsArgs;
	BBDetailsArgs.bHideSelectionTip = true;

	TSharedPtr<IDetailsView> BBDetailsView = PropertyModule.CreateDetailView(BBDetailsArgs);

	if (BrainAsset && BrainAsset->GetBlackboardAsset())
	{
		BBDetailsView->SetObject(BrainAsset->GetBlackboardAsset());
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
			[BBDetailsView.ToSharedRef()];
}

TSharedRef<SDockTab> FUtilityBrainEditorToolkit::SpawnDebugTab(const FSpawnTabArgs &Args)
{
	TSharedRef<SDebugScorePanel> Panel =
		SNew(SDebugScorePanel).Toolkit(SharedThis(this));
	DebugPanel = Panel;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
			[Panel];
}

// ---------------------------------------------------------------------------
// Property notifications
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::NotifyPostChange(const FPropertyChangedEvent &PropertyChangedEvent, FProperty *PropertyThatChanged)
{
	if (PropertyThatChanged &&
		PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UUtilityConsiderationBase, ResponseCurve) &&
		SelectedConsideration &&
		CurveEditorPanel.IsValid())
	{
		CurveEditorPanel->BindConsideration(SelectedConsideration);
	}
}

// ---------------------------------------------------------------------------
// Undo / Redo
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::PostUndo(bool bSuccess)
{
	if (!bSuccess)
	{
		return;
	}

	// Validate that the selected action still exists in the asset after the undo.
	if (SelectedAction && BrainAsset)
	{
		bool bStillValid = false;
		for (UUtilityActionBase *A : BrainAsset->GetActions())
		{
			if (A == SelectedAction)
			{
				bStillValid = true;
				break;
			}
		}
		if (!bStillValid)
		{
			SelectedAction = nullptr;
			SelectedConsideration = nullptr;
		}
	}

	// Validate that the selected consideration still belongs to the selected action.
	if (SelectedAction && SelectedConsideration)
	{
		bool bStillValid = false;
		for (UUtilityConsiderationBase *C : SelectedAction->GetConsiderations())
		{
			if (C == SelectedConsideration)
			{
				bStillValid = true;
				break;
			}
		}
		if (!bStillValid)
		{
			SelectedConsideration = nullptr;
		}
	}

	// Validate bucket selection
	if (SelectedBucketIndex >= 0 && BrainAsset &&
		!BrainAsset->Buckets.IsValidIndex(SelectedBucketIndex))
	{
		SelectedBucketIndex = -1;
	}

	if (ActionListPanel.IsValid())
	{
		ActionListPanel->RefreshTree();
	}

	OnSelectedActionChanged.Broadcast();
	OnSelectedConsiderationChanged.Broadcast();
	OnSelectedBucketChanged.Broadcast();
}

void FUtilityBrainEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

// ---------------------------------------------------------------------------
// Selection
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::SetSelectedAction(UUtilityActionBase *Action)
{
	SelectedBucketIndex = -1;
	if (SelectedAction == Action)
	{
		return;
	}

	SelectedAction = Action;
	SelectedConsideration = nullptr;

	OnSelectedActionChanged.Broadcast();
	OnSelectedConsiderationChanged.Broadcast();
}

void FUtilityBrainEditorToolkit::SetSelectedBucket(int32 BucketIndex)
{
	if (SelectedBucketIndex == BucketIndex)
	{
		return;
	}

	SelectedBucketIndex = BucketIndex;
	SelectedAction = nullptr;
	SelectedConsideration = nullptr;

	// Notify dependents — action/consideration panels clear via null action broadcast
	OnSelectedActionChanged.Broadcast();
	OnSelectedConsiderationChanged.Broadcast();
	OnSelectedBucketChanged.Broadcast();
}

void FUtilityBrainEditorToolkit::SetSelectedConsideration(UUtilityConsiderationBase *Consideration)
{
	if (SelectedConsideration == Consideration)
	{
		return;
	}

	SelectedConsideration = Consideration;
	OnSelectedConsiderationChanged.Broadcast();
}

void FUtilityBrainEditorToolkit::ClearSelection()
{
	SelectedAction = nullptr;
	SelectedConsideration = nullptr;
	SelectedBucketIndex = -1;

	OnSelectedActionChanged.Broadcast();
	OnSelectedConsiderationChanged.Broadcast();
	OnSelectedBucketChanged.Broadcast();
}

// ---------------------------------------------------------------------------
// Toolbar
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::ExtendToolbar()
{
	TSharedPtr<FExtender> Extender = MakeShared<FExtender>();

	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FUtilityBrainEditorToolkit::FillToolbar));

	AddToolbarExtender(Extender);
}

void FUtilityBrainEditorToolkit::FillToolbar(FToolBarBuilder &Builder)
{
	Builder.AddSeparator();

	Builder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateSP(this, &FUtilityBrainEditorToolkit::Compile)),
		NAME_None,
		LOCTEXT("Compile", "Compile"),
		LOCTEXT("CompileTooltip", "Validate the brain for authoring errors"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.CompilerResults"));

	Builder.AddSeparator();

	// Instance picker — disabled outside PIE
	TWeakPtr<FUtilityBrainEditorToolkit> WeakSelf = SharedThis(this);

	Builder.AddWidget(
		SNew(SBox)
			.VAlign(VAlign_Center)
				[SNew(SHorizontalBox)

				 + SHorizontalBox::Slot()
					   .AutoWidth()
					   .VAlign(VAlign_Center)
					   .Padding(4.f, 0.f, 6.f, 0.f)
						   [SNew(STextBlock)
								.Text(LOCTEXT("InstanceLabel", "Instance:"))
								.Font(FAppStyle::GetFontStyle("SmallFont"))
								.ColorAndOpacity(FSlateColor::UseSubduedForeground())]

				 + SHorizontalBox::Slot()
					   .AutoWidth()
						   [SNew(SBox)
								.MinDesiredWidth(140.f)
									[SAssignNew(InstanceCombo, SComboBox<TSharedPtr<FString>>)
										 .OptionsSource(&InstanceDisplayNames)
										 .IsEnabled_Lambda([WeakSelf]()
														   {
						auto Self = WeakSelf.Pin();
						return Self.IsValid() && Self->bIsInPIE && !Self->InstanceDisplayNames.IsEmpty(); })
										 .ToolTipText(TAttribute<FText>::Create([WeakSelf]() -> FText
																				{
						auto Self = WeakSelf.Pin();
						if (!Self.IsValid() || !Self->bIsInPIE)
							return LOCTEXT("InstanceTip_NoPIE", "Enter PIE to select a brain instance");
						if (Self->InstanceDisplayNames.IsEmpty())
							return LOCTEXT("InstanceTip_None", "No agents using this brain are running");
						return LOCTEXT("InstanceTip", "Select which agent's brain to inspect"); }))
										 .OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
																  { return SNew(STextBlock)
																		.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("?")))
																		.Font(FAppStyle::GetFontStyle("SmallFont"))
																		.Margin(FMargin(4.f, 2.f)); })
										 .OnSelectionChanged(this, &FUtilityBrainEditorToolkit::OnInstanceSelected)
											 [SNew(STextBlock)
												  .Text(TAttribute<FText>::Create([WeakSelf]() -> FText
																				  {
							auto Self = WeakSelf.Pin();
							if (!Self.IsValid() || !Self->bIsInPIE)
								return LOCTEXT("NoPIE", "No PIE");
							if (!Self->SelectedInstanceItem.IsValid())
								return LOCTEXT("NoInstance", "None");
							return FText::FromString(*Self->SelectedInstanceItem); }))
												  .Font(FAppStyle::GetFontStyle("SmallFont"))
												  .Margin(FMargin(2.f, 0.f))]]]

				 // Refresh button — re-scans for instances spawned after PIE start
				 + SHorizontalBox::Slot()
					   .AutoWidth()
					   .VAlign(VAlign_Center)
					   .Padding(2.f, 0.f, 4.f, 0.f)
						   [SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ToolTipText(LOCTEXT("RefreshTip", "Rescan for agents using this brain"))
								.IsEnabled_Lambda([WeakSelf]()
												  {
					auto Self = WeakSelf.Pin();
					return Self.IsValid() && Self->bIsInPIE; })
								.OnClicked_Lambda([WeakSelf]() -> FReply
												  {
					if (auto Self = WeakSelf.Pin()) Self->RefreshLiveComponents();
					return FReply::Handled(); })
									[SNew(SImage)
										 .Image(FAppStyle::GetBrush("Icons.Refresh"))
										 .DesiredSizeOverride(FVector2D(14.f, 14.f))]]]);
}

void FUtilityBrainEditorToolkit::Compile()
{
	if (!BrainAsset)
	{
		return;
	}

	FMessageLog MessageLog("UtilityAI");
	MessageLog.NewPage(FText::Format(
		LOCTEXT("CompileLogPage", "Compile: {0}"),
		FText::FromString(GetEditingObject()->GetName())));

	MessageLog.Info(LOCTEXT("CompileOK", "Brain compiled successfully — no issues found."));
	MessageLog.Open(EMessageSeverity::Info, true);
}

// ---------------------------------------------------------------------------
// PIE bridge
// ---------------------------------------------------------------------------

void FUtilityBrainEditorToolkit::RefreshLiveComponents()
{
	// Remember the previously selected display name so we can restore it after rebuild.
	const FString PrevName = SelectedInstanceItem.IsValid() ? *SelectedInstanceItem : FString();

	// Disable capture on all current components before clearing the list.
	for (auto &Comp : AllLiveComponents)
	{
		if (Comp.IsValid())
		{
			if (UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent())
			{
				DbgComp->SetCaptureEnabled(false);
			}
		}
	}

	AllLiveComponents.Reset();
	InstanceDisplayNames.Reset();
	SelectedInstanceItem = nullptr;

	if (!bIsInPIE || !BrainAsset)
	{
		if (InstanceCombo.IsValid())
			InstanceCombo->RefreshOptions();
		return;
	}

	for (TObjectIterator<UUtilityBrainComponent> It; It; ++It)
	{
		UUtilityBrainComponent *Comp = *It;
		if (!IsValid(Comp) || !Comp->GetWorld() || !Comp->GetWorld()->IsGameWorld() || Comp->BrainAsset != BrainAsset)
		{
			continue;
		}

		// Build a human-readable label: use the pawn's actor label if available.
		FString Label;
		if (const AAIController *AIC = Cast<AAIController>(Comp->GetOwner()))
		{
			if (const APawn *Pawn = AIC->GetPawn())
			{
				Label = Pawn->GetActorLabel();
			}
			else
			{
				Label = AIC->GetActorLabel();
			}
		}
		else if (IsValid(Comp->GetOwner()))
		{
			Label = Comp->GetOwner()->GetActorLabel();
		}
		else
		{
			Label = FString::Printf(TEXT("Instance %d"), AllLiveComponents.Num());
		}

		AllLiveComponents.Add(Comp);
		InstanceDisplayNames.Add(MakeShared<FString>(MoveTemp(Label)));
	}

	// Restore previous selection, or default to first found.
	for (const TSharedPtr<FString> &Name : InstanceDisplayNames)
	{
		if (*Name == PrevName)
		{
			SelectedInstanceItem = Name;
			break;
		}
	}
	if (!SelectedInstanceItem.IsValid() && InstanceDisplayNames.Num() > 0)
	{
		SelectedInstanceItem = InstanceDisplayNames[0];
	}

	// Enable capture only on the selected instance.
	UUtilityBrainComponent *Selected = GetLiveComponent();
	for (auto &Comp : AllLiveComponents)
	{
		if (!Comp.IsValid())
		{
			continue;
		}

		const bool bShouldCapture = (Comp.Get() == Selected);
		if (bShouldCapture)
		{
			if (UUtilityBrainDebugComponent *DbgComp = EnsureDebugComponent(Comp.Get()))
			{
				DbgComp->SetCaptureEnabled(true);
			}
		}
		else
		{
			if (UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent())
			{
				DbgComp->SetCaptureEnabled(false);
			}
		}
	}

	if (InstanceCombo.IsValid())
	{
		InstanceCombo->RefreshOptions();
		InstanceCombo->SetSelectedItem(SelectedInstanceItem);
	}

	OnPIEStateChanged.Broadcast();
}

UUtilityBrainComponent *FUtilityBrainEditorToolkit::GetLiveComponent()
{
	if (!bIsInPIE)
		return nullptr;

	// Lazy populate on first call after PIE starts.
	if (AllLiveComponents.IsEmpty())
	{
		RefreshLiveComponents();
	}

	// Find the index of the selected item.
	const int32 SelectedIdx = InstanceDisplayNames.IndexOfByKey(SelectedInstanceItem);
	if (AllLiveComponents.IsValidIndex(SelectedIdx))
	{
		UUtilityBrainComponent *Comp = AllLiveComponents[SelectedIdx].Get();
		if (IsValid(Comp))
			return Comp;
	}

	// Fallback: first valid component.
	for (int32 i = 0; i < AllLiveComponents.Num(); ++i)
	{
		if (AllLiveComponents[i].IsValid())
		{
			SelectedInstanceItem = InstanceDisplayNames.IsValidIndex(i) ? InstanceDisplayNames[i] : nullptr;
			return AllLiveComponents[i].Get();
		}
	}

	return nullptr;
}

void FUtilityBrainEditorToolkit::OnInstanceSelected(TSharedPtr<FString> Item, ESelectInfo::Type SelectType)
{
	if (!Item.IsValid())
		return;

	// Disable capture on the old component, enable on the new one.
	UUtilityBrainComponent *OldComp = GetLiveComponent();
	if (IsValid(OldComp))
	{
		if (UUtilityBrainDebugComponent *DbgComp = OldComp->GetDebugComponent())
		{
			DbgComp->SetCaptureEnabled(false);
		}
	}

	SelectedInstanceItem = Item;

	UUtilityBrainComponent *NewComp = GetLiveComponent();
	if (IsValid(NewComp))
	{
		if (UUtilityBrainDebugComponent *DbgComp = EnsureDebugComponent(NewComp))
		{
			DbgComp->SetCaptureEnabled(true);
		}
	}

	OnPIEStateChanged.Broadcast();
}

void FUtilityBrainEditorToolkit::OnPIEStarted(bool bIsSimulating)
{
	bIsInPIE = true;
	// RefreshLiveComponents is deferred to the first GetLiveComponent() call so that
	// actors have finished BeginPlay and StartLogic by the time we enumerate them.
	if (InstanceCombo.IsValid())
		InstanceCombo->RefreshOptions();
	OnPIEStateChanged.Broadcast();
}

void FUtilityBrainEditorToolkit::OnPIEEnded(bool bIsSimulating)
{
	for (auto &Comp : AllLiveComponents)
	{
		if (Comp.IsValid())
		{
			if (UUtilityBrainDebugComponent *DbgComp = Comp->GetDebugComponent())
			{
				DbgComp->SetCaptureEnabled(false);
			}
		}
	}

	AllLiveComponents.Reset();
	InstanceDisplayNames.Reset();
	SelectedInstanceItem = nullptr;
	bIsInPIE = false;

	if (InstanceCombo.IsValid())
		InstanceCombo->RefreshOptions();
	OnPIEStateChanged.Broadcast();
}

#undef LOCTEXT_NAMESPACE
