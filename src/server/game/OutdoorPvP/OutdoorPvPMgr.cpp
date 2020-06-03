/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "OutdoorPvPMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "DisableMgr.h"
#include "ScriptMgr.h"
#include "DatabaseEnv.h"

OutdoorPvPMgr::OutdoorPvPMgr()
{
    m_UpdateTimer = 0;
}

void OutdoorPvPMgr::Die()
{
    for (auto v : m_OutdoorPvPSet)
        delete v;

    for (auto v : m_OutdoorPvPDatas)
        delete v.second;
}

OutdoorPvPMgr* OutdoorPvPMgr::instance()
{
    static OutdoorPvPMgr instance;
    return &instance;
}

void OutdoorPvPMgr::InitOutdoorPvP()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0       1
    QueryResult result = WorldDatabase.Query("SELECT TypeId, ScriptName FROM outdoorpvp_template");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 outdoor PvP definitions. DB table `outdoorpvp_template` is empty.");
        return;
    }

    uint32 count = 0;
    uint32 typeId = 0;

    do
    {
        Field* fields = result->Fetch();

        typeId = fields[0].GetUInt8();

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_OUTDOORPVP, typeId))
            continue;

        if (typeId >= MAX_OUTDOORPVP_TYPES)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Invalid OutdoorPvPTypes value %u in outdoorpvp_template; skipped.", typeId);
            continue;
        }

        OutdoorPvPData* data = new OutdoorPvPData();
        OutdoorPvPTypes realTypeId = OutdoorPvPTypes(typeId);
        data->TypeId = realTypeId;
        data->ScriptId = sObjectMgr->GetScriptId(fields[1].GetCString());
        m_OutdoorPvPDatas[realTypeId] = data;

        ++count;
    }
    while (result->NextRow());

    for (uint8 i = 1; i < MAX_OUTDOORPVP_TYPES; ++i)
    {
        OutdoorPvPDataMap::iterator iter = m_OutdoorPvPDatas.find(OutdoorPvPTypes(i));
        if (iter == m_OutdoorPvPDatas.end())
        {
            //TC_LOG_ERROR(LOG_FILTER_SQL, "Could not initialize OutdoorPvP object for type ID %u; no entry in database.", uint32(i));
            continue;
        }

        OutdoorPvP* pvp = sScriptMgr->CreateOutdoorPvP(iter->second);
        if (!pvp)
        {
            TC_LOG_ERROR(LOG_FILTER_OUTDOORPVP, "Could not initialize OutdoorPvP object for type ID %u; got nullptr pointer from script.", uint32(i));
            continue;
        }

        if (!pvp->SetupOutdoorPvP())
        {
            TC_LOG_ERROR(LOG_FILTER_OUTDOORPVP, "Could not initialize OutdoorPvP object for type ID %u; SetupOutdoorPvP failed.", uint32(i));
            delete pvp;
            continue;
        }

        m_OutdoorPvPSet.push_back(pvp);
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u outdoor PvP definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void OutdoorPvPMgr::AddZone(uint32 zoneid, OutdoorPvP* handle)
{
    m_OutdoorPvPZone[zoneid] = handle;
    if (AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(zoneid))
    {
        m_OutdoorPvPMap[areaEntry->ContinentID].insert(handle);

        MapEntry const* entry = sMapStore.LookupEntry(areaEntry->ContinentID);
        if (!entry)
            return;

        handle->m_zoneSet.insert(areaEntry->ParentAreaID ? areaEntry->ParentAreaID : zoneid);

        if (Map* map = entry->CanCreatedZone() ? sMapMgr->FindMap(areaEntry->ContinentID, zoneid) : sMapMgr->CreateBaseMap(areaEntry->ContinentID))
        {
            if (!map->OutdoorPvPList)
                map->OutdoorPvPList = GetOutdoorPvPMap(areaEntry->ContinentID);
            handle->SetMap(map);
            handle->Initialize(zoneid);
        }
    }
}

std::set<OutdoorPvP*>* OutdoorPvPMgr::GetOutdoorPvPMap(uint32 ContinentID)
{
    return Trinity::Containers::MapGetValuePtr(m_OutdoorPvPMap, ContinentID);
}

void OutdoorPvPMgr::HandlePlayerEnterZone(ObjectGuid guid, uint32 zoneid)
{
    OutdoorPvPZone::iterator itr = m_OutdoorPvPZone.find(zoneid);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerEnterZone(guid, zoneid);
}

void OutdoorPvPMgr::HandlePlayerLeaveZone(ObjectGuid guid, uint32 zoneid)
{
    OutdoorPvPZone::iterator itr = m_OutdoorPvPZone.find(zoneid);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerLeaveZone(guid, zoneid);
}

void OutdoorPvPMgr::HandlePlayerEnterMap(ObjectGuid guid, uint32 zoneID)
{
    auto itr = m_OutdoorPvPZone.find(zoneID);
    if (itr == m_OutdoorPvPZone.end())
        return;
    
    itr->second->HandlePlayerEnterMap(guid, zoneID);
}

void OutdoorPvPMgr::HandlePlayerLeaveMap(ObjectGuid guid, uint32 zoneID)
{
    auto itr = m_OutdoorPvPMap.find(zoneID);
    if (itr == m_OutdoorPvPMap.end())
        return;

    for (auto v : itr->second)
        v->HandlePlayerLeaveMap(guid, zoneID);
}

void OutdoorPvPMgr::HandlePlayerEnterArea(ObjectGuid guid, uint32 areaID)
{
    auto itr = m_OutdoorPvPZone.find(areaID);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerEnterArea(guid, areaID);
}

void OutdoorPvPMgr::HandlePlayerLeaveArea(ObjectGuid guid, uint32 areaID)
{
    auto itr = m_OutdoorPvPZone.find(areaID);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerLeaveArea(guid, areaID);
}

OutdoorPvP* OutdoorPvPMgr::GetOutdoorPvPToZoneId(uint32 zoneid)
{
    return Trinity::Containers::MapGetValuePtr(m_OutdoorPvPZone, zoneid);
}

void OutdoorPvPMgr::Update(uint32 diff)
{
    m_UpdateTimer += diff;
    if (m_UpdateTimer > OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL)
    {
        for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
            (*itr)->Update(m_UpdateTimer);
        m_UpdateTimer = 0;
    }
}

bool OutdoorPvPMgr::HandleCustomSpell(Player* player, uint32 spellId, GameObject* go)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleCustomSpell(player, spellId, go))
            return true;
    }
    return false;
}

ZoneScript* OutdoorPvPMgr::GetZoneScript(uint32 zoneId)
{
    return Trinity::Containers::MapGetValuePtr(m_OutdoorPvPZone, zoneId);
}

bool OutdoorPvPMgr::HandleOpenGo(Player* player, ObjectGuid guid)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleOpenGo(player, guid))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleGossipOption(Player* player, ObjectGuid guid, uint32 gossipid)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleGossipOption(player, guid, gossipid))
            return;
    }
}

bool OutdoorPvPMgr::CanTalkTo(Player* player, Creature* creature, GossipMenuItems const& gso)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->CanTalkTo(player, creature, gso))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleDropFlag(Player* player, uint32 spellId)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleDropFlag(player, spellId))
            return;
    }
}

void OutdoorPvPMgr::HandleGameEventStart(uint32 event)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        (*itr)->HandleGameEventStart(event);
    }
}

void OutdoorPvPMgr::HandlePlayerResurrects(Player* player, uint32 zoneid)
{
    OutdoorPvPZone::iterator itr = m_OutdoorPvPZone.find(zoneid);
    if (itr == m_OutdoorPvPZone.end())
        return;

    if (itr->second->HasPlayer(player))
        itr->second->HandlePlayerResurrects(player, zoneid);
}
