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

#ifndef ITEM_H
#define ITEM_H

#include "Common.h"
#include "Object.h"
#include "LootMgr.h"
#include "ItemTemplate.h"
#include "ItemEnchantmentMgr.h"

class SpellInfo;
class Bag;
class Unit;
class BattlePetMgr;
struct ItemSetSpellEntry;

namespace WorldPackets
{
    namespace Item
    {
        struct ItemInstance;
    }
}

namespace ItemBonus
{
    namespace Chances
    {
        enum
        {
            Warforged       = 20,
            NextChance      = 55,
        };
    }

    namespace Bonuses
    {
        enum
        {
            TitanforgedEpic          = 3337,
            TitanforgedBlue          = 3338,
            WarforgedEpic            = 3336,
            WarforgedBlue            = 3339,
            EpicBonus                = 665, // +15
        };
    }

    namespace LimitLevel
    {
        enum
        {
            MaxItemLevel    = 985,
        };
    }
}

struct ItemSetEffect
{
    uint32 ItemSetID;
    uint32 EquippedItemCount;
    std::unordered_set<ItemSetSpellEntry const*> SetBonuses;
};

enum InventoryResult : uint8
{
    EQUIP_ERR_OK                                           = 0,
    EQUIP_ERR_CANT_EQUIP_LEVEL_I                           = 1,  // You must reach level %d to use that item.
    EQUIP_ERR_CANT_EQUIP_SKILL                             = 2,  // You aren't skilled enough to use that item.
    EQUIP_ERR_WRONG_SLOT                                   = 3,  // That item does not go in that slot.
    EQUIP_ERR_BAG_FULL                                     = 4,  // That bag is full.
    EQUIP_ERR_BAG_IN_BAG                                   = 5,  // Can't put non-empty bags in other bags.
    EQUIP_ERR_TRADE_EQUIPPED_BAG                           = 6,  // You can't trade equipped bags.
    EQUIP_ERR_AMMO_ONLY                                    = 7,  // Only ammo can go there.
    EQUIP_ERR_PROFICIENCY_NEEDED                           = 8,  // You do not have the required proficiency for that item.
    EQUIP_ERR_NO_SLOT_AVAILABLE                            = 9,  // No equipment slot is available for that item.
    EQUIP_ERR_CANT_EQUIP_EVER                              = 10, // You can never use that item.
    EQUIP_ERR_CANT_EQUIP_EVER_2                            = 11, // You can never use that item.
    EQUIP_ERR_NO_SLOT_AVAILABLE_2                          = 12, // No equipment slot is available for that item.
    EQUIP_ERR_2HANDED_EQUIPPED                             = 13, // Cannot equip that with a two-handed weapon.
    EQUIP_ERR_2HSKILLNOTFOUND                              = 14, // You cannot dual-wield
    EQUIP_ERR_WRONG_BAG_TYPE                               = 15, // That item doesn't go in that container.
    EQUIP_ERR_WRONG_BAG_TYPE_2                             = 16, // That item doesn't go in that container.
    EQUIP_ERR_ITEM_MAX_COUNT                               = 17, // You can't carry any more of those items.
    EQUIP_ERR_NO_SLOT_AVAILABLE_3                          = 18, // No equipment slot is available for that item.
    EQUIP_ERR_CANT_STACK                                   = 19, // This item cannot stack.
    EQUIP_ERR_NOT_EQUIPPABLE                               = 20, // This item cannot be equipped.
    EQUIP_ERR_CANT_SWAP                                    = 21, // These items can't be swapped.
    EQUIP_ERR_SLOT_EMPTY                                   = 22, // That slot is empty.
    EQUIP_ERR_ITEM_NOT_FOUND                               = 23, // The item was not found.
    EQUIP_ERR_DROP_BOUND_ITEM                              = 24, // You can't drop a soulbound item.
    EQUIP_ERR_OUT_OF_RANGE                                 = 25, // Out of range.
    EQUIP_ERR_TOO_FEW_TO_SPLIT                             = 26, // Tried to split more than number in stack.
    EQUIP_ERR_SPLIT_FAILED                                 = 27, // Couldn't split those items.
    EQUIP_ERR_SPELL_FAILED_REAGENTS_GENERIC                = 28, // Missing reagent
    EQUIP_ERR_NOT_ENOUGH_MONEY                             = 29, // You don't have enough money.
    EQUIP_ERR_NOT_A_BAG                                    = 30, // Not a bag.
    EQUIP_ERR_DESTROY_NONEMPTY_BAG                         = 31, // You can only do that with empty bags.
    EQUIP_ERR_NOT_OWNER                                    = 32, // You don't own that item.
    EQUIP_ERR_ONLY_ONE_QUIVER                              = 33, // You can only equip one quiver.
    EQUIP_ERR_NO_BANK_SLOT                                 = 34, // You must purchase that bag slot first
    EQUIP_ERR_NO_BANK_HERE                                 = 35, // You are too far away from a bank.
    EQUIP_ERR_ITEM_LOCKED                                  = 36, // Item is locked.
    EQUIP_ERR_GENERIC_STUNNED                              = 37, // You are stunned
    EQUIP_ERR_PLAYER_DEAD                                  = 38, // You can't do that when you're dead.
    EQUIP_ERR_CLIENT_LOCKED_OUT                            = 39, // You can't do that right now.
    EQUIP_ERR_INTERNAL_BAG_ERROR                           = 40, // Internal Bag Error
    EQUIP_ERR_ONLY_ONE_BOLT                                = 41, // You can only equip one quiver.
    EQUIP_ERR_ONLY_ONE_AMMO                                = 42, // You can only equip one ammo pouch.
    EQUIP_ERR_CANT_WRAP_STACKABLE                          = 43, // Stackable items can't be wrapped.
    EQUIP_ERR_CANT_WRAP_EQUIPPED                           = 44, // Equipped items can't be wrapped.
    EQUIP_ERR_CANT_WRAP_WRAPPED                            = 45, // Wrapped items can't be wrapped.
    EQUIP_ERR_CANT_WRAP_BOUND                              = 46, // Bound items can't be wrapped.
    EQUIP_ERR_CANT_WRAP_UNIQUE                             = 47, // Unique items can't be wrapped.
    EQUIP_ERR_CANT_WRAP_BAGS                               = 48, // Bags can't be wrapped.
    EQUIP_ERR_LOOT_GONE                                    = 49, // Already looted
    EQUIP_ERR_INV_FULL                                     = 50, // Inventory is full.
    EQUIP_ERR_BANK_FULL                                    = 51, // Your bank is full
    EQUIP_ERR_VENDOR_SOLD_OUT                              = 52, // That item is currently sold out.
    EQUIP_ERR_BAG_FULL_2                                   = 53, // That bag is full.
    EQUIP_ERR_ITEM_NOT_FOUND_2                             = 54, // The item was not found.
    EQUIP_ERR_CANT_STACK_2                                 = 55, // This item cannot stack.
    EQUIP_ERR_BAG_FULL_3                                   = 56, // That bag is full.
    EQUIP_ERR_VENDOR_SOLD_OUT_2                            = 57, // That item is currently sold out.
    EQUIP_ERR_OBJECT_IS_BUSY                               = 58, // That object is busy.
    EQUIP_ERR_CANT_BE_DISENCHANTED                         = 59,
    EQUIP_ERR_NOT_IN_COMBAT                                = 60, // You can't do that while in combat
    EQUIP_ERR_NOT_WHILE_DISARMED                           = 61, // You can't do that while disarmed
    EQUIP_ERR_BAG_FULL_4                                   = 62, // That bag is full.
    EQUIP_ERR_CANT_EQUIP_RANK                              = 63, // You don't have the required rank for that item
    EQUIP_ERR_CANT_EQUIP_REPUTATION                        = 64, // You don't have the required reputation for that item
    EQUIP_ERR_TOO_MANY_SPECIAL_BAGS                        = 65, // You cannot equip another bag of that type
    EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW                      = 66, // You can't loot that item now.
    EQUIP_ERR_ITEM_UNIQUE_EQUIPPABLE                       = 67, // You cannot equip more than one of those.
    EQUIP_ERR_VENDOR_MISSING_TURNINS                       = 68, // You do not have the required items for that purchase
    EQUIP_ERR_NOT_ENOUGH_HONOR_POINTS                      = 69, // You don't have enough honor points
    EQUIP_ERR_NOT_ENOUGH_ARENA_POINTS                      = 70, // You don't have enough arena points
    EQUIP_ERR_ITEM_MAX_COUNT_SOCKETED                      = 71, // You have the maximum number of those gems in your inventory or socketed into items.
    EQUIP_ERR_MAIL_BOUND_ITEM                              = 72, // You can't mail soulbound items.
    EQUIP_ERR_INTERNAL_BAG_ERROR_2                         = 73, // Internal Bag Error
    EQUIP_ERR_BAG_FULL_5                                   = 74, // That bag is full.
    EQUIP_ERR_ITEM_MAX_COUNT_EQUIPPED_SOCKETED             = 75, // You have the maximum number of those gems socketed into equipped items.
    EQUIP_ERR_ITEM_UNIQUE_EQUIPPABLE_SOCKETED              = 76, // You cannot socket more than one of those gems into a single item.
    EQUIP_ERR_TOO_MUCH_GOLD                                = 77, // At gold limit
    EQUIP_ERR_NOT_DURING_ARENA_MATCH                       = 78, // You can't do that while in an arena match
    EQUIP_ERR_TRADE_BOUND_ITEM                             = 79, // You can't trade a soulbound item.
    EQUIP_ERR_CANT_EQUIP_RATING                            = 80, // You don't have the personal, team, or battleground rating required to buy that item
    EQUIP_ERR_EVENT_AUTOEQUIP_BIND_CONFIRM                 = 81,
    EQUIP_ERR_NOT_SAME_ACCOUNT                             = 82, // Account-bound items can only be given to your own characters.
    EQUIP_ERR_NO_OUTPUT                                    = 83,
    EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED_IS    = 84, // You can only carry %d %s
    EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED_IS = 85, // You can only equip %d |4item:items in the %s category
    EQUIP_ERR_SCALING_STAT_ITEM_LEVEL_EXCEEDED             = 86, // Your level is too high to use that item
    EQUIP_ERR_PURCHASE_LEVEL_TOO_LOW                       = 87, // You must reach level %d to purchase that item.
    EQUIP_ERR_CANT_EQUIP_NEED_TALENT                       = 88, // You do not have the required talent to equip that.
    EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED_IS = 89, // You can only equip %d |4item:items in the %s category
    EQUIP_ERR_SHAPESHIFT_FORM_CANNOT_EQUIP                 = 90, // Cannot equip item in this form
    EQUIP_ERR_ITEM_INVENTORY_FULL_SATCHEL                  = 91, // Your inventory is full. Your satchel has been delivered to your mailbox.
    EQUIP_ERR_SCALING_STAT_ITEM_LEVEL_TOO_LOW              = 92, // Your level is too low to use that item
    EQUIP_ERR_CANT_BUY_QUANTITY                            = 93, // You can't buy the specified quantity of that item.
    EQUIP_ERR_ITEM_IS_BATTLE_PAY_LOCKED                    = 94, // Your purchased item is still waiting to be unlocked
    EQUIP_ERR_REAGENT_BANK_FULL                            = 95, // Your reagent bank is full
    EQUIP_ERR_REAGENT_BANK_LOCKED                          = 96,
    EQUIP_ERR_WRONG_BAG_TYPE_3                             = 97,
    EQUIP_ERR_CANT_USE_ITEM                                = 98, // You can't use that item.
    EQUIP_ERR_CANT_BE_OBLITERATED                          = 99, // You can't obliterate that item
    EQUIP_ERR_GUILD_BANK_CONJURED_ITEM                     = 100,// You cannot store conjured items in the guild bank
    EQUIP_ERR_CANT_DO_THAT_RIGHT_NOW                       = 101,// You can't do that right now.
};

enum BuyResult
{
    BUY_ERR_CANT_FIND_ITEM                      = 0,
    BUY_ERR_ITEM_ALREADY_SOLD                   = 1,
    BUY_ERR_NOT_ENOUGHT_MONEY                   = 2,
    BUY_ERR_SELLER_DONT_LIKE_YOU                = 4,
    BUY_ERR_DISTANCE_TOO_FAR                    = 5,
    BUY_ERR_ITEM_SOLD_OUT                       = 7,
    BUY_ERR_CANT_CARRY_MORE                     = 8,
    BUY_ERR_RANK_REQUIRE                        = 11,
    BUY_ERR_REPUTATION_REQUIRE                  = 12
};

enum SellResult
{
    SELL_ERR_OK                                  = 0,
    SELL_ERR_CANT_FIND_ITEM                      = 1,
    SELL_ERR_CANT_SELL_ITEM                      = 2,       // merchant doesn't like that item
    SELL_ERR_VENDOR_HATES_YOU                    = 3,       // merchant doesn't like you
    SELL_ERR_YOU_DONT_OWN_THAT_ITEM              = 4,       // you don't own that item
    SELL_ERR_UNK                                 = 5,       // nothing appears...
    SELL_ERR_ONLY_EMPTY_BAG                      = 6,       // can only do with empty bags
    SELL_ERR_VENDOR_DOES_NOT_BUY                 = 7,
    SELL_ERR_REPAIR_DURABILITY                   = 8,
    SELL_ERR_INTERNAL_BAG_ERROR                  = 9,
};

// -1 from client enchantment slot number
enum EnchantmentSlot : uint8
{
    PERM_ENCHANTMENT_SLOT           = 0,
    TEMP_ENCHANTMENT_SLOT           = 1,
    SOCK_ENCHANTMENT_SLOT           = 2,
    SOCK_ENCHANTMENT_SLOT_2         = 3,
    SOCK_ENCHANTMENT_SLOT_3         = 4,
    BONUS_ENCHANTMENT_SLOT          = 5,
    PRISMATIC_ENCHANTMENT_SLOT      = 6,                    // added at apply special permanent enchantment
    USE_ENCHANTMENT_SLOT            = 7,

    MAX_INSPECTED_ENCHANTMENT_SLOT  = 8,

    PROP_ENCHANTMENT_SLOT_0         = 8,                    // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_1         = 9,                    // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_2         = 10,                   // used with RandomSuffix and RandomProperty
    PROP_ENCHANTMENT_SLOT_3         = 11,                   // used with RandomProperty
    PROP_ENCHANTMENT_SLOT_4         = 12,                   // used with RandomProperty
    MAX_ENCHANTMENT_SLOT            = 13
};

#define MAX_VISIBLE_ITEM_OFFSET       2                     // 2 fields per visible item (entry+enchantment)

#define MAX_GEM_SOCKETS               MAX_ITEM_PROTO_SOCKETS// (BONUS_ENCHANTMENT_SLOT-SOCK_ENCHANTMENT_SLOT) and item proto size, equal value expected

enum EnchantmentOffset
{
    ENCHANTMENT_ID_OFFSET       = 0,
    ENCHANTMENT_DURATION_OFFSET = 1,
    ENCHANTMENT_CHARGES_OFFSET  = 2                         // now here not only charges, but something new in wotlk
};

#define MAX_ENCHANTMENT_OFFSET    3

enum EnchantmentSlotMask
{
    ENCHANTMENT_CAN_SOULBOUND  = 0x01,
    ENCHANTMENT_UNK1           = 0x02,
    ENCHANTMENT_UNK2           = 0x04,
    ENCHANTMENT_UNK3                    = 0x08,
    ENCHANTMENT_COLLECTABLE             = 0x100,
    ENCHANTMENT_HIDE_IF_NOT_COLLECTED   = 0x200,
};

enum ItemUpdateState
{
    ITEM_UNCHANGED                               = 0,
    ITEM_CHANGED                                 = 1,
    ITEM_NEW                                     = 2,
    ITEM_REMOVED                                 = 3
};

enum ItemModifier
{
    ITEM_MODIFIER_TRANSMOG_APPEARANCE_ALL_SPECS         = 0,
    ITEM_MODIFIER_TRANSMOG_APPEARANCE_SPEC_1            = 1,
    ITEM_MODIFIER_UPGRADE_ID                            = 2,
    ITEM_MODIFIER_BATTLE_PET_SPECIES_ID                 = 3,
    ITEM_MODIFIER_BATTLE_PET_BREED_DATA                 = 4, // (breedId) | (breedQuality << 24)
    ITEM_MODIFIER_BATTLE_PET_LEVEL                      = 5,
    ITEM_MODIFIER_BATTLE_PET_DISPLAY_ID                 = 6,
    ITEM_MODIFIER_ENCHANT_ILLUSION_ALL_SPECS            = 7,
    ITEM_MODIFIER_ARTIFACT_APPEARANCE_ID                = 8,
    ITEM_MODIFIER_SCALING_STAT_DISTRIBUTION_FIXED_LEVEL = 9,
    ITEM_MODIFIER_ENCHANT_ILLUSION_SPEC_1               = 10,
    ITEM_MODIFIER_TRANSMOG_APPEARANCE_SPEC_2            = 11,
    ITEM_MODIFIER_ENCHANT_ILLUSION_SPEC_2               = 12,
    ITEM_MODIFIER_TRANSMOG_APPEARANCE_SPEC_3            = 13,
    ITEM_MODIFIER_ENCHANT_ILLUSION_SPEC_3               = 14,
    ITEM_MODIFIER_TRANSMOG_APPEARANCE_SPEC_4            = 15,
    ITEM_MODIFIER_ENCHANT_ILLUSION_SPEC_4               = 16,
    ITEM_MODIFIER_CHALLENGE_ID                          = 17,
    ITEM_MODIFIER_CHALLENGE_KEYSTONE_LEVEL              = 18,
    ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_1         = 19,
    ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_2         = 20,
    ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_3         = 21,
    ITEM_MODIFIER_CHALLENGE_KEYSTONE_IS_CHARGED         = 22,
    ITEM_MODIFIER_ARTIFACT_KNOWLEDGE_LEVEL              = 23,
    ITEM_MODIFIER_ARTIFACT_TIER                         = 24,

    MAX_ITEM_MODIFIERS
};

enum ArtifactPowerFlag : uint8
{
    ARTIFACT_POWER_FLAG_GOLD                        = 0x01,
    ARTIFACT_POWER_FLAG_NO_LINK_REQUIRED            = 0x02,
    ARTIFACT_POWER_FLAG_FINAL                       = 0x04,
    ARTIFACT_POWER_FLAG_SCALES_WITH_NUM_POWERS      = 0x08,
    ARTIFACT_POWER_FLAG_DONT_COUNT_FIRST_BONUS_RANK = 0x10,
    ARTIFACT_POWER_FLAG_HAS_RANK                    = 0x20,
    ARTIFACT_POWER_FLAG_RELIC_TALENT                = 0x40,
};

enum ArtifactCategory : uint8
{
    ARTIFACT_CATEGORY_CLASS                        = 1,
    ARTIFACT_CATEGORY_FISH                         = 2,

};

enum Curves
{
    CURVE_ID_ARTIFACT_RELIC_ITEM_LEVEL_BONUS = 1718
};

#define MAX_ITEM_SPELLS 5

bool ItemCanGoIntoBag(ItemTemplate const* proto, ItemTemplate const* pBagProto);

extern ItemModifier const AppearanceModifierSlotBySpec[MAX_SPECIALIZATIONS];
extern ItemModifier const IllusionModifierSlotBySpec[MAX_SPECIALIZATIONS];
extern int32 const ItemTransmogrificationSlots[MAX_INVTYPE];

struct BonusData
{
    int32 ItemStatType[MAX_ITEM_PROTO_STATS];
    int32 ItemStatValue[MAX_ITEM_PROTO_STATS];
    int32 StatPercentEditor[MAX_ITEM_PROTO_STATS];
    float StatPercentageOfSocket[MAX_ITEM_PROTO_STATS];
    uint32 GemItemLevelBonus[MAX_ITEM_PROTO_SOCKETS];
    int32 GemRelicType[MAX_ITEM_PROTO_SOCKETS];
    uint16 GemRelicRankBonus[MAX_ITEM_PROTO_SOCKETS];
    uint32 SocketColor[MAX_ITEM_PROTO_SOCKETS];
    uint32 DisplayToastMethod[2];
    ItemBondingType Bonding;
    uint32 AppearanceModID;
    float RepairCostMultiplier;
    uint32 ScalingStatDistribution;
    uint32 SandboxScalingId;
    uint32 Quality;
    int32 ItemLevelBonus;
    int32 RequiredLevel;
    int32 RelicType;
    int32 RequiredLevelOverride;
    int32 DisenchantLootId;
    int32 DescriptionID;
    bool HasSturdiness;
    bool HasFixedLevel;

    void Initialize(ItemTemplate const* proto);
    void Initialize(WorldPackets::Item::ItemInstance const& itemInstance);
    void AddBonus(uint32 type, int32 const (&values)[3]);

private:
    struct
    {
        int32 AppearanceModPriority;
        int32 ScalingStatDistributionPriority;
        bool HasQualityBonus;
    } _state{};
};

#pragma pack(push, 1)
struct ItemDynamicFieldArtifactPowers
{
    uint32 ArtifactPowerId = 0;
    uint8 PurchasedRank = 0;
    uint8 CurrentRankWithBonus = 0;
    uint16 Padding = 0;
};

struct ItemDynamicFieldGems
{
    uint32 ItemId;
    uint16 BonusListIDs[16];
    uint8 Context;
    uint8 Padding[3];
};

struct SocketTier
{
    SocketTier() = default;
    SocketTier(uint16 first, uint16 second)
        : FirstSpell(first)
        , SecondSpell(second)
    {}

    uint16 FirstSpell = 0;
    uint16 SecondSpell = 0;
};
struct ItemSocketInfo
{
    uint32 unk1 = 0;
    uint32 socketIndex = 0;
    uint32 firstTier = 0;
    uint32 secondTier =0;
    uint32 thirdTier = 0;
    uint32 additionalThirdTier = 0;
    
};
#pragma pack(pop)

class Item : public Object
{
    public:
        static Item* CreateItem(uint32 item, uint32 count, Player const* player = nullptr);
        Item* CloneItem(uint32 count, Player const* player = nullptr) const;

        Item();
        ~Item();

        virtual bool Create(ObjectGuid::LowType const& guidlow, uint32 itemid, Player const* owner);

        ItemTemplate const* GetTemplate() const;

        void RemoveFromWorld() override;

        ObjectGuid GetOwnerGUID() const;

        void SetOwnerGUID(ObjectGuid guid);
        Player* GetOwner()const;

        ItemBondingType GetBonding() const;
        bool IsSturdiness() const;
        void SetBinding(bool val);
        bool IsSoulBound() const;

        bool IsBoundAccountWide() const;
        bool IsBindedNotWith(Player const* player) const;
        bool IsBoundByEnchant() const;
        virtual void SaveToDB(SQLTransaction& trans);
        virtual bool LoadFromDB(ObjectGuid::LowType const& guid, ObjectGuid const& owner_guid, Field* fields, uint32 entry, uint8 oLevel = 0);
        void LoadArtifactData(Player* owner, std::vector<ItemDynamicFieldArtifactPowers>& powers);  // must be called after LoadFromDB to have gems (relics) initialized

        void AddBonuses(uint32 bonusListID);
        void ApplyItemChildEquipment(Player* owner, bool apply);

        static void DeleteFromDB(SQLTransaction& trans, ObjectGuid::LowType itemGuid);
        virtual void DeleteFromDB(SQLTransaction& trans);
        static void DeleteFromInventoryDB(SQLTransaction& trans, ObjectGuid::LowType itemGuid);
        void DeleteFromInventoryDB(SQLTransaction& trans);
        void SaveRefundDataToDB();
        void DeleteRefundDataFromDB(SQLTransaction* trans);

        Bag* ToBag();

        const Bag* ToBag() const;

        bool IsEquipable() const;

        bool IsSuitableForItemLevelCalulcation(bool includeOffHand) const;

        bool IsLocked() const;

        bool IsBag() const;

        bool IsCurrencyToken() const;
        bool IsNotEmptyBag() const;
        bool IsBroken() const;

        bool IsDisable() const;

        bool CantBeUse() const;
        bool CanBeTraded(bool mail = false, bool trade = false) const;
        void SetInTrade(bool b = true);

        bool IsInTrade() const;

        void SetInUse(bool u = true);

        bool IsInUse() const;

        bool HasEnchantRequiredSkill(const Player* player) const;
        uint32 GetEnchantRequiredLevel() const;

        bool IsFitToSpellRequirements(SpellInfo const* spellInfo) const;
        bool IsLimitedToAnotherMapOrZone(uint32 cur_mapId, uint32 cur_zoneId) const;
        bool GemsFitSockets() const;

        uint32 GetCount() const;

        void SetCount(uint32 value);

        uint32 GetMaxStackCount() const;
        uint8 GetGemCountWithID(uint32 GemID) const;
        uint8 GetGemCountWithLimitCategory(uint32 limitCategory) const;
        InventoryResult CanBeMergedPartlyWith(ItemTemplate const* proto) const;
        DynamicFieldStructuredView<ItemDynamicFieldGems> GetGems() const;
        std::map<uint8, ItemSocketInfo> GetArtifactSockets() const;
        ItemDynamicFieldGems const* GetGem(uint16 slot) const;
        void SetGem(uint16 slot, ItemDynamicFieldGems const* gem, uint32 gemScalingLevel);
        void CreateSocketTalents(uint8 socketIndex);
        void AddOrRemoveSocketTalent(uint8 talentIndex, bool add, uint8 socketIndex);

        uint8 GetSlot() const;

        Bag* GetContainer();
        uint8 GetBagSlot() const;
        void SetSlot(uint8 slot);

        uint16 GetPos() const;

        void SetContainer(Bag* container);

        bool IsInBag() const;
        bool IsEquipped() const;

        uint32 GetSkill();

        int32 GetItemRandomPropertyId() const;
        uint32 GetItemSuffixFactor() const;
        void SetItemRandomProperties(ItemRandomEnchantmentId const& randomPropId);
        void UpdateItemSuffixFactor();
        static ItemRandomEnchantmentId GenerateItemRandomPropertyId(uint32 item_id, uint32 spec_id = 0);
        ItemRandomEnchantmentId GetItemRandomEnchantmentId() const;
        void SetEnchantment(EnchantmentSlot slot, uint32 id, uint32 duration, uint32 charges, ObjectGuid = ObjectGuid::Empty);
        void SetEnchantmentDuration(EnchantmentSlot slot, uint32 duration, Player* owner);
        void SetEnchantmentCharges(EnchantmentSlot slot, uint32 charges);
        static void SendEnchantmentLog(Player* player, ObjectGuid const& Caster, ObjectGuid const& item, uint32 ItemID, uint32 SpellID, EnchantmentSlot slot);
        static void SendItemEnchantTimeUpdate(Player* player, ObjectGuid const& Itemguid, uint32 slot, uint32 Duration);
        void ClearEnchantment(EnchantmentSlot slot);
        uint32 GetEnchantmentId(EnchantmentSlot slot) const;
        uint32 GetEnchantmentDuration(EnchantmentSlot slot) const;
        uint32 GetEnchantmentCharges(EnchantmentSlot slot) const;

        std::string const& GetText() const;
        void SetText(std::string const& text);

        void SendTimeUpdate(Player* owner);
        void UpdateDuration(Player* owner, uint32 diff);

        // spell charges (signed but stored as unsigned)
        int32 GetSpellCharges(uint8 index/*0..5*/ = 0) const;
        void SetSpellCharges(uint8 index/*0..5*/, int32 value);

        Loot loot;
        bool m_lootGenerated;

        // Update States
        ItemUpdateState GetState() const;
        void SetState(ItemUpdateState state, Player* forplayer = nullptr);
        void AddToUpdateQueueOf(Player* player);
        void RemoveFromUpdateQueueOf(Player* player);
        bool IsInUpdateQueue() const;
        uint16 GetQueuePos() const;
        void FSetState(ItemUpdateState state);               // forced

        bool hasQuest(uint32 quest_id) const override;
        bool hasInvolvedQuest(uint32 /*quest_id*/) const override;
        bool HasStats() const;
        static bool HasStats(WorldPackets::Item::ItemInstance const& itemInstance, BonusData const* bonus);
        bool IsPotion() const;
        bool IsVellum() const;
        bool IsConjuredConsumable() const;
        bool IsCraftingReagent() const;
        bool IsRangedWeapon() const;

        BonusData const* GetBonus() const;
        uint32 GetQuality() const;
        uint32 GetItemLevel(uint8 ownerLevel = 0, bool isPvP = false) const;
        int32 GetRequiredLevel() const;
        int32 GetItemStatType(uint32 index) const;
        int32 GetItemStatValue(uint32 index, bool isPvP = false) const;
        uint8 GetSocketColor(uint8 index) const;
        uint32 GetAppearanceModId() const;
        void SetAppearanceModId(uint32 appearanceModId);
        uint32 GetArmor() const;
        void GetDamage(float& minDamage, float& maxDamage) const;
        uint32 GetDisplayId(Player const* owner) const;
        ItemModifiedAppearanceEntry const* GetItemModifiedAppearance() const;
        float GetRepairCostMultiplier() const;
        uint32 GetScalingStatDistribution() const;
        uint8 GetDisplayToastMethod(uint8 value = 0) const;
        int32 GetDisenchantLootID() const;
        ItemDisenchantLootEntry const* GetDisenchantLoot(Player const* owner);
        ItemDisenchantLootEntry const* GetDisenchantLoot(ItemTemplate const* itemTemplate, uint32 quality, uint32 itemLevel);
        bool CanBeDisenchanted();
        void SetFixedLevel(uint8 level);

        // Item Refund system
        void SetNotRefundable(Player* owner, bool changestate = true, SQLTransaction* trans = nullptr);
        void SetRefundRecipient(ObjectGuid const& pGuidLow);
        void SetPaidMoney(uint64 money);
        void SetPaidExtendedCost(uint32 iece);
        ObjectGuid GetRefundRecipient();
        uint64 GetPaidMoney();
        uint32 GetPaidExtendedCost();
        void SetDonateItem(bool apply);
        bool GetDonateItem() const; // can't be traded, selled or add on auction

        void UpdatePlayedTime(Player* owner);
        uint32 GetPlayedTime();
        bool IsRefundExpired();

        // Soulbound trade system
        void SetSoulboundTradeable(GuidSet const& allowedLooters);
        void ClearSoulboundTradeable(Player* currentOwner);
        bool CheckSoulboundTradeExpire();

        void BuildUpdate(UpdateDataMapType&) override;
        void BuildDynamicValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;
        void AddToObjectUpdateIfNeeded() override;

        uint32 GetScriptId() const;

        bool IsValidTransmogrificationTarget() const;
        //uint32 GetSpecialPrice(uint32 minimumPrice = 10000) const;
        uint32 GetBuyPrice(bool &success);
        uint32 GetSellPrice();

        uint32 GetVisibleEntry(Player const* owner) const;
        uint16 GetVisibleAppearanceModId(Player const* owner) const;
        uint32 GetVisibleEnchantmentId(Player const* owner) const;
        uint16 GetVisibleItemVisual(Player const* owner) const;


        uint32 GetModifier(ItemModifier modifier) const;
        void SetModifier(ItemModifier modifier, uint32 value);

        void SetScaleIlvl(int64 ilvl);
        uint32 GetScaleIlvl() const;
        void SetArtIlvlBonus(uint32 ilvl);
        uint32 GetArtIlvlBonus() const;

        static bool CanTransmogrifyItemWithItem(Item const* item, ItemModifiedAppearanceEntry const* itemModifiedAppearance);

        ObjectGuid GetChildItem() const;
        void SetChildItem(ObjectGuid childItem);

        void ApplyArtifactPowerEnchantmentBonuses(EnchantmentSlot slot, uint32 enchantId, bool apply, Player* owner);
        void HandleAllBonusTraits(Player* owner, bool apply = true);

        void InitArtifactPowers(uint8 artifactId);
        void InitArtifactsTier(uint8 artifactId);
        void ActivateFishArtifact(uint8 artifactId);
        void CopyArtifactDataFromParent(Item* parent);

        DynamicFieldStructuredView<ItemDynamicFieldArtifactPowers> GetArtifactPowers() const;
        ItemDynamicFieldArtifactPowers const* GetArtifactPower(uint32 artifactPowerId) const;
        void SetArtifactPower(ItemDynamicFieldArtifactPowers const* artifactPower, bool createIfMissing = false);

        void GiveArtifactXp(uint64 amount, Item* sourceItem, uint32 artifactCategoryId);
        uint32 GetTotalPurchasedArtifactPowers() const;

        void SetOwnerLevel(uint8 level);
        uint8 GetOwnerLevel() const;
        void UpgradeLegendary();

        uint8 protected_remove_state = 0;   // 1 - protection for remove enabled, 2 - delay remove item.

        uint16 dungeonEncounterID;
        uint32 createdTime;

        BonusData _bonusData;

        int32 DescriptionID;
        uint32 GetDescriptionID();

        bool GetModsApplied() const { return m_modsApplied;  }
        void SetModsApplied(bool apply) { m_modsApplied = apply; }

    private:
        std::string m_text;
        uint8 m_slot;
        Bag* m_container;
        ItemUpdateState uState;
        int16 uQueuePos;
        bool mb_in_trade;                                   // true if item is currently in trade-window
        bool m_in_use;
        time_t m_lastPlayedTimeUpdate;
        ObjectGuid m_refundRecipient;
        uint64 m_paidMoney;
        uint32 m_paidExtendedCost;
        bool DonateItem = false;
        GuidSet allowedGUIDs;
        ItemRandomEnchantmentId m_randomEnchantment;
        uint32 m_scaleLvl;
        uint32 m_artIlvlBonus;
        ObjectGuid m_childItem;
        std::vector<int16> m_artifactPowerIdToIndex;
        std::array<uint32, MAX_ITEM_PROTO_SOCKETS> m_gemScalingLevels;
        std::map<uint32, uint8> bSavedBonusTraits;
        uint16 m_artifactPowerCount;
        uint8 m_ownerLevel;
        bool m_modsApplied = false;
};

#endif
