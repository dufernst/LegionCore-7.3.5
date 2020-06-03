/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "DB2Stores.h"
#include "ItemTemplate.h"

uint32 const SocketColorToGemTypeMask[19] =
{
    0,
    SOCKET_COLOR_META, // 1
    SOCKET_COLOR_RED, // 2
    SOCKET_COLOR_YELLOW, // 3
    SOCKET_COLOR_BLUE, // 4
    SOCKET_COLOR_HYDRAULIC, // 5
    SOCKET_COLOR_COGWHEEL, // 6
    SOCKET_COLOR_PRISMATIC, // 7
    SOCKET_COLOR_RELIC_IRON, // 8
    SOCKET_COLOR_RELIC_BLOOD, // 9
    SOCKET_COLOR_RELIC_SHADOW, // 10
    SOCKET_COLOR_RELIC_FEL, // 11
    SOCKET_COLOR_RELIC_ARCANE, // 12
    SOCKET_COLOR_RELIC_FROST, // 13
    SOCKET_COLOR_RELIC_FIRE, // 14
    SOCKET_COLOR_RELIC_WATER, // 15
    SOCKET_COLOR_RELIC_LIFE, // 16
    SOCKET_COLOR_RELIC_WIND, // 17
    SOCKET_COLOR_RELIC_HOLY // 18
};

uint32 ItemTemplate::GetArmor(uint32 itemLevel) const
{
    if (itemLevel > 1000) // Now limit on DBC
        itemLevel = 1000;

    uint32 quality = GetQuality() != ITEM_QUALITY_HEIRLOOM ? GetQuality() : ITEM_QUALITY_RARE;
    if (quality > ITEM_QUALITY_ARTIFACT)
        return 0;

    // all items but shields
    if (GetClass() != ITEM_CLASS_ARMOR || GetSubClass() != ITEM_SUBCLASS_ARMOR_SHIELD)
    {
        ItemArmorQualityEntry const* armorQuality = sItemArmorQualityStore.LookupEntry(itemLevel);
        ItemArmorTotalEntry const* armorTotal = sItemArmorTotalStore.LookupEntry(itemLevel);
        if (!armorQuality || !armorTotal)
            return 0;

        uint32 inventoryType = GetInventoryType();
        if (inventoryType == INVTYPE_ROBE)
            inventoryType = INVTYPE_CHEST;

        ArmorLocationEntry const* location = sArmorLocationStore.LookupEntry(inventoryType);
        if (!location)
            return 0;

        if (GetSubClass() < ITEM_SUBCLASS_ARMOR_CLOTH || GetSubClass() > ITEM_SUBCLASS_ARMOR_PLATE)
            return 0;

        float total = 1.0f;
        float locationModifier = 1.0f;
        switch (GetSubClass())
        {
        case ITEM_SUBCLASS_ARMOR_CLOTH:
            total = armorTotal->Cloth;
            locationModifier = location->Clothmodifier;
            break;
        case ITEM_SUBCLASS_ARMOR_LEATHER:
            total = armorTotal->Leather;
            locationModifier = location->Leathermodifier;
            break;
        case ITEM_SUBCLASS_ARMOR_MAIL:
            total = armorTotal->Mail;
            locationModifier = location->Chainmodifier;
            break;
        case ITEM_SUBCLASS_ARMOR_PLATE:
            total = armorTotal->Plate;
            locationModifier = location->Platemodifier;
            break;
        default:
            break;
        }

        return uint32(armorQuality->Qualitymod[quality] * total * locationModifier + 0.5f);
    }

    // shields
    ItemArmorShieldEntry const* shield = sItemArmorShieldStore.LookupEntry(itemLevel);
    if (!shield)
        return 0;

    return uint32(shield->Quality[quality] + 0.5f);
}

void ItemTemplate::GetDamage(uint32 itemLevel, float& minDamage, float& maxDamage) const
{
    if (itemLevel > 1000) // Now limit on DBC
        itemLevel = 1000;

    minDamage = maxDamage = 0.0f;
    uint32 quality = ItemQualities(GetQuality()) != ITEM_QUALITY_HEIRLOOM ? ItemQualities(GetQuality()) : ITEM_QUALITY_RARE;
    if (GetClass() != ITEM_CLASS_WEAPON || quality > ITEM_QUALITY_ARTIFACT)
        return;

    if (GetInventoryType() > INVTYPE_RANGEDRIGHT)
        return;

    float dps = 0.0f;
    switch (GetInventoryType())
    {
        case INVTYPE_AMMO:
            dps = sItemDamageAmmoStore.AssertEntry(itemLevel)->Quality[quality];
            break;
        case INVTYPE_2HWEAPON:
            if (GetFlags2() & ITEM_FLAG2_CASTER_WEAPON)
                dps = sItemDamageTwoHandCasterStore.AssertEntry(itemLevel)->Quality[quality];
            else
                dps = sItemDamageTwoHandStore.AssertEntry(itemLevel)->Quality[quality];
            break;
        case INVTYPE_RANGED:
        case INVTYPE_THROWN:
        case INVTYPE_RANGEDRIGHT:
            switch (GetSubClass())
            {
                case ITEM_SUBCLASS_WEAPON_WAND:
                    dps = sItemDamageOneHandCasterStore.AssertEntry(itemLevel)->Quality[quality];
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_GUN:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    if (GetFlags2() & ITEM_FLAG2_CASTER_WEAPON)
                        dps = sItemDamageTwoHandCasterStore.AssertEntry(itemLevel)->Quality[quality];
                    else
                        dps = sItemDamageTwoHandStore.AssertEntry(itemLevel)->Quality[quality];
                    break;
                default:
                    return;
            }
            break;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPONOFFHAND:
            if (GetFlags2() & ITEM_FLAG2_CASTER_WEAPON)
                dps = sItemDamageOneHandCasterStore.AssertEntry(itemLevel)->Quality[quality];
            else
                dps = sItemDamageOneHandStore.AssertEntry(itemLevel)->Quality[quality];
            break;
        default:
            return;
    }

    float avgDamage = dps * GetDelay() * 0.001f;
    minDamage = (GetDmgVariance() * -0.5f + 1.0f) * avgDamage;
    maxDamage = floor(float(avgDamage * (GetDmgVariance() * 0.5f + 1.0f) + 0.5f));
}

bool ItemTemplate::IsUsableBySpecialization(uint32 specId, uint8 level) const
{
    if (!ItemSpecExist) // if item don`t use spec allways good
        return true;

    if (ChrSpecializationEntry const* chrSpecialization = sChrSpecializationStore.LookupEntry(specId))
    {
        std::size_t levelIndex = 0;
        if (level >= 110)
            levelIndex = 2;
        else if (level > 40)
            levelIndex = 1;

        return Specializations[levelIndex].test(CalculateItemSpecBit(chrSpecialization));
    }

    return false;
}

std::size_t ItemTemplate::CalculateItemSpecBit(ChrSpecializationEntry const* spec)
{
    return (spec->ClassID - 1) * MAX_SPECIALIZATIONS + spec->OrderIndex;
}

bool ItemTemplate::AllowToLooter() const
{
    if (GetQuality() == ITEM_QUALITY_LEGENDARY || GetClass() == ITEM_CLASS_CONSUMABLE)
        return false;

    if (GetId() == 138019)
        return false;

    if (GetFlags() & ITEM_FLAG_HAS_LOOT)
        return false;

    if (IsNotNeedCheck())
        return false;

    return true;
}

uint32 ItemTemplate::GetBaseArmor() const
{
    return GetArmor(GetBaseItemLevel());
}

void ItemTemplate::GetBaseDamage(float& minDamage, float& maxDamage) const
{
    GetDamage(GetBaseItemLevel(), minDamage, maxDamage);
}

// helpers
bool ItemTemplate::CanChangeEquipStateInCombat() const
{
    switch (GetInventoryType())
    {
        case INVTYPE_RELIC:
        case INVTYPE_SHIELD:
        case INVTYPE_HOLDABLE:
            return true;
        default:
            break;
    }

    switch (GetClass())
    {
        case ITEM_CLASS_WEAPON:
        case ITEM_CLASS_PROJECTILE:
            return true;
        default:
            break;
    }

    return false;
}

bool ItemTemplate::IsCurrencyToken() const
{
    return (GetBagFamily() & BAG_FAMILY_MASK_CURRENCY_TOKENS) != 0;
}

bool ItemTemplate::IsNotAppearInGuildNews() const
{
    return (GetFlags3() & ITEM_FLAG3_DONT_DISPLAY_IN_GUILD_NEWS) != 0;
}

bool ItemTemplate::IsPotion() const
{
    return GetClass() == ITEM_CLASS_CONSUMABLE && GetSubClass() == ITEM_SUBCLASS_POTION;
}

bool ItemTemplate::IsVellum() const
{
    return GetClass() == ITEM_CLASS_TRADE_GOODS && (GetFlags3() & ITEM_FLAG3_CAN_STORE_ENCHANTS);
}

bool ItemTemplate::IsConjuredConsumable() const
{
    return GetClass() == ITEM_CLASS_CONSUMABLE && (GetFlags() & ITEM_FLAG_CONJURED);
}

bool ItemTemplate::IsCraftingReagent() const
{
    return (GetFlags2() & ITEM_FLAG2_USED_IN_A_TRADESKILL) != 0;
}

uint32 ItemTemplate::GetMaxStackSize() const
{
    return (GetStackable() == 2147483647 || GetStackable() <= 0) ? uint32(0x7FFFFFFF-1) : uint32(GetStackable());
}

bool ItemTemplate::IsOtherDrops() const
{
    return GetClass() == ITEM_CLASS_CONSUMABLE
            || GetClass() == ITEM_CLASS_MISCELLANEOUS
            || GetClass() == ITEM_CLASS_QUEST
            || IsCraftingReagent()
            || GetQuality() < ITEM_QUALITY_UNCOMMON;
}

bool ItemTemplate::IsRecipe() const
{
    return GetClass() == ITEM_CLASS_RECIPE;
}

bool ItemTemplate::IsNotNeedCheck() const
{
    return GetBonding() == BIND_WHEN_EQUIPED || GetBonding() == BIND_WHEN_USE || GetBonding() == NO_BIND || (GetFlags() & ITEM_FLAG_IS_BOUND_TO_ACCOUNT);
}

bool ItemTemplate::IsRangedWeapon() const
{
    return GetClass() == ITEM_CLASS_WEAPON &&
          (GetSubClass() == ITEM_SUBCLASS_WEAPON_BOW ||
           GetSubClass() == ITEM_SUBCLASS_WEAPON_GUN ||
           GetSubClass() == ITEM_SUBCLASS_WEAPON_CROSSBOW);
}

bool ItemTemplate::IsOneHanded() const
{
    return GetClass() == ITEM_CLASS_WEAPON && (
        GetSubClass() == ITEM_SUBCLASS_WEAPON_SWORD ||
        GetSubClass() == ITEM_SUBCLASS_WEAPON_AXE ||
        GetSubClass() == ITEM_SUBCLASS_WEAPON_MACE ||
        GetSubClass() == ITEM_SUBCLASS_WEAPON_DAGGER ||
        GetSubClass() == ITEM_SUBCLASS_WEAPON_FIST_WEAPON ||
        GetSubClass() == ITEM_SUBCLASS_WEAPON_EXOTIC);
}

bool ItemTemplate::IsTwoHandedWeapon() const
{
    return GetClass() == ITEM_CLASS_WEAPON && !IsOneHanded();
}

bool ItemTemplate::HasStats() const
{
    for (uint8 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        if (GetItemStatType(i) > 0)
            return true;

    return false;
}

bool ItemTemplate::IsLegionLegendary() const
{
    return GetLimitCategory() == 357;
}

bool ItemTemplate::IsLegendaryLoot() const
{
    if (GetId() == 146666 || GetId() == 146669 || GetId() == 146668 || GetId() == 146667) // Craft Legendary Item
        return false;

    if (GetId() == 152626) // Insignia of the Grand Army, is quest drop
        return false;

    if (GetId() == 154172) // Top trinket Legendary Item, Aman'Thul's Vision
        return false;

    if (!HasStats()) // Collected Legendary Item
        return false;

    if (sWorld->getIntConfig(CONFIG_ITEM_LEGENDARY_LIMIT) < GetId()) // Limit by patch
        return false;

    return GetLimitCategory() == 357;
}

bool ItemTemplate::IsLegionLegendaryToken() const
{
    switch (GetId())
    {
        case 147294: // Bone-Wrought Coffer of the Damned
        case 147295: // Demonslayer's Soul-Sealed Satchel
        case 147296: // Living Root-Bound Cache
        case 147297: // Deepwood Ranger's Quiver
        case 147298: // Spell-Secured Pocket of Infinite Depths
        case 147299: // Hand-Carved Jade Puzzle Box
        case 147300: // Light-Bound Reliquary
        case 147301: // Coffer of Twin Faiths
        case 147302: // Hollow Skeleton Key
        case 147303: // Giant Elemental's Closed Stone Fist
        case 147304: // Pocket Keystone to Abandoned World
        case 147305: // Stalwart Champion's War Chest
            return true;
        default:
            break;
    }

    return false;
}

bool ItemTemplate::CanWarforged() const
{
    return (HasStats() || (GetInventoryType() == INVTYPE_TRINKET) || (GetClass() == ITEM_CLASS_GEM && GetSubClass() == ITEM_SUBCLASS_GEM_ARTIFACT_RELIC)) && GetQuality() <= ITEM_QUALITY_EPIC && ItemLevel >= 680;
}

bool ItemTemplate::HasEnchantment() const
{
    switch (GetId())
    {
        case 147275:
        case 147276:
        case 147278:
        case 147242:
        case 147244:
        case 147241:
        case 147243:
        case 147263:
        case 147257:
        case 147229:
        case 147231:
        case 147232:
        case 147230:
        case 147250:
        case 147252:
        case 147249:
        case 147251:
        case 147226:
        case 147228:
        case 147225:
        case 147227:
        case 147269:
        case 147246:
        case 147248:
        case 147245:
        case 147247:
        case 147254:
        case 147253:
        case 147256:
        case 147255:
        case 147238:
        case 147240:
        case 147237:
        case 147239:
        case 147234:
        case 147233:
        case 147236:
        case 147235:
        case 152770:
        case 152771:
        case 152772:
        case 152773:
        case 152774:
        case 152775:
        case 152776:
        case 152777:
        case 152779:
        case 152778:
        case 152780:
        case 152746:
        case 152747:
        case 152748:
        case 152749:
        case 152750:
        case 152751:
        case 152752:
        case 152753:
        case 152754:
        case 152755:
        case 152756:
        case 152757:
        case 152758:
        case 152759:
        case 152760:
        case 152761:
        case 152762:
        case 152763:
        case 152764:
        case 152765:
        case 152766:
        case 152767:
        case 152768:
        case 152769:
            return true;
        default:
            break;
    }

    return GetRandomSelect() || GetItemRandomSuffixGroupID();
}
