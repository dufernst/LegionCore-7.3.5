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

#ifndef TRINITY_LOOTMGR_H
#define TRINITY_LOOTMGR_H

#include "ItemEnchantmentMgr.h"
#include "SharedDefines.h"
#include "ConditionMgr.h"
#include "HashFuctor.h"

namespace WorldPackets
{
    namespace Loot
    {
        class LootResponse;
    }

    namespace Item
    {
        struct ItemInstance;
    }
}

struct Loot;
class Challenge;

typedef cds::container::FeldmanHashSet< cds::gc::HP, ObjectGuid, ObjectGuidHashAccessor > GuidHashSet;

enum RollType
{
    ROLL_PASS         = 0,
    ROLL_NEED         = 1,
    ROLL_GREED        = 2,
    ROLL_DISENCHANT   = 3,
    MAX_ROLL_TYPE     = 4
};

enum RollMask
{
    ROLL_FLAG_TYPE_PASS         = 0x01,
    ROLL_FLAG_TYPE_NEED         = 0x02,
    ROLL_FLAG_TYPE_GREED        = 0x04,
    ROLL_FLAG_TYPE_DISENCHANT   = 0x08,

    ROLL_ALL_TYPE_NO_DISENCHANT = 0x07,
    ROLL_ALL_TYPE_MASK          = 0x0F
};

#define MAX_NR_LOOT_ITEMS 16
// note: the client cannot show more than 16 items total
#define MAX_NR_QUEST_ITEMS 32
// unrelated to the number of quest items shown, just for reserve

enum LootMethod
{
    FREE_FOR_ALL      = 0,
    ROUND_ROBIN       = 1,
    MASTER_LOOT       = 2,
    GROUP_LOOT        = 3,
    NEED_BEFORE_GREED = 4,
    PERSONAL_LOOT     = 5
};

enum PermissionTypes
{
    ALL_PERMISSION              = 0,
    GROUP_PERMISSION            = 1,
    MASTER_PERMISSION           = 2,
    RESTRICTED_PERMISSION       = 3,
    ROUND_ROBIN_PERMISSION      = 4,
    OWNER_PERMISSION            = 5,
    NONE_PERMISSION             = 6
};

// type of Loot Item in Loot View
enum LootItemUIType
{
    LOOT_ITEM_UI_NORMAL            = 0,                        // player can loot the item. //checked. 6.1.2
    LOOT_ITEM_UI_ROLL              = 1,                        // roll is ongoing. player cannot loot. //checked. 6.1.2
    LOOT_ITEM_UI_LOCKED            = 2,                        // item is shown in red. player cannot loot.
    LOOT_ITEM_UI_MASTER            = 3,                        // item can only be distributed by group loot master. //checked. 6.1.2
    LOOT_ITEM_UI_OWNER             = 4,                        // ignore binding confirmation and etc, for single player looting
};

enum LootSlotType
{
    LOOT_SLOT_TYPE_NONE     = 0,
    LOOT_SLOT_TYPE_CURRENCY = 1, //6.1.2
    LOOT_SLOT_TYPE_MONEY    = 2,
    LOOT_SLOT_TYPE_ITEM     = 3, //6.1.2
};

enum LootItemType
{
    LOOT_ITEM_TYPE_ITEM     = 0,
    LOOT_ITEM_TYPE_CURRENCY = 1,
};

enum LootType
{
    LOOT_NONE                   = 0,

    LOOT_CORPSE                 = 1,
    LOOT_PICKPOCKETING          = 2,
    LOOT_FISHING                = 3,
    LOOT_DISENCHANTING          = 4,
                                                            // ignored always by client
    LOOT_SKINNING               = 6,
    LOOT_PROSPECTING            = 7,
    LOOT_MILLING                = 8,

    LOOT_FISHINGHOLE            = 20,                       // unsupported by client, sending LOOT_FISHING instead
    LOOT_INSIGNIA               = 21,                       // unsupported by client, sending LOOT_CORPSE instead
    LOOT_FISHING_JUNK           = 22                        // unsupported by client, sending LOOT_FISHING instead
};

enum LootError
{
    LOOT_ERROR_DIDNT_KILL               = 0,    // You don't have permission to loot that corpse.
    LOOT_ERROR_TOO_FAR                  = 4,    // You are too far away to loot that corpse.
    LOOT_ERROR_BAD_FACING               = 5,    // You must be facing the corpse to loot it.
    LOOT_ERROR_LOCKED                   = 6,    // Someone is already looting that corpse.
    LOOT_ERROR_NOTSTANDING              = 8,    // You need to be standing up to loot something!
    LOOT_ERROR_STUNNED                  = 9,    // You can't loot anything while stunned!
    LOOT_ERROR_PLAYER_NOT_FOUND         = 10,   // Player not found
    LOOT_ERROR_PLAY_TIME_EXCEEDED       = 11,   // Maximum play time exceeded
    LOOT_ERROR_MASTER_INV_FULL          = 12,   // That player's inventory is full
    LOOT_ERROR_MASTER_UNIQUE_ITEM       = 13,   // Player has too many of that item already
    LOOT_ERROR_MASTER_OTHER             = 14,   // Can't assign item to that player
    LOOT_ERROR_ALREADY_PICKPOCKETED     = 15,   // Your target has already had its pockets picked
    LOOT_ERROR_NOT_WHILE_SHAPESHIFTED   = 16,   // You can't do that while shapeshifted.
    LOOT_ERROR_NO_LOOT                  = 17
};

class Player;
class LootStore;
class ConditionMgr;

struct LootStoreItem
{
    std::list<Condition*>  conditions;                               // additional loot condition
    float   chance;                                         // always positive, chance to drop for both quest and non-quest items, chance to be used for refs
    uint32  itemid;                                         // id of the item
    int32   mincountOrRef;                                  // mincount for drop items (positive) or minus referenced TemplateleId (negative)
    uint32  maxcount;                                       // max drop count for the item (mincountOrRef positive) or Ref multiplicator (mincountOrRef negative)
    uint16  lootmode;
    uint8   type;                                           // 0 = item, 1 = currency
    uint8   group :7;
    uint8   ClassificationMask{};
    bool    shared{};
    bool    needs_quest :1;                                 // quest drop (negative ChanceOrQuestChance in DB)

    // Constructor, converting ChanceOrQuestChance -> (chance, needs_quest)
    // displayid is filled in IsValid() which must be called after
    LootStoreItem(uint32 _itemid, uint8 _type, float _chanceOrQuestChance, uint16 _lootmode, uint8 _group, int32 _mincountOrRef, uint32 _maxcount);

    bool Roll(bool rate, bool isDungeon = false, bool isZoneLoot = false) const;                             // Checks if the entry takes it's chance (at loot generation)
    bool IsValid(LootStore const& store, uint32 entry) const;
                                                            // Checks correctness of values
};

struct CurrencyLoot
{
    uint32 Entry;
    uint8 Type;
    uint32 CurrencyId;
    uint32 CurrencyAmount;
    uint32 currencyMaxAmount;
    uint8 lootmode;
    float chance;

    CurrencyLoot(uint32 _entry, uint8 _type, uint32 _CurrencyId, uint32 _CurrencyAmount, uint32 _CurrencyMaxAmount, uint8 _lootmode, float _chance);
};

struct LootItem
{
    struct
    {
        uint32 ItemID = 0;
        uint32 RandomPropertiesSeed = 0;
        ItemRandomEnchantmentId RandomPropertiesID;
        int32 UpgradeID = 0;

        struct
        {
            uint8 Context = 0;
            std::vector<uint32> BonusListIDs;
        } ItemBonus;

    } item;

    uint8   type;                                           // 0 = item, 1 = currency
    ItemQualities quality;
    std::list<Condition*> conditions;                               // additional loot condition
    GuidSet allowedGUIDs;
    uint32  count;
    bool    currency          : 1;
    bool    is_looted         : 1;
    bool    is_blocked        : 1;
    bool    freeforall        : 1;                          // free for all
    bool    is_underthreshold : 1;
    bool    is_counted        : 1;
    bool    needs_quest       : 1;                          // quest drop
    bool    follow_loot_rules : 1;
    Loot* m_loot = nullptr;
    uint8 LootListID = 0;

    LootItem();

    // Constructor, copies most fields from LootStoreItem, generates random count and random suffixes/properties
    // Should be called for non-reference LootStoreItem entries only (mincountOrRef > 0)
    explicit LootItem(LootStoreItem const& li, Loot* loot);
    void init(Loot* loot);

    void InitItem(uint32 itemID, uint32 count, Loot* loot, bool isCurrency = false);

    // Basic checks for player/item compatibility - if false no chance to see the item in the loot
    bool AllowedForPlayer(Player const* player) const;

    void AddAllowedLooter(Player const* player);
    const GuidSet & GetAllowedLooters() const { return allowedGUIDs; }
};

struct QuestItem
{
    uint8   index;                                          // position in quest_items;
    bool    is_looted;

    QuestItem();

    QuestItem(uint8 _index, bool _islooted = false);
};

struct Loot;
class LootTemplate;

typedef std::vector<QuestItem> QuestItemList;
typedef std::vector<LootItem> LootItemList;
typedef std::map<ObjectGuid::LowType, QuestItemList*> QuestItemMap;
typedef std::vector<LootStoreItem> LootStoreItemList;
typedef std::unordered_map<uint32, LootTemplate*> LootTemplateMap;

typedef std::set<uint32> LootIdSet;

class LootStore
{
    public:
        explicit LootStore(std::string name, std::string entryName, bool ratesAllowed, std::string addColum);

        virtual ~LootStore() { Clear(); }

        void Verify() const;

        uint32 LoadAndCollectLootIds(LootIdSet& ids_set);
        void CheckLootRefs(LootIdSet* ref_set = nullptr) const; // check existence reference and remove it from ref_set
        void ReportUnusedIds(LootIdSet const& ids_set) const;
        void ReportNotExistedId(uint32 id) const;

        bool HaveLootFor(uint32 loot_id) const { return m_LootTemplates.find(loot_id) != m_LootTemplates.end(); }
        bool HaveQuestLootFor(uint32 loot_id) const;
        bool HaveQuestLootForPlayer(uint32 loot_id, Player* player) const;

        LootTemplate const* GetLootFor(uint32 loot_id) const;
        void ResetConditions();
        LootTemplate* GetLootForConditionFill(uint32 loot_id);

        char const* GetName() const { return m_name.c_str(); }
        char const* GetEntryName() const { return m_entryName.c_str(); }
        bool IsRatesAllowed() const { return m_ratesAllowed; }
        char const* GetAddColum() const { return m_addColum.c_str(); }
    protected:
        uint32 LoadLootTable();
        void Clear();
    private:
        LootTemplateMap m_LootTemplates;
        std::string m_name;
        std::string m_entryName;
        std::string m_addColum;
        bool m_ratesAllowed;
};

class LootTemplate
{
    class LootGroup;                                       // A set of loot definitions for items (refs are not allowed inside)
    typedef std::vector<LootGroup> LootGroups;

    public:
        // Adds an entry to the group (at loading stage)
        void AddEntry(LootStoreItem& item);
        // Rolls for every item in the template and adds the rolled items the the loot
        void Process(Loot& loot, bool rate, uint8 groupId = 0) const;
        void ProcessOploteChest(Loot& loot) const;
        void ProcessChallengeChest(Loot& loot, uint32 lootId, Challenge* _challenge) const;
        void ProcessItemLoot(Loot& loot) const;
        void CopyConditions(ConditionList conditions);
        void ProcessWorld(Loot& loot, bool rate, bool ignore = false) const;
        void ProcessLuck(Loot& loot, uint32 entry, bool ignore = false) const;
        void ProcessBossLoot(Loot& loot) const;
        void ProcessRareOrGoLoot(Loot& loot) const;

        // True if template includes at least 1 quest drop entry
        bool HasQuestDrop(LootTemplateMap const& store, uint8 groupId = 0) const;
        // True if template includes at least 1 quest drop for an active quest of the player
        bool HasQuestDropForPlayer(LootTemplateMap const& store, Player const* player, uint8 groupId = 0) const;

        // Checks integrity of the template
        void Verify(LootStore const& store, uint32 Id) const;
        void CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const;
        bool addConditionItem(Condition* cond);
        bool isReference(uint32 id);
        bool CheckItemCondition(Player const* player, uint32 itemId, uint8 type) const;

        bool _isZoneLoot = false;

    private:
        LootStoreItemList Entries;                          // not grouped only
        LootGroups        Groups;                           // groups have own (optimised) processing, grouped entries go there
        LootGroups        AutoGroups;                       // auto groups have own (optimised) processing, grouped entries go there
        LootGroups        PersonalGroups;
};

struct Loot
{
    QuestItemMap const& GetPlayerCurrencies() const { return PlayerCurrencies; }
    QuestItemMap const& GetPlayerQuestItems() const { return PlayerQuestItems; }
    QuestItemMap const& GetPlayerFFAItems() const { return PlayerFFAItems; }
    QuestItemMap const& GetPlayerNonQuestNonFFANonCurrencyConditionalItems() const { return PlayerNonQuestNonFFANonCurrencyConditionalItems; }


    std::vector<LootItem> items;
    std::vector<LootItem> quest_items;
    uint32 gold;
    uint8 unlootedCount;
    ObjectGuid roundRobinPlayer;                            // GUID of the player having the Round-Robin ownership for the loot. If 0, round robin owner has released.
    LootType loot_type;                                     // required for achievement system

    uint32 objEntry;
    ObjectGuid objGuid;
    ObjectGuid m_guid;
    ObjectGuid LootSourceGuid;                              ///< Source guid of loot (gameobject, creature)
    uint8 objType;
    uint32 chance;
    uint32 shipmentBuildingType = 0;                       // of garrison
    bool personal = false;
    bool bonusLoot = false;
    bool isBoss = false;
    std::atomic<bool> isClear;
    bool isOpen = false;
    bool isOnlyQuest = false;
    bool isRareOrGo = false;
    bool isRareNext = false;
    bool _isEmissaryLoot = false;
    bool _IsPvPLoot = false;
    bool _isTokenLoot = false;
    bool _isItemLoot = false;
    bool _isLegendaryLoot = false;
    bool _isEventLoot = false;
    bool _specCheck = true;
    float rateLegendary;
    uint32 dungeonEncounterID = 0;
    bool _NoneRaidOrScenarioDungeonLoot = false;
    uint32 _itemContext;
    uint32 _needLevel;
    uint32 _challengeLevel;
    int8 _ExpansionID;
    uint8 _DifficultyID;
    uint32 _DifficultyMask;
    uint8 _ClassificationMask;

    explicit Loot(uint32 _gold = 0);
    ~Loot();

    ObjectGuid const& GetGUID() const { return m_guid; }
    void SetGUID(ObjectGuid const& guid) { m_guid = guid; }

    void SetSource(ObjectGuid const& source) { LootSourceGuid = source; }

    LootItem* GetLootItem(uint32 entry);
    void AddOrReplaceItem(uint32 itemID, uint32 _count, bool isRes = false, bool update = false);

    void clear();
    bool empty() const { return items.empty() && gold == 0; }
    bool isLooted() const { return gold == 0 && unlootedCount == 0; }

    void NotifyItemRemoved(uint8 lootIndex);
    void NotifyQuestItemRemoved(uint8 questIndex);
    void NotifyMoneyRemoved(uint64);
    void AddLooter(ObjectGuid GUID);
    void RemoveLooter(ObjectGuid GUID);
    void GenerateLootGuid(ObjectGuid __objGuid);
    uint32 ReplaceLootID(uint32 lootId);
    bool IsEmissaryLoot(uint32 lootId, WorldObject const* lootFrom);
    bool IsPvPLoot(uint32 lootId, WorldObject const* lootFrom);

    void generateMoneyLoot(uint32 minAmount, uint32 maxAmount, bool isDungeon = false);
    bool FillLoot(uint32 lootId, LootStore const& store, Player* lootOwner, bool noGroup, bool noEmptyError = false, WorldObject const* lootFrom = nullptr);
    void AutoStoreItems(bool isGO = false);

    // Inserts the item into the loot (called by LootTemplate processors)
    void AddItem(LootStoreItem const& item, std::vector<uint32> const& bonusListIDs = std::vector<uint32>());

    // Basic checks for player/item compatibility - if false no chance to see the item in the loot
    bool AllowedForPlayer(Player const* player, uint32 ItemID, uint8 type, bool needs_quest, LootItem* lootItem = nullptr) const;

    LootItem const* GetItemInSlot(uint32 lootSlot) const;
    LootItem* LootItemInSlot(uint32 lootslot, Player* player, QuestItem** qitem = nullptr, QuestItem** ffaitem = nullptr, QuestItem** conditem = nullptr, QuestItem** currency = nullptr);
    uint32 GetMaxSlotInLootFor(Player* player) const;
    bool hasItemFor(Player* player) const;
    bool hasOverThresholdItem() const;
    Player const* GetLootOwner() const { return m_lootOwner; }
    void SetLootOwner(Player* p) { m_lootOwner = p;  }
    uint32 GetItemContext() const { return _itemContext; }

    // Builds data for SMSG_LOOT_RESPONSE
    void BuildLootResponse(WorldPackets::Loot::LootResponse& packet, Player* viewer, PermissionTypes permission = ALL_PERMISSION, ItemQualities t = ITEM_QUALITY_POOR) const;

    QuestItemList* FillCurrencyLoot(Player* player);
    void AddLegendaryItemToDrop();
    void AddLegendaryItem();

    bool IsLastEncounter();

private:
    void FillNotNormalLootFor(Player* player, bool presentAtLooting);
    void FillPersonalLootFor(Player* lootOwner);
    QuestItemList* FillFFALoot(Player* player);
    QuestItemList* FillQuestLoot(Player* player);
    QuestItemList* FillNonQuestNonFFAConditionalLoot(Player* player, bool presentAtLooting);

    GuidHashSet PlayersLooting;
    QuestItemMap PlayerCurrencies;
    QuestItemMap PlayerQuestItems;
    QuestItemMap PlayerFFAItems;
    QuestItemMap PlayerNonQuestNonFFANonCurrencyConditionalItems;

    Player* m_lootOwner;
    int32 _levelBonus;
};

typedef std::map<ObjectGuid, Loot> PersonalLootMap;

extern LootStore LootTemplates_Creature;
extern LootStore LootTemplates_Fishing;
extern LootStore LootTemplates_Gameobject;
extern LootStore LootTemplates_Item;
extern LootStore LootTemplates_Mail;
extern LootStore LootTemplates_Milling;
extern LootStore LootTemplates_Pickpocketing;
extern LootStore LootTemplates_Reference;
extern LootStore LootTemplates_Skinning;
extern LootStore LootTemplates_Disenchant;
extern LootStore LootTemplates_Prospecting;
extern LootStore LootTemplates_Spell;
extern LootStore LootTemplates_World;
extern LootStore LootTemplates_Luck;
extern LootStore LootTemplates_Zone;

void LoadLootTemplates_Creature();
void LoadLootTemplates_Fishing();
void LoadLootTemplates_Gameobject();
void LoadLootTemplates_Item();
void LoadLootTemplates_Mail();
void LoadLootTemplates_Milling();
void LoadLootTemplates_Pickpocketing();
void LoadLootTemplates_Skinning();
void LoadLootTemplates_Disenchant();
void LoadLootTemplates_Prospecting();

void LoadLootTemplates_Spell();
void LoadLootTemplates_World();
void LoadLootTemplates_Reference();
void LoadLootTemplates_Luck();
void LoadLootTemplates_Zone();

inline void LoadLootTables()
{
    LoadLootTemplates_Creature();
    LoadLootTemplates_Fishing();
    LoadLootTemplates_Gameobject();
    LoadLootTemplates_Item();
    LoadLootTemplates_Mail();
    LoadLootTemplates_Milling();
    LoadLootTemplates_Pickpocketing();
    LoadLootTemplates_Skinning();
    LoadLootTemplates_Disenchant();
    LoadLootTemplates_Prospecting();
    LoadLootTemplates_Spell();
    LoadLootTemplates_World();
    LoadLootTemplates_Luck();
    LoadLootTemplates_Zone();

    LoadLootTemplates_Reference();
}

class LootMgr
{
        LootMgr() {}
        ~LootMgr() {}

    public:
        static LootMgr* instance()
        {
            static LootMgr instance;
            return &instance;
        }
        typedef std::map<ObjectGuid, Loot*> LootsMap;

        Loot* GetLoot(ObjectGuid const& guid);
        void AddLoot(Loot* loot);
        void RemoveLoot(ObjectGuid const& guid);

    protected:
        LootsMap m_Loots;
        std::recursive_mutex m_LootsLock;
};

#define sLootMgr LootMgr::instance()

#endif
