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

#include "AccountMgr.h"
#include "AchievementMgr.h"
#include "CalendarPackets.h"
#include "ChatPackets.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Guild.h"
#include <utility>
#include "GuildFinderMgr.h"
#include "GuildMgr.h"
#include "GuildPackets.h"
#include "ItemPackets.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "SocialMgr.h"
#include "SpellAuraEffects.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"

#define MAX_GUILD_BANK_TAB_TEXT_LEN 500
static int64 constexpr EMBLEM_PRICE = 10 * GOLD;

inline uint32 _GetGuildBankTabPrice(uint8 tabId)
{
    switch (tabId)
    {
        case 0: return 100;
        case 1: return 250;
        case 2: return 500;
        case 3: return 1000;
        case 4: return 2500;
        case 5: return 5000;
        default: return 0;
    }
}

void Guild::SendCommandResult(WorldSession* session, GuildCommandType type, GuildCommandError errCode, std::string const& param)
{
    WorldPackets::Guild::GuildCommandResult resultPacket;
    resultPacket.Command = type;
    resultPacket.Result = errCode;
    resultPacket.Name = param;
    if (Player* player = session->GetPlayer())
        player->SendDirectMessage(resultPacket.Write());
}

void Guild::SendSaveEmblemResult(WorldSession* session, GuildEmblemError errCode)
{
    WorldPackets::Guild::PlayerSaveGuildEmblem save;
    save.Error = int32(errCode);
    if (Player* player = session->GetPlayer())
        player->SendDirectMessage(save.Write());
}

Guild::LogHolder::LogHolder(uint32 maxRecords) : m_maxRecords(maxRecords), m_nextGUID(uint32(GUILD_EVENT_LOG_GUID_UNDEFINED))
{
}

Guild::LogHolder::~LogHolder()
{
    // Cleanup
    for (auto& itr : m_log)
        delete itr;
}

// Adds event loaded from database to collection
inline void Guild::LogHolder::LoadEvent(LogEntry* entry)
{
    if (m_nextGUID == uint32(GUILD_EVENT_LOG_GUID_UNDEFINED))
        m_nextGUID = entry->GetGUID();
    m_log.push_front(entry);
}

// Adds new event happened in game.
// If maximum number of events is reached, oldest event is removed from collection.
inline void Guild::LogHolder::AddEvent(SQLTransaction& trans, LogEntry* entry)
{
    // Check max records limit
    if (m_log.size() >= m_maxRecords)
    {
        LogEntry* oldEntry = m_log.front();
        delete oldEntry;
        m_log.pop_front();
    }
    // Add event to list
    m_log.push_back(entry);
    // Save to DB
    entry->SaveToDB(trans);
}

inline uint32 Guild::LogHolder::GetNextGUID()
{
    // Next guid was not initialized. It means there are no records for this holder in DB yet.
    // Start from the beginning.
    if (m_nextGUID == uint32(GUILD_EVENT_LOG_GUID_UNDEFINED))
        m_nextGUID = 0;
    else
        m_nextGUID = (m_nextGUID + 1) % m_maxRecords;
    return m_nextGUID;
}

Guild::GuildNewsLog::GuildNewsLog(Guild* guild) : _guild(guild)
{
}

GuildNewsEntry* Guild::GuildNewsLog::GetNewsById(uint32 id)
{
    return Trinity::Containers::MapGetValuePtr(_newsLog, id);
}

Guild* Guild::GuildNewsLog::GetGuild() const
{
    return _guild;
}

Guild::LogEntry::LogEntry(ObjectGuid::LowType guildId, uint32 guid) : m_guildId(guildId), m_guid(guid), m_timestamp(::time(nullptr))
{
}

Guild::LogEntry::LogEntry(ObjectGuid::LowType guildId, uint32 guid, time_t timestamp) : m_guildId(guildId), m_guid(guid), m_timestamp(timestamp)
{
}

Guild::EventLogEntry::EventLogEntry(ObjectGuid::LowType guildId, uint32 guid, GuildEventLogTypes eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank) :
    LogEntry(guildId, guid), m_eventType(eventType), m_playerGuid1(playerGuid1), m_playerGuid2(playerGuid2), m_newRank(newRank)
{
}

Guild::EventLogEntry::EventLogEntry(ObjectGuid::LowType guildId, uint32 guid, time_t timestamp, GuildEventLogTypes eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank) :
    LogEntry(guildId, guid, timestamp), m_eventType(eventType), m_playerGuid1(playerGuid1), m_playerGuid2(playerGuid2), m_newRank(newRank)
{
}

void Guild::EventLogEntry::SaveToDB(SQLTransaction& trans) const
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_EVENTLOG);
    stmt->setUInt64(0, m_guildId);
    stmt->setUInt32(1, m_guid);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);

    uint8 index = 0;
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_EVENTLOG);
    stmt->setUInt64(index, m_guildId);
    stmt->setUInt32(++index, m_guid);
    stmt->setUInt8(++index, uint8(m_eventType));
    stmt->setUInt64(++index, m_playerGuid1);
    stmt->setUInt64(++index, m_playerGuid2);
    stmt->setUInt8(++index, m_newRank);
    stmt->setUInt64(++index, m_timestamp);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

void Guild::EventLogEntry::WritePacket(WorldPackets::Guild::GuildEventLogQueryResults& packet) const
{
    WorldPackets::Guild::GuildEventEntry eventEntry;
    eventEntry.PlayerGUID = ObjectGuid::Create<HighGuid::Player>(m_playerGuid1);
    eventEntry.OtherGUID = ObjectGuid::Create<HighGuid::Player>(m_playerGuid2);
    eventEntry.TransactionType = uint8(m_eventType);
    eventEntry.TransactionDate = uint32(::time(nullptr) - m_timestamp);
    eventEntry.RankID = uint8(m_newRank);
    packet.Entry.push_back(eventEntry);
}

bool Guild::BankEventLogEntry::IsMoneyEvent(GuildBankEventLogTypes eventType)
{
    return eventType == GUILD_BANK_LOG_DEPOSIT_MONEY || eventType == GUILD_BANK_LOG_WITHDRAW_MONEY || eventType == GUILD_BANK_LOG_REPAIR_MONEY || eventType == GUILD_BANK_LOG_CASH_FLOW_DEPOSIT;
}

bool Guild::BankEventLogEntry::IsMoneyEvent() const
{
    return IsMoneyEvent(m_eventType);
}

Guild::BankEventLogEntry::BankEventLogEntry(ObjectGuid::LowType guildId, uint32 guid, GuildBankEventLogTypes eventType, uint8 tabId, ObjectGuid::LowType playerGuid, uint64 itemOrMoney, uint16 itemStackCount, uint8 destTabId) :
    LogEntry(guildId, guid), m_eventType(eventType), m_bankTabId(tabId), m_playerGuid(playerGuid),
    m_itemOrMoney(itemOrMoney), m_itemStackCount(itemStackCount), m_destTabId(destTabId)
{
}

Guild::BankEventLogEntry::BankEventLogEntry(ObjectGuid::LowType guildId, uint32 guid, time_t timestamp, uint8 tabId, GuildBankEventLogTypes eventType, ObjectGuid::LowType playerGuid, uint64 itemOrMoney, uint16 itemStackCount, uint8 destTabId) :
    LogEntry(guildId, guid, timestamp), m_eventType(eventType), m_bankTabId(tabId), m_playerGuid(playerGuid),
    m_itemOrMoney(itemOrMoney), m_itemStackCount(itemStackCount), m_destTabId(destTabId)
{
}

void Guild::BankEventLogEntry::SaveToDB(SQLTransaction& trans) const
{
    uint8 index = 0;

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_EVENTLOG);
    stmt->setUInt64(index, m_guildId);
    stmt->setUInt32(++index, m_guid);
    stmt->setUInt8(++index, m_bankTabId);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);

    index = 0;
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_BANK_EVENTLOG);
    stmt->setUInt64(index, m_guildId);
    stmt->setUInt32(++index, m_guid);
    stmt->setUInt8(++index, m_bankTabId);
    stmt->setUInt8(++index, uint8(m_eventType));
    stmt->setUInt64(++index, m_playerGuid);
    stmt->setUInt32(++index, m_itemOrMoney);
    stmt->setUInt16(++index, m_itemStackCount);
    stmt->setUInt8(++index, m_destTabId);
    stmt->setUInt64(++index, m_timestamp);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

void Guild::BankEventLogEntry::WritePacket(WorldPackets::Guild::GuildBankLogQueryResults& packet) const
{
    bool hasItem = m_eventType == GUILD_BANK_LOG_DEPOSIT_ITEM || m_eventType == GUILD_BANK_LOG_WITHDRAW_ITEM || m_eventType == GUILD_BANK_LOG_MOVE_ITEM || m_eventType == GUILD_BANK_LOG_MOVE_ITEM2;
    bool itemMoved = (m_eventType == GUILD_BANK_LOG_MOVE_ITEM || m_eventType == GUILD_BANK_LOG_MOVE_ITEM2);

    WorldPackets::Guild::GuildBankLogEntry bankLogEntry;
    bankLogEntry.PlayerGUID = ObjectGuid::Create<HighGuid::Player>(m_playerGuid);
    bankLogEntry.TimeOffset = int32(time(nullptr) - m_timestamp);
    bankLogEntry.EntryType = int8(m_eventType);

    if ((hasItem && m_itemStackCount > 1) || itemMoved)
        bankLogEntry.Count = int32(m_itemStackCount);

    if (IsMoneyEvent())
        bankLogEntry.Money = uint64(m_itemOrMoney);

    if (hasItem)
        bankLogEntry.ItemID = int32(m_itemOrMoney);

    if (itemMoved)
        bankLogEntry.OtherTab = int8(m_destTabId);

    packet.Entry.push_back(bankLogEntry);
}

Guild::NewsLogEntry::NewsLogEntry(ObjectGuid::LowType guildId, uint32 guid, GuildNews type, ObjectGuid playerGuid, uint32 flags, uint32 value, std::string data) :
    LogEntry(guildId, guid), m_type(type), m_playerGuid(playerGuid), m_flags(flags), m_value(value), m_data(data)
{
}

Guild::NewsLogEntry::NewsLogEntry(ObjectGuid::LowType guildId, uint32 guid, time_t timestamp, GuildNews type, ObjectGuid playerGuid, uint32 flags, uint32 value, std::string data) :
    LogEntry(guildId, guid, timestamp), m_type(type), m_playerGuid(playerGuid), m_flags(flags), m_value(value), m_data(data)
{
}

void Guild::NewsLogEntry::SetSticky(bool sticky)
{
    if (sticky)
        m_flags |= 1;
    else
        m_flags &= ~1;
}

void Guild::NewsLogEntry::SaveToDB(SQLTransaction& trans) const
{
    uint8 index = 0;
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_NEWS);
    stmt->setUInt64(index, m_guildId);
    stmt->setUInt32(++index, GetGUID());
    stmt->setUInt8(++index, GetType());
    stmt->setUInt64(++index, GetPlayerGuid().GetCounter());
    stmt->setUInt32(++index, GetFlags());
    stmt->setUInt32(++index, GetValue());
    stmt->setUInt64(++index, GetTimestamp());
    stmt->setString(++index, GetData());
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

void Guild::NewsLogEntry::WritePacket(WorldPackets::Guild::GuildNews& newsPacket) const
{
    WorldPackets::Guild::GuildNewsEvent newsEvent;
    newsEvent.Id = int32(GetGUID());
    newsEvent.MemberGuid = GetPlayerGuid();
    newsEvent.CompletedDate = uint32(GetTimestamp());
    newsEvent.Flags = int32(GetFlags());
    newsEvent.Type = int32(GetType());

    if (GetType() == GUILD_NEWS_PLAYER_ACHIEVEMENT || GetType() == GUILD_NEWS_GUILD_ACHIEVEMENT || GetType() == GUILD_NEWS_DUNGEON_ENCOUNTER)
    {
        newsEvent.Data[0] = GetValue();
        newsEvent.Data[1] = 0;
    }
    else
        for (auto& data : newsEvent.Data)
            data = 0;

    newsEvent.MemberList.push_back(GetPlayerGuid());

    if (GetType() == GUILD_NEWS_ITEM_LOOTED || GetType() == GUILD_NEWS_ITEM_CRAFTED || GetType() == GUILD_NEWS_ITEM_PURCHASED)
    {
        WorldPackets::Item::ItemInstance itemInstance;
        itemInstance.ItemID = GetValue();
        if (!GetData().empty())
            itemInstance.Initialize(GetData());
        newsEvent.Item = itemInstance;
    }

    newsPacket.NewsEvents.push_back(newsEvent);
}

Guild::RankInfo::RankInfo(ObjectGuid::LowType guildId) : m_guildId(guildId), m_rankId(GUILD_RANK_NONE), m_rights(GR_RIGHT_EMPTY), m_bankMoneyPerDay(0)
{
}

Guild::RankInfo::RankInfo(ObjectGuid::LowType guildId, uint32 rankId, std::string const& name, uint32 rights, uint64 money) :
    m_guildId(guildId), m_rankId(rankId), m_name(name), m_rights(rights), m_bankMoneyPerDay(money)
{
}

void Guild::RankInfo::LoadFromDB(Field* fields)
{
    m_rankId = fields[1].GetUInt8();
    m_name = fields[2].GetString();
    m_rights = fields[3].GetUInt32();
    m_bankMoneyPerDay = fields[4].GetUInt32();

    if (m_rankId == GR_GUILDMASTER)                     // Prevent loss of leader rights
        m_rights |= GR_RIGHT_ALL;
}

void Guild::RankInfo::SaveToDB(SQLTransaction& trans) const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_RANK);
    stmt->setUInt64(0, m_guildId);
    stmt->setUInt8(1, m_rankId);
    stmt->setString(2, m_name);
    stmt->setUInt32(3, m_rights);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

void Guild::RankInfo::CreateMissingTabsIfNeeded(uint8 tabs, SQLTransaction& trans, bool logOnCreate /* = false */)
{
    for (uint8 i = 0; i < tabs; ++i)
    {
        GuildBankRightsAndSlots& rightsAndSlots = m_bankTabRightsAndSlots[i];
        if (rightsAndSlots.GetTabId() == i)
            continue;

        rightsAndSlots.SetTabId(i);
        if (m_rankId == GR_GUILDMASTER)
            rightsAndSlots.SetGuildMasterValues();

        if (logOnCreate)
            TC_LOG_ERROR(LOG_FILTER_GUILD, "Guild %u has broken Tab %u for rank %u. Created default tab.", m_guildId, i, m_rankId);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_BANK_RIGHT);
        stmt->setUInt64(0, m_guildId);
        stmt->setUInt8(1, i);
        stmt->setUInt8(2, m_rankId);
        stmt->setUInt8(3, rightsAndSlots.GetRights());
        stmt->setUInt32(4, rightsAndSlots.GetSlots());
        trans->Append(stmt);
    }
}

void Guild::RankInfo::UpdateId(uint32 newId)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_MEMBERS_RANK);
    stmt->setUInt8(0, newId);
    stmt->setUInt64(1, m_guildId);
    stmt->setUInt8(2, m_rankId);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_RANK_ID);
    stmt->setUInt8(0, newId);
    stmt->setUInt64(1, m_guildId);
    stmt->setUInt8(2, m_rankId);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_BANK_RIGHTS_ID);
    stmt->setUInt8(0, newId);
    stmt->setUInt64(1, m_guildId);
    stmt->setUInt8(2, m_rankId);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);

    CharacterDatabase.CommitTransaction(trans);

    SetId(newId);
}

void Guild::RankInfo::SetName(std::string const& name)
{
    if (m_name == name)
        return;

    m_name = name;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_RANK_NAME);
    stmt->setString(0, m_name);
    stmt->setUInt8(1, m_rankId);
    stmt->setUInt64(2, m_guildId);
    CharacterDatabase.Execute(stmt);
}

void Guild::RankInfo::SetRights(uint32 rights)
{
    if (m_rankId == GR_GUILDMASTER)                     // Prevent loss of leader rights
        rights = GR_RIGHT_ALL;

    if (m_rights == rights)
        return;

    m_rights = rights;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_RANK_RIGHTS);
    stmt->setUInt32(0, m_rights);
    stmt->setUInt8(1, m_rankId);
    stmt->setUInt64(2, m_guildId);
    CharacterDatabase.Execute(stmt);
}

bool Guild::RankInfo::operator<(const RankInfo& rank) const
{
    return m_rights > rank.GetRights();
}

bool Guild::RankInfo::operator==(const RankInfo& rank) const
{
    return m_rights == rank.GetRights();
}

uint64 Guild::RankInfo::GetBankMoneyPerDay() const
{
    return m_rankId == GR_GUILDMASTER ? GUILD_WITHDRAW_MONEY_UNLIMITED : m_bankMoneyPerDay;
}

void Guild::RankInfo::SetBankMoneyPerDay(uint32 money)
{
    if (m_rankId == GR_GUILDMASTER)                     // Prevent loss of leader rights
        money = uint32(GUILD_WITHDRAW_MONEY_UNLIMITED);

    if (m_bankMoneyPerDay == money)
        return;

    m_bankMoneyPerDay = money;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_RANK_BANK_MONEY);
    stmt->setUInt32(0, money);
    stmt->setUInt8(1, m_rankId);
    stmt->setUInt64(2, m_guildId);
    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_RANK_BANK_RESET_TIME);
    stmt->setUInt64(0, m_guildId);
    stmt->setUInt8(1, m_rankId);
    CharacterDatabase.Execute(stmt);
}

uint32 Guild::RankInfo::GetBankTabRights(uint8 tabId) const
{
    return tabId < GUILD_BANK_MAX_TABS ? m_bankTabRightsAndSlots[tabId].rights : 0;
}

uint32 Guild::RankInfo::GetBankTabSlotsPerDay(uint8 tabId) const
{
    if (tabId < GUILD_BANK_MAX_TABS)
        return m_rankId == GR_GUILDMASTER ? GUILD_WITHDRAW_SLOT_UNLIMITED : m_bankTabRightsAndSlots[tabId].slots;
    return 0;
}

void Guild::RankInfo::SetBankTabSlotsAndRights(GuildBankRightsAndSlots rightsAndSlots, bool saveToDB)
{
    if (m_rankId == GR_GUILDMASTER)                     // Prevent loss of leader rights
        rightsAndSlots.SetGuildMasterValues();

    GuildBankRightsAndSlots& guildBR = m_bankTabRightsAndSlots[rightsAndSlots.GetTabId()];
    guildBR = rightsAndSlots;

    if (saveToDB)
    {
        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_BANK_RIGHT);
        stmt->setUInt64(0, m_guildId);
        stmt->setUInt8(1, guildBR.GetTabId());
        stmt->setUInt8(2, m_rankId);
        stmt->setUInt8(3, guildBR.GetRights());
        stmt->setUInt32(4, guildBR.GetSlots());
        CharacterDatabase.Execute(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_RANK_BANK_TIME0 + guildBR.GetTabId());
        stmt->setUInt64(0, m_guildId);
        stmt->setUInt8(1, m_rankId);
        CharacterDatabase.Execute(stmt);
    }
}

Guild::BankTab::BankTab(ObjectGuid::LowType const& guildId, uint8 tabId) : m_guildId(guildId), m_tabId(tabId)
{
    memset(m_items, 0, GUILD_BANK_MAX_SLOTS * sizeof(Item*));
}

bool Guild::BankTab::LoadFromDB(Field* fields)
{
    m_name = fields[2].GetString();
    m_icon = fields[3].GetString();
    m_text = fields[4].GetString();
    return true;
}

bool Guild::BankTab::LoadItemFromDB(Field* fields)
{
    uint8 slotId = fields[52].GetUInt8();
    ObjectGuid::LowType itemGuid = fields[0].GetUInt64();
    uint32 itemEntry = fields[1].GetUInt32();
    if (slotId >= GUILD_BANK_MAX_SLOTS)
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Invalid slot for item (GUID: %u, id: %u) in guild bank, skipped.", itemGuid, itemEntry);
        return false;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
    if (!proto)
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Unknown item (GUID: %u, id: %u) in guild bank, skipped.", itemGuid, itemEntry);
        return false;
    }

    Item* pItem = NewItemOrBag(proto);

    if (!pItem->LoadFromDB(itemGuid, ObjectGuid::Empty, fields, itemEntry))
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Item (GUID %u, id: %u) not found in item_instance, deleting from guild bank!", itemGuid, itemEntry);

        PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_NONEXISTENT_GUILD_BANK_ITEM);
        stmt->setUInt64(0, m_guildId);
        stmt->setUInt8(1, m_tabId);
        stmt->setUInt8(2, slotId);
        CharacterDatabase.Execute(stmt);

        delete pItem;

        return false;
    }

    pItem->AddToWorld();
    m_items[slotId] = pItem;
    return true;
}

// Deletes contents of the tab from the world (and from DB if necessary)
void Guild::BankTab::Delete(SQLTransaction& trans, bool removeItemsFromDB)
{
    for (auto item : m_items)
    {
        if (item)
        {
            item->RemoveFromWorld();
            if (removeItemsFromDB)
                item->DeleteFromDB(trans);
            delete item;
        }
    }
}

void Guild::BankTab::SetInfo(std::string const& name, std::string const& icon)
{
    if (m_name == name && m_icon == icon)
        return;

    m_name = name;
    m_icon = icon;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_BANK_TAB_INFO);
    stmt->setString(0, m_name);
    stmt->setString(1, m_icon);
    stmt->setUInt64(2, m_guildId);
    stmt->setUInt8(3, m_tabId);
    CharacterDatabase.Execute(stmt);
}

void Guild::BankTab::SetText(std::string const& text)
{
    if (m_text == text)
        return;

    m_text = text;
    utf8truncate(m_text, MAX_GUILD_BANK_TAB_TEXT_LEN);          // DB and client size limitation

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_BANK_TAB_TEXT);
    stmt->setString(0, m_text);
    stmt->setUInt64(1, m_guildId);
    stmt->setUInt8(2, m_tabId);
    CharacterDatabase.Execute(stmt);
}

Item* Guild::BankTab::GetItem(uint8 slotId) const
{
    return slotId < GUILD_BANK_MAX_SLOTS ? m_items[slotId] : nullptr;
}

bool Guild::BankTab::SetItem(SQLTransaction& trans, uint8 slotId, Item* item)
{
    if (slotId >= GUILD_BANK_MAX_SLOTS)
        return false;

    m_items[slotId] = item;

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_ITEM);
    stmt->setUInt64(0, m_guildId);
    stmt->setUInt8(1, m_tabId);
    stmt->setUInt8(2, slotId);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);

    if (item)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_BANK_ITEM);
        stmt->setUInt64(0, m_guildId);
        stmt->setUInt8(1, m_tabId);
        stmt->setUInt8(2, slotId);
        stmt->setUInt64(3, item->GetGUIDLow());
        CharacterDatabase.ExecuteOrAppend(trans, stmt);

        item->SetGuidValue(ITEM_FIELD_CONTAINED_IN, ObjectGuid::Empty);
        item->SetGuidValue(ITEM_FIELD_OWNER, ObjectGuid::Empty);
        item->FSetState(ITEM_NEW);
        item->SaveToDB(trans);                                 // Not in inventory and can be saved standalone
    }
    return true;
}

void Guild::BankTab::SendText(Guild const* guild, WorldSession* session) const
{
    WorldPackets::Guild::GuildBankTextQueryResult textQuery;
    textQuery.TabId = m_tabId;
    textQuery.Text = m_text;

    if (session)
    {
        if (Player* player = session->GetPlayer())
            player->SendDirectMessage(textQuery.Write());
    }
    else
        guild->BroadcastPacket(textQuery.Write());
}

void Guild::Member::ProfessionInfo::GenerateRecipesMask(std::set<uint32> const& spells)
{
    knownRecipes.GenerateMask(skillId, spells);
}

Guild::Member::Member(ObjectGuid::LowType const& guildId, ObjectGuid guid, uint32 rankId) : m_guildId(guildId), m_guid(guid), m_zoneId(0), m_level(0), m_class(0), m_gender(GENDER_MALE),
m_flags(GUILDMEMBER_STATUS_NONE), m_logoutTime(::time(nullptr)), m_accountId(0), m_rankId(rankId), m_achievementPoints(0), m_totalReputation(0)
{
    memset(m_bankWithdraw, 0, (GUILD_BANK_MAX_TABS + 1) * sizeof(int32));
}

void Guild::Member::SetStats(Player* player)
{
    if (!player->CanContact())
        return;

    m_name = player->GetName();
    m_level = player->getLevel();
    m_class = player->getClass();
    m_zoneId = player->GetCurrentZoneID();
    m_gender = player->getGender();
    m_accountId = player->GetSession()->GetAccountId();
    m_achievementPoints = player->GetAchievementPoints();

    for (uint32 i = 0; i < MAX_GUILD_PROFESSIONS; ++i)
    {
        uint32 id = player->GetUInt32Value(PLAYER_FIELD_PROFESSION_SKILL_LINE + i);
        m_professionInfo[i] = ProfessionInfo(id, player->GetSkillValue(id), player->GetSkillStep(id));
    }

    for (uint32 i = 0; i < MAX_GUILD_PROFESSIONS; ++i)
        m_professionInfo[i].GenerateRecipesMask(player->m_profSpells[i]);
}

void Guild::Member::SaveStatsToDB(SQLTransaction* trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_MEMBER_STATS);
    stmt->setUInt32(0, m_achievementPoints);
    for (uint32 i = 0; i < MAX_GUILD_PROFESSIONS; ++i)
    {
        stmt->setInt32(1 + i * 4, m_professionInfo[i].skillId);
        stmt->setInt32(2 + i * 4, m_professionInfo[i].skillValue);
        stmt->setInt8(3 + i * 4, m_professionInfo[i].skillRank);
        stmt->setString(4 + i * 4, m_professionInfo[i].knownRecipes.GetMaskForSave());
    }
    stmt->setUInt64(9, GetGUID().GetGUIDLow());

    if (trans)
        (*trans)->Append(stmt);
    else
        CharacterDatabase.Execute(stmt);
}

void Guild::Member::SetStats(std::string const& name, uint8 level, uint8 _class, uint32 zoneId, uint32 accountId, uint32 reputation, uint8 gender, uint32 achPoints,
    uint32 profId1, uint32 profValue1, uint8 profRank1, std::string const& recipesMask1, uint32 profId2, uint32 profValue2, uint8 profRank2, std::string const& recipesMask2)
{
    m_name = name;
    m_level = level;
    m_class = _class;
    m_zoneId = zoneId;
    m_gender = gender;
    m_accountId = accountId;
    m_totalReputation = reputation;
    m_achievementPoints = achPoints;

    m_professionInfo[0] = ProfessionInfo(profId1, profValue1, profRank1);
    m_professionInfo[0].knownRecipes.LoadFromString(recipesMask1);
    m_professionInfo[1] = ProfessionInfo(profId2, profValue2, profRank2);
    m_professionInfo[1].knownRecipes.LoadFromString(recipesMask2);
}

void Guild::Member::SetPublicNote(std::string const& publicNote)
{
    if (m_publicNote == publicNote)
        return;

    m_publicNote = publicNote;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_MEMBER_PNOTE);
    stmt->setString(0, publicNote);
    stmt->setUInt64(1, m_guid.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void Guild::Member::SetOfficerNote(std::string const& officerNote)
{
    if (m_officerNote == officerNote)
        return;

    m_officerNote = officerNote;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_MEMBER_OFFNOTE);
    stmt->setString(0, officerNote);
    stmt->setUInt64(1, m_guid.GetCounter());
    CharacterDatabase.Execute(stmt);
}

std::set<uint32> Guild::Member::GetTrackedCriteriaIds() const
{
    return m_trackedCriteriaIds;
}

void Guild::Member::SetTrackedCriteriaIds(std::set<uint32> criteriaIds)
{
    m_trackedCriteriaIds.swap(criteriaIds);
}

bool Guild::Member::IsTrackingCriteriaId(uint32 criteriaId) const
{
    return m_trackedCriteriaIds.find(criteriaId) != m_trackedCriteriaIds.end();
}

bool Guild::Member::IsOnline()
{
    return (m_flags & GUILDMEMBER_STATUS_ONLINE);
}

void Guild::Member::ChangeRank(uint8 newRank)
{
    m_rankId = newRank;

    // Update rank information in player's field, if he is online.
    if (Player* player = FindPlayer())
        player->SetRank(newRank);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_MEMBER_RANK);
    stmt->setUInt8(0, newRank);
    stmt->setUInt64(1, m_guid.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void Guild::Member::UpdateLogoutTime()
{
    m_logoutTime = ::time(nullptr);
}

void Guild::Member::SaveToDB(SQLTransaction& trans) const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_MEMBER);
    stmt->setUInt64(0, m_guildId);
    stmt->setUInt64(1, m_guid.GetCounter());
    stmt->setUInt8(2, m_rankId);
    stmt->setString(3, m_publicNote);
    stmt->setString(4, m_officerNote);
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

// Loads member's data from database.
// If member has broken fields (level, class) returns false.
// In this case member has to be removed from guild.
bool Guild::Member::LoadFromDB(Field* fields)
{
    m_publicNote = fields[3].GetString();
    m_officerNote = fields[4].GetString();
    m_bankRemaining[GUILD_BANK_MAX_TABS].resetTime = fields[5].GetUInt32();
    m_bankRemaining[GUILD_BANK_MAX_TABS].value = fields[6].GetUInt32();
    for (uint8 i = 0; i < GUILD_BANK_MAX_TABS; ++i)
    {
        m_bankRemaining[i].resetTime = fields[7 + i * 2].GetUInt32();
        m_bankRemaining[i].value = fields[8 + i * 2].GetUInt32();
    }

    SetStats(fields[23].GetString(),
             fields[24].GetUInt8(),                         // characters.level
             fields[25].GetUInt8(),                         // characters.class
             fields[26].GetUInt16(),                        // characters.zone
             fields[27].GetUInt32(),                        // characters.account
             fields[29].GetUInt32(),                        // character_reputation.standing
             fields[31].GetUInt8(),                         // characters.gender
             fields[30].GetUInt32(),                        // achievement points
             fields[32].GetUInt32(),                        // prof id 1
             fields[33].GetUInt32(),                        // prof value 1
             fields[34].GetUInt8(),                         // prof rank 1
             fields[35].GetString(),                        // prof recipes mask 1
             fields[36].GetUInt32(),                        // prof id 2
             fields[37].GetUInt32(),                        // prof value 2
             fields[38].GetUInt8(),                         // prof rank 2
             fields[39].GetString()                         // prof recipes mask 2
             );
    m_logoutTime    = fields[28].GetUInt32();               // characters.logout_time

    if (!CheckStats())
        return false;

    if (!m_zoneId)
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Player (GUID: %u) has broken zone-data", m_guid.GetGUIDLow());
        m_zoneId = Player::GetZoneIdFromDB(m_guid);
    }
    return true;
}

// Validate player fields. Returns false if corrupted fields are found.
bool Guild::Member::CheckStats() const
{
    if (m_level < 1)
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Player (GUID: %u) has a broken data in field `characters`.`level`, deleting him from guild!", m_guid.GetGUIDLow());
        return false;
    }

    if (m_class < CLASS_WARRIOR || m_class >= MAX_CLASSES)
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Player (GUID: %u) has a broken data in field `characters`.`class`, deleting him from guild!", m_guid.GetGUIDLow());
        return false;
    }

    if (m_gender != GENDER_FEMALE && m_gender != GENDER_MALE)
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Player (GUID: %u) has a broken data in field `characters`.`gender`, deleting him from guild!", m_guid.GetGUIDLow());
        return false;
    }

    return true;
}

bool Guild::Member::IsRank(uint8 rankId) const
{
    return m_rankId == rankId;
}

bool Guild::Member::IsRankNotLower(uint8 rankId) const
{
    return m_rankId <= rankId;
}

bool Guild::Member::IsSamePlayer(ObjectGuid guid) const
{
    return m_guid == guid;
}

void Guild::Member::ResetValues()
{
    for (uint8 tabId = 0; tabId <= GUILD_BANK_MAX_TABS; ++tabId)
        m_bankWithdraw[tabId] = 0;
}

// Decreases amount of money/slots left for today.
// If (tabId == GUILD_BANK_MAX_TABS) decrease money amount.
// Otherwise decrease remaining items amount for specified tab.
void Guild::Member::DecreaseBankRemainingValue(SQLTransaction& trans, uint8 tabId, uint32 amount)
{
    m_bankRemaining[tabId].value -= amount;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(tabId == GUILD_BANK_MAX_TABS ? CHAR_UPD_GUILD_MEMBER_BANK_REM_MONEY : CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS0 + tabId);
    stmt->setUInt32(0, m_bankRemaining[tabId].value);
    stmt->setUInt64(1, m_guildId);
    stmt->setUInt64(2, m_guid.GetCounter());
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

// Get amount of money/slots left for today.
// If (tabId == GUILD_BANK_MAX_TABS) return money amount.
// Otherwise return remaining items amount for specified tab.
// If reset time was more than 24 hours ago, renew reset time and reset amount to maximum value.
uint32 Guild::Member::GetBankRemainingValue(uint8 tabId, const Guild* guild) const
{
    // Guild master has unlimited amount.
    if (IsRank(GR_GUILDMASTER))
        return tabId == GUILD_BANK_MAX_TABS ? GUILD_WITHDRAW_MONEY_UNLIMITED : GUILD_WITHDRAW_SLOT_UNLIMITED;

    // Check rights for non-money tab.
    if (tabId != GUILD_BANK_MAX_TABS)
        if ((guild->_GetRankBankTabRights(m_rankId, tabId) & GUILD_BANK_RIGHT_VIEW_TAB) != GUILD_BANK_RIGHT_VIEW_TAB)
            return 0;

    auto curTime = uint32(::time(nullptr) / MINUTE); // minutes
    if (curTime > m_bankRemaining[tabId].resetTime + 24 * HOUR / MINUTE)
    {
        auto& rv = const_cast <RemainingValue&> (m_bankRemaining[tabId]);
        rv.resetTime = curTime;
        rv.value = tabId == GUILD_BANK_MAX_TABS ? guild->_GetRankBankMoneyPerDay(m_rankId) : guild->_GetRankBankTabSlotsPerDay(m_rankId, tabId);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(tabId == GUILD_BANK_MAX_TABS ? CHAR_UPD_GUILD_MEMBER_BANK_TIME_MONEY : CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS0 + tabId);
        stmt->setUInt32(0, m_bankRemaining[tabId].resetTime);
        stmt->setUInt32(1, m_bankRemaining[tabId].value);
        stmt->setUInt64(2, m_guildId);
        stmt->setUInt64(3, m_guid.GetCounter());
        CharacterDatabase.Execute(stmt);
    }
    return m_bankRemaining[tabId].value;
}

inline void Guild::Member::ResetTabTimes()
{
    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
        m_bankRemaining[tabId].resetTime = 0;
}

inline void Guild::Member::ResetMoneyTime()
{
    m_bankRemaining[GUILD_BANK_MAX_TABS].resetTime = 0;
}

Player* Guild::Member::FindPlayer() const
{
    return ObjectAccessor::FindPlayer(m_guid);
}

EmblemInfo::EmblemInfo() : m_style(0), m_color(0), m_borderStyle(0), m_borderColor(0), m_backgroundColor(0)
{
}

EmblemInfo::EmblemInfo(int32 style, int32 color, int32 borderStyle, int32 borderColor, int32 backgroundColor)
{
    m_style = style;
    m_color = color;
    m_borderStyle = borderStyle;
    m_borderColor = borderColor;
    m_backgroundColor = backgroundColor;
}

void EmblemInfo::LoadFromDB(Field* fields)
{
    m_style = fields[4].GetInt32();
    m_color = fields[5].GetInt32();
    m_borderStyle = fields[6].GetInt32();
    m_borderColor = fields[7].GetInt32();
    m_backgroundColor = fields[8].GetInt32();
}

void EmblemInfo::WritePacket(WorldPacket& data) const
{
    data << m_color;
    data << m_style;
    data << m_borderColor;
    data << m_borderStyle;
    data << m_backgroundColor;
}

GuildBankRightsAndSlots::GuildBankRightsAndSlots() : tabId(TAB_UNDEFINED), rights(0), slots(0)
{
}

GuildBankRightsAndSlots::GuildBankRightsAndSlots(uint8 _tabId) : tabId(_tabId), rights(0), slots(0)
{
}

GuildBankRightsAndSlots::GuildBankRightsAndSlots(uint8 _tabId, uint8 _rights, uint32 _slots) : tabId(_tabId), rights(_rights), slots(_slots)
{
}

bool GuildBankRightsAndSlots::IsEqual(GuildBankRightsAndSlots const& rhs) const
{
    return rights == rhs.rights && slots == rhs.slots;
}

void GuildBankRightsAndSlots::SetGuildMasterValues()
{
    rights = GUILD_BANK_RIGHT_FULL;
    slots = uint32(GUILD_WITHDRAW_SLOT_UNLIMITED);
}

void EmblemInfo::SaveToDB(ObjectGuid::LowType guildId) const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_EMBLEM_INFO);
    stmt->setInt32(0, m_style);
    stmt->setInt32(1, m_color);
    stmt->setInt32(2, m_borderStyle);
    stmt->setInt32(3, m_borderColor);
    stmt->setInt32(4, m_backgroundColor);
    stmt->setInt64(5, guildId);
    CharacterDatabase.Execute(stmt);
}

bool EmblemInfo::ValidateEmblemColors()
{
    return sGuildColorBackgroundStore.LookupEntry(m_backgroundColor) && sGuildColorBorderStore.LookupEntry(m_borderColor) && sGuildColorEmblemStore.LookupEntry(m_color);
}

Guild::MoveItemData::MoveItemData(Guild* guild, Player* player, uint8 container, uint8 slotId) : m_pGuild(guild), m_pPlayer(player),
m_container(container), m_slotId(slotId), m_pItem(nullptr), m_pClonedItem(nullptr)
{
}

bool Guild::MoveItemData::CheckItem(uint32& splitedAmount)
{
    ASSERT(m_pItem);
    if (splitedAmount > m_pItem->GetCount())
        return false;
    if (splitedAmount == m_pItem->GetCount())
        splitedAmount = 0;
    return true;
}

bool Guild::MoveItemData::CanStore(Item* pItem, bool swap, bool sendError)
{
    m_vec.clear();
    InventoryResult msg = CanStore(pItem, swap);
    if (sendError && msg != EQUIP_ERR_OK)
        m_pPlayer->SendEquipError(msg, pItem);
    return (msg == EQUIP_ERR_OK);
}

bool Guild::MoveItemData::CloneItem(uint32 count)
{
    ASSERT(m_pItem);
    m_pClonedItem = m_pItem->CloneItem(count);
    if (!m_pClonedItem)
    {
        m_pPlayer->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, m_pItem);
        return false;
    }
    return true;
}

void Guild::MoveItemData::LogAction(MoveItemData* pFrom) const
{
    ASSERT(pFrom->GetItem());

    sScriptMgr->OnGuildItemMove(m_pGuild, m_pPlayer, pFrom->GetItem(), pFrom->IsBank(), pFrom->GetContainer(), pFrom->GetSlotId(), IsBank(), GetContainer(), GetSlotId());
}

inline void Guild::MoveItemData::CopySlots(SlotIds& ids) const
{
    for (auto itr : m_vec)
        ids.insert(uint8(itr.pos));
}

Guild::PlayerMoveItemData::PlayerMoveItemData(Guild* guild, Player* player, uint8 container, uint8 slotId) :MoveItemData(guild, player, container, slotId)
{
}

///////////////////////////////////////////////////////////////////////////////
// PlayerMoveItemData
bool Guild::PlayerMoveItemData::InitItem()
{
    m_pItem = m_pPlayer->GetItemByPos(m_container, m_slotId);
    if (m_pItem)
    {
        // Anti-WPE protection. Do not move non-empty bags to bank.
        if (m_pItem->IsNotEmptyBag())
        {
            m_pPlayer->SendEquipError(EQUIP_ERR_DESTROY_NONEMPTY_BAG, m_pItem);
            m_pItem = nullptr;
        }
        // Bound items cannot be put into bank.
        else if (!m_pItem->CanBeTraded())
        {
            m_pPlayer->SendEquipError(EQUIP_ERR_CANT_SWAP, m_pItem);
            m_pItem = nullptr;
        }
    }
    return (m_pItem != nullptr);
}

void Guild::PlayerMoveItemData::RemoveItem(SQLTransaction& trans, MoveItemData* /*pOther*/, uint32 splitedAmount)
{
    if (splitedAmount)
    {
        m_pItem->SetCount(m_pItem->GetCount() - splitedAmount);
        m_pItem->SetState(ITEM_CHANGED, m_pPlayer);
        m_pPlayer->SaveInventoryAndGoldToDB(trans);
    }
    else
    {
        m_pPlayer->MoveItemFromInventory(m_container, m_slotId, true);
        m_pItem->DeleteFromInventoryDB(trans);
        m_pItem = nullptr;
    }
}

Item* Guild::PlayerMoveItemData::StoreItem(SQLTransaction& trans, Item* pItem)
{
    ASSERT(pItem);

    m_pPlayer->MoveItemToInventory(m_vec, pItem, true);
    m_pPlayer->SaveInventoryAndGoldToDB(trans);
    return pItem;
}

void Guild::PlayerMoveItemData::LogBankEvent(SQLTransaction& trans, MoveItemData* pFrom, uint32 count) const
{
    ASSERT(pFrom);
    m_pGuild->_LogBankEvent(trans, GUILD_BANK_LOG_WITHDRAW_ITEM, pFrom->GetContainer(), m_pPlayer->GetGUIDLow(), pFrom->GetItem()->GetEntry(), count);
}

inline InventoryResult Guild::PlayerMoveItemData::CanStore(Item* pItem, bool swap)
{
    return m_pPlayer->CanStoreItem(m_container, m_slotId, m_vec, pItem, swap);
}

Guild::BankMoveItemData::BankMoveItemData(Guild* guild, Player* player, uint8 container, uint8 slotId) : MoveItemData(guild, player, container, slotId)
{
}

///////////////////////////////////////////////////////////////////////////////
// BankMoveItemData
bool Guild::BankMoveItemData::InitItem()
{
    m_pItem = m_pGuild->_GetItem(m_container, m_slotId);
    return (m_pItem != nullptr);
}

bool Guild::BankMoveItemData::HasStoreRights(MoveItemData* pOther) const
{
    ASSERT(pOther);
    // Do not check rights if item is being swapped within the same bank tab
    if (pOther->IsBank() && pOther->GetContainer() == m_container)
        return true;
    return m_pGuild->_MemberHasTabRights(m_pPlayer->GetGUID(), m_container, GUILD_BANK_RIGHT_DEPOSIT_ITEM);
}

bool Guild::BankMoveItemData::HasWithdrawRights(MoveItemData* pOther) const
{
    ASSERT(pOther);
    // Do not check rights if item is being swapped within the same bank tab
    if (pOther->IsBank() && pOther->GetContainer() == m_container)
        return true;
    return (m_pGuild->_GetMemberRemainingSlots(m_pPlayer->GetGUID(), m_container) != 0);
}

void Guild::BankMoveItemData::RemoveItem(SQLTransaction& trans, MoveItemData* pOther, uint32 splitedAmount)
{
    ASSERT(m_pItem);
    if (splitedAmount)
    {
        m_pItem->SetCount(m_pItem->GetCount() - splitedAmount);
        m_pItem->FSetState(ITEM_CHANGED);
        m_pItem->SaveToDB(trans);
    }
    else
    {
        m_pGuild->_RemoveItem(trans, m_container, m_slotId);
        m_pItem = nullptr;
    }
    // Decrease amount of player's remaining items (if item is moved to different tab or to player)
    if (!pOther->IsBank() || pOther->GetContainer() != m_container)
        m_pGuild->_DecreaseMemberRemainingSlots(trans, m_pPlayer->GetGUID(), m_container);
}

Item* Guild::BankMoveItemData::StoreItem(SQLTransaction& trans, Item* pItem)
{
    if (!pItem)
        return nullptr;

    BankTab* pTab = m_pGuild->GetBankTab(m_container);
    if (!pTab)
        return nullptr;

    Item* pLastItem = pItem;
    for (ItemPosCountVec::const_iterator itr = m_vec.begin(); itr != m_vec.end(); )
    {
        ItemPosCount pos(*itr);
        ++itr;

        TC_LOG_DEBUG(LOG_FILTER_GUILD, "GUILD STORAGE: StoreItem tab = %u, slot = %u, item = %u, count = %u", m_container, m_slotId, pItem->GetEntry(), pItem->GetCount()); pLastItem = _StoreItem(trans, pTab, pItem, pos, itr != m_vec.end());
    }
    return pLastItem;
}

void Guild::BankMoveItemData::LogBankEvent(SQLTransaction& trans, MoveItemData* pFrom, uint32 count) const
{
    ASSERT(pFrom->GetItem());
    if (pFrom->IsBank())
        m_pGuild->_LogBankEvent(trans, GUILD_BANK_LOG_MOVE_ITEM, pFrom->GetContainer(), m_pPlayer->GetGUIDLow(), pFrom->GetItem()->GetEntry(), count, m_container);
    else
        m_pGuild->_LogBankEvent(trans, GUILD_BANK_LOG_DEPOSIT_ITEM, m_container, m_pPlayer->GetGUIDLow(), pFrom->GetItem()->GetEntry(), count);
}

void Guild::BankMoveItemData::LogAction(MoveItemData* pFrom) const
{
    MoveItemData::LogAction(pFrom);
    if (!pFrom->IsBank() && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE) && !AccountMgr::IsPlayerAccount(m_pPlayer->GetSession()->GetSecurity()))       // TODO: move to scripts
        sLog->outCommand(m_pPlayer->GetSession()->GetAccountId(),
            "GM %s (Account: %u) deposit item: %s (Entry: %d Count: %u) to guild bank (Guild ID: %u)",
            m_pPlayer->GetName(), m_pPlayer->GetSession()->GetAccountId(),
            pFrom->GetItem()->GetTemplate()->GetName()->Str[m_pPlayer->GetSession()->GetSessionDbLocaleIndex()], pFrom->GetItem()->GetEntry(), pFrom->GetItem()->GetCount(),
            m_pGuild->GetId());
}

Item* Guild::BankMoveItemData::_StoreItem(SQLTransaction& trans, BankTab* pTab, Item* pItem, ItemPosCount& pos, bool clone) const
{
    uint8 slotId = uint8(pos.pos);
    uint32 count = pos.count;
    if (Item* pItemDest = pTab->GetItem(slotId))
    {
        pItemDest->SetCount(pItemDest->GetCount() + count);
        pItemDest->FSetState(ITEM_CHANGED);
        pItemDest->SaveToDB(trans);
        if (!clone)
        {
            pItem->RemoveFromWorld();
            pItem->DeleteFromDB(trans);
            delete pItem;
        }
        return pItemDest;
    }

    if (clone)
        pItem = pItem->CloneItem(count);
    else
        pItem->SetCount(count);

    if (pItem && pTab->SetItem(trans, slotId, pItem))
        return pItem;

    return nullptr;
}

// Tries to reserve space for source item.
// If item in destination slot exists it must be the item of the same entry
// and stack must have enough space to take at least one item.
// Returns false if destination item specified and it cannot be used to reserve space.
bool Guild::BankMoveItemData::_ReserveSpace(uint8 slotId, Item* pItem, Item* pItemDest, uint32& count)
{
    uint32 requiredSpace = pItem->GetMaxStackCount();
    if (pItemDest)
    {
        // Make sure source and destination items match and destination item has space for more stacks.
        if (pItemDest->GetEntry() != pItem->GetEntry() || pItemDest->GetCount() >= pItem->GetMaxStackCount())
            return false;
        requiredSpace -= pItemDest->GetCount();
    }
    // Let's not be greedy, reserve only required space
    requiredSpace = std::min(requiredSpace, count);

    // Reserve space
    ItemPosCount pos(slotId, requiredSpace);
    if (!pos.isContainedIn(m_vec))
    {
        m_vec.push_back(pos);
        count -= requiredSpace;
    }
    return true;
}

void Guild::BankMoveItemData::CanStoreItemInTab(Item* pItem, uint8 skipSlotId, bool merge, uint32& count)
{
    for (uint8 slotId = 0; (slotId < GUILD_BANK_MAX_SLOTS) && (count > 0); ++slotId)
    {
        // Skip slot already processed in CanStore (when destination slot was specified)
        if (slotId == skipSlotId)
            continue;

        Item* pItemDest = m_pGuild->_GetItem(m_container, slotId);
        if (pItemDest == pItem)
            pItemDest = nullptr;

        // If merge skip empty, if not merge skip non-empty
        if ((pItemDest != nullptr) != merge)
            continue;

        _ReserveSpace(slotId, pItem, pItemDest, count);
    }
}

InventoryResult Guild::BankMoveItemData::CanStore(Item* pItem, bool swap)
{
    TC_LOG_DEBUG(LOG_FILTER_GUILD, "GUILD STORAGE: CanStore() tab = %u, slot = %u, item = %u, count = %u",
        m_container, m_slotId, pItem->GetEntry(), pItem->GetCount());

    uint32 count = pItem->GetCount();
    // Soulbound items cannot be moved
    if (pItem->IsSoulBound())
        return EQUIP_ERR_DROP_BOUND_ITEM;

    // Make sure destination bank tab exists
    if (m_container >= m_pGuild->GetPurchasedTabsSize())
        return EQUIP_ERR_WRONG_BAG_TYPE;

    // Slot explicitely specified. Check it.
    if (m_slotId != NULL_SLOT)
    {
        Item* pItemDest = m_pGuild->_GetItem(m_container, m_slotId);
        // Ignore swapped item (this slot will be empty after move)
        if ((pItemDest == pItem) || swap)
            pItemDest = nullptr;

        if (!_ReserveSpace(m_slotId, pItem, pItemDest, count))
            return EQUIP_ERR_CANT_STACK;

        if (count == 0)
            return EQUIP_ERR_OK;
    }

    // Slot was not specified or it has not enough space for all the items in stack
    // Search for stacks to merge with
    if (pItem->GetMaxStackCount() > 1)
    {
        CanStoreItemInTab(pItem, m_slotId, true, count);
        if (count == 0)
            return EQUIP_ERR_OK;
    }

    // Search free slot for item
    CanStoreItemInTab(pItem, m_slotId, false, count);
    if (count == 0)
        return EQUIP_ERR_OK;

    return EQUIP_ERR_BANK_FULL;
}

///////////////////////////////////////////////////////////////////////////////
// Guild
Guild::Guild() : m_id(0), m_flags(0), m_createdDate(0), m_accountsNumber(0), m_bankMoney(0), m_eventLog(nullptr), m_newsLog(nullptr), m_achievementMgr(this), _level(25)
{
    memset(&m_bankEventLog, 0, (GUILD_BANK_MAX_TABS + 1) * sizeof(LogHolder*));
    m_members_online = 0;
    m_lastSave = 0;

    m_ChallengeCount.resize(ChallengeMax);
    for (uint8 i = 0; i < ChallengeMax; ++i)
        m_ChallengeCount[i] = 0;
}

Guild::~Guild()
{
    SQLTransaction temp(nullptr);
    _DeleteBankItems(temp);

    // Cleanup
    delete m_eventLog;
    m_eventLog = nullptr;
    delete m_newsLog;
    m_newsLog = nullptr;

    for (uint8 tabId = 0; tabId <= GUILD_BANK_MAX_TABS; ++tabId)
    {
        delete m_bankEventLog[tabId];
        m_bankEventLog[tabId] = nullptr;
    }

    for (auto& member : m_members)
    {
        delete member.second;
        member.second = nullptr;
    }
}

// Creates new guild with default data and saves it to database.
bool Guild::Create(Player* pLeader, std::string const& name)
{
    // Check if guild with such name already exists
    if (sGuildMgr->GetGuildByName(name))
        return false;

    WorldSession* pLeaderSession = pLeader->GetSession();
    if (!pLeaderSession)
        return false;

    m_id = sGuildMgr->GenerateGuildId();
    m_leaderGuid = pLeader->GetGUID();
    m_name = name;
    m_flags = 0;
    m_info = "";
    m_motd = "No message set.";
    m_bankMoney = 0;
    m_createdDate = ::time(nullptr);
    _level = 25;

    _CreateLogHolders();

    TC_LOG_DEBUG(LOG_FILTER_GUILD, "GUILD: creating guild [%s] for leader %s (%u)", name.c_str(), pLeader->GetName(), GetLeaderGUID().GetCounter());

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_MEMBERS);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    uint8 index = 0;
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD);
    stmt->setUInt64(index, m_id);
    stmt->setString(++index, name);
    stmt->setUInt64(++index, GetLeaderGUID().GetCounter());
    stmt->setUInt32(++index, m_flags);
    stmt->setString(++index, m_info);
    stmt->setString(++index, m_motd);
    stmt->setUInt64(++index, uint32(m_createdDate));
    stmt->setInt32(++index, m_emblemInfo.GetStyle());
    stmt->setInt32(++index, m_emblemInfo.GetColor());
    stmt->setInt32(++index, m_emblemInfo.GetBorderStyle());
    stmt->setInt32(++index, m_emblemInfo.GetBorderColor());
    stmt->setInt32(++index, m_emblemInfo.GetBackgroundColor());
    stmt->setUInt64(++index, m_bankMoney);
    stmt->setUInt32(++index, _level);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
    // Create default ranks
    _CreateDefaultGuildRanks(pLeaderSession->GetSessionDbLocaleIndex());
    // Add guildmaster
    bool ret = AddMember(GetLeaderGUID(), GR_GUILDMASTER);
    if (ret)
        // Call scripts on successful create
        sScriptMgr->OnGuildCreate(this, pLeader, name);

    for (uint8 i = 1; i < ChallengeMax; ++i)
    {
        auto statement = CharacterDatabase.GetPreparedStatement(CHAR_INIT_GUILD_CHALLENGES);
        statement->setInt32(0, GetId());
        statement->setInt32(1, i);
        CharacterDatabase.Execute(statement);
    }

    SendGuildEventRanksUpdated();

    return ret;
}

// Disbands guild and deletes all related data from database
void Guild::Disband()
{
    // Call scripts before guild data removed from database
    sScriptMgr->OnGuildDisband(this);

    SendGuildEventDisbanded();

    // Remove all members
    while (!m_members.empty())
    {
        Members::const_iterator itr = m_members.begin();
        DeleteMember(itr->second->GetGUID(), true);
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_RANKS);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_TABS);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    // Free bank tab used memory and delete items stored in them
    _DeleteBankItems(trans, true);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_ITEMS);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_RIGHTS);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_EVENTLOGS);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_EVENTLOGS);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_REMOVE_GUILD_CHALLENGES);
    stmt->setUInt64(0, m_id);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    sGuildFinderMgr->DeleteGuild(GetGUID());

    sGuildMgr->RemoveGuild(m_id);
}

void Guild::SaveToDB(bool withMembers)
{
    if (m_lastSave > time(nullptr)) // Prevent save if guild already saved
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    m_achievementMgr.SaveToDB(trans);

    if (withMembers)
    {
        for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        {
            if (Player* player = sObjectAccessor->FindPlayer(itr->second->GetGUID()))
            {
                if (!player->CanContact()) //Prevent crash when player change skills
                    continue;

                itr->second->SetStats(player);
                itr->second->SaveStatsToDB(&trans);
            }
        }

        UpdateGuildRecipes();
    }

    CharacterDatabase.CommitTransaction(trans);

    m_lastSave = time(nullptr) + (sWorld->getIntConfig(CONFIG_GUILD_SAVE_INTERVAL) * MINUTE);
}

///////////////////////////////////////////////////////////////////////////////
// HANDLE CLIENT COMMANDS

void Guild::SendRoster(WorldSession* session /*= nullptr*/)
{
    if (!session)
        return;
    WorldPackets::Guild::GuildRoster roster;

    roster.NumAccounts = int32(m_accountsNumber);
    roster.CreateDate = uint32(m_createdDate);
    roster.GuildFlags = m_flags;

    roster.MemberData.reserve(m_members.size());

    for (auto m : m_members)
    {
        Member* member = m.second;
        if (!member)
            continue;

        Player* onlineMember = member->FindPlayer();
        uint8 flags = GUILDMEMBER_STATUS_NONE;

        if (onlineMember && onlineMember->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
            onlineMember = nullptr;

        if (onlineMember)
        {
            flags |= GUILDMEMBER_STATUS_ONLINE;
            if (onlineMember->isAFK())
                flags |= GUILDMEMBER_STATUS_AFK;
            if (onlineMember->isDND())
                flags |= GUILDMEMBER_STATUS_DND;
        }

        WorldPackets::Guild::GuildRosterMemberData memberData;

        memberData.Guid = member->GetGUID();
        memberData.RankID = int32(member->GetRankId());
        memberData.AreaID = int32(onlineMember ? onlineMember->GetCurrentZoneID() : member->GetZoneId());
        memberData.PersonalAchievementPoints = int32(onlineMember ? onlineMember->GetAchievementPoints() : member->GetAchievementPoints());
        memberData.GuildReputation = int32(member->GetTotalReputation());
        memberData.LastSave = float(onlineMember ? 0.0f : float(::time(nullptr) - member->GetLogoutTime()) / DAY);

        for (uint8 i = 0; i < MAX_GUILD_PROFESSIONS; ++i)
        {
            memberData.ProfessionData[i].DbID = uint32(member->GetProfessionInfo(i).skillId);
            memberData.ProfessionData[i].Rank = uint32(member->GetProfessionInfo(i).skillRank);
            memberData.ProfessionData[i].Step = uint32(member->GetProfessionInfo(i).skillValue);
        }

        memberData.VirtualRealmAddress = GetVirtualRealmAddress();
        memberData.Status = flags;
        memberData.Level = onlineMember ? onlineMember->getLevel() : member->GetLevel();
        memberData.ClassID = onlineMember ? onlineMember->getClass() : member->GetClass();
        memberData.Gender = onlineMember ? onlineMember->getGender() : member->GetGender();

        memberData.Authenticated = false;
        memberData.SorEligible = false;

        memberData.Name = member->GetName();
        memberData.Note = member->GetPublicNote();
        memberData.OfficerNote = member->GetOfficerNote();

        roster.MemberData.push_back(memberData);
    }

    roster.WelcomeText = m_motd;
    roster.InfoText = m_info;

    if (Player* player = session->GetPlayer())
        player->SendDirectMessage(roster.Write());
}

void Guild::SendQueryResponse(WorldSession* session)
{
    if (!session)
        return;
    WorldPackets::Guild::QueryGuildInfoResponse response;
    response.GuildGuid = GetGUID();
    response.Info = boost::in_place();
    response.Info->GuildGUID = GetGUID();
    response.Info->VirtualRealmAddress = GetVirtualRealmAddress();
    response.Info->EmblemStyle = m_emblemInfo.GetStyle();
    response.Info->EmblemColor = m_emblemInfo.GetColor();
    response.Info->BorderStyle = m_emblemInfo.GetBorderStyle();
    response.Info->BorderColor = m_emblemInfo.GetBorderColor();
    response.Info->BackgroundColor = m_emblemInfo.GetBackgroundColor();
    response.Info->GuildName = m_name;

    for (uint8 i = 0; i < _GetRanksSize(); ++i)
        response.Info->Ranks.insert(WorldPackets::Guild::QueryGuildInfoResponse::GuildInfo::GuildInfoRank(m_ranks[i].GetId(), i, m_ranks[i].GetName()));

    if (Player* player = session->GetPlayer())
        player->SendDirectMessage(response.Write());
}

void Guild::HandleSetAchievementTracking(WorldSession* session, std::set<uint32> const& achievementIds)
{
    return; // Disable this option

    // ReSharper disable once CppUnreachableCode
    Player* player = session->GetPlayer();
    Member* member = GetMember(player->GetGUID());
    if (!member)
        return;

    std::set<uint32> criteriaIds;

    if (auto progressMap = player->GetAchievementMgr()->GetCriteriaProgressMap())
    {
        for (CriteriaProgressMap::const_iterator itr = progressMap->begin(); itr != progressMap->end(); ++itr)
        {
            if (achievementIds.find(itr->second.achievement->ID) != achievementIds.end())
            {
                CriteriaTreeEntry const* criteriaTree = itr->second.criteriaTree;
                if (criteriaTree->CriteriaID)
                    criteriaIds.insert(criteriaTree->ID);
            }
        }
    }

    member->SetTrackedCriteriaIds(criteriaIds);
    player->GetAchievementMgr()->SendAllTrackedCriterias(player, member->GetTrackedCriteriaIds());
}

void Guild::SendGuildRankInfo(WorldSession* session) const
{
    if (!session)
        return;
    auto const& player = session->GetPlayer();
    if (!player)
        return;

    WorldPackets::Guild::GuildRanks ranks;
    ranks.Ranks.reserve(_GetRanksSize());

    for (uint8 i = 0; i < _GetRanksSize(); i++)
    {
        RankInfo const* rankInfo = GetRankInfo(i);
        if (!rankInfo)
            continue;

        WorldPackets::Guild::GuildRankData rankData;
        rankData.RankID = uint32(rankInfo->GetId());
        rankData.RankOrder = uint32(i);
        rankData.Flags = rankInfo->GetRights();
        rankData.WithdrawGoldLimit = uint32(rankInfo->GetBankMoneyPerDay() / GOLD);
        rankData.RankName = rankInfo->GetName();

        for (uint8 j = 0; j < GUILD_BANK_MAX_TABS; ++j)
        {
            rankData.TabFlags[j] = uint32(rankInfo->GetBankTabRights(j));
            rankData.TabWithdrawItemLimit[j] = uint32(rankInfo->GetBankTabSlotsPerDay(j));
        }

        ranks.Ranks.push_back(rankData);
    }

    player->SendDirectMessage(ranks.Write());
}

void Guild::HandleSetMOTD(WorldSession* session, std::string const& motd)
{
    if (!session)
        return;
    if (m_motd == motd)
        return;

    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(const_cast<std::string&>(motd));

        if (strchr(motd.c_str(), '|'))
        {
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                session->KickPlayer();
            return;
        }
    }

    // Player must have rights to set MOTD
    if (!_HasRankRight(session->GetPlayer(), GR_RIGHT_SETMOTD))
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
    else
    {
        m_motd = motd;

        sScriptMgr->OnGuildMOTDChanged(this, motd);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_MOTD);
        stmt->setString(0, motd);
        stmt->setUInt64(1, m_id);
        CharacterDatabase.Execute(stmt);

        SendGuildEventMOTD();
    }
}

void Guild::HandleSetInfo(WorldSession* session, std::string const& info)
{
    if (!session)
        return;
    if (m_info == info)
        return;

    // Player must have rights to set guild's info
    if (!_HasRankRight(session->GetPlayer(), GR_RIGHT_MODIFY_GUILD_INFO))
        SendCommandResult(session, GUILD_CREATE_S, ERR_GUILD_PERMISSIONS);
    else
    {
        m_info = "";
        m_info = info;

        sScriptMgr->OnGuildInfoChanged(this, info);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_INFO);
        stmt->setString(0, info);
        stmt->setUInt64(1, m_id);
        CharacterDatabase.Execute(stmt);
    }
}

void Guild::HandleSetEmblem(WorldSession* session, const EmblemInfo& emblemInfo)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();
    if (!_IsLeader(player))
        // "Only guild leaders can create emblems."
        SendSaveEmblemResult(session, ERR_GUILDEMBLEM_NOTGUILDMASTER);
    else if (!player->HasEnoughMoney(uint64(EMBLEM_PRICE)))
        // "You can't afford to do that."
        SendSaveEmblemResult(session, ERR_GUILDEMBLEM_NOTENOUGHMONEY);
    else
    {
        player->ModifyMoney(-int64(EMBLEM_PRICE));

        m_emblemInfo = emblemInfo;
        m_emblemInfo.SaveToDB(m_id);

        // "Guild Emblem saved."
        SendSaveEmblemResult(session, ERR_GUILDEMBLEM_SUCCESS);

        SendQueryResponse(session);

        UpdateAchievementCriteria(CRITERIA_TYPE_BUY_GUILD_EMBLEM, 1, 0, 0, nullptr, player);
    }
}

void Guild::HandleSetNewGuildMaster(WorldSession* session, std::string name)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();
    // Only leader can assign new leader
    if (!_IsLeader(player))
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);

    // Old leader must be a member of guild
    else if (Member* pOldLeader = GetMember(player->GetGUID()))
    {
        // New leader must be a member of guild
        if (Member* pNewLeader = GetMember(session, sObjectMgr->GetRealCharName(std::move(name))))
        {
            _SetLeaderGUID(pNewLeader);
            pOldLeader->ChangeRank(GR_INITIATE);

            SendGuildEventNewLeader(pNewLeader, pOldLeader);
        }
    }
    else
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
}

void Guild::HandleSetBankTabInfo(WorldSession* /*session*/, uint8 tabId, std::string const& name, std::string const& icon)
{
    if (BankTab* pTab = GetBankTab(tabId))
    {
        pTab->SetInfo(name, icon);
        SendGuildEventTabModified(tabId, name, icon);
    }
}

void Guild::HandleSetMemberNote(WorldSession* session, std::string const& note, ObjectGuid guid, bool isPublic)
{
    if (!session)
        return;
    // Player must have rights to set public/officer note
    if (!_HasRankRight(session->GetPlayer(), isPublic ? GR_RIGHT_EPNOTE : GR_RIGHT_EOFFNOTE))
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
    else if (Member* member = GetMember(guid))
    {
        if (isPublic)
            member->SetPublicNote(note);
        else
            member->SetOfficerNote(note);

        WorldPackets::Guild::GuildMemberUpdateNote updateNote;
        updateNote.Member = guid;
        updateNote.IsPublic = isPublic;
        updateNote.Note = note;
        BroadcastPacket(updateNote.Write());
    }
}

void Guild::HandleSetRankInfo(WorldSession* session, uint32 rankId, std::string const& name, uint32 rights, uint32 moneyPerDay, GuildBankRightsAndSlotsVec rightsAndSlots)
{
    if (!session)
        return;
    // Only leader can modify ranks
    if (!_IsLeader(session->GetPlayer()))
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
    else if (RankInfo* rankInfo = GetRankInfo(rankId))
    {
        TC_LOG_DEBUG(LOG_FILTER_GUILD, "WORLD: Changed RankName to '%s', rights to 0x%08X", name.c_str(), rights);

        rankInfo->SetName(name);
        rankInfo->SetRights(rights);
        _SetRankBankMoneyPerDay(rankId, moneyPerDay);

        for (GuildBankRightsAndSlotsVec::const_iterator itr = rightsAndSlots.begin(); itr != rightsAndSlots.end(); ++itr)
            _SetRankBankTabRightsAndSlots(rankId, *itr);

        SendGuildEventRankChanged(rankId);
    }
}

void Guild::HandleBuyBankTab(WorldSession* session, uint8 tabId)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();
    if (!player)
        return;

    Member const* member = GetMember(player->GetGUID());
    if (!member)
        return;

    if (tabId != GetPurchasedTabsSize())
        return;

    uint32 tabCost = _GetGuildBankTabPrice(tabId) * GOLD;
    if (!tabCost)
        return;

    if (!player->HasEnoughMoney(uint64(tabCost)))                   // Should not happen, this is checked by client
        return;

    if (!_CreateNewBankTab())
        return;

    player->ModifyMoney(-int64(tabCost));
    SendGuildEventTabAdded();
}

void Guild::HandleSpellEffectBuyBankTab(WorldSession* session, uint8 tabId)
{
    if (!session)
        return;
    if (tabId != GetPurchasedTabsSize())
        return;

    Player* player = session->GetPlayer();
    if (!_CreateNewBankTab())
        return;

    SendGuildEventTabAdded();

    UpdateAchievementCriteria(CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS, tabId + 1, 0, 0, nullptr, player);
}

void Guild::HandleInviteMember(WorldSession* session, std::string const& name)
{
    if (!session)
        return;
    // Strip invisible characters for non-addon messages
    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(const_cast<std::string&>(name));

        if (strchr(name.c_str(), '|'))
        {
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                session->KickPlayer();
            return;
        }
    }

    Player* pInvitee = sObjectAccessor->FindPlayerByName(name);
    if (!pInvitee || pInvitee->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
    {
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PLAYER_NOT_FOUND_S, name);
        return;
    }

    Player* player = session->GetPlayer();
    // Do not show invitations from ignored players
    if (pInvitee->GetSocial()->HasIgnore(player->GetGUID()))
        return;

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && pInvitee->GetTeam() != player->GetTeam())
    {
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_NOT_ALLIED, name);
        return;
    }

    // Invited player cannot be in another guild
    if (pInvitee->GetGuildId())
    {
        SendCommandResult(session, GUILD_INVITE_S, ERR_ALREADY_IN_GUILD_S, name);
        return;
    }

    // Invited player cannot be invited
    if (pInvitee->GetGuildIdInvited())
    {
        SendCommandResult(session, GUILD_INVITE_S, ERR_ALREADY_INVITED_TO_GUILD_S, name);
        return;
    }

    // Inviting player must have rights to invite
    if (!_HasRankRight(player, GR_RIGHT_INVITE))
    {
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
        return;
    }

    TC_LOG_DEBUG(LOG_FILTER_GUILD, "Player %s invited %s to join his Guild", player->GetName(), name.c_str());

    pInvitee->SetGuildIdInvited(m_id, player->GetGUID());
    _LogEvent(GUILD_EVENT_LOG_INVITE_PLAYER, player->GetGUIDLow(), pInvitee->GetGUIDLow());

    WorldPackets::Guild::GuildInvite invite;

    invite.InviterVirtualRealmAddress = GetVirtualRealmAddress();
    invite.GuildVirtualRealmAddress = GetVirtualRealmAddress();
    invite.GuildGUID = GetGUID();

    invite.EmblemStyle = m_emblemInfo.GetStyle();
    invite.EmblemColor = m_emblemInfo.GetColor();
    invite.BorderStyle = m_emblemInfo.GetBorderStyle();
    invite.BorderColor = m_emblemInfo.GetBorderColor();
    invite.Background = m_emblemInfo.GetBackgroundColor();
    invite.AchievementPoints = uint32(GetAchievementMgr().GetAchievementPoints());

    invite.InviterName = player->GetName();
    invite.GuildName = GetName();

    if (Guild* oldGuild = pInvitee->GetGuild())
    {
        invite.OldGuildGUID = oldGuild->GetGUID();
        invite.OldGuildName = oldGuild->GetName();
        invite.OldGuildVirtualRealmAddress = GetVirtualRealmAddress();
    }

    pInvitee->SendDirectMessage(invite.Write());
}

void Guild::HandleAcceptMember(WorldSession* session)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();
    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) &&
        player->GetTeam() != sObjectMgr->GetPlayerTeamByGUID(GetLeaderGUID()))
        return;

    AddMember(player->GetGUID());
}

void Guild::HandleLeaveMember(WorldSession* session)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();
    // If leader is leaving
    if (_IsLeader(player))
    {
        if (m_members.size() > 1)
            // Leader cannot leave if he is not the last member
            SendCommandResult(session, GUILD_QUIT_S, ERR_GUILD_LEADER_LEAVE);
        else
            // Guild is disbanded if leader leaves.
            Disband();
    }
    else
    {
        _LogEvent(GUILD_EVENT_LOG_LEAVE_GUILD, player->GetGUIDLow());
        if (Member * member = GetMember(player->GetGUID()))
            SendGuildEventPlayerLeft(member);

        DeleteMember(player->GetGUID(), false, false);

        SendCommandResult(session, GUILD_QUIT_S, ERR_GUILD_COMMAND_SUCCESS, m_name);
        UpdateGuildRecipes();
    }
}

void Guild::HandleRemoveMember(WorldSession* session, ObjectGuid guid)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();
    Player* removedPlayer = ObjectAccessor::FindPlayer(guid);
    Member* member = GetMember(guid);

    // Player must have rights to remove members
    if (!_HasRankRight(player, GR_RIGHT_REMOVE))
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
    // Removed player must be a member of the guild
    else if (member)
    {
        std::string name = member->GetName();

        // Guild masters cannot be removed
        if (member->IsRank(GR_GUILDMASTER))
            SendCommandResult(session, GUILD_QUIT_S, ERR_GUILD_LEADER_LEAVE);
        // Do not allow to remove player with the same rank or higher
        else
        {
            Member* memberMe = GetMember(player->GetGUID());
            if (!memberMe || member->IsRankNotLower(memberMe->GetRankId()))
                SendCommandResult(session, GUILD_QUIT_S, ERR_GUILD_RANK_TOO_HIGH_S, name);
            else
            {
                // After call to DeleteMember pointer to member becomes invalid
                _LogEvent(GUILD_EVENT_LOG_UNINVITE_PLAYER, player->GetGUIDLow(), guid.GetCounter());
                SendGuildEventPlayerLeft(member, memberMe, true);

                // After call to DeleteMember pointer to member becomes invalid
                DeleteMember(guid, false, true);
            }
        }
    }
    else if (removedPlayer)
        SendCommandResult(session, GUILD_QUIT_S, ERR_GUILD_COMMAND_SUCCESS, removedPlayer->GetName());
}

void Guild::HandleUpdateMemberRank(WorldSession* session, ObjectGuid targetGuid, bool demote)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();

    // Promoted player must be a member of guild
    if (Member* member = GetMember(targetGuid))
    {
        GuildCommandType type = demote ? GUILD_DEMOTE_SS : GUILD_PROMOTE_SS;

        if (!_HasRankRight(player, demote ? GR_RIGHT_DEMOTE : GR_RIGHT_PROMOTE))
        {
            SendCommandResult(session, type, ERR_GUILD_PERMISSIONS);
            return;
        }

        // Player cannot promote himself
        if (member->IsSamePlayer(player->GetGUID()))
        {
            SendCommandResult(session, type, ERR_GUILD_NAME_INVALID);
            return;
        }

        if (demote)
        {
            // Player can demote only lower rank members
            if (member->IsRankNotLower(player->GetRank()))
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_HIGH_S, member->GetName());
                return;
            }
            // Lowest rank cannot be demoted
            if (member->GetRankId() >= _GetLowestRankId())
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_LOW_S, member->GetName());
                return;
            }
        }
        else
        {
            // Allow to promote only to lower rank than member's rank
            // member->GetRank() + 1 is the highest rank that current player can promote to
            if (member->IsRankNotLower(player->GetRank() + 1))
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_HIGH_S, member->GetName());
                return;
            }
        }

        uint32 newRankId = member->GetRankId() + (demote ? 1 : -1);
        member->ChangeRank(newRankId);
        _LogEvent(demote ? GUILD_EVENT_LOG_DEMOTE_PLAYER : GUILD_EVENT_LOG_PROMOTE_PLAYER, player->GetGUIDLow(), member->GetGUID().GetGUIDLow(), newRankId);
        SendGuildRanksUpdate(player->GetGUID(), member->GetGUID(), newRankId, !demote);
    }
}

void Guild::HandleSetMemberRank(WorldSession* session, ObjectGuid targetGuid, ObjectGuid setterGuid, uint32 rank)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();

    // Promoted player must be a member of guild
    if (Member* member = GetMember(targetGuid))
    {
        GuildRankRights rights = GR_RIGHT_PROMOTE;
        GuildCommandType type = GUILD_PROMOTE_SS;

        if (rank > member->GetRankId())
        {
            rights = GR_RIGHT_DEMOTE;
            type = GUILD_DEMOTE_SS;
        }

        bool demote = type == GUILD_DEMOTE_SS;

        if (!_HasRankRight(player, rights))
        {
            SendCommandResult(session, type, ERR_GUILD_PERMISSIONS);
            return;
        }

        // Player cannot promote himself
        if (member->IsSamePlayer(player->GetGUID()))
        {
            SendCommandResult(session, type, ERR_GUILD_NAME_INVALID);
            return;
        }

        if (demote)
        {
            // Player can demote only lower rank members
            if (member->IsRankNotLower(player->GetRank()))
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_HIGH_S, member->GetName());
                return;
            }
            // Lowest rank cannot be demoted
            if (member->GetRankId() >= _GetLowestRankId())
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_LOW_S, member->GetName());
                return;
            }
        }
        else
        {
            // Allow to promote only to lower rank than member's rank
            // member->GetRank() + 1 is the highest rank that current player can promote to
            if (member->IsRankNotLower(player->GetRank() + 1))
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_HIGH_S, member->GetName());
                return;
            }
        }

        member->ChangeRank(rank);
        _LogEvent(demote ? GUILD_EVENT_LOG_DEMOTE_PLAYER : GUILD_EVENT_LOG_PROMOTE_PLAYER, player->GetGUIDLow(), member->GetGUID().GetGUIDLow(), rank);
        SendGuildRanksUpdate(setterGuid, targetGuid, rank, !demote);
    }
}

void Guild::HandleShiftRank(WorldSession* /*session*/, uint32 id, bool up)
{
    uint32 nextID = up ? id - 1 : id + 1;

    RankInfo* rankinfo = GetRankInfo(id);
    RankInfo* rankinfo2 = GetRankInfo(nextID);

    if (!rankinfo || !rankinfo2)
        return;

    RankInfo tmp = NULL;
    tmp = *rankinfo2;
    rankinfo2->SetName(rankinfo->GetName());
    rankinfo2->SetRights(rankinfo->GetRights());
    rankinfo->SetName(tmp.GetName());
    rankinfo->SetRights(tmp.GetRights());

    SendGuildEventRanksUpdated();
}

void Guild::HandleAddNewRank(WorldSession* session, std::string const& name) //, uint32 rankId)
{
    if (!session)
        return;
    if (_GetRanksSize() >= GUILD_RANKS_MAX_COUNT)
        return;

    // Only leader can add new rank
    if (!_IsLeader(session->GetPlayer()))
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
    else
    {
        _CreateRank(name, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
        SendGuildEventRankChanged(_GetRanksSize() - 1);
    }
}

void Guild::HandleRemoveRank(WorldSession* session, uint32 rankId)
{
    if (!session)
        return;
    // Cannot remove rank if total count is minimum allowed by the client
    if (_GetRanksSize() <= GUILD_RANKS_MIN_COUNT)
        return;

    // Only leader can delete ranks
    if (!_IsLeader(session->GetPlayer()))
        SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PERMISSIONS);
    else
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        // Delete bank rights for rank
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_RIGHTS_FOR_RANK);
        stmt->setUInt64(0, m_id);
        stmt->setUInt8(1, rankId);
        CharacterDatabase.ExecuteOrAppend(trans, stmt);
        // Delete rank
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_RANK);
        stmt->setUInt64(0, m_id);
        stmt->setUInt8(1, rankId);
        CharacterDatabase.ExecuteOrAppend(trans, stmt);

        CharacterDatabase.CommitTransaction(trans);

        m_ranks.erase(m_ranks.begin() + rankId);

        // Restruct m_ranks
        for (size_t i = 0; i < m_ranks.size(); ++i)
            if (m_ranks[i].GetId() != i)
                m_ranks[i].UpdateId(i);

        SendGuildEventRanksUpdated();
    }
}

void Guild::HandleMemberDepositMoney(WorldSession* session, uint64 amount, bool cashFlow /*=false*/)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();

    // Call script after validation and before money transfer.
    sScriptMgr->OnGuildMemberDepositMoney(this, player, amount);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // Add money to bank
    _ModifyBankMoney(trans, amount, true);
    // Remove money from player
    player->ModifyMoney(-int64(amount));
    player->SaveGoldToDB(trans);
    // Log GM action (TODO: move to scripts)
    if (!AccountMgr::IsPlayerAccount(player->GetSession()->GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
    {
        sLog->outCommand(player->GetSession()->GetAccountId(),
            "GM %s (Account: %u) deposit money (Amount: " UI64FMTD ") to guild bank (Guild ID %u)",
            player->GetName(), player->GetSession()->GetAccountId(), amount, m_id);
    }
    if (amount >= sWorld->getIntConfig(CONFIG_LOG_GOLD_FROM))
        TC_LOG_DEBUG(LOG_FILTER_GOLD, "GuildDepositMoney: %s GUID %u (Account: %u) deposit money (Amount: " UI64FMTD ") to guild bank (Guild ID %u)", player->GetName(), player->GetGUIDLow(), player->GetSession()->GetAccountId(), amount, m_id);

    // Log guild bank event
    _LogBankEvent(trans, cashFlow ? GUILD_BANK_LOG_CASH_FLOW_DEPOSIT : GUILD_BANK_LOG_DEPOSIT_MONEY, uint8(0), player->GetGUIDLow(), amount);

    CharacterDatabase.CommitTransaction(trans);

    if (!cashFlow)
        SendBankList(session, 0, false);
}

bool Guild::HandleMemberWithdrawMoney(WorldSession* session, uint64 amount, bool repair)
{
    if (!session)
        return false;
    if (m_bankMoney < amount)                               // Not enough money in bank
        return false;

    Player* player = session->GetPlayer();
    if (!_HasRankRight(player, repair ? GR_RIGHT_WITHDRAW_REPAIR : GR_RIGHT_WITHDRAW_GOLD))
        return false;

    int64 remainingMoney = _GetMemberRemainingMoney(player->GetGUID());
    if (!remainingMoney)
        return false;

    if (remainingMoney > 0 && remainingMoney < static_cast<int64>(amount))
        return false;

    // Call script after validation and before money transfer.
    sScriptMgr->OnGuildMemberWitdrawMoney(this, player, amount, repair);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // Update remaining money amount
    if (remainingMoney > 0 && remainingMoney < uint64(GUILD_WITHDRAW_MONEY_UNLIMITED))
        if (Member* member = GetMember(player->GetGUID()))
            member->DecreaseBankRemainingValue(trans, GUILD_BANK_MAX_TABS, amount);
    // Remove money from bank
    _ModifyBankMoney(trans, amount, false);
    // Add money to player (if required)
    if (!repair)
    {
        player->ModifyMoney(amount);
        player->SaveGoldToDB(trans);

        if (amount >= sWorld->getIntConfig(CONFIG_LOG_GOLD_FROM))
            TC_LOG_DEBUG(LOG_FILTER_GOLD, "GuildWithdrawMoney: %s GUID %u (Account: %u) deposit money (Amount: " UI64FMTD ") to guild bank (Guild ID %u)", player->GetName(), player->GetGUIDLow(), player->GetSession()->GetAccountId(), amount, m_id);
    }
    // Log guild bank event
    _LogBankEvent(trans, repair ? GUILD_BANK_LOG_REPAIR_MONEY : GUILD_BANK_LOG_WITHDRAW_MONEY, uint8(0), player->GetGUIDLow(), amount);
    CharacterDatabase.CommitTransaction(trans);

    SendMoneyInfo(session);

    if (!repair)
        SendBankList(session, 0, false);

    return true;
}

void Guild::HandleMemberLogout(WorldSession* session)
{
    RemoveMemberOnline();

    if (!session)
        return;
    Player* player = session->GetPlayer();
    if (Member* member = GetMember(player->GetGUID()))
    {
        member->SetStats(player);
        UpdateGuildRecipes();
        member->UpdateLogoutTime();
        member->SaveStatsToDB(nullptr);
    }

    if (!player->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
        SendGuildEventPresenceChanged(player->GetGUID(), player->GetName(), false);

    SaveToDB(false);
}

void Guild::HandleDisband(WorldSession* session)
{
    if (!session)
        return;
    // Only leader can disband guild
    if (!_IsLeader(session->GetPlayer()))
        SendCommandResult(session, GUILD_QUIT_S, ERR_GUILD_PERMISSIONS);
    else
    {
        Disband();
        TC_LOG_DEBUG(LOG_FILTER_GUILD, "WORLD: Guild Successfully Disbanded");
    }
}

void Guild::HandleGuildPartyRequest(WorldSession* session)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();

    // Make sure player is a member of the guild and that he is in a group.
    if (!IsMember(player->GetGUID()))
        return;

    WorldPackets::Guild::GuildPartyState partyStateResponse;
    partyStateResponse.InGuildParty = (player->GetMap()->GetOwnerGuildId(player->GetTeam()) == GetId());
    partyStateResponse.NumMembers = 0;
    partyStateResponse.NumRequired = 0;
    partyStateResponse.GuildXPEarnedMult = 0.0f;
    player->SendDirectMessage(partyStateResponse.Write());
}

///////////////////////////////////////////////////////////////////////////////
// Send data to client
void Guild::SendEventLog(WorldSession* session) const
{
    if (!session)
        return;
    auto const& player = session->GetPlayer();
    if (!player)
        return;

    GuildLog* logs = m_eventLog->GetGuildLog();
    if (!logs)
        return;

    WorldPackets::Guild::GuildEventLogQueryResults packet;
    packet.Entry.reserve(m_eventLog->GetSize());

    for (auto const& itr : *logs)
    {
        auto eventLog = static_cast<EventLogEntry*>(itr);
        eventLog->WritePacket(packet);
    }

    player->SendDirectMessage(packet.Write());
}

void Guild::SendNewsUpdate(WorldSession* session)
{
    if (!session)
        return;
    auto const& player = session->GetPlayer();
    if (!player)
        return;

    GuildLog* logs = m_newsLog->GetGuildLog();
    if (!logs)
        return;

    WorldPackets::Guild::GuildNews packet;
    packet.NewsEvents.reserve(m_newsLog->GetSize());

    for (auto const& itr : *logs)
    {
        auto* eventLog = static_cast<NewsLogEntry*>(itr);
        eventLog->WritePacket(packet);
    }

    player->SendDirectMessage(packet.Write());
}

void Guild::SendBankLog(WorldSession* session, uint8 tabId) const
{
    if (!session)
        return;
    auto const& player = session->GetPlayer();
    if (!player)
        return;

    if (tabId < GetPurchasedTabsSize() || tabId == GUILD_BANK_MAX_TABS)
    {
        GuildLog* logs = m_bankEventLog[tabId]->GetGuildLog();
        if (!logs)
            return;

        WorldPackets::Guild::GuildBankLogQueryResults packet;
        packet.TabId = int32(tabId);

        //if (tabId == GUILD_BANK_MAX_TABS && hasCashFlow)
        //    packet.WeeklyBonusMoney.Set(uint64(weeklyBonusMoney));

        packet.Entry.reserve(m_bankEventLog[tabId]->GetSize());
        for (auto const& itr : *logs)
        {
            auto* bankEventLog = static_cast<BankEventLogEntry*>(itr);
            bankEventLog->WritePacket(packet);
        }

        player->SendDirectMessage(packet.Write());
    }
}

void Guild::SendBankList(WorldSession* session, uint8 tabId, bool fullUpdate) const
{
    if (!session)
        return;
    auto const& player = session->GetPlayer();
    if (!player)
        return;

    WorldPackets::Guild::GuildBankQueryResults packet;
    packet.Money = m_bankMoney;
    packet.WithdrawalsRemaining = _GetMemberRemainingSlots(player->GetGUID(), tabId);
    packet.Tab = int32(tabId);
    packet.FullUpdate = fullUpdate;

    if (fullUpdate)
    {
        packet.TabInfo.reserve(GetPurchasedTabsSize());
        for (uint8 i = 0; i < GetPurchasedTabsSize(); ++i)
        {
            WorldPackets::Guild::GuildBankTabInfo tabInfo;
            tabInfo.TabIndex = i;
            tabInfo.Name = m_bankTabs[i]->GetName();
            tabInfo.Icon = m_bankTabs[i]->GetIcon();
            packet.TabInfo.push_back(tabInfo);
        }

        if (_MemberHasTabRights(player->GetGUID(), tabId, GUILD_BANK_RIGHT_VIEW_TAB))
        {
            if (BankTab const* tab = GetBankTab(tabId))
            {
                for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS; ++slotId)
                {
                    if (Item* tabItem = tab->GetItem(slotId))
                    {
                        WorldPackets::Guild::GuildBankItemInfo itemInfo;

                        itemInfo.Slot = int32(slotId);
                        itemInfo.Item.ItemID = tabItem->GetEntry();
                        itemInfo.Item.Initialize(tabItem);
                        itemInfo.Count = int32(tabItem->GetCount());
                        itemInfo.Charges = int32(abs(tabItem->GetSpellCharges()));
                        itemInfo.EnchantmentID = int32(tabItem->GetItemRandomPropertyId()); // verify that...
                        itemInfo.OnUseEnchantmentID = 0/*int32(tabItem->GetItemSuffixFactor())*/;
                        itemInfo.Flags = 0;
                        itemInfo.Locked = false;

                        uint8 i = 0;
                        for (ItemDynamicFieldGems const& gemData : tabItem->GetGems())
                        {
                            if (gemData.ItemId)
                            {
                                WorldPackets::Item::ItemGemData gem;
                                gem.Slot = i;
                                gem.Item.Initialize(&gemData);
                                itemInfo.SocketEnchant.push_back(gem);
                            }
                            ++i;
                        }

                        packet.ItemInfo.push_back(itemInfo);
                    }
                }
            }
        }
    }

    player->SendDirectMessage(packet.Write());
}

void Guild::SendBankTabText(WorldSession* session, uint8 tabId) const
{
    if (!session)
        return;
    if (BankTab const* tab = GetBankTab(tabId))
        tab->SendText(this, session);
}

void Guild::SendPermissions(WorldSession* session) const
{
    if (!session)
        return;
    auto const& player = session->GetPlayer();
    if (!player)
        return;

    Member const* member = GetMember(player->GetGUID());
    if (!member)
        return;

    uint8 rankId = member->GetRankId();

    WorldPackets::Guild::GuildPermissionsQueryResults queryResult;
    queryResult.RankID = rankId;
    queryResult.WithdrawGoldLimit = _GetMemberRemainingMoney(member->GetGUID());
    queryResult.Flags = _GetRankRights(rankId);
    queryResult.NumTabs = GetPurchasedTabsSize();
    queryResult.Tab.reserve(GUILD_BANK_MAX_TABS);

    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
    {
        WorldPackets::Guild::GuildPermissionsQueryResults::GuildRankTabPermissions tabPerm;
        tabPerm.Flags = _GetRankBankTabRights(rankId, tabId);
        tabPerm.WithdrawItemLimit = _GetMemberRemainingSlots(member->GetGUID(), tabId);
        queryResult.Tab.push_back(tabPerm);
    }

    player->SendDirectMessage(queryResult.Write());
}

void Guild::SendMoneyInfo(WorldSession* session) const
{
    if (!session)
        return;
    auto const& player = session->GetPlayer();
    if (!player)
        return;

    WorldPackets::Guild::GuildBankRemainingWithdrawMoney packet;
    packet.RemainingWithdrawMoney = int64(_GetMemberRemainingMoney(player->GetGUID()));
    player->SendDirectMessage(packet.Write());
}

void Guild::SendLoginInfo(WorldSession* session)
{
    if (!session)
        return;
    Player* player = session->GetPlayer();
    if (!player)
        return;

    Member* member = GetMember(player->GetGUID());
    if (!member)
        return;

    SendGuildEventMOTD(session);      // verified
    SendGuildRankInfo(session);       // verified!

    if (!player->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
    {
        SendGuildEventPresenceChanged(player->GetGUID(), player->GetName(), true);
        SendGuildEventPresenceChanged(player->GetGUID(), player->GetName(), true, session);
    }


    player->SendDirectMessage(WorldPackets::Guild::GuildMemberDailyReset().Write());

    for (GuildPerkSpellsEntry const* entry : sGuildPerkSpellsStore)
        player->learnSpell(entry->SpellID, true);

    GetAchievementMgr().SendAllAchievementData(player);

    AddMemberOnline();
}

void Guild::SendGuildChallengeUpdated(WorldSession* session /*= nullptr*/)
{
    auto rewards = sGuildMgr->GetGuildChallengeRewardData();
    WorldPackets::Guild::GuildChallengeUpdated update;
    for (uint8 i = 0; i < ChallengeMax; ++i)
    {
        update.Gold[i] = rewards[i].Gold;
        update.CurrentCount[i] = m_ChallengeCount[i];
        update.MaxCount[i] = rewards[i].ChallengeCount;
        update.MaxLevelGold[i] = rewards[i].Gold2;
    }

    if (session != nullptr)
        session->SendPacket(update.Write());
    else
        BroadcastPacket(update.Write());
}

void Guild::CompleteGuildChallenge(uint32 challengeType)
{
    if (challengeType >= ChallengeMax)
        return;

    auto reards = sGuildMgr->GetGuildChallengeRewardData();
    if (m_ChallengeCount[challengeType] >= reards[challengeType].ChallengeCount)
        return;

    m_ChallengeCount[challengeType]++;

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_COMPLETE_GUILD_CHALLENGE);
    stmt->setInt32(0, m_ChallengeCount[challengeType]);
    stmt->setInt32(1, GetId());
    stmt->setInt32(2, challengeType);
    CharacterDatabase.Execute(stmt);

    auto trans = CharacterDatabase.BeginTransaction();
    _ModifyBankMoney(trans, reards[challengeType].Gold2 * GOLD, true);
    CharacterDatabase.CommitTransaction(trans);

    WorldPackets::Guild::GuildChallengeCompleted completed;
    completed.ChallengeType = challengeType;
    completed.CurrentCount = m_ChallengeCount[challengeType];
    completed.MaxCount = reards[challengeType].ChallengeCount;
    completed.GoldAwarded = reards[challengeType].Gold2;
    BroadcastPacket(completed.Write());

    SendGuildChallengeUpdated();
}

///////////////////////////////////////////////////////////////////////////////
// Loading methods
bool Guild::LoadFromDB(Field* fields)
{
    m_id = fields[0].GetUInt64();
    m_name = fields[1].GetString();
    m_leaderGuid = ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64());
    m_flags = fields[3].GetUInt32();
    m_emblemInfo.LoadFromDB(fields);
    m_info = fields[9].GetString();
    m_motd = fields[10].GetString();
    m_createdDate = time_t(fields[11].GetUInt32());
    m_bankMoney = fields[12].GetUInt64();
    _level = fields[13].GetUInt32();

    auto purchasedTabs = uint8(fields[14].GetUInt64());
    if (purchasedTabs > GUILD_BANK_MAX_TABS)
        purchasedTabs = GUILD_BANK_MAX_TABS;

    m_bankTabs.resize(purchasedTabs);
    for (uint8 i = 0; i < purchasedTabs; ++i)
        m_bankTabs[i] = new BankTab(m_id, i);

    _CreateLogHolders();
    return true;
}

void Guild::LoadGuildNewsLogFromDB(Field* fields)
{
    if (!m_newsLog->CanInsert())
        return;

    m_newsLog->LoadEvent(new NewsLogEntry(
        fields[1].GetUInt32(),                              // guid
        fields[6].GetUInt32(),                              // timestamp //64 bits?
        GuildNews(fields[2].GetUInt8()),                    // type
        ObjectGuid::Create<HighGuid::Player>(fields[3].GetUInt64()), // player guid
        fields[4].GetUInt32(),                              // Flags
        fields[5].GetUInt32(),
        fields[7].GetString()));                            // value
}

void Guild::LoadRankFromDB(Field* fields)
{
    RankInfo rankInfo(m_id);
    rankInfo.LoadFromDB(fields);
    m_ranks.push_back(rankInfo);
}

bool Guild::LoadMemberFromDB(Field* fields)
{
    ObjectGuid::LowType lowguid = fields[1].GetUInt64();
    Member *member = new Member(m_id, ObjectGuid::Create<HighGuid::Player>(lowguid), fields[2].GetUInt8());
    if (!member->LoadFromDB(fields))
    {
        _DeleteMemberFromDB(lowguid);
        delete member;
        return false;
    }
    m_members[member->GetGUID()] = member;
    return true;
}

void Guild::LoadBankRightFromDB(Field* fields)
{
                                           // tabId              rights                slots
    GuildBankRightsAndSlots rightsAndSlots(fields[1].GetUInt8(), fields[3].GetUInt8(), fields[4].GetUInt32());
                                  // rankId             tabId
    _SetRankBankTabRightsAndSlots(fields[2].GetUInt8(), rightsAndSlots, false);
}

bool Guild::LoadEventLogFromDB(Field* fields)
{
    if (m_eventLog->CanInsert())
    {
        m_eventLog->LoadEvent(new EventLogEntry(
            m_id,                                       // guild id
            fields[1].GetUInt32(),                      // guid
            time_t(fields[6].GetUInt32()),              // timestamp
            GuildEventLogTypes(fields[2].GetUInt8()),   // event type
            fields[3].GetUInt32(),                      // player guid 1
            fields[4].GetUInt32(),                      // player guid 2
            fields[5].GetUInt8()));                     // rank
        return true;
    }
    return false;
}

bool Guild::LoadBankEventLogFromDB(Field* fields)
{
    uint8 dbTabId = fields[1].GetUInt8();
    bool isMoneyTab = (dbTabId == GUILD_BANK_MONEY_LOGS_TAB);
    if (dbTabId < GetPurchasedTabsSize() || isMoneyTab)
    {
        uint8 tabId = isMoneyTab ? uint8(GUILD_BANK_MAX_TABS) : dbTabId;
        LogHolder* pLog = m_bankEventLog[tabId];
        if (pLog->CanInsert())
        {
            ObjectGuid::LowType guid = fields[2].GetUInt32();
            auto eventType = GuildBankEventLogTypes(fields[3].GetUInt8());
            if (BankEventLogEntry::IsMoneyEvent(eventType))
            {
                if (!isMoneyTab)
                {
                    TC_LOG_ERROR(LOG_FILTER_GUILD, "GuildBankEventLog ERROR: MoneyEvent(LogGuid: %u, Guild: %u) does not belong to money tab (%u), ignoring...", guid, m_id, dbTabId);
                    return false;
                }
            }
            else if (isMoneyTab)
            {
                TC_LOG_ERROR(LOG_FILTER_GUILD, "GuildBankEventLog ERROR: non-money event (LogGuid: %u, Guild: %u) belongs to money tab, ignoring...", guid, m_id);
                return false;
            }

            pLog->LoadEvent(new BankEventLogEntry(
                m_id,                                   // guild id
                guid,                                   // guid
                time_t(fields[8].GetUInt32()),          // timestamp
                dbTabId,                                // tab id
                eventType,                              // event type
                fields[4].GetUInt32(),                  // player guid
                fields[5].GetUInt32(),                  // item or money
                fields[6].GetUInt16(),                  // itam stack count
                fields[7].GetUInt8()));                 // dest tab id
        }
    }
    return true;
}

bool Guild::LoadBankTabFromDB(Field* fields)
{
    uint8 tabId = fields[1].GetUInt8();

    if (tabId >= GetPurchasedTabsSize())
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Invalid tab (tabId: %u) in guild bank, skipped.", tabId);
        return false;
    }

    return m_bankTabs[tabId]->LoadFromDB(fields);
}

bool Guild::LoadBankItemFromDB(Field* fields)
{
    uint8 tabId = fields[51].GetUInt8();

    if (tabId >= GetPurchasedTabsSize())
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Invalid tab for item (GUID: %u, id: #%u) in guild bank, skipped.", fields[0].GetUInt32(), fields[1].GetUInt32());
        return false;
    }

    return m_bankTabs[tabId]->LoadItemFromDB(fields);
}

bool Guild::LoadGuildChallengesFromDB(Field* fields)
{
    if (fields[1].GetInt32() >= ChallengeMax)
        return false;

    m_ChallengeCount[fields[1].GetInt32()] = fields[2].GetInt32();
    return true;
}

// Validates guild data loaded from database. Returns false if guild should be deleted.
bool Guild::Validate()
{
    // Validate ranks data
    // GUILD RANKS represent a sequence starting from 0 = GUILD_MASTER (ALL PRIVILEGES) to max 9 (lowest privileges).
    // The lower rank id is considered higher rank - so promotion does rank-- and demotion does rank++
    // Between ranks in sequence cannot be gaps - so 0, 1, 2, 4 is impossible
    // Min ranks count is 5 and max is 10.
    volatile uint32 guildId = GetId();
    volatile uint32 _getRanksSize = _GetRanksSize();
    volatile uint32 brokenRankId = 0;
    volatile uint32 _leaderGuid = GetLeaderGUID().GetGUIDLow();

    bool broken_ranks = false;
    if (_GetRanksSize() < GUILD_RANKS_MIN_COUNT || _GetRanksSize() > GUILD_RANKS_MAX_COUNT)
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Guild %u has invalid number of ranks, creating new...", m_id);
        broken_ranks = true;
    }
    else
    {
        for (uint8 rankId = 0; rankId < _GetRanksSize(); ++rankId)
        {
            RankInfo* rankInfo = GetRankInfo(rankId);
            if (rankInfo->GetId() != rankId)
            {
                TC_LOG_ERROR(LOG_FILTER_GUILD, "Guild %u has broken rank id %u, creating default set of ranks...", m_id, rankId);
                broken_ranks = true;
            }
            else
            {
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                rankInfo->CreateMissingTabsIfNeeded(GetPurchasedTabsSize(), trans, true);
                CharacterDatabase.CommitTransaction(trans);
            }
        }
    }

    if (broken_ranks)
    {
        m_ranks.clear();
        _CreateDefaultGuildRanks(DEFAULT_LOCALE);
    }

    // Validate members' data
    for (auto& member : m_members)
        if (member.second->GetRankId() > _GetRanksSize())
            member.second->ChangeRank(_GetLowestRankId());

    // Repair the structure of the guild.
    // If the guildmaster doesn't exist or isn't member of the guild
    // attempt to promote another member.
    Member* pLeader = GetMember(m_leaderGuid);
    if (!pLeader)
    {
        DeleteMember(GetLeaderGUID());
        // If no more members left, disband guild
        if (m_members.empty())
        {
            Disband();
            return false;
        }
    }
    else if (!pLeader->IsRank(GR_GUILDMASTER))
        _SetLeaderGUID(pLeader);

    // Check config if multiple guildmasters are allowed
    if (!sConfigMgr->GetBoolDefault("Guild.AllowMultipleGuildMaster", false))
        for (auto& member : m_members)
            if (member.second->GetRankId() == GR_GUILDMASTER && !member.second->IsSamePlayer(GetLeaderGUID()))
                member.second->ChangeRank(GR_OFFICER);

    _UpdateAccountsNumber();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Broadcasts
void Guild::BroadcastToGuild(WorldSession* session, bool officerOnly, std::string const& msg, uint32 language) const
{
    if (session && session->GetPlayer() && _HasRankRight(session->GetPlayer(), officerOnly ? GR_RIGHT_OFFCHATSPEAK : GR_RIGHT_GCHATSPEAK))
    {
        WorldPackets::Chat::Chat packet;
        packet.Initialize(officerOnly ? CHAT_MSG_OFFICER : CHAT_MSG_GUILD, Language(language), session->GetPlayer(), nullptr, msg);
        WorldPacket const* data = packet.Write();
        for (const auto& member : m_members)
            if (Player* player = member.second->FindPlayer())
                if (player->CanContact() && _HasRankRight(player, officerOnly ? GR_RIGHT_OFFCHATLISTEN : GR_RIGHT_GCHATLISTEN) && !player->GetSocial()->HasIgnore(session->GetPlayer()->GetGUID()))
                    player->SendDirectMessage(data);
    }
}

void Guild::BroadcastAddonToGuild(WorldSession* session, bool officerOnly, std::string const& msg, std::string const& prefix) const
{
    if (session && session->GetPlayer() && _HasRankRight(session->GetPlayer(), officerOnly ? GR_RIGHT_OFFCHATSPEAK : GR_RIGHT_GCHATSPEAK))
    {
        WorldPackets::Chat::Chat packet;
        packet.Initialize(officerOnly ? CHAT_MSG_OFFICER : CHAT_MSG_GUILD, LANG_ADDON, session->GetPlayer(), nullptr, msg, 0, "", DEFAULT_LOCALE, prefix);
        WorldPacket const* data = packet.Write();
        for (const auto& member : m_members)
            if (Player* player = member.second->FindPlayer())
                if (player->CanContact() && _HasRankRight(player, officerOnly ? GR_RIGHT_OFFCHATLISTEN : GR_RIGHT_GCHATLISTEN) && !player->GetSocial()->HasIgnore(session->GetPlayer()->GetGUID()) && player->GetSession()->IsAddonRegistered(prefix))
                    player->SendDirectMessage(data);
    }
}

void Guild::BroadcastPacketToRank(WorldPacket const* packet, uint8 rankId) const
{
    for (const auto& member : m_members)
        if (member.second->IsRank(rankId))
            if (Player* player = member.second->FindPlayer())
                player->SendDirectMessage(packet);
}

void Guild::BroadcastPacket(WorldPacket const* packet) const
{
    for (const auto& member : m_members)
        if (Player* player = member.second->FindPlayer())
            player->SendDirectMessage(packet);
}

void Guild::BroadcastPacketIfTrackingAchievement(WorldPacket const* packet, uint32 criteriaId) const
{
    for (auto const& v : m_members)
        if (v.second->IsTrackingCriteriaId(criteriaId))
            if (Player* player = v.second->FindPlayer())
                player->SendDirectMessage(packet);
}

void Guild::MassInviteToEvent(WorldSession* session, uint32 minLevel, uint32 maxLevel, uint32 minRank)
{
    WorldPackets::Calendar::CalendarEventInitialInvites packet;

    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (packet.Invites.size() >= CALENDAR_MAX_INVITES)
        {
            if (Player* player = session->GetPlayer())
                sCalendarMgr->SendCalendarCommandResult(player->GetGUID(), CALENDAR_ERROR_INVITES_EXCEEDED);
            return;
        }

        Member* member = itr->second;
        uint32 level = Player::GetLevelFromDB(member->GetGUID());

        if (member->GetGUID() != session->GetPlayer()->GetGUID() && level >= minLevel && level <= maxLevel && member->IsRankNotLower(minRank))
            packet.Invites.emplace_back(member->GetGUID(), level);
    }

    if (Player* player = session->GetPlayer())
        player->SendDirectMessage(packet.Write());
}

///////////////////////////////////////////////////////////////////////////////
// Members handling
bool Guild::AddMember(ObjectGuid guid, uint8 rankId)
{
    const CharacterInfo* nameData = sWorld->GetCharacterInfo(guid);
    if (!nameData)
        return false;

    Player* player = ObjectAccessor::FindPlayer(guid);
    // Player cannot be in guild
    if (player)
    {
        if (player->GetGuildId() != 0)
            return false;
    }
    else if (nameData->GuildId != 0)
        return false;

    // Remove all player signs from another petitions
    // This will be prevent attempt to join many guilds and corrupt guild data integrity
    Player::RemovePetitionsAndSigns(guid);

    ObjectGuid::LowType lowguid = guid.GetCounter();

    // If rank was not passed, assign lowest possible rank
    if (rankId == GUILD_RANK_NONE)
        rankId = _GetLowestRankId();

    auto member = new Member(m_id, guid, rankId);
    if (player)
        member->SetStats(player);
    else
    {
        bool ok = false;
        // Player must exist
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_DATA_FOR_GUILD);
        stmt->setUInt64(0, lowguid);
        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            Field* fields = result->Fetch();
            member->SetStats(fields[0].GetString(), fields[1].GetUInt8(), fields[2].GetUInt8(), fields[3].GetUInt16(), fields[4].GetUInt32(), fields[5].GetUInt32(), fields[6].GetUInt8(), 0, 0, 0, 0, "", 0, 0, 0, ""); // ach points and professions set on first login

            ok = member->CheckStats();
        }
        if (!ok)
        {
            delete member;
            return false;
        }
    }
    m_members[guid] = member;

    SQLTransaction trans(nullptr);
    member->SaveToDB(trans);

    // If player not in game data in will be loaded from guild tables, so no need to update it!
    if (player)
    {
        player->SetInGuild(m_id);
        player->SetRank(rankId);
        player->SetGuildLevel(GetLevel());
        player->SetGuildIdInvited(0);

        player->AddDelayedEvent(100, [player]() -> void
        {
            for (GuildPerkSpellsEntry const* entry : sGuildPerkSpellsStore)
                player->learnSpell(entry->SpellID, true);

            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(REP_GUILD))
                player->GetReputationMgr().SetReputation(factionEntry, 0);
        });
    }

    _UpdateAccountsNumber();
    _LogEvent(GUILD_EVENT_LOG_JOIN_GUILD, lowguid);
    if (player)
        SendGuildEventPlayerJoined(player->GetGUID(), player->GetName());
    sGuildFinderMgr->RemoveAllMembershipRequestsFromPlayer(guid);
    UpdateGuildRecipes();

    // Call scripts if member was succesfully added (and stored to database)
    sScriptMgr->OnGuildAddMember(this, player, rankId);

    return true;
}

void Guild::DeleteMember(ObjectGuid guid, bool isDisbanding, bool isKicked)
{
    Player* player = ObjectAccessor::FindPlayer(guid);

    // Guild master can be deleted when loading guild and guid doesn't exist in characters table
    // or when he is removed from guild by gm command
    if (GetLeaderGUID() == guid && !isDisbanding)
    {
        Member* oldLeader = nullptr;
        Member* newLeader = nullptr;
        for (auto& member : m_members)
        {
            if (member.first == guid)
                oldLeader = member.second;
            else if (!newLeader || newLeader->GetRankId() > member.second->GetRankId())
                newLeader = member.second;
        }
        if (!newLeader)
        {
            Disband();
            return;
        }

        _SetLeaderGUID(newLeader);

        // If player not online data in data field will be loaded from guild tabs no need to update it !!
        if (Player* newLeaderPlayer = newLeader->FindPlayer())
            newLeaderPlayer->SetRank(GR_GUILDMASTER);

        // If leader does not exist (at guild loading with deleted leader) do not send broadcasts
        if (oldLeader)
        {
            SendGuildEventNewLeader(newLeader, oldLeader, true);
            SendGuildEventPlayerLeft(oldLeader);
        }
    }
    // Call script on remove before member is acutally removed from guild (and database)
    sScriptMgr->OnGuildRemoveMember(this, player, isDisbanding, isKicked);

    delete GetMember(guid);
    m_members.erase(guid);

    // If player not online data in data field will be loaded from guild tabs no need to update it !!
    if (player)
    {
        player->SetInGuild(UI64LIT(0));
        player->SetRank(0);
        player->SetGuildLevel(0);

        for (GuildPerkSpellsEntry const* entry : sGuildPerkSpellsStore)
            player->removeSpell(entry->SpellID, false, false);

        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(REP_GUILD))
            player->GetReputationMgr().SetReputation(factionEntry, 0);
    }

    _DeleteMemberFromDB(guid.GetCounter());

    if (!isDisbanding)
        _UpdateAccountsNumber();
}

bool Guild::ChangeMemberRank(ObjectGuid guid, uint8 newRank)
{
    if (newRank <= _GetLowestRankId())                    // Validate rank (allow only existing ranks)
    {
        if (Member* member = GetMember(guid))
        {
            member->ChangeRank(newRank);
            return true;
        }
    }

    return false;
}

bool Guild::IsMember(ObjectGuid guid)
{
    Members::const_iterator itr = m_members.find(guid);
    return itr != m_members.end();
}

uint32 Guild::GetMembersCount() const
{
    return static_cast<uint32>(m_members.size());
}

///////////////////////////////////////////////////////////////////////////////
// Bank (items move)
void Guild::SwapItems(Player* player, uint8 tabId, uint8 slotId, uint8 destTabId, uint8 destSlotId, uint32 splitedAmount)
{
    if (tabId >= GetPurchasedTabsSize() || slotId >= GUILD_BANK_MAX_SLOTS ||
        destTabId >= GetPurchasedTabsSize() || destSlotId >= GUILD_BANK_MAX_SLOTS)
        return;

    if (tabId == destTabId && slotId == destSlotId)
        return;

    BankMoveItemData from(this, player, tabId, slotId);
    BankMoveItemData to(this, player, destTabId, destSlotId);
    _MoveItems(&from, &to, splitedAmount);
}

void Guild::SwapItemsWithInventory(Player* player, bool toChar, uint8 tabId, uint8 slotId, uint8 playerBag, uint8 playerSlotId, uint32 splitedAmount)
{
    if ((slotId >= GUILD_BANK_MAX_SLOTS && slotId != NULL_SLOT) || tabId >= GetPurchasedTabsSize())
        return;

    BankMoveItemData bankData(this, player, tabId, slotId);
    PlayerMoveItemData charData(this, player, playerBag, playerSlotId);
    if (toChar)
        _MoveItems(&bankData, &charData, splitedAmount);
    else
        _MoveItems(&charData, &bankData, splitedAmount);
}

///////////////////////////////////////////////////////////////////////////////
// Bank tabs
void Guild::SetBankTabText(uint8 tabId, std::string const& text)
{
    if (BankTab* pTab = GetBankTab(tabId))
    {
        pTab->SetText(text);
        SendGuildEventTabTextChanged(tabId);
    }
}

AchievementMgr<Guild>& Guild::GetAchievementMgr()
{
    return m_achievementMgr;
}

uint32 Guild::_GetRanksSize() const
{
    return uint32(m_ranks.size());
}

const Guild::RankInfo* Guild::GetRankInfo(uint32 rankId) const
{
    return rankId < _GetRanksSize() ? &m_ranks[rankId] : nullptr;
}

Guild::RankInfo* Guild::GetRankInfo(uint32 rankId)
{
    return rankId < _GetRanksSize() ? &m_ranks[rankId] : nullptr;
}

bool Guild::_HasRankRight(Player* player, uint32 right) const
{
    return (_GetRankRights(player->GetRank()) & right) != GR_RIGHT_EMPTY;
}

uint32 Guild::_GetLowestRankId() const
{
    return uint32(m_ranks.size() - 1);
}

Guild::BankTab* Guild::GetBankTab(uint8 tabId)
{
    return tabId < m_bankTabs.size() ? m_bankTabs[tabId] : nullptr;
}

const Guild::BankTab* Guild::GetBankTab(uint8 tabId) const
{
    return tabId < m_bankTabs.size() ? m_bankTabs[tabId] : nullptr;
}

const Guild::Member* Guild::GetMember(ObjectGuid guid) const
{
    return Trinity::Containers::MapGetValuePtr(m_members, guid);
}

Guild::Member* Guild::GetMember(ObjectGuid guid)
{
    return Trinity::Containers::MapGetValuePtr(m_members, guid);
}

Guild::Member* Guild::GetMember(WorldSession* session, std::string const& name)
{
    for (auto& member : m_members)
        if (member.second->GetName() == name)
            return member.second;

    SendCommandResult(session, GUILD_INVITE_S, ERR_GUILD_PLAYER_NOT_IN_GUILD_S, name);
    return nullptr;
}

void Guild::_DeleteMemberFromDB(ObjectGuid::LowType const& lowguid)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_MEMBER);
    stmt->setUInt64(0, lowguid);
    CharacterDatabase.Execute(stmt);
}

///////////////////////////////////////////////////////////////////////////////
// Private methods
void Guild::_CreateLogHolders()
{
    m_eventLog = new LogHolder(sWorld->getIntConfig(CONFIG_GUILD_EVENT_LOG_COUNT));
    m_newsLog = new LogHolder(sWorld->getIntConfig(CONFIG_GUILD_NEWS_LOG_COUNT));
    for (uint8 tabId = 0; tabId <= GUILD_BANK_MAX_TABS; ++tabId)
        m_bankEventLog[tabId] = new LogHolder(sWorld->getIntConfig(CONFIG_GUILD_BANK_EVENT_LOG_COUNT));
}

bool Guild::_CreateNewBankTab()
{
    if (GetPurchasedTabsSize() >= GUILD_BANK_MAX_TABS)
        return false;

    uint8 tabId = GetPurchasedTabsSize();                      // Next free id
    m_bankTabs.push_back(new BankTab(m_id, tabId));

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement*  stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_TAB);
    stmt->setUInt64(0, m_id);
    stmt->setUInt8(1, tabId);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_BANK_TAB);
    stmt->setUInt64(0, m_id);
    stmt->setUInt8(1, tabId);
    trans->Append(stmt);

    ++tabId;
    for (auto& rank : m_ranks)
        rank.CreateMissingTabsIfNeeded(tabId, trans, false);

    CharacterDatabase.CommitTransaction(trans);
    return true;
}

void Guild::_CreateDefaultGuildRanks(LocaleConstant loc)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_RANKS);
    stmt->setUInt64(0, m_id);
    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_BANK_RIGHTS);
    stmt->setUInt64(0, m_id);
    CharacterDatabase.Execute(stmt);

    _CreateRank(sObjectMgr->GetTrinityString(LANG_GUILD_MASTER, loc), GR_RIGHT_ALL);
    _CreateRank(sObjectMgr->GetTrinityString(LANG_GUILD_OFFICER, loc), GR_RIGHT_ALL);
    _CreateRank(sObjectMgr->GetTrinityString(LANG_GUILD_VETERAN, loc), GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    _CreateRank(sObjectMgr->GetTrinityString(LANG_GUILD_MEMBER, loc), GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    _CreateRank(sObjectMgr->GetTrinityString(LANG_GUILD_INITIATE, loc), GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
}

void Guild::_CreateRank(std::string const& name, uint32 rights)
{
    if (_GetRanksSize() >= GUILD_RANKS_MAX_COUNT)
        return;

    // Ranks represent sequence 0, 1, 2, ... where 0 means guildmaster
    uint32 newRankId = _GetRanksSize();

    RankInfo info(m_id, newRankId, name, rights, 0);
    m_ranks.push_back(info);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    info.CreateMissingTabsIfNeeded(GetPurchasedTabsSize(), trans);
    info.SaveToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

// Updates the number of accounts that are in the guild
// Player may have many characters in the guild, but with the same account
void Guild::_UpdateAccountsNumber()
{
    // We use a set to be sure each element will be unique
    std::set<uint32> accountsIdSet;
    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        accountsIdSet.insert(itr->second->GetAccountId());

    m_accountsNumber = accountsIdSet.size();
}

// Detects if player is the guild master.
// Check both leader guid and player's rank (otherwise multiple feature with
// multiple guild masters won't work)
bool Guild::_IsLeader(Player* player) const
{
    if (player->GetGUID() == GetLeaderGUID())
        return true;

    if (const Member* member = GetMember(player->GetGUID()))
        return member->IsRank(GR_GUILDMASTER);

    return false;
}

void Guild::_DeleteBankItems(SQLTransaction& trans, bool removeItemsFromDB)
{
    for (uint8 tabId = 0; tabId < GetPurchasedTabsSize(); ++tabId)
    {
        m_bankTabs[tabId]->Delete(trans, removeItemsFromDB);
        delete m_bankTabs[tabId];
        m_bankTabs[tabId] = nullptr;
    }
    m_bankTabs.clear();
}

bool Guild::_ModifyBankMoney(SQLTransaction& trans, uint64 amount, bool add)
{
    if (add)
        m_bankMoney += amount;
    else
    {
        // Check if there is enough money in bank.
        if (m_bankMoney < amount)
            return false;
        m_bankMoney -= amount;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_BANK_MONEY);
    stmt->setUInt64(0, m_bankMoney);
    stmt->setUInt64(1, m_id);
    trans->Append(stmt);
    return true;
}

void Guild::_SetLeaderGUID(Member* pLeader)
{
    if (!pLeader)
        return;

    m_leaderGuid = pLeader->GetGUID();
    pLeader->ChangeRank(GR_GUILDMASTER);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_LEADER);
    stmt->setUInt64(0, GetLeaderGUID().GetCounter());
    stmt->setUInt64(1, m_id);
    CharacterDatabase.Execute(stmt);
}

void Guild::_SetRankBankMoneyPerDay(uint32 rankId, uint32 moneyPerDay)
{
    if (RankInfo* rankInfo = GetRankInfo(rankId))
    {
        for (auto& member : m_members)
            if (member.second->IsRank(rankId))
                member.second->ResetMoneyTime();

        rankInfo->SetBankMoneyPerDay(moneyPerDay);
    }
}

void Guild::_SetRankBankTabRightsAndSlots(uint8 rankId, GuildBankRightsAndSlots rightsAndSlots, bool saveToDB)
{
    if (rightsAndSlots.GetTabId() >= GetPurchasedTabsSize())
        return;

    if (RankInfo* rankInfo = GetRankInfo(rankId))
    {
        for (auto& member : m_members)
            if (member.second->IsRank(rankId))
                member.second->ResetTabTimes();

        rankInfo->SetBankTabSlotsAndRights(rightsAndSlots, saveToDB);
    }
}

inline std::string Guild::_GetRankName(uint32 rankId) const
{
    if (const RankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetName();
    return "<unknown>";
}

inline uint32 Guild::_GetRankRights(uint32 rankId) const
{
    if (const RankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetRights();
    return 0;
}

inline uint32 Guild::_GetRankBankMoneyPerDay(uint32 rankId) const
{
    if (const RankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetBankMoneyPerDay();
    return 0;
}

inline uint32 Guild::_GetRankBankTabSlotsPerDay(uint32 rankId, uint8 tabId) const
{
    if (tabId < GetPurchasedTabsSize())
        if (const RankInfo* rankInfo = GetRankInfo(rankId))
            return rankInfo->GetBankTabSlotsPerDay(tabId);
    return 0;
}

inline uint32 Guild::_GetRankBankTabRights(uint32 rankId, uint8 tabId) const
{
    if (const RankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetBankTabRights(tabId);
    return 0;
}

inline int32 Guild::_GetMemberRemainingSlots(ObjectGuid guid, uint8 tabId) const
{
    if (const Member* member = GetMember(guid))
        return member->GetBankRemainingValue(tabId, this);
    return 0;
}

inline int32 Guild::_GetMemberRemainingMoney(ObjectGuid guid) const
{
    if (const Member* member = GetMember(guid))
        return member->GetBankRemainingValue(GUILD_BANK_MAX_TABS, this);
    return 0;
}

inline void Guild::_DecreaseMemberRemainingSlots(SQLTransaction& trans, ObjectGuid guid, uint8 tabId)
{
    // Remaining slots must be more then 0
    if (uint32 remainingSlots = _GetMemberRemainingSlots(guid, tabId))
        // Ignore guild master
        if (remainingSlots < uint32(GUILD_WITHDRAW_SLOT_UNLIMITED))
            if (Member* member = GetMember(guid))
                member->DecreaseBankRemainingValue(trans, tabId, 1);
}

inline bool Guild::_MemberHasTabRights(ObjectGuid guid, uint8 tabId, uint32 rights) const
{
    if (const Member* member = GetMember(guid))
    {
        // Leader always has full rights
        if (member->IsRank(GR_GUILDMASTER) || GetLeaderGUID() == guid)
            return true;
        return (_GetRankBankTabRights(member->GetRankId(), tabId) & rights) == rights;
    }
    return false;
}

// Add new event log record
inline void Guild::_LogEvent(GuildEventLogTypes eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank)
{
    std::lock_guard<std::recursive_mutex> _event_lock(m_event_lock);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    m_eventLog->AddEvent(trans, new EventLogEntry(m_id, m_eventLog->GetNextGUID(), eventType, playerGuid1, playerGuid2, newRank));
    CharacterDatabase.CommitTransaction(trans);

    sScriptMgr->OnGuildEvent(this, uint8(eventType), playerGuid1, playerGuid2, newRank);
}

// Add new bank event log record
void Guild::_LogBankEvent(SQLTransaction& trans, GuildBankEventLogTypes eventType, uint8 tabId, ObjectGuid::LowType lowguid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId)
{
    if (tabId > GUILD_BANK_MAX_TABS)
        return;

    // not logging moves within the same tab
    if (eventType == GUILD_BANK_LOG_MOVE_ITEM && tabId == destTabId)
        return;

    uint8 dbTabId = tabId;
    if (BankEventLogEntry::IsMoneyEvent(eventType))
    {
        tabId = GUILD_BANK_MAX_TABS;
        dbTabId = GUILD_BANK_MONEY_LOGS_TAB;
    }
    LogHolder* pLog = m_bankEventLog[tabId];
    pLog->AddEvent(trans, new BankEventLogEntry(m_id, pLog->GetNextGUID(), eventType, dbTabId, lowguid, itemOrMoney, itemStackCount, destTabId));

    sScriptMgr->OnGuildBankEvent(this, uint8(eventType), tabId, lowguid, itemOrMoney, itemStackCount, destTabId);
}

inline Item* Guild::_GetItem(uint8 tabId, uint8 slotId) const
{
    if (const BankTab* tab = GetBankTab(tabId))
        return tab->GetItem(slotId);
    return nullptr;
}

inline void Guild::_RemoveItem(SQLTransaction& trans, uint8 tabId, uint8 slotId)
{
    if (BankTab* pTab = GetBankTab(tabId))
        pTab->SetItem(trans, slotId, nullptr);
}

void Guild::_MoveItems(MoveItemData* pSrc, MoveItemData* pDest, uint32 splitedAmount)
{
    // 1. Initialize source item
    if (!pSrc->InitItem())
        return; // No source item

    // 2. Check source item
    if (!pSrc->CheckItem(splitedAmount))
        return; // Source item or splited amount is invalid
    /*
    if (pItemSrc->GetCount() == 0)
    {
        TC_LOG_FATAL(LOG_FILTER_GENERAL, "Guild::SwapItems: Player %s(GUIDLow: %u) tried to move item %u from tab %u slot %u to tab %u slot %u, but item %u has a stack of zero!",
            player->GetName(), player->GetGUIDLow(), pItemSrc->GetEntry(), tabId, slotId, destTabId, destSlotId, pItemSrc->GetEntry());
        //return; // Commented out for now, uncomment when it's verified that this causes a crash!!
    }
    // */

    // 3. Check destination rights
    if (!pDest->HasStoreRights(pSrc))
        return; // Player has no rights to store item in destination

    // 4. Check source withdraw rights
    if (!pSrc->HasWithdrawRights(pDest))
        return; // Player has no rights to withdraw items from source

    // 5. Check split
    if (splitedAmount)
    {
        // 5.1. Clone source item
        if (!pSrc->CloneItem(splitedAmount))
            return; // Item could not be cloned

        // 5.2. Move splited item to destination
        _DoItemsMove(pSrc, pDest, true, splitedAmount);
    }
    else // 6. No split
    {
        // 6.1. Try to merge items in destination (pDest->GetItem() == nullptr)
        if (!_DoItemsMove(pSrc, pDest, false)) // Item could not be merged
        {
            // 6.2. Try to swap items
            // 6.2.1. Initialize destination item
            if (!pDest->InitItem())
                return;

            // 6.2.2. Check rights to store item in source (opposite direction)
            if (!pSrc->HasStoreRights(pDest))
                return; // Player has no rights to store item in source (opposite direction)

            if (!pDest->HasWithdrawRights(pSrc))
                return; // Player has no rights to withdraw item from destination (opposite direction)

            // 6.2.3. Swap items (pDest->GetItem() != nullptr)
            _DoItemsMove(pSrc, pDest, true);
        }
    }

    // 7. Send changes
    SendGuildEventBankContentsChanged();    //call client to send list
    _SendBankContentUpdate(pSrc, pDest);   //after SendGuildEventBankSlotChanged client ask for double update, but we will send full
}

bool Guild::_DoItemsMove(MoveItemData* pSrc, MoveItemData* pDest, bool sendError, uint32 splitedAmount)
{
    Item* pDestItem = pDest->GetItem();
    bool swap = (pDestItem != nullptr);

    Item* pSrcItem = pSrc->GetItem(splitedAmount != 0);
    // 1. Can store source item in destination
    if (!pDest->CanStore(pSrcItem, swap, sendError))
        return false;

    // 2. Can store destination item in source
    if (swap)
        if (!pSrc->CanStore(pDestItem, true, true))
            return false;

    // GM LOG (TODO: move to scripts)
    pDest->LogAction(pSrc);
    if (swap)
        pSrc->LogAction(pDest);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // 3. Log bank events
    pDest->LogBankEvent(trans, pSrc, pSrcItem->GetCount());
    if (swap)
        pSrc->LogBankEvent(trans, pDest, pDestItem->GetCount());

    // 4. Remove item from source
    pSrc->RemoveItem(trans, pDest, splitedAmount);

    // 5. Remove item from destination
    if (swap)
        pDest->RemoveItem(trans, pSrc);

    // 6. Store item in destination
    pDest->StoreItem(trans, pSrcItem);

    // 7. Store item in source
    if (swap)
        pSrc->StoreItem(trans, pDestItem);

    CharacterDatabase.CommitTransaction(trans);
    return true;
}

void Guild::_SendBankContentUpdate(MoveItemData* pSrc, MoveItemData* pDest) const
{
    ASSERT(pSrc->IsBank() || pDest->IsBank());

    uint8 tabId = 0;
    SlotIds slots;
    if (pSrc->IsBank()) // B ->
    {
        tabId = pSrc->GetContainer();
        slots.insert(pSrc->GetSlotId());
        if (pDest->IsBank()) // B -> B
        {
            // Same tab - add destination slots to collection
            if (pDest->GetContainer() == pSrc->GetContainer())
                pDest->CopySlots(slots);
            else // Different tabs - send second message
            {
                SlotIds destSlots;
                pDest->CopySlots(destSlots);
                _SendBankContentUpdate(pDest->GetContainer(), destSlots);
            }
        }
    }
    else if (pDest->IsBank()) // C -> B
    {
        tabId = pDest->GetContainer();
        pDest->CopySlots(slots);
    }

    _SendBankContentUpdate(tabId, slots);
}

void Guild::_SendBankContentUpdate(uint8 tabId, SlotIds slots) const
{
    if (BankTab const* tab = GetBankTab(tabId))
    {
        WorldPackets::Guild::GuildBankQueryResults packet;
        packet.FullUpdate = true;          // @todo
        packet.Tab = int32(tabId);
        packet.Money = m_bankMoney;

        for (auto const& v : slots)
        {
            WorldPackets::Guild::GuildBankItemInfo itemInfo;

            itemInfo.Slot = v;
            itemInfo.Flags = 0;
            itemInfo.Locked = false;

            if (Item const* tabItem = tab->GetItem(v))
            {
                itemInfo.Item.ItemID = tabItem->GetEntry();
                itemInfo.Item.Initialize(tabItem);
                itemInfo.Count = tabItem->GetCount();
                itemInfo.Charges = abs(tabItem->GetSpellCharges());
                itemInfo.OnUseEnchantmentID = 0/*int32(tabItem->GetItemSuffixFactor())*/;

                uint8 i = 0;
                for (ItemDynamicFieldGems const& gemData : tabItem->GetGems())
                {
                    if (gemData.ItemId)
                    {
                        WorldPackets::Item::ItemGemData gem;
                        gem.Slot = i;
                        gem.Item.Initialize(&gemData);
                        itemInfo.SocketEnchant.push_back(gem);
                    }
                    ++i;
                }
            }

            packet.ItemInfo.push_back(itemInfo);
        }

        for (auto const& x : m_members)
            if (_MemberHasTabRights(x.second->GetGUID(), tabId, GUILD_BANK_RIGHT_VIEW_TAB))
                if (Player* player = x.second->FindPlayer())
                {
                    packet.WithdrawalsRemaining = _GetMemberRemainingSlots(x.second->GetGUID(), tabId);
                    player->SendDirectMessage(packet.Write());
                }
    }
}

void Guild::SendGuildRanksUpdate(ObjectGuid setterGuid, ObjectGuid targetGuid, uint32 rank, bool promote)
{
    Member* member = GetMember(targetGuid);
    ASSERT(member);

    WorldPackets::Guild::GuildSendRankChange rankChange;
    rankChange.Officer = setterGuid;
    rankChange.Other = targetGuid;
    rankChange.RankID = rank;
    rankChange.Promote = promote;
    BroadcastPacket(rankChange.Write());
}

void Guild::RewardReputation(Player* player, uint32 amount)
{
    if (!amount || !player)
        return;

    if (GetMember(player->GetGUID()))
    {
        // Guild Champion Tabard
        if (Unit::AuraEffectList const* auras = player->GetAuraEffectsByType(SPELL_AURA_MOD_REPUTATION_GAIN_PCT))
            for (Unit::AuraEffectList::const_iterator i = auras->begin(); i != auras->end(); ++i)
                AddPct(amount, (*i)->GetAmount());

        if (amount)
        {
            if (FactionEntry const* entry = sFactionStore.LookupEntry(REP_GUILD))
                player->GetReputationMgr().ModifyReputation(entry, int32(amount));
        }
    }
}

AchievementMgr<Guild> const& Guild::GetAchievementMgr() const
{
    return m_achievementMgr;
}

void Guild::ResetWeek()
{
    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        itr->second->ResetValues();
        if (Player* player = itr->second->FindPlayer())
            player->SendDirectMessage(WorldPackets::Guild::GuildMemberDailyReset().Write());
    }
}

void Guild::SendGuildEventBankContentsChanged()
{
    BroadcastPacket(WorldPackets::Guild::GuildEventBankContentsChanged().Write());
}

void Guild::SendEventBankMoneyChanged()
{
    WorldPackets::Guild::GuildEventBankMoneyChanged eventPacket;
    eventPacket.Money = GetBankMoney();
    BroadcastPacket(eventPacket.Write());
}

void Guild::SendGuildEventDisbanded()
{
    BroadcastPacket(WorldPackets::Guild::GuildEventDisbanded().Write());
}

void Guild::SendGuildEventMOTD(WorldSession* session)
{
    WorldPackets::Guild::GuildEventMotd eventPacket;
    eventPacket.MotdText = GetMOTD();

    if (session)
    {
        if (Player* player = session->GetPlayer())
            player->SendDirectMessage(eventPacket.Write());
    }
    else
        BroadcastPacket(eventPacket.Write());
}

void Guild::SendGuildEventNewLeader(Member* newLeader, Member* oldLeader, bool isSelfPromoted)
{
    WorldPackets::Guild::GuildEventNewLeader eventPacket;
    eventPacket.SelfPromoted = isSelfPromoted;

    if (newLeader)
    {
        eventPacket.NewLeaderGUID = newLeader->GetGUID();
        eventPacket.NewLeaderName = newLeader->GetName();
        eventPacket.NewLeaderVirtualRealmAddress = GetVirtualRealmAddress();
    }

    if (oldLeader)
    {
        eventPacket.OldLeaderGUID = oldLeader->GetGUID();
        eventPacket.OldLeaderName = oldLeader->GetName();
        eventPacket.OldLeaderVirtualRealmAddress = GetVirtualRealmAddress();
    }

    BroadcastPacket(eventPacket.Write());
}

void Guild::SendGuildEventPlayerJoined(ObjectGuid const& guid, std::string name)
{
    WorldPackets::Guild::GuildEventPlayerJoined joinNotificationPacket;
    joinNotificationPacket.Guid = guid;
    joinNotificationPacket.Name = name;
    joinNotificationPacket.VirtualRealmAddress = GetVirtualRealmAddress();
    BroadcastPacket(joinNotificationPacket.Write());
}

void Guild::SendGuildEventPlayerLeft(Member* leaver, Member* remover, bool isRemoved)
{
    WorldPackets::Guild::GuildEventPlayerLeft eventPacket;
    eventPacket.Removed = isRemoved;
    eventPacket.LeaverGUID = leaver->GetGUID();
    eventPacket.LeaverName = leaver->GetName();
    eventPacket.LeaverVirtualRealmAddress = GetVirtualRealmAddress();

    if (isRemoved && remover != nullptr)
    {
        eventPacket.RemoverGUID = remover->GetGUID();
        eventPacket.RemoverName = remover->GetName();
        eventPacket.RemoverVirtualRealmAddress = GetVirtualRealmAddress();
    }

    BroadcastPacket(eventPacket.Write());
}

void Guild::SendGuildEventPresenceChanged(ObjectGuid const& guid, std::string name, bool online, WorldSession* session)
{
    WorldPackets::Guild::GuildEventPresenceChange eventPacket;
    eventPacket.Guid = guid;
    eventPacket.Name = name;
    eventPacket.VirtualRealmAddress = GetVirtualRealmAddress();
    eventPacket.LoggedOn = online;
    eventPacket.Mobile = false;

    if (session)
    {
        if (Player* player = session->GetPlayer())
            player->SendDirectMessage(eventPacket.Write());
    }
    else
        BroadcastPacket(eventPacket.Write());
}

void Guild::SendGuildEventRankChanged(uint32 rankId)
{
    WorldPackets::Guild::GuildEventRankChanged packet;
    packet.RankID = rankId;
    BroadcastPacket(packet.Write());
}

void Guild::SendGuildEventRanksUpdated()
{
    BroadcastPacket(WorldPackets::Guild::GuildEventRanksUpdated().Write());
}

void Guild::SendGuildEventTabAdded()
{
    BroadcastPacket(WorldPackets::Guild::GuildEventTabAdded().Write());
}

void Guild::SendGuildEventTabModified(uint8 tabId, std::string name, std::string icon)
{
    WorldPackets::Guild::GuildEventTabModified packet;
    packet.Tab = tabId;
    packet.Name = name;
    packet.Icon = icon;
    BroadcastPacket(packet.Write());
}

void Guild::SendGuildEventTabTextChanged(uint32 tabId)
{
    WorldPackets::Guild::GuildEventTabTextChanged eventPacket;
    eventPacket.Tab = tabId;
    BroadcastPacket(eventPacket.Write());
}

Guild::KnownRecipesMap const& Guild::GetGuildRecipes()
{
    std::lock_guard<std::recursive_mutex> guard(m_guildRecipeslock);
    return _guildRecipes;
}

Guild::KnownRecipes& Guild::GetGuildRecipes(uint32 skillId)
{
    std::lock_guard<std::recursive_mutex> guard(m_guildRecipeslock);
    return _guildRecipes[skillId];
}

void Guild::AddMemberOnline()
{
    m_members_online++;
}

void Guild::RemoveMemberOnline()
{
    if (m_members_online > 0)
        m_members_online--;
}

uint32 Guild::GetMembersOnline() const
{
    return m_members_online;
}

EmblemInfo const& Guild::GetEmblemInfo() const
{
    return m_emblemInfo;
}

uint8 Guild::GetPurchasedTabsSize() const
{
    return uint8(m_bankTabs.size());
}

void Guild::AddGuildNews(uint8 type, ObjectGuid guid, uint32 flags, uint32 value, Item* item)
{
    std::lock_guard<std::recursive_mutex> _newsevent_lock(m_newsevent_lock);

    std::ostringstream dataListIDs;
    if (item)
    {
        dataListIDs << item->GetItemSuffixFactor() << ' ';
        dataListIDs << item->GetItemRandomPropertyId() << ' ';
        dataListIDs << item->GetUInt32Value(ITEM_FIELD_CONTEXT) << ' ';
        for (uint32 bonusListID : item->GetDynamicValues(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS))
            dataListIDs << bonusListID << ' ';
    }

    auto news = new NewsLogEntry(m_id, m_newsLog->GetNextGUID(), GuildNews(type), guid, flags, value, dataListIDs.str());

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    m_newsLog->AddEvent(trans, news);
    CharacterDatabase.CommitTransaction(trans);

    WorldPackets::Guild::GuildNews newsPacket;
    newsPacket.NewsEvents.reserve(1);
    news->WritePacket(newsPacket);
    BroadcastPacket(newsPacket.Write());
}

void Guild::HandleNewsSetSticky(WorldSession* session, uint32 newsId, bool sticky)
{
    GuildLog* logs = m_newsLog->GetGuildLog();
    auto itr = logs->begin();
    while (itr != logs->end() && (*itr)->GetGUID() != newsId)
        ++itr;

    if (itr == logs->end())
        return;

    auto news = static_cast<NewsLogEntry*>(*itr);
    news->SetSticky(sticky);

    WorldPackets::Guild::GuildNews newsPacket;
    newsPacket.NewsEvents.reserve(1);
    news->WritePacket(newsPacket);
    if (Player* player = session->GetPlayer())
        player->SendDirectMessage(newsPacket.Write());
}

void Guild::KnownRecipes::GenerateMask(uint32 skillId, std::set<uint32> const& spells)
{
    Clear();

    uint32 index = 0;
    for (SkillLineAbilityEntry const* entry : sDB2Manager._skillLineAbilityContainer[skillId])
    {
        ++index;
        if (spells.find(entry->Spell) == spells.end())
            continue;

        if (index / 8 > KNOW_RECIPES_MASK_SIZE)
            break;

        recipesMask[index / 8] |= 1 << (index % 8);
    }
}

std::string Guild::KnownRecipes::GetMaskForSave() const
{
    std::stringstream ss;
    for (auto i : recipesMask)
        ss << uint32(i) << " ";

    return ss.str();
}

void Guild::KnownRecipes::LoadFromString(std::string const& str)
{
    Clear();

    Tokenizer tok(str, ' ');
    for (size_t i = 0; i < tok.size(); ++i)
        recipesMask[i] = atoi(tok[i]);
}

Guild::Member::RemainingValue::RemainingValue() : value(0), resetTime(0)
{
}

Guild::Member::ProfessionInfo::ProfessionInfo(uint32 _skillId, uint32 _skillValue, uint32 _skillRank) : skillId(_skillId), skillValue(_skillValue), skillRank(_skillRank)
{
}

Guild::Member::ProfessionInfo::ProfessionInfo() : skillId(0), skillValue(0), skillRank(0)
{
}

Guild::KnownRecipes::KnownRecipes()
{
    Clear();
}

void Guild::KnownRecipes::Clear()
{
    memset(recipesMask, 0, sizeof(recipesMask));
}

bool Guild::KnownRecipes::IsEmpty() const
{
    for (auto i : recipesMask)
        if (i)
            return false;

    return true;
}

void Guild::UpdateGuildRecipes(uint32 skillId)
{
    std::lock_guard<std::recursive_mutex> guard(m_guildRecipeslock);
    if (skillId)
        _guildRecipes[skillId].Clear();
    else
        _guildRecipes.clear();

    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        for (uint32 i = 0; i < MAX_GUILD_PROFESSIONS; ++i)
        {
            auto const& info = itr->second->GetProfessionInfo(i);

            if (info.skillId && (!skillId || info.skillId == skillId))
                for (uint32 j = 0; j < KNOW_RECIPES_MASK_SIZE; ++j)
                    _guildRecipes[info.skillId].recipesMask[j] |= info.knownRecipes.recipesMask[j];
        }
    }
}

void Guild::SendGuildMembersForRecipeResponse(WorldSession* session, uint32 skillId, uint32 spellId)
{
    uint32 index = 0;
    bool found = false;
    for (SkillLineAbilityEntry const* entry : sDB2Manager._skillLineAbilityContainer[skillId])
    {
        ++index;
        if (entry->Spell == spellId)
        {
            found = true;
            break;
        }
    }

    if (!found || index / 8 > KNOW_RECIPES_MASK_SIZE)
        return;

    GuidSet guids;
    for (auto const& v : m_members)
    {
        for (uint32 i = 0; i < MAX_GUILD_PROFESSIONS; ++i)
        {
            Member::ProfessionInfo const& info = v.second->GetProfessionInfo(i);
            if (info.skillId == skillId && info.knownRecipes.recipesMask[index / 8] & (1 << (index % 8)))
                guids.insert(v.second->GetGUID());
        }
    }

    WorldPackets::Guild::QueryGuildMembersForRecipeReponse response;
    response.SkillLineID = skillId;
    response.SpellID = spellId;
    for (auto const& guid : guids)
        response.Member.emplace_back(guid);
    session->GetPlayer()->SendDirectMessage(response.Write());
}

void Guild::SendGuildMemberRecipesResponse(WorldSession* session, ObjectGuid playerGuid, uint32 skillId)
{
    Member* member = GetMember(playerGuid);
    if (!member)
        return;

    for (uint32 i = 0; i < MAX_GUILD_PROFESSIONS; ++i)
    {
        Member::ProfessionInfo const& info = member->GetProfessionInfo(i);
        if (info.skillId == skillId)
        {
            WorldPackets::Guild::GuildMemberRecipes packet;
            packet.Member = playerGuid;
            packet.SkillLineID = info.skillId;
            packet.SkillRank = info.skillRank;
            packet.SkillStep = info.skillValue;
            for (uint16 x = 0; x < KNOW_RECIPES_MASK_SIZE; ++x)
                packet.SkillLineBitArray[x] = info.knownRecipes.recipesMask[x];
            session->GetPlayer()->SendDirectMessage(packet.Write());
            return;
        }
    }
}

void Guild::UpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 /*= 0*/, uint32 miscValue2 /*= 0*/, uint32 miscValue3 /*= 0*/, Unit* unit /*= NULL*/, Player* referencePlayer /*= NULL*/)
{
    AchievementCachePtr referenceCache = std::make_shared<AchievementCache>(referencePlayer, unit, type, miscValue1, miscValue2, miscValue3);

    GetAchievementMgr().UpdateAchievementCriteria(referenceCache);
}


void Guild::SetGuildName(const std::string& name)
{
    m_name = name;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_NAME);
    stmt->setString(0, m_name);
    stmt->setUInt64(1, m_id);
    CharacterDatabase.Execute(stmt);

    WorldPackets::Guild::GuildNameChanged nameChanged;
    nameChanged.GuildGUID = GetGUID();
    nameChanged.GuildName = name;
    // global or visible players?
    // ClntObjMgrEnumVisibleObjectsPtr(CGGuildInfo::UpdateGuildNameUnitCallback, guildGUID);
    sWorld->SendGlobalMessage(nameChanged.Write());
}

void Guild::SetRename(bool apply)
{
    if (apply)
        m_flags |= GUILD_FLAG_RENAME;
    else
        m_flags &= ~GUILD_FLAG_RENAME;

    // TODO: temporary only for rename
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_FLAGS);
    stmt->setUInt32(0, m_flags);
    stmt->setUInt64(1, m_id);
    CharacterDatabase.Execute(stmt);

    WorldPackets::Guild::GuildFlaggedForRename flagged;
    flagged.FlagSet = apply;
    BroadcastPacket(flagged.Write());
}

bool Guild::IsFlaggedForRename() const
{
    return (GUILD_FLAG_RENAME & m_flags);
}
