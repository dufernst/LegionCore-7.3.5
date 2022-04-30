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

#include <cds/gc/hp.h>

#include "BattlefieldMgr.h"
#include "BattlegroundMgr.h"
#include "BrawlersGuild.h"
#include "CellImpl.h"
#include "DisableMgr.h"
#include "DynamicTree.h"
#include "GridInfo.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstancePackets.h"
#include "InstanceScript.h"
#include "LFGMgr.h"
#include "Map.h"
#include "MapInstanced.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "MMapFactory.h"
#include "ObjectAccessor.h"
#include "ObjectGridLoader.h"
#include "ObjectMgr.h"
#include "OutdoorPvPMgr.h"
#include "ScenarioMgr.h"
#include "ScriptMgr.h"
#include "StringFormat.h"
#include "Totem.h"
#include "Transport.h"
#include "Vehicle.h"
#include "VMapFactory.h"
#include "WeatherMgr.h"
#include "WildBattlePet.h"
#include "WorldStateMgr.h"
#include "GuildMgr.h"

namespace {

union u_map_magic
{
    char asChar[4];
    uint32 asUInt;
};

u_map_magic MapMagic        = { {'M','A','P','S'} };
u_map_magic MapVersionMagic = { {'v','1','.','9'} };
u_map_magic MapAreaMagic    = { {'A','R','E','A'} };
u_map_magic MapHeightMagic  = { {'M','H','G','T'} };
u_map_magic MapLiquidMagic  = { {'M','L','I','Q'} };

#define DEFAULT_GRID_EXPIRY     300
#define MAX_GRID_LOAD_TIME      50
#define MAX_CREATURE_ATTACK_RADIUS  (45.0f * sWorld->getRate(RATE_CREATURE_AGGRO))

typedef void (*GridStateUpdate)(Map &, Map::GridContainerType::iterator, uint32);

void InvalidStateUpdate(Map &, Map::GridContainerType::iterator, uint32) { }

void ActiveStateUpdate(Map &map, Map::GridContainerType::iterator itr, uint32 diff)
{
    auto &grid = itr->second;
    auto &info = grid.getGridInfo();

    // Only check grid activity every (grid_expiry/10) ms, because it's really useless to do it every cycle
    info.UpdateTimeTracker(diff);
    if (!info.getTimeTracker().Passed())
        return;

    if (!grid.GetWorldObjectCountInNGrid<Player>() && !map.ActiveObjectsNearGrid(grid))
    {
        Trinity::ObjectGridStoper worker;
        grid.VisitAllGrids(Trinity::makeGridVisitor(worker));
        grid.SetGridState(GRID_STATE_IDLE);

        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Grid[%u, %u] on map %u moved to IDLE state",
                     grid.getX(), grid.getY(), map.GetId());
    }
    else
    {
        map.ResetGridExpiry(grid, 0.1f);
    }
}

void IdleStateUpdate(Map &map, Map::GridContainerType::iterator itr, uint32)
{
    auto &grid = itr->second;

    map.ResetGridExpiry(grid);
    grid.SetGridState(GRID_STATE_REMOVAL);
    TC_LOG_DEBUG(LOG_FILTER_MAPS, "Grid[%u, %u] on map %u moved to REMOVAL state",
                 grid.getX(), grid.getY(), map.GetId());
}

void RemovalStateUpdate(Map &map, Map::GridContainerType::iterator itr, uint32 diff)
{
    auto &grid = itr->second;
    auto &info = grid.getGridInfo();

    if (info.getUnloadLock())
        return;

    info.UpdateTimeTracker(diff);
    if (!info.getTimeTracker().Passed())
        return;

    if (!map.UnloadGrid(itr, false))
    {
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Grid[%u, %u] for map %u differed unloading due to players or active objects nearby",
                     grid.getX(), grid.getY(), map.GetId());
        map.ResetGridExpiry(grid);
    }
}

GridStateUpdate si_GridStates[MAX_GRID_STATE] =
{
    &InvalidStateUpdate,
    &ActiveStateUpdate,
    &IdleStateUpdate,
    &RemovalStateUpdate
};

struct CorpseGridReset final
{
    void Visit(CorpseMapType &m)
    {
        while (!m.empty())
            m.back()->RemoveFromGrid();
    }

    template <typename NotInterested>
    void Visit(NotInterested &) { }
};

} // namespace

void Map::VisitNearbyCellsOf(WorldObject* obj)
{
    // Check for valid position
    if (!obj->IsPositionValid())
        return;

    ObjectUpdater objectUpdater;

    // for creature
    auto gridVisitor(Trinity::makeGridVisitor(objectUpdater));
    // for pets
    auto worldVisitor(Trinity::makeWorldVisitor(objectUpdater));

    // Update mobs/objects in ALL visible cells around object!
    CellArea area = Cell::CalculateCellArea(obj->GetPositionX(), obj->GetPositionY(), obj->GetGridActivationRange());

    for (uint32 x = area.low_bound.x_coord; x <= area.high_bound.x_coord; ++x)
    {
        for (uint32 y = area.low_bound.y_coord; y <= area.high_bound.y_coord; ++y)
        {
            // marked cells are those that have been visited
            // don't visit the same cell twice
            uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x; // TOTAL_NUMBER_OF_CELLS_PER_MAP = 512
            if (isCellMarked(cell_id))
                continue;

            markCell(cell_id);

            Cell cell(CellCoord(x, y));
            cell.SetNoCreate();

            Visit(cell, gridVisitor);
            Visit(cell, worldVisitor);

            uint32 pullY = y / sWorld->getIntConfig(CONFIG_SIZE_CELL_FOR_PULL); // Max y = MAX_NUMBER_OF_GRIDS - 1
            uint32 pullX = x / sWorld->getIntConfig(CONFIG_SIZE_CELL_FOR_PULL); // Max x = MAX_NUMBER_OF_GRIDS - 1
            // count of pull is TOTAL_NUMBER_OF_CELLS_PER_MAP / CONFIG_SIZE_CELL_FOR_PULL
            uint32 pullId = (pullY * (TOTAL_NUMBER_OF_CELLS_PER_MAP / sWorld->getIntConfig(CONFIG_SIZE_CELL_FOR_PULL))) + pullX;
            std::vector<WorldObject*>& collectObjects = i_objectUpdater[pullY % 2][(pullY + pullX) % 2][pullId];
            for (auto& obj : objectUpdater.i_collectObjects)
                if (i_objectTest.find(obj) == i_objectTest.end())
                {
                    i_objectTest.insert(obj);
                    collectObjects.push_back(obj);
                }
            objectUpdater.i_collectObjects.clear();
        }
    }
}

void Map::updateCollected(std::vector<WorldObject*>& objectsToUpdate, uint32 diff, volatile uint32 _mapId, volatile uint32 _instanceId)
{
    if (b_isMapUnload)
        return;

    if (!objectsToUpdate.empty())
    {
        for (auto& object : objectsToUpdate)
        {
            try
            {
                if (object && object->IsInWorld() && !object->IsDelete() && !object->IsPreDelete())
                {
                    if (b_isMapUnload)
                        return;

                    uint32 _s = getMSTime();
                    object->Update(diff);
                    uint32 _ms = GetMSTimeDiffToNow(_s);
                    if (_ms > 200)
                    {
                        sLog->outDiff("ObjectUpdater::updateCollected: type - %u, e:%u(g:%u). time - %ums diff %u isActiveObject %u visible %f mapId %u i_InstanceId %u",
                            object->GetTypeId(), object->GetEntry(), object->GetGUIDLow(), _ms, diff, object->isActiveObject(), object->GetVisibilityRange(), object->GetMapId(), i_InstanceId);

                        if (Unit* unit = object->ToUnit())
                            sLog->outDiff("ObjectUpdater::updateCollected: e:%u(g:%u) castCount %u targetCount %u eventCount %u functionCount %u",
                                unit->GetEntry(), unit->GetGUIDLow(), unit->_castCount, unit->_targetCount, unit->_eventCount, unit->_functionCount);
                    }
                }
            }
            catch (std::exception& e)
            {
                sLog->outTryCatch("Exception caught in Map::updateCollected %s _mapId %u InstanceId %u guid %s", e.what(), _mapId, _instanceId, object->GetGUID().ToString().c_str());

                object->m_isUpdate = false;
                if (Creature* creature = object->ToCreature())
                    creature->Respawn(true);
                else if (GameObject* gameobject = object->ToGameObject())
                    gameobject->Respawn();
                else
                {
                    object->RemoveFromWorld();
                    object->AddObjectToRemoveList();
                }
            }
        }
        objectsToUpdate.clear();
    }
}

ZoneDynamicInfo::ZoneDynamicInfo()
{ 
    MusicID = 0;
    WeatherID = WEATHER_STATE_FINE;
    WeatherGrade = 0.0f;
    OverrideLightID = 0;
    LightFadeInTime = 0;
    DefaultWeather = nullptr;
}

Map::~Map()
{
    sScriptMgr->OnDestroyMap(this);
    DepopulateBattlePet();
    m_scenarios.clear();

    Map::UnloadAll();

    for (auto worldObject = i_worldObjects.begin(); worldObject != i_worldObjects.end(); ++worldObject)
    {
        WorldObject* obj = *worldObject;
        ASSERT(obj->IsWorldObject());
        //ASSERT(obj->IsCorpse());
        if (obj->GetMap() != this)
            continue;

        if (!obj->IsInWorld())
            continue;

        obj->RemoveFromWorld();
        obj->ResetMap();
    }

    if (!m_scriptSchedule.empty())
        sScriptMgr->DecreaseScheduledScriptCount(m_scriptSchedule.size());

    if (threadPool)
    {
        threadPool->stop();
        delete threadPool;
    }

    // MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(GetId(), i_InstanceId);

    b_isMapStop = true;
}

MapEntry const* Map::GetEntry() const
{
    return i_mapEntry;
}

bool Map::CanUnload(uint32 diff)
{
    if (!m_unloadTimer)
        return false;
    if (m_unloadTimer <= diff)
        return true;
    m_unloadTimer -= diff;
    return false;
}

bool Map::ExistMap(uint32 mapid, int gx, int gy)
{
    std::string fileName = Trinity::StringFormat("%smaps/%04u_%02u_%02u.map", sWorld->GetDataPath().c_str(), mapid, gx, gy);

    bool ret = false;
    FILE* pf=fopen(fileName.c_str(), "rb");

    if (!pf)
        TC_LOG_ERROR(LOG_FILTER_MAPS, "Map file '%s': does not exist!", fileName.c_str());
    else
    {
        map_fileheader header;
        if (fread(&header, sizeof(header), 1, pf) == 1)
        {
            if (header.mapMagic != MapMagic.asUInt || header.versionMagic != MapVersionMagic.asUInt)
                TC_LOG_ERROR(LOG_FILTER_MAPS, "Map file '%s' is from an incompatible clientversion. Please recreate using the mapextractor.", fileName.c_str());
            else
                ret = true;
        }
        fclose(pf);
    }

    return ret;
}

bool Map::ExistVMap(uint32 mapid, int gx, int gy)
{
    if (VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager())
    {
        if (vmgr->isMapLoadingEnabled())
        {
            bool exists = vmgr->existsMap((sWorld->GetDataPath()+ "vmaps").c_str(),  mapid, gx, gy);
            if (!exists)
            {
                std::string name = vmgr->getDirFileName(mapid, gx, gy);
                TC_LOG_ERROR(LOG_FILTER_MAPS, "VMap file '%s' is missing or points to wrong version of vmap file. Redo vmaps with latest version of vmap_assembler.exe.", (sWorld->GetDataPath()+"vmaps/"+name).c_str());
                return false;
            }
        }
    }

    return true;
}

Map const* Map::GetParent() const
{
    return m_parentMap;
}

void Map::LoadMMap(int gx, int gy)
{
    if (!DisableMgr::IsPathfindingEnabled(GetId()))
        return;

    bool mmapLoadResult = MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld->GetDataPath(), GetId(), gx, gy);

    if (mmapLoadResult)
        TC_LOG_DEBUG(LOG_FILTER_MMAPS, "MMAP loaded name:%s, id:%d, x:%d, y:%d (mmap rep.: x:%d, y:%d)", GetMapName(), GetId(), gx, gy, gx, gy);
    else
        TC_LOG_DEBUG(LOG_FILTER_MMAPS, "Could not load MMAP name:%s, id:%d, x:%d, y:%d (mmap rep.: x:%d, y:%d)", GetMapName(), GetId(), gx, gy, gx, gy);
}

void Map::LoadVMap(int gx, int gy)
{
    if (!VMAP::VMapFactory::createOrGetVMapManager()->isMapLoadingEnabled())
        return;

    int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap((sWorld->GetDataPath()+ "vmaps").c_str(),  GetId(), gx, gy);
    switch (vmapLoadResult)
    {
        case VMAP::VMAP_LOAD_RESULT_OK:
            TC_LOG_DEBUG(LOG_FILTER_VMAPS, "VMAP loaded name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), gx, gy, gx, gy);
            break;
        case VMAP::VMAP_LOAD_RESULT_ERROR:
            TC_LOG_DEBUG(LOG_FILTER_VMAPS, "Could not load VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), gx, gy, gx, gy);
            break;
        case VMAP::VMAP_LOAD_RESULT_IGNORED:
            TC_LOG_DEBUG(LOG_FILTER_VMAPS, "Ignored VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), gx, gy, gx, gy);
            break;
        default:
            break;
    }
}

void Map::LoadMap(int gx, int gy, bool reload)
{
    LoadMapImpl(this, gx, gy, reload);
    // for (Map* childBaseMap : *m_childTerrainMaps)
        // childBaseMap->LoadMap(gx, gy, reload);
}

void Map::LoadMapImpl(Map* map, int gx, int gy, bool reload)
{
    if (map->i_InstanceId != 0)
    {
        if (map->GridMaps[gx][gy])
            return;

        // load grid map for base map
        if (!map->m_parentMap->GridMaps[gx][gy])
            map->m_parentMap->EnsureGridCreated_i(GridCoord(63-gx, 63-gy));

        static_cast<MapInstanced*>(map->m_parentMap)->AddGridMapReference(GridCoord(gx, gy));
        map->GridMaps[gx][gy] = map->m_parentMap->GridMaps[gx][gy];
        return;
    }

    if (map->GridMaps[gx][gy] && !reload)
        return;

    //map already load, delete it before reloading (Is it necessary? Do we really need the ability the reload maps during runtime?)
    if (map->GridMaps[gx][gy])
    {
        #ifdef WIN32
        TC_LOG_INFO(LOG_FILTER_MAPS, "Unloading previously loaded map %u before reloading.", map->GetId());
        #endif
        sScriptMgr->OnUnloadGridMap(map, map->GridMaps[gx][gy], gx, gy);

        delete (map->GridMaps[gx][gy]);
        map->GridMaps[gx][gy]= nullptr;
    }

    // map file name
    std::string fileName = Trinity::StringFormat("%smaps/%04u_%02u_%02u.map", sWorld->GetDataPath().c_str(), map->GetId(), gx, gy);
    #ifdef WIN32
    TC_LOG_INFO(LOG_FILTER_MAPS, "Loading map %s gx: %i, gy: %i", fileName.c_str(), gx, gy);
    #endif
    // loading data
    map->GridMaps[gx][gy] = new GridMap();
    if (!map->GridMaps[gx][gy]->loadData(fileName.c_str()))
        TC_LOG_ERROR(LOG_FILTER_MAPS, "Error loading map file: \n %s\n", fileName.c_str());

    sScriptMgr->OnLoadGridMap(map, map->GridMaps[gx][gy], gx, gy);
}

void Map::UnloadMap(int gx, int gy)
{
    // for (Map* childBaseMap : *m_childTerrainMaps)
        // childBaseMap->UnloadMap(gx, gy);

    UnloadMapImpl(this, gx, gy);
}

void Map::UnloadMapImpl(Map* map, int gx, int gy)
{
    if (map->i_InstanceId == 0)
    {
        if (map->GridMaps[gx][gy])
        {
            map->GridMaps[gx][gy]->unloadData();
            delete map->GridMaps[gx][gy];
        }
    }
    else
        static_cast<MapInstanced*>(map->m_parentMap)->RemoveGridMapReference(GridCoord(gx, gy));

    map->GridMaps[gx][gy] = nullptr;
}

void Map::UpdateOutdoorPvPScript()
{
    if (!m_parentMap || !CanCreatedZone() || !i_InstanceId)
        return;

    if (!m_parentMap->OutdoorPvPList || m_parentMap->OutdoorPvPList->empty())
    {
        if (!sOutdoorPvPMgr->GetOutdoorPvPMap(m_parentMap->GetId()))
            return;

        auto set = *sOutdoorPvPMgr->GetOutdoorPvPMap(m_parentMap->GetId()); // we need copy it else we do it twice
        for (auto itr : set)
            if (itr && itr->ThisZone(i_InstanceId))
            {
                if (itr->GetMap() && itr->GetMap() != this)
                {
                    if (itr->SizeZones() == 1)
                        itr->SetMap(this);
                    else
                    {
                        itr->RemoveZone(i_InstanceId);
                        auto pvp = sScriptMgr->CreateOutdoorPvP(sOutdoorPvPMgr->GetOutdoorPvPData(OutdoorPvPTypes(itr->GetTypeId())));
                        if (pvp)
                        {
                            sOutdoorPvPMgr->AddOutdoorPvP(pvp);
                            pvp->RegisterZone(i_InstanceId);
                        }
                    }
                }
                else
                    itr->RegisterZone(i_InstanceId);
            }

        OutdoorPvPList = sOutdoorPvPMgr->GetOutdoorPvPMap(m_parentMap->GetId());
    }
    else
    {
        auto set = *(m_parentMap->OutdoorPvPList); // we need copy it else we do it twice
        for (auto itr : set)
            if (itr && itr->ThisZone(i_InstanceId))
            {
                if (itr->SizeZones() > 1)
                {
                    itr->RemoveZone(i_InstanceId);
                    auto pvp = sScriptMgr->CreateOutdoorPvP(sOutdoorPvPMgr->GetOutdoorPvPData(OutdoorPvPTypes(itr->GetTypeId())));
                    if (pvp)
                    {
                        sOutdoorPvPMgr->AddOutdoorPvP(pvp);
                        pvp->RegisterZone(i_InstanceId);
                    }
                }
                else
                {
                    if (itr->GetMap())
                        itr->SetMap(this);
                    else
                        itr->RegisterZone(i_InstanceId);
                }
            }

        OutdoorPvPList = m_parentMap->OutdoorPvPList;
    }
}

void Map::LoadMapAndVMap(int gx, int gy)
{
    LoadMap(gx, gy);
    if (i_InstanceId == 0)
    {
        LoadVMap(gx, gy);                                   // Only load the data for the base map
        LoadMMap(gx, gy);
    }
}

Map::Map(uint32 id, time_t expiry, uint32 InstanceId, Difficulty difficulty, Map* _parent) :
m_updateTime(0), m_sessionTime(0), m_mapLoopCounter(0),
m_activeNonPlayersIter(m_activeNonPlayers.end()), i_grids(), GridMaps()
{
    i_mapEntry = sMapStore.LookupEntry(id);
    i_difficulty = difficulty;
    i_lootDifficulty = difficulty;
    i_InstanceId = InstanceId;
    m_unloadTimer = 0;
    m_VisibleDistance = DEFAULT_VISIBILITY_DISTANCE;
    m_VisibilityNotifyPeriod = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
    i_gridExpiry = expiry;
    i_scriptLock = false;
    b_isMapUnload = false;
    b_isMapStop = false;
    OutdoorPvPList = nullptr;
    BattlefieldList = nullptr;

    if (IsBattlegroundOrArena())
    {
        i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_PVP_MAP_UPDATE));
        i_timer_obj.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_PVP_MAP_UPDATE));
    }
    else if (IsDungeon())
    {
        i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_INSTANCE_UPDATE));
        i_timer_obj.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_INSTANCE_UPDATE));
    }
    else
    {
        i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_MAPUPDATE));
        i_timer_obj.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_OBJECT_UPDATE));
    }

    i_timer_se.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_MAP_SESSION_UPDATE));
    i_timer_op.SetInterval(1000); // OutdoorPvP timer update
    m_respawnChallenge = 0;

    if (CanCreatedZone() || CanCreatedThread())
    {
        threadPool = new ThreadPoolMap();
        threadPool->start(sWorld->getIntConfig(CONFIG_MAP_NUMTHREADS));
    }
    else
        threadPool = nullptr;

    m_parentMap = (_parent ? _parent : this);

    //lets initialize visibility distance for map
    Map::InitVisibilityDistance();

    _weatherUpdateTimer.SetInterval(time_t(1 * IN_MILLISECONDS));

    MMAP::MMapFactory::createOrGetMMapManager()->loadMapInstance(sWorld->GetDataPath(), GetId(), GetThreadID());

    sScriptMgr->OnCreateMap(this);

    m_Transports.clear();

    if(float _distMap = GetVisibleDistance(TYPE_VISIBLE_MAP, id))
        m_VisibleDistance = _distMap;

    _defaultLight = sDB2Manager.GetDefaultMapLight(id);

    // Create brawler guild only for this map
    if (id == 369 || id == 1043)
        m_brawlerGuild = new BrawlersGuild(id == 369 ? ALLIANCE_GUILD : HORDE_GUILD, this);
    else
        m_brawlerGuild = nullptr;

    m_activeEntry = 0;
    m_activeEncounter = 0;

    if (id == 1669) // Argus, temporary hack
    {
        LoadGrid(-2666.37f, 8813.06f);
        LoadGrid(607.212f, 1520.7f);
        LoadGrid(4842.4f, 9889.84f);
    }

    m_currentSession = nullptr;
}

void Map::InitVisibilityDistance()
{
    //init visibility for continents
    m_VisibleDistance = World::GetMaxVisibleDistanceOnContinents();
    m_VisibilityNotifyPeriod = World::GetVisibilityNotifyPeriodOnContinents();
}

float Map::GetVisibilityRange(uint32 zoneId /*= 0*/, uint32 areaId /*= 0*/) const
{
    if (areaId)
        if (float distArea = GetVisibleDistance(TYPE_VISIBLE_AREA, areaId))
            return distArea;

    if (zoneId)
        if (float distZone = GetVisibleDistance(TYPE_VISIBLE_ZONE, zoneId))
            return distZone;

    return m_VisibleDistance;
}

void Map::AddToGrid(Player *obj, Cell const &cell)
{
    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ngrid->GetGrid(cell.CellX(), cell.CellY()).AddWorldObject(obj);
}

void Map::AddToGrid(Creature *obj, Cell const &cell)
{
    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    if (obj->IsWorldObject())
        ngrid->GetGrid(cell.CellX(), cell.CellY()).AddWorldObject(obj);
    else
        ngrid->GetGrid(cell.CellX(), cell.CellY()).AddGridObject(obj);

    obj->SetCurrentCell(cell);
}

void Map::AddToGrid(GameObject *obj, Cell const &cell)
{
    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ngrid->GetGrid(cell.CellX(), cell.CellY()).AddGridObject(obj);

    obj->SetCurrentCell(cell);
}

template<>
void Map::AddToGrid(DynamicObject* obj, Cell const& cell)
{
    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ngrid->GetGrid(cell.CellX(), cell.CellY()).AddGridObject(obj);

    obj->SetCurrentCell(cell);
}

template<>
void Map::AddToGrid(AreaTrigger* obj, Cell const& cell)
{
    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ngrid->GetGrid(cell.CellX(), cell.CellY()).AddGridObject(obj);

    obj->SetCurrentCell(cell);
}

void Map::SwitchGridContainers(Creature* obj, bool on)
{
    ASSERT(!obj->IsPermanentWorldObject());
    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    if (!p.IsCoordValid())
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "Map::SwitchGridContainers: Object %s has invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID().ToString().c_str(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if (!IsGridLoaded(GridCoord(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    if (sLog->ShouldLog(LOG_FILTER_MAPS, LOG_LEVEL_DEBUG))
    {
        uint32 const grid_x = cell.data.Part.grid_x;
        uint32 const grid_y = cell.data.Part.grid_y;
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Switch object %s from grid[%u, %u] %u", obj->GetGUID().ToString().c_str(), grid_x, grid_y, on);
    }

    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(ngrid != nullptr);

    auto &grid = ngrid->GetGrid(cell.CellX(), cell.CellY());

    if (obj->IsInGrid())
        obj->RemoveFromGrid();

    if (on)
    {
        grid.AddWorldObject(obj);
        AddWorldObject(obj);
    }
    else
    {
        grid.AddGridObject(obj);
        RemoveWorldObject(obj);
    }

    obj->m_isTempWorldObject = on;
}

void Map::SwitchGridContainers(GameObject* obj, bool on)
{
    ASSERT(!obj->IsPermanentWorldObject());
    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    if (!p.IsCoordValid())
    {
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Map::SwitchGridContainers: Object %s has invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID().ToString().c_str(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if (!IsGridLoaded(GridCoord(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    if (sLog->ShouldLog(LOG_FILTER_MAPS, LOG_LEVEL_DEBUG))
    {
        // Extract bitfield values
        uint32 const grid_x = cell.data.Part.grid_x;
        uint32 const grid_y = cell.data.Part.grid_y;

        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Switch object %s from grid[%u, %u] %u", obj->GetGUID().ToString().c_str(), grid_x, grid_y, on);
    }

    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(ngrid != NULL);

    auto &grid = ngrid->GetGrid(cell.CellX(), cell.CellY());

    if (obj->IsInGrid())
        obj->RemoveFromGrid(); //This step is not really necessary but we want to do ASSERT in remove/add

    grid.AddGridObject(obj);
    if (on)
        AddWorldObject(obj);
    else
        RemoveWorldObject(obj);
}

template<class T>
void Map::DeleteFromWorld(T* obj)
{
    // Note: In case resurrectable corpse and pet its removed from global lists in own destructor
    obj->SetDelete();
    delete obj;
}

template<>
void Map::DeleteFromWorld(Player* player)
{
    sObjectAccessor->RemoveObject(player);
    player->SetDelete();
    // m_Functions.AddFunction([player]() -> void {delete player;}, m_Functions.CalculateTime(120000));
    delete player;
}

template<>
void Map::DeleteFromWorld(Transport* transport)
{
    sObjectAccessor->RemoveObject(transport);
    delete transport;
}

//Create NGrid so the object can be added to it
//But object data is not loaded here
void Map::EnsureGridCreated(const GridCoord &p)
{
    if (!getNGrid(p.x_coord, p.y_coord))
    {
        GridGuardType guard(i_gridLock);
        EnsureGridCreated_i(p);
    }
}

//Create NGrid so the object can be added to it
//But object data is not loaded here
void Map::EnsureGridCreated_i(const GridCoord &p)
{
    if (getNGrid(p.x_coord, p.y_coord))
        return;

    // Such way allows to retrieve hint position for insert, so it
    // will have amortized constant complexity instead of logarithmic
    size_t const key = p.x_coord * MAX_NUMBER_OF_GRIDS + p.y_coord;
    auto lb = i_loadedGrids.lower_bound(key);
    if (lb != i_loadedGrids.end() && !i_loadedGrids.key_comp()(key, lb->first))
        return;

    TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creating grid[%u, %u] for map %u instance %u", p.x_coord, p.y_coord, GetId(), i_InstanceId);

    auto const itr = i_loadedGrids.emplace_hint(lb,
        std::piecewise_construct,
        std::forward_as_tuple(key),
        std::forward_as_tuple(p.x_coord, p.y_coord, i_gridExpiry, CanUnloadMap()));

    auto &ngrid = itr->second;

    ngrid.SetGridState(GRID_STATE_IDLE);
    setNGrid(&ngrid, p.x_coord, p.y_coord);

    //z coord
    int gx = (MAX_NUMBER_OF_GRIDS - 1) - p.x_coord;
    int gy = (MAX_NUMBER_OF_GRIDS - 1) - p.y_coord;

    if (!GridMaps[gx][gy])
        LoadMapAndVMap(gx, gy);
}

//Load NGrid and make it active
void Map::EnsureGridLoadedForActiveObject(const Cell &cell, WorldObject* object)
{
    EnsureGridLoaded(cell);
    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(ngrid != nullptr);

    // refresh grid state & timer
    if (ngrid->GetGridState() != GRID_STATE_ACTIVE)
    {
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Active object " UI64FMTD " triggers loading of grid [%u, %u] on map %u", object->GetGUID().GetCounter(), cell.GridX(), cell.GridY(), GetId());
        ResetGridExpiry(*ngrid, 0.1f);
        ngrid->SetGridState(GRID_STATE_ACTIVE);
    }
}

//Create NGrid and load the object data in it
bool Map::EnsureGridLoaded(const Cell &cell)
{
    EnsureGridCreated(GridCoord(cell.GridX(), cell.GridY()));

    auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(ngrid != nullptr);

    if (ngrid->isGridObjectDataLoaded())
        return false;

    TC_LOG_DEBUG(LOG_FILTER_MAPS, "Loading grid[%u, %u] for map %u instance %u", cell.GridX(), cell.GridY(), GetId(), i_InstanceId);

    ngrid->setGridObjectDataLoaded(true);

    Trinity::ObjectGridLoader::LoadN(*ngrid, this, cell);

    //Hook for garrisones spawn system
    onEnsureGridLoaded(ngrid, cell);

    // Add resurrectable corpses to world object list in grid
    sObjectAccessor->AddCorpsesToGrid(GridCoord(cell.GridX(), cell.GridY()), ngrid->GetGrid(cell.CellX(), cell.CellY()), this);

    Balance();
    return true;
}

void Map::LoadGrid(float x, float y)
{
    EnsureGridLoaded(Cell(x, y));
}

bool Map::AddPlayerToMap(Player* player, bool initPlayer /*= true*/)
{
    CellCoord cellCoord = Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY());
    if (!cellCoord.IsCoordValid())
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "Map::Add: Player (GUID: %u) has invalid coordinates X:%f Y:%f grid cell [%u:%u]", player->GetGUIDLow(), player->GetPositionX(), player->GetPositionY(), cellCoord.x_coord, cellCoord.y_coord);
        return false;
    }

    Cell cell(cellCoord);
    EnsureGridLoadedForActiveObject(cell, player);
    AddToGrid(player, cell);

    // Check if we are adding to correct map
    // ASSERT (player->GetMap() == this);
    if (player->GetMap() != this)
        return false;

    player->SetMap(this);
    if (!player->IsInWorld())
        player->AddToWorld();

    if (!player->IsInWorld())
        return false;

    if (initPlayer)
        SendInitSelf(player);

    SendInitTransports(player);

    if (initPlayer)
        player->ClearClient();

    if (player->IsSpectator() && player->IsSpectateRemoving())
        player->SetSpectate(false);

    player->UpdateObjectVisibility(false);

    sScriptMgr->OnPlayerEnterMap(this, player);
    sOutdoorPvPMgr->HandlePlayerEnterMap(player->GetGUID(), player->GetCurrentZoneID());

    return true;
}

template<class T>
void Map::InitializeObject(T* /*obj*/)
{
}

template<>
void Map::InitializeObject(Creature* obj)
{
    obj->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
}

template<class T>
bool Map::AddToMap(T *obj)
{
    //TODO: Needs clean up. An object should not be added to map twice.
    if (obj->IsInWorld() && obj->IsInGrid())
    {
        ASSERT(obj->IsInGrid());
        obj->UpdateObjectVisibility(true);
        return true;
    }

    CellCoord cellCoord = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    //It will create many problems (including crashes) if an object is not added to grid after creation
    //The correct way to fix it is to make AddToMap return false and delete the object if it is not added to grid
    //But now AddToMap is used in too many places, I will just see how many ASSERT failures it will cause
    ASSERT(cellCoord.IsCoordValid());
    if (!cellCoord.IsCoordValid())
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "Map::Add: Object " UI64FMTD " has invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID().GetCounter(), obj->GetPositionX(), obj->GetPositionY(), cellCoord.x_coord, cellCoord.y_coord);
        return false; //Should delete object
    }

    Cell cell(cellCoord);
    if (obj->isActiveObject())
        EnsureGridLoadedForActiveObject(cell, obj);
    else
        EnsureGridCreated(GridCoord(cell.GridX(), cell.GridY()));
    AddToGrid(obj, cell);
    #ifdef TRINITY_DEBUG
    TC_LOG_DEBUG(LOG_FILTER_MAPS, "Object %u enters grid[%u, %u]", obj->GetGUID(), cell.GridX(), cell.GridY());
    #endif

    //Must already be set before AddToMap. Usually during obj->Create.
    //obj->SetMap(this);
    if (!obj->IsInWorld())
        obj->AddToWorld();

    if (GameObject* go = obj->ToGameObject())
        if (StaticTransport* staticTransport = go->ToStaticTransport())
            AddStaticTransport(staticTransport);

    InitializeObject(obj);

    if (obj->isActiveObject())
        AddToActive(obj);

    //something, such as vehicle, needs to be update immediately
    //also, trigger needs to cast spell, if not update, cannot see visual
    obj->UpdateObjectVisibility(true);

    if (Creature* creature = obj->ToCreature())
        AddBattlePet(creature);

    return true;
}

template<>
bool Map::AddToMap(Transport* obj)
{
    //TODO: Needs clean up. An object should not be added to map twice.
    if (obj->IsInWorld())
        return true;

    CellCoord cellCoord = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    if (!cellCoord.IsCoordValid())
    {
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Map::Add: Object %s has invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID().ToString().c_str(), obj->GetPositionX(), obj->GetPositionY(), cellCoord.x_coord, cellCoord.y_coord);
        return false; //Should delete object
    }

    obj->AddToWorld();
    AddTransport(obj);

    // Broadcast creation to players
    ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->GetTransport() != obj/* && (player->GetDistance(obj) <= MAX_VISIBILITY_DISTANCE || IsBattlegroundOrArena())*/)
        {
            UpdateData data(GetId());
            obj->BuildCreateUpdateBlockForPlayer(&data, player);
            WorldPacket packet;
            if (data.BuildPacket(&packet))
                player->SendDirectMessage(&packet);
        }
    });

    return true;
}

bool Map::IsGridLoaded(const GridCoord &p) const
{
    auto const ngrid = getNGrid(p.x_coord, p.y_coord);
    return ngrid && ngrid->isGridObjectDataLoaded();
}

void Map::Update(const uint32 t_diff)
{
    if (b_isMapUnload) // Need update if start unload???
        return;

    volatile uint32 _mapId = GetId();
    volatile uint32 _instanceId = GetInstanceId();

    uint32 _s = getMSTime();

    m_Functions.Update(t_diff);

    _dynamicTree.update(t_diff);

    /// update active cells around players and active objects
    resetMarkedCells();

    // update worldsessions for existing players
    for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        if (b_isMapUnload)
            return;

        if (Player* player = m_mapRefIter->getSource())
        {
            if (player->IsDelete() || player->IsPreDelete()) // If object in delete list or ported don`t update it
                continue;

            WorldSession* session = player->GetSession();
            m_currentSession = session;
            if (!session || session->PlayerLoading() || session->PlayerLogout()) // Prevent update if player not in map fulling
                continue;

            if (player->IsChangeMap() || player->GetMap() != this || session->GetMap() != this)
                continue;

            try
            {
                // Can be not in world after WorldSession::Update
                if (player->IsInWorld())
                {
                    uint32 _ss = getMSTime();
                    player->Update(t_diff);
                    uint32 _mssu = GetMSTimeDiffToNow(_ss);
                    VisitNearbyCellsOf(player);
                    uint32 _mss = GetMSTimeDiffToNow(_ss);
                    if (_mss > 250)
                        sLog->outDiff("player->Update: type - player, g:%u castCount %u targetCount %u All %ums player %ums mapId %u diff %u i_InstanceId %u activeEntry %u", player->GetGUIDLow(), player->_castCount, player->_targetCount, _mss, _mssu, GetId(), t_diff, i_InstanceId, m_activeEntry);
                }
                if (player->IsRemoveFromMap())
                {
                    RemovePlayerFromMap(player, false);
                    player->SetRemoveFromMap(false);
                    session->SetCanLogout();
                }
            }
            catch (std::exception& e)
            {
                // sLog->outTryCatch("\n\n//==-----------------------------------------------------------------------------------------------------==//");
                sLog->outTryCatch("Exception caught Player in Map::Update %s _mapId %u InstanceId %u", e.what(), _mapId, _instanceId);

                if (m_currentSession)
                    m_currentSession->KickPlayer();
            }
        }
    }

    m_currentSession = nullptr;

    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("Map::Update Player mapId %u Update time - %ums diff %u Players online: %u i_InstanceId %u activeEntry %u", GetId(), _ms, t_diff, m_sessions.size(), i_InstanceId, m_activeEntry);

    if (b_isMapUnload)
        return;

    // non-player active objects, increasing iterator in the loop in case of object removal
    for (auto &obj: m_activeNonPlayers)
    {
        if (obj && obj->IsInWorld())
        {
            if (CanCreatedZone())
            {
                if (obj->GetCurrentZoneID() == i_InstanceId)
                    VisitNearbyCellsOf(obj);
            }
            else
                VisitNearbyCellsOf(obj);
        }
    }

    uint32 collectedCount = 0;
    for (auto const _stepY : {0, 1})
    {
        for (auto const _stepX : {0, 1})
        {
            for (auto& collected : i_objectUpdater[_stepY][_stepX])
            {
                if (collected.second.empty())
                    continue;

                collectedCount += collected.second.size();

                if (threadPool)
                {
                    threadPool->schedule([collected, t_diff, this]() mutable {
                    updateCollected(collected.second, t_diff, GetId(), GetInstanceId());
                    });
                }
                else
                    updateCollected(collected.second, t_diff, GetId(), GetInstanceId());
            }

            if (threadPool)
                threadPool->wait();

            i_objectUpdater[_stepY][_stepX].clear();
        }
    }
    i_objectTest.clear();

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 250)
        sLog->outDiff("Map::Update Collected mapId %u Update time - %ums diff %u Players online: %u i_InstanceId %u activeEntry %u collectedCount %u", GetId(), _ms, t_diff, m_sessions.size(), i_InstanceId, m_activeEntry, collectedCount);

    // sWorldStateMgr.MapUpdate(this);

    ///- Process necessary scripts
    if (!m_scriptSchedule.empty())
    {
        i_scriptLock = true;
        ScriptsProcess();
        i_scriptLock = false;
    }


    if (_weatherUpdateTimer.Passed())
    {
        for (auto&& zoneInfo : _zoneDynamicInfo)
            if (zoneInfo.second.DefaultWeather && !zoneInfo.second.DefaultWeather->Update(_weatherUpdateTimer.GetInterval()))
                zoneInfo.second.DefaultWeather.reset();

        _weatherUpdateTimer.Reset();
    }

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 250)
        sLog->outDiff("Map::Update ScriptsProcess mapId %u Update time - %ums diff %u Players online: %u i_InstanceId %u activeEntry %u activeEncounter %u", GetId(), _ms, t_diff, m_sessions.size(), i_InstanceId, m_activeEntry, m_activeEncounter);

    if (b_isMapUnload)
        return;

    MoveAllCreaturesInMoveList();
    MoveAllGameObjectsInMoveList();
    MoveAllDynamicObjectsInMoveList();
    MoveAllAreaTriggersInMoveList();

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 250)
        sLog->outDiff("Map::Update MoveAll mapId %u Update time - %ums diff %u Players online: %u i_InstanceId %u activeEntry %u activeEncounter %u", GetId(), _ms, t_diff, m_sessions.size(), i_InstanceId, m_activeEntry, m_activeEncounter);

    std::set<ObjectGuid> objectsTemp;

    if (!i_objects.empty())
    {
        std::lock_guard<std::recursive_mutex> _lock(i_objectLock);
        std::swap(objectsTemp, i_objects);
    }

    if (b_isMapUnload)
        return;

    for (auto &guid : objectsTemp)
    {
        if (Object* obj = ObjectAccessor::GetObject(this, guid))
        {
            if (threadPool)
            {
                threadPool->schedule([obj] 
                {
                    UpdateDataMapType update_players;
                    obj->BuildUpdate(update_players);

                    WorldPacket packet;
                    for (auto& updatePlayer : update_players)
                    {
                        if (updatePlayer.second.BuildPacket(&packet))
                            updatePlayer.first->SendDirectMessage(&packet);
                        packet.clear();
                    }}
                );
            }
            else
            {
                UpdateDataMapType update_players;
                obj->BuildUpdate(update_players);

                WorldPacket packet;
                for (auto& updatePlayer : update_players)
                {
                    if (updatePlayer.second.BuildPacket(&packet))
                        updatePlayer.first->SendDirectMessage(&packet);
                    packet.clear();
                }
            }
        }
    }
    objectsTemp.clear();

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 250)
        sLog->outDiff("Map::Update UpdateDataMap mapId %u Update time - %ums diff %u Players online: %u i_InstanceId %u activeEntry %u activeEncounter %u", GetId(), _ms, t_diff, m_sessions.size(), i_InstanceId, m_activeEntry, m_activeEncounter);

    std::set<Object*> objectsAddTemp;
    if (!i_objectsAddToMap.empty())
    {
        std::lock_guard<std::recursive_mutex> _objectsAddToMap_lock(m_objectsAddToMap_lock);
        std::swap(objectsAddTemp, i_objectsAddToMap);
    }
    for (auto& _object : objectsAddTemp)
    {
        if (_object->IsCreature())
            AddToMap(_object->ToCreature());
        if (_object->IsGameObject())
            AddToMap(_object->ToGameObject());
    }
    objectsAddTemp.clear();

    for (auto& scenario : m_scenarios)
        scenario->Update(t_diff);

    if (threadPool)
        threadPool->wait();

    if (m_brawlerGuild)
        m_brawlerGuild->Update(t_diff);

    i_timer_op.Update(t_diff);
    if (i_timer_op.Passed())
    {
        UpdateOutdoorPvP(uint32(i_timer_op.GetCurrent()));
        i_timer_op.SetCurrent(0);
    }

    _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 500) // Only lags
        sLog->outDiff("Map::Update mapId %u Update time - %ums diff %u Players online: %u i_InstanceId %u activeEntry %u activeEncounter %u", GetId(), _ms, t_diff, m_sessions.size(), i_InstanceId, m_activeEntry, m_activeEncounter);
}

void Map::UpdateSessions(uint32 diff)
{
    volatile uint32 _mapId = GetId();
    volatile uint32 _instanceId = GetInstanceId();

    uint32 _s = getMSTime();

    WorldSessionPtr sess = nullptr;
    while (addSessQueue.next(sess))
        AddSession_(sess);

    // update worldsessions for existing players
    for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
    {
        next = itr;
        ++next;

        ///- and remove not active sessions from the list
        WorldSessionPtr pSession = itr->second;
        m_currentSession = &*pSession;
        if (!pSession || pSession->GetMap() != this)
        {
            m_sessions.erase(itr);
            continue;
        }

        if (Player* sPlayer = pSession->GetPlayer())
        {
            if (pSession->GetPlayer()->IsChangeMap())
            {
                pSession->m_Functions.Update(diff);
                continue;
            }

            sPlayer->HandleSpellInQueue();
        }

        if (!pSession->Update(diff, this))    // As interval = 0
        {
            pSession->LogoutPlayer(true);
            pSession->SetMap(nullptr);
            m_sessions.erase(itr);
        }
    }
    m_currentSession = nullptr;

    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 500)
        sLog->outDiff("Map::UpdateSessions mapId %u Update time - %ums diff %u", GetId(), _ms, diff);
}

void Map::UpdateOutdoorPvP(uint32 diff)
{
    if (b_isMapUnload)
        return;

    uint32 _s = getMSTime();

    if (OutdoorPvPList && !OutdoorPvPList->empty())
        for (auto itr : *OutdoorPvPList)
            if (itr && itr->GetMap() == this)
                itr->Update(diff);

    if (BattlefieldList && !BattlefieldList->empty())
        for (auto itr : *BattlefieldList)
            if (itr && itr->GetMap() == this)
                if (itr->IsEnabled())
                    itr->Update(diff);

    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 500)
        sLog->outDiff("Map::UpdateOutdoorPvP mapId %u Update time - %ums diff %u", GetId(), _ms, diff);
}

uint32 Map::GetCurrentDiff() const
{
    return i_timer.GetCurrent();
}

void Map::RemovePlayerFromMap(Player* player, bool remove)
{
    sOutdoorPvPMgr->HandlePlayerLeaveMap(player->GetGUID(), player->GetCurrentZoneID());

    if (InstanceScript* data_s = player->GetInstanceScript())
        data_s->OnPlayerLeaveForScript(player);

    player->RemoveFromWorld();
    if (!remove)
        SendRemoveTransports(player);

    player->UpdateObjectVisibility(true);
    if (player->IsInGrid())
        player->RemoveFromGrid();

    if (remove)
    {
        player->SetPreDelete();
        DeleteFromWorld(player);
        sScriptMgr->OnPlayerLeaveMap(this, player);
    }
}

template<class T>
void Map::RemoveFromMap(T *obj, bool remove)
{
    if (Creature* creature = obj->ToCreature())
        RemoveBattlePet(creature);

    obj->RemoveFromWorld();
    if (obj->isActiveObject())
        RemoveFromActive(obj);

    obj->UpdateObjectVisibility(true);
    if (obj->IsInGrid())
        obj->RemoveFromGrid();

    if (GameObject* go = obj->ToGameObject())
        if (StaticTransport* staticTransport = go->ToStaticTransport())
            RemoveStaticTransport(staticTransport);

    obj->ResetMap();

    if (remove)
    {
        // if option set then object already saved at this moment
        if (!sWorld->getBoolConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime();
        DeleteFromWorld(obj);
    }
}

template<>
void Map::RemoveFromMap(Transport* obj, bool remove)
{
    obj->RemoveFromWorld();

    UpdateData data(GetId());
    obj->BuildOutOfRangeUpdateBlock(&data);
    WorldPacket packet;
    if (data.BuildPacket(&packet))
    {
        ApplyOnEveryPlayer([&](Player* player)
        {
            if (player->GetTransport() != obj)
                player->SendDirectMessage(&packet);
        });
    }

    RemoveTransport(obj);
    obj->ResetMap();

    if (remove)
    {
        // if option set then object already saved at this moment
        if (!sWorld->getBoolConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime();
        DeleteFromWorld(obj);
    }
}

void Map::PlayerRelocation(Player* player, float x, float y, float z, float orientation)
{
    ASSERT(player);

    Cell old_cell(player->GetPositionX(), player->GetPositionY());
    Cell new_cell(x, y);

    player->Relocate(x, y, z, orientation);
    if (player->IsVehicle())
        player->GetVehicleKit()->RelocatePassengers();

    if (old_cell.DiffGrid(new_cell) || old_cell.DiffCell(new_cell))
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Player %s relocation grid[%u, %u]cell[%u, %u]->grid[%u, %u]cell[%u, %u]", player->GetName(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        if (player->IsInGrid())
            player->RemoveFromGrid();

        if (old_cell.DiffGrid(new_cell))
            EnsureGridLoadedForActiveObject(new_cell, player);

        AddToGrid(player, new_cell);
    }

    player->OnRelocated();
}

void Map::CreatureRelocation(Creature* creature, float x, float y, float z, float ang, bool respawnRelocationOnFail)
{
    Cell const old_cell(creature->GetCurrentCell());
    Cell new_cell(x, y);

    if (!respawnRelocationOnFail && !getNGrid(new_cell.GridX(), new_cell.GridY()))
        return;

    // delay creature move for grid/cell to grid/cell moves
    if (old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
        #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creature (GUID: %u Entry: %u) added to moving list from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", creature->GetGUIDLow(), creature->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif
        AddCreatureToMoveList(creature, x, y, z, ang);
        // in diffcell/diffgrid case notifiers called at finishing move creature in Map::MoveAllCreaturesInMoveList
    }
    else
    {
        creature->Relocate(x, y, z, ang);
        if (creature->IsVehicle())
            creature->GetVehicleKit()->RelocatePassengers();
        RemoveCreatureFromMoveList(creature);
        creature->OnRelocated();
    }
}

void Map::GameObjectRelocation(GameObject* go, float x, float y, float z, float orientation, bool respawnRelocationOnFail)
{
    Cell const old_cell(go->GetCurrentCell());
    Cell new_cell(x, y);

    if (!respawnRelocationOnFail && !getNGrid(new_cell.GridX(), new_cell.GridY()))
        return;

    // delay go move for grid/cell to grid/cell moves
    if (old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s Entry: %u) added to moving list from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif
        AddGameObjectToMoveList(go, x, y, z, orientation);
        // in diffcell/diffgrid case notifiers called at finishing move go in Map::MoveAllGameObjectsInMoveList
    }
    else
    {
        go->Relocate(x, y, z, orientation);
        go->UpdateModelPosition(true);
        go->UpdateObjectVisibility(false);
        RemoveGameObjectFromMoveList(go);
    }
}

void Map::DynamicObjectRelocation(DynamicObject* dynObj, float x, float y, float z, float orientation)
{
    Cell const old_cell(dynObj->GetCurrentCell());
    Cell new_cell(x, y);

    if (!getNGrid(new_cell.GridX(), new_cell.GridY()))
        return;

    // delay creature move for grid/cell to grid/cell moves
    if (old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s) added to moving list from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", dynObj->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif
        AddDynamicObjectToMoveList(dynObj, x, y, z, orientation);
        // in diffcell/diffgrid case notifiers called at finishing move dynObj in Map::MoveAllGameObjectsInMoveList
    }
    else
    {
        dynObj->Relocate(x, y, z, orientation);
        dynObj->UpdateObjectVisibility(false);
        RemoveDynamicObjectFromMoveList(dynObj);
    }
}

void Map::AreaTriggerRelocation(AreaTrigger* at, float x, float y, float z, float orientation)
{
    Cell const old_cell(at->GetCurrentCell());
    Cell new_cell(x, y);

    if (!getNGrid(new_cell.GridX(), new_cell.GridY()))
        return;

    // delay areatrigger move for grid/cell to grid/cell moves
    if (old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "AreaTrigger (%s) added to moving list from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", at->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif
        AddAreaTriggerToMoveList(at, x, y, z, orientation);
        // in diffcell/diffgrid case notifiers called at finishing move at in Map::MoveAllAreaTriggersInMoveList
    }
    else
    {
        at->Relocate(x, y, z, orientation);
        // at->UpdateShape();
        at->UpdateObjectVisibility(false);
        RemoveAreaTriggerFromMoveList(at);
    }
}

void Map::AddCreatureToMoveList(Creature* c, float x, float y, float z, float ang)
{
    _creatureToMoveLock.lock();
    if (c->_moveState == MAP_OBJECT_CELL_MOVE_NONE)
        _creaturesToMove.push_back(c);
    c->SetNewCellPosition(x, y, z, ang);
    _creatureToMoveLock.unlock();
}

void Map::RemoveCreatureFromMoveList(Creature* c)
{
    _creatureToMoveLock.lock();
    if (c->_moveState == MAP_OBJECT_CELL_MOVE_ACTIVE)
        c->_moveState = MAP_OBJECT_CELL_MOVE_INACTIVE;
    _creatureToMoveLock.unlock();
}

void Map::AddGameObjectToMoveList(GameObject* go, float x, float y, float z, float ang)
{
    _gameObjectsToMoveLock.lock();
    if (go->_moveState == MAP_OBJECT_CELL_MOVE_NONE)
        _gameObjectsToMove.push_back(go);
    go->SetNewCellPosition(x, y, z, ang);
    _gameObjectsToMoveLock.unlock();
}

void Map::RemoveGameObjectFromMoveList(GameObject* go)
{
    _gameObjectsToMoveLock.lock();
    if (go->_moveState == MAP_OBJECT_CELL_MOVE_ACTIVE)
        go->_moveState = MAP_OBJECT_CELL_MOVE_INACTIVE;
    _gameObjectsToMoveLock.unlock();
}

void Map::AddDynamicObjectToMoveList(DynamicObject* dynObj, float x, float y, float z, float ang)
{
    _dynamicObjectsToMoveLock.lock();
    if (dynObj->_moveState == MAP_OBJECT_CELL_MOVE_NONE)
        _dynamicObjectsToMove.push_back(dynObj);
    dynObj->SetNewCellPosition(x, y, z, ang);
    _dynamicObjectsToMoveLock.unlock();
}

void Map::RemoveDynamicObjectFromMoveList(DynamicObject* dynObj)
{
    _dynamicObjectsToMoveLock.lock();
    if (dynObj->_moveState == MAP_OBJECT_CELL_MOVE_ACTIVE)
        dynObj->_moveState = MAP_OBJECT_CELL_MOVE_INACTIVE;
    _dynamicObjectsToMoveLock.unlock();
}

void Map::AddAreaTriggerToMoveList(AreaTrigger* at, float x, float y, float z, float ang)
{
    _areaTriggersToMoveLock.lock();
    if (at->_moveState == MAP_OBJECT_CELL_MOVE_NONE)
        _areaTriggersToMove.push_back(at);
    at->SetNewCellPosition(x, y, z, ang);
    _areaTriggersToMoveLock.unlock();
}

void Map::RemoveAreaTriggerFromMoveList(AreaTrigger* at)
{
    _areaTriggersToMoveLock.lock();
    if (at->_moveState == MAP_OBJECT_CELL_MOVE_ACTIVE)
        at->_moveState = MAP_OBJECT_CELL_MOVE_INACTIVE;
    _areaTriggersToMoveLock.unlock();
}

void Map::MoveAllCreaturesInMoveList()
{
    std::vector<Creature*> creaturesToMove;
    _creatureToMoveLock.lock();
    std::swap(creaturesToMove, _creaturesToMove);
    _creatureToMoveLock.unlock();

    for (std::vector<Creature*>::iterator itr = creaturesToMove.begin(); itr != creaturesToMove.end(); ++itr)
    {
        Creature* c = *itr;
        if (!c || c->FindMap() != this) //pet is teleported to another map
            continue;

        volatile uint32 creatureEntry = c->GetEntry();

        if (c->_moveState != MAP_OBJECT_CELL_MOVE_ACTIVE)
        {
            c->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
            continue;
        }

        c->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
        if (!c->IsInWorld())
            continue;

        // do move or do move to respawn or remove creature if previous all fail
        if (CreatureCellRelocation(c, Cell(c->_newPosition.m_positionX, c->_newPosition.m_positionY)))
        {
            // update position and visibility for server and client
            c->Relocate(c->_newPosition);
            c->UpdateObjectVisibility(false);
        }
        else
        {
            // if creature can't be move in new cell/grid (not loaded) move it to repawn cell/grid
            // creature coordinates will be updated and notifiers send
            if (!CreatureRespawnRelocation(c, false))
            {
                // ... or unload (if respawn grid also not loaded)
                #ifdef TRINITY_DEBUG
                    TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creature (GUID: %u Entry: %u) cannot be move to unloaded respawn grid.", c->GetGUIDLow(), c->GetEntry());
                #endif
                //AddObjectToRemoveList(Pet*) should only be called in Pet::Remove
                //This may happen when a player just logs in and a pet moves to a nearby unloaded cell
                //To avoid this, we can load nearby cells when player log in
                //But this check is always needed to ensure safety
                //TODO: pets will disappear if this is outside CreatureRespawnRelocation
                //need to check why pet is frequently relocated to an unloaded cell
                if (c->isPet())
                    c->ToPet()->Remove();
                else
                    AddObjectToRemoveList(c);
            }
        }
    }
}

void Map::MoveAllGameObjectsInMoveList()
{
    std::vector<GameObject*> gameObjectsToMove;
    _gameObjectsToMoveLock.lock();
    std::swap(gameObjectsToMove, _gameObjectsToMove);
    _gameObjectsToMoveLock.unlock();

    for (auto go : gameObjectsToMove)
    {
        if (go->FindMap() != this) //transport is teleported to another map
            continue;

        if (go->_moveState != MAP_OBJECT_CELL_MOVE_ACTIVE)
        {
            go->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
            continue;
        }

        go->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
        if (!go->IsInWorld())
            continue;

        // do move or do move to respawn or remove creature if previous all fail
        if (GameObjectCellRelocation(go, Cell(go->_newPosition.m_positionX, go->_newPosition.m_positionY)))
        {
            // update pos
            go->Relocate(go->_newPosition);
            go->UpdateModelPosition();
            go->UpdateObjectVisibility(false);
        }
        else
        {
            // if GameObject can't be move in new cell/grid (not loaded) move it to repawn cell/grid
            // GameObject coordinates will be updated and notifiers send
            if (!GameObjectRespawnRelocation(go, false))
            {
                // ... or unload (if respawn grid also not loaded)
#ifdef TRINITY_DEBUG
                TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s Entry: %u) cannot be move to unloaded respawn grid.", go->GetGUID().ToString().c_str(), go->GetEntry());
#endif
                AddObjectToRemoveList(go);
            }
        }
    }
}

void Map::MoveAllDynamicObjectsInMoveList()
{
    std::vector<DynamicObject*> dynamicObjectsToMove;
    _dynamicObjectsToMoveLock.lock();
    std::swap(dynamicObjectsToMove, _dynamicObjectsToMove);
    _dynamicObjectsToMoveLock.unlock();

    for (auto dynObj : dynamicObjectsToMove)
    {
        if (dynObj->FindMap() != this) //transport is teleported to another map
            continue;

        if (dynObj->_moveState != MAP_OBJECT_CELL_MOVE_ACTIVE)
        {
            dynObj->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
            continue;
        }

        dynObj->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
        if (!dynObj->IsInWorld())
            continue;

        // do move or do move to respawn or remove creature if previous all fail
        if (DynamicObjectCellRelocation(dynObj, Cell(dynObj->_newPosition.m_positionX, dynObj->_newPosition.m_positionY)))
        {
            // update pos
            dynObj->Relocate(dynObj->_newPosition);
            dynObj->UpdateObjectVisibility(false);
        }
        else
        {
#ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "DynamicObject (%s) cannot be moved to unloaded grid.", dynObj->GetGUID().ToString().c_str());
#endif
        }
    }
}

void Map::MoveAllAreaTriggersInMoveList()
{
    std::vector<AreaTrigger*> areaTriggersToMove;
    _areaTriggersToMoveLock.lock();
    std::swap(areaTriggersToMove, _areaTriggersToMove);
    _areaTriggersToMoveLock.unlock();

    for (auto at : areaTriggersToMove)
    {
        if (at->FindMap() != this) //transport is teleported to another map
            continue;

        if (at->_moveState != MAP_OBJECT_CELL_MOVE_ACTIVE)
        {
            at->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
            continue;
        }

        at->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
        if (!at->IsInWorld())
            continue;

        // do move or do move to respawn or remove creature if previous all fail
        if (AreaTriggerCellRelocation(at, Cell(at->_newPosition.m_positionX, at->_newPosition.m_positionY)))
        {
            // update pos
            at->Relocate(at->_newPosition);
            at->UpdateObjectVisibility(false);
        }
        else
        {
#ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "AreaTrigger (%s) cannot be moved to unloaded grid.", at->GetGUID().ToString().c_str());
#endif
        }
    }
}

bool Map::CreatureCellRelocation(Creature* c, Cell new_cell)
{
    Cell const old_cell(c->GetCurrentCell());
    volatile uint32 creatureEntry = c->GetEntry();
    if (!old_cell.DiffGrid(new_cell))                       // in same grid
    {
        // if in same cell then none do
        if (old_cell.DiffCell(new_cell))
        {
            #ifdef TRINITY_DEBUG
                TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creature (GUID: %u Entry: %u) moved in grid[%u, %u] from cell[%u, %u] to cell[%u, %u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
            #endif

            if (c->IsInGrid())
                c->RemoveFromGrid();
            AddToGrid(c, new_cell);
        }
        else
        {
            #ifdef TRINITY_DEBUG
                TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creature (GUID: %u Entry: %u) moved in same grid[%u, %u]cell[%u, %u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
            #endif
        }

        return true;
    }

    // in diff. grids but active creature
    if (c->isActiveObject())
    {
        EnsureGridLoadedForActiveObject(new_cell, c);

        #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "Active creature (GUID: %u Entry: %u) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        if (c->IsInGrid())
            c->RemoveFromGrid();
        AddToGrid(c, new_cell);

        return true;
    }

    // in diff. loaded grid normal creature
    if (IsGridLoaded(GridCoord(new_cell.GridX(), new_cell.GridY())))
    {
        #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creature (GUID: %u Entry: %u) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        if (c->IsInGrid())
            c->RemoveFromGrid();
        EnsureGridCreated(GridCoord(new_cell.GridX(), new_cell.GridY()));
        AddToGrid(c, new_cell);

        return true;
    }

    // fail to move: normal creature attempt move to unloaded grid
    #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creature (GUID: %u Entry: %u) attempted to move from grid[%u, %u]cell[%u, %u] to unloaded grid[%u, %u]cell[%u, %u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
    #endif
    return false;
}

bool Map::GameObjectCellRelocation(GameObject* go, Cell new_cell)
{
    Cell const& old_cell = go->GetCurrentCell();
    if (!old_cell.DiffGrid(new_cell))                       // in same grid
    {
        // if in same cell then none do
        if (old_cell.DiffCell(new_cell))
        {
            #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s Entry: %u) moved in grid[%u, %u] from cell[%u, %u] to cell[%u, %u].", go->GetGUID().ToString().c_str(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
            #endif

            if (go->IsInGrid())
                go->RemoveFromGrid();
            AddToGrid(go, new_cell);
        }
        else
        {
            #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s Entry: %u) moved in same grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
            #endif
        }

        return true;
    }

    // in diff. grids but active GameObject
    if (go->isActiveObject())
    {
        EnsureGridLoadedForActiveObject(new_cell, go);

        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Active GameObject (%s Entry: %u) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        if (go->IsInGrid())
            go->RemoveFromGrid();
        AddToGrid(go, new_cell);

        return true;
    }

    // in diff. loaded grid normal GameObject
    if (IsGridLoaded(GridCoord(new_cell.GridX(), new_cell.GridY())))
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s Entry: %u) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        if (go->IsInGrid())
            go->RemoveFromGrid();
        EnsureGridCreated(GridCoord(new_cell.GridX(), new_cell.GridY()));
        AddToGrid(go, new_cell);

        return true;
    }

    // fail to move: normal GameObject attempt move to unloaded grid
    #ifdef TRINITY_DEBUG
    TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s Entry: %u) attempted to move from grid[%u, %u]cell[%u, %u] to unloaded grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
    #endif
    return false;
}

bool Map::DynamicObjectCellRelocation(DynamicObject* go, Cell new_cell)
{
    Cell const& old_cell = go->GetCurrentCell();
    if (!old_cell.DiffGrid(new_cell))                       // in same grid
    {
        // if in same cell then none do
        if (old_cell.DiffCell(new_cell))
        {
            #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "DynamicObject (%s) moved in grid[%u, %u] from cell[%u, %u] to cell[%u, %u].", go->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
            #endif

            if (go->IsInGrid())
                go->RemoveFromGrid();
            AddToGrid(go, new_cell);
        }
        else
        {
            #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "DynamicObject (%s) moved in same grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
            #endif
        }

        return true;
    }

    // in diff. grids but active GameObject
    if (go->isActiveObject())
    {
        EnsureGridLoadedForActiveObject(new_cell, go);

        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Active DynamicObject (%s) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        if (go->IsInGrid())
            go->RemoveFromGrid();
        AddToGrid(go, new_cell);

        return true;
    }

    // in diff. loaded grid normal GameObject
    if (IsGridLoaded(GridCoord(new_cell.GridX(), new_cell.GridY())))
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "DynamicObject (%s) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        if (go->IsInGrid())
            go->RemoveFromGrid();
        EnsureGridCreated(GridCoord(new_cell.GridX(), new_cell.GridY()));
        AddToGrid(go, new_cell);

        return true;
    }

    // fail to move: normal GameObject attempt move to unloaded grid
    #ifdef TRINITY_DEBUG
    TC_LOG_DEBUG(LOG_FILTER_MAPS, "DynamicObject (%s) attempted to move from grid[%u, %u]cell[%u, %u] to unloaded grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
    #endif
    return false;
}

bool Map::AreaTriggerCellRelocation(AreaTrigger* at, Cell new_cell)
{
    Cell const& old_cell = at->GetCurrentCell();
    if (!old_cell.DiffGrid(new_cell))                       // in same grid
    {
        // if in same cell then none do
        if (old_cell.DiffCell(new_cell))
        {
#ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "AreaTrigger (%s) moved in grid[%u, %u] from cell[%u, %u] to cell[%u, %u].", at->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
#endif

            if (at->IsInGrid())
                at->RemoveFromGrid();
            AddToGrid(at, new_cell);
        }
        else
        {
#ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "AreaTrigger (%s) moved in same grid[%u, %u]cell[%u, %u].", at->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
#endif
        }

        return true;
    }

    // in diff. grids but active AreaTrigger
    if (at->isActiveObject())
    {
        EnsureGridLoadedForActiveObject(new_cell, at);

#ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Active AreaTrigger (%s) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", at->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
#endif

        if (at->IsInGrid())
            at->RemoveFromGrid();
        AddToGrid(at, new_cell);

        return true;
    }

    // in diff. loaded grid normal AreaTrigger
    if (IsGridLoaded(GridCoord(new_cell.GridX(), new_cell.GridY())))
    {
#ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "AreaTrigger (%s) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", at->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
#endif

        if (at->IsInGrid())
            at->RemoveFromGrid();
        EnsureGridCreated(GridCoord(new_cell.GridX(), new_cell.GridY()));
        AddToGrid(at, new_cell);

        return true;
    }

    // fail to move: normal AreaTrigger attempt move to unloaded grid
#ifdef TRINITY_DEBUG
    TC_LOG_DEBUG(LOG_FILTER_MAPS, "AreaTrigger (%s) attempted to move from grid[%u, %u]cell[%u, %u] to unloaded grid[%u, %u]cell[%u, %u].", at->GetGUID().ToString().c_str(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
#endif
    return false;
}

bool Map::CreatureRespawnRelocation(Creature* c, bool diffGridOnly)
{
    float resp_x, resp_y, resp_z, resp_o;
    c->GetRespawnPosition(resp_x, resp_y, resp_z, &resp_o);

    Cell const old_cell(c->GetCurrentCell());
    Cell resp_cell(resp_x, resp_y);

    //creature will be unloaded with grid
    if (diffGridOnly && !old_cell.DiffGrid(resp_cell))
        return true;

    c->CombatStop();
    c->GetMotionMaster()->Clear();

    #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Creature (GUID: %u Entry: %u) moved from grid[%u, %u]cell[%u, %u] to respawn grid[%u, %u]cell[%u, %u].", c->GetGUIDLow(), c->GetEntry(), c->GetCurrentCell().GridX(), c->GetCurrentCell().GridY(), c->GetCurrentCell().CellX(), c->GetCurrentCell().CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());
    #endif

    // teleport it to respawn point (like normal respawn if player see)
    if (CreatureCellRelocation(c, resp_cell))
    {
        c->Relocate(resp_x, resp_y, resp_z, resp_o);
        c->GetMotionMaster()->Initialize();                 // prevent possible problems with default move generators
        //CreatureRelocationNotify(c, resp_cell, resp_cell.GetCellCoord());
        c->UpdateObjectVisibility(false);
        return true;
    }

    return false;
}

bool Map::GameObjectRespawnRelocation(GameObject* go, bool diffGridOnly)
{
    float resp_x, resp_y, resp_z, resp_o;
    go->GetRespawnPosition(resp_x, resp_y, resp_z, &resp_o);
    Cell resp_cell(resp_x, resp_y);

    //GameObject will be unloaded with grid
    if (diffGridOnly && !go->GetCurrentCell().DiffGrid(resp_cell))
        return true;

    #ifdef TRINITY_DEBUG
    TC_LOG_DEBUG(LOG_FILTER_MAPS, "GameObject (%s Entry: %u) moved from grid[%u, %u]cell[%u, %u] to respawn grid[%u, %u]cell[%u, %u].", go->GetGUID().ToString().c_str(), go->GetEntry(), go->GetCurrentCell().GridX(), go->GetCurrentCell().GridY(), go->GetCurrentCell().CellX(), go->GetCurrentCell().CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());
    #endif

    // teleport it to respawn point (like normal respawn if player see)
    if (GameObjectCellRelocation(go, resp_cell))
    {
        go->Relocate(resp_x, resp_y, resp_z, resp_o);
        go->UpdateObjectVisibility(false);
        return true;
    }

    return false;
}

uint32 Map::GetInstanceId() const
{
    return i_InstanceId;
}

uint8 Map::GetSpawnMode() const
{
    return uint8(i_difficulty);
}

void Map::SetSpawnMode(Difficulty difficulty)
{
    i_difficulty = difficulty;
    i_lootDifficulty = difficulty;
}

Difficulty Map::GetDifficultyID() const
{
    return i_difficulty;
}

Difficulty Map::GetLootDifficulty() const
{
    return i_lootDifficulty;
}

void Map::SetLootDifficulty(Difficulty difficulty)
{
    i_lootDifficulty = difficulty;
}

bool Map::IsRegularDifficulty() const
{
    return GetDifficultyID() == DIFFICULTY_NONE;
}

bool Map::UnloadGrid(GridContainerType::iterator itr, bool unloadAll)
{
    auto &ngrid = itr->second;

    auto const x = ngrid.getX();
    auto const y = ngrid.getY();

    {
        if (!unloadAll)
        {
            //pets, possessed creatures (must be active), transport passengers
            if (ngrid.GetWorldObjectCountInNGrid<Creature>())
                return false;

            if (ActiveObjectsNearGrid(ngrid))
                return false;
        }

        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Unloading grid[%u, %u] for map %u", x, y, GetId());
        #endif

        if (!unloadAll)
        {
            // Finish creature moves, remove and delete all creatures with delayed remove before moving to respawn grids
            // Must know real mob position before move
            MoveAllCreaturesInMoveList();
            MoveAllGameObjectsInMoveList();
            MoveAllDynamicObjectsInMoveList();
            MoveAllAreaTriggersInMoveList();

            // move creatures to respawn grids if this is diff.grid or to remove list
            Trinity::ObjectGridEvacuator worker;
            ngrid.VisitAllGrids(Trinity::makeGridVisitor(worker));

            // Finish creature moves, remove and delete all creatures with delayed remove before unload
            MoveAllCreaturesInMoveList();
            MoveAllGameObjectsInMoveList();
            MoveAllDynamicObjectsInMoveList();
            MoveAllAreaTriggersInMoveList();
        }

        {
            Trinity::ObjectGridCleaner worker;
            ngrid.VisitAllGrids(Trinity::makeGridVisitor(worker));
        }

        RemoveAllObjectsInRemoveList();

        {
            Trinity::ObjectGridUnloader worker;
            ngrid.VisitAllGrids(Trinity::makeGridVisitor(worker));
        }

        RemoveAllObjectsInRemoveList();

        if (!i_objectsToRemove.empty())
        {
            Trinity::ObjectGridUnloader worker;
            ngrid.VisitAllGrids(Trinity::makeGridVisitor(worker));
        }

        RemoveAllObjectsInRemoveList();

        ASSERT(i_objectsToRemove.empty());

        {
            // Resurrectable corpses are cached in ObjectAccessor and must be
            // removed from grid whenever grid is unloaded to avoid dangling
            // pointers
            CorpseGridReset worker;
            ngrid.VisitAllGrids(Trinity::makeWorldVisitor(worker));
        }

        i_loadedGrids.erase(itr);
        setNGrid(nullptr, x, y);
    }
    int gx = (MAX_NUMBER_OF_GRIDS - 1) - x;
    int gy = (MAX_NUMBER_OF_GRIDS - 1) - y;

    // delete grid map, but don't delete if it is from parent map (and thus only reference)
    //+++if (GridMaps[gx][gy]) don't check for GridMaps[gx][gy], we might have to unload vmaps
    {
        if (i_InstanceId == 0)
        {
            if (GridMaps[gx][gy])
            {
                GridMaps[gx][gy]->unloadData();
                delete GridMaps[gx][gy];
            }
            // x and y are swapped
            VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(GetId(), gx, gy);
            MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(GetId(), gx, gy);
        }
        else
            static_cast<MapInstanced*>(m_parentMap)->RemoveGridMapReference(GridCoord(gx, gy));

        GridMaps[gx][gy] = nullptr;
    }
    #ifdef TRINITY_DEBUG
    TC_LOG_DEBUG(LOG_FILTER_MAPS, "Unloading grid[%u, %u] for map %u finished", x, y, GetId());
    #endif
    return true;
}

void Map::RemoveAllPlayers()
{
    if (HavePlayers())
    {
        for (auto& itr : m_mapRefManager)
        {
            Player* player = itr.getSource();
            if (!player->IsBeingTeleportedFar())
            {
                // this is happening for bg
                TC_LOG_ERROR(LOG_FILTER_MAPS, "Map::UnloadAll: player %s is still in map %u during unload, this should not happen!", player->GetName(), GetId());
                player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
            }
        }
    }
}

void Map::UnloadAll()
{
    b_isMapUnload = true;
    m_Functions.Update(120000);

    for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
    {
        next = itr;
        ++next;

        if (WorldSessionPtr pSession = itr->second)
        {
            if (pSession->GetPlayer())
                pSession->LogoutPlayer(true);
            pSession->SetMap(nullptr);
        }
    }

    // clear all delayed moves, useless anyway do this moves before map unload.
    _creaturesToMove.clear();
    _gameObjectsToMove.clear();
    _dynamicObjectsToMove.clear();
    _areaTriggersToMove.clear();

    for (auto i = i_loadedGrids.begin(); i != i_loadedGrids.end();)
        UnloadGrid(i++, true);

    if (threadPool)
        threadPool->wait();
}

void Map::ResetGridExpiry(NGrid& grid, float factor) const
{
    grid.ResetTimeTracker(i_gridExpiry * factor);
}

time_t Map::GetGridExpiry() const
{
    return i_gridExpiry;
}

uint32 Map::GetId() const
{
    return i_mapEntry->ID;
}

uint32 Map::GetParentMap() const
{
    return i_mapEntry->ParentMapID == -1 ? GetId() : i_mapEntry->ParentMapID;
}

// *****************************
// Grid function
// *****************************
GridMap::GridMap()
{
    _flags = 0;
    // Area data
    _gridArea = 0;
    _areaMap = nullptr;
    // Height level data
    _gridHeight = INVALID_HEIGHT;
    _gridGetHeight = &GridMap::getHeightFromFlat;
    _gridIntHeightMultiplier = 0;
    m_V9 = nullptr;
    m_V8 = nullptr;
    _minHeightPlanes = nullptr;
    // Liquid data
    _liquidGlobalEntry = 0;
    _liquidGlobalFlags = 0;
    _liquidOffX   = 0;
    _liquidOffY   = 0;
    _liquidWidth  = 0;
    _liquidHeight = 0;
    _liquidLevel = INVALID_HEIGHT;
    _liquidEntry = nullptr;
    _liquidFlags = nullptr;
    _liquidMap  = nullptr;
    _fileExists = false;
}

GridMap::~GridMap()
{
    unloadData();
}

bool GridMap::loadData(const char* filename)
{
    // Unload old data if exist
    unloadData();

    map_fileheader header;
    // Not return error if file not found
    FILE* in = fopen(filename, "rb");
    if (!in)
        return false;

    _fileExists = true;
    if (fread(&header, sizeof(header), 1, in) != 1)
    {
        fclose(in);
        return false;
    }

    if (header.mapMagic == MapMagic.asUInt && header.versionMagic == MapVersionMagic.asUInt)
    {
        // loadup area data
        if (header.areaMapOffset && !loadAreaData(in, header.areaMapOffset, header.areaMapSize))
        {
            TC_LOG_ERROR(LOG_FILTER_MAPS, "Error loading map area data\n");
            fclose(in);
            return false;
        }
        // loadup height data
        if (header.heightMapOffset && !loadHeightData(in, header.heightMapOffset, header.heightMapSize))
        {
            TC_LOG_ERROR(LOG_FILTER_MAPS, "Error loading map height data\n");
            fclose(in);
            return false;
        }
        // loadup liquid data
        if (header.liquidMapOffset && !loadLiquidData(in, header.liquidMapOffset, header.liquidMapSize))
        {
            TC_LOG_ERROR(LOG_FILTER_MAPS, "Error loading map liquids data\n");
            fclose(in);
            return false;
        }
        fclose(in);
        return true;
    }
    TC_LOG_ERROR(LOG_FILTER_MAPS, "Map file '%s' is from an incompatible clientversion. Please recreate using the mapextractor.", filename);
    fclose(in);
    return false;
}

void GridMap::unloadData()
{
    delete[] _areaMap;
    delete[] m_V9;
    delete[] m_V8;
    delete[] _minHeightPlanes;
    delete[] _liquidEntry;
    delete[] _liquidFlags;
    delete[] _liquidMap;
    _areaMap = nullptr;
    m_V9 = nullptr;
    m_V8 = nullptr;
    _minHeightPlanes = nullptr;
    _liquidEntry = nullptr;
    _liquidFlags = nullptr;
    _liquidMap  = nullptr;
    _gridGetHeight = &GridMap::getHeightFromFlat;
    _fileExists = false;
}

bool GridMap::loadAreaData(FILE* in, uint32 offset, uint32 /*size*/)
{
    map_areaHeader header;
    fseek(in, offset, SEEK_SET);

    if (fread(&header, sizeof(header), 1, in) != 1 || header.fourcc != MapAreaMagic.asUInt)
        return false;

    _gridArea = header.gridArea;
    if (!(header.flags & MAP_AREA_NO_AREA))
    {
        _areaMap = new uint16[16 * 16];
        if (fread(_areaMap, sizeof(uint16), 16*16, in) != 16*16)
            return false;
    }
    return true;
}

bool GridMap::loadHeightData(FILE* in, uint32 offset, uint32 /*size*/)
{
    map_heightHeader header;
    fseek(in, offset, SEEK_SET);

    if (fread(&header, sizeof(header), 1, in) != 1 || header.fourcc != MapHeightMagic.asUInt)
        return false;

    _gridHeight = header.gridHeight;
    if (!(header.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        if ((header.flags & MAP_HEIGHT_AS_INT16))
        {
            m_uint16_V9 = new uint16 [129*129];
            m_uint16_V8 = new uint16 [128*128];
            if (fread(m_uint16_V9, sizeof(uint16), 129*129, in) != 129*129 ||
                fread(m_uint16_V8, sizeof(uint16), 128*128, in) != 128*128)
                return false;
            _gridIntHeightMultiplier = (header.gridMaxHeight - header.gridHeight) / 65535;
            _gridGetHeight = &GridMap::getHeightFromUint16;
        }
        else if ((header.flags & MAP_HEIGHT_AS_INT8))
        {
            m_uint8_V9 = new uint8 [129*129];
            m_uint8_V8 = new uint8 [128*128];
            if (fread(m_uint8_V9, sizeof(uint8), 129*129, in) != 129*129 ||
                fread(m_uint8_V8, sizeof(uint8), 128*128, in) != 128*128)
                return false;
            _gridIntHeightMultiplier = (header.gridMaxHeight - header.gridHeight) / 255;
            _gridGetHeight = &GridMap::getHeightFromUint8;
        }
        else
        {
            m_V9 = new float [129*129];
            m_V8 = new float [128*128];
            if (fread(m_V9, sizeof(float), 129*129, in) != 129*129 ||
                fread(m_V8, sizeof(float), 128*128, in) != 128*128)
                return false;
            _gridGetHeight = &GridMap::getHeightFromFloat;
        }
    }
    else
        _gridGetHeight = &GridMap::getHeightFromFlat;

    if (header.flags & MAP_HEIGHT_HAS_FLIGHT_BOUNDS)
    {
        std::array<int16, 9> maxHeights;
        std::array<int16, 9> minHeights;
        if (fread(maxHeights.data(), sizeof(int16), maxHeights.size(), in) != maxHeights.size() ||
            fread(minHeights.data(), sizeof(int16), minHeights.size(), in) != minHeights.size())
            return false;

        static uint32 constexpr indices[8][3] =
        {
            { 3, 0, 4 },
            { 0, 1, 4 },
            { 1, 2, 4 },
            { 2, 5, 4 },
            { 5, 8, 4 },
            { 8, 7, 4 },
            { 7, 6, 4 },
            { 6, 3, 4 }
        };

        static float constexpr boundGridCoords[9][2] =
        {
            { 0.0f, 0.0f },
            { 0.0f, -266.66666f },
            { 0.0f, -533.33331f },
            { -266.66666f, 0.0f },
            { -266.66666f, -266.66666f },
            { -266.66666f, -533.33331f },
            { -533.33331f, 0.0f },
            { -533.33331f, -266.66666f },
            { -533.33331f, -533.33331f }
        };

        _minHeightPlanes = new G3D::Plane[8];
        for (uint32 quarterIndex = 0; quarterIndex < 8; ++quarterIndex)
            _minHeightPlanes[quarterIndex] = G3D::Plane(
                G3D::Vector3(boundGridCoords[indices[quarterIndex][0]][0], boundGridCoords[indices[quarterIndex][0]][1], minHeights[indices[quarterIndex][0]]),
                G3D::Vector3(boundGridCoords[indices[quarterIndex][1]][0], boundGridCoords[indices[quarterIndex][1]][1], minHeights[indices[quarterIndex][1]]),
                G3D::Vector3(boundGridCoords[indices[quarterIndex][2]][0], boundGridCoords[indices[quarterIndex][2]][1], minHeights[indices[quarterIndex][2]])
            );
    }

    return true;
}

bool GridMap::loadLiquidData(FILE* in, uint32 offset, uint32 /*size*/)
{
    map_liquidHeader header;
    fseek(in, offset, SEEK_SET);

    if (fread(&header, sizeof(header), 1, in) != 1 || header.fourcc != MapLiquidMagic.asUInt)
        return false;

    _liquidGlobalEntry = header.liquidType;
    _liquidGlobalFlags = header.liquidFlags;
    _liquidOffX  = header.offsetX;
    _liquidOffY  = header.offsetY;
    _liquidWidth = header.width;
    _liquidHeight = header.height;
    _liquidLevel  = header.liquidLevel;

    if (!(header.flags & MAP_LIQUID_NO_TYPE))
    {
        _liquidEntry = new uint16[16*16];
        if (fread(_liquidEntry, sizeof(uint16), 16*16, in) != 16*16)
            return false;

        _liquidFlags = new uint8[16*16];
        if (fread(_liquidFlags, sizeof(uint8), 16*16, in) != 16*16)
            return false;
    }
    if (!(header.flags & MAP_LIQUID_NO_HEIGHT))
    {
        _liquidMap = new float[uint32(_liquidWidth) * uint32(_liquidHeight)];
        if (fread(_liquidMap, sizeof(float), _liquidWidth*_liquidHeight, in) != (uint32(_liquidWidth) * uint32(_liquidHeight)))
            return false;
    }
    return true;
}

uint16 GridMap::getArea(float x, float y) const
{
    if (!_areaMap)
        return _gridArea;

    x = 16 * (CENTER_GRID_ID - x / SIZE_OF_GRIDS);
    y = 16 * (CENTER_GRID_ID - y / SIZE_OF_GRIDS);
    int lx = static_cast<int>(x) & 15;
    int ly = static_cast<int>(y) & 15;
    return _areaMap[lx * 16 + ly];
}

float GridMap::getHeightFromFlat(float /*x*/, float /*y*/) const
{
    return _gridHeight;
}

float GridMap::getHeightFromFloat(float x, float y) const
{
    if (!m_V8 || !m_V9)
        return _gridHeight;

    x = MAP_RESOLUTION * (CENTER_GRID_ID - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (CENTER_GRID_ID - y/SIZE_OF_GRIDS);

    int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);
    x -= x_int;
    y -= y_int;
    x_int&=(MAP_RESOLUTION - 1);
    y_int&=(MAP_RESOLUTION - 1);

    // Height stored as: h5 - its v8 grid, h1-h4 - its v9 grid
    // +--------------> X
    // | h1-------h2     Coordinates is:
    // | | \  1  / |     h1 0, 0
    // | |  \   /  |     h2 0, 1
    // | | 2  h5 3 |     h3 1, 0
    // | |  /   \  |     h4 1, 1
    // | | /  4  \ |     h5 1/2, 1/2
    // | h3-------h4
    // V Y
    // For find height need
    // 1 - detect triangle
    // 2 - solve linear equation from triangle points
    // Calculate coefficients for solve h = a*x + b*y + c

    float a, b, c;
    // Select triangle:
    if (x+y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = m_V9[(x_int)*129 + y_int];
            float h2 = m_V9[(x_int+1)*129 + y_int];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h2-h1;
            b = h5-h1-h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = m_V9[x_int*129 + y_int  ];
            float h3 = m_V9[x_int*129 + y_int+1];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = m_V9[(x_int+1)*129 + y_int  ];
            float h4 = m_V9[(x_int+1)*129 + y_int+1];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = m_V9[(x_int)*129 + y_int+1];
            float h4 = m_V9[(x_int+1)*129 + y_int+1];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return a * x + b * y + c;
}

float GridMap::getHeightFromUint8(float x, float y) const
{
    if (!m_uint8_V8 || !m_uint8_V9)
        return _gridHeight;

    x = MAP_RESOLUTION * (CENTER_GRID_ID - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (CENTER_GRID_ID - y/SIZE_OF_GRIDS);

    int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);
    x -= x_int;
    y -= y_int;
    x_int&=(MAP_RESOLUTION - 1);
    y_int&=(MAP_RESOLUTION - 1);

    int32 a, b, c;
    uint8 *V9_h1_ptr = &m_uint8_V9[x_int*128 + x_int + y_int];
    if (x+y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32 h1 = V9_h1_ptr[  0];
            int32 h2 = V9_h1_ptr[129];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h2-h1;
            b = h5-h1-h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32 h1 = V9_h1_ptr[0];
            int32 h3 = V9_h1_ptr[1];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32 h2 = V9_h1_ptr[129];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32 h3 = V9_h1_ptr[  1];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return static_cast<float>((a * x) + (b * y) + c)*_gridIntHeightMultiplier + _gridHeight;
}

float GridMap::getHeightFromUint16(float x, float y) const
{
    if (!m_uint16_V8 || !m_uint16_V9)
        return _gridHeight;

    x = MAP_RESOLUTION * (CENTER_GRID_ID - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (CENTER_GRID_ID - y/SIZE_OF_GRIDS);

    int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);
    x -= x_int;
    y -= y_int;
    x_int&=(MAP_RESOLUTION - 1);
    y_int&=(MAP_RESOLUTION - 1);

    int32 a, b, c;
    uint16 *V9_h1_ptr = &m_uint16_V9[x_int*128 + x_int + y_int];
    if (x+y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32 h1 = V9_h1_ptr[  0];
            int32 h2 = V9_h1_ptr[129];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h2-h1;
            b = h5-h1-h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32 h1 = V9_h1_ptr[0];
            int32 h3 = V9_h1_ptr[1];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32 h2 = V9_h1_ptr[129];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32 h3 = V9_h1_ptr[  1];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return static_cast<float>((a * x) + (b * y) + c)*_gridIntHeightMultiplier + _gridHeight;
}

float GridMap::getMinHeight(float x, float y) const
{
    if (!_minHeightPlanes)
        return -500.0f;

    GridCoord gridCoord = Trinity::ComputeGridCoord(x, y);

    int32 doubleGridX = int32(std::floor(-(x - MAP_HALFSIZE) / CENTER_GRID_OFFSET));
    int32 doubleGridY = int32(std::floor(-(y - MAP_HALFSIZE) / CENTER_GRID_OFFSET));

    float gx = x - (int32(gridCoord.x_coord) - CENTER_GRID_ID + 1) * SIZE_OF_GRIDS;
    float gy = y - (int32(gridCoord.y_coord) - CENTER_GRID_ID + 1) * SIZE_OF_GRIDS;

    uint32 quarterIndex = 0;
    if (doubleGridY & 1)
    {
        if (doubleGridX & 1)
            quarterIndex = 4 + (gx <= gy);
        else
            quarterIndex = 2 + ((-SIZE_OF_GRIDS - gx) > gy);
    }
    else if (doubleGridX & 1)
        quarterIndex = 6 + ((-SIZE_OF_GRIDS - gx) <= gy);
    else
        quarterIndex = gx > gy;

    G3D::Ray ray = G3D::Ray::fromOriginAndDirection(G3D::Vector3(gx, gy, 0.0f), G3D::Vector3::unitZ());
    return ray.intersection(_minHeightPlanes[quarterIndex]).z;
}

float GridMap::getLiquidLevel(float x, float y) const
{
    if (!_liquidMap)
        return _liquidLevel;

    x = MAP_RESOLUTION * (CENTER_GRID_ID - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (CENTER_GRID_ID - y/SIZE_OF_GRIDS);

    int cx_int = (static_cast<int>(x) & (MAP_RESOLUTION-1)) - _liquidOffY;
    int cy_int = (static_cast<int>(y) & (MAP_RESOLUTION-1)) - _liquidOffX;

    if (cx_int < 0 || cx_int >=_liquidHeight)
        return INVALID_HEIGHT;
    if (cy_int < 0 || cy_int >=_liquidWidth)
        return INVALID_HEIGHT;

    return _liquidMap[cx_int*_liquidWidth + cy_int];
}

// Why does this return LIQUID data?
uint8 GridMap::getTerrainType(float x, float y) const
{
    if (!_liquidFlags)
        return 0;

    x = 16 * (CENTER_GRID_ID - x/SIZE_OF_GRIDS);
    y = 16 * (CENTER_GRID_ID - y/SIZE_OF_GRIDS);
    int lx = static_cast<int>(x) & 15;
    int ly = static_cast<int>(y) & 15;
    return _liquidFlags[lx*16 + ly];
}

// Get water state on map
inline ZLiquidStatus GridMap::getLiquidStatus(float x, float y, float z, uint8 ReqLiquidType, LiquidData* data)
{
    // Check water type (if no water return)
    if (!_liquidGlobalFlags && !_liquidFlags)
        return LIQUID_MAP_NO_WATER;

    // Get cell
    float cx = MAP_RESOLUTION * (CENTER_GRID_ID - x/SIZE_OF_GRIDS);
    float cy = MAP_RESOLUTION * (CENTER_GRID_ID - y/SIZE_OF_GRIDS);

    int x_int = static_cast<int>(cx) & (MAP_RESOLUTION-1);
    int y_int = static_cast<int>(cy) & (MAP_RESOLUTION-1);

    // Check water type in cell
    int idx=(x_int>>3)*16 + (y_int>>3);
    uint8 type = _liquidFlags ? _liquidFlags[idx] : _liquidGlobalFlags;
    uint32 entry = _liquidEntry ? _liquidEntry[idx] : _liquidGlobalEntry;
    if (LiquidTypeEntry const* liquidEntry = sLiquidTypeStore.LookupEntry(entry))
    {
        type &= MAP_LIQUID_TYPE_DARK_WATER;
        uint32 liqTypeIdx = liquidEntry->SoundBank;
        if (entry < 21)
        {
            if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(getArea(x, y)))
            {
                uint32 overrideLiquid = area->LiquidTypeID[liquidEntry->SoundBank];
                if (!overrideLiquid && area->ParentAreaID)
                {
                    area = sAreaTableStore.LookupEntry(area->ParentAreaID);
                    if (area)
                        overrideLiquid = area->LiquidTypeID[liquidEntry->SoundBank];
                }

                if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(overrideLiquid))
                {
                    entry = overrideLiquid;
                    liqTypeIdx = liq->SoundBank;
                }
            }
        }

        type |= 1 << liqTypeIdx;
    }

    if (type == 0)
        return LIQUID_MAP_NO_WATER;

    // Check req liquid type mask
    if (ReqLiquidType && !(ReqLiquidType&type))
        return LIQUID_MAP_NO_WATER;

    // Check water level:
    // Check water height map
    int lx_int = x_int - _liquidOffY;
    int ly_int = y_int - _liquidOffX;
    if (lx_int < 0 || lx_int >=_liquidHeight)
        return LIQUID_MAP_NO_WATER;
    if (ly_int < 0 || ly_int >=_liquidWidth)
        return LIQUID_MAP_NO_WATER;

    // Get water level
    float liquid_level = _liquidMap ? _liquidMap[lx_int*_liquidWidth + ly_int] : _liquidLevel;
    // Get ground level (sub 0.2 for fix some errors)
    float ground_level = getHeight(x, y);

    // Check water level and ground level
    if (liquid_level < ground_level || z < ground_level - 2)
        return LIQUID_MAP_NO_WATER;

    // All ok in water -> store data
    if (data)
    {
        data->entry = entry;
        data->type_flags  = type;
        data->level = liquid_level;
        data->depth_level = ground_level;
    }

    // For speed check as int values
    float delta = liquid_level - z;

    if (delta > 2.0f)                   // Under water
        return LIQUID_MAP_UNDER_WATER;
    if (delta > 0.0f)                   // In water
        return LIQUID_MAP_IN_WATER;
    if (delta > -0.1f)                   // Walk on water
        return LIQUID_MAP_WATER_WALK;
                                      // Above water
    return LIQUID_MAP_ABOVE_WATER;
}

inline GridMap* Map::GetGrid(float x, float y)
{
    // half opt method
    int gx = static_cast<int>(CENTER_GRID_ID - x / SIZE_OF_GRIDS);                       //grid x
    int gy = static_cast<int>(CENTER_GRID_ID - y / SIZE_OF_GRIDS);                       //grid y

    //Crash fix
    if (gx >= MAX_NUMBER_OF_GRIDS || gy >= MAX_NUMBER_OF_GRIDS || gx < 0 || gy < 0)
        return nullptr;

    // ensure GridMap is loaded
    EnsureGridCreated(GridCoord((MAX_NUMBER_OF_GRIDS - 1) - gx, (MAX_NUMBER_OF_GRIDS - 1) - gy));

    return GridMaps[gx][gy];
}

float Map::GetWaterOrGroundLevel(std::set<uint32> const& phases, float x, float y, float z, float* ground /*= NULL*/, bool /*swim = false*/) const
{
    if (const_cast<Map*>(this)->GetGrid(x, y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = GetHeight(phases, x, y, z, true, 50.0f);
        if (ground)
            *ground = ground_z;

        LiquidData liquid_status;

        ZLiquidStatus res = getLiquidStatus(x, y, ground_z, MAP_ALL_LIQUIDS, &liquid_status);
        switch (res)
        {
            case LIQUID_MAP_ABOVE_WATER:
                return std::max<float>(liquid_status.level, ground_z);
            case LIQUID_MAP_NO_WATER:
                return ground_z;
            default:
                return liquid_status.level;
        }
    }

    return VMAP_INVALID_HEIGHT_VALUE;
}

float Map::GetGridMapHeigh(float x, float y) const
{
    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
        return gmap->getHeight(x, y);

    return VMAP_INVALID_HEIGHT_VALUE;
}

float Map::GetHeight(float x, float y, float z, bool checkVMap /*= true*/, float maxSearchDist /*= DEFAULT_HEIGHT_SEARCH*/) const
{
    // find raw .map surface under Z coordinates
    float mapHeight = VMAP_INVALID_HEIGHT_VALUE;
    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
    {
        float gridHeight = gmap->getHeight(x, y);
        // look from a bit higher pos to find the floor, ignore under surface case
        if (z + 2.0f > gridHeight)
            mapHeight = gridHeight;
    }

    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;
    if (checkVMap)
    {
        VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
        if (vmgr->isHeightCalcEnabled())
            vmapHeight = vmgr->getHeight(GetId(), x, y, z + 2.0f, maxSearchDist);   // look from a bit higher pos to find the floor
    }

    // mapHeight set for any above raw ground Z or <= INVALID_HEIGHT
    // vmapheight set for any under Z value or <= INVALID_HEIGHT
    if (vmapHeight > INVALID_HEIGHT)
    {
        if (mapHeight > INVALID_HEIGHT)
        {
            // we have mapheight and vmapheight and must select more appropriate

            // we are already under the surface or vmap height above map heigt
            // or if the distance of the vmap height is less the land height distance
            if (vmapHeight > mapHeight || std::fabs(mapHeight - z) > std::fabs(vmapHeight - z))
                return vmapHeight;
            return mapHeight;
            // better use .map surface height
        }
        return vmapHeight;
        // we have only vmapHeight (if have)
    }

    return mapHeight;                               // explicitly use map data
}

float Map::GetMinHeight(Position pos) const
 {
     if (GridMap const* grid = const_cast<Map*>(this)->GetGrid(pos.GetPositionX(), pos.GetPositionY()))
         return grid->getMinHeight(pos.GetPositionX(), pos.GetPositionY());
 
     return -500.0f;
 }

inline bool IsOutdoorWMO(uint32 mogpFlags, int32 /*adtId*/, int32 /*rootId*/, int32 /*groupId*/, WMOAreaTableEntry const* wmoEntry, AreaTableEntry const* atEntry)
{
    bool outdoor = true;

    if (wmoEntry && atEntry)
    {
        if (atEntry->Flags[0] & AREA_FLAG_OUTSIDE)
            return true;
        if (atEntry->Flags[0] & AREA_FLAG_INSIDE)
            return false;
    }

    outdoor = (mogpFlags & 0x8) != 0;

    if (wmoEntry)
    {
        if (wmoEntry->Flags & 4)
            return true;
        if (wmoEntry->Flags & 2)
            outdoor = false;
    }
    return outdoor;
}

bool Map::IsOutdoors(float x, float y, float z) const
{
    uint32 mogpFlags;
    int32 adtId, rootId, groupId;

    // no wmo found? -> outside by default
    if (!GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
        return true;

    AreaTableEntry const* atEntry = nullptr;
    WMOAreaTableEntry const* wmoEntry = sDB2Manager.GetWMOAreaTableEntryByTripple(rootId, adtId, groupId);
    if (wmoEntry)
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Got WMOAreaTableEntry! flag %u, AreaTableID %u", wmoEntry->Flags, wmoEntry->AreaTableID);
        #endif
        atEntry = sAreaTableStore.LookupEntry(wmoEntry->AreaTableID);
    }
    return IsOutdoorWMO(mogpFlags, adtId, rootId, groupId, wmoEntry, atEntry);
}

bool Map::GetAreaInfo(float x, float y, float z, uint32 &flags, int32 &adtId, int32 &rootId, int32 &groupId) const
{
    float vmap_z = z;
    float dynamic_z = z;
    float check_z = z;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    uint32 vflags;
    int32 vadtId;
    int32 vrootId;
    int32 vgroupId;
    uint32 dflags;
    int32 dadtId;
    int32 drootId;
    int32 dgroupId;

    bool hasVmapAreaInfo = vmgr->getAreaInfo(GetId(), x, y, vmap_z, vflags, vadtId, vrootId, vgroupId);
    bool hasDynamicAreaInfo = _dynamicTree.getAreaInfo(x, y, dynamic_z, std::set<uint32>(), false, dflags, dadtId, drootId, dgroupId);
    auto useVmap = [&]() { check_z = vmap_z; flags = vflags; adtId = vadtId; rootId = vrootId; groupId = vgroupId; };
    auto useDyn = [&]() { check_z = dynamic_z; flags = dflags; adtId = dadtId; rootId = drootId; groupId = dgroupId; };

    if (hasVmapAreaInfo)
    {
        if (hasDynamicAreaInfo && dynamic_z > vmap_z)
            useDyn();
        else
            useVmap();
    }
    else if (hasDynamicAreaInfo)
    {
        useDyn();
    }

    if (hasVmapAreaInfo || hasDynamicAreaInfo)
    {
        // check if there's terrain between player height and object height
        if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
        {
            float mapHeight = gmap->getHeight(x, y);
            // z + 2.0f condition taken from GetHeight(), not sure if it's such a great choice...
            if (z + 2.0f > mapHeight &&  mapHeight > check_z)
                return false;
        }
        return true;
    }
    return false;
}

float Map::GetVmapHeight(float x, float y, float z) const
{
    float mapHeight = GetHeight(x, y, z, false);
    if (fabs(mapHeight - z) < 0.1)
        return mapHeight;

    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    if (!vmgr->isLineOfSightCalcEnabled())
        return mapHeight;

    float vmapHeight = vmgr->getHeight(GetId(), x, y, z + 2.0f, z + 2.0f - mapHeight);
    if (vmapHeight > VMAP_INVALID_HEIGHT_VALUE)
        return vmapHeight;

    return mapHeight;
}

uint32 Map::GetAreaId(float x, float y, float z, bool *isOutdoors) const
{
    uint32 mogpFlags;
    int32 adtId, rootId, groupId;
    WMOAreaTableEntry const* wmoEntry = nullptr;
    AreaTableEntry const* atEntry = nullptr;
    bool haveAreaInfo = false;

    GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y);  //Load vmap and grid before get areaInfo.

    if (GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
    {
        haveAreaInfo = true;
        wmoEntry = sDB2Manager.GetWMOAreaTableEntryByTripple(rootId, adtId, groupId);
        if (wmoEntry)
            atEntry = sAreaTableStore.LookupEntry(wmoEntry->AreaTableID);
    }

    uint32 areaId = 0;

    if (atEntry)
        areaId = atEntry->ID;
    else
    {
        if (gmap)
            areaId = gmap->getArea(x, y);

        if (!areaId)
            areaId = i_mapEntry->AreaTableID;
    }

    if (isOutdoors)
    {
        if (haveAreaInfo)
            *isOutdoors = IsOutdoorWMO(mogpFlags, adtId, rootId, groupId, wmoEntry, atEntry);
        else
            *isOutdoors = true;
    }
    return areaId;
}

uint32 Map::GetAreaId(float x, float y, float z) const
 {
     return GetAreaId(x, y, z, nullptr);
 }
 
 uint32 Map::GetZoneId(float x, float y, float z) const
 {
     uint32 areaId = GetAreaId(x, y, z);
     if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId))
         if (area->ParentAreaID)
             return area->ParentAreaID;
 
     return areaId;
 }
 
 void Map::GetZoneAndAreaId(uint32& zoneid, uint32& areaid, float x, float y, float z) const
 {
     areaid = zoneid = GetAreaId(x, y, z);
     if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaid))
         if (area->ParentAreaID)
             zoneid = area->ParentAreaID;
 }

uint8 Map::GetTerrainType(float x, float y) const
{
    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
        return gmap->getTerrainType(x, y);
    return 0;
}

ZLiquidStatus Map::getLiquidStatus(float x, float y, float z, uint8 ReqLiquidType, LiquidData* data) const
{
    ZLiquidStatus result = LIQUID_MAP_NO_WATER;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    float liquid_level = INVALID_HEIGHT;
    float ground_level = INVALID_HEIGHT;
    uint32 liquid_type = 0;
    if (vmgr->GetLiquidLevel(GetId(), x, y, z, ReqLiquidType, liquid_level, ground_level, liquid_type))
    {
        #ifdef TRINITY_DEBUG
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "getLiquidStatus(): vmap liquid level: %f ground: %f type: %u", liquid_level, ground_level, liquid_type);
        #endif
        // Check water level and ground level
        if (liquid_level > ground_level && z > ground_level - 2)
        {
            // All ok in water -> store data
            if (data)
            {
                // hardcoded in client like this
                if (GetId() == 530 && liquid_type == 2)
                    liquid_type = 15;

                uint32 liquidFlagType = 0;
                if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(liquid_type))
                    liquidFlagType = liq->SoundBank;

                if (liquid_type && liquid_type < 21)
                {
                    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(GetAreaId(x, y, z)))
                    {
                        uint32 overrideLiquid = area->LiquidTypeID[liquidFlagType];
                        if (!overrideLiquid && area->ParentAreaID)
                        {
                            area = sAreaTableStore.LookupEntry(area->ParentAreaID);
                            if (area)
                                overrideLiquid = area->LiquidTypeID[liquidFlagType];
                        }

                        if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(overrideLiquid))
                        {
                            liquid_type = overrideLiquid;
                            liquidFlagType = liq->SoundBank;
                        }
                    }
                }

                data->level = liquid_level;
                data->depth_level = ground_level;

                data->entry = liquid_type;
                data->type_flags = 1 << liquidFlagType;
            }

            float delta = liquid_level - z;

            // Get position delta
            if (delta > 2.0f)                   // Under water
                return LIQUID_MAP_UNDER_WATER;
            if (delta > 0.0f)                   // In water
                return LIQUID_MAP_IN_WATER;
            if (delta > -0.1f)                   // Walk on water
                return LIQUID_MAP_WATER_WALK;
            result = LIQUID_MAP_ABOVE_WATER;
        }
    }

    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
    {
        LiquidData map_data;
        ZLiquidStatus map_result = gmap->getLiquidStatus(x, y, z, ReqLiquidType, &map_data);
        // Not override LIQUID_MAP_ABOVE_WATER with LIQUID_MAP_NO_WATER:
        if (map_result != LIQUID_MAP_NO_WATER && (map_data.level > ground_level))
        {
            if (data)
            {
                // hardcoded in client like this
                if (GetId() == 530 && map_data.entry == 2)
                    map_data.entry = 15;

                *data = map_data;
            }
            return map_result;
        }
    }
    return result;
}

float Map::GetWaterLevel(float x, float y) const
{
    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
        return gmap->getLiquidLevel(x, y);
    return 0;
}

bool Map::isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, std::set<uint32> const& phases, DynamicTreeCallback* dCallback /*= nullptr*/) const
{
    return VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(GetId(), x1, y1, z1, x2, y2, z2)
        && _dynamicTree.isInLineOfSight({ x1, y1, z1 }, { x2, y2, z2 }, phases, dCallback);
}

bool Map::getObjectHitPos(std::set<uint32> const& phases, bool otherIsPlayer, Position startPos, Position destPos, float modifyDist, DynamicTreeCallback* dCallback /*= nullptr*/)
{
    G3D::Vector3 resultPos;
    G3D::Vector3 _startPos = G3D::Vector3(startPos.m_positionX, startPos.m_positionY, startPos.m_positionZ);
    G3D::Vector3 _dstPos = G3D::Vector3(destPos.m_positionX, destPos.m_positionY, destPos.m_positionZ);
    return _dynamicTree.getObjectHitPos(phases, otherIsPlayer, _startPos, _dstPos, resultPos, modifyDist, dCallback);
}

bool Map::getObjectHitPos(std::set<uint32> const& phases, bool otherIsPlayer, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist, DynamicTreeCallback* dCallback /*= nullptr*/)
{
    G3D::Vector3 startPos = G3D::Vector3(x1, y1, z1);
    G3D::Vector3 dstPos = G3D::Vector3(x2, y2, z2);

    G3D::Vector3 resultPos;
    bool result = _dynamicTree.getObjectHitPos(phases, otherIsPlayer, startPos, dstPos, resultPos, modifyDist, dCallback);
    rx = resultPos.x;
    ry = resultPos.y;
    rz = resultPos.z;
    return result;
}

float Map::GetHeight(std::set<uint32> const& phases, float x, float y, float z, bool vmap /*= true*/, float maxSearchDist /*= DEFAULT_HEIGHT_SEARCH*/, DynamicTreeCallback* dCallback /*= nullptr*/) const
{
    if (!this)
        return VMAP_INVALID_HEIGHT_VALUE;
    float vmapZ = GetHeight(x, y, z, vmap, maxSearchDist);
    float goZ = _dynamicTree.getHeight(x, y, z, maxSearchDist, phases, dCallback);
    if (vmapZ > goZ && dCallback)
        dCallback->go = nullptr;

    return std::max<float>(vmapZ, goZ);
}

bool Map::IsInWater(float x, float y, float pZ, LiquidData* data) const
{
    // Check surface in x, y point for liquid
    if (const_cast<Map*>(this)->GetGrid(x, y))
    {
        LiquidData liquid_status;
        LiquidData* liquid_ptr = data ? data : &liquid_status;
        if (getLiquidStatus(x, y, pZ, MAP_ALL_LIQUIDS, liquid_ptr))
            return true;
    }
    return false;
}

bool Map::IsUnderWater(G3D::Vector3 pos) const
{
    if (const_cast<Map*>(this)->GetGrid(pos.x, pos.y))
        if (getLiquidStatus(pos.x, pos.y, pos.z, MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN) & LIQUID_MAP_UNDER_WATER)
            return true;

    return false;
}

char const* Map::GetMapName() const
{
    return i_mapEntry ? i_mapEntry->MapName->Str[sObjectMgr->GetDBCLocaleIndex()] : "UNNAMEDMAP\x0";
}

void Map::UpdateObjectVisibility(WorldObject* obj, Cell cell, CellCoord cellpair)
{
    cell.SetNoCreate();
    Trinity::VisibleChangesNotifier notifier(*obj);
    cell.Visit(cellpair, Trinity::makeWorldVisitor(notifier), *this, *obj, obj->GetVisibilityRange());
}

void Map::UpdateObjectsVisibilityFor(Player* player, Cell cell, CellCoord cellpair)
{
    Trinity::VisibleNotifier notifier(*player);

    cell.SetNoCreate();
    cell.Visit(cellpair, Trinity::makeWorldVisitor(notifier), *this, *player, player->GetSightRange());
    cell.Visit(cellpair, Trinity::makeGridVisitor(notifier), *this, *player, player->GetSightRange());

    // send data
    notifier.SendToSelf();
}

void Map::resetMarkedCells()
{
    marked_cells.reset();
}

bool Map::isCellMarked(uint32 pCellId)
{
    return marked_cells.test(pCellId);
}

void Map::markCell(uint32 pCellId)
{
    marked_cells.set(pCellId);
}

bool Map::HavePlayers() const
{
    return !m_mapRefManager.isEmpty();
}

void Map::SendInitSelf(Player* player)
{
    #ifdef TRINITY_DEBUG
    TC_LOG_INFO(LOG_FILTER_MAPS, "Creating player data for himself %u", player->GetGUIDLow());
    #endif

    UpdateData data(player->GetMapId());

    // attach to player data current transport data
    if (Transport* transport = player->GetTransport())
        transport->BuildCreateUpdateBlockForPlayer(&data, player);

    // build data for self presence in world at own client (one time for map)
    player->BuildCreateUpdateBlockForPlayer(&data, player);

    // build other passengers at transport also (they always visible and marked as visible and will not send at visibility update at add to map
    if (Transport* transport = player->GetTransport())
        for (WorldObjectSet::iterator itr = transport->GetPassengers().begin(); itr != transport->GetPassengers().end(); ++itr)
            if (player != (*itr) && player->HaveAtClient(*itr))
                (*itr)->BuildCreateUpdateBlockForPlayer(&data, player);

    WorldPacket packet;
    if (data.BuildPacket(&packet))
        player->GetSession()->SendPacket(&packet);
}

void Map::SendInitTransports(Player* player)
{
    if (m_Transports.empty())
        return;

    UpdateData transData(player->GetMapId());
    for (TransportHashSet::iterator i = m_Transports.begin(); i != m_Transports.end(); ++i)
        // send data for current transport in other place
        if ((*i) != player->GetTransport() && (*i)->GetMapId() == GetId() && player->InSamePhaseId(*i)/* && (player->GetDistance(*i) <= MAX_VISIBILITY_DISTANCE || IsBattlegroundOrArena())*/)
            (*i)->BuildCreateUpdateBlockForPlayer(&transData, player);

    WorldPacket packet;
    if (transData.BuildPacket(&packet))
        player->GetSession()->SendPacket(&packet);
}

void Map::SendRemoveTransports(Player* player)
{
    if (m_Transports.empty())
        return;

    UpdateData transData(player->GetMapId());
    // except used transport
    for (TransportHashSet::iterator i = m_Transports.begin(); i != m_Transports.end(); ++i)
        if ((*i) != player->GetTransport() || (*i)->GetMapId() != GetId())
            (*i)->BuildOutOfRangeUpdateBlock(&transData);

    WorldPacket packet;
    if (transData.BuildPacket(&packet))
        player->GetSession()->SendPacket(&packet);
}

void Map::SendUpdateTransportVisibility(Player* player, std::set<uint32> const& /*previousPhases*/)
{
    // Hack to send out transports
    UpdateData transData(player->GetMapId());
    for (TransportHashSet::iterator i = m_Transports.begin(); i != m_Transports.end(); ++i)
    {
        if (*i == player->GetTransport())
            continue;

        if (player->InSamePhaseId(*i) && !player->HaveAtClient(*i))
            (*i)->BuildCreateUpdateBlockForPlayer(&transData, player);
        else if (!player->InSamePhaseId(*i) && player->HaveAtClient(*i))
            (*i)->BuildOutOfRangeUpdateBlock(&transData);
    }

    WorldPacket packet;
    if (transData.BuildPacket(&packet))
        player->GetSession()->SendPacket(&packet);
}

void Map::LoadAllGrids(float minX, float p_MaxX, float minY, float maxY, Player* player)
{
    float curX = minX;
    float curY = minY;

    do
    {
        do
        {
            EnsureGridLoadedForActiveObject(Cell(CellCoord(Trinity::ComputeCellCoord(curX, curY))), player);
            curY += 5.0f;
        }
        while (curY < maxY);

        curY = minY;
        curX += 5.0f;
    }
    while (curX < p_MaxX);
}

void Map::SendZoneDynamicInfo(uint32 zoneId, Player* player) const
{
    auto itr = _zoneDynamicInfo.find(zoneId);
    if (itr == _zoneDynamicInfo.end())
        return;

    if (auto music = itr->second.MusicID)
        player->SendDirectMessage(WorldPackets::Misc::PlayMusic(music).Write());

    SendZoneWeather(itr->second, player);

    if (auto overrideLightID = itr->second.OverrideLightID)
    {
        WorldPackets::Misc::OverrideLight overrideLight;
        overrideLight.AreaLightID = _defaultLight;
        overrideLight.OverrideLightID = overrideLightID;
        overrideLight.TransitionMilliseconds = itr->second.LightFadeInTime;
        player->SendDirectMessage(overrideLight.Write());
    }
}

void Map::SendZoneWeather(uint32 zoneId, Player* player) const
{
    if (!player->HasAuraType(SPELL_AURA_FORCE_WEATHER))
    {
        auto itr = _zoneDynamicInfo.find(zoneId);
        if (itr == _zoneDynamicInfo.end())
            return;

        SendZoneWeather(itr->second, player);
    }
}

void Map::SendZoneWeather(ZoneDynamicInfo const& zoneDynamicInfo, Player* player) const
{
    if (auto weatherId = zoneDynamicInfo.WeatherID)
        player->SendDirectMessage(WorldPackets::Misc::Weather(weatherId, zoneDynamicInfo.WeatherGrade).Write());
    else if (zoneDynamicInfo.DefaultWeather)
        zoneDynamicInfo.DefaultWeather->SendWeatherUpdateToPlayer(player);
    else
        Weather::SendFineWeatherUpdateToPlayer(player);
}

void Map::SetZoneMusic(uint32 zoneID, uint32 musicID)
{
    _zoneDynamicInfo[zoneID].MusicID = musicID;

    WorldPackets::Misc::PlayMusic playMusic(musicID);
    playMusic.Write();

    ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->GetCurrentZoneID() == zoneID && !player->HasAuraType(SPELL_AURA_FORCE_WEATHER))
            player->SendDirectMessage(playMusic.GetRawPacket());
    });
}

Weather* Map::GetOrGenerateZoneDefaultWeather(uint32 zoneId)
{
    auto weatherData = WeatherMgr::GetWeatherData(zoneId);
    if (!weatherData)
        return nullptr;

    auto& info = _zoneDynamicInfo[zoneId];
    if (!info.DefaultWeather)
    {
        info.DefaultWeather = Trinity::make_unique<Weather>(zoneId, weatherData);
        info.DefaultWeather->ReGenerate();
        info.DefaultWeather->UpdateWeather();
    }

    return info.DefaultWeather.get();
}

void Map::SetZoneWeather(uint32 zoneID, WeatherState weatherID, float weatherGrade)
{
    auto& info = _zoneDynamicInfo[zoneID];
    info.WeatherID = weatherID;
    info.WeatherGrade = weatherGrade;

    WorldPackets::Misc::Weather weather(weatherID, weatherGrade);
    weather.Write();

    ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->GetCurrentZoneID() == zoneID)
            player->SendDirectMessage(weather.GetRawPacket());
    });
}

void Map::SetZoneOverrideLight(uint32 zoneID, uint32 lightID, uint32 fadeInTime)
{
    auto& info = _zoneDynamicInfo[zoneID];
    info.OverrideLightID = lightID;
    info.LightFadeInTime = fadeInTime;

    WorldPackets::Misc::OverrideLight overrideLight;
    overrideLight.AreaLightID = _defaultLight;
    overrideLight.OverrideLightID = lightID;
    overrideLight.TransitionMilliseconds = fadeInTime;
    overrideLight.Write();

    ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->GetCurrentZoneID() == zoneID)
            player->SendDirectMessage(overrideLight.GetRawPacket());
    });
}

inline void Map::setNGrid(NGrid *grid, uint32 x, uint32 y)
{
    if (x >= MAX_NUMBER_OF_GRIDS || y >= MAX_NUMBER_OF_GRIDS)
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "map::setNGrid() Invalid grid coordinates found: %d, %d!", x, y);
        //ASSERT(false);
        return;
    }
    i_grids[x][y] = grid;
}

void Map::DelayedUpdate(const uint32 t_diff)
{
    volatile uint32 _mapId = GetId();
    volatile uint32 _instanceId = GetInstanceId();

    RemoveAllObjectsInRemoveList();

    // Don't unload grids if it's battleground, since we may have manually added GOs, creatures, those doesn't load from DB at grid re-load !
    // This isn't really bother us, since as soon as we have instanced BG-s, the whole map unloads as the BG gets ended
    if (!IsBattlegroundOrArena())
    {
        for (auto i = i_loadedGrids.begin(); i != i_loadedGrids.end();)
        {
            auto const state = i->second.GetGridState();
            si_GridStates[state](*this, i++, t_diff);
        }
    }
}

void Map::AddObjectToRemoveList(WorldObject* obj)
{
    if (obj->IsPreDelete() || obj->IsDelete()) // Don`t delete twice
        return;

    ASSERT(obj->GetMapId() == GetId() && obj->GetInstanceId() == GetInstanceId());

    obj->CleanupsBeforeDelete(false);                            // remove or simplify at least cross referenced links

    obj->SetPreDelete();
    std::lock_guard<std::recursive_mutex> guard(i_objectsToRemove_lock);
    i_objectsToRemove.emplace(obj);
    //TC_LOG_DEBUG(LOG_FILTER_MAPS, "Object (GUID: %u TypeId: %u) added to removing list.", obj->GetGUIDLow(), obj->GetTypeId());
}

void Map::AddObjectToSwitchList(WorldObject* obj, bool on)
{
    ASSERT(obj->GetMapId() == GetId() && obj->GetInstanceId() == GetInstanceId());
    // i_objectsToSwitch is iterated only in Map::RemoveAllObjectsInRemoveList() and it uses
    // the contained objects only if IsCreature() , so we can return in all other cases
    if (!obj->IsCreature() && !obj->IsGameObject())
        return;

    std::map<WorldObject*, bool>::iterator itr = i_objectsToSwitch.find(obj);
    if (itr == i_objectsToSwitch.end())
        i_objectsToSwitch.insert(itr, std::make_pair(obj, on));
    else if (itr->second != on)
        i_objectsToSwitch.erase(itr);
    else
        ASSERT(false);
}

void Map::RemoveAllObjectsInRemoveList()
{
    while (!i_objectsToSwitch.empty())
    {
        std::map<WorldObject*, bool>::iterator itr = i_objectsToSwitch.begin();
        WorldObject* obj = itr->first;
        bool on = itr->second;
        i_objectsToSwitch.erase(itr);

        if (!obj->IsPermanentWorldObject())
        {
            switch (obj->GetTypeId())
            {
                case TYPEID_UNIT:
                    SwitchGridContainers(obj->ToCreature(), on);
                    break;
                case TYPEID_GAMEOBJECT:
                    SwitchGridContainers(obj->ToGameObject(), on);
                    break;
                default:
                    break;
            }
        }
    }

    std::set<WorldObject*> _objectsToRemove;
    {
        std::lock_guard<std::recursive_mutex> guard(i_objectsToRemove_lock);
        std::swap(_objectsToRemove, i_objectsToRemove);
    }
    for (std::set<WorldObject*>::iterator itr = _objectsToRemove.begin(); itr != _objectsToRemove.end(); ++itr)
    {
        WorldObject* obj = *itr;
        if (!obj || obj->IsDelete())
            continue;

        volatile uint32 entryorguid = obj->IsPlayer() ? obj->GetGUIDLow() : obj->GetEntry();
        volatile uint32 mapId = GetId();
        //volatile uint32 instanceId = GetInstanceId();

        switch (obj->GetTypeId())
        {
            case TYPEID_CORPSE:
            {
                Corpse* corpse = ObjectAccessor::GetCorpse(*obj, obj->GetGUID());
                if (!corpse)
                    TC_LOG_ERROR(LOG_FILTER_MAPS, "Tried to delete corpse/bones %u that is not in map.", obj->GetGUIDLow());
                else
                    RemoveFromMap(corpse, true);
                break;
            }
        case TYPEID_DYNAMICOBJECT:
            RemoveFromMap(static_cast<DynamicObject*>(obj), true);
            break;
        case TYPEID_AREATRIGGER:
            RemoveFromMap(static_cast<AreaTrigger*>(obj), true);
            break;
        case TYPEID_GAMEOBJECT:
            {
                GameObject* go = obj->ToGameObject();
                if (Transport* transport = go->ToTransport())
                    RemoveFromMap(transport, true);
                else
                    RemoveFromMap(go, true);
                break;
            }
        case TYPEID_CONVERSATION:
            RemoveFromMap(static_cast<Conversation*>(obj), true);
            break;
        case TYPEID_EVENTOBJECT:
            RemoveFromMap(static_cast<EventObject*>(obj), true);
            break;
        case TYPEID_UNIT:
        {
            // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "RemoveAllObjectsInRemoveList %u", obj->GetEntry());
            // in case triggered sequence some spell can continue casting after prev CleanupsBeforeDelete call
            // make sure that like sources auras/etc removed before destructor start
            obj->ToCreature()->CleanupsBeforeDelete();
            volatile uint32 isInWorld_ = obj->IsInWorld();

            RemoveFromMap(obj->ToCreature(), true);
            //volatile uint32 appliedAurasCount = obj->ToUnit()->GetAppliedAuras().size();
            //volatile uint32 ownedAurasCount = obj->ToUnit()->GetOwnedAuras().size();
            break;
        }
        default:
            TC_LOG_ERROR(LOG_FILTER_MAPS, "Non-grid object (TypeId: %u) is in grid object remove list, ignored.", obj->GetTypeId());
            break;
        }
    }
}

uint32 Map::GetPlayersCountExceptGMs() const
{
    uint32 count = 0;
    for (const auto& itr : m_mapRefManager)
        if (!itr.getSource()->isGameMaster())
            ++count;
    return count;
}

void Map::SendToPlayers(WorldPacket const* data) const
{
    for (MapRefManager::const_iterator itr = m_mapRefManager.begin(), next; itr != m_mapRefManager.end(); itr = next)
    {
        next = itr;
        ++next;
        itr->getSource()->SendDirectMessage(data);
    }
}

void Map::ApplyOnEveryPlayer(std::function<void(Player*)> function)
{
    auto const& players = GetPlayers();
    if (players.isEmpty())
        return;

    for (auto const& itr : players)
        if (auto player = itr.getSource())
            if (player->CanContact() && player->GetMap() == this)
                function(player);
}

bool Map::ActiveObjectsNearGrid(NGrid const &ngrid) const
{
    CellCoord cell_min(ngrid.getX() * MAX_NUMBER_OF_CELLS, ngrid.getY() * MAX_NUMBER_OF_CELLS);
    CellCoord cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord+MAX_NUMBER_OF_CELLS);

    //we must find visible range in cells so we unload only non-visible cells...
    float viewDist = m_VisibleDistance;
    int cell_range = static_cast<int>(ceilf(viewDist / SIZE_OF_GRID_CELL)) + 1;

    cell_min.dec_x(cell_range);
    cell_min.dec_y(cell_range);
    cell_max.inc_x(cell_range);
    cell_max.inc_y(cell_range);

    for (const auto& iter : m_mapRefManager)
    {
        Player const * const player = iter.getSource();
        if (!player)
            continue;

        auto const p = Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY());
        if ((cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) && (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
        
    }

    for (auto const &obj : m_activeNonPlayers)
    {
        auto const p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
        if ((cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) && (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
    }

    return false;
}

void Map::AddWorldObject(WorldObject* obj)
{
    i_worldObjects.emplace(obj);
}

void Map::RemoveWorldObject(WorldObject* obj)
{
    i_worldObjects.erase(worldObjectHashGen(obj));
}

WorldObjectSet& Map::GetAllWorldObjectOnMap()
{
    return i_worldObjects;
}

WorldObjectSet const& Map::GetAllWorldObjectOnMap() const
{
    return i_worldObjects;
}

void Map::AddToActive(Creature* c)
{
    AddToActiveHelper(c);

    // also not allow unloading spawn grid to prevent creating creature clone at load
    if (!c->isPet() && c->GetDBTableGUIDLow())
    {
        float x, y, z;
        c->GetRespawnPosition(x, y, z);
        GridCoord p = Trinity::ComputeGridCoord(x, y);
        if (getNGrid(p.x_coord, p.y_coord))
            getNGrid(p.x_coord, p.y_coord)->incUnloadActiveLock();
        else
        {
            GridCoord p2 = Trinity::ComputeGridCoord(c->GetPositionX(), c->GetPositionY());
            TC_LOG_ERROR(LOG_FILTER_MAPS, "Active creature (GUID: %u Entry: %u) added to grid[%u, %u] but spawn grid[%u, %u] was not loaded.",
                c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
        }
    }
}

void Map::RemoveFromActive(Creature* c)
{
    RemoveFromActiveHelper(c);

    // also allow unloading spawn grid
    if (!c->isPet() && c->GetDBTableGUIDLow())
    {
        float x, y, z;
        c->GetRespawnPosition(x, y, z);
        GridCoord p = Trinity::ComputeGridCoord(x, y);
        if (getNGrid(p.x_coord, p.y_coord))
            getNGrid(p.x_coord, p.y_coord)->decUnloadActiveLock();
        else
        {
            GridCoord p2 = Trinity::ComputeGridCoord(c->GetPositionX(), c->GetPositionY());
            TC_LOG_ERROR(LOG_FILTER_MAPS, "Active creature (GUID: %u Entry: %u) removed from grid[%u, %u] but spawn grid[%u, %u] was not loaded.",
                c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
        }
    }
}

template bool Map::AddToMap(Corpse*);
template bool Map::AddToMap(Creature*);
template bool Map::AddToMap(GameObject*);
template bool Map::AddToMap(DynamicObject*);
template bool Map::AddToMap(AreaTrigger*);
template bool Map::AddToMap(Conversation*);

template void Map::RemoveFromMap(Corpse*, bool);
template void Map::RemoveFromMap(Creature*, bool);
template void Map::RemoveFromMap(GameObject*, bool);
template void Map::RemoveFromMap(DynamicObject*, bool);
template void Map::RemoveFromMap(AreaTrigger*, bool);
template void Map::RemoveFromMap(Conversation*, bool);

/* ******* Dungeon Instance Maps ******* */

InstanceMap::InstanceMap(uint32 id, time_t expiry, uint32 InstanceId, Difficulty difficulty, Map* _parent) : Map(id, expiry, InstanceId, difficulty, _parent)
{
    m_resetAfterUnload = false;
    m_unloadWhenEmpty = false;
    i_data = nullptr;
    i_script_id = 0;

    //lets initialize visibility distance for dungeons
    InstanceMap::InitVisibilityDistance();

    if(float _distMap = GetVisibleDistance(TYPE_VISIBLE_MAP, id))
        m_VisibleDistance = _distMap;

    // the timer is started by default, and stopped when the first player joins
    // this make sure it gets unloaded if for some reason no player joins
    m_unloadTimer = std::max(sWorld->getIntConfig(CONFIG_INSTANCE_UNLOAD_DELAY), static_cast<uint32>(MIN_UNLOAD_DELAY));
}

InstanceMap::~InstanceMap()
{
    delete i_data;
    i_data = nullptr;
}

void InstanceMap::InitVisibilityDistance()
{
    //init visibility distance for instances
    m_VisibleDistance = World::GetMaxVisibleDistanceInInstances();
    m_VisibilityNotifyPeriod = World::GetVisibilityNotifyPeriodInInstances();
}

/*
    Do map specific checks to see if the player can enter
*/
bool InstanceMap::CanEnter(Player* player)
{
    if (player->GetMapRef().getTarget() == this)
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "InstanceMap::CanEnter - player %s(%u) already in map %d, %d, %d!", player->GetName(), player->GetGUIDLow(), GetId(), GetInstanceId(), GetSpawnMode());
        //ASSERT(false);
        return false;
    }

    // allow GM's to enter
    if (player->isGameMaster())
        return Map::CanEnter(player);

    // cannot enter if the instance is full (player cap), GMs don't count
    uint32 maxPlayers = GetMaxPlayers();
    if (GetPlayersCountExceptGMs() >= maxPlayers && GetId() != 1191)
    {
        TC_LOG_INFO(LOG_FILTER_MAPS, "MAP: Instance '%u' of map '%s' cannot have more than '%u' players. Player '%s' rejected", GetInstanceId(), GetMapName(), maxPlayers, player->GetName());
        player->SendTransferAborted(GetId(), TRANSFER_ABORT_MAX_PLAYERS);
        return false;
    }

    // cannot enter while an encounter is in progress on raids
    /*Group* group = player->GetGroup();
    if (!player->isGameMaster() && group && group->InCombatToInstance(GetInstanceId()) && player->GetMapId() != GetId())*/
    if (IsRaid() && GetInstanceScript() && GetInstanceScript()->IsEncounterInProgress())
    {
        //TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MAP: Instance '%u' of map '%s' IsEncounterInProgress %u", GetInstanceId(), GetMapName(), GetInstanceScript()->IsEncounterInProgress());
        player->SendTransferAborted(GetId(), TRANSFER_ABORT_ZONE_IN_COMBAT);
        return false;
    }

    // Check for challenge
    if (InstanceScript* data_s = GetInstanceScript())
        if (Challenge* challenge = data_s->GetChallenge())
            if (challenge->_run && challenge->_challengers.find(player->GetGUID()) == challenge->_challengers.end())
            {
                player->SendTransferAborted(GetId(), TRANSFER_ABORT_LOCKED_TO_DIFFERENT_INSTANCE);
                return false;
            }

    // cannot enter if instance is in use by another party/soloer that have a
    // permanent save in the same instance id

    PlayerList const &playerList = GetPlayers();
    if (!playerList.isEmpty())
    {
        if (!player->GetGroup() && GetId() != 1191) // player has not group and there is someone inside, deny entry & except ashran - replace this retarded check a bit later ^^
        {
            player->SendTransferAborted(GetId(), TRANSFER_ABORT_MAX_PLAYERS);
            return false;
        }

        for (const auto& i : playerList)
        {
            if (Player* iPlayer = i.getSource())
            {
                if (iPlayer->isGameMaster()) // bypass GMs
                    continue;

                if (!iPlayer->GetGroup() && GetId() != 1191) // player has not group and there is someone inside, deny entry
                {
                    player->SendTransferAborted(GetId(), TRANSFER_ABORT_MAX_PLAYERS);
                    return false;
                }
                // player inside instance has no group or his groups is different to entering player's one, deny entry
                if (iPlayer->GetGroup() != player->GetGroup() && GetId() != 1191)
                {
                    player->SendTransferAborted(GetId(), TRANSFER_ABORT_MAX_PLAYERS);
                    return false;
                }
                break;
            }
        }
    }

    return Map::CanEnter(player);
}

/*
    Do map specific checks and add the player to the map if successful.
*/
bool InstanceMap::AddPlayerToMap(Player* player, bool initPlayer /*= true*/)
{
    // TODO: Not sure about checking player level: already done in HandleAreaTrigger
    // GMs still can teleport player in instance.
    // Is it needed?

    // Dungeon only code
    if (IsDungeon())
    {
        Group* group = player->GetGroup();

        // get or create an instance save for the map
        InstanceSave* mapSave = sInstanceSaveMgr->GetInstanceSave(GetInstanceId());
        if (!mapSave)
        {
            std::string data;
            TC_LOG_INFO(LOG_FILTER_MAPS, "InstanceMap::Add: creating instance save for map %d spawnmode %d with instance id %d", GetId(), GetSpawnMode(), GetInstanceId());
            mapSave = sInstanceSaveMgr->AddInstanceSave(GetId(), GetInstanceId(), GetDifficultyID(), 0, data, 0, true);
        }

        if (IsScenario() || sObjectMgr->HasScenarioInMap(GetId()))
        {
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "InstanceMap::Add: creating Scenario map %d spawnmode %d instanceid %d", GetId(), GetSpawnMode(), GetInstanceId());

            if(!sScenarioMgr->GetScenario(GetInstanceId()))
            {
                bool find = false;
                if (player->GetScenarioId())
                {
                    if (lfg::LFGDungeonData const* data = sLFGMgr->GetLFGDungeon(player->GetScenarioId(), static_cast<uint16>(GetId())))
                    {
                        if (sScenarioMgr->HasScenarioStep(data, player))
                        {
                            sScenarioMgr->AddScenario(this, data, player, true);
                            find = true;
                        }
                    }
                }
                if (!find)
                {
                    lfg::LFGDungeonData const* data = group ? group->m_dungeon : nullptr;
                    if (!data)
                        data = sLFGMgr->GetLFGDungeon(GetId(), GetDifficultyID(), player->GetTeam());
                    if (data)
                        if (sScenarioMgr->HasScenarioStep(data, player))
                            sScenarioMgr->AddScenario(this, data, player);
                }
            }
        }

        // check for existing instance binds
        InstancePlayerBind* playerBind = player->GetBoundInstance(GetId(), GetDifficultyID());
        if (playerBind && playerBind->perm && GetEntry()->ExpansionID < EXPANSION_WARLORDS_OF_DRAENOR)
        {
            if (IsGarrison())
                player->BindToInstance(mapSave, false);
            else if (mapSave->SaveIsOld() && mapSave->GetExtended())
            {
                WorldPackets::Instance::PendingRaidLock lock;
                lock.TimeUntilLock = i_data ? i_data->GetCompletedEncounterMask() : 0;
                lock.CompletedMask = 60000;
                lock.Extending = false;
                lock.WarningOnly = false; // events it throws:  1 : INSTANCE_LOCK_WARNING   0 : INSTANCE_LOCK_STOP / INSTANCE_LOCK_START
                player->SendDirectMessage(lock.Write());
                player->SetPendingBind(mapSave->GetInstanceId(), 60000);
            }
            else if (playerBind->save && playerBind->save->SaveIsOld() && !playerBind->save->GetExtended())
            {
                player->UnbindInstance(GetId(), GetDifficultyID());
                player->BindToInstance(mapSave, false);
            }
            else if (playerBind->save != mapSave) // cannot enter other instances if bound permanently
            {
                if (mapSave && playerBind->save)
                    TC_LOG_ERROR(LOG_FILTER_MAPS, "InstanceMap::Add: player %s(%d) is permanently bound to instance %d, %d, %d, %d, %d, %d but he is being put into instance %d, %d, %d, %d, %d, %d", player->GetName(), player->GetGUIDLow(), playerBind->save->GetMapId(), playerBind->save->GetInstanceId(), playerBind->save->GetDifficultyID(), playerBind->save->GetPlayerCount(), playerBind->save->GetGroupCount(), playerBind->save->CanReset(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficultyID(), mapSave->GetPlayerCount(), mapSave->GetGroupCount(), mapSave->CanReset());
                return false;
            }
        }
        else
        {
            if (group)
            {
                // solo saves should be reset when entering a group
                InstanceGroupBind* groupBind = group->GetBoundInstance(this);
                if (playerBind && playerBind->save != mapSave && GetEntry()->ExpansionID <= EXPANSION_WARLORDS_OF_DRAENOR)
                {
                    TC_LOG_ERROR(LOG_FILTER_MAPS, "InstanceMap::Add: player %s(%d) is being put into instance %d, %d, %d, %d, %d, %d but he is in group %d and is bound to instance %d, %d, %d, %d, %d, %d!", player->GetName(), player->GetGUIDLow(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficultyID(), mapSave->GetPlayerCount(), mapSave->GetGroupCount(), mapSave->CanReset(), group->GetLeaderGUID().GetGUIDLow(), playerBind->save->GetMapId(), playerBind->save->GetInstanceId(), playerBind->save->GetDifficultyID(), playerBind->save->GetPlayerCount(), playerBind->save->GetGroupCount(), playerBind->save->CanReset());
                    if (groupBind)
                        TC_LOG_ERROR(LOG_FILTER_MAPS, "InstanceMap::Add: the group is bound to the instance %d, %d, %d, %d, %d, %d", groupBind->save->GetMapId(), groupBind->save->GetInstanceId(), groupBind->save->GetDifficultyID(), groupBind->save->GetPlayerCount(), groupBind->save->GetGroupCount(), groupBind->save->CanReset());
                    //ASSERT(false);
                    return false;
                }
                // bind to the group or keep using the group save
                if (!groupBind)
                    group->BindToInstance(mapSave, false);
                else
                {
                    // cannot jump to a different instance without resetting it
                    if (groupBind->save != mapSave)
                    {
                        if (mapSave)
                        {
                            TC_LOG_ERROR(LOG_FILTER_MAPS, "MapSave players: %d, group count: %d", mapSave->GetPlayerCount(), mapSave->GetGroupCount());
                            TC_LOG_ERROR(LOG_FILTER_MAPS, "InstanceMap::Add: player %s(%d) is being put into instance %d, %d, %d but he is in group %d which is bound to instance %d, %d, %d!", player->GetName(), player->GetGUIDLow(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficultyID(), group->GetLeaderGUID().GetGUIDLow(), groupBind->save->GetMapId(), groupBind->save->GetInstanceId(), groupBind->save->GetDifficultyID());
                        }
                        else
                            TC_LOG_ERROR(LOG_FILTER_MAPS, "MapSave NULL");
                        if (groupBind->save)
                            TC_LOG_ERROR(LOG_FILTER_MAPS, "GroupBind save players: %d, group count: %d", groupBind->save->GetPlayerCount(), groupBind->save->GetGroupCount());
                        else
                            TC_LOG_ERROR(LOG_FILTER_MAPS, "GroupBind save NULL");
                        return false;
                    }
                    // if the group/leader is permanently bound to the instance
                    // players also become permanently bound when they enter
                    if (groupBind->perm || mapSave->SaveIsOld() && mapSave->GetExtended())
                    {
                        WorldPackets::Instance::PendingRaidLock lock;
                        lock.TimeUntilLock = i_data ? i_data->GetCompletedEncounterMask() : 0;
                        lock.CompletedMask = 60000;
                        lock.Extending = false;
                        lock.WarningOnly = false; // events it throws:  1 : INSTANCE_LOCK_WARNING   0 : INSTANCE_LOCK_STOP / INSTANCE_LOCK_START
                        player->SendDirectMessage(lock.Write());
                        player->SetPendingBind(mapSave->GetInstanceId(), 60000);
                    }
                }
            }
            else
            {
                // set up a solo bind or continue using it
                if (!playerBind || IsGarrison())
                {
                    if (!IsScenario())
                        player->BindToInstance(mapSave, false);
                }
                else if (playerBind->save && playerBind->save->SaveIsOld() && !playerBind->save->GetExtended())
                {
                    player->UnbindInstance(GetId(), GetDifficultyID());
                    player->BindToInstance(mapSave, false);
                }
                else if (mapSave->SaveIsOld() && mapSave->GetExtended())
                {
                    WorldPackets::Instance::PendingRaidLock lock;
                    lock.TimeUntilLock = i_data ? i_data->GetCompletedEncounterMask() : 0;
                    lock.CompletedMask = 60000;
                    lock.Extending = false;
                    lock.WarningOnly = false; // events it throws:  1 : INSTANCE_LOCK_WARNING   0 : INSTANCE_LOCK_STOP / INSTANCE_LOCK_START
                    player->SendDirectMessage(lock.Write());
                    player->SetPendingBind(mapSave->GetInstanceId(), 60000);
                }
                else if(playerBind->save != mapSave && GetEntry()->ExpansionID <= EXPANSION_WARLORDS_OF_DRAENOR)
                {
                    // cannot jump to a different instance without resetting it
                    //ASSERT(playerBind->save == mapSave);
                    if (mapSave && playerBind->save)
                        TC_LOG_ERROR(LOG_FILTER_MAPS, "InstanceMap::Add: player %s(%d) is permanently bound to instance %d, %d, %d, %d, %d, %d but he is being put into instance %d, %d, %d, %d, %d, %d", player->GetName(), player->GetGUIDLow(), playerBind->save->GetMapId(), playerBind->save->GetInstanceId(), playerBind->save->GetDifficultyID(), playerBind->save->GetPlayerCount(), playerBind->save->GetGroupCount(), playerBind->save->CanReset(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficultyID(), mapSave->GetPlayerCount(), mapSave->GetGroupCount(), mapSave->CanReset());
                    return false;
                }
            }
        }
    }

    // for normal instances cancel the reset schedule when the
    // first player enters (no players yet)
    SetResetSchedule(false);

    #ifdef TRINITY_DEBUG
    TC_LOG_INFO(LOG_FILTER_MAPS, "MAP: Player '%s' entered instance '%u' of map '%s'", player->GetName(), GetInstanceId(), GetMapName());
    #endif
    // initialize unload state
    m_unloadTimer = 0;
    m_resetAfterUnload = false;
    m_unloadWhenEmpty = false;

    if (InstanceScript* data_s = GetInstanceScript())
    {
        data_s->OnPlayerEnter(player);
        data_s->OnPlayerEnterForScript(player);
    }

    player->AddDelayedEvent(10, [player]() -> void
    {
        if (player)
            player->OnEnterMap(); // UpdatePhase
    });

    // this will acquire the same mutex so it cannot be in the previous block
    Map::AddPlayerToMap(player, initPlayer);

    SendInstanceGroupSizeChanged();

    return true;
}

void InstanceMap::Update(const uint32 t_diff)
{
    Map::Update(t_diff);

    if (i_data)
    {
        i_data->Update(t_diff);
        i_data->UpdateForScript(t_diff);
    }
}

void InstanceMap::RemovePlayerFromMap(Player* player, bool remove)
{
    #ifdef TRINITY_DEBUG
    TC_LOG_INFO(LOG_FILTER_MAPS, "MAP: Removing player '%s' from instance '%u' of map '%s' before relocating to another map", player->GetName(), GetInstanceId(), GetMapName());
    #endif
    //if last player set unload timer
    if (!m_unloadTimer && m_mapRefManager.getSize() == 1)
        m_unloadTimer = m_unloadWhenEmpty ? MIN_UNLOAD_DELAY : std::max(sWorld->getIntConfig(CONFIG_INSTANCE_UNLOAD_DELAY), static_cast<uint32>(MIN_UNLOAD_DELAY));

    Map::RemovePlayerFromMap(player, remove);
    // for normal instances schedule the reset after all players have left
    SetResetSchedule(true);
    sInstanceSaveMgr->UnloadInstanceSave(GetInstanceId());

    SendInstanceGroupSizeChanged();
}

void InstanceMap::CreateInstanceData(InstanceSave* save)
{
    if (i_data != nullptr)
        return;

    InstanceTemplate const* mInstance = sObjectMgr->GetInstanceTemplate(GetId());
    if (mInstance)
    {
        i_script_id = mInstance->ScriptId;
        i_data = sScriptMgr->CreateInstanceData(this);
    }

    if (!i_data)
        return;

    i_data->Initialize();
    i_data->CreateInstance();

    if (save)
    {
        std::string data = save->GetData();
        i_data->SetCompletedEncountersMask(save->GetCompletedEncounterMask());
        if (data != "")
        {
            #ifdef TRINITY_DEBUG
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "Loading instance data for `%s` with id %u", sObjectMgr->GetScriptName(i_script_id).c_str(), i_InstanceId);
            #endif
            i_data->Load(data.c_str());
        }
    }
}

/*
    Returns true if there are no players in the instance
*/
bool InstanceMap::Reset(uint8 method)
{
    // note: since the map may not be loaded when the instance needs to be reset
    // the instance must be deleted from the DB by InstanceSaveManager

    if (HavePlayers())
    {
        if (method == INSTANCE_RESET_ALL || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // notify the players to leave the instance so it can be reset
            for (auto& itr : m_mapRefManager)
                itr.getSource()->SendResetFailedNotify();
        }
        else
        {
            if (method == INSTANCE_RESET_GLOBAL)
                // set the homebind timer for players inside (1 minute)
                for (auto& itr : m_mapRefManager)
                    itr.getSource()->m_InstanceValid = false;

            // the unload timer is not started
            // instead the map will unload immediately after the players have left
            m_unloadWhenEmpty = true;
            m_resetAfterUnload = true;
        }
    }
    else
    {
        // unloaded at next update
        m_unloadTimer = MIN_UNLOAD_DELAY;
        m_resetAfterUnload = true;
    }

    return m_mapRefManager.isEmpty();
}

void InstanceMap::PermBindAllPlayers(Player* source)
{
    if (!IsDungeon() || IsLfr() || isChallenge())
        return;

    InstanceSave* save = sInstanceSaveMgr->GetInstanceSave(GetInstanceId());
    if (!save)
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "Cannot bind player (GUID: %u, Name: %s), because no instance save is available for instance map (Name: %s, Entry: %u, InstanceId: %u)!", source->GetGUIDLow(), source->GetName(), source->GetMap()->GetMapName(), source->GetMapId(), GetInstanceId());
        return;
    }

    save->SetPerm(true);
    save->SetExtended(false);

    if (MapDifficultyEntry const* mapDiff = sDB2Manager.GetMapDifficultyData(save->GetMapId(), save->GetDifficultyID()))
        save->SetResetTime(sWorld->getInstanceResetTime(mapDiff->GetRaidDuration()));

    Group* group = source->GetGroup();
    // group members outside the instance group don't get bound
    for (auto& itr : m_mapRefManager)
    {
        Player* player = itr.getSource();
        // players inside an instance cannot be bound to other instances
        // some players may already be permanently bound, in this case nothing happens
        InstancePlayerBind* bind = player->GetBoundInstance(save->GetMapId(), save->GetDifficultyID());
        if (!bind || !bind->perm)
        {
            player->BindToInstance(save, true);
            player->SendDirectMessage(WorldPackets::Instance::InstanceSaveCreated(false).Write());
            player->SendCalendarRaidLockout(save, true);
        }

        // if the leader is not in the instance the group will not get a perm bind
        if (group && group->GetLeaderGUID() == player->GetGUID())
            group->BindToInstance(save, true);
    }
}

std::string const& InstanceMap::GetScriptName() const
{
    return sObjectMgr->GetScriptName(i_script_id);
}

bool InstanceHasScript(WorldObject const* obj, char const* scriptName)
{
    if (InstanceMap* instance = obj->GetMap()->ToInstanceMap())
        return instance->GetScriptName() == scriptName;

    return false;
}

void InstanceMap::UnloadAll()
{
    if (!m_worldCrashChecker)
        ASSERT(!HavePlayers());

    if (m_resetAfterUnload == true)
        DeleteRespawnTimes();

    volatile uint32 _mspId = GetId();
    volatile uint32 _instanceId = GetInstanceId();

    Map::UnloadAll();
}

void InstanceMap::SendResetWarnings(uint32 timeLeft) const
{
    for (const auto& itr : m_mapRefManager)
        itr.getSource()->SendInstanceResetWarning(GetId(), itr.getSource()->GetDifficultyID(GetEntry()), timeLeft);
}

void InstanceMap::SendInstanceGroupSizeChanged() const
{
    for (const auto& itr : m_mapRefManager)
        itr.getSource()->SendDirectMessage(WorldPackets::Instance::InstanceGroupSizeChanged(GetPlayersCountExceptGMs()).Write());
}

void InstanceMap::SetResetSchedule(bool on)
{
    // only for normal instances
    // the reset time is only scheduled when there are no payers inside
    // it is assumed that the reset time will rarely (if ever) change while the reset is scheduled
    if (IsDungeon() && !HavePlayers() && !IsRaidOrHeroicDungeon())
    {
        if (InstanceSave* save = sInstanceSaveMgr->GetInstanceSave(GetInstanceId()))
            sInstanceSaveMgr->ScheduleReset(on, save->GetResetTime(), InstanceSaveManager::InstResetEvent(0, GetId(), Difficulty(GetSpawnMode()), GetInstanceId()));
        else
            TC_LOG_ERROR(LOG_FILTER_MAPS, "InstanceMap::SetResetSchedule: cannot turn schedule %s, there is no save information for instance (map [id: %u, name: %s], instance id: %u, difficulty: %u)",
                on ? "on" : "off", GetId(), GetMapName(), GetInstanceId(), Difficulty(GetSpawnMode()));
    }
}

void InstanceMap::UpdatePhasing()
{
    int8 step = -1;
    if (uint32 instanceId = GetInstanceId())
        if (Scenario* progress = sScenarioMgr->GetScenario(instanceId))
            step = progress->GetCurrentStep();

    ApplyOnEveryPlayer([&, step](Player* player)
    {
        if (!player->CanContact())
            return;

        player->AddDelayedEvent(100, [player, step]() -> void
        {
            PhaseUpdateData phaseUdateData;
            phaseUdateData.AddConditionType(CONDITION_INSTANCE_INFO);

            if (step >= 0)
                phaseUdateData.AddScenarioUpdate(step);
            player->GetPhaseMgr().NotifyConditionChanged(phaseUdateData);
        });
    });
}

MapDifficultyEntry const* Map::GetMapDifficulty() const
{
    return sDB2Manager.GetMapDifficultyData(GetId(), GetDifficultyID());
}

uint32 Map::GetDifficultyLootItemContext(bool isQuest, bool maxLevel, bool isBoss) const
{
    if (isBoss || IsRaid())
    {
        if (MapDifficultyEntry const* mapDifficulty = sDB2Manager.GetMapDifficultyData(GetId(), i_lootDifficulty))
            if (mapDifficulty->ItemContext)
            {
                if (mapDifficulty->ItemContext == 17) // option for non max level player
                {
                    if (!maxLevel)
                        return mapDifficulty->ItemContext;
                }
                else
                    return mapDifficulty->ItemContext;
            }

        if (DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(i_lootDifficulty))
            return difficulty->ItemContext;
    }

    if (!isQuest && GetEntry()->ExpansionID == EXPANSION_LEGION) // For legion map we use default index 21 for auto scaling item
        return 21;

    return 0;
}

uint16 Map::GetMapMaxPlayers() const
{
    if (i_mapEntry)
        return i_mapEntry->MaxPlayers;

    return 0;
}

bool Map::IsHeroic() const
{
    if (DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(i_difficulty))
        return difficulty->Flags & (DIFFICULTY_FLAG_HEROIC | DIFFICULTY_FLAG_DISPLAY_HEROIC);

    return false;
}

bool Map::IsNeedRecalc() const
{
    if (DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(i_difficulty))
        return difficulty->MinPlayers != difficulty->MaxPlayers;

    return true;
}

bool Map::IsCanScale() const
{
    switch (i_difficulty)
    {
        case DIFFICULTY_NORMAL:
        case DIFFICULTY_HC_SCENARIO:
        case DIFFICULTY_N_SCENARIO:
        case DIFFICULTY_EVENT_DUNGEON:
        case DIFFICULTY_EVENT_SCENARIO:
        case DIFFICULTY_PVEVP_SCENARIO:
        case DIFFICULTY_EVENT_SCENARIO_6:
        case DIFFICULTY_WORLD_PVP_SCENARIO_2:
            return true;
    }

    return false;
}

uint32 Map::GetMaxPlayer() const
{
    if (DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(i_difficulty))
        return difficulty->MaxPlayers;

    return 10;
}


uint32 Map::GetMinPlayer() const
{
    if (DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(i_difficulty))
        return difficulty->MinPlayers;

    return 1;
}

bool Map::GetEntrancePos(int32& mapid, float& x, float& y)
{
    if (!i_mapEntry)
        return false;
    return i_mapEntry->GetEntrancePos(mapid, x, y);
}

uint32 InstanceMap::GetMaxPlayers() const
{
    if (MapDifficultyEntry const* mapDiff = GetMapDifficulty())
    {
        if (mapDiff->MaxPlayers/* || IsRegularDifficulty()*/)    // Normal case (expect that regular difficulty always have correct maxplayers)
            return mapDiff->MaxPlayers;
            // DBC have 0 maxplayers for heroic instances with expansion < 2
        // The heroic entry exists, so we don't have to check anything, simply return normal max players
        MapDifficultyEntry const* normalDiff = sDB2Manager.GetMapDifficultyData(GetId(), DIFFICULTY_NORMAL);
        return normalDiff ? normalDiff->MaxPlayers : 0;
    }
        // I'd rather ASSERT(false);
    return 0;
}

uint32 InstanceMap::GetMaxResetDelay() const
{
    MapDifficultyEntry const* mapDiff = GetMapDifficulty();
    return mapDiff ? mapDiff->GetRaidDuration() : 0;
}

const WorldLocation* InstanceMap::GetClosestGraveYard(float x, float y, float z)
{
    WorldLocation const* location = nullptr;

    if (i_data) // For use in instance script
        if (location = i_data->GetClosestGraveYard(x, y, z))
            return location;

    float dist = 10000.0f;
    bool found = false;

    if(std::vector<WorldLocation> const* graveYard = sObjectMgr->GetInstanceGraveYard(GetId()))
    {
        for (auto& data : *graveYard)
        {
            float dist2 = (data.GetPositionX() - x)*(data.GetPositionX() - x) + (data.GetPositionY() - y)*(data.GetPositionY() - y) + (data.GetPositionZ() - z)*(data.GetPositionZ() - z);
            if (found)
            {
                if (dist2 < dist)
                {
                    dist = dist2;
                    location = &data;
                }
            }
            else
            {
                found = true;
                dist = dist2;
                location = &data;
            }
        }
    }
    return location;
}
/* ******* Zone Instance Maps ******* */

ZoneMap::ZoneMap(uint32 id, time_t expiry, uint32 zoneId, Map* _parent, Difficulty difficulty) : Map(id, expiry, zoneId, difficulty, _parent)
{
    ZoneMap::InitVisibilityDistance();

    if(float _distMap = GetVisibleDistance(TYPE_VISIBLE_MAP, id))
        m_VisibleDistance = _distMap;
}

ZoneMap::~ZoneMap()
{ }

void ZoneMap::InitVisibilityDistance()
{
    m_VisibleDistance = World::GetMaxVisibleDistanceOnContinents();
    m_VisibilityNotifyPeriod = World::GetVisibilityNotifyPeriodOnContinents();
}

bool ZoneMap::CanEnter(Player* player)
{
    return Map::CanEnter(player);
}

bool ZoneMap::AddPlayerToMap(Player* player, bool initPlayer /*= true*/)
{
    player->AddDelayedEvent(10, [player]() -> void
    {
        if (player)
            player->OnEnterMap(); // UpdatePhase
    });

    // player->m_InstanceValid = true;
    return Map::AddPlayerToMap(player, initPlayer);
}

void ZoneMap::RemovePlayerFromMap(Player* player, bool remove)
{
    Map::RemovePlayerFromMap(player, remove);
}

TempSummon* Map::SummonCreature(uint32 entry, Position const& pos, SummonPropertiesEntry const* properties /*= NULL*/, uint32 duration /*= 0*/, Unit* summoner /*= NULL*/, ObjectGuid targetGuid /*= 0*/, uint32 spellId /*= 0*/, int32 vehId /*= 0*/, ObjectGuid viewerGuid /*= 0*/, GuidUnorderedSet* viewersList /*= NULL*/)
{
    if (summoner && entry != 114975 && entry != 44548 && entry != 230018 && entry != 230019)  // hack for brawguild
    {
        std::list<Creature*> creatures;
        summoner->GetAliveCreatureListWithEntryInGrid(creatures, entry, 110.0f);
        if (creatures.size() > (GetInstanceId() ? 100 : 50))
            return nullptr;
    }

    uint32 mask = UNIT_MASK_SUMMON;
    if (properties)
    {
        switch (properties->Control)
        {
            case SUMMON_CATEGORY_PET:
                mask = UNIT_MASK_GUARDIAN;
                break;
            case SUMMON_CATEGORY_PUPPET:
                mask = UNIT_MASK_PUPPET;
                break;
            case SUMMON_CATEGORY_VEHICLE:
                if (properties->ID == 3384) //hardfix despawn npc 63872
                    mask = UNIT_MASK_SUMMON;
                else
                    mask = UNIT_MASK_MINION;
                break;
            case SUMMON_CATEGORY_WILD:
            case SUMMON_CATEGORY_ALLY:
            case SUMMON_CATEGORY_UNK:
            {
                switch (properties->Title)
                {
                    case SUMMON_TYPE_MINION:
                    case SUMMON_TYPE_GUARDIAN:
                    case SUMMON_TYPE_GUARDIAN2:
                    case SUMMON_TYPE_LIGHTWELL:
                        mask = UNIT_MASK_GUARDIAN;
                        break;
                    case SUMMON_TYPE_TOTEM:
                    case SUMMON_TYPE_BANNER:
                    case SUMMON_TYPE_STATUE:
                        mask = UNIT_MASK_TOTEM;
                        break;
                    case SUMMON_TYPE_VEHICLE:
                    case SUMMON_TYPE_VEHICLE2:
                    case SUMMON_TYPE_GATE:
                        mask = UNIT_MASK_SUMMON;
                        break;
                    case SUMMON_TYPE_MINIPET:
                        mask = UNIT_MASK_MINION;
                        break;
                    default:
                    {
                        if (properties->Flags & 512 || properties->ID == 2921 || properties->ID == 3459 || properties->ID == 3097) // Mirror Image, Summon Gargoyle
                            mask = UNIT_MASK_GUARDIAN;
                        if (properties->ID == 3934 || properties->ID == 3234 || properties->ID == 3712 || properties->ID == 3456 || properties->ID == 3687) // Totem Mastery, Transcendence, Call Dreadstalkers, Efflorescence, Call to the Void
                            mask = UNIT_MASK_TOTEM;
                        break;
                    }
                }
                break;
            }
            default:
                return nullptr;
        }
    }

	if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS) && (entry == 121661 || entry == 69791 || entry == 69792))
		mask = UNIT_MASK_GUARDIAN;

    uint32 phase = PHASEMASK_NORMAL;
    uint32 team = 0;
    std::set<uint32> phaseIds;

    if (summoner)
    {
        phase = summoner->GetPhaseMask();
        phaseIds = summoner->GetPhases();
        if (summoner->IsPlayer())
            team = summoner->ToPlayer()->GetTeam();
    }

    TempSummon* summon = nullptr;
    switch (mask)
    {
        case UNIT_MASK_SUMMON:
            summon = new TempSummon(properties, summoner, false);
            break;
        case UNIT_MASK_GUARDIAN:
            summon = new Guardian(properties, summoner, false);
            break;
        case UNIT_MASK_PUPPET:
            summon = new Puppet(properties, summoner);
            break;
        case UNIT_MASK_TOTEM:
            summon = new Totem(properties, summoner);
            break;
        case UNIT_MASK_MINION:
            summon = new Minion(properties, summoner, false);
            break;
        default:
            return nullptr;
    }

    if (!summon->Create(sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate(), this, phase, entry, vehId, team, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation()))
    {
        delete summon;
        return nullptr;
    }

    summon->SetPhaseId(phaseIds, false);

    summon->SetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL, spellId);
    if (summoner)
    {
        summon->SetTratsport(summoner->GetTransport());
        summon->SetGuidValue(UNIT_FIELD_DEMON_CREATOR, summoner->GetGUID());
        if (properties && (properties->Flags & SUMMON_PROP_FLAG_PERSONAL_SPAWN) != 0)
            summon->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
    }

    summon->SetTargetGUID(targetGuid);

    summon->SetHomePosition(pos);

    summon->InitStats(duration);

    if (!viewerGuid.IsEmpty())
        summon->AddPlayerInPersonnalVisibilityList(viewerGuid);

    if (viewersList)
        summon->AddPlayersInPersonnalVisibilityList(*viewersList);

    AddToMap(summon->ToCreature());
    summon->InitSummon();
    summon->CastPetAuras(true);

    //TC_LOG_DEBUG(LOG_FILTER_PETS, "Map::SummonCreature summoner %u entry %i mask %i", summoner ? summoner->GetGUID() : 0, entry, mask);

    //ObjectAccessor::UpdateObjectVisibility(summon);

    return summon;
}

GameObject* Map::SummonGameObject(uint32 entry, Position pos, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime)
{
    return SummonGameObject(entry, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), ang, rotation0, rotation1, rotation2, rotation3, respawnTime);
}

GameObject* Map::SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime, ObjectGuid viewerGuid, GuidUnorderedSet* viewersList, bool hasCreator /*= true*/)
{
    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if (!goinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject template %u not found in database!", entry);
        return nullptr;
    }
    Map* map = this;
    GameObject* go = sObjectMgr->IsStaticTransport(entry) ? new StaticTransport : new GameObject;
    if (!go->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), entry, map, 1, Position(x, y, z, ang), G3D::Quat(rotation0, rotation1, rotation2, rotation3), 100, GO_STATE_READY))
    {
        delete go;
        return nullptr;
    }
    go->SetRespawnTime(respawnTime);

    go->SetSpawnedByDefault(false);

    if (!viewerGuid.IsEmpty())
        go->AddPlayerInPersonnalVisibilityList(viewerGuid);

    if (viewersList)
        go->AddPlayersInPersonnalVisibilityList(*viewersList);

    map->AddToMap(go);

    return go;
}

void Map::SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list /*= NULL*/)
{
    std::vector<TempSummonData> const* data = sObjectMgr->GetSummonGroup(GetId(), SUMMONER_TYPE_MAP, group);
    if (!data)
        return;

    for (const auto& itr : *data)
        if (TempSummon* summon = SummonCreature(itr.entry, itr.pos, nullptr, itr.time))
            if (list)
                list->push_back(summon);
}

AreaTrigger* Map::GetAreaTrigger(ObjectGuid const& guid)
{
    return ObjectAccessor::GetObjectInMap(guid, this, static_cast<AreaTrigger*>(nullptr));
}

Creature* Map::GetCreature(ObjectGuid const& guid)
{
    return ObjectAccessor::GetObjectInMap(guid, this, static_cast<Creature*>(nullptr));
}

GameObject* Map::GetGameObject(ObjectGuid const& guid)
{
    return ObjectAccessor::GetObjectInMap(guid, this, static_cast<GameObject*>(nullptr));
}

DynamicObject* Map::GetDynamicObject(ObjectGuid const& guid)
{
    return ObjectAccessor::GetObjectInMap(guid, this, static_cast<DynamicObject*>(nullptr));
}

EventObject* Map::GetEventObject(ObjectGuid const& guid)
{
    return ObjectAccessor::GetObjectInMap(guid, this, static_cast<EventObject*>(nullptr));
}

Transport* Map::GetTransport(ObjectGuid const& guid)
{
    GameObject* go = ObjectAccessor::GetObjectInMap(guid, this, static_cast<GameObject*>(nullptr));
    if (!go)
        return nullptr;

    if (Transport* t = go->ToTransport())
        return t;

    return go->ToStaticTransport();
}

StaticTransport* Map::GetStaticTransport(ObjectGuid const& guid)
{
    return Trinity::Containers::MapGetValuePtr(m_StaticTransports, guid);
}

MapInstanced* Map::ToMapInstanced()
{
    if (Instanceable())
        return reinterpret_cast<MapInstanced*>(this);
    return nullptr;
}

MapInstanced const* Map::ToMapInstanced() const
{
    if (Instanceable()) 
        return (const MapInstanced*)((MapInstanced*)this);
    return nullptr;
}

InstanceMap* Map::ToInstanceMap()
{
    if (IsDungeon())
        return reinterpret_cast<InstanceMap*>(this);
    return nullptr;
}

const InstanceMap* Map::ToInstanceMap() const
{
    if (IsDungeon())
        return (const InstanceMap*)((InstanceMap*)this);
    return nullptr;
}

void Map::UpdateIteratorBack(Player* player)
{
    if (m_mapRefIter == player->GetMapRef())
        m_mapRefIter = m_mapRefIter->nocheck_prev();
}

void Map::SaveCreatureRespawnTime(ObjectGuid::LowType const& dbGuid, time_t respawnTime)
{
    if (!respawnTime)
    {
        // Delete only
        RemoveCreatureRespawnTime(dbGuid);
        return;
    }

    i_lockCreatureRespawn.lock();
    _creatureRespawnTimes[dbGuid] = respawnTime;
    i_lockCreatureRespawn.unlock();

    if (respawnTime > (time(nullptr) + 900))
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CREATURE_RESPAWN);
        stmt->setUInt64(0, dbGuid);
        stmt->setUInt32(1, uint32(respawnTime));
        stmt->setUInt16(2, GetId());
        stmt->setUInt32(3, GetInstanceId());
        CharacterDatabase.Execute(stmt);
    }
}

void Map::RemoveCreatureRespawnTime(ObjectGuid::LowType const& dbGuid)
{
    i_lockCreatureRespawn.lock();
    _creatureRespawnTimes.erase(dbGuid);
    i_lockCreatureRespawn.unlock();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CREATURE_RESPAWN);
    stmt->setUInt64(0, dbGuid);
    stmt->setUInt16(1, GetId());
    stmt->setUInt32(2, GetInstanceId());
    CharacterDatabase.Execute(stmt);
}

void Map::SaveGORespawnTime(ObjectGuid::LowType const& dbGuid, time_t respawnTime)
{
    if (!respawnTime)
    {
        // Delete only
        RemoveGORespawnTime(dbGuid);
        return;
    }

    i_lockGoRespawn.lock();
    _goRespawnTimes[dbGuid] = respawnTime;
    i_lockGoRespawn.unlock();

    if (respawnTime > (time(nullptr) + 900))
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GO_RESPAWN);
        stmt->setUInt64(0, dbGuid);
        stmt->setUInt32(1, uint32(respawnTime));
        stmt->setUInt16(2, GetId());
        stmt->setUInt32(3, GetInstanceId());
        CharacterDatabase.Execute(stmt);
    }
}

void Map::RemoveGORespawnTime(ObjectGuid::LowType const& dbGuid)
{
    i_lockGoRespawn.lock();
    _goRespawnTimes.erase(dbGuid);
    i_lockGoRespawn.unlock();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GO_RESPAWN);
    stmt->setUInt64(0, dbGuid);
    stmt->setUInt16(1, GetId());
    stmt->setUInt32(2, GetInstanceId());
    CharacterDatabase.Execute(stmt);
}

void Map::LoadRespawnTimes()
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CREATURE_RESPAWNS);
    stmt->setUInt16(0, GetId());
    stmt->setUInt32(1, GetInstanceId());
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            ObjectGuid::LowType loguid      = fields[0].GetUInt64();
            uint32 respawnTime = fields[1].GetUInt32();

            _creatureRespawnTimes[loguid] = time_t(respawnTime);
        } while (result->NextRow());
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GO_RESPAWNS);
    stmt->setUInt16(0, GetId());
    stmt->setUInt32(1, GetInstanceId());
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            ObjectGuid::LowType loguid      = fields[0].GetUInt64();
            uint32 respawnTime = fields[1].GetUInt32();

            _goRespawnTimes[loguid] = time_t(respawnTime);
        } while (result->NextRow());
    }
}

void Map::DeleteRespawnTimes()
{
    i_lockCreatureRespawn.lock();
    _creatureRespawnTimes.clear();
    i_lockCreatureRespawn.unlock();

    i_lockGoRespawn.lock();
    _goRespawnTimes.clear();
    i_lockGoRespawn.unlock();

    DeleteRespawnTimesInDB(GetId(), GetInstanceId());
}

void Map::DeleteRespawnTimesInDB(uint16 mapId, uint32 instanceId)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CREATURE_RESPAWN_BY_INSTANCE);
    stmt->setUInt16(0, mapId);
    stmt->setUInt32(1, instanceId);
    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GO_RESPAWN_BY_INSTANCE);
    stmt->setUInt16(0, mapId);
    stmt->setUInt32(1, instanceId);
    CharacterDatabase.Execute(stmt);
}

time_t Map::GetLinkedRespawnTime(ObjectGuid const& guid) const
{
    ObjectGuid linkedGuid = sObjectMgr->GetLinkedRespawnGuid(guid);
    switch (linkedGuid.GetHigh())
    {
        case HighGuid::Creature:
            return GetCreatureRespawnTime(linkedGuid.GetCounter());
        case HighGuid::GameObject:
            return GetGORespawnTime(linkedGuid.GetCounter());
        default:
            break;
    }

    return time_t(0);
}

time_t Map::GetCreatureRespawnTime(ObjectGuid::LowType const& dbGuid) const
{
    auto itr = _creatureRespawnTimes.find(dbGuid);
    if (itr != _creatureRespawnTimes.end())
        return itr->second;
    return time_t(0);
}

time_t Map::GetGORespawnTime(ObjectGuid::LowType const& dbGuid) const
{
    auto itr = _goRespawnTimes.find(dbGuid);
    if (itr != _goRespawnTimes.end())
        return itr->second;
    return time_t(0);
}

void Map::loadGridsInRange(Position const &center, float radius)
{
    auto const posX = center.GetPositionX();
    auto const posY = center.GetPositionY();

    CellCoord standingCellCoord(Trinity::ComputeCellCoord(posX, posY));
    if (!standingCellCoord.IsCoordValid())
        return;

    if (radius > SIZE_OF_GRIDS)
        radius = SIZE_OF_GRIDS;

    CellArea area = Cell::CalculateCellArea(posX, posY, radius);
    if (!area)
        return;

    for (uint32 x = area.low_bound.x_coord; x <= area.high_bound.x_coord; ++x)
    {
        for (uint32 y = area.low_bound.y_coord; y <= area.high_bound.y_coord; ++y)
        {
            CellCoord cellCoord(x, y);
            if (cellCoord != standingCellCoord)
                EnsureGridLoaded(Cell(cellCoord));
        }
    }
}

bool Map::IsRemovalGrid(float x, float y) const
{
    GridCoord p = Trinity::ComputeGridCoord(x, y);
    return !getNGrid(p.x_coord, p.y_coord) || getNGrid(p.x_coord, p.y_coord)->GetGridState() == GRID_STATE_REMOVAL;
}

bool Map::IsGridLoaded(float x, float y) const
{
    return IsGridLoaded(Trinity::ComputeGridCoord(x, y));
}

void Map::LoadGrid(Position & pos)
{
    LoadGrid(pos.GetPositionX(), pos.GetPositionY());
}

uint32 Map::GetGridCount()
{
    uint32 count = 0;
    for (auto itr: i_loadedGrids)
        count++;

    return count;
}

WorldObject* Map::GetActiveObjectWithEntry(uint32 entry)
{
    // non-player active objects, increasing iterator in the loop in case of object removal
    for (m_activeNonPlayersIter = m_activeNonPlayers.begin(); m_activeNonPlayersIter != m_activeNonPlayers.end();)
    {
        WorldObject* obj = *m_activeNonPlayersIter;
        ++m_activeNonPlayersIter;

        if (!obj || !obj->IsInWorld() || obj->GetEntry() !=entry)
            continue;

        return obj;
    }
    return nullptr;
}

void Map::UpdateEncounterState(EncounterCreditType type, uint32 creditEntry, Unit* source, Unit* unit)
{
    InstanceScript* instance = source->GetInstanceScript();
    Difficulty diff = GetDifficultyID();

    uint32 completedEncounters = 0;
    if (instance)
        completedEncounters = instance->GetCompletedEncounterMask();

    DungeonEncounterList const* encounters = sObjectMgr->GetDungeonEncounterList(GetId(), diff);
    if (!encounters)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "UpdateEncounterState DungeonEncounterList not find");
        return;
    }

    ObjectGuid groupGuid;
    Player* player = unit->ToPlayer();
    if (player)
        if (Group* group = player->GetGroup())
            groupGuid = group->GetGUID();

    uint32 dungeonId = sLFGMgr->GetDungeon(groupGuid);
    bool isLFGEncounter = false;
    uint32 fullEncounterIndex = 0;
    if (lfg::LfgReward const* reward = sLFGMgr->GetDungeonReward(dungeonId, unit->getLevel()))
    {
        if (reward->encounterMask)
        {
            isLFGEncounter = true;
            fullEncounterIndex = reward->encounterMask;
        }
    }

    uint32 encounterId = 0;
    bool findCustomDungeon = false;

    for (auto encounter : *encounters)
    {
        uint32 completedEncounter = 1 << encounter->dbcEntry->Bit;
        if (encounter->creditType == type && encounter->creditEntry == creditEntry)
        {
            if (!isLFGEncounter || (completedEncounter & fullEncounterIndex))
                completedEncounters |= completedEncounter;

            encounterId = encounter->dbcEntry->ID;
            if (encounter->lastEncounterDungeon && !isLFGEncounter)
            {
                dungeonId = encounter->lastEncounterDungeon;
                findCustomDungeon = true;
                
                TC_LOG_DEBUG(LOG_FILTER_LFG, "UpdateEncounterState: Instance %s (instanceId %u) completed encounter %s. Credit Dungeon: %u, diff %i, creditEntry %i",
                    GetMapName(), GetInstanceId(), encounter->dbcEntry->Name->Get(LOCALE_enUS), dungeonId, diff, creditEntry);
            }
        }
        if (!isLFGEncounter)
            fullEncounterIndex |= completedEncounter;
    }

    // TC_LOG_DEBUG(LOG_FILTER_LFG, "UpdateEncounterState dungeonId %u fullEncounterIndex %u completedEncounters %u", dungeonId, fullEncounterIndex, completedEncounters);

    if (dungeonId && (fullEncounterIndex == completedEncounters || findCustomDungeon)) // For auto find dungeonId disable find last encounter from base, need 
    {
        auto const& players = GetPlayers();
        for (const auto& itr : players)
        {
            if (auto player2 = itr.getSource())
            {
                if (dungeonId)
                {
                    if (auto grp = player2->GetGroup())
                    {
                        if (grp->isLFGGroup())
                        {
                            sLFGMgr->FinishDungeon(grp->GetGUID(), dungeonId);
                            break;
                        }

                        if (player)
                            if (auto guild = player->GetGuild())
                                if (grp->IsGuildGroup())
                                    guild->AddGuildNews(GUILD_NEWS_DUNGEON_ENCOUNTER, player->GetGUID(), 0, encounterId);
                    }
                }
                else
                    player2->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_INSTANCE, static_cast<InstanceMap*>(this)->GetMaxPlayers());
            }
        }
    }

    if (instance)
        instance->SetCompletedEncountersMask(completedEncounters);

    sLFGMgr->SetCompletedMask(groupGuid, completedEncounters);
}

void Map::AddUpdateObject(Object* obj)
{
    std::lock_guard<std::recursive_mutex> _lock(i_objectLock);
    i_objects.emplace(obj->GetGUID());
}

void Map::RemoveUpdateObject(Object* obj)
{
    std::lock_guard<std::recursive_mutex> _lock(i_objectLock);
    i_objects.erase(obj->GetGUID());
}

void Map::AddToMapWait(Object* obj)
{
    std::lock_guard<std::recursive_mutex> _objectsAddToMap_lock(m_objectsAddToMap_lock);
    i_objectsAddToMap.emplace(obj);
}

void Map::UpdateLoop(volatile uint32 _mapID)
{
    cds::threading::Manager::attachThread();

    uint32 realCurrTime = 0;
    uint32 realPrevTime = getMSTime();

    uint32 prevSleepTime = 0;

    // TC_LOG_ERROR(LOG_FILTER_WORLDSERVER, "Map::UpdateLoop Run _mapID %u thread %u", _mapID, std::this_thread::get_id());

    while (!b_isMapStop)
    {
        if (m_worldCrashChecker) // Crashing detected, need stop map
        {
            m_Transports.clear();
            UnloadAll();
            b_isMapStop = true;
            TC_LOG_ERROR(LOG_FILTER_WORLDSERVER, "Map::UpdateLoop Crash _mapID %u thread %u", _mapID, std::this_thread::get_id());
            break;
        }

        try
        {
            m_mapLoopCounter++;
            uint32 slepp = sWorld->getIntConfig(CONFIG_INTERVAL_MAP_SESSION_UPDATE);
            if (!Instanceable() && !CanCreatedZone() && !HavePlayers())
                slepp = 1000;

            realCurrTime = getMSTime();

            uint32 diff = getMSTimeDiff(realPrevTime, realCurrTime);

            i_timer.Update(diff);
            i_timer_se.Update(diff);

            if (i_timer_se.Passed())
            {
                uint32 _s = getMSTime();
                UpdateSessions(uint32(i_timer_se.GetCurrent()));
                m_sessionTime = GetMSTimeDiffToNow(_s);

                i_timer_se.SetCurrent(0);
            }

            if (i_timer.Passed())
            {
                uint32 _s = getMSTime();
                uint32 curr = uint32(i_timer.GetCurrent());
                Update(curr);
                DelayedUpdate(curr);
                UpdateTransport(curr);
                m_updateTime = GetMSTimeDiffToNow(_s);

                i_timer.SetCurrent(0);
            }

            if (i_timer_bp.Passed())
            {
                PopulateBattlePet(uint32(i_timer_bp.GetCurrent()));
                i_timer_bp.SetCurrent(0);
            }

            realPrevTime = realCurrTime;

            if (diff <= slepp + prevSleepTime)
            {
                prevSleepTime = slepp + prevSleepTime - diff;
                std::this_thread::sleep_for(Milliseconds(prevSleepTime));
            }
            else
                prevSleepTime = 0;
        }
        catch (std::exception& e)
        {
            // sLog->outTryCatch("\n\n//==-----------------------------------------------------------------------------------------------------==//");
            sLog->outTryCatch("Exception caught in Map::UpdateLoop %s _mapID %u InstanceId %u", e.what(), _mapID, i_InstanceId);

            if (m_currentSession)
                m_currentSession->KickPlayer();
        }
        catch (...)
        {
            // sLog->outTryCatch("\n\n//==-----------------------------------------------------------------------------------------------------==//");
            sLog->outTryCatch("Exception caught in Map::UpdateLoop _mapID %u InstanceId %u", _mapID, i_InstanceId);

            if (m_currentSession)
                m_currentSession->KickPlayer();
        }
    }

    //TC_LOG_ERROR(LOG_FILTER_WORLDSERVER, "Map::UpdateLoop Stop _mapID %u thread %u", _mapID, std::this_thread::get_id());

    cds::threading::Manager::detachThread();
}

void Map::SetMapUpdateInterval()
{
    if (IsBattlegroundOrArena())
    {
        i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_PVP_MAP_UPDATE));
        i_timer.Reset();
        i_timer_obj.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_PVP_MAP_UPDATE));
        i_timer_obj.Reset();
    }
    else if (IsDungeon())
    {
        i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_INSTANCE_UPDATE));
        i_timer.Reset();
        i_timer_obj.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_INSTANCE_UPDATE));
        i_timer_obj.Reset();
    }
    else
    {
        i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_MAPUPDATE));
        i_timer.Reset();
        i_timer_obj.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_OBJECT_UPDATE));
        i_timer_obj.Reset();
    }

    i_timer_se.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_MAP_SESSION_UPDATE));
    i_timer_se.Reset();

    i_timer_op.SetInterval(1000); // OutdoorPvP timer update
    i_timer_op.Reset();

    i_timer_bp.SetInterval(30000); // WildBattlePet timer update
    i_timer_bp.Reset();
}

uint32 Map::GetUpdateTime() const
{
    return m_updateTime;
}

uint32 Map::GetSessionTime() const
{
    return m_sessionTime;
}

void Map::AddSession(WorldSessionPtr s)
{
    if (!s)
        return;

    addSessQueue.add(s);
}

void Map::AddSession_(WorldSessionPtr s)
{
    if (!s)
        return;

    SessionMap::const_iterator old = m_sessions.find(s->GetAccountId());
    if (old != m_sessions.end())
        m_sessions.erase(old);

    m_sessions[s->GetAccountId()] = s;
}

/// Find a session by its id
WorldSessionPtr Map::FindSession(uint32 id) const
{
    SessionMap::const_iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end())
        return itr->second;                                 // also can return NULL for kicked session
    return nullptr;
}

void Map::AddTransport(Transport* t)
{
    m_Transports.insert(t);
}

void Map::RemoveTransport(Transport* t)
{
    m_Transports.erase(transportHashGen(t));
}

void Map::AddStaticTransport(StaticTransport* t)
{
    m_StaticTransports[t->GetGUID()] = t;
}

void Map::RemoveStaticTransport(StaticTransport* t)
{
    m_StaticTransports.erase(t->GetGUID());
}

void Map::UpdateTransport(uint32 diff)
{
    if (b_isMapUnload)
        return;

    for (TransportHashSet::iterator i = m_Transports.begin(); i != m_Transports.end(); ++i)
        if (Transport* t = *i)
            t->Update(diff);
}

bool Map::CanCreatedZone() const
{
    return GetId() == 1220 || GetId() == 1669/* || GetId() == 1779*/;
}

bool Map::CanCreatedThread() const
{
    switch(GetId())
    {
        // case 30: // Alterac Valley
        // case 628: // Isle of Conquest
        // case 1493: // Vault of the Wardens
        // case 1477: // Halls of Valor
        // case 1466: // Darkheart Thicket
        // case 1501: // Black Rook Hold
        // case 1492: // Maw of Souls
        // case 1544: // Assault on Violet Hold
        // case 1456: // Eye of Azshara
        // case 1571: // Court of Stars
        // case 1516: // The Arcway
        // case 1458: // Neltharion's Lair
        // case 1651: // Karazhan
        // case 1520: // The Emerald Nightmare
        // case 1530: // The Nighthold
        // case 1677: // Cathedral of Eternal Night
        // case 1676: // Tomb of Sargeras
        case 1712: // Antorus, the Burning Throne
            return true;
        default:
            break;
    }
    return false;
}

bool Map::CanUnloadMap()
{
    if (IsScenario())
        return false;

    return sWorld->getBoolConfig(CONFIG_GRID_UNLOAD);
}

void Map::TerminateThread()
{
    if (threadPool)
    {
        threadPool->terminate();
        delete threadPool;
        threadPool = new ThreadPoolMap();
        threadPool->start(sWorld->getIntConfig(CONFIG_MAP_NUMTHREADS));
    }
}

void Map::AddBattlePet(Creature* creature)
{
    if (sWildBattlePetMgr->IsBattlePet(creature->GetEntry()))
        m_wildBattlePetPool[creature->GetCurrentZoneID()][creature->GetEntry()].ToBeReplaced.insert(creature);
    else if (creature->isWildBattlePet())
        sWildBattlePetMgr->EnableWildBattle(creature);
}

void Map::RemoveBattlePet(Creature* creature)
{
    if (sWildBattlePetMgr->IsBattlePet(creature->GetEntry()))
        m_wildBattlePetPool[creature->GetCurrentZoneID()][creature->GetEntry()].ToBeReplaced.erase(creature);
}

void Map::PopulateBattlePet(uint32 diff)
{
    uint32 _s = getMSTime();
    for (auto& zone : m_wildBattlePetPool)
    {
        uint16 zoneId = zone.first;
        for (auto& iter : zone.second)
        {
            uint32 entry = iter.first;
            WildPetPoolTemplate* petTemplate = sWildBattlePetMgr->GetWildPetTemplate(GetId(), zoneId, entry);
            if (!petTemplate)
                continue;

            sWildBattlePetMgr->Populate(petTemplate, &iter.second);
        }
    }
    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("Map::PopulateBattlePet mapId %u Update time - %ums diff %u", GetId(), _ms, diff);
}

void Map::DepopulateBattlePet()
{
    for (auto& zone : m_wildBattlePetPool)
        for (auto& iter : zone.second)
            sWildBattlePetMgr->Depopulate(&iter.second);
}

WildBattlePetPool* Map::GetWildBattlePetPool(Creature* creature)
{
    if (!creature)
        return nullptr;

    return &m_wildBattlePetPool[creature->GetCurrentZoneID()][creature->GetEntry()];
}

void Map::AddMaxVisible(Object* obj)
{
    if (b_isMapUnload)
        return;

    std::lock_guard<std::recursive_mutex> guard(i_MaxVisibleList_lock);
    m_MaxVisibleList.insert(obj);
}

void Map::RemoveMaxVisible(Object* obj)
{
    if (b_isMapUnload)
        return;

    std::lock_guard<std::recursive_mutex> guard(i_MaxVisibleList_lock);
    m_MaxVisibleList.erase(obj);
}
