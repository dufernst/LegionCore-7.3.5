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

#include "BattlegroundMgr.h"
#include "LootMgr.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Util.h"
#include "SharedDefines.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "Group.h"
#include "LootPackets.h"
#include "ItemPackets.h"
#include "Garrison.h"
#include "GarrisonMgr.h"
#include "ScenarioMgr.h"
#include "Challenge.h"
#include "ChallengeMgr.h"
#include "SpellAuraEffects.h"
#include "DatabaseEnv.h"

static Rates const qualityToRate[MAX_ITEM_QUALITY] =
{
    RATE_DROP_ITEM_POOR,                                    // ITEM_QUALITY_POOR
    RATE_DROP_ITEM_NORMAL,                                  // ITEM_QUALITY_NORMAL
    RATE_DROP_ITEM_UNCOMMON,                                // ITEM_QUALITY_UNCOMMON
    RATE_DROP_ITEM_RARE,                                    // ITEM_QUALITY_RARE
    RATE_DROP_ITEM_EPIC,                                    // ITEM_QUALITY_EPIC
    RATE_DROP_ITEM_LEGENDARY,                               // ITEM_QUALITY_LEGENDARY
    RATE_DROP_ITEM_ARTIFACT,                                // ITEM_QUALITY_ARTIFACT
};

LootStore LootTemplates_Creature("creature_loot_template",           "creature entry",                  true,  ", shared");
LootStore LootTemplates_Disenchant("disenchant_loot_template",       "item disenchant id",              true,  "");
LootStore LootTemplates_Fishing("fishing_loot_template",             "area id",                         true,  "");
LootStore LootTemplates_Gameobject("gameobject_loot_template",       "gameobject entry",                true,  "");
LootStore LootTemplates_Item("item_loot_template",                   "item entry",                      true,  "");
LootStore LootTemplates_Mail("mail_loot_template",                   "mail template id",                false, "");
LootStore LootTemplates_Milling("milling_loot_template",             "item entry (herb)",               true,  "");
LootStore LootTemplates_Pickpocketing("pickpocketing_loot_template", "creature pickpocket lootid",      true,  "");
LootStore LootTemplates_Prospecting("prospecting_loot_template",     "item entry (ore)",                true,  "");
LootStore LootTemplates_Reference("reference_loot_template",         "reference id",                    false, "");
LootStore LootTemplates_Skinning("skinning_loot_template",           "creature skinning id",            true,  "");
LootStore LootTemplates_Spell("spell_loot_template",                 "spell id (random item creating)", false, "");
LootStore LootTemplates_World("world_loot_template",                 "expansion",                       false, "");
LootStore LootTemplates_Luck("luck_loot_template",                   "creature entry",                  false, "");
LootStore LootTemplates_Zone("zone_loot_template",                   "zoneId",                          true,  ", ClassificationMask");

class LootTemplate::LootGroup                               // A set of loot definitions for items (refs are not allowed)
{
    public:
        void AddEntry(LootStoreItem& item);                 // Adds an entry to the group (at loading stage)
        bool HasQuestDrop() const;                          // True if group includes at least 1 quest drop entry
        bool HasQuestDropForPlayer(Player const* player) const;
                                                            // The same for active quests of the player
        void Process(Loot& loot, LootTemplate const* tab) const;             // Rolls an item from the group (if any) and adds the item to the loot
        void ProcessAutoGroup(Loot& loot, LootTemplate const* tab) const;         // Rolls an item from the inst group (if any) and adds the item to the loot
        bool ProcessBossLoot(Loot& loot, LootTemplate const* tab) const; // Rolls an item personal
        bool ProcessRareOrGoLoot(Loot& loot, LootTemplate const* tab) const; // Rolls an item personal
        void ProcessItemLoot(Loot& loot, LootTemplate const* tab) const; // Rolls an item personal
        void ProcessWorld(Loot& loot, LootTemplate const* tab, bool ignore) const; // Rolls an item personal
        float RawTotalChance() const;                       // Overall chance for the group (without equal chanced items)
        float TotalChance() const;                          // Overall chance for the group

        void Verify(LootStore const& lootstore, uint32 id, uint8 group_id) const;
        void CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const;
        LootStoreItemList* GetExplicitlyChancedItemList() { return &ExplicitlyChanced; }
        LootStoreItemList* GetEqualChancedItemList() { return &EqualChanced; }
        void CopyConditions(ConditionList conditions);
        LootStoreItemList ExplicitlyChanced;                // Entries with chances defined in DB
        LootStoreItemList EqualChanced;                     // Zero chances - every entry takes the same chance

    private:
        LootStoreItem const* Roll() const;                 // Rolls an item from the group, returns NULL if all miss their chances
};

//Remove all data and free all memory
void LootStore::Clear()
{
    for (LootTemplateMap::const_iterator itr=m_LootTemplates.begin(); itr != m_LootTemplates.end(); ++itr)
        delete itr->second;
    m_LootTemplates.clear();
}

LootStore::LootStore(std::string name, std::string entryName, bool ratesAllowed, std::string addColum): m_name(name), m_entryName(entryName), m_addColum(addColum), m_ratesAllowed(ratesAllowed)
{
}// Checks validity of the loot store
// Actual checks are done within LootTemplate::Verify() which is called for every template
void LootStore::Verify() const
{
    for (LootTemplateMap::const_iterator i = m_LootTemplates.begin(); i != m_LootTemplates.end(); ++i)
        i->second->Verify(*this, i->first);
}

// Loads a *_loot_template DB table into loot store
// All checks of the loaded template are called from here, no error reports at loot generation required
uint32 LootStore::LoadLootTable()
{
    LootTemplateMap::const_iterator tab;

    // Clearing store (for reloading case)
    Clear();

    //                                                  0     1            2               3         4         5             6    ...
    QueryResult result = WorldDatabase.PQuery("SELECT entry, item, ChanceOrQuestChance, lootmode, groupid, mincountOrRef, maxcount%s FROM %s", GetAddColum(), GetName());

    if (!result)
        return 0;

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint8 i = 0;
        uint32 entry               = fields[i++].GetUInt32();
        uint32 item                = abs(fields[i++].GetInt32());
        uint8 type                 = fields[1].GetInt32() >= 0 ? LOOT_ITEM_TYPE_ITEM : LOOT_ITEM_TYPE_CURRENCY;
        float  chanceOrQuestChance = fields[i++].GetFloat();
        uint16 lootmode            = fields[i++].GetUInt16();
        uint8  group               = fields[i++].GetUInt8();
        int32  mincountOrRef       = fields[i++].GetInt32();
        int32  maxcount            = fields[i++].GetUInt32();
        bool shared = false;
        uint8 ClassificationMask = 0;

        // Read aditional colums
        if (!strcmp(GetName(), "creature_loot_template") && result->GetFieldCount() > i)
            shared = fields[i++].GetBool();

        // Read aditional colums
        if (!strcmp(GetName(), "zone_loot_template") && result->GetFieldCount() > i)
            ClassificationMask = fields[i++].GetUInt8();

        // Remove Dauntless gear from loot if patch 7.2 is not released yet
        if (item >= 147212 && item <= 147223 && sWorld->getIntConfig(CONFIG_LEGION_ENABLED_PATCH) < 2)
            continue;

        if (type == LOOT_ITEM_TYPE_ITEM && maxcount > std::numeric_limits<uint8>::max())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: maxcount value (%u) to large. must be less %u - skipped", GetName(), entry, item, maxcount, std::numeric_limits<uint8>::max());
            continue;                                   // error already printed to log/console.
        }

        LootStoreItem storeitem = LootStoreItem(item, type, chanceOrQuestChance, lootmode, group, mincountOrRef, maxcount);
        storeitem.shared = shared;
        storeitem.ClassificationMask = ClassificationMask;

        if (!storeitem.IsValid(*this, entry))            // Validity checks
            continue;

        // Looking for the template of the entry
                                                        // often entries are put together
        if (m_LootTemplates.empty() || tab->first != entry)
        {
            // Searching the template (in case template Id changed)
            tab = m_LootTemplates.find(entry);
            if (tab == m_LootTemplates.end())
            {
                std::pair< LootTemplateMap::iterator, bool > pr = m_LootTemplates.insert(std::make_pair(entry, new LootTemplate));
                tab = pr.first;
            }
        }
        // else is empty - template Id and iter are the same
        // finally iter refers to already existed or just created <entry, LootTemplate>

        // Adds current row to the template
        tab->second->AddEntry(storeitem);
        if (!strcmp(GetName(), "zone_loot_template"))
            tab->second->_isZoneLoot = true;

        ++count;
    }
    while (result->NextRow());

    Verify();                                           // Checks validity of the loot store

    return count;
}

bool LootStore::HaveQuestLootFor(uint32 loot_id) const
{
    LootTemplateMap::const_iterator itr = m_LootTemplates.find(loot_id);
    if (itr == m_LootTemplates.end())
        return false;

    // scan loot for quest items
    return itr->second->HasQuestDrop(m_LootTemplates);
}

bool LootStore::HaveQuestLootForPlayer(uint32 loot_id, Player* player) const
{
    LootTemplateMap::const_iterator tab = m_LootTemplates.find(loot_id);
    if (tab != m_LootTemplates.end())
        if (tab->second->HasQuestDropForPlayer(m_LootTemplates, player))
            return true;

    return false;
}

void LootStore::ResetConditions()
{
    for (LootTemplateMap::iterator itr = m_LootTemplates.begin(); itr != m_LootTemplates.end(); ++itr)
    {
        ConditionList empty;
        (*itr).second->CopyConditions(empty);
    }
}

LootTemplate const* LootStore::GetLootFor(uint32 loot_id) const
{
    return Trinity::Containers::MapGetValuePtr(m_LootTemplates, loot_id);
}

LootTemplate* LootStore::GetLootForConditionFill(uint32 loot_id)
{
    return Trinity::Containers::MapGetValuePtr(m_LootTemplates, loot_id);
}

uint32 LootStore::LoadAndCollectLootIds(LootIdSet& lootIdSet)
{
    uint32 count = LoadLootTable();

    for (LootTemplateMap::const_iterator tab = m_LootTemplates.begin(); tab != m_LootTemplates.end(); ++tab)
        lootIdSet.insert(tab->first);

    return count;
}

void LootStore::CheckLootRefs(LootIdSet* ref_set) const
{
    for (LootTemplateMap::const_iterator ltItr = m_LootTemplates.begin(); ltItr != m_LootTemplates.end(); ++ltItr)
        ltItr->second->CheckLootRefs(m_LootTemplates, ref_set);
}

void LootStore::ReportUnusedIds(LootIdSet const& lootIdSet) const
{
    return;
    // all still listed ids isn't referenced
    for (LootIdSet::const_iterator itr = lootIdSet.begin(); itr != lootIdSet.end(); ++itr)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d isn't %s and not referenced from loot, and then useless.", GetName(), *itr, GetEntryName());
        //WorldDatabase.PExecute("DELETE FROM `%s` WHERE `entry` = %u", GetName(), *itr);
    }
}

void LootStore::ReportNotExistedId(uint32 id) const
{
    TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d (%s) does not exist but used as loot id in DB.", GetName(), id, GetEntryName());
}

//
// --------- LootStoreItem ---------
//

LootStoreItem::LootStoreItem(uint32 _itemid, uint8 _type, float _chanceOrQuestChance, uint16 _lootmode, uint8 _group, int32 _mincountOrRef, uint32 _maxcount) : chance(fabs(_chanceOrQuestChance)), itemid(_itemid), mincountOrRef(_mincountOrRef), maxcount(_maxcount), lootmode(_lootmode),
type(_type), group(_group), needs_quest(_chanceOrQuestChance < 0)
{
}
// Checks if the entry (quest, non-quest, reference) takes it's chance (at loot generation)
// RATE_DROP_ITEMS is no longer used for all types of entries
bool LootStoreItem::Roll(bool rate, bool isDungeon /* = false*/, bool isZoneLoot) const
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootStoreItem::Roll chance %f rate %u mincountOrRef %i type %i itemid %u", chance, rate, mincountOrRef, type, itemid);

    if (chance >= 100.0f)
        return true;

    if (mincountOrRef < 0)                                   // reference case
        return roll_chance_f(chance * (rate ? sWorld->getRate(RATE_DROP_ITEM_REFERENCED) : 1.0f));

    if (type == LOOT_ITEM_TYPE_ITEM)
    {
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemid);

        float qualityModifier = pProto && rate ? sWorld->getRate(qualityToRate[pProto->GetQuality()]) : 1.0f;
        if (isZoneLoot && !isDungeon)
            qualityModifier = rate ? sWorld->getRate(RATE_DROP_ZONE_LOOT) : 1.0f;

        if (isDungeon && sWorld->getBoolConfig(CONFIG_DROP_DUNGEON_ONLY_X1))
            qualityModifier = 1.0f;

        return roll_chance_f(chance * qualityModifier);
    }
    if (type == LOOT_ITEM_TYPE_CURRENCY)
    {
        if (isDungeon && sWorld->getBoolConfig(CONFIG_DROP_DUNGEON_ONLY_X1))
            return roll_chance_f(chance * 1.0f);
        else
            return roll_chance_f(chance * (rate ? sWorld->getRate(RATE_DROP_CURRENCY) : 1.0f));
    }

    return false;
}

// Checks correctness of values
bool LootStoreItem::IsValid(LootStore const& store, uint32 entry) const
{
    if (group >= 1 << 7)                                     // it stored in 7 bit field
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: group (%u) must be less %u - skipped", store.GetName(), entry, itemid, group, 1 << 7);
        return false;
    }

    if (group && type == LOOT_ITEM_TYPE_CURRENCY)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d currency %d: group is set, but currencies must not have group - skipped", store.GetName(), entry, itemid, group, 1 << 7);
        return false;
    }

    if (mincountOrRef == 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: wrong mincountOrRef (%d) - skipped", store.GetName(), entry, itemid, mincountOrRef);
        return false;
    }

    if (mincountOrRef > 0)                                  // item (quest or non-quest) entry, maybe grouped
    {
        if (type == LOOT_ITEM_TYPE_ITEM)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid);
            if (!proto)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: item entry not listed in `item_template` - skipped", store.GetName(), entry, itemid);
                //WorldDatabase.PExecute("DELETE FROM `%s` WHERE `item` = %u", store.GetName(), itemid);
                return false;
            }
        }
        else if (type == LOOT_ITEM_TYPE_CURRENCY)
        {
            CurrencyTypesEntry const* currency = sCurrencyTypesStore.LookupEntry(itemid);
            if (!currency)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d: currency entry %u not exists - skipped", store.GetName(), entry, itemid);
                return false;
            }
        }
        else
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d: has unknown item %u with type %u - skipped", store.GetName(), entry, itemid, type);
            return false;
        }

        /*if (chance == 0 && group == 0)                      // Zero chance is allowed for grouped entries only
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: equal-chanced grouped entry, but group not defined - skipped", store.GetName(), entry, itemid);
            return false;
        }*/

        if (chance != 0 && chance < 0.000001f)             // loot with low chance
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: low chance (%f) - skipped",
                store.GetName(), entry, itemid, chance);
            return false;
        }

        if (int(maxcount) < mincountOrRef)                       // wrong max count
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: max count (%u) less that min count (%i) - skipped", store.GetName(), entry, itemid, int32(maxcount), mincountOrRef);
            return false;
        }
    }
    else                                                    // mincountOrRef < 0
    {
        if (needs_quest)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: quest chance will be treated as non-quest chance", store.GetName(), entry, itemid);
        else if (chance == 0)                              // no chance for the reference
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: zero chance is specified for a reference, skipped", store.GetName(), entry, itemid);
            return false;
        }
    }
    if (!strcmp(store.GetName(), "item_loot_template") && itemid == entry)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %d item %d: itemID same as entry, skipped", store.GetName(), entry, itemid);
        return false;
    }
    return true;                                            // Referenced template existence is checked at whole store level
}

CurrencyLoot::CurrencyLoot(uint32 _entry, uint8 _type, uint32 _CurrencyId, uint32 _CurrencyAmount, uint32 _CurrencyMaxAmount, uint8 _lootmode, float _chance): Entry(_entry), Type(_type), CurrencyId(_CurrencyId),
                                                                                                                                                               CurrencyAmount(_CurrencyAmount), currencyMaxAmount(_CurrencyMaxAmount), lootmode(_lootmode), chance(_chance)
{
}

//
// --------- LootItem ---------
//

LootItem::LootItem(): type(0), quality(ITEM_QUALITY_POOR), count(0), currency(false),
                      is_looted(false), is_blocked(false), freeforall(false), is_underthreshold(false), is_counted(false), needs_quest(false),
                      follow_loot_rules(false)
{
}// Constructor, copies most fields from LootStoreItem and generates random count
LootItem::LootItem(LootStoreItem const& li, Loot* loot)
{
    item.ItemID = li.itemid;
    type        = li.type;
    conditions  = li.conditions;
    currency    = type == LOOT_ITEM_TYPE_CURRENCY;
    count       = urand(li.mincountOrRef, li.maxcount);     // constructor called for mincountOrRef > 0 only
    quality     = ITEM_QUALITY_POOR;
    needs_quest = li.needs_quest;

    is_looted = false;
    is_blocked = false;
    is_underthreshold = false;
    is_counted = false;

    init(loot);
}

void LootItem::InitItem(uint32 itemID, uint32 _count, Loot* loot, bool isCurrency)
{
    item.ItemID = itemID;
    currency = type = isCurrency;
    count = _count;

    init(loot);
}

void LootItem::init(Loot* loot)
{
    m_loot = loot;
    if (currency)
    {
        freeforall = false;
        follow_loot_rules = false;

        float multiplier = sWorld->getRate(RATE_DROP_CURRENCY_AMOUNT);
        if (loot)
            if (Player const* lootOwner = loot->GetLootOwner())
                multiplier *= lootOwner->GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_CURRENCY_LOOT, sCurrencyTypesStore.LookupEntry(item.ItemID)->CategoryID);

        count = uint32(count * multiplier + 0.5f);
    }
    else
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.ItemID);
        freeforall  = proto && (proto->GetFlags() & ITEM_FLAG_MULTI_DROP);
        follow_loot_rules = proto && (proto->FlagsCu & ITEM_FLAGS_CU_FOLLOW_LOOT_RULES);
        count = uint32(count + 0.5f);
        
        item.RandomPropertiesSeed = proto && proto->GetItemRandomSuffixGroupID() ? GenerateEnchSuffixFactor(proto) : 0;
        if (loot)
            item.RandomPropertiesID = Item::GenerateItemRandomPropertyId(item.ItemID, loot->personal ? loot->GetLootOwner()->GetLootSpecID() : 0);
        
        item.UpgradeID = sDB2Manager.GetRulesetItemUpgrade(item.ItemID);

        quality = proto ? ItemQualities(proto->GetQuality()) : ITEM_QUALITY_POOR;
    }
}

// Basic checks for player/item compatibility - if false no chance to see the item in the loot
bool LootItem::AllowedForPlayer(Player const* player) const
{
    // DB conditions check
    if (!sConditionMgr->IsObjectMeetToConditions(const_cast<Player*>(player), conditions))
        return false;

    return true;
}

void LootItem::AddAllowedLooter(const Player* player)
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootItem::AddAllowedLooter ItemID %u player %s", item.ItemID, player->GetGUID().ToString().c_str());
    allowedGUIDs.insert(player->GetGUID());
}

QuestItem::QuestItem(): index(0), is_looted(false)
{
}

QuestItem::QuestItem(uint8 _index, bool _islooted): index(_index), is_looted(_islooted)
{
}

Loot::Loot(uint32 _gold): _DifficultyMask(0)
{
    gold = _gold;
    unlootedCount = 0;
    loot_type = LOOT_CORPSE;
    m_lootOwner = nullptr;
    objType = 0;
    objGuid.Clear();
    objEntry = 0;
    chance = 20; //Default chance for bonus roll
    personal = false;
    isBoss = false;
    bonusLoot = false;
    _isEmissaryLoot = false;
    _IsPvPLoot = false;
    _isTokenLoot = false;
    _isItemLoot = false;
    _specCheck = true;
    isClear = true;
    m_guid.Clear();
    _itemContext = 0;
    _needLevel = 0;
    _levelBonus = 0;
    _challengeLevel = 0;
    rateLegendary = 0.0f;
    _ExpansionID = 0;
    _DifficultyID = 0;
    _ClassificationMask = 0;

    objectCountInWorld[uint8(HighGuid::LootObject)]++;
}

Loot::~Loot()
{
    clear();
    objectCountInWorld[uint8(HighGuid::LootObject)]--;
}

void Loot::GenerateLootGuid(ObjectGuid __objGuid)
{
    m_guid = ObjectGuid::Create<HighGuid::LootObject>(__objGuid.GetMapId(), 0, sObjectMgr->GetGenerator<HighGuid::LootObject>()->Generate());
}

uint32 Loot::ReplaceLootID(uint32 lootId)
{
    if (OploteLoot* oploteLoot = sChallengeMgr->GetOploteLoot(m_lootOwner->GetGUID()))
    {
        _challengeLevel = oploteLoot->ChallengeLevel;
        sChallengeMgr->DeleteOploteLoot(m_lootOwner->GetGUID());
        return Trinity::Containers::SelectRandomContainerElement(ChallengeChestList);
    }

    return lootId;
}

bool Loot::IsEmissaryLoot(uint32 lootId, WorldObject const* lootFrom)
{
    if (lootFrom != nullptr) //Item is not worldobject
        return false;

    switch (lootId)
    {
        case 137560:
        case 137561:
        case 137562:
        case 137563:
        case 137564:
        case 137565:
        case 141350:
        case 157831:
        case 157829:
        case 157828:
        case 157830:
        case 157826:
        case 157824:
        case 157823:
        case 157822:
        case 157827:
        case 157825:
            return true;
    }

    return false;
}

bool Loot::IsPvPLoot(uint32 lootId, WorldObject const* lootFrom)
{
    if (lootFrom != nullptr) //Item is not worldobject
        return false;

    switch (lootId)
    {
        case 147446:
        case 154991:
        case 154992:
            return true;
    }

    return false;
}

LootItem* Loot::GetLootItem(uint32 entry)
{
    for (auto &i : items)
    {
        if (i.item.ItemID == entry)
            return &i;
    }
    return nullptr;
}

void Loot::AddOrReplaceItem(uint32 itemID, uint32 _count, bool isRes, bool update/*=false*/)
{
    LootItem* item = GetLootItem(itemID);
    if (item)
    {
        if (update)
            item->count += _count;
        else
            item->count = _count;
        item->is_looted = false;
    }
    else
    {
        LootItem lootItem;
        lootItem.InitItem(itemID, _count, this, isRes);
        items.push_back(lootItem);

        if (isRes)
            FillCurrencyLoot(m_lootOwner); //register
    }
}

// Inserts the item into the loot (called by LootTemplate processors)
void Loot::AddItem(LootStoreItem const & item, std::vector<uint32> const& bonusListIDs)
{
    LootItem generatedLoot(item, this);
    if (item.type != LOOT_ITEM_TYPE_CURRENCY && (item.needs_quest || items.size() < MAX_NR_LOOT_ITEMS))
    {
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(item.itemid);

        if (!bonusListIDs.empty())
            generatedLoot.item.ItemBonus.BonusListIDs = bonusListIDs;
        else
        {
            if (pProto && pProto->GetQuality() == ITEM_QUALITY_LEGENDARY && pProto->GetExpansion() == EXPANSION_LEGION)
            {
                generatedLoot.item.ItemBonus.BonusListIDs = sObjectMgr->GetItemBonusTree(generatedLoot.item.ItemID, 0, m_lootOwner->getLevel(), 0, 0, _needLevel);
            }
            else if (_isEmissaryLoot)
            {
                auto bonusTree = m_lootOwner->GetWorldQuestBonusTreeMod(nullptr);
                _itemContext = std::get<0>(bonusTree);
                generatedLoot.item.ItemBonus.BonusListIDs = sObjectMgr->GetItemBonusTree(generatedLoot.item.ItemID, _itemContext, m_lootOwner->getLevel(), 0, 0, std::get<1>(bonusTree));
            }
            else if (_isEventLoot)
            {
                int32 ilevel = 0;
                if (pProto->GetBaseItemLevel() == 820 || pProto->GetBaseItemLevel() == 810 || pProto->GetBaseItemLevel() == 855)
                {
                    auto ownerIlvl = m_lootOwner->GetAverageItemLevelEquipped();
                    auto base = pProto->GetBaseItemLevel();
                    if (ownerIlvl > 200 && ownerIlvl < 860)
                        ilevel = 880 - base;
                    else if (ownerIlvl > 860 && ownerIlvl < 880)
                        ilevel = 900 - base;
                    else if (ownerIlvl > 880 && ownerIlvl < 890)
                        ilevel = 905 - base;
                    else if (ownerIlvl > 890 && ownerIlvl < 900)
                        ilevel = 910 - base;
                    else if (ownerIlvl > 900 && ownerIlvl < 910)
                        ilevel = 915 - base;
                    else if (ownerIlvl > 910 && ownerIlvl < 915)
                        ilevel = 920 - base;
                    else if (ownerIlvl > 915 && ownerIlvl < 920)
                        ilevel = 925 - base;
                    else if (ownerIlvl > 920 && ownerIlvl < 925)
                        ilevel = 930 - base;
                    else if (ownerIlvl > 925 && ownerIlvl < 930)
                        ilevel = 935 - base;
                    else if (ownerIlvl > 930 && ownerIlvl < 935)
                        ilevel = 940 - base;
                    else if (ownerIlvl > 935 && ownerIlvl < 940)
                        ilevel = 945 - base;
                    else if (ownerIlvl > 940 && ownerIlvl < 945)
                        ilevel = 950 - base;
                    else if (ownerIlvl > 945 && ownerIlvl < 950)
                        ilevel = 955 - base;
                    else if (ownerIlvl > 950)
                        ilevel = 960 - base;
                }
                generatedLoot.item.ItemBonus.BonusListIDs = sObjectMgr->GetItemBonusTree(generatedLoot.item.ItemID, _itemContext, m_lootOwner->getLevel(), ilevel, _challengeLevel, _needLevel);
            }
            else if (dungeonEncounterID != 0 && _ExpansionID == EXPANSION_LEGION && _NoneRaidOrScenarioDungeonLoot)
            {
                int32 patchBasedILvlBonus = _levelBonus - ((3 - sWorld->getIntConfig(CONFIG_LEGION_ENABLED_PATCH)) * 20);
                generatedLoot.item.ItemBonus.BonusListIDs = sObjectMgr->GetItemBonusTree(generatedLoot.item.ItemID, _itemContext, m_lootOwner->getLevel(), patchBasedILvlBonus, _challengeLevel, _needLevel);
            }
            else
                generatedLoot.item.ItemBonus.BonusListIDs = sObjectMgr->GetItemBonusTree(generatedLoot.item.ItemID, _itemContext, m_lootOwner->getLevel(), _levelBonus, _challengeLevel, _needLevel);
        }

        generatedLoot.item.ItemBonus.Context = _itemContext;
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::AddItem ItemID %i needs_quest %u personal %u isOnlyQuest %u chance %f", generatedLoot.item.ItemID, item.needs_quest, personal, isOnlyQuest, item.chance);

    if (item.needs_quest)
    {
        if (isOnlyQuest && personal) // Don`t generate quest loot in personal loot
            return;

        if (quest_items.size() < MAX_NR_QUEST_ITEMS)
            quest_items.push_back(generatedLoot);

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::AddItem ItemID %i quest_items %u", generatedLoot.item.ItemID, quest_items.size());
    }
    else if (items.size() < MAX_NR_LOOT_ITEMS)
    {
        if (isOnlyQuest && !personal) // Don`t generate loot in loot if loot is personal
            return;

        items.push_back(generatedLoot);

        // non-conditional one-player only items are counted here,
        // free for all items are counted in FillFFALoot(),
        // non-ffa conditionals are counted in FillNonQuestNonFFAConditionalLoot()
        if (item.conditions.empty() && item.type == LOOT_ITEM_TYPE_ITEM)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemid);
            if (!proto || (proto->GetFlags() & ITEM_FLAG_MULTI_DROP) == 0)
                ++unlootedCount;
        }
    }
}

// Auto store personal loot
void Loot::AutoStoreItems(bool isGO)
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loo::tAutoStoreItems items %i quest_items %i", items.size(), quest_items.size());
    for (uint8 i = 0; i < (items.size() + quest_items.size()); ++i)
        m_lootOwner->StoreLootItem(i, this);
}

// Calls processor of corresponding LootTemplate (which handles everything including references)
bool Loot::FillLoot(uint32 lootId, LootStore const& store, Player* lootOwner, bool noGroup, bool noEmptyError, WorldObject const* lootFrom)
{
    // Must be provided
    if (!lootOwner || !lootOwner->CanContact())
        return false;

    Creature* creature = nullptr;
    GameObject* go = nullptr;
    if (lootFrom)
    {
        creature = const_cast<Creature*>(lootFrom->ToCreature());
        go = const_cast<GameObject*>(lootFrom->ToGameObject());
        objEntry = lootFrom->GetEntry();
        objGuid = lootFrom->GetGUID();
        if (creature)
        {
            isBoss = creature->isWorldBoss();
            _isEventLoot = isBoss && (creature->GetCreatureTemplate() && creature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_EVENT_LOOT);
            isRareOrGo = bool(creature->GetTrackingQuestID() && !isBoss);
            dungeonEncounterID = sObjectMgr->GetDungeonEncounterByCreature(creature->GetEntry());
            if (creature->GetCreatureTemplate())
                _ClassificationMask = (1 << creature->GetCreatureTemplate()->Classification);
        }
        if (go)
        {
            if (GameObjectTemplate const* goInfo = go->GetGOInfo())
            {
                dungeonEncounterID = goInfo->chest.DungeonEncounter;
                if (dungeonEncounterID)
                    isBoss = true;
            }
            isRareOrGo = go->IsPersonal();
        }
        if (Map* map = lootFrom->GetMap())
        {
            _NoneRaidOrScenarioDungeonLoot = lootFrom->GetMap()->IsNonRaidDungeon() && !lootFrom->GetMap()->IsScenario();
            _ExpansionID = map->GetEntry()->ExpansionID;
            _DifficultyID = map->GetLootDifficulty();
        }
    }
    else if (Map* map = lootOwner->GetMap())
        _ExpansionID = map->GetEntry()->ExpansionID;

    _DifficultyMask = (1 << (CreatureTemplate::GetDiffFromSpawn(_DifficultyID)));
    _isEmissaryLoot = IsEmissaryLoot(lootId, lootFrom);
    _IsPvPLoot = IsPvPLoot(lootId, lootFrom);
    bool isOploteChest = go && go->GetGOInfo() && go->GetGOInfo()->IsOploteChest();
    rateLegendary = lootOwner->GetRateLegendaryDrop(isBoss, isRareOrGo && !isRareNext, isOploteChest, _isEmissaryLoot || _isTokenLoot, _ExpansionID, _DifficultyID);

    m_lootOwner = lootOwner;

    if (!_isTokenLoot && !_isItemLoot) // TreeMod for token calculate in spelleffect
        _itemContext = lootOwner->GetMap()->GetDifficultyLootItemContext(false, lootOwner->getLevel() == MAX_LEVEL, isBoss);

    Challenge* _challenge = nullptr;
    // Generate data for Challenge
    if (go && isRareOrGo && go->InInstance())
        if (uint32 instanceId = go->GetInstanceId())
            if (Scenario* progress = sScenarioMgr->GetScenario(instanceId))
                if (_challenge = progress->GetChallenge())
                    if (_challenge->_complete)
                        _itemContext = sChallengeMgr->GetLootTreeMod(_levelBonus, _challengeLevel, _challenge);

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillLoot objEntry %i lootId %i isBoss %i DifficultyID %i personal %i isRareOrGo %u isRareNext %u noGroup %u isOnlyQuest %u _itemContext %u store %s", objEntry, lootId, isBoss, _DifficultyID, personal, isRareOrGo, isRareNext, noGroup, isOnlyQuest, _itemContext, store.GetName());

    if (isOploteChest)
    {
        if (sChallengeMgr->HasOploteLoot(lootOwner->GetGUID()))
        {
            lootId = ReplaceLootID(lootId);
            _itemContext = sChallengeMgr->GetLootTreeMod(_levelBonus, _challengeLevel);
        }
        else
            return false;
    }

    LootTemplate const* tab = store.GetLootFor(lootId);

    if (!tab && !shipmentBuildingType)
    {
        if (objType == 2)
            FillNotNormalLootFor(lootOwner, true);

        if (!noEmptyError)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' loot id #%u used but it doesn't have records.", store.GetName(), lootId);

        if (_isLegendaryLoot)
        {
            AddLegendaryItem(); // Add Legendary Item
            isClear = false;
            return true;
        }

        return false;
    }

    items.reserve(MAX_NR_LOOT_ITEMS);
    quest_items.reserve(MAX_NR_QUEST_ITEMS);

    if (tab)
    {
        if (_isEmissaryLoot || _isTokenLoot || _isItemLoot || _IsPvPLoot)
            tab->ProcessItemLoot(*this);
        else if (isOploteChest)
            tab->ProcessOploteChest(*this);
        else if (_challengeLevel)
            tab->ProcessChallengeChest(*this, lootId, _challenge);
        else if(personal && isBoss)
            tab->ProcessBossLoot(*this);
        else if(personal && isRareOrGo)
            tab->ProcessRareOrGoLoot(*this);
        else
            tab->Process(*this, store.IsRatesAllowed());          // Processing is done there, callback via Loot::AddItem()
    }

    if (!isBoss && !strcmp(store.GetName(), "creature_loot_template"))
    {
        if (LootTemplate const* worldTab = LootTemplates_World.GetLootFor(_ExpansionID))
            worldTab->ProcessWorld(*this, store.IsRatesAllowed());

        if (LootTemplate const* zoneTab = LootTemplates_Zone.GetLootFor(lootOwner->GetCurrentZoneID()))
            zoneTab->Process(*this, store.IsRatesAllowed());
    }

    if (personal)
    {
        if (creature)
        {
            if (LootTemplate const* luckTab = LootTemplates_Luck.GetLootFor(creature->GetEntry()))
                luckTab->ProcessLuck(*this, creature->GetEntry());
        }
        else if (dungeonEncounterID)
            if (uint32 creatureId = sObjectMgr->GetCreatureByDungeonEncounter(dungeonEncounterID))
                if (LootTemplate const* luckTab = LootTemplates_Luck.GetLootFor(creatureId))
                    luckTab->ProcessLuck(*this, creatureId);
    }

    if (InstanceScript* instance = lootOwner->GetInstanceScript())
        instance->OnLootChestOpen(lootOwner, this, lootFrom ? lootFrom->ToGameObject() : nullptr);

    // Setting access rights for group loot case
    Group* group = lootOwner->GetGroup();
    if (!noGroup && group && !isOploteChest)
    {
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillLoot !noGroup && group GetMembersCount %u", group->GetMembersCount());

        roundRobinPlayer = lootOwner->GetGUID();

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            if (Player* player = itr->getSource())   // should actually be looted object instead of lootOwner but looter has to be really close so doesnt really matter
                FillNotNormalLootFor(player, player->IsAtGroupRewardDistance(lootOwner));

        for (size_t i = 0; i < items.size(); ++i)
        {
            if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(items[i].item.ItemID))
                if (proto->GetQuality() < uint32(group->GetLootThreshold()))
                    items[i].is_underthreshold = true;
        }
    }
    // ... for nonGroup loot
    else if (isOnlyQuest && (creature && creature->CanShared() || isBoss))
    {
        GuidSet* savethreatlist = creature->GetSaveThreatList();
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillLoot isOnlyQuest && creature && creature->CanShared size %u", savethreatlist->size());

        if (savethreatlist->empty())
            FillNotNormalLootFor(lootOwner, true);
        else
        {
            for (GuidSet::const_iterator itr = savethreatlist->begin(); itr != savethreatlist->end(); ++itr)
            {
                if (Player* looter = ObjectAccessor::GetPlayer(*creature, (*itr)))
                {
                    if (!creature->isTappedBy(looter))
                        continue;

                    FillNotNormalLootFor(looter, true);
                }
            }
        }
    }
    else
    {
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillLoot else");
        FillNotNormalLootFor(lootOwner, true);
    }

    if (!bonusLoot && (noGroup && personal || _challengeLevel && !isOploteChest) && !(_isEmissaryLoot || _isTokenLoot || _isItemLoot || _IsPvPLoot)) // Add looters to personal loot
        FillPersonalLootFor(lootOwner);

    if(lootId == 90406 || lootId == 90399 || lootId == 90397 || lootId == 90400 ||  lootId == 90398 || lootId == 90395 || lootId == 90401)
    {
        for (std::vector<LootItem>::iterator itemCurrent = items.begin(); itemCurrent != items.end(); ++itemCurrent)
            for (std::vector<uint32>::iterator spellId = sSpellMgr->mSpellCreateItemList.begin(); spellId != sSpellMgr->mSpellCreateItemList.end(); ++spellId)
                if (const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(*spellId))
                    if (spellInfo->Effects[EFFECT_0]->ItemType == itemCurrent->item.ItemID)
                        if (!lootOwner->HasActiveSpell(*spellId))
                            lootOwner->learnSpell(*spellId, false);
    }

    sLootMgr->AddLoot(this);
    isClear = false;

    return true;
}

LootItem const* Loot::GetItemInSlot(uint32 lootSlot) const
{
    if (lootSlot < items.size())
        return &items[lootSlot];

    lootSlot -= uint32(items.size());
    if (lootSlot < quest_items.size())
        return &quest_items[lootSlot];

    return nullptr;
}

void Loot::FillNotNormalLootFor(Player* player, bool presentAtLooting)
{
    ObjectGuid::LowType plguid = player->GetGUIDLow();

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillNotNormalLootFor plguid %u presentAtLooting %u PlayerQuestItems %u", plguid, presentAtLooting, PlayerQuestItems.size());

    QuestItemMap::const_iterator qmapitr = PlayerCurrencies.find(plguid);
    if (qmapitr == PlayerCurrencies.end())
        FillCurrencyLoot(player);

    qmapitr = PlayerQuestItems.find(plguid);
    if (qmapitr == PlayerQuestItems.end())
        FillQuestLoot(player);

    qmapitr = PlayerFFAItems.find(plguid);
    if (qmapitr == PlayerFFAItems.end())
        FillFFALoot(player);

    qmapitr = PlayerNonQuestNonFFANonCurrencyConditionalItems.find(plguid);
    if (qmapitr == PlayerNonQuestNonFFANonCurrencyConditionalItems.end())
        FillNonQuestNonFFAConditionalLoot(player, presentAtLooting);

    // if not auto-processed player will have to come and pick it up manually
    if (!presentAtLooting)
        return;

    // Process currency items
    std::list<CurrencyLoot> temp = sObjectMgr->GetCurrencyLoot(objEntry, objType, _DifficultyID);
    for (std::list<CurrencyLoot>::iterator i = temp.begin(); i != temp.end(); ++i)
        if(CurrencyTypesEntry const* proto = sCurrencyTypesStore.LookupEntry(i->CurrencyId))
        {
            auto Roll = static_cast<float>(rand_chance());
            if (i->chance < 100.0f && i->chance < Roll)
                continue;

            uint32 amount = 0.5f + urand(i->CurrencyAmount, i->currencyMaxAmount) * sDB2Manager.GetCurrencyPrecision(proto->ID);
            player->ModifyCurrency(i->CurrencyId, amount, true);

            // log Veiled Argunite and Wakening Essence currency
            if (i->CurrencyId == 1508 || i->CurrencyId == 1533)
                sLog->outWarden("Player %s (GUID: %u) adds a currency value %u (%u) from not normal loot %u and type %u", player->GetName(), player->GetGUIDLow(), amount, i->CurrencyId, objEntry, objType);
        }
}

void Loot::clear()
{
    //If loot not generate or already clear
    if(isClear)
        return;

    isClear = true;

    if(!PlayerCurrencies.empty())
    {
        for (QuestItemMap::const_iterator itr = PlayerCurrencies.begin(); itr != PlayerCurrencies.end(); ++itr)
            delete itr->second;
        PlayerCurrencies.clear();
    }

    if(!PlayerQuestItems.empty())
    {
        for (QuestItemMap::const_iterator itr = PlayerQuestItems.begin(); itr != PlayerQuestItems.end(); ++itr)
            delete itr->second;
        PlayerQuestItems.clear();
    }

    if(!PlayerFFAItems.empty())
    {
        for (QuestItemMap::const_iterator itr = PlayerFFAItems.begin(); itr != PlayerFFAItems.end(); ++itr)
            delete itr->second;
        PlayerFFAItems.clear();
    }

    if(!PlayerNonQuestNonFFANonCurrencyConditionalItems.empty())
    {
        for (QuestItemMap::const_iterator itr = PlayerNonQuestNonFFANonCurrencyConditionalItems.begin(); itr != PlayerNonQuestNonFFANonCurrencyConditionalItems.end(); ++itr)
            delete itr->second;
        PlayerNonQuestNonFFANonCurrencyConditionalItems.clear();
    }

    PlayersLooting.clear();
    items.clear();
    quest_items.clear();
    gold = 0;
    unlootedCount = 0;
    roundRobinPlayer.Clear();
    objType = 0;

    if (!GetGUID().IsEmpty())
        sLootMgr->RemoveLoot(GetGUID());

    chance = 20;
    personal = false;
    bonusLoot = false;
    isBoss = false;
    isOpen = false;
    isOnlyQuest = false;
    isRareOrGo = false;
    _isEmissaryLoot = false;
    _IsPvPLoot = false;
    _isTokenLoot = false;
    _isItemLoot = false;
    _isLegendaryLoot = false;
    _isEventLoot = false;
    _specCheck = true;
    _itemContext = 0;
    _needLevel = 0;
    _levelBonus = 0;
    _challengeLevel = 0;
    rateLegendary = 0.0f;
    _ExpansionID = 0;
    _DifficultyID = 0;
    _ClassificationMask = 0;
}

QuestItemList* Loot::FillCurrencyLoot(Player* player)
{
    QuestItemList* ql = new QuestItemList();

    for (size_t i = 0; i < items.size(); ++i)
    {
        LootItem& item = items[i];
        if (!item.is_looted && item.currency && AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest, &item))
        {
            ql->push_back(QuestItem(i));
            ++unlootedCount;
        }
    }
    if (ql->empty())
    {
        delete ql;
        return nullptr;
    }

    PlayerCurrencies[player->GetGUIDLow()] = ql;
    return ql;
}

QuestItemList* Loot::FillFFALoot(Player* player)
{
    QuestItemList* ql = new QuestItemList();

    for (size_t i = 0; i < items.size(); ++i)
    {
        LootItem &item = items[i];
        if (!item.is_looted && item.freeforall && AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest, &item))
        {
            ql->push_back(QuestItem(i));
            ++unlootedCount;
        }
    }
    if (ql->empty())
    {
        delete ql;
        return nullptr;
    }

    PlayerFFAItems[player->GetGUIDLow()] = ql;
    return ql;
}

QuestItemList* Loot::FillQuestLoot(Player* player)
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillQuestLoot quest_items %u isOnlyQuest %u", quest_items.size(), isOnlyQuest);

    //if (items.size() == MAX_NR_LOOT_ITEMS)
    //    return NULL;

    QuestItemList* ql = new QuestItemList();

    for (size_t i = 0; i < quest_items.size(); ++i)
    {
        LootItem &item = quest_items[i];

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillQuestLoot freeforall %i is_blocked %u is_looted %u ItemID %u AllowedForPlayer %u unlootedCount %u", item.freeforall, item.is_blocked, item.is_looted, item.item.ItemID, AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest), unlootedCount);

        if (!item.is_looted && (AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest, &item) || (item.follow_loot_rules && player->GetGroup() && ((player->GetGroup()->GetLootMethod() == MASTER_LOOT && player->GetGroup()->GetLooterGuid() == player->GetGUID()) || player->GetGroup()->GetLootMethod() != MASTER_LOOT ))))
        {
            ql->push_back(QuestItem(i));

            // quest items get blocked when they first appear in a
            // player's quest vector
            //
            // increase once if one looter only, looter-times if free for all
            if (item.freeforall || !item.is_blocked)
                ++unlootedCount;
            if (!player->GetGroup() || player->GetGroup()->GetLootMethod() != GROUP_LOOT)
                item.is_blocked = true;

            if (ql->size() == MAX_NR_QUEST_ITEMS)
                break;
        }
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillQuestLoot ql %u unlootedCount %u", ql->size(), unlootedCount);

    if (ql->empty())
    {
        delete ql;
        return nullptr;
    }

    PlayerQuestItems[player->GetGUIDLow()] = ql;
    return ql;
}

QuestItemList* Loot::FillNonQuestNonFFAConditionalLoot(Player* player, bool presentAtLooting)
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillNonQuestNonFFAConditionalLoot GetGUIDLow %u presentAtLooting %u GetLootMethod %i", player->GetGUIDLow(), presentAtLooting, player->GetGroup() ? player->GetGroup()->GetLootMethod() : -1);

    QuestItemList* ql = new QuestItemList();

    for (size_t i = 0; i < items.size(); ++i)
    {
        LootItem &item = items[i];
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "FillNonQuestNonFFAConditionalLoot freeforall %i is_blocked %u is_looted %u ItemID %u AllowedForPlayer %u is_underthreshold %u follow_loot_rules %u", item.freeforall, item.is_blocked, item.is_looted, item.item.ItemID, AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest), item.is_underthreshold, item.follow_loot_rules);

        if (!item.is_looted && !item.freeforall && !item.currency && (AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest, &item) || (item.follow_loot_rules && player->GetGroup() && ((player->GetGroup()->GetLootMethod() == MASTER_LOOT && player->GetGroup()->GetLooterGuid() == player->GetGUID()) || player->GetGroup()->GetLootMethod() != MASTER_LOOT))))
        {
            if (presentAtLooting)
            {
                bool allowToLooter = true;
                if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.item.ItemID))
                    if (!proto->AllowToLooter())
                        allowToLooter = false;

                // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillNonQuestNonFFAConditionalLoot ItemID %u allowToLooter %i", item.item.ItemID, allowToLooter);
                if (allowToLooter)
                    item.AddAllowedLooter(player);
            }
            if (!item.conditions.empty())
            {
                ql->push_back(QuestItem(i));
                if (!item.is_counted)
                {
                    ++unlootedCount;
                    item.is_counted = true;
                }
            }
        }
    }
    if (ql->empty())
    {
        delete ql;
        return nullptr;
    }

    PlayerNonQuestNonFFANonCurrencyConditionalItems[player->GetGUIDLow()] = ql;
    return ql;
}

void Loot::FillPersonalLootFor(Player* lootOwner)
{
    if (Group* group = lootOwner->GetGroup())
    {
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootItem::FillPersonalLootFor GetLootMethod %u", group->GetLootMethod());

        // if (group->GetLootMethod() != PERSONAL_LOOT)
            // return;

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Player* player = itr->getSource())
            {
                if (!player->IsAtGroupRewardDistance(lootOwner))
                    continue;

                for (size_t i = 0; i < items.size(); ++i)
                {
                    LootItem &item = items[i];
                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.item.ItemID))
                    {
                        if (!proto->AllowToLooter())
                            continue;

                        if (proto->GetQuality() < uint32(group->GetLootThreshold()))
                            item.is_underthreshold = true;
                    }

                    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::FillPersonalLootFor freeforall %i is_blocked %u is_looted %u ItemID %u AllowedForPlayer %u is_underthreshold %u", item.freeforall, item.is_blocked, item.is_looted, item.item.ItemID, AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest), item.is_underthreshold);

                    if (!item.is_looted && !item.freeforall && !item.currency && !item.is_underthreshold && AllowedForPlayer(player, item.item.ItemID, item.type, item.needs_quest, &item))
                        item.AddAllowedLooter(player);
                }
            }
        }
    }
}

//===================================================

void Loot::NotifyItemRemoved(uint8 lootIndex)
{
    // notify all players that are looting this that the item was removed
    // convert the index to the slot the player sees
    for (GuidHashSet::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); ++i)
    {
        if (Player* player = ObjectAccessor::FindPlayer(*i))
            player->SendNotifyLootItemRemoved(lootIndex, this);
        else
            PlayersLooting.erase_at(i);
    }
}

void Loot::NotifyMoneyRemoved(uint64 gold)
{
    // notify all players that are looting this that the money was removed
    for (GuidHashSet::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); ++i)
    {
        if (Player* player = ObjectAccessor::FindPlayer(*i))
            player->SendNotifyLootMoneyRemoved(this);
        else
            PlayersLooting.erase_at(i);
    }
}

void Loot::NotifyQuestItemRemoved(uint8 questIndex)
{
    // when a free for all questitem is looted
    // all players will get notified of it being removed
    // (other questitems can be looted by each group member)
    // bit inefficient but isn't called often

    for (GuidHashSet::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); ++i)
    {
        if (Player* player = ObjectAccessor::FindPlayer(*i))
        {
            QuestItemMap::const_iterator pq = PlayerQuestItems.find(player->GetGUIDLow());
            if (pq != PlayerQuestItems.end() && pq->second)
            {
                // find where/if the player has the given item in it's vector
                QuestItemList& pql = *pq->second;

                size_t j;
                for (j = 0; j < pql.size(); ++j)
                    if (pql[j].index == questIndex)
                        break;

                if (j < pql.size())
                    player->SendNotifyLootItemRemoved(items.size()+j, this);
            }
        }
        else
            PlayersLooting.erase_at(i);
    }
}

void Loot::AddLooter(ObjectGuid GUID)
{
    PlayersLooting.insert(GUID);
}

void Loot::RemoveLooter(ObjectGuid GUID)
{
    PlayersLooting.erase(ObjectGuidHashGen(GUID));
}

void Loot::generateMoneyLoot(uint32 minAmount, uint32 maxAmount, bool isDungeon /*=false*/)
{
    if (maxAmount > 0)
    {
        if (maxAmount <= minAmount)
            gold = uint32(maxAmount * (isDungeon && sWorld->getBoolConfig(CONFIG_DROP_DUNGEON_ONLY_X1) ? 1.0f : sWorld->getRate(RATE_DROP_MONEY)));
        else if ((maxAmount - minAmount) < 32700)
            gold = uint32(urand(minAmount, maxAmount) * (isDungeon && sWorld->getBoolConfig(CONFIG_DROP_DUNGEON_ONLY_X1) ? 1.0f : sWorld->getRate(RATE_DROP_MONEY)));
        else
            gold = uint32(urand(minAmount >> 8, maxAmount >> 8) * (isDungeon && sWorld->getBoolConfig(CONFIG_DROP_DUNGEON_ONLY_X1) ? 1.0f : sWorld->getRate(RATE_DROP_MONEY))) << 8;
    }
}

LootItem* Loot::LootItemInSlot(uint32 lootSlot, Player* player, QuestItem* *qitem, QuestItem* *ffaitem, QuestItem* *conditem, QuestItem** currency)
{
    LootItem* item = nullptr;
    bool is_looted = true;
    if (lootSlot >= items.size())
    {
        uint32 questSlot = lootSlot - items.size();
        QuestItemMap::const_iterator itr = PlayerQuestItems.find(player->GetGUIDLow());
        if (itr != PlayerQuestItems.end() && questSlot < itr->second->size())
        {
            QuestItem* qitem2 = &itr->second->at(questSlot);
            if (qitem)
                *qitem = qitem2;
            item = &quest_items[qitem2->index];
            is_looted = qitem2->is_looted;
        }
    }
    else
    {
        item = &items[lootSlot];
        if (!item)
            return nullptr;
        is_looted = item->is_looted;
        if (item->currency)
        {
            QuestItemMap::const_iterator itr = PlayerCurrencies.find(player->GetGUIDLow());
            if (itr != PlayerCurrencies.end())
            {
                for (QuestItemList::const_iterator iter = itr->second->begin(); iter != itr->second->end(); ++iter)
                {
                    if (iter->index == lootSlot)
                    {
                        QuestItem* currency2 = (QuestItem*) & (*iter);
                        if (currency)
                            *currency = currency2;
                        is_looted = currency2->is_looted;
                        break;
                    }
                }
            }
        }
        else if (item->freeforall)
        {
            QuestItemMap::const_iterator itr = PlayerFFAItems.find(player->GetGUIDLow());
            if (itr != PlayerFFAItems.end())
            {
                for (QuestItemList::const_iterator iter=itr->second->begin(); iter!= itr->second->end(); ++iter)
                    if (iter->index == lootSlot)
                    {
                        QuestItem* ffaitem2 = (QuestItem*)&(*iter);
                        if (ffaitem)
                            *ffaitem = ffaitem2;
                        is_looted = ffaitem2->is_looted;
                        break;
                    }
            }
        }
        else if (!item->conditions.empty())
        {
            QuestItemMap::const_iterator itr = PlayerNonQuestNonFFANonCurrencyConditionalItems.find(player->GetGUIDLow());
            if (itr != PlayerNonQuestNonFFANonCurrencyConditionalItems.end())
            {
                for (QuestItemList::const_iterator iter=itr->second->begin(); iter!= itr->second->end(); ++iter)
                {
                    if (iter->index == lootSlot)
                    {
                        QuestItem* conditem2 = (QuestItem*)&(*iter);
                        if (conditem)
                            *conditem = conditem2;
                        is_looted = conditem2->is_looted;
                        break;
                    }
                }
            }
        }
    }

    if (is_looted)
        return nullptr;

    return item;
}

uint32 Loot::GetMaxSlotInLootFor(Player* player) const
{
    QuestItemMap::const_iterator itr = PlayerQuestItems.find(player->GetGUIDLow());
    return items.size() + (itr != PlayerQuestItems.end() ?  itr->second->size() : 0);
}

// return true if there is any FFA, quest or conditional item for the player.
bool Loot::hasItemFor(Player* player) const
{
    QuestItemMap const& lootPlayerQuestItems = GetPlayerQuestItems();
    QuestItemMap::const_iterator q_itr = lootPlayerQuestItems.find(player->GetGUIDLow());
    if (q_itr != lootPlayerQuestItems.end())
    {
        QuestItemList* q_list = q_itr->second;
        for (QuestItemList::const_iterator qi = q_list->begin(); qi != q_list->end(); ++qi)
        {
            LootItem const& item = quest_items[qi->index];
            if (!qi->is_looted && !item.is_looted)
                return true;
        }
    }

    QuestItemMap const& lootPlayerFFAItems = GetPlayerFFAItems();
    QuestItemMap::const_iterator ffa_itr = lootPlayerFFAItems.find(player->GetGUIDLow());
    if (ffa_itr != lootPlayerFFAItems.end())
    {
        QuestItemList* ffa_list = ffa_itr->second;
        for (QuestItemList::const_iterator fi = ffa_list->begin(); fi != ffa_list->end(); ++fi)
        {
            LootItem const& item = items[fi->index];
            if (!fi->is_looted && !item.is_looted)
                return true;
        }
    }

    QuestItemMap const& lootPlayerNonQuestNonFFAConditionalItems = GetPlayerNonQuestNonFFANonCurrencyConditionalItems();
    QuestItemMap::const_iterator nn_itr = lootPlayerNonQuestNonFFAConditionalItems.find(player->GetGUIDLow());
    if (nn_itr != lootPlayerNonQuestNonFFAConditionalItems.end())
    {
        QuestItemList* conditional_list = nn_itr->second;
        for (QuestItemList::const_iterator ci = conditional_list->begin(); ci != conditional_list->end(); ++ci)
        {
            LootItem const& item = items[ci->index];
            if (!ci->is_looted && !item.is_looted)
                return true;
        }
    }

    return false;
}

// return true if there is any item over the group threshold (i.e. not underthreshold).
bool Loot::hasOverThresholdItem() const
{
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (!items[i].is_looted && !items[i].is_underthreshold && !items[i].freeforall)
            return true;
    }

    return false;
}

void Loot::BuildLootResponse(WorldPackets::Loot::LootResponse& packet, Player* viewer, PermissionTypes permission, ItemQualities threshold /*= ITEM_QUALITY_POOR*/) const
{
    if (permission == NONE_PERMISSION)
        return;

    packet.Coins = gold;
    packet.Acquired = permission != NONE_PERMISSION;
    switch (permission)
    {
        case GROUP_PERMISSION:
        {
            // if you are not the round-robin group looter, you can only see
            // blocked rolled items and quest items, and !ffa items
            for (size_t i = 0; i < items.size(); ++i)
            {
                if (!items[i].is_looted && !items[i].freeforall && !items[i].currency && items[i].conditions.empty() && AllowedForPlayer(viewer, items[i].item.ItemID, items[i].type, items[i].needs_quest, const_cast<LootItem*>(&items[i])))
                {
                    uint8 slot_type;

                    if (items[i].is_blocked)
                        slot_type = LOOT_ITEM_UI_ROLL;
                    else if (!roundRobinPlayer || !items[i].is_underthreshold || viewer->GetGUID() == roundRobinPlayer)
                    {
                        // no round robin owner or he has released the loot
                        // or it IS the round robin group owner
                        // => item is lootable
                        slot_type = LOOT_ITEM_UI_NORMAL;
                    }
                    else
                        // item shall not be displayed.
                        continue;

                    WorldPackets::Loot::LootItem lootItem;
                    lootItem.LootListID = items[i].LootListID ? items[i].LootListID : i+1;
                    lootItem.Type = LOOT_ITEM_TYPE_ITEM;
                    lootItem.UIType = slot_type;
                    lootItem.Quantity = items[i].count;
                    lootItem.Loot.Initialize(items[i]);
                    packet.Items.push_back(lootItem);
                }
            }
            break;
        }
        case ALL_PERMISSION:
        case MASTER_PERMISSION:
        case OWNER_PERMISSION:
        {
            uint8 slot_type = LOOT_ITEM_UI_NORMAL;
            switch (permission)
            {
                case MASTER_PERMISSION:
                    slot_type = LOOT_ITEM_UI_MASTER;
                    break;
                case OWNER_PERMISSION:
                    slot_type = LOOT_ITEM_UI_OWNER;
                    break;
                default:
                    break;
            }

            for (size_t i = 0; i < items.size(); ++i)
            {
                if (!items[i].is_looted && !items[i].freeforall && !items[i].currency && items[i].conditions.empty() && AllowedForPlayer(viewer, items[i].item.ItemID, items[i].type, items[i].needs_quest, const_cast<LootItem*>(&items[i])))
                {
                    WorldPackets::Loot::LootItem lootItem;
                    lootItem.LootListID = items[i].LootListID ? items[i].LootListID : i+1;
                    lootItem.Type = LOOT_ITEM_TYPE_ITEM;
                    lootItem.UIType = (permission == MASTER_PERMISSION && threshold >= items[i].quality) ? slot_type : LOOT_ITEM_UI_OWNER;
                    lootItem.Quantity = items[i].count;
                    lootItem.Loot.Initialize(items[i]);
                    packet.Items.push_back(lootItem);
                }
            }
            break;
        }
        default:
            return;
    }

    LootItemUIType slotType = permission == OWNER_PERMISSION ? LOOT_ITEM_UI_OWNER : LOOT_ITEM_UI_NORMAL;
    QuestItemMap const& lootPlayerFFAItems = GetPlayerFFAItems();
    QuestItemMap::const_iterator ffa_itr = lootPlayerFFAItems.find(viewer->GetGUIDLow());
    if (ffa_itr != lootPlayerFFAItems.end())
    {
        QuestItemList* ffa_list = ffa_itr->second;
        for (QuestItemList::const_iterator fi = ffa_list->begin(); fi != ffa_list->end(); ++fi)
        {
            LootItem const& item = items[fi->index];
            bool canLoot = (!fi->is_looted && !item.is_looted);
            if (personal)
                canLoot = !item.is_looted;

            if (canLoot)
            {
                WorldPackets::Loot::LootItem lootItem;
                lootItem.LootListID = item.LootListID ? item.LootListID : fi->index+1;
                lootItem.Type = LOOT_ITEM_TYPE_ITEM;
                lootItem.UIType = slotType;
                lootItem.Quantity = item.count;
                lootItem.Loot.Initialize(item);
                packet.Items.push_back(lootItem);
            }
        }
    }

    QuestItemMap const& lootPlayerNonQuestNonFFAConditionalItems = GetPlayerNonQuestNonFFANonCurrencyConditionalItems();
    QuestItemMap::const_iterator nn_itr = lootPlayerNonQuestNonFFAConditionalItems.find(viewer->GetGUIDLow());
    if (nn_itr != lootPlayerNonQuestNonFFAConditionalItems.end())
    {
        QuestItemList* conditional_list = nn_itr->second;
        for (QuestItemList::const_iterator ci = conditional_list->begin(); ci != conditional_list->end(); ++ci)
        {
            LootItem const& item = items[ci->index];
            bool canLoot = (!ci->is_looted && !item.is_looted);
            if (personal)
                canLoot = !item.is_looted;

            if (canLoot)
            {
                WorldPackets::Loot::LootItem lootItem;
                lootItem.LootListID = item.LootListID ? item.LootListID : ci->index+1;
                lootItem.Quantity = item.count;
                lootItem.Loot.Initialize(item);

                if (item.follow_loot_rules)
                {
                    switch (permission)
                    {
                    case MASTER_PERMISSION:
                        lootItem.UIType = LOOT_ITEM_UI_MASTER;
                        break;
                    case RESTRICTED_PERMISSION:
                        lootItem.UIType = item.is_blocked ? LOOT_ITEM_UI_LOCKED : LOOT_ITEM_UI_NORMAL;
                        break;
                    case GROUP_PERMISSION:
                        if (!item.is_blocked)
                            lootItem.UIType = LOOT_ITEM_UI_NORMAL;
                        else
                            lootItem.UIType = LOOT_ITEM_UI_ROLL;
                        break;
                    default:
                        lootItem.UIType = LOOT_ITEM_UI_NORMAL;
                        break;
                    }
                }
                else
                    lootItem.UIType = LOOT_ITEM_UI_NORMAL;

                packet.Items.push_back(lootItem);
            }
        }
    }

    QuestItemMap const& lootPlayerCurrencies = GetPlayerCurrencies();
    QuestItemMap::const_iterator currency_itr = lootPlayerCurrencies.find(viewer->GetGUIDLow());
    if (currency_itr != lootPlayerCurrencies.end())
    {
        QuestItemList* currency_list = currency_itr->second;
        for (QuestItemList::const_iterator ci = currency_list->begin() ; ci != currency_list->end(); ++ci)
        {
            LootItem const& item = items[ci->index];
            WorldPackets::Loot::LootCurrency lootCurrency;
            bool canLoot = (!ci->is_looted && !item.is_looted);
            if (personal)
                canLoot = !item.is_looted;

            if (canLoot)
            {
                lootCurrency.CurrencyID = item.item.ItemID;
                lootCurrency.Quantity = item.count;
                lootCurrency.LootListID = item.LootListID ? item.LootListID : ci->index+1;
                lootCurrency.UIType = LOOT_ITEM_UI_NORMAL;
                packet.Currencies.push_back(lootCurrency);
            }
        }
    }

    QuestItemMap const& lootPlayerQuestItems = GetPlayerQuestItems();

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::BuildLootResponse lootPlayerQuestItems size %i", lootPlayerQuestItems.size());

    QuestItemMap::const_iterator q_itr = lootPlayerQuestItems.find(viewer->GetGUIDLow());
    if (q_itr != lootPlayerQuestItems.end())
    {
        QuestItemList* q_list = q_itr->second;
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::BuildLootResponse QuestItemList size %i", q_list->size());

        for (QuestItemList::const_iterator qi = q_list->begin(); qi != q_list->end(); ++qi)
        {
            LootItem const& item = quest_items[qi->index];
            bool canLoot = (!qi->is_looted && !item.is_looted);
            if (personal)
                canLoot = !item.is_looted;

            // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::BuildLootResponse canLoot %i personal %u LootListID %i is_looted %u ItemID %u", canLoot, personal, item.LootListID, qi->is_looted, item.item.ItemID);

            if (canLoot)
            {
                WorldPackets::Loot::LootItem lootItem;
                lootItem.LootListID = item.LootListID ? item.LootListID : (items.size() + (qi - q_list->begin()) + 1);
                lootItem.Quantity = item.count;
                lootItem.Loot.Initialize(item);

                if (item.follow_loot_rules)
                {
                    switch (permission)
                    {
                        case MASTER_PERMISSION:
                            lootItem.UIType = LOOT_ITEM_UI_MASTER;
                            break;
                        case RESTRICTED_PERMISSION:
                            lootItem.UIType = item.is_blocked ? LOOT_ITEM_UI_LOCKED : LOOT_ITEM_UI_NORMAL;
                            break;
                        case GROUP_PERMISSION:
                            lootItem.UIType = item.is_blocked ? LOOT_ITEM_UI_LOCKED : LOOT_ITEM_UI_NORMAL;
                            if (!item.is_blocked)
                                lootItem.UIType = LOOT_ITEM_UI_NORMAL;
                            else
                                lootItem.UIType = LOOT_ITEM_UI_ROLL;
                            break;
                        default:
                            lootItem.UIType = LOOT_ITEM_UI_NORMAL;
                            break;
                    }
                }
                else
                   lootItem.UIType = LOOT_ITEM_UI_NORMAL;

                packet.Items.push_back(lootItem);
            }
        }
    }

    return;
}

void Loot::AddLegendaryItemToDrop()
{
    Map* map = m_lootOwner->GetMap();

    if (_ExpansionID != EXPANSION_LEGION || !m_lootOwner->CanContact()) // calculate only for legion map
        return;

    float chance = m_lootOwner->CalculateLegendaryDropChance(rateLegendary);

    if (!roll_chance_f(chance))
        return;

    if (uint32 itemID = sObjectMgr->GetRandomLegendaryItem(m_lootOwner))
    {
        sLog->outWarden("Player %s on map %u with killpoints %f and chance %f looted legendary item %u from source: guid (high: " UI64FMTDX ", low: " UI64FMTDX ")", m_lootOwner->GetSession()->GetPlayerName(false).c_str(), m_lootOwner->GetMapId(), m_lootOwner->m_killPoints, chance, itemID, uint8(LootSourceGuid.GetHigh()), LootSourceGuid.GetLowPart());
        LootStoreItem item = LootStoreItem(itemID, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1);
        AddItem(item);
        m_lootOwner->m_killPoints = 0.0f;
    }
}

void Loot::AddLegendaryItem()
{
    if (uint32 itemID = sObjectMgr->GetRandomLegendaryItem(m_lootOwner))
    {
        sLog->outWarden("Player %s on map %u with killpoints %f looted legendary item %u from source: guid (high: " UI64FMTDX ", low: " UI64FMTDX ")", m_lootOwner->GetSession()->GetPlayerName(false).c_str(), m_lootOwner->GetMapId(), m_lootOwner->m_killPoints, itemID, uint8(LootSourceGuid.GetHigh()), LootSourceGuid.GetLowPart());
        LootStoreItem item = LootStoreItem(itemID, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1);
        AddItem(item);
    }
}

bool Loot::IsLastEncounter()
{
    if (objGuid.IsCreature())
    {
        switch (objGuid.GetEntry())
        {
            case 98970:
            case 96028:
            case 91007:
            case 95888:
            case 104218:
            case 99192:
            case 98208:
            case 114790:
            case 124729:
            case 116944:
            case 102387:
            case 102446:
                return true;
            default:
                break;
        }
    }

    if (objGuid.IsGameObject())
    {
        switch (objGuid.GetEntry())
        {
            case 246036:
            case 245847:
                return true;
            default:
                break;
        }
    }
    return false;
}

// Basic checks for player/item compatibility - if false no chance to see the item in the loot
bool Loot::AllowedForPlayer(Player const* player, uint32 ItemID, uint8 type, bool needs_quest, LootItem* lootItem) const
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::AllowedForPlayer ItemID %i type %u", ItemID, type);

    if (lootItem)
        if (!lootItem->AllowedForPlayer(player))
            return false;

    if (type == LOOT_ITEM_TYPE_ITEM)
    {
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(ItemID);
        if (!pProto || !player)
            return false;

        // not show loot for players without profession or those who already know the recipe
        if (pProto->GetFlags() & ITEM_FLAG_HIDE_UNUSABLE_RECIPE)
        {
            if (!const_cast<Player*>(player)->HasSkill(pProto->GetRequiredSkill()))
                return false;
            if (pProto->Effects.size() >= 2 && pProto->Effects[1])
            {
                if (const_cast<Player*>(player)->HasSpell(pProto->Effects[1]->SpellID))
                    return false;

                if (SkillLineAbilityEntry const* skillline = sDB2Manager.GetSkillBySpell(pProto->Effects[1]->SpellID))
                {
                    if (skillline->SupercedesSpell && !const_cast<Player*>(player)->HasSpell(skillline->SupercedesSpell))
                        return false;
                    if (skillline->MinSkillLineRank > const_cast<Player*>(player)->GetSkillValue(pProto->GetRequiredSkill()))
                        return false;
                }
            }
        }

        // not show loot for not own team
        if ((pProto->GetFlags2() & ITEM_FLAG2_FACTION_HORDE) && player->GetTeam() != HORDE)
            return false;

        if ((pProto->GetFlags2() & ITEM_FLAG2_FACTION_ALLIANCE) && player->GetTeam() != ALLIANCE)
            return false;

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::AllowedForPlayer needs_quest %u ItemID %i HasQuestForItem %u", needs_quest, ItemID, player->HasQuestForItem(ItemID));

        // check quest requirements
        if (!(pProto->FlagsCu & ITEM_FLAGS_CU_IGNORE_QUEST_STATUS) && ((needs_quest || (pProto->GetStartQuestID() && player->GetQuestStatus(pProto->GetStartQuestID()) != QUEST_STATUS_NONE)) && !player->HasQuestForItem(ItemID)))
            return false;

        //! GARR_BTYPE_WARMILL support.
        if (ItemID == 113681)
        {
            if (Garrison * gar = const_cast<Player*>(player)->GetGarrison())
                if (gar->GetPlotWithBuildingType(GARR_BTYPE_WARMILL))
                    return true;
            return false;
        }
    }
    else if (type == LOOT_ITEM_TYPE_CURRENCY)
    {
        CurrencyTypesEntry const * currency = sCurrencyTypesStore.LookupEntry(ItemID);
        if (!currency)
            return false;

        if (!player->isGameMaster())
            if (currency->CategoryID == CURRENCY_CATEGORY_ARCHAEOLOGY && !const_cast<Player*>(player)->HasSkill(SKILL_ARCHAEOLOGY))
                return false;
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Loot::AllowedForPlayer ItemID %i allowed true", ItemID);

    return true;
}

//
// --------- LootTemplate::LootGroup ---------
//

// Adds an entry to the group (at loading stage)
void LootTemplate::LootGroup::AddEntry(LootStoreItem& item)
{
    if (item.chance != 0)
        ExplicitlyChanced.push_back(item);
    else
        EqualChanced.push_back(item);
}

// Rolls an item from the group, returns NULL if all miss their chances
LootStoreItem const* LootTemplate::LootGroup::Roll() const
{
    if (!ExplicitlyChanced.empty())                             // First explicitly chanced entries are checked
    {
        float Roll = (float)rand_chance();

        for (size_t i = 0; i < ExplicitlyChanced.size(); ++i)   // check each explicitly chanced entry in the template and modify its chance based on quality.
        {
            if (ExplicitlyChanced[i].chance >= 100.0f)
                return &ExplicitlyChanced[i];

            Roll -= ExplicitlyChanced[i].chance;
            if (Roll < 0)
                return &ExplicitlyChanced[i];
        }
    }
    if (!EqualChanced.empty())                              // If nothing selected yet - an item is taken from equal-chanced part
        return &EqualChanced[irand(0, EqualChanced.size()-1)];

    return nullptr;                                            // Empty drop from the group
}

// True if group includes at least 1 quest drop entry
bool LootTemplate::LootGroup::HasQuestDrop() const
{
    for (LootStoreItemList::const_iterator i=ExplicitlyChanced.begin(); i != ExplicitlyChanced.end(); ++i)
        if (i->needs_quest)
            return true;
    for (LootStoreItemList::const_iterator i=EqualChanced.begin(); i != EqualChanced.end(); ++i)
        if (i->needs_quest)
            return true;
    return false;
}

// True if group includes at least 1 quest drop entry for active quests of the player
bool LootTemplate::LootGroup::HasQuestDropForPlayer(Player const* player) const
{
    for (LootStoreItemList::const_iterator i = ExplicitlyChanced.begin(); i != ExplicitlyChanced.end(); ++i)
        if (player->HasQuestForItem(i->itemid))
            return true;
    for (LootStoreItemList::const_iterator i = EqualChanced.begin(); i != EqualChanced.end(); ++i)
        if (player->HasQuestForItem(i->itemid))
            return true;
    return false;
}

void LootTemplate::LootGroup::CopyConditions(ConditionList /*conditions*/)
{
    for (LootStoreItemList::iterator i = ExplicitlyChanced.begin(); i != ExplicitlyChanced.end(); ++i)
    {
        i->conditions.clear();
    }
    for (LootStoreItemList::iterator i = EqualChanced.begin(); i != EqualChanced.end(); ++i)
    {
        i->conditions.clear();
    }
}

// Rolls an item from the group (if any takes its chance) and adds the item to the loot
void LootTemplate::LootGroup::Process(Loot& loot, LootTemplate const* tab) const
{
    // build up list of possible drops
    LootStoreItemList EqualPossibleDrops = EqualChanced;
    LootStoreItemList ExplicitPossibleDrops = ExplicitlyChanced;

    uint8 uiAttemptCount = 0;
    Player const* lootOwner = loot.GetLootOwner();
    const uint8 uiMaxAttempts = ExplicitlyChanced.size() + EqualChanced.size();

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::Process EqualPossibleDrops %i ExplicitPossibleDrops %i", EqualPossibleDrops.size(), ExplicitPossibleDrops.size());

    while (!ExplicitPossibleDrops.empty() || !EqualPossibleDrops.empty())
    {
        if (uiAttemptCount == uiMaxAttempts)             // already tried rolling too many times, just abort
            return;

        LootStoreItem* item = nullptr;

        // begin rolling (normally called via Roll())
        LootStoreItemList::iterator itr;
        uint8 itemSource = 0;
        if (!ExplicitPossibleDrops.empty())              // First explicitly chanced entries are checked
        {
            itemSource = 1;
            float Roll = (float)rand_chance();
            // check each explicitly chanced entry in the template and modify its chance based on quality
            for (itr = ExplicitPossibleDrops.begin(); itr != ExplicitPossibleDrops.end(); itr = ExplicitPossibleDrops.erase(itr))
            {
                if (itr->chance >= 100.0f)
                {
                    item = &*itr;
                    break;
                }

                Roll -= itr->chance;
                if (Roll < 0)
                {
                    item = &*itr;
                    break;
                }
            }
        }
        if (item == nullptr && !EqualPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
        {
            itemSource = 2;
            itr = EqualPossibleDrops.begin();
            std::advance(itr, irand(0, EqualPossibleDrops.size()-1));
            item = &*itr;
        }
        // finish rolling

        ++uiAttemptCount;

        if (item != nullptr)   // only add this item if roll succeeds and the mode matches
        {
            bool needErase = false;
            if (item->lootmode && !(item->lootmode & loot._DifficultyMask)) // Do not add if instance mode mismatch
                needErase = true;

            if (item->ClassificationMask && !(item->ClassificationMask & loot._ClassificationMask))
                needErase = true;

            if (needErase)
            {
                switch (itemSource)
                {
                    case 1: // item came from ExplicitPossibleDrops
                        ExplicitPossibleDrops.erase(itr);
                        break;
                    case 2: // item came from EqualPossibleDrops
                        EqualPossibleDrops.erase(itr);
                        break;
                }
                continue;
            }

            bool duplicate = false;
            if (!tab->CheckItemCondition(lootOwner, item->itemid, item->type))
                duplicate = true;

            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(item->itemid))
            {
                if (tab->_isZoneLoot)
                    if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                        duplicate = true;

                uint8 _item_counter = 0;
                for (LootItemList::const_iterator _item = loot.items.begin(); _item != loot.items.end(); ++_item)
                    if (_item->item.ItemID == item->itemid)                             // search through the items that have already dropped
                    {
                        ++_item_counter;
                        if (_proto->GetInventoryType() == 0 && _item_counter == 3)      // Non-equippable items are limited to 3 drops
                            duplicate = true;
                        else if (_proto->GetInventoryType() != 0 && _item_counter == 1) // Equippable item are limited to 1 drop
                            duplicate = true;
                    }
            }
            if (duplicate) // if item->itemid is a duplicate, remove it
                switch (itemSource)
                {
                    case 1: // item came from ExplicitPossibleDrops
                        ExplicitPossibleDrops.erase(itr);
                        break;
                    case 2: // item came from EqualPossibleDrops
                        EqualPossibleDrops.erase(itr);
                        break;
                }
            else           // otherwise, add the item and exit the function
            {
                if(item->shared) //shared very low chance to and one item
                {
                    if(!roll_chance_f(0.5f))
                    {
                        switch (itemSource)
                        {
                            case 1: // item came from ExplicitPossibleDrops
                                ExplicitPossibleDrops.erase(itr);
                                break;
                            case 2: // item came from EqualPossibleDrops
                                EqualPossibleDrops.erase(itr);
                                break;
                        }
                        continue;
                    }
                }
                loot.AddItem(*item);
                return;
            }
        }
    }
}

// Rolls an item from the group (if any takes its chance) and adds the item to the loot
void LootTemplate::LootGroup::ProcessAutoGroup(Loot& loot, LootTemplate const* tab) const
{
    // build up list of possible drops
    LootStoreItemList EqualPossibleDrops = EqualChanced;

    uint8 uiAttemptCount = 0;
    uint8 uiCountAdd = 0;
    bool uiShared = false;
    Player const* lootOwner = loot.GetLootOwner();
    uint8 uiDropCount = sObjectMgr->GetCountFromSpawn(loot._DifficultyID, loot._ExpansionID);
    const uint8 uiMaxAttempts = EqualChanced.size();

    if (loot.IsLastEncounter())
        if (lootOwner->HasAura(225787)) // Sign of the Warrior
            if(roll_chance_f(20.0f))
                uiDropCount++;

    loot.AddLegendaryItemToDrop(); //Generate Loot Legendary Item

    while (!EqualPossibleDrops.empty())
    {
        if (uiAttemptCount == uiMaxAttempts)             // already tried rolling too many times, just abort
            return;

        LootStoreItem* item = nullptr;

        // begin rolling (normally called via Roll())
        LootStoreItemList::iterator itr;

        if (!EqualPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
        {
            itr = EqualPossibleDrops.begin();
            std::advance(itr, irand(0, EqualPossibleDrops.size()-1));
            item = &*itr;
        }
        // finish rolling

        ++uiAttemptCount;

        if (!tab->CheckItemCondition(lootOwner, item->itemid, item->type))
        {
            EqualPossibleDrops.erase(itr);
            continue;
        }

        if (item != nullptr)   // only add this item if roll succeeds and the mode matches
        {
            if (!(item->lootmode & loot._DifficultyMask) || (uiShared && item->shared))                          // Do not add if instance mode mismatch
            {
                EqualPossibleDrops.erase(itr);
                continue;
            }

            bool duplicate = false;
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(item->itemid))
            {
                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    duplicate = true;

                uint8 _item_counter = 0;
                for (LootItemList::const_iterator _item = loot.items.begin(); _item != loot.items.end(); ++_item)
                    if (_item->item.ItemID == item->itemid)                             // search through the items that have already dropped
                    {
                        ++_item_counter;
                        if (_proto->GetInventoryType() == 0 && _item_counter == 3)      // Non-equippable items are limited to 3 drops
                            duplicate = true;
                        else if (_proto->GetInventoryType() != 0 && _item_counter == 1) // Equippable item are limited to 1 drop
                            duplicate = true;
                    }

                if (_proto->GetQuality() == ITEM_QUALITY_LEGENDARY) // Custom chance for Legendary item
                {
                    if(!roll_chance_f(item->chance) || _proto->GetExpansion() == EXPANSION_LEGION)
                        duplicate = true;
                }
            }
            if(item->shared) //shared very low chance to and one item
            {
                if(roll_chance_f(0.5f))
                    uiShared = true;
                else
                {
                    EqualPossibleDrops.erase(itr);
                    continue;
                }
            }
            if (duplicate) // if item->itemid is a duplicate, remove it
                EqualPossibleDrops.erase(itr);
            else           // otherwise, add the item and exit the function
            {
                loot.AddItem(*item);
                uiCountAdd++;
                if(uiDropCount <= uiCountAdd)
                    return;
            }
        }
    }
}

void LootTemplate::LootGroup::ProcessItemLoot(Loot& loot, LootTemplate const* tab) const
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::LootGroup::ProcessItemLoot EqualChanced %u", EqualChanced.size());

    Player const* lootOwner = loot.GetLootOwner();
    LootStoreItemList ItemPossibleDrops;
    LootStoreItemList APPossibleDrops;

    for (LootStoreItemList::const_iterator i = EqualChanced.begin(); i != EqualChanced.end(); ++i)
    {
        bool _delete = false;
        if (!tab->CheckItemCondition(lootOwner, i->itemid, i->type))
            _delete = true;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if (i->needs_quest || _proto->IsOtherDrops() || _proto->IsRecipe())
                {
                    _delete = true;
                    APPossibleDrops.push_back(*i);
                }

                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    _delete = true;
            }
            else
                _delete = true;
        }
        if (i->type == LOOT_ITEM_TYPE_CURRENCY)
        {
            APPossibleDrops.push_back(*i);
            _delete = true;
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
            _delete = true;

        if (!_delete)
            ItemPossibleDrops.push_back(*i);
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::LootGroup::ProcessItemLoot ItemPossibleDrops %u", ItemPossibleDrops.size());

    if (!ItemPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
    {
        LootStoreItemList::iterator itr = ItemPossibleDrops.begin();
        std::advance(itr, irand(0, ItemPossibleDrops.size()-1));
        LootStoreItem* item = &*itr;
        loot.AddItem(*item);
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::LootGroup::ProcessItemLoot APPossibleDrops %u", APPossibleDrops.size());

    for (LootStoreItemList::const_iterator i = APPossibleDrops.begin(); i != APPossibleDrops.end(); ++i)
    {
        if (!i->Roll(false))
            continue;                                         // Bad luck for the entry

        loot.AddItem(*i);                                 // Chance is already checked, just add
    }
}

void LootTemplate::LootGroup::ProcessWorld(Loot& loot, LootTemplate const* tab, bool ignore) const
{
    Player const* lootOwner = loot.GetLootOwner();
    LootStoreItemList ItemPossibleDrops;
    LootStoreItemList EqualPossibleDrops = EqualChanced;
    std::copy(ExplicitlyChanced.begin(), ExplicitlyChanced.end(), std::back_inserter(EqualPossibleDrops));

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessWorld EqualPossibleDrops %u ignore %u", EqualPossibleDrops.size(), ignore);

    for (LootStoreItemList::const_iterator i = EqualPossibleDrops.begin(); i != EqualPossibleDrops.end(); ++i)
    {
        bool _delete = false;
        if (!ignore && !tab->CheckItemCondition(lootOwner, i->itemid, i->type))
            _delete = true;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    _delete = true;
            }
            else
                _delete = true;
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
            _delete = true;

        if (!_delete)
            ItemPossibleDrops.push_back(*i);
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessWorld ItemPossibleDrops %u ignore %u", ItemPossibleDrops.size(), ignore);

    if (!ItemPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
    {
        LootStoreItemList::iterator itr = ItemPossibleDrops.begin();
        std::advance(itr, irand(0, ItemPossibleDrops.size()-1));
        LootStoreItem* item = &*itr;
        loot.AddItem(*item);
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessWorld AddItem itemid %i", itr->itemid);
    }
}

// Rolls an item from the group (if any takes its chance) and adds the item to the loot
bool LootTemplate::LootGroup::ProcessBossLoot(Loot& loot, LootTemplate const* tab) const
{
    // build up list of possible drops
    LootStoreItemList EqualPossibleDrops = EqualChanced;
    std::map<uint8, LootStoreItemList> PossibleDrops;
    std::copy(ExplicitlyChanced.begin(), ExplicitlyChanced.end(), std::back_inserter(EqualPossibleDrops));

    uint8 uiAttemptCount = 0;
    uint8 uiCountAdd = 0;
    Player const* lootOwner = loot.GetLootOwner();
    const uint8 uiMaxAttempts = EqualPossibleDrops.size();

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessBossLoot EqualChanced %i ExplicitlyChanced %i", uiMaxAttempts, ExplicitlyChanced.size());

    loot.AddLegendaryItemToDrop(); //Generate Loot Legendary Item

    for (LootStoreItemList::const_iterator i = EqualPossibleDrops.begin(); i != EqualPossibleDrops.end(); ++i)
    {
        if (i->lootmode != 0 && !(i->lootmode & loot._DifficultyMask)) // Do not add if instance mode mismatch
            continue;

        if (!tab->CheckItemCondition(lootOwner, i->itemid, i->type))
            continue;

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessBossLoot itemid %u chance %f lootmode %u", i->itemid, i->chance, i->lootmode);

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if (_proto->IsOtherDrops() || _proto->IsRecipe() || i->needs_quest)
                    continue;

                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    continue;

                PossibleDrops[_proto->GetInventoryType()].push_back(*i);
            }
        }
    }

    while (!PossibleDrops.empty())
    {
        if (uiAttemptCount == uiMaxAttempts)             // already tried rolling too many times, just abort
            return false;

        std::map<uint8, LootStoreItemList>::iterator iter = PossibleDrops.begin();
        std::advance(iter, irand(0, PossibleDrops.size()-1));

        LootStoreItem* item = nullptr;

        // begin rolling (normally called via Roll())
        LootStoreItemList::iterator itr;

        if (!iter->second.empty()) // If nothing selected yet - an item is taken from equal-chanced part
        {
            itr = iter->second.begin();
            std::advance(itr, irand(0, iter->second.size()-1));
            item = &*itr;
        }
        else
        {
            PossibleDrops.erase(iter);
            continue;
        }
        // finish rolling

        ++uiAttemptCount;

        if (item)
        {
            loot.AddItem(*item);
            return true;
        }
    }

    return false;
}

// Rolls an item from the group (if any takes its chance) and adds the item to the loot
bool LootTemplate::LootGroup::ProcessRareOrGoLoot(Loot& loot, LootTemplate const* tab) const
{
    // build up list of possible drops
    LootStoreItemList EqualPossibleDrops = EqualChanced;
    std::copy(ExplicitlyChanced.begin(), ExplicitlyChanced.end(), std::back_inserter(EqualPossibleDrops));

    uint8 uiAttemptCount = 0;
    Player const* lootOwner = loot.GetLootOwner();
    const uint8 uiMaxAttempts = EqualPossibleDrops.size();

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessRareOrGoLoot EqualPossibleDrops %i ExplicitlyChanced %i", uiMaxAttempts, ExplicitlyChanced.size());

    loot.AddLegendaryItemToDrop(); //Generate Loot Legendary Item

    while (!EqualPossibleDrops.empty())
    {
        if (uiAttemptCount == uiMaxAttempts)             // already tried rolling too many times, just abort
            return false;

        LootStoreItem* item = nullptr;

        // begin rolling (normally called via Roll())
        LootStoreItemList::iterator itr;

        if (!EqualPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
        {
            itr = EqualPossibleDrops.begin();
            std::advance(itr, irand(0, EqualPossibleDrops.size()-1));
            item = &*itr;
        }
        // finish rolling

        ++uiAttemptCount;

        if (!item)
        {
            EqualPossibleDrops.erase(itr);
            continue;
        }

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessRareOrGoLoot itemid %i type %u", item->itemid, item->type);

        if (!tab->CheckItemCondition(lootOwner, item->itemid, item->type))
        {
            EqualPossibleDrops.erase(itr);
            continue;
        }

        if (item->lootmode != 0 && !(item->lootmode & loot._DifficultyMask))                          // Do not add if instance mode mismatch
        {
            // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessRareOrGoLoot lootmode %i _DifficultyMask %i", item->lootmode, loot._DifficultyMask);
            EqualPossibleDrops.erase(itr);
            continue;
        }

        if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(item->itemid))
        {
            if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
            {
                EqualPossibleDrops.erase(itr);
                continue;
            }
            // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessRareOrGoLoot lootmode %i ItemSpecExist %u itemid %i", item->lootmode, _proto->ItemSpecExist, item->itemid);
        }
        else
        {
            // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessRareOrGoLoot AllowableClass %i", _proto->AllowableClass);
            EqualPossibleDrops.erase(itr);
            continue;
        }

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootGroup::ProcessRareOrGoLoot AddItem itemid %i", item->itemid);

        // otherwise, add the item
        loot.AddItem(*item);
        return true;
    }

    return false;
}

// Overall chance for the group without equal chanced items
float LootTemplate::LootGroup::RawTotalChance() const
{
    float result = 0;

    for (LootStoreItemList::const_iterator i=ExplicitlyChanced.begin(); i != ExplicitlyChanced.end(); ++i)
        if (!i->needs_quest)
            result += i->chance;

    return result;
}

// Overall chance for the group
float LootTemplate::LootGroup::TotalChance() const
{
    float result = RawTotalChance();

    if (!EqualChanced.empty() && result < 100.0f)
        return 100.0f;

    return result;
}

void LootTemplate::LootGroup::Verify(LootStore const& lootstore, uint32 id, uint8 group_id) const
{
    float chance = RawTotalChance();
    if (chance > 101.0f)                                    // TODO: replace with 100% when DBs will be ready
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %u group %d has total chance > 100%% (%f)", lootstore.GetName(), id, group_id, chance);
    }

    if (chance >= 100.0f && !EqualChanced.empty())
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' entry %u group %d has items with chance=0%% but group total chance >= 100%% (%f)", lootstore.GetName(), id, group_id, chance);
    }
}

void LootTemplate::LootGroup::CheckLootRefs(LootTemplateMap const& /*store*/, LootIdSet* ref_set) const
{
    for (LootStoreItemList::const_iterator ieItr=ExplicitlyChanced.begin(); ieItr != ExplicitlyChanced.end(); ++ieItr)
    {
        if (ieItr->mincountOrRef < 0)
        {
            if (!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if (ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }

    for (LootStoreItemList::const_iterator ieItr=EqualChanced.begin(); ieItr != EqualChanced.end(); ++ieItr)
    {
        if (ieItr->mincountOrRef < 0)
        {
            if (!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if (ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }
}

//
// --------- LootTemplate ---------
//

// Adds an entry to the group (at loading stage)
void LootTemplate::AddEntry(LootStoreItem& item)
{
    if (item.group > 0 && item.mincountOrRef > 0)           // Group
    {
        if (item.group >= Groups.size())
            Groups.resize(item.group);                      // Adds new group the the loot template if needed
        Groups[item.group-1].AddEntry(item);                // Adds new entry to the group
    }
    else if (item.group == 0 && item.mincountOrRef > 0 && item.chance == 0)           // Extra Group for auto loot
    {
        uint32 group = 1;
        if (group >= AutoGroups.size())
            AutoGroups.resize(group);                      // Adds new group the the loot template if needed
        AutoGroups[group-1].AddEntry(item);                // Adds new entry to the group
    }
    else                                                    // Non-grouped entries and references are stored together
    {
        if (item.mincountOrRef < 0)
        {
            Entries.push_back(item); // References do not require a valid itemId
        }
        else if(item.type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(item.itemid))
            {
                bool isResurse = _proto->GetClass() == ITEM_CLASS_MISCELLANEOUS || _proto->GetClass() == ITEM_CLASS_QUEST || _proto->IsCraftingReagent() || _proto->GetQuality() < ITEM_QUALITY_UNCOMMON || _proto->IsRecipe();
                if (_proto->GetItemNameDescriptionID() == 13219 && item.chance < 100 && !item.needs_quest)
                {
                    uint32 group = 1;
                    if (group >= PersonalGroups.size())
                        PersonalGroups.resize(group);
                    PersonalGroups[group-1].AddEntry(item);
                }
                else if (_proto->GetQuality() == ITEM_QUALITY_RARE && item.chance < 100 && !item.needs_quest && !isResurse)
                {
                    uint32 group = 2;
                    if (group >= PersonalGroups.size())
                        PersonalGroups.resize(group);
                    PersonalGroups[group-1].AddEntry(item);
                }
                Entries.push_back(item);
            }
        }
        else if(item.type == LOOT_ITEM_TYPE_CURRENCY)
        {
            if (item.itemid == 1220 && item.chance < 100) // Order Resources for rare or go have custom group
            {
                uint32 group = 3;
                if (group >= PersonalGroups.size())
                    PersonalGroups.resize(group);
                PersonalGroups[group-1].AddEntry(item);
                // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::AddEntry LOOT_ITEM_TYPE_CURRENCY 1220 group %i PersonalGroups %i", group, PersonalGroups.size());
            }
            Entries.push_back(item);
        }
        else
            Entries.push_back(item); // If refferrens Id not item
    }
}

void LootTemplate::CopyConditions(ConditionList conditions)
{
    for (LootStoreItemList::iterator i = Entries.begin(); i != Entries.end(); ++i)
        i->conditions.clear();

    for (LootGroups::iterator i = Groups.begin(); i != Groups.end(); ++i)
        i->CopyConditions(conditions);
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::Process(Loot& loot, bool rate, uint8 groupId) const
{
    if (groupId)                                            // Group reference uses own processing of the group
    {
        if (groupId > Groups.size())
            return;                                         // Error message already printed at loading stage

        Groups[groupId-1].Process(loot, this);
        return;
    }

    Player const* lootOwner = loot.GetLootOwner();

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::Process isBoss %i _DifficultyMask %i loot.chance %u bonusLoot %i Entries %u AutoGroups %u Groups %u", loot.isBoss, loot._DifficultyMask, loot.chance, loot.bonusLoot, Entries.size(), AutoGroups.size(), Groups.size());

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->lootmode != 0 && !(i->lootmode & loot._DifficultyMask))    // Do not add if instance mode mismatch
            continue;

        if (i->ClassificationMask && !(i->ClassificationMask & loot._ClassificationMask))
            continue;

        if (!i->Roll(rate, lootOwner->GetMap()->IsDungeon(), _isZoneLoot))
            continue;                                         // Bad luck for the entry

        if (!CheckItemCondition(lootOwner, i->itemid, i->type))
            continue;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if (_isZoneLoot)
                    if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                        continue;

                uint8 _item_counter = 0;
                LootItemList::const_iterator _item = loot.items.begin();
                for (; _item != loot.items.end(); ++_item)
                    if (_item->item.ItemID == i->itemid)                               // search through the items that have already dropped
                    {
                        ++_item_counter;
                        if (_proto->GetInventoryType() == 0 && _item_counter == 3)     // Non-equippable items are limited to 3 drops
                            continue;
                        if (_proto->GetInventoryType() != 0 && _item_counter == 1) // Equippable item are limited to 1 drop
                            continue;
                    }
                if (_item != loot.items.end())
                    continue;
            }
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
        {
            LootTemplate const* Referenced = LootTemplates_Reference.GetLootFor(-i->mincountOrRef);
            if (!Referenced)
                continue;                                     // Error message already printed at loading stage

            uint32 maxcount = uint32(float(i->maxcount) * sWorld->getRate(RATE_DROP_ITEM_REFERENCED_AMOUNT));
            for (uint32 loop = 0; loop < maxcount; ++loop)    // Ref multiplicator
                Referenced->Process(loot, rate, i->group);
        }
        else                                                  // Plain entries (not a reference, not grouped)
            loot.AddItem(*i);                                 // Chance is already checked, just add

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::Process mincountOrRef %i itemid %i chance %f", i->mincountOrRef, i->itemid, i->chance);
    }

    // Now processing groups
    for (LootGroups::const_iterator i = AutoGroups.begin(); i != AutoGroups.end(); ++i)
        i->ProcessAutoGroup(loot, this);

    // Now processing groups
    for (LootGroups::const_iterator i = Groups.begin(); i != Groups.end(); ++i)
        i->Process(loot, this);
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::ProcessBossLoot(Loot& loot) const
{
    bool canGetInstItem = roll_chance_i(loot.chance); //Can get item or get gold

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessBossLoot isBoss %i canGetInstItem %i _DifficultyMask %i loot.chance %u bonusLoot %i", loot.isBoss, canGetInstItem, loot._DifficultyMask, loot.chance, loot.bonusLoot);

    // Now processing groups
    if (canGetInstItem)
    {
        for (LootGroups::const_iterator i = AutoGroups.begin(); i != AutoGroups.end(); ++i)
            i->ProcessBossLoot(loot, this);
    }

    Player const* lootOwner = loot.GetLootOwner();
    if (loot.bonusLoot)
    {
        if(!canGetInstItem)
        {
            if (loot._ExpansionID == EXPANSION_LEGION)
            {
                LootStoreItem item = LootStoreItem(lootOwner->GetMap()->IsRaid() ? 147581 : 138786, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1);
                loot.AddItem(item);
            }
            else
                loot.generateMoneyLoot(150000, 500000, lootOwner->GetMap()->IsDungeon()); //Generate money if loot not roll
        }
        return;
    }

    if (!canGetInstItem && lootOwner->GetMap()->IsRaid())
        return;

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->lootmode != 0 && !(i->lootmode & loot._DifficultyMask)) // Do not add if instance mode mismatch
            continue;

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessBossLoot itemid %u chance %f lootmode %u lootmode %u", i->itemid, i->chance, i->lootmode, i->lootmode);

        if (!CheckItemCondition(lootOwner, i->itemid, i->type))
            continue;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    continue;

                if (!i->Roll(false))
                    continue;                                         // Bad luck for the entry

                loot.AddItem(*i);
            }
        }
        if (i->type == LOOT_ITEM_TYPE_CURRENCY)
        {
            if (!i->Roll(false))
                continue;

            loot.AddItem(*i);
        }

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessBossLoot AddItem itemid %i", i->itemid);
    }
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::ProcessRareOrGoLoot(Loot& loot) const
{
    Player const* lootOwner = loot.GetLootOwner();

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->lootmode != 0 && !(i->lootmode & loot._DifficultyMask)) // Do not add if instance mode mismatch
            continue;

        if (!CheckItemCondition(lootOwner, i->itemid, i->type))
            continue;

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessRareOrGoLoot itemid %u chance %f lootmode %u", i->itemid, i->chance, i->lootmode);

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    continue;

                if (_proto->GetItemNameDescriptionID() == 13219 && i->chance < 100) // Only CA delete
                    continue;

                bool isResurse = _proto->GetClass() == ITEM_CLASS_MISCELLANEOUS || _proto->GetClass() == ITEM_CLASS_QUEST || _proto->IsCraftingReagent() || _proto->GetQuality() < ITEM_QUALITY_UNCOMMON || _proto->IsRecipe();
                if (!isResurse && _proto->GetQuality() == ITEM_QUALITY_RARE && i->chance < 100)
                    continue;

                if (loot.isRareNext && _proto->GetQuality() > ITEM_QUALITY_RARE) // Prevent farm epic item
                    continue;

                if (!i->Roll(false))
                    continue;
            }
            else
                continue;
        }
        else if (i->type == LOOT_ITEM_TYPE_CURRENCY)
        {
            if (loot.isRareNext)
                continue;

            if (i->itemid == 1220 && i->chance < 100) // Order Resources for rare or go have custom group
                continue;

            if (!i->Roll(false))
                continue;

            loot.AddItem(*i);
            continue;
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
            continue;
            // Plain entries (not a reference, not grouped)
        loot.AddItem(*i);
        // Chance is already checked, just add

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessRareOrGoLoot AddItem itemid %i", i->itemid);
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessRareOrGoLoot PersonalGroups %i", PersonalGroups.size());

    if (!loot.isRareNext)
        for (LootGroups::const_iterator i = PersonalGroups.begin(); i != PersonalGroups.end(); ++i)
            i->ProcessRareOrGoLoot(loot, this);
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::ProcessWorld(Loot& loot, bool rate, bool ignore) const
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessWorld _ExpansionID %u _DifficultyMask %i", loot._ExpansionID, loot._DifficultyMask);

    Player const* lootOwner = loot.GetLootOwner();

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessWorld itemid %u chance %f lootmode %u _ExpansionID %u", i->itemid, i->chance, i->lootmode, loot._ExpansionID);

        if (i->lootmode != 0 && !(i->lootmode & loot._DifficultyMask)) // Do not add if instance mode mismatch
            continue;

        if (!ignore)
        {
            // Don`t check condition for Referenced loot
            if (!CheckItemCondition(lootOwner, i->itemid, i->type))
                continue;
        }

        if (!i->Roll(rate))
            continue;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    continue;

                uint8 _item_counter = 0;
                LootItemList::const_iterator _item = loot.items.begin();
                for (; _item != loot.items.end(); ++_item)
                    if (_item->item.ItemID == i->itemid)                               // search through the items that have already dropped
                    {
                        ++_item_counter;
                        if (_proto->GetInventoryType() == 0 && _item_counter == 3)     // Non-equippable items are limited to 3 drops
                            continue;
                        if (_proto->GetInventoryType() != 0 && _item_counter == 1) // Equippable item are limited to 1 drop
                            continue;
                    }
                if (_item != loot.items.end())
                    continue;
            }
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
        {
            LootTemplate const* Referenced = LootTemplates_Reference.GetLootFor(-i->mincountOrRef);
            if (!Referenced)
                continue;                                     // Error message already printed at loading stage

            for (uint32 loop = 0; loop < i->maxcount; ++loop)    // Ref multiplicator
                Referenced->ProcessWorld(loot, rate, true);
        }
        else                                                  // Plain entries (not a reference, not grouped)
            loot.AddItem(*i);                                 // Chance is already checked, just add

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessWorld AddItem itemid %i", i->itemid);
    }

    // Now processing groups
    for (LootGroups::const_iterator i = Groups.begin(); i != Groups.end(); ++i)
        i->ProcessWorld(loot, this, ignore);

    // Rolling grouped items
    for (LootGroups::const_iterator i = AutoGroups.begin(); i != AutoGroups.end(); ++i)
        i->ProcessWorld(loot, this, ignore);
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::ProcessLuck(Loot& loot, uint32 entry, bool ignore) const
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessLuck _ExpansionID %u _DifficultyMask %i entry %u", loot._ExpansionID, loot._DifficultyMask, entry);

    Player const* lootOwner = loot.GetLootOwner();

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->lootmode != 0 && !(i->lootmode & loot._DifficultyMask)) // Do not add if instance mode mismatch
            continue;

        float chance = (i->chance * loot.rateLegendary) * (lootOwner->GetKillCreaturePoints(entry) / sWorld->getFloatConfig(CONFIG_CAP_KILL_CREATURE_POINTS));
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessLuck itemid %u chance %f lootmode %u _ExpansionID %u chance %f", i->itemid, i->chance, i->lootmode, loot._ExpansionID, chance);
        if (!ignore && !roll_chance_f(chance))
            continue;

        // Don`t check condition for Referenced loot
        if (!CheckItemCondition(lootOwner, i->itemid, i->type))
            continue;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    continue;

                uint8 _item_counter = 0;
                LootItemList::const_iterator _item = loot.items.begin();
                for (; _item != loot.items.end(); ++_item)
                    if (_item->item.ItemID == i->itemid)                               // search through the items that have already dropped
                    {
                        ++_item_counter;
                        if (_proto->GetInventoryType() == 0 && _item_counter == 3)     // Non-equippable items are limited to 3 drops
                            continue;
                        if (_proto->GetInventoryType() != 0 && _item_counter == 1) // Equippable item are limited to 1 drop
                            continue;
                    }
                if (_item != loot.items.end())
                    continue;
            }
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
        {
            LootTemplate const* Referenced = LootTemplates_Reference.GetLootFor(-i->mincountOrRef);
            if (!Referenced)
                continue;                                     // Error message already printed at loading stage

            for (uint32 loop = 0; loop < i->maxcount; ++loop)    // Ref multiplicator
                Referenced->ProcessLuck(loot, entry, true);
        }
        else                                                  // Plain entries (not a reference, not grouped)
            loot.AddItem(*i);                                 // Chance is already checked, just add

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootTemplate::ProcessLuck AddItem itemid %i", i->itemid);
    }
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::ProcessItemLoot(Loot& loot) const
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessItemLoot Entries %u AutoGroups %u PersonalGroups %u", Entries.size(), AutoGroups.size(), PersonalGroups.size());

    Player const* lootOwner = loot.GetLootOwner();
    if (loot._IsPvPLoot)
    {
        if (PvpReward* reward = sBattlegroundMgr->GetPvpReward(PvpReward_Arena_2v2))
        {
            uint32 itemID = 0;
            uint32 rating = 0;
            uint32 needLevel = 0;
            const_cast<Player*>(lootOwner)->GetPvPRatingAndLevel(reward, PvpReward_Arena_2v2, rating, needLevel, false);
            std::vector<uint32> bonusListIDs = const_cast<Player*>(lootOwner)->GetPvPRewardItem(itemID, PvpReward_Arena_2v2, rating, false, needLevel);
            if (itemID)
            {
                LootStoreItem item = LootStoreItem(itemID, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1);
                loot.AddItem(item, bonusListIDs);
            }
        }
        return;
    }

    LootStoreItemList ItemPossibleDrops;
    LootStoreItemList APPossibleDrops;

    if (loot._isEmissaryLoot || loot._itemContext == 43)
        loot.AddLegendaryItemToDrop(); //Generate Loot Legendary Item

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        bool _delete = false;
        if (!CheckItemCondition(lootOwner, i->itemid, i->type))
            _delete = true;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if (i->needs_quest || _proto->IsOtherDrops() || _proto->IsRecipe())
                {
                    _delete = true;
                    APPossibleDrops.push_back(*i);
                }

                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    _delete = true;
            }
            else
                _delete = true;
        }
        if (i->type == LOOT_ITEM_TYPE_CURRENCY)
        {
            APPossibleDrops.push_back(*i);
            _delete = true;
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
        {
            _delete = true;
            LootTemplate const* Referenced = LootTemplates_Reference.GetLootFor(-i->mincountOrRef);
            if (!Referenced)
                continue;                                     // Error message already printed at loading stage

            for (uint32 loop = 0; loop < i->maxcount; ++loop)    // Ref multiplicator
                Referenced->ProcessItemLoot(loot);
        }

        if (!_delete)
            ItemPossibleDrops.push_back(*i);
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessItemLoot ItemPossibleDrops %u APPossibleDrops %u Groups %u", ItemPossibleDrops.size(), APPossibleDrops.size(), Groups.size());

    if (!ItemPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
    {
        LootStoreItemList::iterator itr = ItemPossibleDrops.begin();
        std::advance(itr, irand(0, ItemPossibleDrops.size()-1));
        LootStoreItem* item = &*itr;
        loot.AddItem(*item);
    }

    for (LootStoreItemList::const_iterator i = APPossibleDrops.begin(); i != APPossibleDrops.end(); ++i)
    {
        if (!i->Roll(false))
            continue;                                         // Bad luck for the entry

        loot.AddItem(*i);                                 // Chance is already checked, just add
    }

    for (LootGroups::const_iterator i = Groups.begin(); i != Groups.end(); ++i)
        i->Process(loot, this);

    // Rolling grouped items
    for (LootGroups::const_iterator i = AutoGroups.begin(); i != AutoGroups.end(); ++i)
        i->ProcessItemLoot(loot, this);
}

void LootTemplate::ProcessOploteChest(Loot& loot) const
{
    Player const* lootOwner = loot.GetLootOwner();
    LootStoreItemList ItemPossibleDrops;
    LootStoreItemList OtherPossibleDrops;

    LootStoreItemList tempDrops = Entries;
    for (LootGroups::const_iterator i = AutoGroups.begin(); i != AutoGroups.end(); ++i)
    {
        std::copy(i->ExplicitlyChanced.begin(), i->ExplicitlyChanced.end(), std::back_inserter(tempDrops));
        std::copy(i->EqualChanced.begin(), i->EqualChanced.end(), std::back_inserter(tempDrops));
    }

    loot.AddLegendaryItemToDrop(); //Generate Loot Legendary Item

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = tempDrops.begin(); i != tempDrops.end(); ++i)
    {
        bool _delete = false;
        if(i->needs_quest) //Don`t add quest item
        {
            OtherPossibleDrops.push_back(*i);
            _delete = true;
        }

        if (!CheckItemCondition(lootOwner, i->itemid, i->type))
            _delete = true;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if(!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    _delete = true;

                if(_proto->GetId() == 138019)
                    _delete = true;

                if (!_delete && (_proto->IsOtherDrops() || _proto->IsRecipe()))
                {
                    _delete = true;
                    if (_proto->GetItemNameDescriptionID() != 13219) // Only CA delete from loot
                        OtherPossibleDrops.push_back(*i);
                }
            }
            else
                _delete = true;
        }

        if (i->type == LOOT_ITEM_TYPE_CURRENCY)
        {
            OtherPossibleDrops.push_back(*i);
            _delete = true;
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
            _delete = true;

        if (!_delete)
            ItemPossibleDrops.push_back(*i);
    }

    if (!ItemPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
    {
        LootStoreItemList::iterator itr = ItemPossibleDrops.begin();
        std::advance(itr, irand(0, ItemPossibleDrops.size()-1));
        LootStoreItem* item = &*itr;
        loot.AddItem(*item);
    }

    uint32 addCA = sChallengeMgr->GetCAForOplote(loot._challengeLevel);
    if (addCA)
    {
        LootStoreItem item = LootStoreItem(addCA, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1);
        loot.AddItem(item);
    }

    uint32 countBigCA = 0;
    uint32 addBigCA = sChallengeMgr->GetBigCAForOplote(loot._challengeLevel, countBigCA);
    if (addBigCA)
    {
        LootStoreItem item = LootStoreItem(addBigCA, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, countBigCA, countBigCA);
        loot.AddItem(item);
    }

    LootStoreItem item = LootStoreItem(sWorld->getIntConfig(CONFIG_CHALLENGE_ADD_ITEM), (LootItemType)sWorld->getIntConfig(CONFIG_CHALLENGE_ADD_ITEM_TYPE), 0.0f, 0, 0, sWorld->getIntConfig(CONFIG_CHALLENGE_ADD_ITEM_COUNT), sWorld->getIntConfig(CONFIG_CHALLENGE_ADD_ITEM_COUNT));
    loot.AddItem(item);

    for (LootStoreItemList::const_iterator i = OtherPossibleDrops.begin(); i != OtherPossibleDrops.end(); ++i)
    {
        if (!loot.AllowedForPlayer(lootOwner, i->itemid, i->type, i->needs_quest))
            continue;

        if (!i->Roll(false))
            continue;

        loot.AddItem(*i);
    }

    if (loot._challengeLevel > 2) // Prevent bug with 1 level key Oo
    {
        const_cast<Player*>(lootOwner)->m_challengeKeyInfo.Level = loot._challengeLevel - 1;
        loot.AddItem(LootStoreItem(138019, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1));
    }
}

void LootTemplate::ProcessChallengeChest(Loot& loot, uint32 lootId, Challenge* _challenge) const
{
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessChallengeChest Entries %u AutoGroups %u PersonalGroups %u", Entries.size(), AutoGroups.size(), PersonalGroups.size());

    Player const* lootOwner = loot.GetLootOwner();
    LootStoreItemList ItemPossibleDrops;
    LootStoreItemList OtherPossibleDrops;

    loot.AddLegendaryItemToDrop(); //Generate Loot Legendary Item

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        bool _delete = false;
        if (i->needs_quest) //Don`t add quest item
        {
            OtherPossibleDrops.push_back(*i);
            _delete = true;
        }

        if (!CheckItemCondition(lootOwner, i->itemid, i->type))
            _delete = true;

        if (i->type == LOOT_ITEM_TYPE_ITEM)
        {
            if (ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(i->itemid))
            {
                if (!lootOwner->CanGetItemForLoot(_proto, loot._specCheck))
                    _delete = true;

                if (!_delete && (_proto->IsOtherDrops() || _proto->IsRecipe()))
                {
                    _delete = true;
                    if (_proto->GetItemNameDescriptionID() != 13219) // Only CA delete from loot
                        OtherPossibleDrops.push_back(*i);
                }
            }
            else
                _delete = true;
        }
        if (i->type == LOOT_ITEM_TYPE_CURRENCY)
        {
            OtherPossibleDrops.push_back(*i);
            _delete = true;
        }

        if (i->mincountOrRef < 0 && i->type == LOOT_ITEM_TYPE_ITEM)              // References processing
            _delete = true;

        if (!_delete)
            ItemPossibleDrops.push_back(*i);
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessChallengeChest ItemPossibleDrops %i", ItemPossibleDrops.size());

    if (!ItemPossibleDrops.empty()) // If nothing selected yet - an item is taken from equal-chanced part
    {
        uint32 allItemCount = _challenge->GetItemCount(lootOwner->GetGUID());
        Trinity::Containers::RandomResizeList(ItemPossibleDrops, _challenge->GetItemCount(lootOwner->GetGUID()));

        for (auto const& item : ItemPossibleDrops)
        {
            loot.AddItem(item);
            sLog->outWarden("Player %s on map %u looted item %u from: challenge chest (Level %u, LevelBonus %u, ChallengeFinishTime %u) allItemCount %u ItemPossibleDrops %u",
            lootOwner->GetName(), lootOwner->GetMapId(), item.itemid, _challenge->GetChallengeLevel() - _challenge->GetLevelBonus(), _challenge->GetLevelBonus(), _challenge->GetChallengeTimer(), allItemCount, ItemPossibleDrops.size());
        }
    }

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessChallengeChest OtherPossibleDrops %i", OtherPossibleDrops.size());

    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessChallengeChest loot.items %i", loot.items.size());
    uint32 addCA = 0;
    if (_challenge->_complete)
        addCA = sChallengeMgr->GetCAForLoot(_challenge, lootId);

    if (addCA)
    {
        LootStoreItem item = LootStoreItem(addCA, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1);
        loot.AddItem(item);
    }

    uint32 addBigCA = 0;
    uint32 countBigCA = 0;
    if (_challenge->_complete)
        addBigCA = sChallengeMgr->GetBigCAForLoot(_challenge, lootId, countBigCA);

    if (addBigCA && countBigCA)
    {
        LootStoreItem item = LootStoreItem(addBigCA, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, countBigCA, countBigCA);
        loot.AddItem(item);
    }

    for (LootStoreItemList::const_iterator i = OtherPossibleDrops.begin(); i != OtherPossibleDrops.end(); ++i)
    {
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessChallengeChest OtherPossibleDrops itemid %i chance %f", i->itemid, i->chance);

        if (!loot.AllowedForPlayer(lootOwner, i->itemid, i->type, i->needs_quest))
            continue;

        if (!i->Roll(false))
            continue;                                         // Bad luck for the entry

        loot.AddItem(*i);                                 // Chance is already checked, just add

        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "ProcessChallengeChest AddItem itemid %i", i->itemid);
    }

    loot.generateMoneyLoot(900000, 1500000, lootOwner->GetMap()->IsDungeon());

    if (!const_cast<Player*>(lootOwner)->m_challengeKeyInfo.IsActive() && !sChallengeMgr->HasOploteLoot(lootOwner->GetGUID()))
    {
        const_cast<Player*>(lootOwner)->m_challengeKeyInfo.Level = _challenge->GetChallengeLevel() - 1;
        LootStoreItem item = LootStoreItem(138019, LOOT_ITEM_TYPE_ITEM, 0.0f, 0, 0, 1, 1);
        loot.AddItem(item);
    }
}

// True if template includes at least 1 quest drop entry
bool LootTemplate::HasQuestDrop(LootTemplateMap const& store, uint8 groupId) const
{
    if (groupId)                                            // Group reference
    {
        if (groupId > Groups.size())
            return false;                                   // Error message [should be] already printed at loading stage
        return Groups[groupId-1].HasQuestDrop();
    }

    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->mincountOrRef < 0)                           // References
            return LootTemplates_Reference.HaveQuestLootFor(-i->mincountOrRef);
        if (i->needs_quest)
            return true;
        // quest drop found
    }

    // Now processing groups
    for (LootGroups::const_iterator i = Groups.begin(); i != Groups.end(); ++i)
        if (i->HasQuestDrop())
            return true;

    return false;
}

// True if template includes at least 1 quest drop for an active quest of the player
bool LootTemplate::HasQuestDropForPlayer(LootTemplateMap const& store, Player const* player, uint8 groupId) const
{
    if (groupId)                                            // Group reference
    {
        if (groupId > Groups.size())
            return false;                                   // Error message already printed at loading stage
        return Groups[groupId-1].HasQuestDropForPlayer(player);
    }

    // Checking non-grouped entries
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->mincountOrRef < 0)                           // References processing
        {
            LootTemplateMap::const_iterator Referenced = store.find(-i->mincountOrRef);
            if (Referenced == store.end())
                continue;                                   // Error message already printed at loading stage
            if (Referenced->second->HasQuestDropForPlayer(store, player, i->group))
                return true;
        }
        else if (player->HasQuestForItem(i->itemid))
            return true;                                    // active quest drop found
    }

    // Now checking groups
    for (LootGroups::const_iterator i = Groups.begin(); i != Groups.end(); ++i)
        if (i->HasQuestDropForPlayer(player))
            return true;

    return false;
}

// Checks integrity of the template
void LootTemplate::Verify(LootStore const& lootstore, uint32 id) const
{
    // Checking group chances
    for (size_t i=0; i < Groups.size(); ++i)
        Groups[i].Verify(lootstore, id, i+1);

    // TODO: References validity checks
}

void LootTemplate::CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const
{
    for (LootStoreItemList::const_iterator ieItr = Entries.begin(); ieItr != Entries.end(); ++ieItr)
    {
        if (ieItr->mincountOrRef < 0)
        {
            if (!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if (ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }

    for (LootGroups::const_iterator grItr = Groups.begin(); grItr != Groups.end(); ++grItr)
        grItr->CheckLootRefs(store, ref_set);
}
bool LootTemplate::addConditionItem(Condition* cond)
{
    if (!cond || !cond->isLoaded())//should never happen, checked at loading
    {
        TC_LOG_ERROR(LOG_FILTER_LOOT, "LootTemplate::addConditionItem: condition is null");
        return false;
    }
    if (!Entries.empty())
    {
        for (LootStoreItemList::iterator i = Entries.begin(); i != Entries.end(); ++i)
        {
            if (i->itemid == uint32(cond->SourceEntry))
            {
                i->conditions.push_back(cond);
                return true;
            }
        }
    }
    if (!Groups.empty())
    {
        for (LootGroups::iterator groupItr = Groups.begin(); groupItr != Groups.end(); ++groupItr)
        {
            LootStoreItemList* itemList = (*groupItr).GetExplicitlyChancedItemList();
            if (!itemList->empty())
            {
                for (LootStoreItemList::iterator i = itemList->begin(); i != itemList->end(); ++i)
                {
                    if ((*i).itemid == uint32(cond->SourceEntry))
                    {
                        (*i).conditions.push_back(cond);
                        return true;
                    }
                }
            }
            itemList = (*groupItr).GetEqualChancedItemList();
            if (!itemList->empty())
            {
                for (LootStoreItemList::iterator i = itemList->begin(); i != itemList->end(); ++i)
                {
                    if ((*i).itemid == uint32(cond->SourceEntry))
                    {
                        (*i).conditions.push_back(cond);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool LootTemplate::isReference(uint32 id)
{
    for (LootStoreItemList::const_iterator ieItr = Entries.begin(); ieItr != Entries.end(); ++ieItr)
    {
        if (ieItr->itemid == id && ieItr->mincountOrRef < 0)
            return true;
    }
    return false;//not found or not reference
}

bool LootTemplate::CheckItemCondition(Player const* player, uint32 itemId, uint8 type) const
{
    if (type == LOOT_ITEM_TYPE_ITEM)
    {
        // TEMP: remove later
        if (player && player->GetSession()->GetSecurity() == SEC_PLAYER)
        {
            if (itemId == 147218 || itemId == 147212 || itemId == 147214 || itemId == 147215 || itemId == 147216 || itemId == 147217 || itemId == 147213 ||
                itemId == 147219 || itemId == 147220 || itemId == 147221 || itemId == 147222 || itemId == 147223)
                return false;
        }

        ConditionList conditionsList = sConditionMgr->GetConditionsForItemLoot(1, itemId);
        if (!sConditionMgr->IsObjectMeetToConditions(const_cast<Player*>(player), conditionsList))
            return false;
    }
    else
    {
        ConditionList conditionsList = sConditionMgr->GetConditionsForItemLoot(2, itemId);
        if (!sConditionMgr->IsObjectMeetToConditions(const_cast<Player*>(player), conditionsList))
            return false;
    }

    return true;
}

void LoadLootTemplates_Creature()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading creature loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet, lootIdSetUsed;
    uint32 count = LootTemplates_Creature.LoadAndCollectLootIds(lootIdSet);

    // Remove real entries and check loot existence
    CreatureTemplateContainerMap const* ctc = sObjectMgr->GetCreatureTemplates();
    for (auto &v : *ctc)
    {
        if (uint32 lootid = v.second.lootid)
        {
            if (lootIdSet.find(lootid) == lootIdSet.end())
            {
                LootTemplates_Creature.ReportNotExistedId(lootid);
                //WorldDatabase.PExecute("update creature_template set lootid = 0 WHERE `lootid` = %u", lootid);
            }
            else
                lootIdSetUsed.insert(lootid);
        }
    }

    for (LootIdSet::const_iterator itr = lootIdSetUsed.begin(); itr != lootIdSetUsed.end(); ++itr)
        lootIdSet.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Creature.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature loot templates. DB table `creature_loot_template` is empty");
}

void LoadLootTemplates_Disenchant()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading disenchanting loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    //LootIdSet lootIdSetUsed;
    uint32 count = LootTemplates_Disenchant.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    /*ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        if (uint32 lootid = itr->second.DisenchantID)
        {
            if (lootIdSet.find(lootid) != lootIdSet.end())
                lootIdSetUsed.insert(lootid);
        }
    }

    for (LootIdSet::const_iterator itr = lootIdSetUsed.begin(); itr != lootIdSetUsed.end(); ++itr)
        lootIdSet.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Disenchant.ReportUnusedIds(lootIdSet);*/

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u disenchanting loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 disenchanting loot templates. DB table `disenchant_loot_template` is empty");
}

void LoadLootTemplates_Fishing()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading fishing loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Fishing.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    for (AreaTableEntry const* areaEntry : sAreaTableStore)
        if (lootIdSet.find(areaEntry->ID) != lootIdSet.end())
            lootIdSet.erase(areaEntry->ID);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Fishing.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u fishing loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 fishing loot templates. DB table `fishing_loot_template` is empty");
}

void LoadLootTemplates_Gameobject()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading gameobject loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet, lootIdSetUsed;
    uint32 count = LootTemplates_Gameobject.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplates();
    for (GameObjectTemplateContainer::const_iterator itr = gotc->begin(); itr != gotc->end(); ++itr)
    {
        if (uint32 lootid = itr->second.entry)
        {
            if (lootIdSet.find(lootid) != lootIdSet.end())
                lootIdSetUsed.insert(lootid);
        }
    }

    for (LootIdSet::const_iterator itr = lootIdSetUsed.begin(); itr != lootIdSetUsed.end(); ++itr)
        lootIdSet.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Gameobject.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u gameobject loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 gameobject loot templates. DB table `gameobject_loot_template` is empty");
}

void LoadLootTemplates_Item()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading item loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Item.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        if (lootIdSet.find(itr->second.GetId()) != lootIdSet.end() && itr->second.GetFlags() & ITEM_FLAG_HAS_LOOT)
            lootIdSet.erase(itr->second.GetId());

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Item.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u item loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 item loot templates. DB table `item_loot_template` is empty");
}

void LoadLootTemplates_Milling()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading milling loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Milling.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        if (!(itr->second.GetFlags() & ITEM_FLAG_IS_MILLABLE))
            continue;

        if (lootIdSet.find(itr->second.GetId()) != lootIdSet.end())
            lootIdSet.erase(itr->second.GetId());
    }

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Milling.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u milling loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 milling loot templates. DB table `milling_loot_template` is empty");
}

void LoadLootTemplates_Pickpocketing()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading pickpocketing loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet, lootIdSetUsed;
    uint32 count = LootTemplates_Pickpocketing.LoadAndCollectLootIds(lootIdSet);

    // Remove real entries and check loot existence
    CreatureTemplateContainerMap const* ctc = sObjectMgr->GetCreatureTemplates();
    for (auto &v : *ctc)
    {
        if (uint32 lootid = v.second.pickpocketLootId)
        {
            if (lootIdSet.find(lootid) == lootIdSet.end())
                LootTemplates_Pickpocketing.ReportNotExistedId(lootid);
            else
                lootIdSetUsed.insert(lootid);
        }
    }

    for (LootIdSet::const_iterator itr = lootIdSetUsed.begin(); itr != lootIdSetUsed.end(); ++itr)
        lootIdSet.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Pickpocketing.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u pickpocketing loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 pickpocketing loot templates. DB table `pickpocketing_loot_template` is empty");
}

void LoadLootTemplates_Prospecting()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading prospecting loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Prospecting.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        if (!(itr->second.GetFlags() & ITEM_FLAG_IS_PROSPECTABLE))
            continue;

        if (lootIdSet.find(itr->second.GetId()) != lootIdSet.end())
            lootIdSet.erase(itr->second.GetId());
    }

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Prospecting.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u prospecting loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 prospecting loot templates. DB table `prospecting_loot_template` is empty");
}

void LoadLootTemplates_Mail()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading mail loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Mail.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    for (auto const& entry : sMailTemplateStore)
        if (lootIdSet.find(entry->ID) != lootIdSet.end())
            lootIdSet.erase(entry->ID);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Mail.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u mail loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 mail loot templates. DB table `mail_loot_template` is empty");
}

void LoadLootTemplates_Skinning()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading skinning loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet, lootIdSetUsed;
    uint32 count = LootTemplates_Skinning.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    CreatureTemplateContainerMap const* ctc = sObjectMgr->GetCreatureTemplates();
    for (auto &v : *ctc)
    {
        if (uint32 lootid = v.second.SkinLootId)
        {
            if (lootIdSet.find(lootid) == lootIdSet.end())
                LootTemplates_Skinning.ReportNotExistedId(lootid);
            else
                lootIdSetUsed.insert(lootid);
        }
    }

    for (LootIdSet::const_iterator itr = lootIdSetUsed.begin(); itr != lootIdSetUsed.end(); ++itr)
        lootIdSet.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Skinning.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u skinning loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 skinning loot templates. DB table `skinning_loot_template` is empty");
}

void LoadLootTemplates_Spell()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading spell loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Spell.LoadAndCollectLootIds(lootIdSet);

    // remove real entries and check existence loot
    for (uint32 spell_id = 1; spell_id < sSpellMgr->GetSpellInfoStoreSize(); ++spell_id)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);
        if (!spellInfo)
            continue;

        // possible cases
        if (!spellInfo->IsLootCrafting())
            continue;

        if (lootIdSet.find(spell_id) == lootIdSet.end())
        {
            // not report about not trainable spells (optionally supported by DB)
            // ignore 61756 (Northrend Inscription Research (FAST QA VERSION) for example
            //if (!(spellInfo->HasAttribute(SPELL_ATTR0_NOT_SHAPESHIFT)) || (spellInfo->Effects[0]->ItemType == 0))
            //{
            //    LootTemplates_Spell.ReportNotExistedId(spell_id);
            //    WorldDatabase.PExecute("DELETE FROM `spell_loot_template` WHERE entry = %u", spell_id);
            //}
        }
        else
            lootIdSet.erase(spell_id);
    }

    // output error for any still listed (not referenced from appropriate table) ids
    //LootTemplates_Spell.ReportUnusedIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell loot templates. DB table `spell_loot_template` is empty");
}

void LoadLootTemplates_World()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading World loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_World.LoadAndCollectLootIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u World loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 World loot templates. DB table `fishing_loot_template` is empty");
}

void LoadLootTemplates_Luck()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Luck loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Luck.LoadAndCollectLootIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Luck loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Luck loot templates. DB table `fishing_loot_template` is empty");
}

void LoadLootTemplates_Zone()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Zone loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    uint32 count = LootTemplates_Zone.LoadAndCollectLootIds(lootIdSet);

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Zone loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Zone loot templates. DB table `fishing_loot_template` is empty");
}

void LoadLootTemplates_Reference()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading reference loot templates...");

    uint32 oldMSTime = getMSTime();

    LootIdSet lootIdSet;
    LootTemplates_Reference.LoadAndCollectLootIds(lootIdSet);

    // check references and remove used
    LootTemplates_Creature.CheckLootRefs(&lootIdSet);
    LootTemplates_Fishing.CheckLootRefs(&lootIdSet);
    LootTemplates_Gameobject.CheckLootRefs(&lootIdSet);
    LootTemplates_Item.CheckLootRefs(&lootIdSet);
    LootTemplates_Milling.CheckLootRefs(&lootIdSet);
    LootTemplates_Pickpocketing.CheckLootRefs(&lootIdSet);
    LootTemplates_Skinning.CheckLootRefs(&lootIdSet);
    LootTemplates_Disenchant.CheckLootRefs(&lootIdSet);
    LootTemplates_Prospecting.CheckLootRefs(&lootIdSet);
    LootTemplates_Mail.CheckLootRefs(&lootIdSet);
    LootTemplates_World.CheckLootRefs(&lootIdSet);
    LootTemplates_Luck.CheckLootRefs(&lootIdSet);
    LootTemplates_Zone.CheckLootRefs(&lootIdSet);
    LootTemplates_Reference.CheckLootRefs(&lootIdSet);

    // output error for any still listed ids (not referenced from any loot table)
    LootTemplates_Reference.ReportUnusedIds(lootIdSet);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded refence loot templates in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

Loot* LootMgr::GetLoot(ObjectGuid const& guid)
{
    std::lock_guard<std::recursive_mutex> _lock(m_LootsLock);
    return Trinity::Containers::MapGetValuePtr(m_Loots, guid);
}

void LootMgr::AddLoot(Loot* loot)
{
    std::lock_guard<std::recursive_mutex> _lock(m_LootsLock);
    if (!loot->GetGUID())
        loot->GenerateLootGuid(loot->objGuid);
    //ASSERT(!loot->GetGUID().IsEmpty());

    LootsMap::iterator itr = m_Loots.find(loot->GetGUID());
    if (itr == m_Loots.end())
        m_Loots.emplace(loot->GetGUID(), loot);
    else
        itr->second = loot;
    //m_Loots[loot->GetGUID()] = loot;
    // TC_LOG_DEBUG(LOG_FILTER_LOOT, "LootMgr::AddLoot guid %s size %i", loot->GetGUID().ToString().c_str(), m_Loots.size());
}

void LootMgr::RemoveLoot(ObjectGuid const& guid)
{
    std::lock_guard<std::recursive_mutex> _lock(m_LootsLock);
    LootsMap::iterator itr = m_Loots.find(guid);
    if (itr == m_Loots.end())
        return;

    //m_Loots.erase(itr);
    itr->second = nullptr;
}
