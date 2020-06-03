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
#include "World.h"
#include "ObjectAccessor.h"
#include "Conversation.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "ConversationData.h"
#include "UpdateFieldFlags.h"

ConversationSpawnData::ConversationSpawnData(): id(0), posX(0), posY(0), posZ(0), orientation(0), mapid(0), dbData(true)
{
}

Conversation::Conversation() : WorldObject(false), _caster(nullptr), _spellId(0), _duration(0)
{
    m_objectType |= TYPEMASK_CONVERSATION;
    m_objectTypeId = TYPEID_CONVERSATION;

    m_updateFlag = UPDATEFLAG_STATIONARY_POSITION;

    m_valuesCount = CONVERSATION_END;
    _dynamicValuesCount = CONVERSATION_DYNAMIC_END;

    _fieldNotifyFlags = UF_FLAG_PUBLIC | UF_FLAG_DYNAMIC | UF_FLAG_UNK0X100;
    // updateMask.SetCount(_dynamicValuesCount);
    objectCountInWorld[uint8(HighGuid::Conversation)]++;
}

Conversation::~Conversation()
{
    objectCountInWorld[uint8(HighGuid::Conversation)]--;
}

void Conversation::AddToWorld()
{
    ///- Register the Conversation for guid lookup and for caster
    if (!IsInWorld())
    {
        if(ObjectAccessor::GetUnit(*this, GetCasterGUID())) // Check caster before add to world
        {
            sObjectAccessor->AddObject(this);
            WorldObject::AddToWorld();
            BindToCaster();
        }
    }
}

void Conversation::RemoveFromWorld()
{
    ///- Remove the Conversation from the accessor and from all lists of objects in world
    if (IsInWorld())
    {
        // dynobj could get removed in Aura::RemoveAura
        if (!IsInWorld())
            return;

        UnbindFromCaster();
        WorldObject::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

bool Conversation::CreateConversation(ObjectGuid::LowType guidlow, uint32 triggerEntry, Unit* caster, SpellInfo const* info, Position const& pos)
{
    if (!caster)
        return false;

    SetMap(caster->GetMap());
    Relocate(pos);
    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Conversation (spell %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", info ? info->Id : 0, GetPositionX(), GetPositionY());
        return false;
    }

    Object::_Create(ObjectGuid::Create<HighGuid::Conversation>(GetMapId(), triggerEntry, guidlow));
    SetPhaseMask(caster->GetPhaseMask(), false);
    SetPhaseId(caster->GetPhases(), false);

    SetEntry(triggerEntry);
    SetObjectScale(1.0f);
    casterGUID = caster->GetGUID();

    if (Player* player = caster->ToPlayer())
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                if (Player* member = itr->getSource())
                    AddPlayerInPersonnalVisibilityList(member->GetGUID());
            }
        }
        else
            AddPlayerInPersonnalVisibilityList(caster->GetGUID());
    }

    uint32 duration = 0;
    if (!GenerateDynamicFieldData(triggerEntry, caster, duration))
        return false;

    if (!duration)
        duration = 30000;

    // possible it should be add.
    duration += 5000;

    SetUInt32Value(CONVERSATION_FIELD_LAST_LINE_END_TIME, duration);
    SetDuration(duration);
    // setActive(true);

    if (!GetMap()->AddToMap(this))
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Conversation (spell %u) not created. Suggested coordinates (X: %f Y: %f). Error on add to map.", info ? info->Id : 0, GetPositionX(), GetPositionY());
        return false;
    }

    return true;
}

void Conversation::Update(uint32 p_time)
{
    bool expired = false;

    if (GetDuration())
    {
        auto data = playing.find(casterGUID);
        if (data == playing.end())
            return;
       
        uint32 const dur = getMSTime() - data->second;
        if (dur > GetUInt32Value(CONVERSATION_FIELD_LAST_LINE_END_TIME))
            expired = true;

        if (expired)
            Remove();
    }
}

void Conversation::Remove()
{
    if (m_isUpdate)
        return;

    m_isUpdate = true;

    if (IsInWorld())
    {
        RemoveFromWorld();
        AddObjectToRemoveList();
    }
    m_isUpdate = false;
}

void Conversation::BindToCaster()
{
    if (!GetCasterGUID())
        return;
    ASSERT(!_caster);
    _caster = ObjectAccessor::GetUnit(*this, GetCasterGUID());
    if (!_caster)
        return;
    ASSERT(_caster->GetMap() == GetMap());
    //_caster->_RegisterConversationObject(this);
}

void Conversation::UnbindFromCaster()
{
    if (!GetCasterGUID())
        return;
    if (!_caster)
        return;

    //_caster->_UnregisterConversationObject(this);
    _caster = nullptr;
}

void Conversation::BuildDynamicValuesUpdate(uint8 updateType, ByteBuffer* data, Player* target) const
{
    if (!target)
        return;

    std::size_t blockCount = UpdateMask::GetBlockCount(_dynamicValuesCount);

    uint32* flags = nullptr;
    uint32 visibleFlag = GetDynamicUpdateFieldData(target, flags);

    *data << uint8(blockCount);
    std::size_t maskPos = data->wpos();
    data->resize(data->size() + blockCount * sizeof(UpdateMask::BlockType));

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Item::BuildDynamicValuesUpdate GetEntry %u flags %u blockCount %u visibleFlag %u maskPos %u _dynamicValuesCount %u _changesMask %u _fieldNotifyFlags %u BlockSize %u",
    // GetEntry(), flags, blockCount, visibleFlag, maskPos, _dynamicValuesCount, _changesMask[ITEM_FIELD_MODIFIERS_MASK], _fieldNotifyFlags, data->size() + blockCount * sizeof(UpdateMask::BlockType));

    for (uint16 index = 0; index < _dynamicValuesCount; ++index)
    {
        std::vector<uint32> const& values = _dynamicValues[index];
        if (_fieldNotifyFlags & flags[index] ||
            ((updateType == UPDATETYPE_VALUES ? _dynamicChangesMask[index] != UpdateMask::UNCHANGED : !values.empty()) && (flags[index] & visibleFlag)))
        {
            UpdateMask::SetUpdateBit(data->contents() + maskPos, index);

            std::size_t arrayBlockCount = UpdateMask::GetBlockCount(values.size());
            *data << uint16(UpdateMask::EncodeDynamicFieldChangeType(arrayBlockCount, _dynamicChangesMask[index], updateType));
            if (updateType == UPDATETYPE_VALUES && _dynamicChangesMask[index] == UpdateMask::VALUE_AND_SIZE_CHANGED)
                *data << uint32(values.size());

            std::size_t arrayMaskPos = data->wpos();

            data->resize(data->size() + arrayBlockCount * sizeof(UpdateMask::BlockType));

            for (std::size_t v = 0; v < values.size(); ++v)
            {
                if (updateType == UPDATETYPE_VALUES ? _dynamicChangesArrayMask[index][v] : values[v])
                {
                    UpdateMask::SetUpdateBit(data->contents() + arrayMaskPos, v);
                    *data << uint32(values[v]);
                }
            }
        }
    }
}

void Conversation::BuildValuesUpdate(uint8 updateType, ByteBuffer* data, Player* target) const
{
    if (!target)
        return;

    std::size_t blockCount = UpdateMask::GetBlockCount(m_valuesCount);

    uint32* flags = nullptr;
    uint32 visibleFlag = GetUpdateFieldData(target, flags);
    ASSERT(flags);

    *data << uint8(blockCount);
    std::size_t maskPos = data->wpos();
    data->resize(data->size() + blockCount * sizeof(UpdateMask::BlockType));

    for (uint16 index = 0; index < m_valuesCount; ++index)
    {
        if (_fieldNotifyFlags & flags[index] ||
            ((updateType == UPDATETYPE_VALUES ? _changesMask[index] : m_uint32Values[index]) && (flags[index] & visibleFlag)))
        {
            UpdateMask::SetUpdateBit(data->contents() + maskPos, index);
            *data << m_uint32Values[index];
        }
    }
}

bool Conversation::LoadConversationFromDB(ObjectGuid::LowType guid, Map* map, bool addToMap)
{
    ConversationSpawnData const* data = sConversationDataStore->GetConversationData(guid);
    if (!data)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: %u) not found in table `creature`, can't load. ", guid);
        return false;
    }

    if (!map->Instanceable())
    {
        /*if (map->GetConversation(ObjectGuid::Create<HighGuid::Conversation>(data->mapid, data->id, guid)))
            return false;*/
    }
    else
        guid = sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate();

    std::vector<ConversationData> const* conversationData = sConversationDataStore->GetConversationData(data->id);
    std::vector<ConversationCreature> const* conversationCreature = sConversationDataStore->GetConversationCreature(data->id);
    std::vector<ConversationActor> const* conversationActor = sConversationDataStore->GetConversationActor(data->id);

    bool isActor = conversationActor && !conversationActor->empty();
    bool isCreature = conversationCreature && !conversationCreature->empty();
    bool hasData = conversationData && !conversationData->empty();

    if (!hasData || (!isActor && !isCreature))
        return false;

    SetMap(map);
    Relocate(data->posX, data->posY, data->posZ, data->orientation);
    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Conversation (conversation %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", data->id, GetPositionX(), GetPositionY());
        return false;
    }

    Object::_Create(ObjectGuid::Create<HighGuid::Conversation>(GetMapId(), data->id, guid));
    SetPhaseMask(data->phaseMask, false);
    SetPhaseId(data->PhaseID, false);

    SetEntry(data->id);
    SetObjectScale(1.0f);

    uint32 duration = 30000;
    if (!GenerateDynamicFieldData(data->id, this, duration))
        return false;

    SetUInt32Value(CONVERSATION_FIELD_LAST_LINE_END_TIME, duration);
    // setActive(true);

    return !(addToMap && !GetMap()->AddToMap(this));
}

bool Conversation::CanNeverSee2(WorldObject const* seer) const
{
    if (!seer->IsPlayer())
        return true;

    auto data = playing.find(seer->GetGUID());
    if (data == playing.end())
    {
        const_cast<Conversation*>(this)->playing[seer->GetGUID()] = getMSTime();
        return false;
    }
    uint32 const dur = getMSTime() - data->second;
    return dur > GetUInt32Value(CONVERSATION_FIELD_LAST_LINE_END_TIME);
}

bool Conversation::LoadFromDB(ObjectGuid::LowType guid, Map* map)
{
    return LoadConversationFromDB(guid, map, false);
}

DynamicFieldStructuredView<ConversationDynamicFieldActors> Conversation::GetActors() const
{
    return GetDynamicStructuredValues<ConversationDynamicFieldActors>(CONVERSATION_DYNAMIC_FIELD_ACTORS);
}

ConversationDynamicFieldActors const* Conversation::GetActor(uint8 index) const
{
    return GetDynamicStructuredValue<ConversationDynamicFieldActors>(CONVERSATION_DYNAMIC_FIELD_ACTORS, index);
}

void Conversation::SetActor(ConversationDynamicFieldActors const* actorInfo, uint8 index, bool createIfMissing /*= false*/)
{
    SetDynamicStructuredValue(CONVERSATION_DYNAMIC_FIELD_ACTORS, index, actorInfo);
}

DynamicFieldStructuredView<ConversationDynamicFieldCreatures> Conversation::GetCreatures() const
{
    return GetDynamicStructuredValues<ConversationDynamicFieldCreatures>(CONVERSATION_DYNAMIC_FIELD_ACTORS);
}

ConversationDynamicFieldCreatures const* Conversation::GetCreature(uint8 index) const
{
    return GetDynamicStructuredValue<ConversationDynamicFieldCreatures>(CONVERSATION_DYNAMIC_FIELD_ACTORS, index);
}

void Conversation::SetCreature(ConversationDynamicFieldCreatures const* creatureInfo, uint8 index, bool createIfMissing /*= false*/)
{
    SetDynamicStructuredValue(CONVERSATION_DYNAMIC_FIELD_ACTORS, index, creatureInfo);
}

DynamicFieldStructuredView<ConversationDynamicFieldLines> Conversation::GetLines() const
{
    return GetDynamicStructuredValues<ConversationDynamicFieldLines>(CONVERSATION_DYNAMIC_FIELD_LINES);
}

ConversationDynamicFieldLines const* Conversation::GetLine(uint8 index) const
{
    return GetDynamicStructuredValue<ConversationDynamicFieldLines>(CONVERSATION_DYNAMIC_FIELD_LINES, index);
}

void Conversation::SetLine(ConversationDynamicFieldLines const* lineInfo, uint8 index, bool createIfMissing /*= false*/)
{
    SetDynamicStructuredValue(CONVERSATION_DYNAMIC_FIELD_LINES, index, lineInfo);
}

bool Conversation::GenerateDynamicFieldData(uint32 triggerEntry, WorldObject* sercher, uint32& duration)
{
    std::vector<ConversationData> const* conversationData = sConversationDataStore->GetConversationData(triggerEntry);
    std::vector<ConversationCreature> const* conversationCreature = sConversationDataStore->GetConversationCreature(triggerEntry);
    std::vector<ConversationActor> const* conversationActor = sConversationDataStore->GetConversationActor(triggerEntry);

    bool isActor = conversationActor && !conversationActor->empty();
    bool isCreature = conversationCreature && !conversationCreature->empty();
    bool hasData = conversationData && !conversationData->empty();

    if(!hasData || (!isActor && !isCreature))
        return false;

    if (isActor)
    {
        for (const auto& itr : *conversationActor)
        {
            ConversationDynamicFieldActors actorData;
            memset(&actorData, 0, sizeof(actorData));
            actorData.actorId = itr.actorId;
            actorData.creatureId = itr.creatureId;
            actorData.displayId = itr.displayId;
            actorData.unk1 = itr.unk1;
            actorData.unk2 = itr.unk2;
            actorData.unk3 = itr.unk3;
            SetActor(&actorData, itr.id, true);

            if (itr.duration)
                duration += itr.duration;
        }
    }
    else if(isCreature)
    {
        for (const auto& itr : *conversationCreature)
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(0xFFFFFFFFFF);

            if (itr.creatureId)
            {
                Creature* creature = sercher->FindNearestCreature(itr.creatureId, sercher->GetVisibilityRange());
                if (!creature)
                {
                    TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Conversation not created. Suggested coordinates (X: %f Y: %f). Can't find npc entry %u", GetPositionX(), GetPositionY(),
                                   itr.creatureId);
                    return false;
                }

                guid = creature->GetGUID();
            }

            ConversationDynamicFieldCreatures creatureData;
            memset(&creatureData, 0, sizeof(creatureData));
            creatureData.GuidLow = guid.GetLowPart();
            creatureData.GuidHight = guid.GetHighPart();
            creatureData.unk1 = itr.unk1;
            creatureData.unk2 = itr.unk2;
            SetCreature(&creatureData, itr.id, true);

            if (itr.duration)
                duration += itr.duration;
        }
    }

    for (const auto& itr : *conversationData)
    {
        ConversationDynamicFieldLines lineData;
        memset(&lineData, 0, sizeof(lineData));
        lineData.id = itr.id;
        lineData.textId = itr.textId;
        lineData.unk1 = itr.unk1;
        lineData.unk2 = itr.unk2;
        SetLine(&lineData, itr.idx, true);
    }

    return true;
}
