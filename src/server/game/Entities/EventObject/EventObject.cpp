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
#include "DatabaseEnv.h"
#include "EventObject.h"
#include "EventObjectData.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "World.h"

EventObject::EventObject() : WorldObject(false), m_DBTableGuid(0)
{
    m_objectType |= TYPEMASK_EVENTOBJECT;
    m_objectTypeId = TYPEID_EVENTOBJECT;

    m_updateFlag = UPDATEFLAG_NONE;

    m_valuesCount = OBJECT_END;
    _dynamicValuesCount = OBJECT_DYNAMIC_END;
    objectCountInWorld[uint8(HighGuid::EventObject)]++;
}

EventObject::~EventObject()
{
    objectCountInWorld[uint8(HighGuid::EventObject)]--;
}

void EventObject::AddToWorld()
{
    ///- Register the EventObject for guid lookup and for caster
    if (!IsInWorld())
    {
        sObjectAccessor->AddObject(this);
        WorldObject::AddToWorld();
    }
}

void EventObject::RemoveFromWorld()
{
    ///- Remove the EventObject from the accessor and from all lists of objects in world
    if (IsInWorld())
    {
        // dynobj could get removed in Aura::RemoveAura
        if (!IsInWorld())
            return;

        WorldObject::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

void EventObject::Update(uint32 /*p_time*/)
{
    if (!eventTemplate || eventTemplate->radius <= 0.0f)
        return;

    if (m_isUpdate)
        return;

    m_isUpdate = true;

    for (GuidSet::iterator itr = affectedPlayers.begin(), next; itr != affectedPlayers.end(); itr = next)
    {
        next = itr;
        ++next;

        Player* player = ObjectAccessor::GetPlayer(*this, *itr);
        if (!player || player->GetDistance(this) > eventTemplate->radius)
        {
            affectedPlayers.erase(itr);
            if (player)
                sScriptMgr->OnEventObject(player, this, false);
        }
    }

    std::list<Player*> playerList;
    GetPlayerListInGrid(playerList, eventTemplate->radius);
    for (auto player : playerList)
    {
        if (!player->InSamePhase(this))
            continue;

        if (ActivatedForPlayer(player))
            continue;

        if (!sScriptMgr->IsEOMeetConditions(player, this))
            continue;

        //if (!player->InInstance() || !player->isInCombat())
        //{
            if (eventTemplate->SpellID)
                player->CastSpell(player, eventTemplate->SpellID, true);

            if (eventTemplate->WorldSafeLocID)
                if (WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(eventTemplate->WorldSafeLocID))
                    player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, entry->Loc.O * M_PI / 180.0f);
        //}

        sScriptMgr->OnEventObject(player, this, true);
        affectedPlayers.insert(player->GetGUID());
        if (eventTemplate->Flags & EO_FLAG_ONE_ACTION) // only one action
            affectedPermamentPlayers.insert(player->GetGUID());
    }

    m_isUpdate = false;
}

void EventObject::MoveInLineOfSight(Unit* who)
{
}

void EventObject::Remove()
{
    if (IsInWorld())
    {
        RemoveFromWorld();
        AddObjectToRemoveList();
    }
}

bool EventObject::LoadEventObjectFromDB(ObjectGuid::LowType guid, Map* map)
{
    EventObjectData const* data = sEventObjectDataStore->GetEventObjectData(guid);
    if (!data)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "EventObject (GUID: %u) not found in table `eventobject`, can't load. ", guid);
        return false;
    }

    m_DBTableGuid = guid;
    if (map->GetInstanceId() != 0) guid = sObjectMgr->GetGenerator<HighGuid::EventObject>()->Generate();

    SetMap(map);
    Relocate(data->Pos);
    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "EventObject (EventObject %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", data->id, GetPositionX(), GetPositionY());
        return false;
    }

    eventTemplate = sEventObjectDataStore->GetEventObjectTemplate(data->id);

    Object::_Create(ObjectGuid::Create<HighGuid::EventObject>(GetMapId(), data->id, guid));
    SetPhaseMask(data->phaseMask, false);
    SetPhaseId(data->PhaseID, false);

    SetEntry(data->id);
    SetObjectScale(1.0f);

    return true;
}

bool EventObject::Create(ObjectGuid::LowType guidlow, Map* map, uint32 phaseMask, uint32 entry, float x, float y, float z, float ang, float radius, uint32 spell, uint32 worldsafe)
{
    ASSERT(map);
    SetMap(map);
    SetPhaseMask(phaseMask, false);

    eventTemplate = sEventObjectDataStore->GetEventObjectTemplate(entry);
    if (!eventTemplate)
        eventTemplate = sEventObjectDataStore->AddEventObjectTemplate(entry, radius, spell, worldsafe);

    Relocate(x, y, z, ang);

    Object::_Create(ObjectGuid::Create<HighGuid::EventObject>(GetMapId(), entry, guidlow));

    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_UNITS, "EventObject::Create(): given coordinates for eventobject (guidlow %d, entry %d) are not valid (X: %f, Y: %f, Z: %f, O: %f)", guidlow, entry, x, y, z, ang);
        return false;
    }

    SetEntry(entry);

    return true;
}

void EventObject::SaveToDB(uint32 mapid, uint64 spawnMask, uint32 phaseMask)
{
    // update in loaded data
    if (!m_DBTableGuid)
        m_DBTableGuid = GetGUIDLow();

    EventObjectData& data = sEventObjectDataStore->NewOrExistEventObjectData(m_DBTableGuid);

    uint32 zoneId = 0;
    uint32 areaId = 0;
    sMapMgr->GetZoneAndAreaId(zoneId, areaId, mapid, GetPositionX(), GetPositionY(), GetPositionZ());

    data.id = GetEntry();
    data.mapid = mapid;
    data.zoneId = zoneId;
    data.areaId = areaId;
    data.phaseMask = phaseMask;
    data.spawnMask = spawnMask;
    data.Pos = GetPosition();

    // update in DB
    SQLTransaction trans = WorldDatabase.BeginTransaction();

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_EVENTOBJECT);
    stmt->setUInt64(0, m_DBTableGuid);
    trans->Append(stmt);

    uint8 index = 0;

    stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_EVENTOBJECT);
    stmt->setUInt64(index++, m_DBTableGuid);
    stmt->setUInt32(index++, GetEntry());
    stmt->setUInt16(index++, uint16(mapid));
    stmt->setUInt32(index++, zoneId);
    stmt->setUInt32(index++, areaId);
    stmt->setUInt64(index++, spawnMask);
    stmt->setUInt16(index++, uint16(GetPhaseMask()));
    stmt->setFloat(index++,  GetPositionX());
    stmt->setFloat(index++,  GetPositionY());
    stmt->setFloat(index++,  GetPositionZH());
    stmt->setFloat(index++,  GetOrientation());
    trans->Append(stmt);

    WorldDatabase.CommitTransaction(trans);
}

uint32 EventObject::GetScriptId() const
{
    if (eventTemplate)
        return eventTemplate->ScriptID;

    return 0;
}

bool EventObject::ActivatedForPlayer(Player* player) const
{
    if ((eventTemplate->Flags & EO_FLAG_ONE_ACTION) && affectedPermamentPlayers.find(player->GetGUID()) != affectedPermamentPlayers.end())
        return true;
    return affectedPlayers.find(player->GetGUID()) != affectedPlayers.end();
}
