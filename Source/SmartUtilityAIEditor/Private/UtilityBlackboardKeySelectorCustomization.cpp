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

#include "UtilityBlackboardKeySelectorCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"

#include "BehaviorTree/BlackboardData.h"
#include "Core/UtilityBrainAsset.h"
#include "Core/UtilityBlackboardKeySelector.h"

TSharedRef<IPropertyTypeCustomization> FUtilityBlackboardKeySelectorCustomization::MakeInstance()
{
	return MakeShared<FUtilityBlackboardKeySelectorCustomization>();
}

void FUtilityBlackboardKeySelectorCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructHandle,
	FDetailWidgetRow &HeaderRow,
	IPropertyTypeCustomizationUtils &CustomizationUtils)
{
	KeyNameHandle = StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUtilityBlackboardKeySelector, SelectedKeyName));

	KeyOptions.Reset();
	KeyOptions.Add(MakeShared<FName>(NAME_None));

	UBlackboardData *BBData = FindBlackboardAsset(StructHandle);
	if (BBData)
	{
		// Collect keys from the asset and its parent chain
		TArray<FName> AllKeys;
		for (UBlackboardData *Current = BBData; Current; Current = Current->Parent)
		{
			for (const FBlackboardEntry &Entry : Current->Keys)
			{
				AllKeys.AddUnique(Entry.EntryName);
			}
		}
		AllKeys.Sort(FNameLexicalLess());
		for (const FName &Key : AllKeys)
		{
			KeyOptions.Add(MakeShared<FName>(Key));
		}
	}

	HeaderRow
		.NameContent()
			[StructHandle->CreatePropertyNameWidget()]
		.ValueContent()
		.MinDesiredWidth(200.f)
			[SAssignNew(ComboBox, SComboBox<TSharedPtr<FName>>)
				 .OptionsSource(&KeyOptions)
				 .OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
										  { return SNew(STextBlock)
												.Text(FText::FromName(Item.IsValid() ? *Item : NAME_None)); })
				 .OnSelectionChanged(this, &FUtilityBlackboardKeySelectorCustomization::OnKeySelected)
					 [SNew(STextBlock)
						  .Text(this, &FUtilityBlackboardKeySelectorCustomization::GetCurrentKeyText)]];

	// Sync initial combo selection to stored value
	FName CurrentValue = NAME_None;
	if (KeyNameHandle.IsValid())
	{
		KeyNameHandle->GetValue(CurrentValue);
	}
	for (const TSharedPtr<FName> &Option : KeyOptions)
	{
		if (Option.IsValid() && *Option == CurrentValue)
		{
			ComboBox->SetSelectedItem(Option);
			break;
		}
	}
}

void FUtilityBlackboardKeySelectorCustomization::OnKeySelected(TSharedPtr<FName> NewSelection, ESelectInfo::Type /*SelectInfo*/)
{
	if (NewSelection.IsValid() && KeyNameHandle.IsValid())
	{
		KeyNameHandle->SetValue(*NewSelection);
	}
}

FText FUtilityBlackboardKeySelectorCustomization::GetCurrentKeyText() const
{
	FName CurrentName = NAME_None;
	if (KeyNameHandle.IsValid())
	{
		KeyNameHandle->GetValue(CurrentName);
	}
	return FText::FromName(CurrentName);
}

UBlackboardData *FUtilityBlackboardKeySelectorCustomization::FindBlackboardAsset(TSharedRef<IPropertyHandle> StructHandle) const
{
	TArray<UObject *> OuterObjects;
	StructHandle->GetOuterObjects(OuterObjects);

	for (UObject *Obj : OuterObjects)
	{
		for (UObject *Current = Obj; Current; Current = Current->GetOuter())
		{
			if (UUtilityBrainAsset *BrainAsset = Cast<UUtilityBrainAsset>(Current))
			{
				return BrainAsset->GetBlackboardAsset();
			}
		}
	}
	return nullptr;
}
