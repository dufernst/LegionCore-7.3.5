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

#ifndef TRINITY_OBJECTACCESSOR_H
#define TRINITY_OBJECTACCESSOR_H

#include "GridDefines.h"
#include "UpdateData.h"
#include "Object.h"
#include "Player.h"
#include "Transport.h"
#include <safe_ptr.h>

class Creature;
class Corpse;
class Unit;
class GameObject;
class DynamicObject;
class WorldObject;
class Vehicle;
class Map;
class WorldRunnable;
class Transport;
class EventObject;

static uint32 const INCREMENT_COUNTER = 5000000;

template <class T>
class HashMapHolder
{
public:
    typedef std::unordered_map<ObjectGuid, T*> MapType;
    typedef std::unordered_map<std::string, T*> MapTypeStr;
    typedef std::vector<T*> MapTypeVector;

    static void Insert(T* o)
    {
        volatile uint32 _guidlow = o->GetGUIDLow(); // For debug
        volatile uint32 _sizeV = _size; // For debug
        if (_guidlow >= _size) // If guid buged don`t check it
            return;

        _objectVector[_guidlow] = o;
        if (o->IsPlayer())
        {
            i_lock.lock();
            _objectMap[o->GetGUID()] = o;
            std::string _name = o->GetName();
            std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
            _objectMapStr[_name] = o;
            i_lock.unlock();
        }
    }

    static void Remove(T* o)
    {
        volatile uint32 _guidlow = o->GetGUIDLow(); // For debug
        volatile uint32 _sizeV = _size; // For debug
        if (_guidlow >= _size) // If guid buged don`t check it
            return;

        _objectVector[_guidlow] = NULL;
        if (o->IsPlayer())
        {
            i_lock.lock();
            _objectMap.erase(o->GetGUID());
            std::string _name = o->GetName();
            std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
            _objectMapStr.erase(_name);
            i_lock.unlock();
        }
    }

    static T* Find(ObjectGuid guid)
    {
        if (_checkLock)
        {
            i_lockVector.lock_shared();
            i_lockVector.unlock_shared();
        }

        volatile uint32 _guidlow = guid.GetGUIDLow(); // For debug
        volatile uint32 _sizeV = _size; // For debug
        if (_guidlow >= _size) // If guid buged don`t check it
            return nullptr;
        return _objectVector[_guidlow];
    }

    static T* FindLow(ObjectGuid::LowType guidLow)
    {
        if (guidLow < _size)
            return _objectVector[guidLow];

        return nullptr;
    }

    static T* FindStr(std::string name)
    {
        i_lock.lock_shared();
        typename MapTypeStr::iterator itr = _objectMapStr.find(name);
        i_lock.unlock_shared();
        return (itr != _objectMapStr.end()) ? itr->second : NULL;
    }

    static void SetSize(uint64 size)
    {
        if (_objectVector.size() < (size + INCREMENT_COUNTER))
        {
            _checkLock = true;
            i_lockVector.lock();
            _objectVector.resize(size + INCREMENT_COUNTER * 3);
            _size = _objectVector.size();
            i_lockVector.unlock();
            _checkLock = false;
        }
    }

    static MapType& GetContainer() { return _objectMap; }

    static sf::contention_free_shared_mutex< >& GetLock() { return i_lock; }

    static uint32 _size;

private:

    //Non instanceable only static
    HashMapHolder() { _checkLock = false; _size = 0; }

    static sf::contention_free_shared_mutex< > i_lock;
    static sf::contention_free_shared_mutex< > i_lockVector;
    static MapType _objectMap;
    static MapTypeStr _objectMapStr;
    static MapTypeVector _objectVector;
    static std::atomic<bool> _checkLock;
};

class ObjectAccessor
{
    ObjectAccessor();
    ~ObjectAccessor();
    ObjectAccessor(const ObjectAccessor&) = delete;
    ObjectAccessor& operator=(const ObjectAccessor&) = delete;

public:
    // TODO: override these template functions for each holder type and add assertions

    static ObjectAccessor* instance()
    {
        static ObjectAccessor instance;
        return &instance;
    }

    template<class T> static T* GetObjectInOrOutOfWorld(ObjectGuid guid, T* /*typeSpecifier*/)
    {
        return HashMapHolder<T>::Find(guid);
    }

    static Unit* GetObjectInOrOutOfWorld(ObjectGuid guid, Unit* /*typeSpecifier*/)
    {
        if (guid.IsPlayer())
            return static_cast<Unit*>(GetObjectInOrOutOfWorld(guid, static_cast<Player*>(nullptr)));

        if (guid.IsPet())
            return static_cast<Unit*>(GetObjectInOrOutOfWorld(guid, static_cast<Pet*>(nullptr)));

        return static_cast<Unit*>(GetObjectInOrOutOfWorld(guid, static_cast<Creature*>(nullptr)));
    }

    // returns object if is in world
    template<class T> static T* GetObjectInWorld(ObjectGuid guid, T* /*typeSpecifier*/)
    {
        return HashMapHolder<T>::Find(guid);
    }

    // Player may be not in world while in ObjectAccessor
    static Player* GetObjectInWorld(ObjectGuid guid, Player* /*typeSpecifier*/)
    {
        Player* player = HashMapHolder<Player>::Find(guid);
        if (player && player->IsInWorld() && !player->IsPreDelete())
            return player;
        return nullptr;
    }

    static Unit* GetObjectInWorld(ObjectGuid guid, Unit* /*typeSpecifier*/)
    {
        if (guid.IsPlayer())
            return static_cast<Unit*>(GetObjectInWorld(guid, static_cast<Player*>(nullptr)));

        if (guid.IsPet())
            return static_cast<Unit*>(GetObjectInWorld(guid, static_cast<Pet*>(nullptr)));

        return static_cast<Unit*>(GetObjectInWorld(guid, static_cast<Creature*>(nullptr)));
    }

    // returns object if is in map
    template<class T> static T* GetObjectInMap(ObjectGuid guid, Map* map, T* /*typeSpecifier*/)
    {
        // ASSERT(map);
        if (!map)
            return nullptr;

        if (T* obj = GetObjectInWorld(guid, static_cast<T*>(nullptr)))
            if (obj->GetMap() == map && !obj->IsPreDelete())
                return obj;

        return nullptr;
    }

    template<class T> static T* GetObjectInWorld(uint32 mapid, float x, float y, ObjectGuid guid, T* /*fake*/)
    {
        T* obj = HashMapHolder<T>::Find(guid);
        if (!obj || obj->GetMapId() != mapid || obj->IsPreDelete())
            return nullptr;

        CellCoord p = Trinity::ComputeCellCoord(x, y);
        if (!p.IsCoordValid())
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "ObjectAccessor::GetObjectInWorld: invalid coordinates supplied X:%f Y:%f grid cell [%u:%u]", x, y, p.x_coord, p.y_coord);
            return nullptr;
        }

        CellCoord q = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
        if (!q.IsCoordValid())
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "ObjectAccessor::GetObjecInWorld: object (GUID: %u TypeId: %u) has invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUIDLow(), obj->GetTypeId(), obj->GetPositionX(), obj->GetPositionY(), q.x_coord, q.y_coord);
            return nullptr;
        }

        int32 dx = int32(p.x_coord) - int32(q.x_coord);
        int32 dy = int32(p.y_coord) - int32(q.y_coord);

        if (dx > -2 && dx < 2 && dy > -2 && dy < 2)
            return obj;
        return nullptr;
    }

    // these functions return objects only if in map of specified object
    static WorldObject* GetWorldObject(WorldObject const&, ObjectGuid);
    static Object* GetObjectByTypeMask(WorldObject const&, ObjectGuid, uint32 typemask);
    static Corpse* GetCorpse(WorldObject const& u, ObjectGuid guid);
    static GameObject* GetGameObject(WorldObject const& u, ObjectGuid guid);
    static DynamicObject* GetDynamicObject(WorldObject const& u, ObjectGuid guid);
    static AreaTrigger* GetAreaTrigger(WorldObject const& u, ObjectGuid guid);
    static Conversation* GetConversation(WorldObject const& u, ObjectGuid guid);
    static Unit* GetUnit(WorldObject const&, ObjectGuid guid);
    static Creature* GetCreature(WorldObject const& u, ObjectGuid guid);
    static Pet* GetPet(WorldObject const&, ObjectGuid guid);
    static Player* GetPlayer(WorldObject const&, ObjectGuid guid);
    static Player* FindPlayer(Map* map, ObjectGuid guid);
    static Creature* GetCreatureOrPetOrVehicle(WorldObject const&, ObjectGuid);
    static Transport* GetTransport(WorldObject const& u, ObjectGuid guid);
    static EventObject* GetEventObject(WorldObject const& u, ObjectGuid guid);
    static Object* GetObject(Map* map, ObjectGuid guid);

    // these functions return objects if found in whole world
    // ACCESS LIKE THAT IS NOT THREAD SAFE
    static Pet* FindPet(ObjectGuid const& g);
    static Player* FindPlayer(ObjectGuid const& g, bool inWorld = true);
    static Unit* FindUnit(ObjectGuid const& g);
    static GameObject* FindGameObject(ObjectGuid const& guid);
    static Creature* FindCreature(ObjectGuid const& guid);

    static Player* FindPlayerByName(std::string name);

    // when using this, you must use the hashmapholder's lock
    static HashMapHolder<Player>::MapType const& GetPlayers()
    {
        return HashMapHolder<Player>::GetContainer();
    }

    template<class T> static void AddObject(T* object)
    {
        HashMapHolder<T>::Insert(object);
    }

    template<class T> static void RemoveObject(T* object)
    {
        HashMapHolder<T>::Remove(object);
    }

    static void SaveAllPlayers();
    static void SetGuidSize(HighGuid type, uint64 size);

    //Thread safe
    Corpse* GetCorpseForPlayerGUID(ObjectGuid guid);
    void RemoveCorpse(Corpse* corpse);
    void AddCorpse(Corpse* corpse);
    void AddCorpsesToGrid(GridCoord const& gridpair, Grid& grid, Map* map);
    Corpse* ConvertCorpseForPlayer(ObjectGuid player_guid, bool insignia = false);

    //Thread unsafe
    void Update(uint32 diff);
    void RemoveOldCorpses();
    void UnloadAll();

private:
    typedef std::unordered_map<ObjectGuid, Corpse*> Player2CorpsesMapType;

    Player2CorpsesMapType i_player2corpse;
    std::recursive_mutex i_corpseLock;
};

#define sObjectAccessor ObjectAccessor::instance()

#endif
