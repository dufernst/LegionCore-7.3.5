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

#include <stdlib.h>
#include "ItemEnchantmentMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Containers.h"
#include "ObjectMgr.h"

struct EnchStoreItem
{
    ItemRandomEnchantmentType type;
    uint32 ench;
    float chance;

    EnchStoreItem() : type(ItemRandomEnchantmentType::Property), ench(0), chance(0) { }
    EnchStoreItem(ItemRandomEnchantmentType _type, uint32 _ench, float _chance) : type(_type), ench(_ench), chance(_chance) { }
};

typedef std::vector<EnchStoreItem> EnchStoreList;

class EnchantmentStore
{
    std::unordered_map<uint32, EnchStoreList> _data[2];

public:
    std::unordered_map<uint32, EnchStoreList>& operator[](ItemRandomEnchantmentType type)
    {
        ASSERT(type != ItemRandomEnchantmentType::BonusList, "Random bonus lists do not have their own storage, use Suffix for them");
        return _data[uint8(type)];
    }

} static RandomItemEnch;

class EnchantmentItemStore
{
public:
    EnchStoreList _data;
} static ItemBonusEnch;

void LoadRandomEnchantmentsTable()
{
    uint32 oldMSTime = getMSTime();

    RandomItemEnch[ItemRandomEnchantmentType::Property].clear();
    RandomItemEnch[ItemRandomEnchantmentType::Suffix].clear();
    ItemBonusEnch._data.clear();

    //                                                 0      1     2      3
    QueryResult result = WorldDatabase.Query("SELECT entry, type, ench, chance FROM item_enchantment_template");

    if (result)
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();

            uint32 entry = fields[0].GetUInt32();
            ItemRandomEnchantmentType type = ItemRandomEnchantmentType(fields[1].GetUInt8());
            uint32 ench = fields[2].GetUInt32();
            float chance = fields[3].GetFloat();

            switch (type)
            {
                case ItemRandomEnchantmentType::Property:
                    if (!sItemRandomPropertiesStore.LookupEntry(ench))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Property %u used in `item_enchantment_template` by entry %u doesn't have exist in ItemRandomProperties.db2", ench, entry);
                        continue;
                    }
                    break;
                case ItemRandomEnchantmentType::Suffix:
                    if (!sItemRandomSuffixStore.LookupEntry(ench))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Suffix %u used in `item_enchantment_template` by entry %u doesn't have exist in ItemRandomSuffix.db2", ench, entry);
                        continue;
                    }
                    break;
                case ItemRandomEnchantmentType::BonusList:
                    if (!sDB2Manager.GetItemBonusList(ench))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Bonus list %u used in `item_enchantment_template` by entry %u doesn't have exist in ItemBonus.db2", ench, entry);
                        continue;
                    }
                    break;
                default:
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Invalid random enchantment type specified in `item_enchantment_template` table for `entry` %u `ench` %u", entry, ench);
                    break;
            }

            if (chance < 0.000001f || chance > 100.0f)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Random item enchantment for entry %u type %u ench %u has invalid chance %f", entry, uint32(type), ench, chance);
                continue;
            }

            switch (type)
            {
                case ItemRandomEnchantmentType::Property:
                    RandomItemEnch[ItemRandomEnchantmentType::Property][entry].emplace_back(type, ench, chance);
                    break;
                case ItemRandomEnchantmentType::Suffix:
                case ItemRandomEnchantmentType::BonusList: // random bonus lists use RandomSuffix field in ItemSparse.db2
                    RandomItemEnch[ItemRandomEnchantmentType::Suffix][entry].emplace_back(type, ench, chance);
                    break;
                default:
                    break;
            }

            ++count;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Item Enchantment definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Item Enchantment definitions. DB table `item_enchantment_template` is empty.");

    //                                      0       1
    result = WorldDatabase.Query("SELECT BonusID, chance FROM item_enchantment_bonus");
    if (result)
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();

            uint32 BonusID = fields[0].GetUInt32();
            float chance = fields[1].GetFloat();

            if (chance < 0.000001f || chance > 100.0f)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Random item enchantment for BonusID %u has invalid chance %f", BonusID, chance);
                continue;
            }

            ItemBonusEnch._data.emplace_back(ItemRandomEnchantmentType::BonusList, BonusID, chance);

            ++count;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Item Enchantment Bonus definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Item Enchantment Bonus definitions. DB table `item_enchantment_bonus` is empty.");
}

ItemRandomEnchantmentId GetItemEnchantMod(int32 entry, ItemRandomEnchantmentType type, uint32 ItemID, uint32 spec_id)
{
    ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(ItemID);
    if (!itemProto)
        return{};

    if (entry == -1)
        return{};

    EnchStoreList enchList;

    if (type == ItemRandomEnchantmentType::Suffix)
        enchList = ItemBonusEnch._data;

    if (enchList.empty())
    {
        if (!entry)
            return{};

        auto tab = RandomItemEnch[type].find(entry);
        if (tab == RandomItemEnch[type].end())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Item RandomProperty / RandomSuffix id #%u spec_id %u used in `item_template` but it does not have records in `item_enchantment_template` table.", entry, spec_id);
            return{};
        }
        enchList = tab->second;
    }

    auto selectedItr = Trinity::Containers::SelectRandomWeightedContainerElement(enchList, [type, spec_id](EnchStoreItem const& enchant)
    {
        if(spec_id)
        {
            if(!CheckSpecProp(enchant.ench, type, spec_id))
                return 0.0f;
        }
        return enchant.chance;
    });

    return{ selectedItr->type, selectedItr->ench };
}

uint32 GetItemThirdStat(uint32 ItemID, bool& sosketSlot1, bool& sosketSlot2, bool& sosketSlot3)
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(ItemID);
    if (!proto)
        return 0;

    EnchStoreList enchList;

    if (proto->MaxDurability)
    {
        if (sosketSlot1)
        {
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 40, 22.71235f);
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 41, 22.71235f);
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 42, 22.71235f);
        }
        if (sosketSlot2)
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 43, 22.71235f);
        if (sosketSlot3)
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 1808, 9.1506f);
    }
    else
    {
        if (sosketSlot1)
        {
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 40, 22.71235f);
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 41, 22.71235f);
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 42, 22.71235f);
        }
        if (sosketSlot2)
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 1808, 22.71235f);
        if (sosketSlot3)
            enchList.emplace_back(ItemRandomEnchantmentType::BonusListAddition, 1808, 9.1506f);
    }

    if (enchList.empty())
        return 0;

    auto selectedItr = Trinity::Containers::SelectRandomWeightedContainerElement(enchList, [](EnchStoreItem const& enchant)
    {
        return enchant.chance;
    });

    switch (selectedItr->ench)
    {
        case 40:
        case 41:
        case 42:
            sosketSlot1 = false;
            break;
        case 43:
            sosketSlot2 = false;
            break;
        case 1808:
            if (!proto->MaxDurability && !sosketSlot3)
                sosketSlot2 = false;
            sosketSlot3 = false;
            break;
    }

    return selectedItr->ench;
}

uint32 GenerateEnchSuffixFactor(ItemTemplate const* itemProto, uint32 level)
{
    if (!itemProto)
        return 0;

    if (!itemProto->GetItemRandomSuffixGroupID())
        return 0;

    return GetRandomPropertyPoints(level, itemProto->GetQuality(), itemProto->GetInventoryType(), itemProto->GetSubClass());
}

uint32 GetRandomPropertyPoints(uint32 itemLevel, uint32 quality, uint32 inventoryType, uint32 subClass)
{
    uint32 propIndex;

    switch (inventoryType)
    {
        case INVTYPE_HEAD:
        case INVTYPE_BODY:
        case INVTYPE_CHEST:
        case INVTYPE_LEGS:
        case INVTYPE_RANGED:
        case INVTYPE_2HWEAPON:
        case INVTYPE_ROBE:
        case INVTYPE_THROWN:
            propIndex = 0;
            break;
        case INVTYPE_RANGEDRIGHT:
            if (subClass == ITEM_SUBCLASS_WEAPON_WAND)
                propIndex = 3;
            else
                propIndex = 0;
            break;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPONOFFHAND:
            propIndex = 3;
            break;
        case INVTYPE_SHOULDERS:
        case INVTYPE_WAIST:
        case INVTYPE_FEET:
        case INVTYPE_HANDS:
        case INVTYPE_TRINKET:
            propIndex = 1;
            break;
        case INVTYPE_NECK:
        case INVTYPE_WRISTS:
        case INVTYPE_FINGER:
        case INVTYPE_SHIELD:
        case INVTYPE_CLOAK:
        case INVTYPE_HOLDABLE:
            propIndex = 2;
            break;
        case INVTYPE_RELIC:
            propIndex = 4;
            break;
        default:
            return 0;
    }

    RandPropPointsEntry const* randPropPointsEntry = sRandPropPointsStore.LookupEntry(itemLevel);
    if (!randPropPointsEntry)
        return 0;

    switch (quality)
    {
        case ITEM_QUALITY_UNCOMMON:
            return randPropPointsEntry->Good[propIndex];
        case ITEM_QUALITY_RARE:
        case ITEM_QUALITY_HEIRLOOM:
            return randPropPointsEntry->Superior[propIndex];
        case ITEM_QUALITY_EPIC:
        case ITEM_QUALITY_LEGENDARY:
        case ITEM_QUALITY_ARTIFACT:
            return randPropPointsEntry->Epic[propIndex];
        default:
            break;
    }

    return 0;
}

bool CheckSpecProp(uint32 ench, ItemRandomEnchantmentType type, uint32 SpecID)
{
    if (type == ItemRandomEnchantmentType::Property)
    {
        if(ItemRandomPropertiesEntry const* random_id = sItemRandomPropertiesStore.LookupEntry(ench))
        {
            for (auto enchantmentID : random_id->Enchantment)
            {
                if (SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchantmentID))
                {
                    for (uint8 s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
                    {
                        if (pEnchant->Effect[s] != ITEM_ENCHANTMENT_TYPE_STAT)
                            continue;

                        if (!CheckStatsSpec(pEnchant->EffectArg[s], SpecID))
                            return false;
                    }
                }
            }
        }
    }
    else if (type == ItemRandomEnchantmentType::Suffix)
    {
        if(ItemRandomSuffixEntry const* random_id = sItemRandomSuffixStore.LookupEntry(ench))
        {
            for (auto enchantmentID : random_id->Enchantment)
            {
                if (SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchantmentID))
                {
                    for (uint8 s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
                    {
                        if (pEnchant->Effect[s] != ITEM_ENCHANTMENT_TYPE_STAT)
                            continue;

                        if (!CheckStatsSpec(pEnchant->EffectArg[s], SpecID))
                            return false;
                    }
                }
            }
        }
    }
    else if (type == ItemRandomEnchantmentType::BonusList)
    {
        if (DB2Manager::ItemBonusList const* bonuses = sDB2Manager.GetItemBonusList(ench))
        {
            for (ItemBonusEntry const* itemBonus : *bonuses)
                if (itemBonus->Type == ITEM_BONUS_STAT)
                    if (!CheckStatsSpec(itemBonus->Value[0], SpecID))
                        return false;
        }
    }
    return true;
}

bool CheckStatsSpec(uint32 StatType, uint32 SpecID)
{
    for (ItemSpecEntry const* itemSpec : sItemSpecStore)
    {
        if (itemSpec->SpecializationID != SpecID)
            continue;

        switch (StatType)
        {
            case ITEM_MOD_AGILITY:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_AGILITY)
                    return true;
                break;
            case ITEM_MOD_STRENGTH:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_STRENGTH)
                    return true;
                break;
            case ITEM_MOD_INTELLECT:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_INTELLECT)
                    return true;
                break;
            case ITEM_MOD_SPIRIT:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_SPIRIT)
                    return true;
                break;
            case ITEM_MOD_DODGE_RATING:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_DODGE)
                    return true;
                break;
            case ITEM_MOD_PARRY_RATING:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_PARRY)
                    return true;
                break;
            case ITEM_MOD_CRIT_MELEE_RATING:
            case ITEM_MOD_CRIT_RANGED_RATING:
            case ITEM_MOD_CRIT_SPELL_RATING:
            case ITEM_MOD_CRIT_RATING:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_CRIT)
                    return true;
                break;
            case ITEM_MOD_HASTE_MELEE_RATING:
            case ITEM_MOD_HASTE_RANGED_RATING:
            case ITEM_MOD_HASTE_SPELL_RATING:
            case ITEM_MOD_HASTE_RATING:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_HASTE)
                    return true;
                break;
            case ITEM_MOD_HIT_RATING:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_HIT)
                    return true;
                break;
            case ITEM_MOD_EXTRA_ARMOR:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_BONUS_ARMOR)
                    return true;
                break;
            case ITEM_MOD_AGI_STR_INT:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_STRENGTH)
                    return true;
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_STRENGTH)
                    return true;
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_INTELLECT)
                    return true;
                break;
            case ITEM_MOD_AGI_STR:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_AGILITY)
                    return true;
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_STRENGTH)
                    return true;
                break;
            case ITEM_MOD_AGI_INT:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_AGILITY)
                    return true;
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_INTELLECT)
                    return true;
                break;
            case ITEM_MOD_STR_INT:
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_INTELLECT)
                    return true;
                if (itemSpec->SecondaryStat == ITEM_SPEC_STAT_STRENGTH)
                    return true;
                break;
            default:
                break;
        }
    }
    return false;
}
