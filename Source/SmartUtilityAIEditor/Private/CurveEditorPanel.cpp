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

#include "CurveEditorPanel.h"
#include "UtilityBrainEditorToolkit.h"
#include "Core/UtilityConsiderationBase.h"
#include "Curves/CurveFloat.h"
#include "Curves/RichCurve.h"

// CurveEditor module
#include "CurveEditor.h"
#include "ICurveEditorModule.h"
#include "RichCurveEditorModel.h"
#include "SCurveEditorPanel.h" // engine widget — class named SCurveEditorPanel

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Styling/AppStyle.h"
#include "ScopedTransaction.h"
#include "Framework/Application/SlateApplication.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "CurveEditorPanel"

TArray<TSharedPtr<FCurvePresetDef>> SUtilityCurveEditorPanel::BuildPresets()
{
	using FK = FCurvePresetDef::FKey;

	TArray<TSharedPtr<FCurvePresetDef>> Out;

	auto Add = [&](FText Name, TArray<FK> Keys)
	{
		auto P = MakeShared<FCurvePresetDef>();
		P->Name = Name;
		P->Keys = MoveTemp(Keys);
		Out.Add(P);
	};

	Add(LOCTEXT("Linear", "Linear"),
		{
			FK{0.f, 0.f, RCIM_Linear, RCTM_Auto},
			FK{1.f, 1.f, RCIM_Linear, RCTM_Auto},
		});

	Add(LOCTEXT("EaseIn", "Ease In"),
		{
			FK{0.f, 0.f, RCIM_Cubic, RCTM_User, /*arrive*/ 0.f, /*leave*/ 0.f},
			FK{1.f, 1.f, RCIM_Cubic, RCTM_User, /*arrive*/ 2.f, /*leave*/ 2.f},
		});

	Add(LOCTEXT("EaseOut", "Ease Out"),
		{
			FK{0.f, 0.f, RCIM_Cubic, RCTM_User, /*arrive*/ 0.f, /*leave*/ 2.f},
			FK{1.f, 1.f, RCIM_Cubic, RCTM_User, /*arrive*/ 0.f, /*leave*/ 0.f},
		});

	Add(LOCTEXT("SCurve", "S-Curve"),
		{
			FK{0.f, 0.f, RCIM_Cubic, RCTM_Auto},
			FK{0.5f, 0.5f, RCIM_Cubic, RCTM_Auto},
			FK{1.f, 1.f, RCIM_Cubic, RCTM_Auto},
		});

	Add(LOCTEXT("Inverse", "Inverse Linear"),
		{
			FK{0.f, 1.f, RCIM_Linear, RCTM_Auto},
			FK{1.f, 0.f, RCIM_Linear, RCTM_Auto},
		});

	Add(LOCTEXT("Bell", "Bell"),
		{
			FK{0.f, 0.f, RCIM_Cubic, RCTM_Auto},
			FK{0.5f, 1.f, RCIM_Cubic, RCTM_Auto},
			FK{1.f, 0.f, RCIM_Cubic, RCTM_Auto},
		});

	Add(LOCTEXT("ClampHigh", "Clamp High"),
		{
			FK{0.f, 0.f, RCIM_Cubic, RCTM_Auto},
			FK{0.5f, 1.f, RCIM_Constant, RCTM_Auto},
		});

	return Out;
}

void SUtilityCurveEditorPanel::Construct(const FArguments &InArgs)
{
	ToolkitPtr = InArgs._Toolkit;
	Presets = BuildPresets();
	CurveEditor = MakeShared<FCurveEditor>();
	CurveEditor->InitCurveEditor(FCurveEditorInitParams{});
	CurveEditor->GridLineLabelFormatXAttribute = FText::FromString(TEXT("{0}"));

	CurveEditorWidget = SNew(SCurveEditorPanel, CurveEditor.ToSharedRef())
							.MinimumViewPanelHeight(200.f);

	SAssignNew(ContentBox, SBox);

	ChildSlot
		[SNew(SBorder)
			 .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			 .Padding(0.f)
				 [ContentBox.ToSharedRef()]];

	RebuildContent(nullptr);

	if (auto Toolkit = ToolkitPtr.Pin())
	{
		TWeakPtr<SUtilityCurveEditorPanel> WeakSelf = SharedThis(this);
		Toolkit->OnSelectedConsiderationChanged.AddLambda([WeakSelf]()
														  {
			auto Pinned = WeakSelf.Pin();
			if (!Pinned.IsValid()) { return; }
			auto InnerToolkit = Pinned->ToolkitPtr.Pin();
			if (InnerToolkit.IsValid())
			{
				Pinned->BindConsideration(InnerToolkit->GetSelectedConsideration());
			} });
	}
}

void SUtilityCurveEditorPanel::BindConsideration(UUtilityConsiderationBase *Consideration)
{
	UnbindCurve();
	UCurveFloat *Curve = Consideration ? Consideration->ResponseCurve.Get() : nullptr;
	RebuildContent(Curve);

	if (Curve)
	{
		auto Model = MakeUnique<FRichCurveEditorModelRaw>(&Curve->FloatCurve, Curve);
		Model->SetShortDisplayName(FText::FromString(Curve->GetName()));
		BoundCurveID = CurveEditor->AddCurve(MoveTemp(Model));
		bHasBoundCurve = true;
		CurveEditor->PinCurve(BoundCurveID);
		CurveEditor->ZoomToFitAll();
	}
}

void SUtilityCurveEditorPanel::UnbindCurve()
{
	if (bHasBoundCurve)
	{
		CurveEditor->UnpinCurve(BoundCurveID);
		CurveEditor->RemoveCurve(BoundCurveID);
		bHasBoundCurve = false;
	}
}

void SUtilityCurveEditorPanel::RebuildContent(UCurveFloat *Curve)
{
	if (!ContentBox.IsValid())
	{
		return;
	}

	if (Curve)
	{
		ContentBox->SetContent(
			SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight().Padding(4.f)[MakePresetCombo(Curve)] + SVerticalBox::Slot().FillHeight(1.f)[CurveEditorWidget.ToSharedRef()]);
	}
	else
	{
		ContentBox->SetContent(MakeNoCurveMessage());
	}
}

TSharedRef<SWidget> SUtilityCurveEditorPanel::MakeNoCurveMessage()
{
	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
			[SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[SNew(STextBlock).Text(LOCTEXT("NoCurve", "No response curve assigned")).ColorAndOpacity(FSlateColor::UseSubduedForeground())] + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(LOCTEXT("NoCurveHint", "Select a consideration, then assign a UCurveFloat\n"
																																																																																						   "to its Response Curve property in the Details panel."))
																																																																												  .Font(FAppStyle::GetFontStyle("SmallFont"))
																																																																												  .ColorAndOpacity(FSlateColor::UseSubduedForeground())
																																																																												  .Justification(ETextJustify::Center)] +
			 SVerticalBox::Slot()
				 .AutoHeight()
				 .HAlign(HAlign_Center)
				 .Padding(0.f, 12.f, 0.f, 0.f)
					 [SNew(SButton)
						  .Text(LOCTEXT("CreateCurveBtn", "Create New Curve"))
						  .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
						  .IsEnabled_Lambda([WeakToolkit = ToolkitPtr]()
											{
					auto T = WeakToolkit.Pin();
					return T.IsValid() && T->GetSelectedConsideration() != nullptr; })
						  .OnClicked(this, &SUtilityCurveEditorPanel::OnCreateNewCurveClicked)]];
}

TSharedRef<SWidget> SUtilityCurveEditorPanel::MakePresetCombo(UCurveFloat *Curve)
{
	TWeakObjectPtr<UCurveFloat> WeakCurve(Curve);
	TWeakPtr<SUtilityCurveEditorPanel> WeakSelf = SharedThis(this);

	return SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)[SNew(STextBlock).Text(LOCTEXT("PresetLabel", "Apply Preset:")).Font(FAppStyle::GetFontStyle("SmallFont"))] + SHorizontalBox::Slot().AutoWidth()[SNew(SComboBox<TSharedPtr<FCurvePresetDef>>).OptionsSource(&Presets).OnGenerateWidget_Lambda([](TSharedPtr<FCurvePresetDef> Item) -> TSharedRef<SWidget>
																																																																																									{ return SNew(STextBlock)
																																																																																										  .Text(Item.IsValid() ? Item->Name : FText::GetEmpty())
																																																																																										  .Margin(FMargin(4.f, 2.f)); })
																																																																		   .OnSelectionChanged_Lambda([WeakSelf, WeakCurve](TSharedPtr<FCurvePresetDef> Item, ESelectInfo::Type SelectType)
																																																																									  {
		if (SelectType == ESelectInfo::OnNavigation)
		{
			return;
		}
		if (!Item.IsValid())
		{
			return;
		}
		UCurveFloat *Target = WeakCurve.Get();
		if (!Target)
		{
			return;
		}
		if (auto Pinned = WeakSelf.Pin())
		{
			Pinned->ApplyPreset(Item, Target);
		} })
																																																																		   .Content()[SNew(STextBlock).Text(LOCTEXT("PickPreset", "Pick shape…")).Font(FAppStyle::GetFontStyle("SmallFont"))]] +
		   SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
												  .Text(LOCTEXT("CreateCurveBtn", "Create New Curve"))
												  .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
												  .IsEnabled_Lambda([WeakToolkit = ToolkitPtr]()
																	{
		auto T = WeakToolkit.Pin();
		return T.IsValid() && T->GetSelectedConsideration() != nullptr; })
												  .OnClicked(this, &SUtilityCurveEditorPanel::OnCreateNewCurveClicked)];
}

void SUtilityCurveEditorPanel::ApplyPreset(
	TSharedPtr<FCurvePresetDef> Preset,
	UCurveFloat *TargetCurve)
{
	if (!Preset.IsValid() || !TargetCurve)
	{
		return;
	}

	FScopedTransaction Transaction(FText::Format(
		LOCTEXT("ApplyPresetTx", "Apply Curve Preset '{0}'"), Preset->Name));

	TargetCurve->Modify();
	FRichCurve &Curve = TargetCurve->FloatCurve;
	Curve.Reset();

	for (const FCurvePresetDef::FKey &K : Preset->Keys)
	{
		const FKeyHandle Handle = Curve.AddKey(K.Time, K.Value);
		Curve.SetKeyInterpMode(Handle, K.Interp);

		if (K.TangentMode == RCTM_User)
		{
			Curve.SetKeyTangentMode(Handle, RCTM_User, /*bAutoSetTangents=*/false);
			FRichCurveKey &RichKey = Curve.GetKey(Handle);
			RichKey.ArriveTangent = K.ArriveTangent;
			RichKey.LeaveTangent = K.LeaveTangent;
		}
		else if (K.TangentMode != RCTM_Auto)
		{
			Curve.SetKeyTangentMode(Handle, K.TangentMode);
		}
	}

	TargetCurve->PostEditChange();

	UnbindCurve();
	auto Model = MakeUnique<FRichCurveEditorModelRaw>(&Curve, TargetCurve);
	Model->SetShortDisplayName(FText::FromString(TargetCurve->GetName()));
	BoundCurveID = CurveEditor->AddCurve(MoveTemp(Model));
	bHasBoundCurve = true;
	CurveEditor->PinCurve(BoundCurveID);
	CurveEditor->ZoomToFitAll();
}

FReply SUtilityCurveEditorPanel::OnCreateNewCurveClicked()
{
	auto Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid())
	{
		return FReply::Handled();
	}

	UUtilityConsiderationBase *Consideration = Toolkit->GetSelectedConsideration();
	if (!Consideration)
	{
		return FReply::Handled();
	}

	// State collected by the dialog
	bool bConfirmed = false;
	FString PickedPath = TEXT("/Game");
	TSharedPtr<SEditableTextBox> NameBox;

	FPathPickerConfig PathPickerConfig;
	PathPickerConfig.DefaultPath = TEXT("/Game");
	PathPickerConfig.bFocusSearchBoxWhenOpened = false;
	PathPickerConfig.OnPathSelected = FOnPathSelected::CreateLambda([&PickedPath](const FString &NewPath)
																	{ PickedPath = NewPath; });

	FContentBrowserModule &CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TSharedRef<SWidget> PathPickerWidget = CBModule.Get().CreatePathPicker(PathPickerConfig);

	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
										   .Title(LOCTEXT("CreateCurveWindowTitle", "Create Response Curve Asset"))
										   .SizingRule(ESizingRule::UserSized)
										   .ClientSize(FVector2D(450.f, 420.f))
										   .SupportsMinimize(false)
										   .SupportsMaximize(false);

	TWeakPtr<SWindow> WeakPickerWindow = PickerWindow;

	PickerWindow->SetContent(
		SNew(SVerticalBox)

		// Folder picker
		+ SVerticalBox::Slot()
			  .FillHeight(1.f)
			  .Padding(8.f, 8.f, 8.f, 4.f)
				  [SNew(SBorder)
					   .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						   [PathPickerWidget]]

		+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(8.f, 4.f)
				  [SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[SNew(STextBlock).Text(LOCTEXT("AssetNameLabel", "Name:"))] + SHorizontalBox::Slot().FillWidth(1.f)[SAssignNew(NameBox, SEditableTextBox).Text(FText::FromString(TEXT("NewResponseCurve")))]]

		+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(8.f, 4.f, 8.f, 8.f)
				  [SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.f) + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[SNew(SButton).Text(LOCTEXT("CreateBtn", "Create")).ButtonStyle(FAppStyle::Get(), "PrimaryButton").OnClicked_Lambda([&bConfirmed, WeakPickerWindow]() -> FReply
																																																																	{
					bConfirmed = true;
					if (auto W = WeakPickerWindow.Pin()) { W->RequestDestroyWindow(); }
					return FReply::Handled(); })] +
				   SHorizontalBox::Slot()
					   .AutoWidth()
						   [SNew(SButton)
								.Text(LOCTEXT("CancelBtn", "Cancel"))
								.OnClicked_Lambda([WeakPickerWindow]() -> FReply
												  {
					if (auto W = WeakPickerWindow.Pin()) { W->RequestDestroyWindow(); }
					return FReply::Handled(); })]]);

	FSlateApplication::Get().AddModalWindow(PickerWindow, FSlateApplication::Get().GetActiveTopLevelWindow());

	if (!bConfirmed)
	{
		return FReply::Handled();
	}

	FString AssetName = NameBox.IsValid() ? NameBox->GetText().ToString().TrimStartAndEnd() : FString();
	if (AssetName.IsEmpty() || PickedPath.IsEmpty())
	{
		return FReply::Handled();
	}

	AssetName = AssetName.Replace(TEXT(" "), TEXT("_"));

	const FString PackageName = PickedPath / AssetName;
	UPackage *Package = CreatePackage(*PackageName);
	if (!Package)
	{
		return FReply::Handled();
	}

	Package->FullyLoad();

	UCurveFloat *NewCurve = NewObject<UCurveFloat>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
	if (!NewCurve)
	{
		return FReply::Handled();
	}

	NewCurve->FloatCurve.AddKey(0.f, 0.f);
	NewCurve->FloatCurve.AddKey(1.f, 1.f);

	FAssetRegistryModule::AssetCreated(NewCurve);
	Package->MarkPackageDirty();

	FScopedTransaction Transaction(LOCTEXT("CreateCurveTx", "Create and Assign Response Curve"));
	Consideration->Modify();
	Consideration->ResponseCurve = NewCurve;

	BindConsideration(Consideration);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
