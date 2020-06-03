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
#include "Corpse.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "DatabaseEnv.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"

Corpse::Corpse(CorpseType type) : WorldObject(type != CORPSE_BONES), m_type(type)
{
    m_objectType |= TYPEMASK_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;

    m_updateFlag = UPDATEFLAG_STATIONARY_POSITION;

    m_valuesCount = CORPSE_END;
    _dynamicValuesCount = CORPSE_DYNAMIC_END;

    m_time = time(nullptr);

    lootForBody = false;
    lootRecipient = nullptr;
    objectCountInWorld[uint8(HighGuid::Corpse)]++;
}

Corpse::~Corpse()
{
    objectCountInWorld[uint8(HighGuid::Corpse)]--;
}

void Corpse::AddToWorld()
{
    ///- Register the corpse for guid lookup
    if (!IsInWorld())
        sObjectAccessor->AddObject(this);

    Object::AddToWorld();
}

void Corpse::RemoveFromWorld()
{
    ///- Remove the corpse from the accessor
    if (IsInWorld())
        sObjectAccessor->RemoveObject(this);

    WorldObject::RemoveFromWorld();
}

bool Corpse::Create(ObjectGuid::LowType guidlow, Map* map)
{
    SetMap(map);
    Object::_Create(ObjectGuid::Create<HighGuid::Corpse>(map->GetId(), 0, guidlow));
    loot.SetSource(GetGUID());
    return true;
}

bool Corpse::Create(ObjectGuid::LowType guidlow, Player* owner)
{
    ASSERT(owner);

    Relocate(*owner);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_PLAYER, "Corpse (guidlow %d, owner %s) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow, owner->GetName(), owner->GetPositionX(), owner->GetPositionY());
        return false;
    }

    //we need to assign owner's map for corpse
    //in other way we will get a crash in Corpse::SaveToDB()
    SetMap(owner->GetMap());

    Object::_Create(ObjectGuid::Create<HighGuid::Corpse>(GetMapId(), 0, guidlow));
    //SetPhaseMask(owner->GetPhaseMask(), false);
    //SetPhaseId(owner->GetPhases(), false);

    SetObjectScale(1);
    SetGuidValue(CORPSE_FIELD_OWNER, owner->GetGUID());

    loot.SetSource(GetGUID());

    _gridCoord = Trinity::ComputeGridCoord(GetPositionX(), GetPositionY());

    return true;
}

void Corpse::SaveToDB()
{
    // prevent DB data inconsistence problems and duplicates
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    DeleteFromDB(trans);

    uint16 index = 0;
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CORPSE);
    stmt->setUInt64(index++, GetGUIDLow());                                           // corpseGuid
    stmt->setUInt64(index++, GetOwnerGUID().GetCounter());                            // guid
    stmt->setFloat (index++, GetPositionX());                                         // posX
    stmt->setFloat (index++, GetPositionY());                                         // posY
    stmt->setFloat (index++, GetPositionZ());                                         // posZ
    stmt->setFloat (index++, GetOrientation());                                       // orientation
    stmt->setUInt16(index++, GetMapId());                                             // mapId
    stmt->setUInt32(index++, GetUInt32Value(CORPSE_FIELD_DISPLAY_ID));                // displayId
    stmt->setString(index++, _ConcatFields(CORPSE_FIELD_ITEMS, EQUIPMENT_SLOT_END));  // itemCache
    stmt->setUInt32(index++, GetUInt32Value(CORPSE_FIELD_BYTES_1));                   // bytes1
    stmt->setUInt32(index++, GetUInt32Value(CORPSE_FIELD_BYTES_2));                   // bytes2
    stmt->setUInt8 (index++, GetUInt32Value(CORPSE_FIELD_FLAGS));                     // flags
    stmt->setUInt8 (index++, GetUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS));             // dynFlags
    stmt->setUInt32(index++, uint32(m_time));                                         // time
    stmt->setUInt8 (index++, GetType());                                              // corpseType
    stmt->setUInt32(index++, GetInstanceId());                                        // instanceId
    stmt->setUInt16(index++, GetPhaseMask());                                         // phaseMask
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void Corpse::DeleteBonesFromWorld()
{
    ASSERT(GetType() == CORPSE_BONES);
    Corpse* corpse = ObjectAccessor::GetCorpse(*this, GetGUID());

    if (!corpse)
    {
        TC_LOG_ERROR(LOG_FILTER_PLAYER, "Bones %u not found in world.", GetGUIDLow());
        return;
    }

    AddObjectToRemoveList();
}

void Corpse::DeleteFromDB(SQLTransaction& trans)
{
    PreparedStatement* stmt = nullptr;
    if (GetType() == CORPSE_BONES)
    {
        // Only specific bones
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CORPSE);
        stmt->setUInt64(0, GetGUIDLow());
    }
    else
    {
        // all corpses (not bones)
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PLAYER_CORPSES);
        stmt->setUInt64(0, GetOwnerGUID().GetCounter());
    }
    trans->Append(stmt);
}

ObjectGuid Corpse::GetOwnerGUID() const
{
    return GetGuidValue(CORPSE_FIELD_OWNER);
}

time_t const& Corpse::GetGhostTime() const
{
    return m_time;
}

void Corpse::ResetGhostTime()
{
    m_time = time(nullptr);
}

CorpseType Corpse::GetType() const
{
    return m_type;
}

GridCoord const& Corpse::GetGridCoord() const
{
    return _gridCoord;
}

void Corpse::SetGridCoord(GridCoord const& gridCoord)
{
    _gridCoord = gridCoord;
}

void Corpse::Say(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    MonsterSay(textId, language, TargetGuid);
}

void Corpse::Yell(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    MonsterYell(textId, language, TargetGuid);
}

void Corpse::TextEmote(int32 textId, ObjectGuid TargetGuid)
{
    MonsterTextEmote(textId, TargetGuid);
}

void Corpse::Whisper(int32 textId, ObjectGuid receiver)
{
    MonsterWhisper(textId, receiver);
}

void Corpse::YellToZone(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    MonsterYellToZone(textId, language, TargetGuid);
}

bool Corpse::LoadCorpseFromDB(ObjectGuid::LowType guid, Field* fields)
{
    //        0     1     2     3            4      5          6          7       8       9      10        11    12          13          14          15         16
    // SELECT posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, flags, dynFlags, time, corpseType, instanceId, phaseMask, corpseGuid, guid FROM corpse WHERE corpseType <> 0

    float posX   = fields[0].GetFloat();
    float posY   = fields[1].GetFloat();
    float posZ   = fields[2].GetFloat();
    float o      = fields[3].GetFloat();
    uint32 mapId = fields[4].GetUInt16();

    Object::_Create(ObjectGuid::Create<HighGuid::Corpse>(mapId, 0, guid));

    SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, fields[5].GetUInt32());
    _LoadIntoDataField(fields[6].GetString(), CORPSE_FIELD_ITEMS, EQUIPMENT_SLOT_END);
    
    SetFloatValue(OBJECT_FIELD_SCALE, 1.0f);
    SetUInt32Value(CORPSE_FIELD_BYTES_1, fields[7].GetUInt32());
    SetUInt32Value(CORPSE_FIELD_BYTES_2, fields[8].GetUInt32());
    SetUInt32Value(CORPSE_FIELD_FLAGS, fields[9].GetUInt8());
    SetUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, fields[10].GetUInt8());
    SetGuidValue(CORPSE_FIELD_OWNER, ObjectGuid::Create<HighGuid::Player>(fields[16].GetUInt64()));
    if (CharacterInfo const* characterInfo = sWorld->GetCharacterInfo(GetGuidValue(CORPSE_FIELD_OWNER)))
        SetUInt32Value(CORPSE_FIELD_FACTION_TEMPLATE, sChrRacesStore.AssertEntry(characterInfo->Race)->FactionID);

    m_time = time_t(fields[11].GetUInt32());

    uint32 instanceId  = fields[13].GetUInt32();
    uint32 phaseMask   = fields[14].GetUInt16();

    // place
    SetLocationInstanceId(instanceId);
    SetLocationMapId(mapId);
    SetPhaseMask(phaseMask, false);
    Relocate(posX, posY, posZ, o);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_PLAYER, "Corpse (guid: %u, owner: %u) is not created, given coordinates are not valid (X: %f, Y: %f, Z: %f)",
            GetGUIDLow(), GetOwnerGUID().GetCounter(), posX, posY, posZ);
        return false;
    }

    _gridCoord = Trinity::ComputeGridCoord(GetPositionX(), GetPositionY());
    return true;
}

bool Corpse::IsExpired(time_t t) const
{
    if (m_type == CORPSE_BONES)
        return m_time < t - 5 * MINUTE;
    return m_time < t - 3 * DAY;
}
