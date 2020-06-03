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
#include "DatabaseEnv.h"
#include "GridNotifiers.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "ScriptMgr.h"
#include "UpdateFieldFlags.h"

DynamicObject::DynamicObject(bool isWorldObject) : WorldObject(isWorldObject), _aura(nullptr), _removedAura(nullptr), _caster(nullptr), _duration(0), _isViewpoint(false)
{
    m_objectType |= TYPEMASK_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;

    m_updateFlag = UPDATEFLAG_STATIONARY_POSITION;

    m_valuesCount = DYNAMIC_OBJECT_END;
    _dynamicValuesCount = DYNAMICOBJECT_DYNAMIC_END;

    _fieldNotifyFlags = UF_FLAG_DYNAMIC;
    objectCountInWorld[uint8(HighGuid::DynamicObject)]++;
}

void DynamicObject::BuildValuesUpdate(uint8 updateType, ByteBuffer* data, Player* target) const
{
    if (!target)
        return;

    auto const& caster = GetCaster();

    std::size_t blockCount = UpdateMask::GetBlockCount(m_valuesCount);

    uint32* flags = DynamicObjectUpdateFieldFlags;
    uint32 visibleFlag = UF_FLAG_PUBLIC;
    if (GetCasterGUID() == target->GetGUID())
        visibleFlag |= UF_FLAG_OWNER;

    *data << uint8(blockCount);
    std::size_t maskPos = data->wpos();
    data->resize(data->size() + blockCount * sizeof(UpdateMask::BlockType));

    for (uint16 index = 0; index < m_valuesCount; ++index)
    {
        if (_fieldNotifyFlags & flags[index] || ((updateType == UPDATETYPE_VALUES ? _changesMask[index] : m_uint32Values[index]) && (flags[index] & visibleFlag)))
        {
            UpdateMask::SetUpdateBit(data->contents() + maskPos, index);

            if (index == AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID)
            {
                uint32 visualId = GetVisualId();
                if (auto hostilSpellVisualID = sDB2Manager.GetHostileSpellVisualId(visualId))
                    if (caster && hostilSpellVisualID && caster->IsHostileTo(target))
                        visualId = hostilSpellVisualID;

                *data << visualId;
            }
            else
                *data << m_uint32Values[index];                // other cases
        }
    }
}

DynamicObject::~DynamicObject()
{
    // make sure all references were properly removed
    ASSERT(!_aura);
    ASSERT(!_caster);
    ASSERT(!_isViewpoint);
    objectCountInWorld[uint8(HighGuid::DynamicObject)]--;
}

void DynamicObject::AddToWorld()
{
    ///- Register the dynamicObject for guid lookup and for caster
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

void DynamicObject::RemoveFromWorld()
{
    ///- Remove the dynamicObject from the accessor and from all lists of objects in world
    if (IsInWorld())
    {
        if (_isViewpoint)
            RemoveCasterViewpoint();

        if (_aura)
            RemoveAura();

        // dynobj could get removed in Aura::RemoveAura
        if (!IsInWorld())
            return;

        UnbindFromCaster();
        WorldObject::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

bool DynamicObject::CreateDynamicObject(ObjectGuid::LowType guidlow, Unit* caster, uint32 spellId, Position const& pos, float radius, DynamicObjectType type)
{
    SetMap(caster->GetMap());
    Relocate(pos);
    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "DynamicObject (spell %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", spellId, GetPositionX(), GetPositionY());
        return false;
    }

    WorldObject::_Create(ObjectGuid::Create<HighGuid::DynamicObject>(GetMapId(), spellId, guidlow));
    SetPhaseMask(caster->GetPhaseMask(), false);

    SetEntry(spellId);
    SetObjectScale(1.0f);
    SetGuidValue(DYNAMICOBJECT_FIELD_CASTER, caster->GetGUID());

    SetUInt32Value(DYNAMICOBJECT_FIELD_TYPE, type);

    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
        SetUInt32Value(DYNAMICOBJECT_FIELD_SPELL_XSPELL_VISUAL_ID, spellInfo->GetSpellXSpellVisualId(caster));

    SetUInt32Value(DYNAMICOBJECT_FIELD_SPELL_ID, spellId);
    SetFloatValue(DYNAMICOBJECT_FIELD_RADIUS, G3D::fuzzyEq(radius, 0.0f) ? 1.0f : radius);
    SetUInt32Value(DYNAMICOBJECT_FIELD_CAST_TIME, getMSTime());

    if (IsWorldObject())
        setActive(true);    //must before add to map to be put in world container

    Transport* transport = caster->GetTransport();
    if (transport)
    {
        float x, y, z, o;
        pos.GetPosition(x, y, z, o);
        transport->CalculatePassengerOffset(x, y, z, &o);
        m_movementInfo.transport.Pos.Relocate(x, y, z, o);

        // This object must be added to transport before adding to map for the client to properly display it
        transport->AddPassenger(this);
    }

    if (!GetMap()->AddToMap(this))
    {
        // Returning false will cause the object to be deleted - remove from transport
        if (transport)
            transport->RemovePassenger(this);
        return false;
    }

    return true;
}

void DynamicObject::Update(uint32 p_time)
{
    if (m_isUpdate)
        return;

    m_isUpdate = true;

    bool expired = false;

    if (_aura)
    {
        if (!_aura->IsRemoved())
            _aura->UpdateOwner(p_time, this);

        // _aura may be set to null in Aura::UpdateOwner call
        if (_aura && (_aura->IsRemoved() || _aura->IsExpired()))
            expired = true;
    }
    else
    {
        if (GetDuration() > int32(p_time))
            _duration -= p_time;
        else
            expired = true;
    }

    if (expired)
        Remove();

    m_isUpdate = false;
}

void DynamicObject::Remove()
{
    if (IsInWorld())
    {
        RemoveFromWorld();
        AddObjectToRemoveList();
    }
}

int32 DynamicObject::GetDuration() const
{
    if (!_aura)
        return _duration;
    return _aura->GetDuration();
}

void DynamicObject::SetDuration(int32 newDuration)
{
    if (!_aura)
        _duration = newDuration;
    else
        _aura->SetDuration(newDuration);
}

void DynamicObject::Delay(int32 delaytime)
{
    SetDuration(GetDuration() - delaytime);
}

void DynamicObject::SetAura(Aura* aura)
{
    ASSERT(!_aura && aura);
    _aura = aura;
}

void DynamicObject::RemoveAura()
{
    ASSERT(_aura && !_removedAura);
    _removedAura = _aura;
    _aura = nullptr;
    if (!_removedAura->IsRemoved())
        _removedAura->_Remove(AURA_REMOVE_BY_DEFAULT);
}

void DynamicObject::SetCasterViewpoint()
{
    if (Player* caster = _caster->ToPlayer())
    {
        caster->SetViewpoint(this, true);
        _isViewpoint = true;
    }
}

void DynamicObject::RemoveCasterViewpoint()
{
    if (Player* caster = _caster->ToPlayer())
    {
        caster->SetViewpoint(this, false);
        _isViewpoint = false;
    }
}

void DynamicObject::BindToCaster()
{
    ASSERT(!_caster);
    _caster = ObjectAccessor::GetUnit(*this, GetCasterGUID());
    ASSERT(_caster);
    ASSERT(_caster->GetMap() == GetMap());
    _caster->_RegisterDynObject(this);
}

void DynamicObject::UnbindFromCaster()
{
    ASSERT(_caster);
    _caster->_UnregisterDynObject(this);
    _caster = nullptr;
}
