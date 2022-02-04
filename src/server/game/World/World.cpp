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

/** \file
    \ingroup world
*/

#include <atomic>
#include <boost/algorithm/string.hpp>

#include "AccountMgr.h"
#include "AchievementMgr.h"
#include "AddonMgr.h"
#include "Anticheat.h"
#include "AreaTriggerData.h"
#include "AuctionHouseMgr.h"
#include "AuctionHouseBot.h"
#include "AuthenticationPackets.h"
#include "BattlefieldMgr.h"
#include "BattlegroundMgr.h"
#include "BattlenetRpcErrorCodes.h"
#include "BattlePayData.h"
#include "BattlePetData.h"
#include "BlackMarketMgr.h"
#include "BracketMgr.h"
#include "CalendarMgr.h"
#include "CellImpl.h"
#include "ChallengeMgr.h"
#include "Channel.h"
#include "CharacterData.h"
#include "CharacterDatabaseCleaner.h"
#include "Chat.h"
#include "ChatPackets.h"
#include "Common.h"
#include "ConditionMgr.h"
#include "Config.h"
#include "ContributionMgr.h"
#include "ConversationData.h"
#include "CreatureAIRegistry.h"
#include "CreatureGroups.h"
#include "CreatureTextMgr.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "DisableMgr.h"
#include "EventObjectData.h"
#include "GameEventMgr.h"
#include "GameTables.h"
#include "GarrisonMgr.h"
#include "GitRevision.h"
#include "GlobalFunctional.h"
#include "GossipData.h"
#include "GridNotifiersImpl.h"
#include "GroupMgr.h"
#include "GuildFinderMgr.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "ItemEnchantmentMgr.h"
#include "Language.h"
#include "LFGListMgr.h"
#include "LFGMgr.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "MMapFactory.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "PetBattleSystem.h"
#include "Player.h"
#include "PlayerDump.h"
#include "PoolMgr.h"
#include "QuestData.h"
#include "ScenarioMgr.h"
#include "ScriptMgr.h"
#include "ScriptsData.h"
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "SmartAI.h"
#include "SpellMgr.h"
#include "TaxiPathGraph.h"
#include "ThreadPoolMgr.hpp"
#include "TicketMgr.h"
#include "TransportMgr.h"
#include "Util.h"
#include "Vehicle.h"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "Warden.h"
#include "WardenMgr.h"
#include "WaypointMovementGenerator.h"
#include "WeatherMgr.h"
#include "WildBattlePet.h"
#include "WordFilterMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "WorldStateMgr.h"

uint8 World::m_ExitCode = SHUTDOWN_EXIT_CODE;
std::atomic<uint32> World::m_worldLoopCounter(0);

float World::m_MaxVisibleDistanceOnContinents = DEFAULT_VISIBILITY_DISTANCE;
float World::m_MaxVisibleDistanceInInstances  = DEFAULT_VISIBILITY_INSTANCE;
float World::m_MaxVisibleDistanceInBG   = DEFAULT_VISIBILITY_BGARENAS;
float World::m_MaxVisibleDistanceInArenas   = DEFAULT_VISIBILITY_BGARENAS;

int32 World::m_visibility_notify_periodOnContinents = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
int32 World::m_visibility_notify_periodInInstances  = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
int32 World::m_visibility_notify_periodInBGArenas   = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
int32 World::m_visibilityAINotifyDelay   = DEFAULT_VISIBILITY_NOTIFY_PERIOD;

float World::m_visibilityRelocationLowerLimit = 20.0f;
float World::m_visibilityRelocationLowerLimitC = 20.0f;
float World::Relocation_UpdateUnderwateLimit = 20.0f;
float World::ZoneUpdateDistanceRangeLimit = 5.0f;

uint64 World::SendSize[OPCODE_COUNT] = { 0 };
uint64 World::SendCount[OPCODE_COUNT] = { 0 };

/// World constructor
World::World() : isEventKillStart(false), mail_timer(0), mail_timer_expires(0), blackmarket_timer(0), m_updateTime(0), m_currentTime(0), m_sessionCount(0), m_maxSessionCount(0),
rate_values{}, m_int_configs{}, m_bool_configs{}, m_float_configs{}, m_NextRandomBGReset(0), m_NextServerRestart(0)
{
    m_playerLimit = 0;
    m_allowedSecurityLevel = SEC_PLAYER;
    m_allowMovement = true;
    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_gameTime = time(nullptr);
    m_startTime = m_gameTime;
    m_maxActiveSessionCount = 0;
    m_maxQueuedSessionCount = 0;
    m_PlayerCount = 0;
    m_MaxPlayerCount = 0;
    m_NextDailyQuestReset = 0;
    m_NextWeeklyReset = 0;
    m_NextCurrencyReset = 0;
    m_NextInstanceDailyReset = 0;
    m_NextInstanceWeeklyReset = 0;
    m_NextChallengeKeyReset = 0;
    m_NextWorldQuestHourlyReset = 0;
    m_NextWorldQuestDailyReset = 0;
    m_NextInvasionPointReset = 0;
    m_NextBanWaveTime = 0;

    m_pvpMysticCount = 0;

    m_defaultDbcLocale = LOCALE_enUS;

    m_updateTimeSum = 0;
    m_updateTimeCount = 0;

    m_isClosed = false;

    m_CleaningFlags = 0;
}

/// World destructor
World::~World()
{
    CliCommandHolder* command = nullptr;
    while (cliCmdQueue.next(command))
        delete command;

    VMAP::VMapFactory::clear();
    MMAP::MMapFactory::clear();

    for (size_t i = 0; i < _characterInfoStore.size(); ++i)
        if (_characterInfoStore[i])
            delete _characterInfoStore[i];

    //TODO free addSessQueue
}

/// Find a player in a specified zone
Player* World::FindPlayerInZone(uint32 zone)
{
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second)
            continue;

        Player* player = itr->second->GetPlayer();
        if (!player)
            continue;

        if (player->IsInWorld() && player->GetCurrentZoneID() == zone)
        {
            // Used by the weather system. We return the player to broadcast the change weather message to him and all players in the zone.
            return player;
        }
    }
    return nullptr;
}

bool World::IsClosed() const
{
    return m_isClosed;
}

void World::SetClosed(bool val)
{
    m_isClosed = val;

    // Invert the value, for simplicity for scripters.
    sScriptMgr->OnOpenStateChange(!val);
}

void World::SetMotd(std::string motd)
{
    _motd.clear();
    boost::split(_motd, motd, boost::is_any_of("@"));

    sScriptMgr->OnMotdChange(motd);
}

StringVector const& World::GetMotd() const
{
    return _motd;
}

World* World::instance()
{
    static World instance;
    return &instance;
}/// Find a session by its id
WorldSessionPtr World::FindSession(uint32 id) const
{
    SessionMap::const_iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end())
        return itr->second;                                 // also can return nullptr for kicked session
    return nullptr;
}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
    ///- Find the session, kick the user, but we can't delete session at this moment to prevent iterator invalidation
    SessionMap::const_iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end())
    {
        // if (itr->second->PlayerLoading())
            // return false;

        if (itr->second)
            itr->second->KickPlayer();
    }

    return true;
}

void World::AddSession(WorldSessionPtr s)
{
    addSessQueue.add(s);
}

void World::AddInstanceSocket(std::weak_ptr<WorldSocket> sock, uint64 connectToKey)
{
    _linkSocketQueue.add(std::make_pair(sock, connectToKey));
}

void World::AddSession_(WorldSessionPtr s)
{
    ASSERT (s);

    //NOTE - Still there is race condition in WorldSession* being used in the Sockets

    ///- kick already loaded player with same account (if any) and remove session
    ///- if player is in loading and want to load again, return
    if (!RemoveSession(s->GetAccountId()))
    {
        s->KickPlayer();
        return;
    }

    // decrease session counts only at not reconnection case
    bool decrease_session = true;

    // if session already exist, prepare to it deleting at next world update
    // NOTE - KickPlayer() should be called on "old" in RemoveSession()
    {
        SessionMap::const_iterator old = m_sessions.find(s->GetAccountId());

        if (old != m_sessions.end())
        {
            // prevent decrease sessions count if session queued
            if (RemoveQueuedPlayer(old->second))
                decrease_session = false;
            // not remove replaced session form queue if listed

            if (old->second)
            {
                old->second->SetMap(nullptr);
                old->second->KickPlayer();
            }

            m_sessions.erase(old);
        }
    }

    m_sessions[s->GetAccountId()] = s;

    uint32 Sessions = GetActiveAndQueuedSessionCount();
    uint32 pLimit = GetPlayerAmountLimit();
    uint32 QueueSize = GetQueuedSessionCount();             //number of players in the queue

    //so we don't count the user trying to
    //login as a session and queue the socket that we are using
    if (decrease_session)
        --Sessions;

    if (pLimit > 0 && Sessions >= pLimit && AccountMgr::IsPlayerAccount(s->GetSecurity()) && !HasRecentlyDisconnected(s))
    {
        AddQueuedPlayer(s);
        UpdateMaxSessionCounters();
        TC_LOG_INFO(LOG_FILTER_GENERAL, "PlayerQueue: Account id %u is in Queue Position (%u).", s->GetAccountId(), ++QueueSize);
        return;
    }

    s->InitializeSession();

    UpdateMaxSessionCounters();

    // Updates the population
    if (pLimit > 0)
    {
        float popu = static_cast<float>(GetActiveSessionCount());              // updated number of users on the server
        popu /= pLimit;
        popu *= 2;
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Server Population (%f).", popu);
    }
}

void World::ProcessLinkInstanceSocket(std::pair<std::weak_ptr<WorldSocket>, uint64> linkInfo)
{
    if (std::shared_ptr<WorldSocket> sock = linkInfo.first.lock())
    {
        if (!sock->IsOpen())
            return;

        WorldSession::ConnectToKey key;
        key.Raw = linkInfo.second;

        WorldSessionPtr session = FindSession(uint32(key.Fields.AccountId));
        if (!session || session->GetConnectToInstanceKey() != linkInfo.second)
        {
            sock->SendAuthResponseError(ERROR_TIMED_OUT);
            sock->DelayedCloseSocket();
            if (session)
                session->SetforceExit();
            return;
        }

        sock->SetWorldSession(session);
        session->AddInstanceConnection(sock);
        session->HandleContinuePlayerLogin();
    }
}

bool World::HasRecentlyDisconnected(WorldSessionPtr session)
{
    if (!session)
        return false;

    if (uint32 tolerance = getIntConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
    {
        for (DisconnectMap::iterator i = m_disconnects.begin(); i != m_disconnects.end();)
        {
            if (difftime(i->second, time(nullptr)) < tolerance)
            {
                if (i->first == session->GetAccountId())
                    return true;
                ++i;
            }
            else
                m_disconnects.erase(i++);
        }
    }
    return false;
 }

int32 World::GetQueuePos(WorldSessionPtr sess)
{
    uint32 position = 1;

    for (Queue::const_iterator iter = m_QueuedPlayer.begin(); iter != m_QueuedPlayer.end(); ++iter, ++position)
        if ((*iter) == sess)
            return position;

    return 0;
}

void World::AddQueuedPlayer(WorldSessionPtr sess)
{
    sess->SetInQueue(true);
    m_QueuedPlayer.push_back(sess);

    // The 1st SMSG_AUTH_RESPONSE needs to contain other info too.
    sess->SendAuthResponse(ERROR_OK, true, GetQueuePos(sess));
}

bool World::RemoveQueuedPlayer(WorldSessionPtr sess)
{
    // sessions count including queued to remove (if removed_session set)
    uint32 sessions = GetActiveSessionCount();

    uint32 position = 1;
    Queue::iterator iter = m_QueuedPlayer.begin();

    // search to remove and count skipped positions
    bool found = false;

    for (; iter != m_QueuedPlayer.end(); ++iter, ++position)
    {
        if ((*iter) == sess)
        {
            sess->SetInQueue(false);
            sess->ResetTimeOutTime();
            iter = m_QueuedPlayer.erase(iter);
            found = true;                                   // removing queued session
            break;
        }
    }

    // iter point to next socked after removed or end()
    // position store position of removed socket and then new position next socket after removed

    // if session not queued then we need decrease sessions count
    if (!found && sessions)
        --sessions;

    // accept first in queue
    if ((!m_playerLimit || sessions < m_playerLimit) && !m_QueuedPlayer.empty())
    {
        WorldSessionPtr pop_sess = m_QueuedPlayer.front();
        pop_sess->InitializeSession();
        m_QueuedPlayer.pop_front();

        // update iter to point first queued socket or end() if queue is empty now
        iter = m_QueuedPlayer.begin();
        position = 1;
    }

    // update position from iter to end()
    // iter point to first not updated socket, position store new position
    for (; iter != m_QueuedPlayer.end(); ++iter, ++position)
        (*iter)->SendAuthWaitQue(position);

    return found;
}

/// Initialize config values
void World::LoadConfigSettings(bool reload)
{
    if (reload)
    {
        std::string configError;
        if (!sConfigMgr->Reload(configError))
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "World settings reload fail: %s.", configError.c_str());
            return;
        }
        sLog->LoadFromConfig();
    }

    m_defaultDbcLocale = LocaleConstant(sConfigMgr->GetIntDefault("DBC.Locale", 0));
    if (m_defaultDbcLocale >= MAX_LOCALES || m_defaultDbcLocale < LOCALE_enUS || m_defaultDbcLocale == LOCALE_none)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Incorrect DBC.Locale! Must be >= 0 and < %d and not %d (set to 0)", MAX_LOCALES, LOCALE_none);
        m_defaultDbcLocale = LOCALE_enUS;
    }

    ///- Read the player limit and the Message of the day from the config file
    SetPlayerAmountLimit(sConfigMgr->GetIntDefault("MaxAccountsOnline", 1000));
    SetMotd(sConfigMgr->GetStringDefault("Motd", "Welcome to a Trinity Core Server."));

    ///- Read ticket system setting from the config file
    m_bool_configs[CONFIG_ALLOW_TICKETS] = sConfigMgr->GetBoolDefault("AllowTickets", true);

    ///- Get string for new logins (newly created characters)
    SetNewCharString(sConfigMgr->GetStringDefault("PlayerStart.String", ""));

    ///- Send server info on login?
    m_int_configs[CONFIG_ENABLE_SINFO_LOGIN] = sConfigMgr->GetIntDefault("Server.LoginInfo", 0);

    ///- Read all rates from the config file
    rate_values[RATE_HEALTH]      = sConfigMgr->GetFloatDefault("Rate.Health", 1);
    if (rate_values[RATE_HEALTH] < 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Rate.Health (%f) must be > 0. Using 1 instead.", rate_values[RATE_HEALTH]);
        rate_values[RATE_HEALTH] = 1;
    }
    rate_values[RATE_POWER_MANA]  = sConfigMgr->GetFloatDefault("Rate.Mana", 1);
    if (rate_values[RATE_POWER_MANA] < 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Rate.Mana (%f) must be > 0. Using 1 instead.", rate_values[RATE_POWER_MANA]);
        rate_values[RATE_POWER_MANA] = 1;
    }
    rate_values[RATE_POWER_RAGE_INCOME] = sConfigMgr->GetFloatDefault("Rate.Rage.Income", 1);
    rate_values[RATE_POWER_RAGE_LOSS]   = sConfigMgr->GetFloatDefault("Rate.Rage.Loss", 1);
    if (rate_values[RATE_POWER_RAGE_LOSS] < 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Rate.Rage.Loss (%f) must be > 0. Using 1 instead.", rate_values[RATE_POWER_RAGE_LOSS]);
        rate_values[RATE_POWER_RAGE_LOSS] = 1;
    }
    rate_values[RATE_POWER_RUNICPOWER_INCOME] = sConfigMgr->GetFloatDefault("Rate.RunicPower.Income", 1);
    rate_values[RATE_POWER_RUNICPOWER_LOSS]   = sConfigMgr->GetFloatDefault("Rate.RunicPower.Loss", 1);
    if (rate_values[RATE_POWER_RUNICPOWER_LOSS] < 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Rate.RunicPower.Loss (%f) must be > 0. Using 1 instead.", rate_values[RATE_POWER_RUNICPOWER_LOSS]);
        rate_values[RATE_POWER_RUNICPOWER_LOSS] = 1;
    }
    rate_values[RATE_POWER_FOCUS]  = sConfigMgr->GetFloatDefault("Rate.Focus", 1.0f);
    rate_values[RATE_POWER_ENERGY] = sConfigMgr->GetFloatDefault("Rate.Energy", 1.0f);

    rate_values[RATE_SKILL_DISCOVERY]      = sConfigMgr->GetFloatDefault("Rate.Skill.Discovery", 1.0f);

    rate_values[RATE_DROP_ITEM_POOR]       = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Poor", 1.0f);
    rate_values[RATE_DROP_ITEM_NORMAL]     = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Normal", 1.0f);
    rate_values[RATE_DROP_ITEM_UNCOMMON]   = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Uncommon", 1.0f);
    rate_values[RATE_DROP_ITEM_RARE]       = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Rare", 1.0f);
    rate_values[RATE_DROP_ITEM_EPIC]       = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Epic", 1.0f);
    rate_values[RATE_DROP_ITEM_LEGENDARY]  = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Legendary", 1.0f);
    rate_values[RATE_DROP_ITEM_ARTIFACT]   = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Artifact", 1.0f);
    rate_values[RATE_DROP_ITEM_REFERENCED] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Referenced", 1.0f);
    rate_values[RATE_DROP_ITEM_REFERENCED_AMOUNT] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.ReferencedAmount", 1.0f);
    m_bool_configs[CONFIG_DROP_DUNGEON_ONLY_X1] = sConfigMgr->GetBoolDefault("Rate.Drop.Dungeon", false);
    rate_values[RATE_DROP_CURRENCY]        =  sConfigMgr->GetFloatDefault("Rate.Drop.Currency", 1.0f);
    rate_values[RATE_DROP_CURRENCY_AMOUNT] =  sConfigMgr->GetFloatDefault("Rate.Drop.Currency.Amount", 1.0f);
    rate_values[RATE_DROP_MONEY]    = sConfigMgr->GetFloatDefault("Rate.Drop.Money", 1.0f);
    rate_values[RATE_DROP_ZONE_LOOT]    = sConfigMgr->GetFloatDefault("Rate.Drop.Zone.Loot", 1.0f);
    
    // XP Rates
    rate_values[RATE_XP_KILL]       = sConfigMgr->GetFloatDefault("Rate.XP.Kill", 1.0f);
    rate_values[RATE_XP_QUEST]      = sConfigMgr->GetFloatDefault("Rate.XP.Quest", 1.0f);
    rate_values[RATE_XP_EXPLORE]    = sConfigMgr->GetFloatDefault("Rate.XP.Explore", 1.0f);
    rate_values[RATE_XP_GATHERING]  = sConfigMgr->GetFloatDefault("Rate.XP.Gathering", 1.0f);

    rate_values[RATE_REPAIRCOST]    = sConfigMgr->GetFloatDefault("Rate.RepairCost", 1.0f);
    if (rate_values[RATE_REPAIRCOST] < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Rate.RepairCost (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_REPAIRCOST]);
        rate_values[RATE_REPAIRCOST] = 0.0f;
    }
    rate_values[RATE_REPUTATION_GAIN]  = sConfigMgr->GetFloatDefault("Rate.Reputation.Gain", 1.0f);
    rate_values[RATE_REPUTATION_LOWLEVEL_KILL]  = sConfigMgr->GetFloatDefault("Rate.Reputation.LowLevel.Kill", 1.0f);
    rate_values[RATE_REPUTATION_LOWLEVEL_QUEST]  = sConfigMgr->GetFloatDefault("Rate.Reputation.LowLevel.Quest", 1.0f);
    rate_values[RATE_REPUTATION_RECRUIT_A_FRIEND_BONUS] = sConfigMgr->GetFloatDefault("Rate.Reputation.RecruitAFriendBonus", 0.1f);
    rate_values[RATE_CREATURE_NORMAL_DAMAGE]          = sConfigMgr->GetFloatDefault("Rate.Creature.Normal.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_ELITE_DAMAGE]     = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.Elite.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_DAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RAREELITE.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RARE_DAMAGE]      = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RARE.Damage", 1.0f);
    rate_values[RATE_CREATURE_NORMAL_HP]          = sConfigMgr->GetFloatDefault("Rate.Creature.Normal.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_ELITE_HP]     = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.Elite.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_HP] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RAREELITE.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_HP] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RARE_HP]      = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RARE.HP", 1.0f);
    rate_values[RATE_CREATURE_NORMAL_SPELLDAMAGE]          = sConfigMgr->GetFloatDefault("Rate.Creature.Normal.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE]     = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.Elite.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RAREELITE.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RARE_SPELLDAMAGE]      = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RARE.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_AGGRO]  = sConfigMgr->GetFloatDefault("Rate.Creature.Aggro", 1.0f);
    rate_values[RATE_REST_INGAME]                    = sConfigMgr->GetFloatDefault("Rate.Rest.InGame", 1.0f);
    rate_values[RATE_REST_OFFLINE_IN_TAVERN_OR_CITY] = sConfigMgr->GetFloatDefault("Rate.Rest.Offline.InTavernOrCity", 1.0f);
    rate_values[RATE_REST_OFFLINE_IN_WILDERNESS]     = sConfigMgr->GetFloatDefault("Rate.Rest.Offline.InWilderness", 1.0f);
    rate_values[RATE_DAMAGE_FALL]  = sConfigMgr->GetFloatDefault("Rate.Damage.Fall", 1.0f);
    rate_values[RATE_AUCTION_TIME]  = sConfigMgr->GetFloatDefault("Rate.Auction.Time", 1.0f);
    rate_values[RATE_AUCTION_DEPOSIT] = sConfigMgr->GetFloatDefault("Rate.Auction.Deposit", 1.0f);
    rate_values[RATE_AUCTION_CUT] = sConfigMgr->GetFloatDefault("Rate.Auction.Cut", 1.0f);
    rate_values[RATE_HONOR] = sConfigMgr->GetFloatDefault("Rate.Honor", 1.0f);
    rate_values[RATE_HONOR_QB] = sConfigMgr->GetFloatDefault("Rate.Honor.QuestBg", 1.0f);
    rate_values[RATE_MINING_AMOUNT] = sConfigMgr->GetFloatDefault("Rate.Mining.Amount", 1.0f);
    rate_values[RATE_MINING_NEXT]   = sConfigMgr->GetFloatDefault("Rate.Mining.Next", 1.0f);
    rate_values[RATE_TALENT] = sConfigMgr->GetFloatDefault("Rate.Talent", 1.0f);
    if (rate_values[RATE_TALENT] < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Rate.Talent (%f) must be > 0. Using 1 instead.", rate_values[RATE_TALENT]);
        rate_values[RATE_TALENT] = 1.0f;
    }
    rate_values[RATE_MOVESPEED] = sConfigMgr->GetFloatDefault("Rate.MoveSpeed", 1.0f);
    rate_values[RATE_ONLINE] = sConfigMgr->GetFloatDefault("Rate.Online", 1.0f);
    rate_values[RATE_DONATE] = sConfigMgr->GetFloatDefault("Rate.Donate", 1.0f);
    if (rate_values[RATE_MOVESPEED] < 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Rate.MoveSpeed (%f) must be > 0. Using 1 instead.", rate_values[RATE_MOVESPEED]);
        rate_values[RATE_MOVESPEED] = 1.0f;
    }
    for (uint8 i = 0; i < MAX_MOVE_TYPE; ++i) playerBaseMoveSpeed[i] = baseMoveSpeed[i] * rate_values[RATE_MOVESPEED];
    rate_values[RATE_CORPSE_DECAY_LOOTED] = sConfigMgr->GetFloatDefault("Rate.Corpse.Decay.Looted", 0.5f);

    rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = sConfigMgr->GetFloatDefault("TargetPosRecalculateRange", 1.5f);
    if (rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] < CONTACT_DISTANCE)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "TargetPosRecalculateRange (%f) must be >= %f. Using %f instead.", rate_values[RATE_TARGET_POS_RECALCULATION_RANGE], CONTACT_DISTANCE, CONTACT_DISTANCE);
        rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = CONTACT_DISTANCE;
    }
    else if (rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] > NOMINAL_MELEE_RANGE)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "TargetPosRecalculateRange (%f) must be <= %f. Using %f instead.",
            rate_values[RATE_TARGET_POS_RECALCULATION_RANGE], NOMINAL_MELEE_RANGE, NOMINAL_MELEE_RANGE);
        rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = NOMINAL_MELEE_RANGE;
    }

    rate_values[RATE_DURABILITY_LOSS_ON_DEATH]  = sConfigMgr->GetFloatDefault("DurabilityLoss.OnDeath", 10.0f);
    if (rate_values[RATE_DURABILITY_LOSS_ON_DEATH] < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DurabilityLoss.OnDeath (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_ON_DEATH]);
        rate_values[RATE_DURABILITY_LOSS_ON_DEATH] = 0.0f;
    }
    if (rate_values[RATE_DURABILITY_LOSS_ON_DEATH] > 100.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DurabilityLoss.OnDeath (%f) must be <= 100. Using 100.0 instead.", rate_values[RATE_DURABILITY_LOSS_ON_DEATH]);
        rate_values[RATE_DURABILITY_LOSS_ON_DEATH] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_ON_DEATH] = rate_values[RATE_DURABILITY_LOSS_ON_DEATH] / 100.0f;

    rate_values[RATE_DURABILITY_LOSS_DAMAGE] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Damage", 0.5f);
    if (rate_values[RATE_DURABILITY_LOSS_DAMAGE] < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DurabilityLossChance.Damage (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_DAMAGE]);
        rate_values[RATE_DURABILITY_LOSS_DAMAGE] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_ABSORB] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Absorb", 0.5f);
    if (rate_values[RATE_DURABILITY_LOSS_ABSORB] < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DurabilityLossChance.Absorb (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_ABSORB]);
        rate_values[RATE_DURABILITY_LOSS_ABSORB] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_PARRY] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Parry", 0.05f);
    if (rate_values[RATE_DURABILITY_LOSS_PARRY] < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DurabilityLossChance.Parry (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_PARRY]);
        rate_values[RATE_DURABILITY_LOSS_PARRY] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_BLOCK] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Block", 0.05f);
    if (rate_values[RATE_DURABILITY_LOSS_BLOCK] < 0.0f)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DurabilityLossChance.Block (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_BLOCK]);
        rate_values[RATE_DURABILITY_LOSS_BLOCK] = 0.0f;
    }

    m_bool_configs[CONFIG_DURABILITY_LOSS_IN_PVP] = sConfigMgr->GetBoolDefault("DurabilityLoss.InPvP", false);

    m_int_configs[CONFIG_COMPRESSION] = sConfigMgr->GetIntDefault("Compression", 1);
    if (m_int_configs[CONFIG_COMPRESSION] < 1 || m_int_configs[CONFIG_COMPRESSION] > 9)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Compression level (%i) must be in range 1..9. Using default compression level (1).", m_int_configs[CONFIG_COMPRESSION]);
        m_int_configs[CONFIG_COMPRESSION] = 1;
    }
    m_bool_configs[CONFIG_ADDON_CHANNEL] = sConfigMgr->GetBoolDefault("AddonChannel", true);
    m_bool_configs[CONFIG_CLEAN_CHARACTER_DB] = sConfigMgr->GetBoolDefault("CleanCharacterDB", false);
    m_int_configs[CONFIG_PERSISTENT_CHARACTER_CLEAN_FLAGS] = sConfigMgr->GetIntDefault("PersistentCharacterCleanFlags", 0);
    m_int_configs[CONFIG_CHAT_CHANNEL_LEVEL_REQ] = sConfigMgr->GetIntDefault("ChatLevelReq.Channel", 1);
    m_int_configs[CONFIG_CHAT_WHISPER_LEVEL_REQ] = sConfigMgr->GetIntDefault("ChatLevelReq.Whisper", 1);
    m_int_configs[CONFIG_CHAT_SAY_LEVEL_REQ] = sConfigMgr->GetIntDefault("ChatLevelReq.Say", 1);
    m_int_configs[CONFIG_TRADE_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Trade", 1);
    m_int_configs[CONFIG_TICKET_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Ticket", 1);
    m_int_configs[CONFIG_AUCTION_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Auction", 1);
    m_int_configs[CONFIG_MAIL_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Mail", 1);
    m_int_configs[CONFIG_MAIL_GOLD_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelGoldReq.Mail", 15);
    m_bool_configs[CONFIG_ALLOW_PLAYER_COMMANDS] = sConfigMgr->GetBoolDefault("AllowPlayerCommands", false);
    m_bool_configs[CONFIG_PRESERVE_CUSTOM_CHANNELS] = sConfigMgr->GetBoolDefault("PreserveCustomChannels", false);
    m_int_configs[CONFIG_PRESERVE_CUSTOM_CHANNEL_DURATION] = sConfigMgr->GetIntDefault("PreserveCustomChannelDuration", 14);
    m_bool_configs[CONFIG_GRID_UNLOAD] = sConfigMgr->GetBoolDefault("GridUnload", true);
    m_int_configs[CONFIG_INTERVAL_SAVE] = sConfigMgr->GetIntDefault("PlayerSaveInterval", 15 * MINUTE * IN_MILLISECONDS);
    m_int_configs[CONFIG_INTERVAL_DISCONNECT_TOLERANCE] = sConfigMgr->GetIntDefault("DisconnectToleranceInterval", 0);

    m_int_configs[CONFIG_INTERVAL_GRIDCLEAN] = sConfigMgr->GetIntDefault("GridCleanUpDelay", 5 * MINUTE * IN_MILLISECONDS);
    if (m_int_configs[CONFIG_INTERVAL_GRIDCLEAN] < MIN_GRID_DELAY)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "GridCleanUpDelay (%i) must be greater %u. Use this minimal value.", m_int_configs[CONFIG_INTERVAL_GRIDCLEAN], MIN_GRID_DELAY);
        m_int_configs[CONFIG_INTERVAL_GRIDCLEAN] = MIN_GRID_DELAY;
    }
    if (reload)
        sMapMgr->SetGridCleanUpDelay(m_int_configs[CONFIG_INTERVAL_GRIDCLEAN]);

    m_int_configs[CONFIG_INTERVAL_MAPUPDATE] = sConfigMgr->GetIntDefault("MapUpdateInterval", 100);
    if (m_int_configs[CONFIG_INTERVAL_MAPUPDATE] < MIN_MAP_UPDATE_DELAY)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MapUpdateInterval (%i) must be greater %u. Use this minimal value.", m_int_configs[CONFIG_INTERVAL_MAPUPDATE], MIN_MAP_UPDATE_DELAY);
        m_int_configs[CONFIG_INTERVAL_MAPUPDATE] = MIN_MAP_UPDATE_DELAY;
    }

    m_int_configs[CONFIG_INTERVAL_INSTANCE_UPDATE] = sConfigMgr->GetIntDefault("InstanceUpdateInterval", 50);
    if (m_int_configs[CONFIG_INTERVAL_INSTANCE_UPDATE] < MIN_MAP_UPDATE_DELAY)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "InstanceUpdateInterval (%i) must be greater %u. Use this minimal value.", m_int_configs[CONFIG_INTERVAL_INSTANCE_UPDATE], MIN_MAP_UPDATE_DELAY);
        m_int_configs[CONFIG_INTERVAL_INSTANCE_UPDATE] = MIN_MAP_UPDATE_DELAY;
    }

    m_int_configs[CONFIG_INTERVAL_PVP_MAP_UPDATE] = sConfigMgr->GetIntDefault("PvpMapUpdateInterval", 25);
    if (m_int_configs[CONFIG_INTERVAL_PVP_MAP_UPDATE] < 25)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "PvpMapUpdateInterval (%i) must be greater %u. Use this minimal value.", m_int_configs[CONFIG_INTERVAL_PVP_MAP_UPDATE], 25);
        m_int_configs[CONFIG_INTERVAL_PVP_MAP_UPDATE] = 25;
    }

    m_int_configs[CONFIG_INTERVAL_OBJECT_UPDATE] = sConfigMgr->GetIntDefault("ObjectUpdateInterval", 200);
    if (m_int_configs[CONFIG_INTERVAL_OBJECT_UPDATE] < MIN_MAP_UPDATE_DELAY)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "ObjectUpdateInterval (%i) must be greater %u. Use this minimal value.", m_int_configs[CONFIG_INTERVAL_OBJECT_UPDATE], MIN_MAP_UPDATE_DELAY);
        m_int_configs[CONFIG_INTERVAL_OBJECT_UPDATE] = MIN_MAP_UPDATE_DELAY;
    }

    if (reload)
        sMapMgr->SetMapUpdateInterval(m_int_configs[CONFIG_INTERVAL_MAPUPDATE]);

    m_int_configs[CONFIG_INTERVAL_MAP_SESSION_UPDATE] = sConfigMgr->GetIntDefault("MapSessionUpdateInterval", 10);

    m_int_configs[CONFIG_INTERVAL_CHANGEWEATHER] = sConfigMgr->GetIntDefault("ChangeWeatherInterval", 10 * MINUTE * IN_MILLISECONDS);

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("WorldServerPort", 8085);
        if (val != m_int_configs[CONFIG_PORT_WORLD])
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "WorldServerPort option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_PORT_WORLD]);

        val = sConfigMgr->GetIntDefault("InstanceServerPort", 8086);
        if (val != m_int_configs[CONFIG_PORT_INSTANCE])
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "InstanceServerPort option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_PORT_INSTANCE]);
    }
    else
    {
        m_int_configs[CONFIG_PORT_WORLD] = sConfigMgr->GetIntDefault("WorldServerPort", 8085);
        m_int_configs[CONFIG_PORT_INSTANCE] = sConfigMgr->GetIntDefault("InstanceServerPort", 8086);
    }

    m_int_configs[CONFIG_SOCKET_TIMEOUTTIME] = sConfigMgr->GetIntDefault("SocketTimeOutTime", 900000);
    m_int_configs[CONFIG_SESSION_ADD_DELAY] = sConfigMgr->GetIntDefault("SessionAddDelay", 10000);
    m_int_configs[CONFIG_WORLD_SESSION_EXPIRE_TIME] = sConfigMgr->GetIntDefault("WorldSessionExpireTime", 60000);

    m_int_configs[CONFIG_WORLD_PLAYER_COMMAND_TIMER] = sConfigMgr->GetIntDefault("PlayerCommand.Timer", 5000);
    m_bool_configs[CONFIG_PLAYER_AURA_SCAN_COMMAND] = sConfigMgr->GetBoolDefault("PlayerCommand.AuraScan", true);
    m_int_configs[CONFIG_PLAYER_INVISIBLE_STATUS_COMMAND] = sConfigMgr->GetIntDefault("PlayerCommand.InvisibleStatus", 1800);

    m_int_configs[CONFIG_ARENA_1V1_COUNTDOWN] = sConfigMgr->GetIntDefault("Countdown.Arena1v1", 0);
    m_int_configs[CONFIG_ARENA_2V2_COUNTDOWN] = sConfigMgr->GetIntDefault("Countdown.Arena2v2", 0);
    m_int_configs[CONFIG_ARENA_3V3_COUNTDOWN] = sConfigMgr->GetIntDefault("Countdown.Arena3v3", 0);

    m_float_configs[CONFIG_GROUP_XP_DISTANCE] = sConfigMgr->GetFloatDefault("MaxGroupXPDistance", 74.0f);
    m_float_configs[CONFIG_MAX_RECRUIT_A_FRIEND_DISTANCE] = sConfigMgr->GetFloatDefault("MaxRecruitAFriendBonusDistance", 100.0f);

    /// \todo Add MonsterSight and GuarderSight (with meaning) in worldserver.conf or put them as define
    m_float_configs[CONFIG_SIGHT_MONSTER] = sConfigMgr->GetFloatDefault("MonsterSight", 50);
    m_float_configs[CONFIG_SIGHT_GUARDER] = sConfigMgr->GetFloatDefault("GuarderSight", 50);

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("GameType", 0);
        if (val != m_int_configs[CONFIG_GAME_TYPE])
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "GameType option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_GAME_TYPE]);
    }
    else
        m_int_configs[CONFIG_GAME_TYPE] = sConfigMgr->GetIntDefault("GameType", 0);

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT);
        if (val != m_int_configs[CONFIG_REALM_ZONE])
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "RealmZone option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_REALM_ZONE]);
    }
    else
        m_int_configs[CONFIG_REALM_ZONE] = sConfigMgr->GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT);

    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_ACCOUNTS]            = sConfigMgr->GetBoolDefault("AllowTwoSide.Accounts", true);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CALENDAR]= sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Calendar", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT]    = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Chat", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL] = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Channel", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP]   = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Group", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD]   = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Guild", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION] = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Auction", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_MAIL]    = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Mail", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFG]     = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Lfg", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFR]     = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Lfr", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_WHO_LIST]            = sConfigMgr->GetBoolDefault("AllowTwoSide.WhoList", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND]          = sConfigMgr->GetBoolDefault("AllowTwoSide.AddFriend", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_TRADE]               = sConfigMgr->GetBoolDefault("AllowTwoSide.trade", false);
    m_int_configs[CONFIG_STRICT_PLAYER_NAMES]                 = sConfigMgr->GetIntDefault ("StrictPlayerNames",  0);
    m_int_configs[CONFIG_STRICT_CHARTER_NAMES]                = sConfigMgr->GetIntDefault ("StrictCharterNames", 0);
    m_int_configs[CONFIG_STRICT_PET_NAMES]                    = sConfigMgr->GetIntDefault ("StrictPetNames",     0);

    m_int_configs[CONFIG_MIN_PLAYER_NAME]                     = sConfigMgr->GetIntDefault ("MinPlayerName",  2);
    if (m_int_configs[CONFIG_MIN_PLAYER_NAME] < 1 || m_int_configs[CONFIG_MIN_PLAYER_NAME] > MAX_PLAYER_NAME)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MinPlayerName (%i) must be in range 1..%u. Set to 2.", m_int_configs[CONFIG_MIN_PLAYER_NAME], MAX_PLAYER_NAME);
        m_int_configs[CONFIG_MIN_PLAYER_NAME] = 2;
    }

    m_int_configs[CONFIG_MIN_CHARTER_NAME]                    = sConfigMgr->GetIntDefault ("MinCharterName", 2);
    if (m_int_configs[CONFIG_MIN_CHARTER_NAME] < 1 || m_int_configs[CONFIG_MIN_CHARTER_NAME] > MAX_CHARTER_NAME)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MinCharterName (%i) must be in range 1..%u. Set to 2.", m_int_configs[CONFIG_MIN_CHARTER_NAME], MAX_CHARTER_NAME);
        m_int_configs[CONFIG_MIN_CHARTER_NAME] = 2;
    }

    m_int_configs[CONFIG_MIN_PET_NAME]                        = sConfigMgr->GetIntDefault ("MinPetName",     2);
    if (m_int_configs[CONFIG_MIN_PET_NAME] < 1 || m_int_configs[CONFIG_MIN_PET_NAME] > MAX_PET_NAME)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MinPetName (%i) must be in range 1..%u. Set to 2.", m_int_configs[CONFIG_MIN_PET_NAME], MAX_PET_NAME);
        m_int_configs[CONFIG_MIN_PET_NAME] = 2;
    }

    m_int_configs[CONFIG_CHARACTER_CREATING_DISABLED] = sConfigMgr->GetIntDefault("CharacterCreating.Disabled", 0);
    m_int_configs[CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK] = sConfigMgr->GetIntDefault("CharacterCreating.Disabled.RaceMask", 0);
    m_int_configs[CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK] = sConfigMgr->GetIntDefault("CharacterCreating.Disabled.ClassMask", 0);

    m_int_configs[CONFIG_CHARACTERS_PER_REALM] = sConfigMgr->GetIntDefault("CharactersPerRealm", 50);
    if (m_int_configs[CONFIG_CHARACTERS_PER_REALM] < 1 || m_int_configs[CONFIG_CHARACTERS_PER_REALM] > 50)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "CharactersPerRealm (%i) must be in range 1..50. Set to 50.", m_int_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_int_configs[CONFIG_CHARACTERS_PER_REALM] = 50;
    }

    // must be after CONFIG_CHARACTERS_PER_REALM
    m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT] = sConfigMgr->GetIntDefault("CharactersPerAccount", 50);
    if (m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT] < m_int_configs[CONFIG_CHARACTERS_PER_REALM])
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "CharactersPerAccount (%i) can't be less than CharactersPerRealm (%i).", m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT], m_int_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT] = m_int_configs[CONFIG_CHARACTERS_PER_REALM];
    }

    m_int_configs[CONFIG_HEROIC_CHARACTERS_PER_REALM] = sConfigMgr->GetIntDefault("HeroicCharactersPerRealm", 1);
    if (int32(m_int_configs[CONFIG_HEROIC_CHARACTERS_PER_REALM]) < 0 || m_int_configs[CONFIG_HEROIC_CHARACTERS_PER_REALM] > 10)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "HeroicCharactersPerRealm (%i) must be in range 0..10. Set to 1.", m_int_configs[CONFIG_HEROIC_CHARACTERS_PER_REALM]);
        m_int_configs[CONFIG_HEROIC_CHARACTERS_PER_REALM] = 1;
    }

    m_int_configs[CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER] = sConfigMgr->GetIntDefault("CharacterCreating.MinLevelForHeroicCharacter", 0);

    m_int_configs[CONFIG_DEMON_HUNTERS_PER_REALM] = sConfigMgr->GetIntDefault("DemonHuntersPerRealm", 12);
    if (int32(m_int_configs[CONFIG_DEMON_HUNTERS_PER_REALM]) < 0 || m_int_configs[CONFIG_DEMON_HUNTERS_PER_REALM] > 12)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DemonHuntersPerRealm (%i) must be in range 0..12. Set to 1.", m_int_configs[CONFIG_DEMON_HUNTERS_PER_REALM]);
        m_int_configs[CONFIG_DEMON_HUNTERS_PER_REALM] = 1;
    }

    m_int_configs[CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_DEMON_HUNTER] = sConfigMgr->GetIntDefault("CharacterCreating.MinLevelForDemonHunter", 0);

    m_int_configs[CONFIG_SKIP_CINEMATICS] = sConfigMgr->GetIntDefault("SkipCinematics", 0);
    if (int32(m_int_configs[CONFIG_SKIP_CINEMATICS]) < 0 || m_int_configs[CONFIG_SKIP_CINEMATICS] > 2)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "SkipCinematics (%i) must be in range 0..2. Set to 0.", m_int_configs[CONFIG_SKIP_CINEMATICS]);
        m_int_configs[CONFIG_SKIP_CINEMATICS] = 0;
    }

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("MaxPlayerLevel", MAX_LEVEL);
        if (val != m_int_configs[CONFIG_MAX_PLAYER_LEVEL])
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MaxPlayerLevel option can't be changed at config reload, using current value (%u).", m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
    }
    else
        m_int_configs[CONFIG_MAX_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("MaxPlayerLevel", MAX_LEVEL);

    m_int_configs[CONFIG_MAX_SKILL_VALUE] = sConfigMgr->GetIntDefault("MaxSkillValue", DEFAULT_MAX_SKILL_VALUE);
    m_int_configs[CONFIG_TRANSFER_GOLD_LIMIT] = sConfigMgr->GetIntDefault("TransferGoldLimit", 100000000); // 10k gold for default limit transfer

    m_int_configs[CONFIG_LOG_GOLD_FROM] = sConfigMgr->GetIntDefault("LogGoldFrom", 100000000); // 10k gold for default limit transfer

    if (m_int_configs[CONFIG_MAX_PLAYER_LEVEL] > MAX_LEVEL)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MaxPlayerLevel (%i) must be in range 1..%u. Set to %u.", m_int_configs[CONFIG_MAX_PLAYER_LEVEL], MAX_LEVEL, MAX_LEVEL);
        m_int_configs[CONFIG_MAX_PLAYER_LEVEL] = MAX_LEVEL;
    }

    m_int_configs[CONFIG_MIN_DUALSPEC_LEVEL] = sConfigMgr->GetIntDefault("MinDualSpecLevel", 30);

    m_int_configs[CONFIG_START_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("StartPlayerLevel", 1);
    if (m_int_configs[CONFIG_START_PLAYER_LEVEL] < 1)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to 1.", m_int_configs[CONFIG_START_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_PLAYER_LEVEL] = 1;
    }
    else if (m_int_configs[CONFIG_START_PLAYER_LEVEL] > m_int_configs[CONFIG_MAX_PLAYER_LEVEL])
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to %u.", m_int_configs[CONFIG_START_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_PLAYER_LEVEL] = m_int_configs[CONFIG_MAX_PLAYER_LEVEL];
    }

    m_int_configs[CONFIG_START_PLAYER_MONEY] = sConfigMgr->GetIntDefault("StartPlayerMoney", 0);
    if (int32(m_int_configs[CONFIG_START_PLAYER_MONEY]) < 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "StartPlayerMoney (%i) must be in range 0.." UI64FMTD ". Set to %u.", m_int_configs[CONFIG_START_PLAYER_MONEY], MAX_MONEY_AMOUNT, 0);
        m_int_configs[CONFIG_START_PLAYER_MONEY] = 0;
    }
    else if (m_int_configs[CONFIG_START_PLAYER_MONEY] > 0x7FFFFFFF-1) // TODO: (See MAX_MONEY_AMOUNT)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "StartPlayerMoney (%i) must be in range 0..%u. Set to %u.",
            m_int_configs[CONFIG_START_PLAYER_MONEY], 0x7FFFFFFF-1, 0x7FFFFFFF-1);
        m_int_configs[CONFIG_START_PLAYER_MONEY] = 0x7FFFFFFF-1;
    }

    m_int_configs[CONFIG_CURRENCY_RESET_HOUR] = sConfigMgr->GetIntDefault("Currency.ResetHour", 3);
    if (m_int_configs[CONFIG_CURRENCY_RESET_HOUR] > 23)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Currency.ResetHour (%i) can't be load. Set to 6.", m_int_configs[CONFIG_CURRENCY_RESET_HOUR]);
        m_int_configs[CONFIG_CURRENCY_RESET_HOUR] = 3;
    }
    m_int_configs[CONFIG_CURRENCY_RESET_DAY] = sConfigMgr->GetIntDefault("Currency.ResetDay", 3);
    if (m_int_configs[CONFIG_CURRENCY_RESET_DAY] > 6)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Currency.ResetDay (%i) can't be load. Set to 3.", m_int_configs[CONFIG_CURRENCY_RESET_DAY]);
        m_int_configs[CONFIG_CURRENCY_RESET_DAY] = 3;
    }
    m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL] = sConfigMgr->GetIntDefault("Currency.ResetInterval", 7);
    if (int32(m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL]) <= 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Currency.ResetInterval (%i) must be > 0, set to default 7.", m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL]);
        m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL] = 7;
    }

    m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("RecruitAFriend.MaxLevel", 105);
    if (m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL] > m_int_configs[CONFIG_MAX_PLAYER_LEVEL])
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "RecruitAFriend.MaxLevel (%i) must be in the range 0..MaxLevel(%u). Set to %u.",
            m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL], 105);
        m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL] = 105;
    }

    m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL_DIFFERENCE] = sConfigMgr->GetIntDefault("RecruitAFriend.MaxDifference", 5);
    m_bool_configs[CONFIG_ALL_TAXI_PATHS] = sConfigMgr->GetBoolDefault("AllFlightPaths", false);
    m_bool_configs[CONFIG_INSTANT_TAXI] = sConfigMgr->GetBoolDefault("InstantFlightPaths", false);

    m_bool_configs[CONFIG_INSTANCE_IGNORE_LEVEL] = sConfigMgr->GetBoolDefault("Instance.IgnoreLevel", true);
    m_bool_configs[CONFIG_INSTANCE_IGNORE_RAID]  = sConfigMgr->GetBoolDefault("Instance.IgnoreRaid", true);

    m_bool_configs[CONFIG_CAST_UNSTUCK] = sConfigMgr->GetBoolDefault("CastUnstuck", true);
    m_int_configs[CONFIG_MAX_SPELL_CASTS_IN_CHAIN]  = sConfigMgr->GetIntDefault("MaxSpellCastsInChain", 10);
    m_int_configs[CONFIG_INSTANCE_RESET_TIME_HOUR]  = sConfigMgr->GetIntDefault("Instance.ResetTimeHour", 4);
    m_int_configs[CONFIG_INSTANCE_DAILY_RESET]  = sConfigMgr->GetIntDefault("Instance.DailyReset", 1);
    m_int_configs[CONFIG_INSTANCE_WEEKLY_RESET]  = sConfigMgr->GetIntDefault("Instance.WeeklyReset", 7);
    m_int_configs[CONFIG_INSTANCE_UNLOAD_DELAY] = sConfigMgr->GetIntDefault("Instance.UnloadDelay", 30 * MINUTE * IN_MILLISECONDS);

    m_int_configs[CONFIG_CHALLENGE_KEY_RESET]  = sConfigMgr->GetIntDefault("Challenge.Key.Reset", 7);
    m_bool_configs[CONFIG_CHALLENGE_ENABLED] = sConfigMgr->GetBoolDefault("Challenge.Enabled", true);

    m_int_configs[CONFIG_MAX_PRIMARY_TRADE_SKILL] = sConfigMgr->GetIntDefault("MaxPrimaryTradeSkill", 2);
    m_int_configs[CONFIG_MIN_PETITION_SIGNS] = sConfigMgr->GetIntDefault("MinPetitionSigns", 0);
    if (m_int_configs[CONFIG_MIN_PETITION_SIGNS] > 9)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MinPetitionSigns (%i) must be in range 0..9. Set to 9.", m_int_configs[CONFIG_MIN_PETITION_SIGNS]);
        m_int_configs[CONFIG_MIN_PETITION_SIGNS] = 9;
    }

    m_int_configs[CONFIG_GM_LOGIN_STATE]        = sConfigMgr->GetIntDefault("GM.LoginState", 0);
    m_int_configs[CONFIG_GM_VISIBLE_STATE]      = sConfigMgr->GetIntDefault("GM.Visible", 1);
    m_int_configs[CONFIG_GM_CHAT]               = sConfigMgr->GetIntDefault("GM.Chat", 0);
    m_int_configs[CONFIG_GM_WHISPERING_TO]      = sConfigMgr->GetIntDefault("GM.WhisperingTo", 1);

    m_int_configs[CONFIG_GM_LEVEL_IN_GM_LIST]   = sConfigMgr->GetIntDefault("GM.InGMList.Level", SEC_ADMINISTRATOR);
    m_int_configs[CONFIG_GM_LEVEL_IN_WHO_LIST]  = sConfigMgr->GetIntDefault("GM.InWhoList.Level", SEC_ADMINISTRATOR);
    m_bool_configs[CONFIG_GM_LOG_TRADE]         = sConfigMgr->GetBoolDefault("GM.LogTrade", false);
    m_int_configs[CONFIG_START_GM_LEVEL]        = sConfigMgr->GetIntDefault("GM.StartLevel", 1);
    if (m_int_configs[CONFIG_START_GM_LEVEL] < m_int_configs[CONFIG_START_PLAYER_LEVEL])
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "GM.StartLevel (%i) must be in range StartPlayerLevel(%u)..%u. Set to %u.",
            m_int_configs[CONFIG_START_GM_LEVEL], m_int_configs[CONFIG_START_PLAYER_LEVEL], MAX_LEVEL, m_int_configs[CONFIG_START_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_GM_LEVEL] = m_int_configs[CONFIG_START_PLAYER_LEVEL];
    }
    else if (m_int_configs[CONFIG_START_GM_LEVEL] > MAX_LEVEL)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "GM.StartLevel (%i) must be in range 1..%u. Set to %u.", m_int_configs[CONFIG_START_GM_LEVEL], MAX_LEVEL, MAX_LEVEL);
        m_int_configs[CONFIG_START_GM_LEVEL] = MAX_LEVEL;
    }
    m_bool_configs[CONFIG_ALLOW_GM_GROUP]       = sConfigMgr->GetBoolDefault("GM.AllowInvite", true);
    m_bool_configs[CONFIG_ALLOW_GM_FRIEND]      = sConfigMgr->GetBoolDefault("GM.AllowFriend", true);
    m_bool_configs[CONFIG_GM_LOWER_SECURITY] = sConfigMgr->GetBoolDefault("GM.LowerSecurity", false);
    m_float_configs[CONFIG_CHANCE_OF_GM_SURVEY] = sConfigMgr->GetFloatDefault("GM.TicketSystem.ChanceOfGMSurvey", 50.0f);

    m_int_configs[CONFIG_GROUP_VISIBILITY] = sConfigMgr->GetIntDefault("Visibility.GroupMode", 1);

    m_int_configs[CONFIG_MAIL_DELIVERY_DELAY] = sConfigMgr->GetIntDefault("MailDeliveryDelay", HOUR);

    m_int_configs[CONFIG_UPTIME_UPDATE] = sConfigMgr->GetIntDefault("UpdateUptimeInterval", 10);
    if (int32(m_int_configs[CONFIG_UPTIME_UPDATE]) <= 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "UpdateUptimeInterval (%i) must be > 0, set to default 10.", m_int_configs[CONFIG_UPTIME_UPDATE]);
        m_int_configs[CONFIG_UPTIME_UPDATE] = 10;
    }
    if (reload)
    {
        m_timers[WUPDATE_UPTIME].SetInterval(m_int_configs[CONFIG_UPTIME_UPDATE]*MINUTE*IN_MILLISECONDS);
        m_timers[WUPDATE_UPTIME].Reset();
    }

    // log db cleanup interval
    m_int_configs[CONFIG_LOGDB_CLEARINTERVAL] = sConfigMgr->GetIntDefault("LogDB.Opt.ClearInterval", 10);
    if (int32(m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]) <= 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LogDB.Opt.ClearInterval (%i) must be > 0, set to default 10.", m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]);
        m_int_configs[CONFIG_LOGDB_CLEARINTERVAL] = 10;
    }
    if (reload)
    {
        m_timers[WUPDATE_CLEANDB].SetInterval(m_int_configs[CONFIG_LOGDB_CLEARINTERVAL] * MINUTE * IN_MILLISECONDS);
        m_timers[WUPDATE_CLEANDB].Reset();
    }
    m_int_configs[CONFIG_LOGDB_CLEARTIME] = sConfigMgr->GetIntDefault("LogDB.Opt.ClearTime", 1209600); // 14 days default
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Will clear `logs` table of entries older than %i seconds every %u minutes.",
        m_int_configs[CONFIG_LOGDB_CLEARTIME], m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]);

    m_int_configs[CONFIG_SKILL_CHANCE_ORANGE] = sConfigMgr->GetIntDefault("SkillChance.Orange", 100);
    m_int_configs[CONFIG_SKILL_CHANCE_YELLOW] = sConfigMgr->GetIntDefault("SkillChance.Yellow", 75);
    m_int_configs[CONFIG_SKILL_CHANCE_GREEN]  = sConfigMgr->GetIntDefault("SkillChance.Green", 25);
    m_int_configs[CONFIG_SKILL_CHANCE_GREY]   = sConfigMgr->GetIntDefault("SkillChance.Grey", 0);

    m_int_configs[CONFIG_SKILL_CHANCE_MINING_STEPS]  = sConfigMgr->GetIntDefault("SkillChance.MiningSteps", 75);
    m_int_configs[CONFIG_SKILL_CHANCE_SKINNING_STEPS]   = sConfigMgr->GetIntDefault("SkillChance.SkinningSteps", 75);

    m_bool_configs[CONFIG_SKILL_PROSPECTING] = sConfigMgr->GetBoolDefault("SkillChance.Prospecting", false);
    m_bool_configs[CONFIG_SKILL_MILLING] = sConfigMgr->GetBoolDefault("SkillChance.Milling", false);

    m_int_configs[CONFIG_SKILL_GAIN_CRAFTING]  = sConfigMgr->GetIntDefault("SkillGain.Crafting", 1);

    m_int_configs[CONFIG_SKILL_GAIN_GATHERING]  = sConfigMgr->GetIntDefault("SkillGain.Gathering", 1);

    m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] = sConfigMgr->GetIntDefault("MaxOverspeedPings", 2);
    if (m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] != 0 && m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] < 2)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "MaxOverspeedPings (%i) must be in range 2..infinity (or 0 to disable check). Set to 2.", m_int_configs[CONFIG_MAX_OVERSPEED_PINGS]);
        m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] = 2;
    }

    m_bool_configs[CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY] = sConfigMgr->GetBoolDefault("SaveRespawnTimeImmediately", true);
    m_bool_configs[CONFIG_WEATHER] = sConfigMgr->GetBoolDefault("ActivateWeather", true);

    m_int_configs[CONFIG_DISABLE_BREATHING] = sConfigMgr->GetIntDefault("DisableWaterBreath", SEC_CONSOLE);

    m_bool_configs[CONFIG_ANTI_FLOOD_LFG] = sConfigMgr->GetBoolDefault("Anti.Flood.on.LFG", false);
    m_bool_configs[CONFIG_ANTI_FLOOD_PM] = sConfigMgr->GetBoolDefault("Anti.Flood.on.PM", false);
    m_int_configs[CONFIG_ANTI_FLOOD_COUNT_OF_MESSAGES] = sConfigMgr->GetIntDefault("Anti.Flood.MessageCount", 4);
    m_bool_configs[CONFIG_ANTI_FLOOD_HWID_BANS_ALLOW] = sConfigMgr->GetBoolDefault("Anti.Flood.Hwid.Bans.Allow", false);
    m_bool_configs[CONFIG_ANTI_FLOOD_HWID_MUTE_ALLOW] = sConfigMgr->GetBoolDefault("Anti.Flood.Hwid.Mute.Allow", false);
    m_bool_configs[CONFIG_ANTI_FLOOD_HWID_KICK_ALLOW] = sConfigMgr->GetBoolDefault("Anti.Flood.Hwid.Kick.Allow", false);
    m_int_configs[CONFIG_ANTI_FLOOD_HWID_BANS_COUNT] = sConfigMgr->GetIntDefault("Anti.Flood.Hwid.Bans.Count.Required", 0);
    m_int_configs[CONFIG_COMPLAINTS_REQUIRED] = sConfigMgr->GetIntDefault("Complaints.Required", 0);
    m_int_configs[CONFIG_COMPLAINTS_PENALTY1] = sConfigMgr->GetIntDefault("Complaints.Penalty1", 10); // < 0 => ban, > 0 mute
    m_int_configs[CONFIG_COMPLAINTS_PENALTY2] = sConfigMgr->GetIntDefault("Complaints.Penalty2", 60); // < 0 => ban, > 0 mute
    m_int_configs[CONFIG_COMPLAINTS_PENALTY3] = sConfigMgr->GetIntDefault("Complaints.Penalty3", 1440); // < 0 => ban, > 0 mute
    
    m_int_configs[CONFIG_CHATFLOOD_MESSAGE_COUNT] = sConfigMgr->GetIntDefault("ChatFlood.MessageCount", 0);
    m_int_configs[CONFIG_CHATFLOOD_MESSAGE_DELAY] = sConfigMgr->GetIntDefault("ChatFlood.MessageDelay", 1);
    m_int_configs[CONFIG_CHATFLOOD_MUTE_TIME]     = sConfigMgr->GetIntDefault("ChatFlood.MuteTime", 10);

    m_int_configs[CONFIG_WORD_FILTER_MUTE_DURATION] = sConfigMgr->GetIntDefault("WordFilter.MuteDuration", 0);
    m_bool_configs[CONFIG_WORD_FILTER_ENABLE]       = sConfigMgr->GetBoolDefault("WordFilter.Enable", false);
    m_bool_configs[CONFIG_SHARE_ENABLE]             = sConfigMgr->GetBoolDefault("Share.Enable", false);
    m_bool_configs[CONFIG_IPSET_ENABLE]             = sConfigMgr->GetBoolDefault("Ipset.Enable", false);

    m_int_configs[CONFIG_EVENT_ANNOUNCE] = sConfigMgr->GetIntDefault("Event.Announce", 0);

    m_float_configs[CONFIG_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS] = sConfigMgr->GetFloatDefault("CreatureFamilyFleeAssistanceRadius", 30.0f);
    m_float_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS] = sConfigMgr->GetFloatDefault("CreatureFamilyAssistanceRadius", 10.0f);
    m_int_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY]  = sConfigMgr->GetIntDefault("CreatureFamilyAssistanceDelay", 1500);
    m_int_configs[CONFIG_CREATURE_FAMILY_FLEE_DELAY]        = sConfigMgr->GetIntDefault("CreatureFamilyFleeDelay", 7000);

    m_int_configs[CONFIG_WORLD_BOSS_LEVEL_DIFF] = sConfigMgr->GetIntDefault("WorldBossLevelDiff", 3);

    // note: disable value (-1) will assigned as 0xFFFFFFF, to prevent overflow at calculations limit it to max possible player level MAX_LEVEL(100)
    m_int_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] = sConfigMgr->GetIntDefault("Quests.LowLevelHideDiff", 4);
    if (m_int_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_int_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] = MAX_LEVEL;
    m_int_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] = sConfigMgr->GetIntDefault("Quests.HighLevelHideDiff", 7);
    if (m_int_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_int_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] = MAX_LEVEL;
    m_bool_configs[CONFIG_QUEST_IGNORE_RAID] = sConfigMgr->GetBoolDefault("Quests.IgnoreRaid", true);
    m_bool_configs[CONFIG_QUEST_IGNORE_AUTO_ACCEPT] = sConfigMgr->GetBoolDefault("Quests.IgnoreAutoAccept", true);
    m_bool_configs[CONFIG_QUEST_IGNORE_AUTO_COMPLETE] = sConfigMgr->GetBoolDefault("Quests.IgnoreAutoComplete", true);

    m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR] = sConfigMgr->GetIntDefault("Battleground.Random.ResetHour", 6);
    if (m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR] > 23)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Battleground.Random.ResetHour (%i) can't be load. Set to 6.", m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR]);
        m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR] = 6;
    }

    m_bool_configs[CONFIG_DETECT_POS_COLLISION] = sConfigMgr->GetBoolDefault("DetectPosCollision", true);

    m_bool_configs[CONFIG_RESTRICTED_LFG_CHANNEL]      = sConfigMgr->GetBoolDefault("Channel.RestrictedLfg", false);
    m_bool_configs[CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL] = sConfigMgr->GetBoolDefault("Channel.SilentlyGMJoin", false);

    m_bool_configs[CONFIG_TALENTS_INSPECTING]           = sConfigMgr->GetBoolDefault("TalentsInspecting", true);
    m_bool_configs[CONFIG_CHAT_FAKE_MESSAGE_PREVENTING] = sConfigMgr->GetBoolDefault("ChatFakeMessagePreventing", false);
    m_int_configs[CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY] = sConfigMgr->GetIntDefault("ChatStrictLinkChecking.Severity", 0);
    m_int_configs[CONFIG_CHAT_STRICT_LINK_CHECKING_KICK] = sConfigMgr->GetIntDefault("ChatStrictLinkChecking.Kick", 0);

    m_int_configs[CONFIG_CORPSE_DECAY_NORMAL]    = sConfigMgr->GetIntDefault("Corpse.Decay.NORMAL", 60);
    m_int_configs[CONFIG_CORPSE_DECAY_RARE]      = sConfigMgr->GetIntDefault("Corpse.Decay.RARE", 120);
    m_int_configs[CONFIG_CORPSE_DECAY_ELITE]     = sConfigMgr->GetIntDefault("Corpse.Decay.ELITE", 120);
    m_int_configs[CONFIG_CORPSE_DECAY_RAREELITE] = sConfigMgr->GetIntDefault("Corpse.Decay.RAREELITE", 120);
    m_int_configs[CONFIG_CORPSE_DECAY_WORLDBOSS] = sConfigMgr->GetIntDefault("Corpse.Decay.WORLDBOSS", 120);

    m_int_configs[CONFIG_DEATH_SICKNESS_LEVEL]           = sConfigMgr->GetIntDefault ("Death.SicknessLevel", 11);
    m_bool_configs[CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP] = sConfigMgr->GetBoolDefault("Death.CorpseReclaimDelay.PvP", false);
    m_bool_configs[CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE] = sConfigMgr->GetBoolDefault("Death.CorpseReclaimDelay.PvE", false);
    m_bool_configs[CONFIG_DEATH_BONES_WORLD]              = sConfigMgr->GetBoolDefault("Death.Bones.World", true);
    m_bool_configs[CONFIG_DEATH_BONES_BG_OR_ARENA]        = sConfigMgr->GetBoolDefault("Death.Bones.BattlegroundOrArena", true);

    m_float_configs[CONFIG_THREAT_RADIUS] = sConfigMgr->GetFloatDefault("ThreatRadius", 60.0f);

    // always use declined names in the russian client
    m_bool_configs[CONFIG_DECLINED_NAMES_USED] = sConfigMgr->GetBoolDefault("DeclinedNames", false);

    m_float_configs[CONFIG_LISTEN_RANGE_SAY]       = sConfigMgr->GetFloatDefault("ListenRange.Say", 25.0f);
    m_float_configs[CONFIG_LISTEN_RANGE_TEXTEMOTE] = sConfigMgr->GetFloatDefault("ListenRange.TextEmote", 25.0f);
    m_float_configs[CONFIG_LISTEN_RANGE_YELL]      = sConfigMgr->GetFloatDefault("ListenRange.Yell", 300.0f);

    m_bool_configs[CONFIG_BATTLEGROUND_CAST_DESERTER]                = sConfigMgr->GetBoolDefault("Battleground.CastDeserter", true);
    m_bool_configs[CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE]       = sConfigMgr->GetBoolDefault("Battleground.QueueAnnouncer.Enable", false);
    m_bool_configs[CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY]   = sConfigMgr->GetBoolDefault("Battleground.QueueAnnouncer.PlayerOnly", false);
    m_int_configs[CONFIG_BATTLEGROUND_INVITATION_TYPE]               = sConfigMgr->GetIntDefault ("Battleground.InvitationType", 0);
    m_int_configs[CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER]        = sConfigMgr->GetIntDefault ("Battleground.PrematureFinishTimer", 5 * MINUTE * IN_MILLISECONDS);
    m_int_configs[CONFIG_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH]  = sConfigMgr->GetIntDefault ("Battleground.PremadeGroupWaitForMatch", 30 * MINUTE * IN_MILLISECONDS);
    m_bool_configs[CONFIG_BG_XP_FOR_KILL]                            = sConfigMgr->GetBoolDefault("Battleground.GiveXPForKills", false);
    m_int_configs[CONFIG_ARENA_MAX_RATING_DIFFERENCE]                = sConfigMgr->GetIntDefault ("Arena.MaxRatingDifference", 150);
    m_int_configs[CONFIG_ARENA_RATING_DISCARD_TIMER]                 = sConfigMgr->GetIntDefault ("Arena.RatingDiscardTimer", 600);
    m_int_configs[CONFIG_ARENA_RATED_UPDATE_TIMER]                   = sConfigMgr->GetIntDefault ("Arena.RatedUpdateTimer", 5 * IN_MILLISECONDS);
    m_bool_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS]              = sConfigMgr->GetBoolDefault("Arena.AutoDistributePoints", false);
    m_int_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS]        = sConfigMgr->GetIntDefault ("Arena.AutoDistributeInterval", 7);
    m_bool_configs[CONFIG_ARENA_QUEUE_ANNOUNCER_ENABLE]              = sConfigMgr->GetBoolDefault("Arena.QueueAnnouncer.Enable", false);
    m_bool_configs[CONFIG_ARENA_QUEUE_ANNOUNCER_PLAYERONLY]          = sConfigMgr->GetBoolDefault("Arena.QueueAnnouncer.PlayerOnly", false);
    m_int_configs[CONFIG_ARENA_SEASON_ID]                            = sConfigMgr->GetIntDefault ("Arena.ArenaSeason.ID", 25);
    m_int_configs[CONFIG_ARENA_START_RATING]                         = sConfigMgr->GetIntDefault ("Arena.ArenaStartRating", 0);
    m_int_configs[CONFIG_ARENA_START_PERSONAL_RATING]                = sConfigMgr->GetIntDefault ("Arena.ArenaStartPersonalRating", 1000);
    m_int_configs[CONFIG_ARENA_START_MATCHMAKER_RATING]              = sConfigMgr->GetIntDefault ("Arena.ArenaStartMatchmakerRating", 1500);
    m_bool_configs[CONFIG_ARENA_SEASON_IN_PROGRESS]                  = sConfigMgr->GetBoolDefault("Arena.ArenaSeason.InProgress", true);
    m_bool_configs[CONFIG_ARENA_LOG_EXTENDED_INFO]                   = sConfigMgr->GetBoolDefault("ArenaLog.ExtendedInfo", false);
    m_bool_configs[CONFIG_ARENA_DEATHMATCH]                          = sConfigMgr->GetBoolDefault("DeathMatch", false);

    m_bool_configs[CONFIG_OFFHAND_CHECK_AT_SPELL_UNLEARN]            = sConfigMgr->GetBoolDefault("OffhandCheckAtSpellUnlearn", true);

    if (int32 clientCacheId = sConfigMgr->GetIntDefault("ClientCacheVersion", 0))
    {
        // overwrite DB/old value
        if (clientCacheId > 0)
        {
            m_int_configs[CONFIG_CLIENTCACHE_VERSION] = clientCacheId;
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Client cache version set to: %u", clientCacheId);
        }
        else
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "ClientCacheVersion can't be negative %d, ignored.", clientCacheId);
    }

    if (int32 hotfixCacheId = sConfigMgr->GetIntDefault("HotfixCacheVersion", 0))
    {
        // overwrite DB/old value
        if (hotfixCacheId > 0)
        {
            m_int_configs[CONFIG_HOTFIX_CACHE_VERSION] = hotfixCacheId;
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Hotfix cache version set to: %u", hotfixCacheId);
        }
        else
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "HotfixCacheVersion can't be negative %d, ignored.", hotfixCacheId);
    }

    m_int_configs[CONFIG_INSTANT_LOGOUT] = sConfigMgr->GetIntDefault("InstantLogout", SEC_MODERATOR);

    m_int_configs[CONFIG_GUILD_EVENT_LOG_COUNT] = sConfigMgr->GetIntDefault("Guild.EventLogRecordsCount", GUILD_EVENTLOG_MAX_RECORDS);
    if (m_int_configs[CONFIG_GUILD_EVENT_LOG_COUNT] > GUILD_EVENTLOG_MAX_RECORDS)
        m_int_configs[CONFIG_GUILD_EVENT_LOG_COUNT] = GUILD_EVENTLOG_MAX_RECORDS;
    m_int_configs[CONFIG_GUILD_BANK_EVENT_LOG_COUNT] = sConfigMgr->GetIntDefault("Guild.BankEventLogRecordsCount", GUILD_BANKLOG_MAX_RECORDS);
    if (m_int_configs[CONFIG_GUILD_BANK_EVENT_LOG_COUNT] > GUILD_BANKLOG_MAX_RECORDS)
        m_int_configs[CONFIG_GUILD_BANK_EVENT_LOG_COUNT] = GUILD_BANKLOG_MAX_RECORDS;
    m_int_configs[CONFIG_GUILD_NEWS_LOG_COUNT] = sConfigMgr->GetIntDefault("Guild.NewsLogRecordsCount", GUILD_NEWSLOG_MAX_RECORDS);
    if (m_int_configs[CONFIG_GUILD_NEWS_LOG_COUNT] > GUILD_NEWSLOG_MAX_RECORDS)
        m_int_configs[CONFIG_GUILD_NEWS_LOG_COUNT] = GUILD_NEWSLOG_MAX_RECORDS;

    //visibility on continents
    m_MaxVisibleDistanceOnContinents = sConfigMgr->GetFloatDefault("Visibility.Distance.Continents", DEFAULT_VISIBILITY_DISTANCE);
    if (m_MaxVisibleDistanceOnContinents < 45*sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.Continents can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceOnContinents = 45*sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceOnContinents > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.Continents can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceOnContinents = MAX_VISIBILITY_DISTANCE;
    }

    m_visibilityRelocationLowerLimit = sConfigMgr->GetFloatDefault("Visibility.RelocationLowerLimit", 10.f);
    m_visibilityRelocationLowerLimitC = sConfigMgr->GetFloatDefault("Visibility.RelocationLowerLimitC", 10.f);
    m_visibilityAINotifyDelay = sConfigMgr->GetIntDefault("Visibility.AINotifyDelay", DEFAULT_VISIBILITY_NOTIFY_PERIOD);
    Relocation_UpdateUnderwateLimit = sConfigMgr->GetFloatDefault("Relocation.UpdateUnderwateLimit", 20.f);
    ZoneUpdateDistanceRangeLimit = sConfigMgr->GetFloatDefault("Zone.UpdateDistanceRage", 5.f);

    //visibility in instances
    m_MaxVisibleDistanceInInstances = sConfigMgr->GetFloatDefault("Visibility.Distance.Instances", DEFAULT_VISIBILITY_INSTANCE);
    if (m_MaxVisibleDistanceInInstances < 45*sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.Instances can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInInstances = 45*sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInInstances > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.Instances can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInInstances = MAX_VISIBILITY_DISTANCE;
    }

    //visibility in BG
    m_MaxVisibleDistanceInBG = sConfigMgr->GetFloatDefault("Visibility.Distance.BG", DEFAULT_VISIBILITY_BGARENAS);
    if (m_MaxVisibleDistanceInBG < 45*sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.BG can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInBG = 45*sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInBG > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.BG can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInBG = MAX_VISIBILITY_DISTANCE;
    }

    //visibility in Arenas
    m_MaxVisibleDistanceInArenas = sConfigMgr->GetFloatDefault("Visibility.Distance.Arenas", DEFAULT_VISIBILITY_BGARENAS);
    if (m_MaxVisibleDistanceInArenas < 45*sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.Arenas can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInArenas = 45*sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInArenas > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Visibility.Distance.Arenas can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInArenas = MAX_VISIBILITY_DISTANCE;
    }

    m_visibility_notify_periodOnContinents = sConfigMgr->GetIntDefault("Visibility.Notify.Period.OnContinents", DEFAULT_VISIBILITY_NOTIFY_PERIOD);
    m_visibility_notify_periodInInstances = sConfigMgr->GetIntDefault("Visibility.Notify.Period.InInstances",   DEFAULT_VISIBILITY_NOTIFY_PERIOD);
    m_visibility_notify_periodInBGArenas = sConfigMgr->GetIntDefault("Visibility.Notify.Period.InBGArenas",    DEFAULT_VISIBILITY_NOTIFY_PERIOD);

    ///- Load the CharDelete related config options
    m_int_configs[CONFIG_CHARDELETE_METHOD] = sConfigMgr->GetIntDefault("CharDelete.Method", 0);
    m_int_configs[CONFIG_CHARDELETE_MIN_LEVEL] = sConfigMgr->GetIntDefault("CharDelete.MinLevel", 0);
    m_int_configs[CONFIG_CHARDELETE_KEEP_DAYS] = sConfigMgr->GetIntDefault("CharDelete.KeepDays", 30);

    ///- Read the "Data" directory from the config file
    std::string dataPath = sConfigMgr->GetStringDefault("DataDir", "./");
    if (dataPath.at(dataPath.length()-1) != '/' && dataPath.at(dataPath.length()-1) != '\\')
        dataPath.push_back('/');

    if (reload)
    {
        if (dataPath != m_dataPath)
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "DataDir option can't be changed at worldserver.conf reload, using current value (%s).", m_dataPath.c_str());
    }
    else
    {
        m_dataPath += dataPath;
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Using DataDir %s", m_dataPath.c_str());
    }

    // MMap related
    m_bool_configs[CONFIG_ENABLE_MMAPS] = sConfigMgr->GetBoolDefault("mmap.enablePathFinding", true);
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "WORLD: MMap data directory is: %smmaps", m_dataPath.c_str());

    m_bool_configs[CONFIG_VMAP_INDOOR_CHECK] = sConfigMgr->GetBoolDefault("vmap.enableIndoorCheck", false);
    bool enableIndoor = sConfigMgr->GetBoolDefault("vmap.enableIndoorCheck", true);
    bool enableLOS = sConfigMgr->GetBoolDefault("vmap.enableLOS", true);
    bool enableHeight = sConfigMgr->GetBoolDefault("vmap.enableHeight", true);
    bool enablePetLOS = sConfigMgr->GetBoolDefault("vmap.petLOS", true);

    if (!enableHeight)
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "VMap height checking disabled! Creatures movements and other various things WILL be broken! Expect no support.");

    VMAP::VMapFactory::createOrGetVMapManager()->setEnableLineOfSightCalc(enableLOS);
    VMAP::VMapFactory::createOrGetVMapManager()->setEnableHeightCalc(enableHeight);
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "VMap support included. LineOfSight:%i, getHeight:%i, indoorCheck:%i PetLOS:%i", enableLOS, enableHeight, enableIndoor, enablePetLOS);
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "VMap data directory is: %svmaps", m_dataPath.c_str());

    m_int_configs[CONFIG_MAX_WHO] = sConfigMgr->GetIntDefault("MaxWhoListReturns", 49);
    m_bool_configs[CONFIG_LIMIT_WHO_ONLINE] = sConfigMgr->GetBoolDefault("LimitWhoOnline", true);
    m_bool_configs[CONFIG_PET_LOS] = sConfigMgr->GetBoolDefault("vmap.petLOS", true);

    m_bool_configs[CONFIG_START_ALL_EXPLORED] = sConfigMgr->GetBoolDefault("PlayerStart.MapsExplored", false);
    m_bool_configs[CONFIG_START_ALL_REP] = sConfigMgr->GetBoolDefault("PlayerStart.AllReputation", false);

    m_bool_configs[CONFIG_PVP_TOKEN_ENABLE] = sConfigMgr->GetBoolDefault("PvPToken.Enable", false);
    m_int_configs[CONFIG_PVP_TOKEN_MAP_TYPE] = sConfigMgr->GetIntDefault("PvPToken.MapAllowType", 4);
    m_int_configs[CONFIG_PVP_TOKEN_ID] = sConfigMgr->GetIntDefault("PvPToken.ItemID", 29434);
    m_int_configs[CONFIG_PVP_TOKEN_COUNT] = sConfigMgr->GetIntDefault("PvPToken.ItemCount", 1);
    if (m_int_configs[CONFIG_PVP_TOKEN_COUNT] < 1)
        m_int_configs[CONFIG_PVP_TOKEN_COUNT] = 1;

    m_bool_configs[CONFIG_NO_RESET_TALENT_COST] = sConfigMgr->GetBoolDefault("NoResetTalentsCost", false);
    m_bool_configs[CONFIG_SHOW_KICK_IN_WORLD] = sConfigMgr->GetBoolDefault("ShowKickInWorld", false);
    m_int_configs[CONFIG_INTERVAL_LOG_UPDATE] = sConfigMgr->GetIntDefault("RecordUpdateTimeDiffInterval", 60000);
    m_int_configs[CONFIG_MIN_LOG_UPDATE] = sConfigMgr->GetIntDefault("MinRecordUpdateTimeDiff", 100);
    m_int_configs[CONFIG_NUMTHREADS] = sConfigMgr->GetIntDefault("MapUpdate.Threads", 1);
    m_int_configs[CONFIG_MAP_NUMTHREADS] = sConfigMgr->GetIntDefault("MapUpdate.Map.Threads", 1);
    m_int_configs[CONFIG_MAX_RESULTS_LOOKUP_COMMANDS] = sConfigMgr->GetIntDefault("Command.LookupMaxResults", 0);

    // chat logging
    m_bool_configs[CONFIG_CHATLOG_CHANNEL] = sConfigMgr->GetBoolDefault("ChatLogs.Channel", false);
    m_bool_configs[CONFIG_CHATLOG_WHISPER] = sConfigMgr->GetBoolDefault("ChatLogs.Whisper", false);
    m_bool_configs[CONFIG_CHATLOG_SYSCHAN] = sConfigMgr->GetBoolDefault("ChatLogs.SysChan", false);
    m_bool_configs[CONFIG_CHATLOG_PARTY] = sConfigMgr->GetBoolDefault("ChatLogs.Party", false);
    m_bool_configs[CONFIG_CHATLOG_RAID] = sConfigMgr->GetBoolDefault("ChatLogs.Raid", false);
    m_bool_configs[CONFIG_CHATLOG_GUILD] = sConfigMgr->GetBoolDefault("ChatLogs.Guild", false);
    m_bool_configs[CONFIG_CHATLOG_PUBLIC] = sConfigMgr->GetBoolDefault("ChatLogs.Public", false);
    m_bool_configs[CONFIG_CHATLOG_ADDON] = sConfigMgr->GetBoolDefault("ChatLogs.Addon", false);
    m_bool_configs[CONFIG_CHATLOG_BGROUND] = sConfigMgr->GetBoolDefault("ChatLogs.Battleground", false);

    // New anticheat
    m_int_configs[CONFIG_NEW_ANTICHEAT_MODE] = sConfigMgr->GetIntDefault("ServerAnticheat.Mode", 0);

    // Warden
    m_bool_configs[CONFIG_WARDEN_ENABLED]              = sConfigMgr->GetBoolDefault("Warden.Enabled", false);
    m_int_configs[CONFIG_WARDEN_CLIENT_CHECK_HOLDOFF]  = sConfigMgr->GetIntDefault("Warden.ClientCheckHoldOff", 25000);
    m_int_configs[CONFIG_WARDEN_CLIENT_RESPONSE_DELAY] = sConfigMgr->GetIntDefault("Warden.ClientResponseDelay", 60000);
    m_int_configs[CONFIG_WARDEN_BASIC_SECURITY_LEVEL_REQ]  = sConfigMgr->GetIntDefault("Warden.BasicSecurityLevelReq", 40);
    m_int_configs[CONFIG_WARDEN_MAX_SECURITY_LEVEL_REQ] = sConfigMgr->GetIntDefault("Warden.MaxSecurityLevelReq", 100);

    // Dungeon finder
    m_int_configs[CONFIG_LFG_OPTIONSMASK] = sConfigMgr->GetIntDefault("DungeonFinder.OptionsMask", 1);
    m_bool_configs[CONFIG_LFG_DEBUG_JOIN] = sConfigMgr->GetBoolDefault("DungeonFinder.DebugJoin", false);
    m_bool_configs[CONFIG_LFG_FORCE_MINPLAYERS] = sConfigMgr->GetBoolDefault("DungeonFinder.ForceMinplayers", false);
    m_int_configs[CONFIG_LFG_SHORTAGE_CHECK_INTERVAL] = sConfigMgr->GetIntDefault("DungeonFinder.ShortageCheckInterval", 5);
    m_int_configs[CONFIG_LFG_SHORTAGE_PERCENT] = sConfigMgr->GetIntDefault("DungeonFinder.ShortagePercent", 50);
    m_int_configs[CONFIG_LFG_MAX_QUEUES] = sConfigMgr->GetIntDefault("DungeonFinder.MaxQueues", 7);

    // DBC_ItemAttributes
    m_bool_configs[CONFIG_DBC_ENFORCE_ITEM_ATTRIBUTES] = sConfigMgr->GetBoolDefault("DBC.EnforceItemAttributes", true);

    // Max instances per hour
    m_int_configs[CONFIG_MAX_INSTANCES_PER_HOUR] = sConfigMgr->GetIntDefault("AccountInstancesPerHour", 255);

    // AutoBroadcast
    m_bool_configs[CONFIG_AUTOBROADCAST] = sConfigMgr->GetBoolDefault("AutoBroadcast.On", false);
    m_int_configs[CONFIG_AUTOBROADCAST_CENTER] = sConfigMgr->GetIntDefault("AutoBroadcast.Center", 0);
    m_int_configs[CONFIG_AUTOBROADCAST_INTERVAL] = sConfigMgr->GetIntDefault("AutoBroadcast.Timer", 60000);

    if (reload)
    {
        m_timers[WUPDATE_AUTOBROADCAST].SetInterval(m_int_configs[CONFIG_AUTOBROADCAST_INTERVAL]);
        m_timers[WUPDATE_AUTOBROADCAST].Reset();
    }

    // MySQL ping time interval
    m_int_configs[CONFIG_DB_PING_INTERVAL] = sConfigMgr->GetIntDefault("MaxPingTime", 30);

    // Guild save interval
    m_bool_configs[CONFIG_GUILD_LEVELING_ENABLED] = sConfigMgr->GetBoolDefault("Guild.LevelingEnabled", true);
    m_int_configs[CONFIG_GUILD_SAVE_INTERVAL] = sConfigMgr->GetIntDefault("Guild.SaveInterval", 15);
    m_int_configs[CONFIG_GUILD_MAX_LEVEL] = sConfigMgr->GetIntDefault("Guild.MaxLevel", 25);
    m_int_configs[CONFIG_GUILD_UNDELETABLE_LEVEL] = sConfigMgr->GetIntDefault("Guild.UndeletableLevel", 4);
    rate_values[RATE_XP_GUILD_MODIFIER] = sConfigMgr->GetFloatDefault("Guild.XPModifier", 0.25f);
    m_int_configs[CONFIG_GUILD_DAILY_XP_CAP] = sConfigMgr->GetIntDefault("Guild.DailyXPCap", 7807500);
    m_int_configs[CONFIG_GUILD_WEEKLY_REP_CAP] = sConfigMgr->GetIntDefault("Guild.WeeklyReputationCap", 4375);

    // misc
    m_bool_configs[CONFIG_PDUMP_NO_PATHS] = sConfigMgr->GetBoolDefault("PlayerDump.DisallowPaths", true);
    m_bool_configs[CONFIG_PDUMP_NO_OVERWRITE] = sConfigMgr->GetBoolDefault("PlayerDump.DisallowOverwrite", true);

    //Auto restart system
    m_int_configs[CONFIG_AUTO_SERVER_RESTART_HOUR] = sConfigMgr->GetIntDefault("Server.Auto.RestartHour", 5);
    if (m_int_configs[CONFIG_AUTO_SERVER_RESTART_HOUR] > 23)
    {
        m_int_configs[CONFIG_AUTO_SERVER_RESTART_HOUR] = 5;
    }
    m_bool_configs[CONFIG_DISABLE_RESTART] = sConfigMgr->GetBoolDefault("DisableRestart", true);
    m_bool_configs[CONFIG_DISABLE_NEW_ONLINE] = sConfigMgr->GetBoolDefault("DisableUpdateOnlineTable", false);
    m_bool_configs[CONFIG_DISABLE_DONATELOADING] = sConfigMgr->GetBoolDefault("DisableDonateLoading", false);
    
    m_bool_configs[CONFIG_BLACKMARKET_ENABLED] = sConfigMgr->GetBoolDefault("BlackMarket.Enabled", true);
    m_bool_configs[CONFIG_FEATURE_SYSTEM_BPAY_STORE_ENABLED] = sConfigMgr->GetBoolDefault("Bpay.Enabled", true);

    m_int_configs[CONFIG_BLACKMARKET_MAXAUCTIONS] = sConfigMgr->GetIntDefault("BlackMarket.MaxAuctions", 12);
    m_int_configs[CONFIG_BLACKMARKET_UPDATE_PERIOD] = sConfigMgr->GetIntDefault("BlackMarket.UpdatePeriod", 24);

    // call ScriptMgr if we're reloading the configuration
    m_bool_configs[CONFIG_WINTERGRASP_ENABLE] = sConfigMgr->GetBoolDefault("Wintergrasp.Enable", false);
    m_int_configs[CONFIG_WINTERGRASP_PLR_MAX] = sConfigMgr->GetIntDefault("Wintergrasp.PlayerMax", 100);
    m_int_configs[CONFIG_WINTERGRASP_PLR_MIN] = sConfigMgr->GetIntDefault("Wintergrasp.PlayerMin", 0);
    m_int_configs[CONFIG_WINTERGRASP_PLR_MIN_LVL] = sConfigMgr->GetIntDefault("Wintergrasp.PlayerMinLvl", 77);
    m_int_configs[CONFIG_WINTERGRASP_BATTLETIME] = sConfigMgr->GetIntDefault("Wintergrasp.BattleTimer", 30);
    m_int_configs[CONFIG_WINTERGRASP_NOBATTLETIME] = sConfigMgr->GetIntDefault("Wintergrasp.NoBattleTimer", 150);
    m_int_configs[CONFIG_WINTERGRASP_RESTART_AFTER_CRASH] = sConfigMgr->GetIntDefault("Wintergrasp.CrashRestartTimer", 10);

    m_int_configs[CONFIG_AHBOT_UPDATE_INTERVAL] = sConfigMgr->GetIntDefault("AuctionHouseBot.Update.Interval", 20);

    // Tol'Barad
    m_bool_configs[CONFIG_TOL_BARAD_ENABLE] = sConfigMgr->GetBoolDefault("TolBarad.Enable", false);
    m_int_configs[CONFIG_TOL_BARAD_PLR_MAX] = sConfigMgr->GetIntDefault("TolBarad.PlayerMax", 100);
    m_int_configs[CONFIG_TOL_BARAD_PLR_MIN] = sConfigMgr->GetIntDefault("TolBarad.PlayerMin", 0);
    m_int_configs[CONFIG_TOL_BARAD_PLR_MIN_LVL] = sConfigMgr->GetIntDefault("TolBarad.PlayerMinLvl", 85);
    m_int_configs[CONFIG_TOL_BARAD_BATTLETIME] = sConfigMgr->GetIntDefault("TolBarad.BattleTimer", 15);
    m_int_configs[CONFIG_TOL_BARAD_NOBATTLETIME] = sConfigMgr->GetIntDefault("TolBarad.NoBattleTimer", 150);

    //Announce server for a ban    
    m_bool_configs[CONFIG_ANNOUNCE_BAN] = sConfigMgr->GetBoolDefault("AnnounceBan", false);
    m_bool_configs[CONFIG_ANNOUNCE_MUTE] = sConfigMgr->GetBoolDefault("AnnounceMute", false);
    m_bool_configs[CONFIG_SPELL_FORBIDDEN] = sConfigMgr->GetBoolDefault("SpellForbidden", false);

    // Archaeology
    m_bool_configs[CONFIG_ARCHAEOLOGY_ENABLED] = sConfigMgr->GetBoolDefault("Archaeology.Enabled", true);
    m_float_configs[CONFIG_ARCHAEOLOGY_RARE_BASE_CHANCE] = sConfigMgr->GetFloatDefault("Archaeology.RareBaseChance", 10.0f);
    m_float_configs[CONFIG_ARCHAEOLOGY_RARE_MAXLEVEL_CHANCE] = sConfigMgr->GetFloatDefault("Archaeology.RareMaxLevelChance", 10.0f);

    m_bool_configs[CONFIG_CHECK_MT_SESSION] = sConfigMgr->GetBoolDefault("World.MT.Session", false);

    // Garrison
    m_bool_configs[CONFIG_DISABLE_GARE_UPGRADE] = sConfigMgr->GetBoolDefault("Garrison.DisableUpgrade", false);

    //Respawn system for start project
    m_bool_configs[CONFIG_RESPAWN_FROM_PLAYER_ENABLED] = sConfigMgr->GetBoolDefault("RespawnTimeFromPlayer", false);
    m_int_configs[CONFIG_RESPAWN_FROM_PLAYER_COUNT] = sConfigMgr->GetIntDefault("RespawnTimeFromPlayerCount", 10);

    m_bool_configs[CONFIG_PET_BATTLES] = sConfigMgr->GetBoolDefault("PetBattles", true);
    m_bool_configs[CONFIG_DONATE_ON_TESTS] = sConfigMgr->GetBoolDefault("Donate.On.Tests", false);

    m_int_configs[CONFIG_ARTIFACT_RESEARCH_TIMER] = sConfigMgr->GetIntDefault("Artifact.Research.Timer", 432000); // in sec

    m_float_configs[CONFIG_CAP_KILLPOINTS] = sConfigMgr->GetFloatDefault("Cap.KillPoints", 150.0f);
    m_float_configs[CONFIG_CAP_KILL_CREATURE_POINTS] = sConfigMgr->GetFloatDefault("Cap.KillCreaturePoints", 150.0f);

    m_bool_configs[CONFIG_WORLD_QUEST] = sConfigMgr->GetBoolDefault("WorldQuest", true);
    m_int_configs[CONFIG_WORLD_QUEST_RESET_TIME_HOUR]  = sConfigMgr->GetIntDefault("WorldQuest.ResetTimeHour", 4);
    m_int_configs[CONFIG_WORLD_QUEST_HOURLY_RESET]  = sConfigMgr->GetIntDefault("WorldQuest.HourlyReset", 6);
    m_int_configs[CONFIG_WORLD_QUEST_DAILY_RESET]  = sConfigMgr->GetIntDefault("WorldQuest.DailyReset", 1);
    m_int_configs[CONFIG_INVASION_POINT_RESET]  = sConfigMgr->GetIntDefault("WorldQuest.InvasionPoint", 2);

    m_int_configs[CONFIG_WORLD_QUEST_MIN_ITEMLEVEL]  = sConfigMgr->GetIntDefault("WorldQuest.ItemLevel.Min", 805);
    m_int_configs[CONFIG_WORLD_QUEST_ITEMLEVEL_CAP]  = sConfigMgr->GetIntDefault("WorldQuest.ItemLevel.Max", 855);

    // Cross Faction BG
    m_bool_configs[CONFIG_CROSSFACTIONBG] = sConfigMgr->GetBoolDefault("MixedBGs", false);

    m_bool_configs[CONFIG_RESTRUCT_GUID] = sConfigMgr->GetBoolDefault("Restruct.Guid", false);

    //Time Zone
    m_serverTimeTZ = sConfigMgr->GetStringDefault("ServerTimeTZ", "Europe/Paris");// == number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC
    m_gameTimeTZ = sConfigMgr->GetStringDefault("GameTimeTZ", "Europe/Paris"); // == number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC

    m_int_configs[CONFIG_MAX_PRESTIGE_LEVEL]  = sConfigMgr->GetIntDefault("MaxPrestigeLevel", 14);

    m_int_configs[CONFIG_SIZE_CELL_FOR_PULL]  = sConfigMgr->GetIntDefault("SizeCellForPull", 8);

    m_bool_configs[CONFIG_ANTICHEAT_ENABLED] = sConfigMgr->GetBoolDefault("Anticheat.Enable", false);
    m_bool_configs[CONFIG_ANTICHEAT_ANTI_MULTI_JUMP_ENABLED] = sConfigMgr->GetBoolDefault("Anticheat.AntiMultiJump.Enable", false);
    m_bool_configs[CONFIG_ANTICHEAT_ANTI_SPEED_HACK_ENABLED] = sConfigMgr->GetBoolDefault("Anticheat.AntiSpeedHack.Enable", false);
    m_bool_configs[CONFIG_ANTICHEAT_USE_INTERPOLATION] = sConfigMgr->GetBoolDefault("Anticheat.AntiSpeedHack.UseInterpolation", false);
    m_bool_configs[CONFIG_ANTICHEAT_ANTI_WALL_CLIMB_ENABLED] = sConfigMgr->GetBoolDefault("Anticheat.AntiWallClimb.Enable", false);
    m_bool_configs[CONFIG_ANTICHEAT_ANTI_WATER_WALK_ENABLED] = sConfigMgr->GetBoolDefault("Anticheat.AntiWaterWalk.Enable", false);
    m_bool_configs[CONFIG_ANTICHEAT_NOTIFY_CHEATERS] = sConfigMgr->GetBoolDefault("Anticheat.NotifyCheaters", false);
    m_bool_configs[CONFIG_ANTICHEAT_LOG_DATA] = sConfigMgr->GetBoolDefault("Anticheat.LogData", false);
    m_bool_configs[CONFIG_ANTICHEAT_DETAIL_LOG] = sConfigMgr->GetBoolDefault("Anticheat.Detail.Log", false);

    m_int_configs[CONFIG_ANTICHEAT_MAX_ALLOWED_DESYNC] = sConfigMgr->GetIntDefault("Anticheat.MaxAllowedDesync", 0);
    m_int_configs[CONFIG_ANTICHEAT_GM_ANNOUNCE_MASK] = sConfigMgr->GetIntDefault("Anticheat.GMAnnounceMask", 0);

    m_bool_configs[CONFIG_OBLITERUM_LEVEL_ENABLE]  = sConfigMgr->GetBoolDefault("Obliterum.LevelEnable", true);

    m_int_configs[CONFIG_CHALLENGE_LEVEL_LIMIT] = sConfigMgr->GetIntDefault("Challenge.LevelLimit", 30);
    m_int_configs[CONFIG_CHALLENGE_LEVEL_MAX] = sConfigMgr->GetIntDefault("Challenge.LevelMax", 15);
    m_int_configs[CONFIG_CHALLENGE_LEVEL_STEP] = sConfigMgr->GetIntDefault("Challenge.LevelStep", 0);
    m_int_configs[CONFIG_CHALLENGE_ADD_ITEM] = sConfigMgr->GetIntDefault("Challenge.AddItem", 138019);
    m_int_configs[CONFIG_CHALLENGE_ADD_ITEM_TYPE] = sConfigMgr->GetIntDefault("Challenge.AddItemType", 1);
    m_int_configs[CONFIG_CHALLENGE_ADD_ITEM_COUNT] = sConfigMgr->GetIntDefault("Challenge.AddItemCount", 120);
	m_int_configs[CONFIG_CHALLENGE_MANUAL_AFFIX1] = sConfigMgr->GetIntDefault("Challenge.Manual.Affix1", 0);
	m_int_configs[CONFIG_CHALLENGE_MANUAL_AFFIX2] = sConfigMgr->GetIntDefault("Challenge.Manual.Affix2", 0);
	m_int_configs[CONFIG_CHALLENGE_MANUAL_AFFIX3] = sConfigMgr->GetIntDefault("Challenge.Manual.Affix3", 0);

    m_bool_configs[CONFIG_PVP_LEVEL_ENABLE]  = sConfigMgr->GetBoolDefault("PvP.LevelEnable", true);
    m_int_configs[CONFIG_PVP_ACTIVE_SEASON] = sConfigMgr->GetIntDefault("PvP.ActiveSeason", 0);
    m_int_configs[CONFIG_PVP_ACTIVE_STEP] = sConfigMgr->GetIntDefault("PvP.ActiveStep", 0);

    m_bool_configs[CONFIG_PARAGON_ENABLE]  = sConfigMgr->GetBoolDefault("ParagonEnable", true);

    m_bool_configs[CONFIG_ARTIFACT_TIER_ENABLE]  = sConfigMgr->GetBoolDefault("ArtifactTierEnable", true);

	m_int_configs[CONFIG_WEIGHTED_MYTHIC_KEYSTONE] = sConfigMgr->GetIntDefault("Dungeon.WeightedMythicKeystone.Enabled", 1);
	m_int_configs[CONFIG_PLAYER_AFK_TIMEOUT] = sConfigMgr->GetIntDefault("Player.AFKTimeout", 0);

	m_bool_configs[CONFIG_PLAYER_CONTROL_GUARDIAN_PETS] = sConfigMgr->GetBoolDefault("Player.ControlGuardianPets", true);

	m_bool_configs[CONFIG_PLAYER_UNLIMITED_LEGION_LEGENDARIES] = sConfigMgr->GetBoolDefault("Player.UnlimitedLegionLegendaries", true);
	m_int_configs[CONFIG_PLAYER_LEGION_LEGENDARY_EQUIP_COUNT] = sConfigMgr->GetIntDefault("Player.LegionLegendaryEquipCount", 0);
	m_bool_configs[CONFIG_PLAYER_ALLOW_PVP_TALENTS_ALL_THE_TIME] = sConfigMgr->GetBoolDefault("Player.AllowPVPTalentsAllTheTime", false);

	// Honor for elites and guards
	m_bool_configs[CONFIG_GAIN_HONOR_GUARD] = sConfigMgr->GetBoolDefault("Custom.GainHonorOnGuardKill", true);
	m_bool_configs[CONFIG_GAIN_HONOR_ELITE] = sConfigMgr->GetBoolDefault("Custom.GainHonorOnEliteKill", true);

    // Legion patch configuration
    m_int_configs[CONFIG_LEGION_ENABLED_PATCH] = sConfigMgr->GetIntDefault("Game.Patch", 3);
    if (m_int_configs[CONFIG_LEGION_ENABLED_PATCH] == 1)
    {
        m_int_configs[CONFIG_ITEM_LEGENDARY_LIMIT] = sConfigMgr->GetIntDefault("Item.LegendaryLimit", 144439);
        m_int_configs[CONFIG_ITEM_LEGENDARY_LEVEL] = sConfigMgr->GetIntDefault("Item.LegendaryLevel", 895);

        m_int_configs[CONFIG_OBLITERUM_LEVEL_START] = sConfigMgr->GetIntDefault("Obliterum.LevelStart", 815);
        m_int_configs[CONFIG_OBLITERUM_LEVEL_MIN] = sConfigMgr->GetIntDefault("Obliterum.LevelMin", 815);
        m_int_configs[CONFIG_OBLITERUM_LEVEL_MAX] = sConfigMgr->GetIntDefault("Obliterum.LevelMax", 855);

        m_int_configs[CONFIG_MAX_ITEM_LEVEL] = sConfigMgr->GetIntDefault("Max.ItemLevel", 925);

        m_int_configs[CONFIG_ARTIFACT_KNOWLEDGE_CAP]  = sConfigMgr->GetIntDefault("Artifact.Knowledge.Cap", 25);
        m_int_configs[CONFIG_ARTIFACT_KNOWLEDGE_START]  = sConfigMgr->GetIntDefault("Artifact.Knowledge.Start", 0);
    }
    else if (m_int_configs[CONFIG_LEGION_ENABLED_PATCH] == 2)
    {
        m_int_configs[CONFIG_ITEM_LEGENDARY_LIMIT] = sConfigMgr->GetIntDefault("Item.LegendaryLimit", 147910);
        m_int_configs[CONFIG_ITEM_LEGENDARY_LEVEL] = sConfigMgr->GetIntDefault("Item.LegendaryLevel", 970);

        m_int_configs[CONFIG_OBLITERUM_LEVEL_START] = sConfigMgr->GetIntDefault("Obliterum.LevelStart", 835);
        m_int_configs[CONFIG_OBLITERUM_LEVEL_MIN] = sConfigMgr->GetIntDefault("Obliterum.LevelMin", 835);
        m_int_configs[CONFIG_OBLITERUM_LEVEL_MAX] = sConfigMgr->GetIntDefault("Obliterum.LevelMax", 875);

        m_int_configs[CONFIG_MAX_ITEM_LEVEL] = sConfigMgr->GetIntDefault("Max.ItemLevel", 955);

        m_int_configs[CONFIG_ARTIFACT_KNOWLEDGE_CAP]  = sConfigMgr->GetIntDefault("Artifact.Knowledge.Cap", 40);
        m_int_configs[CONFIG_ARTIFACT_KNOWLEDGE_START]  = sConfigMgr->GetIntDefault("Artifact.Knowledge.Start", 25);
    }
    else
    {
        m_int_configs[CONFIG_ITEM_LEGENDARY_LIMIT] = sConfigMgr->GetIntDefault("Item.LegendaryLimit", 160000);
        m_int_configs[CONFIG_ITEM_LEGENDARY_LEVEL] = sConfigMgr->GetIntDefault("Item.LegendaryLevel", 1000);

        m_int_configs[CONFIG_OBLITERUM_LEVEL_START] = sConfigMgr->GetIntDefault("Obliterum.LevelStart", 0);
        m_int_configs[CONFIG_OBLITERUM_LEVEL_MIN] = sConfigMgr->GetIntDefault("Obliterum.LevelMin", 850);
        m_int_configs[CONFIG_OBLITERUM_LEVEL_MAX] = sConfigMgr->GetIntDefault("Obliterum.LevelMax", 900);

        m_int_configs[CONFIG_MAX_ITEM_LEVEL] = sConfigMgr->GetIntDefault("Max.ItemLevel", 985);

        m_int_configs[CONFIG_ARTIFACT_KNOWLEDGE_CAP]  = sConfigMgr->GetIntDefault("Artifact.Knowledge.Cap", 55);
        m_int_configs[CONFIG_ARTIFACT_KNOWLEDGE_START]  = sConfigMgr->GetIntDefault("Artifact.Knowledge.Start", 40);
    }

    sAnticheatMgr->LoadConfig();

    if (reload)
        sScriptMgr->OnConfigLoad(reload);
}

extern void LoadGameObjectModelList(std::string const& dataPath);

/// Initialize the World
void World::SetInitialWorldSettings()
{
    ///- Server startup begin
    uint32 startupBegin = getMSTime();

    ///- Initialize the random number generator
    srand(static_cast<uint32>(time(nullptr)));

    ///- Initialize VMapManager function pointers (to untangle game/collision circular deps)
    if (VMAP::VMapManager2* vmmgr2 = dynamic_cast<VMAP::VMapManager2*>(VMAP::VMapFactory::createOrGetVMapManager()))
    {
        vmmgr2->GetLiquidFlagsPtr = &DB2Manager::GetLiquidFlags;
        vmmgr2->IsVMAPDisabledForPtr = &DisableMgr::IsVMAPDisabledFor;
    }

    ///- Initialize config settings
    LoadConfigSettings();

    ///- Initialize Allowed Security Level
    LoadDBAllowedSecurityLevel();

    ///- Init highest guids before any table loading to prevent using not initialized guids in some code.
    sObjectMgr->SetHighestGuids();

    ///- Initialize pool manager
    sPoolMgr->Initialize();

    ///- Initialize game event manager
    sGameEventMgr->Initialize();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading anticheat library");
    sAnticheatMgr->LoadFromDB();

    ///- Initialize warden and anticheat systems
    if (m_bool_configs[CONFIG_WARDEN_ENABLED])
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Warden Modules...");
        _wardenMgr->LoadWardenModules("Win");
        _wardenMgr->LoadWardenModules("Wn64");
        _wardenMgr->LoadWardenModules("OSX");

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Warden Checks...");
        _wardenMgr->LoadWardenChecks();

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Warden Action Overrides...");
        _wardenMgr->LoadWardenOverrides();
    }


    ///- Loading strings. Getting no records means core load has to be canceled because no error message can be output.

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Trinity strings...");
    if (!sObjectMgr->LoadTrinityStrings())
        exit(1);                                            // Error message displayed in function already

    ///- Update the realm entry in the database with the realm type from the config file
    //No SQL injection as values are treated as integers

    // not send custom type REALM_FFA_PVP to realm list
    uint32 server_type = IsFFAPvPRealm() ? uint32(REALM_TYPE_PVP) : getIntConfig(CONFIG_GAME_TYPE);
    uint32 realm_zone = getIntConfig(CONFIG_REALM_ZONE);

    LoginDatabase.PExecute("UPDATE realmlist SET icon = %u, timezone = %u WHERE id = '%d'", server_type, realm_zone, realm.Id.Realm);      // One-time query

    ///- Remove the bones (they should not exist in DB though) and old corpses after a restart
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_OLD_CORPSES);
    stmt->setUInt32(0, 3 * DAY);
    CharacterDatabase.Execute(stmt);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading db2 info...");
    sDB2Manager.LoadStores(m_dataPath, m_defaultDbcLocale);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading hotfix info...");
    sDB2Manager.LoadHotfixData();
    HotfixDatabase.Close();

    LoadGameTables(m_dataPath);

    //Load weighted graph on taxi nodes path
    sTaxiPathGraph.Initialize();

    std::unordered_map<uint32, std::vector<uint32>> mapData;
    for (MapEntry const* mapEntry : sMapStore)
    {
        mapData.insert(std::make_pair(mapEntry->ID, std::vector<uint32>()));
        if (mapEntry->ParentMapID != -1)
            mapData[mapEntry->ParentMapID].push_back(mapEntry->ID);
    }

    if (VMAP::VMapManager2* vmmgr2 = dynamic_cast<VMAP::VMapManager2*>(VMAP::VMapFactory::createOrGetVMapManager()))
        vmmgr2->InitializeThreadUnsafe(mapData);

    MMAP::MMapManager* mmmgr = MMAP::MMapFactory::createOrGetMMapManager();
    mmmgr->InitializeThreadUnsafe(mapData);

    sSpellMgr->LoadSpellInfoStore();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading TalentSpellInfo store....");
    sSpellMgr->LoadTalentSpellInfo();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading SkillLineAbilityMultiMap Data...");
    sSpellMgr->LoadSkillLineAbilityMap();

    sSpellMgr->LoadSpellCustomAttr();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading GameObject models...");
    LoadGameObjectModelList(m_dataPath);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading World Map Difficulty Stats...");
    sObjectMgr->LoadWorldMapDiffycultyStat();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading World Visible Distance...");
    sObjectMgr->LoadWorldVisibleDistance();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading World Rate Info...");
    sObjectMgr->LoadWorldRateInfo();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Script Names...");
    sObjectMgr->LoadScriptNames();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Instance Template...");
    sObjectMgr->LoadInstanceTemplate();

    // Must be called before `creature_respawn`/`gameobject_respawn` tables
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading instances...");
    sInstanceSaveMgr->LoadInstances();

    if (m_bool_configs[CONFIG_RESTRUCT_GUID])
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Restructuring Creatures GUIDs...");
        sObjectMgr->RestructCreatureGUID();

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Restructuring Gameobjects GUIDs...");
        sObjectMgr->RestructGameObjectGUID();
    }

    std::thread locales([]() -> void
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Localization strings...");
        uint32 oldMSTime = getMSTime();
        sObjectMgr->LoadCreatureLocales();
        sObjectMgr->LoadGameObjectLocales();
        sQuestDataStore->LoadQuestTemplateLocale();
        sQuestDataStore->LoadQuestOfferRewardLocale();
        sQuestDataStore->LoadQuestRequestItemsLocale();
        sQuestDataStore->LoadQuestObjectivesLocale();
        sObjectMgr->LoadPageTextLocales();
        sGossipDataStore->LoadGossipMenuItemsLocales();
        sQuestDataStore->LoadPointOfInterestLocales();
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Localization strings loaded in %u ms", GetMSTimeDiffToNow(oldMSTime));
    });

    sObjectMgr->SetDBCLocaleIndex(GetDefaultDbcLocale());        // Get once for all the locale index of DBC language (console/broadcasts)

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Loading Letter Analogs...");
    sWordFilterMgr->LoadLetterAnalogs();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Loading Bad Words...");
    sWordFilterMgr->LoadBadWords();
    
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Loading Bad Sentences...");
    sWordFilterMgr->LoadBadSentences();
    
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Loading Complaints...");
    sWordFilterMgr->LoadComplaints();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Page Texts...");
    sObjectMgr->LoadPageTexts();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Object Templates...");         // must be after LoadPageTexts
    sObjectMgr->LoadGameObjectTemplate();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Transport templates...");
    sTransportMgr->LoadTransportTemplates();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Transport animations and rotations...");
    sTransportMgr->LoadTransportAnimationAndRotation();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Rank Data...");
    sSpellMgr->LoadSpellRanks();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Required Data...");
    sSpellMgr->LoadSpellRequired();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Group types...");
    sSpellMgr->LoadSpellGroups();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Learn Skills...");
    sSpellMgr->LoadSpellLearnSkills();                           // must be after LoadSpellRanks

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Learn Spells...");
    sSpellMgr->LoadSpellLearnSpells();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Proc Event conditions...");
    sSpellMgr->LoadSpellProcEvents();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Proc conditions and data...");
    sSpellMgr->LoadSpellProcs();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Bonus Data...");
    sSpellMgr->LoadSpellBonusess();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Aggro Spells Definitions...");
    sSpellMgr->LoadSpellThreats();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Group Stack Rules...");
    sSpellMgr->LoadSpellGroupStackRules();

    sSpellMgr->LoadSpellInfoSpellSpecificAndAuraState();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading forbidden spells...");
    sSpellMgr->LoadForbiddenSpells();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell Phase Dbc Info...");
    sObjectMgr->LoadSpellPhaseInfo();

    sAreaTriggerDataStore->LoadAreaTriggerForces();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading NPC Texts...");
    sObjectMgr->LoadNPCText();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Enchant Spells Proc datas...");
    sSpellMgr->LoadSpellEnchantProcData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Item Random Enchantments Table...");
    LoadRandomEnchantmentsTable();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Disables");
    DisableMgr::LoadDisables();                                 // must be before loading quests and items

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Items...");                         // must be after LoadRandomEnchantmentsTable and LoadPageTexts
    sObjectMgr->LoadItemTemplates();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Item set names...");                // must be after LoadItemPrototypes
    sObjectMgr->LoadItemTemplateAddon();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading Item Scripts...");                 // must be after LoadItemPrototypes
    sObjectMgr->LoadItemScriptNames();

    sConversationDataStore->LoadConversationData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Model Based Info Data...");
    sObjectMgr->LoadCreatureModelInfo();
    
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature template outfits..."); // must be before LoadCreatureTemplates
    sObjectMgr->LoadCreatureOutfits();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Equipment templates...");
    sObjectMgr->LoadEquipmentTemplates();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Texts...");
    sCreatureTextMgr->LoadCreatureTexts();

    sObjectMgr->LoadWDBCreatureTemplates();
    sObjectMgr->LoadCreatureTemplates();
    sEventObjectDataStore->LoadEventObjectTemplates();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature template addons...");
    sObjectMgr->LoadCreatureTemplateAddons();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature difficulty stat...");
    sObjectMgr->LoadCreatureDifficultyStat();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Reputation Reward Rates...");
    sObjectMgr->LoadReputationRewardRate();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Reputation OnKill Data...");
    sObjectMgr->LoadReputationOnKill();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Reputation Spillover Data...");
    sObjectMgr->LoadReputationSpilloverTemplate();

    sQuestDataStore->LoadPointsOfInterest();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Base Stats...");
    sObjectMgr->LoadCreatureClassLevelStats();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Data...");
    sObjectMgr->LoadCreatures();

    sEventObjectDataStore->LoadEventObjects();

    sConversationDataStore->LoadConversations();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Temporary Summon Data...");
    sObjectMgr->LoadTempSummons();                               // must be after LoadCreatureTemplates() and LoadGameObjectTemplates()

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading pet levelup spells...");
    sSpellMgr->LoadPetLevelupSpellMap();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading pet default spells additional to levelup spells...");
    sSpellMgr->LoadPetDefaultSpells();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Addon Data...");
    sObjectMgr->LoadCreatureAddons();                            // must be after LoadCreatureTemplates() and LoadCreatures()
    sObjectMgr->LoadCreatureActionData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Display Choice Data...");
    sObjectMgr->LoadDisplayChoiceData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Choices Locales...");
    sObjectMgr->LoadPlayerChoicesLocale();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Seamless Teleport Data...");
    sObjectMgr->LoadSeamlessTeleportData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Treasure Data...");
    sObjectMgr->LoadPersonalLootTemplate();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading GameObject Action Data...");
    sObjectMgr->LoadGameObjectActionData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Gameobject Data...");
    sObjectMgr->LoadGameobjects();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Linked Respawn...");
    sObjectMgr->LoadLinkedRespawn();                             // must be after LoadCreatures(), LoadGameObjects()

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Weather Data...");
    WeatherMgr::LoadWeatherData();

    sQuestDataStore->LoadQuests();  // must be loaded after DBCs, creature_template, item_template, gameobject tables

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Gameobject quest visual...");
    sObjectMgr->LoadGameObjectQuestVisual();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Checking Quest Disables");
    DisableMgr::CheckQuestDisables();                           // must be after loading quests

    sQuestDataStore->LoadQuestPOI();
    sQuestDataStore->LoadQuestRelations();                            // must be after quest load

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Scenario POI");
    sObjectMgr->LoadScenarioPOI();

    sWorldStateMgr.Initialize();
    sContributionMgr.Initialize();

    InitWorldQuestHourlyResetTime();
    InitWorldQuestDailyResetTime();
    InitInvasionPointResetTime();
    sQuestDataStore->LoadWorldQuestTemplates();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Objects Pooling Data...");
    sPoolMgr->LoadFromDB();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Data...");               // must be after loading pools fully
    sGameEventMgr->LoadFromDB();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading UNIT_NPC_FLAG_SPELLCLICK Data..."); // must be after LoadQuests
    sObjectMgr->LoadNPCSpellClickSpells();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Vehicle Template Accessories...");
    sObjectMgr->LoadVehicleTemplateAccessories();                // must be after LoadCreatureTemplates() and LoadNPCSpellClickSpells()

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading SpellArea Data...");                // must be after quest load
    sSpellMgr->LoadSpellAreas();

    sAreaTriggerDataStore->LoadAreaTriggerTeleports();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Access Requirements...");
    sObjectMgr->LoadAccessRequirements();                        // must be after item template load

    sAreaTriggerDataStore->LoadQuestAreaTriggers();     // must be after LoadQuests
    sAreaTriggerDataStore->LoadTavernAreaTriggers();
    sAreaTriggerDataStore->LoadAreaTriggerActionsAndData();
    sAreaTriggerDataStore->LoadAreaTriggerScripts();

    LoadCharacterNameData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading LFG entrance positions..."); // Must be after areatriggers
    sLFGMgr->LoadLFGDungeons(true);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Dungeon boss data...");
    sObjectMgr->LoadInstanceEncounters();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading LFG rewards...");
    sLFGMgr->LoadRewards();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Graveyard-zone links...");
    sObjectMgr->LoadGraveyardZones();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading spell pet auras...");
    sSpellMgr->LoadSpellPetAuras();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Spell target coordinates...");
    sSpellMgr->LoadSpellTargetPositions();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading enchant custom attributes...");
    sSpellMgr->LoadEnchantCustomAttr();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading linked spells...");
    sSpellMgr->LoadSpellLinked();
    sSpellMgr->LoadTalentSpellLinked();
    sSpellMgr->LoadSpellConcatenateAura();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Prco Check spells...");
    sSpellMgr->LoadSpellPrcoCheck();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading triggered spells...");
    sSpellMgr->LoadSpellTriggered();
    sSpellMgr->LoadSpellVisual();
    sSpellMgr->LoadSpellPendingCast();
    sSpellMgr->LoadSpellScene();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Create Data...");
    sObjectMgr->LoadPlayerInfo();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Exploration BaseXP Data...");
    sObjectMgr->LoadExplorationBaseXP();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Pet Name Parts...");
    sObjectMgr->LoadPetNames();

    CharacterDatabaseCleaner::CleanDatabase();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading the max pet number...");
    sObjectMgr->LoadPetNumber();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading pet level stats...");
    sObjectMgr->LoadPetStats();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player Corpses...");
    sObjectMgr->LoadCorpses();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Player level dependent mail rewards...");
    sObjectMgr->LoadMailLevelRewards();

    // Loot tables
    LoadLootTables();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Skill Discovery Table...");
    LoadSkillDiscoveryTable();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Skill Extra Item Table...");
    LoadSkillExtraItemTable();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Skill Fishing base level requirements...");
    sObjectMgr->LoadFishingBaseSkillLevel();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading skill tier info...");
    sObjectMgr->LoadSkillTiers();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Achievements...");
    sAchievementMgr->LoadAchievementReferenceList();
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Criteria Lists...");
    sAchievementMgr->LoadCriteriaList();
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Achievement Criteria Data...");
    sAchievementMgr->LoadAchievementCriteriaData();
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Achievement Rewards...");
    sAchievementMgr->LoadRewards();
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Achievement Reward Locales...");
    sAchievementMgr->LoadRewardLocales();
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Completed Achievements...");
    sAchievementMgr->LoadCompletedAchievements();

    ///- Load dynamic data tables from the database
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Item Auctions...");
    sAuctionMgr->LoadAuctionItems();
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Auctions...");
    sAuctionMgr->LoadAuctions();

    if (m_bool_configs[CONFIG_BLACKMARKET_ENABLED])
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Black Market Templates...");
        sBlackMarketMgr->LoadTemplates();

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Black Market Auctions...");
        sBlackMarketMgr->LoadAuctions();
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Currencys Loot...");
    sObjectMgr->LoadCurrencysLoot();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Guild XP for level...");
    sGuildMgr->LoadGuildXpForLevel();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Guild rewards...");
    sGuildMgr->LoadGuildRewards();
    sGuildMgr->LoadGuildChallengeRewardInfo();

    sGuildMgr->LoadGuilds();

    sGuildFinderMgr->LoadFromDB();

    sBracketMgr->LoadCharacterBrackets();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Groups...");
    sGroupMgr->LoadGroups();

    sCharacterDataStore->LoadReservedPlayersNames();

    sQuestDataStore->LoadGameObjectForQuests();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading BattleMasters...");
    sBattlegroundMgr->LoadBattleMastersEntry();
    sBattlegroundMgr->LoadPvpRewards();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading GameTeleports...");
    sObjectMgr->LoadGameTele();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Gossip menu...");
    sGossipDataStore->LoadGossipMenu();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Gossip menu options...");
    sGossipDataStore->LoadGossipMenuItems();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Vendors...");
    sObjectMgr->LoadVendors();                                   // must be after load CreatureTemplate and ItemTemplate
    
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Donate Vendors...");
    sObjectMgr->LoadDonateVendors(); 
    
    m_timers[WUPDATE_DONATE_AND_SERVICES].SetInterval( 3 * HOUR * IN_MILLISECONDS);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Trainers...");
    sObjectMgr->LoadTrainerSpell();                              // must be after load CreatureTemplate

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Waypoints...");
    sWaypointMgr->Load();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading SmartAI Waypoints...");
    sSmartWaypointMgr->LoadFromDB();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Creature Formations...");
    sFormationMgr->LoadCreatureFormations();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading World States...");              // must be loaded before battleground, outdoor PvP and conditions
    LoadWorldStates();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Phase definitions...");
    sObjectMgr->LoadPhaseDefinitions();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Scenario data...");
    sObjectMgr->LoadScenarioData();
    
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Scenario Step Spell data...");
    sObjectMgr->LoadScenarioSpellData();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Conditions...");
    sConditionMgr->LoadConditions();

    sCharacterDataStore->LoadFactionChangeAchievements();
    sCharacterDataStore->LoadFactionChangeSpells();
    sCharacterDataStore->LoadFactionChangeItems();
    sCharacterDataStore->LoadFactionChangeReputations();
    sCharacterDataStore->LoadFactionChangeTitles();

    CollectionMgr::LoadMountDefinitions();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading GM tickets...");
    sTicketMgr->LoadTickets();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading GM surveys...");
    sTicketMgr->LoadSurveys();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading client addons...");
    AddonMgr::LoadFromDB();

    ///- Handle outdated emails (delete/return)
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Returning old mails...");
    sObjectMgr->ReturnOrDeleteOldMails(false);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Autobroadcasts...");
    LoadAutobroadcasts();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading challenge save...");
    sChallengeMgr->LoadFromDB();

    ///- Load and initialize scripts
    sScriptDataStore->LoadQuestStartScripts();                         // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sScriptDataStore->LoadQuestEndScripts();                           // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sScriptDataStore->LoadSpellScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sScriptDataStore->LoadGameObjectScripts();                         // must be after load Creature/Gameobject(Template/Data)
    sScriptDataStore->LoadEventScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sScriptDataStore->LoadWaypointScripts();
    sScriptDataStore->LoadDbScriptStrings();                            // must be after Load*Scripts calls
    sScriptDataStore->LoadSpellScriptNames();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading DeathMatch Store Products...");
    sObjectMgr->LoadDeathMatchStore();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Initializing Scripts...");
    sScriptMgr->Initialize();
    sScriptMgr->OnConfigLoad(false);                                // must be done after the ScriptMgr has been properly initialized

    sScriptDataStore->ValidateSpellScripts();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading SmartAI scripts...");
    sSmartScriptMgr->LoadSmartAIFromDB();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Calendar data...");
    sCalendarMgr->LoadFromDB();

    ///- Initialize game time and timers
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Initialize game time and timers");
    m_gameTime = time(nullptr);
    m_startTime = m_gameTime;

    LoginDatabase.PExecute("INSERT INTO uptime (realmid, starttime, uptime, revision) VALUES(%u, %u, 0, '%s')", realm.Id.Realm, uint32(m_startTime), GitRevision::GetFullVersion());       // One-time query

    m_timers[WUPDATE_AUCTIONS].SetInterval(MINUTE*IN_MILLISECONDS);
    m_timers[WUPDATE_UPTIME].SetInterval(m_int_configs[CONFIG_UPTIME_UPDATE]*MINUTE*IN_MILLISECONDS);
                                                            //Update "uptime" table based on configuration entry in minutes.
    m_timers[WUPDATE_CORPSES].SetInterval(20 * MINUTE * IN_MILLISECONDS);
                                                            //erase corpses every 20 minutes
    m_timers[WUPDATE_CLEANDB].SetInterval(m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]*MINUTE*IN_MILLISECONDS);
                                                            // clean logs table every 14 days by default
    m_timers[WUPDATE_AUTOBROADCAST].SetInterval(getIntConfig(CONFIG_AUTOBROADCAST_INTERVAL));
    m_timers[WUPDATE_DELETECHARS].SetInterval(DAY*IN_MILLISECONDS); // check for chars to delete every day

    m_timers[WUPDATE_AHBOT].SetInterval(getIntConfig(CONFIG_AHBOT_UPDATE_INTERVAL) * IN_MILLISECONDS); // every 20 sec

    m_timers[WUPDATE_PINGDB].SetInterval(getIntConfig(CONFIG_DB_PING_INTERVAL)*MINUTE*IN_MILLISECONDS);    // Mysql ping time in minutes

    m_timers[WUPDATE_GUILDSAVE].SetInterval(getIntConfig(CONFIG_GUILD_SAVE_INTERVAL) * MINUTE * IN_MILLISECONDS);

    m_timers[WUPDATE_BLACKMARKET].SetInterval(10 * IN_MILLISECONDS);
    blackmarket_timer = 0;

    //to set mailtimer to return mails every day between 4 and 5 am
    //mailtimer is increased when updating auctions
    //one second is 1000 -(tested on win system)
    /// @todo Get rid of magic numbers
    tm localTm;
    localtime_r(&m_gameTime, &localTm);
    mail_timer = ((((localTm.tm_hour + 20) % 24)* HOUR * IN_MILLISECONDS) / m_timers[WUPDATE_AUCTIONS].GetInterval());
                                                            //1440
    mail_timer_expires = ((DAY * IN_MILLISECONDS) / (m_timers[WUPDATE_AUCTIONS].GetInterval()));
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Mail timer set to: " UI64FMTD ", mail return is called every " UI64FMTD " minutes", uint64(mail_timer), uint64(mail_timer_expires));

    ///- Initilize static helper structures
    AIRegistry::Initialize();

    ///- Initialize MapManager
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Starting Map System");
    sMapMgr->Initialize();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Starting Game Event system...");
    uint32 nextGameEvent = sGameEventMgr->StartSystem();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);    //depend on next event

    // Delete all characters which have been deleted X days before
    Player::DeleteOldCharacters();

    TC_LOG_INFO(LOG_FILTER_AUCTIONHOUSE, "Initialize AuctionHouseBot...");
    sAuctionBot->Initialize();

    // Delete all custom channels which haven't been used for PreserveCustomChannelDuration days.
    Channel::CleanOldChannelsInDB();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Starting Arena Season...");
    sGameEventMgr->StartArenaSeason();

    sTicketMgr->Initialize();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Initializing Mailbox queue system...");
    m_timers[WUPDATE_MAILBOXQUEUE].SetInterval( 2 * MINUTE * 1000 );

    ///- Initialize Battlegrounds
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Starting Battleground System");
    sBattlegroundMgr->CreateInitialBattlegrounds();
    sBattlegroundMgr->InitializeBrawlData();
    CharacterDatabase.Execute("UPDATE `character_battleground_data` SET `team` = 0"); // Need update if crash server

    ///- Initialize outdoor pvp
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Starting Outdoor PvP System");
    sOutdoorPvPMgr->InitOutdoorPvP();

    ///- Initialize Battlefield
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Starting Battlefield System");
    sBattlefieldMgr->InitBattlefield();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Transports...");
    sTransportMgr->SpawnContinentTransports();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Deleting expired bans...");
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate <= UNIX_TIMESTAMP() AND unbandate<>bandate");      // One-time query

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Update Challenge Key...");
    CharacterDatabase.Execute("UPDATE `challenge_key` SET `InstanceID` = 0"); // Need update if crash server on active key

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Calculate next daily quest reset time...");
    InitDailyQuestResetTime();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Calculate next weekly reset time...");
    InitWeeklyResetTime();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Calculate random battleground reset time...");
    InitRandomBGResetTime();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Calculate next currency reset time...");
    InitCurrencyResetTime();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Calculate next instanse reset time...");
    InitInstanceDailyResetTime();
    InitInstanceWeeklyResetTime();
    InitChallengeKeyResetTime();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Calculate next banwave time...");
    InitBanWaveTime();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Init Auto Restart time..." );
    InitServerAutoRestartTime();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading race and class expansion requirements...");
    sObjectMgr->LoadRaceAndClassExpansionRequirements();
    sCharacterDataStore->LoadCharacterTemplates();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading realm names...");
    sObjectMgr->LoadRealmNames();

    sBattlePayDataStore->Initialize();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Initializing Opcodes...");
    opcodeTable.Initialize();
    WorldPackets::Auth::ConnectTo::InitializeEncryption();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading research site to zone mapping data...");
    sObjectMgr->LoadResearchSiteToZoneData();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading archaeology digsite positions...");
    sObjectMgr->LoadDigSitePositions();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading garrisons...");
    sGarrisonMgr.Initialize();

    sBattlePetDataStore->Initialize();
    sWildBattlePetMgr->Load();

    sWorldStateMgr.CreateWorldStatesIfNeed();

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading realm name...");

    m_realmName = "Legion servers";
    QueryResult realmResult = LoginDatabase.PQuery("SELECT name FROM realmlist WHERE id = %u", realm.Id.Realm);
    if (realmResult)
        m_realmName = (*realmResult)[0].GetString();

    for (size_t i = 0; i < m_realmName.size(); ++i)
        if (m_realmName[i] != ' ')
            m_trimmedRealmName += m_realmName[i];

    locales.join();
    uint32 startupDuration = GetMSTimeDiffToNow(startupBegin);

    TC_LOG_INFO(LOG_FILTER_WORLDSERVER, "World initialized in %u minutes %u seconds", (startupDuration / 60000), ((startupDuration % 60000) / 1000));
    sLog->EnableDBAppenders();
}

void World::RecordTimeDiff(const char *text, ...)
{
    if (m_updateTimeCount != 1)
        return;
    if (!text)
    {
        m_currentTime = getMSTime();
        return;
    }

    uint32 thisTime = getMSTime();
    uint32 diff = getMSTimeDiff(m_currentTime, thisTime);

    if (diff > m_int_configs[CONFIG_MIN_LOG_UPDATE])
    {
        va_list ap;
        char str[256];
        va_start(ap, text);
        vsnprintf(str, 256, text, ap);
        va_end(ap);
        sLog->outDiff("Difftime %s: %u.", str, diff);
    }

    m_currentTime = thisTime;
}

void World::LoadAutobroadcasts()
{
    uint32 oldMSTime = getMSTime();

    m_Autobroadcasts.clear();

    QueryResult result = WorldDatabase.Query("SELECT text FROM autobroadcast");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 autobroadcasts definitions. DB table `autobroadcast` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {

        Field* fields = result->Fetch();
        std::string message = fields[0].GetString();

        m_Autobroadcasts.push_back(message);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u autobroadcasts definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

/// Update the World !
void World::Update(uint32 diff)
{
    m_updateTime = diff;

    if (m_int_configs[CONFIG_INTERVAL_LOG_UPDATE])
    {
        if (m_updateTimeSum > m_int_configs[CONFIG_INTERVAL_LOG_UPDATE])
        {
            sLog->outDiff("Update time diff: %u. Players online: %u.", m_updateTimeSum / m_updateTimeCount, GetActiveSessionCountDiff());
            m_updateTimeSum = m_updateTime;
            m_updateTimeCount = 1;
            sMapMgr->LogInfoAllMaps();
        }
        else
        {
            m_updateTimeSum += m_updateTime;
            ++m_updateTimeCount;
        }
    }

    ///- Update the different timers
    for (int i = 0; i < WUPDATE_COUNT; ++i)
    {
        if (m_timers[i].GetCurrent() >= 0)
            m_timers[i].Update(diff);
        else
            m_timers[i].SetCurrent(0);
    }

    RecordTimeDiff(nullptr);
    ///- Update the game time and check for shutdown time
    _UpdateGameTime();

    /// Handle daily quests reset time
    if (m_gameTime > m_NextDailyQuestReset)
    {
        ResetDailyQuests();
        m_NextDailyQuestReset += DAY;
    }

    if (m_gameTime > m_NextWeeklyReset)
        ResetWeekly();

    if (m_gameTime > m_NextRandomBGReset)
        ResetRandomBG();

    if (m_gameTime > m_NextCurrencyReset)
        ResetCurrencyWeekCap();

    if (m_gameTime > m_NextInstanceDailyReset)
        InstanceDailyResetTime();

    if (m_gameTime > m_NextWorldQuestHourlyReset)
        WorldQuestHourlyResetTime();

    if (m_gameTime > m_NextWorldQuestDailyReset)
        WorldQuestDailyResetTime();

    if (m_gameTime > m_NextInvasionPointReset)
        InvasionPointResetTime();

    if (m_gameTime > m_NextInstanceWeeklyReset)
    {
        InstanceWeeklyResetTime();
        ResetLootCooldown();
    }

    if (m_gameTime > m_NextChallengeKeyReset)
        ChallengeKeyResetTime();

    if (m_gameTime > m_NextBanWaveTime)
        StartBanWave();

    if (!m_bool_configs[CONFIG_DISABLE_RESTART])
        if (m_gameTime > m_NextServerRestart)
            AutoRestartServer();

    /// Handle auctions when the timer has passed
    if (m_timers[WUPDATE_AUCTIONS].OnTimerPassReset())
    {
        ///- Update mails (return old mails with item, or delete them)
        //(tested... works on win)
        if (++mail_timer > mail_timer_expires)
        {
            mail_timer = 0;
            sObjectMgr->ReturnOrDeleteOldMails(true);
        }

        ///- Handle expired auctions
        sAuctionMgr->Update();
    }

    if (m_timers[WUPDATE_BLACKMARKET].OnTimerPassReset())
    {
        sBlackMarketMgr->Update();

        ///- Update blackmarket, refresh auctions if necessary
        if ((blackmarket_timer * m_timers[WUPDATE_BLACKMARKET].GetInterval() >= getIntConfig(CONFIG_BLACKMARKET_UPDATE_PERIOD) * HOUR * IN_MILLISECONDS) || !blackmarket_timer)
        {
            sBlackMarketMgr->RefreshAuctions();
            blackmarket_timer = 1; // timer is 0 on startup
        }
        else
            ++blackmarket_timer;
    }

    /// <li> Handle AHBot operations
    if (m_timers[WUPDATE_AHBOT].Passed())
    {
        sAuctionBot->Update();
        m_timers[WUPDATE_AHBOT].Reset();
    }

    sWorldStateMgr.Update(diff);
    sContributionMgr.Update(diff);

    /// <li> Handle session updates when the timer has passed
    UpdateSessions(diff);

    /// <li> Update uptime table
    if (m_timers[WUPDATE_UPTIME].Passed())
    {
        uint32 tmpDiff = uint32(m_gameTime - m_startTime);
        uint32 maxOnlinePlayers = GetMaxPlayerCount();

        m_timers[WUPDATE_UPTIME].Reset();

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_UPTIME_PLAYERS);

        stmt->setUInt32(0, tmpDiff);
        stmt->setUInt16(1, uint16(maxOnlinePlayers));
        stmt->setUInt32(2, realm.Id.Realm);
        stmt->setUInt32(3, uint32(m_startTime));

        LoginDatabase.Execute(stmt);

        if (!m_bool_configs[CONFIG_DISABLE_NEW_ONLINE])
            LoginDatabase.PQuery("REPLACE INTO `online` (`realmID`, `online`, `diff`, `uptime`) VALUES ('%u', '%u', '%u', '%u')", realm.Id.Realm, GetActiveSessionCount(), GetUpdateTime(), GetUptime());
    }

    /// <li> Clean logs table
    if (sWorld->getIntConfig(CONFIG_LOGDB_CLEARTIME) > 0) // if not enabled, ignore the timer
    {
        if (m_timers[WUPDATE_CLEANDB].Passed())
        {
            m_timers[WUPDATE_CLEANDB].Reset();

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_OLD_LOGS);

            stmt->setUInt32(0, sWorld->getIntConfig(CONFIG_LOGDB_CLEARTIME));
            stmt->setUInt32(1, uint32(time(nullptr)));

            LoginDatabase.Execute(stmt);
        }
    }

    /// <li> Handle all other objects
    ///- Update objects when the timer has passed (maps, transport, creatures, ...)
    sMapMgr->Update(diff);

    if (sWorld->getBoolConfig(CONFIG_AUTOBROADCAST))
    {
        if (m_timers[WUPDATE_AUTOBROADCAST].Passed())
        {
            m_timers[WUPDATE_AUTOBROADCAST].Reset();
            SendAutoBroadcast();
        }
    }

    sBattlegroundMgr->Update(diff);

    ///- Delete all characters which have been deleted X days before
    if (m_timers[WUPDATE_DELETECHARS].Passed())
    {
        m_timers[WUPDATE_DELETECHARS].Reset();
        Player::DeleteOldCharacters();
    }

    sLFGMgr->Update(diff);

    sLFGListMgr->Update(diff);

    sGroupMgr->Update(diff);

    // execute callbacks from sql queries that were queued recently
    ProcessQueryCallbacks();

    ///- Erase corpses once every 20 minutes
    if (m_timers[WUPDATE_CORPSES].Passed())
    {
        m_timers[WUPDATE_CORPSES].Reset();
        sObjectAccessor->RemoveOldCorpses();
    }

    ///- Process Game events when necessary
    if (m_timers[WUPDATE_EVENTS].Passed())
    {
        m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
        uint32 nextGameEvent = sGameEventMgr->Update();
        m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
        m_timers[WUPDATE_EVENTS].Reset();
    }

    if (m_timers[WUPDATE_MAILBOXQUEUE].Passed())
    {
        m_timers[WUPDATE_MAILBOXQUEUE].Reset();
        ProcessMailboxQueue();
        Transfer();
    }
    
    if (m_timers[WUPDATE_DONATE_AND_SERVICES].Passed())
    {
        m_timers[WUPDATE_DONATE_AND_SERVICES].Reset();
        sObjectMgr->LoadDonateVendors();
    }

    ///- Ping to keep MySQL connections alive
    if (m_timers[WUPDATE_PINGDB].Passed())
    {
        m_timers[WUPDATE_PINGDB].Reset();
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Ping MySQL to keep connection alive");
        CharacterDatabase.KeepAlive();
        LoginDatabase.KeepAlive();
        WorldDatabase.KeepAlive();
    }

    if (m_timers[WUPDATE_GUILDSAVE].Passed())
    {
        m_timers[WUPDATE_GUILDSAVE].Reset();
        sGuildMgr->SaveGuilds();
    }

    sPetBattleSystem->Update(diff);

    sInstanceSaveMgr->Update();

    // And last, but not least handle the issued cli commands
    ProcessCliCommands();

    UpdateGlobalMessage();

    RecordTimeDiff("UpdateGlobalMessage");
}

void World::ForceGameEventUpdate()
{
    m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
    uint32 nextGameEvent = sGameEventMgr->Update();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
    m_timers[WUPDATE_EVENTS].Reset();
}

/// Send a packet to all players (except self if mentioned)
void World::SendGlobalMessage(WorldPacket const* packet, WorldSession* self, uint32 team)
{
    _messageQueueLock.lock();
    _messageQueue.push(GlobalMessageData(packet, self, team));
    _messageQueueLock.unlock();
}

void World::UpdateGlobalMessage()
{
    if (_messageQueue.empty())
        return;

    std::queue<GlobalMessageData> aMessageQueue;
    {
        _messageQueueLock.lock();
        std::swap(aMessageQueue, _messageQueue);
        _messageQueueLock.unlock();
    }

    if (aMessageQueue.empty())
        return;

    while (!aMessageQueue.empty())
    {
        GlobalMessageData* message = &aMessageQueue.front();

        for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        {
            if (itr->second &&
                itr->second->GetPlayer() &&
                itr->second->GetPlayer()->IsInWorld() &&
                itr->second.get() != message->self &&
                (message->team == 0 || itr->second->GetPlayer()->GetTeam() == message->team))
            {
                itr->second->SendPacket(&message->packet);
            }
        }
        aMessageQueue.pop();
    }
}

/// Send a packet to all GMs (except self if mentioned)
void World::SendGlobalGMMessage(WorldPacket const* packet, WorldSession* self, uint32 team)
{
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        // check if session and can receive global GM Messages and its not self
        WorldSession* session = itr->second.get();
        if (!session || session == self || AccountMgr::IsPlayerAccount(itr->second->GetSecurity()))
            continue;

        // Player should be in world
        Player* player = session->GetPlayer();
        if (!player || !player->IsInWorld() /*|| !player->IsSpectator()*/)
            continue;

        // Send only to same team, if team is given
        if (!team || player->GetTeam() == team)
            player->SendDirectMessage(packet);
    }
}

namespace Trinity
{
    class WorldWorldTextBuilder
    {
        public:
            typedef std::vector<WorldPackets::Packet*> WorldPacketList;
            static size_t const BufferSize = 2048;

            explicit WorldWorldTextBuilder(uint32 textId, va_list* args = nullptr) : i_textId(textId), i_args(args) { }

            void operator()(WorldPacketList& dataList, LocaleConstant locale)
            {
                char const* text = sObjectMgr->GetTrinityString(i_textId, locale);

                char strBuffer[BufferSize];
                if (i_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap, *i_args);
                    vsnprintf(strBuffer, BufferSize, text, ap);
                    va_end(ap);
                }
                else
                {
                    std::strncpy(strBuffer, text, BufferSize);
                    strBuffer[BufferSize - 1] = '\0';
                }

                do_helper(dataList, strBuffer);
            }

        private:
            static void do_helper(WorldPacketList& dataList, char* text)
            {
                while (char* line = ChatHandler::LineFromMessage(text))
                {
                    WorldPackets::Chat::Chat* packet = new WorldPackets::Chat::Chat();
                    packet->Initialize(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
                    packet->Write();
                    dataList.push_back(packet);
                }
            }

            uint32 i_textId;
            va_list* i_args;
    };
}                                                           // namespace Trinity

/// Send a System Message to all players (except self if mentioned)
void World::SendWorldText(int32 string_id, ...)
{
    va_list ap;
    va_start(ap, string_id);

    Trinity::WorldWorldTextBuilder wt_builder(string_id, &ap);
    Trinity::LocalizedPacketListDo<Trinity::WorldWorldTextBuilder> wt_do(wt_builder);
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld())
            continue;

        wt_do(itr->second->GetPlayer());
    }

    va_end(ap);
}

/// Send a System Message to all GMs (except self if mentioned)
void World::SendGMText(int32 string_id, ...)
{
    va_list ap;
    va_start(ap, string_id);

    Trinity::WorldWorldTextBuilder wt_builder(string_id, &ap);
    Trinity::LocalizedPacketListDo<Trinity::WorldWorldTextBuilder> wt_do(wt_builder);
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld())
            continue;

        if (itr->second->GetPlayer()->IsSpectator() || AccountMgr::IsPlayerAccount(itr->second->GetSecurity()))
            continue;

        wt_do(itr->second->GetPlayer());
    }

    va_end(ap);
}

/// DEPRECATED, only for debug purpose. Send a System Message to all players (except self if mentioned)
void World::SendGlobalText(const char* text, WorldSession* self)
{
    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(text);
    char* pos = buf;

    while (char* line = ChatHandler::LineFromMessage(pos))
    {
        WorldPackets::Chat::Chat packet;
        packet.Initialize(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        SendGlobalMessage(packet.Write(), self);
    }

    free(buf);
}

/// Send a packet to all players (or players selected team) in the zone (except self if mentioned)
bool World::SendZoneMessage(uint32 zone, WorldPacket const* packet, WorldSession* self, uint32 team)
{
    bool foundPlayerToSend = false;

    for (auto itr = m_sessions.cbegin(); itr != m_sessions.cend(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second->GetPlayer()->GetCurrentZoneID() == zone &&
            itr->second.get() != self &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team))
        {
            itr->second->SendPacket(packet);
            foundPlayerToSend = true;
        }
    }

    return foundPlayerToSend;
}

/// Send a System Message to all players in the zone (except self if mentioned)
void World::SendZoneText(uint32 zone, char const* text, WorldSession* self /*= nullptr*/, uint32 team /*= 0*/)
{
    WorldPackets::Chat::Chat packet;
    packet.Initialize(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, text);
    SendZoneMessage(zone, packet.Write(), self, team);
}

/// Kick (and save) all players
void World::KickAll()
{
    m_QueuedPlayer.clear();                                 // prevent send queue update packet and login queued sessions

    // session not removed at kick and will removed in next update tick
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        itr->second->KickPlayer();
        itr->second->SetMap(nullptr);
    }
}

/// Kick (and save) all players with security level less `sec`
void World::KickAllLess(AccountTypes sec)
{
    // session not removed at kick and will removed in next update tick
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetSecurity() < sec)
            itr->second->KickPlayer();
}

/// Ban an account or ban an IP address, duration will be parsed using TimeStringToSecs if it is positive, otherwise permban
BanReturn World::BanAccount(BanMode mode, std::string nameOrIP, std::string duration, std::string reason, std::string author, bool queued)
{
    uint32 duration_secs = TimeStringToSecs(duration);
    PreparedQueryResult resultAccounts = PreparedQueryResult(nullptr); //used for kicking
    PreparedStatement* stmt = nullptr;

    ///- Update the database with ban information
    switch (mode)
    {
        case BAN_IP:
            // No SQL injection with prepared statements
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_IP);
            stmt->setString(0, nameOrIP);
            resultAccounts = LoginDatabase.Query(stmt);
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_IP_BANNED);
            stmt->setString(0, nameOrIP);
            stmt->setUInt32(1, duration_secs);
            stmt->setString(2, author);
            stmt->setString(3, reason);
            LoginDatabase.Execute(stmt);
            break;
        case BAN_ACCOUNT:
            // No SQL injection with prepared statements
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME);
            stmt->setString(0, nameOrIP);
            resultAccounts = LoginDatabase.Query(stmt);
            break;
        case BAN_CHARACTER:
            // No SQL injection with prepared statements
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_BY_NAME);
            stmt->setString(0, nameOrIP);
            resultAccounts = CharacterDatabase.Query(stmt);
            break;
        default:
            return BAN_SYNTAX_ERROR;
    }

    if (!resultAccounts)
    {
        if (mode == BAN_IP)
            return BAN_SUCCESS;                             // ip correctly banned but nobody affected (yet)
        return BAN_NOTFOUND;
        // Nobody to ban
    }

    ///- Disconnect all affected players (for IP it can be several)
    SQLTransaction trans = LoginDatabase.BeginTransaction();
    do
    {
        Field* fieldsAccount = resultAccounts->Fetch();
        uint32 account = fieldsAccount[0].GetUInt32();

        if (mode != BAN_IP)
        {
            // make sure there is only one active ban
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BANNED);
            stmt->setUInt32(0, account);
            PreparedQueryResult banresult = LoginDatabase.Query(stmt);
            if (banresult)
            {
                stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_BANNED);
                stmt->setUInt32(0, duration_secs);
                stmt->setUInt32(1, account);
                trans->Append(stmt);
            }
            else
            {
                // No SQL injection with prepared statements
                stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_BANNED);
                stmt->setUInt32(0, account);
                stmt->setUInt32(1, duration_secs);
                stmt->setString(2, author);
                stmt->setString(3, reason);
                trans->Append(stmt);
            }
        }

        if (!queued)
        {
            if (WorldSessionPtr sess = FindSession(account))
                if (std::string(sess->GetPlayerName()) != author)
                    sess->KickPlayer();
        }

    } while (resultAccounts->NextRow());

    LoginDatabase.CommitTransaction(trans);

    return BAN_SUCCESS;
}

/// Remove a ban from an account or IP address
bool World::RemoveBanAccount(BanMode mode, std::string nameOrIP)
{
    PreparedStatement* stmt = nullptr;
    if (mode == BAN_IP)
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_IP_NOT_BANNED);
        stmt->setString(0, nameOrIP);
        LoginDatabase.Execute(stmt);
    }
    else
    {
        uint32 account = 0;
        if (mode == BAN_ACCOUNT)
            account = AccountMgr::GetId(nameOrIP);
        else if (mode == BAN_CHARACTER)
            account = ObjectMgr::GetPlayerAccountIdByPlayerName(nameOrIP);

        if (!account)
            return false;

        //NO SQL injection as account is uint32
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_NOT_BANNED);
        stmt->setUInt32(0, account);
        LoginDatabase.Execute(stmt);
    }
    return true;
}

void World::FlagAccount(uint32 accountId, uint32 duration, std::string reason, std::string author)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ACCOUNT_FLAGGED);
    stmt->setUInt32(0, accountId);
    stmt->setUInt32(1, duration);
    stmt->setString(2, author);
    stmt->setString(3, reason);
    CharacterDatabase.Execute(stmt);
}

void World::BanFlaggedAccounts()
{
    //NO SQL injection as account is uint32
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_FLAGGED_ALL);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    do
    {
        Field* fields = result->Fetch();
        uint32 accountid = fields[0].GetUInt32();
        uint32 duration = fields[1].GetUInt32();
        std::string author = fields[2].GetString();
        std::string banreason = fields[3].GetString();

        std::stringstream durationStr;
        if (duration == 0)
            durationStr << "-1";
        else
            durationStr << duration << "s";

        std::string accountName;
        AccountMgr::GetName(accountid, accountName);

        // ban flagged account
        BanAccount(BAN_ACCOUNT, accountName, durationStr.str(), banreason, author);

        // delete flagged account
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ACCOUNT_FLAGGED);
        stmt->setUInt32(0, accountid);
        trans->Append(stmt);

    } while (result->NextRow());

    CharacterDatabase.CommitTransaction(trans);
}

/// Ban an account or ban an IP address, duration will be parsed using TimeStringToSecs if it is positive, otherwise permban
BanReturn World::BanCharacter(std::string name, std::string duration, std::string reason, std::string author)
{
    Player* pBanned = sObjectAccessor->FindPlayerByName(name);
    ObjectGuid::LowType guid = 0;

    uint32 duration_secs = TimeStringToSecs(duration);

    /// Pick a player to ban if not online
    if (!pBanned)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_BY_NAME);
        stmt->setString(0, name);
        PreparedQueryResult resultCharacter = CharacterDatabase.Query(stmt);

        if (!resultCharacter)
            return BAN_NOTFOUND;                                    // Nobody to ban

        guid = (*resultCharacter)[0].GetUInt64();
    }
    else
        guid = pBanned->GetGUIDLow();

    // make sure there is only one active ban
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_BAN);
    stmt->setUInt64(0, guid);
    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_BAN);
    stmt->setUInt64(0, guid);
    stmt->setUInt32(1, duration_secs);
    stmt->setString(2, author);
    stmt->setString(3, reason);
    CharacterDatabase.Execute(stmt);

    if (pBanned)
        pBanned->GetSession()->KickPlayer();

    return BAN_SUCCESS;
}

/// Remove a ban from a character
bool World::RemoveBanCharacter(std::string name)
{
    Player* pBanned = sObjectAccessor->FindPlayerByName(name);
    ObjectGuid::LowType guid = 0;

    /// Pick a player to ban if not online
    if (!pBanned)
    {
        auto nameData = sWorld->GetCharacterInfo(name);
        if (!nameData)
            return false;

        guid = nameData->Guid;
    }
    else
        guid = pBanned->GetGUIDLow();

    if (!guid)
        return false;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_BAN);
    stmt->setUInt64(0, guid);
    CharacterDatabase.Execute(stmt);
    return true;
}

void World::MuteAccount(uint32 accountId, int64 duration, std::string reason, std::string author, WorldSession* session /* = nullptr */)
{

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);

    int64 muteTime = (duration>0 ? time(NULL) + duration * MINUTE : 4294967295);

    QueryResult result = LoginDatabase.PQuery("SELECT mutetime FROM account WHERE id = %u", accountId);

    if (result)
    {
        Field* fields = result->Fetch();
        uint32 muteNow      = fields[0].GetUInt32();
        if (muteNow > muteTime)
            return;
    }

    LoginDatabase.EscapeString(reason);

    std::string playerName;

    if (!session)
        session = FindSession(accountId).get();

    if (session)
    {
        if (session->m_muteTime > muteTime)
            return;

        session->m_muteTime = muteTime;

        if (Player* player = session->GetPlayer())
        {
            playerName = player->GetName();
            ChatHandler(player).PSendSysMessage(LANG_YOUR_CHAT_DISABLED,  duration, reason.c_str());
        }
    }

    stmt->setInt64(0, muteTime);

    stmt->setUInt32(1, accountId);
    LoginDatabase.Execute(stmt);

    LoginDatabase.PQuery("INSERT INTO account_muted VALUES (%u, UNIX_TIMESTAMP(), UNIX_TIMESTAMP() + %u, '%s', '%s', 1)", accountId, (duration >= 0 ? duration * MINUTE : 4294967295), playerName.c_str(), reason.c_str());
  //  LoginDatabase.PQuery("UPDATE account SET mutetime = UNIX_TIMESTAMP() + %u WHERE id = %u", (duration >= 0 ? duration * MINUTE : 4294967295), accountId);
}

/// Update the game time
void World::_UpdateGameTime()
{
    ///- update the time
    time_t thisTime = time(nullptr);
    uint32 elapsed = uint32(thisTime - m_gameTime);
    m_gameTime = thisTime;

    ///- if there is a shutdown timer
    if (!IsStopped() && m_ShutdownTimer > 0 && elapsed > 0)
    {
        ///- ... and it is overdue, stop the world (set m_stopEvent)
        if (m_ShutdownTimer <= elapsed)
        {
            if (!(m_ShutdownMask & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount() == 0)
                m_stopEvent = true;                         // exist code already set
            else
                m_ShutdownTimer = 1;                        // minimum timer value to wait idle state
        }
        ///- ... else decrease it and if necessary display a shutdown countdown to the users
        else
        {
            m_ShutdownTimer -= elapsed;

            ShutdownMsg();
        }
    }
}

/// Shutdown the server
void World::ShutdownServ(uint32 time, uint32 options, uint8 exitcode)
{
    // ignore if server shutdown at next tick
    if (IsStopped())
        return;

    m_ShutdownMask = options;
    m_ExitCode = exitcode;

    ///- If the shutdown time is 0, set m_stopEvent (except if shutdown is 'idle' with remaining sessions)
    if (time == 0)
    {
        if (!(options & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount() == 0)
            m_stopEvent = true;                             // exist code already set
        else
            m_ShutdownTimer = 1;                            //So that the session count is re-evaluated at next world tick
    }
    ///- Else set the shutdown timer and warn users
    else
    {
        m_ShutdownTimer = time;
        ShutdownMsg(true);
    }

    sScriptMgr->OnShutdownInitiate(ShutdownExitCode(exitcode), ShutdownMask(options));
}

/// Display a shutdown message to the user(s)
void World::ShutdownMsg(bool show, Player* player)
{
    // not show messages for idle shutdown mode
    if (m_ShutdownMask & SHUTDOWN_MASK_IDLE)
        return;

    ///- Display a message every 12 hours, hours, 5 minutes, minute, 5 seconds and finally seconds
    if (show ||
        (m_ShutdownTimer < 5* MINUTE && (m_ShutdownTimer % 15) == 0) || // < 5 min; every 15 sec
        (m_ShutdownTimer < 15 * MINUTE && (m_ShutdownTimer % MINUTE) == 0) || // < 15 min ; every 1 min
        (m_ShutdownTimer < 30 * MINUTE && (m_ShutdownTimer % (5 * MINUTE)) == 0) || // < 30 min ; every 5 min
        (m_ShutdownTimer < 12 * HOUR && (m_ShutdownTimer % HOUR) == 0) || // < 12 h ; every 1 h
        (m_ShutdownTimer > 12 * HOUR && (m_ShutdownTimer % (12 * HOUR)) == 0)) // > 12 h ; every 12 h
    {
        std::string str = secsToTimeString(m_ShutdownTimer);

        ServerMessageType msgid = (m_ShutdownMask & SHUTDOWN_MASK_RESTART) ? SERVER_MSG_RESTART_TIME : SERVER_MSG_SHUTDOWN_TIME;

        SendServerMessage(msgid, str.c_str(), player);
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Server is %s in %s", (m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"), str.c_str());
    }
}

/// Cancel a planned server shutdown
void World::ShutdownCancel()
{
    // nothing cancel or too later
    if (!m_ShutdownTimer || m_stopEvent)
        return;

    ServerMessageType msgid = (m_ShutdownMask & SHUTDOWN_MASK_RESTART) ? SERVER_MSG_RESTART_CANCELLED : SERVER_MSG_SHUTDOWN_CANCELLED;

    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_ExitCode = SHUTDOWN_EXIT_CODE;                       // to default value
    SendServerMessage(msgid);

    TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Server %s cancelled.", (m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"));

    sScriptMgr->OnShutdownCancel();
}

void World::SendServerMessage(ServerMessageType type, char const* text, Player* player)
{
    WorldPackets::Chat::ChatServerMessage message;
    message.MessageID = type;
    message.StringParam = text;

    if (player)
        player->SendDirectMessage(message.Write());
    else
        SendGlobalMessage(message.Write());
}

void World::UpdateSessions(uint32 diff)
{
    std::pair<std::weak_ptr<WorldSocket>, uint64> linkInfo;
    while (_linkSocketQueue.next(linkInfo))
        ProcessLinkInstanceSocket(std::move(linkInfo));

    ///- Add new sessions
    WorldSessionPtr sess = nullptr;
    while (addSessQueue.next(sess))
        AddSession_(sess);

    ///- Then send an update signal to remaining ones
    for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
    {
        next = itr;
        ++next;

        ///- and remove not active sessions from the list
        WorldSessionPtr pSession = itr->second;
        if (!pSession)
        {
            m_sessions.erase(itr);
            continue;
        }
        if (pSession->GetMap()) // Not update session on map
            continue;

        if (!pSession->Update(diff))    // As interval = 0
        {
            uint32 accuntId = itr->second->GetAccountId();
            if (!RemoveQueuedPlayer(itr->second) && getIntConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
                m_disconnects[accuntId] = time(nullptr);
            RemoveQueuedPlayer(pSession);
            pSession->LogoutPlayer(true);
            m_sessions.erase(itr);
        }
    }
}

void World::setRate(Rates rate, float value)
{
    rate_values[rate] = value;
}

float World::getRate(Rates rate) const
{
    return rate_values[rate];
}

void World::setBoolConfig(WorldBoolConfigs index, bool value)
{
    if (index < BOOL_CONFIG_VALUE_COUNT)
        m_bool_configs[index] = value;
}

bool World::getBoolConfig(WorldBoolConfigs index) const
{
    return index < BOOL_CONFIG_VALUE_COUNT ? m_bool_configs[index] : 0;
}

void World::setFloatConfig(WorldFloatConfigs index, float value)
{
    if (index < FLOAT_CONFIG_VALUE_COUNT)
        m_float_configs[index] = value;
}

float World::getFloatConfig(WorldFloatConfigs index) const
{
    return index < FLOAT_CONFIG_VALUE_COUNT ? m_float_configs[index] : 0;
}

void World::setIntConfig(WorldIntConfigs index, uint32 value)
{
    if (index < INT_CONFIG_VALUE_COUNT)
        m_int_configs[index] = value;
}

uint32 World::getIntConfig(WorldIntConfigs index) const
{
    return index < INT_CONFIG_VALUE_COUNT ? m_int_configs[index] : 0;
}

// This handles the issued and queued CLI commands
void World::ProcessCliCommands()
{
    CliCommandHolder::Print* zprint = nullptr;
    void* callbackArg = nullptr;
    CliCommandHolder* command = nullptr;
    while (cliCmdQueue.next(command))
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "CLI command under processing...");
        zprint = command->m_print;
        callbackArg = command->m_callbackArg;
        CliHandler handler(callbackArg, zprint);
        handler.ParseCommands(command->m_command);
        if (command->m_commandFinished)
            command->m_commandFinished(callbackArg, !handler.HasSentErrorMessage());
        delete command;
    }
}

void World::SendAutoBroadcast()
{
    if (m_Autobroadcasts.empty())
        return;

    auto const& msg = Trinity::Containers::SelectRandomContainerElement(m_Autobroadcasts);

    uint32 abcenter = sWorld->getIntConfig(CONFIG_AUTOBROADCAST_CENTER);

    if (abcenter == 0)
        sWorld->SendWorldText(LANG_AUTO_BROADCAST, msg.c_str());

    else if (abcenter == 1)
        sWorld->SendGlobalMessage(WorldPackets::Chat::PrintNotification(msg).Write());
    else if (abcenter == 2)
    {
        sWorld->SendWorldText(LANG_AUTO_BROADCAST, msg.c_str());
        sWorld->SendGlobalMessage(WorldPackets::Chat::PrintNotification(msg).Write());
    }

    TC_LOG_DEBUG(LOG_FILTER_GENERAL, "AutoBroadcast: '%s'", msg.c_str());
}

void World::UpdateRealmCharCount(uint32 accountId)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_COUNT);
    stmt->setUInt32(0, accountId);
    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&World::_UpdateRealmCharCount, this, std::placeholders::_1)));
}

void World::_UpdateRealmCharCount(PreparedQueryResult resultCharCount)
{
    if (resultCharCount)
    {
        Field* fields = resultCharCount->Fetch();
        uint32 accountId = fields[0].GetUInt32();
        uint8 charCount = uint8(fields[1].GetUInt64());
        
        auto trans = LoginDatabase.BeginTransaction();

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_REALM_CHARACTERS_BY_REALM);
        stmt->setUInt32(0, accountId);
        stmt->setUInt32(1, realm.Id.Realm);
        trans->Append(stmt);

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
        stmt->setUInt8(0, charCount);
        stmt->setUInt32(1, accountId);
        stmt->setUInt32(2, realm.Id.Realm);
        trans->Append(stmt);

        LoginDatabase.CommitTransaction(trans);
    }
}

void World::InitWeeklyResetTime()
{
    time_t insttime = sWorld->getWorldState(WS_WEEKLY_RESET_TIME);

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_wday = 3; // Wednesday
    localTm.tm_hour = 4; // 4 Hours
    localTm.tm_min = 0; // 0 Minutes
    localTm.tm_sec = 0; // Secs

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += WEEK;

    // normalize reset time
    m_NextWeeklyReset = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_WEEKLY_RESET_TIME, m_NextWeeklyReset);

    if (sWorld->getWorldState(WS_CURRENT_ARTIFACT_KNOWLEDGE) < sWorld->getIntConfig(CONFIG_ARTIFACT_KNOWLEDGE_START))
        sWorld->setWorldState(WS_CURRENT_ARTIFACT_KNOWLEDGE, sWorld->getIntConfig(CONFIG_ARTIFACT_KNOWLEDGE_START));
}

void World::InitDailyQuestResetTime()
{
    time_t mostRecentQuestTime;

    QueryResult result = CharacterDatabase.Query("SELECT MAX(time) FROM character_queststatus_daily");
    if (result)
    {
        Field* fields = result->Fetch();
        mostRecentQuestTime = time_t(fields[0].GetUInt32());
    }
    else
        mostRecentQuestTime = 0;

    // client built-in time for reset is 6:00 AM
    // FIX ME: client not show day start time
    time_t curTime = time(nullptr);
    tm localTm;
    localtime_r(&curTime, &localTm);
    localTm.tm_hour = 6;
    localTm.tm_min  = 0;
    localTm.tm_sec  = 0;

    // current day reset time
    time_t curDayResetTime = mktime(&localTm);

    // last reset time before current moment
    time_t resetTime = (curTime < curDayResetTime) ? curDayResetTime - DAY : curDayResetTime;

    // need reset (if we have quest time before last reset time (not processed by some reason)
    if (mostRecentQuestTime && mostRecentQuestTime <= resetTime)
        m_NextDailyQuestReset = mostRecentQuestTime;
    else // plan next reset time
        m_NextDailyQuestReset = (curTime >= curDayResetTime) ? curDayResetTime + DAY : curDayResetTime;
}

void World::InitRandomBGResetTime()
{
    time_t bgtime = sWorld->getWorldState(WS_BG_DAILY_RESET_TIME);
    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm;
    localtime_r(&curTime, &localTm);
    localTm.tm_hour = getIntConfig(CONFIG_RANDOM_BG_RESET_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // current day reset time
    time_t nextDayResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextDayResetTime)
        nextDayResetTime += DAY;

    // normalize reset time
    m_NextRandomBGReset = bgtime ? bgtime : nextDayResetTime;

    if (!bgtime)
        sWorld->setWorldState(WS_BG_DAILY_RESET_TIME, m_NextRandomBGReset);
}

void World::InitCurrencyResetTime()
{
    time_t currencytime = sWorld->getWorldState(WS_CURRENCY_RESET_TIME);
    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm;
    localtime_r(&curTime, &localTm);
    localTm.tm_wday = getIntConfig(CONFIG_CURRENCY_RESET_DAY);
    localTm.tm_hour = getIntConfig(CONFIG_CURRENCY_RESET_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // current week reset time
    time_t nextWeekResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextWeekResetTime)
        nextWeekResetTime += getIntConfig(CONFIG_CURRENCY_RESET_INTERVAL) * DAY;

    // normalize reset time
    m_NextCurrencyReset = currencytime ? currencytime : nextWeekResetTime;

    if (!currencytime)
        sWorld->setWorldState(WS_CURRENCY_RESET_TIME, m_NextCurrencyReset);
}

void World::InitInstanceDailyResetTime()
{
    time_t insttime = sWorld->getWorldState(WS_INSTANCE_DAILY_RESET_TIME);
    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_hour = getIntConfig(CONFIG_INSTANCE_RESET_TIME_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += getIntConfig(CONFIG_INSTANCE_DAILY_RESET) * DAY;

    // normalize reset time
    m_NextInstanceDailyReset = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_INSTANCE_DAILY_RESET_TIME, m_NextInstanceDailyReset);
}

void World::InitInstanceWeeklyResetTime()
{
    time_t insttime = sWorld->getWorldState(WS_INSTANCE_WEEKLY_RESET_TIME);

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_wday = 3;
    localTm.tm_hour = getIntConfig(CONFIG_INSTANCE_RESET_TIME_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += getIntConfig(CONFIG_INSTANCE_WEEKLY_RESET) * DAY;

    // normalize reset time
    m_NextInstanceWeeklyReset = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_INSTANCE_WEEKLY_RESET_TIME, m_NextInstanceWeeklyReset);
}

void World::InitChallengeKeyResetTime()
{
    time_t insttime = sWorld->getWorldState(WS_CHALLENGE_KEY_RESET_TIME);

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_wday = 3;
    localTm.tm_hour = getIntConfig(CONFIG_INSTANCE_RESET_TIME_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += getIntConfig(CONFIG_CHALLENGE_KEY_RESET) * DAY;

    // normalize reset time
    m_NextChallengeKeyReset = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_CHALLENGE_KEY_RESET_TIME, m_NextChallengeKeyReset);

    if (!sWorld->getWorldState(WS_CHALLENGE_LAST_RESET_TIME))
        sWorld->setWorldState(WS_CHALLENGE_LAST_RESET_TIME, m_NextChallengeKeyReset - (7 * DAY));
}

void World::InitWorldQuestHourlyResetTime()
{
    time_t insttime = sWorld->getWorldState(WS_WORLDQUEST_HOURLY_RESET_TIME);

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_hour = getIntConfig(CONFIG_WORLD_QUEST_RESET_TIME_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += getIntConfig(CONFIG_WORLD_QUEST_HOURLY_RESET) * HOUR;

    // normalize reset time
    m_NextWorldQuestHourlyReset = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_WORLDQUEST_HOURLY_RESET_TIME, m_NextWorldQuestHourlyReset);
}

void World::InitInvasionPointResetTime()
{
    time_t insttime = sWorld->getWorldState(WS_INVASION_POINT_RESET_TIME);

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_hour = getIntConfig(CONFIG_WORLD_QUEST_RESET_TIME_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += getIntConfig(CONFIG_INVASION_POINT_RESET) * HOUR;

    // normalize reset time
    m_NextInvasionPointReset = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_INVASION_POINT_RESET_TIME, m_NextInvasionPointReset);
}

void World::InitWorldQuestDailyResetTime()
{
    time_t insttime = sWorld->getWorldState(WS_WORLDQUEST_DAILY_RESET_TIME);

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_hour = getIntConfig(CONFIG_WORLD_QUEST_RESET_TIME_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += getIntConfig(CONFIG_WORLD_QUEST_DAILY_RESET) * DAY;

    // normalize reset time
    m_NextWorldQuestDailyReset = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_WORLDQUEST_DAILY_RESET_TIME, m_NextWorldQuestDailyReset);
}

void World::InitBanWaveTime()
{
    time_t insttime = sWorld->getWorldState(WS_BAN_WAVE_TIME);

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_wday = 5;
    localTm.tm_hour = 4;
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // Daily reset time
    time_t nextResetTime = mktime(&localTm);

    // next reset time before current moment
    while (curTime >= nextResetTime)
        nextResetTime += 2 * DAY;

    // normalize reset time
    m_NextBanWaveTime = insttime ? insttime : nextResetTime;

    if (!insttime)
        sWorld->setWorldState(WS_BAN_WAVE_TIME, m_NextBanWaveTime);
}

time_t World::getInstanceResetTime(uint32 resetTime)
{
    switch(resetTime)
    {
        case 1:
            return sWorld->getWorldState(WS_INSTANCE_DAILY_RESET_TIME);
        case 2:
            return sWorld->getWorldState(WS_INSTANCE_WEEKLY_RESET_TIME);
        case 3:
            return sWorld->getWorldState(WS_WEEKLY_RESET_TIME);
        default:
            return time(nullptr) + 2 * HOUR;
    }
}

time_t World::getNextInstanceDailyReset()
{
    return m_NextInstanceDailyReset;
}

time_t World::getNextInstanceWeeklyReset()
{
    return m_NextInstanceWeeklyReset;
}

time_t World::getNextChallengeKeyReset()
{
    return m_NextChallengeKeyReset;
}

void World::ResetDailyQuests()
{
    TC_LOG_INFO(LOG_FILTER_GENERAL, "Daily quests reset for all characters.");

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_QUEST_STATUS_DAILY);
    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_GARRISON_FOLLOWER_ACTIVATIONS);
    stmt->setUInt32(0, 1);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (Player* player = itr->second->GetPlayer())
            player->AddDelayedEvent(100, [player]() -> void { player->DailyReset(); });

    // change available dailies
    sPoolMgr->ChangeDailyQuests();
}

void World::ResetCurrencyWeekCap()
{
    CharacterDatabase.Execute("UPDATE `character_currency` SET `week_count` = 0");

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (Player* player = itr->second->GetPlayer())
            player->AddDelayedEvent(100, [player]() -> void { player->ResetCurrencyWeekCap(); });

    m_NextCurrencyReset = time_t(m_NextCurrencyReset + DAY * getIntConfig(CONFIG_CURRENCY_RESET_INTERVAL));
    sWorld->setWorldState(WS_CURRENCY_RESET_TIME, m_NextCurrencyReset);
}

void World::InstanceDailyResetTime()
{
    time_t curTime = time(nullptr);
    m_NextInstanceDailyReset = time_t(m_NextInstanceDailyReset + DAY * getIntConfig(CONFIG_INSTANCE_DAILY_RESET));

    while (curTime >= m_NextInstanceDailyReset)
        m_NextInstanceDailyReset += getIntConfig(CONFIG_INSTANCE_DAILY_RESET) * DAY;

    sWorld->setWorldState(WS_INSTANCE_DAILY_RESET_TIME, m_NextInstanceDailyReset);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    for (auto& mapDifficultyPair : sDB2Manager.GetAllMapsDifficultyes())
    {
        for (auto& difficultyPair : mapDifficultyPair.second)
        {
            uint32 mapid = mapDifficultyPair.first;
            Difficulty difficulty = static_cast<Difficulty>(difficultyPair.first);

            MapEntry const* i_map = sMapStore.LookupEntry(mapid);
            if (!i_map)
                continue;

            if (i_map->IsNonRaidDungeon())
                sInstanceSaveMgr->ResetOrWarnAll(mapid, difficulty, trans);
        }
    }
    CharacterDatabase.CommitTransaction(trans);
    CharacterDatabase.WaitExecution();

    if (sWorld->getBoolConfig(CONFIG_WORLD_QUEST))
        sQuestDataStore->GenerateWorldQuestUpdate();
}

void World::InstanceWeeklyResetTime()
{
    time_t curTime = time(nullptr);
    m_NextInstanceWeeklyReset = time_t(m_NextInstanceWeeklyReset + DAY * getIntConfig(CONFIG_INSTANCE_WEEKLY_RESET));

    while (curTime >= m_NextInstanceWeeklyReset)
        m_NextInstanceWeeklyReset += getIntConfig(CONFIG_INSTANCE_WEEKLY_RESET) * DAY;

    sWorld->setWorldState(WS_INSTANCE_WEEKLY_RESET_TIME, m_NextInstanceWeeklyReset);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    for (auto& mapDifficultyPair : sDB2Manager.GetAllMapsDifficultyes())
    {
        for (auto& difficultyPair : mapDifficultyPair.second)
        {
            uint32 mapid = mapDifficultyPair.first;
            MapEntry const* i_map = sMapStore.LookupEntry(mapid);
            if (!i_map)
                continue;

            if (i_map->IsRaid())
                sInstanceSaveMgr->ResetOrWarnAll(mapid, static_cast<Difficulty>(difficultyPair.first), trans);
        }
    }
    CharacterDatabase.CommitTransaction(trans);
    CharacterDatabase.WaitExecution();
}

void World::ChallengeKeyResetTime()
{
    sChallengeMgr->GenerateCurrentWeekAffixes();
    sChallengeMgr->GenerateOploteLoot();

    CharacterDatabase.PQuery("DELETE FROM challenge_key WHERE timeReset < %u", m_NextChallengeKeyReset);
    CharacterDatabase.Query("DELETE FROM item_instance WHERE itemEntry = 138019");

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (Player* player = itr->second->GetPlayer())
                player->AddDelayedEvent(100, [player]() -> void { player->ResetChallengeKey(); });

    time_t curTime = time(nullptr);
    time_t m_LastChallengeKeyReset = m_NextChallengeKeyReset;
    m_NextChallengeKeyReset = time_t(m_NextChallengeKeyReset + DAY * getIntConfig(CONFIG_CHALLENGE_KEY_RESET));

    while (curTime >= m_NextChallengeKeyReset)
        m_NextChallengeKeyReset += getIntConfig(CONFIG_CHALLENGE_KEY_RESET) * DAY;

    sWorld->setWorldState(WS_CHALLENGE_KEY_RESET_TIME, m_NextChallengeKeyReset);
    sWorld->setWorldState(WS_CHALLENGE_LAST_RESET_TIME, m_LastChallengeKeyReset);
}

void World::WorldQuestHourlyResetTime()
{
    time_t curTime = time(nullptr);
    m_NextWorldQuestHourlyReset = time_t(m_NextWorldQuestHourlyReset + HOUR * getIntConfig(CONFIG_WORLD_QUEST_HOURLY_RESET));

    while (curTime >= m_NextWorldQuestHourlyReset)
        m_NextWorldQuestHourlyReset += getIntConfig(CONFIG_WORLD_QUEST_HOURLY_RESET) * HOUR;

    sWorld->setWorldState(WS_WORLDQUEST_HOURLY_RESET_TIME, m_NextWorldQuestHourlyReset);

    if (sWorld->getBoolConfig(CONFIG_WORLD_QUEST))
        sQuestDataStore->GenerateWorldQuestUpdate();
}

void World::InvasionPointResetTime()
{
    time_t curTime = time(nullptr);
    m_NextInvasionPointReset = time_t(m_NextInvasionPointReset + HOUR * getIntConfig(CONFIG_INVASION_POINT_RESET));

    while (curTime >= m_NextInvasionPointReset)
        m_NextInvasionPointReset += getIntConfig(CONFIG_INVASION_POINT_RESET) * HOUR;

    sWorld->setWorldState(WS_INVASION_POINT_RESET_TIME, m_NextInvasionPointReset);

    if (sWorld->getBoolConfig(CONFIG_WORLD_QUEST))
        sQuestDataStore->GenerateInvasionPointUpdate();
}

void World::WorldQuestDailyResetTime()
{
    time_t curTime = time(nullptr);
    m_NextWorldQuestDailyReset = time_t(m_NextWorldQuestDailyReset + DAY * getIntConfig(CONFIG_WORLD_QUEST_DAILY_RESET));

    while (curTime >= m_NextWorldQuestDailyReset)
        m_NextWorldQuestDailyReset += getIntConfig(CONFIG_WORLD_QUEST_DAILY_RESET) * DAY;

    sWorld->setWorldState(WS_WORLDQUEST_DAILY_RESET_TIME, m_NextWorldQuestDailyReset);
}

void World::ResetLootCooldown()
{
    CharacterDatabase.Execute("DELETE FROM character_loot_cooldown WHERE respawnTime != 0");
    CharacterDatabase.Execute("DELETE FROM character_lfg_cooldown WHERE respawnTime != 0");

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (auto player = itr->second->GetPlayer())
            player->AddDelayedEvent(100, [player]() -> void { player->ResetLootCooldown(); });
}

void World::StartBanWave()
{
    BanFlaggedAccounts();

    time_t curTime = time(nullptr);
    uint32 periodDays = urand(1, 3);
    m_NextBanWaveTime = time_t(m_NextBanWaveTime + DAY * periodDays);

    while (curTime >= m_NextBanWaveTime)
        m_NextBanWaveTime += periodDays * DAY;

    sWorld->setWorldState(WS_BAN_WAVE_TIME, m_NextBanWaveTime);
}

void World::LoadDBAllowedSecurityLevel()
{
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_REALMLIST_SECURITY_LEVEL);
    stmt->setInt32(0, int32(realm.Id.Realm));
    if (PreparedQueryResult result = LoginDatabase.Query(stmt))
        SetPlayerSecurityLimit(AccountTypes(uint8(result->Fetch()->GetUInt8())));
}

void World::SetPlayerSecurityLimit(AccountTypes _sec)
{
    AccountTypes sec = _sec < SEC_CONSOLE ? _sec : SEC_PLAYER;
    bool update = sec > m_allowedSecurityLevel;
    m_allowedSecurityLevel = sec;
    if (update)
        KickAllLess(m_allowedSecurityLevel);
}

void World::ResetWeekly()
{
    CharacterDatabase.Execute(CharacterDatabase.GetPreparedStatement(CHAR_DEL_QUEST_STATUS_WEEKLY));
    CharacterDatabase.Execute(CharacterDatabase.GetPreparedStatement(CHAR_UPD_WEEKLY_BRACKET));

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (auto player = itr->second->GetPlayer())
            player->AddDelayedEvent(100, [player]() -> void { if (player) player->ResetWeeklyQuestStatus(); });

    sBracketMgr->ResetWeekly();

    time_t curTime = time(nullptr);
    m_NextWeeklyReset = time_t(m_NextWeeklyReset + WEEK);

    while (curTime >= m_NextWeeklyReset)
        m_NextWeeklyReset += WEEK;

    sWorld->setWorldState(WS_WEEKLY_RESET_TIME, m_NextWeeklyReset);

    // change available weeklies
    sPoolMgr->ChangeWeeklyQuests();

    uint32 newAK = sWorld->getWorldState(WS_CURRENT_ARTIFACT_KNOWLEDGE) + 1;
    if (newAK <= sWorld->getIntConfig(CONFIG_ARTIFACT_KNOWLEDGE_CAP))
        sWorld->setWorldState(WS_CURRENT_ARTIFACT_KNOWLEDGE, newAK);
}

void World::ResetEventSeasonalQuests(uint16 event_id)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_QUEST_STATUS_SEASONAL);
    stmt->setUInt16(0,event_id);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (auto player = itr->second->GetPlayer())
            player->AddDelayedEvent(100, [player, event_id]() -> void { if (player) player->ResetSeasonalQuestStatus(event_id); });
}

std::string World::GetRealmName()
{
    return m_realmName;
}

std::string World::GetTrimmedRealmName()
{
    return m_trimmedRealmName;
}

void World::ResetRandomBG()
{
    TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Random BG status reset for all characters.");

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_BATTLEGROUND_RANDOM);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (auto player = itr->second->GetPlayer())
            player->AddDelayedEvent(100, [player]() -> void { player->SetWinToday(false); });

    m_NextRandomBGReset = time_t(m_NextRandomBGReset + DAY);
    sWorld->setWorldState(WS_BG_DAILY_RESET_TIME, m_NextRandomBGReset);
}

void World::UpdateMaxSessionCounters()
{
    m_maxActiveSessionCount = std::max(m_maxActiveSessionCount, uint32(m_sessions.size()-m_QueuedPlayer.size()));
    m_maxQueuedSessionCount = std::max(m_maxQueuedSessionCount, uint32(m_QueuedPlayer.size()));
}

const SessionMap& World::GetAllSessions() const
{
    return m_sessions;
}

uint32 World::GetActiveAndQueuedSessionCount() const
{
    if (m_sessionCount > 50)
        return m_sessionCount * getRate(RATE_ONLINE);
    return m_sessionCount;
}

uint32 World::GetActiveSessionCount() const
{
    if ((m_sessionCount - m_QueuedPlayer.size()) > 50)
        return (m_sessionCount - m_QueuedPlayer.size()) * getRate(RATE_ONLINE);
    return (m_sessionCount - m_QueuedPlayer.size());
}

uint32 World::GetActiveSessionCountDiff() const
{
    return (m_sessionCount - m_QueuedPlayer.size());
}

uint32 World::GetQueuedSessionCount() const
{
    return m_QueuedPlayer.size();
}

uint32 World::GetMaxQueuedSessionCount() const
{
    return m_maxQueuedSessionCount;
}

uint32 World::GetMaxActiveSessionCount() const
{
    if (m_sessionCount > 50)
        return m_sessionCount * getRate(RATE_ONLINE);
    return m_sessionCount;
}

uint32 World::GetPlayerCount() const
{
    return m_PlayerCount;
}

uint32 World::GetMaxPlayerCount() const
{
    return m_MaxPlayerCount;
}

void World::IncreasePlayerCount()
{
    m_PlayerCount++;
    m_MaxPlayerCount = std::max(m_MaxPlayerCount, m_PlayerCount);
}

void World::DecreasePlayerCount()
{
    m_PlayerCount--;
}

uint32 World::GetSessionCount() const
{
    return m_sessionCount;
}

uint32 World::GetMaxSessionCount() const
{
    return m_maxSessionCount;
}

void World::IncreaseSessionCount()
{
    m_sessionCount++;
    m_maxSessionCount = std::max(m_maxSessionCount, m_sessionCount);
}

void World::DecreaseSessionCount()
{
    m_sessionCount--;
}

void World::LoadDBVersion()
{
    if (QueryResult result = WorldDatabase.Query("SELECT db_version, cache_id, hotfix_cache_id FROM version LIMIT 1"))
    {
        Field* fields = result->Fetch();

        m_DBVersion = fields[0].GetString();

        m_int_configs[CONFIG_CLIENTCACHE_VERSION] = fields[1].GetUInt32();
        m_int_configs[CONFIG_HOTFIX_CACHE_VERSION] = fields[2].GetUInt32();
    }

    if (m_DBVersion.empty())
        m_DBVersion = "Unknown world database.";
}

void World::ProcessStartEvent()
{
    isEventKillStart = true;
}

void World::ProcessStopEvent()
{
    isEventKillStart = false;
}

void World::UpdateAreaDependentAuras()
{
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second && itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())
        {
            itr->second->GetPlayer()->UpdateAreaDependentAuras(itr->second->GetPlayer()->GetCurrentAreaID());
            itr->second->GetPlayer()->UpdateZoneDependentAuras(itr->second->GetPlayer()->GetCurrentZoneID());
        }
}

void World::LoadWorldStates()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT entry, value FROM worldstates");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 world states. DB table `worldstates` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        m_worldstates[fields[0].GetUInt32()] = fields[1].GetUInt32();
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u world states in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

bool World::IsPvPRealm() const
{
    return (getIntConfig(CONFIG_GAME_TYPE) == REALM_TYPE_PVP || getIntConfig(CONFIG_GAME_TYPE) == REALM_TYPE_RPPVP ||
        getIntConfig(CONFIG_GAME_TYPE) == REALM_TYPE_FFA_PVP);
}

bool World::IsFFAPvPRealm() const
{
    return getIntConfig(CONFIG_GAME_TYPE) == REALM_TYPE_FFA_PVP;
}

// Setting a worldstate will save it to DB
void World::setWorldState(uint32 index, uint32 value)
{
    WorldStatesMap::const_iterator it = m_worldstates.find(index);
    if (it != m_worldstates.end())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_WORLDSTATE);

        stmt->setUInt32(0, uint32(value));
        stmt->setUInt32(1, index);

        CharacterDatabase.Execute(stmt);
    }
    else
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_WORLDSTATE);

        stmt->setUInt32(0, index);
        stmt->setUInt32(1, uint32(value));

        CharacterDatabase.Execute(stmt);
    }
    m_worldstates[index] = value;
}

uint32 World::getWorldState(uint32 index) const
{
    WorldStatesMap::const_iterator it = m_worldstates.find(index);
    return it != m_worldstates.end() ? it->second : 0;
}

void World::ProcessQueryCallbacks()
{
    _queryProcessor.ProcessReadyQueries();
}

void World::LoadCharacterNameData()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading character name data");

    QueryResult result = CharacterDatabase.Query("SELECT c.guid, c.name, c.race, c.gender, c.class, c.level, c.account, c.zone, gm.rank, gm.guildid, c.specialization FROM  characters AS c LEFT JOIN guild_member AS gm ON c.guid = gm.guid WHERE deleteDate IS NULL OR deleteDate = 0");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "No character name data loaded, empty query");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        AddCharacterInfo(fields[0].GetUInt64(), fields[1].GetString(),
            fields[3].GetUInt8() /*gender*/, fields[2].GetUInt8() /*race*/, fields[4].GetUInt8() /*class*/, fields[5].GetUInt8() /*level*/,
            fields[6].GetUInt32() /*account*/, fields[7].GetUInt16() /*zoneId*/, fields[8].GetUInt32() /*guildId*/, fields[9].GetUInt32() /*rankId*/, fields[10].GetUInt16() /*SpecId*/);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loaded name data for %u characters", count);
}

void World::AddCharacterInfo(ObjectGuid::LowType guid, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level, uint32 accountId, uint8 zoneId, uint8 rankId, uint32 guildId, uint32 specid)
{
    if (_characterInfoStore.size() <= guid)
        _characterInfoStore.resize(guid + 10000, nullptr);

    CharacterInfo* data = new CharacterInfo;
    data->Guid = guid;
    data->Name = name;
    data->Race = race;
    data->Sex = gender;
    data->Class = playerClass;
    data->Level = level;
    data->AccountId = accountId;
    data->ZoneId = zoneId;
    data->RankId = rankId;
    data->GuildId = guildId;
    data->SpecId = specid;

    _characterInfoStore[guid] = data;
    nameMap[name] = data;
}

void World::UpdateCharacterInfo(ObjectGuid const& guid, std::string const& name, uint8 gender /*= GENDER_NONE*/, uint8 race /*= RACE_NONE*/)
{
    if (_characterInfoStore.size() <= guid.GetGUIDLow())
        return;

    CharacterInfo* info = _characterInfoStore[guid.GetGUIDLow()];
    if (!info)
        return;

    DeleteCharName(info->Name);
    info->Name = name;

    if (gender != GENDER_NONE)
        info->Sex = gender;

    if (race != RACE_NONE)
        info->Race = race;

    AddCharacterName(name, info);
    SendGlobalMessage(WorldPackets::Misc::InvalidatePlayer(guid).Write());
}

void World::UpdateCharacterAccount(uint32 guid, uint32 BnetAccountId)
{
    if (_characterInfoStore.size() <= guid)
        return;

    CharacterInfo* info = _characterInfoStore[guid];
    if (!info)
        return;

    info->BnetAccountId = BnetAccountId;
}

void World::UpdateCharacterInfoLevel(ObjectGuid::LowType guid, uint8 level)
{
    if (_characterInfoStore.size() <= guid)
        return;

    CharacterInfo* info = _characterInfoStore[guid];
    if (!info)
        return;

    info->Level = level;
}

void World::UpdateCharacterInfoDeleted(ObjectGuid::LowType guid, bool deleted, std::string const* name /*= nullptr*/)
{
    auto info = _characterInfoStore[guid];
    if (!info)
        return;

    info->IsDeleted = deleted;

    if (name)
        info->Name = *name;
}

void World::UpdateCharacterNameDataZoneGuild(ObjectGuid::LowType guid, uint16 zoneId, uint16 guildId, uint8 rankId)
{
    if (_characterInfoStore.size() <= guid)
        return;

    CharacterInfo* info = _characterInfoStore[guid];
    if (!info)
        return;

    info->ZoneId = zoneId;
    info->RankId = rankId;
    info->GuildId = guildId;
}

uint32 World::GetCleaningFlags() const
{
    return m_CleaningFlags;
}

void World::SetCleaningFlags(uint32 flags)
{
    m_CleaningFlags = flags;
}

CharacterInfo const* World::GetCharacterInfo(ObjectGuid const& guid) const
{
    if (_characterInfoStore.size() <= guid.GetGUIDLow())
        return nullptr;

    return _characterInfoStore[guid.GetGUIDLow()];
}

CharacterInfo const* World::GetCharacterInfo(std::string name) const
{
    return Trinity::Containers::MapGetValuePtr(nameMap, name);
}

void World::DeleteCharacterNameData(ObjectGuid::LowType guid)
{
    if (_characterInfoStore.size() <= guid)
        return;

    CharacterInfo* info = _characterInfoStore[guid];
    if (!info)
        return;

    info->IsDeleted = true;

    nameMap.erase(info->Name);
    _characterInfoStore[guid] = nullptr;
    delete info;
}

void World::UpdatePhaseDefinitions()
{
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)	
        if (itr->second && itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())	
            itr->second->GetPlayer()->GetPhaseMgr().NotifyStoresReloaded();	
}

bool World::CheckCharacterName(std::string name)
{
    if (nameMap.find(name) != nameMap.end())
        return false;

    return true;
}

void World::AddCharacterName(std::string name, CharacterInfo* nameData)
{
    nameMap[name] = nameData;
}

void World::DeleteCharName(std::string name)
{
    nameMap.erase(name);
}

void World::ProcessMailboxQueue()
{
    //QueryResult result = CharacterDatabase.Query("SELECT id FROM mailbox_queue LIMIT 10000");
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_MAILBOX_QUEUE);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if(result)
    {
        do
        {
            Field* f = result->Fetch();

            time_t deliver_time = m_gameTime;
            time_t expire_time = deliver_time + 30 * DAY;

            uint32 i = 0;
            uint32 id = f[i++].GetUInt32();
            uint32 messageType = f[i++].GetUInt32();
            uint32 stationery = f[i++].GetUInt32();
            uint64 sender_guid = f[i++].GetUInt64();
            ObjectGuid receiver_guid = ObjectGuid::Create<HighGuid::Player>(f[i++].GetUInt64());
            std::string subject = f[i++].GetCString();
            std::string body = f[i++].GetCString();
            uint32 money = f[i++].GetUInt32();
            uint32 itemid = f[i++].GetUInt32();
            uint32 itemcount = f[i++].GetUInt32();

            if(itemid != 0 && itemid != 49426 && itemid != 38186 && itemid != 62062 && itemid != 47241 && money)
                money = 0;

            ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(itemid);
            if(itemid && !pProto)
                continue;
            
            uint8 countmails = 1;
            uint32 stack = pProto ? pProto->GetMaxStackSize() : uint32(0x7FFFFFFF-1);
            if (itemid != 0)
            {
                if (itemcount > MAX_MAIL_ITEMS*stack)
                {
                    uint32 tempitemcount = itemcount;
                    while (tempitemcount > MAX_MAIL_ITEMS*stack)
                    {
                        countmails++;
                        tempitemcount -= MAX_MAIL_ITEMS*stack;
                    }
                }
            }
            for (uint8 j = 0; j < countmails; ++j)
            {
                uint32 mailId = sObjectMgr->GenerateMailID();
                Player * receiver = ObjectAccessor::FindPlayer(receiver_guid);
                std::vector<Item*> ItemsOnMail;
                ItemsOnMail.clear();
                
                uint32 countitems = 0;
                if (itemid != 0)
                {
                    if (itemcount <= MAX_MAIL_ITEMS*stack)
                        countitems = itemcount;
                    else
                    {
                        countitems = MAX_MAIL_ITEMS*stack;
                        itemcount -= MAX_MAIL_ITEMS*stack;
                    }
                }
                
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                uint32 tempcountitems = countitems;
                while (countitems && itemid)
                {
                    Item *attachment = nullptr;

                    if(itemid != 0)
                    {
                        if (countitems > stack)
                        {
                            attachment = Item::CreateItem ( itemid, stack, receiver );
                            countitems -= stack;
                        }
                        else
                        {
                            attachment = Item::CreateItem ( itemid, countitems, receiver );
                            countitems = 0;
                        }
                        
                        if(attachment)
                        {
                            attachment->SaveToDB(trans);
                            ItemsOnMail.push_back(attachment);
                        }
                    }
                 
                    if(attachment != nullptr)
                        trans->PAppend("REPLACE INTO mail_items (mail_id,item_guid,receiver) VALUES ('%u', '%u', '%u')", mailId, attachment->GetGUIDLow(), receiver_guid.GetGUIDLow());

                    TC_LOG_ERROR(LOG_FILTER_REMOTECOMMAND,"Sending mail to %u, Item:%u , ItemCount:%u, %s", receiver_guid.GetGUIDLow(), itemid,itemcount,body.c_str());

                }
                
                // Add to DB
                CharacterDatabase.EscapeString(subject);
                CharacterDatabase.EscapeString(body);
                // trans->PAppend("REPLACE INTO mail (id,messageType,stationery,mailTemplateId,sender,receiver,subject,body,has_items,expire_time,deliver_time,money,cod,checked) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%s', '%s', '%u', '" UI64FMTD "','" UI64FMTD "', '%u', '%u', '%d')",
                //  mailId, messageType, stationery, 0, sender_guid, receiver_guid.GetGUIDLow(), subject.c_str(), body.c_str(), (attachment != nullptr ? 1 : 0), (uint64)expire_time, (uint64)deliver_time, money, 0, 0);
                PreparedStatement* stmtt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL);
                uint8 index = 0;
                stmtt->setUInt32(index++, mailId);
                stmtt->setUInt8 (index++, uint8(messageType));
                stmtt->setInt8  (index++, int8(stationery));
                stmtt->setUInt16(index++, 0);
                stmtt->setUInt64(index++, sender_guid);
                stmtt->setUInt64(index++, receiver_guid.GetGUIDLow());
                stmtt->setString(index++, subject);
                stmtt->setString(index++, body);
                stmtt->setBool  (index++, tempcountitems);
                stmtt->setUInt64(index++, uint64(expire_time));
                stmtt->setUInt64(index++, uint64(deliver_time));
                stmtt->setUInt64(index++, money);
                stmtt->setUInt64(index++, 0);
                stmtt->setUInt8 (index, uint8(0));
                trans->Append(stmtt);
                
                CharacterDatabase.CommitTransaction(trans);

                // For online receiver update in game mail status and data
                if (receiver)
                {
                    receiver->AddNewMailDeliverTime(deliver_time);

                    if (receiver->IsMailsLoaded())
                    {
                        Mail *m = new Mail;
                        m->messageID = mailId;
                        m->messageType = messageType;
                        m->stationery = stationery;
                        m->mailTemplateId = 0;
                        m->sender = sender_guid;
                        m->receiver = receiver->GetGUIDLow();
                        m->subject = subject;
                        m->body = body;

                        if(!ItemsOnMail.empty())
                            for (Item* pitem : ItemsOnMail)
                                m->AddItem(pitem->GetGUIDLow(), pitem->GetEntry());

                        m->expire_time = expire_time;
                        m->deliver_time = deliver_time;
                        m->money = money;
                        m->COD = 0;
                        m->checked = 0;
                        m->state = MAIL_STATE_UNCHANGED;

                        receiver->AddMail(m);                           // to insert new mail to beginning of maillist

                        if(!ItemsOnMail.empty())
                            for (Item* pitem : ItemsOnMail)
                                receiver->AddMItem(pitem);
                    }
                    else if(!ItemsOnMail.empty())
                            for (Item* pitem : ItemsOnMail)
                                delete pitem;
                }
                else if(!ItemsOnMail.empty())
                            for (Item* pitem : ItemsOnMail)
                                delete pitem;

                //CharacterDatabase.PExecute("DELETE FROM mailbox_queue WHERE id = '%u'", id);
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAILBOX_QUEUE);
                stmt->setUInt32(0, id);
                CharacterDatabase.Execute(stmt);
            }

        } while (result->NextRow());

        TC_LOG_DEBUG(LOG_FILTER_WORLDSERVER, "MailboxQueue: Complete");
    }
    else
        TC_LOG_DEBUG(LOG_FILTER_WORLDSERVER, "MailboxQueue: No pending mail");
}

void World::InitServerAutoRestartTime()
{
    time_t serverRestartTime = sWorld->getWorldState(WS_AUTO_SERVER_RESTART_TIME);
    if (!serverRestartTime)
        m_NextServerRestart = time_t(time(nullptr));         // game time not yet init

    // generate time by config
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_hour = getIntConfig(CONFIG_AUTO_SERVER_RESTART_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // current day reset time
    time_t nextDayRestartTime = mktime(&localTm);

    // next reset time before current moment
    if (curTime >= nextDayRestartTime)
        nextDayRestartTime += DAY;

    // normalize reset time
    m_NextServerRestart = serverRestartTime < curTime ? nextDayRestartTime - DAY : nextDayRestartTime;

    if (!serverRestartTime)
        sWorld->setWorldState(WS_AUTO_SERVER_RESTART_TIME, m_NextServerRestart);

    if (!m_bool_configs[CONFIG_DISABLE_RESTART])
        m_NextServerRestart += DAY*1;
}

void World::AutoRestartServer()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Automatic server restart 60s.");

    sWorld->ShutdownServ(60, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE);

    m_NextServerRestart = time_t(m_NextServerRestart + DAY*1);
    sWorld->setWorldState(WS_AUTO_SERVER_RESTART_TIME, m_NextServerRestart);
}

void World::Transfer()
{
    //state 0 waiting to dump on source server
    //state 1 load dump char to dest server
    //state 2 error to many char on acc
    //state 3 dump char to waiting buy transfer
    //state 4 waiting buy transfer
    QueryResult toDump = LoginDatabase.PQuery("SELECT `id`, `account`, `perso_guid`, `to`, `state` FROM transferts WHERE `from` = %u AND state IN (0,3)", realm.Id.Realm);
    QueryResult toLoad = LoginDatabase.PQuery("SELECT `id`, `account`, `perso_guid`, `from`, `dump`, `toacc`, `transferId` FROM transferts WHERE `to` = %u AND state = 1", realm.Id.Realm);

    TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "World::Transfer()");

    if(toDump)
    {
        do
        {
            Field* field = toDump->Fetch();
            uint32 transaction = field[0].GetUInt32();
            ObjectGuid::LowType guid = field[2].GetUInt64();
            uint32 to = field[3].GetUInt32();
            uint32 state = field[4].GetUInt32();

            if (Player * pPlayer = sObjectMgr->GetPlayerByLowGUID(guid))
            {
                pPlayer->GetSession()->SendNotification("You must logout to transfer your char");
                continue;
            }

            CharacterDatabase.PExecute("DELETE FROM `group_member` WHERE `memberGuid` = '%u'",  guid);
            CharacterDatabase.PExecute("DELETE FROM `guild_member` WHERE `guid` = '%u'",        guid);

            std::string dump;
            DumpReturn dumpState = PlayerDumpWriter().WriteDump(guid, dump);

            TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Transfer toDump guid %u, dump %u", guid, dumpState);

            if (dumpState == DUMP_SUCCESS)
            {
                CharacterDatabase.PExecute("UPDATE `characters` SET `at_login` = '512', `deleteInfos_Name` = `name`, `deleteInfos_Account` = `account`, `deleteDate` ='" UI64FMTD "', `name` = '', `account` = 0, `transfer` = '%u' WHERE `guid` = '" UI64FMTD "'", uint64(time(nullptr)), to, guid);
                PreparedStatement * stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_DUMP);
                if(stmt)
                {
                    stmt->setString(0, dump);
                    if(state == 3)
                        stmt->setUInt32(1, 4);
                    else
                        stmt->setUInt32(1, 1);
                    stmt->setUInt32(2, transaction);
                    LoginDatabase.Execute(stmt);
                }
            }
            else
            {
                LoginDatabase.PQuery("UPDATE `transferts` SET `error` = %u WHERE `id` = '%u'", dumpState, transaction);
                continue;
            }
        }
        while(toDump->NextRow());
    }

    if(toLoad)
    {
        do
        {
            Field* field = toLoad->Fetch();
            uint32 transaction = field[0].GetUInt32();
            uint32 account = field[1].GetUInt32();
            ObjectGuid::LowType guid = field[2].GetUInt64();
            uint32 from = field[3].GetUInt32();
            std::string dump = field[4].GetString();
            uint32 toacc = field[5].GetUInt32();
            uint32 transferId = field[6].GetUInt32();
            uint32 newguid = 0;
            if(!toacc)
                toacc = account;

            DumpReturn dumpState = PlayerDumpReader().LoadDump(toacc, dump, "", newguid);

            TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Transfer toLoad guid %u, dump %u", guid, dumpState);

            if (dumpState == DUMP_SUCCESS)
            {
                LoginDatabase.PQuery("DELETE FROM `transferts` WHERE `id` = %u", transaction);
                PreparedStatement * stmt = LoginDatabase.GetPreparedStatement(LOGIN_ADD_TRANSFERTS_LOGS);
                if(stmt)
                {
                    stmt->setUInt32(0, transaction);
                    stmt->setUInt32(1, account);
                    stmt->setUInt64(2, guid);
                    stmt->setUInt32(3, from);
                    stmt->setUInt32(4, realm.Id.Realm);
                    stmt->setString(5, dump);
                    stmt->setUInt32(6, toacc);
                    stmt->setUInt32(7, newguid);
                    stmt->setUInt32(8, transferId);
                    LoginDatabase.Execute(stmt);
                    if(transferId)
                    {
                        if (realm.Id.Realm == 59)
                            LoginDatabase.PQuery("UPDATE `transfer_requests` SET `guid` = '%u' WHERE `id` = '%u'", newguid, transferId);
                        else
                            LoginDatabase.PQuery("UPDATE `transfer_requests` SET `status` = '%u', `guid` = '%u' WHERE `id` = '%u'", dumpState, newguid, transferId);
                    }
                }
            }
            else
            {
                LoginDatabase.PQuery("UPDATE `transferts` SET `error` = '%u', `nb_attempt` = `nb_attempt` + 1 WHERE `id` = '%u'", dumpState, transaction);
                if (transferId && realm.Id.Realm != 59)
                    LoginDatabase.PQuery("UPDATE `transfer_requests` SET `status` = '%u' WHERE `id` = '%u'", dumpState, transferId);
                continue;
            }
        }
        while (toLoad->NextRow());
    }
}

Realm realm;

uint32 GetVirtualRealmAddress()
{
    return uint32(realm.Id.Region) << 24 | uint32(realm.Id.Site) << 16 | realm.Id.Realm;
}

uint32 World::GetRealmId() const
{
    return realm.Id.Realm;
}

CliCommandHolder::CliCommandHolder(void* callbackArg, const char* command, Print* zprint, CommandFinished* commandFinished): m_callbackArg(callbackArg), m_print(zprint), m_commandFinished(commandFinished)
{
    size_t len = strlen(command) + 1;
    m_command = new char[len];
    memcpy(m_command, command, len);
}

CliCommandHolder::~CliCommandHolder()
{
    delete[] m_command;
}

GlobalMessageData::GlobalMessageData(WorldPacket const* _packet, WorldSession* _self, uint32 _team) : packet(*_packet), self(_self), team(_team)
{}
