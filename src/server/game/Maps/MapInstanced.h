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

#ifndef TRINITY_MAP_INSTANCED_H
#define TRINITY_MAP_INSTANCED_H

#include "Map.h"
#include "InstanceSaveMgr.h"
#include "DBCEnums.h"

class GarrisonMap;

typedef std::unordered_map< uint32, Map*> InstancedMaps;

class MapInstanced : public Map
{
    friend class MapManager;

    public:

        MapInstanced(uint32 id, time_t expiry);
        ~MapInstanced() {}

        // functions overwrite Map versions
        void Update(const uint32) override;
        void DelayedUpdate(const uint32 diff) override;
        void UpdateTransport(uint32 diff) override;
        void UpdateSessions(uint32 diff) override;
        void StopInstance();

        void UnloadAll() override;
        bool CanEnter(Player* player) override;

        Map* CreateInstanceForPlayer(const uint32 mapId, Player* player);
        Map* CreateZoneForPlayer(const uint32 mapId, Player* player);
        Map* FindInstanceMap(uint32 instanceId) const;
        Map* FindGarrisonMap(uint32 instanceId) const;

        bool DestroyInstance(InstancedMaps::iterator &itr);
        bool DestroyGarrison(InstancedMaps::iterator &itr);

        void AddGridMapReference(const GridCoord& p);
        void RemoveGridMapReference(GridCoord const& p);

        InstancedMaps &GetInstancedMaps() { return m_InstancedMaps; }
        void InitVisibilityDistance() override;

        void TerminateThread();

        InstancedMaps m_InstancedMaps;
        InstancedMaps m_GarrisonedMaps;
        std::map<uint32, std::thread*> _zoneThreads;

    private:
        InstanceMap* CreateInstance(uint32 InstanceId, InstanceSave* save, Difficulty difficulty);
        GarrisonMap* CreateGarrison(uint32 instanceId, Player* owner);
        BattlegroundMap* CreateBattleground(uint32 InstanceId, Battleground* bg);
        ZoneMap* CreateZoneMap(uint32 zoneId, Player* player);

        std::recursive_mutex m_lock;

        uint16 GridMapReference[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
};
#endif
