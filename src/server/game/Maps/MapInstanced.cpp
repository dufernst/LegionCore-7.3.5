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

#include "Battleground.h"
#include "GarrisonMap.h"
#include "Group.h"
#include "LFGMgr.h"
#include "InstanceSaveMgr.h"
#include "MapInstanced.h"
#include "MapManager.h"
#include "MMapFactory.h"
#include "ObjectMgr.h"
#include "VMapFactory.h"
#include "World.h"
#include "InstanceScript.h"
#include "ScenarioMgr.h"
#include "WorldStateMgr.h"

MapInstanced::MapInstanced(uint32 id, time_t expiry) : Map(id, expiry, 0, DIFFICULTY_NORMAL)
{
    // initialize instanced maps list
    m_InstancedMaps.clear();
    m_GarrisonedMaps.clear();

    _zoneThreads.clear();
    // fill with zero
    memset(&GridMapReference, 0, MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS*sizeof(uint16));
}

void MapInstanced::InitVisibilityDistance()
{
    //initialize visibility distances for all instance copies
    if (!m_InstancedMaps.empty())
        for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
            (*i).second->InitVisibilityDistance();

    if (!m_GarrisonedMaps.empty())
        for (InstancedMaps::iterator i = m_GarrisonedMaps.begin(); i != m_GarrisonedMaps.end(); ++i)
            (*i).second->InitVisibilityDistance();
}

void MapInstanced::TerminateThread()
{
    if (!m_InstancedMaps.empty())
        for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
            (*i).second->TerminateThread();

    if (!m_GarrisonedMaps.empty())
        for (InstancedMaps::iterator i = m_GarrisonedMaps.begin(); i != m_GarrisonedMaps.end(); ++i)
            (*i).second->TerminateThread();

    Map::TerminateThread();
}

void MapInstanced::Update(const uint32 t)
{
    volatile uint32 _mapId = GetId();

    // take care of loaded GridMaps (when unused, unload it!)
    Map::Update(t);

    // update the instanced maps
    InstancedMaps::iterator i = m_InstancedMaps.begin();

    while (i != m_InstancedMaps.end())
    {
        if (Map* const instanced = i->second)
        {
            if (instanced->CanCreatedZone())
            {
                ++i;
                continue;
            }

            if (instanced->CanUnload(t))
            {
                if (!DestroyInstance(i))                             // iterator incremented
                {
                    //m_unloadTimer
                }
            }
            else
            {
                // update only here, because it may schedule some bad things before delete
                if (threadPool)
                    threadPool->schedule([instanced, t] { instanced->Update(t); });
                else
                    instanced->Update(t);
                ++i;
            }
        }
        else
            DestroyInstance(i);
    }

    // update the Garrisoned maps
    i = m_GarrisonedMaps.begin();

    while (i != m_GarrisonedMaps.end())
    {
        if (Map* const instanced = i->second)
        {
            if (instanced->CanUnload(t))
            {
                if (!DestroyGarrison(i))                             // iterator incremented
                {
                    //m_unloadTimer
                }
            }
            else
            {
                // update only here, because it may schedule some bad things before delete
                if (threadPool)
                    threadPool->schedule([instanced, t] { instanced->Update(t); });
                else
                    instanced->Update(t);
                ++i;
            }
        }
    }

    if (threadPool)
        threadPool->wait();
}

void MapInstanced::UpdateSessions(const uint32 diff)
{
    volatile uint32 _mapId = GetId();

    // Session not need thread
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        if (Map* const instanced = i->second)
            if (!instanced->IsMapUnload() && !instanced->CanCreatedZone())
                instanced->UpdateSessions(diff);

    for (InstancedMaps::iterator i = m_GarrisonedMaps.begin(); i != m_GarrisonedMaps.end(); ++i)
        if (Map* const instanced = i->second)
            if (!instanced->IsMapUnload())
                    instanced->UpdateSessions(diff);
}

void MapInstanced::DelayedUpdate(const uint32 diff)
{
    volatile uint32 _mapId = GetId();

    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
    {
        if (Map* const instanced = i->second)
        {
            if (!instanced->IsMapUnload() && !instanced->CanCreatedZone())
            {
                if (BattlegroundMap* map = instanced->ToBgMap())
                    if (Battleground* bg = map->GetBG())
                    {
                        std::lock_guard<std::recursive_mutex> _bg_lock(bg->m_bg_lock);
                        if (bg && !bg->ToBeDeleted())
                            bg->Update(diff);
                    }

                instanced->DelayedUpdate(diff);
            }
        }
    }

    for (InstancedMaps::iterator i = m_GarrisonedMaps.begin(); i != m_GarrisonedMaps.end(); ++i)
        if (Map* const instanced = i->second)
            if (!instanced->IsMapUnload())
                instanced->DelayedUpdate(diff);

    Map::DelayedUpdate(diff); // this may be removed
}

void MapInstanced::UpdateTransport(uint32 diff)
{
    volatile uint32 _mapId = GetId();

    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        if (Map* instanced = i->second)
            if (!instanced->IsMapUnload() && !instanced->CanCreatedZone())
                instanced->UpdateTransport(diff);

    for (InstancedMaps::iterator i = m_GarrisonedMaps.begin(); i != m_GarrisonedMaps.end(); ++i)
        if (Map* instanced = i->second)
            if (!instanced->IsMapUnload())
                instanced->UpdateTransport(diff);

    Map::UpdateTransport(diff); // this may be removed
}

void MapInstanced::UnloadAll()
{
    // Unload instanced maps
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        i->second->UnloadAll();

    for (InstancedMaps::iterator i = m_GarrisonedMaps.begin(); i != m_GarrisonedMaps.end(); ++i)
        i->second->UnloadAll();

    // Delete the maps only after everything is unloaded to prevent crashes
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        delete i->second;
    for (InstancedMaps::iterator i = m_GarrisonedMaps.begin(); i != m_GarrisonedMaps.end(); ++i)
        delete i->second;

    m_InstancedMaps.clear();
    m_GarrisonedMaps.clear();

    for (auto thread : _zoneThreads)
    {
        thread.second->join();
        delete thread.second;
    }
    _zoneThreads.clear();

    // Unload own grids (just dummy(placeholder) grids, neccesary to unload GridMaps!)
    Map::UnloadAll();
}

void MapInstanced::StopInstance()
{
    volatile uint32 _mapId = GetId();

    // Session not need thread
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        if (Map* const instanced = i->second)
            instanced->SetMapStop();
}

/*
- return the right instance for the object, based on its InstanceId
- create the instance if it's not created already
- the player is not actually added to the instance (only in InstanceMap::Add)
*/
Map* MapInstanced::CreateInstanceForPlayer(const uint32 mapId, Player* player)
{
    if (GetId() != mapId || !player)
        return nullptr;

    Map* map = nullptr;
    uint32 newInstanceId = 0;                       // instanceId of the resulting map

    if (IsBattlegroundOrArena())
    {
        // instantiate or find existing bg map for player
        // the instance id is set in battlegroundid
        newInstanceId = player->GetBattlegroundId();
        if (!newInstanceId)
            return nullptr;

        map = sMapMgr->FindMap(mapId, newInstanceId);
        if (!map)
        {
            if (Battleground* bg = player->GetBattleground())
                map = CreateBattleground(newInstanceId, bg);
            else
            {
                player->TeleportToBGEntryPoint();
                return nullptr;
            }
        }
    }
    else if (!IsGarrison())
    {
        Group* group = player->GetGroup();
        Difficulty diff = DIFFICULTY_NORMAL;
        InstancePlayerBind* pBind = nullptr;

        if (group)
        {
            diff = group->GetDifficultyID(GetEntry());
            if (group->GetLeaderGUID() == player->GetGUID())
                pBind = player->GetBoundInstance(GetId(), diff);
            else if (Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
            {
                pBind = leader->GetBoundInstance(GetId(), diff);

                if (GetEntry()->ExpansionID < EXPANSION_LEGION)
                {
                    InstancePlayerBind* tempBind = player->GetBoundInstance(GetId(), diff);
                    if (pBind && tempBind && pBind->save != tempBind->save)
                    {
                        uint32 allMask = pBind->save->GetCompletedEncounterMask() & tempBind->save->GetCompletedEncounterMask();
                        if (allMask == tempBind->save->GetCompletedEncounterMask())
                        {
                            player->UnbindInstance(GetId(), diff);
                            player->BindToInstance(pBind->save, pBind->perm);
                        }
                    }
                }
            }
            else
                pBind = player->GetBoundInstance(GetId(), diff);
        }
        else
        {
            diff = player->GetDifficultyID(GetEntry());
            pBind = player->GetBoundInstance(GetId(), diff);
        }

        InstanceSave* pSave = pBind ? pBind->save : nullptr;

        if (pSave && pSave->SaveIsOld() && !pSave->GetExtended())
        {
            player->UnbindInstance(GetId(), diff);
            pSave = nullptr;
            pBind = nullptr;
        }

        // the player's permanent player bind is taken into consideration first
        // then the player's group bind and finally the solo bind.
        if (!pBind || !pBind->perm || GetEntry()->ExpansionID >= EXPANSION_WARLORDS_OF_DRAENOR)
        {
            InstanceGroupBind* groupBind = nullptr;
            // use the player's difficulty setting (it may not be the same as the group's)
            if (group)
            {
                groupBind = group->GetBoundInstance(this);
                if (groupBind)
                    pSave = groupBind->save;
                if (pSave && pSave->SaveIsOld() && !pSave->GetExtended())
                {
                    group->UnbindInstance(GetId(), diff);
                    pSave = nullptr;
                }
            }
        }

        if (pSave)
        {
            // solo/perm/group
            newInstanceId = pSave->GetInstanceId();
            map = FindInstanceMap(newInstanceId);
            // it is possible that the save exists but the map doesn't
            if (!map)
                map = CreateInstance(newInstanceId, pSave, pSave->GetDifficultyID());
        }
        else
        {
            // if no instanceId via group members or instance saves is found
            // the instance will be created for the first time
            newInstanceId = sMapMgr->GenerateInstanceId();

            //Seems it is now possible, but I do not know if it should be allowed
            //ASSERT(!FindInstanceMap(NewInstanceId));
            if (const MapEntry* entry = sMapStore.LookupEntry(mapId))
                if (entry->IsScenario())
                {
                    DifficultyEntry const* difficultyEntry = sDifficultyStore.LookupEntry(diff);
                    if (!difficultyEntry || difficultyEntry->InstanceType != entry->InstanceType)
                        if (MapDifficultyEntry const* defaultDifficulty = sDB2Manager.GetDefaultMapDifficulty(entry->ID))
                            diff = Difficulty(defaultDifficulty->DifficultyID);
                }

            // Find correct difficulty
            MapDifficultyEntry const* mapDiff = sDB2Manager.GetMapDifficultyData(mapId, diff);
            if (!mapDiff)
                mapDiff = sDB2Manager.GetDefaultMapDifficulty(mapId);
            if (mapDiff)
                diff = Difficulty(mapDiff->DifficultyID);

            map = FindInstanceMap(newInstanceId);
            if (!map)
                map = CreateInstance(newInstanceId, nullptr, diff);
            else if(map->GetDifficultyID() != diff)
                map->SetSpawnMode(diff);

            std::string data;
            pSave = sInstanceSaveMgr->AddInstanceSave(GetId(), newInstanceId, diff, 0, data, 0, true);
            if (group)
            {
                group->BindToInstance(pSave, false);
                if (lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(sLFGMgr->GetDungeon(group->GetGUID())))
                    if (dungeon->dbc && dungeon->dbc->MentorItemLevel && dungeon->dbc->MentorCharLevel && dungeon->difficulty == DIFFICULTY_HEROIC) // Detect timewalk
                        map->SetLootDifficulty(DIFFICULTY_TIMEWALKING);
            }
        }
    }
    else
    {
        newInstanceId = player->GetGUIDLow();
        map = FindGarrisonMap(newInstanceId);
        if (!map)
            map = CreateGarrison(newInstanceId, player);
    }

    if (map)
        sWorldStateMgr.CreateInstanceState(mapId, newInstanceId);

    return map;
}

InstanceMap* MapInstanced::CreateInstance(uint32 InstanceId, InstanceSave* save, Difficulty difficulty)
{
    // load/create a map
    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    // make sure we have a valid map id
    const MapEntry* entry = sMapStore.LookupEntry(GetId());
    if (!entry)
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "CreateInstance: no entry for map %d", GetId());
        return nullptr;
    }
    const InstanceTemplate* iTemplate = sObjectMgr->GetInstanceTemplate(GetId());
    if (!iTemplate && !entry->IsScenario())
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "CreateInstance: no instance template for map %d", GetId());
        return nullptr;
    }

    sDB2Manager.GetDownscaledMapDifficultyData(GetId(), difficulty);

    TC_LOG_DEBUG(LOG_FILTER_MAPS, "MapInstanced::CreateInstance: %s map instance %d for %d created with difficulty %u", save ? "" : "new ", InstanceId, GetId(), difficulty);

    InstanceMap* map = new InstanceMap(GetId(), GetGridExpiry(), InstanceId, difficulty, this);
    ASSERT(map->IsDungeon());

    map->LoadRespawnTimes();

    map->CreateInstanceData(save);

    sTransportMgr->CreateInstanceTransports(map);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

GarrisonMap* MapInstanced::CreateGarrison(uint32 instanceId, Player* owner)
{
    // load/create a map
    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    GarrisonMap* map = new GarrisonMap(GetId(), GetGridExpiry(), instanceId, this, owner->GetGUID());
    ASSERT(map->IsGarrison());
    map->CreateInstanceData(nullptr);

    sTransportMgr->CreateInstanceTransports(map);

    m_GarrisonedMaps[instanceId] = map;
    return map;
}

Map* MapInstanced::CreateZoneForPlayer(const uint32 mapId, Player* player)
{
    if (GetId() != mapId || !player)
        return nullptr;

    uint32 newZoneId = GetZoneId(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
    Map* map = FindInstanceMap(newZoneId);
    if (!map)
        map = CreateZoneMap(newZoneId, player);

    return map;
}

Map* MapInstanced::FindInstanceMap(uint32 instanceId) const
{
    InstancedMaps::const_iterator i = m_InstancedMaps.find(instanceId);
    return (i == m_InstancedMaps.end() ? nullptr : i->second);
}

Map* MapInstanced::FindGarrisonMap(uint32 instanceId) const
{
    InstancedMaps::const_iterator i = m_GarrisonedMaps.find(instanceId);
    return (i == m_GarrisonedMaps.end() ? nullptr : i->second);
}

ZoneMap* MapInstanced::CreateZoneMap(uint32 zoneId, Player* player)
{
    // load/create a map
    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    ZoneMap* map = new ZoneMap(GetId(), GetGridExpiry(), zoneId, this, DIFFICULTY_NONE);
    map->LoadRespawnTimes();
    m_InstancedMaps[zoneId] = map;

    map->UpdateOutdoorPvPScript();

    _zoneThreads[zoneId] = new std::thread(&Map::UpdateLoop, map, zoneId);
    return map;
}

BattlegroundMap* MapInstanced::CreateBattleground(uint32 InstanceId, Battleground* bg)
{
    // load/create a map
    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    TC_LOG_DEBUG(LOG_FILTER_MAPS, "MapInstanced::CreateBattleground: map bg %d for %d created.", InstanceId, GetId());

    BattlegroundMap* map = new BattlegroundMap(GetId(), GetGridExpiry(), InstanceId, this, DIFFICULTY_NORMAL);
    ASSERT(map->IsBattlegroundOrArena());
    map->SetBG(bg);
    map->InitVisibilityDistance();
    bg->SetBgMap(map);

    sTransportMgr->CreateInstanceTransports(map);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

// increments the iterator after erase
bool MapInstanced::DestroyInstance(InstancedMaps::iterator &itr)
{
    Map* uMap = itr->second;
    if (!uMap)
    {
        ++itr;
        return false;
    }

    uMap->RemoveAllPlayers();
    if (uMap->HavePlayers())
    {
        ++itr;
        return false;
    }

    uMap->UnloadAll();
    // should only unload VMaps if this is the last instance and grid unloading is enabled
    if (m_InstancedMaps.size() <= 1 && CanUnloadMap())
    {
        VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(uMap->GetId());
        MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(uMap->GetId());
        // in that case, unload grids of the base map, too
        // so in the next map creation, (EnsureGridCreated actually) VMaps will be reloaded
        Map::UnloadAll();
    }

    sScenarioMgr->RemoveScenario(uMap->GetInstanceId());

    sWorldStateMgr.DeleteInstanceState(uMap->GetInstanceId());

    // erase map
    delete itr->second;
    m_InstancedMaps.erase(itr++);

    return true;
}

// increments the iterator after erase
bool MapInstanced::DestroyGarrison(InstancedMaps::iterator &itr)
{
    Map* uMap = itr->second;
    uMap->RemoveAllPlayers();
    if (uMap->HavePlayers())
    {
        ++itr;
        return false;
    }

    uMap->UnloadAll();
    // should only unload VMaps if this is the last instance and grid unloading is enabled
    if (m_GarrisonedMaps.size() <= 1 && CanUnloadMap())
    {
        VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(uMap->GetId());
        MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(uMap->GetId());
        // in that case, unload grids of the base map, too
        // so in the next map creation, (EnsureGridCreated actually) VMaps will be reloaded
        Map::UnloadAll();
    }

    sWorldStateMgr.DeleteInstanceState(uMap->GetInstanceId());

    // erase map
    delete itr->second;
    m_GarrisonedMaps.erase(itr++);

    return true;
}

void MapInstanced::AddGridMapReference(const GridCoord& p)
{
    ++GridMapReference[p.x_coord][p.y_coord];
    SetUnloadReferenceLock(GridCoord(63 - p.x_coord, 63 - p.y_coord), true);
}

void MapInstanced::RemoveGridMapReference(GridCoord const& p)
{
    --GridMapReference[p.x_coord][p.y_coord];
    if (!GridMapReference[p.x_coord][p.y_coord])
        SetUnloadReferenceLock(GridCoord(63 - p.x_coord, 63 - p.y_coord), false);
}

bool MapInstanced::CanEnter(Player* /*player*/)
{
    //ASSERT(false);
    return true;
}
