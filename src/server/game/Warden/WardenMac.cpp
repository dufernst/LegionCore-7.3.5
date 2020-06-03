/*/*
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

#include "AccountMgr.h"
#include "Cryptography/SessionKeyGeneration.h"
#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "WardenMac.h"
#include "WardenMgr.h"

WardenMac::WardenMac(WorldSession* session) : Warden(session)
{
    // hack
    _currentModule = _wardenMgr->GetModuleById("6680ABDF47865365A331628F97022DEDAA3C523D17A1A85394B7002C8E6161F3", "Wn64");
}

WardenMac::~WardenMac()
{
}

void WardenMac::HandleHashResultSpecial(ByteBuffer &buff)
{
    uint8 clientSeedHash[20];
    buff.read(clientSeedHash, 20);

    std::string hashStr = _wardenMgr->ByteArrayToString(clientSeedHash, 20);
    std::string realHashStr = "D88209ABAFB8AAC1973461DB06E4C7CCBE02C265";

    // destroy cheaters (Win)
    if (hashStr == "20046B327D453A4601630C40571E955D9D8DE8C7")
    {
        sLog->outWarden("%s has Mac OS X in account data, but hash result from OS Windows. Action: Ban", _session->GetPlayerName(false).c_str());

        std::string accountName;
        AccountMgr::GetName(_session->GetAccountId(), accountName);

        sWorld->BanAccount(BAN_ACCOUNT, accountName, "2592000s", "Packet interception", "Anticheat");
        return;
    }

    if (strcmp(hashStr.c_str(), realHashStr.c_str()))
    {
        sLog->outWarden("%s failed special hash reply. Action: Kick", _session->GetPlayerName(false).c_str());
        KickPlayer();
        return;
    }

    uint8 baseClientKeySeed_Wn64[16] = { 0x32, 0x1C, 0xD0, 0xDC, 0x61, 0x56, 0x80, 0x08, 0x77, 0x54, 0xF9, 0xAA, 0x2B, 0xCE, 0x7C, 0x37 };
    uint8 baseServerKeySeed_Wn64[16] = { 0xDD, 0xA2, 0x7D, 0x02, 0x1F, 0x11, 0x7F, 0xD3, 0x99, 0x93, 0x2A, 0xB0, 0x2F, 0xC8, 0xDB, 0x4E };

    ARC4::rc4_init(&_clientRC4State, baseClientKeySeed_Wn64, 16);
    ARC4::rc4_init(&_serverRC4State, baseServerKeySeed_Wn64, 16);

    // we must send windows module and wait for CMSG_MODULE_FAILED, on real Windows module loaded and executed
    RequestModule();
}

void WardenMac::HandleModuleFailed()
{
    StopTimers(0xFF);
    SetNewState(WARDEN_MODULE_LOADED);
}

void WardenMac::ActivateModule()
{
    sLog->outWarden("%s has Mac OS X in account data, but Windows module are loaded. Action: Ban", _session->GetPlayerName(false).c_str());

    std::string accountName;
    AccountMgr::GetName(_session->GetAccountId(), accountName);

    sWorld->BanAccount(BAN_ACCOUNT, accountName, "2592000s", "Packet interception", "Anticheat");
}