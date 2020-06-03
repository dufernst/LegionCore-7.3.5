/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#include <openssl/sha.h>

#include "AccountMgr.h"
#include "ByteBuffer.h"
#include "ChatPackets.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "MiscPackets.h"
#include "Opcodes.h"
#include "SessionKeyGeneration.h"
#include "SHA1.h"
#include "Util.h"
#include "Warden.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"

Warden::Warden(WorldSession* session) : _session(session), _state(WARDEN_NOT_INITIALIZED), _currentSessionFlagged(false), _checkSequenceIndex(0)
{
    _lastUpdateTime = getMSTime();
    _currentModule = nullptr;

    CreateTimers();
}

Warden::~Warden()
{
    for (auto & itr : _timers)
        delete itr.second;
}

void Warden::TestFunc()
{
}

bool Warden::Create(BigNumber *k)
{
    if (!_currentModule)
    {
        sLog->outWarden("Current module is not found. Abort Warden (%s)! Account %u (%s)", _session->GetOS().c_str(), _session->GetAccountId(), _session->GetAccountName().c_str());
        return false;
    }

    uint8 tempClientKeySeed[16];
    uint8 tempServerKeySeed[16];

    SessionKeyGenerator<SHA1Hash> WK(k->AsByteArray().get(), k->GetNumBytes());
    WK.Generate(tempClientKeySeed, 16);
    WK.Generate(tempServerKeySeed, 16);

    ARC4::rc4_init(&_clientRC4State, tempClientKeySeed, 16);
    ARC4::rc4_init(&_serverRC4State, tempServerKeySeed, 16);

    //sLog->outWarden("Module Key: %s", ByteArrayToHexStr(_currentModule->Key, 16).c_str());
    //sLog->outWarden("Module ID: %s", ByteArrayToHexStr(_currentModule->ID, 32).c_str());
    //RequestModule();

    //TC_LOG_DEBUG(LOG_FILTER_WARDEN, "Server side warden for client %u initializing...", session->GetAccountId());
    //TC_LOG_DEBUG(LOG_FILTER_WARDEN, "C->S Key: %s", ByteArrayToHexStr(_inputKey, 16).c_str());
    //TC_LOG_DEBUG(LOG_FILTER_WARDEN, "S->C Key: %s", ByteArrayToHexStr(_outputKey, 16).c_str());
    //TC_LOG_DEBUG(LOG_FILTER_WARDEN, "Seed: %s", ByteArrayToHexStr(_seed, 16).c_str());
    //TC_LOG_DEBUG(LOG_FILTER_WARDEN, "Loading Module...");
    SetNewState(WARDEN_MODULE_NOT_LOADED);

    return true;
}

void Warden::ConnectToMaievModule()
{
    SetNewState(WARDEN_MODULE_CONNECTING_TO_MAIEV);

    uint8 baseChallengeSeed[16] = { 0xDD, 0xA2, 0x7D, 0x02, 0xA3, 0x9B, 0x81, 0x3D, 0x55, 0x15, 0xC8, 0xA5, 0x17, 0xF1, 0xE2, 0x4D };

    ByteBuffer buff;
    buff << uint8(WARDEN_SMSG_HASH_REQUEST);
    buff.append(baseChallengeSeed, 16);

    // Encrypt with warden RC4 key
    EncryptData(const_cast<uint8*>(buff.contents()), buff.size());

    WorldPacket pkt(SMSG_WARDEN_DATA, buff.size() + sizeof(uint32));
    pkt << uint32(buff.size());
    pkt.append(buff);
    _session->SendPacket(&pkt);
}

void Warden::RequestModule()
{
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: Request module(0x00)");

    SetNewState(WARDEN_MODULE_REQUESTING);

    // Create packet structure
    WardenModuleUse request;
    request.Command = WARDEN_SMSG_MODULE_USE;

    memcpy(request.ModuleId, _currentModule->ID, 32);
    memcpy(request.ModuleKey, _currentModule->Key, 16);
    request.Size = _currentModule->CompressedSize;

    // Encrypt with warden RC4 key.
    EncryptData((uint8*)&request, sizeof(WardenModuleUse));

    WorldPacket pkt(SMSG_WARDEN_DATA, sizeof(WardenModuleUse) + sizeof(uint32));
    pkt << uint32(sizeof(WardenModuleUse));
    pkt.append((uint8*)&request, sizeof(WardenModuleUse));
    _session->SendPacket(&pkt);

    RestartTimer(5, 1 * MINUTE * IN_MILLISECONDS);
}

void Warden::SendModuleToClient()
{
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: Send module to client(0x01)");

    // CMSG_MODULE_MISSING, change state and stop timer
    SetNewState(WARDEN_MODULE_SENDING);
    StopTimer(5);

    // Create packet structure
    WardenModuleTransfer packet;

    uint32 sizeLeft = _currentModule->CompressedSize;
    uint32 pos = 0;
    uint16 chunkSize = 0;
    while (sizeLeft > 0)
    {
        chunkSize = sizeLeft < 500 ? sizeLeft : 500;
        packet.Command = WARDEN_SMSG_MODULE_CACHE;
        packet.ChunkSize = chunkSize;
        memcpy(packet.Data, &_currentModule->CompressedData[pos], chunkSize);
        sizeLeft -= chunkSize;
        pos += chunkSize;

        uint16 packetSize = sizeof(uint8) + sizeof(uint16) + chunkSize;
        EncryptData((uint8*)&packet, packetSize);

        WorldPacket pkt(SMSG_WARDEN_DATA, packetSize + sizeof(uint32));
        pkt << uint32(packetSize);
        pkt.append((uint8*)&packet, packetSize);
        _session->SendPacket(&pkt);
    }

    RestartTimer(6, 1 * MINUTE * IN_MILLISECONDS);
}

void Warden::RequestHash()
{
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: Request hash(0x05)");

    SetNewState(WARDEN_MODULE_SEND_CHALLENGE);

    // Create packet structure
    WardenHashRequest Request;
    Request.Command = WARDEN_SMSG_HASH_REQUEST;
    memcpy(Request.Seed, _currentModule->Seed, 16);

    // Encrypt with warden RC4 key.
    EncryptData((uint8*)&Request, sizeof(WardenHashRequest));

    WorldPacket pkt(SMSG_WARDEN_DATA, sizeof(WardenHashRequest) + sizeof(uint32));
    pkt << uint32(sizeof(WardenHashRequest));
    pkt.append((uint8*)&Request, sizeof(WardenHashRequest));
    _session->SendPacket(&pkt);

    RestartTimer(7, 1 * MINUTE * IN_MILLISECONDS);
}

void Warden::Update()
{
    uint32 currentUpdateTime = getMSTime();
    const uint32 diff = currentUpdateTime - _lastUpdateTime;
    _lastUpdateTime = currentUpdateTime;

    UpdateTimers(diff);
    //LogTimers();
}

void Warden::CreateTimers()
{
    uint32 reqLevel = sWorld->getIntConfig(CONFIG_WARDEN_BASIC_SECURITY_LEVEL_REQ);

    CreateTimer(1, 20 * IN_MILLISECONDS, reqLevel); // check timer (0x01)
    CreateTimer(2, 1 * MINUTE * IN_MILLISECONDS, reqLevel); // client response timer (0x02)
    CreateTimer(3, 10 * IN_MILLISECONDS, 0, false); // pending kick timer (0x04)
    CreateTimer(4, 15 * IN_MILLISECONDS, reqLevel); // send challenge timer after login in world (0x08)
    CreateTimer(5, 1 * MINUTE * IN_MILLISECONDS, 0, false); // state WARDEN_MODULE_REQUESTING (wait CMSG_MODULE_MISSING / CMSG_MODULE_OK) (0x10)
    CreateTimer(6, 1 * MINUTE * IN_MILLISECONDS, 0, false); // state WARDEN_MODULE_SENDING (wait CMSG_MODULE_OK) (0x20)
    CreateTimer(7, 1 * MINUTE * IN_MILLISECONDS, 0, false); // state WARDEN_MODULE_SEND_CHALLENGE (wait CMSG_HASH_RESULT) (0x40)
    CreateTimer(8, 35 * IN_MILLISECONDS, 60); // extended check timer (0x80)
}

void Warden::CreateTimer(uint32 id, uint32 time, uint32 reqLevel, bool reqPlayerInWorld)
{
    WardenTimer* timer = new WardenTimer(time, reqLevel, reqPlayerInWorld);
    _timers[id] = timer;
}

void Warden::UpdateTimers(const uint32 diff)
{
    _wardenTimerUpdate.lock();

    for (auto & itr : _timers)
    {
        if (WardenTimer* timer = itr.second)
        {
            if (!timer->Active())
                continue;

            Player* player = _session->GetPlayer();

            if (!timer->CheckConditions(player))
                continue;

            if (timer->Expired(diff))
            {
                timer->Stop();
                DoAction(itr.first, diff);
            }
            else
                timer->Continue(diff);
        }
    }

    _wardenTimerUpdate.unlock();
}

void Warden::StartTimers(uint32 mask)
{
    for (auto & itr : _timers)
    {
        if (mask & (1 << (itr.first - 1)))
            if (WardenTimer* timer = itr.second)
                timer->Start();
    }
}

void Warden::StopTimers(uint32 mask)
{
    for (auto & itr : _timers)
    {
        if (mask & (1 << (itr.first - 1)))
            if (WardenTimer* timer = itr.second)
                timer->Stop();
    }
}

void Warden::LogTimers(const uint32 diff)
{
    sLog->outWarden("WARDEN TIMERS LOG BEGIN, %s with latency %u, IP %s and module state %s (%s), diff %u", _session->GetPlayerName(false).c_str(), _session->GetLatency(),
        _session->GetRemoteAddress().c_str(), GetStateString().c_str(), _session->GetOS().c_str(), diff);

    for (auto & itr : _timers)
    {
        if (WardenTimer* timer = itr.second)
            sLog->outWarden("Timer %u : active %s, currentTime %u", itr.first, timer->Active() ? "true" : "false", timer->GetCurrentTime());
    }

    sLog->outWarden("WARDEN TIMERS LOG END");
}

void Warden::DoAction(uint32 id, const uint32 diff)
{
    bool kick = false;

    switch (id)
    {
        case 1:
        {
            RequestBaseData();
            break;
        }
        case 2:
        {
            sLog->outWarden("%s with latency %u, IP %s and module state %s lost Warden module (%s) connection with lastPacketTime: server %u, client %u - disconnecting client",
                _session->GetPlayerName(false).c_str(), _session->GetLatency(), _session->GetRemoteAddress().c_str(), GetStateString().c_str(), _session->GetOS().c_str(), _lastPacketSendTime, _lastPacketRecvTime);
            kick = true;
            break;
        }
        case 3:
        {
            kick = true;
            break;
        }
        case 4:
        {
            RequestHash();
            break;
        }
        case 5:
        case 6:
        case 7:
        {
            sLog->outWarden("%s with latency %u, IP %s and module state %s exceeded Warden module (%s, timer %u) response delay for more than 1 minute with lastPacketTime: server %u, client %u - disconnecting client",
                _session->GetPlayerName(false).c_str(), _session->GetLatency(), _session->GetRemoteAddress().c_str(), GetStateString().c_str(), _session->GetOS().c_str(), id, _lastPacketSendTime, _lastPacketRecvTime);
            //LogTimers(diff);
            kick = true;
            break;
        }
        case 8:
        {
            SendExtendedData();
            break;
        }
    }

    if (kick)
        _session->KickPlayer();
}

void Warden::DecryptData(uint8* buffer, uint32 length)
{
    ARC4::rc4_process(&_clientRC4State, buffer, length);
}

void Warden::EncryptData(uint8* buffer, uint32 length)
{
    ARC4::rc4_process(&_serverRC4State, buffer, length);
}

bool Warden::IsValidCheckSum(uint32 checksum, const uint8* data, const uint16 length)
{
    uint32 newChecksum = BuildChecksum(data, length);

    if (checksum != newChecksum)
        return false;
    return true;
}

uint32 Warden::BuildChecksum(const uint8* data, uint32 length)
{
    uint8 hash[20];
    SHA1(data, length, hash);
    uint32 checkSum = 0;
    for (uint8 i = 0; i < 5; ++i)
        checkSum = checkSum ^ *(uint32*)(&hash[0] + i * 4);

    return checkSum;
}

void Warden::SetPlayerLocked(uint16 checkId)
{
    SetNewState(WARDEN_MODULE_SET_PLAYER_LOCK);

    std::string message = "Anticheat: Unknown internal error";

    WardenCheck* wd = _wardenMgr->GetCheckDataById(checkId);

    if (wd)
    {
        switch (wd->Type)
        {
            case MPQ_CHECK: message = "Anticheat: Banned MPQ patches detected."; break;
            case LUA_STR_CHECK: message = "Anticheat: Banned addons detected."; break;
            case PAGE_CHECK_A:
            case PAGE_CHECK_B:
            {
                message = "Anticheat: Injected cheat code detected.";
                break;
            }
            case MEM_CHECK: message = "Anticheat: Unknown cheat detected."; break;
            case MODULE_CHECK: message = "Anticheat: Banned DLLs detected."; break;
            case PROC_CHECK: message = "Anticheat: API hooks detected."; break;
            default: message = "Anticheat: Unknown suspicious program detected."; break;
        }

        if (wd->BanReason != "")
            message = "Anticheat: " + wd->BanReason + " detected.";
    }

    if (Player* player = _session->GetPlayer())
    {
        WorldPackets::Chat::Chat packet;
        packet.Initialize(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, player, player, message);
        player->SendDirectMessage(packet.Write());
    }

    StopTimers(0xFF);
    RestartTimer(3, 10 * IN_MILLISECONDS);
}

void Warden::KickPlayer()
{
    SetNewState(WARDEN_MODULE_SET_PLAYER_LOCK);
    StopTimers(0xFF);
    RestartTimer(3, 2 * IN_MILLISECONDS);
}

std::string Warden::Penalty(uint16 checkId)
{
    WardenCheck* check = _wardenMgr->GetCheckDataById(checkId);

    if (check)
    {
        switch (check->Action)
        {
            case WARDEN_ACTION_LOG:
                return "None";
            case WARDEN_ACTION_INSTANT_KICK:
            {
                KickPlayer();
                return "Kick (instant)";
            }
            case WARDEN_ACTION_BAN:
            {
                std::stringstream duration;
                // calculate ban time
                uint32 banTime = CalcBanTime();
                // warden_overrides
                if (check->BanTime != 0xFFFFFFFF) // default
                    banTime = check->BanTime;

                if (banTime == 0)
                    duration << "-1";
                else
                    duration << banTime << "s";

                std::string accountName;
                AccountMgr::GetName(_session->GetAccountId(), accountName);
                std::stringstream banReason;

                if (check->BanReason == "")
                    banReason << "Unknown Hack" << " (CheckId: " << checkId << ") (Realm: " << sWorld->GetRealmName() << ")";
                else
                    banReason << check->BanReason << " (CheckId: " << checkId << ") (Realm: " << sWorld->GetRealmName() << ")";

                sWorld->BanAccount(BAN_ACCOUNT, accountName, duration.str(), banReason.str(), "Anticheat", true);
                KickPlayer();
                return "Ban";
            }
            case WARDEN_ACTION_PENDING_KICK:
            {
                SetPlayerLocked(checkId);
                return "Kick (with message)";
            }
            case WARDEN_ACTION_FLAG_ACCOUNT:
            {
                std::stringstream duration;
                // get ban time for next ban apply
                uint32 banTime = GetBanTime();
                // warden_overrides
                if (check->BanTime != 0xFFFFFFFF) // default
                    banTime = check->BanTime;

                std::string accountName;
                AccountMgr::GetName(_session->GetAccountId(), accountName);
                std::stringstream banReason;

                if (check->BanReason == "")
                    banReason << "Unknown Hack" << " (CheckId: " << checkId << ") (Realm: " << sWorld->GetRealmName() << ")";
                else
                    banReason << check->BanReason << " (CheckId: " << checkId << ") (Realm: " << sWorld->GetRealmName() << ")";

                sWorld->FlagAccount(_session->GetAccountId(), banTime, banReason.str(), "Anticheat");
                return "FlagAccount";
            }
            default:
                break;
        }
    }

    // impossible, but..
    KickPlayer();
    return "Unknown Error";
}

uint32 Warden::CalcBanTime()
{
    uint32 accountId = _session->GetAccountId();
    uint32 banTime = 72 * HOUR;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_WARDEN_BAN_ATTEMPTS);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_WARDEN_BAN_ATTEMPTS);
        stmt->setUInt32(0, accountId);
        stmt->setUInt16(1, 1);
        CharacterDatabase.Execute(stmt);
        return banTime;
    }

    Field* fields = result->Fetch();
    uint16 banAttempts = fields[0].GetUInt16();

    // TODO: while harcoded
    switch (banAttempts)
    {
        case 1: banTime = 2 * WEEK; break;
        case 2: banTime = MONTH; break;
        case 3: banTime = 2 * MONTH; break;
        case 4: banTime = 3 * MONTH; break;
        default:
            banTime = 0; break;
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_WARDEN_BAN_ATTEMPTS);
    stmt->setUInt32(0, accountId);
    CharacterDatabase.Execute(stmt);

    return banTime;
}

uint32 Warden::GetBanTime()
{
    uint32 accountId = _session->GetAccountId();
    uint32 banTime = 72 * HOUR;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_WARDEN_BAN_ATTEMPTS);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return banTime;

    Field* fields = result->Fetch();
    uint16 banAttempts = fields[0].GetUInt16();

    // TODO: while harcoded
    switch (banAttempts)
    {
        case 1: banTime = 2 * WEEK; break;
        case 2: banTime = MONTH; break;
        case 3: banTime = 2 * MONTH; break;
        case 4: banTime = 3 * MONTH; break;
        default:
            banTime = 0; break;
    }

    return banTime;
}

std::string Warden::GetStateString()
{
    WardenState state = GetState();

    switch (state)
    {
        case WARDEN_NOT_INITIALIZED: return "WARDEN_NOT_INITIALIZED";
        case WARDEN_MODULE_NOT_LOADED: return "WARDEN_MODULE_NOT_LOADED";
        case WARDEN_MODULE_CONNECTING_TO_MAIEV: return "WARDEN_MODULE_CONNECTING_TO_MAIEV";
        case WARDEN_MODULE_REQUESTING: return "WARDEN_MODULE_REQUESTING";
        case WARDEN_MODULE_SENDING: return "WARDEN_MODULE_SENDING";
        case WARDEN_MODULE_LOADED: return "WARDEN_MODULE_LOADED";
        case WARDEN_MODULE_SEND_CHALLENGE: return "WARDEN_MODULE_SEND_CHALLENGE";
        case WARDEN_MODULE_READY: return "WARDEN_MODULE_READY";
        case WARDEN_MODULE_SET_PLAYER_LOCK: return "WARDEN_MODULE_SET_PLAYER_LOCK";
        default: return "WARDEN_MODULE_ERROR_STRING";
    }
}

void WorldSession::HandleTwitterConnect(WorldPackets::Misc::TwitterConnect& /*packet*/)
{
    // clear money and some currency - TEMP disabled
    /*_player->SetMoney(0);
    _player->SetCurrency(1220, 0);
    _player->SetCurrency(1224, 0);
    _player->SetCurrency(1342, 0);*/

    //sLog->outWarden("%s failed Warden check %u. Action: %s", GetPlayerName(false).c_str(), 51, _warden->Penalty(51).c_str());
}

void WorldSession::HandleTwitterDisconnect(WorldPackets::Misc::TwitterDisconnect& /*packet*/)
{
    //_warden->Penalty(100);
}

void WorldSession::HandleResetChallengeModeCheat(WorldPackets::Misc::ResetChallengeModeCheat& /*packet*/)
{
    sLog->outWarden("%s failed Warden check %u. Action: %s", GetPlayerName(false).c_str(), 52, _warden->Penalty(52).c_str());
}