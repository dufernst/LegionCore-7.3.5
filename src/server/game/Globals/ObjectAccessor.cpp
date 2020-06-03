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

#include "ObjectAccessor.h"
#include "ObjectMgr.h"

#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "EventObject.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include "Item.h"
#include "Corpse.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "Map.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "ObjectDefines.h"
#include "MapInstanced.h"
#include "World.h"
#include "ThreadPoolMgr.hpp"


ObjectAccessor::ObjectAccessor()
{
}

ObjectAccessor::~ObjectAccessor()
{
}

WorldObject* ObjectAccessor::GetWorldObject(WorldObject const& p, ObjectGuid guid)
{
    switch (guid.GetHigh())
    {
        case HighGuid::Player:        return GetPlayer(p, guid);
        case HighGuid::Transport:
        case HighGuid::GameObject:    return GetGameObject(p, guid);
        case HighGuid::Vehicle:
        case HighGuid::Creature:      return GetCreature(p, guid);
        case HighGuid::Pet:           return GetPet(p, guid);
        case HighGuid::DynamicObject: return GetDynamicObject(p, guid);
        case HighGuid::AreaTrigger:   return GetAreaTrigger(p, guid);
        case HighGuid::Corpse:        return GetCorpse(p, guid);
        case HighGuid::Conversation:  return GetConversation(p, guid);
        case HighGuid::EventObject:   return GetEventObject(p, guid);
        default:                      return nullptr;
    }
}

Object* ObjectAccessor::GetObject(Map* map, ObjectGuid guid)
{
    switch (guid.GetHigh())
    {
        case HighGuid::Player:        return GetObjectInMap(guid, map, static_cast<Player*>(nullptr));
        case HighGuid::Transport:
        case HighGuid::GameObject:    return GetObjectInMap(guid, map, static_cast<GameObject*>(nullptr));
        case HighGuid::Vehicle:
        case HighGuid::Creature:      return GetObjectInMap(guid, map, static_cast<Creature*>(nullptr));
        case HighGuid::Pet:           return GetObjectInMap(guid, map, static_cast<Pet*>(nullptr));
        case HighGuid::DynamicObject: return GetObjectInMap(guid, map, static_cast<DynamicObject*>(nullptr));
        case HighGuid::AreaTrigger:   return GetObjectInMap(guid, map, static_cast<AreaTrigger*>(nullptr));
        case HighGuid::Corpse:        return GetObjectInMap(guid, map, static_cast<Corpse*>(nullptr));
        case HighGuid::Conversation:  return GetObjectInMap(guid, map, static_cast<Conversation*>(nullptr));
        case HighGuid::EventObject:   return GetObjectInMap(guid, map, static_cast<EventObject*>(nullptr));
        default:                      return nullptr;
    }
}

void ObjectAccessor::SetGuidSize(HighGuid type, uint64 size)
{
    switch (type)
    {
        case HighGuid::Player:
            HashMapHolder<Player>::SetSize(size);
            break;
        case HighGuid::Transport:
        case HighGuid::GameObject:
            HashMapHolder<GameObject>::SetSize(size);
            break;
        case HighGuid::Vehicle:
            HashMapHolder<Vehicle>::SetSize(size);
            break;
        case HighGuid::Creature:
            HashMapHolder<Creature>::SetSize(size);
            break;
        case HighGuid::Pet:
            HashMapHolder<Pet>::SetSize(size);
            break;
        case HighGuid::DynamicObject:
            HashMapHolder<DynamicObject>::SetSize(size);
            break;
        case HighGuid::AreaTrigger:
            HashMapHolder<AreaTrigger>::SetSize(size);
            break;
        case HighGuid::Corpse:
            HashMapHolder<Corpse>::SetSize(size);
            break;
        case HighGuid::Conversation:
            HashMapHolder<Conversation>::SetSize(size);
            break;
        case HighGuid::EventObject:
            HashMapHolder<EventObject>::SetSize(size);
            break;
        default:
            break;
    }
}

Object* ObjectAccessor::GetObjectByTypeMask(WorldObject const& p, ObjectGuid guid, uint32 typemask)
{
    switch (guid.GetHigh())
    {
        case HighGuid::Item:
            if (typemask & TYPEMASK_ITEM && p.IsPlayer())
                return static_cast<Player const&>(p).GetItemByGuid(guid);
            break;
        case HighGuid::Player:
            if (typemask & TYPEMASK_PLAYER)
                return GetPlayer(p, guid);
            break;
        case HighGuid::Transport:
        case HighGuid::GameObject:
            if (typemask & TYPEMASK_GAMEOBJECT)
                return GetGameObject(p, guid);
            break;
        case HighGuid::Creature:
        case HighGuid::Vehicle:
            if (typemask & TYPEMASK_UNIT)
                return GetCreature(p, guid);
            break;
        case HighGuid::Pet:
            if (typemask & TYPEMASK_UNIT)
                return GetPet(p, guid);
            break;
        case HighGuid::DynamicObject:
            if (typemask & TYPEMASK_DYNAMICOBJECT)
                return GetDynamicObject(p, guid);
            break;
        case HighGuid::AreaTrigger:
            if (typemask & TYPEMASK_AREATRIGGER)
                return GetAreaTrigger(p, guid);
        case HighGuid::Conversation:
            if (typemask & TYPEMASK_CONVERSATION)
                return GetConversation(p, guid);
        case HighGuid::EventObject:
            if (typemask & TYPEMASK_EVENTOBJECT)
                return GetEventObject(p, guid);
        case HighGuid::Corpse:
            break;
        default:
            break;
    }

    return nullptr;
}

Corpse* ObjectAccessor::GetCorpse(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<Corpse*>(nullptr));
}

GameObject* ObjectAccessor::GetGameObject(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<GameObject*>(nullptr));
}

DynamicObject* ObjectAccessor::GetDynamicObject(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<DynamicObject*>(nullptr));
}

AreaTrigger* ObjectAccessor::GetAreaTrigger(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<AreaTrigger*>(nullptr));
}

Conversation* ObjectAccessor::GetConversation(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<Conversation*>(nullptr));
}

EventObject* ObjectAccessor::GetEventObject(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<EventObject*>(nullptr));
}

Unit* ObjectAccessor::GetUnit(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<Unit*>(nullptr));
}

Creature* ObjectAccessor::GetCreature(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<Creature*>(nullptr));
}

Pet* ObjectAccessor::GetPet(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<Pet*>(nullptr));
}

Player* ObjectAccessor::GetPlayer(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<Player*>(nullptr));
}

Player* ObjectAccessor::FindPlayer(Map* map, ObjectGuid guid)
{
    return GetObjectInMap(guid, map, static_cast<Player*>(nullptr));
}

Transport* ObjectAccessor::GetTransport(WorldObject const& u, ObjectGuid guid)
{
    return GetObjectInMap(guid, u.GetMap(), static_cast<Transport*>(nullptr));
}

Creature* ObjectAccessor::GetCreatureOrPetOrVehicle(WorldObject const& u, ObjectGuid guid)
{
    if (guid.IsPet())
        return GetPet(u, guid);

    if (guid.IsCreatureOrVehicle())
        return GetCreature(u, guid);

    return nullptr;
}

Pet* ObjectAccessor::FindPet(ObjectGuid const& guid)
{
    return GetObjectInWorld(guid, static_cast<Pet*>(nullptr));
}

Player* ObjectAccessor::FindPlayer(ObjectGuid const& guid, bool checInWorld/*=true*/)
{
    Player* res = GetObjectInWorld(guid, static_cast<Player*>(nullptr));
    if (res && !res->IsInWorld())
        return nullptr;
    return res;
}

Unit* ObjectAccessor::FindUnit(ObjectGuid const& guid)
{
    return GetObjectInWorld(guid, static_cast<Unit*>(nullptr));
}

GameObject* ObjectAccessor::FindGameObject(ObjectGuid const& guid)
{
    return GetObjectInWorld(guid, static_cast<GameObject*>(nullptr));
}

Creature* ObjectAccessor::FindCreature(ObjectGuid const& guid)
{
    return GetObjectInWorld(guid, static_cast<Creature*>(nullptr));
}

Player* ObjectAccessor::FindPlayerByName(std::string name)
{
    name = sObjectMgr->GetRealCharName(name);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    return HashMapHolder<Player>::FindStr(name);
}

void ObjectAccessor::SaveAllPlayers()
{
    HashMapHolder<Player>::GetLock().lock_shared();
    for (auto &pair : GetPlayers())
        pair.second->SaveToDB();
    HashMapHolder<Player>::GetLock().unlock_shared();
}

Corpse* ObjectAccessor::GetCorpseForPlayerGUID(ObjectGuid guid)
{
    std::lock_guard<std::recursive_mutex> _lock(i_corpseLock);

    Player2CorpsesMapType::iterator iter = i_player2corpse.find(guid);
    if (iter == i_player2corpse.end())
        return nullptr;

    ASSERT(iter->second->GetType() != CORPSE_BONES);

    return iter->second;
}

void ObjectAccessor::RemoveCorpse(Corpse* corpse)
{
    ASSERT(corpse && corpse->GetType() != CORPSE_BONES);

    /// @todo more works need to be done for corpse and other world object
    if (Map* map = corpse->FindMap())
    {
        corpse->DestroyForNearbyPlayers();
        if (corpse->IsInGrid())
            map->RemoveFromMap(corpse, false);
        else
        {
            corpse->RemoveFromWorld();
            corpse->ResetMap();
        }
    }
    else
        corpse->RemoveFromWorld();

    // Critical section
    {
        std::lock_guard<std::recursive_mutex> _lock(i_corpseLock);

        Player2CorpsesMapType::iterator iter = i_player2corpse.find(corpse->GetOwnerGUID());
        if (iter == i_player2corpse.end()) // TODO: Fix this
            return;

        // build mapid*cellid -> guid_set map
        CellCoord cellCoord = Trinity::ComputeCellCoord(corpse->GetPositionX(), corpse->GetPositionY());
        sObjectMgr->DeleteCorpseCellData(corpse->GetMapId(), cellCoord.GetId(), corpse->GetOwnerGUID());

        i_player2corpse.erase(iter);
    }
}

void ObjectAccessor::AddCorpse(Corpse* corpse)
{
    ASSERT(corpse && corpse->GetType() != CORPSE_BONES);

    // Critical section
    {
        std::lock_guard<std::recursive_mutex> _lock(i_corpseLock);

        ASSERT(i_player2corpse.find(corpse->GetOwnerGUID()) == i_player2corpse.end());
        i_player2corpse[corpse->GetOwnerGUID()] = corpse;

        // build mapid*cellid -> guid_set map
        CellCoord cellCoord = Trinity::ComputeCellCoord(corpse->GetPositionX(), corpse->GetPositionY());
        sObjectMgr->AddCorpseCellData(corpse->GetMapId(), cellCoord.GetId(), corpse->GetOwnerGUID(), corpse->GetInstanceId());
    }
}

void ObjectAccessor::AddCorpsesToGrid(GridCoord const& gridpair, Grid& grid, Map* map)
{
    std::lock_guard<std::recursive_mutex> _lock(i_corpseLock);

    for (Player2CorpsesMapType::iterator iter = i_player2corpse.begin(); iter != i_player2corpse.end(); ++iter)
    {
        // We need this check otherwise a corpose may be added to a grid twice
        if (iter->second->IsInGrid())
            continue;

        if (iter->second->GetGridCoord() == gridpair)
        {
            // verify, if the corpse in our instance (add only corpses which are)
            if (map->Instanceable())
            {
                if (iter->second->GetInstanceId() == map->GetInstanceId())
                    grid.AddWorldObject(iter->second);
            }
            else
                grid.AddWorldObject(iter->second);
        }
    }
}

Corpse* ObjectAccessor::ConvertCorpseForPlayer(ObjectGuid player_guid, bool insignia /*=false*/)
{
    Corpse* corpse = GetCorpseForPlayerGUID(player_guid);
    if (!corpse)
    {
        //in fact this function is called from several places
        //even when player doesn't have a corpse, not an error
        return nullptr;
    }

    TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Deleting Corpse and spawned bones.");

    // Map can be NULL
    Map* map = corpse->FindMap();

    // remove corpse from player_guid -> corpse map and from current map
    RemoveCorpse(corpse);

    // remove corpse from DB
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    corpse->DeleteFromDB(trans);
    CharacterDatabase.CommitTransaction(trans);

    Corpse* bones = nullptr;
    // create the bones only if the map and the grid is loaded at the corpse's location
    // ignore bones creating option in case insignia

    if (map && (insignia ||
        (map->IsBattlegroundOrArena() ? sWorld->getBoolConfig(CONFIG_DEATH_BONES_BG_OR_ARENA) : sWorld->getBoolConfig(CONFIG_DEATH_BONES_WORLD))) &&
        !map->IsRemovalGrid(corpse->GetPositionX(), corpse->GetPositionY()))
    {
        // Create bones, don't change Corpse
        bones = new Corpse;
        bones->Create(corpse->GetGUIDLow(), map);

        for (uint8 i = OBJECT_FIELD_TYPE + 1; i < CORPSE_END; ++i)                    // don't overwrite guid and object type
            bones->SetUInt32Value(i, corpse->GetUInt32Value(i));

        bones->SetGridCoord(corpse->GetGridCoord());
        // bones->m_time = m_time;                              // don't overwrite time
        // bones->m_type = m_type;                              // don't overwrite type
        bones->Relocate(*corpse);
        bones->SetPhaseMask(corpse->GetPhaseMask(), false);

        bones->SetUInt32Value(CORPSE_FIELD_FLAGS, corpse->GetUInt32Value(CORPSE_FIELD_FLAGS) | CORPSE_FLAG_BONES);

        // add bones in grid store if grid loaded where corpse placed
        map->AddToMap(bones);
    }

    if (corpse->IsInGrid())
        corpse->RemoveFromGrid();

    // all references to the corpse should be removed at this point
    delete corpse;

    return bones;
}

void ObjectAccessor::RemoveOldCorpses()
{
    time_t now = time(nullptr);
    Player2CorpsesMapType::iterator next;
    for (Player2CorpsesMapType::iterator itr = i_player2corpse.begin(); itr != i_player2corpse.end(); itr = next)
    {
        next = itr;
        ++next;

        if (!itr->second->IsExpired(now))
            continue;

        ConvertCorpseForPlayer(itr->first);
    }
}

void ObjectAccessor::Update(uint32 /*diff*/)
{
}

void ObjectAccessor::UnloadAll()
{
    for (auto itr = i_player2corpse.begin(); itr != i_player2corpse.end(); ++itr)
    {
        auto &corpse = itr->second;

        corpse->RemoveFromWorld();
        if (corpse->IsInGrid())
            corpse->RemoveFromGrid();

        delete corpse;
    }
}

/// Define the static members of HashMapHolder

template <class T> std::unordered_map<ObjectGuid, T*> HashMapHolder<T>::_objectMap;
template <class T> std::unordered_map<std::string, T*> HashMapHolder<T>::_objectMapStr;
template <class T> std::vector<T*> HashMapHolder<T>::_objectVector;
template <class T> sf::contention_free_shared_mutex< > HashMapHolder<T>::i_lock;
template <class T> sf::contention_free_shared_mutex< > HashMapHolder<T>::i_lockVector;
template <class T> uint32 HashMapHolder<T>::_size;
template <class T> std::atomic<bool> HashMapHolder<T>::_checkLock;

/// Global definitions for the hashmap storage

template class HashMapHolder<Player>;
template class HashMapHolder<Pet>;
template class HashMapHolder<GameObject>;
template class HashMapHolder<DynamicObject>;
template class HashMapHolder<Creature>;
template class HashMapHolder<Corpse>;
template class HashMapHolder<Transport>;
template class HashMapHolder<AreaTrigger>;
template class HashMapHolder<Conversation>;
template class HashMapHolder<EventObject>;

template Player* ObjectAccessor::GetObjectInWorld<Player>(uint32 mapid, float x, float y, ObjectGuid guid, Player* /*fake*/);
template Pet* ObjectAccessor::GetObjectInWorld<Pet>(uint32 mapid, float x, float y, ObjectGuid guid, Pet* /*fake*/);
template Creature* ObjectAccessor::GetObjectInWorld<Creature>(uint32 mapid, float x, float y, ObjectGuid guid, Creature* /*fake*/);
template Corpse* ObjectAccessor::GetObjectInWorld<Corpse>(uint32 mapid, float x, float y, ObjectGuid guid, Corpse* /*fake*/);
template GameObject* ObjectAccessor::GetObjectInWorld<GameObject>(uint32 mapid, float x, float y, ObjectGuid guid, GameObject* /*fake*/);
template DynamicObject* ObjectAccessor::GetObjectInWorld<DynamicObject>(uint32 mapid, float x, float y, ObjectGuid guid, DynamicObject* /*fake*/);
template Transport* ObjectAccessor::GetObjectInWorld<Transport>(uint32 mapid, float x, float y, ObjectGuid guid, Transport* /*fake*/);
template AreaTrigger* ObjectAccessor::GetObjectInWorld<AreaTrigger>(uint32 mapid, float x, float y, ObjectGuid guid, AreaTrigger* /*fake*/);
template Conversation* ObjectAccessor::GetObjectInWorld<Conversation>(uint32 mapid, float x, float y, ObjectGuid guid, Conversation* /*fake*/);
template EventObject* ObjectAccessor::GetObjectInWorld<EventObject>(uint32 mapid, float x, float y, ObjectGuid guid, EventObject* /*fake*/);
