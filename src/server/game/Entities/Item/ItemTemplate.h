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

#ifndef _ITEMPROTOTYPE_H
#define _ITEMPROTOTYPE_H

#include <bitset>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "DB2Structure.h"
#include "SharedDefines.h"

class Player;

enum ItemModType
{
    ITEM_MOD_MANA                     = 0,
    ITEM_MOD_HEALTH                   = 1,
    ITEM_MOD_AGILITY                  = 3,
    ITEM_MOD_STRENGTH                 = 4,
    ITEM_MOD_INTELLECT                = 5,
    ITEM_MOD_SPIRIT                   = 6,
    ITEM_MOD_STAMINA                  = 7,
    ITEM_MOD_DEFENSE_SKILL_RATING     = 12,
    ITEM_MOD_DODGE_RATING             = 13,
    ITEM_MOD_PARRY_RATING             = 14,
    ITEM_MOD_BLOCK_RATING             = 15,
    ITEM_MOD_HIT_MELEE_RATING         = 16,
    ITEM_MOD_HIT_RANGED_RATING        = 17,
    ITEM_MOD_HIT_SPELL_RATING         = 18,
    ITEM_MOD_CRIT_MELEE_RATING        = 19,
    ITEM_MOD_CRIT_RANGED_RATING       = 20,
    ITEM_MOD_CRIT_SPELL_RATING        = 21,
    ITEM_MOD_HIT_TAKEN_MELEE_RATING   = 22,
    ITEM_MOD_HIT_TAKEN_RANGED_RATING  = 23,
    ITEM_MOD_HIT_TAKEN_SPELL_RATING   = 24,
    ITEM_MOD_CRIT_TAKEN_MELEE_RATING  = 25,
    ITEM_MOD_CRIT_TAKEN_RANGED_RATING = 26,
    ITEM_MOD_CRIT_TAKEN_SPELL_RATING  = 27,
    ITEM_MOD_HASTE_MELEE_RATING       = 28,
    ITEM_MOD_HASTE_RANGED_RATING      = 29,
    ITEM_MOD_HASTE_SPELL_RATING       = 30,
    ITEM_MOD_HIT_RATING               = 31,
    ITEM_MOD_CRIT_RATING              = 32,
    ITEM_MOD_HIT_TAKEN_RATING         = 33,
    ITEM_MOD_CRIT_TAKEN_RATING        = 34,
    ITEM_MOD_RESILIENCE_RATING        = 35,
    ITEM_MOD_HASTE_RATING             = 36,
    ITEM_MOD_EXPERTISE_RATING         = 37,
    ITEM_MOD_ATTACK_POWER             = 38,
    ITEM_MOD_RANGED_ATTACK_POWER      = 39,
    ITEM_MOD_VERSATILITY              = 40,
    ITEM_MOD_SPELL_HEALING_DONE       = 41,
    ITEM_MOD_SPELL_DAMAGE_DONE        = 42,
    ITEM_MOD_MANA_REGENERATION        = 43,
    ITEM_MOD_ARMOR_PENETRATION_RATING = 44,
    ITEM_MOD_SPELL_POWER              = 45,
    ITEM_MOD_HEALTH_REGEN             = 46,
    ITEM_MOD_SPELL_PENETRATION        = 47,
    ITEM_MOD_BLOCK_VALUE              = 48,
    ITEM_MOD_MASTERY_RATING           = 49,
    ITEM_MOD_EXTRA_ARMOR              = 50,
    ITEM_MOD_FIRE_RESISTANCE          = 51,
    ITEM_MOD_FROST_RESISTANCE         = 52,
    ITEM_MOD_HOLY_RESISTANCE          = 53,
    ITEM_MOD_SHADOW_RESISTANCE        = 54,
    ITEM_MOD_NATURE_RESISTANCE        = 55,
    ITEM_MOD_ARCANE_RESISTANCE        = 56,
    ITEM_MOD_PVP_POWER                = 57,
    ITEM_MOD_CR_AMPLIFY               = 58,
    ITEM_MOD_CR_MULTISTRIKE           = 59,
    ITEM_MOD_CR_READINESS             = 60,
    ITEM_MOD_CR_SPEED                 = 61,
    ITEM_MOD_CR_LIFESTEAL             = 62,
    ITEM_MOD_CR_AVOIDANCE             = 63,
    ITEM_MOD_CR_STURDINESS            = 64,
    ITEM_MOD_CR_UNUSED_7              = 65,
    ITEM_MOD_CR_CLEAVE                = 66,
    ITEM_MOD_CR_UNUSED_9              = 67,
    ITEM_MOD_CR_UNUSED_10             = 68,
    ITEM_MOD_CR_UNUSED_11             = 69,
    ITEM_MOD_CR_UNUSED_12             = 70,
    ITEM_MOD_AGI_STR_INT              = 71,
    ITEM_MOD_AGI_STR                  = 72,
    ITEM_MOD_AGI_INT                  = 73,
    ITEM_MOD_STR_INT                  = 74
};

enum ItemSpelltriggerType
{
    ITEM_SPELLTRIGGER_ON_USE          = 0,                  // use after equip cooldown
    ITEM_SPELLTRIGGER_ON_EQUIP        = 1,
    ITEM_SPELLTRIGGER_CHANCE_ON_HIT   = 2,
    ITEM_SPELLTRIGGER_SOULSTONE       = 4,
    /*
     * ItemSpelltriggerType 5 might have changed on 2.4.3/3.0.3: Such auras
     * will be applied on item pickup and removed on item loss - maybe on the
     * other hand the item is destroyed if the aura is removed ("removed on
     * death" of spell 57348 makes me think so)
     */
    ITEM_SPELLTRIGGER_ON_NO_DELAY_USE = 5,                  // no equip cooldown
    ITEM_SPELLTRIGGER_LEARN_SPELL_ID  = 6                   // used in item_template.spell_2 with spell_id with SPELL_GENERIC_LEARN in spell_1
};

#define MAX_ITEM_SPELLTRIGGER           7

enum ItemBondingType : uint8
{
    NO_BIND                                     = 0,
    BIND_WHEN_PICKED_UP                         = 1,
    BIND_WHEN_EQUIPED                           = 2,
    BIND_WHEN_USE                               = 3,
    BIND_QUEST_ITEM                             = 4,
    BIND_QUEST_ITEM1                            = 5,         // not used in game

    MAX_BIND_TYPE
};

/* TODO
    // need to know cases when using item is not allowed in shapeshift
    ITEM_FLAG_USE_WHEN_SHAPESHIFTED    = 0x00800000, // Item can be used in shapeshift forms
*/

enum ItemFieldFlags
{
    ITEM_FLAG_SOULBOUND     = 0x00000001, // Item is soulbound and cannot be traded <<--
    ITEM_FLAG_TRANSLATED    = 0x00000002,
    ITEM_FLAG_UNLOCKED      = 0x00000004, // Item had lock but can be opened now
    ITEM_FLAG_WRAPPED       = 0x00000008, // Item is wrapped and contains another item
    ITEM_FLAG_DISABLE       = 0x00000010, // ?
    ITEM_FLAG_UNK3          = 0x00000020, // ?
    ITEM_FLAG_UNK4          = 0x00000040, // ?
    ITEM_FLAG_UNK5          = 0x00000080, // ?
    ITEM_FLAG_BOP_TRADEABLE = 0x00000100, // Allows trading soulbound items
    ITEM_FLAG_READABLE      = 0x00000200, // Opens text page when right clicked
    ITEM_FLAG_UNK6          = 0x00000400, // ?
    ITEM_FLAG_UNK7          = 0x00000800, // ?
    ITEM_FLAG_REFUNDABLE    = 0x00001000, // Item can be returned to vendor for its original cost (extended cost)
    ITEM_FLAG_UNK8          = 0x00002000, // ?
    ITEM_FLAG_UNK9          = 0x00004000, // ?
    ITEM_FLAG_UNK10         = 0x00008000, // ?
    ITEM_FLAG_UNK11         = 0x00010000, // ?
    ITEM_FLAG_UNK12         = 0x00020000, // ?
    ITEM_FLAG_UNK13         = 0x00040000, // ?
    ITEM_FLAG_CHILD         = 0x00080000,
    ITEM_FLAG_UNK15         = 0x00100000, // ?
    ITEM_FLAG_NEW_ITEM      = 0x00200000,
    ITEM_FLAG_UNK17         = 0x00400000, // ?
    ITEM_FLAG_UNK18         = 0x00800000, // ?
    ITEM_FLAG_UNK19         = 0x01000000, // ?
    ITEM_FLAG_UNK20         = 0x02000000, // ?
    ITEM_FLAG_UNK21         = 0x04000000, // ?
    ITEM_FLAG_UNK22         = 0x08000000, // ?
    ITEM_FLAG_UNK23         = 0x10000000, // ?
    ITEM_FLAG_UNK24         = 0x20000000, // ?
    ITEM_FLAG_UNK25         = 0x40000000, // ?
    ITEM_FLAG_UNK26         = 0x80000000, // ?
};

enum ItemFlags : uint32
{
    ITEM_FLAG_NO_PICKUP                         = 0x00000001,
    ITEM_FLAG_CONJURED                          = 0x00000002, // Conjured item
    ITEM_FLAG_HAS_LOOT                          = 0x00000004, // Item can be right clicked to open for loot
    ITEM_FLAG_HEROIC_TOOLTIP                    = 0x00000008, // Makes green "Heroic" text appear on item
    ITEM_FLAG_DEPRECATED                        = 0x00000010, // Cannot equip or use
    ITEM_FLAG_NO_USER_DESTROY                   = 0x00000020, // Item can not be destroyed, except by using spell (item can be reagent for spell)
    ITEM_FLAG_PLAYERCAST                        = 0x00000040,
    ITEM_FLAG_NO_EQUIP_COOLDOWN                 = 0x00000080, // No default 30 seconds cooldown when equipped
    ITEM_FLAG_MULTI_LOOT_QUEST                  = 0x00000100,
    ITEM_FLAG_IS_WRAPPER                        = 0x00000200, // Item can wrap other items
    ITEM_FLAG_USES_RESOURCES                    = 0x00000400,
    ITEM_FLAG_MULTI_DROP                        = 0x00000800, // Looting this item does not remove it from available loot
    ITEM_FLAG_ITEM_PURCHASE_RECORD              = 0x00001000, // Item can be returned to vendor for its original cost (extended cost)
    ITEM_FLAG_PETITION                          = 0x00002000, // Item is guild or arena charter
    ITEM_FLAG_HAS_TEXT                          = 0x00004000,
    ITEM_FLAG_NO_DISENCHANT                     = 0x00008000,
    ITEM_FLAG_REAL_DURATION                     = 0x00010000,
    ITEM_FLAG_NO_CREATOR                        = 0x00020000,
    ITEM_FLAG_IS_PROSPECTABLE                   = 0x00040000, // Item can be prospected
    ITEM_FLAG_UNIQUE_EQUIPPABLE                 = 0x00080000, // You can only equip one of these
    ITEM_FLAG_IGNORE_FOR_AURAS                  = 0x00100000,
    ITEM_FLAG_IGNORE_DEFAULT_ARENA_RESTRICTIONS = 0x00200000, // Item can be used during arena match
    ITEM_FLAG_NO_DURABILITY_LOSS                = 0x00400000,
    ITEM_FLAG_USE_WHEN_SHAPESHIFTED             = 0x00800000, // Item can be used in shapeshift forms
    ITEM_FLAG_HAS_QUEST_GLOW                    = 0x01000000,
    ITEM_FLAG_HIDE_UNUSABLE_RECIPE              = 0x02000000, // Profession recipes: can only be looted if you meet requirements and don't already know it
    ITEM_FLAG_NOT_USEABLE_IN_ARENA              = 0x04000000, // Item cannot be used in arena
    ITEM_FLAG_IS_BOUND_TO_ACCOUNT               = 0x08000000, // Item binds to account and can be sent only to your own characters
    ITEM_FLAG_NO_REAGENT_COST                   = 0x10000000, // Spell is cast ignoring reagents
    ITEM_FLAG_IS_MILLABLE                       = 0x20000000, // Item can be milled
    ITEM_FLAG_REPORT_TO_GUILD_CHAT              = 0x40000000,
    ITEM_FLAG_NO_PROGRESSIVE_LOOT               = 0x80000000
};

enum ItemFlags2 : uint32
{
    ITEM_FLAG2_FACTION_HORDE                            = 0x00000001,
    ITEM_FLAG2_FACTION_ALLIANCE                         = 0x00000002,
    ITEM_FLAG2_DONT_IGNORE_BUY_PRICE                    = 0x00000004, // when item uses extended cost, gold is also required
    ITEM_FLAG2_CLASSIFY_AS_CASTER                       = 0x00000008,
    ITEM_FLAG2_CLASSIFY_AS_PHYSICAL                     = 0x00000010,
    ITEM_FLAG2_EVERYONE_CAN_ROLL_NEED                   = 0x00000020,
    ITEM_FLAG2_NO_TRADE_BIND_ON_ACQUIRE                 = 0x00000040,
    ITEM_FLAG2_CAN_TRADE_BIND_ON_ACQUIRE                = 0x00000080,
    ITEM_FLAG2_CAN_ONLY_ROLL_GREED                      = 0x00000100,
    ITEM_FLAG2_CASTER_WEAPON                            = 0x00000200,
    ITEM_FLAG2_DELETE_ON_LOGIN                          = 0x00000400,
    ITEM_FLAG2_INTERNAL_ITEM                            = 0x00000800,
    ITEM_FLAG2_NO_VENDOR_VALUE                          = 0x00001000,
    ITEM_FLAG2_SHOW_BEFORE_DISCOVERED                   = 0x00002000,
    ITEM_FLAG2_OVERRIDE_GOLD_COST                       = 0x00004000,
    ITEM_FLAG2_IGNORE_DEFAULT_RATED_BG_RESTRICTIONS     = 0x00008000,
    ITEM_FLAG2_NOT_USABLE_IN_RATED_BG                   = 0x00010000,
    ITEM_FLAG2_BNET_ACCOUNT_TRADE_OK                    = 0x00020000,
    ITEM_FLAG2_CONFIRM_BEFORE_USE                       = 0x00040000,
    ITEM_FLAG2_REEVALUATE_BONDING_ON_TRANSFORM          = 0x00080000,
    ITEM_FLAG2_NO_TRANSFORM_ON_CHARGE_DEPLETION         = 0x00100000,
    ITEM_FLAG2_NO_ALTER_ITEM_VISUAL                     = 0x00200000,
    ITEM_FLAG2_NO_SOURCE_FOR_ITEM_VISUAL                = 0x00400000,
    ITEM_FLAG2_IGNORE_QUALITY_FOR_ITEM_VISUAL_SOURCE    = 0x00800000,
    ITEM_FLAG2_NO_DURABILITY                            = 0x01000000,
    ITEM_FLAG2_ROLE_TANK                                = 0x02000000,
    ITEM_FLAG2_ROLE_HEALER                              = 0x04000000,
    ITEM_FLAG2_ROLE_DAMAGE                              = 0x08000000,
    ITEM_FLAG2_CAN_DROP_IN_CHALLENGE_MODE               = 0x10000000,
    ITEM_FLAG2_NEVER_STACK_IN_LOOT_UI                   = 0x20000000,
    ITEM_FLAG2_DISENCHANT_TO_LOOT_TABLE                 = 0x40000000,
    ITEM_FLAG2_USED_IN_A_TRADESKILL                     = 0x80000000
};

enum ItemFlags3
{
    ITEM_FLAG3_DONT_DESTROY_ON_QUEST_ACCEPT                 = 0x00000001,
    ITEM_FLAG3_ITEM_CAN_BE_UPGRADED                         = 0x00000002,
    ITEM_FLAG3_UPGRADE_FROM_ITEM_OVERRIDES_DROP_UPGRADE     = 0x00000004,
    ITEM_FLAG3_ALWAYS_FFA_IN_LOOT                           = 0x00000008,
    ITEM_FLAG3_HIDE_UPGRADE_LEVELS_IF_NOT_UPGRADED          = 0x00000010,
    ITEM_FLAG3_UPDATE_INTERACTIONS                          = 0x00000020,
    ITEM_FLAG3_UPDATE_DOESNT_LEAVE_PROGRESSIVE_WIN_HISTORY  = 0x00000040,
    ITEM_FLAG3_IGNORE_ITEM_HISTORY_TRACKER                  = 0x00000080,
    ITEM_FLAG3_IGNORE_ITEM_LEVEL_CAP_IN_PVP                 = 0x00000100,
    ITEM_FLAG3_DISPLAY_AS_HEIRLOOM                          = 0x00000200, // Item appears as having heirloom quality ingame regardless of its real quality (does not affect stat calculation)
    ITEM_FLAG3_SKIP_USE_CHECK_ON_PICKUP                     = 0x00000400,
    ITEM_FLAG3_OBSOLETE                                     = 0x00000800,
    ITEM_FLAG3_DONT_DISPLAY_IN_GUILD_NEWS                   = 0x00001000, // Item is not included in the guild news panel
    ITEM_FLAG3_PVP_TOURNAMENT_GEAR                          = 0x00002000,
    ITEM_FLAG3_REQUIRES_STACK_CHANGE_LOG                    = 0x00004000,
    ITEM_FLAG3_UNUSED_FLAG                                  = 0x00008000,
    ITEM_FLAG3_HIDE_NAME_SUFFIX                             = 0x00010000,
    ITEM_FLAG3_PUSH_LOOT                                    = 0x00020000,
    ITEM_FLAG3_DONT_REPORT_LOOT_LOG_TO_PARTY                = 0x00040000,
    ITEM_FLAG3_ALWAYS_ALLOW_DUAL_WIELD                      = 0x00080000,
    ITEM_FLAG3_OBLITERATABLE                                = 0x00100000,
    ITEM_FLAG3_ACTS_AS_TRANSMOG_HIDDEN_VISUAL_OPTION        = 0x00200000,
    ITEM_FLAG3_EXPIRE_ON_WEEKLY_RESET                       = 0x00400000,
    ITEM_FLAG3_DOESNT_SHOW_UP_IN_TRANSMOG_UNTIL_COLLECTED   = 0x00800000,
    ITEM_FLAG3_CAN_STORE_ENCHANTS                           = 0x01000000
};

enum ItemFlagsCustom
{
    ITEM_FLAGS_CU_DURATION_REAL_TIME    = 0x0001,   // Item duration will tick even if player is offline
    ITEM_FLAGS_CU_IGNORE_QUEST_STATUS   = 0x0002,   // No quest status will be checked when this item drops
    ITEM_FLAGS_CU_FOLLOW_LOOT_RULES     = 0x0004,   // Item will always follow group/master/need before greed looting rules
};

enum ItemVendorType
{
    ITEM_VENDOR_TYPE_NONE     = 0,
    ITEM_VENDOR_TYPE_ITEM     = 1,
    ITEM_VENDOR_TYPE_CURRENCY = 2,
};

enum BAG_FAMILY_MASK
{
    BAG_FAMILY_MASK_NONE                      = 0x00000000,
    BAG_FAMILY_MASK_ARROWS                    = 0x00000001,
    BAG_FAMILY_MASK_BULLETS                   = 0x00000002,
    BAG_FAMILY_MASK_SOUL_SHARDS               = 0x00000004,
    BAG_FAMILY_MASK_LEATHERWORKING_SUPP       = 0x00000008,
    BAG_FAMILY_MASK_INSCRIPTION_SUPP          = 0x00000010,
    BAG_FAMILY_MASK_HERBS                     = 0x00000020,
    BAG_FAMILY_MASK_ENCHANTING_SUPP           = 0x00000040,
    BAG_FAMILY_MASK_ENGINEERING_SUPP          = 0x00000080,
    BAG_FAMILY_MASK_KEYS                      = 0x00000100,
    BAG_FAMILY_MASK_GEMS                      = 0x00000200,
    BAG_FAMILY_MASK_MINING_SUPP               = 0x00000400,
    BAG_FAMILY_MASK_SOULBOUND_EQUIPMENT       = 0x00000800,
    BAG_FAMILY_MASK_VANITY_PETS               = 0x00001000,
    BAG_FAMILY_MASK_CURRENCY_TOKENS           = 0x00002000,
    BAG_FAMILY_MASK_QUEST_ITEMS               = 0x00004000,
    BAG_FAMILY_MASK_FISHING_SUPP              = 0x00008000,
    BAG_FAMILY_MASK_COOKING_SUPP              = 0x00010000,
};

enum SocketColor
{
    SOCKET_COLOR_META                           = 0x00001,
    SOCKET_COLOR_RED                            = 0x00002,
    SOCKET_COLOR_YELLOW                         = 0x00004,
    SOCKET_COLOR_BLUE                           = 0x00008,
    SOCKET_COLOR_HYDRAULIC                      = 0x00010, // not used
    SOCKET_COLOR_COGWHEEL                       = 0x00020,
    SOCKET_COLOR_PRISMATIC                      = 0x0000E,
    SOCKET_COLOR_RELIC_IRON                     = 0x00040,
    SOCKET_COLOR_RELIC_BLOOD                    = 0x00080,
    SOCKET_COLOR_RELIC_SHADOW                   = 0x00100,
    SOCKET_COLOR_RELIC_FEL                      = 0x00200,
    SOCKET_COLOR_RELIC_ARCANE                   = 0x00400,
    SOCKET_COLOR_RELIC_FROST                    = 0x00800,
    SOCKET_COLOR_RELIC_FIRE                     = 0x01000,
    SOCKET_COLOR_RELIC_WATER                    = 0x02000,
    SOCKET_COLOR_RELIC_LIFE                     = 0x04000,
    SOCKET_COLOR_RELIC_WIND                     = 0x08000,
    SOCKET_COLOR_RELIC_HOLY                     = 0x10000
};

extern uint32 const SocketColorToGemTypeMask[19];

enum InventoryType
{
    INVTYPE_NON_EQUIP                           = 0,
    INVTYPE_HEAD                                = 1,
    INVTYPE_NECK                                = 2,
    INVTYPE_SHOULDERS                           = 3,
    INVTYPE_BODY                                = 4,
    INVTYPE_CHEST                               = 5,
    INVTYPE_WAIST                               = 6,
    INVTYPE_LEGS                                = 7,
    INVTYPE_FEET                                = 8,
    INVTYPE_WRISTS                              = 9,
    INVTYPE_HANDS                               = 10,
    INVTYPE_FINGER                              = 11,
    INVTYPE_TRINKET                             = 12,
    INVTYPE_WEAPON                              = 13,
    INVTYPE_SHIELD                              = 14,
    INVTYPE_RANGED                              = 15,
    INVTYPE_CLOAK                               = 16,
    INVTYPE_2HWEAPON                            = 17,
    INVTYPE_BAG                                 = 18,
    INVTYPE_TABARD                              = 19,
    INVTYPE_ROBE                                = 20,
    INVTYPE_WEAPONMAINHAND                      = 21,
    INVTYPE_WEAPONOFFHAND                       = 22,
    INVTYPE_HOLDABLE                            = 23,
    INVTYPE_AMMO                                = 24,
    INVTYPE_THROWN                              = 25,
    INVTYPE_RANGEDRIGHT                         = 26,
    INVTYPE_RELIC                               = 28
};

#define MAX_INVTYPE                               29

enum ItemClass
{
    ITEM_CLASS_CONSUMABLE                       = 0,
    ITEM_CLASS_CONTAINER                        = 1,
    ITEM_CLASS_WEAPON                           = 2,
    ITEM_CLASS_GEM                              = 3,
    ITEM_CLASS_ARMOR                            = 4,
    ITEM_CLASS_REAGENT                          = 5,
    ITEM_CLASS_PROJECTILE                       = 6,
    ITEM_CLASS_TRADE_GOODS                      = 7,
    ITEM_CLASS_ITEM_ENHANCEMENT                 = 8,
    ITEM_CLASS_RECIPE                           = 9,
    ITEM_CLASS_MONEY                            = 10, // OBSOLETE
    ITEM_CLASS_QUIVER                           = 11,
    ITEM_CLASS_QUEST                            = 12,
    ITEM_CLASS_KEY                              = 13,
    ITEM_CLASS_PERMANENT                        = 14, // OBSOLETE
    ITEM_CLASS_MISCELLANEOUS                    = 15,
    ITEM_CLASS_GLYPH                            = 16,
    ITEM_CLASS_BATTLE_PET                       = 17,
    ITEM_CLASS_WOW_TOKEN                        = 18,

    MAX_ITEM_CLASS
};

enum ItemSubclassConsumable
{
    ITEM_SUBCLASS_CONSUMABLE                    = 0,
    ITEM_SUBCLASS_POTION                        = 1,
    ITEM_SUBCLASS_ELIXIR                        = 2,
    ITEM_SUBCLASS_FLASK                         = 3,
    ITEM_SUBCLASS_SCROLL                        = 4,
    ITEM_SUBCLASS_FOOD_DRINK                    = 5,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT              = 6,
    ITEM_SUBCLASS_BANDAGE                       = 7,
    ITEM_SUBCLASS_CONSUMABLE_OTHER              = 8,
    ITEM_SUBCLASS_VANTUS_RUNE                   = 9,

    MAX_ITEM_SUBCLASS_CONSUMABLE
};

enum ItemSubclassContainer
{
    ITEM_SUBCLASS_CONTAINER                     = 0,
    ITEM_SUBCLASS_SOUL_CONTAINER                = 1,
    ITEM_SUBCLASS_HERB_CONTAINER                = 2,
    ITEM_SUBCLASS_ENCHANTING_CONTAINER          = 3,
    ITEM_SUBCLASS_ENGINEERING_CONTAINER         = 4,
    ITEM_SUBCLASS_GEM_CONTAINER                 = 5,
    ITEM_SUBCLASS_MINING_CONTAINER              = 6,
    ITEM_SUBCLASS_LEATHERWORKING_CONTAINER      = 7,
    ITEM_SUBCLASS_INSCRIPTION_CONTAINER         = 8,
    ITEM_SUBCLASS_TACKLE_CONTAINER              = 9,
    ITEM_SUBCLASS_COOKING_BAG                   = 10,

    MAX_ITEM_SUBCLASS_CONTAINER
};

enum ItemSubclassWeapon
{
    ITEM_SUBCLASS_WEAPON_AXE                    = 0,  // One-Handed Axes
    ITEM_SUBCLASS_WEAPON_AXE2                   = 1,  // Two-Handed Axes
    ITEM_SUBCLASS_WEAPON_BOW                    = 2,
    ITEM_SUBCLASS_WEAPON_GUN                    = 3,
    ITEM_SUBCLASS_WEAPON_MACE                   = 4,  // One-Handed Maces
    ITEM_SUBCLASS_WEAPON_MACE2                  = 5,  // Two-Handed Maces
    ITEM_SUBCLASS_WEAPON_POLEARM                = 6,
    ITEM_SUBCLASS_WEAPON_SWORD                  = 7,  // One-Handed Swords
    ITEM_SUBCLASS_WEAPON_SWORD2                 = 8,  // Two-Handed Swords
    ITEM_SUBCLASS_WEAPON_WARGLAIVES             = 9,
    ITEM_SUBCLASS_WEAPON_STAFF                  = 10,
    ITEM_SUBCLASS_WEAPON_EXOTIC                 = 11, // One-Handed Exotics
    ITEM_SUBCLASS_WEAPON_EXOTIC2                = 12, // Two-Handed Exotics
    ITEM_SUBCLASS_WEAPON_FIST_WEAPON            = 13,
    ITEM_SUBCLASS_WEAPON_MISCELLANEOUS          = 14,
    ITEM_SUBCLASS_WEAPON_DAGGER                 = 15,
    ITEM_SUBCLASS_WEAPON_THROWN                 = 16,
    ITEM_SUBCLASS_WEAPON_SPEAR                  = 17,
    ITEM_SUBCLASS_WEAPON_CROSSBOW               = 18,
    ITEM_SUBCLASS_WEAPON_WAND                   = 19,
    ITEM_SUBCLASS_WEAPON_FISHING_POLE           = 20,

    MAX_ITEM_SUBCLASS_WEAPON
};

#define ITEM_SUBCLASS_MASK_WEAPON_RANGED (\
    (1 << ITEM_SUBCLASS_WEAPON_BOW) | (1 << ITEM_SUBCLASS_WEAPON_GUN) |\
    (1 << ITEM_SUBCLASS_WEAPON_CROSSBOW))


enum ItemSubclassGem
{
    ITEM_SUBCLASS_GEM_INTELLECT                 = 0,
    ITEM_SUBCLASS_GEM_AGILITY                   = 1,
    ITEM_SUBCLASS_GEM_STRENGTH                  = 2,
    ITEM_SUBCLASS_GEM_STAMINA                   = 3,
    ITEM_SUBCLASS_GEM_SPIRIT                    = 4,
    ITEM_SUBCLASS_GEM_CRITICAL_STRIKE           = 5,
    ITEM_SUBCLASS_GEM_MASTERY                   = 6,
    ITEM_SUBCLASS_GEM_HASTE                     = 7,
    ITEM_SUBCLASS_GEM_VERSATILITY               = 8,
    ITEM_SUBCLASS_GEM_OTHER                     = 9,
    ITEM_SUBCLASS_GEM_MULTIPLE_STATS            = 10,
    ITEM_SUBCLASS_GEM_ARTIFACT_RELIC            = 11,

    MAX_ITEM_SUBCLASS_GEM
};

enum ItemSubclassArmor
{
    ITEM_SUBCLASS_ARMOR_MISCELLANEOUS           = 0,
    ITEM_SUBCLASS_ARMOR_CLOTH                   = 1,
    ITEM_SUBCLASS_ARMOR_LEATHER                 = 2,
    ITEM_SUBCLASS_ARMOR_MAIL                    = 3,
    ITEM_SUBCLASS_ARMOR_PLATE                   = 4,
    ITEM_SUBCLASS_ARMOR_COSMETIC                = 5,
    ITEM_SUBCLASS_ARMOR_SHIELD                  = 6,
    ITEM_SUBCLASS_ARMOR_LIBRAM                  = 7,
    ITEM_SUBCLASS_ARMOR_IDOL                    = 8,
    ITEM_SUBCLASS_ARMOR_TOTEM                   = 9,
    ITEM_SUBCLASS_ARMOR_SIGIL                   = 10,
    ITEM_SUBCLASS_ARMOR_RELIC                   = 11,

    MAX_ITEM_SUBCLASS_ARMOR
};

enum ItemSubclassReagent
{
    ITEM_SUBCLASS_REAGENT                       = 0,

    MAX_ITEM_SUBCLASS_REAGENT
};

enum ItemSubclassProjectile
{
    ITEM_SUBCLASS_WAND                          = 0, // OBSOLETE
    ITEM_SUBCLASS_BOLT                          = 1, // OBSOLETE
    ITEM_SUBCLASS_ARROW                         = 2,
    ITEM_SUBCLASS_BULLET                        = 3,
    ITEM_SUBCLASS_THROWN                        = 4, // OBSOLETE

    MAX_ITEM_SUBCLASS_PROJECTILE
};

enum ItemSubclassTradeGoods
{
    ITEM_SUBCLASS_TRADE_GOODS                   = 0,
    ITEM_SUBCLASS_PARTS                         = 1,
    ITEM_SUBCLASS_EXPLOSIVES                    = 2,
    ITEM_SUBCLASS_DEVICES                       = 3,
    ITEM_SUBCLASS_JEWELCRAFTING                 = 4,
    ITEM_SUBCLASS_CLOTH                         = 5,
    ITEM_SUBCLASS_LEATHER                       = 6,
    ITEM_SUBCLASS_METAL_STONE                   = 7,
    ITEM_SUBCLASS_MEAT                          = 8,
    ITEM_SUBCLASS_HERB                          = 9,
    ITEM_SUBCLASS_ELEMENTAL                     = 10,
    ITEM_SUBCLASS_TRADE_GOODS_OTHER             = 11,
    ITEM_SUBCLASS_ENCHANTING                    = 12,
    ITEM_SUBCLASS_MATERIAL                      = 13,
    ITEM_SUBCLASS_ENCHANTMENT                   = 14,
    ITEM_SUBCLASS_WEAPON_ENCHANTMENT            = 15,
    ITEM_SUBCLASS_INSCRIPTION                   = 16,
    ITEM_SUBCLASS_EXPLOSIVES_DEVICES            = 17,

    MAX_ITEM_SUBCLASS_TRADE_GOODS
};

enum ItemSubclassItemEnhancement
{
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_HEAD                 = 0,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_NECK                 = 1,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_SHOULDER             = 2,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_CLOAK                = 3,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_CHEST                = 4,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_WRIST                = 5,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_HANDS                = 6,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_WAIST                = 7,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_LEGS                 = 8,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_FEET                 = 9,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_FINGER               = 10,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_WEAPON               = 11,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_TWO_HANDED_WEAPON    = 12,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT_SHIELD_OFF_HAND      = 13,

    MAX_ITEM_SUBCLASS_ITEM_ENHANCEMENT
};

enum ItemSubclassGeneric
{
    ITEM_SUBCLASS_GENERIC                       = 0,  // OBSOLETE

    MAX_ITEM_SUBCLASS_GENERIC                   = 13 + 1
};

enum ItemSubclassRecipe
{
    ITEM_SUBCLASS_BOOK                          = 0,
    ITEM_SUBCLASS_LEATHERWORKING_PATTERN        = 1,
    ITEM_SUBCLASS_TAILORING_PATTERN             = 2,
    ITEM_SUBCLASS_ENGINEERING_SCHEMATIC         = 3,
    ITEM_SUBCLASS_BLACKSMITHING                 = 4,
    ITEM_SUBCLASS_COOKING_RECIPE                = 5,
    ITEM_SUBCLASS_ALCHEMY_RECIPE                = 6,
    ITEM_SUBCLASS_FIRST_AID_MANUAL              = 7,
    ITEM_SUBCLASS_ENCHANTING_FORMULA            = 8,
    ITEM_SUBCLASS_FISHING_MANUAL                = 9,
    ITEM_SUBCLASS_JEWELCRAFTING_RECIPE          = 10,
    ITEM_SUBCLASS_INSCRIPTION_TECHNIQUE         = 11,

    MAX_ITEM_SUBCLASS_RECIPE
};

enum ItemSubclassMoney
{
    ITEM_SUBCLASS_MONEY                         = 0,  // OBSOLETE

    MAX_ITEM_SUBCLASS_MONEY
};

enum ItemSubclassQuiver
{
    ITEM_SUBCLASS_QUIVER0                       = 0, // OBSOLETE
    ITEM_SUBCLASS_QUIVER1                       = 1, // OBSOLETE
    ITEM_SUBCLASS_QUIVER                        = 2,
    ITEM_SUBCLASS_AMMO_POUCH                    = 3,

    MAX_ITEM_SUBCLASS_QUIVER
};

enum ItemSubclassQuest
{
    ITEM_SUBCLASS_QUEST                         = 0,
    ITEM_SUBCLASS_QUEST_UNK3                    = 3, // 1 item (33604)
    ITEM_SUBCLASS_QUEST_UNK8                    = 8, // 2 items (37445, 49700)

    MAX_ITEM_SUBCLASS_QUEST
};

enum ItemSubclassKey
{
    ITEM_SUBCLASS_KEY                           = 0,
    ITEM_SUBCLASS_LOCKPICK                      = 1,

    MAX_ITEM_SUBCLASS_KEY
};

enum ItemSubclassPermanent
{
    ITEM_SUBCLASS_PERMANENT                     = 0,

    MAX_ITEM_SUBCLASS_PERMANENT
};

enum ItemSubclassJunk
{
    ITEM_SUBCLASS_JUNK                          = 0,
    ITEM_SUBCLASS_JUNK_REAGENT                  = 1,
    ITEM_SUBCLASS_JUNK_PET                      = 2,
    ITEM_SUBCLASS_JUNK_HOLIDAY                  = 3,
    ITEM_SUBCLASS_JUNK_OTHER                    = 4,
    ITEM_SUBCLASS_JUNK_MOUNT                    = 5,

    MAX_ITEM_SUBCLASS_JUNK
};

enum ItemSubclassGlyph
{
    ITEM_SUBCLASS_GLYPH_WARRIOR                 = 1,
    ITEM_SUBCLASS_GLYPH_PALADIN                 = 2,
    ITEM_SUBCLASS_GLYPH_HUNTER                  = 3,
    ITEM_SUBCLASS_GLYPH_ROGUE                   = 4,
    ITEM_SUBCLASS_GLYPH_PRIEST                  = 5,
    ITEM_SUBCLASS_GLYPH_DEATH_KNIGHT            = 6,
    ITEM_SUBCLASS_GLYPH_SHAMAN                  = 7,
    ITEM_SUBCLASS_GLYPH_MAGE                    = 8,
    ITEM_SUBCLASS_GLYPH_WARLOCK                 = 9,
    ITEM_SUBCLASS_GLYPH_MONK                    = 10,
    ITEM_SUBCLASS_GLYPH_DRUID                   = 11,

    MAX_ITEM_SUBCLASS_GLYPH                     = 16 + 1,
};

enum ItemSubclassBattlePet
{
    ITEM_SUBCLASS_BATTLE_PET_HUMANOID             = 0,
    ITEM_SUBCLASS_BATTLE_PET_DRAGONKIN            = 1,
    ITEM_SUBCLASS_BATTLE_PET_FLYING               = 2,
    ITEM_SUBCLASS_BATTLE_PET_UNDEAD               = 3,
    ITEM_SUBCLASS_BATTLE_PET_CRITTER              = 4,
    ITEM_SUBCLASS_BATTLE_PET_MAGIC                = 5,
    ITEM_SUBCLASS_BATTLE_PET_ELEMENTAL            = 6,
    ITEM_SUBCLASS_BATTLE_PET_BEAST                = 7,
    ITEM_SUBCLASS_BATTLE_PET_AQUATIC              = 8,
    ITEM_SUBCLASS_BATTLE_PET_MECHANICAL           = 9,

    MAX_ITEM_SUBCLASS_BATTLE_PET

};

#pragma pack(push, 1)

#define MAX_ITEM_PROTO_FLAGS 4
#define MAX_ITEM_PROTO_DAMAGES 2                            // changed in 3.1.0
#define MAX_ITEM_PROTO_SOCKETS 3
#define MAX_ITEM_PROTO_SPELLS  5
#define MAX_ITEM_PROTO_STATS  10

enum ItemLevelConstants : uint32
{
    MIN_ITEM_LEVEL = 1,
    MAX_ITEM_LEVEL = 1300
};

template<class T> class DBCStorage;
template<class T> class DB2Storage;
struct ItemEffectEntry;
struct ChrSpecializationEntry;

struct ItemTemplate
{
    ItemEntry const* BasicData;
    ItemSparseEntry const* ExtendedData;

    uint32 GetId() const { return BasicData->ID; }
    uint32 GetClass() const { return BasicData->ClassID; }
    uint32 GetSubClass() const { return BasicData->SubclassID; }
    InventoryType GetInventoryType() const { return InventoryType(BasicData->InventoryType); }

    uint8 GetSocketType(uint8 index) const { ASSERT(index < MAX_ITEM_PROTO_SOCKETS); return ExtendedData->SocketType[index]; }
    int32 GetItemStatType(uint32 index) const { ASSERT(index < MAX_ITEM_PROTO_STATS); return ExtendedData->StatModifierBonusStat[index]; }
    int32 GetItemStatValue(uint32 index) const { ASSERT(index < MAX_ITEM_PROTO_STATS); return ExtendedData->ItemStatValue[index]; }
    int32 GetItemStatAllocation(uint32 index) const { ASSERT(index < MAX_ITEM_PROTO_STATS); return ExtendedData->StatPercentEditor[index]; }
    float GetItemStatSocketCostMultiplier(uint32 index) const { ASSERT(index < MAX_ITEM_PROTO_STATS); return ExtendedData->StatPercentageOfSocket[index]; }

    uint32 GetFlags() const { return ExtendedData->Flags[0]; }
    uint32 GetFlags2() const { return ExtendedData->Flags[1]; }
    uint32 GetFlags3() const { return ExtendedData->Flags[2]; }
    uint32 GetFlags4() const { return ExtendedData->Flags[3]; }
    uint32 GetQuality() const { return ExtendedData->OverallQualityID; }
    float GetPriceRandomValue() const { return ExtendedData->PriceRandomValue; }
    float GetPriceVariance() const { return ExtendedData->PriceVariance; }
    uint8 GetRequiredExpansion() const { return ExtendedData->ExpansionID; }
    float GetRangedModRange() const { return ExtendedData->ItemRange; }
    float GetQualityModifier() const { return ExtendedData->QualityModifier; }
    float  GetDmgVariance() const { return ExtendedData->DmgVariance; }
    LocalizedString* GetName() const { return ExtendedData->Display; }
    uint32 GetBaseItemLevel() const { return ItemLevel; }
    uint32 GetDelay() const { return ExtendedData->ItemDelay; }
    uint32 GetScalingStatDistribution() const { return ExtendedData->ScalingStatDistributionID; }
    int32 GetBaseRequiredLevel() const { return ExtendedData->RequiredLevel; }
    uint32 GetSocketBonus() const { return ExtendedData->SocketMatch_enchantment_id; }
    uint32 GetGemProperties() const { return ExtendedData->GemProperties; }
    uint32 GetArtifactID() const { return ExtendedData->ArtifactID; }
    uint8 GetExpansion() const { return ExtendedData->ExpansionID; }
    int32 GetRandomSelect() const { return ExtendedData->RandomSelect; }
    int32 GetItemRandomSuffixGroupID() const { return ExtendedData->ItemRandomSuffixGroupID; }
    uint32 GetSpellWeightCategory() const { return ExtendedData->SpellWeightCategory; }
    uint32 GetSpellWeight() const { return ExtendedData->SpellWeight; }
    int32 GetBuyPrice() const { return ExtendedData->BuyPrice; }
    uint32 GetSellPrice() const { return ExtendedData->SellPrice; }
    uint32 GetItemNameDescriptionID() const { return ExtendedData->ItemNameDescriptionID; }
    uint32 GetRequiredHoliday() const { return ExtendedData->RequiredHoliday; }
    uint32 GetLimitCategory() const { return ExtendedData->LimitCategory; }
    uint32 GetPageMaterialID() const { return ExtendedData->PageMaterialID; }
    uint32 GetDamageDamageType() const { return ExtendedData->DamageDamageType; }
    uint32 GetContainerSlots() const { return ExtendedData->ContainerSlots; }
    uint32 GetPageID() const { return ExtendedData->PageID; }
    int32 GetMaterial() const { return ExtendedData->Material; }
    uint32 GetMinReputation() const { return ExtendedData->MinReputation; }
    uint32 GetMinFactionID() const { return ExtendedData->MinFactionID; }
    uint32 GetRequiredPVPMedal() const { return ExtendedData->RequiredPVPMedal; }
    uint32 GetRequiredPVPRank() const { return ExtendedData->RequiredPVPRank; }
    uint32 GetRequiredAbility() const { return ExtendedData->RequiredAbility; }
    uint32 GetTotemCategoryID() const { return ExtendedData->TotemCategoryID; }
    uint32 GetBagFamily() const { return ExtendedData->BagFamily; }
    int32 GetMaxCount() const { return ExtendedData->MaxCount; }
    int32 GetStackable() const { return ExtendedData->Stackable; }
    uint32 GetLockID() const { return ExtendedData->LockID; }
    uint32 GetBonding() const { return ExtendedData->Bonding; }
    uint32 GetSheatheType() const { return ExtendedData->SheatheType; }
    uint32 GetItemSet() const { return ExtendedData->ItemSet; }
    uint32 GetRequiredSkill() const { return ExtendedData->RequiredSkill; }
    uint32 GetRequiredSkillRank() const { return ExtendedData->RequiredSkillRank; }
    uint32 GetLanguageID() const { return ExtendedData->LanguageID; }
    uint32 GetStartQuestID() const { return ExtendedData->StartQuestID; }
    uint32 GetArea() const { return ExtendedData->ZoneBound; }
    uint32 GetMap() const { return ExtendedData->InstanceBound; }
    uint32 GetDurationInInventory() const { return ExtendedData->DurationInInventory; }

    uint32 ItemLevel;
    uint32 VendorStackCount;
    uint32 AllowableClass;
    int64  AllowableRace;
    uint32 MaxDurability;
    std::vector<ItemEffectEntry const*> Effects;

    // extra fields, not part of db2 files
    float  SpellPPMRate;
    uint32 ScriptId;
    uint32 FoodType;
    uint32 MinMoneyLoot;
    uint32 MaxMoneyLoot;
    bool   ItemSpecExist;
    uint32 FlagsCu;
    std::bitset<MAX_CLASSES * MAX_SPECIALIZATIONS> Specializations[3];  // one set for 1-40 level range and another for 41-109 and one for 110
    uint32 ItemSpecClassMask;
    bool IsMultiClass;
    bool IsSingleClass;

    bool AllowToLooter() const;

    uint32 GetBaseArmor() const;
    void GetBaseDamage(float& minDamage, float& maxDamage) const;

    // helpers
    bool CanChangeEquipStateInCombat() const;

    bool IsCurrencyToken() const;
    bool IsNotAppearInGuildNews() const;

    uint32 GetMaxStackSize() const;

    bool IsPotion() const;
    bool IsVellum() const;
    bool IsConjuredConsumable() const;
    bool IsCraftingReagent() const;
    bool IsOtherDrops() const;
    bool IsRecipe() const;
    bool IsNotNeedCheck() const;

    bool IsRangedWeapon() const;
    bool IsOneHanded() const;
    bool IsTwoHandedWeapon() const;

    uint32 GetArmor(uint32 itemLevel) const;
    void GetDamage(uint32 itemLevel, float& minDamage, float& maxDamage) const;
    bool IsUsableBySpecialization(uint32 specId, uint8 level) const;
    static std::size_t CalculateItemSpecBit(ChrSpecializationEntry const* spec);

    bool HasStats() const;
    bool IsLegionLegendary() const;
    bool IsLegendaryLoot() const;
    bool IsLegionLegendaryToken() const;
    bool CanWarforged() const;
    bool HasEnchantment() const;
};

// Benchmarked: Faster than std::map (insert/find)
typedef std::unordered_map<uint32, ItemTemplate> ItemTemplateContainer;

#pragma pack(pop)

#endif
