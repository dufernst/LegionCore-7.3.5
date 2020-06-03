/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "ObjectGridLoader.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Creature.h"
#include "Vehicle.h"
#include "GameObject.h"
#include "Corpse.h"
#include "AreaTrigger.h"
#include "Conversation.h"
#include "EventObject.h"
#include "World.h"
#include "CellImpl.h"
#include "CreatureAI.h"

namespace {

template <typename T>
void AddObjectHelper(Cell const &cell, Map* map, T *obj)
{
    map->AddToGrid(obj, cell);

    obj->AddToWorld();
    if (obj->isActiveObject())
        map->AddToActive(obj);
}

template <typename T>
uint32 LoadHelper(CellGuidSet const &cellGuids, Cell const &cell, Map *map)
{
    uint32 count = 0;

    for (auto const &guid : cellGuids)
    {
        auto const obj = new T();
        if (!obj->LoadFromDB(guid, map))
        {
            delete obj;
            continue;
        }

        AddObjectHelper(cell, map, obj);
        ++count;
    }

    return count;
}

uint32 LoadHelperST(CellGuidSet const &cellGuids, Cell const &cell, Map *map)
{
    uint32 count = 0;

    for (auto const &guid : cellGuids)
    {
        GameObject* obj = new StaticTransport();
        if (!obj->LoadFromDB(guid, map))
        {
            delete obj;
            continue;
        }

        AddObjectHelper(cell, map, obj);
        map->AddStaticTransport((StaticTransport*)obj);
        ++count;
    }

    return count;
}

uint32 LoadHelper(CellCorpseMap const &cellCorpses, Cell const &cell, Map *map)
{
    uint32 count = 0;

    for (auto const &kvPair : cellCorpses)
    {
        if (kvPair.second != map->GetInstanceId())
            continue;

        auto const obj = sObjectAccessor->GetCorpseForPlayerGUID(kvPair.first);
        if (!obj)
            continue;

        // TODO: this is a hack
        // corpse's map should be reset when the map is unloaded
        // but it may still exist when the grid is unloaded but map is not
        // in that case map == currMap
        obj->SetMap(map);

        if (obj->IsInGrid())
        {
            obj->AddToWorld();
            continue;
        }

        AddObjectHelper(cell, map, obj);
        ++count;
    }

    return count;
}

} // namespace

namespace Trinity {

void ObjectGridLoader::LoadN(NGrid const &grid, Map *map, Cell cell)
{
    uint32 gameObjects = 0;
    uint32 creatures = 0;
    uint32 corpses = 0;
    uint32 conversations = 0;
    uint32 eventobjects = 0;

    for (uint32 x = 0; x < MAX_NUMBER_OF_CELLS; ++x)
    {
        cell.data.Part.cell_x = x;
        for (uint32 y = 0; y < MAX_NUMBER_OF_CELLS; ++y)
        {
            cell.data.Part.cell_y = y;

            // Load creatures and gameobjects
            if (auto const cellGuids = sObjectMgr->GetCellObjectGuids(map->GetId(), map->GetSpawnMode(), cell.GetCellCoord().GetId()))
            {
                creatures += LoadHelper<Creature>(cellGuids->creatures, cell, map);
                gameObjects += LoadHelper<GameObject>(cellGuids->gameobjects, cell, map);
                gameObjects += LoadHelperST(cellGuids->statictransports, cell, map);
                conversations += LoadHelper<Conversation>(cellGuids->conversation, cell, map);
                eventobjects += LoadHelper<EventObject>(cellGuids->eventobject, cell, map);
            }

            // Load corpses (not bones)
            if (auto const cellGuids = sObjectMgr->GetCellObjectGuids(map->GetId(), 0, cell.GetCellCoord().GetId()))
            {
                // corpses are always added to spawn mode 0 and they are spawned by their instance id
                corpses += LoadHelper(cellGuids->corpses, cell, map);
            }
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "%u GameObjects, %u Creatures %u Conversations %u EventObjects, and %u Corpses/Bones loaded for grid [%d, %d] on map %u",
                 gameObjects, creatures, conversations, eventobjects, corpses, grid.getX(), grid.getY(), map->GetId());
}


void ObjectGridEvacuator::Visit(CreatureMapType &m)
{
    // creature in unloading grid can have respawn point in another grid
    // if it will be unloaded then it will not respawn in original grid until unload/load original grid
    // move to respawn point to prevent this case. For player view in respawn grid this will be normal respawn.

    std::size_t count = m.size();

    for (std::size_t i = 0; i < count;)
    {
        auto &creature = m[i];

        if (!creature->isPet())
            creature->GetMap()->CreatureRespawnRelocation(creature, true);

        // If creature respawned in different grid, size will change
        if (m.size() == count)
            ++i;
        else
            count = m.size();
    }
}

void ObjectGridEvacuator::Visit(GameObjectMapType &m)
{
    // gameobject in unloading grid can have respawn point in another grid
    // if it will be unloaded then it will not respawn in original grid until unload/load original grid
    // move to respawn point to prevent this case. For player view in respawn grid this will be normal respawn.
    std::size_t count = m.size();

    for (std::size_t i = 0; i < count;)
    {
        auto &go = m[i];

        go->GetMap()->GameObjectRespawnRelocation(go, true);

        if (m.size() == count)
            ++i;
        else
            count = m.size();
    }
}

template <typename AnyMapType>
void ObjectGridUnloader::Visit(AnyMapType &m)
{
    for (auto &obj : m)
    {
        // if option set then object already saved at this moment
        if (!sWorld->getBoolConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime();

        if (GameObject* go = obj->ToGameObject())
            if (StaticTransport* staticTransport = go->ToStaticTransport())
                obj->GetMap()->RemoveStaticTransport(staticTransport);

        // Some creatures may summon other temp summons in CleanupsBeforeDelete()
        // So we need this even after cleaner (maybe we can remove cleaner)
        // Example: Flame Leviathan Turret 33139 is summoned when a creature is deleted
        // TODO: Check if that script has the correct logic. Do we really need to summon something before deleting?
        volatile uint32 entryorguid = obj->IsPlayer() ? obj->GetGUIDLow() : obj->GetEntry();
        obj->CleanupsBeforeDelete();
        obj->RemoveFromGrid();
        obj->AddObjectToRemoveList();
    }
}

void ObjectGridStoper::Visit(CreatureMapType &m)
{
    // _lock.lock();
    // stop any fights at grid de-activation and remove dynobjects created at cast by creatures
    for (auto &source : m)
    {
        if (!source->IsInWorld())
            continue;

        source->RemoveAllDynObjects();
        source->RemoveAllAreaObjects();       // Calls RemoveFromWorld, needs to be after RemoveAllAuras or we invalidate the Owner pointer of the aura

        if (source->isInCombat())
        {
            // For crash info
            volatile uint32 guid = source->GetGUIDLow();
            volatile uint32 entry = source->GetEntry();

            // If creature calling RemoveCharmedBy during EnterEvadeMode, RemoveCharmedBy call AIM_Initialize so AI() pointer may be corrupt
            // Maybe we need to lock AI during the call of EnterEvadeMode ?
            switch(source->GetEntry())
            {
                case 46499:
                case 62982:
                case 67236:
                case 35814:
                    break;
                default:
                    if (!source->isAnySummons() && !source->isTrainingDummy())
                        source->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
                    break;
            }

            source->CombatStop();
            source->DeleteThreatList();
            source->ClearSaveThreatTarget();
        }
    }
    // _lock.unlock();
}

template <typename AnyMapType>
void ObjectGridCleaner::Visit(AnyMapType &m)
{
    // look like we have a crash wile accessing to DynamicObject map here
    // I Guess it's DynamicObject delete pointer, we need to look at it anyway ...
    for (auto &object : m)
    {
        if (!object)
            continue;

        // For crash info
        volatile uint32 guid = object->GetGUIDLow();
        volatile uint32 entry = object->GetEntry();
        volatile uint32 isInWorld = object->IsInWorld();

        object->CleanupsBeforeDelete();
    }
}

} // namespace Trinity

template void Trinity::ObjectGridUnloader::Visit(CreatureMapType &);
template void Trinity::ObjectGridUnloader::Visit(GameObjectMapType &);
template void Trinity::ObjectGridUnloader::Visit(DynamicObjectMapType &);
template void Trinity::ObjectGridUnloader::Visit(CorpseMapType &);
template void Trinity::ObjectGridUnloader::Visit(AreaTriggerMapType &);
template void Trinity::ObjectGridUnloader::Visit(ConversationMapType &);
template void Trinity::ObjectGridUnloader::Visit(EventObjectMapType &);

template void Trinity::ObjectGridCleaner::Visit(CreatureMapType &);
template void Trinity::ObjectGridCleaner::Visit(GameObjectMapType &);
template void Trinity::ObjectGridCleaner::Visit(DynamicObjectMapType &);
template void Trinity::ObjectGridCleaner::Visit(CorpseMapType &);
template void Trinity::ObjectGridCleaner::Visit(AreaTriggerMapType &);
template void Trinity::ObjectGridCleaner::Visit(ConversationMapType &);
template void Trinity::ObjectGridCleaner::Visit(EventObjectMapType &);
