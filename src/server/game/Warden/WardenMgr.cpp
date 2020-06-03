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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "SHA256.h"
#include "Database/DatabaseEnv.h"
#include "Util.h"
#include "WardenMgr.h"
#include "Warden.h"
#include <stdio.h>
#include <sys/types.h>
#ifndef WIN32
    #include <dirent.h>
#endif

WardenMgr::WardenMgr()
{
    moduleStore.clear();
}

WardenMgr::~WardenMgr()
{
    for (uint16 i = 0; i < checkStore.size(); ++i)
        delete checkStore[i];

    for (ModuleContainer::const_iterator itr = moduleStore.begin(); itr != moduleStore.end(); ++itr)
    {
        WardenModule* mod = itr->second;

        if (mod)
        {
            delete[] mod->CompressedData;
            delete mod;
        }
    }
}

void WardenMgr::LoadWardenChecks()
{
    QueryResult result = WorldDatabase.Query("SELECT MAX(id) FROM warden_checks");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Warden checks. DB table `warden_checks` is empty!");
        return;
    }

    Field* fields = result->Fetch();

    uint16 maxCheckId = fields[0].GetUInt16();
    checkStore.resize(maxCheckId + 1);

    //                                    0    1    2    3     4     5        6      7         8        9       10
    result = WorldDatabase.Query("SELECT id, type, os, data, str, address, length, result, banreason, action, comment FROM warden_checks ORDER BY id ASC");

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint16 id             = fields[0].GetUInt16();
        uint8 checkType       = fields[1].GetUInt8();
        std::string os        = fields[2].GetString();
        std::string data      = fields[3].GetString();
        std::string str       = fields[4].GetString();
        uint32 address        = fields[5].GetUInt32();
        uint8 length          = fields[6].GetUInt8();
        std::string result    = fields[7].GetString();
        std::string banreason = fields[8].GetString();
        uint8 action          = fields[9].GetUInt8();
        std::string comment   = fields[10].GetString();

        WardenCheck* wardenCheck = new WardenCheck(checkType, os, data, str, address, length, result, banreason, (WardenActions)action, comment);
        if (checkType != INTERNAL_CHECK)
            BaseChecksIdPool[os].push_back(id);

        if (comment.empty())
            wardenCheck->Comment = "Undocumented Check";
        else
            wardenCheck->Comment = comment;

        checkStore[id] = wardenCheck;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u warden checks.", count);
}

void WardenMgr::LoadWardenOverrides()
{
    //                                                  0        1       2        3
    QueryResult result = WorldDatabase.Query("SELECT checkId, action, banTime, enabled FROM warden_overrides");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Warden action overrides. DB table `warden_overrides` is empty!");
        return;
    }

    uint32 count = 0;

    boost::unique_lock<boost::shared_mutex> lock(_checkStoreLock);

    do
    {
        Field* fields = result->Fetch();

        uint16 checkId = fields[0].GetUInt16();
        int8 action = fields[1].GetInt8();
        int32 banTime = fields[2].GetInt32();
        bool enabled = fields[3].GetBool();

        // Check if action value is in range (0-4, see WardenActions enum)
        if (action >= MAX_WARDEN_ACTIONS)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Warden check override action out of range (ID: %u, action: %u)", checkId, action);
        // Check if check actually exists before accessing the CheckStore vector
        else if (checkId > checkStore.size())
            TC_LOG_ERROR(LOG_FILTER_SQL, "Warden check action override for non-existing check (ID: %u, action: %u), skipped", checkId, action);
        else
        {
            checkStore[checkId]->Enabled = enabled;

            if (enabled)
            {
                checkStore[checkId]->Action = WardenActions(action);

                if (checkStore[checkId]->Action == WARDEN_ACTION_BAN || checkStore[checkId]->Action == WARDEN_ACTION_FLAG_ACCOUNT)
                    checkStore[checkId]->BanTime = banTime;
            }

            ++count;
        }
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u warden action overrides.", count);
}

void WardenMgr::LoadWardenModules(std::string os)
{
    boost::unique_lock<boost::shared_mutex> lock(_moduleStoreLock);

#ifndef WIN32

    DIR *dirp;
    struct dirent *dp;
    uint32 count = 0;

    std::string rawPath = "./warden_modules/" + os;
    dirp = opendir(rawPath.c_str());

    if (!dirp)
        return;

    while (dirp)
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            int nameLength = strlen(dp->d_name);

            if (!memcmp(&dp->d_name[nameLength - 4], ".bin", 4))
            {
                if (LoadModule(dp->d_name, os))
                    ++count;
            }
        }
        else
        {
            if (errno != 0)
            {
                closedir(dirp);
                return;
            }
            break;
        }
    }

    if (dirp)
        closedir(dirp);

#else
    WIN32_FIND_DATA fil;
    uint32 count = 0;

    std::string rawPath = "./warden_modules/" + os + "/*.bin";
    HANDLE hFil = FindFirstFile(rawPath.c_str(), &fil);

    if (hFil == INVALID_HANDLE_VALUE)
        return;
    do
    {
        if (LoadModule(fil.cFileName, os))
            ++count;
    } 
    while (FindNextFile(hFil, &fil));

    FindClose(hFil);

#endif
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u %s warden modules.", count, os.c_str());
}

bool WardenMgr::LoadModule(const char * fileName, std::string os)
{
    char fn_module[256];
    std::string _path = "./warden_modules/" + os + "/%s";
    snprintf(fn_module, 256, _path.c_str(), fileName);

    FILE * f_mod = fopen(fn_module, "rb");

    if (f_mod == nullptr)
        return false;

    // read module
    fseek(f_mod, 0, SEEK_END);
    uint32 len = ftell(f_mod);
    fseek(f_mod, 0, SEEK_SET);

    // data assign
    WardenModule *mod = new WardenModule();
    mod->CompressedSize = len;
    mod->CompressedData = new uint8[len];
    fread(mod->CompressedData, len, 1, f_mod);

    // SHA-2 hash
    SHA256Hash sha;
    sha.Initialize();
    sha.UpdateData(mod->CompressedData, len);
    sha.Finalize();

    for (uint8 i = 0; i < 32; i++)
        mod->ID[i] = sha.GetDigest()[i];

    fclose(f_mod);

    std::string fn(fileName);
    std::string fn_t = fn.substr(0, fn.length() - 4);
    std::string path = "./warden_modules/" + os + "/";
    path += fn_t;
    path += ".key";

    FILE * f_key = fopen(path.c_str(), "rb");

    if (f_key == nullptr)
        return false;

    // read key and other data
    fread(mod->Key, 16, 1, f_key);
    fread(mod->Seed, 16, 1, f_key);
    fread(mod->ServerKeySeed, 16, 1, f_key);
    fread(mod->ClientKeySeed, 16, 1, f_key);
    fread(mod->ClientKeySeedHash, 20, 1, f_key);
    fread(mod->CheckTypes, 9, 1, f_key);

    fclose(f_key);

    // set module os and convert id to hash for binary search in map
    mod->os = os;
    std::string hash = ByteArrayToString(mod->ID, 32);

    moduleStore[hash] = mod;
    return true;
}

WardenCheck* WardenMgr::GetCheckDataById(uint16 Id)
{
    if (Id < checkStore.size())
        return checkStore[Id];

    return nullptr;
}

WardenModule* WardenMgr::GetModuleById(std::string hash, std::string os)
{
    return Trinity::Containers::MapGetValuePtr(moduleStore, hash);
}

std::string WardenMgr::ByteArrayToString(const uint8 * data, uint16 length)
{
    std::ostringstream ss;

    // convert packet data to string
    for (uint32 i = 0; i < length; ++i)
    {
        if (int(data[i]) < 16)
            ss << std::uppercase << std::hex << "0" << int(data[i]) << "";
        else
            ss << std::uppercase << std::hex << int(data[i]) << "";
    }

    std::string data_str = ss.str();
    return data_str;
}

std::vector<uint16>::iterator WardenMgr::GetRandomCheckFromList(std::vector<uint16>::iterator begin, std::vector<uint16>::iterator end)
{
    const unsigned long n = std::distance(begin, end);
    const unsigned long divisor = (RAND_MAX + 1) / n;

    unsigned long k;
    do { k = std::rand() / divisor; } while (k >= n);

    std::advance(begin, k);
    return begin;
}
