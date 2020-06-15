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

#include "CellImpl.h"
#include "Chat.h"
#include "ChatPackets.h"
#include "Config.h"
#include "Corpse.h"
#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "Group.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "InstanceScript.h"
#include "Log.h"
#include "MapInstanced.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ScenarioMgr.h"
#include "ThreadPoolMgr.hpp"
#include "World.h"
#include "WorldPacket.h"

MapManager::MapManager(): _nextInstanceId(0), _mapInfoCounter(0)
{
    _mapCount = sMapStore.GetNumRows() + 1;
    i_maps.assign(_mapCount, nullptr);
    _mapThreads.assign(_mapCount, nullptr);
    i_gridCleanUpDelay = sWorld->getIntConfig(CONFIG_INTERVAL_GRIDCLEAN);
}

MapManager::~MapManager()
{
}

void MapManager::Initialize()
{
}

void MapManager::InitializeVisibilityDistanceInfo()
{
    for (uint16 i = 0; i < _mapCount; ++i)
        if (Map* map = i_maps[i])
            map->InitVisibilityDistance();
}

MapManager* MapManager::instance()
{
    static MapManager instance;
    return &instance;
}

Map* MapManager::CreateBaseMap(uint32 id)
{
    Map* map = FindBaseMap(id);

    if (map == nullptr)
    {
        MapEntry const* entry = sMapStore.LookupEntry(id);
        ASSERT(entry);

        if (entry->Instanceable() || entry->CanCreatedZone())
            map = new MapInstanced(id, i_gridCleanUpDelay);
        else
        {
            map = new Map(id, i_gridCleanUpDelay, 0, DIFFICULTY_NONE);
            map->LoadRespawnTimes();
        }

        i_maps[id] = map;
        _mapThreads[id] = new std::thread(&Map::UpdateLoop, map, id);
    }

    ASSERT(map);
    return map;
}

Map* MapManager::FindBaseNonInstanceMap(uint32 mapId) const
{
    Map* map = FindBaseMap(mapId);
    if (map && map->Instanceable())
        return nullptr;
    return map;
}

Map* MapManager::CreateMap(uint32 id, Player* player)
{
    Map* m = CreateBaseMap(id);

    if (m && m->Instanceable())
        m = static_cast<MapInstanced*>(m)->CreateInstanceForPlayer(id, player);

    if (m && m->CanCreatedZone())
        m = static_cast<MapInstanced*>(m)->CreateZoneForPlayer(id, player);

    return m;
}

Map* MapManager::FindMap(uint32 mapid, uint32 instanceId) const
{
    Map* map = FindBaseMap(mapid);
    if (!map)
        return nullptr;

    if (map->Instanceable() || map->CanCreatedZone())
    {
        if (!map->IsGarrison())
        {
            if (auto map_ = static_cast<MapInstanced*>(map)->FindInstanceMap(instanceId))
                return map_;
        }
        else
        {
            if (auto map_ = static_cast<MapInstanced*>(map)->FindGarrisonMap(instanceId))
                return map_;
        }
    }

    if (!map->Instanceable()) // return base map else for "starting"
        instanceId = 0;

    return instanceId == 0 ? map : nullptr;
}

uint32 MapManager::GetAreaId(uint32 mapid, float x, float y, float z) const
{
    Map const* m = const_cast<MapManager*>(this)->CreateBaseMap(mapid);
    return m->GetAreaId(x, y, z);
}

uint32 MapManager::GetZoneId(uint32 mapid, float x, float y, float z) const
{
    Map const* m = const_cast<MapManager*>(this)->CreateBaseMap(mapid);
    return m->GetZoneId(x, y, z);
}

void MapManager::GetZoneAndAreaId(uint32& zoneid, uint32& areaid, uint32 mapid, float x, float y, float z)
{
    Map const* m = const_cast<MapManager*>(this)->CreateBaseMap(mapid);
    m->GetZoneAndAreaId(zoneid, areaid, x, y, z);
}

bool MapManager::CanPlayerEnter(uint32 mapid, Player* player, bool loginCheck)
{
    MapEntry const* entry = sMapStore.LookupEntry(mapid);
    if (!entry)
       return false;

    if (player->isGameMaster())
        return true;

    if (!entry->IsDungeon() || entry->IsGarrison())
        return true;

    if (loginCheck && entry->IsScenario())
        return false;

    InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(mapid);
    if (!instance && !entry->IsScenario())
        return false;

    Difficulty targetDifficulty = player->GetDifficultyID(entry);
    
    MapDifficultyEntry const* mapDiff = sDB2Manager.GetMapDifficultyData(entry->ID, targetDifficulty); //The player has a heroic mode and tries to enter into instance which has no a heroic mode
    if (!mapDiff)
    {
        // Send aborted message for dungeons
        if (entry->IsNonRaidDungeon() && !entry->IsScenario())
        {
            player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY, targetDifficulty);
            return false;
        }
            
        mapDiff = sDB2Manager.GetDownscaledMapDifficultyData(entry->ID, targetDifficulty); // attempt to downscale
    }

    if (!mapDiff)
        return false;

    auto playerConditionID = sDB2Manager.GetPlayerConditionForMapDifficulty(mapDiff->ID);
    if (playerConditionID && !sConditionMgr->IsPlayerMeetingCondition(player, playerConditionID))
    {
        player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY, targetDifficulty);
        return false;
    }

    if (!player->isAlive())
    {
        if (Corpse* corpse = player->GetCorpse())
        {
            // let enter in ghost mode in instance that connected to inner instance with corpse
            uint32 corpseMap = corpse->GetMapId();
            do
            {
                if (corpseMap == mapid)
                    break;

                InstanceTemplate const* corpseInstance = sObjectMgr->GetInstanceTemplate(corpseMap);
                corpseMap = corpseInstance ? corpseInstance->Parent : 0;
            } while (corpseMap);

            if (!corpseMap)
            {
                WorldPackets::Misc::AreaTriggerNoCorpse packet;
                player->SendDirectMessage(packet.Write());
                TC_LOG_DEBUG(LOG_FILTER_MAPS, "MAP: Player '%s' does not have a corpse in instance '%s' and cannot enter.", player->GetName(), entry->MapName->Str[sObjectMgr->GetDBCLocaleIndex()]);
                return false;
            }
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "MAP: Player '%s' has corpse in instance '%s' and can enter.", player->GetName(), entry->MapName->Str[sObjectMgr->GetDBCLocaleIndex()]);
            player->ResurrectPlayer(0.5f, false);
            player->SpawnCorpseBones();
        }
        else
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "Map::CanPlayerEnter - player '%s' is dead but does not have a corpse!", player->GetName());
    }
    
    Group* group = player->GetGroup();
    if (entry->IsRaid() && entry->ExpansionID > EXPANSION_WARLORDS_OF_DRAENOR)
    {
        // can only enter in a raid group
        if ((!group || !group->isRaidGroup()) && !sWorld->getBoolConfig(CONFIG_INSTANCE_IGNORE_RAID))
        {
            WorldPackets::Chat::ChatNotInParty packet;
            packet.SlashCmd = group ? 3 : 2; // req: 3 - raid, 2 - group
            player->SendDirectMessage(packet.Write());
            TC_LOG_DEBUG(LOG_FILTER_MAPS, "MAP: Player '%s' must be in a raid group to enter instance '%s'", player->GetName(), entry->MapName->Str[sObjectMgr->GetDBCLocaleIndex()]);
            return false;
        }
    }

    //Get instance where player's group is bound & its map
    if (group)
    {
        InstanceGroupBind* boundInstance = group->GetBoundInstance(entry);
        if (boundInstance && boundInstance->save)
        {
            if (Map* boundMap = sMapMgr->FindMap(mapid, boundInstance->save->GetInstanceId()))
                if (!loginCheck && !boundMap->CanEnter(player))
                    return false;

            if (entry->ExpansionID < EXPANSION_LEGION && !boundInstance->save->SaveIsOld())
            {
                if (InstancePlayerBind* tempBind = player->GetBoundInstance(mapid, targetDifficulty))
                {
                    uint32 allMask = boundInstance->save->GetCompletedEncounterMask() & tempBind->save->GetCompletedEncounterMask();
                    if (allMask != tempBind->save->GetCompletedEncounterMask() && !tempBind->save->SaveIsOld())
                    {
                        player->SendTransferAborted(mapid, TRANSFER_ABORT_LOCKED_TO_DIFFERENT_INSTANCE, targetDifficulty);
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        InstancePlayerBind* boundInstance = player->GetBoundInstance(mapid, targetDifficulty);
        if (boundInstance && boundInstance->save)
            if (Map* boundMap = sMapMgr->FindMap(mapid, boundInstance->save->GetInstanceId()))
                if (!loginCheck && !boundMap->CanEnter(player))
                    return false;
    }

    //Other requirements
    return player->Satisfy(sObjectMgr->GetAccessRequirement(mapid, targetDifficulty), mapid, true);
}

void MapManager::Update(uint32 /*diff*/)
{
}

void MapManager::SetGridCleanUpDelay(uint32 t)
{
    if (t < MIN_GRID_DELAY)
        i_gridCleanUpDelay = MIN_GRID_DELAY;
    else
        i_gridCleanUpDelay = t;
}

void MapManager::DoDelayedMovesAndRemoves()
{
}

bool MapManager::ExistMapAndVMap(uint32 mapid, float x, float y)
{
    GridCoord p = Trinity::ComputeGridCoord(x, y);

    int gx=63-p.x_coord;
    int gy=63-p.y_coord;

    return Map::ExistMap(mapid, gx, gy) && Map::ExistVMap(mapid, gx, gy);
}

bool MapManager::IsValidMAP(uint32 mapid, bool startUp)
{
    if (MapEntry const* mEntry = sMapStore.LookupEntry(mapid))
    {
        if (startUp || mEntry->IsGarrison())
            return true;

        if (mEntry->IsDungeon() && !mEntry->IsScenario())
            return sObjectMgr->GetInstanceTemplate(mapid);

        return true;
    }
    return false;
    // TODO: add check for battleground template
}

bool MapManager::IsValidMapCoord(uint32 mapid, float x, float y)
{
    return IsValidMAP(mapid, false) && Trinity::IsValidMapCoord(x, y);
}

bool MapManager::IsValidMapCoord(uint32 mapid, float x, float y, float z)
{
    return IsValidMAP(mapid, false) && Trinity::IsValidMapCoord(x, y, z);
}

bool MapManager::IsValidMapCoord(uint32 mapid, float x, float y, float z, float o)
{
    return IsValidMAP(mapid, false) && Trinity::IsValidMapCoord(x, y, z, o);
}

bool MapManager::IsValidMapCoord(WorldLocation const& loc)
{
    return IsValidMapCoord(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation());
}

void MapManager::SetMapUpdateInterval(uint32 t)
{
    if (t < MIN_MAP_UPDATE_DELAY)
        t = MIN_MAP_UPDATE_DELAY;

    for (uint16 i = 0; i < _mapCount; ++i)
    {
        if (Map* map = i_maps[i])
            map->SetMapUpdateInterval();
    }
}

void MapManager::UnloadAll()
{
    sInstanceSaveMgr->UnloadAll();

    // Stop map befor unlooad map
    for (uint16 i = 0; i < _mapCount; ++i)
    {
        if (Map* map = i_maps[i])
        {
            if (map->CanCreatedZone())
                if (MapInstanced* inst = static_cast<MapInstanced*>(map))
                    inst->StopInstance();

            map->SetMapStop();
            map->m_Transports.clear();
        }
    }

    // Wait when map is stop update
    std::this_thread::sleep_for(Milliseconds(1000));

    for (uint16 i = 0; i < _mapCount; ++i)
    {
        if (Map* map = i_maps[i])
        {
            i_maps[i] = nullptr;
            map->UnloadAll();
            delete map;
        }
    }

    for (auto* thread : _mapThreads)
    {
        if (thread)
            thread->join();
        delete thread;
    }

    sGuildMgr->UnloadAll();
    sScenarioMgr->UnloadAll();
}

uint32 MapManager::GetNumInstances()
{
    uint32 ret = 0;
    for (uint16 i = 0; i < _mapCount; ++i)
    {
        if (Map* map = i_maps[i])
        {
            if (!map->Instanceable())
                continue;

            auto& maps = static_cast<MapInstanced*>(map)->GetInstancedMaps();
            for (auto& itr : maps)
                if (itr.second->IsDungeon())
                    ret++;
        }
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInInstances()
{
    uint32 ret = 0;
    for (uint16 i = 0; i < _mapCount; ++i)
    {
        if (Map* map = i_maps[i])
        {
            if (!map->Instanceable())
                continue;

            auto& maps = static_cast<MapInstanced*>(map)->GetInstancedMaps();
            for (auto& itr : maps)
                if (itr.second->IsDungeon())
                    ret += static_cast<InstanceMap*>(itr.second)->GetPlayerCount();
        }
    }
    return ret;
}

void MapManager::InitInstanceIds()
{
    _nextInstanceId = 1;

    if (QueryResult result = CharacterDatabase.Query("SELECT MAX(instance) FROM character_instance"))
    {
        uint32 instanceId = (*result)[0].GetUInt32();
        if (instanceId > _nextInstanceId)
            _nextInstanceId = instanceId + 1;
    }

    if (QueryResult result = CharacterDatabase.Query("SELECT MAX(instance) FROM group_instance"))
    {
        uint32 instanceId = (*result)[0].GetUInt32();
        if (instanceId > _nextInstanceId)
            _nextInstanceId = instanceId + 1;
    }
}

uint32 MapManager::GenerateInstanceId()
{
    return _nextInstanceId++;
}

void MapManager::FindSessionInAllMaps(uint32 accId, ChatHandler* handler)
{
    if (!handler)
        return;

    for (uint16 i = 0; i < _mapCount; ++i)
    {
        if (Map* map = i_maps[i])
        {
            if (!map->Instanceable())
            {
                if (map->FindSession(accId))
                    handler->PSendSysMessage("Session find for accountID %u in map %u", accId, map->GetId());
                continue;
            }

            auto& maps = static_cast<MapInstanced*>(map)->GetInstancedMaps();
            for (auto& itr : maps)
                if (itr.second->FindSession(accId))
                    handler->PSendSysMessage("Session find for accountID %u in map %u InstanceId %u", accId, itr.second->GetId(), itr.second->GetInstanceId());
        }
    }
}

void MapManager::LogInfoAllMaps()
{
    if (_mapInfoCounter < 10) // One of 10 minuts
    {
        _mapInfoCounter++;
        return;
    }
    _mapInfoCounter = 0;

    sLog->outMapInfo("LogInfoAllMaps NumInstances %u NumPlayersInInstances %u.", GetNumInstances(), GetNumPlayersInInstances());
    sLog->outMapInfo("LogInfoAllMaps Size AT %i Conversation %i Corpse %i Creature %i DO %i EO %i GO %i Player %i",
    HashMapHolder<AreaTrigger>::_size, HashMapHolder<Conversation>::_size, HashMapHolder<Corpse>::_size, HashMapHolder<Creature>::_size, HashMapHolder<DynamicObject>::_size,
    HashMapHolder<EventObject>::_size, HashMapHolder<GameObject>::_size, HashMapHolder<Player>::_size);

    sLog->outMapInfo("LogInfoAllMaps AreaTrigger %i Conversation %i Corpse %i Creature %i DynamicObject %i EventObject %i GameObject %i Item %i Pet %i Player %i Transport %i Vehicle %i LootObject %i Scenario %i Spell %i",
    objectCountInWorld[uint8(HighGuid::AreaTrigger)], objectCountInWorld[uint8(HighGuid::Conversation)], objectCountInWorld[uint8(HighGuid::Corpse)], objectCountInWorld[uint8(HighGuid::Creature)], objectCountInWorld[uint8(HighGuid::DynamicObject)],
    objectCountInWorld[uint8(HighGuid::EventObject)], objectCountInWorld[uint8(HighGuid::GameObject)], objectCountInWorld[uint8(HighGuid::Item)], objectCountInWorld[uint8(HighGuid::Pet)], objectCountInWorld[uint8(HighGuid::Player)],
    objectCountInWorld[uint8(HighGuid::Transport)], objectCountInWorld[uint8(HighGuid::Vehicle)], objectCountInWorld[uint8(HighGuid::LootObject)], objectCountInWorld[uint8(HighGuid::Scenario)], objectCountInWorld[uint8(HighGuid::Spell)]);

    for (uint32 i = 0; i < creatureCountInWorld.size(); ++i)
        if (creatureCountInWorld[i] > 10000)
            sLog->outMapInfo("LogInfoAllMaps creatureCountInWorld Entry %u Count %u.", i, creatureCountInWorld[i]);

    for (uint32 i = 0; i < spellCountInWorld.size(); ++i)
        if (spellCountInWorld[i] > 10000)
            sLog->outMapInfo("LogInfoAllMaps spellCountInWorld SpellID %u Count %u.", i, spellCountInWorld[i]);

    for (uint16 i = 0; i < 10000; ++i)
        if (creatureCountInArea[i] > 10000)
            sLog->outMapInfo("LogInfoAllMaps creatureCountInArea AreaID %u Count %u.", i, creatureCountInArea[i]);

    for (uint16 i = 0; i < _mapCount; ++i)
    {
        if (Map* map = i_maps[i])
        {
            uint32 worldObjectCount = map->GetAllWorldObjectOnMap().size();
            if (!map->Instanceable())
            {
                // sLog->outMapInfo("LogInfoAllMaps mapId %u worldObjectCount: %u.", i, worldObjectCount);
                continue;
            }

            auto& maps = static_cast<MapInstanced*>(map)->GetInstancedMaps();
            for (auto& itr : maps)
                if (Map* instance = itr.second)
                    worldObjectCount += instance->GetAllWorldObjectOnMap().size();

            if (maps.size() > 10) // Only actual instance
                sLog->outMapInfo("LogInfoAllMaps mapId %u instanceCount %u worldObjectCount: %u.", i, maps.size(), worldObjectCount);
        }
    }
}

void MapManager::SetUnloadGarrison(uint32 lowGuid)
{
    if (Map* map = FindMap(1152, lowGuid))
        if (InstanceMap* instance = map->ToInstanceMap())
            instance->Reset(INSTANCE_RESET_GLOBAL);
    if (Map* map = FindMap(1153, lowGuid))
        if (InstanceMap* instance = map->ToInstanceMap())
            instance->Reset(INSTANCE_RESET_GLOBAL);
    if (Map* map = FindMap(1154, lowGuid))
        if (InstanceMap* instance = map->ToInstanceMap())
            instance->Reset(INSTANCE_RESET_GLOBAL);
    if (Map* map = FindMap(1158, lowGuid))
        if (InstanceMap* instance = map->ToInstanceMap())
            instance->Reset(INSTANCE_RESET_GLOBAL);
    if (Map* map = FindMap(1159, lowGuid))
        if (InstanceMap* instance = map->ToInstanceMap())
            instance->Reset(INSTANCE_RESET_GLOBAL);
    if (Map* map = FindMap(1160, lowGuid))
        if (InstanceMap* instance = map->ToInstanceMap())
            instance->Reset(INSTANCE_RESET_GLOBAL);
}
