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

#include "HmacHash.h"
#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "Database/DatabaseEnv.h"
#include "World.h"
#include "Player.h"
#include "Util.h"
#include "WardenWin.h"
#include "WardenMgr.h"
#include "AccountMgr.h"
#include "SpellAuraEffects.h"

WardenWin::WardenWin(WorldSession* session) : Warden(session)
{
    if (session->GetOS() == "Win")
        _currentModule = _wardenMgr->GetModuleById("F0582A2E364D2445CE85FAC4953B3F7C742A3F487BDCF2467E7CED508D9C1C12", "Win");  //stable last
    else
        _currentModule = _wardenMgr->GetModuleById("6680ABDF47865365A331628F97022DEDAA3C523D17A1A85394B7002C8E6161F3", "Wn64");
}

void WardenWin::InitializeModule()
{
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: Initialize module(0x03)");

    ByteBuffer buff;
    //InitializeMPQCheckFunc(buff);
    InitializeLuaCheckFunc(buff);
    //InitializeTimeCheckFunc(buff);

    // Encrypt with warden RC4 key.
    EncryptData(const_cast<uint8*>(buff.contents()), buff.size());

    WorldPacket pkt(SMSG_WARDEN_DATA, buff.size() + sizeof(uint32));
    pkt << uint32(buff.size());
    pkt.append(buff);
    _session->SendPacket(&pkt);
}

void WardenWin::ActivateModule()
{
    StopTimers(0x30);

    SetNewState(WARDEN_MODULE_LOADED);
    RestartTimer(4, urand(15, 25) * IN_MILLISECONDS);
}

void WardenWin::InitializeMPQCheckFunc(ByteBuffer& buff)
{
    buff << uint8(WARDEN_SMSG_MODULE_INITIALIZE);
    buff << uint16(20);

    WardenInitModuleMPQFunc request;
    request.Type = 1;
    request.Flag = 0;
    request.MpqFuncType = 2;
    request.StringBlock = 0;
    request.OpenFile = 0x0001466C;
    request.GetFileSize = 0x00012048;
    request.ReadFile = 0x00012EF4;
    request.CloseFile = 0x000137A8;

    buff << uint32(BuildChecksum((uint8*)&request, 20));
    buff.append((uint8*)&request, sizeof(WardenInitModuleMPQFunc));
}

void WardenWin::InitializeLuaCheckFunc(ByteBuffer& buff)
{
    buff << uint8(WARDEN_SMSG_MODULE_INITIALIZE);
    buff << uint16(12);

    WardenInitModuleLUAFunc request;
    request.Type = 4;
    request.Flag = 0;
    request.StringBlock = 0;
    request.GetText = 0x000B39CC;
    request.UnkFunction = 0x0032A57E;
    request.LuaFuncType = 1;

    buff << uint32(BuildChecksum((uint8*)&request, 12));
    buff.append((uint8*)&request, sizeof(WardenInitModuleLUAFunc));
}

void WardenWin::InitializeTimeCheckFunc(ByteBuffer& buff)
{
    buff << uint8(WARDEN_SMSG_MODULE_INITIALIZE);
    buff << uint16(8);

    WardenInitModuleTimeFunc request;
    request.Type = 1;
    request.Flag = 1;
    request.StringBlock = 0;
    request.PerfCounter = 0x0010D627;
    request.TimeFuncType = 1;

    buff << uint32(BuildChecksum((uint8*)&request, 8));
    buff.append((uint8*)&request, sizeof(WardenInitModuleTimeFunc));
}

void WardenWin::HandleHashResult(ByteBuffer &buff)
{
    StopTimer(7);

    uint8 clientSeedHash[20];
    buff.read(clientSeedHash, 20);

    std::string hashStr = _wardenMgr->ByteArrayToString(clientSeedHash, 20);
    std::string realHashStr = _wardenMgr->ByteArrayToString(_currentModule->ClientKeySeedHash, 20);

    if (strcmp(hashStr.c_str(), realHashStr.c_str()))
    {
        sLog->outWarden("%s failed hash reply. Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: Request hash reply - succeeded");

    ARC4::rc4_init(&_clientRC4State, _currentModule->ClientKeySeed, 16);
    ARC4::rc4_init(&_serverRC4State, _currentModule->ServerKeySeed, 16);

    SetNewState(WARDEN_MODULE_READY);
    RestartTimer(1, 1 * MINUTE * IN_MILLISECONDS);
    if (_session->GetOS() == "Win")
        RestartTimer(8, 35 * IN_MILLISECONDS);
}

void WardenWin::HandleHashResultSpecial(ByteBuffer &buff)
{
    uint8 clientSeedHash[20];
    buff.read(clientSeedHash, 20);

    std::string hashStr = _wardenMgr->ByteArrayToString(clientSeedHash, 20);
    std::string realHashStr = _session->GetOS() == "Win" ? "20046B327D453A4601630C40571E955D9D8DE8C7" : "D88209ABAFB8AAC1973461DB06E4C7CCBE02C265";

    if (strcmp(hashStr.c_str(), realHashStr.c_str()))
    {
        sLog->outWarden("%s failed special hash reply. Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    uint8 baseClientKeySeed_Win[16] = { 0xB9, 0x9E, 0xEA, 0x36, 0x0C, 0xF4, 0xF1, 0x62, 0xA5, 0xC8, 0xF3, 0x34, 0xC6, 0x0E, 0x6B, 0x2C };
    uint8 baseServerKeySeed_Win[16] = { 0x80, 0x83, 0xF1, 0xFB, 0x31, 0x99, 0xB6, 0x3F, 0x33, 0x02, 0xCE, 0xBD, 0x29, 0x2D, 0x9A, 0x65 };

    uint8 baseClientKeySeed_Wn64[16] = { 0x32, 0x1C, 0xD0, 0xDC, 0x61, 0x56, 0x80, 0x08, 0x77, 0x54, 0xF9, 0xAA, 0x2B, 0xCE, 0x7C, 0x37 };
    uint8 baseServerKeySeed_Wn64[16] = { 0xDD, 0xA2, 0x7D, 0x02, 0x1F, 0x11, 0x7F, 0xD3, 0x99, 0x93, 0x2A, 0xB0, 0x2F, 0xC8, 0xDB, 0x4E };

    ARC4::rc4_init(&_clientRC4State, _session->GetOS() == "Win" ? baseClientKeySeed_Win : baseClientKeySeed_Wn64, 16);
    ARC4::rc4_init(&_serverRC4State, _session->GetOS() == "Win" ? baseServerKeySeed_Win : baseServerKeySeed_Wn64, 16);

    // now request module from server
    RequestModule();
}

void WardenWin::HandleModuleFailed()
{
    sLog->outWarden("%s has received CMSG_WARDEN_MODULE_FAILED. Action: Kick", _session->GetPlayerName(false).c_str());
    KickPlayer();
}

void WardenWin::HandleExtendedData(ByteBuffer &buff)
{
    // destroy EWT
    uint8 result;
    buff >> result;

    // TEST!
    if (!result && !_currentSessionFlagged)
    {
        uint32 nextBanwaveTime = sWorld->getWorldState(WS_BAN_WAVE_TIME);
        nextBanwaveTime ^= 0x1234BEEF;
        sLog->outWarden("%s failed Warden check %u, nextBanwaveTime %u. Action: %s", _session->GetPlayerName(false).c_str(), 53, nextBanwaveTime, Penalty(53).c_str());
        _currentSessionFlagged = true;
    }
}

void WardenWin::HandleStringData(ByteBuffer &buff)
{
    uint16 result;
    buff >> result;

    if (result)
    {
        sLog->outWarden("%s failed Warden check %u, but have unknown error with code %u. Action: None", _session->GetPlayerName(false).c_str(), 51, result);
        return;
    }
    else
    {
        std::string windowClassName;
        buff >> windowClassName;

        // analyze classname
        bool bot = false;
        if (windowClassName.find("WindowsForms10.Window.8.app.0.141b42a_r14_ad1") != std::string::npos)
            bot = true;
        else if (windowClassName.find("HwndWrapper[") != std::string::npos && windowClassName.find(".exe;;") != std::string::npos)
            bot = true;

        if (bot)
            sLog->outWarden("%s failed Warden check %u, botWindowsClassName: %s. Action: %s", _session->GetPlayerName(false).c_str(), 51, windowClassName.c_str(), Penalty(51).c_str());
        else
            sLog->outWarden("%s failed Warden check %u, but bot pattern is not detected, windowsClassName: %s. Action: None", _session->GetPlayerName(false).c_str(), 51, windowClassName.c_str());
    }
}

void WardenWin::HandleFailedSync(uint32 clientSeqIndex)
{
    /*++_clientSyncFailCount;
    if (_clientSyncFailCount < 3)
        sLog->outWarden("%s has received wrong Warden packet with index %u, but need index %u. Total failcount %u and module state %s. Action: None", _session->GetPlayerName(false).c_str(), clientSeqIndex, _sequencePacketIndex, _clientSyncFailCount, GetStateString().c_str());
    else
    {
        _clientSyncFailCount = 0;
        sLog->outWarden("%s lost sync with Warden module (%s), module state %s. Action: Kick", _session->GetPlayerName(false).c_str(), _session->GetOS().c_str(), GetStateString().c_str());
        KickPlayer();
    }*/
}

void WardenWin::BuildSequenceHeader(ByteBuffer &buff)
{
    _sequencePacketIndex++;

    // save 3 last sequences
    if (_sequencePacketIndex > 3)
        _sequencePacketIndex = 1;

    //sLog->outWarden("SequenceHeaderBuild::PacketIndex %u", _sequencePacketIndex);

    uint8 xorByte = _currentModule->ClientKeySeed[0];

    buff << uint8(_currentModule->CheckTypes[MEM_CHECK] ^ xorByte);
    buff << uint8(0x00);

    switch (_sequencePacketIndex)
    {
        case 1:
        {
            if (_session->GetOS() == "Wn64")
                WriteAddress<uint64>(buff, 0x14106F1B0);
            else
                WriteAddress<uint32>(buff, 0xEA50B8);
            break;
        }
        case 2:
        {
            if (_session->GetOS() == "Wn64")
                WriteAddress<uint64>(buff, 0x14106F1C0);
            else
                WriteAddress<uint32>(buff, 0xEA50C8);
            break;
        }
        case 3:
        {
            if (_session->GetOS() == "Wn64")
                WriteAddress<uint64>(buff, 0x14106F1D0);
            else
                WriteAddress<uint32>(buff, 0xEA50D8);
            break;
        }
    }

    buff << uint8(0x6);
}

void WardenWin::BuildBaseChecksRequest(ByteBuffer &buff)
{
    _currentChecks.clear();

    std::string os = _session->GetOS();

    if (_baseChecksList[os].empty())
        _baseChecksList[os].assign(_wardenMgr->BaseChecksIdPool[os].begin(), _wardenMgr->BaseChecksIdPool[os].end());

    ByteBuffer stringBuff;
    ByteBuffer dataBuff;

    uint8 xorByte = _currentModule->ClientKeySeed[0];

    // separator
    dataBuff << uint8(0x00);

    // TIME_CHECK
    if (os == "Win")
        dataBuff << uint8(_currentModule->CheckTypes[TIME_CHECK] ^ xorByte);

    // Simple check own server clients - bugfix with empty warden packet
    dataBuff << uint8(_currentModule->CheckTypes[MEM_CHECK] ^ xorByte);
    dataBuff << uint8(0x00);
    if (os == "Wn64")
        WriteAddress<uint64>(dataBuff, 0x141C0E05C);
    else
        WriteAddress<uint32>(dataBuff, 0x15D205C);
    dataBuff << uint8(0x7);

    //BuildSequenceHeader(dataBuff);

    /*uint8 index = 1;

    boost::unique_lock<boost::shared_mutex> lock(_wardenMgr->_checkStoreLock);

    // limited by 6 checks for test, now it don't depend on size
    while (true)
    {
        if (_baseChecksList[os].empty())
            break;

        if (_currentChecks.size() >= 6)
            break;

        auto itr = _wardenMgr->GetRandomCheckFromList(_baseChecksList[os].begin(), _baseChecksList[os].end());
        uint16 id = (*itr);
        _baseChecksList[os].erase(itr);

        WardenCheck* wd = _wardenMgr->GetCheckDataById(id);

        if (!wd)
            continue;

        if (!_currentModule->CheckTypes[wd->Type])
            continue;

        if (!wd->Enabled)
            continue;

        AddCheckData(id, dataBuff, stringBuff, index);
        _currentChecks.push_back(id);
    }*/

    if (!stringBuff.empty())
        buff.append(stringBuff);
    if (!dataBuff.empty())
        buff.append(dataBuff);
}

void WardenWin::AddCheckData(uint16 id, ByteBuffer &buff, ByteBuffer &stringBuff, uint8 &index)
{
    WardenCheck* wd = _wardenMgr->GetCheckDataById(id);

    if (!wd)
        return;

    uint8 xorByte = _currentModule->ClientKeySeed[0];

    // TODO: support PROC_CHECK double string
    switch (wd->Type)
    {
        case MPQ_CHECK:
        case LUA_STR_CHECK:
        case DRIVER_CHECK:
            stringBuff << uint8(wd->Str.size());
            stringBuff.append(wd->Str.c_str(), wd->Str.size());
            break;
        default:
            break;
    }

    buff << uint8(_currentModule->CheckTypes[wd->Type] ^ xorByte);
    switch (wd->Type)
    {
        case MEM_CHECK:
        {
            buff << uint8(0x00);
            if (_session->GetOS() == "Wn64")
                WriteAddress<uint64>(buff, wd->Address);
            else
                WriteAddress<uint32>(buff, wd->Address);
            buff << uint8(wd->Length);
            break;
        }
        case PAGE_CHECK_A:
        case PAGE_CHECK_B:
        {
            BigNumber d;
            d.SetHexStr(wd->Data.c_str());
            buff.append(d.AsByteArray(24, false).get(), 24);
            buff << uint32(wd->Address);
            buff << uint8(wd->Length);
            break;
        }
        case MPQ_CHECK:
        case LUA_STR_CHECK:
        {
            buff << uint8(index++);
            break;
        }
        case DRIVER_CHECK:
        {
            BigNumber d;
            d.SetHexStr(wd->Data.c_str());
            buff.append(d.AsByteArray(24, false).get(), 24);
            buff << uint8(index++);
            break;
        }
        case MODULE_CHECK:
        case PROC_CHECK:
        {
            uint32 seed = static_cast<uint32>(rand32());
            buff << uint32(seed);
            HmacSha1 hmac(4, (uint8*)&seed);
            hmac.UpdateData(wd->Str);
            hmac.Finalize();
            buff.append(hmac.GetDigest(), hmac.GetLength());
            break;
        }
        default:
            break;                                      // Should never happen
    }
}

void WardenWin::RequestBaseData()
{
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: Request static data");

    uint8 xorByte = _currentModule->ClientKeySeed[0];

    if (!sWorld->getBoolConfig(CONFIG_WARDEN_ENABLED))
        return;

    ByteBuffer buff;
    buff << uint8(WARDEN_SMSG_CHEAT_CHECKS_REQUEST);
    BuildBaseChecksRequest(buff);
    buff << uint8(xorByte);

    // Encrypt with warden RC4 key
    EncryptData(const_cast<uint8*>(buff.contents()), buff.size());

    WorldPacket pkt(SMSG_WARDEN_DATA, buff.size() + sizeof(uint32));
    pkt << uint32(buff.size());
    pkt.append(buff);
    _session->SendPacket(&pkt);

    // while only for checks packet (0x02)
    _lastPacketSendTime = getMSTime();
    // update check timer and set client response timer if not tick
    RestartTimer(1, urand(40, 60) * IN_MILLISECONDS);
    //if (!IsTimerActive(2))
        //RestartTimer(2, 1 * MINUTE * IN_MILLISECONDS);

    //std::stringstream stream;
    //stream << "WARDEN: Sent check id's: ";
    //for (std::vector<uint16>::iterator itr = _currentChecks.begin(); itr != _currentChecks.end(); ++itr)
        //stream << *itr << " ";

    //sLog->outWarden("%s", stream.str().c_str());
    //sWorld->SendServerMessage(SERVER_MSG_STRING, stream.str().c_str(), _session->GetPlayer());
}

void WardenWin::SendExtendedData()
{
    Player* plr = _session->GetPlayer();

    if (!plr || !plr->IsInWorld() || plr->isBeingLoaded() || plr->IsBeingTeleported())
    {
        // skip check request and re-schedule timer
        RestartTimer(8, urand(30, 35) * IN_MILLISECONDS);
        return;
    }

    _checkSequenceIndex++;

    if (_checkSequenceIndex > 4)
        _checkSequenceIndex = 1;

    // less check frequency
    if (_checkSequenceIndex == 4)
    {
        if (roll_chance_i(60))
            _checkSequenceIndex = 1;
    }

    //sLog->outWarden("SendExtendedData with sequenceIndex %u", _checkSequenceIndex);

    BigNumber p;
    ByteBuffer strings;
    
    WorldPacket data(SMSG_GAME_TIME_UPDATE);
    data << uint8(0xAA);  // cipher byte

    switch (_checkSequenceIndex)
    {
        // basic anti-debugger check - not used
        case 0:
        {
            p.SetHexStr("5589E583EC605057565389F356FF1540F3E50083C60D5650FF15D0F1E50089C18D55A0FF1504F2E5005250FFD1807DA0017412B8673263008038E9740C5B5E5F5889EC5DC331C0FFD0B8C5814900FFD0EBEB");
            strings.WriteString("kernel32.dll");
            strings << uint8(0x00);
            strings.WriteString("CheckRemoteDebuggerPresent");
            strings << uint8(0x00);
            break;
        }
        // farm bots
        case 1:
        case 2:
        {
            p.SetHexStr("5589E581EC600100005057565389F356FF1540F3E50089C783C60B5657FF15D0F1E5008985A0FEFFFF83C60C5657FF15D0F1E5008985A4FEFFFF83C60E566A00FF95A0FEFFFF85C075085B5E5F5889EC5DC368"
                "000100008D8DA8FEFFFF5150FF95A4FEFFFF85C0744083C0038D8DA5FEFFFF894DA88945AC8945B031C0C685A5FEFFFF07668985A6FEFFFF8B0D80061F018B012DA447000031D28B55B0528D9DA5FEFFFF53FFD0EBA18D"
                "8DA5FEFFFF894DA8C745AC03000000C745B003000000FF1520F5E500EBB5");
            strings.WriteString("user32.dll");
            strings << uint8(0x00);
            strings.WriteString("FindWindowW");
            strings << uint8(0x00);
            strings.WriteString("GetClassNameA");
            strings << uint8(0x00);
            std::string baseName = (_checkSequenceIndex == 1) ? (std::string(plr->GetName()) + " - 1.7.2 (21213)") : (std::string(plr->GetName()) + " - " + sWorld->GetRealmName());
            std::wstring convertedName;
            Utf8toWStr(baseName, convertedName);
            strings.append(convertedName.c_str(), convertedName.length());
            strings << uint16(0x00);
            break;
        }
        // rotation bots
        case 3:
        {
            p.SetHexStr("5589E589FF50568B0DB85F170151B8D34E5400FFD08B0DB85F170151B8B1775400FFD06A006A016A008B0DB85F170151B8504C5400FFD08B0DB85F17016AFF51B87F575400FFD085C075188B0DB85F17016AFD51B85E565400FFD083C42C5889EC5DC3B879279400FFD0EBDF");
            strings.WriteString("if DoItOptions ~= nil or Pulse_Engine ~= nil then return true else return false end");
            strings << uint8(0x00);
            break;
        }
        case 4:
        {
            p.SetHexStr("5589E589FF81EC600100005057565350598B0D80061F01898DA0FEFFFF8B012DBC8200008985A4FEFFFF31C08D9DC4FEFFFF899DB4FEFFFF89F640C60306884301408985B8FEFFFF89F687DB8985BCFEFFFFB8673263008038F4741A6A0289D28B85A4FEFFFF05183B000053FFD05B5E5F5889EC5DC331C0884301EBDF");
            break;
        }
        default:
            break;
    }

    data << uint16(p.GetNumBytes());
    data.append(p.AsByteArray(0, false).get(), p.GetNumBytes());
    if (!strings.empty())
        data.append(strings);
    _session->SendPacket(&data);

    RestartTimer(8, urand(30, 35) * IN_MILLISECONDS);
}

void WardenWin::HandleData(ByteBuffer &buff)
{
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: Handle common data");

    _lastPacketRecvTime = getMSTime();

    // stop client response wait timer
    StopTimer(2);

    if (!sWorld->getBoolConfig(CONFIG_WARDEN_ENABLED))
    {
        buff.rfinish();
        return;
    }

    // read and validate length+checksum - for all packets
    uint16 length;
    buff >> length;
    uint32 checksum;
    buff >> checksum;

    if (!length)
    {
        buff.rfinish();
        sLog->outWarden("%s failed packet length. Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    if (!IsValidCheckSum(checksum, buff.contents() + buff.rpos(), length))
    {
        buff.rfinish();
        sLog->outWarden("%s failed checksum. Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    // modified TIME_CHECK result - only for x32
    if (_session->GetOS() == "Win")
    {
        uint8 handlerBytes[16];
        buff.read(handlerBytes, 16);
        std::string handlerStr = _wardenMgr->ByteArrayToString(handlerBytes, 16);

        if (handlerStr != "8B0D80061F018B012D531A0000FFE000")
        {
            buff.rfinish();
            sLog->outWarden("INTEGRITY_CHECK fail, CheckId %u account Id %u", 50, _session->GetAccountId());
            sLog->outWarden("%s failed Warden check %u. Action: %s", _session->GetPlayerName(false).c_str(), 50, Penalty(50).c_str());
            return;
        }

        uint8 bytes1[4];
        buff.read(bytes1, 4);

        std::string bytes1Str = _wardenMgr->ByteArrayToString(bytes1, 4);

        if (bytes1Str != "E909DFFF")
        {
            buff.rfinish();
            sLog->outWarden("INTEGRITY_CHECK fail, CheckId %u account Id %u", 50, _session->GetAccountId());
            sLog->outWarden("%s failed Warden check %u. Action: %s", _session->GetPlayerName(false).c_str(), 50, Penalty(50).c_str());
            return;
        }

        uint8 bytes2[4];
        buff.read(bytes2, 4);

        std::string bytes2Str = _wardenMgr->ByteArrayToString(bytes2, 4);

        if (bytes2Str != "590E557A")
        {
            if (bytes2Str != "F3A4FFD0")
            {
                buff.rfinish();
                sLog->outWarden("INTEGRITY_CHECK fail, CheckId %u account Id %u", 50, _session->GetAccountId());
                sLog->outWarden("%s failed Warden check %u. Action: %s", _session->GetPlayerName(false).c_str(), 50, Penalty(50).c_str());
                return;
            }
        }
    }

    // real TIME_CHECK result - not used since 3.3.5a
    /*uint8 timeCheckResult;
    buff >> timeCheckResult;

    if (!timeCheckResult)
    {
        buff.rfinish();
        sLog->outWarden("Player %s (guid: %u, account: %u) failed TIME_CHECK. Action: Kick", _session->GetPlayerName().c_str(), _session->GetGuidLow(), _session->GetAccountId());
        _session->KickPlayer();
        return;
    }

    uint32 newClientTicks;
    buff >> newClientTicks;

    uint32 ticksNow = getMSTime();
    uint32 ourTicks = newClientTicks + (ticksNow - _serverTicks);

    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "ServerTicks %u", ticksNow);         // Now
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "RequestTicks %u", _serverTicks);    // At request
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "Ticks %u", newClientTicks);         // At response
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "Ticks diff %u", ourTicks - newClientTicks);*/

    // read "header"
    uint8 headerRes;
    buff >> headerRes;

    if (headerRes)
    {
        buff.rfinish();
        sLog->outWarden("%s has unknown client (memory check function wasn't called). Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    uint8 bytes[7];
    buff.read(bytes, 7);
    std::string headerStr = _wardenMgr->ByteArrayToString(bytes, 7);

    bool found = (headerStr.find("65706963776F77") != std::string::npos) || (headerStr.find("75776F77") != std::string::npos);

    if (!found)
    {
        buff.rfinish();
        sLog->outWarden("%s has unknown client (wrong data). Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    // read sequence block
    /*uint8 seqRes;
    buff >> seqRes;

    if (seqRes)
    {
        buff.rfinish();
        sLog->outWarden("%s has failed (memory check function wasn't called). Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    uint8 seqBytes[6];
    buff.read(seqBytes, 6);
    std::string sequencesStr = _wardenMgr->ByteArrayToString(seqBytes, 6);

    uint8 clientSequencePacketIndex = 0;
    if (sequencesStr.find("434F50504552") != std::string::npos)
        clientSequencePacketIndex = 1;
    else if (sequencesStr.find("53494C564552") != std::string::npos)
        clientSequencePacketIndex = 2;
    else if (sequencesStr.find("474F4C44") != std::string::npos)
        clientSequencePacketIndex = 3;

    //sLog->outWarden("SequenceHeaderRead::PacketIndex %u, FromClient %u, sequenceString %s", _sequencePacketIndex, clientSequencePacketIndex, sequencesStr.c_str());

    // call read check response handler
    if (clientSequencePacketIndex == _sequencePacketIndex)
    {
        --_clientSyncFailCount;

        if (_clientSyncFailCount < 0)
            _clientSyncFailCount = 0;

        HandleChecks(buff);
    }
    else
    {
        buff.rfinish();
        HandleFailedSync(clientSequencePacketIndex);
    }*/
}

void WardenWin::HandleChecks(ByteBuffer &buff)
{
    WardenCheck *rd = nullptr;

    boost::unique_lock<boost::shared_mutex> lock(_wardenMgr->_checkStoreLock);

    for (auto itr = _currentChecks.begin(); itr != _currentChecks.end(); ++itr)
    {
        rd = _wardenMgr->GetCheckDataById(*itr);

        // remove whole packet if data array is corrupted or check in packet was disabled
        if (!rd || !rd->Enabled)
        {
            buff.rfinish();
            sLog->outWarden("Warden check with Id %u has disabled or checkStorage has corrupted data. Action: None", *itr);
            return;
        }

        uint8 type = rd->Type;

        switch (type)
        {
            case MEM_CHECK:
            {
                uint8 memResult;
                buff >> memResult;

                if (memResult)
                {
                    buff.rfinish();
                    sLog->outWarden("Function for MEM_CHECK hasn't been called, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    KickPlayer();
                    return;
                }

                std::string packet_data = _wardenMgr->ByteArrayToString(buff.contents() + buff.rpos(), rd->Length);
                if (strcmp(rd->Result.c_str(), packet_data.c_str()))
                {
                    buff.rfinish();
                    sLog->outWarden("MEM_CHECK fail CheckId %u account Id %u, realData - %s, failedData - %s", *itr, _session->GetAccountId(), rd->Result.c_str(), packet_data.c_str());
                    sLog->outWarden("%s failed Warden check %u. Action: %s", _session->GetPlayerName(false).c_str(), *itr, Penalty(*itr).c_str());
                    return;
                }

                buff.rpos(buff.rpos() + rd->Length);
                break;
            }
            case PAGE_CHECK_A:
            case PAGE_CHECK_B:
            case DRIVER_CHECK:
            case MODULE_CHECK:
            case PROC_CHECK:
            {
                std::string packet_data = _wardenMgr->ByteArrayToString(buff.contents() + buff.rpos(), 1);
                if (strcmp(rd->Result.c_str(), packet_data.c_str()))
                {
                    buff.rfinish();

                    if (type == PAGE_CHECK_A || type == PAGE_CHECK_B)
                        sLog->outWarden("PAGE_CHECK fail, CheckId %u, account Id %u, realData - %s, failedData - %s", *itr, _session->GetAccountId(), rd->Result.c_str(), packet_data.c_str());
                    if (type == MODULE_CHECK)
                        sLog->outWarden("MODULE_CHECK fail, CheckId %u, account Id %u, realData - %s, failedData - %s", *itr, _session->GetAccountId(), rd->Result.c_str(), packet_data.c_str());
                    if (type == DRIVER_CHECK)
                        sLog->outWarden("DRIVER_CHECK fail, CheckId %u, account Id %u, realData - %s, failedData - %s", *itr, _session->GetAccountId(), rd->Result.c_str(), packet_data.c_str());
                    if (type == PROC_CHECK)
                        sLog->outWarden("PROC_CHECK fail, CheckId %u, account Id %u, realData - %s, failedData - %s", *itr, _session->GetAccountId(), rd->Result.c_str(), packet_data.c_str());

                    sLog->outWarden("%s failed Warden check %u. Action: %s", _session->GetPlayerName(false).c_str(), *itr, Penalty(*itr).c_str());
                    return;
                }

                buff.rpos(buff.rpos() + 1);
                break;
            }
            // not used since 7.0.3, maybe need find fnction for CASC?
            case MPQ_CHECK:
            {
                /*uint8 mpqResult;
                buff >> mpqResult;

                if (mpqResult)
                {
                    buff.rfinish();
                    sLog->outWarden("Function for MPQ_CHECK hasn't been called, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    _session->KickPlayer();
                    return;
                }

                std::string packet_data = _wardenMgr->ByteArrayToString(buff.contents() + buff.rpos(), 20);

                // handle locales hash
                std::string extResult = GetMPQHashForLocales(*itr);
                if (extResult != "")
                    rd->Result = extResult;

                if (strcmp(rd->Result.c_str(), packet_data.c_str()))
                {
                    buff.rfinish();
                    sLog->outWarden("RESULT MPQ_CHECK fail, CheckId %u account Id %u, realData - %s, failedData - %s", *itr, _session->GetAccountId(), rd->Result.c_str(), packet_data.c_str());
                    sLog->outWarden("%s failed Warden check %u. Action: %s", _session->GetPlayerName(false).c_str(), *itr, Penalty(*itr).c_str());
                    SetPlayerLocked(*itr);
                    return;
                }

                buff.rpos(buff.rpos() + 20);*/
                break;
            }
            case LUA_STR_CHECK:
            {
                uint8 luaResult;
                buff >> luaResult;

                if (luaResult)
                {
                    buff.rfinish();
                    sLog->outWarden("Function for LUA_STR_CHECK hasn't been called, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    KickPlayer();
                    return;
                }

                uint8 luaStrLen;
                buff >> luaStrLen;

                if (luaStrLen)
                {
                    char *str = new char[luaStrLen + 1];
                    memset(str, 0, luaStrLen + 1);
                    memcpy(str, buff.contents() + buff.rpos(), luaStrLen);
                    sLog->outWarden("LUA_STR_CHECK fail, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    sLog->outWarden("Lua string found: %s", str);
                    delete[] str;

                    buff.rfinish();
                    sLog->outWarden("%s failed Warden check %u. Action: %s", _session->GetPlayerName(false).c_str(), *itr, Penalty(*itr).c_str());
                    return;
                }
                break;
            }
            default:
                break;
        }
    }

    // right place, but fucking returns in checks
    //_state = WARDEN_MODULE_READY;
    //_checkTimer = sWorld->getIntConfig(CONFIG_WARDEN_STATIC_CHECK_HOLDOFF);
}

// blizzard modified WriteBits/WriteBytes function
// some simplified because it is not known why this function is needed at all
template<class T>
void WardenWin::WriteAddress(ByteBuffer &buff, T address)
{
    uint8 mask = 0;
    for (uint8 i = 0; i < sizeof(T); ++i)
        mask += (1 << i);

    buff << mask;
    buff << address;
}