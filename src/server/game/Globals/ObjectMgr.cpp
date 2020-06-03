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

#include "AreaTriggerData.h"
#include "Chat.h"
#include "Common.h"
#include "Configuration/Config.h"
#include "CreatureTextMgr.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "EventObjectData.h"
#include "GameTables.h"
#include "Garrison.h"
#include "GossipDef.h"
#include "GroupMgr.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "Language.h"
#include "LFGMgr.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Packets/BattlePayPackets.h"
#include "QuestData.h"
#include "ScriptMgr.h"
#include "ScriptsData.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Util.h"
#include "Vehicle.h"
#include "World.h"

VisibleDistanceMap sVisibleDistance[TYPE_VISIBLE_MAX];
WorldRateMap sWorldRateInfo[TYPE_RATE_MAX];
int32 playerCountInArea[10000];
int32 objectCountInWorld[50]; // HighGuid::Count
std::vector<int32> creatureCountInWorld;
std::vector<int32> spellCountInWorld;
int32 creatureCountInArea[10000];

bool SpellClickInfo::IsFitToRequirements(Unit const* clicker, Unit const* clickee) const
{
    Player const* playerClicker = clicker->ToPlayer();
    if (!playerClicker)
        return true;

    Unit const* summoner = nullptr;
    // Check summoners for party
    if (clickee->isSummon())
        summoner = clickee->ToTempSummon()->GetSummoner();
    if (!summoner)
        summoner = clickee;

    // This only applies to players
    switch (userType)
    {
        case SPELL_CLICK_USER_FRIEND:
            if (!playerClicker->IsFriendlyTo(summoner))
                return false;
            break;
        case SPELL_CLICK_USER_RAID:
            if (!playerClicker->IsInRaidWith(summoner))
                return false;
            break;
        case SPELL_CLICK_USER_PARTY:
            if (!playerClicker->IsInPartyWith(summoner))
                return false;
            break;
        case SPELL_CLICK_USER_NOT_IN_COMBAT:
            if (playerClicker->isInCombat())
                return false;
            break;
        default:
            break;
    }

    return true;
}

float GetVisibleDistance(uint32 type, uint32 id)
{
    VisibleDistanceMap::const_iterator itr = sVisibleDistance[type].find(id);
    if (itr != sVisibleDistance[type].end())
        return itr->second;

    return 0.0f;
}

float GetRateInfo(Creature* creature)
{
    if (!creature)
        return sWorld->getRate(RATE_XP_KILL);

    WorldRateMap::const_iterator itr = sWorldRateInfo[TYPE_RATE_AREA].find(creature->GetCurrentAreaID());
    if (itr != sWorldRateInfo[TYPE_RATE_AREA].end())
        return itr->second;

    itr = sWorldRateInfo[TYPE_RATE_ZONE].find(creature->GetCurrentZoneID());
    if (itr != sWorldRateInfo[TYPE_RATE_ZONE].end())
        return itr->second;

    itr = sWorldRateInfo[TYPE_RATE_MAP].find(creature->GetMapId());
    if (itr != sWorldRateInfo[TYPE_RATE_MAP].end())
        return itr->second;

    return sWorld->getRate(RATE_XP_KILL);
}

void AddPlayerToArea(uint16 areaId)
{
    playerCountInArea[areaId]++;
}

void RemovePlayerFromArea(uint16 areaId)
{
    playerCountInArea[areaId]--;
}

int32 GetPlayerFromArea(uint16 areaId)
{
    return playerCountInArea[areaId];
}

template<> ObjectGuidGenerator<HighGuid::Player>* ObjectMgr::GetGenerator() { return &_playerGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Creature>* ObjectMgr::GetGenerator() { return &_creatureGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Pet>* ObjectMgr::GetGenerator() { return &_petGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Vehicle>* ObjectMgr::GetGenerator() { return &_vehicleGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Item>* ObjectMgr::GetGenerator() { return &_itemGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::GameObject>* ObjectMgr::GetGenerator() { return &_gameObjectGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::DynamicObject>* ObjectMgr::GetGenerator() { return &_dynamicObjectGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Corpse>* ObjectMgr::GetGenerator() { return &_corpseGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::AreaTrigger>* ObjectMgr::GetGenerator() { return &_areaTriggerGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::LootObject>* ObjectMgr::GetGenerator() { return &_lootObjectGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Transport>* ObjectMgr::GetGenerator() { return &_moTransportGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::BattlePet>* ObjectMgr::GetGenerator() { return &_BattlePetGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::PetBattle>* ObjectMgr::GetGenerator() { return &_PetBattleGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::LFGList>* ObjectMgr::GetGenerator() { return &_LFGListGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::LFGObject>* ObjectMgr::GetGenerator() { return &_LFGObjectGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Conversation>* ObjectMgr::GetGenerator() { return &_conversationGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Cast>* ObjectMgr::GetGenerator() { return &_castGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::EventObject>* ObjectMgr::GetGenerator() { return &_EventObjectGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Scenario>* ObjectMgr::GetGenerator() { return &_scenarioGuidGenerator; }
template<> ObjectGuidGenerator<HighGuid::Vignette>* ObjectMgr::GetGenerator() { return &_vignetteGuidGenerator; }

template<HighGuid type>
ObjectGuidGenerator<type>* ObjectMgr::GetGenerator()
{
    return nullptr;
}

ItemSpecStats::ItemSpecStats(ItemEntry const* item, ItemSparseEntry const* sparse): ItemType(0), ItemSpecStatCount(0)
{
    memset(ItemSpecStatTypes, -1, sizeof(ItemSpecStatTypes));

    if (item->ClassID == ITEM_CLASS_WEAPON)
    {
        ItemType = 5;
        switch (item->SubclassID)
        {
        case ITEM_SUBCLASS_WEAPON_AXE:
            AddStat(ITEM_SPEC_STAT_ONE_HANDED_AXE);
            break;
        case ITEM_SUBCLASS_WEAPON_AXE2:
            AddStat(ITEM_SPEC_STAT_TWO_HANDED_AXE);
            break;
        case ITEM_SUBCLASS_WEAPON_BOW:
            AddStat(ITEM_SPEC_STAT_BOW);
            break;
        case ITEM_SUBCLASS_WEAPON_GUN:
            AddStat(ITEM_SPEC_STAT_GUN);
            break;
        case ITEM_SUBCLASS_WEAPON_MACE:
            AddStat(ITEM_SPEC_STAT_ONE_HANDED_MACE);
            break;
        case ITEM_SUBCLASS_WEAPON_MACE2:
            AddStat(ITEM_SPEC_STAT_TWO_HANDED_MACE);
            break;
        case ITEM_SUBCLASS_WEAPON_POLEARM:
            AddStat(ITEM_SPEC_STAT_POLEARM);
            break;
        case ITEM_SUBCLASS_WEAPON_SWORD:
            AddStat(ITEM_SPEC_STAT_ONE_HANDED_SWORD);
            break;
        case ITEM_SUBCLASS_WEAPON_SWORD2:
            AddStat(ITEM_SPEC_STAT_TWO_HANDED_SWORD);
            break;
        case ITEM_SUBCLASS_WEAPON_WARGLAIVES:
            AddStat(ITEM_SPEC_STAT_WARGLAIVES);
            break;
        case ITEM_SUBCLASS_WEAPON_STAFF:
            AddStat(ITEM_SPEC_STAT_STAFF);
            break;
        case ITEM_SUBCLASS_WEAPON_FIST_WEAPON:
            AddStat(ITEM_SPEC_STAT_FIST_WEAPON);
            break;
        case ITEM_SUBCLASS_WEAPON_DAGGER:
            AddStat(ITEM_SPEC_STAT_DAGGER);
            break;
        case ITEM_SUBCLASS_WEAPON_THROWN:
            AddStat(ITEM_SPEC_STAT_THROWN);
            break;
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            AddStat(ITEM_SPEC_STAT_CROSSBOW);
            break;
        case ITEM_SUBCLASS_WEAPON_WAND:
            AddStat(ITEM_SPEC_STAT_WAND);
            break;
        default:
            break;
        }
    }
    else if (item->ClassID == ITEM_CLASS_ARMOR)
    {
        switch (item->SubclassID)
        {
        case ITEM_SUBCLASS_ARMOR_CLOTH:
            if (sparse->InventoryType != INVTYPE_CLOAK)
            {
                ItemType = 1;
                break;
            }

            ItemType = 0;
            AddStat(ITEM_SPEC_STAT_CLOAK);
            break;
        case ITEM_SUBCLASS_ARMOR_LEATHER:
            ItemType = 2;
            break;
        case ITEM_SUBCLASS_ARMOR_MAIL:
            ItemType = 3;
            break;
        case ITEM_SUBCLASS_ARMOR_PLATE:
            ItemType = 4;
            break;
        default:
            if (item->SubclassID == ITEM_SUBCLASS_ARMOR_SHIELD)
            {
                ItemType = 6;
                AddStat(ITEM_SPEC_STAT_SHIELD);
            }
            else if (item->SubclassID > ITEM_SUBCLASS_ARMOR_SHIELD && item->SubclassID <= ITEM_SUBCLASS_ARMOR_RELIC)
            {
                ItemType = 6;
                AddStat(ITEM_SPEC_STAT_RELIC);
            }
            else
                ItemType = 0;
            break;
        }
    }
    else if (item->ClassID == ITEM_CLASS_GEM)
    {
        ItemType = 7;
        if (GemPropertiesEntry const* gem = sGemPropertiesStore.LookupEntry(sparse->GemProperties))
        {
            if (gem->Type & SOCKET_COLOR_RELIC_IRON)
                AddStat(ITEM_SPEC_STAT_RELIC_IRON);
            if (gem->Type & SOCKET_COLOR_RELIC_BLOOD)
                AddStat(ITEM_SPEC_STAT_RELIC_BLOOD);
            if (gem->Type & SOCKET_COLOR_RELIC_SHADOW)
                AddStat(ITEM_SPEC_STAT_RELIC_SHADOW);
            if (gem->Type & SOCKET_COLOR_RELIC_FEL)
                AddStat(ITEM_SPEC_STAT_RELIC_FEL);
            if (gem->Type & SOCKET_COLOR_RELIC_ARCANE)
                AddStat(ITEM_SPEC_STAT_RELIC_ARCANE);
            if (gem->Type & SOCKET_COLOR_RELIC_FROST)
                AddStat(ITEM_SPEC_STAT_RELIC_FROST);
            if (gem->Type & SOCKET_COLOR_RELIC_FIRE)
                AddStat(ITEM_SPEC_STAT_RELIC_FIRE);
            if (gem->Type & SOCKET_COLOR_RELIC_WATER)
                AddStat(ITEM_SPEC_STAT_RELIC_WATER);
            if (gem->Type & SOCKET_COLOR_RELIC_LIFE)
                AddStat(ITEM_SPEC_STAT_RELIC_LIFE);
            if (gem->Type & SOCKET_COLOR_RELIC_WIND)
                AddStat(ITEM_SPEC_STAT_RELIC_WIND);
            if (gem->Type & SOCKET_COLOR_RELIC_HOLY)
                AddStat(ITEM_SPEC_STAT_RELIC_HOLY);
        }
    }
    else
        ItemType = 0;

    for (uint32 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        if (sparse->StatModifierBonusStat[i] != -1)
            AddModStat(sparse->StatModifierBonusStat[i]);
}

void ItemSpecStats::AddStat(ItemSpecStat statType)
{
    if (ItemSpecStatCount >= MAX_ITEM_PROTO_STATS)
        return;

    for (uint32 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        if (ItemSpecStatTypes[i] == uint32(statType))
            return;

    ItemSpecStatTypes[ItemSpecStatCount++] = statType;
}

void ItemSpecStats::AddModStat(int32 itemStatType)
{
    switch (itemStatType)
    {
    case ITEM_MOD_AGILITY:
        AddStat(ITEM_SPEC_STAT_AGILITY);
        break;
    case ITEM_MOD_STRENGTH:
        AddStat(ITEM_SPEC_STAT_STRENGTH);
        break;
    case ITEM_MOD_INTELLECT:
        AddStat(ITEM_SPEC_STAT_INTELLECT);
        break;
    case ITEM_MOD_DODGE_RATING:
        AddStat(ITEM_SPEC_STAT_DODGE);
        break;
    case ITEM_MOD_PARRY_RATING:
        AddStat(ITEM_SPEC_STAT_PARRY);
        break;
    case ITEM_MOD_CRIT_MELEE_RATING:
    case ITEM_MOD_CRIT_RANGED_RATING:
    case ITEM_MOD_CRIT_SPELL_RATING:
    case ITEM_MOD_CRIT_RATING:
        AddStat(ITEM_SPEC_STAT_CRIT);
        break;
    case ITEM_MOD_HASTE_RATING:
        AddStat(ITEM_SPEC_STAT_HASTE);
        break;
    case ITEM_MOD_HIT_RATING:
        AddStat(ITEM_SPEC_STAT_HIT);
        break;
    case ITEM_MOD_EXTRA_ARMOR:
        AddStat(ITEM_SPEC_STAT_BONUS_ARMOR);
        break;
    case ITEM_MOD_AGI_STR_INT:
        AddStat(ITEM_SPEC_STAT_AGILITY);
        AddStat(ITEM_SPEC_STAT_STRENGTH);
        AddStat(ITEM_SPEC_STAT_INTELLECT);
        break;
    case ITEM_MOD_AGI_STR:
        AddStat(ITEM_SPEC_STAT_AGILITY);
        AddStat(ITEM_SPEC_STAT_STRENGTH);
        break;
    case ITEM_MOD_AGI_INT:
        AddStat(ITEM_SPEC_STAT_AGILITY);
        AddStat(ITEM_SPEC_STAT_INTELLECT);
        break;
    case ITEM_MOD_STR_INT:
        AddStat(ITEM_SPEC_STAT_STRENGTH);
        AddStat(ITEM_SPEC_STAT_INTELLECT);
        break;
    default:
        break;
    }
}

ObjectMgr::ObjectMgr(): _mailId(0), DBCLocaleIndex(), _playerInfo{}
{
    _auctionId = 1;
    _equipmentSetGuid = 1;
    _itemTextId = 1;
    _hiPetNumber = 1;
    _voidItemId = 1;
    _skipUpdateCount = 1;
    _reportComplaintID = 1;
    _supportTicketSubmitBugID = 1;
    m_donate_waite = false;
}

ObjectMgr::~ObjectMgr()
{

    for (uint8 race = 0; race < MAX_RACES; ++race)
    {
        for (uint8 class_ = 0; class_ < MAX_CLASSES; ++class_)
        {
            if (_playerInfo[race][class_])
                delete[] _playerInfo[race][class_]->levelInfo;

            delete _playerInfo[race][class_];
        }
    }

    for (CacheVendorItemContainer::iterator itr = _cacheVendorItemStore.begin(); itr != _cacheVendorItemStore.end(); ++itr)
        itr->second.Clear();
    
    for (CacheDonateVendorItemContainer::iterator itr = _cacheDonateVendorItemStore.begin(); itr != _cacheDonateVendorItemStore.end(); ++itr)
        itr->second.Clear();
    

    _cacheTrainerSpellStore.clear();

    for (DungeonEncounterContainer::iterator itr =_dungeonEncounterStore.begin(); itr != _dungeonEncounterStore.end(); ++itr)
        for (DungeonEncounterList::iterator encounterItr = itr->second.begin(); encounterItr != itr->second.end(); ++encounterItr)
            delete *encounterItr;
}

ObjectMgr* ObjectMgr::instance()
{
    static ObjectMgr instance;
    return &instance;
}

std::list<CurrencyLoot> ObjectMgr::GetCurrencyLoot(uint32 entry, uint8 type, uint8 spawnMode)
{
    std::list<CurrencyLoot> temp;
    uint16 diffMask = (1 << (CreatureTemplate::GetDiffFromSpawn(spawnMode)));
    for (CurrencysLoot::iterator itr = _currencysLoot.begin(); itr != _currencysLoot.end(); ++itr)
    {
        if (itr->Entry == entry && itr->Type == type && (itr->lootmode == 0 || (itr->lootmode & diffMask)))
            temp.push_back(*itr);
    }
    return temp;
}

void ObjectMgr::AddLocaleString(std::string&& value, LocaleConstant localeConstant, StringVector& data)
{
    if (value.empty())
        return;

    if (data.size() <= size_t(localeConstant))
        data.resize(localeConstant + 1);

    data[localeConstant] = std::move(value);
}

void ObjectMgr::GetLocaleString(StringVector const& data, LocaleConstant localeConstant, std::string& value)
{
    if (data.size() > size_t(localeConstant) && !data[localeConstant].empty())
        value = data[localeConstant];
}

void ObjectMgr::LoadWorldVisibleDistance()
{
    uint32 oldMSTime = getMSTime();

    for (uint8 i = 0; i < TYPE_VISIBLE_MAX; ++i)
        sVisibleDistance[i].clear();

    QueryResult result = WorldDatabase.Query("SELECT `type`, `id`, `distance` FROM `world_visible_distance`");

    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 type = fields[0].GetUInt32();
        uint32 id = fields[1].GetUInt32();
        float distance = fields[2].GetFloat();
        if(type > TYPE_VISIBLE_MAX)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded world visible distance type %u error", type);
            continue;
        }

        sVisibleDistance[type][id] = distance;

        count++;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u world visible distance in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadWorldMapDiffycultyStat()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT `mapId`, `difficultyId`, `dmg_multiplier` FROM `world_map_difficulty_stats`");

    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 mapId = fields[0].GetInt32();
        uint8 difficultyId = fields[1].GetUInt8();
        float dmg_multiplier = fields[2].GetFloat();

        if (mapId < 0 || difficultyId > MAX_DIFFICULTY)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded world map difficulty stats error - mapId: %i, difficultyId: %u", mapId, difficultyId);
            continue;
        }

        _mapDifficultyStat.resize(mapId + 1);
        _mapDifficultyStat[mapId].resize(MAX_DIFFICULTY, 1.0f);

        if (difficultyId > 0)
            _mapDifficultyStat[mapId][difficultyId] = dmg_multiplier;
        else
        {
            for (int32 i = 0; i < MAX_DIFFICULTY; ++i)
                _mapDifficultyStat[mapId][i] = dmg_multiplier;
        }

        count++;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u world map difficulty stats in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadWorldRateInfo()
{
    uint32 oldMSTime = getMSTime();

    for (uint8 i = 0; i < TYPE_RATE_MAX; ++i)
        sWorldRateInfo[i].clear();

    QueryResult result = WorldDatabase.Query("SELECT `type`, `id`, `rate` FROM `world_rate_info`");
    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 type = fields[0].GetUInt32();
        uint32 id = fields[1].GetUInt32();
        float rate = fields[2].GetFloat();
        if(type > TYPE_RATE_MAX)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded world rate type %u error", type);
            continue;
        }

        sWorldRateInfo[type][id] = rate;

        count++;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u world rate in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadCreatureLocales()
{
    uint32 oldMSTime = getMSTime();

    _creatureLocaleStore.clear();

    //                                               0   1       2      3         4      5      6      7      8         9         10        11
    QueryResult result = WorldDatabase.Query("SELECT ID, Locale, Title, TitleAlt, Name1, Name2, Name3, Name4, NameAlt1, NameAlt2, NameAlt3, NameAlt4 FROM creature_template_wdb_locale");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        LocaleConstant locale = GetLocaleByName(fields[1].GetString());
        if (locale == LOCALE_none)
            continue;

        auto& data = _creatureLocaleStore[fields[0].GetUInt32()];
        AddLocaleString(fields[2].GetString(), locale, data.Title);
        AddLocaleString(fields[3].GetString(), locale, data.TitleAlt);

        for (uint8 i = 0; i < MAX_CREATURE_NAMES; ++i)
            AddLocaleString(fields[4 + i].GetString(), locale, data.Name[i]);

        for (uint8 i = 0; i < MAX_CREATURE_NAMES; ++i)
            AddLocaleString(fields[8 + i].GetString(), locale, data.NameAlt[i]);

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature locale strings in %u ms", static_cast<uint32>(_creatureLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadWDBCreatureTemplates()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query(
    //        0      1      2      3      4       5         6        7         8         9      10        11         12      13         14          15
    "SELECT Entry, Name1, Name2, Name3, Name4, NameAlt1, NameAlt2, NameAlt3, NameAlt4, Title, TitleAlt, CursorName, Type, TypeFlags, TypeFlags2, RequiredExpansion, "
    //16           17              18          19        20         21        22           23           24        25          26          27          28
    "Family, Classification, MovementInfoID, HpMulti, PowerMulti, Leader, KillCredit1, KillCredit2, VignetteID, DisplayId1, DisplayId2, DisplayId3, DisplayId4, "
    //29           30           31          32          33          34          35          36        37        38          39              40
    "FlagQuest, QuestItem1, QuestItem2, QuestItem3, QuestItem4, QuestItem5, QuestItem6, QuestItem7, QuestItem8, QuestItem9, QuestItem10,  VerifiedBuild FROM creature_template_wdb;");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature template definitions. DB table `creature_template_wdb` is empty.");
        return;
    }

    _creatureTemplateStore.resize(result->GetRowCount() + 1);

    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();

        uint32 entry = fields[index++].GetUInt32();

        if (entry >= _creatureTemplateStore.size())
            _creatureTemplateStore.resize(entry + 1);

        CreatureTemplate& creatureTemplate = _creatureTemplateStoreMap[entry];
        _creatureTemplateStore[entry] = &creatureTemplate;

        creatureTemplate.Entry = entry;

        for (uint8 i = 0; i < MAX_CREATURE_NAMES; ++i)
            creatureTemplate.Name[i] = fields[index++].GetString();
        for (uint8 i = 0; i < MAX_CREATURE_NAMES; ++i)
            creatureTemplate.NameAlt[i] = fields[index++].GetString();
        creatureTemplate.Title = fields[index++].GetString();
        creatureTemplate.TitleAlt = fields[index++].GetString();
        creatureTemplate.CursorName = fields[index++].GetString();
        creatureTemplate.Type = uint32(fields[index++].GetUInt8());
        for (uint8 i = 0; i < MAX_TYPE_FLAGS; ++i)
            creatureTemplate.TypeFlags[i] = fields[index++].GetUInt32();
        creatureTemplate.RequiredExpansion = uint32(fields[index++].GetInt16());
        creatureTemplate.Family = fields[index++].GetUInt32();
        creatureTemplate.Classification = uint32(fields[index++].GetUInt8());
        creatureTemplate.MovementInfoID = fields[index++].GetUInt32();
        creatureTemplate.HpMulti = fields[index++].GetFloat();
        creatureTemplate.PowerMulti = fields[index++].GetFloat();
        creatureTemplate.Leader = fields[index++].GetBool();
        for (uint8 i = 0; i < MAX_KILL_CREDIT; ++i)
        {
            creatureTemplate.KillCredit[i] = fields[index++].GetUInt32();
            if (creatureTemplate.KillCredit[i] && (creatureTemplate.RequiredExpansion >= EXPANSION_WARLORDS_OF_DRAENOR || creatureTemplate.RequiredExpansion == -1))
                creatureTemplate.QuestPersonalLoot = true;
        }
        creatureTemplate.VignetteID = fields[index++].GetUInt32();
        for (uint8 i = 0; i < MAX_CREATURE_MODELS; ++i)
            creatureTemplate.Modelid[i] = fields[index++].GetInt32();
        creatureTemplate.FlagQuest = fields[index++].GetUInt32();
        for (uint8 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
        {
            creatureTemplate.QuestItem[i] = fields[index++].GetUInt32();
            if (creatureTemplate.QuestItem[i] && (creatureTemplate.RequiredExpansion >= EXPANSION_WARLORDS_OF_DRAENOR || creatureTemplate.RequiredExpansion == -1))
                creatureTemplate.QuestPersonalLoot = true;
        }
        creatureTemplate.VerifiedBuild = fields[index++].GetUInt32();

        if (VignetteEntry const* vignette = sVignetteStore.LookupEntry(creatureTemplate.VignetteID))
            creatureTemplate.TrackingQuestID = vignette->VisibleTrackingQuestID;
    }
    while (result->NextRow());

    for (auto& v : _creatureTemplateStoreMap)
        CheckCreatureTemplateWDB(&v.second);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature WDB templates in %u ms", static_cast<uint32>(_creatureTemplateStoreMap.size()), GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::CheckCreatureTemplateWDB(CreatureTemplate* cInfo)
{
    if (!cInfo || !cInfo->Entry)
        return;

    CreatureDisplayInfoEntry const* displayScaleEntry = nullptr;
    for (uint8 i = 0; i < MAX_CREATURE_MODELS; ++i)
    {
        if (cInfo->Modelid[i])
        {
            CreatureDisplayInfoEntry const* displayEntry = sCreatureDisplayInfoStore.LookupEntry(sObjectMgr->GetCreatureDisplay(cInfo->Modelid[i]));
            if (!displayEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) lists non-existing Modelid[0] id (%i), this can crash the client.", cInfo->Entry, cInfo->Modelid[i]);
                cInfo->Modelid[i] = 0;
            }
            else if (!displayScaleEntry)
                displayScaleEntry = displayEntry;
    
            if (!GetCreatureModelInfo(sObjectMgr->GetCreatureDisplay(cInfo->Modelid[i])))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "No model data exist for `Modelid[%u]` = %i listed by creature (Entry: %u).", i, cInfo->Modelid[i], cInfo->Entry);
                //WorldDatabase.PExecute("REPLACE INTO `creature_model_info` (`DisplayID`, `BoundingRadius`, `CombatReach`, `DisplayID_Other_Gender`, `hostileId`) values (%u, 1, 1, 0, 0)", cInfo->Modelid[i]);
                //! Create default data for model.
                //! ToDo: move to function
                if (CreatureDisplayInfoEntry const* creatureDisplay = sCreatureDisplayInfoStore.LookupEntry(cInfo->Modelid[i]))
                {
                    CreatureModelInfo& modelInfo = _creatureModelStore[cInfo->Modelid[i]];
                    modelInfo.bounding_radius = 2.0f;
                    modelInfo.combat_reach = 3.0f;
                    modelInfo.gender = creatureDisplay->Gender;
                    modelInfo.displayId_other_gender = 0;
                    modelInfo.hostileId = 0;
                }
            }
        }
    }

    if (!displayScaleEntry)
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) does not have any existing display id in Modelid1/Modelid2/Modelid3/Modelid[3].", cInfo->Entry);

    for (uint8 k = 0; k < MAX_KILL_CREDIT; ++k)
        if (cInfo->KillCredit[k])
            if (!GetCreatureTemplate(cInfo->KillCredit[k]))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) lists non-existing creature entry %u in `KillCredit%d`.", cInfo->Entry, cInfo->KillCredit[k], k + 1);
                cInfo->KillCredit[k] = 0;
            }

    if (cInfo->Type && !sCreatureTypeStore.LookupEntry(cInfo->Type))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has invalid creature type (%u) in `type`.", cInfo->Entry, cInfo->Type);
        cInfo->Type = CREATURE_TYPE_HUMANOID;
    }

    if (cInfo->Family && !sCreatureFamilyStore.LookupEntry(cInfo->Family))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has invalid creature family (%u) in `family`.", cInfo->Entry, cInfo->Family);
        cInfo->Family = 0;
    }

    if (cInfo->RequiredExpansion > (MAX_EXPANSIONS - 1))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_template` lists creature (Entry: %u) with `exp` %i. Ignored and set to 0.", cInfo->Entry, cInfo->RequiredExpansion);
        cInfo->RequiredExpansion = 0;
    }
}

void ObjectMgr::LoadCreatureTemplates()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query(
    //        0           1            2        3         4        5        6          7           8          9       10       11
    "SELECT entry, gossip_menu_id, minlevel, maxlevel, faction, npcflag, npcflag2, speed_walk, speed_run, speed_fly, scale, dmgschool,"
    //12                 13              14               15          16          17           18           19            20            21
    "dmg_multiplier, baseattacktime, rangeattacktime, unit_class, unit_flags, unit_flags2, unit_flags3, dynamicflags, trainer_type, trainer_spell, "
    //22            23            24           25           26          27           28           29
    "trainer_class, trainer_race, lootid, pickpocketloot, skinloot, resistance1, resistance2, resistance3, "
    //30          31           32           33       34      35      36      37      38      39      40           41            42       43       44
    "resistance4, resistance5, resistance6, spell1, spell2, spell3, spell4, spell5, spell6, spell7, spell8, PetSpellDataId, VehicleId, mingold, maxgold, "
    //45     46             47          48           49              50         51           52                    53           54
    "AIName, MovementType, InhabitType, HoverHeight, Mana_mod_extra, Armor_mod, RegenHealth, mechanic_immune_mask, flags_extra, ScriptName, "
    //55                 56              57                58                59            60             61
    "ScaleLevelMin, ScaleLevelMax, ScaleLevelDelta, ScaleLevelDuration, ControllerID, WorldEffects, PassiveSpells,"
    //62                         63                 64                 65              66          67          68           69
    "StateWorldEffectID, SpellStateVisualID, SpellStateAnimID, SpellStateAnimKitID, IgnoreLos, AffixState, MaxVisible, SandboxScalingID"
    " FROM creature_template;");

    uint32 count = 0;
    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();
        uint32 entry = fields[index++].GetUInt32();

        if (entry >= _creatureTemplateStore.size())
            _creatureTemplateStore.resize(entry + 1, nullptr);

        if (entry >= creatureCountInWorld.size())
            creatureCountInWorld.resize(entry + 1, 0);

        CreatureTemplate& creatureTemplate = _creatureTemplateStoreMap[entry];
        _creatureTemplateStore[entry] = &creatureTemplate;

        creatureTemplate.GossipMenuId      = fields[index++].GetUInt32();
        creatureTemplate.minlevel          = std::min(fields[index++].GetUInt8(), CREATURE_MAX_LEVEL);
        creatureTemplate.maxlevel          = std::min(fields[index++].GetUInt8(), CREATURE_MAX_LEVEL);
        creatureTemplate.faction         = uint32(fields[index++].GetUInt16());
        creatureTemplate.npcflag           = fields[index++].GetUInt32();
        creatureTemplate.npcflag2          = fields[index++].GetUInt32();
        creatureTemplate.speed_walk        = fields[index++].GetFloat();
        creatureTemplate.speed_run         = fields[index++].GetFloat();
        creatureTemplate.speed_fly         = fields[index++].GetFloat();
        creatureTemplate.scale             = fields[index++].GetFloat();
        creatureTemplate.dmgschool         = uint32(fields[index++].GetInt8());
        creatureTemplate.dmg_multiplier    = fields[index++].GetFloat();
        creatureTemplate.baseattacktime    = fields[index++].GetUInt32();
        creatureTemplate.rangeattacktime   = fields[index++].GetUInt32();
        creatureTemplate.unit_class        = uint32(fields[index++].GetUInt8());
        creatureTemplate.unit_flags        = fields[index++].GetUInt32();
        creatureTemplate.unit_flags2       = fields[index++].GetUInt32();
        creatureTemplate.unit_flags3       = fields[index++].GetUInt32();
        creatureTemplate.dynamicflags      = fields[index++].GetUInt32();
        creatureTemplate.trainer_type      = uint32(fields[index++].GetUInt8());
        creatureTemplate.trainer_spell     = fields[index++].GetUInt32();
        creatureTemplate.trainer_class     = uint32(fields[index++].GetUInt8());
        creatureTemplate.trainer_race      = uint32(fields[index++].GetUInt8());
        creatureTemplate.lootid            = fields[index++].GetUInt32();
        creatureTemplate.pickpocketLootId  = fields[index++].GetUInt32();
        creatureTemplate.SkinLootId        = fields[index++].GetUInt32();

        for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            creatureTemplate.resistance[i] = fields[index++].GetInt16();

        for (uint8 i = 0; i < CREATURE_MAX_SPELLS; ++i)
            creatureTemplate.spells[i] = fields[index++].GetUInt32();

        creatureTemplate.PetSpellDataId = fields[index++].GetUInt32();
        creatureTemplate.VehicleId      = fields[index++].GetUInt32();
        creatureTemplate.mingold        = fields[index++].GetUInt32();
        creatureTemplate.maxgold        = fields[index++].GetUInt32();
        creatureTemplate.AIName         = fields[index++].GetString();
        creatureTemplate.MovementType   = uint32(fields[index++].GetUInt8());
        creatureTemplate.InhabitType    = uint32(fields[index++].GetUInt8());
        creatureTemplate.HoverHeight    = fields[index++].GetFloat();
        creatureTemplate.ModManaExtra   = fields[index++].GetFloat();
        creatureTemplate.ModArmor       = fields[index++].GetFloat();
        creatureTemplate.RegenHealth        = fields[index++].GetBool();
        creatureTemplate.MechanicImmuneMask = fields[index++].GetUInt32();
        creatureTemplate.flags_extra        = fields[index++].GetUInt32();
        creatureTemplate.ScriptID           = GetScriptId(fields[index++].GetCString());
        creatureTemplate.ScaleLevelMin      = std::min(fields[index++].GetUInt8(), CREATURE_MAX_LEVEL);
        creatureTemplate.ScaleLevelMax      = std::min(fields[index++].GetUInt8(), CREATURE_MAX_LEVEL);
        creatureTemplate.ScaleLevelDelta    = std::min(fields[index++].GetUInt8(), CREATURE_MAX_LEVEL);
        creatureTemplate.ScaleLevelDuration = std::min(fields[index++].GetUInt16(), std::numeric_limits<uint16>::max());
        creatureTemplate.ControllerID       = fields[index++].GetInt32();
        Tokenizer WorldEffects(fields[index++].GetString(), ' ');
        for (char const* token : WorldEffects)
        {
            uint32 WorldEffectID = atol(token);
            creatureTemplate.WorldEffects.push_back(WorldEffectID);
        }
        Tokenizer PassiveSpells(fields[index++].GetString(), ' ');
        for (char const* token : PassiveSpells)
        {
            uint32 PassiveSpellID = atol(token);
            creatureTemplate.PassiveSpells.push_back(PassiveSpellID);
        }
        creatureTemplate.StateWorldEffectID = fields[index++].GetUInt32();
        creatureTemplate.SpellStateVisualID = fields[index++].GetUInt32();
        creatureTemplate.SpellStateAnimID = fields[index++].GetUInt32();
        creatureTemplate.SpellStateAnimKitID = fields[index++].GetUInt32();
        creatureTemplate.IgnoreLos = fields[index++].GetBool();
        creatureTemplate.AffixState = fields[index++].GetUInt32();
        creatureTemplate.MaxVisible = fields[index++].GetBool();
        creatureTemplate.SandboxScalingID = fields[index++].GetUInt16();

        if(creatureTemplate.TypeFlags[0] & CREATURE_TYPEFLAGS_BOSS)
        {
            //Save loot spell
            if(creatureTemplate.spells[6])
                _creatureSpellBonus[creatureTemplate.spells[6]] = entry;
            //Save bonus loot spell
            if(creatureTemplate.spells[7])
                _creatureSpellBonus[creatureTemplate.spells[7]] = entry;
        }
        ++count;
    }
    while (result->NextRow());

    //                                                      0      1          2             3
    if (QueryResult results = WorldDatabase.Query("SELECT entry, spell, difficultyMask, timerCast FROM creature_template_spell;"))
    {
        do
        {
            Field* fields = results->Fetch();

            uint32 entry = fields[0].GetUInt32();
            if (entry >= _creatureTemplateStore.size())
                continue;

            CreatureTemplate* creatureTemplate = _creatureTemplateStore[entry];
            if (!creatureTemplate)
                continue;

            uint32 SpellID = fields[1].GetUInt32();
            CreatureSpell& spell = creatureTemplate->CreatureSpells[SpellID];
            spell.SpellID = SpellID;
            spell.DifficultyMask = fields[2].GetUInt32();
            spell.TimerCast = fields[3].GetUInt32();
        }
        while (results->NextRow());

        for (auto& v : _creatureTemplateStoreMap)
        {
            auto tempSpells = v.second.CreatureSpells;
            auto* spells = &v.second.CreatureSpells;
            for (auto& spell : tempSpells)
            {
                SpellInfo const* triggerInfo = sSpellMgr->GetSpellInfo(spell.second.SpellID);
                if (!triggerInfo)
                {
                    for (CreatureSpellList::iterator itr = spells->begin(); itr != spells->end(); ++itr)
                        if (itr->second.SpellID == spell.second.SpellID)
                        {
                            spells->erase(itr);
                            break;
                        }
                    continue;
                }

                for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                {
                    if (triggerInfo->EffectMask < uint32(1 << j)) // Prevent circle if effect not exist
                        break;

                    if ((triggerInfo->EffectMask & (1 << j)) == 0)
                        continue;

                    if (triggerInfo->Effects[j]->TriggerSpell)
                    {
                        for (CreatureSpellList::iterator itr = spells->begin(); itr != spells->end(); ++itr)
                            if (itr->second.SpellID == triggerInfo->Effects[j]->TriggerSpell)
                            {
                                spells->erase(itr);
                                break;
                            }
                    }
                }
            }
            for (auto& spell : v.second.CreatureSpells)
                spell.second.Text = sCreatureTextMgr->FindSpellInText(v.second.Entry, spell.second.SpellID);
        }
    }

    // Checking needs to be done after loading because of the difficulty self referencing
    for (auto& v : _creatureTemplateStoreMap)
        CheckCreatureTemplate(&v.second);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadCreatureDifficultyStat()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0        1             2             3
    QueryResult result = WorldDatabase.Query("SELECT entry, difficulty, dmg_multiplier, HealthModifier FROM creature_difficulty_stat;");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature difficulty stat definitions. DB table `creature_difficulty_stat` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (entry >= _creatureDifficultyStatStore.size())
            _creatureDifficultyStatStore.resize(entry + 1);

        CreatureDifficultyStat creatureDiffStat;
        creatureDiffStat.Entry           = entry;
        creatureDiffStat.Difficulty      = fields[1].GetUInt8();
        creatureDiffStat.dmg_multiplier  = fields[2].GetFloat();
        creatureDiffStat.ModHealth       = fields[3].GetFloat();

        _creatureDifficultyStatStore[entry].push_back(creatureDiffStat);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature difficulty stat  definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadCreatureTemplateAddons()
{
    uint32 oldMSTime = getMSTime();

    //                                                0       1       2      3       4       5      6
    QueryResult result = WorldDatabase.Query("SELECT entry, path_id, mount, bytes1, bytes2, emote, auras FROM creature_template_addon");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature template addon definitions. DB table `creature_template_addon` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (!sObjectMgr->GetCreatureTemplate(entry))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Creature template (Entry: %u) does not exist but has a record in `creature_template_addon`", entry);
            continue;
        }

        CreatureAddon& creatureAddon = _creatureTemplateAddonStore[entry];

        creatureAddon.path_id = fields[1].GetUInt32();
        creatureAddon.mount   = fields[2].GetUInt32();
        creatureAddon.bytes1  = fields[3].GetUInt32();
        creatureAddon.bytes2  = fields[4].GetUInt32();
        creatureAddon.emote   = fields[5].GetUInt32();

        Tokenizer tokens(fields[6].GetString(), ' ');
        uint8 i = 0;
        creatureAddon.auras.resize(tokens.size());
        for (Tokenizer::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
        {
            SpellInfo const* AdditionalSpellInfo = sSpellMgr->GetSpellInfo(uint32(atol(*itr)));
            if (!AdditionalSpellInfo || AdditionalSpellInfo->HasAura(SPELL_AURA_CONTROL_VEHICLE))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has wrong spell %u defined in `auras` field in `creature_template_addon`.", entry, uint32(atol(*itr)));
                continue;
            }
            creatureAddon.auras[i++] = uint32(atol(*itr));
        }

        if (creatureAddon.mount)
        {
            if (!sCreatureDisplayInfoStore.LookupEntry(creatureAddon.mount))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has invalid displayInfoId (%u) for mount defined in `creature_template_addon`", entry, creatureAddon.mount);
                creatureAddon.mount = 0;
            }
        }

        if (!sEmotesStore.LookupEntry(creatureAddon.emote))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has invalid emote (%u) defined in `creature_addon`.", entry, creatureAddon.emote);
            creatureAddon.emote = 0;
        }

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature template addons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::CheckCreatureTemplate(CreatureTemplate const* cInfo)
{
    if (!cInfo || !cInfo->Entry)
        return;
   
    if (cInfo->faction)
    {
        FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction);
        if (!factionTemplate)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has non-existing faction template (%u).", cInfo->Entry, cInfo->faction);
    }

    if (!cInfo->unit_class || ((1 << (cInfo->unit_class-1)) & CLASSMASK_ALL_CREATURES) == 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has invalid unit_class (%u) in creature_template. Set to 1 (UNIT_CLASS_WARRIOR).", cInfo->Entry, cInfo->unit_class);
        const_cast<CreatureTemplate*>(cInfo)->unit_class = UNIT_CLASS_WARRIOR;
    }

    if (cInfo->dmgschool >= MAX_SPELL_SCHOOL)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has invalid spell school value (%u) in `dmgschool`.", cInfo->Entry, cInfo->dmgschool);
        const_cast<CreatureTemplate*>(cInfo)->dmgschool = SPELL_SCHOOL_NORMAL;
    }

    if (cInfo->baseattacktime == 0)
        const_cast<CreatureTemplate*>(cInfo)->baseattacktime  = BASE_ATTACK_TIME;

    if (cInfo->rangeattacktime == 0)
        const_cast<CreatureTemplate*>(cInfo)->rangeattacktime = BASE_ATTACK_TIME;

    if ((cInfo->npcflag & UNIT_NPC_FLAG_TRAINER) && cInfo->trainer_type >= MAX_TRAINER_TYPE)
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has wrong trainer type %u.", cInfo->Entry, cInfo->trainer_type);

    if (cInfo->speed_walk == 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has wrong value (%f) in speed_walk, set to 1.", cInfo->Entry, cInfo->speed_walk);
        const_cast<CreatureTemplate*>(cInfo)->speed_walk = 1.0f;
    }

    if (cInfo->speed_run == 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has wrong value (%f) in speed_run, set to 1.14286.", cInfo->Entry, cInfo->speed_run);
        const_cast<CreatureTemplate*>(cInfo)->speed_run = 1.14286f;
    }

    if (cInfo->Family && !sCreatureFamilyStore.LookupEntry(cInfo->Family) && cInfo->Family != CREATURE_FAMILY_HORSE_CUSTOM)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has invalid creature family (%u) in `family`.", cInfo->Entry, cInfo->Family);
        const_cast<CreatureTemplate*>(cInfo)->Family = 0;
    }

    if (cInfo->InhabitType <= 0 || cInfo->InhabitType > INHABIT_ANYWHERE)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has wrong value (%u) in `InhabitType`, creature will not correctly walk/swim/fly.", cInfo->Entry, cInfo->InhabitType);
        const_cast<CreatureTemplate*>(cInfo)->InhabitType = INHABIT_ANYWHERE;
    }

    if (cInfo->HoverHeight < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has wrong value (%f) in `HoverHeight`", cInfo->Entry, cInfo->HoverHeight);
        const_cast<CreatureTemplate*>(cInfo)->HoverHeight = 1.0f;
    }

    if (cInfo->VehicleId)
    {
        VehicleEntry const* vehId = sVehicleStore.LookupEntry(cInfo->VehicleId);
        if (!vehId)
        {
             TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has a non-existing VehicleId (%u). This *WILL* cause the client to freeze!", cInfo->Entry, cInfo->VehicleId);
             const_cast<CreatureTemplate*>(cInfo)->VehicleId = 0;
        }
    }

    for (uint8 j = 0; j < CREATURE_MAX_SPELLS; ++j)
    {
        if (cInfo->spells[j] && !sSpellMgr->GetSpellInfo(cInfo->spells[j]))
        {
            WorldDatabase.PExecute("UPDATE creature_template SET spell%d = 0 WHERE entry = %u", j+1, cInfo->Entry);
            TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has non-existing Spell%d (%u), set to 0.", cInfo->Entry, j+1, cInfo->spells[j]);
            const_cast<CreatureTemplate*>(cInfo)->spells[j] = 0;
        }
    }

    if (cInfo->MovementType >= MAX_DB_MOTION_TYPE)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (Entry: %u) has wrong movement generator type (%u), ignored and set to IDLE.", cInfo->Entry, cInfo->MovementType);
        const_cast<CreatureTemplate*>(cInfo)->MovementType = IDLE_MOTION_TYPE;
    }

    if (uint32 badFlags = (cInfo->flags_extra & ~CREATURE_FLAG_EXTRA_DB_ALLOWED))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_template` lists creature (Entry: %u) with disallowed `flags_extra` %u, removing incorrect flag.", cInfo->Entry, badFlags);
        const_cast<CreatureTemplate*>(cInfo)->flags_extra &= CREATURE_FLAG_EXTRA_DB_ALLOWED;
    }

    if (cInfo->minlevel < 1 || cInfo->minlevel >= STRONG_MAX_LEVEL)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (ID: %u): MinLevel %i is not within [1, 255], value has been set to 1.", cInfo->Entry, cInfo->minlevel);
        const_cast<CreatureTemplate*>(cInfo)->minlevel = 1;
    }

    if (cInfo->maxlevel < 1 || cInfo->maxlevel >= STRONG_MAX_LEVEL)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (ID: %u): MaxLevel %i is not within [1, 255], value has been set to 1.", cInfo->Entry, cInfo->maxlevel);
        const_cast<CreatureTemplate*>(cInfo)->maxlevel = 1;
    }

    if (cInfo->RequiredExpansion == -1)
    {
        if (MAX_LEVEL > cInfo->minlevel)
            const_cast<CreatureTemplate*>(cInfo)->minlevel = MAX_LEVEL;
        if (MAX_LEVEL > cInfo->maxlevel)
            const_cast<CreatureTemplate*>(cInfo)->maxlevel = MAX_LEVEL;
        const_cast<CreatureTemplate*>(cInfo)->RequiredExpansion = CURRENT_EXPANSION;
    }
    //
    FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    if (!entry)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (template id: %u) has invalid faction (faction template id) #%u", cInfo->Entry, cInfo->faction);
        const_cast<CreatureTemplate*>(cInfo)->faction = 35;
    }
}

void ObjectMgr::LoadCreatureAddons()
{
    uint32 oldMSTime = getMSTime();

    //                                                0       1       2      3       4       5      6
    QueryResult result = WorldDatabase.Query("SELECT guid, path_id, mount, bytes1, bytes2, emote, auras FROM creature_addon");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature addon definitions. DB table `creature_addon` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        ObjectGuid::LowType guid = fields[0].GetUInt64();

        CreatureData const* creData = GetCreatureData(guid);
        if (!creData)
        {
            WorldDatabase.PExecute("DELETE FROM creature_addon WHERE guid = %u", guid);
            //TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: " UI64FMTD ") does not exist but has a record in `creature_addon`", guid);
            continue;
        }

        CreatureAddon& creatureAddon = _creatureAddonStore[guid];

        creatureAddon.path_id = fields[1].GetUInt32();
        if (creData->movementType == WAYPOINT_MOTION_TYPE && !creatureAddon.path_id)
        {
            const_cast<CreatureData*>(creData)->movementType = IDLE_MOTION_TYPE;
            TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID " UI64FMTD ") has movement type set to WAYPOINT_MOTION_TYPE but no path assigned", guid);
        }

        creatureAddon.mount   = fields[2].GetUInt32();
        creatureAddon.bytes1  = fields[3].GetUInt32();
        creatureAddon.bytes2  = fields[4].GetUInt32();
        creatureAddon.emote   = fields[5].GetUInt32();

        Tokenizer tokens(fields[6].GetString(), ' ');
        uint8 i = 0;
        creatureAddon.auras.resize(tokens.size());
        for (Tokenizer::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
        {
            SpellInfo const* AdditionalSpellInfo = sSpellMgr->GetSpellInfo(uint32(atol(*itr)));
            if (!AdditionalSpellInfo || AdditionalSpellInfo->HasAura(SPELL_AURA_CONTROL_VEHICLE))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: " UI64FMTD ") has wrong spell %u defined in `auras` field in `creature_addon`.", guid, uint32(atol(*itr)));
                continue;
            }
            creatureAddon.auras[i++] = uint32(atol(*itr));
        }

        if (creatureAddon.mount)
        {
            if (!sCreatureDisplayInfoStore.LookupEntry(creatureAddon.mount))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: " UI64FMTD ") has invalid displayInfoId (%u) for mount defined in `creature_addon`", guid, creatureAddon.mount);
                creatureAddon.mount = 0;
            }
        }

        if (!sEmotesStore.LookupEntry(creatureAddon.emote))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: " UI64FMTD ") has invalid emote (%u) defined in `creature_addon`.", guid, creatureAddon.emote);
            creatureAddon.emote = 0;
        }

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature addons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

CreatureAddon const* ObjectMgr::GetCreatureAddon(ObjectGuid::LowType const& lowguid)
{
    return Trinity::Containers::MapGetValuePtr(_creatureAddonStore, lowguid);
}

CreatureAddon const* ObjectMgr::GetCreatureTemplateAddon(uint32 entry)
{
    return Trinity::Containers::MapGetValuePtr(_creatureTemplateAddonStore, entry);
}

EquipmentInfo const* ObjectMgr::GetEquipmentInfo(uint32 entry, int8& id)
{
    EquipmentInfoContainer::const_iterator itr = _equipmentInfoStore.find(entry);
    if (itr == _equipmentInfoStore.end())
        return nullptr;

    if (itr->second.empty())
        return nullptr;

    if (id == -1) // select a random element
    {
        auto ritr = itr->second.begin();
        std::advance(ritr, urand(0u, itr->second.size() - 1));
        id = std::distance(itr->second.begin(), ritr) + 1;
        return &ritr->second;
    }

    return Trinity::Containers::MapGetValuePtr(itr->second, id);
}

void ObjectMgr::LoadEquipmentTemplates()
{
    uint32 oldMSTime = getMSTime();

    //                                                   0       1      2        3        4        5        6       7
    QueryResult result = WorldDatabase.Query("SELECT CreatureID, ID, ItemID1, ItemID2, ItemID3, ItemID4, ItemID5, ItemID6 FROM creature_equip_template");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature equipment templates. DB table `creature_equip_template` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint8 id = fields[1].GetUInt8();

        EquipmentInfo& equipmentInfo = _equipmentInfoStore[entry][id];

        equipmentInfo.ItemEntry[0] = fields[2].GetUInt32();
        equipmentInfo.ItemEntry[1] = fields[3].GetUInt32();
        equipmentInfo.ItemEntry[2] = fields[4].GetUInt32();
        equipmentInfo.ItemEntry[3] = fields[5].GetUInt32();
        equipmentInfo.ItemEntry[4] = fields[6].GetUInt32();
        equipmentInfo.ItemEntry[5] = fields[7].GetUInt32();

        for (uint8 i = 0; i < MAX_EQUIPMENT_ITEMS; ++i)
        {
            if (!equipmentInfo.ItemEntry[i])
                continue;

            ItemEntry const* dbcItem = sItemStore.LookupEntry(equipmentInfo.ItemEntry[i]);

            if (!dbcItem)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Unknown item (entry=%u) in creature_equip_template.itemEntry%u for entry = %u, forced to 0.",
                    equipmentInfo.ItemEntry[i], i+1, entry);
                equipmentInfo.ItemEntry[i] = 0;
                //WorldDatabase.PExecute("UPDATE creature_equip_template SET ItemID1 = %u, ItemID2 = %u, ItemID3 = %u WHERE CreatureID = %u", equipmentInfo.ItemEntry[0], equipmentInfo.ItemEntry[1], equipmentInfo.ItemEntry[2], entry);
                continue;
            }

            if (dbcItem->InventoryType != INVTYPE_WEAPON &&
                dbcItem->InventoryType != INVTYPE_SHIELD &&
                dbcItem->InventoryType != INVTYPE_RANGED &&
                dbcItem->InventoryType != INVTYPE_2HWEAPON &&
                dbcItem->InventoryType != INVTYPE_WEAPONMAINHAND &&
                dbcItem->InventoryType != INVTYPE_WEAPONOFFHAND &&
                dbcItem->InventoryType != INVTYPE_HOLDABLE &&
                dbcItem->InventoryType != INVTYPE_THROWN &&
                dbcItem->InventoryType != INVTYPE_RANGEDRIGHT)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Item (entry=%u) in creature_equip_template.itemEntry%u for entry = %u is not equipable in a hand, forced to 0.",
                    equipmentInfo.ItemEntry[i], i+1, entry);
                equipmentInfo.ItemEntry[i] = 0;
                //WorldDatabase.PExecute("UPDATE creature_equip_template SET ItemID1 = %u, ItemID2 = %u, ItemID3 = %u WHERE CreatureID = %u", equipmentInfo.ItemEntry[0], equipmentInfo.ItemEntry[1], equipmentInfo.ItemEntry[2], entry);
            }
        }

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u equipment templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

CreatureModelInfo const* ObjectMgr::GetCreatureModelInfo(uint32 modelId)
{
    return Trinity::Containers::MapGetValuePtr(_creatureModelStore, modelId);
}

int32 ObjectMgr::ChooseDisplayId(uint32 /*team*/, const CreatureTemplate* cinfo, const CreatureData* data /*= NULL*/)
{
    // Load creature model (display id)
    uint32 display_id = 0;

    if (!data || data->displayid == 0)
    {
        if (!(cinfo->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER))
            return cinfo->GetRandomValidModelId();

        switch (cinfo->Entry)
        {
            //! Scouting Map. Should have this display. Protect from DB update.uint32 ObjectMgr::GetTaxiMountDisplayId
            case 99428: display_id = 69089; break;
            case 98672: display_id = 64062; break;
            case 98695: display_id = 71171; break;
            default:
                break;
        }
    }
    else
        return data->displayid;

    if (display_id)
        return display_id;

    return cinfo->GetFirstInvisibleModel();
}

void ObjectMgr::ChooseCreatureFlags(const CreatureTemplate* cinfo, uint32& npcflag, uint32& npcflag2, uint32& unit_flags, uint32& unit_flags3, uint32& dynamicflags, const CreatureData* data /*= NULL*/)
{
    npcflag = cinfo->npcflag;
    npcflag2 = cinfo->npcflag2;
    unit_flags = cinfo->unit_flags;
    unit_flags3 = cinfo->unit_flags3;
    dynamicflags = cinfo->dynamicflags;

    if (data)
    {
        if (data->npcflag)
            npcflag = data->npcflag;

        if (data->npcflag2)
            npcflag2 = data->npcflag2;

        if (data->unit_flags)
            unit_flags = data->unit_flags;

        if (data->unit_flags3)
            unit_flags3 = data->unit_flags3;

        if (data->dynamicflags)
            dynamicflags = data->dynamicflags;
    }
}

CreatureModelInfo const* ObjectMgr::GetCreatureModelRandomGender(uint32* displayID)
{
    CreatureModelInfo const* modelInfo = GetCreatureModelInfo(*displayID);
    if (!modelInfo)
        return nullptr;

    // If a model for another gender exists, 50% chance to use it
    if (modelInfo->displayId_other_gender != 0 && urand(0, 1) == 0)
    {
        CreatureModelInfo const* minfo_tmp = GetCreatureModelInfo(modelInfo->displayId_other_gender);
        if (!minfo_tmp)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Model (Entry: %u) has displayId_other_gender %u not found in table `creature_model_info`. ", *displayID, modelInfo->displayId_other_gender);
        else
        {
            // DisplayID changed
            *displayID = modelInfo->displayId_other_gender;
            return minfo_tmp;
        }
    }

    return modelInfo;
}

void ObjectMgr::LoadCreatureModelInfo()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT DisplayID, BoundingRadius, CombatReach, DisplayID_Other_Gender, hostileId FROM creature_model_info");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature model definitions. DB table `creature_model_info` is empty.");
        return;
    }

    uint32 trigggerCreatureModelFileID[5] = { 124640, 124641, 124642, 343863, 439302 };
    _creatureModelStore.rehash(result->GetRowCount());
    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 displayId = fields[0].GetUInt32();

        CreatureDisplayInfoEntry const* creatureDisplay = sCreatureDisplayInfoStore.LookupEntry(displayId);
        if (!creatureDisplay)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_model_info` has model for not existed display id (%u).", displayId);
            //WorldDatabase.PExecute("DELETE FROM creature_model_info WHERE DisplayID = %u", displayId);
            continue;
        }
 
        CreatureModelInfo& modelInfo = _creatureModelStore[displayId];

        modelInfo.bounding_radius      = fields[1].GetFloat();
        modelInfo.combat_reach         = fields[2].GetFloat();
        modelInfo.gender               = creatureDisplay->Gender;
        modelInfo.displayId_other_gender = fields[3].GetUInt32();
        modelInfo.hostileId            = fields[4].GetUInt32();

        // Checks
        if (modelInfo.gender > GENDER_NONE || modelInfo.gender == GENDER_UNKNOWN)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_model_info` has wrong gender (%u) for display id (%u).", uint32(modelInfo.gender), displayId);
            modelInfo.gender = GENDER_MALE;
        }

        if (modelInfo.displayId_other_gender && !sCreatureDisplayInfoStore.LookupEntry(modelInfo.displayId_other_gender))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_model_info` has not existed alt.gender model (%u) for existed display id (%u).", modelInfo.displayId_other_gender, displayId);
            //WorldDatabase.PExecute("UPDATE creature_model_info SET DisplayID_Other_Gender = 0 WHERE DisplayID = %u", displayId);
            modelInfo.displayId_other_gender = 0;
        }

        if (modelInfo.hostileId && !sCreatureDisplayInfoStore.LookupEntry(modelInfo.hostileId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_model_info` has not existed alt.hostileId model (%u) for existed display id (%u).", modelInfo.hostileId, displayId);
            modelInfo.hostileId = 0;
        }

        if (modelInfo.combat_reach < 0.1f)
            modelInfo.combat_reach = DEFAULT_COMBAT_REACH;

        if (CreatureModelDataEntry const* modelData = sCreatureModelDataStore.LookupEntry(creatureDisplay->ModelID))
            for (uint32 i = 0; i < std::extent<decltype(trigggerCreatureModelFileID)>::value; ++i)
                if (uint32(modelData->FileDataID) == trigggerCreatureModelFileID[i])
                {
                    modelInfo.is_trigger = true;
                    break;
                }

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature model based info in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadLinkedRespawn()
{
    uint32 oldMSTime = getMSTime();

    _linkedRespawnStore.clear();
    //                                                 0        1          2
    QueryResult result = WorldDatabase.Query("SELECT guid, linkedGuid, linkType FROM linked_respawn ORDER BY guid ASC");

    if (!result)
    {
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        ObjectGuid::LowType guidLow = fields[0].GetUInt64();
        ObjectGuid::LowType linkedGuidLow = fields[1].GetUInt64();
        uint8  linkType = fields[2].GetUInt8();

        ObjectGuid guid, linkedGuid;
        bool error = false;
        switch (linkType)
        {
            case CREATURE_TO_CREATURE:
            {
                const CreatureData* slave = GetCreatureData(guidLow);
                if (!slave)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get creature data for GUIDLow " UI64FMTD "", guidLow);
                    error = true;
                    break;
                }

                const CreatureData* master = GetCreatureData(linkedGuidLow);
                if (!master)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get creature data for GUIDLow " UI64FMTD "", linkedGuidLow);
                    error = true;
                    break;
                }

                const MapEntry* const map = sMapStore.LookupEntry(master->mapid);
                if (!map || !map->Instanceable() || (master->mapid != slave->mapid))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Creature '" UI64FMTD "' linking to '%u' on an unpermitted map.", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                if (!(master->spawnMask & slave->spawnMask))  // they must have a possibility to meet (normal/heroic difficulty)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LinkedRespawn: Creature '" UI64FMTD "' linking to '%u' with not corresponding spawnMask", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                guid = ObjectGuid::Create<HighGuid::Creature>(slave->mapid, slave->id, guidLow);
                linkedGuid = ObjectGuid::Create<HighGuid::Creature>(master->mapid, master->id, linkedGuidLow);
                break;
            }
            case CREATURE_TO_GO:
            {
                const CreatureData* slave = GetCreatureData(guidLow);
                if (!slave)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get creature data for GUIDLow " UI64FMTD "", guidLow);
                    error = true;
                    break;
                }

                const GameObjectData* master = GetGOData(linkedGuidLow);
                if (!master)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get gameobject data for GUIDLow " UI64FMTD "", linkedGuidLow);
                    error = true;
                    break;
                }

                const MapEntry* const map = sMapStore.LookupEntry(master->mapid);
                if (!map || !map->Instanceable() || (master->mapid != slave->mapid))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Creature '" UI64FMTD "' linking to '%u' on an unpermitted map.", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                if (!(master->spawnMask & slave->spawnMask))  // they must have a possibility to meet (normal/heroic difficulty)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LinkedRespawn: Creature '" UI64FMTD "' linking to '" UI64FMTD "' with not corresponding spawnMask", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                guid = ObjectGuid::Create<HighGuid::Creature>(slave->mapid, slave->id, guidLow);
                linkedGuid = ObjectGuid::Create<HighGuid::GameObject>(master->mapid, master->id, linkedGuidLow);
                break;
            }
            case GO_TO_GO:
            {
                const GameObjectData* slave = GetGOData(guidLow);
                if (!slave)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get gameobject data for GUIDLow " UI64FMTD "", guidLow);
                    error = true;
                    break;
                }

                const GameObjectData* master = GetGOData(linkedGuidLow);
                if (!master)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get gameobject data for GUIDLow " UI64FMTD "", linkedGuidLow);
                    error = true;
                    break;
                }

                const MapEntry* const map = sMapStore.LookupEntry(master->mapid);
                if (!map || !map->Instanceable() || (master->mapid != slave->mapid))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Creature '" UI64FMTD "' linking to '" UI64FMTD "' on an unpermitted map.", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                if (!(master->spawnMask & slave->spawnMask))  // they must have a possibility to meet (normal/heroic difficulty)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LinkedRespawn: Creature '" UI64FMTD "' linking to '" UI64FMTD "' with not corresponding spawnMask", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                guid = ObjectGuid::Create<HighGuid::GameObject>(slave->mapid, slave->id, guidLow);
                linkedGuid = ObjectGuid::Create<HighGuid::GameObject>(master->mapid, master->id, linkedGuidLow);
                break;
            }
            case GO_TO_CREATURE:
            {
                const GameObjectData* slave = GetGOData(guidLow);
                if (!slave)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get gameobject data for GUIDLow " UI64FMTD "", guidLow);
                    error = true;
                    break;
                }

                const CreatureData* master = GetCreatureData(linkedGuidLow);
                if (!master)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Couldn't get creature data for GUIDLow " UI64FMTD "", linkedGuidLow);
                    error = true;
                    break;
                }

                const MapEntry* const map = sMapStore.LookupEntry(master->mapid);
                if (!map || !map->Instanceable() || (master->mapid != slave->mapid))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Creature '" UI64FMTD "' linking to '" UI64FMTD "' on an unpermitted map.", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                if (!(master->spawnMask & slave->spawnMask))  // they must have a possibility to meet (normal/heroic difficulty)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LinkedRespawn: Creature '" UI64FMTD "' linking to '" UI64FMTD "' with not corresponding spawnMask", guidLow, linkedGuidLow);
                    error = true;
                    break;
                }

                guid = ObjectGuid::Create<HighGuid::GameObject>(slave->mapid, slave->id, guidLow);
                linkedGuid = ObjectGuid::Create<HighGuid::Creature>(master->mapid, master->id, linkedGuidLow);
                break;
            }
            default:
                break;
        }

        if (!error)
            _linkedRespawnStore[guid] = linkedGuid;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded " UI64FMTD " linked respawns in %u ms", uint64(_linkedRespawnStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

bool ObjectMgr::SetCreatureLinkedRespawn(ObjectGuid::LowType const& guidLow, ObjectGuid::LowType const& linkedGuidLow)
{
    if (!guidLow)
        return false;

    const CreatureData* master = GetCreatureData(guidLow);
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Creature>(master->mapid, master->id, guidLow);

    if (!linkedGuidLow) // we're removing the linking
    {
        _linkedRespawnStore.erase(guid);
        PreparedStatement *stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CRELINKED_RESPAWN);
        stmt->setUInt64(0, guidLow);
        WorldDatabase.Execute(stmt);
        return true;
    }

    const CreatureData* slave = GetCreatureData(linkedGuidLow);

    const MapEntry* const map = sMapStore.LookupEntry(master->mapid);
    if (!map || !map->Instanceable() || (master->mapid != slave->mapid))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature '" UI64FMTD "' linking to '" UI64FMTD "' on an unpermitted map.", guidLow, linkedGuidLow);
        return false;
    }

    if (!(master->spawnMask & slave->spawnMask))  // they must have a possibility to meet (normal/heroic difficulty)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "LinkedRespawn: Creature '" UI64FMTD "' linking to '" UI64FMTD "' with not corresponding spawnMask", guidLow, linkedGuidLow);
        return false;
    }

    ObjectGuid linkedGuid = ObjectGuid::Create<HighGuid::Creature>(slave->mapid, slave->id, linkedGuidLow);

    _linkedRespawnStore[guid] = linkedGuid;
    PreparedStatement *stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_CREATURE_LINKED_RESPAWN);
    stmt->setUInt64(0, guidLow);
    stmt->setUInt64(1, linkedGuidLow);
    WorldDatabase.Execute(stmt);
    return true;
}

void ObjectMgr::LoadTempSummons()
{
    uint32 oldMSTime = getMSTime();

    _tempSummonDataStore.clear();   // needed for reload case

    //                                               0           1             2        3      4           5           6           7              8        9          10         11          12
    QueryResult result = WorldDatabase.Query("SELECT summonerId, summonerType, groupId, entry, position_x, position_y, position_z, orientation, count, actionType, distance, summonType, summonTime FROM creature_summon_groups");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 temp summons. DB table `creature_summon_groups` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 summonerId               = fields[0].GetUInt32();
        SummonerType summonerType       = SummonerType(fields[1].GetUInt8());
        uint8 group                     = fields[2].GetUInt8();

        switch (summonerType)
        {
            case SUMMONER_TYPE_CREATURE:
                if (!GetCreatureTemplate(summonerId))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_summon_groups` has summoner with non existing entry %u for creature summoner type, skipped.", summonerId);
                    continue;
                }
                break;
            case SUMMONER_TYPE_GAMEOBJECT:
                if (!GetGameObjectTemplate(summonerId))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_summon_groups` has summoner with non existing entry %u for gameobject summoner type, skipped.", summonerId);
                    continue;
                }
                break;
            case SUMMONER_TYPE_MAP:
                if (!sMapStore.LookupEntry(summonerId))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_summon_groups` has summoner with non existing entry %u for map summoner type, skipped.", summonerId);
                    continue;
                }
                break;
            default:
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_summon_groups` has unhandled summoner type %u for summoner %u, skipped.", summonerType, summonerId);
                continue;
        }

        TempSummonData data;
        data.entry                      = fields[3].GetUInt32();

        if (!GetCreatureTemplate(data.entry))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_summon_groups` has creature in group [Summoner ID: %u, Summoner Type: %u, Group ID: %u] with non existing creature entry %u, skipped.", summonerId, summonerType, group, data.entry);
            continue;
        }

        float posX                      = fields[4].GetFloat();
        float posY                      = fields[5].GetFloat();
        float posZ                      = fields[6].GetFloat();
        float orientation               = fields[7].GetFloat();

        data.pos.Relocate(posX, posY, posZ, orientation);

        data.count                      = fields[8].GetUInt8();
        data.actionType                 = fields[9].GetUInt8();
        data.distance                   = fields[10].GetFloat();
        data.sumType                    = TempSummonType(fields[11].GetUInt8());

        if (data.sumType > TEMPSUMMON_MANUAL_DESPAWN)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_summon_groups` has unhandled temp summon type %u in group [Summoner ID: %u, Summoner Type: %u, Group ID: %u] for creature entry %u, skipped.", data.sumType, summonerId, summonerType, group, data.entry);
            continue;
        }

        data.time                       = fields[12].GetUInt32();

        TempSummonGroupKey key(summonerId, summonerType, group);
        _tempSummonDataStore[key].push_back(data);

        ++count;

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u temp summons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

std::vector<TempSummonData> const* ObjectMgr::GetSummonGroup(uint32 summonerId, SummonerType summonerType, uint8 group) const
{
    return Trinity::Containers::MapGetValuePtr(_tempSummonDataStore, TempSummonGroupKey(summonerId, summonerType, group));
}

void ObjectMgr::LoadCreatures()
{
    uint32 oldMSTime = getMSTime();

    //                                                      0            1      2      3        4           5           6           7           8            9            10            11
    QueryResult result = WorldDatabase.Query("SELECT creature.guid, id, map, zoneId, areaId, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, "
    //        12            13         14          15           16        17         18          19             20                21                   22                    23                    24
        "currentwaypoint, curhealth, curmana, MovementType, spawnMask, phaseMask, eventEntry, pool_entry, creature.npcflag, creature.npcflag2, creature.unit_flags, creature.unit_flags3, creature.dynamicflags, "
    //           25                26          27       28		   29	    30		    31               32
        "creature.isActive, creature.PhaseId, AiID, MovementID, MeleeID, skipClone, personal_size, isTeemingSpawn "
        "FROM creature "
        "LEFT OUTER JOIN game_event_creature ON creature.guid = game_event_creature.guid "
        "LEFT OUTER JOIN pool_creature ON creature.guid = pool_creature.guid "
        "ORDER BY `map` ASC, `guid` ASC");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creatures. DB table `creature` is empty.");
        return;
    }

    // Build single time for check spawnmask
    std::map<uint32, uint64> spawnMasks;
    for (auto& mapDifficultyPair : sDB2Manager.GetAllMapsDifficultyes())
        for (auto& difficultyPair : mapDifficultyPair.second)
            spawnMasks[mapDifficultyPair.first] |= (UI64LIT(1) << difficultyPair.first);

    _creatureDataStore.rehash(result->GetRowCount());

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint8 index = 0;

        ObjectGuid::LowType guid = fields[index++].GetUInt64();
        uint32 entry        = fields[index++].GetUInt32();

        CreatureTemplate const* cInfo = GetCreatureTemplate(entry);
        if (!cInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` has creature (GUID: " UI64FMTD ") with non existing creature entry %u, skipped.", guid, entry);
            continue;
        }

        CreatureData& data = _creatureDataStore[guid];
        data.guid           = guid;
        data.id             = entry;
        data.mapid          = fields[index++].GetUInt16();
        data.zoneId         = fields[index++].GetUInt16();
        data.areaId         = fields[index++].GetUInt16();
        data.displayid      = fields[index++].GetUInt32();
        data.equipmentId    = fields[index++].GetInt8();
        data.posX           = fields[index++].GetFloat();
        data.posY           = fields[index++].GetFloat();
        data.posZ           = fields[index++].GetFloat();
        data.orientation    = fields[index++].GetFloat();
        data.spawntimesecs  = fields[index++].GetUInt32();
        data.spawndist      = fields[index++].GetFloat();
        data.currentwaypoint= fields[index++].GetUInt32();
        data.curhealth      = fields[index++].GetUInt32();
        data.curmana        = fields[index++].GetUInt32();
        data.movementType   = fields[index++].GetUInt8();
        data.spawnMask      = fields[index++].GetUInt64();
        data.phaseMask      = fields[index++].GetUInt32();
        int16 gameEvent     = fields[index++].GetInt16();
        uint32 PoolId       = fields[index++].GetUInt32();
        data.npcflag        = fields[index++].GetUInt32();
        data.npcflag2       = fields[index++].GetUInt32();
        data.unit_flags     = fields[index++].GetUInt32();
        data.unit_flags3	= fields[index++].GetUInt32();
        data.dynamicflags   = fields[index++].GetUInt32();
        data.isActive       = fields[index++].GetBool();

        Tokenizer phasesToken(fields[index++].GetString(), ' ', 100);
        for (auto itr : phasesToken)
            if (PhaseEntry const* phase = sPhaseStore.LookupEntry(uint32(strtoull(itr, nullptr, 10))))
                data.PhaseID.insert(phase->ID);

        data.AiID = fields[index++].GetUInt32();
        data.MovementID = fields[index++].GetUInt32();
        data.MeleeID = fields[index++].GetUInt32();
        data.skipClone       = fields[index++].GetBool();
        data.personalSize    = fields[index++].GetFloat();
        data.isTeemingSpawn  = fields[index++].GetBool();
        data.gameEvent = gameEvent;
        data.pool = PoolId;
        // data.MaxVisible = cInfo->MaxVisible;

        // check near npc with same entry.
        // auto lastCreature = lastEntryCreature.find(entry);
        // if (lastCreature != lastEntryCreature.end())
        // {
            // if (data.mapid == lastCreature->second->mapid)
            // {
                // float dx1 = lastCreature->second->posX - data.posX;
                // float dy1 = lastCreature->second->posY - data.posY;
                // float dz1 = lastCreature->second->posZ - data.posZ;

                // float distsq1 = dx1*dx1 + dy1*dy1 + dz1*dz1;
                // if (distsq1 < 0.5f && data.skipClone == 0)
                // {
                    // // split phaseID
                    // for (auto phaseID : data.PhaseID)
                        // lastCreature->second->PhaseID.insert(phaseID);

                    // lastCreature->second->phaseMask |= data.phaseMask;
                    // lastCreature->second->spawnMask |= data.spawnMask;
                    // WorldDatabase.PExecute("UPDATE creature SET phaseMask = %u, spawnMask = " UI64FMTD " WHERE guid = %u", lastCreature->second->phaseMask, lastCreature->second->spawnMask, lastCreature->second->guid);
                    // WorldDatabase.PExecute("DELETE FROM creature WHERE guid = %u", guid);
                    // TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have clone npc %u witch stay too close (dist: %f). original npc guid %u. npc with guid %u will be deleted.", entry, distsq1, lastCreature->second->guid, guid);
                    // continue;
                // }
            // }else
                // lastEntryCreature[entry] = &data;

        // }else
            // lastEntryCreature[entry] = &data;

        MapEntry const* mapEntry = sMapStore.LookupEntry(data.mapid);
        if (!mapEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (GUID: " UI64FMTD ") that spawned at not existed map (Id: %u), skipped.", guid, data.mapid);
            continue;
        }

        if (!mapEntry->IsBattlegroundOrArena())
        {
            if (!IsTransportMap(data.mapid) && data.spawnMask & ~spawnMasks[data.mapid])
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (GUID: " UI64FMTD ") that have wrong spawn mask " UI64FMTD " including not supported difficulty modes for map (Id: %u) spawnMasks[data.mapid]: %u.", guid, data.spawnMask, data.mapid, spawnMasks[data.mapid]);
                WorldDatabase.PExecute("UPDATE creature SET spawnMask = " UI64FMTD " WHERE guid = %u", spawnMasks[data.mapid], guid);
                data.spawnMask = spawnMasks[data.mapid];
            }
        }

        // -1 random, 0 no equipment,
        if (data.equipmentId != 0)
        {
            if (!GetEquipmentInfo(data.id, data.equipmentId))
            {
                //WorldDatabase.PExecute("UPDATE creature SET equipment_id = 0 WHERE guid = %u", guid);
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (Entry: %u) with equipment_id %u not found in table `creature_equip_template`, set to no equipment.", data.id, data.equipmentId);
                data.equipmentId = 0;
            }
        }

        if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
        {
            if (!mapEntry || !mapEntry->IsDungeon())
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (GUID: " UI64FMTD " Entry: %u) with `creature_template`.`flags_extra` including CREATURE_FLAG_EXTRA_INSTANCE_BIND but creature are not in instance.", guid, data.id);
        }

        if (data.spawndist < 0.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (GUID: " UI64FMTD " Entry: %u) with `spawndist`< 0, set to 0.", guid, data.id);
            data.spawndist = 0.0f;
        }
        else if (data.movementType == RANDOM_MOTION_TYPE)
        {
            if (data.spawndist == 0.0f)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (GUID: " UI64FMTD " Entry: %u) with `MovementType`=1 (random movement) but with `spawndist`=0, replace by idle movement type (0).", guid, data.id);
                data.movementType = IDLE_MOTION_TYPE;
            }
        }
        else if (data.movementType == IDLE_MOTION_TYPE)
        {
            if (data.spawndist != 0.0f)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (GUID: " UI64FMTD " Entry: %u) with `MovementType`=0 (idle) have `spawndist`<>0, set to 0.", guid, data.id);
                data.spawndist = 0.0f;
            }
        }

        if (data.phaseMask == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature` have creature (GUID: " UI64FMTD " Entry: %u) with `phaseMask`=0 (not visible for anyone), set to 1.", guid, data.id);
            data.phaseMask = 1;
        }

        // Add to grid if not managed by the game event or pool system
        if (gameEvent == 0 && PoolId == 0)
            AddCreatureToGrid(guid, &data);

        if (!data.zoneId || !data.areaId)
        {
            uint32 zoneId = 0;
            uint32 areaId = 0;

            sMapMgr->GetZoneAndAreaId(zoneId, areaId, data.mapid, data.posX, data.posY, data.posZ);

            data.zoneId = zoneId;
            data.areaId = areaId;

            WorldDatabase.PExecute("UPDATE creature SET zoneId = %u, areaId = %u WHERE guid = %u", zoneId, areaId, guid);
        }

        ++count;

    } while (result->NextRow());
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creatures in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadCreatureAIInstance()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT entry, bossid, bossidtoactivete FROM creature_ai_instance");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature AI instance. DB table `creature_ai_instance` is empty.");
        return;
    }

    _creatureAIInstance.rehash(result->GetRowCount());
    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        CreatureAIInstance& aiinstance = _creatureAIInstance[entry];

        aiinstance.entry = entry;
        aiinstance.bossid = fields[1].GetUInt32();
        aiinstance.bossidactivete = fields[2].GetUInt32();

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature AI instance in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadPersonalLootTemplate()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT entry, `type`, `chance`, lootspellId, bonusspellId, cooldownid, cooldowntype, respawn, goEntry  FROM personal_loot_template");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 personal loot. DB table `personal_loot_template` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        uint32 type = fields[1].GetUInt8();
        uint32 chance = fields[2].GetUInt8();
        uint32 lootspellId = fields[3].GetUInt32();
        uint32 bonusspellId = fields[4].GetUInt32();
        uint32 cooldownid = fields[5].GetUInt32();
        uint32 ccooldowntype = fields[6].GetUInt8();
        uint32 respawn = fields[7].GetUInt8();
        uint32 goEntry = fields[8].GetUInt32();

        PersonalLootData& personalloot = _PersonalLootStore[type][entry];
        personalloot.entry = entry;
        personalloot.type = type;
        personalloot.chance = chance;
        personalloot.lootspellId = lootspellId;
        personalloot.bonusspellId = bonusspellId;
        personalloot.cooldownid = cooldownid;
        personalloot.cooldowntype = ccooldowntype;
        personalloot.respawn = respawn;
        personalloot.goEntry = goEntry;

        if(lootspellId)
        {
            PersonalLootData& personallootForSpell = _PersonalLootBySpellStore[lootspellId];
            personallootForSpell.entry = entry;
            personallootForSpell.type = type;
            personallootForSpell.chance = chance;
            personallootForSpell.lootspellId = lootspellId;
            personallootForSpell.bonusspellId = bonusspellId;
            personallootForSpell.cooldownid = cooldownid;
            personallootForSpell.cooldowntype = ccooldowntype;
            personallootForSpell.respawn = respawn;
            personallootForSpell.goEntry = goEntry;
        }
        if(bonusspellId)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(bonusspellId);
            if(!spellInfo || !spellInfo->Effects[0]->TriggerSpell)
                continue;

            PersonalLootData& personallootForSpell = _PersonalLootBySpellStore[spellInfo->Effects[0]->TriggerSpell];
            personallootForSpell.entry = entry;
            personallootForSpell.type = type;
            personallootForSpell.chance = chance;
            personallootForSpell.lootspellId = lootspellId;
            personallootForSpell.bonusspellId = bonusspellId;
            personallootForSpell.cooldownid = cooldownid;
            personallootForSpell.cooldowntype = ccooldowntype;
            personallootForSpell.respawn = respawn;
            personallootForSpell.goEntry = goEntry;
        }
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u personal loot in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::AddEventObjectToGrid(ObjectGuid::LowType const& guid, EventObjectData const* data)
{
    uint64 mask = data->spawnMask;
    for (uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if (mask & 1)
        {
            if (data->mapid >= _mapObjectGuidsStore.size())
                _mapObjectGuidsStore.resize(data->mapid + 1);

            if (i >= _mapObjectGuidsStore[data->mapid].size())
                _mapObjectGuidsStore[data->mapid].resize(i + 1);

            CellCoord cellCoord = Trinity::ComputeCellCoord(data->Pos.GetPositionX(), data->Pos.GetPositionY());
            CellObjectGuids& cell_guids = _mapObjectGuidsStore[data->mapid][i][cellCoord.GetId()];
            cell_guids.eventobject.insert(guid);
        }
    }
}

void ObjectMgr::AddConversationToGrid(ObjectGuid::LowType const& guid, ConversationSpawnData const* data)
{
    uint64 mask = data->spawnMask;
    for (uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if (mask & 1)
        {
            if (data->mapid >= _mapObjectGuidsStore.size())
                _mapObjectGuidsStore.resize(data->mapid + 1);

            if (i >= _mapObjectGuidsStore[data->mapid].size())
                _mapObjectGuidsStore[data->mapid].resize(i + 1);

            CellCoord cellCoord = Trinity::ComputeCellCoord(data->posX, data->posY);
            CellObjectGuids& cell_guids = _mapObjectGuidsStore[data->mapid][i][cellCoord.GetId()];
            cell_guids.conversation.insert(guid);
        }
    }
}

void ObjectMgr::AddCreatureToGrid(ObjectGuid::LowType const& guid, CreatureData const* data)
{
    uint64 mask = data->spawnMask;
    for (uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if (mask & 1)
        {
            if (data->mapid >= _mapObjectGuidsStore.size())
                _mapObjectGuidsStore.resize(data->mapid + 1);

            if (i >= _mapObjectGuidsStore[data->mapid].size())
                _mapObjectGuidsStore[data->mapid].resize(i + 1);

            CellCoord cellCoord = Trinity::ComputeCellCoord(data->posX, data->posY);
            CellObjectGuids& cell_guids = _mapObjectGuidsStore[data->mapid][i][cellCoord.GetId()];
            cell_guids.creatures.insert(guid);
        }
    }
}

void ObjectMgr::RemoveCreatureFromGrid(ObjectGuid::LowType const& guid, CreatureData const* data)
{
    uint64 mask = data->spawnMask;
    for (uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if (mask & 1)
        {
            if (data->mapid >= _mapObjectGuidsStore.size())
                _mapObjectGuidsStore.resize(data->mapid + 1);

            if (i >= _mapObjectGuidsStore[data->mapid].size())
                _mapObjectGuidsStore[data->mapid].resize(i + 1);

            CellCoord cellCoord = Trinity::ComputeCellCoord(data->posX, data->posY);
            CellObjectGuids& cell_guids = _mapObjectGuidsStore[data->mapid][i][cellCoord.GetId()];
            cell_guids.creatures.erase(guid);
        }
    }
}

ObjectGuid::LowType ObjectMgr::AddGOData(uint32 entry, uint32 mapId, float x, float y, float z, float o, uint32 spawntimedelay, float rotation0, float rotation1, float rotation2, float rotation3, uint32 aid /*= 0*/)
{
    GameObjectTemplate const* goinfo = GetGameObjectTemplate(entry);
    if (!goinfo)
        return UI64LIT(0);

    Map* map = sMapMgr->CreateBaseMap(mapId);
    if (!map)
        return UI64LIT(0);

    ObjectGuid::LowType guid = GetGenerator<HighGuid::GameObject>()->Generate();
    GameObjectData& data = NewGOData(guid);
    data.id             = entry;
    data.mapid          = mapId;
    data.posX           = x;
    data.posY           = y;
    data.posZ           = z;
    data.orientation    = o;
    data.rotation.x      = rotation0;
    data.rotation.y      = rotation1;
    data.rotation.z      = rotation2;
    data.rotation.w      = rotation3;
    data.spawntimesecs  = spawntimedelay;
    data.animprogress   = 100;
    data.spawnMask      = 1;
    data.go_state       = GO_STATE_READY;
    data.phaseMask      = PHASEMASK_NORMAL;
    data.artKit         = goinfo->type == GAMEOBJECT_TYPE_CONTROL_ZONE ? 21 : 0;
    data.dbData = false;
    data.MaxVisible = goinfo->MaxVisible;
    data.AiID = aid;
    AddGameobjectToGrid(guid, &data);

    // Spawn if necessary (loaded grids only)
    // We use spawn coords to spawn
    if (!map->Instanceable() && map->IsGridLoaded(x, y))
    {
        GameObject* go = sObjectMgr->IsStaticTransport(entry) ? new StaticTransport : new GameObject;
        if (!go->LoadGameObjectFromDB(guid, map))
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "AddGOData: cannot add gameobject entry %u to map", entry);
            delete go;
            return UI64LIT(0);
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_MAPS, "AddGOData: dbguid " UI64FMTD " entry %u map %u x %f y %f z %f o %f", guid, entry, mapId, x, y, z, o);

    return guid;
}

bool ObjectMgr::MoveCreData(ObjectGuid::LowType const& guid, uint32 mapId, Position pos)
{
    CreatureData& data = NewOrExistCreatureData(guid);
    if (!data.id)
        return false;

    RemoveCreatureFromGrid(guid, &data);
    if (data.posX == pos.GetPositionX() && data.posY == pos.GetPositionY() && data.posZ == pos.GetPositionZ())
        return true;
    data.posX = pos.GetPositionX();
    data.posY = pos.GetPositionY();
    data.posZ = pos.GetPositionZ();
    data.orientation = pos.GetOrientation();
    AddCreatureToGrid(guid, &data);

    // Spawn if necessary (loaded grids only)
    if (Map* map = sMapMgr->CreateBaseMap(mapId))
    {
        // We use spawn coords to spawn
        if (!map->Instanceable() && map->IsGridLoaded(data.posX, data.posY))
        {
            Creature* creature = new Creature;
            if (!creature->LoadCreatureFromDB(guid, map))
            {
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "AddCreature: cannot add creature entry %u to map", guid);
                delete creature;
                return false;
            }
        }
    }
    return true;
}

ObjectGuid::LowType ObjectMgr::AddCreData(uint32 entry, uint32 /*team*/, uint32 mapId, float x, float y, float z, float o, uint32 spawntimedelay)
{
    CreatureTemplate const* cInfo = GetCreatureTemplate(entry);
    if (!cInfo)
        return UI64LIT(0);

    uint32 level = cInfo->minlevel == cInfo->maxlevel ? cInfo->minlevel : urand(cInfo->minlevel, cInfo->maxlevel);
    if (cInfo->ScaleLevelDuration)
        level = cInfo->ScaleLevelDuration;

    CreatureBaseStats const* stats = GetCreatureBaseStats(level, cInfo->unit_class);

    ObjectGuid::LowType guid = GetGenerator<HighGuid::Creature>()->Generate();
    CreatureData& data = NewOrExistCreatureData(guid);
    data.id = entry;
    data.mapid = mapId;
    data.displayid = 0;
    data.equipmentId = 0;
    data.posX = x;
    data.posY = y;
    data.posZ = z;
    data.orientation = o;
    data.spawntimesecs = spawntimedelay;
    data.spawndist = 0;
    data.currentwaypoint = 0;
    data.curhealth = stats->GenerateHealth(cInfo);
    data.curmana = stats->GenerateMana(cInfo);
    data.movementType = cInfo->MovementType;
    data.spawnMask = 1;
    data.phaseMask = PHASEMASK_NORMAL;
    data.dbData = false;
    data.npcflag = cInfo->npcflag;
    data.npcflag2 = cInfo->npcflag2;
    data.unit_flags = cInfo->unit_flags;
    data.unit_flags3 = cInfo->unit_flags3;
    data.dynamicflags = cInfo->dynamicflags;
    // data.MaxVisible = cInfo->MaxVisible;

    AddCreatureToGrid(guid, &data);

    // Spawn if necessary (loaded grids only)
    if (Map* map = sMapMgr->CreateBaseMap(mapId))
    {
        // We use spawn coords to spawn
        if (!map->Instanceable() && !map->IsRemovalGrid(x, y))
        {
            Creature* creature = new Creature;
            if (!creature->LoadCreatureFromDB(guid, map))
            {
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "AddCreature: cannot add creature entry %u to map", entry);
                delete creature;
                return UI64LIT(0);
            }
        }
    }

    return guid;
}

void ObjectMgr::LoadGameobjects()
{
    uint32 oldMSTime = getMSTime();

    uint32 count = 0;

    //                                                0                1   2    3         4           5           6        7           8
    QueryResult result = WorldDatabase.Query("SELECT gameobject.guid, id, map, zoneId, areaId, position_x, position_y, position_z, orientation, "
    //      9          10         11          12         13          14             15      16         17         18        19          20          21      22      23 
        "rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state, isActive, spawnMask, phaseMask, eventEntry, pool_entry, PhaseId, AiID, personal_size "
        "FROM gameobject LEFT OUTER JOIN game_event_gameobject ON gameobject.guid = game_event_gameobject.guid "
        "LEFT OUTER JOIN pool_gameobject ON gameobject.guid = pool_gameobject.guid ORDER BY `map` ASC, `guid` ASC");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 gameobjects. DB table `gameobject` is empty.");

        return;
    }

    // build single time for check spawnmask
    std::map<uint32, uint64> spawnMasks;
    for (auto& mapDifficultyPair : sDB2Manager.GetAllMapsDifficultyes())
        for (auto& difficultyPair : mapDifficultyPair.second)
            spawnMasks[mapDifficultyPair.first] |= (UI64LIT(1) << difficultyPair.first);

    std::map<uint32, GameObjectData*> lastEntryGo;
    _gameObjectDataStore.rehash(result->GetRowCount());
    do
    {
        Field* fields = result->Fetch();

        ObjectGuid::LowType guid = fields[0].GetUInt64();
        uint32 entry        = fields[1].GetUInt32();

        GameObjectTemplate const* gInfo = GetGameObjectTemplate(entry);
        if (!gInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD ") with non existing gameobject entry %u, skipped.", guid, entry);
            continue;
        }

        if (!gInfo->displayId)
        {
            switch (gInfo->type)
            {
                case GAMEOBJECT_TYPE_TRAP:
                case GAMEOBJECT_TYPE_SPELL_FOCUS:
                    break;
                default:
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: " UI64FMTD " Entry %u GoType: %u) doesn't have a displayId (%u), not loaded.", guid, entry, gInfo->type, gInfo->displayId);
                    break;
            }
        }

        if (gInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(gInfo->displayId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: " UI64FMTD " Entry %u GoType: %u) has an invalid displayId (%u), not loaded.", guid, entry, gInfo->type, gInfo->displayId);
            continue;
        }

        GameObjectData& data = _gameObjectDataStore[guid];

        data.id             = entry;
        data.guid           = guid;
        data.mapid          = fields[2].GetUInt16();
        data.zoneId         = fields[3].GetUInt16();
        data.areaId         = fields[4].GetUInt16();
        data.posX           = fields[5].GetFloat();
        data.posY           = fields[6].GetFloat();
        data.posZ           = fields[7].GetFloat();
        data.orientation    = fields[8].GetFloat();
        data.rotation.x     = fields[9].GetFloat();
        data.rotation.y     = fields[10].GetFloat();
        data.rotation.z     = fields[11].GetFloat();
        data.rotation.w     = fields[12].GetFloat();
        data.spawntimesecs  = fields[13].GetInt32();
        data.AiID           = fields[22].GetInt32();
        data.personalSize   = fields[23].GetFloat();

        MapEntry const* mapEntry = sMapStore.LookupEntry(data.mapid);
        if (!mapEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) spawned on a non-existed map (Id: %u), skip", guid, data.id, data.mapid);
            continue;
        }

        if (!data.zoneId || !data.areaId)
        {
            uint32 zoneId = 0;
            uint32 areaId = 0;

            sMapMgr->GetZoneAndAreaId(zoneId, areaId, data.mapid, data.posX, data.posY, data.posZ);

            data.zoneId = zoneId;
            data.areaId = areaId;

            WorldDatabase.PExecute("UPDATE gameobject SET zoneId = %u, areaId = %u WHERE guid = %u", zoneId, areaId, guid);
        }

        if (data.spawntimesecs == 0 && gInfo->IsDespawnAtAction())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) with `spawntimesecs` (0) value, but the gameobejct is marked as despawnable at action.", guid, data.id);
        }

        data.animprogress   = fields[14].GetUInt8();
        data.artKit         = 0;

        uint32 go_state     = fields[15].GetUInt8();
        if (go_state >= MAX_GO_STATE)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) with invalid `state` (%u) value, skip", guid, data.id, go_state);
            continue;
        }
        data.go_state       = GOState(go_state);

        data.isActive       = fields[16].GetBool();

        data.spawnMask      = fields[17].GetUInt64();

        if (!IsTransportMap(data.mapid) && data.spawnMask & ~spawnMasks[data.mapid])
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) that has wrong spawn mask " UI64FMTD " including not supported difficulty modes for map (Id: %u), skip", guid, data.id, data.spawnMask, data.mapid);
            data.spawnMask = spawnMasks[data.mapid];
            WorldDatabase.PExecute("UPDATE gameobject SET spawnMask = " UI64FMTD " WHERE guid = %u", spawnMasks[data.mapid], guid);
        }

        data.phaseMask      = fields[18].GetUInt16();
        int16 gameEvent     = fields[19].GetInt16();
        uint32 PoolId        = fields[20].GetUInt32();

        data.gameEvent = gameEvent;
        data.pool = PoolId;
        data.MaxVisible = gInfo->MaxVisible;

        if (data.rotation.x < -1.0f || data.rotation.x > 1.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) with invalid rotation2 (%f) value, skip", guid, data.id, data.rotation.x);
            continue;
        }

        if (data.rotation.y < -1.0f || data.rotation.y > 1.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) with invalid rotation3 (%f) value, skip", guid, data.id, data.rotation.y);
            continue;
        }

        if (data.rotation.z < -1.0f || data.rotation.z > 1.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) with invalid rotationZ (%f) value, skip", guid, data.id, data.rotation.z);
            continue;
        }

        if (data.rotation.w < -1.0f || data.rotation.w > 1.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) with invalid rotationW (%f) value, skip", guid, data.id, data.rotation.w);
            continue;
        }

        Tokenizer phasesToken(fields[21].GetString(), ' ', 100);
        for (auto itr : phasesToken)
            if (PhaseEntry const* phase = sPhaseStore.LookupEntry(uint32(strtoull(itr, nullptr, 10))))
                data.PhaseID.insert(phase->ID);

        if (data.phaseMask == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` has gameobject (GUID: " UI64FMTD " Entry: %u) with `phaseMask`=0 (not visible for anyone), set to 1.", guid, data.id);
            data.phaseMask = 1;
        }

        if (gInfo->type == GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: " UI64FMTD " Entry %u GoType: %u) GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT not spawn in world", guid, entry, gInfo->type);
            continue;
        }

        if (gInfo->type == GAMEOBJECT_TYPE_GARRISON_SHIPMENT)
        {
            CharShipmentContainerEntry const* shipmentConteinerEntry = sCharShipmentContainerStore.LookupEntry(gInfo->garrisonShipment.ShipmentContainer);
            if (!shipmentConteinerEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: " UI64FMTD " Entry %u GoType: %u) GAMEOBJECT_TYPE_GARRISON_SHIPMENT no shipment conteiner ID", guid, entry, gInfo->type);
                continue;
            }

            if (shipmentConteinerEntry->GarrTypeID != GARRISON_TYPE_CLASS_ORDER)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: " UI64FMTD " Entry %u GoType: %u) GAMEOBJECT_TYPE_GARRISON_SHIPMENT has GarrTypeID != GARRISON_TYPE_CLASS_ORDER", guid, entry, gInfo->type);
                continue;
            }
        }

        // check near go with same entry.
        auto lastGo = lastEntryGo.find(entry);
        if (lastGo != lastEntryGo.end())
        {
            if (data.mapid == lastGo->second->mapid)
            {
                float dx1 = lastGo->second->posX - data.posX;
                float dy1 = lastGo->second->posY - data.posY;
                float dz1 = lastGo->second->posZ - data.posZ;

                float distsq1 = dx1*dx1 + dy1*dy1 + dz1*dz1;
                if (distsq1 < 0.5f)
                {
                    // split phaseID
                    for (auto phaseID : data.PhaseID)
                        lastGo->second->PhaseID.insert(phaseID);

                    lastGo->second->phaseMask |= data.phaseMask;
                    lastGo->second->spawnMask |= data.spawnMask;
                    WorldDatabase.PExecute("UPDATE gameobject SET phaseMask = %u, spawnMask = " UI64FMTD " WHERE guid = %u", lastGo->second->phaseMask, lastGo->second->spawnMask, lastGo->second->guid);
                    WorldDatabase.PExecute("DELETE FROM gameobject WHERE guid = %u", guid);
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject` have clone go %u witch stay too close (dist: %f). original go guid %u. go with guid %u will be deleted.", entry, distsq1, lastGo->second->guid, guid);
                    continue;
                }
            }
            else
                lastEntryGo[entry] = &data;

        }
        else
            lastEntryGo[entry] = &data;

        if (gameEvent == 0 && PoolId == 0)                      // if not this is to be managed by GameEvent System or Pool system
            AddGameobjectToGrid(guid, &data);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %lu gameobjects in %u ms", static_cast<unsigned long>(_gameObjectDataStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::AddGameobjectToGrid(ObjectGuid::LowType const& guid, GameObjectData const* data)
{
    uint64 mask = data->spawnMask;
    for (uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if (mask & 1)
        {
            if (data->mapid >= _mapObjectGuidsStore.size())
                _mapObjectGuidsStore.resize(data->mapid + 1);

            if (i >= _mapObjectGuidsStore[data->mapid].size())
                _mapObjectGuidsStore[data->mapid].resize(i + 1);

            CellCoord cellCoord = Trinity::ComputeCellCoord(data->posX, data->posY);
            CellObjectGuids& cell_guids = _mapObjectGuidsStore[data->mapid][i][cellCoord.GetId()];
            if (sObjectMgr->IsStaticTransport(data->id))
                cell_guids.statictransports.insert(guid);
            else
                cell_guids.gameobjects.insert(guid);
        }
    }
}

void ObjectMgr::RemoveGameobjectFromGrid(ObjectGuid::LowType const& guid, GameObjectData const* data)
{
    uint64 mask = data->spawnMask;
    for (uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if (mask & 1)
        {
            if (data->mapid >= _mapObjectGuidsStore.size())
                _mapObjectGuidsStore.resize(data->mapid + 1);

            if (i >= _mapObjectGuidsStore[data->mapid].size())
                _mapObjectGuidsStore[data->mapid].resize(i + 1);

            CellCoord cellCoord = Trinity::ComputeCellCoord(data->posX, data->posY);
            CellObjectGuids& cell_guids = _mapObjectGuidsStore[data->mapid][i][cellCoord.GetId()];
            if (sObjectMgr->IsStaticTransport(data->id))
                cell_guids.statictransports.erase(guid);
            else
                cell_guids.gameobjects.erase(guid);
        }
    }
}

Player* ObjectMgr::GetPlayerByLowGUID(ObjectGuid::LowType const& lowguid) const
{
    return ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(lowguid));
}

// name must be checked to correctness (if received) before call this function
ObjectGuid ObjectMgr::GetPlayerGUIDByName(std::string name)
{
    if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(sObjectMgr->GetRealCharName(name)))
        return ObjectGuid::Create<HighGuid::Player>(nameData->Guid);

    return ObjectGuid::Empty;
}

bool ObjectMgr::GetPlayerNameByGUID(ObjectGuid const& guid, std::string& name)
{
    // prevent DB access for online player
    if (Player* player = ObjectAccessor::FindPlayer(guid))
    {
        name = player->GetName();
        return true;
    }

    if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(guid))
    {
        name = nameData->Name;
        return true;
    }

    return false;
}

uint32 ObjectMgr::GetPlayerTeamByGUID(ObjectGuid const& guid) const
{
    // prevent DB access for online player
    if (Player* player = ObjectAccessor::FindPlayer(guid))
        return Player::TeamForRace(player->getRace());

    if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(guid))
        return Player::TeamForRace(nameData->Race);

    return 0;
}

uint32 ObjectMgr::GetPlayerAccountIdByGUID(ObjectGuid const& guid)
{
    // prevent DB access for online player
    if (Player* player = ObjectAccessor::FindPlayer(guid))
        return player->GetSession()->GetAccountId();

    if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(guid))
        return nameData->AccountId;

    return 0;
}

uint32 ObjectMgr::GetPlayerAccountIdByPlayerName(std::string const& name)
{
    if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(sObjectMgr->GetRealCharName(name)))
        return nameData->AccountId;

    return 0;
}

uint32 FillMaxDurability(uint32 itemClass, uint32 itemSubClass, uint32 inventoryType, uint32 quality, uint32 itemLevel)
{
    if (itemClass != ITEM_CLASS_ARMOR && itemClass != ITEM_CLASS_WEAPON)
        return 0;

    static float const qualityMultipliers[MAX_ITEM_QUALITY] =
    {
        1.0f, 1.0f, 1.0f, 1.17f, 1.37f, 1.68f, 0.0f, 0.0f
    };

    static float const armorMultipliers[MAX_INVTYPE] =
    {
        0.00f, // INVTYPE_NON_EQUIP
        0.59f, // INVTYPE_HEAD
        0.00f, // INVTYPE_NECK
        0.59f, // INVTYPE_SHOULDERS
        0.00f, // INVTYPE_BODY
        1.00f, // INVTYPE_CHEST
        0.35f, // INVTYPE_WAIST
        0.75f, // INVTYPE_LEGS
        0.49f, // INVTYPE_FEET
        0.35f, // INVTYPE_WRISTS
        0.35f, // INVTYPE_HANDS
        0.00f, // INVTYPE_FINGER
        0.00f, // INVTYPE_TRINKET
        0.00f, // INVTYPE_WEAPON
        1.00f, // INVTYPE_SHIELD
        0.00f, // INVTYPE_RANGED
        0.00f, // INVTYPE_CLOAK
        0.00f, // INVTYPE_2HWEAPON
        0.00f, // INVTYPE_BAG
        0.00f, // INVTYPE_TABARD
        1.00f, // INVTYPE_ROBE
        0.00f, // INVTYPE_WEAPONMAINHAND
        0.00f, // INVTYPE_WEAPONOFFHAND
        0.00f, // INVTYPE_HOLDABLE
        0.00f, // INVTYPE_AMMO
        0.00f, // INVTYPE_THROWN
        0.00f, // INVTYPE_RANGEDRIGHT
        0.00f,
        0.00f, // INVTYPE_RELIC
    };

    static float const weaponMultipliers[MAX_ITEM_SUBCLASS_WEAPON] =
    {
        0.89f, // ITEM_SUBCLASS_WEAPON_AXE
        1.03f, // ITEM_SUBCLASS_WEAPON_AXE2
        0.77f, // ITEM_SUBCLASS_WEAPON_BOW
        0.77f, // ITEM_SUBCLASS_WEAPON_GUN
        0.89f, // ITEM_SUBCLASS_WEAPON_MACE
        1.03f, // ITEM_SUBCLASS_WEAPON_MACE2
        1.03f, // ITEM_SUBCLASS_WEAPON_POLEARM
        0.89f, // ITEM_SUBCLASS_WEAPON_SWORD
        1.03f, // ITEM_SUBCLASS_WEAPON_SWORD2
        1.00f, // ITEM_SUBCLASS_WEAPON_WARGLAIVES
        1.03f, // ITEM_SUBCLASS_WEAPON_STAFF
        0.00f, // ITEM_SUBCLASS_WEAPON_EXOTIC
        0.00f, // ITEM_SUBCLASS_WEAPON_EXOTIC2
        0.64f, // ITEM_SUBCLASS_WEAPON_FIST_WEAPON
        0.00f, // ITEM_SUBCLASS_WEAPON_MISCELLANEOUS
        0.64f, // ITEM_SUBCLASS_WEAPON_DAGGER
        0.64f, // ITEM_SUBCLASS_WEAPON_THROWN
        0.00f, // ITEM_SUBCLASS_WEAPON_SPEAR
        0.77f, // ITEM_SUBCLASS_WEAPON_CROSSBOW
        0.64f, // ITEM_SUBCLASS_WEAPON_WAND
        0.64f, // ITEM_SUBCLASS_WEAPON_FISHING_POLE
    };

    float levelPenalty = 1.0f;
    if (itemLevel <= 28)
        levelPenalty = 0.966f - float(28u - itemLevel) / 54.0f;

    if (itemClass == ITEM_CLASS_ARMOR)
    {
        if (inventoryType > INVTYPE_ROBE)
            return 0;

        return 5 * uint32(23.0f * qualityMultipliers[quality] * armorMultipliers[inventoryType] * levelPenalty + 0.5f);
    }

    return 5 * uint32(17.0f * qualityMultipliers[quality] * weaponMultipliers[itemSubClass] * levelPenalty + 0.5f);
};

void ObjectMgr::LoadItemTemplates()
{
    uint32 oldMSTime = getMSTime();
    uint32 sparseCount = 0;

    for (ItemSparseEntry const* sparse : sItemSparseStore)
    {
        ItemEntry const* db2Data = sItemStore.LookupEntry(sparse->ID);
        if (!db2Data)
            continue;

        ItemTemplate& itemTemplate = _itemTemplateStore[sparse->ID];

        itemTemplate.BasicData = db2Data;
        itemTemplate.ExtendedData = sparse;
        itemTemplate.VendorStackCount = sparse->VendorStackCount > 0 ? sparse->VendorStackCount : 1;
        itemTemplate.AllowableClass = sparse->AllowableClass ? sparse->AllowableClass : -1;
        itemTemplate.AllowableRace = sparse->AllowableRace ? sparse->AllowableRace : -1;
        itemTemplate.ItemLevel = sparse->ItemLevel < 1 ? 1 : sparse->ItemLevel;
        itemTemplate.MaxDurability = FillMaxDurability(db2Data->ClassID, db2Data->SubclassID, sparse->InventoryType, sparse->OverallQualityID, sparse->ItemLevel);
        itemTemplate.ScriptId = 0;
        itemTemplate.FoodType = 0;
        itemTemplate.MinMoneyLoot = 0;
        itemTemplate.MaxMoneyLoot = 0;
        itemTemplate.FlagsCu = 0;
        itemTemplate.ItemSpecClassMask = 0;
        itemTemplate.SpellPPMRate = 0.0f;
        ++sparseCount;

        itemTemplate.IsMultiClass = false;
        itemTemplate.IsSingleClass = false;
        if (sparse->AllowableClass && sparse->AllowableClass != -1)
        {
            uint8 conterClass = 0;
            for (uint8 i = CLASS_WARRIOR; i < MAX_CLASSES; ++i)
            {
                if (sparse->AllowableClass & (1 << (i - 1)))
                    conterClass++;
            }
            if (conterClass == 1)
                itemTemplate.IsSingleClass = true;
            if (conterClass > 1)
                itemTemplate.IsMultiClass = true;
        }

        if (std::vector<ItemSpecOverrideEntry const*> const* itemSpecOverrides = sDB2Manager.GetItemSpecOverrides(sparse->ID))
        {
            itemTemplate.ItemSpecExist = true;
            for (ItemSpecOverrideEntry const* itemSpecOverride : *itemSpecOverrides)
            {
                if (ChrSpecializationEntry const* specialization = sChrSpecializationStore.LookupEntry(itemSpecOverride->SpecID))
                {
                    itemTemplate.ItemSpecClassMask |= 1 << (specialization->ClassID - 1);
                    itemTemplate.Specializations[0].set(ItemTemplate::CalculateItemSpecBit(specialization));
                    itemTemplate.Specializations[1] |= itemTemplate.Specializations[0];
                    itemTemplate.Specializations[2] |= itemTemplate.Specializations[0];
                }
            }
        }
        else
        {
            ItemSpecStats itemSpecStats(db2Data, sparse);

            for (ItemSpecEntry const* itemSpec : sItemSpecStore)
            {
                if (itemSpecStats.ItemType != itemSpec->ItemType)
                    continue;

                bool hasPrimary = itemSpec->PrimaryStat == ITEM_SPEC_STAT_NONE;
                bool hasSecondary = itemSpec->SecondaryStat == ITEM_SPEC_STAT_NONE;
                for (uint32 i = 0; i < itemSpecStats.ItemSpecStatCount; ++i)
                {
                    if (itemSpecStats.ItemSpecStatTypes[i] == itemSpec->PrimaryStat)
                        hasPrimary = true;
                    if (itemSpecStats.ItemSpecStatTypes[i] == itemSpec->SecondaryStat)
                        hasSecondary = true;
                }

                if (!hasPrimary || !hasSecondary)
                    continue;

                if (ChrSpecializationEntry const* specialization = sChrSpecializationStore.LookupEntry(itemSpec->SpecializationID))
                {
                    if ((1 << (specialization->ClassID - 1)) & sparse->AllowableClass)
                    {
                        itemTemplate.ItemSpecClassMask |= 1 << (specialization->ClassID - 1);
                        std::size_t specBit = ItemTemplate::CalculateItemSpecBit(specialization);
                        itemTemplate.Specializations[0].set(specBit);
                        if (itemSpec->MaxLevel > 40)
                            itemTemplate.Specializations[1].set(specBit);
                        if (itemSpec->MaxLevel >= 110)
                            itemTemplate.Specializations[2].set(specBit);
                        itemTemplate.ItemSpecExist = true;
                    }
                }
            }
        }

        // Items that have no specializations set can be used by everyone
        for (auto& specs : itemTemplate.Specializations)
            if (specs.count() == 0)
                specs.set();

        // Mantid Amber Sliver
        if (itemTemplate.GetId() == 95373)
            if (CurrencyTypesEntry const* curr = sCurrencyTypesStore.LookupEntry(CURRENCY_TYPE_MANTID_ARCHAEOLOGY_FRAGMENT))
                const_cast<ItemSparseEntry*>(itemTemplate.ExtendedData)->SpellWeightCategory = curr->SpellCategory;

        if (itemTemplate.IsLegendaryLoot()) // Collected Legendary Item
            LegendaryItemList.push_back(&itemTemplate);

        if (itemTemplate.IsLegionLegendaryToken()) // Collected Legendary Token
            LegendaryTokenList.push_back(&itemTemplate);
    }

    for (ItemEffectEntry const* effectEntry : sItemEffectStore)
    {
        auto itemItr = _itemTemplateStore.find(effectEntry->ItemID);
        if (itemItr == _itemTemplateStore.end())
            continue;

        itemItr->second.Effects.push_back(effectEntry);
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u item templates from ItemSparse.db2 in %u ms", sparseCount, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadItemTemplateAddon()
{
    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT Id, FlagsCu, FoodType, MinMoneyLoot, MaxMoneyLoot, SpellPPMChance FROM item_template_addon");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 itemId = fields[0].GetUInt32();
            if (!GetItemTemplate(itemId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Item %u specified in `item_template_addon` does not exist, skipped.", itemId);
                continue;
            }

            uint32 minMoneyLoot = fields[3].GetUInt32();
            uint32 maxMoneyLoot = fields[4].GetUInt32();
            if (minMoneyLoot > maxMoneyLoot)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Minimum money loot specified in `item_template_addon` for item %u was greater than maximum amount, swapping.", itemId);
                std::swap(minMoneyLoot, maxMoneyLoot);
            }
            ItemTemplate& itemTemplate = _itemTemplateStore[itemId];
            itemTemplate.FlagsCu = fields[1].GetUInt32();
            itemTemplate.FoodType = fields[2].GetUInt8();
            itemTemplate.MinMoneyLoot = minMoneyLoot;
            itemTemplate.MaxMoneyLoot = maxMoneyLoot;
            itemTemplate.SpellPPMRate = fields[5].GetFloat();
            ++count;
        } while (result->NextRow());
    }
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u item addon templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadItemScriptNames()
{
    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT Id, ScriptName FROM item_script_names");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 itemId = fields[0].GetUInt32();
            if (!GetItemTemplate(itemId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Item %u specified in `item_script_names` does not exist, skipped.", itemId);
                continue;
            }

            _itemTemplateStore[itemId].ScriptId = GetScriptId(fields[1].GetCString());
            ++count;
        } while (result->NextRow());
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u item script names in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
ItemTemplate const* ObjectMgr::GetItemTemplate(uint32 entry)
{
    return Trinity::Containers::MapGetValuePtr(_itemTemplateStore, entry);
}

void ObjectMgr::LoadVehicleTemplateAccessories()
{
    uint32 oldMSTime = getMSTime();

    _vehicleTemplateAccessoryStore.clear();                           // needed for reload case
    _vehicleAttachmentOffsetStore.clear();

    uint32 count = 0;

    //                                                     0              1              2          3           4             5              6          7          8          9
    QueryResult result = WorldDatabase.Query("SELECT `EntryOrAura`, `accessory_entry`, `seat_id`, `minion`, `summontype`, `summontimer`, `offsetX`, `offsetY`, `offsetZ`, `offsetO` FROM `vehicle_template_accessory`");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 vehicle template accessories. DB table `vehicle_template_accessory` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        int32  uiEntryOrAura= fields[0].GetInt32();
        uint32 uiAccessory  = fields[1].GetUInt32();
        int8   uiSeat       = int8(fields[2].GetInt8());
        bool   bMinion      = fields[3].GetBool();
        uint8  uiSummonType = fields[4].GetUInt8();
        uint32 uiSummonTimer= fields[5].GetUInt32();
        float offsetX       = fields[6].GetFloat();
        float offsetY       = fields[7].GetFloat();
        float offsetZ       = fields[8].GetFloat();
        float offsetO       = fields[9].GetFloat();

        if (uiEntryOrAura > 0 && !sObjectMgr->GetCreatureTemplate(uiEntryOrAura))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `vehicle_template_accessory`: creature template entry %u does not exist.", uiEntryOrAura);
            continue;
        }

        if (uiEntryOrAura < 0 && !sSpellMgr->GetSpellInfo(uiEntryOrAura * -1))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `vehicle_template_accessory`: aura %u does not exist.", uiEntryOrAura);
            continue;
        }

        if (!sObjectMgr->GetCreatureTemplate(uiAccessory))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `vehicle_template_accessory`: Accessory %u does not exist.", uiAccessory);
            continue;
        }

        if (uiEntryOrAura > 0 && _spellClickInfoStore.find(uiEntryOrAura) == _spellClickInfoStore.end())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `vehicle_template_accessory`: creature template entry %u has no data in npc_spellclick_spells", uiEntryOrAura);
            continue;
        }

        _vehicleTemplateAccessoryStore[uiEntryOrAura].push_back(VehicleAccessory(uiAccessory, uiSeat, bMinion, uiSummonType, uiSummonTimer, Position{offsetX, offsetY, offsetZ, offsetO}));

        ++count;
    }
    while (result->NextRow());

    result = WorldDatabase.Query("SELECT `entry`, `seat_id`, `offsetX`, `offsetY`, `offsetZ`, `offsetO` FROM `vehicle_attachment_offset`");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 vehicle attachment offset. DB table `vehicle_attachment_offset` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint8 i = 0;
        int32 entry = fields[i++].GetInt32();
        int8 seatId = int8(fields[i++].GetInt8());
        float offsetX = fields[i++].GetFloat();
        float offsetY = fields[i++].GetFloat();
        float offsetZ = fields[i++].GetFloat();
        float offsetO = fields[i++].GetFloat();

        if (entry > 0 && !sObjectMgr->GetCreatureTemplate(entry))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `vehicle_attachment_offset`: creature template entry %u does not exist.", entry);
            continue;
        }

        _vehicleAttachmentOffsetStore[entry].push_back(VehicleAttachmentOffset(entry, seatId, Position{offsetX, offsetY, offsetZ, offsetO}));
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Vehicle Template Accessories in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadPetStats()
{
    uint32 oldMSTime = getMSTime();
    _petStatsStore.clear();

    //                                                 0      1     2       3        4          5           6        7            8           9       10      11           12         13
    QueryResult result = WorldDatabase.Query("SELECT entry, `hp`, `ap`, `ap_type`, `spd`, `school_mask`, `state`, `energy`, `energy_type`, `armor`, `type`, `damage`, `maxspdorap`, `haste` FROM pet_stats");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 pet stats. DB table `pet_stats` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        if (!sObjectMgr->GetCreatureTemplate(entry))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong creature id %u in `pet_stats` table, ignoring.", entry);
            continue;
        }

        PetStats stats;
        stats.hp = fields[1].GetFloat();
        stats.ap   = fields[2].GetFloat();
        stats.ap_type  = fields[3].GetInt32();
        stats.spd  = fields[4].GetFloat();
        stats.school_mask  = fields[5].GetInt32();
        stats.state  = fields[6].GetInt32();
        stats.energy  = fields[7].GetInt32();
        stats.energy_type  = fields[8].GetInt32();
        stats.armor  = fields[9].GetFloat();
        stats.type  = fields[10].GetInt32();
        stats.damage  = fields[11].GetFloat();
        stats.maxspdorap  = fields[12].GetInt32();
        stats.haste  = fields[13].GetInt32();
        _petStatsStore[entry] = stats;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u pet stats definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

PetStats const* ObjectMgr::GetPetStats(uint32 creature_id) const
{
    return Trinity::Containers::MapGetValuePtr(_petStatsStore, creature_id);
}

void ObjectMgr::PlayerCreateInfoAddItemHelper(uint32 race_, uint32 class_, uint32 itemId, int32 count, std::vector<uint32> bonusListIDs)
{
    if (race_ >= MAX_RACES ||  class_>= MAX_CLASSES)
        return;

    if (count > 0)
    {
        if (PlayerInfo* info = _playerInfo[race_][class_])
            info->item.push_back(PlayerCreateInfoItem(itemId, count, bonusListIDs));
    }
    else
    {
        if (count < -1)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Invalid count %i specified on item %u be removed from original player create info (use -1)!", count, itemId);

        bool doneOne = false;
        for (CharStartOutfitEntry const* entry : sCharStartOutfitStore)
        {
            if (entry->ClassID == class_ && entry->RaceID == race_)
            {
                bool found = false;
                for (uint8 x = 0; x < MAX_OUTFIT_ITEMS; ++x)
                {
                    if (entry->ItemID[x] > 0 && uint32(entry->ItemID[x]) == itemId)
                    {
                        found = true;
                        const_cast<CharStartOutfitEntry*>(entry)->ItemID[x] = 0;
                        break;
                    }
                }

                if (!found)
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Item %u specified to be removed from original create info not found in dbc!", itemId);

                if (!doneOne)
                    doneOne = true;
                else
                    break;
            }
        }
    }
}

void ObjectMgr::PlayerCreateInfoAddQuestHelper(uint32 race_, uint32 class_, uint32 questId)
{
    if (race_ >= MAX_RACES || class_>= MAX_CLASSES)
        return;
    if (PlayerInfo* info = _playerInfo[race_][class_])
        info->quests.push_back(questId);
}

void ObjectMgr::PlayerCreateInfoAddSpellHelper(uint32 race_, uint32 class_, uint32 spellId)
{
    if (race_ >= MAX_RACES || class_>= MAX_CLASSES)
        return;
    if (PlayerInfo* info = _playerInfo[race_][class_])
        info->spell.push_back(spellId);
}

void ObjectMgr::PlayerCreateInfoAddActionHelper(uint32 race_, uint32 class_, PlayerCreateInfoAction action)
{
    if (race_ >= MAX_RACES || class_>= MAX_CLASSES)
        return;
    if (PlayerInfo* info = _playerInfo[race_][class_])
        info->action.push_back(action);
}

void ObjectMgr::LoadPlayerInfo()
{
    uint32 oldMSTime = getMSTime();
    {
        // Load playercreate
        //                                                0     1      2    3        4          5           6
        QueryResult result = WorldDatabase.Query("SELECT race, class, map, zone, position_x, position_y, position_z, orientation FROM playercreateinfo");

        if (!result)
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 player create definitions. DB table `playercreateinfo` is empty.");
            exit(1);
        }
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();

            uint32 current_race  = fields[0].GetUInt8();
            uint32 current_class = fields[1].GetUInt8();
            uint32 mapId         = fields[2].GetUInt16();
            uint32 areaId        = fields[3].GetUInt32(); // zone
            float  positionX     = fields[4].GetFloat();
            float  positionY     = fields[5].GetFloat();
            float  positionZ     = fields[6].GetFloat();
            float  orientation   = fields[7].GetFloat();

            if (current_race >= MAX_RACES)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong race %u in `playercreateinfo` table, ignoring.", current_race);
                continue;
            }

            ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(current_race);
            if (!rEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong race %u in `playercreateinfo` table, ignoring.", current_race);
                continue;
            }

            if (current_class >= MAX_CLASSES)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong class %u in `playercreateinfo` table, ignoring.", current_class);
                continue;
            }

            if (!sChrClassesStore.LookupEntry(current_class))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong class %u in `playercreateinfo` table, ignoring.", current_class);
                continue;
            }

            // accept DB data only for valid position (and non instanceable)
            if (!MapManager::IsValidMapCoord(mapId, positionX, positionY, positionZ, orientation))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong home position for class %u race %u pair in `playercreateinfo` table, ignoring.", current_class, current_race);
                continue;
            }

            if (sMapStore.LookupEntry(mapId)->Instanceable())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Home position in instanceable map for class %u race %u pair in `playercreateinfo` table, ignoring.", current_class, current_race);
                continue;
            }

            PlayerInfo* info = new PlayerInfo();
            info->mapId = mapId;
            info->areaId = areaId;
            info->positionX = positionX;
            info->positionY = positionY;
            info->positionZ = positionZ;
            info->orientation = orientation;
            info->displayId_m = rEntry->MaleDisplayID;
            info->displayId_f = rEntry->FemaleDisplayID;
            _playerInfo[current_race][current_class] = info;

            ++count;
        }
        while (result->NextRow());

        _playerInfo[RACE_NONE][CLASS_NONE] = new PlayerInfo();
        _playerInfo[RACE_NONE][CLASS_NONE]->displayId_m = sChrRacesStore.LookupEntry(RACE_BLOODELF)->MaleDisplayID;
        _playerInfo[RACE_NONE][CLASS_NONE]->displayId_f = sChrRacesStore.LookupEntry(RACE_BLOODELF)->FemaleDisplayID;

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u player create definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    // Load playercreate items
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create Items Data...");
    {
        oldMSTime = getMSTime();
        //                                                0     1      2       3         4
        QueryResult result = WorldDatabase.Query("SELECT race, class, itemid, amount, BonusListID FROM playercreateinfo_item");
        if (!result)
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 custom player create items. DB table `playercreateinfo_item` is empty.");
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt8();
                if (current_race >= MAX_RACES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong race %u in `playercreateinfo_item` table, ignoring.", current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt8();
                if (current_class >= MAX_CLASSES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong class %u in `playercreateinfo_item` table, ignoring.", current_class);
                    continue;
                }

                uint32 item_id = fields[2].GetUInt32();
                if (!GetItemTemplate(item_id))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Item id %u (race %u class %u) in `playercreateinfo_item` table but not listed in `item_template`, ignoring.", item_id, current_race, current_class);
                    continue;
                }

                int32 amount   = fields[3].GetUInt32();
                if (!amount)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Item id %u (class %u race %u) have amount == 0 in `playercreateinfo_item` table, ignoring.", item_id, current_race, current_class);
                    continue;
                }

                Tokenizer bonusListIDsTok(fields[4].GetString(), ' ');
                std::vector<uint32> bonusListIDs;
                for (char const* token : bonusListIDsTok)
                    bonusListIDs.push_back(atol(token));

                if (!current_race && !current_class)
                    PlayerCreateInfoAddItemHelper(RACE_NONE, CLASS_NONE, item_id, amount, bonusListIDs);
                else if (!current_race || !current_class)
                {
                    uint32 min_race = current_race ? current_race : 1;
                    uint32 max_race = current_race ? current_race + 1 : MAX_RACES;
                    uint32 min_class = current_class ? current_class : 1;
                    uint32 max_class = current_class ? current_class + 1 : MAX_CLASSES;
                    for (uint32 r = min_race; r < max_race; ++r)
                        for (uint32 c = min_class; c < max_class; ++c)
                            PlayerCreateInfoAddItemHelper(r, c, item_id, amount, bonusListIDs);
                }
                else
                    PlayerCreateInfoAddItemHelper(current_race, current_class, item_id, amount, bonusListIDs);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u custom player create items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create Skill Data...");
    {
        oldMSTime = getMSTime();

        for (SkillRaceClassInfoEntry const* rcInfo : sSkillRaceClassInfoStore)
            if (rcInfo->Availability == 1)
                for (uint32 raceIndex = RACE_HUMAN; raceIndex < MAX_RACES; ++raceIndex)
                    if (rcInfo->RaceMask == -1ll || ((UI64LIT(1) << (raceIndex - 1)) & rcInfo->RaceMask))
                        for (uint32 classIndex = CLASS_WARRIOR; classIndex < MAX_CLASSES; ++classIndex)
                            if (rcInfo->ClassMask == -1 || ((1 << (classIndex - 1)) & rcInfo->ClassMask))
                                if (PlayerInfo* info = _playerInfo[raceIndex][classIndex])
                                    info->skills.push_back(rcInfo);

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded player create skills in %u s", GetMSTimeDiffToNow(oldMSTime) / IN_MILLISECONDS);
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create Spell Data...");
    {
        oldMSTime = getMSTime();
        //                                                0     1      2    
        QueryResult result = WorldDatabase.Query("SELECT race, class, spell FROM playercreateinfo_spell");
        if (result)
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt8();
                if (current_race >= MAX_RACES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong race %u in `playercreateinfo_action` table, ignoring.", current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt8();
                if (current_class >= MAX_CLASSES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong class %u in `playercreateinfo_action` table, ignoring.", current_class);
                    continue;
                }

                uint32 spellID = fields[2].GetUInt32();
                if (!sSpellMgr->GetSpellInfo(spellID))
                    continue;

                if (!current_race && !current_class)
                    PlayerCreateInfoAddSpellHelper(RACE_NONE, CLASS_NONE, spellID);
                else if (!current_race || !current_class)
                {
                    uint32 min_race = current_race ? current_race : 1;
                    uint32 max_race = current_race ? current_race + 1 : MAX_RACES;
                    uint32 min_class = current_class ? current_class : 1;
                    uint32 max_class = current_class ? current_class + 1 : MAX_CLASSES;
                    for (uint32 r = min_race; r < max_race; ++r)
                        for (uint32 c = min_class; c < max_class; ++c)
                            PlayerCreateInfoAddSpellHelper(r, c, spellID);
                }
                else
                    PlayerCreateInfoAddSpellHelper(current_race, current_class, spellID);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u player create spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }
    
    // Load playercreate actions
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create Action Data...");
    {
        oldMSTime = getMSTime();
        //                                                0     1      2       3       4
        QueryResult result = WorldDatabase.Query("SELECT race, class, button, action, type FROM playercreateinfo_action");
        if (result)
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt8();
                if (current_race >= MAX_RACES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong race %u in `playercreateinfo_action` table, ignoring.", current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt8();
                if (current_class >= MAX_CLASSES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong class %u in `playercreateinfo_action` table, ignoring.", current_class);
                    continue;
                }

                PlayerCreateInfoAction action(fields[2].GetUInt16(), fields[3].GetUInt32(), fields[4].GetUInt16());

                if (!current_race && !current_class)
                    PlayerCreateInfoAddActionHelper(RACE_NONE, CLASS_NONE, action);
                else if (!current_race || !current_class)
                {
                    uint32 min_race = current_race ? current_race : 1;
                    uint32 max_race = current_race ? current_race + 1 : MAX_RACES;
                    uint32 min_class = current_class ? current_class : 1;
                    uint32 max_class = current_class ? current_class + 1 : MAX_CLASSES;
                    for (uint32 r = min_race; r < max_race; ++r)
                        for (uint32 c = min_class; c < max_class; ++c)
                            PlayerCreateInfoAddActionHelper(r, c, action);
                }
                else
                    PlayerCreateInfoAddActionHelper(current_race, current_class, action);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u player create actions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // Loading levels data (class/race dependent)
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create Level Stats Data...");
    {
        oldMSTime = getMSTime();
        //                                                 0     1      2      3    4    5    6    7
        QueryResult result = WorldDatabase.Query("SELECT race, class, level, str, agi, sta, inte, spi FROM player_levelstats");
        if (!result)
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 level stats definitions. DB table `player_levelstats` is empty.");
            exit(1);
        }

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt8();
            if (current_race >= MAX_RACES)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong race %u in `player_levelstats` table, ignoring.", current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt8();
            if (current_class >= MAX_CLASSES)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong class %u in `player_levelstats` table, ignoring.", current_class);
                continue;
            }

            uint32 current_level = fields[2].GetUInt8();
            if (current_level > MAX_LEVEL)
            {
                if (current_level > STRONG_MAX_LEVEL)        // hardcoded level maximum
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong (> %u) level %u in `player_levelstats` table, ignoring.", STRONG_MAX_LEVEL, current_level);
                else
                {
                    TC_LOG_INFO(LOG_FILTER_GENERAL, "Unused (> MaxPlayerLevel in worldserver.conf) level %u in `player_levelstats` table, ignoring.", current_level);
                    ++count;                                // make result loading percent "expected" correct in case disabled detail mode for example.
                }
                continue;
            }

            if (PlayerInfo* info = _playerInfo[current_race][current_class])
            {
                if (!info->levelInfo)
                    info->levelInfo = new PlayerLevelInfo[MAX_LEVEL];

                PlayerLevelInfo& levelInfo = info->levelInfo[current_level - 1];
                for (uint8 i = 0; i < MAX_STATS; i++)
                    levelInfo.stats[i] = fields[i + 3].GetUInt16();
            }

            ++count;
        }
        while (result->NextRow());

        // Fill gaps and check integrity
        for (uint8 race = 0; race < MAX_RACES; ++race)
        {
            // skip non existed races
            if (!sChrRacesStore.LookupEntry(race))
                continue;

            for (uint8 class_ = 0; class_ < MAX_CLASSES; ++class_)
            {
                if (!sChrClassesStore.LookupEntry(class_))
                    continue;

                PlayerInfo* info = _playerInfo[race][class_];
                if (!info)
                    continue;
                uint8 level = 1;
                switch (class_)
                {
                    case CLASS_DEATH_KNIGHT:
                        level = START_DK_LEVEL;
                        break;
                    case CLASS_DEMON_HUNTER:
                        level = START_DH_LEVEL;
                        break;
                    default:
                        break;
                }

                if (!info->levelInfo || info->levelInfo[level - 1].stats[0] == 0)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Race %i Class %i does not have stats data for base lvl (%i)!", race, class_, level - 1);
                    exit(1);
                }

                for (; level < MAX_LEVEL; ++level)
                {
                    if (info->levelInfo[level].stats[0] == 0)
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Race %i Class %i Level %i does not have stats data. Using stats data of level %i.", race, class_, level + 1, level);
                        info->levelInfo[level] = info->levelInfo[level - 1];
                    }
                }
            }
        }

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u level stats definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    // Loading xp per level data
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create XP Data...");
    {
        oldMSTime = getMSTime();

        _playerXPperLevel.resize(sXpGameTable.GetTableRowCount(), 0);
        for (uint32 level = 1; level < sXpGameTable.GetTableRowCount(); ++level)
            _playerXPperLevel[level] = sXpGameTable.GetRow(level)->Total;
        //                                                 0        1
        QueryResult result  = WorldDatabase.Query("SELECT Level, Experience FROM player_xp_for_level");
        uint32 count = 0;
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();

                uint32 current_level = fields[0].GetUInt8();
                uint32 current_xp    = fields[1].GetUInt32();

                if (current_level >= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
                {
                    if (current_level > STRONG_MAX_LEVEL)        // hardcoded level maximum
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong (> %u) level %u in `player_xp_for_level` table, ignoring.", STRONG_MAX_LEVEL, current_level);
                    else
                    {
                        TC_LOG_INFO(LOG_FILTER_GENERAL, "Unused (> MaxPlayerLevel in worldserver.conf) level %u in `player_xp_for_levels` table, ignoring.", current_level);
                        ++count;                                // make result loading percent "expected" correct in case disabled detail mode for example.
                    }
                    continue;
                }

                _playerXPperLevel[current_level] = current_xp;
                ++count;
            }
            while (result->NextRow());
        }

        // fill level gaps - only accounting levels > MAX_LEVEL
        for (uint8 level = 1; level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
        {
            if (_playerXPperLevel[level] == 0)
            {
                TC_LOG_INFO(LOG_FILTER_GENERAL, "Level %i does not have XP for level data. Using data of level [%i] + 12000.", level + 1, level);
                _playerXPperLevel[level] = _playerXPperLevel[level - 1] + 12000;
            }
        }

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u xp for level definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
	
	// Load playercreate quests (for fun?)
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create Quests Data...");
    {
        oldMSTime = getMSTime();
        //                                                0     1      2       3
        QueryResult result = WorldDatabase.Query("SELECT race, class, questid FROM playercreateinfo_quest");
        if (!result)
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 custom player create quests. DB table `playercreateinfo_quest` is empty.");
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt8();
                if (current_race >= MAX_RACES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong race %u in `playercreateinfo_quest` table, ignoring.", current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt8();
                if (current_class >= MAX_CLASSES)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong class %u in `playercreateinfo_quest` table, ignoring.", current_class);
                    continue;
                }

                uint32 quest_id = fields[2].GetUInt32();
                if (!sQuestDataStore->GetQuestTemplate(quest_id))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Quest id %u (race %u class %u) in `playercreateinfo_quest` table but not listed in `quest_template`, ignoring.", quest_id, current_race, current_class);
                    continue;
                }

                if (!current_race && !current_class)
                    PlayerCreateInfoAddQuestHelper(RACE_NONE, CLASS_NONE, quest_id);
                else if (!current_race || !current_class)
                {
                    uint32 min_race = current_race ? current_race : 1;
                    uint32 max_race = current_race ? current_race + 1 : MAX_RACES;
                    uint32 min_class = current_class ? current_class : 1;
                    uint32 max_class = current_class ? current_class + 1 : MAX_CLASSES;
                    for (uint32 r = min_race; r < max_race; ++r)
                        for (uint32 c = min_class; c < max_class; ++c)
                            PlayerCreateInfoAddQuestHelper(r, c, quest_id);
                }
                else
                    PlayerCreateInfoAddQuestHelper(current_race, current_class, quest_id);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u custom player create quests in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }
}

uint32 ObjectMgr::GetPlayerClassLevelInfo(uint32 class_, uint8 level) const
{
    if (level < 1 || class_ >= MAX_CLASSES)
        return 0;

    level = std::min(level, MAX_LEVEL);

    GtBaseMPEntry const* mp = sBaseMPGameTable.GetRow(level);
    if (!mp)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Tried to get non-existant Class-Level combination data for base hp/mp. Class %u Level %u", class_, level);
        return 0;
    }

    return uint32(GetGameTableColumnForClass(mp, class_));
}

PlayerInfo const* ObjectMgr::GetPlayerInfo(uint32 race, uint32 class_) const
{
    if (race >= MAX_RACES)
        return nullptr;

    if (class_ >= MAX_CLASSES)
        return nullptr;

    PlayerInfo const* info = _playerInfo[race][class_];
    if (!info || info->displayId_m == 0 || info->displayId_f == 0)
        return nullptr;

    return info;
}

void ObjectMgr::GetPlayerLevelInfo(uint32 race, uint32 class_, uint8 level, PlayerLevelInfo* info) const
{
    if (level < 1 || race >= MAX_RACES || class_ >= MAX_CLASSES)
        return;

    PlayerInfo const* pInfo = _playerInfo[race][class_];
    if (pInfo->displayId_m == 0 || pInfo->displayId_f == 0)
        return;

    if (level <= MAX_LEVEL)
        *info = pInfo->levelInfo[level-1];
    else
        BuildPlayerLevelInfo(race, class_, level, info);
}

void ObjectMgr::BuildPlayerLevelInfo(uint8 race, uint8 _class, uint8 level, PlayerLevelInfo* info) const
{
    // base data (last known level)
    *info = _playerInfo[race][_class]->levelInfo[sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)-1];

    // if conversion from uint32 to uint8 causes unexpected behaviour, change lvl to uint32
    for (uint8 lvl = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)-1; lvl < level; ++lvl)
    {
        switch (_class)
        {
            case CLASS_WARRIOR:
                info->stats[STAT_STRENGTH]  += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 36 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 9 && !(lvl%2) ? 1: 0);
                break;
            case CLASS_PALADIN:
                info->stats[STAT_STRENGTH]  += (lvl > 3  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 7 && !(lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 6 && (lvl%2) ? 1: 0);
                break;
            case CLASS_HUNTER:
                info->stats[STAT_STRENGTH]  += (lvl > 4  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && (lvl%2) ? 1: 0);
                break;
            case CLASS_ROGUE:
            case CLASS_DEMON_HUNTER:
                info->stats[STAT_STRENGTH]  += (lvl > 5  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 16 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && !(lvl%2) ? 1: 0);
                break;
            case CLASS_PRIEST:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 22 ? 2: (lvl > 1 ? 1: 0));
                break;
            case CLASS_SHAMAN:
                info->stats[STAT_STRENGTH]  += (lvl > 34 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 4 ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 7 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 5 ? 1: 0);
                break;
            case CLASS_MAGE:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 24 ? 2: (lvl > 1 ? 1: 0));
                break;
            case CLASS_WARLOCK:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
                break;
            case CLASS_DRUID:
                info->stats[STAT_STRENGTH]  += (lvl > 38 ? 2: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 32 ? 2: (lvl > 4 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 2: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 38 ? 3: (lvl > 4 ? 1: 0));
            default:
                break;
        }
    }
}

void ObjectMgr::LoadPageTexts()
{
    uint32 oldMSTime = getMSTime();

    //                                               0   1     2           3                  4
    QueryResult result = WorldDatabase.Query("SELECT ID, Text, NextPageID, PlayerConditionID, Flags FROM page_text");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 page texts. DB table `page_text` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        PageText& pageText = _pageTextStore[fields[0].GetUInt32()];
        pageText.Text = fields[1].GetString();
        pageText.NextPageID = fields[2].GetUInt32();
        pageText.PlayerConditionID = fields[3].GetInt32();
        pageText.Flags = fields[4].GetUInt8();

        ++count;
    } while (result->NextRow());

    for (auto const& itr : _pageTextStore)
        if (itr.second.NextPageID)
            if (_pageTextStore.find(itr.second.NextPageID) == _pageTextStore.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Page text (Id: %u) has not existing next page (Id: %u)", itr.first, itr.second.NextPageID);
            }
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u page texts in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

PageText const* ObjectMgr::GetPageText(uint32 pageEntry)
{
    return Trinity::Containers::MapGetValuePtr(_pageTextStore, pageEntry);
}

void ObjectMgr::LoadPageTextLocales()
{
    uint32 oldMSTime = getMSTime();

    _pageTextLocaleStore.clear();

    //                                               0      1     2
    QueryResult result = WorldDatabase.Query("SELECT ID, Locale, Text FROM page_text_locale");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        LocaleConstant locale = GetLocaleByName(fields[1].GetString());
        if (locale == LOCALE_enUS)
            continue;

        PageTextLocale& data = _pageTextLocaleStore[fields[0].GetUInt32()];
        AddLocaleString(fields[2].GetString(), locale, data.Text);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %lu PageText locale strings in %u ms", static_cast<unsigned long>(_pageTextLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadInstanceTemplate()
{
    uint32 oldMSTime = getMSTime();

    _instanceTemplateVector.assign(sMapStore.GetNumRows() + 1, nullptr);

    //                                                0     1       2        4             5
    QueryResult result = WorldDatabase.Query("SELECT map, parent, script, allowMount, bonusChance FROM instance_template");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 instance templates. DB table `page_text` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint16 mapID = fields[0].GetUInt16();

        if (!MapManager::IsValidMAP(mapID, true))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "ObjectMgr::LoadInstanceTemplate: bad mapid %d for template!", mapID);
            continue;
        }

        InstanceTemplate instanceTemplate;

        instanceTemplate.AllowMount     = fields[3].GetBool();
        instanceTemplate.Parent         = uint32(fields[1].GetUInt16());
        instanceTemplate.ScriptId       = sObjectMgr->GetScriptId(fields[2].GetCString());
        instanceTemplate.bonusChance    = fields[3].GetUInt32();

        _instanceTemplateStore[mapID] = instanceTemplate;
        _instanceTemplateVector[mapID] = &_instanceTemplateStore[mapID];

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u instance templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

uint32 ObjectMgr::GetEntryByBonusSpell(uint32 spellId) const
{
    CreatureSpellBonusList::const_iterator itr = _creatureSpellBonus.find(spellId);
    if (itr != _creatureSpellBonus.end())
        return itr->second;
    return 0;
}

InstanceTemplate const* ObjectMgr::GetInstanceTemplate(uint32 mapID)
{
    return _instanceTemplateVector[mapID];
}

void ObjectMgr::LoadInstanceEncounters()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0         1            2                3                4
    QueryResult result = WorldDatabase.Query("SELECT entry, creditType, creditEntry, lastEncounterDungeon, difficulty FROM instance_encounters");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 instance encounters, table is empty!");
        return;
    }

    for (int32 i = 0; i < MAX_DIFFICULTY; ++i)
        _dungeonEncounterVector[i].assign(sMapStore.GetNumRows() + 1, nullptr);

    uint32 count = 0;
    std::map<uint32, DungeonEncounterEntry const*> dungeonLastBosses;
    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        uint8 creditType = fields[1].GetUInt8();
        uint32 creditEntry = fields[2].GetUInt32();
        uint32 lastEncounterDungeon = fields[3].GetUInt16();
        int32 DungeonDifficult = fields[4].GetInt32();

        DungeonEncounterEntry const* dungeonEncounter = sDungeonEncounterStore.LookupEntry(entry);
        if (!dungeonEncounter)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `instance_encounters` has an invalid encounter id %u, skipped!", entry);
            continue;
        }

        if (lastEncounterDungeon && !sLFGMgr->GetLFGDungeonEntry(lastEncounterDungeon))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `instance_encounters` has an encounter %u (%s) marked as final for invalid dungeon id %u, skipped!", entry, dungeonEncounter->Name->Str[LOCALE_enUS], lastEncounterDungeon);
            continue;
        }

        std::map<uint32, DungeonEncounterEntry const*>::const_iterator itr = dungeonLastBosses.find(lastEncounterDungeon);
        if (lastEncounterDungeon)
        {
            if (itr != dungeonLastBosses.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `instance_encounters` specified encounter %u (%s) as last encounter but %u (%s) is already marked as one, skipped!", entry, dungeonEncounter->Name->Str[LOCALE_enUS], itr->second->ID, itr->second->Name->Str[LOCALE_enUS]);
                continue;
            }

            dungeonLastBosses[lastEncounterDungeon] = dungeonEncounter;
        }

        switch (creditType)
        {
            case ENCOUNTER_CREDIT_KILL_CREATURE:
            {
                CreatureTemplate const* creatureInfo = GetCreatureTemplate(creditEntry);
                if (!creatureInfo)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `instance_encounters` has an invalid creature (entry %u) linked to the encounter %u (%s), skipped!", creditEntry, entry, dungeonEncounter->Name->Str[LOCALE_enUS]);
                    continue;
                }
                const_cast<CreatureTemplate*>(creatureInfo)->flags_extra |= CREATURE_FLAG_EXTRA_DUNGEON_BOSS;
                break;
            }
            case ENCOUNTER_CREDIT_CAST_SPELL:
                if (!sSpellMgr->GetSpellInfo(creditEntry))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `instance_encounters` has an invalid spell (entry %u) linked to the encounter %u (%s), skipped!", creditEntry, entry, dungeonEncounter->Name->Str[LOCALE_enUS]);
                    continue;
                }
                break;
            default:
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `instance_encounters` has an invalid credit type (%u) for encounter %u (%s), skipped!", creditType, entry, dungeonEncounter->Name->Str[LOCALE_enUS]);
                continue;
        }

        if (dungeonEncounter->DifficultyID > 0 && dungeonEncounter->DifficultyID != DungeonDifficult)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `instance_encounters` overvrite dificult from dbc for encounter %u. DBC %i, DB %i", entry, dungeonEncounter->DifficultyID, DungeonDifficult);
            DungeonDifficult = dungeonEncounter->DifficultyID;
        }

        _creatureToDungeonEncounter[creditEntry] = entry;
        if (_dungeonEncounterToCreature.size() <= entry)
            _dungeonEncounterToCreature.resize(entry + 1, 0);
        _dungeonEncounterToCreature[entry] = creditEntry;

        if (DungeonDifficult > 0)
        {
            DungeonEncounterList& encounters = _dungeonEncounterStore[MAKE_PAIR32(dungeonEncounter->MapID, DungeonDifficult)];
            encounters.push_back(new DungeonEncounter(dungeonEncounter, EncounterCreditType(creditType), creditEntry, lastEncounterDungeon));
            _dungeonEncounterVector[DungeonDifficult][dungeonEncounter->MapID] = &encounters;
        }
        else
            for (int32 i = 0; i < MAX_DIFFICULTY; ++i)
            {
                DungeonEncounterList& encounters = _dungeonEncounterStore[MAKE_PAIR32(dungeonEncounter->MapID, Difficulty(i))];
                encounters.push_back(new DungeonEncounter(dungeonEncounter, EncounterCreditType(creditType), creditEntry, lastEncounterDungeon));
                _dungeonEncounterVector[i][dungeonEncounter->MapID] = &encounters;
            }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u instance encounters in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

NpcText const* ObjectMgr::GetNpcText(uint32 textID) const
{
    return Trinity::Containers::MapGetValuePtr(_npcTextStore, textID);
}

void ObjectMgr::LoadNPCText()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT ID, "
        "Probability0, Probability1, Probability2, Probability3, Probability4, Probability5, Probability6, Probability7, "
        "BroadcastTextID0, BroadcastTextID1, BroadcastTextID2, BroadcastTextID3, BroadcastTextID4, BroadcastTextID5, BroadcastTextID6, BroadcastTextID7"
        " FROM npc_text");

    int count = 0;
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u npc texts", count);
        return;
    }
    _npcTextStore.rehash(result->GetRowCount());

    do
    {
        ++count;
        int cic = 0;

        Field* fields = result->Fetch();

        uint32 textID = fields[cic++].GetUInt32();
        if (!textID)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `npc_text` has record wit reserved id 0, ignore.");
            continue;
        }

        NpcText& npcText = _npcTextStore[textID];

        for (uint8 i = 0; i < MAX_NPC_TEXT_OPTIONS; ++i)
        {
            npcText.Data[i].Probability      = fields[1 + i].GetFloat();
            npcText.Data[i].BroadcastTextID  = fields[9 + i].GetUInt32();
        }
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u npc texts in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

//not very fast function but it is called only once a day, or on starting-up
void ObjectMgr::ReturnOrDeleteOldMails(bool serverUp)
{
    uint32 oldMSTime = getMSTime();

    time_t curTime = time(nullptr);
    tm lt;
    localtime_r(&curTime, &lt);
    uint64 basetime(curTime);
    TC_LOG_INFO(LOG_FILTER_GENERAL, "Returning mails current time: hour: %d, minute: %d, second: %d ", lt.tm_hour, lt.tm_min, lt.tm_sec);

    // Delete all old mails without item and without body immediately, if starting server
    if (!serverUp)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_EMPTY_EXPIRED_MAIL);
        stmt->setUInt64(0, basetime);
        CharacterDatabase.Execute(stmt);
    }
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_EXPIRED_MAIL);
    stmt->setUInt64(0, basetime);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
    {
        return;                                             // any mails need to be returned or deleted
    }

    std::map<uint32 /*messageId*/, MailItemInfoVec> itemsCache;
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_EXPIRED_MAIL_ITEMS);
    stmt->setUInt32(0, static_cast<uint32>(basetime));
    if (PreparedQueryResult items = CharacterDatabase.Query(stmt))
    {
        MailItemInfo item;
        do
        {
            Field* fields = items->Fetch();
            item.item_guid = fields[0].GetUInt64();
            item.item_template = fields[1].GetUInt32();
            uint32 mailId = fields[2].GetUInt32();
            itemsCache[mailId].push_back(item);
        } while (items->NextRow());
    }

    uint32 deletedCount = 0;
    uint32 returnedCount = 0;
    do
    {
        Field* fields = result->Fetch();
        Mail* m = new Mail;
        m->messageID      = fields[0].GetUInt32();
        m->messageType    = fields[1].GetUInt8();
        m->sender         = fields[2].GetUInt64();
        m->receiver       = fields[3].GetUInt64();
        bool has_items    = fields[4].GetBool();
        m->expire_time    = time_t(fields[5].GetUInt32());
        m->deliver_time   = 0;
        m->COD            = fields[6].GetUInt64();
        m->checked        = fields[7].GetUInt8();
        m->mailTemplateId = fields[8].GetInt16();

        Player* player = nullptr;
        if (serverUp)
            player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(m->receiver));

        if (player && player->m_mailsLoaded)
        {                                                   // this code will run very improbably (the time is between 4 and 5 am, in game is online a player, who has old mail
            // his in mailbox and he has already listed his mails)
            delete m;
            continue;
        }

        // Delete or return mail
        if (has_items)
        {
            // read items from cache
            m->items.swap(itemsCache[m->messageID]);

            // if it is mail from non-player, or if it's already return mail, it shouldn't be returned, but deleted
            if (m->messageType != MAIL_NORMAL || (m->checked & (MAIL_CHECK_MASK_COD_PAYMENT | MAIL_CHECK_MASK_RETURNED)))
            {
                // mail open and then not returned
                for (MailItemInfoVec::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
                {
                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_INSTANCE);
                    stmt->setUInt64(0, itr2->item_guid);
                    CharacterDatabase.Execute(stmt);
                }
            }
            else
            {
                // Mail will be returned
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_MAIL_RETURNED);
                stmt->setUInt64(0, m->receiver);
                stmt->setUInt64(1, m->sender);
                stmt->setUInt32(2, basetime + 30 * DAY);
                stmt->setUInt32(3, basetime);
                stmt->setUInt8 (4, uint8(MAIL_CHECK_MASK_RETURNED));
                stmt->setUInt32(5, m->messageID);
                CharacterDatabase.Execute(stmt);
                for (MailItemInfoVec::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
                {
                    // Update receiver in mail items for its proper delivery, and in instance_item for avoid lost item at sender delete
                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_MAIL_ITEM_RECEIVER);
                    stmt->setUInt64(0, m->sender);
                    stmt->setUInt64(1, itr2->item_guid);
                    CharacterDatabase.Execute(stmt);

                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ITEM_OWNER);
                    stmt->setUInt64(0, m->sender);
                    stmt->setUInt64(1, itr2->item_guid);
                    CharacterDatabase.Execute(stmt);
                }
                delete m;
                ++returnedCount;
                continue;
            }
        }

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_BY_ID);
        stmt->setUInt32(0, m->messageID);
        CharacterDatabase.Execute(stmt);
        delete m;
        ++deletedCount;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Processed %u expired mails: %u deleted and %u returned in %u ms", deletedCount + returnedCount, deletedCount, returnedCount, GetMSTimeDiffToNow(oldMSTime));
}

uint32 ObjectMgr::GetNearestTaxiNode(float x, float y, float z, uint32 mapid, Player* player)
{
    bool found = false;
    float dist = 10000;
    uint32 id = 0;

    uint32 requireFlag = (player->GetTeam() == ALLIANCE) ? TAXI_NODE_FLAG_ALLIANCE : TAXI_NODE_FLAG_HORDE;
    for (TaxiNodesEntry const* node : sTaxiNodesStore)
    {
        if (!node || node->ContinentID != mapid || !(node->Flags & requireFlag))
            continue;

        if (node->ConditionID)
        {
            if ((node->Flags & TAXI_NODE_FLAG_UNK2) && !sPlayerConditionStore.LookupEntry(node->ConditionID)) //TAXI_NODE_FLAG_UNK2 not use if condition not found in db2
                continue;
            if (!sConditionMgr->IsPlayerMeetingCondition(player, node->ConditionID))
                continue;
        }

        uint16 field   = static_cast<uint16>((node->ID - 1) / 8);
        uint32 submask = 1 << ((node->ID - 1) % 8);

        // skip not taxi network nodes
        if ((sTaxiNodesMask[field] & submask) == 0)
            continue;

        float dist2 = (node->Pos.X - x)*(node->Pos.X - x) + (node->Pos.Y - y)*(node->Pos.Y - y) + (node->Pos.Z - z)*(node->Pos.Z - z);
        if (found)
        {
            if (dist2 < dist)
            {
                dist = dist2;
                id = node->ID;
            }
        }
        else
        {
            found = true;
            dist = dist2;
            id = node->ID;
        }
    }

    return id;
}

void ObjectMgr::GetTaxiPath(uint32 source, uint32 destination, uint32 &path, uint32 &cost)
{
    TaxiPathSetBySource::iterator src_i = sTaxiPathSetBySource.find(source);
    if (src_i == sTaxiPathSetBySource.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    TaxiPathSetForSource& pathSet = src_i->second;

    TaxiPathSetForSource::iterator dest_i = pathSet.find(destination);
    if (dest_i == pathSet.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    cost = dest_i->second.price;
    path = dest_i->second.ID;
}

uint32 ObjectMgr::GetTaxiMountDisplayId(uint32 id, uint32 team, bool allowed_alt_team /* = false */)
{
    uint32 mount_id = 0;

    // select mount creature id
    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(id);
    if (node)
    {
        uint32 mount_entry;
        if (team == ALLIANCE)
            mount_entry = node->MountCreatureID[1];
        else
            mount_entry = node->MountCreatureID[0];

        // Fix for Alliance not being able to use Acherus taxi
        // only one mount type for both sides
        if (mount_entry == 0 && allowed_alt_team)
        {
            // Simply reverse the selection. At least one team in theory should have a valid mount ID to choose.
            mount_entry = team == ALLIANCE ? node->MountCreatureID[0] : node->MountCreatureID[1];
        }

        CreatureTemplate const* mount_info = GetCreatureTemplate(mount_entry);
        if (mount_info)
        {
            mount_id = sObjectMgr->GetCreatureDisplay(mount_info->GetRandomValidModelId());
            if (!mount_id)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "No displayid found for the taxi mount with the entry %u! Can't load it!", mount_entry);
                return 0;
            }
        }
    }

    // minfo is not actually used but the mount_id was updated
    GetCreatureModelRandomGender(&mount_id);

    return mount_id;
}

void ObjectMgr::LoadGraveyardZones()
{
    uint32 oldMSTime = getMSTime();

    GraveYardStore.clear();                                  // need for reload case

    //                                                0       1         2
    QueryResult result = WorldDatabase.Query("SELECT id, ghost_zone, faction FROM game_graveyard_zone");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 graveyard-zone links. DB table `game_graveyard_zone` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 safeLocId = fields[0].GetUInt32();
        uint32 zoneId = fields[1].GetUInt32();
        uint32 team   = fields[2].GetUInt16();

        WorldSafeLocsEntry const* safeLocEntry = sWorldSafeLocsStore.LookupEntry(safeLocId);
        if (!safeLocEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_graveyard_zone` has a record for not existing graveyard (WorldSafeLocs.dbc id) %u, skipped.", safeLocId);
            continue;
        }

        if (!zoneId)
        {
            // Query for select graveyard
            // SELECT ID,0,0 FROM `test`.`dbc_worldsafelocs` WHERE (AreaName LIKE '%GY%' OR AreaName LIKE '%Graveyard%' OR AreaName LIKE '%SBV%' OR AreaName LIKE '%IAG%' OR AreaName LIKE '%CAG%' OR AreaName LIKE '%Safe%') AND id NOT IN (SELECT id FROM `trinworld703`.`game_graveyard_zone`);
            zoneId = sMapMgr->GetZoneId(safeLocEntry->MapID, safeLocEntry->Loc.X, safeLocEntry->Loc.Y, safeLocEntry->Loc.Z);
            AreaTableEntry const* zone = sAreaTableStore.LookupEntry(zoneId);
            if (!zone)
                continue;

            switch (zone->FactionGroupMask)
            {
                case AREATEAM_ALLY:
                    team = 469;
                    break;
                case AREATEAM_HORDE:
                    team = 67;
                    break;
                default:
                    break;
            }

            WorldDatabase.PExecute("UPDATE game_graveyard_zone SET ghost_zone = %u, `faction` = %u WHERE id = %u and ghost_zone = 0", zoneId, team, safeLocId);

            // update in DB
            MapEntry const* map = sMapStore.LookupEntry(safeLocEntry->MapID);
            if (map && !map->Instanceable())
            {
                float angel = (safeLocEntry->Loc.O * M_PI) / 180;
                float x = safeLocEntry->Loc.X + 4.0f * std::cos(angel);
                float y = safeLocEntry->Loc.Y + 4.0f * std::sin(angel);
                Trinity::NormalizeMapCoord(x);
                Trinity::NormalizeMapCoord(y);

                uint64 dbGuid = sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate();
                SQLTransaction trans = WorldDatabase.BeginTransaction();
                PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CREATURE);
                stmt->setUInt64(0, dbGuid);
                trans->Append(stmt);

                uint8 index = 0;

                stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_CREATURE);
                stmt->setUInt64(index++, dbGuid);
                stmt->setUInt32(index++, 6491);
                stmt->setUInt16(index++, uint16(safeLocEntry->MapID));
                stmt->setUInt32(index++, zoneId);
                stmt->setUInt32(index++, 0); // Area
                stmt->setUInt64(index++, 1); // SpawnMask
                stmt->setUInt16(index++, 1); // PhaseMask
                stmt->setUInt32(index++, 5233); // ModelId
                stmt->setUInt8(index++, 0); // EquipmentId
                stmt->setFloat(index++,  x);
                stmt->setFloat(index++,  y);
                stmt->setFloat(index++,  safeLocEntry->Loc.Z);
                stmt->setFloat(index++,  angel > M_PI ? angel - M_PI : angel + M_PI);
                stmt->setUInt32(index++, 600); // Respawn time
                stmt->setFloat(index++,  0); // Resp distance
                stmt->setUInt32(index++, 0);
                stmt->setUInt32(index++, 8240); // HP
                stmt->setUInt32(index++, 8240); // MP
                stmt->setUInt8(index++,  0);
                stmt->setUInt32(index++, 0);
                stmt->setUInt32(index++, 0);
                stmt->setUInt32(index++, 0);
                stmt->setUInt32(index++, 0);
                trans->Append(stmt);
                WorldDatabase.CommitTransaction(trans);
            }
        }

        if (!sAreaTableStore.LookupEntry(zoneId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_graveyard_zone` has a record for not existing zone id (%u), skipped.", zoneId);
            continue;
        }

        if (team != 0 && team != HORDE && team != ALLIANCE)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_graveyard_zone` has a record for non player faction (%u), skipped.", team);
            continue;
        }

        if (!AddGraveYardLink(safeLocId, zoneId, team, false))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_graveyard_zone` has a duplicate record for Graveyard (ID: %u) and Zone (ID: %u), skipped.", safeLocId, zoneId);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u graveyard-zone links in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

WorldSafeLocsEntry const* ObjectMgr::GetDefaultGraveYard(uint32 team)
{
    enum DefaultGraveyard
    {
        HORDE_GRAVEYARD = 10, // Crossroads
        ALLIANCE_GRAVEYARD = 4, // Westfall
    };

    if (team == HORDE)
        return sWorldSafeLocsStore.LookupEntry(HORDE_GRAVEYARD);
    if (team == ALLIANCE)
        return sWorldSafeLocsStore.LookupEntry(ALLIANCE_GRAVEYARD);
    return nullptr;
}

WorldSafeLocsEntry const* ObjectMgr::GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team, bool outInstance /*= false*/)
{
    // search for zone associated closest graveyard
    uint32 zoneId = sMapMgr->GetZoneId(MapId, x, y, z);

    if (!zoneId)
    {
        if (z > -500)
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "ZoneId not found for map %u coords (%f, %f, %f)", MapId, x, y, z);
            return GetDefaultGraveYard(team);
        }
    }

    // Simulate std. algorithm:
    //   found some graveyard associated to (ghost_zone, ghost_map)
    //
    //   if mapId == graveyard.mapId (ghost in plain zone or city or battleground) and search graveyard at same map
    //     then check faction
    //   if mapId != graveyard.mapId (ghost in instance) and search any graveyard associated
    //     then check faction
    GraveYardContainer::const_iterator graveLow  = GraveYardStore.lower_bound(zoneId);
    GraveYardContainer::const_iterator graveUp   = GraveYardStore.upper_bound(zoneId);
    MapEntry const* mapEntry = sMapStore.LookupEntry(MapId);
    // not need to check validity of map object; MapId _MUST_ be valid here

    if (graveLow == graveUp && !mapEntry->IsBattleArena())
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_graveyard_zone` incomplete: Zone %u Team %u does not have a linked graveyard.", zoneId, team);
        return nullptr;
    }

    // at corpse map
    bool foundNear = false;
    float distNear = 10000;
    WorldSafeLocsEntry const* entryNear = nullptr;

    // at entrance map for corpse map
    bool foundEntr = false;
    float distEntr = 10000;
    WorldSafeLocsEntry const* entryEntr = nullptr;

    // some where other
    WorldSafeLocsEntry const* entryFar = nullptr;

    for (GraveYardContainer::const_iterator itr = graveLow; itr != graveUp; ++itr)
    {
        GraveYardData const& data = itr->second;

        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(data.safeLocId);
        if (!entry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_graveyard_zone` has record for not existing graveyard (WorldSafeLocs.dbc id) %u, skipped.", data.safeLocId);
            continue;
        }

        // skip enemy faction graveyard
        // team == 0 case can be at call from .neargrave
        if (data.team != 0 && team != 0 && data.team != team)
            continue;

        // find now nearest graveyard at other map
        if (MapId != entry->MapID)
        {
            // if find graveyard at different map from where entrance placed (or no entrance data), use any first
            if (!mapEntry || !outInstance && (mapEntry->CorpseMapID < 0 || uint32(mapEntry->CorpseMapID) != entry->MapID || (mapEntry->CorpsePos.X == 0.0f && mapEntry->CorpsePos.Y == 0.0f)))
            {
                // not have any corrdinates for check distance anyway
                entryFar = entry;
                continue;
            }

            // at entrance map calculate distance (2D);
            float dist2 = (entry->Loc.X - mapEntry->CorpsePos.X)*(entry->Loc.X - mapEntry->CorpsePos.X)
                + (entry->Loc.Y - mapEntry->CorpsePos.Y)*(entry->Loc.Y - mapEntry->CorpsePos.Y);

            if (foundEntr)
            {
                if (dist2 < distEntr)
                {
                    distEntr = dist2;
                    entryEntr = entry;
                }
            }
            else
            {
                foundEntr = true;
                distEntr = dist2;
                entryEntr = entry;
            }
        }
        // find now nearest graveyard at same map
        else
        {
            float dist2 = (entry->Loc.X - x)*(entry->Loc.X - x) + (entry->Loc.Y - y)*(entry->Loc.Y - y) + (entry->Loc.Z - z)*(entry->Loc.Z - z);
            if (foundNear)
            {
                if (dist2 < distNear)
                {
                    distNear = dist2;
                    entryNear = entry;
                }
            }
            else
            {
                foundNear = true;
                distNear = dist2;
                entryNear = entry;
            }
        }
    }

    // search for the nearest graveyard outside the dungeon
    if (outInstance && mapEntry->IsDungeon())
    {
        if (entryEntr)
            return entryEntr;
        if (entryNear)
            return entryNear;
    }

    if (entryNear)
        return entryNear;

    if (entryEntr)
        return entryEntr;

    return entryFar;
}

GraveYardData const* ObjectMgr::FindGraveYardData(uint32 id, uint32 zoneId)
{
    GraveYardContainer::const_iterator graveLow  = GraveYardStore.lower_bound(zoneId);
    GraveYardContainer::const_iterator graveUp   = GraveYardStore.upper_bound(zoneId);

    for (GraveYardContainer::const_iterator itr = graveLow; itr != graveUp; ++itr)
    {
        if (itr->second.safeLocId == id)
            return &itr->second;
    }

    return nullptr;
}

bool ObjectMgr::AddGraveYardLink(uint32 id, uint32 zoneId, uint32 team, bool persist /*= true*/)
{
    if (FindGraveYardData(id, zoneId))
        return false;

    // add link to loaded data
    GraveYardData data;
    data.safeLocId = id;
    data.team = team;

    GraveYardStore.insert(std::make_pair(zoneId, data));

    // add link to DB
    if (persist)
    {
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_GRAVEYARD_ZONE);

        stmt->setUInt32(0, id);
        stmt->setUInt32(1, zoneId);
        stmt->setUInt16(2, uint16(team));

        WorldDatabase.Execute(stmt);
    }

    return true;
}

void ObjectMgr::RemoveGraveYardLink(uint32 id, uint32 zoneId, uint32 team, bool persist /*= false*/)
{
    GraveYardContainer::iterator graveLow  = GraveYardStore.lower_bound(zoneId);
    GraveYardContainer::iterator graveUp   = GraveYardStore.upper_bound(zoneId);
    if (graveLow == graveUp)
    {
        //TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_graveyard_zone` incomplete: Zone %u Team %u does not have a linked graveyard.", zoneId, team);
        return;
    }

    bool found = false;

    GraveYardContainer::iterator itr;

    for (itr = graveLow; itr != graveUp; ++itr)
    {
        GraveYardData & data = itr->second;

        // skip not matching safezone id
        if (data.safeLocId != id)
            continue;

        // skip enemy faction graveyard at same map (normal area, city, or battleground)
        // team == 0 case can be at call from .neargrave
        if (data.team != 0 && team != 0 && data.team != team)
            continue;

        found = true;
        break;
    }

    // no match, return
    if (!found)
        return;

    // remove from links
    GraveYardStore.erase(itr);

    // remove link from DB
    if (persist)
    {
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GRAVEYARD_ZONE);

        stmt->setUInt32(0, id);
        stmt->setUInt32(1, zoneId);
        stmt->setUInt16(2, uint16(team));

        WorldDatabase.Execute(stmt);
    }
}

void ObjectMgr::AddInstanceGraveYard(uint32 mapId, float x, float y, float z, float o)
{
    if (mapId >= _instanceGraveYardStore.size())
        _instanceGraveYardStore.resize(mapId + 1);

    // WorldLocation loc = WorldLocation(mapId, x, y, z, o);
    _instanceGraveYardStore[mapId].emplace_back(mapId, x, y, z, o);
}

std::vector<WorldLocation> const* ObjectMgr::GetInstanceGraveYard(uint32 mapId) const
{
    if (mapId >= _instanceGraveYardStore.size())
        return nullptr;
    return &_instanceGraveYardStore[mapId];
}

AccessRequirement const* ObjectMgr::GetAccessRequirement(int32 mapid, Difficulty difficulty, uint16 dungeonId) const
{
    return Trinity::Containers::MapGetValuePtr(_accessRequirementStore, AccessRequirementKey(mapid, uint8(difficulty), dungeonId));
}

void ObjectMgr::LoadAccessRequirements()
{
    uint32 oldMSTime = getMSTime();

    _accessRequirementStore.clear();                                  // need for reload case

    //                                               0      1           2          3          4     5      6             7             8                      9                  10                       11
    QueryResult result = WorldDatabase.Query("SELECT mapid, difficulty, level_min, level_max, item, item2, quest_done_A, quest_done_H, completed_achievement, quest_failed_text, completed_achievement_A, dungeonId FROM access_requirement");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 access requirement definitions. DB table `access_requirement` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        ++count;

        int32 mapid = fields[0].GetInt32();
        uint8 difficulty = fields[1].GetUInt8();
        uint16 dungeonId = fields[11].GetUInt32();
        AccessRequirementKey requirement_ID (mapid, difficulty, dungeonId);

        AccessRequirement ar;

        ar.mapid                    = mapid;
        ar.difficulty               = difficulty;
        ar.dungeonId                = dungeonId;
        ar.levelMin                 = fields[2].GetUInt8();
        ar.levelMax                 = fields[3].GetUInt8();
        ar.item                     = fields[4].GetUInt32();
        ar.item2                    = fields[5].GetUInt32();
        ar.quest_A                  = fields[6].GetUInt32();
        ar.quest_H                  = fields[7].GetUInt32();
        ar.achievement              = fields[8].GetUInt32();
        ar.questFailedText          = fields[9].GetString();
        ar.achievement_A            = fields[10].GetUInt32();

        if (ar.item)
        {
            ItemTemplate const* pProto = GetItemTemplate(ar.item);
            if (!pProto)
            {
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "Key item %u does not exist for map %u difficulty %u, removing key requirement.", ar.item, mapid, difficulty);
                ar.item = 0;
            }
        }

        if (ar.item2)
        {
            ItemTemplate const* pProto = GetItemTemplate(ar.item2);
            if (!pProto)
            {
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "Second item %u does not exist for map %u difficulty %u, removing key requirement.", ar.item2, mapid, difficulty);
                ar.item2 = 0;
            }
        }

        if (ar.quest_A)
        {
            if (!sQuestDataStore->GetQuestTemplate(ar.quest_A))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Required Alliance Quest %u not exist for map %u difficulty %u, remove quest done requirement.", ar.quest_A, mapid, difficulty);
                ar.quest_A = 0;
            }
        }

        if (ar.quest_H)
        {
            if (!sQuestDataStore->GetQuestTemplate(ar.quest_H))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Required Horde Quest %u not exist for map %u difficulty %u, remove quest done requirement.", ar.quest_H, mapid, difficulty);
                ar.quest_H = 0;
            }
        }

        if (ar.achievement)
        {
            if (!sAchievementStore.LookupEntry(ar.achievement))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Required Achievement %u not exist for map %u difficulty %u, remove quest done requirement.", ar.achievement, mapid, difficulty);
                ar.achievement = 0;
            }
        }

        if (ar.achievement_A)
        {
            if (!sAchievementStore.LookupEntry(ar.achievement_A))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Required achievement_A %u not exist for map %u difficulty %u, remove quest done requirement.", ar.achievement_A, mapid, difficulty);
                ar.achievement_A = 0;
            }
        }

        _accessRequirementStore[requirement_ID] = ar;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u access requirement definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::SetHighestGuids()
{
    memset(playerCountInArea, 0, sizeof(int32)*10000);
    memset(creatureCountInArea, 0, sizeof(int32)*10000);
    memset(objectCountInWorld, 0, sizeof(int32)*50);

    QueryResult result = CharacterDatabase.Query("SELECT MAX(guid) FROM characters");
    if (result)
    {
        _playerGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::Player, (*result)[0].GetUInt64() + 1);
    }

    if (result = WorldDatabase.Query("SELECT MAX(guid) FROM creature"))
    {
        _creatureGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::Creature, (*result)[0].GetUInt64() + 1);
    }

    if (result = CharacterDatabase.Query("SELECT MAX(guid) FROM item_instance"))
        _itemGuidGenerator.Set((*result)[0].GetUInt64() + 1);

    // Cleanup other tables from not existed guids ( >= _hiItemGuid)
    CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item >= '%u'", _itemGuidGenerator.GetNextAfterMaxUsed());      // One-time query
    CharacterDatabase.PExecute("DELETE FROM mail_items WHERE item_guid >= '%u'", _itemGuidGenerator.GetNextAfterMaxUsed());          // One-time query
    CharacterDatabase.PExecute("DELETE FROM auctionhouse WHERE itemguid >= '%u'", _itemGuidGenerator.GetNextAfterMaxUsed());         // One-time query
    CharacterDatabase.PExecute("DELETE FROM guild_bank_item WHERE item_guid >= '%u'", _itemGuidGenerator.GetNextAfterMaxUsed());     // One-time query

    if (result = WorldDatabase.Query("SELECT MAX(guid) FROM gameobject"))
    {
        _gameObjectGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::GameObject, (*result)[0].GetUInt64() + 1);
    }

    if (result = WorldDatabase.Query("SELECT MAX(guid) FROM transports"))
    {
        _moTransportGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::Transport, (*result)[0].GetUInt64() + 1);
    }

    if (result = CharacterDatabase.Query("SELECT MAX(id) FROM auctionhouse"))
        _auctionId = (*result)[0].GetUInt32()+1;

    if (result = CharacterDatabase.Query("SELECT MAX(id) FROM mail"))
        _mailId = (*result)[0].GetUInt32()+1;

    if (result = CharacterDatabase.Query("SELECT MAX(corpseGuid) FROM corpse"))
    {
        _corpseGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::Corpse, (*result)[0].GetUInt64() + 1);
    }

    if (result = WorldDatabase.Query("SELECT MAX(guid) FROM conversation"))
    {
        _conversationGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::Conversation, (*result)[0].GetUInt64() + 1);
    }

    if (result = CharacterDatabase.Query("SELECT MAX(setguid) FROM character_equipmentsets"))
        _equipmentSetGuid = (*result)[0].GetUInt64()+1;

    if (result = CharacterDatabase.Query("SELECT MAX(ID) FROM report_complaints"))
        _reportComplaintID = (*result)[0].GetUInt64()+1;

    if (result = CharacterDatabase.Query("SELECT MAX(ID) FROM report_bugreport"))
        _supportTicketSubmitBugID = (*result)[0].GetUInt64()+1;

    if (result = CharacterDatabase.Query("SELECT MAX(guildId) FROM guild"))
        sGuildMgr->SetNextGuildId((*result)[0].GetUInt64()+1);

    if (result = CharacterDatabase.Query("SELECT MAX(guid) FROM groups"))
        sGroupMgr->SetGroupDbStoreSize((*result)[0].GetUInt32()+1);

    if (result = CharacterDatabase.Query("SELECT MAX(itemId) from character_void_storage"))
        _voidItemId = (*result)[0].GetUInt64()+1;

    if (result = CharacterDatabase.Query("SELECT MAX(id) FROM account_battlepet"))
        _BattlePetGuidGenerator.Set((*result)[0].GetUInt64() + 1);

    if (result = WorldDatabase.Query("SELECT MAX(guid) FROM eventobject"))
    {
        _EventObjectGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::EventObject, (*result)[0].GetUInt64() + 1);
    }

    if (result = CharacterDatabase.Query("SELECT MAX(ID) FROM challenge"))
    {
        _scenarioGuidGenerator.Set((*result)[0].GetUInt64() + 1);
        ObjectAccessor::SetGuidSize(HighGuid::Scenario, (*result)[0].GetUInt64() + 1);
    }

    _PetBattleGuidGenerator.GetNextAfterMaxUsed();
    _LFGListGuidGenerator.GetNextAfterMaxUsed();
    _LFGObjectGuidGenerator.GetNextAfterMaxUsed();
    _vignetteGuidGenerator.GetNextAfterMaxUsed();
}

uint64 ObjectMgr::GenerateReportComplaintID()
{
    if (_reportComplaintID >= std::numeric_limits<uint64>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "_reportComplaintID overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return _reportComplaintID++;
}

uint64 ObjectMgr::GenerateSupportTicketSubmitBugID()
{
    if (_supportTicketSubmitBugID >= std::numeric_limits<uint64>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "_supportTicketSubmitBugID overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return _supportTicketSubmitBugID++;
}

MailLevelReward const* ObjectMgr::GetMailLevelReward(uint32 level, uint32 raceMask)
{
    MailLevelRewardContainer::const_iterator map_itr = _mailLevelRewardStore.find(level);
    if (map_itr == _mailLevelRewardStore.end())
        return nullptr;

    for (MailLevelRewardList::const_iterator set_itr = map_itr->second.begin(); set_itr != map_itr->second.end(); ++
         set_itr)
        if (set_itr->raceMask & raceMask)
            return &*set_itr;

    return nullptr;
}

CellObjectGuids const* ObjectMgr::GetCellObjectGuids(uint16 mapid, uint8 spawnMode, uint32 cell_id) const
{
    if (mapid >= _mapObjectGuidsStore.size())
        return nullptr;

    if (spawnMode >= _mapObjectGuidsStore[mapid].size())
        return nullptr;

    if (_mapObjectGuidsStore[mapid][spawnMode].empty())
        return nullptr;

    return Trinity::Containers::MapGetValuePtr(_mapObjectGuidsStore[mapid][spawnMode], cell_id);
}

CellObjectGuidsMap const* ObjectMgr::GetMapObjectGuids(uint16 mapid, uint8 spawnMode) const
{
    if (mapid >= _mapObjectGuidsStore.size())
        return nullptr;

    if (spawnMode >= _mapObjectGuidsStore[mapid].size())
        return nullptr;

    return &_mapObjectGuidsStore[mapid][spawnMode];
}

uint32 ObjectMgr::GenerateAuctionID()
{
    if (_auctionId >= std::numeric_limits<uint32>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Auctions ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return _auctionId++;
}

uint64 ObjectMgr::GenerateEquipmentSetGuid()
{
    if (_equipmentSetGuid >= std::numeric_limits<uint64>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "EquipmentSetInfo guid overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return _equipmentSetGuid++;
}

uint32 ObjectMgr::GenerateMailID()
{
    if (_mailId >= std::numeric_limits<uint32>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Mail ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return _mailId++;
}

void ObjectMgr::LoadGameObjectLocales()
{
    uint32 oldMSTime = getMSTime();

    _gameObjectLocaleStore.clear();                           // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, "
        "name_loc1, name_loc2, name_loc3, name_loc4, name_loc5, name_loc6, name_loc7, name_loc8, name_loc9, name_loc10, "
        "castbarcaption_loc1, castbarcaption_loc2, castbarcaption_loc3, castbarcaption_loc4, "
        "castbarcaption_loc5, castbarcaption_loc6, castbarcaption_loc7, castbarcaption_loc8, castbarcaption_loc9, castbarcaption_loc10 FROM locales_gameobject");

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        GameObjectLocale& data = _gameObjectLocaleStore[entry];

        for (uint8 i = 1; i < TOTAL_LOCALES; ++i)
            AddLocaleString(fields[i].GetString(), LocaleConstant(i), data.Name);

        for (uint8 i = 1; i < TOTAL_LOCALES; ++i)
            AddLocaleString(fields[i + (TOTAL_LOCALES - 1)].GetString(), LocaleConstant(i), data.CastBarCaption);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %lu gameobject locale strings in %u ms", static_cast<unsigned long>(_gameObjectLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

inline void CheckGOLockId(GameObjectTemplate const* goInfo, uint32 dataN, uint32 N)
{
    if (sLockStore.LookupEntry(dataN))
        return;

    TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u GoType: %u) have data%d=%u but lock (Id: %u) not found.",
        goInfo->entry, goInfo->type, N, goInfo->GetLockId(), goInfo->GetLockId());
}

inline void CheckGOLinkedTrapId(GameObjectTemplate const* goInfo, uint32 dataN, uint32 N)
{
    if (GameObjectTemplate const* trapInfo = sObjectMgr->GetGameObjectTemplate(dataN))
    {
        if (trapInfo->type != GAMEOBJECT_TYPE_TRAP)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u GoType: %u) have data%d=%u but GO (Entry %u) have not GAMEOBJECT_TYPE_TRAP (%u) type.",
            goInfo->entry, goInfo->type, N, dataN, dataN, GAMEOBJECT_TYPE_TRAP);
    }
}

inline void CheckGOSpellId(GameObjectTemplate const* goInfo, uint32 dataN, uint32 N)
{
    if (sSpellMgr->GetSpellInfo(dataN) || !dataN)
        return;

    TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u GoType: %u) have data%d=%u but Spell (Entry %u) not exist.",
        goInfo->entry, goInfo->type, N, dataN, dataN);
}

inline void CheckAndFixGOChairHeightId(GameObjectTemplate const* goInfo, uint32 const& dataN, uint32 N)
{
    if (dataN <= (UNIT_STAND_STATE_SIT_HIGH_CHAIR-UNIT_STAND_STATE_SIT_LOW_CHAIR))
        return;

    TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u GoType: %u) have data%d=%u but correct chair height in range 0..%i.",
        goInfo->entry, goInfo->type, N, dataN, UNIT_STAND_STATE_SIT_HIGH_CHAIR-UNIT_STAND_STATE_SIT_LOW_CHAIR);

    // prevent client and server unexpected work
    const_cast<uint32&>(dataN) = 0;
}

inline void CheckGONoDamageImmuneId(GameObjectTemplate* goTemplate, uint32 dataN, uint32 N)
{
    // 0/1 correct values
    if (dataN <= 1)
        return;

    TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u GoType: %u) have data%d=%u but expected boolean (0/1) noDamageImmune field value.", goTemplate->entry, goTemplate->type, N, dataN);
}

inline void CheckGOConsumable(GameObjectTemplate const* goInfo, uint32 dataN, uint32 N)
{
    // 0/1 correct values
    if (dataN <= 1)
        return;

    TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u GoType: %u) have data%d=%u but expected boolean (0/1) consumable field value.",
        goInfo->entry, goInfo->type, N, dataN);
}

void ObjectMgr::LoadGameObjectTemplate()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0      1      2        3       4             5          6      7       8     9        10         11          12
    QueryResult result = WorldDatabase.Query("SELECT entry, type, displayId, name, IconName, castBarCaption, unk1, faction, flags, size, questItem1, questItem2, questItem3, "
    //                                            13          14          15       16     17     18     19     20     21     22     23     24     25      26      27      28
                                             "questItem4, questItem5, questItem6, Data0, Data1, Data2, Data3, Data4, Data5, Data6, Data7, Data8, Data9, Data10, Data11, Data12, "
    //                                          29      30      31      32      33      34      35      36      37      38      39      40      41      42      43      44
                                             "Data13, Data14, Data15, Data16, Data17, Data18, Data19, Data20, Data21, Data22, Data23, Data24, Data25, Data26, Data27, Data28, "
    //                                          45      46      47      48       49       50        51          52              53                 54                55                  56                  57
                                             "Data29, Data30, Data31, Data32, unkInt32, AIName, ScriptName, WorldEffectID, SpellVisualID, SpellStateVisualID, SpellStateAnimID, SpellStateAnimKitID, StateWorldEffectID, "
    //                                          58             59          60      61
                                             "MaxVisible, IgnoreDynLos, MinGold, MaxGold FROM gameobject_template");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 gameobject definitions. DB table `gameobject_template` is empty.");
        return;
    }

    std::list<uint32> tempList = sDB2Manager.GetGameObjectsList();

    _gameObjectTemplateStore.rehash(result->GetRowCount() + tempList.size());

    for (uint32 const& itr : tempList)
    {
        if (GameObjectsEntry const* goe = sGameObjectsStore.LookupEntry(itr))
        {
            GameObjectTemplate& got = _gameObjectTemplateStore[goe->ID];

            got.entry          = goe->ID;
            got.type           = goe->OwnerID;
            got.displayId      = goe->DisplayID;
            got.name           = goe->Name->Str[DEFAULT_LOCALE];
            got.IconName       = "";
            got.castBarCaption = "";
            got.unk1           = "";
            got.faction        = 0;
            got.flags          = 0;
            got.size           = goe->Scale;

            for (uint8 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
                got.QuestItems[i] = 0;

            for (uint8 i = 0; i < GO_DBC_DATA_COUNT/*MAX_GAMEOBJECT_DATA*/; ++i)
                got.raw.data[i] = goe->PropValue[i];

            got.RequiredLevel = 0;
            got.AIName = "";
            got.ScriptId = GetScriptId("");

            //++count;
        }
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        GameObjectTemplate& got = _gameObjectTemplateStore[entry];
        got.entry          = entry;
        got.type           = uint32(fields[1].GetUInt8());
        got.displayId      = fields[2].GetUInt32();
        got.name           = fields[3].GetString();
        got.IconName       = fields[4].GetString();
        got.castBarCaption = fields[5].GetString();
        got.unk1           = fields[6].GetString();
        got.faction        = uint32(fields[7].GetUInt16());
        got.flags          = fields[8].GetUInt32();
        got.size           = fields[9].GetFloat();

        for (uint8 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
            got.QuestItems[i] = fields[10 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_GAMEOBJECT_DATA; ++i)
            got.raw.data[i] = fields[16 + i].GetInt32();

        got.RequiredLevel = fields[49].GetInt32();
        got.AIName = fields[50].GetString();
        got.ScriptId = GetScriptId(fields[51].GetCString());
        got.visualQuestID = 0;
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].WorldEffectID = fields[52].GetInt32();
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellVisualID = fields[53].GetInt32();
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateVisualID = fields[54].GetInt32();
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateAnimID = fields[55].GetInt32();
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateAnimKitID = fields[56].GetInt32();
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID = fields[57].GetInt32();

        if (got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].WorldEffectID && !sWorldEffectStore[got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].WorldEffectID])
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "GameObject ID: %u has invalid WorldEffectID (%u) in `gameobject_addon`, set to 0.", entry, got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].WorldEffectID);
            got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].WorldEffectID = 0;
        }

        if (got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID && !sWorldEffectStore[got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID])
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "GameObject ID: %u has invalid WorldEffectID (%u) in `gameobject_addon`, set to 0.", entry, got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID);
            got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID = 0;
        }

        got.MaxVisible = fields[58].GetBool();
        got.IgnoreDynLos = fields[59].GetBool();
        got.MinGold = fields[60].GetUInt32();
        got.MaxGold = fields[61].GetUInt32();

        // Checks

        switch (got.type)
        {
            case GAMEOBJECT_TYPE_DOOR:                      //0
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 1);
                CheckGONoDamageImmuneId(&got, got.door.noDamageImmune, 3);
                break;
            case GAMEOBJECT_TYPE_BUTTON:                    //1
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 1);
                CheckGONoDamageImmuneId(&got, got.button.noDamageImmune, 4);
                break;
            case GAMEOBJECT_TYPE_QUESTGIVER:                //2
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);
                CheckGONoDamageImmuneId(&got, got.questgiver.noDamageImmune, 5);
                break;
            case GAMEOBJECT_TYPE_CHEST:                     //3
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);

                CheckGOConsumable(&got, got.chest.consumable, 3);

                if (got.chest.linkedTrap)              // linked trap
                    CheckGOLinkedTrapId(&got, got.chest.linkedTrap, 7);
                break;
            case GAMEOBJECT_TYPE_TRAP:                      //6
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);
                break;
            case GAMEOBJECT_TYPE_CHAIR:                     //7
                CheckAndFixGOChairHeightId(&got, got.chair.chairheight, 1);
                break;
            case GAMEOBJECT_TYPE_SPELL_FOCUS:               //8
                if (got.spellFocus.spellFocusType)
                    if (!sSpellFocusObjectStore.LookupEntry(got.spellFocus.spellFocusType))
                        TC_LOG_ERROR(LOG_FILTER_SQL, "GameObject (Entry: %u GoType: %u) have data0=%u but SpellFocus (Id: %u) not exist.",
                        entry, got.type, got.spellFocus.spellFocusType, got.spellFocus.spellFocusType);

                if (got.spellFocus.linkedTrap)        // linked trap
                    CheckGOLinkedTrapId(&got, got.spellFocus.linkedTrap, 2);
                break;
            case GAMEOBJECT_TYPE_GOOBER:                    //10
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);

                CheckGOConsumable(&got, got.goober.consumable, 3);

                if (got.goober.pageID)                  // pageId
                    if (!GetPageText(got.goober.pageID))
                        TC_LOG_ERROR(LOG_FILTER_SQL, "GameObject (Entry: %u GoType: %u) have data7=%u but PageText (Entry %u) not exist.",
                        entry, got.type, got.goober.pageID, got.goober.pageID);

                CheckGONoDamageImmuneId(&got, got.goober.noDamageImmune, 11);
                if (got.goober.linkedTrap)            // linked trap
                    CheckGOLinkedTrapId(&got, got.goober.linkedTrap, 12);
                break;
            case GAMEOBJECT_TYPE_AREADAMAGE:                //12
            case GAMEOBJECT_TYPE_CAMERA:                    //13
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);
                break;
            case GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT:              //15
                if (got.moTransport.taxiPathID)
                {
                    if (got.moTransport.taxiPathID >= sTaxiPathNodesByPath.size() || sTaxiPathNodesByPath[got.moTransport.taxiPathID].empty())
                        TC_LOG_ERROR(LOG_FILTER_SQL, "GameObject (Entry: %u GoType: %u) have data0=%u but TaxiPath (Id: %u) not exist.",
                        entry, got.type, got.moTransport.taxiPathID, got.moTransport.taxiPathID);
                }
                if (uint32 transportMap = got.moTransport.SpawnMap)
                    _transportMaps.insert(transportMap);
                break;
            case GAMEOBJECT_TYPE_RITUAL:          //18
                break;
            case GAMEOBJECT_TYPE_SPELLCASTER:               //22
                CheckGOSpellId(&got, got.spellCaster.spell, 0);
                break;
            case GAMEOBJECT_TYPE_FLAGSTAND:                 //24
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);
                CheckGONoDamageImmuneId(&got, got.flagStand.noDamageImmune, 5);
                break;
            case GAMEOBJECT_TYPE_FISHINGHOLE:               //25
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 4);
                break;
            case GAMEOBJECT_TYPE_FLAGDROP:                  //26
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);
                CheckGONoDamageImmuneId(&got, got.flagDrop.noDamageImmune, 3);
                break;
            case GAMEOBJECT_TYPE_BARBER_CHAIR:              //32
                CheckAndFixGOChairHeightId(&got, got.barberChair.chairheight, 0);
                break;
            case GAMEOBJECT_TYPE_NEW_FLAG:                  //36
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);
                break;
            case GAMEOBJECT_TYPE_GARRISON_BUILDING:
                if (uint32 transportMap = got.garrisonBuilding.SpawnMap)
                   _transportMaps.insert(transportMap);
                break;
            case GAMEOBJECT_TYPE_GATHERING_NODE:            //50
                if (got.GetLockId())
                    CheckGOLockId(&got, got.GetLockId(), 0);
                if (got.gatheringNode.linkedTrap)
                    CheckGOLinkedTrapId(&got, got.gatheringNode.linkedTrap, 20);
                break;
            default:
                break;
        }

       ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u game object templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadGameObjectQuestVisual()
{
    uint32 oldMSTime = getMSTime();

    //                                                  0       1           2                                   3                               4                               5
    QueryResult result = WorldDatabase.Query("SELECT `goID`, `questID`, `incomplete_state_spell_visual`, `incomplete_state_world_effect`, `complete_state_spell_visual`, `complete_state_world_effect` FROM gameobject_quest_visual");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 gameobject quest visual. DB table `gameobject_quest_visual` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 goID = fields[0].GetUInt32();
        uint32 questID = fields[1].GetUInt32();
        if (!GetGameObjectTemplate(goID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u) not exist on table `gameobject_quest_visual`", goID);
            continue;
        }
        if (questID && !sQuestDataStore->GetQuestTemplate(questID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Quest (ID: %u) not exist on table `gameobject_quest_visual`", questID);
            continue;
        }
        GameObjectTemplate& got = _gameObjectTemplateStore[goID];
        got.visualQuestID = questID;
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateVisualID = fields[2].GetUInt32();
        got.visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID = fields[3].GetUInt32();
        got.visualData[GO_VISUAL_AFTER_COMPLETEQUEST].SpellStateVisualID = fields[4].GetUInt32();
        got.visualData[GO_VISUAL_AFTER_COMPLETEQUEST].StateWorldEffectID = fields[5].GetUInt32();
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u game object quest visual in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadExplorationBaseXP()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT level, basexp FROM exploration_basexp");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 BaseXP definitions. DB table `exploration_basexp` is empty.");

        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint8 level  = fields[0].GetUInt8();
        uint32 basexp = fields[1].GetInt32();
        _baseXPTable[level] = basexp;
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u BaseXP definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

uint32 ObjectMgr::GetBaseXP(uint8 level)
{
    return _baseXPTable[level] ? _baseXPTable[level] : 0;
}

uint32 ObjectMgr::GetXPForLevel(uint8 level) const
{
    if (level < _playerXPperLevel.size())
        return _playerXPperLevel[level];
    return 0;
}

int32 ObjectMgr::GetFishingBaseSkillLevel(uint32 entry) const
{
    FishingBaseSkillContainer::const_iterator itr = _fishingBaseForAreaStore.find(entry);
    return itr != _fishingBaseForAreaStore.end() ? itr->second : 0;
}

SkillTiersEntry const* ObjectMgr::GetSkillTier(uint32 skillTierId) const
{
    return Trinity::Containers::MapGetValuePtr(_skillTiers, skillTierId);
}

void ObjectMgr::LoadPetNames()
{
    uint32 oldMSTime = getMSTime();
    //                                                0     1      2
    QueryResult result = WorldDatabase.Query("SELECT word, entry, half FROM pet_name_generation");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 pet name parts. DB table `pet_name_generation` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        std::string word = fields[0].GetString();
        uint32 entry     = fields[1].GetUInt32();
        bool   half      = fields[2].GetBool();
        if (half)
            _petHalfName1[entry].push_back(word);
        else
            _petHalfName0[entry].push_back(word);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u pet name parts in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadPetNumber()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT MAX(id) FROM character_pet");
    if (result)
    {
        Field* fields = result->Fetch();
        _hiPetNumber = fields[0].GetUInt32()+1;
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded the max pet number: %d in %u ms", _hiPetNumber-1, GetMSTimeDiffToNow(oldMSTime));
}

std::string ObjectMgr::GeneratePetName(uint32 entry)
{
    StringVector & list0 = _petHalfName0[entry];
    StringVector & list1 = _petHalfName1[entry];

    if (list0.empty() || list1.empty())
    {
        CreatureTemplate const* cinfo = GetCreatureTemplate(entry);
        const char* petname = sDB2Manager.GetPetName(cinfo->Family, sWorld->GetDefaultDbcLocale());
        if (!petname)
            return cinfo->Name[0];

        return std::string(petname);
    }

    return *(list0.begin()+urand(0, list0.size()-1)) + *(list1.begin()+urand(0, list1.size()-1));
}

uint32 ObjectMgr::GeneratePetNumber()
{
    return ++_hiPetNumber;
}

uint64 ObjectMgr::GenerateVoidStorageItemId()
{
    return ++_voidItemId;
}

void ObjectMgr::LoadCurrencysLoot()
{
    //                                                  0       1       2           3                   4               5       6
    QueryResult result = WorldDatabase.PQuery("SELECT entry, type, currencyId, currencyAmount, currencyMaxAmount, lootmode, chance FROM currency_loot");
    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* field = result->Fetch();

        uint32 entry = field[0].GetUInt32();
        uint8 type = field[1].GetInt8();
        uint32 currencyId = field[2].GetUInt32();
        uint32 currencyAmount = field[3].GetUInt32();
        uint32 currencyMaxAmount = field[4].GetUInt32();
        uint32 lootmode = field[5].GetUInt32();
        float chance = field[6].GetFloat();

        if (type < 1)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Currency 'type' can not be < 1 (entry = %u type = %i)", entry, type);
            continue;
        }
        if (type > 3)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Currency 'type' can not be > 3 (entry = %u type = %i)", entry, type);
            continue;
        }

        if (chance > 100.0f || !chance)
            chance = 100.0f;

        CurrencyLoot loot = CurrencyLoot(entry, type, currencyId, currencyAmount, currencyMaxAmount, lootmode, chance);
        _currencysLoot.push_back(loot);
        ++count;
    }
    while (result->NextRow());

    if (count)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loaded %u currency loot definition", count);
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loaded 0 currency loot definition. Table is empty!");
}

void ObjectMgr::LoadCorpses()
{
    //        0     1     2     3            4      5          6          7       8       9      10        11    12          13          14          15         16
    // SELECT posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, flags, dynFlags, time, corpseType, instanceId, phaseMask, corpseGuid, guid FROM corpse WHERE corpseType <> 0

    uint32 oldMSTime = getMSTime();

    PreparedQueryResult result = CharacterDatabase.Query(CharacterDatabase.GetPreparedStatement(CHAR_SEL_CORPSES));
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 corpses. DB table `corpse` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        ObjectGuid::LowType guid = fields[16].GetUInt64();
        CorpseType type = CorpseType(fields[12].GetUInt8());
        if (type >= MAX_CORPSE_TYPE)
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Corpse (guid: %u) have wrong corpse type (%u), not loading.", guid, type);
            continue;
        }

        Corpse* corpse = new Corpse(type);
        if (!corpse->LoadCorpseFromDB(guid, fields))
        {
            delete corpse;
            continue;
        }

        sObjectAccessor->AddCorpse(corpse);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u corpses in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadReputationRewardRate()
{
    uint32 oldMSTime = getMSTime();

    _repRewardRateStore.clear();                             // for reload case

    uint32 count = 0; //                                0          1            2             3
    QueryResult result = WorldDatabase.Query("SELECT faction, quest_rate, creature_rate, spell_rate FROM reputation_reward_rate");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, ">> Loaded `reputation_reward_rate`, table is empty!");

        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 factionId        = fields[0].GetUInt32();

        RepRewardRate repRate;

        repRate.quest_rate      = fields[1].GetFloat();
        repRate.creature_rate   = fields[2].GetFloat();
        repRate.spell_rate      = fields[3].GetFloat();

        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionId);
        if (!factionEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `reputation_reward_rate`", factionId);
            continue;
        }

        if (repRate.quest_rate < 0.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table reputation_reward_rate has quest_rate with invalid rate %f, skipping data for faction %u", repRate.quest_rate, factionId);
            continue;
        }

        if (repRate.creature_rate < 0.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table reputation_reward_rate has creature_rate with invalid rate %f, skipping data for faction %u", repRate.creature_rate, factionId);
            continue;
        }

        if (repRate.spell_rate < 0.0f)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table reputation_reward_rate has spell_rate with invalid rate %f, skipping data for faction %u", repRate.spell_rate, factionId);
            continue;
        }

        _repRewardRateStore[factionId] = repRate;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u reputation_reward_rate in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadReputationOnKill()
{
    uint32 oldMSTime = getMSTime();

    // For reload case
    _repOnKillStore.clear();

    uint32 count = 0;

    //                                                    0            1          2          3
    QueryResult result = WorldDatabase.Query("SELECT creature_id, RewFaction, RewValue, MaxStanding FROM creature_onkill_reputation");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature award reputation definitions. DB table `creature_onkill_reputation` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 creature_id = fields[0].GetUInt32();

        ReputationOnKillEntry repOnKill;
        repOnKill.RepFaction          = fields[1].GetInt16();
        repOnKill.RepValue            = fields[2].GetInt32();
        repOnKill.ReputationCap       = fields[3].GetUInt8();

        if (!GetCreatureTemplate(creature_id))
        {
            WorldDatabase.PExecute("DELETE FROM creature_onkill_reputation WHERE creature_id = %u", creature_id);
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_onkill_reputation` have data for not existed creature entry (%u), skipped", creature_id);
            continue;
        }

        if (repOnKill.RepFaction)
        {
            FactionEntry const* factionEntry1 = sFactionStore.LookupEntry(repOnKill.RepFaction);
            if (!factionEntry1)
            {
                WorldDatabase.PExecute("DELETE FROM creature_onkill_reputation WHERE creature_id = %u and RewFaction = %u", creature_id, repOnKill.RepFaction);
                TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `creature_onkill_reputation`", repOnKill.RepFaction);
                continue;
            }
        }

        _repOnKillStore[creature_id].push_back(repOnKill);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature award reputation definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadReputationSpilloverTemplate()
{
    uint32 oldMSTime = getMSTime();

    _repSpilloverTemplateStore.clear();                      // for reload case

    uint32 count = 0; //                                0         1        2       3        4       5       6         7        8      9        10       11     12        13       14     15
    QueryResult result = WorldDatabase.Query("SELECT faction, faction1, rate_1, rank_1, faction2, rate_2, rank_2, faction3, rate_3, rank_3, faction4, rate_4, rank_4, faction5, rate_5, rank_5 FROM reputation_spillover_template");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded `reputation_spillover_template`, table is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 factionId                = fields[0].GetUInt16();

        RepSpilloverTemplate repTemplate;

        repTemplate.faction[0]          = fields[1].GetUInt16();
        repTemplate.faction_rate[0]     = fields[2].GetFloat();
        repTemplate.faction_rank[0]     = fields[3].GetUInt8();
        repTemplate.faction[1]          = fields[4].GetUInt16();
        repTemplate.faction_rate[1]     = fields[5].GetFloat();
        repTemplate.faction_rank[1]     = fields[6].GetUInt8();
        repTemplate.faction[2]          = fields[7].GetUInt16();
        repTemplate.faction_rate[2]     = fields[8].GetFloat();
        repTemplate.faction_rank[2]     = fields[9].GetUInt8();
        repTemplate.faction[3]          = fields[10].GetUInt16();
        repTemplate.faction_rate[3]     = fields[11].GetFloat();
        repTemplate.faction_rank[3]     = fields[12].GetUInt8();
        repTemplate.faction[4]          = fields[13].GetUInt16();
        repTemplate.faction_rate[4]     = fields[14].GetFloat();
        repTemplate.faction_rank[4]     = fields[15].GetUInt8();

        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionId);

        if (!factionEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `reputation_spillover_template`", factionId);
            continue;
        }

        if (factionEntry->ParentFactionID == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u in `reputation_spillover_template` does not belong to any team, skipping", factionId);
            continue;
        }

        for (uint32 i = 0; i < MAX_SPILLOVER_FACTIONS; ++i)
        {
            if (repTemplate.faction[i])
            {
                FactionEntry const* factionSpillover = sFactionStore.LookupEntry(repTemplate.faction[i]);

                if (!factionSpillover)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Spillover faction (faction.dbc) %u does not exist but is used in `reputation_spillover_template` for faction %u, skipping", repTemplate.faction[i], factionId);
                    continue;
                }

                if (factionSpillover->ReputationIndex < 0)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Spillover faction (faction.dbc) %u for faction %u in `reputation_spillover_template` can not be listed for client, and then useless, skipping", repTemplate.faction[i], factionId);
                    continue;
                }

                if (repTemplate.faction_rank[i] >= MAX_REPUTATION_RANK)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Rank %u used in `reputation_spillover_template` for spillover faction %u is not valid, skipping", repTemplate.faction_rank[i], repTemplate.faction[i]);
                    continue;
                }
            }
        }

        FactionEntry const* factionEntry0 = sFactionStore.LookupEntry(repTemplate.faction[0]);
        if (repTemplate.faction[0] && !factionEntry0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `reputation_spillover_template`", repTemplate.faction[0]);
            continue;
        }
        FactionEntry const* factionEntry1 = sFactionStore.LookupEntry(repTemplate.faction[1]);
        if (repTemplate.faction[1] && !factionEntry1)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `reputation_spillover_template`", repTemplate.faction[1]);
            continue;
        }
        FactionEntry const* factionEntry2 = sFactionStore.LookupEntry(repTemplate.faction[2]);
        if (repTemplate.faction[2] && !factionEntry2)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `reputation_spillover_template`", repTemplate.faction[2]);
            continue;
        }
        FactionEntry const* factionEntry3 = sFactionStore.LookupEntry(repTemplate.faction[3]);
        if (repTemplate.faction[3] && !factionEntry3)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `reputation_spillover_template`", repTemplate.faction[3]);
            continue;
        }
        FactionEntry const* factionEntry4 = sFactionStore.LookupEntry(repTemplate.faction[4]);
        if (repTemplate.faction[4] && !factionEntry4)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Faction (faction.dbc) %u does not exist but is used in `reputation_spillover_template`", repTemplate.faction[4]);
            continue;
        }

        _repSpilloverTemplateStore[factionId] = repTemplate;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u reputation_spillover_template in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
void ObjectMgr::LoadScenarioPOI()
{
    uint32 oldMSTime = getMSTime();

    _scenarioPOIStore.clear();                          // need for reload case

    uint32 count = 0;
    uint32 criteriaTreeIdMax = 0;

    //                                                      0           1      2          3            4         5       6          7                8
    QueryResult result = WorldDatabase.Query("SELECT criteriaTreeId, BlobID, MapID, WorldMapAreaID, `Floor`, Priority, Flags, WorldEffectID, PlayerConditionID FROM scenario_poi order by criteriaTreeId");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 scenario POI definitions. DB table `scenario_poi` is empty.");
        return;
    }

    //                                               0               1   2  3
    QueryResult points = WorldDatabase.Query("SELECT criteriaTreeId, id, x, y FROM scenario_poi_points ORDER BY criteriaTreeId DESC, idx");

    std::vector<std::vector<std::vector<ScenarioPOIPoint> > > POIs;

    if (points)
    {
        // The first result should have the highest questId
        Field* fields = points->Fetch();
        criteriaTreeIdMax = fields[0].GetUInt32();
        POIs.resize(criteriaTreeIdMax + 1);

        do
        {
            fields = points->Fetch();

            uint32 criteriaTreeId     = fields[0].GetUInt32();
            uint32 id                 = fields[1].GetUInt32();
            int32  x                  = fields[2].GetInt32();
            int32  y                  = fields[3].GetInt32();

            if (POIs[criteriaTreeId].size() <= id + 1)
                POIs[criteriaTreeId].resize(id + 10);

            ScenarioPOIPoint point(x, y);
            POIs[criteriaTreeId][id].push_back(point);
        } while (points->NextRow());
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 criteriaTreeId     = fields[0].GetUInt32();
        uint32 BlobID             = fields[1].GetUInt32();
        uint32 MapID              = fields[2].GetUInt32();
        uint32 WorldMapAreaID     = fields[3].GetUInt32();
        uint32 Floor              = fields[4].GetUInt32();
        uint32 Priority           = fields[5].GetUInt32();
        uint32 Flags              = fields[6].GetUInt32();
        uint32 WorldEffectID      = fields[7].GetUInt32();
        uint32 PlayerConditionID  = fields[8].GetUInt32();

        ScenarioPOI POI(BlobID, MapID, WorldMapAreaID, Floor, Priority, Flags, WorldEffectID, PlayerConditionID);

        if(criteriaTreeId <= criteriaTreeIdMax && !POIs[criteriaTreeId].empty())
            POI.points = POIs[criteriaTreeId][BlobID];

        _scenarioPOIStore[criteriaTreeId].push_back(POI);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u scenario POI definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadNPCSpellClickSpells()
{
    uint32 oldMSTime = getMSTime();

    _spellClickInfoStore.clear();
    //                                                0          1         2            3           4
    QueryResult result = WorldDatabase.Query("SELECT npc_entry, spell_id, cast_flags, user_type, add_npc_flag FROM npc_spellclick_spells");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spellclick spells. DB table `npc_spellclick_spells` is empty.");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 npc_entry = fields[0].GetUInt32();
        CreatureTemplate const* cInfo = GetCreatureTemplate(npc_entry);
        if (!cInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table npc_spellclick_spells references unknown creature_template %u. Skipping entry.", npc_entry);
            continue;
        }

        uint32 spellid = fields[1].GetUInt32();
        SpellInfo const* spellinfo = sSpellMgr->GetSpellInfo(spellid);
        if (!spellinfo)
        {
            //TC_LOG_ERROR(LOG_FILTER_SQL, "Table npc_spellclick_spells references unknown spellid %u. Skipping entry.", spellid);
            WorldDatabase.PExecute("DELETE FROM npc_spellclick_spells WHERE spell_id = %u", spellid);
            continue;
        }

        uint8 userType = fields[3].GetUInt16();
        if (userType >= SPELL_CLICK_USER_MAX)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table npc_spellclick_spells references unknown user type %u. Skipping entry.", uint32(userType));

        uint8 castFlags = fields[2].GetUInt8();
        SpellClickInfo info;
        info.spellId = spellid;
        info.castFlags = castFlags;
        info.userType = SpellClickUserTypes(userType);
        _spellClickInfoStore.insert(std::make_pair(npc_entry, info));

        bool addFlag = fields[4].GetBool();

        if (addFlag)
            if (auto data = _creatureTemplateStore[npc_entry])
                data->npcflag |= UNIT_NPC_FLAG_SPELLCLICK;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spellclick definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::DeleteCreatureData(ObjectGuid::LowType const& guid)
{
    // remove mapid*cellid -> guid_set map
    CreatureData const* data = GetCreatureData(guid);
    if (data)
        RemoveCreatureFromGrid(guid, data);

    _creatureDataStore.erase(guid);
}

ObjectGuid ObjectMgr::GetLinkedRespawnGuid(ObjectGuid const& guid) const
{
    LinkedRespawnContainer::const_iterator itr = _linkedRespawnStore.find(guid);
    if (itr == _linkedRespawnStore.end()) return ObjectGuid::Empty;
    return itr->second;
}

void ObjectMgr::DeleteGOData(ObjectGuid::LowType const& guid)
{
    // remove mapid*cellid -> guid_set map
    GameObjectData const* data = GetGOData(guid);
    if (data)
        RemoveGameobjectFromGrid(guid, data);

    _gameObjectDataStore.erase(guid);
}

void ObjectMgr::AddCorpseCellData(uint32 mapid, uint32 cellid, ObjectGuid player_guid, uint32 instance)
{
    if (mapid >= _mapObjectGuidsStore.size())
        _mapObjectGuidsStore.resize(mapid + 1);

    if (_mapObjectGuidsStore[mapid].empty())
        _mapObjectGuidsStore[mapid].resize(1);

    // corpses are always added to spawn mode 0 and they are spawned by their instance id
    CellObjectGuids& cell_guids = _mapObjectGuidsStore[mapid][0][cellid];
    cell_guids.corpses[player_guid] = instance;
}

void ObjectMgr::DeleteCorpseCellData(uint32 mapid, uint32 cellid, ObjectGuid player_guid)
{
    if (mapid >= _mapObjectGuidsStore.size())
        _mapObjectGuidsStore.resize(mapid + 1);

    if (_mapObjectGuidsStore[mapid].empty())
        _mapObjectGuidsStore[mapid].resize(1);

    // corpses are always added to spawn mode 0 and they are spawned by their instance id
    CellObjectGuids& cell_guids = _mapObjectGuidsStore[mapid][0][cellid];
    cell_guids.corpses.erase(player_guid);
}

bool ObjectMgr::LoadTrinityStrings(const char* table, int32 min_value, int32 max_value)
{
    uint32 oldMSTime = getMSTime();

    int32 start_value = min_value;
    int32 end_value   = max_value;
    // some string can have negative indexes range
    if (start_value < 0)
    {
        if (end_value >= start_value)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' attempt loaded with invalid range (%d - %d), strings not loaded.", table, min_value, max_value);
            return false;
        }

        // real range (max+1, min+1) exaple: (-10, -1000) -> -999...-10+1
        std::swap(start_value, end_value);
        ++start_value;
        ++end_value;
    }
    else
    {
        if (start_value >= end_value)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' attempt loaded with invalid range (%d - %d), strings not loaded.", table, min_value, max_value);
            return false;
        }
    }

    // cleanup affected map part for reloading case
    for (TrinityStringLocaleContainer::iterator itr = _trinityStringLocaleStore.begin(); itr != _trinityStringLocaleStore.end();)
    {
        if (itr->first >= start_value && itr->first < end_value)
            _trinityStringLocaleStore.erase(itr++);
        else
            ++itr;
    }

    QueryResult result = WorldDatabase.PQuery("SELECT entry, content_default, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8, content_loc9, content_loc10 FROM %s", table);

    if (!result)
    {
        if (min_value == MIN_TRINITY_STRING_ID)              // error only in case internal strings
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">>  Loaded 0 trinity strings. DB table `%s` is empty. Cannot continue.", table);
        else
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 string templates. DB table `%s` is empty.", table);

        return false;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        int32 entry = fields[0].GetInt32();
        if (entry == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` contain reserved entry 0, ignored.", table);
            continue;
        }

        if (entry < start_value || entry >= end_value)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` contain entry %i out of allowed range (%d - %d), ignored.", table, entry, min_value, max_value);
            continue;
        }

        TrinityStringLocale& data = _trinityStringLocaleStore[entry];
        if (!data.Content.empty())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` contain data for already loaded entry  %i (from another table?), ignored.", table, entry);
            continue;
        }

        data.Content.resize(1);
        ++count;

        for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
            AddLocaleString(fields[i + 1].GetString(), LocaleConstant(i), data.Content);
    } while (result->NextRow());

    if (min_value == MIN_TRINITY_STRING_ID)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Trinity strings from table %s in %u ms", count, table, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u string templates from %s in %u ms", count, table, GetMSTimeDiffToNow(oldMSTime));

    return true;
}

bool ObjectMgr::LoadTrinityStrings()
{
    return LoadTrinityStrings("trinity_string", MIN_TRINITY_STRING_ID, MAX_TRINITY_STRING_ID);
}

const char *ObjectMgr::GetTrinityString(int32 entry, LocaleConstant locale_idx) const
{
    if (TrinityStringLocale const* msl = GetTrinityStringLocale(entry))
    {
        if (msl->Content.size() > size_t(locale_idx) && !msl->Content[locale_idx].empty())
            return msl->Content[locale_idx].c_str();

        return msl->Content[DEFAULT_LOCALE].c_str();
    }

    if (entry > 0)
        TC_LOG_ERROR(LOG_FILTER_SQL, "Entry %i not found in `trinity_string` table.", entry);
    else
        TC_LOG_ERROR(LOG_FILTER_SQL, "Trinity string entry %i not found in DB.", entry);
    return "<error>";
}

void ObjectMgr::LoadFishingBaseSkillLevel()
{
    uint32 oldMSTime = getMSTime();

    _fishingBaseForAreaStore.clear();                            // for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, skill FROM skill_fishing_base_level");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 areas for fishing base skill level. DB table `skill_fishing_base_level` is empty.");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry  = fields[0].GetUInt32();
        int32 skill   = fields[1].GetInt16();

        if (!sAreaTableStore.LookupEntry(entry))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaId %u defined in `skill_fishing_base_level` does not exist", entry);
            continue;
        }

        _fishingBaseForAreaStore[entry] = skill;
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u areas for fishing base skill level in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

SkillRangeType GetSkillRangeType(SkillRaceClassInfoEntry const* rcEntry)
{
    SkillLineEntry const* skill = sSkillLineStore.LookupEntry(rcEntry->SkillID);
    if (!skill)
        return SKILL_RANGE_NONE;

    if (sObjectMgr->GetSkillTier(rcEntry->SkillTierID))
        return SKILL_RANGE_RANK;

    if (rcEntry->SkillID == SKILL_RUNEFORGING || rcEntry->SkillID == SKILL_RUNEFORGING_2)
        return SKILL_RANGE_MONO;

    switch (skill->CategoryID)
    {
        case SKILL_CATEGORY_ARMOR:
            return SKILL_RANGE_MONO;
        case SKILL_CATEGORY_LANGUAGES:
            return SKILL_RANGE_LANGUAGE;
        default:
            break;
    }

    return SKILL_RANGE_LEVEL;
}

void ObjectMgr::LoadGameTele()
{
    uint32 oldMSTime = getMSTime();

    _gameTeleStore.clear();                                  // for reload case

    //                                                0       1           2           3           4        5     6
    QueryResult result = WorldDatabase.Query("SELECT id, position_x, position_y, position_z, orientation, map, name FROM game_tele");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">>  Loaded 0 GameTeleports. DB table `game_tele` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 id         = fields[0].GetUInt32();

        GameTele gt;

        gt.position_x     = fields[1].GetFloat();
        gt.position_y     = fields[2].GetFloat();
        gt.position_z     = fields[3].GetFloat();
        gt.orientation    = fields[4].GetFloat();
        gt.mapId          = fields[5].GetUInt16();
        gt.name           = fields[6].GetString();

        if (!MapManager::IsValidMapCoord(gt.mapId, gt.position_x, gt.position_y, gt.position_z, gt.orientation))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong position for id %u (name: %s) in `game_tele` table, ignoring.", id, gt.name.c_str());
            continue;
        }

        if (!Utf8toWStr(gt.name, gt.wnameLow))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Wrong UTF8 name for id %u in `game_tele` table, ignoring.", id);
            continue;
        }

        wstrToLower(gt.wnameLow);

        _gameTeleStore[id] = gt;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u GameTeleports in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

GameTele const* ObjectMgr::GetGameTele(std::string const& name) const
{
    // explicit name case
    std::wstring wname;
    if (!Utf8toWStr(name, wname))
        return nullptr;

    // converting string that we try to find to lower case
    wstrToLower(wname);

    // Alternative first GameTele what contains wnameLow as substring in case no GameTele location found
    const GameTele* alt = nullptr;
    for (GameTeleContainer::const_iterator itr = _gameTeleStore.begin(); itr != _gameTeleStore.end(); ++itr)
    {
        if (itr->second.wnameLow == wname)
            return &itr->second;
        if (alt == nullptr && itr->second.wnameLow.find(wname) != std::wstring::npos)
            alt = &itr->second;
    }

    return alt;
}

bool ObjectMgr::AddGameTele(GameTele& tele)
{
    // find max id
    uint32 new_id = 0;
    for (GameTeleContainer::const_iterator itr = _gameTeleStore.begin(); itr != _gameTeleStore.end(); ++itr)
        if (itr->first > new_id)
            new_id = itr->first;

    // use next
    ++new_id;

    if (!Utf8toWStr(tele.name, tele.wnameLow))
        return false;

    wstrToLower(tele.wnameLow);

    _gameTeleStore[new_id] = tele;

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_GAME_TELE);

    stmt->setUInt32(0, new_id);
    stmt->setFloat(1, tele.position_x);
    stmt->setFloat(2, tele.position_y);
    stmt->setFloat(3, tele.position_z);
    stmt->setFloat(4, tele.orientation);
    stmt->setUInt16(5, uint16(tele.mapId));
    stmt->setString(6, tele.name);

    WorldDatabase.Execute(stmt);

    return true;
}

bool ObjectMgr::DeleteGameTele(std::string const& name)
{
    // explicit name case
    std::wstring wname;
    if (!Utf8toWStr(name, wname))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wname);

    for (GameTeleContainer::iterator itr = _gameTeleStore.begin(); itr != _gameTeleStore.end(); ++itr)
    {
        if (itr->second.wnameLow == wname)
        {
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAME_TELE);

            stmt->setString(0, itr->second.name);

            WorldDatabase.Execute(stmt);

            _gameTeleStore.erase(itr);
            return true;
        }
    }

    return false;
}

bool ObjectMgr::IsActivateDonateService() const
{
    if (auto const* donvencat = GetDonateVendorCat(0))
    {
        for (auto itr = donvencat->categories.begin(); itr != donvencat->categories.end(); ++itr)
        {
            if ((*itr).action == -1)
                return true;
        }
    }

    return false;
}

void ObjectMgr::LoadMailLevelRewards()
{
    uint32 oldMSTime = getMSTime();

    _mailLevelRewardStore.clear();                           // for reload case

    //                                                 0        1             2            3
    QueryResult result = WorldDatabase.Query("SELECT level, raceMask, mailTemplateId, senderEntry FROM mail_level_reward");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">>  Loaded 0 level dependent mail rewards. DB table `mail_level_reward` is empty.");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint8 level           = fields[0].GetUInt8();
        uint32 raceMask       = fields[1].GetUInt32();
        uint32 mailTemplateId = fields[2].GetUInt32();
        uint32 senderEntry    = fields[3].GetUInt32();

        if (level > MAX_LEVEL)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `mail_level_reward` have data for level %u that more supported by client (%u), ignoring.", level, MAX_LEVEL);
            continue;
        }

        if (!(raceMask & RACEMASK_ALL_PLAYABLE))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `mail_level_reward` have raceMask (%u) for level %u that not include any player races, ignoring.", raceMask, level);
            continue;
        }

        if (!sMailTemplateStore.LookupEntry(mailTemplateId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `mail_level_reward` have invalid mailTemplateId (%u) for level %u that invalid not include any player races, ignoring.", mailTemplateId, level);
            continue;
        }

        if (!GetCreatureTemplate(senderEntry))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `mail_level_reward` have not existed sender creature entry (%u) for level %u that invalid not include any player races, ignoring.", senderEntry, level);
            continue;
        }

        _mailLevelRewardStore[level].push_back(MailLevelReward(raceMask, mailTemplateId, senderEntry));

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u level dependent mail rewards in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::AddSpellToTrainer(uint32 entry, uint32 spell, uint32 spellCost, uint32 reqSkill, uint32 reqSkillValue, uint32 reqLevel)
{
    if (entry >= TRINITY_TRAINER_START_REF)
        return;

    CreatureTemplate const* cInfo = GetCreatureTemplate(entry);
    if (!cInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `npc_trainer` contains an entry for a non-existing creature template (Entry: %u), ignoring", entry);
        return;
    }

    if (!(cInfo->npcflag & UNIT_NPC_FLAG_TRAINER))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `npc_trainer` contains an entry for a creature template (Entry: %u) without trainer flag, ignoring", entry);
        return;
    }

    SpellInfo const* spellinfo = sSpellMgr->GetSpellInfo(spell);
    if (!spellinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `npc_trainer` contains an entry (Entry: %u) for a non-existing spell (Spell: %u), ignoring", entry, spell);
        //WorldDatabase.PExecute("DELETE FROM `npc_trainer` WHERE entry = %u and spell = %u", entry, spell);
        return;
    }

    if (uint32 learnSpell = sDB2Manager.GetLearnSpell(spell))
    {
        if (SpellInfo const* spellinfoNew = sSpellMgr->GetSpellInfo(learnSpell))
        {
            spell = learnSpell;
            spellinfo = spellinfoNew;

            // Hack. The recipe does not have to be studied by the test spell
            if (spellinfo->Id == 125761)
                spell = 104298;
        }
    }

    if (!SpellMgr::IsSpellValid(spellinfo))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `npc_trainer` contains an entry (Entry: %u) for a broken spell (Spell: %u), ignoring", entry, spell);
        //WorldDatabase.PExecute("DELETE FROM `npc_trainer` WHERE entry = %u and spell = %u", entry, spell);
        return;
    }

    TrainerSpellData& data = _cacheTrainerSpellStore[entry];

    TrainerSpell& trainerSpell = data.spellList[spell];
    trainerSpell.spell         = spell;
    trainerSpell.spellCost     = spellCost;
    trainerSpell.reqSkill      = reqSkill;
    trainerSpell.reqSkillValue = reqSkillValue;
    trainerSpell.reqLevel      = reqLevel;

    if (!trainerSpell.reqLevel)
        trainerSpell.reqLevel = spellinfo->SpellLevel;

    // calculate learned spell for profession case when stored cast-spell
    trainerSpell.learnedSpell[0] = spell;
    for (uint8 i = 0; i < MAX_TRAINERSPELL_ABILITY_REQS; ++i)
    {
        if (spellinfo->Effects[i]->Effect != SPELL_EFFECT_LEARN_SPELL)
            continue;
        if (trainerSpell.learnedSpell[0] == spell)
            trainerSpell.learnedSpell[0] = 0;
        // player must be able to cast spell on himself
        if (spellinfo->Effects[i]->TargetA.GetTarget() != 0 && spellinfo->Effects[i]->TargetA.GetTarget() != TARGET_UNIT_TARGET_ALLY
            && spellinfo->Effects[i]->TargetA.GetTarget() != TARGET_UNIT_TARGET_ANY && spellinfo->Effects[i]->TargetA.GetTarget() != TARGET_UNIT_CASTER)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `npc_trainer` has spell %u for trainer entry %u with learn effect which has incorrect target type, ignoring learn effect!", spell, entry);
            continue;
        }

        trainerSpell.learnedSpell[i] = spellinfo->Effects[i]->TriggerSpell;

        if (trainerSpell.learnedSpell[i])
        {
            SpellInfo const* learnedSpellInfo = sSpellMgr->GetSpellInfo(trainerSpell.learnedSpell[i]);
            if (learnedSpellInfo && learnedSpellInfo->IsProfession())
                data.trainerType = 2;
        }
    }

    return;
}

void ObjectMgr::LoadTrainerSpell()
{
    uint32 oldMSTime = getMSTime();

    // For reload case
    _cacheTrainerSpellStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT b.entry, a.spell, a.spellcost, a.reqskill, a.reqskillvalue, a.reqlevel FROM npc_trainer AS a "
                                             "INNER JOIN npc_trainer AS b ON a.entry = -(b.spell) "
                                             "UNION SELECT * FROM npc_trainer WHERE spell > 0");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">>  Loaded 0 Trainers. DB table `npc_trainer` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 entry         = fields[0].GetUInt32();
        uint32 spell         = fields[1].GetUInt32();
        uint32 spellCost     = fields[2].GetUInt32();
        uint32 reqSkill      = fields[3].GetUInt16();
        uint32 reqSkillValue = fields[4].GetUInt16();
        uint32 reqLevel      = fields[5].GetUInt8();

        AddSpellToTrainer(entry, spell, spellCost, reqSkill, reqSkillValue, reqLevel);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %d Trainers in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

int ObjectMgr::LoadReferenceVendor(int32 vendor, int32 item, uint8 type, std::set<uint32> *skip_vendors)
{
    // find all items from the reference vendor
    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_NPC_VENDOR_REF);
    stmt->setUInt32(0, uint32(item));
    stmt->setUInt8(1, type);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
        return 0;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 item_id = fields[0].GetInt32();

        // if item is a negative, its a reference
        if (item_id < 0)
            count += LoadReferenceVendor(vendor, -item_id, type, skip_vendors);
        else
        {
            int32  maxcount     = fields[1].GetUInt32();
            uint32 incrtime     = fields[2].GetUInt32();
            uint32 ExtendedCost = fields[3].GetUInt32();
            uint8  type_         = fields[4].GetUInt8();
            uint64 money        = fields[5].GetUInt64();
            uint32 RandomPropertiesSeed = fields[6].GetUInt32();
            uint32 RandomPropertiesID = fields[7].GetUInt32();
            std::vector<uint32> bonusListIDs;
            Tokenizer BonusListID(fields[8].GetString(), ' ');
            for (char const* token : BonusListID)
                bonusListIDs.push_back(atol(token));
            std::vector<int32> ItemModifiers;
            Tokenizer ItemModifier(fields[9].GetString(), ' ');
            for (char const* token : ItemModifier)
                ItemModifiers.push_back(atol(token));
            bool DoNotFilterOnVendor = bool(fields[10].GetUInt8());
            uint8 Context = fields[11].GetUInt8();
            uint32 PlayerConditionID = fields[12].GetUInt32();

            if (!IsVendorItemValid(vendor, item_id, maxcount, incrtime, ExtendedCost, type_, nullptr, skip_vendors))
                continue;

            VendorItemData& vList = _cacheVendorItemStore[vendor];

            vList.AddItem(item_id, maxcount, incrtime, ExtendedCost, type_, money, RandomPropertiesSeed, RandomPropertiesID, bonusListIDs, ItemModifiers, DoNotFilterOnVendor, Context, PlayerConditionID);
            ++count;
        }
    } while (result->NextRow());

    return count;
}

void ObjectMgr::LoadVendors()
{
    uint32 oldMSTime = getMSTime();

    // For reload case
    for (CacheVendorItemContainer::iterator itr = _cacheVendorItemStore.begin(); itr != _cacheVendorItemStore.end(); ++itr)
        itr->second.Clear();
    _cacheVendorItemStore.clear();

    std::set<uint32> skip_vendors;

    QueryResult result = WorldDatabase.Query("SELECT entry, item, maxcount, incrtime, ExtendedCost, type, money, RandomPropertiesSeed, RandomPropertiesID, BonusListID, ItemModifiers, IgnoreFiltering, Context, PlayerConditionID FROM npc_vendor ORDER BY entry, slot ASC");
    if (!result)
    {

        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">>  Loaded 0 Vendors. DB table `npc_vendor` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 entry        = fields[0].GetUInt32();
        int32 item_id      = fields[1].GetInt32();

        // if item is a negative, its a reference
        if (item_id < 0)
            count += LoadReferenceVendor(entry, -item_id, 0, &skip_vendors);
        else
        {
            uint32 maxcount     = fields[2].GetUInt32();
            uint32 incrtime     = fields[3].GetUInt32();
            uint32 ExtendedCost = fields[4].GetUInt32();
            uint8  type         = fields[5].GetUInt8();
            uint64 money        = fields[6].GetUInt64();
            uint32 RandomPropertiesSeed = fields[7].GetUInt32();
            uint32 RandomPropertiesID = fields[8].GetUInt32();
            std::vector<uint32> bonusListIDs;
            Tokenizer BonusListID(fields[9].GetString(), ' ');
            for (char const* token : BonusListID)
                bonusListIDs.push_back(atol(token));
            std::vector<int32> ItemModifiers;
            Tokenizer ItemModifier(fields[10].GetString(), ' ');
            for (char const* token : ItemModifier)
                ItemModifiers.push_back(atol(token));
            bool DoNotFilterOnVendor = bool(fields[11].GetUInt8());
            uint8 Context = fields[12].GetUInt8();
            uint32 PlayerConditionID = fields[13].GetUInt32();

            if (!IsVendorItemValid(entry, item_id, maxcount, incrtime, ExtendedCost, type, nullptr, &skip_vendors))
                continue;

            VendorItemData& vList = _cacheVendorItemStore[entry];

            vList.AddItem(item_id, maxcount, incrtime, ExtendedCost, type, money, RandomPropertiesSeed, RandomPropertiesID, bonusListIDs, ItemModifiers, DoNotFilterOnVendor, Context, PlayerConditionID);
            ++count;
        }
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %d Vendors in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
 
void ObjectMgr::LoadDonateVendors()
{ 
    if (sWorld->getBoolConfig(CONFIG_DISABLE_DONATELOADING))
        return;

    // donate venodrs for Tokens    
    uint32 oldMSTime = getMSTime();
    m_donate_waite = true;
    std::lock_guard<std::recursive_mutex> guard(m_donate_lock);

    _cacheDonateVendorItemStore.clear();
    _fakedonvendorcat.clear();
    _reversfakedonvendorcat.clear();
    _donvenadds.clear();
    _donvendorcat.clear();
    
    uint32 count = 0;  
    
    // float donateDiscount;
    
    QueryResult result_discount = LoginDatabase.PQuery("SELECT rate from store_discounts where enable = 1 AND UNIX_TIMESTAMP(start) <= UNIX_TIMESTAMP() AND UNIX_TIMESTAMP(end) >= UNIX_TIMESTAMP()");
    
    if (!result_discount)
        donateDiscount = 1;
    else
    {
        Field* fielddi = result_discount->Fetch();
        donateDiscount = fielddi[0].GetFloat();
    }
    
    QueryResult result_don = LoginDatabase.PQuery("SELECT `s_p`.`category`, `s_p`.`item`, `s_p_r`.`token`, `s_p`.`enable`, `s_p`.`bonus`, `s_p`.`id`, `s_c`.`type`, `spl`.type, `spl`.`ru`, `spl`.`us`, `spl`.`kr`, `spl`.`fr`, `spl`.`de`, `spl`.`cn`, `spl`.`tw`, `spl`.`es`, `spl`.`mx`,`spl`.`pt`, `spl`.`br`, `spl`.`it`, s_p.faction FROM `store_products` AS s_p JOIN `store_categories` AS s_c ON `s_c`.`id` = `s_p`.`category` JOIN `store_product_realms` AS s_p_r ON `s_p_r`.`product` = `s_p`.`id` LEFT JOIN `store_product_locales` AS spl ON `spl`.`product` = `s_p`.`item` WHERE `s_p`.`enable` = '1' AND `s_p_r`.`realm` = '%u' AND `s_p_r`.`enable` = 1  ORDER by `s_p`.`category`;", sWorld->GetRealmId());
    if (result_don)
    {
        // it's some magic
        int32 currentFakeCat = 250000;
        int32 oldFakeCat = currentFakeCat;
        int32 currentCat = 0;
        int32 currentcount = 1;
        bool useFakeCat = false;
        do
        {
            Field* fields = result_don->Fetch();

            int32 entry        = fields[0].GetInt32();
            int32 item_id       = fields[1].GetInt32();
            uint32 DonateCost   = uint32(ceil(fields[2].GetUInt32() * donateDiscount)); //цениик
            uint8 enable        = fields[3].GetUInt8();
            std::vector<uint32> bonusListIDs;
            Tokenizer BonusListID(fields[4].GetString(), ':');
            for (char const* token : BonusListID)
                bonusListIDs.push_back(atol(token));
            
            int32 storeId       = fields[5].GetInt32();
            
            uint32 ExtendedCost     = 0;
            uint32 incrtime     = 0;
            uint32  type         = fields[6].GetUInt32();
                      
            if (!fields[7].GetString().empty())
                if (fields[7].GetUInt32() != type)
                    continue;
            
            type += 1; // for correct values like enum

            if (type >= DONATE_TYPE_MAX)
                continue;
            
            uint64 money     = 0;
            
            if (enable == 0)
                continue;
            if (DonateCost == 0)
                continue;
            
            if (currentCat != entry)
            {
                currentCat = entry;
                currentcount = 0;
                useFakeCat = false;
                oldFakeCat = entry;
            } 
            else if (currentcount > 150 || (type > DONATE_TYPE_ITEM && currentcount >= 12))
            {
                if (useFakeCat) // just in fakes categories, then can change old fake, else it display original id
                    oldFakeCat = currentFakeCat++;
                else
                    currentFakeCat++;
                
                currentcount -=  (type == DONATE_TYPE_ITEM ? 150 : 13);
                _fakedonvendorcat[entry].push_back(currentFakeCat);
                _reversfakedonvendorcat[currentFakeCat] = entry;
                useFakeCat = true;
                TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Continue in new FakeCat = %d from old cat = %d", currentFakeCat, entry);
            }
            
            if (useFakeCat)
                entry = currentFakeCat;

            if (type == DONATE_TYPE_ITEM)
            {
                VendorItemData& vList = _cacheDonateVendorItemStore[entry];

                vList.AddItem(item_id, 0, incrtime, ExtendedCost, ITEM_VENDOR_TYPE_ITEM, money, 0, 0, bonusListIDs, std::vector<int32>(), false, 0, 0, storeId, DonateCost);
            }
            else
            {
                DonVenCatBase& sub_base = _donvendorcat[entry];
                    
                DonVenAdd data;
                data.action = item_id;
                data.cost = DonateCost;
                data.type = type;
                data.storeId = storeId;
                
                uint8 index = 8;
                data.Names[8] = (!fields[index].GetString().empty() ? fields[index].GetString() : "");
                for (uint8 i = 0; i < 12; ++i)
                    if (i != 8)
                        data.Names[i] = (!fields[++index].GetString().empty() ? fields[index].GetString(): data.Names[8]);
                
                data.faction = fields[++index].GetUInt8();
                sub_base.additionals.push_back(data);

                if (useFakeCat)
                {
                    sub_base.prev_page = oldFakeCat;
                    
                    DonVenCatBase& prev_base = _donvendorcat[oldFakeCat];
                    prev_base.next_page = entry;
                }
                
                _donvenadds[type][item_id] = data;

                /*
                switch(type)
                {
                    case DONATE_TYPE_TITLE:
                        _donvenadds[DONATE_TYPE_TITLE][item_id] = data;
                        break;

                    default:
                        continue;
                }

                */
            }
            ++count;
            ++currentcount;
        
        }
        while (result_don->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %d Donate Vendors in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
    
    
    
    oldMSTime = getMSTime();
    count = 0;

    QueryResult result_cat = LoginDatabase.PQuery("SELECT `sc`.`pid`, `sc`.`id`, `scr`.`return`, `scl`.`name_ru`, `scl`.`name_us`, `scl`.`name_kr`, `scl`.`name_fr`, `scl`.`name_de`, `scl`.`name_cn`, `scl`.`name_tw`, `scl`.`name_es`, `scl`.`name_mx`,`scl`.`name_pt`, `scl`.`name_br`, `scl`.`name_it`,  `scl`.`description_ru`, `scl`.`description_us`, `scl`.`description_kr`, `scl`.`description_fr`, `scl`.`description_de`, `scl`.`description_cn`, `scl`.`description_tw`, `scl`.`description_es`, `scl`.`description_mx`, `scl`.`description_ru`, `scl`.`description_pt`, `scl`.`description_br`, `scl`.`description_it`, `sc`.`type`, `sc`.`faction` FROM `store_categories` AS sc LEFT JOIN `store_category_locales` AS scl ON `scl`.`category` = `sc`.`id` LEFT JOIN `store_category_realms` AS scr ON `scr`.`category` = `sc`.`id` WHERE `sc`.`enable` = '1' AND `scr`.`realm` = '%u' AND `scr`.`enable` = 1 GROUP BY `sc`.`id` ORDER BY `sc`.`sort` DESC, `sc`.`pid` ASC, `sc`.`id` ASC;", sWorld->GetRealmId());
    
    std::vector<uint32> correct_availables{};
    if (result_cat)
    {
    
         do
        {
            Field* fields = result_cat->Fetch();

            DonVenCat data;
            uint32 index = 0;
            int32 Parent        = fields[index].GetInt32();
            data.action = fields[++index].GetInt32();;
            bool returnable = fields[++index].GetBool();
            
            data.Names[8] =  (!fields[++index].GetString().empty() ? fields[index].GetString(): "");
            for (uint8 i = 0; i < 12; ++i)
                if (i != 8)
                    data.Names[i] = (!fields[++index].GetString().empty() ? fields[index].GetString(): data.Names[8]);
            
            std::string default_description = !fields[++index].GetString().empty() ? fields[index].GetString(): "";            
            default_description = !fields[++index].GetString().empty() ? fields[index].GetString(): default_description; // if exist eng, then use eng, else use rus
            
            data.Names[0] += default_description;
            for (uint8 i = 1; i < 12; ++i)
                data.Names[i] += (!fields[++index].GetString().empty() ? fields[index].GetString(): default_description);

            uint32  type         = fields[++index].GetUInt32() + 1;
            uint8 faction = fields[++index].GetUInt8();
            data.type = type;
            
            if (data.action >= 230100 || type != DONATE_TYPE_ITEM)
            {
                data.Names[8] += (returnable ?  "|cff008000 [Есть возврат]|r" :  "|cffFF0000 [Нет возврата]|r");
                for (uint8 i = 0; i < 12; ++i)
                    if (i != 8)
                        data.Names[i] += (returnable ?  "|cff008000 [Returnable]|r" :  "|cffFF0000 [No Returnable]|r");
            }

            if (type == DONATE_TYPE_MORPH)
            {
                data.is_available_for_preview = true;
                if (Parent)
                    correct_availables.push_back(Parent);
            }

            data.faction = faction;

            DonVenCatBase& sub_base = _donvendorcat[data.action];
            sub_base.type = type;
            sub_base.parent = Parent;
            
            DonVenCatBase& base = _donvendorcat[Parent];
            base.categories.push_back(data);


            if(std::vector<int32> const* fakeCat = sObjectMgr->GetFakeDonateVendorCat(data.action)) // push fake cat on this menu
            {
                int8 counter = 2;
                for (std::vector<int32>::const_iterator itr = fakeCat->begin(); itr != fakeCat->end(); ++itr)
                {
                    if (type == DONATE_TYPE_ITEM)
                    {
                        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> FakeCat = %d was created succesfull", *itr);
                        DonVenCat datafake;
                        datafake.action = uint32(*itr);
                        for (uint8 i = 0; i < 12; ++i)
                            datafake.Names[i] = data.Names[i] + " #" + std::to_string(counter);
                        datafake.type = type;
                        datafake.faction = faction;
                        
                        base.categories.push_back(datafake);
                    }
                    
                    DonVenCatBase& sub_base2 = _donvendorcat[uint32(*itr)];
                    sub_base2.type = type;
                    sub_base2.parent = Parent;
                    ++counter;
                }
            }
            ++count;    
        }
        while (result_cat->NextRow());
        
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %d Donate Vendor Categories in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    for (const auto& id : correct_availables)
    {
        uint32 action = id;
        const auto& cat = _donvendorcat[action];
        uint32 parent = cat.parent;
        do
        {
            auto& parent_cat = _donvendorcat[parent];
            for (auto& categori : parent_cat.categories)
                if (categori.action == action)
                {
                    categori.is_available_for_preview = true;
                    break;
                }

            if (parent != parent_cat.parent) // not both 0
            {
                action = parent;
                parent = parent_cat.parent;
            }
            else
                break;
        } while (true);
    }
    // reload
    oldMSTime = getMSTime();
    count = 0;
    
    _priceforlevelupStore.clear();
    _priceforArtlevelupStore.clear();
    
    QueryResult result_level = LoginDatabase.PQuery("SELECT level, token, type FROM `store_level_prices` WHERE realm = %u", sWorld->GetRealmId());
    
    if (result_level)
    {
    
        do
        {
            Field* fields = result_level->Fetch();

            uint32 level        = fields[0].GetUInt32();
            float cost          = ceil(fields[1].GetFloat() * donateDiscount);
            uint8 type          = fields[2].GetUInt8();
            
            switch(type)
            {
                case 0:
                    if (_priceforlevelupStore.size() <= level)
                        _priceforlevelupStore.resize(level + 1);
                    _priceforlevelupStore[level] = cost;
                    break;
                case 1:
                    if (_priceforArtlevelupStore.size() <= level)
                        _priceforArtlevelupStore.resize(level + 1);
                    _priceforArtlevelupStore[level] = cost;
                    break;
                case 2: _priceforAddBonusStore[level] = cost; break;
                default:
                    break;
            }
            
            ++count;    
        } while (result_level->NextRow());
        
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %d cost for levelup (lvl and art) in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
    m_donate_waite = false;
}

void ObjectMgr::AddVendorItem(uint32 entry, uint32 item, int32 maxcount, uint32 incrtime, uint32 extendedCost, uint8 type, uint64 money, bool persist /*= true*/)
{
    VendorItemData& vList = _cacheVendorItemStore[entry];
    vList.AddItem(item, maxcount, incrtime, extendedCost, type, money);

    if (persist)
    {
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_NPC_VENDOR);

        stmt->setUInt32(0, entry);
        stmt->setUInt32(1, item);
        stmt->setUInt8(2, maxcount);
        stmt->setUInt32(3, incrtime);
        stmt->setUInt32(4, extendedCost);
        stmt->setUInt8(5, type);

        WorldDatabase.Execute(stmt);
    }
}

bool ObjectMgr::RemoveVendorItem(uint32 entry, uint32 item, uint8 type, bool persist /*= true*/)
{
    CacheVendorItemContainer::iterator  iter = _cacheVendorItemStore.find(entry);
    if (iter == _cacheVendorItemStore.end())
        return false;

    if (!iter->second.RemoveItem(item, type))
        return false;

    if (persist)
    {
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_NPC_VENDOR);

        stmt->setUInt32(0, entry);
        stmt->setUInt32(1, item);
        stmt->setUInt8(2, type);

        WorldDatabase.Execute(stmt);
    }

    return true;
}

bool ObjectMgr::IsVendorItemValid(uint32 vendor_entry, uint32 id, int32 maxcount, uint32 incrtime, uint32 ExtendedCost, uint8 type, Player* player, std::set<uint32>* skip_vendors, uint32 ORnpcflag) const
{
    CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(vendor_entry);
    if (!cInfo)
    {
        if (player)
            ChatHandler(player).SendSysMessage(LANG_COMMAND_VENDORSELECTION);
        else
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `(game_event_)npc_vendor` have data for not existed creature template (Entry: %u), ignore", vendor_entry);
        return false;
    }

    if (!((cInfo->npcflag | ORnpcflag) & UNIT_NPC_FLAG_VENDOR))
    {
        if (!skip_vendors || skip_vendors->count(vendor_entry) == 0)
        {
            if (player)
                ChatHandler(player).SendSysMessage(LANG_COMMAND_VENDORSELECTION);
            else
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `(game_event_)npc_vendor` have data for not creature template (Entry: %u) without vendor flag, ignore", vendor_entry);

            if (skip_vendors)
                skip_vendors->insert(vendor_entry);
        }
        return false;
    }


    if ((type == ITEM_VENDOR_TYPE_ITEM && !sObjectMgr->GetItemTemplate(id)) ||
        (type == ITEM_VENDOR_TYPE_CURRENCY && !sCurrencyTypesStore.LookupEntry(id)))
    {
        if (player)
            ChatHandler(player).PSendSysMessage(LANG_ITEM_NOT_FOUND, id, type);
        else
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `(game_event_)npc_vendor` for Vendor (Entry: %u) have in item list non-existed item (%u, type %u), ignore", vendor_entry, id, type);
            //WorldDatabase.PExecute("DELETE FROM npc_vendor WHERE entry = %u and item = %u", vendor_entry, id);
        }
        return false;
    }

    if (ExtendedCost && !sItemExtendedCostStore.LookupEntry(ExtendedCost))
    {
        if (player)
            ChatHandler(player).PSendSysMessage(LANG_EXTENDED_COST_NOT_EXIST, ExtendedCost);
        else
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `(game_event_)npc_vendor` have Item (Entry: %u) with wrong ExtendedCost (%u) for vendor (%u), ignore", id, ExtendedCost, vendor_entry);
        return false;
    }

    if (type == ITEM_VENDOR_TYPE_ITEM) // not applicable to currencies
    {
        if (maxcount > 0 && incrtime == 0)
        {
            if (player)
                ChatHandler(player).PSendSysMessage("MaxCount != 0 (%u) but IncrTime == 0", maxcount);
            else
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `(game_event_)npc_vendor` has `maxcount` (%u) for item %u of vendor (Entry: %u) but `incrtime`=0, ignore", maxcount, id, vendor_entry);
            return false;
        }
        if (maxcount == 0 && incrtime > 0)
        {
            if (player)
                ChatHandler(player).PSendSysMessage("MaxCount == 0 but IncrTime<>= 0");
            else
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `(game_event_)npc_vendor` has `maxcount`=0 for item %u of vendor (Entry: %u) but `incrtime`<>0, ignore", id, vendor_entry);
            return false;
        }
    }

    VendorItemData const* vItems = GetNpcVendorItemList(vendor_entry);
    if (!vItems)
        return true;                                        // later checks for non-empty lists

    if (vItems->FindItemCostPair(id, ExtendedCost, type))
    {
        if (player)
            ChatHandler(player).PSendSysMessage(LANG_ITEM_ALREADY_IN_LIST, id, ExtendedCost, type);
        else
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `npc_vendor` has duplicate items %u (with extended cost %u, type %u) for vendor (Entry: %u), ignoring", id, ExtendedCost, type, vendor_entry);
        return false;
    }

    return true;
}

VendorItemData const* ObjectMgr::GetNpcVendorItemList(uint32 entry) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    return Trinity::Containers::MapGetValuePtr(_cacheVendorItemStore, entry);
}

VendorItemData const* ObjectMgr::GetNpcDonateVendorItemList(int32 entry) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    return Trinity::Containers::MapGetValuePtr(_cacheDonateVendorItemStore, entry);
}

DonVenCatBase const* ObjectMgr::GetDonateVendorCat(int32 entry) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    return Trinity::Containers::MapGetValuePtr(_donvendorcat, entry);
}

std::vector<int32> const* ObjectMgr::GetFakeDonateVendorCat(int32 entry) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    return Trinity::Containers::MapGetValuePtr(_fakedonvendorcat, entry);
}

int32 const* ObjectMgr::GetRealDonateVendorCat(int32 entry) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    return Trinity::Containers::MapGetValuePtr(_reversfakedonvendorcat, entry);
}

DonVenAdd const* ObjectMgr::GetDonateVendorAdditionalInfo(uint32 type, uint32 entry) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    auto ptr = _donvenadds.find(type);
    if (ptr == _donvenadds.end())
        return nullptr;

    return Trinity::Containers::MapGetValuePtr((*ptr).second, entry);
}

std::vector<DeathMatchStore> const* ObjectMgr::GetDeathMatchStore(uint8 type) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    return Trinity::Containers::MapGetValuePtr(_dmProducts, type);
}

std::vector<DeathMatchStore> const* ObjectMgr::GetDeathMatchStoreById(uint32 id) const
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    return Trinity::Containers::MapGetValuePtr(_dmProductsById, id);
}

float const* ObjectMgr::GetPriceForLevelUp(uint8 level) 
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    if (_priceforlevelupStore.size() <= level)
        return nullptr;
    return &_priceforlevelupStore[level];
}

float const* ObjectMgr::GetPriceForArtLevelUp(uint8 level) 
{
    if (m_donate_waite)
        std::lock_guard<std::recursive_mutex> guard(m_donate_lock);
    if (_priceforArtlevelupStore.size() <= level)
        return nullptr;
    return &_priceforArtlevelupStore[level];
}

PriceForAddBonus const* ObjectMgr::GetPricesForAddBonus()
{
    return &_priceforAddBonusStore;
};

const float& ObjectMgr::GetDonateDiscount() const
{
    return donateDiscount;
}

void ObjectMgr::LoadScriptNames()
{
    uint32 oldMSTime = getMSTime();

    _scriptNamesStore.push_back("");
    QueryResult result = WorldDatabase.Query(
      "SELECT DISTINCT(ScriptName) FROM achievement_criteria_data WHERE ScriptName <> '' AND type = 11 "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM achievement_reward WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM battleground_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM creature_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM gameobject_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM item_script_names WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM areatrigger_scripts WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM spell_script_names WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM transports WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM game_weather WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM conditions WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM outdoorpvp_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM spell_scene WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM eventobject_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(script) FROM instance_template WHERE script <> ''");

    if (!result)
    {

        TC_LOG_ERROR(LOG_FILTER_SQL, ">> Loaded empty set of Script Names!");
        return;
    }

    uint32 count = 1;

    do
    {
        _scriptNamesStore.push_back((*result)[0].GetString());
        ++count;
    }
    while (result->NextRow());

    std::sort(_scriptNamesStore.begin(), _scriptNamesStore.end());

#ifdef SCRIPTS
    for (size_t i = 1; i < _scriptNamesStore.size(); ++i)
        UnusedScriptNames.push_back(_scriptNamesStore[i]);
#endif

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %d Script Names in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

ObjectMgr::ScriptNameContainer& ObjectMgr::GetScriptNames()
{
    return _scriptNamesStore;
}

std::string const& ObjectMgr::GetScriptName(uint32 id) const
{
    static std::string const empty;
    return (id < _scriptNamesStore.size()) ? _scriptNamesStore[id] : empty;
}

uint32 ObjectMgr::GetScriptId(const char *name)
{
    // use binary search to find the script name in the sorted vector
    // assume "" is the first element
    if (!name)
        return 0;

    ScriptNameContainer::const_iterator itr = std::lower_bound(_scriptNamesStore.begin(), _scriptNamesStore.end(), name);
    if (itr == _scriptNamesStore.end() || *itr != name)
        return 0;

    return uint32(itr - _scriptNamesStore.begin());
}

SpellClickInfoMapBounds ObjectMgr::GetSpellClickInfoMapBounds(uint32 creatureID) const
{
    return _spellClickInfoStore.equal_range(creatureID);
}

bool LoadTrinityStrings(const char* table, int32 start_value, int32 end_value)
{
    // MAX_DB_SCRIPT_STRING_ID is max allowed negative value for scripts (scrpts can use only more deep negative values
    // start/end reversed for negative values
    if (start_value > MAX_DB_SCRIPT_STRING_ID || end_value >= start_value)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table '%s' load attempted with range (%d - %d) reserved by Trinity, strings not loaded.", table, start_value, end_value+1);
        return false;
    }

    return sObjectMgr->LoadTrinityStrings(table, start_value, end_value);
}

CreatureBaseStats const* ObjectMgr::GetCreatureBaseStats(uint8 level, uint8 unitClass)
{
    CreatureBaseStatsContainer::const_iterator it = _creatureBaseStatsStore.find(MAKE_PAIR16(level, unitClass));

    if (it != _creatureBaseStatsStore.end())
        return &(it->second);

    struct DefaultCreatureBaseStats : public CreatureBaseStats
    {
        DefaultCreatureBaseStats()
        {
            BaseArmor = 1;
            AttackPower = 1;
            RangedAttackPower = 1;
            for (uint8 j = 0; j < MAX_EXPANSIONS; ++j)
            {
                BaseHealth[j] = 1;
                BaseDamage[j] = 1;
            }
            BaseMana = 0;
        }
    };
    static const DefaultCreatureBaseStats def_stats;
    return &def_stats;
}

void ObjectMgr::LoadCreatureClassLevelStats()
{
    uint32 oldMSTime = getMSTime();
    //                                                  0     1       2        3            4
    QueryResult result = WorldDatabase.Query("SELECT level, class, basemana, attackpower, rangedattackpower FROM creature_classlevelstats");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature base stats. DB table `creature_classlevelstats` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint8 Level = fields[0].GetUInt8();
        uint8 Class = fields[1].GetUInt8();

        GtArmorMitigationByLvlEntry const* armor = sArmorMitigationByLvlGameTable.GetRow(Level);
        GtBaseMPEntry const* mp = sBaseMPGameTable.GetRow(Level);

        CreatureBaseStats stats;
        stats.BaseMana = mp ? GetGameTableColumnForClass(mp, Class) : fields[2].GetUInt32();
        stats.BaseArmor = armor ? uint32(armor->Mitigation / 2.6f) : 1;
        stats.AttackPower = fields[3].GetUInt16();
        stats.RangedAttackPower = fields[4].GetUInt16();

        if (!Class || ((1 << (Class - 1)) & CLASSMASK_ALL_CREATURES) == 0)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Creature base stats for level %u has invalid class %u", Level, Class);

        for (uint8 i = 0; i < MAX_EXPANSIONS; ++i)
        {
            stats.BaseHealth[i] = GetGameTableColumnForClass(sNpcTotalHpGameTable[i].GetRow(Level), Class);
            stats.BaseDamage[i] = GetGameTableColumnForClass(sNpcDamageByClassGameTable[i].GetRow(Level), Class);
        }

        _creatureBaseStatsStore[MAKE_PAIR16(Level, Class)] = stats;

        ++count;
    }
    while (result->NextRow());

    CreatureTemplateContainerMap const* ctc = sObjectMgr->GetCreatureTemplates();
    for (auto &v : *ctc)
        for (uint16 lvl = v.second.minlevel; lvl <= v.second.maxlevel; ++lvl)
            if (_creatureBaseStatsStore.find(MAKE_PAIR16(lvl, v.second.unit_class)) == _creatureBaseStatsStore.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Missing base stats for creature class %u level %u", v.second.unit_class, lvl);
            }
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature base stats in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadPhaseDefinitions()
{
    _PhaseDefinitionStore.clear();

    uint32 oldMSTime = getMSTime();

    //                                                 0       1       2         3            4           5          6             7
    QueryResult result = WorldDatabase.Query("SELECT zoneId, entry, phasemask, phaseId, PreloadMapID, VisibleMapID, flags, UiWorldMapAreaID FROM `phase_definitions` ORDER BY `entry` ASC");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 phasing definitions. DB table `phase_definitions` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field *fields = result->Fetch();

        PhaseDefinition pd;

        pd.zoneId                = fields[0].GetUInt32();
        pd.entry                 = fields[1].GetUInt16();
        pd.phasemask             = fields[2].GetUInt64();
        pd.terrainswapmap        = fields[4].GetUInt16();
        pd.wmAreaId              = fields[5].GetUInt16();
        pd.flags                 = fields[6].GetUInt8();
        pd.uiWmAreaId            = fields[7].GetUInt16();

        Tokenizer phasesToken(fields[3].GetString(), ' ', 100);
        for (auto itr : phasesToken)
            if (PhaseEntry const* phase = sPhaseStore.LookupEntry(uint32(strtoull(itr, nullptr, 10))))
                pd.phaseId.push_back(phase->ID);

        // Checks
        if ((pd.flags & PHASE_FLAG_OVERWRITE_EXISTING) && (pd.flags & PHASE_FLAG_NEGATE_PHASE))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Flags defined in phase_definitions in zoneId %d and entry %u does contain PHASE_FLAG_OVERWRITE_EXISTING and PHASE_FLAG_NEGATE_PHASE. Setting flags to PHASE_FLAG_OVERWRITE_EXISTING", pd.zoneId, pd.entry);
            pd.flags &= ~PHASE_FLAG_NEGATE_PHASE;
        }

        if (pd.terrainswapmap > 0)
        {
            const MapEntry* const map = sMapStore.LookupEntry(pd.terrainswapmap);
            if (!map)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "DB table `phase_definitions` has not existen terrainswapmap %u", pd.terrainswapmap);
                continue;
            }
        }

        _PhaseDefinitionStore[pd.zoneId].push_back(pd);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u phasing definitions in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSpellPhaseInfo()
{
    _SpellPhaseStore.clear();

    uint32 oldMSTime = getMSTime();

    //                                               0       1            2            3
    QueryResult result = WorldDatabase.Query("SELECT id, phasemask, terrainswapmap, phaseId FROM `spell_phase`");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell dbc infos. DB table `spell_phase` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *fields = result->Fetch();

        SpellPhaseInfo spellPhaseInfo;
        spellPhaseInfo.spellId                = fields[0].GetUInt32();

        SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellPhaseInfo.spellId);
        if (!spell)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u defined in `spell_phase` does not exists, skipped.", spellPhaseInfo.spellId);
            continue;
        }

        spellPhaseInfo.phasemask              = fields[1].GetUInt32();
        spellPhaseInfo.terrainswapmap         = fields[2].GetUInt32();
        spellPhaseInfo.phaseId                = fields[3].GetUInt32();
        
        _SpellPhaseStore[spellPhaseInfo.spellId] = spellPhaseInfo;

        ++count;
    }
    while (result->NextRow());
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell dbc infos in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

bool ObjectMgr::IsStaticTransport(uint32 entry)
{
    if (GameObjectTemplate const* goinfo = GetGameObjectTemplate(entry))
        return goinfo->type == GAMEOBJECT_TYPE_TRANSPORT;

    return false;
}

GameObjectTemplate const* ObjectMgr::GetGameObjectTemplate(uint32 entry)
{
    return Trinity::Containers::MapGetValuePtr(_gameObjectTemplateStore, entry);
}

const std::vector<CreatureDifficultyStat>* ObjectMgr::GetDifficultyStat(uint32 entry) const
{
    if (entry >= _creatureDifficultyStatStore.size())
        return nullptr;

    return &_creatureDifficultyStatStore[entry];
}

CreatureDifficultyStat const* ObjectMgr::GetCreatureDifficultyStat(uint32 entry, uint8 diff) const
{
    if (std::vector<CreatureDifficultyStat> const* diffStat = GetDifficultyStat(entry))
        for (std::vector<CreatureDifficultyStat>::const_iterator itr = diffStat->begin(); itr != diffStat->end(); ++itr)
            if (itr->Difficulty == diff)
                return &(*itr);

    return nullptr;
}

CreatureTemplate const* ObjectMgr::GetCreatureTemplate(uint32 entry)
{
    if (entry >= _creatureTemplateStore.size())
        return nullptr;

    return _creatureTemplateStore[entry];
}

VehicleAccessoryList const* ObjectMgr::GetVehicleAccessoryList(Vehicle* veh) const
{
    if (Creature* cre = veh->GetBase()->ToCreature())
    {
        // Give preference to GUID-based accessories
        VehicleAccessoryContainer::const_iterator itr = _vehicleAccessoryStore.find(cre->GetDBTableGUIDLow());
        if (itr != _vehicleAccessoryStore.end())
            return &itr->second;
    }

    // Otherwise return entry-based
    VehicleAccessoryTemplateContainer::const_iterator itr = _vehicleTemplateAccessoryStore.find(veh->GetCreatureEntry());
    if (itr != _vehicleTemplateAccessoryStore.end())
        return &itr->second;

    // For Player witch come vehicle by 296 aura
    if (veh->GetRecAura())
        return Trinity::Containers::MapGetValuePtr(_vehicleTemplateAccessoryStore, veh->GetRecAura() * -1);
    
    return nullptr;
}

VehicleAttachmentOffset const* ObjectMgr::GetVehicleAttachmentOffset(Vehicle* veh, int8 SeatId) const
{
    // Otherwise return entry-based
    VehicleAttachmentOffsetContainer::const_iterator itr = _vehicleAttachmentOffsetStore.find(veh->GetCreatureEntry());
    if (itr == _vehicleAttachmentOffsetStore.end())
        return nullptr;

    for (VehicleAttachmentOffsetList::const_iterator iter = itr->second.begin(); iter != itr->second.end(); ++iter)
        if (SeatId == iter->SeatId)  // only install minions on evade mode
            return &(*iter);

    return nullptr;
}

float ObjectMgr::GetMapDifficultyStat(uint32 mapId, uint8 difficultyId)
{
    if (_mapDifficultyStat.size() <= mapId)
        return 1.0f;
    if (_mapDifficultyStat[mapId].size() <= difficultyId)
        return 1.0f;
    return _mapDifficultyStat[mapId][difficultyId];
}

DungeonEncounterList const* ObjectMgr::GetDungeonEncounterList(uint32 mapId, Difficulty difficulty)
{
    return _dungeonEncounterVector[difficulty][mapId];
}

uint16 ObjectMgr::GetDungeonEncounterByCreature(uint32 creatureId)
{
    CreatureToDungeonEncounterMap::const_iterator itr = _creatureToDungeonEncounter.find(creatureId);
    if (itr != _creatureToDungeonEncounter.end())
        return itr->second;
    return 0;
}

uint32 ObjectMgr::GetCreatureByDungeonEncounter(uint32 EncounterID)
{
    if (EncounterID >= _dungeonEncounterToCreature.size())
        return 0;
    return _dungeonEncounterToCreature[EncounterID];
}

void ObjectMgr::RestructCreatureGUID()
{
    QueryResult result = WorldDatabase.Query("SELECT guid FROM creature ORDER BY guid;");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "RestructCreatureGUID failed");
        return;
    }

    // First step
    WorldDatabase.PExecute("DROP TABLE IF EXISTS creaturerestruct;");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS creature_addonrestruct");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS waypoint_datarestruct;");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS creature_formationsrestruct;");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS game_event_creaturerestruct;");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS pool_creaturerestruct;");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS linked_respawnrestruct;");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS `restructcreature`;");
    WorldDatabase.PExecute("DROP TABLE IF EXISTS `restructgameobject`;");

    // First step
    WorldDatabase.PExecute("CREATE TABLE creaturerestruct LIKE creature;");
    WorldDatabase.PExecute("CREATE TABLE creature_addonrestruct LIKE creature_addon;");
    WorldDatabase.PExecute("CREATE TABLE waypoint_datarestruct LIKE waypoint_data;");
    WorldDatabase.PExecute("CREATE TABLE creature_formationsrestruct LIKE creature_formations;");
    WorldDatabase.PExecute("CREATE TABLE game_event_creaturerestruct LIKE game_event_creature;");
    WorldDatabase.PExecute("CREATE TABLE pool_creaturerestruct LIKE pool_creature;");
    WorldDatabase.PExecute("CREATE TABLE linked_respawnrestruct LIKE linked_respawn;");
    WorldDatabase.PExecute("CREATE TABLE `restructcreature` ( `oldGUID` int(11) NOT NULL,  `newGUID` int(11) NOT NULL, PRIMARY KEY (`oldGUID`,`newGUID`) ) ENGINE=InnoDB DEFAULT CHARSET=utf8;");
    WorldDatabase.PExecute("CREATE TABLE `restructgameobject` ( `oldGUID` int(11) NOT NULL, `newGUID` int(11) NOT NULL, PRIMARY KEY (`oldGUID`,`newGUID`)) ENGINE=InnoDB DEFAULT CHARSET=utf8;");

    WorldDatabase.PExecute("INSERT INTO creaturerestruct SELECT * FROM creature;");
    WorldDatabase.PExecute("INSERT INTO creature_addonrestruct SELECT * FROM creature_addon;");
    WorldDatabase.PExecute("INSERT INTO waypoint_datarestruct SELECT * FROM waypoint_data;");
    WorldDatabase.PExecute("INSERT INTO creature_formationsrestruct SELECT * FROM creature_formations;");
    WorldDatabase.PExecute("INSERT INTO game_event_creaturerestruct SELECT * FROM game_event_creature;");
    WorldDatabase.PExecute("INSERT INTO pool_creaturerestruct SELECT * FROM pool_creature;");
    WorldDatabase.PExecute("INSERT INTO linked_respawnrestruct SELECT * FROM linked_respawn;");

    WorldDatabase.PExecute("TRUNCATE creature;");
    WorldDatabase.PExecute("TRUNCATE creature_addon;");
    WorldDatabase.PExecute("TRUNCATE waypoint_data;");
    WorldDatabase.PExecute("TRUNCATE creature_formations;");
    WorldDatabase.PExecute("TRUNCATE game_event_creature;");
    WorldDatabase.PExecute("TRUNCATE pool_creature;");
    WorldDatabase.PExecute("TRUNCATE linked_respawn;");
    WorldDatabase.PExecute("TRUNCATE restructCreature;");

    //Second step
    WorldDatabase.PExecute("CREATE TABLE creaturerestruct1 LIKE creature;");
    WorldDatabase.PExecute("CREATE TABLE creature_addonrestruct1 LIKE creature_addon;");
    WorldDatabase.PExecute("CREATE TABLE waypoint_datarestruct1 LIKE waypoint_data;");
    WorldDatabase.PExecute("CREATE TABLE creature_formationsrestruct1 LIKE creature_formations;");
    WorldDatabase.PExecute("CREATE TABLE game_event_creaturerestruct1 LIKE game_event_creature;");
    WorldDatabase.PExecute("CREATE TABLE pool_creaturerestruct1 LIKE pool_creature;");
    WorldDatabase.PExecute("CREATE TABLE linked_respawnrestruct1 LIKE linked_respawn;");

    uint32 newGUID = 1;
    do
    {
        Field *fields = result->Fetch();
        uint32 oldGUID = fields[0].GetUInt32();

        SQLTransaction worldTrans = WorldDatabase.BeginTransaction();

        worldTrans->PAppend("INSERT INTO creaturerestruct1 SELECT * FROM creaturerestruct WHERE guid = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO creature_addonrestruct1 SELECT * FROM creature_addonrestruct WHERE guid = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO waypoint_datarestruct1 SELECT * FROM waypoint_datarestruct WHERE id = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO creature_formationsrestruct1 SELECT * FROM creature_formationsrestruct WHERE leaderGUID = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO creature_formationsrestruct1 SELECT * FROM creature_formationsrestruct WHERE memberGUID = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO game_event_creatureRestruct1 SELECT * FROM game_event_creaturerestruct WHERE guid = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO pool_creaturerestruct1 SELECT * FROM pool_creaturerestruct WHERE guid = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO linked_respawnrestruct1 SELECT * FROM linked_respawnrestruct WHERE guid = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO linked_respawnrestruct1 SELECT * FROM linked_respawnrestruct WHERE linkedGuid = %u;", oldGUID);

        worldTrans->PAppend("UPDATE creaturerestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE creature_addonrestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE creature_addonrestruct1 SET path_id = %u WHERE path_id = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE waypoint_datarestruct1 SET id = %u WHERE id = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE creature_formationsrestruct1 SET leaderGUID = %u WHERE leaderGUID = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE creature_formationsrestruct1 SET memberGUID = %u WHERE memberGUID = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE game_event_creaturerestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE pool_creaturerestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE smart_scripts SET target_param1 = %u WHERE `target_type` = 10 AND target_param1 = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE smart_scripts SET entryorguid = -%u WHERE `source_type` = 0 AND entryorguid = -%u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE linked_respawnrestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE linked_respawnrestruct1 SET linkedGuid = %u WHERE linkedGuid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("INSERT INTO restructcreature VALUE ('%u', '%u');", oldGUID, newGUID);

        worldTrans->PAppend("INSERT INTO creature SELECT * FROM creaturerestruct1 WHERE guid = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO creature_addon SELECT * FROM creature_addonrestruct1 WHERE guid = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO waypoint_data SELECT * FROM waypoint_datarestruct1 WHERE id = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO creature_formations SELECT * FROM creature_formationsrestruct1 WHERE leaderGUID = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO creature_formations SELECT * FROM creature_formationsrestruct1 WHERE memberGUID = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO game_event_creature SELECT * FROM game_event_creaturerestruct1 WHERE guid = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO pool_creature SELECT * FROM pool_creaturerestruct1 WHERE guid = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO linked_respawn SELECT * FROM linked_respawnrestruct1 WHERE guid = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO linked_respawn SELECT * FROM linked_respawnrestruct1 WHERE linkedGuid = %u;", newGUID);

        worldTrans->PAppend("TRUNCATE creaturerestruct1;");
        worldTrans->PAppend("TRUNCATE creature_addonrestruct1;");
        worldTrans->PAppend("TRUNCATE waypoint_datarestruct1;");
        worldTrans->PAppend("TRUNCATE creature_formationsrestruct1;");
        worldTrans->PAppend("TRUNCATE game_event_creaturerestruct1;");
        worldTrans->PAppend("TRUNCATE pool_creaturerestruct1;");
        worldTrans->PAppend("TRUNCATE linked_respawnrestruct1;");

        WorldDatabase.CommitTransaction(worldTrans);
        WorldDatabase.WaitExecution();
        newGUID++;
    } while (result->NextRow());

    WorldDatabase.PExecute("ALTER TABLE creature AUTO_INCREMENT = %u;", newGUID + 1);

    WorldDatabase.PExecute("DROP TABLE creaturerestruct;");
    WorldDatabase.PExecute("DROP TABLE creature_addonrestruct;");
    WorldDatabase.PExecute("DROP TABLE waypoint_datarestruct;");
    WorldDatabase.PExecute("DROP TABLE creature_formationsrestruct;");
    WorldDatabase.PExecute("DROP TABLE game_event_creaturerestruct;");
    WorldDatabase.PExecute("DROP TABLE pool_creaturerestruct;");
    WorldDatabase.PExecute("DROP TABLE linked_respawnrestruct;");
    WorldDatabase.PExecute("DROP TABLE creaturerestruct1;");
    WorldDatabase.PExecute("DROP TABLE creature_addonrestruct1;");
    WorldDatabase.PExecute("DROP TABLE waypoint_datarestruct1;");
    WorldDatabase.PExecute("DROP TABLE creature_formationsrestruct1;");
    WorldDatabase.PExecute("DROP TABLE game_event_creaturerestruct1;");
    WorldDatabase.PExecute("DROP TABLE pool_creaturerestruct1;");
    WorldDatabase.PExecute("DROP TABLE linked_respawnrestruct1;");

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "RestructCreatureGUID: Last guid %u", newGUID);
}

void ObjectMgr::RestructGameObjectGUID()
{
    QueryResult result = WorldDatabase.Query("SELECT guid FROM gameobject ORDER BY guid;");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "RestructGameObjectGUID failed");
        return;
    }

    WorldDatabase.PExecute("CREATE TABLE gameobjectRestruct LIKE gameobject;");
    WorldDatabase.PExecute("CREATE TABLE game_event_gameobjectRestruct LIKE game_event_gameobject;");
    WorldDatabase.PExecute("CREATE TABLE pool_gameobjectRestruct LIKE pool_gameobject;");

    WorldDatabase.PExecute("INSERT INTO gameobjectRestruct SELECT * FROM gameobject;");
    WorldDatabase.PExecute("INSERT INTO game_event_gameobjectRestruct SELECT * FROM game_event_gameobject;");
    WorldDatabase.PExecute("INSERT INTO pool_gameobjectRestruct SELECT * FROM pool_gameobject;");

    WorldDatabase.PExecute("TRUNCATE gameobject;");
    WorldDatabase.PExecute("TRUNCATE game_event_gameobject;");
    WorldDatabase.PExecute("TRUNCATE pool_gameobject;");
    WorldDatabase.PExecute("TRUNCATE restructGameobject;");

    WorldDatabase.PExecute("CREATE TABLE gameobjectRestruct1 LIKE gameobject;");
    WorldDatabase.PExecute("CREATE TABLE game_event_gameobjectRestruct1 LIKE game_event_gameobject;");
    WorldDatabase.PExecute("CREATE TABLE pool_gameobjectRestruct1 LIKE pool_gameobject;");

    uint32 newGUID = 1;
    do
    {
        Field *fields = result->Fetch();
        uint32 oldGUID = fields[0].GetUInt32();

        SQLTransaction worldTrans = WorldDatabase.BeginTransaction();

        worldTrans->PAppend("INSERT INTO gameobjectRestruct1 SELECT * FROM gameobjectRestruct WHERE guid = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO game_event_gameobjectRestruct1 SELECT * FROM game_event_gameobjectRestruct WHERE guid = %u;", oldGUID);
        worldTrans->PAppend("INSERT INTO pool_gameobjectRestruct1 SELECT * FROM pool_gameobjectRestruct WHERE guid = %u;", oldGUID);

        worldTrans->PAppend("UPDATE gameobjectRestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE game_event_gameobjectRestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE pool_gameobjectRestruct1 SET guid = %u WHERE guid = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE smart_scripts SET target_param1 = %u WHERE `target_type` = 14 AND target_param1 = %u;", newGUID, oldGUID);
        worldTrans->PAppend("UPDATE smart_scripts SET entryorguid = -%u WHERE `source_type` = 1 AND entryorguid = -%u;", newGUID, oldGUID);
        worldTrans->PAppend("INSERT INTO restructGameobject VALUE ('%u', '%u');", oldGUID, newGUID);

        worldTrans->PAppend("INSERT INTO gameobject SELECT * FROM gameobjectRestruct1 WHERE guid = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO game_event_gameobject SELECT * FROM game_event_gameobjectRestruct1 WHERE guid = %u;", newGUID);
        worldTrans->PAppend("INSERT INTO pool_gameobject SELECT * FROM pool_gameobjectRestruct1 WHERE guid = %u;", newGUID);

        worldTrans->PAppend("TRUNCATE gameobjectRestruct1;");
        worldTrans->PAppend("TRUNCATE game_event_gameobjectRestruct1;");
        worldTrans->PAppend("TRUNCATE pool_gameobjectRestruct1;");

        WorldDatabase.CommitTransaction(worldTrans);
        WorldDatabase.WaitExecution();

        newGUID++;
    } while (result->NextRow());

    WorldDatabase.PExecute("ALTER TABLE gameobject AUTO_INCREMENT = %u;", newGUID + 1);

    WorldDatabase.PExecute("DROP TABLE gameobjectRestruct;");
    WorldDatabase.PExecute("DROP TABLE game_event_gameobjectRestruct;");
    WorldDatabase.PExecute("DROP TABLE pool_gameobjectRestruct;");

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "RestructGameObjectGUID: Last guid %u", newGUID);
}

void ObjectMgr::LoadResearchSiteToZoneData()
{
    QueryResult result = WorldDatabase.Query("SELECT site_id, zone_id, branch_id FROM archaeology_zones");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 archaeology zones. DB table `archaeology_zones` is empty.");
        return;
    }

    uint32 counter = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 site_id = fields[0].GetUInt32();
        uint32 zone_id = fields[1].GetUInt32();
        uint32 branch_id = fields[2].GetUInt32();

        auto itr = sDB2Manager._researchSiteDataMap.find(site_id);
        if (itr == sDB2Manager._researchSiteDataMap.end())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "DB table `archaeology_zones` has data for nonexistant site id %u", site_id);
            continue;
        }

        ResearchSiteData& data = itr->second;
        data.zone = zone_id;
        data.branch_id = branch_id;

        for (AreaTableEntry const* area : sAreaTableStore)
            if (area->ParentAreaID == zone_id)
            {
                data.level = area->ExplorationLevel;
                break;
            }

        ++counter;
    }
    while (result->NextRow());

    // recheck all research sites
    /*for (auto const& itr : sDB2Manager._researchSiteDataMap)
    {
        if (itr.second.zone == 0 || itr.second.level == 0xFF || itr.second.branch_id == 0)
            TC_LOG_ERROR(LOG_FILTER_SQL, "DB table `archaeology_zones` has not full or does not have data for site id %u: "
            "zone %u level %u branch_id %u",
            itr.second.entry->ID, itr.second.zone, itr.second.level, itr.second.branch_id);
    }*/

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u archaeology zones.", counter);
}

void ObjectMgr::LoadDigSitePositions()
{
    QueryResult result = WorldDatabase.Query("SELECT idx, map, x, y, map_x, map_y, zone, entry FROM archaeology_digsites");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 dig site positions. DB table `archaeology_digsites` is empty.");
        return;
    }

    uint32 counter = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 idx = fields[0].GetUInt32();
        uint32 map = fields[1].GetUInt32();
        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float map_x = fields[4].GetFloat();
        float map_y = fields[5].GetFloat();
        uint32 zoneId = fields[6].GetUInt32();
        uint32 entry = fields[7].GetUInt32();
        bool needUpdateBranch = false;
        uint32 branchId = 0;

        if (!x && !y)
        {
            x = map_x;
            y = map_y;
            sDB2Manager.Zone2MapCoordinates(x, y, zoneId);
            if (x == map_x || y == map_y) // Bugged, zone for convert not found
                continue;

            if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(zoneId))
                map = zone->ContinentID;

            WorldDatabase.PExecute("UPDATE archaeology_digsites SET x = %f, y = %f, map = %u WHERE idx = %u", x, y, map, idx);
            needUpdateBranch = true;
            TC_LOG_ERROR(LOG_FILTER_SQL, "DB table `archaeology_digsites` has data for point idx:%u x:%f y:%f at map %u ADD!", idx, x, y, map);
        }

        bool added = false;
        for (auto& itr : sDB2Manager._researchSiteDataMap)
        {
            ResearchSiteData& data = itr.second;

            if (uint32(data.entry->MapID) != map)
                continue;

            ResearchPOIPoint p;
            p.x = int32(x);
            p.y = int32(y);

            if (Player::IsPointInZone(p, data.points))
            {
                data.digSites.push_back(DigSitePosition(x, y));
                data.find_id = entry;
                added = true;
                if (entry)
                    branchId = Player::GetBranchByGO(entry);
                if(needUpdateBranch && branchId)
                {
                    WorldDatabase.PExecute("REPLACE INTO `archaeology_zones` SET `site_id`='%u',`zone_id`='%u',`branch_id`='%u';", data.entry->ID, zoneId, branchId);
                    WorldDatabase.PExecute("UPDATE archaeology_digsites SET branch_id = %u WHERE idx = %u", branchId, idx);
                }
            }
        }

        if (!added)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "DB table `archaeology_digsites` has data for point idx:%u x:%f y:%f at map %u that does not belong to any digsite!", idx, x, y, map);
            continue;
        }

        ++counter;
    }
    while (result->NextRow());

    std::set<uint32> toRemove;
    for (auto& itr : sDB2Manager._researchSiteDataMap)
    {
        ResearchSiteData& data = itr.second;

        uint32 siteCount = data.digSites.size();
        if (siteCount < MAX_DIGSITE_FINDS)
        {
            //TC_LOG_ERROR(LOG_FILTER_SQL, "Archaeology research site %u has less that %u dig site positions! (%u)", data.entry->ID, MAX_DIGSITE_FINDS, siteCount);
            if (!siteCount)
                toRemove.insert(itr.first);
            else
            {
                while (data.digSites.size() < MAX_DIGSITE_FINDS)
                    data.digSites.push_back(data.digSites[urand(0, data.digSites.size() - 1)]);
            }
        }
    }

    for (std::set<uint32>::const_iterator itr = toRemove.begin(); itr != toRemove.end(); ++itr)
        sDB2Manager._researchSiteDataMap.erase(*itr);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u dig site positions _researchSiteDataMap %u", counter, sDB2Manager._researchSiteDataMap.size());
}

void ObjectMgr::LoadScenarioData()
{
    _scenarioData.clear();
    _scenarioDataList.clear();

    //                                                0            1        2               3
    QueryResult result = WorldDatabase.Query("SELECT `ScenarioID`, `MapID`, `DifficultyID`, `Team`, `Class`, `LfgDungeonID` FROM `scenario_data`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 ScenarioID = fields[i++].GetUInt32();
            ScenarioData& data = _scenarioData[ScenarioID];
            data.MapID = fields[i++].GetUInt32();
            data.DifficultyID = fields[i++].GetUInt32();
            data.Team = fields[i++].GetUInt32();
            data.Class = fields[i++].GetUInt32();
            data.LfgDungeonID = fields[i++].GetUInt32();
            data.ScenarioID = ScenarioID;
            _scenarioDataList[data.MapID].push_back(&data);
            ++counter;
        }
        while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Scenario data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Scenario data. DB table `scenario_data` is empty.");
}

void ObjectMgr::LoadScenarioSpellData()
{
    _scenarioDataSpellStep.clear();

    //                                                  0            1        2       
    QueryResult result = WorldDatabase.Query("SELECT `scenarioId`, `step`, `spell` from `scenario_step_spells`;");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 ScenarioID = fields[i++].GetUInt32();
            ScenarioSpellData data;
            data.StepId = fields[i++].GetUInt32();
            data.Spells = fields[i++].GetUInt32();

            if (_scenarioDataSpellStep.size() <= ScenarioID)
                _scenarioDataSpellStep.resize(ScenarioID + 1);

            _scenarioDataSpellStep[ScenarioID].push_back(data);
            ++counter;
        }
        while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Scenario Step Spell Data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Scenario Step Spell data. DB table `scenario_step_spells` is empty.");
}

void ObjectMgr::LoadCreatureActionData()
{
    for (uint8 j = 0; j < CREATURE_ACTION_TYPE_MAX; ++j)
        _creatureActionDataList[j].clear();

    //                                                  0         1        2        3         4
    QueryResult result = WorldDatabase.Query("SELECT `entry`, `target`, `type`, `spellId`, `action` FROM `creature_action`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            CreatureActionData data;
            data.entry = fields[i++].GetUInt32();
            data.target = fields[i++].GetUInt32();
            data.type = fields[i++].GetUInt32();
            data.spellId = fields[i++].GetUInt32();
            data.action = fields[i++].GetUInt32();
            _creatureActionDataList[data.action][data.entry].push_back(data);
            ++counter;
        }
        while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature_action data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature_action data. DB table `creature_action` is empty.");
}

void ObjectMgr::LoadSeamlessTeleportData()
{
    _seamlessZoneTeleport.clear();
    _seamlessAreaTeleport.clear();

    //                                                  0          1         2           3
    QueryResult result = WorldDatabase.Query("SELECT `ZoneID`, `AreaID`, `FromMapID`, `ToMapID` FROM `world_seamless_teleport`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint16 ZoneID = fields[i++].GetUInt16();
            uint16 AreaID = fields[i++].GetUInt16();
            SeamlessTeleportData& data = ZoneID ? _seamlessZoneTeleport[ZoneID] : _seamlessAreaTeleport[AreaID];
            data.ZoneID = ZoneID;
            data.AreaID = AreaID;
            data.FromMapID = fields[i++].GetUInt16();
            data.ToMapID = fields[i++].GetUInt16();
            ++counter;
        }
        while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u seamless teleport data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 seamless teleport data. DB table `world_seamless_teleport` is empty.");
}

void ObjectMgr::LoadDisplayChoiceData()
{
    auto oldMSTime = getMSTime();
    _playerChoices.clear();

    auto choices = WorldDatabase.Query("SELECT ChoiceId, UiTextureKitId, Question, HideWarboardHeader FROM playerchoice");
    if (!choices)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 player choices. DB table `playerchoice` is empty.");
        return;
    }

    uint32 responseCount = 0;
    uint32 rewardCount = 0;
    uint32 itemRewardCount = 0;
    uint32 currencyRewardCount = 0;
    uint32 factionRewardCount = 0;

    do
    {
        auto fields = choices->Fetch();

        auto choiceId = fields[0].GetInt32();

        auto& choice = _playerChoices[choiceId];
        choice.ChoiceId = choiceId;
        choice.UiTextureKitId = fields[1].GetInt32();
        choice.Question = fields[2].GetString();
        choice.HideWarboardHeader = fields[3].GetBool();

    } while (choices->NextRow());

    if (auto responses = WorldDatabase.Query("SELECT ChoiceId, ResponseId, ChoiceArtFileId, Header, Answer, Description, Confirmation FROM playerchoice_response ORDER BY `Index` ASC"))
    {
        do
        {
            auto fields = responses->Fetch();

            auto choiceId = fields[0].GetInt32();
            auto responseId = fields[1].GetInt32();

            auto choice = Trinity::Containers::MapGetValuePtr(_playerChoices, choiceId);
            if (!choice)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response` references non-existing ChoiceId: %d (ResponseId: %d), skipped", choiceId, responseId);
                continue;
            }

            choice->Responses.emplace_back();

            auto& response = choice->Responses.back();
            response.ResponseId = responseId;
            response.ChoiceArtFileId = fields[2].GetInt32();
            response.Header = fields[3].GetString();
            response.Answer = fields[4].GetString();
            response.Description = fields[5].GetString();
            response.Confirmation = fields[6].GetString();
            ++responseCount;

        } while (responses->NextRow());
    }

    if (auto rewards = WorldDatabase.Query("SELECT ChoiceId, ResponseId, TitleId, PackageId, SkillLineId, SkillPointCount, ArenaPointCount, HonorPointCount, Money, Xp, SpellID FROM playerchoice_response_reward"))
    {
        do
        {
            auto fields = rewards->Fetch();

            auto choiceId = fields[0].GetInt32();
            auto responseId = fields[1].GetInt32();

            auto choice = Trinity::Containers::MapGetValuePtr(_playerChoices, choiceId);
            if (!choice)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward` references non-existing ChoiceId: %d (ResponseId: %d), skipped", choiceId, responseId);
                continue;
            }

            auto responseItr = std::find_if(choice->Responses.begin(), choice->Responses.end(), [responseId](PlayerChoiceResponse const& playerChoiceResponse) { return playerChoiceResponse.ResponseId == responseId; });
            if (responseItr == choice->Responses.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward` references non-existing ResponseId: %d for ChoiceId %d, skipped", responseId, choiceId);
                continue;
            }

            responseItr->Reward = boost::in_place();

            auto* reward = responseItr->Reward.get_ptr();
            reward->TitleId = fields[2].GetInt32();
            reward->PackageId = fields[3].GetInt32();
            reward->SkillLineId = fields[4].GetInt32();
            reward->SkillPointCount = fields[5].GetUInt32();
            reward->ArenaPointCount = fields[6].GetUInt32();
            reward->HonorPointCount = fields[7].GetUInt32();
            reward->Money = fields[8].GetUInt64();
            reward->Xp = fields[9].GetUInt32();
            reward->SpellID = fields[10].GetInt32();
            ++rewardCount;

            if (reward->TitleId && !sCharTitlesStore.LookupEntry(reward->TitleId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward` references non-existing Title %d for ChoiceId %d, ResponseId: %d, set to 0", reward->TitleId, choiceId, responseId);
                reward->TitleId = 0;
            }

            if (reward->PackageId && !sDB2Manager.GetQuestPackageItems(reward->PackageId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward` references non-existing QuestPackage %d for ChoiceId %d, ResponseId: %d, set to 0", reward->TitleId, choiceId, responseId);
                reward->PackageId = 0;
            }

            if (reward->SkillLineId && !sSkillLineStore.LookupEntry(reward->SkillLineId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward` references non-existing SkillLine %d for ChoiceId %d, ResponseId: %d, set to 0", reward->TitleId, choiceId, responseId);
                reward->SkillLineId = 0;
                reward->SkillPointCount = 0;
            }

        } while (rewards->NextRow());
    }

    if (auto rewards = WorldDatabase.Query("SELECT ChoiceId, ResponseId, ItemId, BonusListIDs, Quantity FROM playerchoice_response_reward_item ORDER BY `Index` ASC"))
    {
        do
        {
            auto fields = rewards->Fetch();

            auto choiceId = fields[0].GetInt32();
            auto responseId = fields[1].GetInt32();
            auto itemId = fields[2].GetUInt32();
            Tokenizer bonusListIDsTok(fields[3].GetString(), ' ');
            std::vector<uint32> bonusListIds;
            for (auto token : bonusListIDsTok)
                bonusListIds.push_back(uint32(atol(token)));
            auto quantity = fields[4].GetInt32();

            auto choice = Trinity::Containers::MapGetValuePtr(_playerChoices, choiceId);
            if (!choice)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_item` references non-existing ChoiceId: %d (ResponseId: %d), skipped", choiceId, responseId);
                continue;
            }

            auto responseItr = std::find_if(choice->Responses.begin(), choice->Responses.end(), [responseId](PlayerChoiceResponse const& playerChoiceResponse) { return playerChoiceResponse.ResponseId == responseId; });
            if (responseItr == choice->Responses.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_item` references non-existing ResponseId: %d for ChoiceId %d, skipped", responseId, choiceId);
                continue;
            }

            if (!responseItr->Reward)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_item` references non-existing player choice reward for ChoiceId %d, ResponseId: %d, skipped", choiceId, responseId);
                continue;
            }

            if (!GetItemTemplate(itemId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_item` references non-existing item %u for ChoiceId %d, ResponseId: %d, skipped", itemId, choiceId, responseId);
                continue;
            }

            responseItr->Reward->Items.emplace_back(itemId, std::move(bonusListIds), quantity);

        } while (rewards->NextRow());
    }

    if (auto rewards = WorldDatabase.Query("SELECT ChoiceId, ResponseId, CurrencyId, Quantity FROM playerchoice_response_reward_currency ORDER BY `Index` ASC"))
    {
        do
        {
            auto fields = rewards->Fetch();

            auto choiceId = fields[0].GetInt32();
            auto responseId = fields[1].GetInt32();
            auto currencyId = fields[2].GetUInt32();
            auto quantity = fields[3].GetInt32();

            auto choice = Trinity::Containers::MapGetValuePtr(_playerChoices, choiceId);
            if (!choice)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_currency` references non-existing ChoiceId: %d (ResponseId: %d), skipped", choiceId, responseId);
                continue;
            }

            auto responseItr = std::find_if(choice->Responses.begin(), choice->Responses.end(), [responseId](PlayerChoiceResponse const& playerChoiceResponse) { return playerChoiceResponse.ResponseId == responseId; });
            if (responseItr == choice->Responses.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_currency` references non-existing ResponseId: %d for ChoiceId %d, skipped", responseId, choiceId);
                continue;
            }

            if (!responseItr->Reward)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_currency` references non-existing player choice reward for ChoiceId %d, ResponseId: %d, skipped", choiceId, responseId);
                continue;
            }

            if (!sCurrencyTypesStore.LookupEntry(currencyId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_currency` references non-existing currency %u for ChoiceId %d, ResponseId: %d, skipped", currencyId, choiceId, responseId);
                continue;
            }

            responseItr->Reward->Currency.emplace_back(currencyId, quantity);

        } while (rewards->NextRow());
    }

    if (auto rewards = WorldDatabase.Query("SELECT ChoiceId, ResponseId, FactionId, Quantity FROM playerchoice_response_reward_faction ORDER BY `Index` ASC"))
    {
        do
        {
            auto fields = rewards->Fetch();

            auto choiceId = fields[0].GetInt32();
            auto responseId = fields[1].GetInt32();
            auto factionId = fields[2].GetUInt32();
            auto quantity = fields[3].GetInt32();

            auto choice = Trinity::Containers::MapGetValuePtr(_playerChoices, choiceId);
            if (!choice)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_faction` references non-existing ChoiceId: %d (ResponseId: %d), skipped", choiceId, responseId);
                continue;
            }

            auto responseItr = std::find_if(choice->Responses.begin(), choice->Responses.end(), [responseId](PlayerChoiceResponse const& playerChoiceResponse) { return playerChoiceResponse.ResponseId == responseId; });
            if (responseItr == choice->Responses.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_faction` references non-existing ResponseId: %d for ChoiceId %d, skipped", responseId, choiceId);
                continue;
            }

            if (!responseItr->Reward)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_faction` references non-existing player choice reward for ChoiceId %d, ResponseId: %d, skipped", choiceId, responseId);
                continue;
            }

            if (!sFactionStore.LookupEntry(factionId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_response_reward_faction` references non-existing faction %u for ChoiceId %d, ResponseId: %d, skipped", factionId, choiceId, responseId);
                continue;
            }

            responseItr->Reward->Faction.emplace_back(factionId, quantity);

        } while (rewards->NextRow());
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded " SZFMTD " player choices, %u responses, %u rewards, %u item rewards, %u currency rewards and %u faction rewards in %u ms.",
        _playerChoices.size(), responseCount, rewardCount, itemRewardCount, currencyRewardCount, factionRewardCount, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadPlayerChoicesLocale()
{
    auto oldMSTime = getMSTime();

    _playerChoiceLocales.clear();

    //                                                   0         1       2
    if (auto result = WorldDatabase.Query("SELECT ChoiceId, locale, Question FROM playerchoice_locale"))
    {
        do
        {
            auto fields = result->Fetch();

            auto choiceId = fields[0].GetUInt32();
            auto localeName = fields[1].GetString();

            if (!GetPlayerChoice(choiceId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_locale` references non-existing ChoiceId: %d for locale %s, skipped", choiceId, localeName.c_str());
                continue;
            }

            auto locale = GetLocaleByName(localeName);
            if (locale == LOCALE_enUS)
                continue;

            auto& data = _playerChoiceLocales[choiceId];
            AddLocaleString(fields[2].GetString(), locale, data.Question);
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded " SZFMTD " Player Choice locale strings in %u ms", _playerChoiceLocales.size(), GetMSTimeDiffToNow(oldMSTime));
    }

    oldMSTime = getMSTime();

    //                                                   0         1           2       3       4       5            6
    if (auto result = WorldDatabase.Query("SELECT ChoiceID, ResponseID, locale, Header, Answer, Description, Confirmation FROM playerchoice_response_locale"))
    {
        std::size_t count = 0;
        do
        {
            auto fields = result->Fetch();

            auto choiceId = fields[0].GetInt32();
            auto responseId = fields[1].GetInt32();
            auto localeName = fields[2].GetString();

            auto itr = _playerChoiceLocales.find(choiceId);
            if (itr == _playerChoiceLocales.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_locale` references non-existing ChoiceId: %d for ResponseId %d locale %s, skipped", choiceId, responseId, localeName.c_str());
                continue;
            }

            auto playerChoice = ASSERT_NOTNULL(GetPlayerChoice(choiceId));
            if (!playerChoice->GetResponse(responseId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `playerchoice_locale` references non-existing ResponseId: %d for ChoiceId %d locale %s, skipped", responseId, choiceId, localeName.c_str());
                continue;
            }

            auto locale = GetLocaleByName(localeName);
            if (locale == LOCALE_enUS)
                continue;

            auto& data = itr->second.Responses[responseId];
            AddLocaleString(fields[3].GetString(), locale, data.Header);
            AddLocaleString(fields[4].GetString(), locale, data.Answer);
            AddLocaleString(fields[5].GetString(), locale, data.Description);
            AddLocaleString(fields[6].GetString(), locale, data.Confirmation);
            ++count;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded " SZFMTD " Player Choice Response locale strings in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
}

void ObjectMgr::LoadGameObjectActionData()
{
    _gameObjectActionMap.clear();

    //                                                  0           1           2              3              4       5    6    7    8     9
    QueryResult result = WorldDatabase.Query("SELECT `entry`, `ActionType`, `SpellID`, `WorldSafeLocID`, `Distance`, `X`, `Y`, `Z`, `O`, MapID FROM `gameobject_action`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            GameObjectActionData data;
            uint32 entry = fields[i++].GetUInt32();
            data.ActionType = fields[i++].GetUInt8();
            data.SpellID = fields[i++].GetUInt32();
            data.WorldSafeLocID = fields[i++].GetUInt32();
            data.Distance = fields[i++].GetUInt32();
            data.X = fields[i++].GetFloat();
            data.Y = fields[i++].GetFloat();
            data.Z = fields[i++].GetFloat();
            data.O = fields[i++].GetFloat();
            data.MapID = fields[i++].GetUInt32();

            _gameObjectActionMap[entry].push_back(data);
            ++counter;
        }
        while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u gameobject_action data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 gameobject_action data. DB table `gameobject_action` is empty.");
}

ScenarioData const* ObjectMgr::GetScenarioOnMap(uint32 mapId, uint32 difficultyID /*= 0*/, uint32 team /*= 0*/, uint32 pl_class /*= 0*/, uint32 dungeonID /*= 0*/) const
{
    auto const& itr = _scenarioDataList.find(mapId);
    if (itr == _scenarioDataList.end())
        return nullptr;

    bool find = false;
    ScenarioData const* data = nullptr;
    for (auto const& v : itr->second)
    {
        if (!find)
            data = v;

        if (v->LfgDungeonID)
            if (v->LfgDungeonID == dungeonID)
                return v;

        if (v->DifficultyID && v->DifficultyID == difficultyID)
        {
            if (v->Team)
            {
                if (v->Team == team)
                {
                    if (v->Class)
                    {
                        if (v->Class == pl_class)
                        {
                            data = v;
                            break;
                        }
                        continue;
                    }
                    data = v;
                    break;
                }
                continue;
            }
            if (v->Class)
            {
                if (v->Class == pl_class)
                {
                    data = v;
                    break;
                }
            }
            else
            {
                data = v;
                find = true;
            }
        }
        
        if(!v->DifficultyID)
        {
            if (v->Team && v->Team == team)
            {
                if (v->Class)
                {
                    if (v->Class == pl_class)
                    {
                        data = v;
                        break;
                    }
                }
                else
                {
                    data = v;
                    find = true;
                }
            }
            if (!v->Team)
            {
                if (v->Class)
                {
                    if (v->Class == pl_class)
                    {
                        data = v;
                        break;
                    }
                }
                else
                {
                    data = v;
                    find = true;
                }
            }
        }
    }

    return data;
}

std::vector<uint32> ObjectMgr::GetItemBonusTree(uint32 ItemID, uint32 itemBonusTreeMod, uint32 ownerLevel, int32 levelBonus, int32 challengeLevel, int32 needLevel, bool onlyBonus)
{
    ItemTemplate const* pProto = GetItemTemplate(ItemID);
    std::vector<uint32> bonusListIDs = GetItemBonusForLevel(ItemID, itemBonusTreeMod, ownerLevel, needLevel);
    if (!pProto)
        return bonusListIDs;

    if (pProto->GetQuality() < ITEM_QUALITY_LEGENDARY)
    {
        if (challengeLevel)
            ReplaceChallengeLevel(bonusListIDs, challengeLevel);

        uint8 Quality = 0;
        int8 canSecondStat = 3;
        bool sosketSlot1 = true;
        bool sosketSlot2 = true;
        bool sosketSlot3 = true;
        if (!onlyBonus && pProto->HasStats() && roll_chance_f(10.0f)) // Genarate second stats
        {
            if (uint32 statID = GetItemThirdStat(ItemID, sosketSlot1, sosketSlot2, sosketSlot3))
                bonusListIDs.push_back(statID);
            canSecondStat--;
        }

        if (!onlyBonus && pProto->CanWarforged() && roll_chance_i(ItemBonus::Chances::Warforged)) // Chance for proc additional bonus
        {
            uint32 baseLevel = pProto->ItemLevel;
            uint32 bonusLevel = GetItemBonusLevel(ItemID, ownerLevel, Quality, bonusListIDs) + levelBonus;

            DeleteBonusUpLevel(bonusListIDs);

            int32 delta = (bonusLevel - baseLevel) + 5;
            int32 countBonus = 1;
            while(true)
            {
                if (roll_chance_i(ItemBonus::Chances::NextChance))
                {
                    delta += 5;
                    countBonus++;
                    if (canSecondStat > 0 && !onlyBonus && pProto->GetClass() != ITEM_CLASS_GEM && roll_chance_f(10.0f)) // Genarate second stats
                    {
                        canSecondStat--;
                        if (uint32 statID = GetItemThirdStat(ItemID, sosketSlot1, sosketSlot2, sosketSlot3))
                            bonusListIDs.push_back(statID);
                    }
                }
                else
                    break;
                if ((baseLevel + delta) >= sWorld->getIntConfig(CONFIG_MAX_ITEM_LEVEL)) // Need calculate exist bonus befor check
                    break;
            }

            if (auto bonus = sDB2Manager.GetItemBonusListForItemLevelDelta(delta))
                bonusListIDs.emplace_back(bonus);

            if (pProto->GetQuality() == ITEM_QUALITY_EPIC)
            {
                if (countBonus > 2)
                    bonusListIDs.push_back(ItemBonus::Bonuses::TitanforgedEpic);
                else
                    bonusListIDs.push_back(ItemBonus::Bonuses::WarforgedEpic);
            }
            else
            {
                if (urand(0, 100) > 50)
                {
                    if (countBonus > 2)
                        bonusListIDs.push_back(ItemBonus::Bonuses::TitanforgedBlue);
                    else
                        bonusListIDs.push_back(ItemBonus::Bonuses::WarforgedBlue);
                }
                else
                {
                    if (countBonus > 2)
                        bonusListIDs.push_back(ItemBonus::Bonuses::TitanforgedEpic);
                    else
                        bonusListIDs.push_back(ItemBonus::Bonuses::WarforgedEpic);
                }
            }
        }
        else if (levelBonus)
        {
            uint32 bonusLevel = GetItemBonusLevel(ItemID, ownerLevel, Quality, bonusListIDs) + levelBonus;

            DeleteBonusUpLevel(bonusListIDs);

            int16 delta = bonusLevel - pProto->ItemLevel;
            if (auto bonus = sDB2Manager.GetItemBonusListForItemLevelDelta(delta))
                bonusListIDs.emplace_back(bonus);

            if (bonusLevel >= 845 && pProto->GetQuality() != ITEM_QUALITY_ARTIFACT) // May be other?
                bonusListIDs.push_back(3528); // Epic
        }
    }

    DeleteBugBonus(bonusListIDs, pProto->HasEnchantment(), pProto->GetQuality() < ITEM_QUALITY_LEGENDARY, pProto->HasStats());

    return bonusListIDs;
}

std::vector<uint32> ObjectMgr::GetItemBonusForLevel(uint32 ItemID, uint32 itemBonusTreeMod, uint32 ownerLevel, int32 needLevel)
{
    ItemTemplate const* pProto = GetItemTemplate(ItemID);
    uint32 itemLevelFromBonus = 0;

    std::vector<uint32> bonusListIDs = sDB2Manager.GetItemBonusTree(ItemID, itemBonusTreeMod, itemLevelFromBonus);
    if (!needLevel)
        needLevel = itemLevelFromBonus;

    if (!pProto || !needLevel)
        return bonusListIDs;

    if (pProto->GetQuality() != ITEM_QUALITY_LEGENDARY && (pProto->HasStats() || (pProto->GetClass() == ITEM_CLASS_GEM && pProto->GetSubClass() == ITEM_SUBCLASS_GEM_ARTIFACT_RELIC)))
    {
        DeleteBonusUpLevel(bonusListIDs);

        if (needLevel >= 845 && pProto->GetQuality() != ITEM_QUALITY_ARTIFACT) // May be other?
            bonusListIDs.push_back(3528); // Epic

        int16 delta = needLevel - pProto->ItemLevel;
        if (auto bonus = sDB2Manager.GetItemBonusListForItemLevelDelta(delta))
            bonusListIDs.emplace_back(bonus);
    }

    DeleteBugBonus(bonusListIDs, pProto->HasEnchantment(), pProto->GetQuality() < ITEM_QUALITY_LEGENDARY, pProto->HasStats());

    return bonusListIDs;
}

uint32 ObjectMgr::ReplaceChallengeLevel(std::vector<uint32>& bonusListIDs, int32 challengeLevel)
{
    uint32 replaceFrom = 3410;
    uint32 replaceTo;
    switch (challengeLevel)
    {
        case 0:
        case 1:
        case 2:
            replaceTo = 3411;
            break;
        case 3:
            replaceTo = 3411;
            break;
        case 4:
            replaceTo = 3412;
            break;
        case 5:
            replaceTo = 3413;
            break;
        case 6:
            replaceTo = 3414;
            break;
        case 7:
            replaceTo = 3415;
            break;
        case 8:
            replaceTo = 3416;
            break;
        case 9:
            replaceTo = 3417;
            break;
        case 10:
            replaceTo = 3418;
            break;
        case 11:
            replaceTo = 3509;
            break;
        case 12:
            replaceTo = 3510;
            break;
        case 13:
            replaceTo = 3534;
            break;
        case 14:
            replaceTo = 3535;
            break;
        case 15:
        default:
            replaceTo = 3536;
            break;
    }

    for (uint32 index = 0; index < bonusListIDs.size(); ++index)
        if (bonusListIDs[index] == replaceFrom)
            bonusListIDs[index] = replaceTo;

    return 0;
}

uint32 ObjectMgr::FindChallengeLevel(std::vector<uint32>& bonusListIDs)
{
    uint32 challengeLevel = 0;
    for (uint32 index = 0; index < bonusListIDs.size(); ++index)
    {
        switch (bonusListIDs[index])
        {
            case 3410:
                challengeLevel = 2;
                break;
            case 3411:
                challengeLevel = 3;
                break;
            case 3412:
                challengeLevel = 4;
                break;
            case 3413:
                challengeLevel = 5;
                break;
            case 3414:
                challengeLevel = 6;
                break;
            case 3415:
                challengeLevel = 7;
                break;
            case 3416:
                challengeLevel = 8;
                break;
            case 3417:
                challengeLevel = 9;
                break;
            case 3418:
                challengeLevel = 10;
                break;
            case 3509:
                challengeLevel = 11;
                break;
            case 3510:
                challengeLevel = 12;
                break;
            case 3534:
                challengeLevel = 13;
                break;
            case 3535:
                challengeLevel = 14;
                break;
            case 3536:
                challengeLevel = 15;
                break;
            default:
                break;
        }
        bonusListIDs[index] = 0;
    }

    return challengeLevel;
}

void ObjectMgr::DeleteBonusUpLevel(std::vector<uint32>& bonusListIDs)
{
    // Is bonus = +0, 1372 = -100, 1672 = +200, 3329 = +400, 3130 = +201, 3128 = -101, 2829 = -400
    for (uint32 index = 0; index < bonusListIDs.size(); ++index)
        if ((bonusListIDs[index] >= 1372 && bonusListIDs[index] <= 1672) || (bonusListIDs[index] >= 2829 && bonusListIDs[index] <= 3329))
            bonusListIDs[index] = 0; // Clear after
}

void ObjectMgr::DeleteBugBonus(std::vector<uint32>& bonusListIDs, bool canSecond, bool canThird, bool hasStat)
{
    // Is bonus = +0, 1372 = -100, 1672 = +200, 3329 = +400, 3130 = +201, 3128 = -101, 2829 = -400
    bool forged = false;
    bool qualitie = false;
    bool level = false;
    bool enchant = false;
    for (uint32 index = 0; index < bonusListIDs.size(); ++index)
    {
        if (bonusListIDs[index] == 3527 || bonusListIDs[index] == 3528)
        {
            if (qualitie)
                bonusListIDs[index] = 0;
            else
                qualitie = true;
        }
        if (bonusListIDs[index] == 3336 || bonusListIDs[index] == 3337 || bonusListIDs[index] == 3338 || bonusListIDs[index] == 3339)
        {
            if (forged)
                bonusListIDs[index] = 0;
            else
                forged = true;
        }
        if ((bonusListIDs[index] >= 1372 && bonusListIDs[index] <= 1672) || (bonusListIDs[index] >= 2829 && bonusListIDs[index] <= 3329))
        {
            if (level)
                bonusListIDs[index] = 0;
            else
                level = true;
        }
        if (!canSecond)
        {
            if (bonusListIDs[index] >= 1676 && bonusListIDs[index] <= 1721)
                bonusListIDs[index] = 0;
        }
        else
        {
            if (bonusListIDs[index] >= 1676 && bonusListIDs[index] <= 1721)
            {
                if (enchant)
                    bonusListIDs[index] = 0;
                else
                    enchant = true;
            }
        }
        if (!canThird)
        {
            if (bonusListIDs[index] >= 40 && bonusListIDs[index] <= 42)
                bonusListIDs[index] = 0;
            if (bonusListIDs[index] == 1808)
                bonusListIDs[index] = 0;
            if (bonusListIDs[index] >= 1676 && bonusListIDs[index] <= 1721)
                bonusListIDs[index] = 0;
        }
        if (!hasStat)
        {
            if (bonusListIDs[index] >= 40 && bonusListIDs[index] <= 42)
                bonusListIDs[index] = 0;
            if (bonusListIDs[index] == 1808)
                bonusListIDs[index] = 0;
        }
    }

    // Clear bug bonus
    std::vector<uint32> bonusListID;
    for (uint32 index = 0; index < bonusListIDs.size(); ++index)
        if (bonusListIDs[index])
            bonusListID.push_back(bonusListIDs[index]);

    bonusListIDs = bonusListID;
}

bool ObjectMgr::CheckDuplicateBonusUpLevel(std::vector<uint32>& bonusListIDs)
{
    bool bonus = false;
    bool duplicate = false;
    // Is bonus = +0, 1372 = -100, 1672 = +200, 3329 = +400, 3130 = +201, 3128 = -101, 2829 = -400
    for (uint32 index = 0; index < bonusListIDs.size(); ++index)
        if ((bonusListIDs[index] >= 1372 && bonusListIDs[index] <= 1672) || (bonusListIDs[index] >= 2829 && bonusListIDs[index] <= 3329))
        {
            if (bonus)
                duplicate = true;
            bonus = true;
        }
    return duplicate;
}

uint32 ObjectMgr::GetItemBonusLevel(uint32 ItemID, uint32 ownerLevel, uint8& Quality, std::vector<uint32>& bonusListIDs)
{
    ItemTemplate const* pProto = GetItemTemplate(ItemID);
    if (!pProto)
        return 0;

    std::set<uint32> bonusSetIDs;
    for (uint32 bonusListID : bonusListIDs)
        bonusSetIDs.insert(bonusListID);

    std::set<uint32> copyBonusSetIDs(bonusSetIDs);

    Quality = pProto->GetQuality();
    uint32 itemLevel = pProto->ItemLevel;
    uint32 ScalingStatDistribution = pProto->GetScalingStatDistribution();
    int32 ItemLevelBonus = 0;

    //int32 ItemLevelOverridePriority = std::numeric_limits<int32>::max();
    int32 ScalingStatDistributionPriority = std::numeric_limits<int32>::max();
    bool HasQualityBonus = false;

    for (uint32 bonusListID : copyBonusSetIDs)
    {
        if (DB2Manager::ItemBonusList const* bonuses = sDB2Manager.GetItemBonusList(bonusListID))
        {
            for (ItemBonusEntry const* bonus : *bonuses)
            {
                switch (bonus->Type)
                {
                    case ITEM_BONUS_ITEM_LEVEL:
                        ItemLevelBonus += bonus->Value[0];
                        break;
                    case ITEM_BONUS_QUALITY:
                        if (!HasQualityBonus)
                        {
                            Quality = static_cast<uint32>(bonus->Value[0]);
                            HasQualityBonus = true;
                        }
                        else if (Quality < static_cast<uint32>(bonus->Value[0]))
                            Quality = static_cast<uint32>(bonus->Value[0]);
                        break;
                    case ITEM_BONUS_SCALING_STAT_DISTRIBUTION:
                        if (bonus->Value[1] < ScalingStatDistributionPriority)
                        {
                            ScalingStatDistribution = static_cast<uint32>(bonus->Value[0]);
                            ScalingStatDistributionPriority = bonus->Value[1];
                        }
                        bonusSetIDs.erase(bonusListID);
                        break;
                    case ITEM_BONUS_SCALING_STAT_DISTRIBUTION_FIXED:
                        if (bonus->Value[1] < ScalingStatDistributionPriority)
                        {
                            ScalingStatDistribution = static_cast<uint32>(bonus->Value[0]);
                            ScalingStatDistributionPriority = bonus->Value[1];
                        }
                        bonusSetIDs.erase(bonusListID);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if (ScalingStatDistributionEntry const* ssd = sScalingStatDistributionStore.LookupEntry(ScalingStatDistribution))
    {
        if (uint32 scaleLvl = uint32(sDB2Manager.GetCurveValueAt(ssd->PlayerLevelToItemLevelCurveID, ownerLevel)))
            itemLevel = scaleLvl;
    }

    itemLevel += ItemLevelBonus;

    bonusListIDs.clear();
    for (uint32 bonusListID : bonusSetIDs)
        bonusListIDs.push_back(bonusListID);

    return std::min(std::max(itemLevel, uint32(MIN_ITEM_LEVEL)), uint32(MAX_ITEM_LEVEL));
}

const std::vector<GameObjectActionData>* ObjectMgr::GetGameObjectActionData(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_gameObjectActionMap, entry);
}

PlayerChoice const* ObjectMgr::GetPlayerChoice(int32 choiceId) const
{
    return Trinity::Containers::MapGetValuePtr(_playerChoices, choiceId);
}

PlayerChoiceLocale const* ObjectMgr::GetPlayerChoiceLocale(int32 ChoiceID) const
{
    auto itr = _playerChoiceLocales.find(ChoiceID);
    if (itr == _playerChoiceLocales.end()) return nullptr;
    return &itr->second;
}

const std::vector<CreatureActionData>* ObjectMgr::GetCreatureActionData(uint32 entry, uint8 action) const
{
    return Trinity::Containers::MapGetValuePtr(_creatureActionDataList[action], entry);
}

const SeamlessTeleportData* ObjectMgr::GetSeamlessTeleportZone(uint16 zoneId) const
{
    return Trinity::Containers::MapGetValuePtr(_seamlessZoneTeleport, zoneId);
}

const SeamlessTeleportData* ObjectMgr::GetSeamlessTeleportArea(uint16 areaId) const
{
    return Trinity::Containers::MapGetValuePtr(_seamlessAreaTeleport, areaId);
}

uint8 ObjectMgr::GetCountFromSpawn(uint8 spawnmode, uint32 expansion)
{
    switch (spawnmode)
    {
        case DIFFICULTY_NONE:
        case DIFFICULTY_NORMAL:
        case DIFFICULTY_HEROIC:
            return 1;
        case DIFFICULTY_10_N:
        case DIFFICULTY_10_HC:
            return 2;
        case DIFFICULTY_25_N:
        case DIFFICULTY_25_HC:
        case DIFFICULTY_40:
            return 4;
        case DIFFICULTY_LFR:
        case DIFFICULTY_MYTHIC_KEYSTONE:
            return 3;
        case DIFFICULTY_NORMAL_RAID:
            if (expansion == EXPANSION_THE_BURNING_CRUSADE || expansion == EXPANSION_WRATH_OF_THE_LICH_KING)
                return 3;
        default:
            break;
    }

    return 1;
}

void ObjectMgr::LoadRaceAndClassExpansionRequirements()
{
    uint32 oldMSTime = getMSTime();
    _raceUnlockRequirementStore.clear();

    //                                               0       1
    QueryResult result = WorldDatabase.Query("SELECT raceID, expansion, achievementId FROM `race_unlock_requirement`");

    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 raceID = fields[0].GetInt8();
            uint8 expansion = fields[1].GetInt8();
            uint32 achievementId = fields[2].GetUInt32();

            auto raceEntry = sChrRacesStore.LookupEntry(raceID);
            if (!raceEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Race %u defined in `race_unlock_requirement` does not exists, skipped.", raceID);
                continue;
            }

            if (expansion >= MAX_EXPANSIONS)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Race %u defined in `race_unlock_requirement` has incorrect expansion %u, skipped.", raceID, expansion);
                continue;
            }

            auto& raceUnlockRequirement = _raceUnlockRequirementStore[raceID];
            raceUnlockRequirement.Expansion = expansion;
            raceUnlockRequirement.AchievementId = achievementId;

            ++count;
        }
        while (result->NextRow());
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u race expansion requirements in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 race expansion requirements. DB table `race_unlock_requirement` is empty.");

    oldMSTime = getMSTime();
    _classExpansionRequirementStore.clear();

    //                                   0        1
    result = WorldDatabase.Query("SELECT classID, expansion FROM `class_expansion_requirement`");

    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 classID = fields[0].GetInt8();
            uint8 expansion = fields[1].GetInt8();

            ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(classID);
            if (!classEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Class %u defined in `class_expansion_requirement` does not exists, skipped.", classID);
                continue;
            }

            if (expansion >= MAX_EXPANSIONS)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Class %u defined in `class_expansion_requirement` has incorrect expansion %u, skipped.", classID, expansion);
                continue;
            }
            _classExpansionRequirementStore[classID] = expansion;

            ++count;
        }
        while (result->NextRow());
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u class expansion requirements in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 class expansion requirements. DB table `class_expansion_requirement` is empty.");
}

void ObjectMgr::LoadRealmNames()
{
    uint32 oldMSTime = getMSTime();
    _realmNameStore.clear();

    //                                               0   1
    QueryResult result = LoginDatabase.Query("SELECT id, name FROM `realmlist`");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 realm names. DB table `realmlist` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 realm = fields[0].GetUInt32();
        std::string realmName = fields[1].GetString();

        _realmNameStore[realm] = realmName;

        ++count;
    }
    while (result->NextRow());
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u realm names in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

std::string ObjectMgr::GetRealmName(uint32 realm) const
{
    RealmNameContainer::const_iterator iter = _realmNameStore.find(realm);
    return iter != _realmNameStore.end() ? iter->second : "";
}

std::string ObjectMgr::GetNormalizedRealmName(uint32 realmId) const
{
    std::string name = GetRealmName(realmId);
    name.erase(std::remove_if(name.begin(), name.end(), ::isspace), name.end());
    return name;
}

std::string ObjectMgr::GetRealCharName(std::string name)
{
    std::string::size_type n = name.find("-");
    if (n == std::string::npos)
        return name;
    return name.substr(0, n);
}

RaceUnlockRequirement const* ObjectMgr::GetRaceUnlockRequirement(uint8 race) const
{
    return Trinity::Containers::MapGetValuePtr(_raceUnlockRequirementStore, race);
}

ExpansionRequirementContainer const& ObjectMgr::GetClassExpansionRequirements() const
{
    return _classExpansionRequirementStore;
}

uint8 ObjectMgr::GetClassExpansionRequirement(uint8 class_) const
{
    auto itr = _classExpansionRequirementStore.find(class_);
    if (itr != _classExpansionRequirementStore.end())
        return itr->second;
    return EXPANSION_CLASSIC;
}

uint8 ObjectMgr::GetboundTypeFromDifficulty(uint8 difficulty)
{
    switch (difficulty)
    {
    case DIFFICULTY_MYTHIC_DUNGEON:
    case DIFFICULTY_MYTHIC_RAID:
        return 2;
    case DIFFICULTY_HEROIC:
    case DIFFICULTY_10_HC:
    case DIFFICULTY_25_HC:
    case DIFFICULTY_HEROIC_RAID:
        return 1;
    default:
        return 0;
    }
}


void ObjectMgr::LoadSkillTiers()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT ID, Value1, Value2, Value3, Value4, Value5, Value6, Value7, Value8, Value9, Value10, "
                                             " Value11, Value12, Value13, Value14, Value15, Value16 FROM skill_tiers");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        SkillTiersEntry& tier = _skillTiers[fields[0].GetUInt32()];
        for (uint32 i = 0; i < MAX_SKILL_STEP; ++i)
            tier.Value[i] = fields[1 + i].GetUInt32();

    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u skill max values in %u ms", uint32(_skillTiers.size()), GetMSTimeDiffToNow(oldMSTime));
}

uint32 ObjectMgr::GetRandomLegendaryItem(Player* lootOwner) const
{
    if (!lootOwner)
        return 0;

    ItemTemplate const* LegendaryItem = nullptr;
    std::vector<ItemTemplate const*> singleSpec;
    std::vector<ItemTemplate const*> multiSpec;
    std::vector<ItemTemplate const*> otherSpec;
    std::vector<ItemTemplate const*> singleClass;
    std::vector<ItemTemplate const*> multiClass;
    std::vector<ItemTemplate const*> otherClass;
    std::vector<ItemTemplate const*> tokens;

    for (auto proto : LegendaryItemList)
    {
        if (!proto)
            continue;

        if (lootOwner->AllLegendarys.find(proto->GetId()) != lootOwner->AllLegendarys.end())
            continue;

        // Class check
        if (lootOwner->CanGetItemForLoot(proto, false))
        {
            if (proto->IsMultiClass)
                multiClass.emplace_back(proto);
            else if (proto->IsSingleClass)
                singleClass.emplace_back(proto);
            else
                otherClass.emplace_back(proto);
        }

        // Speck check
        if (lootOwner->CanGetItemForLoot(proto))
        {
            if (proto->IsMultiClass)
                multiSpec.emplace_back(proto);
            else if (proto->IsSingleClass)
                singleSpec.emplace_back(proto);
            else
                otherSpec.emplace_back(proto);
        }
    }

    if (!multiSpec.empty())
        LegendaryItem = Trinity::Containers::SelectRandomContainerElement(multiSpec);
    else if (!otherSpec.empty())
        LegendaryItem = Trinity::Containers::SelectRandomContainerElement(otherSpec);
    else if (!singleSpec.empty())
        LegendaryItem = Trinity::Containers::SelectRandomContainerElement(singleSpec);

    // If item not found for selected spec, get random item for this class
    if (!LegendaryItem)
    {
        if (!multiClass.empty())
            LegendaryItem = Trinity::Containers::SelectRandomContainerElement(multiClass);
        else if (!otherClass.empty())
            LegendaryItem = Trinity::Containers::SelectRandomContainerElement(otherClass);
        else if (!singleClass.empty())
            LegendaryItem = Trinity::Containers::SelectRandomContainerElement(singleClass);
    }

    // Enable in full 7.3.5
    if (!LegendaryItem) // If all legendary in this char, generate for other class
    {
        uint32 classMask = lootOwner->GetSession() ? lootOwner->GetSession()->m_classMask : 0;
        for (auto proto : LegendaryTokenList)
        {
            if (!proto)
                continue;

            if (proto->AllowableClass & classMask)
                tokens.emplace_back(proto);
        }
        if (!tokens.empty())
            LegendaryItem = Trinity::Containers::SelectRandomContainerElement(tokens);
    }

    return LegendaryItem ? LegendaryItem->GetId() : 0;
}

void ObjectMgr::LoadCreatureOutfits()
{
    uint32 oldMSTime = getMSTime();

    _creatureOutfitStore.clear();   // for reload case (test only)

    QueryResult result = WorldDatabase.Query("SELECT entry, race, class, gender, skin, face, hair, haircolor, facialhair, feature1, feature2, feature3, "
        "head, shoulders, body, chest, waist, legs, feet, wrists, hands, tabard, back FROM creature_template_outfits");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creature outfits. DB table `creature_template_outfits` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 i = 0;
        uint32 entry   = fields[i++].GetUInt32();

        CreatureOutfit co;

        co.race         = fields[i++].GetUInt8();
        const ChrRacesEntry* rEntry = sChrRacesStore.LookupEntry(co.race);
        if (!rEntry)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Outfit entry %u in `creature_template_outfits` has incorrect race (%u).", entry, uint32(co.race));
            continue;
        }

        co.Class = fields[i++].GetUInt8();
        const ChrClassesEntry* cEntry = sChrClassesStore.LookupEntry(co.Class);
        if (!cEntry)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Outfit entry %u in `creature_template_outfits` has incorrect class (%u).", entry, uint32(co.Class));
            continue;
        }

        co.gender       = fields[i++].GetUInt8();
        switch (co.gender)
        {
        case GENDER_FEMALE: co.displayId = rEntry->FemaleDisplayID; break;
        case GENDER_MALE:   co.displayId = rEntry->MaleDisplayID; break;
        default:
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Outfit entry %u in `creature_template_outfits` has invalid gender %u", entry, uint32(co.gender));
            continue;
        }

        co.skin         = fields[i++].GetUInt8();
        co.face         = fields[i++].GetUInt8();
        co.hair         = fields[i++].GetUInt8();
        co.haircolor    = fields[i++].GetUInt8();
        co.facialhair   = fields[i++].GetUInt8();
        for (uint32 j = 0; j < CreatureOutfit::max_custom_displays; ++j)
            co.customdisplay[j] = fields[i++].GetUInt8();
        for (uint32 j = 0; j < CreatureOutfit::max_outfit_displays; ++j)
        {
            int64 displayInfo = fields[i + j].GetInt64();
            if (displayInfo > 0) // entry
            {
                uint32 item_entry = static_cast<uint32>(displayInfo & 0xFFFFFFFF);
                uint32 appearancemodid = static_cast<uint32>(displayInfo >> 32);
                if (uint32 display = sDB2Manager.GetItemDisplayId(item_entry, appearancemodid))
                    co.outfit[j] = display;
                else
                {
                    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Creature entry %u in `creature_template_outfits` has invalid (item entry, appearance) combination: %u, %u. Value in DB: %s", entry, item_entry, appearancemodid, std::to_string(displayInfo).c_str());
                    co.outfit[j] = 0;
                }
            }
            else // display
                co.outfit[j] = static_cast<uint32>(-displayInfo);
        }

        _creatureOutfitStore[entry] = co;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature outfits in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

uint32 ObjectMgr::GetCreatureDisplay(int32 modelid) const
{
    if (modelid >= 0)
        return modelid;

    const CreatureOutfitContainer& outfits = GetCreatureOutfitMap();
    CreatureOutfitContainer::const_iterator it = outfits.find(-modelid);
    if (it != outfits.end())
        return 11686; // invisible for mirror image

    return 0;
}


void ObjectMgr::LoadDeathMatchStore()
{ 
    // donate venodrs for Tokens    
    uint32 oldMSTime = getMSTime();
    
    _dmProducts.clear();
    _dmProductsById.clear();
    
    uint32 count = 0;  
    
    // float donateDiscount;
    
    QueryResult result = CharacterDatabase.PQuery("SELECT type, product, cost, name from character_deathmatch_products");
    
    if (result)
    {
        do
        {
            Field* field = result->Fetch();
            uint8 type = field[0].GetUInt8();
            if (type >= DM_TYPE_MAX)
                continue;

            std::string product = field[1].GetString();
            uint32 product_uint = 0;

            if (type == DM_TYPE_MORPH)
            {
                product_uint = atoi(product.c_str());
                if (!product_uint)
                    continue;
            }

            uint32 cost = field[2].GetUInt32();
            std::string name = field[3].GetString();

            if (product_uint != 0)
            {
                _dmProducts[type].push_back(DeathMatchStore(name, type, product_uint, cost, ++count));
                _dmProductsById[count].push_back(DeathMatchStore(name, type, product_uint, cost, count));
            }
            else
            {
                _dmProducts[type].push_back(DeathMatchStore(name, type, product, cost, ++count));
                _dmProductsById[count].push_back(DeathMatchStore(name, type, product, cost, count));
            }

        } while (result->NextRow());
    }
    
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %d DeathMatch Store products in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
