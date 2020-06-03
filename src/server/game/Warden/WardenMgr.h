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

#ifndef _WARDENCHECKMGR_H
#define _WARDENCHECKMGR_H

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "BigNumber.h"

enum WardenCheckType
{
    INTERNAL_CHECK  = 0,
    TIME_CHECK      = 0,
    MEM_CHECK       = 1,
    PAGE_CHECK_A    = 2,
    PAGE_CHECK_B    = 3,
    MPQ_CHECK       = 4,
    DRIVER_CHECK    = 5,
    MODULE_CHECK    = 6,
    LUA_STR_CHECK   = 7,
    PROC_CHECK      = 8,
    LUA_EXEC_CHECK  = 9,
    GET_SYSTEM_INFO = 10,
    MAX_CHECK_TYPE
};

struct WardenModule
{
    WardenModule()
    {
        memset(ID, 0, 32);
        memset(Key, 0, 16);
        memset(Seed, 0, 16);
        memset(ServerKeySeed, 0, 16);
        memset(ClientKeySeed, 0, 16);
        memset(ClientKeySeedHash, 0, 20);
        memset(CheckTypes, 0, MAX_CHECK_TYPE);
        os = "";
    }

    uint8 ID[32];
    uint8 Key[16];
    uint8 Seed[16];
    uint8 ServerKeySeed[16];
    uint8 ClientKeySeed[16];
    uint8 ClientKeySeedHash[20];
    uint8 CheckTypes[MAX_CHECK_TYPE];
    std::string os;
    //RC4_Context ClientRC4State;
    //RC4_Context ServerRC4State;
    uint8* CompressedData;
    uint32 CompressedSize;
};

enum WardenActions
{
    WARDEN_ACTION_LOG          = 0,
    WARDEN_ACTION_INSTANT_KICK = 1,
    WARDEN_ACTION_BAN          = 2,
    WARDEN_ACTION_PENDING_KICK = 3,
    WARDEN_ACTION_FLAG_ACCOUNT = 4,
    MAX_WARDEN_ACTIONS
};

struct WardenCheck
{
    WardenCheck(uint8 checkType, std::string os, std::string data, std::string str, uint32 address, uint8 length, std::string result, std::string banReason, WardenActions action, std::string comment) : Type(checkType), OS(os),
        Data(data), Str(str), Address(address), Length(length), Result(result), BanReason(banReason), Action(action), BanTime(0xFFFFFFFF), Enabled(true), Comment(comment) {}

    uint8 Type;
    std::string OS;
    std::string Data;
    std::string Str;
    uint64 Address;
    uint8 Length;
    std::string Result;
    std::string BanReason;
    enum WardenActions Action;
    int32 BanTime;
    bool Enabled;
    std::string Comment;
};

class WardenMgr
{
    private:
        WardenMgr();
        ~WardenMgr();

        WardenMgr(WardenMgr const&) = delete;
        WardenMgr& operator= (WardenMgr const&) = delete;

    public:
        static WardenMgr* instance()
        {
            static WardenMgr instance;
            return &instance;
        }
        // We have a linear key without any gaps, so we use vector for fast access
        typedef std::vector<WardenCheck*> CheckContainer;
        typedef std::unordered_map<std::string, WardenModule*> ModuleContainer;

        // module helpers
        void LoadWardenModules(std::string os);
        bool LoadModule(const char * FileName, std::string os);
        WardenModule* GetModuleById(std::string name, std::string os);

        // check helpers
        WardenCheck* GetCheckDataById(uint16 Id);

        std::map<std::string, std::vector<uint16>> BaseChecksIdPool;

        void LoadWardenChecks();
        void LoadWardenOverrides();

        // utlities
        std::string ByteArrayToString(const uint8 * data, uint16 length);
        std::vector<uint16>::iterator GetRandomCheckFromList(std::vector<uint16>::iterator begin, std::vector<uint16>::iterator end);

        boost::shared_mutex _checkStoreLock;
        boost::shared_mutex _moduleStoreLock;

    private:
        CheckContainer checkStore;
        ModuleContainer moduleStore;
};

#define _wardenMgr WardenMgr::instance()

#endif
