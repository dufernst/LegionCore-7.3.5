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

#ifndef _WARDEN_BASE_H
#define _WARDEN_BASE_H

#include <map>
#include "ARC4.h"
#include "BigNumber.h"
#include "ByteBuffer.h"
#include "WardenMgr.h"
#include "WardenTimer.h"

class WorldSession;

enum WardenOpcodes
{
    // Client->Server
    WARDEN_CMSG_MODULE_MISSING                  = 0,
    WARDEN_CMSG_MODULE_OK                       = 1,
    WARDEN_CMSG_CHEAT_CHECKS_RESULT             = 2,
    WARDEN_CMSG_MEM_CHECKS_RESULT               = 3,        // only sent if MEM_CHECK bytes doesn't match
    WARDEN_CMSG_HASH_RESULT                     = 4,
    WARDEN_CMSG_MODULE_FAILED                   = 5,        // this is sent when client failed to load uploaded module due to cache fail
    WARDEN_CMSG_EXTENDED_DATA                   = 6,
    WARDEN_CMSG_STRING_DATA                     = 7,

    // Server->Client
    WARDEN_SMSG_MODULE_USE                      = 0,
    WARDEN_SMSG_MODULE_CACHE                    = 1,
    WARDEN_SMSG_CHEAT_CHECKS_REQUEST            = 2,
    WARDEN_SMSG_MODULE_INITIALIZE               = 3,
    WARDEN_SMSG_MEM_CHECKS_REQUEST              = 4,        // byte len; while(!EOF) { byte unk(1); byte index(++); string module(can be 0); int offset; byte len; byte[] bytes_to_compare[len]; }
    WARDEN_SMSG_HASH_REQUEST                    = 5
};

enum WardenState
{
    WARDEN_NOT_INITIALIZED            = 0,
    WARDEN_MODULE_NOT_LOADED          = 1,
    WARDEN_MODULE_CONNECTING_TO_MAIEV = 2,
    WARDEN_MODULE_REQUESTING          = 3,
    WARDEN_MODULE_SENDING             = 4,
    WARDEN_MODULE_LOADED              = 5,
    WARDEN_MODULE_SEND_CHALLENGE      = 6,
    WARDEN_MODULE_READY               = 7,
    WARDEN_MODULE_SET_PLAYER_LOCK     = 8
};

#pragma pack(push,1)


struct WardenModuleUse
{
    uint8 Command;
    uint8 ModuleId[32];
    uint8 ModuleKey[16];
    uint32 Size;
};

struct WardenModuleTransfer
{
    uint8 Command;
    uint16 ChunkSize;
    uint8 Data[500];
};

struct WardenInitModuleMPQFunc
{
    uint8 Type;
    uint8 Flag;
    uint8 MpqFuncType;
    uint8 StringBlock;
    uint32 OpenFile;
    uint32 GetFileSize;
    uint32 ReadFile;
    uint32 CloseFile;
};

struct WardenInitModuleLUAFunc
{
    uint8 Type;
    uint8 Flag;
    uint8 StringBlock;
    uint32 GetText;
    uint32 UnkFunction;
    uint8 LuaFuncType;
};

struct WardenInitModuleTimeFunc
{
    uint8 Type;
    uint8 Flag;
    uint8 StringBlock;
    uint32 PerfCounter;
    uint8 TimeFuncType;
};

struct WardenHashRequest
{
    uint8 Command;
    uint8 Seed[16];
};

#pragma pack(pop)

class Warden
{
    friend class WardenWin;
    friend class WardenMac;

    public:
        Warden(WorldSession* session);
        virtual ~Warden();

        bool Create(BigNumber *k);
        void SendModuleToClient();
        void RequestModule();
        void ConnectToMaievModule();
        void RequestHash();
        void Update();
        void UpdateTimers(const uint32 diff);

        // temp
        void TestFunc();
        void LogTimers(const uint32 diff);
        virtual void ActivateModule() = 0;

        virtual void HandleHashResult(ByteBuffer &buff) = 0;
        virtual void HandleHashResultSpecial(ByteBuffer &buff) = 0;
        virtual void HandleModuleFailed() = 0;
        virtual void RequestBaseData() = 0;
        virtual void SendExtendedData() = 0;
        virtual void HandleExtendedData(ByteBuffer &buff) = 0;
        virtual void HandleStringData(ByteBuffer &buff) = 0;

        virtual void HandleData(ByteBuffer &buff) = 0;

        void DecryptData(uint8* buffer, uint32 length);
        void EncryptData(uint8* buffer, uint32 length);

        //uint8* GetInputKey() { return _clientKeySeed; }
        //uint8* GetOutputKey() { return _serverKeySeed; }

        WardenState GetState() { return _state; }
        std::string GetStateString();
        void SetNewState(WardenState newState) { _state = newState;  }

        static bool IsValidCheckSum(uint32 checksum, const uint8 *data, const uint16 length);
        static uint32 BuildChecksum(const uint8 *data, uint32 length);

        // If no check is passed, the default action from config is executed
        std::string Penalty(uint16 checkId);
        uint32 CalcBanTime();
        uint32 GetBanTime();
        void SetPlayerLocked(uint16 checkId);
        void KickPlayer();

        // timer functions
        void CreateTimers();
        void CreateTimer(uint32 id, uint32 time, uint32 reqLevel, bool reqPlayerInWorld = true);
        void StartTimers(uint32 mask);
        void StopTimers(uint32 mask);
        void StartTimer(uint32 id) { _timers[id]->Start(); }
        void StopTimer(uint32 id) { _timers[id]->Stop(); }
        void RestartTimer(uint32 id, uint32 newTime)
        { 
            _timers[id]->Stop();
            _timers[id]->SetCurrentTime(newTime);
            _timers[id]->Start();
        }
        void DelayTimer(uint32 id, uint32 delayTime)
        { 
            _timers[id]->Stop();
            uint32 newTime = _timers[id]->GetCurrentTime() + delayTime;
            _timers[id]->SetCurrentTime(newTime);
            _timers[id]->Start();
        }
        void ResetTimer(uint32 id)
        { 
            _timers[id]->Stop();
            _timers[id]->Reset();
        }
        void DoAction(uint32 id, const uint32 diff);
        bool TimerEnabled(uint32 id) { return _timers[id]->Active(); }

    private:
        WorldSession* _session;
        WardenModule* _currentModule;

        RC4_Context _clientRC4State;
        RC4_Context _serverRC4State;
        WardenState _state;
        uint32 _lastUpdateTime;
        uint32 _lastPacketSendTime;
        uint32 _lastPacketRecvTime;
        std::unordered_map<uint32, WardenTimer*> _timers;

        std::mutex _wardenTimerUpdate;

        // temp
        bool _currentSessionFlagged;
        uint32 _checkSequenceIndex;
};

#endif
