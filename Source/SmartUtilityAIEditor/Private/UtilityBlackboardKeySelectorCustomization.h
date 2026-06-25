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
#include "IPropertyTypeCustomization.h"
#include "Widgets/Input/SComboBox.h"

class FUtilityBlackboardKeySelectorCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow &HeaderRow, IPropertyTypeCustomizationUtils &CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle, IDetailChildrenBuilder &ChildBuilder, IPropertyTypeCustomizationUtils &CustomizationUtils) override {}

private:
	class UBlackboardData *FindBlackboardAsset(TSharedRef<IPropertyHandle> StructHandle) const;
	void OnKeySelected(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);
	FText GetCurrentKeyText() const;

	TSharedPtr<IPropertyHandle> KeyNameHandle;
	TArray<TSharedPtr<FName>> KeyOptions;
	TSharedPtr<SComboBox<TSharedPtr<FName>>> ComboBox;
};
