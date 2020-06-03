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

#include "EventObjectData.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"

EventObjectDataStoreMgr::EventObjectDataStoreMgr()
{
}

EventObjectDataStoreMgr::~EventObjectDataStoreMgr()
{
}

EventObjectDataStoreMgr* EventObjectDataStoreMgr::instance()
{
    static EventObjectDataStoreMgr instance;
    return &instance;
}

void EventObjectDataStoreMgr::LoadEventObjectTemplates()
{
    uint32 oldMSTime = getMSTime();

    //                                               0      1      2       3            4             5          6
    QueryResult result = WorldDatabase.Query("SELECT entry, name, radius, SpellID, WorldSafeLocID, ScriptName, Flags FROM eventobject_template;");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "EventObjectDataStoreMgr::LoadEventObjectTemplates() >> Loaded 0 eventobject. DB table `eventobject_template` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();

        EventObjectTemplate& eventObjectTemplate = _eventObjectTemplateMap[fields[index++].GetUInt32()];

        eventObjectTemplate.Name = fields[index++].GetString();
        eventObjectTemplate.radius = fields[index++].GetFloat();
        int32 SpellID = fields[index++].GetInt32();
        eventObjectTemplate.SpellID = abs(SpellID);
        eventObjectTemplate.RemoveSpell = SpellID < 0;
        eventObjectTemplate.WorldSafeLocID = fields[index++].GetUInt32();
        eventObjectTemplate.ScriptID = sObjectMgr->GetScriptId(fields[index++].GetCString());
        eventObjectTemplate.Flags = fields[index++].GetUInt32();
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "EventObjectDataStoreMgr::LoadEventObjectTemplates() >> Loaded %u eventobject template in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void EventObjectDataStoreMgr::LoadEventObjects()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0    1   2      3      4       5           6           7           8            9            10        11  
    QueryResult result = WorldDatabase.Query("SELECT guid, id, map, zoneId, areaId, position_x, position_y, position_z, orientation, spawnMask, phaseMask, PhaseId "
        "FROM eventobject ORDER BY `map` ASC, `guid` ASC");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "EventObjectDataStoreMgr::LoadEventObjects() >> Loaded 0 eventobject. DB table `eventobject` is empty.");
        return;
    }

    // Build single time for check spawnmask
    std::map<uint32, uint64> spawnMasks;
    for (auto& mapDifficultyPair : sDB2Manager.GetAllMapsDifficultyes())
        for (auto& difficultyPair : mapDifficultyPair.second)
            spawnMasks[mapDifficultyPair.first] |= (UI64LIT(1) << difficultyPair.first);

    _eventObjectData.rehash(result->GetRowCount());
    std::map<uint32, EventObjectData*> lastEntry;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint8 index = 0;

        ObjectGuid::LowType guid = fields[index++].GetUInt64();
        uint32 entry = fields[index++].GetUInt32();

        EventObjectData& data = _eventObjectData[guid];
        data.guid = guid;
        data.id = entry;
        data.mapid = fields[index++].GetUInt16();
        data.zoneId = fields[index++].GetUInt16();
        data.areaId = fields[index++].GetUInt16();
        data.Pos.m_positionX = fields[index++].GetFloat();
        data.Pos.m_positionY = fields[index++].GetFloat();
        data.Pos.m_positionZ = fields[index++].GetFloat();
        data.Pos.m_orientation = fields[index++].GetFloat();
        data.spawnMask = fields[index++].GetUInt64();
        data.phaseMask = fields[index++].GetUInt32();

        Tokenizer phasesToken(fields[index++].GetString(), ' ', 100);
        for (auto itr : phasesToken)
            if (PhaseEntry const* phase = sPhaseStore.LookupEntry(uint32(strtoull(itr, nullptr, 10))))
                data.PhaseID.insert(phase->ID);

        // check near npc with same entry.
        auto lastObject = lastEntry.find(entry);
        if (lastObject != lastEntry.end())
        {
            if (data.mapid == lastObject->second->mapid)
            {
                float dx1 = lastObject->second->Pos.GetPositionX() - data.Pos.GetPositionX();
                float dy1 = lastObject->second->Pos.GetPositionY() - data.Pos.GetPositionY();
                float dz1 = lastObject->second->Pos.GetPositionZ() - data.Pos.GetPositionZ();

                float distsq1 = dx1*dx1 + dy1*dy1 + dz1*dz1;
                if (distsq1 < 0.5f)
                {
                    // split phaseID
                    for (auto phaseID : data.PhaseID)
                        lastObject->second->PhaseID.insert(phaseID);

                    lastObject->second->phaseMask |= data.phaseMask;
                    lastObject->second->spawnMask |= data.spawnMask;
                    WorldDatabase.PExecute("UPDATE eventobject SET phaseMask = %u, spawnMask = " UI64FMTD " WHERE guid = %u", lastObject->second->phaseMask, lastObject->second->spawnMask, lastObject->second->guid);
                    WorldDatabase.PExecute("DELETE FROM eventobject WHERE guid = %u", guid);
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadEventObjects >> Table `eventobject` have clone npc %u witch stay too close (dist: %f). original npc guid %u. npc with guid %u will be deleted.", entry, distsq1, lastObject->second->guid, guid);
                    continue;
                }
            }
            else
                lastEntry[entry] = &data;
        }
        else
            lastEntry[entry] = &data;

        if (!sMapStore.LookupEntry(data.mapid))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadEventObjects >> Table `eventobject` have eventobject (GUID: " UI64FMTD ") that spawned at not existed map (Id: %u), skipped.", guid, data.mapid);
            continue;
        }

        if (data.spawnMask & ~spawnMasks[data.mapid])
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadEventObjects >> Table `eventobject` have eventobject (GUID: " UI64FMTD ") that have wrong spawn mask %u including not supported difficulty modes for map (Id: %u) spawnMasks[data.mapid]: %u.", guid, data.spawnMask, data.mapid, spawnMasks[data.mapid]);
            WorldDatabase.PExecute("UPDATE eventobject SET spawnMask = " UI64FMTD " WHERE guid = %u", spawnMasks[data.mapid], guid);
            data.spawnMask = spawnMasks[data.mapid];
        }

        if (data.phaseMask == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadEventObjects >> Table `eventobject` have eventobject (GUID: " UI64FMTD " Entry: %u) with `phaseMask`=0 (not visible for anyone), set to 1.", guid, data.id);
            data.phaseMask = 1;
        }

        // Add to grid if not managed by the game event or pool system
        sObjectMgr->AddEventObjectToGrid(guid, &data);

        if (!data.zoneId || !data.areaId)
        {
            uint32 zoneId = 0;
            uint32 areaId = 0;

            sMapMgr->GetZoneAndAreaId(zoneId, areaId, data.mapid, data.Pos.GetPositionX(), data.Pos.GetPositionY(), data.Pos.GetPositionZ());

            data.zoneId = zoneId;
            data.areaId = areaId;

            WorldDatabase.PExecute("UPDATE eventobject SET zoneId = %u, areaId = %u WHERE guid = %u", zoneId, areaId, guid);
        }

        ++count;

    } while (result->NextRow());
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "EventObjectDataStoreMgr::LoadEventObjects() >> Loaded %u eventobject in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

EventObjectTemplate const* EventObjectDataStoreMgr::GetEventObjectTemplate(uint32 entry)
{
    return Trinity::Containers::MapGetValuePtr(_eventObjectTemplateMap, entry);
}

EventObjectTemplate const* EventObjectDataStoreMgr::AddEventObjectTemplate(uint32 entry, float radius, uint32 spell, uint32 worldsafe)
{
    EventObjectTemplate& eventObjectTemplate = _eventObjectTemplateMap[entry];
    eventObjectTemplate.Name = "";
    eventObjectTemplate.radius = radius;
    eventObjectTemplate.SpellID = spell;
    eventObjectTemplate.WorldSafeLocID = worldsafe;
    eventObjectTemplate.ScriptID = 0;

    WorldDatabase.PExecute("INSERT INTO eventobject_template SET SpellID = %u, radius = %f, WorldSafeLocID = %u, entry = %u", spell, radius, worldsafe, entry);

    return &eventObjectTemplate;
}

EventObjectData const* EventObjectDataStoreMgr::GetEventObjectData(ObjectGuid::LowType const& guid) const
{
    return Trinity::Containers::MapGetValuePtr(_eventObjectData, guid);
}

EventObjectData& EventObjectDataStoreMgr::NewOrExistEventObjectData(ObjectGuid::LowType const& guid)
{
    return _eventObjectData[guid];
}
