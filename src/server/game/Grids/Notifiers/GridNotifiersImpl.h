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

#ifndef TRINITY_GRIDNOTIFIERSIMPL_H
#define TRINITY_GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Corpse.h"
#include "Player.h"
#include "UpdateData.h"
#include "CreatureAI.h"
#include "SpellAuras.h"

template <typename AnyMapType>
void Trinity::VisibleNotifier::Visit(AnyMapType &m)
{
    for (auto &object : m)
    {
        vis_guids.erase(object->GetGUID());
        i_player.UpdateVisibilityOf(object, i_data, i_visibleNow);
    }
}

inline void Trinity::VisibleNotifier::Visit(EventObjectMapType &m)
{
    for (auto &object : m)
        vis_guids.erase(object->GetGUID());
}

// SEARCHERS & LIST SEARCHERS & WORKERS

// WorldObject searchers & workers

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(GameObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_GAMEOBJECT))
        return;

    // already found
    if (i_object)
        return;

    for (auto &obj : m)
    {
        if (obj->InSamePhase(i_phaseMask) && i_check(obj))
        {
            i_object = obj;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(PlayerMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_PLAYER))
        return;

    // already found
    if (i_object)
        return;

    for (auto &player : m)
    {
        if (player->InSamePhase(i_phaseMask) && i_check(player))
        {
            i_object = player;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(CreatureMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CREATURE))
        return;

    // already found
    if (i_object)
        return;

    for (auto &creature : m)
    {
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
        {
            i_object = creature;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(CorpseMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CORPSE))
        return;

    // already found
    if (i_object)
        return;

    for (auto &corpse : m)
    {
        if (corpse->InSamePhase(i_phaseMask) && i_check(corpse))
        {
            i_object = corpse;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(DynamicObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_DYNAMICOBJECT))
        return;

    // already found
    if (i_object)
        return;

    for (auto &obj : m)
    {
        if (obj->InSamePhase(i_phaseMask) && i_check(obj))
        {
            i_object = obj;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(AreaTriggerMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_AREATRIGGER))
        return;

    // already found
    if (i_object)
        return;

    for (auto &trigger : m)
    {
        if (trigger->InSamePhase(i_phaseMask) && i_check(trigger))
        {
            i_object = trigger;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(ConversationMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CONVERSATION))
        return;

    // already found
    if (i_object)
        return;

    for (auto &conver : m)
    {
        if (conver->InSamePhase(i_phaseMask) && i_check(conver))
        {
            i_object = conver;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(EventObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_EVENTOBJECT))
        return;

    // already found
    if (i_object)
        return;

    for (auto &event : m)
    {
        if (event->InSamePhase(i_phaseMask) && i_check(event))
        {
            i_object = event;
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(GameObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_GAMEOBJECT))
        return;

    for (auto &obj : m)
        if (obj->InSamePhase(i_phaseMask) && i_check(obj))
            i_object = obj;
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(PlayerMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_PLAYER))
        return;

    for (auto &player : m)
        if (player->InSamePhase(i_phaseMask) && i_check(player))
            i_object = player;
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(CreatureMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CREATURE))
        return;

    for (auto &creature : m)
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
            i_object = creature;
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(CorpseMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CORPSE))
        return;

    for (auto &corpse : m)
        if (corpse->InSamePhase(i_phaseMask) && i_check(corpse))
            i_object = corpse;
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(DynamicObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_DYNAMICOBJECT))
        return;

    for (auto &obj : m)
        if (obj->InSamePhase(i_phaseMask) && i_check(obj))
            i_object = obj;
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(AreaTriggerMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_AREATRIGGER))
        return;

    for (auto &trigger : m)
        if (trigger->InSamePhase(i_phaseMask) && i_check(trigger))
            i_object = trigger;
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(ConversationMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CONVERSATION))
        return;

    for (auto &conver : m)
        if (conver->InSamePhase(i_phaseMask) && i_check(conver))
            i_object = conver;
}

template<class Check>
void Trinity::WorldObjectLastSearcher<Check>::Visit(EventObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_EVENTOBJECT))
        return;

    for (auto &event : m)
        if (event->InSamePhase(i_phaseMask) && i_check(event))
            i_object = event;
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(PlayerMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_PLAYER))
        return;

    for (auto &player : m)
        if (i_check(player))
            i_objects.push_back(player);
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(CreatureMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CREATURE))
        return;

    for (auto &creature : m)
        if (i_check(creature))
            i_objects.push_back(creature);
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(CorpseMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CORPSE))
        return;

    for (auto &corpse : m)
        if (i_check(corpse))
            i_objects.push_back(corpse);
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(GameObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_GAMEOBJECT))
        return;

    for (auto &obj : m)
        if (i_check(obj))
            i_objects.push_back(obj);
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(DynamicObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_DYNAMICOBJECT))
        return;

    for (auto &obj : m)
        if (i_check(obj))
            i_objects.push_back(obj);
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(AreaTriggerMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_AREATRIGGER))
        return;

    for (auto &trigger : m)
        if (i_check(trigger))
            i_objects.push_back(trigger);
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(ConversationMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CONVERSATION))
        return;

    for (auto &conver : m)
        if (i_check(conver))
            i_objects.push_back(conver);
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(EventObjectMapType &m)
{
    if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_EVENTOBJECT))
        return;

    for (auto &event : m)
        if (i_check(event))
            i_objects.push_back(event);
}

// Gameobject searchers

template<class Check>
void Trinity::GameObjectSearcher<Check>::Visit(GameObjectMapType &m)
{
    // already found
    if (i_object)
        return;

    for (auto &obj : m)
    {
        if (obj->InSamePhase(i_phaseMask) && i_check(obj))
        {
            i_object = obj;
            return;
        }
    }
}

template<class Check>
void Trinity::GameObjectLastSearcher<Check>::Visit(GameObjectMapType &m)
{
    for (auto &obj : m)
        if (obj->InSamePhase(i_phaseMask) && i_check(obj))
            i_object = obj;
}

template<class Check>
void Trinity::GameObjectListSearcher<Check>::Visit(GameObjectMapType &m)
{
    for (auto &obj : m)
        if (obj->InSamePhase(i_phaseMask) && i_check(obj))
            i_objects.push_back(obj);
}

// Unit searchers

template<class Check>
void Trinity::UnitSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if (i_object)
        return;

    for (auto &creature : m)
    {
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
        {
            i_object = creature;
            return;
        }
    }
}

template<class Check>
void Trinity::UnitSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if (i_object)
        return;

    for (auto &player : m)
    {
        if (player->InSamePhase(i_phaseMask) && i_check(player))
        {
            i_object = player;
            return;
        }
    }
}

template<class Check>
void Trinity::UnitLastSearcher<Check>::Visit(CreatureMapType &m)
{
    for (auto &creature : m)
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
            i_object = creature;
}

template<class Check>
void Trinity::UnitLastSearcher<Check>::Visit(PlayerMapType &m)
{
    for (auto &player : m)
        if (player->InSamePhase(i_phaseMask) && i_check(player))
            i_object = player;
}

template<class Check>
void Trinity::UnitListSearcher<Check>::Visit(PlayerMapType &m)
{
    for (auto &player : m)
        if (player->InSamePhase(i_phaseMask) && i_check(player))
            i_objects.push_back(player);
}

template<class Check>
void Trinity::UnitListSearcher<Check>::Visit(CreatureMapType &m)
{
    for (auto &creature : m)
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
            i_objects.push_back(creature);
}

template<class Check>
void Trinity::AreaTriggerListSearcher<Check>::Visit(AreaTriggerMapType &m)
{
    for (auto &trigger : m)
        if (trigger->InSamePhase(i_phaseMask) && i_check(trigger))
            i_objects.push_back(trigger);
}
 
// Creature searchers

template<class Check>
void Trinity::CreatureSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if (i_object)
        return;

    for (auto &creature : m)
    {
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
        {
            i_object = creature;
            return;
        }
    }
}

template<class Check>
void Trinity::CreatureLastSearcher<Check>::Visit(CreatureMapType &m)
{
    for (auto &creature : m)
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
            i_object = creature;
}

template<class Check>
void Trinity::CreatureListSearcher<Check>::Visit(CreatureMapType &m)
{
    for (auto &creature : m)
        if (creature->InSamePhase(i_phaseMask) && i_check(creature))
            i_objects.push_back(creature);
}

template<class Check>
void Trinity::PlayerListSearcher<Check>::Visit(PlayerMapType &m)
{
    for (auto &player : m)
        if (player->InSamePhase(i_phaseMask) && i_check(player))
            i_objects.push_back(player);
}

template<class Check>
void Trinity::PlayerSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if (i_object)
        return;

    for (auto &player : m)
    {
        if (player->InSamePhase(i_phaseMask) && i_check(player))
        {
            i_object = player;
            return;
        }
    }
}

template<class Check>
void Trinity::PlayerLastSearcher<Check>::Visit(PlayerMapType& m)
{
    for (auto &player : m)
        if (player->InSamePhase(i_phaseMask) && i_check(player))
            i_object = player;
}

template<class Builder>
void Trinity::LocalizedPacketDo<Builder>::operator()(Player* p)
{
    LocaleConstant localeConstant = p->GetSession()->GetSessionDbLocaleIndex();
    uint32 cache_idx = localeConstant + 1;
    WorldPackets::Packet* data;

    // create if not cached yet
    if (_dataCache.size() < cache_idx + 1 || !_dataCache[cache_idx])
    {
        if (_dataCache.size() < cache_idx + 1)
            _dataCache.resize(cache_idx + 1);

        data = _builder(localeConstant);

        ASSERT(data->GetSize() == 0);

        data->Write();

        _dataCache[cache_idx] = data;
    }
    else
        data = _dataCache[cache_idx];

    p->SendDirectMessage(data->GetRawPacket());
}

template<class Builder>
void Trinity::LocalizedPacketListDo<Builder>::operator()(Player* p)
{
    LocaleConstant localeConstant = p->GetSession()->GetSessionDbLocaleIndex();
    uint32 cache_idx = localeConstant + 1;
    WorldPacketList* data;

    // create if not cached yet
    if (_dataCache.size() < cache_idx + 1 || _dataCache[cache_idx].empty())
    {
        if (_dataCache.size() < cache_idx + 1)
            _dataCache.resize(cache_idx + 1);

        data = &_dataCache[cache_idx];

        _builder(*data, localeConstant);
    }
    else
        data = &_dataCache[cache_idx];

    for (size_t i = 0; i < data->size(); ++i)
        p->SendDirectMessage((*data)[i]->GetRawPacket());
}

#endif                                                      // TRINITY_GRIDNOTIFIERSIMPL_H
