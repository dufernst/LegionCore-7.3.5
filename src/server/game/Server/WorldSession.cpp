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
    \ingroup u2w
*/

#include <zlib.h>
#include <utility>

#include "AccountMgr.h"
#include "Anticheat.h"
#include "AuthenticationPackets.h"
#include "BattlefieldMgr.h"
#include "BattlegroundMgr.h"
#include "BattlenetPackets.h"
#include "BattlenetRpcErrorCodes.h"
#include "BattlePayMgr.h"
#include "CharacterData.h"
#include "CharacterPackets.h"
#include "ChatPackets.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "Chat.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "LFGListMgr.h"
#include "Log.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "PacketUtilities.h"
#include "Player.h"
#include "RealmList.h"
#include "ScriptMgr.h"
#include "SocialMgr.h"
#include "SystemPackets.h"
#include "WardenMac.h"
#include "WardenWin.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "WorldSocket.h"

#define MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE 100

WorldPackets::Null::Null(WorldPacket&& packet): ClientPacket(std::move(packet))
{
}

void WorldPackets::Null::Read()
{
    _worldPacket.rfinish();
}

WorldSession::WorldSession(uint32 id, std::string&& name, const std::shared_ptr<WorldSocket>& sock, AccountTypes sec, uint8 expansion, time_t mute_time, std::string os, LocaleConstant locale, uint32 recruiter, bool isARecruiter, AuthFlags flag, int64 balance):
m_muteTime(mute_time), m_timeOutTime(0), _countPenaltiesHwid(0), _player(nullptr), m_map(nullptr), _security(sec), _accountId(id), m_expansion(expansion), m_accountExpansion(expansion), _logoutTime(0), m_inQueue(false), m_playerLogout(false), m_playerRecentlyLogout(false),
m_playerSave(false), m_sessionDbLocaleIndex(locale), m_latency(0), _tutorialsChanged(false), recruiterId(recruiter), isRecruiter(isARecruiter), timeCharEnumOpcode(0), playerLoginCounter(0), forceExit(false), m_sUpdate(false), wardenModuleFailed(false), atAuthFlag(flag), canLogout(false), battlePayBalance(balance)
{
    _os = std::move(os);
    _accountName = std::move(name);

    memset(_tutorials, 0, sizeof(_tutorials));
    _battlenetRequestToken = 0;

    m_clientTimeDelay = 0;

    _warden = nullptr;

    _battlePayMgr = std::make_shared<BattlepayManager>(this);

    _filterAddonMessages = false;

    if (sock)
    {
        m_Address = sock->GetRemoteIpAddress().to_string();
        ResetTimeOutTime();
        LoginDatabase.PExecute("UPDATE account SET online = 1 WHERE id = %u;", GetAccountId());     // One-time query
    }

    m_Socket[CONNECTION_TYPE_REALM] = sock;
    _instanceConnectKey.Raw = UI64LIT(0);

    expireTime = sWorld->getIntConfig(CONFIG_WORLD_SESSION_EXPIRE_TIME);

    sWorld->IncreaseSessionCount();
    m_IsPetBattleJournalLocked = false;
    
    // Personal Rate
    QueryResult result = LoginDatabase.PQuery("SELECT rate from `account_rates` where realm = %u and account = %u", sWorld->GetRealmId(), GetAccountId());
    
    if (result)
    {
        Field* fields = result->Fetch();
        SetPersonalXPRate(fields[0].GetFloat());
    }

    m_achievement.assign(MAX_ACHIEVEMENT, false);
}

/// WorldSession destructor
WorldSession::~WorldSession()
{
    ///- unload player if not unloaded
    if (_player)
        LogoutPlayer(true);

    /// - If have unclosed socket, close it
    for (auto & i : m_Socket)
    {
        if (i)
        {
            i->CloseSocket();
            i.reset();
        }
    }

    delete _warden;

    ///- empty incoming packet queue
    WorldPacket* packet = nullptr;
    while (_recvQueue.next(packet))
        delete packet;

    LoginDatabase.PExecute("UPDATE account SET online = 0 WHERE id = %u;", GetAccountId());     // One-time query
    sWorld->DecreaseSessionCount();
}

/// Get the player name
std::string WorldSession::GetPlayerName(bool simple /* = true */) const
 {
    std::string name = "[Player: ";
    uint32 guidLow = 0;

    if (Player* player = GetPlayer())
    {
        name.append(player->GetName());
        guidLow = player->GetGUIDLow();
    }
    else
        name.append("<none>");

    if (!simple)
    {
        std::ostringstream ss;
        ss << " (Guid: " << guidLow << ", Account: " << GetAccountId() << ")";
        name.append(ss.str());
    }

    name.append("]");
    return name;
}

/// Get player guid if available. Use for logging purposes only
ObjectGuid::LowType WorldSession::GetGuidLow() const
{
    return GetPlayer() ? GetPlayer()->GetGUIDLow() : 0;
}

/// Send a packet to the client
void WorldSession::SendPacket(WorldPacket const* packet, bool forced /*= false*/)
{
    uint32 opcode = packet->GetOpcode();
    if (opcode == NULL_OPCODE)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Prevented sending of NULL_OPCODE to %s", GetPlayerName(false).c_str());
        return;
    }
    if (opcode == MAX_OPCODE)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Prevented sending of wrong opcode to %s", GetPlayerName(false).c_str());
        return;
    }

    ServerOpcodeHandler const* handler = opcodeTable[static_cast<OpcodeServer>(opcode)];
    if (!handler)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Prevented sending of opcode %u with non existing handler to %s", opcode, GetPlayerName().c_str());
        return;
    }

    ConnectionType conIdx = handler->ConnectionIndex;
    if (packet->GetConnection() != CONNECTION_TYPE_DEFAULT)
    {
        if (packet->GetConnection() != CONNECTION_TYPE_INSTANCE && IsInstanceOnlyOpcode(opcode))
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Prevented sending of instance only opcode %u with connection type %u to %s", opcode, packet->GetConnection(), GetPlayerName().c_str());
            return;
        }

        conIdx = packet->GetConnection();
    }

    if (!m_Socket[conIdx])
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Prevented sending of %s to non existent socket %u to %s", GetOpcodeNameForLogging(static_cast<OpcodeServer>(opcode)).c_str(), conIdx, GetPlayerName().c_str());
        return;
    }

    if (!forced && handler->Status == STATUS_UNHANDLED)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Prevented sending disabled opcode %s to %s", GetOpcodeNameForLogging(static_cast<OpcodeServer>(opcode)).c_str(), GetPlayerName().c_str());
        return;
    }

    uint32 packetSize = packet->size();
    uint32 start_time = getMSTime();
    const_cast<WorldPacket*>(packet)->FlushBits();

    if (m_Socket[conIdx]) // http://pastebin.com/8ntVgj49
        m_Socket[conIdx]->SendPacket(*packet);

    if ((getMSTime() - start_time) > 50)
        sLog->outDiff(" >> SendPacket DIFF %u player_guid %u _mapID_ %i AccountId %u opcode %u packetSize %u", getMSTime() - start_time, (_player && !_player->IsDelete()) ? _player->GetGUIDLow() : 0, (_player && !_player->IsDelete()) ? _player->GetMapId() : -1, GetAccountId(), opcode, packetSize);
}

/// Add an incoming packet to the queue
void WorldSession::QueuePacket(WorldPacket* new_packet)
{
    _recvQueue.add(new_packet);
}

/// Logging helper for unexpected opcodes
void WorldSession::LogUnexpectedOpcode(WorldPacket* packet, const char* status, const char *reason)
{
    #ifdef WIN32
    TC_LOG_ERROR(LOG_FILTER_OPCODES, "Received unexpected opcode %s Status: %s Reason: %s from %s",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(packet->GetOpcode())).c_str(), status, reason, GetPlayerName(false).c_str());
    #endif
}

/// Logging helper for unexpected opcodes
void WorldSession::LogUnprocessedTail(WorldPacket const* packet)
{
    if (!sLog->ShouldLog(LOG_FILTER_NETWORKIO, LOG_LEVEL_TRACE) || packet->rpos() >= packet->wpos())
        return;

    #ifdef WIN32
    TC_LOG_ERROR(LOG_FILTER_OPCODES, "Unprocessed tail data (read stop at %u from %u) Opcode %s from %s",
        uint32(packet->rpos()), uint32(packet->wpos()), GetOpcodeNameForLogging(static_cast<OpcodeClient>(packet->GetOpcode())).c_str(), GetPlayerName(false).c_str());
    packet->print_storage();
    #endif
}

/// Update the WorldSession (triggered by World update)
bool WorldSession::Update(uint32 diff, Map* map)
{
    if (m_sUpdate)
        return true;

    m_sUpdate = true;

    uint32 _s = getMSTime();
    /// Update Timeout timer.
    UpdateTimeOutTime(diff);
    m_Functions.Update(diff);

    ///- Before we process anything:
    /// If necessary, kick the player from the character select screen
    if (IsConnectionIdle())
        if (m_Socket[CONNECTION_TYPE_REALM])
            m_Socket[CONNECTION_TYPE_REALM]->CloseSocket();

    if (_warden)
        _warden->Update();

    ///- Retrieve packets from the receive queue and call the appropriate handlers
    /// not process packets if socket already closed
    WorldPacket* packet = nullptr;
    //! Delete packet after processing by default
    bool deletePacket = true;
    std::vector<WorldPacket*> requeuePackets;
    uint32 processedPackets = 0;

    // don`t delete this, need for debug info in crashlog
    volatile uint32 _player_guid_ = (_player && !_player->IsDelete()) ? _player->GetGUIDLow() : 0;
    volatile int32 _mapID_ = (_player && !_player->IsDelete()) ? _player->GetMapId() : -1;
    // volatile Position* _pos_ = new Position(*_player);

    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("WorldSession::Update 0 _mapID_ %i Update time - %ums diff %u _player_guid_ %u _recvQueue %u", _mapID_, _ms, diff, _player_guid_, _recvQueue.size());

    if (_recvQueue.size() > 20000) // Prevent ddos
    {
        sLog->outSpamm("WorldSession::Update ddos KickPlayer _mapID_ %i Update time - %ums diff %u _player_guid_ %u _recvQueue %u", _mapID_, _ms, diff, _player_guid_, _recvQueue.size());
        while (_recvQueue.next(packet))
            delete packet;
        KickPlayer();
    }

    while (m_Socket[CONNECTION_TYPE_REALM] && map == m_map && _recvQueue.next(packet))
    {
        OpcodeClient opcode = static_cast<OpcodeClient>(packet->GetOpcode());
        uint32 packetSize = packet->size();
        ClientOpcodeHandler const* opHandle = opcodeTable[opcode];
        try
        {
            switch (opHandle->Status)
            {
                case STATUS_LOGGEDIN:
                    if (!_player)
                    {
                        // skip STATUS_LOGGEDIN opcode unexpected errors if player logout sometime ago - this can be network lag delayed packets
                        //! If player didn't log out a while ago, it means packets are being sent while the server does not recognize
                        //! the client to be in world yet. We will re-add the packets to the bottom of the queue and process them later.
                        if (!m_playerRecentlyLogout)
                        {
                            requeuePackets.push_back(packet);
                            deletePacket = false;
                            #ifdef WIN32
                            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Re-enqueueing packet with opcode %s with with status STATUS_LOGGEDIN. Player is currently not in world yet.", GetOpcodeNameForLogging(opcode).c_str());
                            #endif
                        }
                    }
                    else if (_player->IsInWorld())
                        opHandle->Call(this, *packet);
                    // lag can cause STATUS_LOGGEDIN opcodes to arrive after the player started a transfer
                    break;
                case STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT:
                    if (!_player && !m_playerRecentlyLogout && !m_playerLogout) // There's a short delay between _player = null and m_playerRecentlyLogout = true during logout
                        LogUnexpectedOpcode(packet, "STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT", "the player has not logged in yet and not recently logout");
                    else
                        opHandle->Call(this, *packet); // not expected _player or must checked in packet hanlder
                    break;
                case STATUS_TRANSFER:
                    if (!_player)
                        LogUnexpectedOpcode(packet, "STATUS_TRANSFER", "the player has not logged in yet");
                    else if (_player->IsInWorld())
                        LogUnexpectedOpcode(packet, "STATUS_TRANSFER", "the player is still in world");
                    else
                        opHandle->Call(this, *packet);
                    break;
                case STATUS_AUTHED:
                    // prevent cheating with skip queue wait
                    if (m_inQueue)
                    {
                        LogUnexpectedOpcode(packet, "STATUS_AUTHED", "the player not pass queue yet");
                        break;
                    }

                    // some auth opcodes can be recieved before STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT opcodes
                    // however when we recieve CMSG_ENUM_CHARACTERS we are surely no longer during the logout process.
                    if (packet->GetOpcode() == CMSG_ENUM_CHARACTERS)
                        m_playerRecentlyLogout = false;

                    opHandle->Call(this, *packet);
                    break;
                case STATUS_NEVER:
                    #ifdef WIN32
                    TC_LOG_ERROR(LOG_FILTER_OPCODES, "Received not allowed opcode %s from %s", GetOpcodeNameForLogging(opcode).c_str(), GetPlayerName(false).c_str());
                    #endif
                    break;
                case STATUS_UNHANDLED:
                    #ifdef WIN32
                    TC_LOG_ERROR(LOG_FILTER_OPCODES, "Received not handled opcode %s from %s", GetOpcodeNameForLogging(opcode).c_str(), GetPlayerName(false).c_str());
                    #endif
                    break;
            }
        }
        catch (WorldPackets::PacketArrayMaxCapacityException const& pamce)
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "PacketArrayMaxCapacityException: %s while parsing %s from %s.",
                pamce.what(), GetOpcodeNameForLogging(opcode).c_str(), GetPlayerName(false).c_str());
        }
        catch (ByteBufferException const&)
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSession::Update ByteBufferException occured while parsing a packet (opcode: %u) from client %s, accountid=%i. Skipped packet.",
                packet->GetOpcode(), GetRemoteAddress().c_str(), GetAccountId());
            packet->hexlike();
        }

        if (deletePacket)
            delete packet;

        deletePacket = true;
        processedPackets++;

        //process only a max amout of packets in 1 Update() call.
        //Any leftover will be processed in next update
        if (processedPackets > MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE)
            break;

        _ms = GetMSTimeDiffToNow(_s);
        if (_ms > 500) // If many time wait break
        {
            sLog->outDiff("WorldSession::Update 1 _mapID_ %i Update time - %ums diff %u _player_guid_ %u opcode %s packetSize %u", _mapID_, _ms, diff, _player_guid_, GetOpcodeNameForLogging(opcode).c_str(), packetSize);
            break;
        }
    }

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("WorldSession::Update 1 _mapID_ %i Update time - %ums diff %u _player_guid_ %u _recvQueue %u", _mapID_, _ms, diff, _player_guid_, _recvQueue.size());

    _recvQueue.readd(requeuePackets.begin(), requeuePackets.end());

    if (map != m_map)
    {
        m_sUpdate = false;
        return true;
    }

    ProcessQueryCallbacks();

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("WorldSession::Update 2 _mapID_ %i Update time - %ums diff %u _player_guid_ %u _recvQueue %u", _mapID_, _ms, diff, _player_guid_, _recvQueue.size());

    if (map != m_map)
    {
        m_sUpdate = false;
        return true;
    }

    //check if we are safe to proceed with logout
    //logout procedure should happen only in World::UpdateSessions() method!!!
    // if (updater.ProcessLogout())
    {
        if (canLogout) // Logout only if remove from map
        {
            LogoutPlayer(true);
            canLogout = false;
        }

        time_t currTime = time(nullptr);
        ///- If necessary, log the player out
        if (ShouldLogOut(currTime) && m_playerLoading.IsEmpty())
        {
            if (Player* player = GetPlayer())
                player->SetRemoveFromMap(true);
            else
                canLogout = true; // To next tick
        }

        ///- Cleanup socket pointer if need
        if ((m_Socket[CONNECTION_TYPE_REALM] && !m_Socket[CONNECTION_TYPE_REALM]->IsOpen()) || (m_Socket[CONNECTION_TYPE_INSTANCE] && !m_Socket[CONNECTION_TYPE_INSTANCE]->IsOpen()))
        {
            expireTime -= expireTime > diff ? diff : expireTime;
            if (expireTime < diff || forceExit || !GetPlayer())
            {
                if (m_Socket[CONNECTION_TYPE_REALM])
                {
                    m_Socket[CONNECTION_TYPE_REALM]->CloseSocket();
                    m_Socket[CONNECTION_TYPE_REALM].reset();
                }
                if (m_Socket[CONNECTION_TYPE_INSTANCE])
                {
                    m_Socket[CONNECTION_TYPE_INSTANCE]->CloseSocket();
                    m_Socket[CONNECTION_TYPE_INSTANCE].reset();
                }
            }
        }

        if (!m_Socket[CONNECTION_TYPE_REALM])
        {
            m_sUpdate = false;
            return false;                                       //Will remove this session from the world session map
        }
    }

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("WorldSession::Update 3 _mapID_ %i Update time - %ums diff %u _player_guid_ %u _recvQueue %u", _mapID_, _ms, diff, _player_guid_, _recvQueue.size());

    m_sUpdate = false;
    return true;
}

uint32 WorldSession::GetCountWardenPacketsInQueue()
{
    //for (auto itr = _recvQueue.)
    return 0;
}

/// %Log the player out
void WorldSession::LogoutPlayer(bool Save)
{
    if (m_playerLogout)
        return;

    uint32 _s = getMSTime();
    m_playerLogout = true;
    m_playerSave = Save;

    if (_player)
    {
        Player* player = _player;
        // _player->GetAchievementMgr().ClearMap();
        sLFGListMgr->RemovePlayerDueToLogout(_player->GetGUIDLow());

        if (_player->GetMap())
        {
            sOutdoorPvPMgr->HandlePlayerLeaveZone(_player->GetGUID(), _player->GetCurrentZoneID());
            sOutdoorPvPMgr->HandlePlayerLeaveArea(_player->GetGUID(), _player->GetCurrentAreaID());
        }

        _player->SetDelete();
        _player->SetChangeMap(false); // If not disable crash on clear evecnt function

        // finish pending transfers before starting the logout
        if (_player->IsBeingTeleportedFar())
        {
            _player->SetMapId(_player->m_homebindMapId);
            _player->Relocate(_player->m_homebindX, _player->m_homebindY, _player->m_homebindZ, _player->GetOrientation());
            _player->RemoveAurasWithInterruptFlags(SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING));
            _player->RemoveAurasByType(SPELL_AURA_OVERRIDE_SPELLS);
            _player->RemoveAurasByType(SPELL_AURA_MOD_NEXT_SPELL);
            _player->SetSemaphoreTeleportFar(false);
        }

        ObjectGuid lguid = _player->GetLootGUID();
        if (!lguid.IsEmpty())
            DoLootRelease(lguid);

        ///- If the player just died before logging out, make him appear as a ghost
        //FIXME: logout must be delayed in case lost connection with client in time of combat
        if (_player->GetDeathTimer())
        {
            _player->getHostileRefManager().deleteReferences();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();
        }
        else if (!_player->getAttackers()->empty() && _player->isInCombat())
        {
            bool _killer = false;
            // build set of player who attack _player or who have pet attacking of _player
            std::set<Player*> aset;
            for (auto itr : *_player->getAttackers())
            {
                Unit* owner = itr->GetOwner();           // including player controlled case
                if (owner && owner->IsPlayer())
                    aset.insert(owner->ToPlayer());
                else if (itr->IsPlayer())
                    aset.insert(itr->ToPlayer());

                if (!itr->isTrainingDummy())
                    _killer = true;
            }
            // CombatStop() method is removing all attackers from the AttackerSet
            // That is why it must be AFTER building current set of attackers
            _player->CombatStop();
            _player->getHostileRefManager().setOnlineOfflineState(false);

            if (_killer)
            {
                _player->RemoveAllAurasOnDeath();
                _player->SetPvPDeath(!aset.empty());
                _player->KillPlayer();
                _player->BuildPlayerRepop();
                _player->RepopAtGraveyard();
            }

            // give honor to all attackers from set like group case
            for (auto itr = aset.begin(); itr != aset.end(); ++itr)
                (*itr)->RewardHonor(_player, aset.size());

            // give bg rewards and update counters like kill by first from attackers
            // this can't be called for all attackers.
            if (!aset.empty())
                if (Battleground* bg = _player->GetBattleground())
                    bg->HandleKillPlayer(_player, *aset.begin());
        }
        else if (_player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        {
            // this will kill character by SPELL_AURA_SPIRIT_OF_REDEMPTION
            _player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
            _player->KillPlayer();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();
        }
        else if (_player->HasPendingBind())
        {
            _player->RepopAtGraveyard();
            _player->SetPendingBind(0, 0);
        }

        if (Battleground* bg = _player->GetBattleground())
        {
            if (bg->IsArena() && !bg->IsWargame() && (bg->GetStatus() == STATUS_IN_PROGRESS || bg->GetStatus() == STATUS_WAIT_JOIN) && bg->GetJoinType() != MS::Battlegrounds::JoinType::Arena1v1)
                player->SendOperationsAfterDelay(OAD_ARENA_DESERTER);
        }

        sBattlefieldMgr->EventPlayerLoggedOut(_player);

        //drop a flag if player is carrying it
        if (Battleground* bg = _player->GetBattleground())
        {
            if (_player->IsSpectator())
                _player->LeaveBattleground();
            else
            {
                if (_player->HasAura(43680) || _player->HasAura(43681))
                    _player->LeaveBattleground();
                else
                    bg->EventPlayerLoggedOut(_player);
            }
        }

        ///- Teleport to home if the player is in an invalid instance
        if (!_player->m_InstanceValid && !_player->isGameMaster())
        {
            _player->SetMapId(_player->m_homebindMapId);
            _player->Relocate(_player->m_homebindX, _player->m_homebindY, _player->m_homebindZ, _player->GetOrientation());
            _player->RemoveAurasWithInterruptFlags(SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING));
            _player->RemoveAurasByType(SPELL_AURA_OVERRIDE_SPELLS);
            _player->RemoveAurasByType(SPELL_AURA_MOD_NEXT_SPELL);
            _player->SetSemaphoreTeleportFar(false);
        }

        for (int i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        {
            if (uint8 bgQueueTypeId = _player->GetBattlegroundQueueTypeId(i))
            {
                if (MS::Battlegrounds::CheckIsBattlegroundTypeByBgTypeID(bgQueueTypeId) || MS::Battlegrounds::CheckIsArenaTypeByBgTypeID(bgQueueTypeId))
                    if (_player->IsInvitedForBattlegroundQueueType(bgQueueTypeId))
                        player->SendOperationsAfterDelay(OAD_ARENA_DESERTER);

                _player->RemoveBattlegroundQueueId(bgQueueTypeId);
                sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId).RemovePlayer(_player->GetGUID(), true);
            }
        }

        // Repop at GraveYard or other player far teleport will prevent saving player because of not present map
        // Teleport player immediately for correct player save
        if (_player->IsBeingTeleportedFar())
        {
            _player->SetMapId(_player->m_homebindMapId);
            _player->Relocate(_player->m_homebindX, _player->m_homebindY, _player->m_homebindZ, _player->GetOrientation());
            _player->RemoveAurasWithInterruptFlags(SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING));
            _player->RemoveAurasByType(SPELL_AURA_OVERRIDE_SPELLS);
            _player->RemoveAurasByType(SPELL_AURA_MOD_NEXT_SPELL);
            _player->SetSemaphoreTeleportFar(false);
        }

        ///- If the player is in a guild, update the guild roster and broadcast a logout message to other guild members
        if (!_player->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
            if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
                guild->HandleMemberLogout(this);

        _player->UnlearnSpellsFromOtherClasses();
        _player->UnsummonCurrentBattlePetIfAny(true);

        ///- Remove pet
        if (_player->getClass() != CLASS_WARLOCK)
            _player->RemovePet(nullptr);
        else if (Pet* _pet = _player->GetPet())
            _pet->SavePetToDB();

        ///- empty buyback items and save the player in the database
        // some save parts only correctly work in case player present in map/player_lists (pets, etc)
        if (Save)
        {
            for (int j = BUYBACK_SLOT_START; j < BUYBACK_SLOT_END; ++j)
            {
                uint32 eslot = j - BUYBACK_SLOT_START;
                _player->SetGuidValue(PLAYER_FIELD_INV_SLOTS + (j * 4), ObjectGuid::Empty);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE + eslot, 0);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP + eslot, 0);
            }
            _player->UpdatePlayerNameData();
            _player->SaveToDB();
        }

        ///- Leave all channels before player delete...
        _player->CleanupChannels();

        ///- If the player is in a group (or invited), remove him. If the group if then only 1 person, disband the group.
        _player->UninviteFromGroup();

        // remove player from the group if he is:
        // a) in group; b) not in raid group; c) logging out normally (not being kicked or disconnected)
        //if (_player->GetGroup() && !_player->GetGroup()->isRaidGroup() && m_Socket[CONNECTION_TYPE_REALM])
        //    _player->RemoveFromGroup();

        //! Send update to group and reset stored max enchanting level
        if (_player->GetGroup())
        {
            _player->GetGroup()->SendUpdate();
            _player->GetGroup()->ResetMaxEnchantingLevel();
        }

        //! Broadcast a logout message to the player's friends
        if (!_player->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
            sSocialMgr->SendFriendStatus(_player, FRIEND_OFFLINE, _player->GetGUID(), true);

        sSocialMgr->RemovePlayerSocial(_player->GetGUID());

        //! Call script hook before deletion
        sScriptMgr->OnPlayerLogout(_player);

        //! Remove the player from the world
        // the player may not be in the world when logging out
        // e.g if he got disconnected during a transfer to another map
        // calls to GetMap in this case may cause crashes
        volatile uint32 guidDebug = _player->GetGUIDLow();
        _player->CleanupsBeforeDelete();
        TC_LOG_INFO(LOG_FILTER_CHARACTER, "Account: %d (IP: %s) Logout Character:[%s] (GUID: %u) Level: %d", GetAccountId(), GetRemoteAddress().c_str(), _player->GetName(), _player->GetGUIDLow(), _player->getLevel());

        //! Send the 'logout complete' packet to the client
        //! Client will respond by sending 3x CMSG_CANCEL_TRADE, which we currently dont handle
        //! Send before delete. As need guid.
        if (m_Socket[CONNECTION_TYPE_REALM])
            SendPacket(WorldPackets::Character::LogoutComplete().Write());

        if (Map* _map = _player->FindMap())
            _map->RemovePlayerFromMap(_player, true);

        SetPlayer(nullptr); //! Pointer already deleted during RemovePlayerFromMap

        //! Since each account can only have one online character at any given time, ensure all characters for active account are marked as offline
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ACCOUNT_ONLINE);
        stmt->setUInt32(0, GetAccountId());
        CharacterDatabase.Execute(stmt);
    }

    if (m_Socket[CONNECTION_TYPE_INSTANCE])
    {
        m_Socket[CONNECTION_TYPE_INSTANCE]->CloseSocket();
        m_Socket[CONNECTION_TYPE_INSTANCE].reset();
    }

    // SetMap(NULL);
    m_playerLogout = false;
    m_playerSave = false;
    m_playerRecentlyLogout = true;
    LogoutRequest(0);

    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("WorldSession::LogoutPlayer wait - %ums", _ms);
}

/// Kick a player out of the World
void WorldSession::KickPlayer()
{
    for (auto & i : m_Socket)
    {
        if (i)
        {
            i->CloseSocket();
            forceExit = true;
        }
    }
}

void WorldSession::SendNotification(const char *format, ...)
{
    if (format)
    {
        va_list ap;
        char szStr[1024];
        szStr[0] = '\0';
        va_start(ap, format);
        vsnprintf(szStr, 1024, format, ap);
        va_end(ap);

        SendPacket(WorldPackets::Chat::PrintNotification(szStr).Write());
    }
}

void WorldSession::SendNotification(uint32 string_id, ...)
{
    char const* format = GetTrinityString(string_id);
    if (format)
    {
        va_list ap;
        char szStr[1024];
        szStr[0] = '\0';
        va_start(ap, string_id);
        vsnprintf(szStr, 1024, format, ap);
        va_end(ap);

        SendPacket(WorldPackets::Chat::PrintNotification(szStr).Write());
    }
}

const char* WorldSession::GetTrinityString(int32 entry) const
{
    return sObjectMgr->GetTrinityString(entry, GetSessionDbLocaleIndex());
}

void WorldSession::Handle_NULL(WorldPackets::Null& null)
{
    TC_LOG_ERROR(LOG_FILTER_OPCODES, "Received unhandled opcode %s from %s",
        GetOpcodeNameForLogging(null.GetOpcode()).c_str(), GetPlayerName(false).c_str());
}

void WorldSession::Handle_EarlyProccess(WorldPacket& recvPacket)
{
    TC_LOG_ERROR(LOG_FILTER_OPCODES, "Received opcode %s that must be processed in WorldSocket::OnRead from %s",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvPacket.GetOpcode())).c_str(), GetPlayerName(false).c_str());
}

void WorldSession::SendConnectToInstance(WorldPackets::Auth::ConnectToSerial serial)
{
    TC_LOG_INFO(LOG_FILTER_OPCODES, "WorldSession::SendConnectToInstance");

    auto instanceAddress = sRealmList->GetAddressForClient(_realmID);

    _instanceConnectKey.Fields.AccountId = GetAccountId();
    _instanceConnectKey.Fields.ConnectionType = CONNECTION_TYPE_INSTANCE;
    _instanceConnectKey.Fields.Key = urand(0, 0x7FFFFFFF);

    WorldPackets::Auth::ConnectTo connectTo;
    connectTo.Key = _instanceConnectKey.Raw;
    connectTo.Serial = serial;
    connectTo.Payload.Port = sWorld->getIntConfig(CONFIG_PORT_INSTANCE);
    if (instanceAddress.is_v4())
    {
        memcpy(connectTo.Payload.Where.data(), instanceAddress.to_v4().to_bytes().data(), 4);
        connectTo.Payload.Type = WorldPackets::Auth::ConnectTo::IPv4;
    }
    else
    {
        memcpy(connectTo.Payload.Where.data(), instanceAddress.to_v6().to_bytes().data(), 16);
        connectTo.Payload.Type = WorldPackets::Auth::ConnectTo::IPv6;
    }
    connectTo.Con = CONNECTION_TYPE_INSTANCE;

    SendPacket(connectTo.Write());
}

void WorldSession::LoadAccountData(PreparedQueryResult const& result, uint32 mask)
{
    for (uint32 i = 0; i < NUM_ACCOUNT_DATA_TYPES; ++i)
        if (mask & (1 << i))
            m_accountData[i] = AccountData();

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 type = fields[0].GetUInt8();
        if (type >= NUM_ACCOUNT_DATA_TYPES)
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Table `%s` have invalid account data type (%u), ignore.",
                mask == GLOBAL_CACHE_MASK ? "account_data" : "character_account_data", type);
            continue;
        }

        if ((mask & (1 << type)) == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Table `%s` have non appropriate for table  account data type (%u), ignore.",
                mask == GLOBAL_CACHE_MASK ? "account_data" : "character_account_data", type);
            continue;
        }

        m_accountData[type].Time = time_t(fields[1].GetUInt32());
        m_accountData[type].Data = fields[2].GetString();
    }
    while (result->NextRow());
}

AccountData* WorldSession::GetAccountData(AccountDataType type)
{
    return &m_accountData[type];
}

void WorldSession::SetAccountData(AccountDataType type, time_t tm /*= time_t(0)*/, std::string const& data /*= ""*/)
{
    if ((1 << type) & GLOBAL_CACHE_MASK)
    {
        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_ACCOUNT_DATA);
        stmt->setUInt32(0, GetAccountId());
        stmt->setUInt8(1, type);
        stmt->setUInt32(2, uint32(tm));
        stmt->setString(3, data);
        CharacterDatabase.Execute(stmt);
    }
    else
    {
        // _player can be NULL and packet received after logout but m_GUID still store correct guid
        if (!m_GUIDLow)
            return;

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_PLAYER_ACCOUNT_DATA);
        stmt->setUInt64(0, m_GUIDLow);
        stmt->setUInt8(1, type);
        stmt->setUInt32(2, uint32(tm));
        stmt->setString(3, data);
        CharacterDatabase.Execute(stmt);
    }

    m_accountData[type].Time = tm;
    m_accountData[type].Data = data;
}

void WorldSession::SendSetTimeZoneInformation()
{
    WorldPackets::System::SetTimeZoneInformation packet;
    packet.ServerTimeTZ = sWorld->m_serverTimeTZ;    //RTL: Europe/Paris
    packet.GameTimeTZ = sWorld->m_gameTimeTZ;      //RTL: Europe/Paris
    SendPacket(packet.Write());
}

void WorldSession::LoadTutorialsData(PreparedQueryResult const& result)
{
    memset(_tutorials, 0, sizeof(uint32) * MAX_ACCOUNT_TUTORIAL_VALUES);

    if (result)
        for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
            _tutorials[i] = (*result)[i].GetUInt32();

    _tutorialsChanged = false;
}

void WorldSession::LoadCharacterTemplates(PreparedQueryResult const& result)
{
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();
        CharacterTemplateData& templateData = charTemplateData[id];

        templateData.id = id;
        templateData.level = fields[1].GetUInt8();
        templateData.iLevel = fields[2].GetUInt32();
        templateData.money = fields[3].GetUInt32();
        templateData.artifact = fields[4].GetBool();
        templateData.transferId = fields[5].GetUInt32();
        templateData.templateId = fields[6].GetUInt32();
        templateData.charTemplate = sCharacterDataStore->GetCharacterTemplate(templateData.templateId);
    }
    while (result->NextRow());
}

void WorldSession::SendTutorialsData()
{
    WorldPackets::Misc::TutorialFlags packet;
    memcpy(packet.TutorialData, _tutorials, sizeof(_tutorials));
    SendPacket(packet.Write());
}

void WorldSession::SaveTutorialsData(SQLTransaction &trans)
{
    if (!_tutorialsChanged)
        return;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HAS_TUTORIALS);
    stmt->setUInt32(0, GetAccountId());
    bool hasTutorials = bool(CharacterDatabase.Query(stmt));
    // Modify data in DB
    stmt = CharacterDatabase.GetPreparedStatement(hasTutorials ? CHAR_UPD_TUTORIALS : CHAR_INS_TUTORIALS);
    for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
        stmt->setUInt32(i, _tutorials[i]);
    stmt->setUInt32(MAX_ACCOUNT_TUTORIAL_VALUES, GetAccountId());
    trans->Append(stmt);

    _tutorialsChanged = false;
}

uint32 WorldSession::GetTutorialInt(uint8 index) const
{
    return _tutorials[index];
}

void WorldSession::SetTutorialInt(uint8 index, uint32 value)
{
    if (_tutorials[index] != value)
    {
        _tutorials[index] = value;
        _tutorialsChanged = true;
    }
}

bool WorldSession::IsAddonRegistered(std::string const& prefix)
{
    if (!_player || _player->IsChangeMap())
        return true;

    if (!_filterAddonMessages) // if we have hit the softcap (64) nothing should be filtered
        return true;

    if (_registeredAddonPrefixes.empty())
        return false;

    auto itr = std::find(_registeredAddonPrefixes.begin(), _registeredAddonPrefixes.end(), prefix);
    return itr != _registeredAddonPrefixes.end();
}

void WorldSession::SetPlayer(Player* player)
{
    _player = player;

    // set m_GUID that can be used while player loggined and later until m_playerRecentlyLogout not reset
    if (_player)
        m_GUIDLow = _player->GetGUIDLow();
}

void WorldSession::ProcessQueryCallbacks()
{
    _queryProcessor.ProcessReadyQueries();

    if (_realmAccountLoginCallback.valid() && _realmAccountLoginCallback.wait_for(std::chrono::seconds(0)) == std::future_status::ready &&
        _accountLoginCallback.valid() && _accountLoginCallback.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        InitializeSessionCallback(_realmAccountLoginCallback.get(), _accountLoginCallback.get());

    if (_charLoginCallback.valid() && _charLoginCallback.wait_for(Seconds(0)) == std::future_status::ready)
        HandlePlayerLogin(reinterpret_cast<LoginQueryHolder*>(_charLoginCallback.get()));
}

bool WorldSession::InitializeWarden(BigNumber* k, std::string const& os)
{
    if (os == "Win" || os == "Wn64")
        _warden = new WardenWin(this);
    else
        _warden = new WardenMac(this);

    return _warden && _warden->Create(k);
}

void WorldSession::RemoveAuthFlag(AuthFlags f)
{
    atAuthFlag = AuthFlags(atAuthFlag & ~f);
    SaveAuthFlag();
}

void WorldSession::AddAuthFlag(AuthFlags f)
{
    atAuthFlag = AuthFlags(atAuthFlag | f);
    SaveAuthFlag();
}

void WorldSession::SaveAuthFlag()
{
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_AT_AUTH_FLAG);
    stmt->setUInt16(0, atAuthFlag);
    stmt->setUInt32(1, GetAccountId());
    LoginDatabase.Execute(stmt);
}

ObjectGuid WorldSession::GetAccountGUID() const
{
    return ObjectGuid::Create<HighGuid::WowAccount>(GetAccountId());
}

ObjectGuid  WorldSession::GetBattlenetAccountGUID() const
{
    return ObjectGuid::Create<HighGuid::BNetAccount>(GetAccountId());
}

class AccountInfoQueryHolderPerRealm : public SQLQueryHolder
{
public:
    enum
    {
        GLOBAL_ACCOUNT_DATA = 0,
        TUTORIALS,
        ACHIEVEMENTS,

        MAX_QUERIES
    };

    AccountInfoQueryHolderPerRealm()
    {
        SetSize(MAX_QUERIES);
    }

    bool Initialize(uint32 accountId)
    {
        bool ok = true;

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_DATA);
        stmt->setUInt32(0, accountId);
        ok = SetPreparedQuery(GLOBAL_ACCOUNT_DATA, stmt) && ok;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_TUTORIALS);
        stmt->setUInt32(0, accountId);
        ok = SetPreparedQuery(TUTORIALS, stmt) && ok;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_ACHIEVEMENTS);
        stmt->setUInt32(0, accountId);
        ok = SetPreparedQuery(ACHIEVEMENTS, stmt) && ok;

        return ok;
    }
};

class AccountInfoQueryHolder : public SQLQueryHolder
{
public:
    enum
    {
        GLOBAL_REALM_CHARACTER_COUNTS   = 0,
        GLOBAL_REALM_CHARACTER_TEMPLATE = 1,

        MAX_QUERIES
    };

    AccountInfoQueryHolder()
    {
        SetSize(MAX_QUERIES);
    }

    bool Initialize(uint32 accountId, uint32 _realmID)
    {
        bool ok = true;

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_CHARACTER_COUNTS_BY_ACCOUNT_ID);
        stmt->setUInt32(0, accountId);
        ok = SetPreparedQuery(GLOBAL_REALM_CHARACTER_COUNTS, stmt) && ok;

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_CHARACTER_TEMPLATE);
        stmt->setUInt32(0, accountId);
        stmt->setUInt32(1, _realmID);
        ok = SetPreparedQuery(GLOBAL_REALM_CHARACTER_TEMPLATE, stmt) && ok;

        return ok;
    }
};

void WorldSession::InitializeSession()
{
    auto realmHolder = new AccountInfoQueryHolderPerRealm();
    if (!realmHolder->Initialize(GetAccountId()))
    {
        delete realmHolder;
        SendAuthResponse(ERROR_INTERNAL, false);
        return;
    }

    auto holder = new AccountInfoQueryHolder();
    if (!holder->Initialize(GetAccountId(), _realmID))
    {
        delete realmHolder;
        delete holder;
        SendAuthResponse(ERROR_INTERNAL, false);
        return;
    }

    _realmAccountLoginCallback = CharacterDatabase.DelayQueryHolder(realmHolder);
    _accountLoginCallback = LoginDatabase.DelayQueryHolder(holder);
}

void WorldSession::InitializeSessionCallback(SQLQueryHolder* realmHolder, SQLQueryHolder* holder)
{
    LoadAccountData(realmHolder->GetPreparedResult(AccountInfoQueryHolderPerRealm::GLOBAL_ACCOUNT_DATA), GLOBAL_CACHE_MASK);
    LoadTutorialsData(realmHolder->GetPreparedResult(AccountInfoQueryHolderPerRealm::TUTORIALS));
    LoadCharacterTemplates(holder->GetPreparedResult(AccountInfoQueryHolder::GLOBAL_REALM_CHARACTER_TEMPLATE));
    LoadAchievement(realmHolder->GetPreparedResult(AccountInfoQueryHolderPerRealm::ACHIEVEMENTS));

    if (!m_inQueue)
        SendAuthResponse(ERROR_OK, false);
    else
        SendAuthWaitQue(0);

    SetInQueue(false);
    ResetTimeOutTime();

    SendSetTimeZoneInformation();
    SendFeatureSystemStatusGlueScreen();
    SendClientCacheVersion(sWorld->getIntConfig(CONFIG_CLIENTCACHE_VERSION));
    SendHotfixList(int32(sWorld->getIntConfig(CONFIG_HOTFIX_CACHE_VERSION)));
    SendTutorialsData();
    SendDisplayPromo();

    if (PreparedQueryResult characterCountsResult = holder->GetPreparedResult(AccountInfoQueryHolder::GLOBAL_REALM_CHARACTER_COUNTS))
    {
        do
        {
            Field* fields = characterCountsResult->Fetch();
            _realmCharacterCounts[Battlenet::RealmHandle{ fields[3].GetUInt8(), fields[4].GetUInt8(), fields[2].GetUInt32() }.GetAddress()] = fields[1].GetUInt8();

        }
        while (characterCountsResult->NextRow());
    }

    SendPacket(WorldPackets::Battlenet::SetSessionState(1).Write());

    bool wardenActive = sWorld->getBoolConfig(CONFIG_WARDEN_ENABLED);
    if (wardenActive)
    {
        if (_warden && _warden->GetState() == WARDEN_MODULE_NOT_LOADED)
            _warden->ConnectToMaievModule();
    }

    delete realmHolder;
    delete holder;
}

void WorldSession::SetPersonalXPRate(float rate)
{
    if (!rate)
        LoginDatabase.PExecute("delete from `account_rates` where realm = %u and account = %u", sWorld->GetRealmId(), GetAccountId());
    else
        LoginDatabase.PExecute("REPLACE INTO `account_rates` (`account`, `realm`, `rate`) VALUES ('%u', '%u', '%f');", GetAccountId(), sWorld->GetRealmId(), rate);
    
    PersonalXPRate = rate;
}

CharacterTemplateData* WorldSession::GetCharacterTemplateData(uint32 id)
{
    return Trinity::Containers::MapGetValuePtr(charTemplateData, id);
}

bool WorldSession::HasAchievement(uint32 achievementId)
{
    if (achievementId >= m_achievement.size())
        return false;

    return m_achievement[achievementId];
}

void WorldSession::SetAchievement(uint32 achievementId)
{
    if (achievementId >= m_achievement.size())
        return;

    m_achievement[achievementId] = true;
}

void WorldSession::LoadAchievement(PreparedQueryResult const& result)
{
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        SetAchievement(fields[1].GetUInt32());
    }
    while (result->NextRow());
}

void WorldSession::ProcessAnticheatAction(const char* detector, const char* reason, uint32 cheatAction, uint32 banSeconds)
{
    const char* action = "";
    if (cheatAction & CHEAT_ACTION_BAN_IP_ACCOUNT)
    {
        action = "Account+IP banned.";
        if (GetSecurity() == SEC_PLAYER)
        {
            std::string _reason = std::string("CHEAT") + ": " + reason;
            sWorld->BanAccount(BAN_ACCOUNT, _player->GetName(), secsToTimeString(banSeconds, true), _reason, detector);
            std::stringstream banIpReason;
            banIpReason << "Cf account " << _player->GetName();
            sWorld->BanAccount(BAN_IP, GetRemoteAddress(), secsToTimeString(banSeconds, true), banIpReason.str().c_str(), detector);
        }
    }
    else if (cheatAction & CHEAT_ACTION_BAN_ACCOUNT)
    {
        action = "Banned.";
        std::string _reason = std::string("CHEAT") + ": " + reason;
        if (GetSecurity() == SEC_PLAYER)
            sWorld->BanAccount(BAN_ACCOUNT, _player->GetName(), secsToTimeString(banSeconds, true), _reason, detector);
    }
    else if (cheatAction & CHEAT_ACTION_KICK)
    {
        action = "Kicked.";
        if (GetSecurity() == SEC_PLAYER)
            KickPlayer();
    }
    else if (cheatAction & CHEAT_ACTION_REPORT_GMS)
        action = "Announced to GMs.";
    else if (!(cheatAction & CHEAT_ACTION_LOG))
        return;

    if (cheatAction & CHEAT_ACTION_REPORT_GMS)
    {
        std::stringstream oss;
        ObjectGuid pguid;
        if (Player* p = GetPlayer())
            pguid = p->GetGUID();
        else
            oss << "[Account " << _player->GetName() << "]";
        oss << reason;
        if (cheatAction >= CHEAT_ACTION_KICK)
            oss << " " << action;

        //if (GetSecurity() == SEC_PLAYER)
          //  ChannelMgr::AnnounceBothFactionsChannel(detector, pguid, oss.str().c_str());
    }
    std::string playerDesc;
    if (_player)
        playerDesc = _player->GetShortDescription();
    else
    {
        std::stringstream oss;
        oss << "<None> [" << _player->GetName() << ":" << GetAccountId() << "@" << GetRemoteAddress().c_str() << "]";
        playerDesc = oss.str();
    }
    sLog->outAnticheat("%s %s: %s %s", playerDesc.c_str(), detector, reason, action);
}

void WorldSession::LookupPlayerSearchCommand(PreparedQueryResult result, int32 limit)
{
    ChatHandler chH = ChatHandler(this);

    if (!result)
    {
        chH.PSendSysMessage(LANG_NO_PLAYERS_FOUND);
        chH.SetSentErrorMessage(true);
        return;
    }

    int32 counter = 0;
    uint32 count = 0;
    uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

    do
    {
        if (maxResults && count++ == maxResults)
        {
            chH.PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
            return;
        }

        Field* fields           = result->Fetch();
        uint32 accountId        = fields[0].GetUInt32();
        std::string accountName = fields[1].GetString();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_GUID_NAME_BY_ACC);
        stmt->setUInt32(0, accountId);
        PreparedQueryResult result2 = CharacterDatabase.Query(stmt);

        chH.PSendSysMessage(LANG_LOOKUP_PLAYER_ACCOUNT, accountName.c_str(), accountId);

        { // baninfo
            std::string bannedby = "unknown";
            std::string banreason = "";
            int64 banTime = -1;

            PreparedStatement* stmt1 = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO_BANS);
            stmt1->setUInt32(0, accountId);
            PreparedQueryResult result3 = LoginDatabase.Query(stmt1);
            if (result3)
            {
                Field* fields = result3->Fetch();
                banTime = int64(fields[1].GetBool() ? 0 : fields[0].GetUInt32());
                bannedby = fields[2].GetString();
                banreason = fields[3].GetString();
            }

            if (banTime >= 0)
                chH.PSendSysMessage(LANG_PINFO_BAN, banTime > 0 ? secsToTimeString(banTime - time(NULL), true).c_str() : "permanently", bannedby.c_str(), banreason.c_str());
        }

        if (result2)
        {
            do
            {
                Field* characterFields  = result2->Fetch();
                uint32 guid             = characterFields[0].GetUInt32();
                std::string name        = characterFields[1].GetString();

                if (sObjectMgr->GetPlayerByLowGUID(guid) != NULL) // online
                    chH.PSendSysMessage(363, name.c_str(), guid);
                else
                    chH.PSendSysMessage(LANG_LOOKUP_PLAYER_CHARACTER, name.c_str(), guid);
                ++counter;
            }
            while (result2->NextRow() && (limit == -1 || counter < limit));
        }
    }
    while (result->NextRow());

    if (counter == 0) // empty accounts only
    {
        chH.PSendSysMessage(LANG_NO_PLAYERS_FOUND);
        chH.SetSentErrorMessage(true);
        return;
    }

    return;
}

void WorldSession::BanListHelper(PreparedQueryResult result)
{
    ChatHandler chH = ChatHandler(this);

    if (!result)
    {
        chH.PSendSysMessage(LANG_BANLIST_NOACCOUNT);
        return;
    }

    chH.PSendSysMessage(LANG_BANLIST_MATCHINGACCOUNT);

    // Chat short output
    if (chH.GetSession())
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 accountid = fields[0].GetUInt32();

            QueryResult banResult = LoginDatabase.PQuery("SELECT account.username FROM account, account_banned WHERE account_banned.id='%u' AND account_banned.id=account.id", accountid);
            if (banResult)
            {
                Field* fields2 = banResult->Fetch();
                chH.PSendSysMessage("%s", fields2[0].GetCString());
            }
        }
        while (result->NextRow());
    }
    // Console wide output
    else
    {
        chH.SendSysMessage(LANG_BANLIST_ACCOUNTS);
        chH.SendSysMessage(" ===============================================================================");
        chH.SendSysMessage(LANG_BANLIST_ACCOUNTS_HEADER);
        do
        {
            chH.SendSysMessage("-------------------------------------------------------------------------------");
            Field* fields = result->Fetch();
            uint32 accountId = fields[0].GetUInt32();

            std::string accountName;

            // "account" case, name can be get in same query
            if (result->GetFieldCount() > 1)
                accountName = fields[1].GetString();
            // "character" case, name need extract from another DB
            else
                AccountMgr::GetName(accountId, accountName);

            // No SQL injection. id is uint32.
            QueryResult banInfo = LoginDatabase.PQuery("SELECT bandate, unbandate, bannedby, banreason FROM account_banned WHERE id = %u ORDER BY unbandate", accountId);
            if (banInfo)
            {
                Field* fields2 = banInfo->Fetch();
                do
                {
                    time_t timeBan = time_t(fields2[0].GetUInt32());
                    tm tmBan;
                    localtime_r(&timeBan, &tmBan);

                    if (fields2[0].GetUInt32() == fields2[1].GetUInt32())
                    {
                        chH.PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                            accountName.c_str(), tmBan.tm_year%100, tmBan.tm_mon+1, tmBan.tm_mday, tmBan.tm_hour, tmBan.tm_min,
                            fields2[2].GetCString(), fields2[3].GetCString());
                    }
                    else
                    {
                        time_t timeUnban = time_t(fields2[1].GetUInt32());
                        tm tmUnban;
                        localtime_r(&timeUnban, &tmUnban);
                        chH.PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                            accountName.c_str(), tmBan.tm_year%100, tmBan.tm_mon+1, tmBan.tm_mday, tmBan.tm_hour, tmBan.tm_min,
                            tmUnban.tm_year%100, tmUnban.tm_mon+1, tmUnban.tm_mday, tmUnban.tm_hour, tmUnban.tm_min,
                            fields2[2].GetCString(), fields2[3].GetCString());
                    }
                }
                while (banInfo->NextRow());
            }
        }
        while (result->NextRow());

        chH.SendSysMessage(" ===============================================================================");
    }

    return;
}
