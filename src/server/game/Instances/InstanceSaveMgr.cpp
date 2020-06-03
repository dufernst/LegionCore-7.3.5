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

#include "Common.h"
#include "Player.h"
#include "GridNotifiers.h"
#include "Log.h"
#include "CellImpl.h"
#include "Map.h"
#include "MapManager.h"
#include "MapInstanced.h"
#include "InstanceSaveMgr.h"
#include <utility>
#include "Timer.h"
#include "GridNotifiersImpl.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Group.h"
#include "InstanceScript.h"
#include "ScenarioMgr.h"

InstanceSaveManager::InstanceSaveManager(): lock_instLists(false)
{
}

InstanceSaveManager::~InstanceSaveManager()
{
    // it is undefined whether this or objectmgr will be unloaded first
    // so we must be prepared for both cases
    lock_instLists = true;
    for (auto & itr : m_instanceSaveById)
    {
        InstanceSave* save = itr.second;

        for (InstanceSave::PlayerListType::iterator itr2 = save->m_playerList.begin(), next = itr2; itr2 != save->m_playerList.end(); itr2 = next)
        {
            ++next;
            (*itr2)->UnbindInstance(save->GetMapId(), save->GetDifficultyID(), true);
        }
        save->m_playerList.clear();

        for (InstanceSave::GroupListType::iterator itr2 = save->m_groupList.begin(), next = itr2; itr2 != save->m_groupList.end(); itr2 = next)
        {
            ++next;
            (*itr2)->UnbindInstance(save->GetMapId(), save->GetDifficultyID(), true);
        }
        save->m_groupList.clear();
        delete save;
    }
}

InstanceSaveManager* InstanceSaveManager::instance()
{
    static InstanceSaveManager instance;
    return &instance;
}

void InstanceSaveManager::UnloadAll()
{
    // it is undefined whether this or objectmgr will be unloaded first
    // so we must be prepared for both cases
    lock_instLists = true;
    for (auto & itr : m_instanceSaveById)
    {
        InstanceSave* save = itr.second;

        for (InstanceSave::PlayerListType::iterator itr2 = save->m_playerList.begin(), next = itr2; itr2 != save->m_playerList.end(); itr2 = next)
        {
            ++next;
            (*itr2)->UnbindInstance(save->GetMapId(), save->GetDifficultyID(), true);
        }
        save->m_playerList.clear();

        for (InstanceSave::GroupListType::iterator itr2 = save->m_groupList.begin(), next = itr2; itr2 != save->m_groupList.end(); itr2 = next)
        {
            ++next;
            (*itr2)->UnbindInstance(save->GetMapId(), save->GetDifficultyID(), true);
        }
        save->m_groupList.clear();
        delete save;
    }
    _instanceSaveLock.lock();
    m_instanceSaveById.clear();
    _instanceSaveLock.unlock();
}

/*
- adding instance into manager
- called from InstanceMap::Add, _LoadBoundInstances, LoadGroups
*/
InstanceSave* InstanceSaveManager::AddInstanceSave(uint32 mapId, uint32 instanceId, Difficulty difficulty, uint32 completedEncounter, std::string data, time_t resetTime, bool canReset, bool load)
{
    if (InstanceSave* old_save = GetInstanceSave(instanceId))
        return old_save;

    const MapEntry* entry = sMapStore.LookupEntry(mapId);
    if (!entry)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "InstanceSaveManager::AddInstanceSave: wrong mapid = %d, instanceid = %d!", mapId, instanceId);
        return nullptr;
    }

    if (instanceId == 0)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "InstanceSaveManager::AddInstanceSave: mapid = %d, wrong instanceid = %d!", mapId, instanceId);
        return nullptr;
    }

    DifficultyEntry const* difficultyEntry = sDifficultyStore.LookupEntry(difficulty);
    if (!difficultyEntry || difficultyEntry->InstanceType != entry->InstanceType)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "InstanceSaveManager::AddInstanceSave: mapid = %d, instanceid = %d, wrong dificalty %u!", mapId, instanceId, difficulty);
        return nullptr;
    }

    // initialize reset time
    // for normal instances if no creatures are killed the instance will reset in two hours
    if (entry->InstanceType != MAP_RAID && difficulty <= DIFFICULTY_NORMAL)
    {
        time_t _resetTime = time(nullptr) + 2 * HOUR;
        // normally this will be removed soon after in InstanceMap::Add, prevent error
        ScheduleReset(true, _resetTime, InstResetEvent(0, mapId, difficulty, instanceId));
    }

    TC_LOG_DEBUG(LOG_FILTER_MAPS, "InstanceSaveManager::AddInstanceSave: mapid = %d, instanceid = %d", mapId, instanceId);

    if (!resetTime)
        if (MapDifficultyEntry const* mapDiff = sDB2Manager.GetMapDifficultyData(mapId, difficulty))
            resetTime = sWorld->getInstanceResetTime(mapDiff->GetRaidDuration());

    _instanceSaveLock.lock();
    InstanceSave* save = new InstanceSave(mapId, instanceId, difficulty, completedEncounter, std::move(data), resetTime, canReset);
    m_instanceSaveById[instanceId] = save;
    _instanceSaveLock.unlock();
    return save;
}

InstanceSave* InstanceSaveManager::GetInstanceSave(uint32 InstanceId)
{
    return Trinity::Containers::MapGetValuePtr(m_instanceSaveById, InstanceId);
}

void InstanceSaveManager::DeleteInstanceFromDB(uint32 instanceid)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_BY_INSTANCE);
    stmt->setUInt32(0, instanceid);
    CharacterDatabase.DirectExecute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_INSTANCE_BY_INSTANCE);
    stmt->setUInt32(0, instanceid);
    CharacterDatabase.DirectExecute(stmt);
    // Respawn times should be deleted only when the map gets unloaded
}

void InstanceSaveManager::RemoveInstanceSave(uint32 InstanceId)
{
    InstanceSaveHashMap::iterator itr = m_instanceSaveById.find(InstanceId);
    if (itr != m_instanceSaveById.end())
    {
        _instanceSaveLock.lock();
        itr->second->SetToDelete(true);
        m_instanceSaveById.erase(itr);
        _instanceSaveLock.unlock();
    }
}

void InstanceSaveManager::UnloadInstanceSave(uint32 InstanceId)
{
    if (InstanceSave* save = GetInstanceSave(InstanceId))
    {
        save->UnloadIfEmpty();
        if (save->m_toDelete)
            delete save;
    }
}

InstanceSave::InstanceSave(uint16 MapId, uint32 InstanceId, Difficulty difficulty, uint32 completedEncounter, std::string data, time_t resetTime, bool canReset)
: m_instanceid(InstanceId), m_mapid(MapId), m_difficulty(difficulty), m_canReset(canReset), m_toDelete(false),
m_perm(false), m_extended(false), m_completedEncounter(completedEncounter), m_data(std::move(data)), m_resetTime(resetTime)
{
    m_canBeSave = difficulty != DIFFICULTY_LFR && difficulty != DIFFICULTY_HC_SCENARIO && difficulty != DIFFICULTY_N_SCENARIO && difficulty != DIFFICULTY_LFR_RAID;
}

InstanceSave::~InstanceSave()
{
    // the players and groups must be unbound before deleting the save
    ASSERT(m_playerList.empty() && m_groupList.empty());
}

/*
    Called from AddInstanceSave
*/
void InstanceSave::SaveToDB()
{
    // save instance data too
    if (Map* map = sMapMgr->FindMap(GetMapId(), m_instanceid))
    {
        ASSERT(map->IsDungeon());
        if (InstanceScript* instanceScript = dynamic_cast<InstanceMap*>(map)->GetInstanceScript())
        {
            m_data = instanceScript->GetSaveData();
            m_completedEncounter = instanceScript->GetCompletedEncounterMask();
        }
    }
}

// to cache or not to cache, that is the question
InstanceTemplate const* InstanceSave::GetTemplate()
{
    return sObjectMgr->GetInstanceTemplate(m_mapid);
}

MapEntry const* InstanceSave::GetMapEntry()
{
    return sMapStore.LookupEntry(m_mapid);
}

void InstanceSave::AddPlayer(Player* player)
{
    _playerListLock.lock();
    m_playerList.push_back(player);
    _playerListLock.unlock();
}

bool InstanceSave::RemovePlayer(Player* player)
{
    _playerListLock.lock();
    m_playerList.remove(player);
    bool isStillValid = UnloadIfEmpty();
    _playerListLock.unlock();

    //delete here if needed, after releasing the lock
    if (m_toDelete)
        delete this;

    return isStillValid;
}

void InstanceSave::AddGroup(Group* group)
{
    _groupListLock.lock();
    m_groupList.push_back(group);
    _groupListLock.unlock();
}

bool InstanceSave::RemoveGroup(Group* group)
{
    _groupListLock.lock();
    m_groupList.remove(group);
    _groupListLock.unlock();
    return UnloadIfEmpty();
}

void InstanceSave::DeleteFromDB()
{
    InstanceSaveManager::DeleteInstanceFromDB(GetInstanceId());
}

/* true if the instance save is still valid */
bool InstanceSave::UnloadIfEmpty()
{
    if (m_playerList.empty() && m_groupList.empty())
    {
        // don't remove the save if there are still players inside the map
        if (Map* map = sMapMgr->FindMap(GetMapId(), GetInstanceId()))
            if (map->HavePlayers())
                return true;

        if (!sInstanceSaveMgr->lock_instLists)
            sInstanceSaveMgr->RemoveInstanceSave(GetInstanceId());

        return false;
    }
    return true;
}

void InstanceSave::SetToDelete(bool toDelete)
{
    m_toDelete = toDelete;
}

InstanceSaveManager::InstResetEvent::InstResetEvent(): difficulty(DIFFICULTY_NORMAL), mapid(0), instanceId(0), type(0)
{
}

InstanceSaveManager::InstResetEvent::InstResetEvent(uint8 t, uint32 _mapid, Difficulty d, uint16 _instanceid): difficulty(d), mapid(_mapid), instanceId(_instanceid), type(t)
{
}

bool InstanceSaveManager::InstResetEvent::operator==(const InstResetEvent& e) const
{
    return e.instanceId == instanceId;
}

void InstanceSaveManager::LoadInstances()
{
    uint32 oldMSTime = getMSTime();

    // Delete invalid character_instance and group_instance references
    CharacterDatabase.DirectExecute("DELETE ci.* FROM character_instance AS ci LEFT JOIN characters AS c ON ci.guid = c.guid WHERE c.guid IS NULL");
    CharacterDatabase.DirectExecute("DELETE gi.* FROM group_instance     AS gi LEFT JOIN groups     AS g ON gi.guid = g.guid WHERE g.guid IS NULL");

    // Delete invalid instance references
    // CharacterDatabase.DirectExecute("DELETE i.* FROM instance AS i LEFT JOIN character_instance AS ci ON i.id = ci.instance LEFT JOIN group_instance AS gi ON i.id = gi.instance WHERE ci.guid IS NULL AND gi.guid IS NULL");

    // Delete invalid references to instance
    CharacterDatabase.DirectExecute("DELETE FROM creature_respawn WHERE instanceId > 0 AND instanceId NOT IN (SELECT instance FROM character_instance)");
    CharacterDatabase.DirectExecute("DELETE FROM gameobject_respawn WHERE instanceId > 0 AND instanceId NOT IN (SELECT instance FROM character_instance)");
    // CharacterDatabase.DirectExecute("DELETE tmp.* FROM character_instance AS tmp LEFT JOIN instance ON tmp.instance = instance.id WHERE tmp.instance > 0 AND instance.id IS NULL");
    // CharacterDatabase.DirectExecute("DELETE tmp.* FROM group_instance     AS tmp LEFT JOIN instance ON tmp.instance = instance.id WHERE tmp.instance > 0 AND instance.id IS NULL");

    // Clean invalid references to instance
    CharacterDatabase.DirectExecute("UPDATE corpse SET instanceId = 0 WHERE instanceId > 0 AND instanceId NOT IN (SELECT instance FROM character_instance)");
    CharacterDatabase.DirectExecute("UPDATE characters AS tmp LEFT JOIN character_instance ON tmp.instance_id = character_instance.instance SET tmp.instance_id = 0 WHERE tmp.instance_id > 0 AND character_instance.instance IS NULL");

    // Initialize instance id storage (Needs to be done after the trash has been clean out)
    sMapMgr->InitInstanceIds();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded instances in %u ms", GetMSTimeDiffToNow(oldMSTime));

}

void InstanceSaveManager::ScheduleReset(bool add, time_t time, InstResetEvent event)
{
    _resetTimeLock.lock();
    if (!add)
    {
        // find the event in the queue and remove it
        ResetTimeQueue::iterator itr;
        std::pair<ResetTimeQueue::iterator, ResetTimeQueue::iterator> range = m_resetTimeQueue.equal_range(time);
        for (itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second == event)
            {
                m_resetTimeQueue.erase(itr);
                return;
            }
        }

        // in case the reset time changed (should happen very rarely), we search the whole queue
        if (itr == range.second)
        {
            for (itr = m_resetTimeQueue.begin(); itr != m_resetTimeQueue.end(); ++itr)
            {
                if (itr->second == event)
                {
                    m_resetTimeQueue.erase(itr);
                    return;
                }
            }

            if (itr == m_resetTimeQueue.end())
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "InstanceSaveManager::ScheduleReset: cannot cancel the reset, the event(%d, %d, %d) was not found!", event.type, event.mapid, event.instanceId);
        }
    }
    else
        m_resetTimeQueue.insert(std::make_pair(time, event));
    _resetTimeLock.unlock();
}

void InstanceSaveManager::Update()
{
    time_t now = time(nullptr);

    _resetTimeLock.lock();
    while (!m_resetTimeQueue.empty())
    {
        time_t t = m_resetTimeQueue.begin()->first;
        if (t >= now)
            break;

        InstResetEvent &event = m_resetTimeQueue.begin()->second;
        if (event.type == 0)
        {
            // for individual normal instances, max creature respawn + X hours
            _ResetInstance(event.mapid, event.instanceId);
            m_resetTimeQueue.erase(m_resetTimeQueue.begin());
        }
    }
    _resetTimeLock.unlock();
}

void InstanceSaveManager::_ResetSave(InstanceSaveHashMap::iterator &itr)
{
    // TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "InstanceSaveManager::_ResetSave GetMapId %u GetDifficultyID %u GetResetTime %u", itr->second->GetMapId(), itr->second->GetDifficultyID(), itr->second->GetResetTime());

    // unbind all players bound to the instance
    // do not allow UnbindInstance to automatically unload the InstanceSaves
    lock_instLists = true;

    InstanceSave::PlayerListType &pList = itr->second->m_playerList;
    while (!pList.empty())
    {
        Player* player = *(pList.begin());
        player->UnbindInstance(itr->second->GetMapId(), itr->second->GetDifficultyID(), true);
    }

    InstanceSave::GroupListType &gList = itr->second->m_groupList;
    while (!gList.empty())
    {
        Group* group = *(gList.begin());
        group->UnbindInstance(itr->second->GetMapId(), itr->second->GetDifficultyID(), true);
    }

    _instanceSaveLock.lock();
    delete itr->second;
    m_instanceSaveById.erase(itr++);
    _instanceSaveLock.unlock();

    lock_instLists = false;
}

void InstanceSaveManager::_ResetInstance(uint32 mapid, uint32 instanceId)
{
    // TC_LOG_DEBUG(LOG_FILTER_SERVER_LOADING, "InstanceSaveMgr::_ResetInstance mapid %u, instanceId %u", mapid, instanceId);

    Map const* map = sMapMgr->CreateBaseMap(mapid);
    if (!map->Instanceable())
        return;

    InstanceSaveHashMap::iterator itr = m_instanceSaveById.find(instanceId);
    if (itr != m_instanceSaveById.end())
        _ResetSave(itr);

    DeleteInstanceFromDB(instanceId);                       // even if save not loaded

    Map* iMap = ((MapInstanced*)map)->FindInstanceMap(instanceId);

    if (iMap && iMap->IsDungeon())
        dynamic_cast<InstanceMap*>(iMap)->Reset(INSTANCE_RESET_RESPAWN_DELAY);

    if (iMap)
        iMap->DeleteRespawnTimes();
    else
        Map::DeleteRespawnTimesInDB(mapid, instanceId);
}

void InstanceSaveManager::ResetOrWarnAll(uint32 mapid, Difficulty difficulty, SQLTransaction& trans)
{
    // global reset for all instances of the given map
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry || !mapEntry->Instanceable() || difficulty == DIFFICULTY_LFR || difficulty == DIFFICULTY_HC_SCENARIO || difficulty == DIFFICULTY_N_SCENARIO || difficulty == DIFFICULTY_LFR_RAID)
        return;

    // remove all binds to instances of the given map
    for (InstanceSaveHashMap::iterator itr = m_instanceSaveById.begin(); itr != m_instanceSaveById.end();)
    {
        if (itr->second && itr->second->GetMapId() == mapid && itr->second->GetDifficultyID() == difficulty)
        {
            if (itr->second->GetExtended())
            {
                itr->second->SetExtended(false);
                ++itr;
            }
            else if ((itr->second->GetResetTime() + MONTH) <= time(nullptr))
                _ResetSave(itr);
            else
                ++itr;

            // TC_LOG_DEBUG(LOG_FILTER_SERVER_LOADING, "InstanceSaveMgr::ResetOrWarnAll mapid %u, difficulty %u time %u GetResetTime %u", mapid, difficulty, time(NULL), (itr->second->GetResetTime() + MONTH));
        }
        else
            ++itr;
    }

    // delete them from the DB, even if not loaded
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_INSTANCE_BY_MAP_DIFF);
    stmt->setUInt16(0, uint16(mapid));
    stmt->setUInt8(1, uint8(difficulty));
    stmt->setUInt32(2, time(nullptr));
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_BY_MAP_DIFF);
    stmt->setUInt16(0, uint16(mapid));
    stmt->setUInt8(1, uint8(difficulty));
    trans->Append(stmt);

    uint32 resetTime = 0;
    if (MapDifficultyEntry const* mapDiff = sDB2Manager.GetMapDifficultyData(mapid, difficulty))
        resetTime = sWorld->getInstanceResetTime(mapDiff->GetRaidDuration());

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_INSTANCE_EXTENDED);
    stmt->setUInt32(0, resetTime);
    stmt->setUInt16(1, uint16(mapid));
    stmt->setUInt8(2, uint8(difficulty));
    trans->Append(stmt);

    // note: this isn't fast but it's meant to be executed very rarely
    Map const* map = sMapMgr->CreateBaseMap(mapid);          // _not_ include difficulty
    InstancedMaps &instMaps = ((MapInstanced*)map)->GetInstancedMaps();

    for (auto & instMap : instMaps)
    {
        Map* map2 = instMap.second;
        if (!map2->IsDungeon())
            continue;

        dynamic_cast<InstanceMap*>(map2)->Reset(INSTANCE_RESET_GLOBAL);
    }

    // TODO: delete creature/gameobject respawn times even if the maps are not loaded
}

uint32 InstanceSaveManager::GetNumBoundPlayersTotal()
{
    uint32 ret = 0;
    for (auto & itr : m_instanceSaveById)
        ret += itr.second->GetPlayerCount();

    return ret;
}

uint32 InstanceSaveManager::GetNumBoundGroupsTotal()
{
    uint32 ret = 0;
    for (auto & itr : m_instanceSaveById)
        ret += itr.second->GetGroupCount();

    return ret;
}
