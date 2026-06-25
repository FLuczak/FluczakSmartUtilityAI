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

#include "SmartUtilityAIEditor.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "PropertyEditorModule.h"
#include "UtilityBrainAssetTypeActions.h"
#include "UtilityAIEditorStyle.h"
#include "UtilityBlackboardKeySelectorCustomization.h"
#include "UtilityBrainDebugger.h"
#include "Core/UtilityBlackboardKeySelector.h"
#include "GameplayDebugger.h"

#define LOCTEXT_NAMESPACE "FSmartUtilityAIEditorModule"

void FSmartUtilityAIEditorModule::StartupModule()
{
	FUtilityAIEditorStyle::Initialize();

	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UtilityAIAssetCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("UtilityAI")),
		NSLOCTEXT("UtilityAI", "Category", "Utility AI"));

	BrainAssetTypeActions = MakeShared<FUtilityBrainAssetTypeActions>(UtilityAIAssetCategory);
	AssetTools.RegisterAssetTypeActions(BrainAssetTypeActions.ToSharedRef());

	FPropertyEditorModule &PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FUtilityBlackboardKeySelector::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUtilityBlackboardKeySelectorCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger &Debugger = IGameplayDebugger::Get();
		Debugger.RegisterCategory(TEXT("UtilityAI"),
								  IGameplayDebugger::FOnGetCategory::CreateStatic(&FUtilityBrainDebugger::MakeInstance),
								  EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
		Debugger.NotifyCategoriesChanged();
	}
}

void FSmartUtilityAIEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(BrainAssetTypeActions.ToSharedRef());
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule &PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FUtilityBlackboardKeySelector::StaticStruct()->GetFName());
	}

	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger &Debugger = IGameplayDebugger::Get();
		Debugger.UnregisterCategory(TEXT("UtilityAI"));
		Debugger.NotifyCategoriesChanged();
	}

	FUtilityAIEditorStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSmartUtilityAIEditorModule, SmartUtilityAIEditor)
