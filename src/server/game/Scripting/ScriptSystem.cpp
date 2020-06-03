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

#include "ScriptSystem.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "ScriptMgr.h"
#include "GlobalFunctional.h"

ScriptPointVector const SystemMgr::_empty;

SystemMgr* SystemMgr::instance()
{
    static SystemMgr instance;
    return &instance;
}

void SystemMgr::LoadScriptTexts()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Script Texts...");
    LoadTrinityStrings("script_texts", TEXT_SOURCE_RANGE, 1+(TEXT_SOURCE_RANGE*2));

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Script Texts additional data...");
    uint32 oldMSTime = getMSTime();

    //                                                 0      1      2      3
    QueryResult result = WorldDatabase.Query("SELECT entry, sound, type, language, emote FROM script_texts");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 additional Script Texts data. DB table `script_texts` is empty.");
        return;
    }

    uint32 uiCount = 0;

    do
    {
        Field* pFields = result->Fetch();
        StringTextData temp;

        int32 iId           = pFields[0].GetInt32();
        temp.uiSoundId     = pFields[1].GetUInt32();
        temp.uiType        = pFields[2].GetUInt8();
        temp.uiLanguage    = pFields[3].GetUInt8();
        temp.uiEmote       = pFields[4].GetUInt16();

        if (iId >= 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `script_texts` is not a negative value.", iId);
            continue;
        }

        if (iId > TEXT_SOURCE_RANGE || iId <= TEXT_SOURCE_RANGE*2)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `script_texts` is out of accepted entry range for table.", iId);
            continue;
        }

        if (!GetLanguageDescByID(temp.uiLanguage))
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `script_texts` using Language %u but Language does not exist.", iId, temp.uiLanguage);

        if (temp.uiType > CHAT_TYPE_ZONE_YELL)
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `script_texts` has Type %u but this Chat Type does not exist.", iId, temp.uiType);

        m_mTextDataMap[iId] = temp;
        ++uiCount;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u additional Script Texts data in %u ms", uiCount, GetMSTimeDiffToNow(oldMSTime));
}

void SystemMgr::LoadScriptTextsCustom()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Custom Texts...");
    LoadTrinityStrings("custom_texts", TEXT_SOURCE_RANGE*2, 1+(TEXT_SOURCE_RANGE*3));

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Custom Texts additional data...");

    QueryResult result = WorldDatabase.Query("SELECT entry, sound, type, language, emote FROM custom_texts");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 additional Custom Texts data. DB table `custom_texts` is empty.");
        return;
    }

    uint32 uiCount = 0;

    do
    {
        Field* pFields = result->Fetch();
        StringTextData temp;

        int32 iId             = pFields[0].GetInt32();
        temp.uiSoundId        = pFields[1].GetUInt32();
        temp.uiType           = pFields[2].GetUInt8();
        temp.uiLanguage       = pFields[3].GetUInt8();
        temp.uiEmote          = pFields[4].GetUInt16();

        if (iId >= 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `custom_texts` is not a negative value.", iId);
            continue;
        }

        if (iId > TEXT_SOURCE_RANGE*2 || iId <= TEXT_SOURCE_RANGE*3)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `custom_texts` is out of accepted entry range for table.", iId);
            continue;
        }

        if (!GetLanguageDescByID(temp.uiLanguage))
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `custom_texts` using Language %u but Language does not exist.", iId, temp.uiLanguage);

        if (temp.uiType > CHAT_TYPE_ZONE_YELL)
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: Entry %i in table `custom_texts` has Type %u but this Chat Type does not exist.", iId, temp.uiType);

        m_mTextDataMap[iId] = temp;
        ++uiCount;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u additional Custom Texts data.", uiCount);
}

void SystemMgr::LoadScriptWaypoints()
{
    uint32 oldMSTime = getMSTime();

    // Drop Existing Waypoint list
    m_mPointMoveMap.clear();

    uint64 uiCreatureCount = 0;

    // Load Waypoints
    QueryResult result = WorldDatabase.Query("SELECT COUNT(entry) FROM script_waypoint GROUP BY entry");
    if (result)
        uiCreatureCount = result->GetRowCount();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Script Waypoints for " UI64FMTD " creature(s)...", uiCreatureCount);

    //                                     0       1         2           3           4           5
    result = WorldDatabase.Query("SELECT entry, pointid, location_x, location_y, location_z, waittime FROM script_waypoint ORDER BY pointid");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 Script Waypoints. DB table `script_waypoint` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* pFields = result->Fetch();
        ScriptPointMove temp;

        temp.uiCreatureEntry   = pFields[0].GetUInt32();
        uint32 uiEntry          = temp.uiCreatureEntry;
        temp.uiPointId         = pFields[1].GetUInt32();
        temp.fX                = pFields[2].GetFloat();
        temp.fY                = pFields[3].GetFloat();
        temp.fZ                = pFields[4].GetFloat();
        temp.uiWaitTime        = pFields[5].GetUInt32();

        CreatureTemplate const* pCInfo = sObjectMgr->GetCreatureTemplate(temp.uiCreatureEntry);

        if (!pCInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: DB table script_waypoint has waypoint for non-existant creature entry %u", temp.uiCreatureEntry);
            continue;
        }

        if (!pCInfo->ScriptID)
            TC_LOG_ERROR(LOG_FILTER_SQL, "TSCR: DB table script_waypoint has waypoint for creature entry %u, but creature does not have ScriptName defined and then useless.", temp.uiCreatureEntry);

        m_mPointMoveMap[uiEntry].push_back(temp);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Script Waypoint nodes in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

StringTextData const* SystemMgr::GetTextData(int32 textId) const
{
    return Trinity::Containers::MapGetValuePtr(m_mTextDataMap, textId);
}

ScriptPointVector const& SystemMgr::GetPointMoveList(uint32 creatureEntry) const
{
    PointMoveMap::const_iterator itr = m_mPointMoveMap.find(creatureEntry);
    if (itr == m_mPointMoveMap.end())
        return _empty;
    return itr->second;
}
