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

#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "WaypointManager.h"
#include "Log.h"

WaypointMgr::WaypointMgr()
{
}

WaypointMgr::~WaypointMgr()
{
    for (WaypointPathContainer::iterator itr = _waypointStore.begin(); itr != _waypointStore.end(); ++itr)
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        itr->second.clear();
    }

    for (WaypointPathContainer::iterator itr = _waypointScriptStore.begin(); itr != _waypointScriptStore.end(); ++itr)
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        itr->second.clear();
    }

    _waypointStore.clear();
    _waypointScriptStore.clear();
}

void WaypointMgr::Load()
{
    uint32 oldMSTime = getMSTime();

    //                                                0    1         2           3          4            5           6        7      8       9        10              11
    QueryResult result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, orientation, move_flag, speed, delay, action, action_chance, delay_chance FROM waypoint_data ORDER BY id, point");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 waypoints. DB table `waypoint_data` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        uint32 pathId = fields[0].GetUInt32();
        WaypointPath& path = _waypointStore[pathId];

        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float z = fields[4].GetFloat();
        float o = fields[5].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[1].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->run = fields[6].GetUInt8();
        wp->speed = fields[7].GetFloat();
        wp->delay = fields[8].GetUInt32();
        wp->event_id = fields[9].GetUInt32();
        wp->event_chance = fields[10].GetInt16();
        wp->delay_chance = fields[11].GetInt16();

        path.push_back(wp);
        ++count;
    }
    while (result->NextRow());

    //                                    0    1         2           3          4            5           6        7      8       9        10
    result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, orientation, move_flag, speed, delay, action, action_chance FROM waypoint_data_script ORDER BY id, point");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 waypoints. DB table `waypoint_data_script` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        uint32 pathId = fields[0].GetUInt32();
        WaypointPath& path = _waypointScriptStore[pathId];

        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float z = fields[4].GetFloat();
        float o = fields[5].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[1].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->run = fields[6].GetUInt8();
        wp->speed = fields[7].GetFloat();
        wp->delay = fields[8].GetUInt32();
        wp->event_id = fields[9].GetUInt32();
        wp->event_chance = fields[10].GetInt16();

        path.push_back(wp);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u waypoints in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void WaypointMgr::ReloadPath(uint32 id)
{
    WaypointPathContainer::iterator itr = _waypointStore.find(id);
    if (itr != _waypointStore.end())
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        _waypointStore.erase(itr);
    }

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_DATA_BY_ID);
    stmt->setUInt32(0, id);
    PreparedQueryResult result = WorldDatabase.Query(stmt);
    if (!result)
        return;

    WaypointPath& path = _waypointStore[id];

    do
    {
        Field* fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        float x = fields[1].GetFloat();
        float y = fields[2].GetFloat();
        float z = fields[3].GetFloat();
        float o = fields[4].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[0].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->run = fields[5].GetUInt8();
        wp->speed = fields[6].GetFloat();
        wp->delay = fields[7].GetUInt32();
        wp->event_id = fields[8].GetUInt32();
        wp->event_chance = fields[9].GetUInt8();
        wp->delay_chance = fields[10].GetUInt8();

        path.push_back(wp);

    }
    while (result->NextRow());

    itr = _waypointScriptStore.find(id);
    if (itr != _waypointScriptStore.end())
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        _waypointScriptStore.erase(itr);
    }

    stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_DATA_SCRIPT_BY_ID);
    stmt->setUInt32(0, id);
    result = WorldDatabase.Query(stmt);
    if (!result)
        return;

    WaypointPath& pathS = _waypointScriptStore[id];

    do
    {
        Field* fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        float x = fields[1].GetFloat();
        float y = fields[2].GetFloat();
        float z = fields[3].GetFloat();
        float o = fields[4].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[0].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->run = fields[5].GetUInt8();
        wp->speed = fields[6].GetFloat();
        wp->delay = fields[7].GetUInt32();
        wp->event_id = fields[8].GetUInt32();
        wp->event_chance = fields[9].GetUInt8();

        pathS.push_back(wp);

    }
    while (result->NextRow());
}
