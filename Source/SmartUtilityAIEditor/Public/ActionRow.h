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
#include "Widgets/Views/STableRow.h"

// SMultiColumnTableRow subclass used by SActionListPanel. Defines the visual
// layout of a single row in the action tree: name text, bucket color dot,
// interrupt policy icon, and an optional score badge during PIE. Handles
// drag-and-drop source/target for reordering.
template <typename ItemType>
class SActionRow : public SMultiColumnTableRow<ItemType>
{
public:
    using Super = SMultiColumnTableRow<ItemType>;

    SLATE_BEGIN_ARGS(SActionRow<ItemType>) {}
    SLATE_END_ARGS()

    void Construct(const typename Super::FArguments &InArgs, const TSharedRef<STableViewBase> &OwnerTableView)
    {
        Super::Construct(InArgs, OwnerTableView);
    }
};
