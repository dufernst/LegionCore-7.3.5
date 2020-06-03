/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ITEM_ENCHANTMENT_MGR_H
#define _ITEM_ENCHANTMENT_MGR_H

#include "Common.h"

struct ItemTemplate;

enum class ItemRandomEnchantmentType : uint8
{
    Property = 0,
    Suffix = 1,
    BonusList = 2,
    BonusListAddition = 3
};

struct ItemRandomEnchantmentId
{
    ItemRandomEnchantmentId() = default;
    ItemRandomEnchantmentId(ItemRandomEnchantmentType type, uint32 id) : Type(type), Id(id) { }

    ItemRandomEnchantmentType Type = ItemRandomEnchantmentType::Property;
    uint32 Id = 0;
};

void LoadRandomEnchantmentsTable();
ItemRandomEnchantmentId GetItemEnchantMod(int32 entry, ItemRandomEnchantmentType type, uint32 ItemID, uint32 spec_id);
uint32 GetItemThirdStat(uint32 ItemID, bool& sosketSlot1, bool& sosketSlot2, bool& sosketSlot3);
uint32 GenerateEnchSuffixFactor(ItemTemplate const* proto, uint32 level = 0);
uint32 GetRandomPropertyPoints(uint32 itemLevel, uint32 quality, uint32 inventoryType, uint32 subclass);
bool CheckSpecProp(uint32 ench, ItemRandomEnchantmentType type, uint32 SpecID);
bool CheckStatsSpec(uint32 StatType, uint32 SpecID);
#endif

