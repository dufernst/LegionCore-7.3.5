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

#ifndef TRINITY_MAPMANAGER_H
#define TRINITY_MAPMANAGER_H

#include "Common.h"
#include "Map.h"
#include "Position.h"

class WorldLocation;
class ChatHandler;

class MapManager
{
    typedef std::vector<Map*> MapMapType;

    MapManager();
    ~MapManager();

    MapManager(const MapManager &) = delete;
    MapManager& operator=(const MapManager &) = delete;

    public:
        static MapManager* instance();

        Map* CreateBaseMap(uint32 mapId);
        Map* FindBaseNonInstanceMap(uint32 mapId) const;
        Map* CreateMap(uint32 mapId, Player* player);
        Map* FindMap(uint32 mapId, uint32 instanceId) const;

        uint32 GetAreaId(uint32 mapid, float x, float y, float z) const;
        uint32 GetZoneId(uint32 mapid, float x, float y, float z) const;
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid, uint32 mapid, float x, float y, float z);

        void Initialize();
        void Update(uint32);

        void SetGridCleanUpDelay(uint32 t);

        void SetMapUpdateInterval(uint32 t);

        //void LoadGrid(int mapid, int instId, float x, float y, const WorldObject* obj, bool no_unload = false);
        void UnloadAll();

        static bool ExistMapAndVMap(uint32 mapid, float x, float y);
        static bool IsValidMAP(uint32 mapid, bool startUp);

        static bool IsValidMapCoord(uint32 mapid, float x, float y);
        static bool IsValidMapCoord(uint32 mapid, float x, float y, float z);
        static bool IsValidMapCoord(uint32 mapid, float x, float y, float z, float o);
        static bool IsValidMapCoord(WorldLocation const& loc);

        void DoDelayedMovesAndRemoves();

        bool CanPlayerEnter(uint32 mapid, Player* player, bool loginCheck = false);
        void InitializeVisibilityDistanceInfo();

        /* statistics */
        uint32 GetNumInstances();
        uint32 GetNumPlayersInInstances();

        // Instance ID management
        void InitInstanceIds();
        uint32 GenerateInstanceId();

        void FindSessionInAllMaps(uint32 accId, ChatHandler* handler);
        void LogInfoAllMaps();
        void SetUnloadGarrison(uint32 lowGuid);

        Map* FindBaseMap(uint32 mapId) const { return i_maps[mapId]; }
        std::vector<std::thread*> _mapThreads;
        uint16 _mapCount;

    private:

        uint32 i_gridCleanUpDelay;
        MapMapType i_maps;

        uint32 _nextInstanceId;
        uint8 _mapInfoCounter;

};
#define sMapMgr MapManager::instance()
#endif
