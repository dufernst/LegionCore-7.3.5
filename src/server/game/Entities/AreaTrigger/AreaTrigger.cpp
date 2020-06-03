/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "AreaTriggerData.h"
#include "GridNotifiers.h"
#include "MoveSpline.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellPackets.h"
#include "Spline.h"
#include "UpdateFieldFlags.h"
#include "Vehicle.h"
#include "VMapFactory.h"

AreaTriggerScaleData::AreaTriggerScaleData()
{
    timeToTargetScale = 0;

    memset(OverrideScale, 0, sizeof(OverrideScale));
    memset(ExtraScale, 0, sizeof(ExtraScale));

    ExtraScale[5].floatValue = 1.0f;
    ExtraScale[6].integerValue = 1;
}

AreaTriggerInfo::AreaTriggerInfo() : Radius(0.0f), RadiusTarget(0.0f), RandomRadiusOfSpawn(0.0f), LocationZOffset(0.0f),
LocationZOffsetTarget(0.0f), windX(0.0f), windY(0.0f), windZ(0.0f), windSpeed(0.0f), RollPitchYaw1X(0.0f), RollPitchYaw1Y(0.0f), RollPitchYaw1Z(0.0f),
TargetRollPitchYawX(0.0f), TargetRollPitchYawY(0.0f), TargetRollPitchYawZ(0.0f), Distance(0.0f),
Speed(0.0f), RePatchSpeed(0.0f), AngleToCaster(0.0f), AnglePointA(0.0f), AnglePointB(0.0f), Param(0.0f), spellId(0),
DecalPropertiesId(0), activationDelay(0), updateDelay(0), customEntry(0), moveType(0), waitTime(0),
hitType(0), MoveCurveID(0), ElapsedTime(0), MorphCurveID(0), FacingCurveID(0), ScaleCurveID(0),
HasFollowsTerrain(0), HasAttached(0), HasAbsoluteOrientation(0), HasDynamicShape(0), HasFaceMovementDir(0), hasAreaTriggerBox(0), windType(0),
polygon(0), VisualID(0), maxCount(0), maxActiveTargets(0), isMoving(false), RePatch(false), OnDestinationReachedDespawn(true), WithObjectSize(false),
AliveOnly(true), AllowBoxCheck(false)
{
}

AreaTrigger::ActionInfo::ActionInfo() : hitCount(0), charges(0), recoveryTime(0), amount(0), onDespawn(false), action(nullptr)
{
}

AreaTrigger::ActionInfo::ActionInfo(AreaTriggerAction const* _action) : hitCount(0), charges(_action->maxCharges), recoveryTime(0), amount(_action->amount), onDespawn(_action->onDespawn), action(_action)
{
}

AreaTrigger::AreaTrigger() : WorldObject(false), _range(0.0f), m_CastItem(nullptr), m_aura(nullptr), _caster(nullptr), _duration(0), _activationDelay(0), _updateDelay(0), _scaleDelay(0), _sequenceDelay(0), _sequenceStep(0),
_liveTime(0), _radius(1.0f), _realEntry(0), _reachedDestination(false), _lastSplineIndex(0), _movementTime(0), _nextMoveTime(0), _waitTime(0), _on_unload(false), _on_despawn(false), _on_remove(false), _hitCount(1),
_areaTriggerCylinder(false), _canMove(false), _currentWP(0), movespline(new Movement::MoveSpline()), m_spellInfo(nullptr), m_spell(nullptr), m_withoutCaster(false)
{
    m_objectType |= TYPEMASK_AREATRIGGER;
    m_objectTypeId = TYPEID_AREATRIGGER;

    m_updateFlag = UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_AREA_TRIGGER;

    m_valuesCount = AREA_TRIGGER_END;
    m_moveAngleLos = 0.0f;
    _dynamicValuesCount = AREATRIGGER_DYNAMIC_END;

    _CircleData = nullptr;

    _spline.VerticesPoints.clear();
    _spline.TimeToTarget = 0;
    _spline.ElapsedTimeForMovement = 0;
    objectCountInWorld[uint8(HighGuid::AreaTrigger)]++;
}

void AreaTrigger::BuildValuesUpdate(uint8 updateType, ByteBuffer* data, Player* target) const
{
    if (!target)
        return;

    auto const& caster = GetCaster();

    std::size_t blockCount = UpdateMask::GetBlockCount(m_valuesCount);

    uint32* flags = AreaTriggerUpdateFieldFlags;
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
                uint32 visualId = m_uint32Values[index];
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

AreaTrigger::~AreaTrigger()
{
    objectCountInWorld[uint8(HighGuid::AreaTrigger)]--;
    delete movespline;
    delete _CircleData;
}

void AreaTrigger::AddToWorld()
{
    ///- Register the AreaTrigger for guid lookup and for caster
    if (!IsInWorld())
    {
        if (m_withoutCaster || ObjectAccessor::GetUnit(*this, GetCasterGUID())) // Check caster before add to world
        {
            sObjectAccessor->AddObject(this);
            WorldObject::AddToWorld();
            if (!m_withoutCaster)
                BindToCaster();
        }
    }
}

void AreaTrigger::RemoveFromWorld()
{
#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::RemoveFromWorld Entry: %u CustomEntry: %u", GetRealEntry(), GetCustomEntry());
#endif

    ///- Remove the AreaTrigger from the accessor and from all lists of objects in world
    if (IsInWorld())
    {
        if (!DebugPolygon.empty())
        {
            for (auto& creature : DebugPolygon)
            {
                if (!creature || !creature->IsInWorld())
                    continue;
                creature->DespawnOrUnsummon();
            }
            DebugPolygon.clear();
        }
        _ai->OnRemove();
        UnbindFromCaster();
        WorldObject::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

bool AreaTrigger::CreateAreaTrigger(ObjectGuid::LowType guidlow, uint32 triggerEntry, Unit* caster, SpellInfo const* info, Position const& pos, Position const& posMove,
    Spell* spell /*= nullptr*/, ObjectGuid targetGuid /*= ObjectGuid::Empty*/, uint32 customEntry /*= 0*/, ObjectGuid castID /*= ObjectGuid::Empty*/, Map* map /* = nullptr */)
{
    m_spellInfo = info;
    m_spell = spell;
    m_withoutCaster = !caster;

    // if (!info)
    // {
        // TC_LOG_ERROR(LOG_FILTER_GENERAL, "AreaTrigger (entry %u) caster %s no spellInfo", triggerEntry, caster->ToString().c_str());
        // return false;
    // }

    // Caster not in world, might be spell triggered from aura removal
    if (caster && !caster->IsInWorld())
    {
        // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::CreateAreaTrigger guid %u, GetEntry() %u IsInWorld %u", caster->GetGUIDLow(), caster->GetEntry(), caster->IsInWorld());
        return false;
    }

    // if (!caster->isAlive())
    // {
        // TC_LOG_ERROR(LOG_FILTER_GENERAL, "AreaTrigger (spell %u) caster %s is dead ", info ? info->Id : 0, caster->ToString().c_str());
        // return false;
    // }

    SetMap(caster ? caster->GetMap() : map);
    Relocate(pos);
    SetOrientation(pos.GetOrientation());
    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "AreaTrigger (spell %u) not created. Invalid coordinates (X: %f Y: %f)", info ? info->Id : 0, GetPositionX(), GetPositionY());
        return false;
    }

    AreaTriggerInfo const* infoAt = nullptr;
    if (triggerEntry)
        infoAt = sAreaTriggerDataStore->GetAreaTriggerInfo(triggerEntry);
    if (customEntry)
        infoAt = sAreaTriggerDataStore->GetAreaTriggerInfoByEntry(customEntry);

    if (infoAt)
    {
        atInfo = *infoAt;

        if (atInfo.RandomRadiusOfSpawn) // it's needed for spawn in random point near original position
        {
            Position mypos = pos;
            GetNearPosition(mypos, frand(0, atInfo.RandomRadiusOfSpawn), frand(0, 6.28f));
            Relocate(mypos);
        }

        _activationDelay = atInfo.activationDelay;

        for (AreaTriggerActionList::const_iterator itr = atInfo.actions.begin(); itr != atInfo.actions.end(); ++itr)
            _actionInfo[itr->id] = ActionInfo(&*itr);
    }
    else
        return false;

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger (spell %u) coordinates (X: %f Y: %f) _actionInfo %u", info ? info->Id : 0, GetPositionX(), GetPositionY(), _actionInfo.size());
#endif

    Object::_Create(ObjectGuid::Create<HighGuid::AreaTrigger>(GetMapId(), atInfo.customEntry, guidlow));
    if (caster)
    {
        SetPhaseMask(caster->GetPhaseMask(), false);
        SetPhaseId(caster->GetPhases(), false);
    }

    if (spell && !spell->CanDestoyCastItem())
        m_CastItem = spell->m_CastItem;

    _realEntry = triggerEntry;
    SetEntry(atInfo.customEntry);
    int32 duration = info && caster ? info->GetDuration(caster->GetSpawnMode()) : -1;
    if (!duration)
        duration = -1;

    _canMove = atInfo.isMoving;
    if (caster)
    {
        Player* modOwner = caster->GetSpellModOwner();
        if (duration != -1 && modOwner && info)
        {
            modOwner->ApplySpellMod(info->Id, SPELLMOD_DURATION, duration);
            if (info->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION_RECOVERY))
                duration *= caster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE);
        }
    }

    SetDuration(duration);
    SetObjectScale(1);

    if (targetGuid && atInfo.HasAttached)
        m_movementInfo.transport.Guid = targetGuid;
    else if (Transport* transport = caster ? caster->GetTransport() : nullptr)
    {
        float x, y, z, o;
        pos.GetPosition(x, y, z, o);
        transport->CalculatePassengerOffset(x, y, z, &o);
        m_movementInfo.transport.Pos.Relocate(x, y, z, o);

        // This object must be added to transport before adding to map for the client to properly display it
        transport->AddPassenger(this);
    }

    AI_Initialize();

    //if(atInfo.HasAbsoluteOrientation)
        //SetOrientation(0.0f);
    // on some spells radius set on dummy aura, not on create effect.
    // overwrite by radius from spell if exist.

    SetGuidValue(AREATRIGGER_FIELD_CASTER, caster ? caster->GetGUID() : ObjectGuid::Empty);

    CalculateRadius(spell);

    if (atInfo.Polygon.Height && !atInfo.polygon)
        _areaTriggerCylinder = true;

    SetGuidValue(AREATRIGGER_FIELD_CREATING_EFFECT_GUID, castID);
    if (info)
    {
        uint32 _infoId = info->Id;

        //Custom visual
        if (atInfo.VisualID < 0)
            _infoId = abs(atInfo.VisualID);

        SetUInt32Value(AREATRIGGER_FIELD_SPELL_ID, info->Id);
        SetUInt32Value(AREATRIGGER_FIELD_SPELL_FOR_VISUALS, _infoId);
        SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, info->GetSpellXSpellVisualId(caster));
    }
    else
    {
        if (SpellInfo const* spellInfo_ = sSpellMgr->GetSpellInfo(atInfo.VisualID))
        {
            SetUInt32Value(AREATRIGGER_FIELD_SPELL_FOR_VISUALS, spellInfo_->Id);
            SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, spellInfo_->GetSpellXSpellVisualId(caster));
        }
        else
        {
            for (auto const& v : sSpellXSpellVisualStore)
                if (v->SpellVisualID == atInfo.VisualID)
                    SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, v->ID);
        }
    }

    SetFloatValue(AREATRIGGER_FIELD_BOUNDS_RADIUS_2_D, atInfo.Radius);

    SetUInt32Value(AREATRIGGER_FIELD_EXTRA_SCALE_CURVE + 5, 1065353217);
    SetUInt32Value(AREATRIGGER_FIELD_EXTRA_SCALE_CURVE + 6, 1);

    if (atInfo.DecalPropertiesId)
        SetUInt32Value(AREATRIGGER_FIELD_DECAL_PROPERTIES_ID, atInfo.DecalPropertiesId);
    SetTargetGuid(targetGuid);

    if (GetSpellInfo())
        _range = GetSpellInfo()->GetMaxRange() < _radius ? _radius : GetSpellInfo()->GetMaxRange(); //If spline not set client crash, set default to 15m
    else
        _range = _radius;

    if (atInfo.Distance != 0.0f)
        _range = atInfo.Distance;

    if (_range > 533.0f)
        _range = 250.0f;

    _ai->CalculateDuration(_duration);

    if (info && (GetCustomEntry() == 13251 && (caster && caster->IsCreature() && caster->GetEntry() != 116407) || info->Id == 233530)) //! Hack!!!, because it is so stupid and don't work else
        _canMove = false;

    // culculate destination point
    if (isMoving() && caster)
        CalculateSplinePosition(pos, posMove, caster);

    if (_duration > 0)
    {
        SetUInt32Value(AREATRIGGER_FIELD_DURATION, _duration);
        SetUInt32Value(AREATRIGGER_FIELD_TIME_TO_TARGET_SCALE, _duration);
    }
    if (_spline.TimeToTarget)
        SetUInt32Value(AREATRIGGER_FIELD_TIME_TO_TARGET, _spline.TimeToTarget);

    if (caster)
        FillCustomData(caster);

    if (!GetMap()->AddToMap(this))
        return false;

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::Create AreaTrigger caster %s spellID %u spell rage %f dist %f dest - X:%f,Y:%f,Z:%f _nextMoveTime %i duration %i",
        caster ? caster->GetGUID().ToString().c_str() : ObjectGuid::Empty.ToString().c_str(), info ? info->Id : 0, _radius, GetSpellInfo() ? GetSpellInfo()->GetMaxRange() : 0.0f, _spline.VerticesPoints.empty() ? 0.0f : _spline.VerticesPoints[0].x, _spline.VerticesPoints.empty() ? 0.0f : _spline.VerticesPoints[0].y, _spline.VerticesPoints.empty() ? 0.0f : _spline.VerticesPoints[0].z, _nextMoveTime, duration);
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::Create isMoving %i _range %f _spline.VerticesPoints %i moveType %i", isMoving(), _range, _spline.VerticesPoints.size(), atInfo.moveType);
#endif

    if (atInfo.maxCount && info && caster)
    {
        std::list<AreaTrigger*> oldTriggers;
        caster->GetAreaObjectList(oldTriggers, info->Id);
        oldTriggers.sort(Trinity::GuidValueSorterPred());
        while (oldTriggers.size() > atInfo.maxCount)
        {
            AreaTrigger* at = oldTriggers.front();
            oldTriggers.remove(at);
            if (at->GetCasterGUID() == caster->GetGUID())
                at->Remove(false);
        }
    }
    UpdateAffectedList(0, AT_ACTION_MOMENT_SPAWN);

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger end (spell %u) coordinates (X: %f Y: %f)", info ? info->Id : 0, GetPositionX(), GetPositionY());
    DebugVisualizePolygon();
#endif

    _ai->OnCreate();

    return true;
}

void AreaTrigger::FillCustomData(Unit* caster)
{
    if (GetCustomEntry())
        SetEntry(GetCustomEntry());

    switch (GetSpellId())
    {
        case 191034: // Starfall
        {
            if (caster->HasAura(200726)) // Celestial Downpour (PvP Talent)
                atInfo.maxCount = 1;
            else
                atInfo.maxCount = 3;
            break;
        }
        case 202770: // Fury of Elune
        {
            SetFloatValue(AREATRIGGER_FIELD_OVERRIDE_SCALE_CURVE + 6, -0.005729166f);
            SetFloatValue(AREATRIGGER_FIELD_EXTRA_SCALE_CURVE + 5, 1.0f);
            SetFloatValue(AREATRIGGER_FIELD_EXTRA_SCALE_CURVE + 6, -0.005729166f);
            SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, 55664);
            break;
        }
        case 192753: //Eye of Azshara: Tidal Wave
        {
            _scaleData.OverrideScale[0].integerValue = _liveTime;
            _scaleData.OverrideScale[1].floatValue = 0.0f;
            _scaleData.OverrideScale[2].floatValue = 2.0f;
            _scaleData.OverrideScale[3].floatValue = 1.0f;
            _scaleData.OverrideScale[4].floatValue = 2.0f;
            _scaleData.OverrideScale[5].integerValue = 0x10000000;
            _scaleData.OverrideScale[6].integerValue = 1;
            _scaleDelay = 100;
            break;
        }
        case 143961:    //OO: Defiled Ground
            //ToDo: should cast only 1/4 of circle
            SetSpellId(143960);
            SetDuration(-1);
            _radius = 8.0f;
            //infrontof
            break;
        case 166539:    //WOD: Q34392
        {
            m_movementInfo.transport.Pos.Relocate(0, 0, 0);
            m_movementInfo.transport.MoveTime = 0;
            m_movementInfo.transport.VehicleSeatIndex = 64;
            m_movementInfo.transport.Guid = caster->GetGUID();
            m_movementInfo.transport.VehicleRecID = 0;

            caster->SetAnimKitId(6591);
            break;
        }
        case 201591: // Stampede
        case 201610: // Stampede
        {
            switch (urand(0, 5))
            {
                case 0:
                    SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, 59432);
                    break;
                case 1:
                    SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, 59434);
                    break;
                case 2:
                    SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, 59435);
                    break;
                case 3:
                    SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, 59436);
                    break;
                case 4:
                    SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, 59437);
                    break;
                case 5:
                    SetUInt32Value(AREATRIGGER_FIELD_SPELL_XSPELL_VISUAL_ID, 59439);
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

void AreaTrigger::UpdateAffectedList(uint32 /*p_time*/, AreaTriggerActionMoment actionM)
{
    if (atInfo.actions.empty() || !IsInWorld())
        return;

    if (actionM & AT_ACTION_MOMENT_ENTER)
    {
        for (GuidList::iterator itr = affectedPlayers.begin(), next; itr != affectedPlayers.end(); itr = next)
        {
            next = itr;
            ++next;

            Unit* unit = ObjectAccessor::GetUnit(*this, *itr);
            if (!unit || !unit->isAlive())
            {
                AffectUnitLeave(AT_ACTION_MOMENT_LEAVE);

                if (unit)
                    _ai->OnUnitExit(unit);

                affectedPlayers.erase(itr);
                if (affectedPlayers.empty())
                    AffectUnit(unit, AT_ACTION_MOMENT_LEAVE_ALL);
                continue;
            }

            if (!IsInArea(unit))
            {
                affectedPlayers.erase(itr);
                AffectUnit(unit, AT_ACTION_MOMENT_LEAVE);
                _ai->OnUnitExit(unit);
                if (affectedPlayers.empty())
                    AffectUnit(unit, AT_ACTION_MOMENT_LEAVE_ALL);
            }
        }

        if (atInfo.maxActiveTargets && affectedPlayers.size() >= atInfo.maxActiveTargets)
            return;

        std::list<Unit*> unitList;

        if (!_ai->CreateUnitList(unitList))
            GetAttackableUnitListInRange(unitList, GetRadius(), atInfo.AliveOnly);

        for (auto& unit : unitList)
        {
            if (!IsUnitAffected(unit->GetGUID()))
            {
                if (!IsInArea(unit))
                    continue;

                if (!CheckValidateTargets(unit, actionM))
                {
                    // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::UpdateAffectedList !CheckValidateTargets GetGUID %s", unit->GetGUID().ToString().c_str());
                    continue;
                }

                affectedPlayers.push_back(unit->GetGUID());
                AffectUnit(unit, actionM);
                _ai->OnUnitEnter(unit);

                if (atInfo.maxActiveTargets && affectedPlayers.size() >= atInfo.maxActiveTargets)
                    break;
            }
        }
    }
    else
    {
        for (GuidList::iterator itr = affectedPlayers.begin(), next; itr != affectedPlayers.end(); itr = next)
        {
            next = itr;
            ++next;

            Unit* unit = ObjectAccessor::GetUnit(*this, *itr);

            if (unit && actionM == AT_ACTION_MOMENT_REMOVE)
                _ai->BeforeRemove(unit);

            if (!unit || !unit->isAlive())
            {
                AffectUnitLeave(AT_ACTION_MOMENT_LEAVE);
                affectedPlayers.erase(itr);
                continue;
            }

            AffectUnit(unit, actionM);

            if (actionM == AT_ACTION_MOMENT_REMOVE)
                affectedPlayers.erase(itr);
        }

        AffectOwner(actionM);
    }
}

void AreaTrigger::UpdateActionCharges(uint32 p_time)
{
    for (auto & itr : _actionInfo)
    {
        ActionInfo& info = itr.second;
        if (!info.charges || !info.action->chargeRecoveryTime)
            continue;
        if (info.charges >= info.action->maxCharges)
            continue;

        info.recoveryTime += p_time;
        if (info.recoveryTime >= info.action->chargeRecoveryTime)
        {
            info.recoveryTime -= info.action->chargeRecoveryTime;
            ++info.charges;
            if (info.charges == info.action->maxCharges)
                info.recoveryTime = 0;
        }
    }
}

bool AreaTrigger::GetAreaTriggerCylinder() const
{
    return _areaTriggerCylinder;
}

bool AreaTrigger::HasTargetRollPitchYaw() const
{
    return atInfo.TargetRollPitchYawX != 0.0f || atInfo.TargetRollPitchYawY != 0.0f || atInfo.TargetRollPitchYawZ != 0.0f;
}

bool AreaTrigger::HasPolygon() const
{
    return atInfo.polygon && !atInfo.hasAreaTriggerBox && !atInfo.Polygon.Vertices.empty();
}

bool AreaTrigger::HasCircleData() const
{
    return _CircleData != nullptr;
}

AreaTriggerCircle const* AreaTrigger::GetCircleData() const
{
    return _CircleData;
}

void AreaTrigger::Update(uint32 p_time)
{
    if (_on_remove || m_isUpdate)
        return;

    m_isUpdate = true;

    _liveTime += p_time;
    //TMP. For debug info.
    //uint32 spell = GetSpellId();

    UpdateActionCharges(p_time);
    UpdateSplinePosition(p_time);
    UpdateRotation(p_time);
    UpdateScale(p_time);
    UpdateSequence(p_time);

    if (GetDuration() != -1)
    {
        if (GetDuration() > int32(p_time))
            _duration -= p_time;
        else
        {
            Remove(!_on_despawn); // expired
            m_isUpdate = false;
            return;
        }
    }

    if (_activationDelay)
    {
        if (_activationDelay > p_time)
            _activationDelay -= p_time;
        else
        {
            _activationDelay = 0;
            UpdateAffectedList(p_time, AT_ACTION_MOMENT_ON_ACTIVATE);
        }
    }
    else
        UpdateAffectedList(p_time, AT_ACTION_MOMENT_ENTER);

    if (atInfo.updateDelay)
    {
        if (_updateDelay > p_time)
            _updateDelay -= p_time;
        else
        {
            if (_caster && m_spellInfo && m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME))
                _updateDelay = int32(atInfo.updateDelay * _caster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE));
            else
                _updateDelay = atInfo.updateDelay;
            ActionOnUpdate(p_time);
        }
    }

    _ai->OnUpdate(p_time);
    m_isUpdate = false;
}

bool AreaTrigger::IsUnitAffected(ObjectGuid guid) const
{
    return std::find(affectedPlayers.begin(), affectedPlayers.end(), guid) != affectedPlayers.end();
}

void AreaTrigger::AffectUnit(Unit* unit, AreaTriggerActionMoment actionM)
{
    for (auto & itr : _actionInfo)
    {
        ActionInfo& info = itr.second;
        if (!(info.action->moment & actionM))
            continue;

        DoAction(unit, info);
        // if(unit != _caster)
            // AffectOwner(actionM);//action if action on area trigger
    }
}

void AreaTrigger::AffectUnitLeave(AreaTriggerActionMoment actionM)
{
    for (auto & itr : _actionInfo)
    {
        ActionInfo& info = itr.second;
        if (!(info.action->moment & actionM))
            continue;

        DoActionLeave(info);
    }
}

void AreaTrigger::AffectOwner(AreaTriggerActionMoment actionM)
{
    for (auto & itr : _actionInfo)
    {
        ActionInfo& info = itr.second;
        if (!(info.action->targetFlags & AT_TARGET_FLAG_ALWAYS_TRIGGER))
            continue;
        if (!(info.action->moment & actionM))
            continue;

        DoAction(_caster, info);
    }
}

void AreaTrigger::ActionOnUpdate(uint32 /*p_time*/)
{
#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "ActionOnUpdate");
#endif

    // Action for AOE spell with cast on dest
    for (auto & itr : _actionInfo)
    {
        ActionInfo& info = itr.second;
        if (!(info.action->moment & AT_ACTION_MOMENT_UPDATE))
            continue;

        DoAction(_caster, info);
    }

    // Action for single target spell
    for (GuidList::iterator itr = affectedPlayers.begin(), next; itr != affectedPlayers.end(); itr = next)
    {
        next = itr;
        ++next;

        Unit* unit = ObjectAccessor::GetUnit(*this, *itr);
        if (!unit)
            continue;

        for (auto & iter : _actionInfo)
        {
            ActionInfo& info = iter.second;
            if (!(info.action->moment & AT_ACTION_MOMENT_UPDATE_TARGET))
                continue;

            DoAction(unit, info);
        }
    }

    _ai->ActionOnUpdate(affectedPlayers);
}

void AreaTrigger::ActionOnDespawn()
{
    for (auto & itr : _actionInfo)
    {
        ActionInfo& info = itr.second;
        if (!(info.action->moment & AT_ACTION_MOMENT_ON_DESPAWN))
            continue;

        DoAction(_caster, info);
    }
}

bool AreaTrigger::_HasActionsWithCharges(AreaTriggerActionMoment action /*= AT_ACTION_MOMENT_ENTER*/)
{
    for (auto & itr : _actionInfo)
    {
        ActionInfo& info = itr.second;
        if (info.action->moment & action)
        {
            if (info.action->auraCaster > 0 && !_caster->HasAura(info.action->auraCaster))
                continue;
            if (info.action->auraCaster < 0 && _caster->HasAura(abs(info.action->auraCaster)))
                continue;
            if (info.action->hasspell > 0 && !_caster->HasSpell(info.action->hasspell))
                continue;
            if (info.action->hasspell < 0 && _caster->HasSpell(abs(info.action->hasspell)))
                continue;

            if (info.charges || !info.action->maxCharges)
                return true;
        }
    }
    return false;
}

void AreaTrigger::DoAction(Unit* unit, ActionInfo& action)
{
    Unit* caster = _caster;
    if (!caster && m_withoutCaster)
        caster = unit;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(action.action->spellId);
    // if (!spellInfo)
        // return;

    // do not process depleted actions
    if (!caster || !unit || !action.charges && action.action->maxCharges)
        return;

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::DoAction caster %s unit %s type %u spellID %u, moment %u, targetFlags %u",
        caster->GetGUID().ToString().c_str(), unit->GetGUID().ToString().c_str(), action.action->actionType, action.action->spellId, action.action->moment, action.action->targetFlags);
#endif

    if (action.action->targetFlags & AT_TARGET_FLAG_FRIENDLY)
        if (!caster || !caster->IsFriendlyTo(unit) || unit->isTotem())
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_HOSTILE)
        if (!caster || !caster->IsHostileTo(unit))
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_VALIDATTACK)
        if (!caster || !caster->IsValidAttackTarget(unit) || unit->isTotem())
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_NOT_IN_LOS)
        if (!IsWithinLOSInMap(unit))
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_NOT_IN_LOS_Z)
        if (!IsWithinLOS(GetPositionX(), GetPositionY(), unit->GetPositionZ()))
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_OWNER)
        if (unit->GetGUID() != GetCasterGUID())
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_PLAYER)
        if (!unit->IsPlayer())
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_NOT_PET)
        if (unit->isPet())
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_NOT_FULL_HP)
        if (unit->IsFullHealth())
            return;
    if (action.action->targetFlags & AT_TARGET_FLAG_GROUP_OR_RAID)
        if (!unit->IsInRaidWith(caster) && !unit->IsInPartyWith(caster))
            return;

    // should cast on self.
    if (spellInfo && (spellInfo->EffectMask & (1 << EFFECT_0)) != 0)
        if ((spellInfo->Effects[EFFECT_0]->TargetA.GetTarget() == TARGET_UNIT_CASTER
            || action.action->targetFlags & AT_TARGET_FLAG_CASTER_IS_TARGET) && !(action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_CASTER_2))
            caster = unit;

    if (action.action->targetFlags & AT_TARGET_FLAG_CASTER_AURA_TARGET)
        if (Unit* target = ObjectAccessor::GetUnit(*this, GetTargetGuid()))
            caster = target;

    //action on self
    if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_CASTER)
        unit = _caster;

    if (action.action->targetFlags & AT_TARGET_FLAG_NOT_AURA_TARGET)
        if (GetTargetGuid() && GetTargetGuid() == unit->GetGUID())
            return;

    if (action.action->targetFlags & AT_TARGET_FLAG_CAST_AURA_TARGET)
        if (Unit* target = ObjectAccessor::GetUnit(*this, GetTargetGuid()))
            unit = target;

    if (action.action->targetFlags & AT_TARGET_FLAT_IN_FRONT)
        if (!HasInArc(static_cast<float>(M_PI), unit))
            return;

    if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_SUMMONER)
        if (Unit* summoner = _caster->GetAnyOwner())
            if (unit->GetGUID() != summoner->GetGUID())
                return;

    if (action.action->targetFlags & AT_TARGET_FLAG_NOT_OWNER)
        if (unit->GetGUID() == GetCasterGUID())
            return;

    if (action.action->targetFlags & AT_TARGET_FLAG_NPC_ENTRY)
        if (action.amount != unit->GetEntry() || unit->IsPlayer())
            return;

    if (action.action->targetFlags & AT_TARGET_FLAG_SCRIPT)
        if (!_ai->IsValidTarget(_caster, unit, action.action->moment))
            return;

    if (action.action->hasAura > 0 && !unit->HasAura(action.action->hasAura))
        return;
    if (action.action->hasAura < 0 && unit->HasAura(abs(action.action->hasAura)))
        return;

    if (action.action->hasAura2 > 0 && !unit->HasAura(action.action->hasAura2))
        return;
    if (action.action->hasAura2 < 0 && unit->HasAura(abs(action.action->hasAura2)))
        return;

    if (action.action->hasAura3 > 0 && !unit->HasAura(action.action->hasAura3))
        return;
    if (action.action->hasAura3 < 0 && unit->HasAura(abs(action.action->hasAura3)))
        return;

    if (action.action->auraCaster > 0 && !_caster->HasAura(action.action->auraCaster))
        return;
    if (action.action->auraCaster < 0 && _caster->HasAura(abs(action.action->auraCaster)))
        return;

    if (action.action->hasspell > 0 && !_caster->HasSpell(action.action->hasspell))
        return;
    if (action.action->hasspell < 0 && _caster->HasSpell(abs(action.action->hasspell)))
        return;

    if (action.action->minDistance && GetDistance(unit) > action.action->minDistance)
        return;
     
    if (!CheckActionConditions(*action.action, unit))
        return;

    if (spellInfo && (action.action->targetFlags & AT_TARGET_FLAG_NOT_FULL_ENERGY))
    {
        Powers energeType = POWER_NULL;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->EffectMask < uint32(1 << i))
                break;

            if (spellInfo->Effects[i]->Effect == SPELL_EFFECT_ENERGIZE)
                energeType = Powers(spellInfo->Effects[i]->MiscValue);
        }

        if (energeType == POWER_NULL || unit->GetMaxPower(energeType) == 0 || unit->GetMaxPower(energeType) == unit->GetPower(energeType))
            return;
    }

    if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_PASSANGER)
    {
        if (Vehicle* veh = unit->GetVehicleKit())
        {
            for (uint8 i = 0; i < MAX_VEHICLE_SEATS; i++)
                if (Unit* pass = veh->GetPassenger(i))
                    unit = pass;
        }
    }

    if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_PASSANGER_VEH)
    {
        if (Vehicle* veh = unit->GetVehicleKit())
        {
            for (uint8 i = 0; i < MAX_VEHICLE_SEATS; i++)
                if (Unit* pass = veh->GetPassenger(i))
                {
                    if (Vehicle* vehPas = pass->GetVehicleKit())
                    {
                        bool empty = true;
                        for (uint8 j = 0; j < MAX_VEHICLE_SEATS; j++)
                            if (vehPas->GetPassenger(j))
                            {
                                empty = false;
                                break;
                            }
                        if (empty)
                        {
                            unit = pass;
                            break;
                        }
                    }
                }
        }
    }

    if (action.action->hitMaxCount < 0)
    {
        if (!affectedPlayersForAllTime.empty())
            for (auto & itr : affectedPlayersForAllTime)
                if (unit->GetGUID() == itr)
                    return;
    }
    else if (action.action->hitMaxCount && int32(action.hitCount) >= action.action->hitMaxCount)
        return;

    bool CallScriptAreaTriggerCast = false;

    switch (action.action->actionType)
    {
        case AT_ACTION_TYPE_CAST_SPELL: // 0
        {
            if (_on_remove)
                return;

            if (caster)
            {
                if (m_CastItem && !m_CastItem->IsInWorld())
                    m_CastItem = nullptr;

                if (action.action->targetFlags & AT_TARGET_FLAG_CAST_AT_SRC)
                    caster->CastSpell(GetPositionX(), GetPositionY(), GetPositionZH(), action.action->spellId, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER), m_CastItem);
                else
                    caster->CastSpell(unit, action.action->spellId, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER), m_CastItem);

                CallScriptAreaTriggerCast = true;
            }
            break;
        }
        case AT_ACTION_TYPE_REMOVE_AURA: // 1
        {
            unit->RemoveAura(action.action->spellId);       //only one aura should be removed.
            break;
        }
        case AT_ACTION_TYPE_ADD_STACK: // 2
        {
           // if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_CASTER)
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(action.action->spellId))
                    if (Aura* aura = Aura::TryRefreshStackOrCreate(spellInfo, MAX_EFFECT_MASK, unit, unit))
                        aura->ApplyForTargets();
            break;
        }
        case AT_ACTION_TYPE_REMOVE_STACK: // 3
        {
            if (Aura* aura = unit->GetAura(action.action->spellId))
                aura->ModStackAmount(-1);
            break;
        }
        case AT_ACTION_TYPE_CHANGE_SCALE: //limit scale by hit // 4
        {
            SetSphereScale(action.action->scaleStep, atInfo.updateDelay, false, action.action->scaleMin, action.action->scaleMax, action.action->scaleVisualUpdate);
            break;
        }
        case AT_ACTION_TYPE_SHARE_DAMAGE: // 5
        {
            if (caster && spellInfo)
            {
                float bp0 = spellInfo->GetEffect(EFFECT_0, caster->GetSpawnMode())->BasePoints / _hitCount;
                float bp1 = spellInfo->GetEffect(EFFECT_1, caster->GetSpawnMode())->BasePoints / _hitCount;
                float bp2 = spellInfo->GetEffect(EFFECT_2, caster->GetSpawnMode())->BasePoints / _hitCount;

                if (action.action->targetFlags & AT_TARGET_FLAG_CAST_AT_SRC)
                {
                    SpellCastTargets targets;
                    targets.SetCaster(caster);
                    targets.SetDst(GetPositionX(), GetPositionY(), GetPositionZH(), GetOrientation());

                    CustomSpellValues values;
                    if (bp0)
                        values.AddSpellMod(SPELLVALUE_BASE_POINT0, bp0);
                    if (bp1)
                        values.AddSpellMod(SPELLVALUE_BASE_POINT1, bp1);
                    if (bp2)
                        values.AddSpellMod(SPELLVALUE_BASE_POINT2, bp2);
                    caster->CastSpell(targets, spellInfo, &values, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER));
                }
                else
                    caster->CastCustomSpell(unit, action.action->spellId, &bp0, &bp1, &bp2, true);
            }
            break;
        }
        case AT_ACTION_TYPE_APPLY_MOVEMENT_FORCE: // 6
        {
            if (unit->IsPlayer())
                if (unit->GetForceGUID().IsEmpty() && !unit->HasAuraType(SPELL_AURA_DISABLE_MOVEMENT_FORCE))
                    unit->ToPlayer()->SendMovementForce(this, Position(atInfo.windX, atInfo.windY, atInfo.windZ), atInfo.windSpeed, atInfo.windType, true);
            break;
        }
        case AT_ACTION_TYPE_REMOVE_MOVEMENT_FORCE: // 7
        {
            if (unit->IsPlayer())
                unit->ToPlayer()->SendMovementForce(this, Position(atInfo.windX, atInfo.windY, atInfo.windZ), atInfo.windSpeed, atInfo.windType);
            break;
        }
        case AT_ACTION_TYPE_CHANGE_DURATION_ANY_AT: // 8
        {
            if (!spellInfo)
                break;

            float searchRange = 0.0f;
            for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (spellInfo->EffectMask < uint32(1 << j))
                    break;

                if (float radius = spellInfo->Effects[j]->CalcRadius(caster))
                    searchRange = radius;
            }
            if (!searchRange)
                break;
            std::list<AreaTrigger*> atlist;
            GetAreaTriggerListWithEntryInGrid(atlist, GetEntry(), searchRange);
            if (!atlist.empty())
            {
                for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); ++itr)
                {
                    if (AreaTrigger* at = *itr)
                        if (at->GetDuration() > 500)
                            at->SetDuration(100);
                }
            }
            break;
        }
        case AT_ACTION_TYPE_CHANGE_AMOUNT_FROM_HEALT: // 9
        {
            if (_on_remove || !action.amount)
                return;

            if (caster)
            {
                float health = unit->GetMaxHealth() - unit->GetHealth();
                if (health >= action.amount)
                {
                    health = action.amount;
                    action.amount = 0;
                }
                else
                    action.amount -= health;

                caster->CastCustomSpell(unit, action.action->spellId, &health, &health, &health, true);
            }

            if (!action.amount && action.charges > 0)
            {
                _on_despawn = true;
                SetDuration(0);
            }
            return;
        }
        case AT_ACTION_TYPE_RE_PATCH_TO_CASTER: // 10
        {
            _spline.VerticesPoints.clear();
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
            _spline.VerticesPoints.emplace_back(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
            _canMove = true;
            _reachedDestination = true;
            InitSplines();
            SetDuration(_spline.TimeToTarget);
            break;
        }
        case AT_ACTION_TYPE_SET_AURA_CUSTOM_ADD: // 11
        {
            if (Aura* aura = _caster->GetAura(action.action->spellId))
            {
                aura->ModCustomData(1);
                aura->RecalculateAmountOfEffects(true);
            }
            break;
        }
        case AT_ACTION_TYPE_SET_AURA_CUSTOM_REMOVE: // 12
        {
            if (Aura* aura = _caster->GetAura(action.action->spellId))
            {
                aura->ModCustomData(-1);
                aura->RecalculateAmountOfEffects(true);
            }
            break;
        }
        case AT_ACTION_TYPE_REMOVE_AURA_BY_CASTER: // 13
        {
            unit->RemoveAura(action.action->spellId, caster->GetGUID());       //only one aura should be removed.
            break;
        }
        case AT_ACTION_TYPE_CAST_SPELL_NOT_TRIGGER: // 14
        {
            if (_on_remove)
                return;

            if (caster)
            {
                if (m_CastItem && !m_CastItem->IsInWorld())
                    m_CastItem = nullptr;

                if (action.action->targetFlags & AT_TARGET_FLAG_CAST_AT_SRC)
                    caster->CastSpell(GetPositionX(), GetPositionY(), GetPositionZH(), action.action->spellId, TriggerCastFlags(TRIGGERED_CASTED_BY_AREATRIGGER), m_CastItem);
                else
                    caster->CastSpell(unit, action.action->spellId, TriggerCastFlags(TRIGGERED_CASTED_BY_AREATRIGGER), m_CastItem);

                CallScriptAreaTriggerCast = true;
            }
            break;
        }
        case AT_ACTION_TYPE_NO_ACTION: // 15
        {
            // Need for CheckValidateTargets true and any action on script
            CallScriptAreaTriggerCast = true;
            break;
        }
        case AT_ACTION_TYPE_RE_PATCH: // 16
        {
            _spline.VerticesPoints[0] = G3D::Vector3(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints[1] = G3D::Vector3(GetPositionX(), GetPositionY(), GetPositionZ());
            _reachedDestination = true;
            InitSplines();
			if(atInfo.spellId != 84714) // Fix for mage frozen orb appearing to long
				SetDuration(_spline.TimeToTarget);
            break;
        }
        case AT_ACTION_TYPE_REMOVE_OWNED_AURA: // 17
        {
            if (m_aura)
                m_aura->Remove();
            break;
        }
        case AT_ACTION_TYPE_OWNER_CAST_SPELL: // 18
        {
            if (_on_remove)
                return;

            if (caster)
            {
                if (m_CastItem && !m_CastItem->IsInWorld())
                    m_CastItem = nullptr;

                Unit* owner = caster->GetAnyOwner();
                if (!owner)
                    return;

                if (action.action->targetFlags & AT_TARGET_FLAG_CAST_AT_SRC)
                    caster->CastSpell(GetPositionX(), GetPositionY(), GetPositionZH(), action.action->spellId, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER), m_CastItem, nullptr, owner->GetGUID());
                else
                    caster->CastSpell(unit, action.action->spellId, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER), m_CastItem, nullptr, owner->GetGUID());

                CallScriptAreaTriggerCast = true;
            }
            break;
        }
        case AT_ACTION_TYPE_RE_PATCH_SCRIPT: // 19
        {
            _spline.VerticesPoints.clear();
            Position startPos, endPos;
            std::vector<Position> path{};
            Position pos = { GetPositionX(), GetPositionY(), GetPositionZ() };
            if (_ai->CalculateSpline(&pos, startPos, endPos, path))
            {
                Relocate(startPos); // Set other position
                _spline.VerticesPoints.emplace_back(startPos.GetPositionX(), startPos.GetPositionY(), startPos.GetPositionZ());
                _spline.VerticesPoints.emplace_back(startPos.GetPositionX(), startPos.GetPositionY(), startPos.GetPositionZ());

                if (!path.empty())
                    for (auto const& _pos : path)
                        _spline.VerticesPoints.emplace_back(_pos.GetPositionX(), pos.GetPositionY(), _pos.GetPositionZ());

                _spline.VerticesPoints.emplace_back(endPos.GetPositionX(), endPos.GetPositionY(), endPos.GetPositionZ());
                _spline.VerticesPoints.emplace_back(endPos.GetPositionX(), endPos.GetPositionY(), endPos.GetPositionZ());
                _canMove = true;
                _reachedDestination = true;
                InitSplines();
                SetDuration(_spline.TimeToTarget);
            }
            break;
        }
        case AT_ACTION_TYPE_CASTER_GUID_REMOVE_AURA: // 20
        {
            if (_caster)
                unit->RemoveAura(action.action->spellId, _caster->GetGUID());
            break;
        }
        default:
            break;
    }

    if (CallScriptAreaTriggerCast)
    {
        if (_caster)
            if (auto creature = _caster->ToCreature())
                if (creature->IsAIEnabled)
                    creature->AI()->OnAreaTriggerCast(_caster, unit, action.action->spellId, GetSpellId());
    }

    if (action.action->hitMaxCount < 0)
        affectedPlayersForAllTime.push_back(unit->GetGUID());

    action.hitCount++;
    if (atInfo.hitType & (1 << action.action->actionType))
        _hitCount++;

    // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::DoAction action _hitCount %i hitCount %i hitMaxCount %i hitType %i actionType %i Duration %u", _hitCount, action.hitCount, action.action->hitMaxCount, atInfo.hitType, action.action->actionType, GetDuration());

    if (action.charges > 0)
    {
        --action.charges;
        //unload at next update.
        if (!action.charges && (!_HasActionsWithCharges() || action.onDespawn)) //_noActionsWithCharges check any action at enter.
        {
            // Hack
            if (GetCustomEntry() == 15012)
            {
                if (auto map = GetMap())
                    if (map->ToBgMap())
                        map->ToBgMap()->GetBG()->DoAction(true, GetGUID());
            }

            _on_despawn = true;
            SetDuration(atInfo.sequenceTemplate.animationid1 ? 1000 : 0); // Need wait for played animation

            if (action.onDespawn && m_aura)
                m_aura->Remove();
        }
    }

    if (atInfo.sequenceTemplate.animationid1)
    {
        WorldPackets::Spells::AreaTriggerSequence sequence;
        sequence.TriggerGUID = GetGUID();
        sequence.SequenceAnimationID = atInfo.sequenceTemplate.animationid1;
        sequence.SequenceEntered = atInfo.sequenceTemplate.entered1;
        _caster->SendMessageToSet(sequence.Write(), true);
    }
}

void AreaTrigger::DoActionLeave(ActionInfo& action)
{
    Unit* caster = _caster;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(action.action->spellId);
    // if (!spellInfo)
        // return;

    // do not process depleted actions
    if (!caster || !action.charges && action.action->maxCharges)
        return;

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::DoActionLeave caster %s type %u spellID %u, moment %u, targetFlags %u",
        caster->GetGUID().ToString().c_str(), action.action->actionType, action.action->spellId, action.action->moment, action.action->targetFlags);
#endif

    switch (action.action->actionType)
    {
        case AT_ACTION_TYPE_REMOVE_AURA:
        {
            if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_CASTER)
                _caster->RemoveAura(action.action->spellId);       //only one aura should be removed.
            break;
        }
        case AT_ACTION_TYPE_ADD_STACK:
        {
          //  if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_CASTER)
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(action.action->spellId))
                    if (Aura* aura = Aura::TryRefreshStackOrCreate(spellInfo, MAX_EFFECT_MASK, _caster, _caster))
                        aura->ApplyForTargets();
            break;
        }
        case AT_ACTION_TYPE_REMOVE_STACK:
        {
            if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_CASTER)
                if (Aura* aura = _caster->GetAura(action.action->spellId))
                    aura->ModStackAmount(-1);
            break;
        }
        case AT_ACTION_TYPE_CHANGE_SCALE: //limit scale by hit
        {
            SetSphereScale(action.action->scaleStep, atInfo.updateDelay, false, action.action->scaleMin, action.action->scaleMax, action.action->scaleVisualUpdate);
            break;
        }
        case AT_ACTION_TYPE_CHANGE_DURATION_ANY_AT:
        {
            if (!spellInfo)
                break;

            float searchRange = 0.0f;
            for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (spellInfo->EffectMask < uint32(1 << j))
                    break;

                if (float radius = spellInfo->Effects[j]->CalcRadius(caster))
                    searchRange = radius;
            }
            if (!searchRange)
                break;
            std::list<AreaTrigger*> atlist;
            GetAreaTriggerListWithEntryInGrid(atlist, GetEntry(), searchRange);
            if (!atlist.empty())
            {
                for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); ++itr)
                {
                    if (AreaTrigger* at = *itr)
                        if (at->GetDuration() > 500)
                            at->SetDuration(100);
                }
            }
            break;
        }
        case AT_ACTION_TYPE_SET_AURA_CUSTOM_ADD: // 11
        {
            if (Aura* aura = _caster->GetAura(action.action->spellId))
            {
                aura->ModCustomData(1);
                aura->RecalculateAmountOfEffects(true);
            }
            break;
        }
        case AT_ACTION_TYPE_SET_AURA_CUSTOM_REMOVE: // 12
        {
            if (Aura* aura = _caster->GetAura(action.action->spellId))
            {
                aura->ModCustomData(-1);
                aura->RecalculateAmountOfEffects(true);
            }
            break;
        }
        default:
            break;
    }

    action.hitCount++;
    if (atInfo.hitType & (1 << action.action->actionType))
        _hitCount++;

    if (action.charges > 0)
    {
        --action.charges;
        //unload at next update.
        if (!action.charges && (!_HasActionsWithCharges() || action.onDespawn)) //_noActionsWithCharges check any action at enter.
        {
            _on_despawn = true;
            SetDuration(0);

            if (action.onDespawn && m_aura)
                m_aura->Remove();
        }
    }
}

void AreaTrigger::Remove(bool duration)
{
    if (_on_remove)
        return;
    if (_on_unload)
        return;
    _on_unload = true;

    if (IsInWorld())
    {
        if (_caster && m_spellInfo)
            _caster->SendSpellPlayOrphanVisual(m_spellInfo, false);

        if (duration)
        {
            UpdateAffectedList(0, AT_ACTION_MOMENT_DESPAWN);//remove from world with time
            ActionOnDespawn();
        }
        else
            UpdateAffectedList(0, AT_ACTION_MOMENT_LEAVE);//remove from world in action

        UpdateAffectedList(0, AT_ACTION_MOMENT_REMOVE);//any remove from world

        if (_caster)
        {
            if (auto creature = _caster->ToCreature())
                if (creature->IsAIEnabled)
                    creature->AI()->OnAreaTriggerDespawn(GetSpellId(), GetPosition(), duration);
        }

        // Possibly this?
        if (!IsInWorld())
            return;

        RemoveFromWorld();
        AddObjectToRemoveList();
    }
}

void AreaTrigger::Despawn()
{
    if (_on_remove)
        return;
    _on_remove = true;

    if (IsInWorld())
    {
        ActionOnDespawn();
        UpdateAffectedList(0, AT_ACTION_MOMENT_DESPAWN);//remove from world with time
        UpdateAffectedList(0, AT_ACTION_MOMENT_REMOVE);//any remove from world

        // Possibly this?
        if (!IsInWorld())
            return;

        RemoveFromWorld();
        AddObjectToRemoveList();
    }
}

uint32 AreaTrigger::GetSpellId() const
{
    return GetUInt32Value(AREATRIGGER_FIELD_SPELL_ID);
}

void AreaTrigger::SetSpellId(uint32 spell)
{
    return SetUInt32Value(AREATRIGGER_FIELD_SPELL_ID, spell);
}

void AreaTrigger::SetRadius(float radius)
{
    _radius = radius;
}

float AreaTrigger::GetVisualScale(bool target /*=false*/) const
{
    if (_areaTriggerCylinder) // Send only for sphere
        return 0.0f;

    if (target)
        return atInfo.RadiusTarget;

    return atInfo.Radius;
}

float AreaTrigger::GetRadius() const
{
    float radius = _radius;
    if (atInfo.HasDynamicShape && atInfo.MorphCurveID && isMoving())
        radius = G3D::lerp(atInfo.Radius, atInfo.RadiusTarget, sDB2Manager.GetCurveValueAt(atInfo.MorphCurveID, GetProgress()));

    return radius;
}

ObjectGuid AreaTrigger::GetCasterGUID() const
{
    return GetGuidValue(AREATRIGGER_FIELD_CASTER);
}

Unit* AreaTrigger::GetCaster() const
{
    return ObjectAccessor::GetUnit(*this, GetCasterGUID());
}

void AreaTrigger::SetTargetGuid(ObjectGuid targetGuid)
{
    _targetGuid = targetGuid;
}

ObjectGuid AreaTrigger::GetTargetGuid() const
{
    return _targetGuid;
}

Unit* AreaTrigger::GetTarget() const
{
    return ObjectAccessor::GetUnit(*this, GetTargetGuid());
}

int32 AreaTrigger::GetDuration() const
{
    return _duration;
}

void AreaTrigger::SetDuration(int32 newDuration)
{
    _duration = newDuration;
}

int32 AreaTrigger::GeTimeToTarget() const
{
    return _spline.TimeToTarget;
}

void AreaTrigger::SetTimeToTarget(int32 timeToTarget)
{
    _spline.TimeToTarget = timeToTarget;
}

void AreaTrigger::Delay(int32 delaytime)
{
    SetDuration(GetDuration() - delaytime);
}

bool AreaTrigger::CheckActionConditions(AreaTriggerAction const& action, Unit* unit)
{
    if (m_withoutCaster)
        return true;

    Unit* caster = GetCaster();
    if (!caster)
        return false;

    ConditionSourceInfo srcInfo = ConditionSourceInfo(caster, unit);
    return sConditionMgr->IsObjectMeetToConditions(srcInfo, sConditionMgr->GetConditionsForAreaTriggerAction(GetEntry(), action.id));
}

void AreaTrigger::BindToCaster()
{
    ASSERT(!_caster);
    _caster = ObjectAccessor::GetUnit(*this, GetCasterGUID());
    ASSERT(_caster);
    ASSERT(_caster->GetMap() == GetMap());
    _caster->_RegisterAreaObject(this);
}

void AreaTrigger::UnbindFromCaster()
{
    if (_caster)
    {
        _caster->_UnregisterAreaObject(this);
        _caster = nullptr;
    }
}

SpellInfo const* AreaTrigger::GetSpellInfo()
{
    return m_spellInfo;
}

SpellValue const* AreaTrigger::GetSpellValue()
{
    return m_spell ? m_spell->m_spellValue : nullptr;
}

void AreaTrigger::InitSplines()
{
    if (_spline.VerticesPoints.size() < 2)
        return;

    _movementTime = 0;

    if (_reachedDestination && atInfo.RePatchSpeed)
        _ai->ModifyMoveSpeed(atInfo.RePatchSpeed);

    Movement::MoveSplineInitArgs args;
    args.splineId = Movement::splineIdGen.NewId();
    args.path = _spline.VerticesPoints;
    args.velocity = _reachedDestination ? atInfo.RePatchSpeed ? atInfo.RePatchSpeed : atInfo.Speed : atInfo.Speed;

    if (atInfo.MoveCurveID)
    {
        Movement::SpellEffectExtraData extraData;
        extraData.ProgressCurveId = atInfo.MoveCurveID;
        args.spellEffectExtra = extraData;
    }
    args.flags.cyclic = HasCircleData();

    movespline->Initialize(args);

    _spline.TimeToTarget = movespline->Duration();

    if (_CircleData)
        _CircleData->TimeToTarget = _spline.TimeToTarget;

    #ifdef WIN32
        TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "InitSplines TimeToTarget %i _liveTime %u RePatchSpeed %f Speed %f movespline %s", _spline.TimeToTarget, _liveTime, atInfo.RePatchSpeed, atInfo.Speed, movespline->ToString().c_str());
    #endif

    if (_reachedDestination)
    {
        _spline.ElapsedTimeForMovement = _liveTime;

        WorldPackets::Spells::AreaTriggerReShape reshape;
        reshape.TriggerGUID = GetGUID();
        reshape.Spline = boost::in_place();
        reshape.Spline = _spline;
        SendMessageToSet(reshape.Write(), true);
    }

    _reachedDestination = false;
}

void AreaTrigger::UpdateSplinePosition(uint32 diff)
{
    ObjectGuid targetGuid = GetTargetGuid();
    if (!targetGuid.IsEmpty() && !atInfo.isCircle)
    {
        if (!UpdatePosition(targetGuid))
        {
            if (GetDuration() > 100)
                SetDuration(100);
            return;
        }
    }

    if (!isMoving() || _spline.VerticesPoints.empty())
        return;

    if (atInfo.isCircle && !atInfo.circleTemplate.CanLoop)
    {
        Relocate(_spline.VerticesPoints[0].x + _caster->GetPositionX(), _spline.VerticesPoints[0].y + _caster->GetPositionY(), _spline.VerticesPoints[0].z + _caster->GetPositionZ());
        return;
    }

    if (movespline->Finalized())
    {
        if (atInfo.OnDestinationReachedDespawn)
            Remove();
        return;
    }

    // Prevent crash on empty spline
    if (!movespline->Initialized())
    {
        movespline->_Interrupt();
        return;
    }

    movespline->updateState(diff);
    _reachedDestination = movespline->Finalized();

    if (_currentWP != movespline->_currentSplineIdx())
    {
        _ai->OnChangeSplineId(movespline->_currentSplineIdx(), _currentWP);
        _currentWP = movespline->_currentSplineIdx();
    }

    Movement::Location loc = movespline->ComputePosition();
    if (atInfo.circleTemplate.HasTarget)
    {
        if (_caster->GetGUID() == targetGuid || !targetGuid)
            Relocate(loc.x + _caster->GetPositionX(), loc.y + _caster->GetPositionY(), loc.z + _caster->GetPositionZ(), loc.orientation);
        else if (Unit* target = ObjectAccessor::GetUnit(*this, targetGuid))
            Relocate(loc.x + target->GetPositionX(), loc.y + target->GetPositionY(), loc.z + target->GetPositionZ(), loc.orientation);
    }
    else
        Relocate(loc.x, loc.y, loc.z, loc.orientation);

    _movementTime += diff;

    if (_reachedDestination)
    {
        _ai->OnDestinationReached();
        ReCalculateSplinePosition();
    }

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::UpdateSplinePosition %f %f %f %i  _reachedDestination %u", GetPositionX(), GetPositionY(), GetPositionZ(), _movementTime, _reachedDestination);
    DebugVisualizePosition();
#endif
}

void AreaTrigger::UpdateRotation(uint32 diff)
{
    if (!isMoving() || !_spline.VerticesPoints.empty())
        return;

    //TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::UpdateRotation _waitTime %i _movementTime %i _nextMoveTime %i diff %i o %f", _waitTime, _movementTime, _nextMoveTime, diff, GetOrientation());

    if (_waitTime > 0)
    {
        if (diff >= _waitTime)
            _waitTime = 0;
        else
            _waitTime -= diff;
        return;
    }

    _movementTime += diff;

    if (_movementTime >= _nextMoveTime)
    {
        float o = GetOrientation() + atInfo.Speed * (_movementTime / 1000);
        _movementTime = 0;
        _nextMoveTime = 1000;
        SetOrientation(o);
        //float x = GetPositionX() + 12 * std::cos(o);
        //float y = GetPositionY() + 12 * std::sin(o);
        //_caster->SummonCreature(44548, x, y, GetPositionZ(), o,TEMPSUMMON_TIMED_DESPAWN, 20000); // For visual point test
    }
}

bool AreaTrigger::isMoving() const
{
    return _canMove;
}

bool AreaTrigger::HasSpline() const
{
    return !_spline.VerticesPoints.empty() && !HasCircleData();
}

bool AreaTrigger::IsInArea(Unit* unit)
{
    if (atInfo.hasAreaTriggerBox && atInfo.AllowBoxCheck)
        return IsInBox(unit);

    if (atInfo.polygon)
        return IsInPolygon(unit);

    return IsInHeight(unit) && unit->IsWithinDistInMap(this, GetRadius());
}

bool AreaTrigger::IsInHeight(Unit* unit)
{
    float _height = atInfo.Polygon.Height;
    if (atInfo.hasAreaTriggerBox)
        _height = atInfo.LocationZOffset;

    if (!_height)
        return true;

    return unit->GetPositionZH() - GetPositionZH() <= atInfo.Polygon.Height;
}

bool AreaTrigger::IsInBox(Unit* unit)
{
    float x_source = unit->GetPositionX() - GetPositionX(); //Point X on polygon
    float y_source = unit->GetPositionY() - GetPositionY(); //Point Y on polygon
    float z_source = unit->GetPositionY() - GetPositionY(); //Point Z on polygon

    return atInfo.box.contains({ x_source, y_source, z_source });
}

bool AreaTrigger::IsInPolygon(Unit* unit)
{
    if (atInfo.Polygon.Vertices.size() < 3)
        return false;

    if (!IsInHeight(unit))
        return false;

    std::vector<TaggedPosition<Position::XY>>* _points;
    if (!atInfo.Polygon.VerticesTarget.empty())
        _points = &atInfo.Polygon.VerticesTarget;
    else
        _points = &atInfo.Polygon.Vertices;

    static const int q_patt[2][2] = { {0,1}, {3,2} };
    float x_source = unit->GetPositionX() - GetPositionX(); //Point X on polygon
    float y_source = unit->GetPositionY() - GetPositionY(); //Point Y on polygon
    float angle = atan2(y_source, x_source) - GetOrientation(); angle = (angle >= 0) ? angle : 2 * M_PI + angle;

    float dist = sqrt(x_source*x_source + y_source*y_source);
    float x = dist * std::cos(angle);
    float y = dist * std::sin(angle);

    auto pred_pt = (*_points)[_points->size() - 1];
    pred_pt.Pos.m_positionX -= x;
    pred_pt.Pos.m_positionY -= y;

    int pred_q = q_patt[pred_pt.Pos.m_positionY < 0][pred_pt.Pos.m_positionX < 0];

    // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::IsInPolygon x_source %f y_source %f angle %f dist %f x %f y %f pred_ptX %f pred_ptY %f pred_q %u GetGUID %s", x_source, y_source, angle, dist, x, y, pred_pt.x, pred_pt.y, pred_q, unit->GetGUID().ToString().c_str());

    int w = 0;
    int i = 0;

    for (auto cur_pt : *_points)
    {
        if (!ActivePointPolygon[i])
        {
            i++;
            continue;
        }

        cur_pt.Pos.m_positionX -= x;
        cur_pt.Pos.m_positionY -= y;

        int q = q_patt[cur_pt.Pos.m_positionY < 0][cur_pt.Pos.m_positionX < 0];

        switch (q - pred_q)
        {
            case -3:
                ++w;
                break;
            case 3:
                --w;
                break;
            case -2:
                if (pred_pt.Pos.m_positionX * cur_pt.Pos.m_positionY >= pred_pt.Pos.m_positionY * cur_pt.Pos.m_positionX)
                    ++w;
                break;
            case 2:
                if (!(pred_pt.Pos.m_positionX * cur_pt.Pos.m_positionY >= pred_pt.Pos.m_positionY * cur_pt.Pos.m_positionX))
                    --w;
                break;
            default:
                break;
        }

        pred_pt = cur_pt;
        pred_q = q;
        i++;
    }

    return w != 0;
}

float AreaTrigger::CalculateRadiusPolygon()
{
    //calc maxDist for search zone
    float distance = 0.0f;
    for (auto const& v : atInfo.Polygon.Vertices)
    {
        float distsq = fabs(v.Pos.m_positionX) > fabs(v.Pos.m_positionY) ? fabs(v.Pos.m_positionX) : fabs(v.Pos.m_positionY);
        if (distsq > distance)
            distance = distsq;
    }

    for (auto const& v : atInfo.Polygon.VerticesTarget)
    {
        float distsq = fabs(v.Pos.m_positionX) > fabs(v.Pos.m_positionY) ? fabs(v.Pos.m_positionX) : fabs(v.Pos.m_positionY);
        if (distsq > distance)
            distance = distsq;
    }

    if (!atInfo.Polygon.VerticesTarget.empty())
        ActivePointPolygon.resize(atInfo.Polygon.VerticesTarget.size(), true);
    else
        ActivePointPolygon.resize(atInfo.Polygon.Vertices.size(), true);

    // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::CalculateRadiusPolygon distance %f", distance);

    return distance;
}

AreaTriggerSpline AreaTrigger::GetSplineInfo() const
{
    return _spline;
}

GuidList* AreaTrigger::GetAffectedPlayers()
{
    return &affectedPlayers;
}

GuidList* AreaTrigger::GetAffectedPlayersForAllTime()
{
    return &affectedPlayersForAllTime;
}

void AreaTrigger::CastAction()
{
    if (_on_remove)
        return;
    UpdateAffectedList(0, AT_ACTION_MOMENT_ON_CAST_ACTION);
}

bool AreaTrigger::UpdatePosition(ObjectGuid targetGuid)
{
    if (_on_remove || !IsInWorld())
        return true;

    Unit* caster = GetCaster();
    if (!caster)
        return true;

    if (caster->GetGUID() == targetGuid)
    {
        if (caster->GetMap() == GetMap())
        {
            bool turn = GetOrientation() != caster->GetOrientation();
            bool relocated = GetPositionX() != caster->GetPositionX() || GetPositionY() != caster->GetPositionY();

            if (relocated)
            {
                GetMap()->AreaTriggerRelocation(this, caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZH(), caster->GetOrientation());
                #ifdef WIN32
                DebugVisualizePolygon();
                #endif
            }
            else if (turn)
            {
                SetOrientation(caster->GetOrientation());
                #ifdef WIN32
                DebugVisualizePolygon();
                #endif
            }
        }
        else
        {
            GetMap()->RemoveFromMap(this, false);
            Relocate(*caster);
            SetMap(caster->GetMap());
            GetMap()->AddToMap(this);
        }
        return true;
    }
    if (Unit* target = ObjectAccessor::GetUnit(*this, targetGuid))
    {
        if (GetMap() == target->GetMap())
        {
            bool turn = GetOrientation() != target->GetOrientation();
            bool relocated = GetPositionX() != target->GetPositionX() || GetPositionY() != target->GetPositionY();

            if (relocated)
                GetMap()->AreaTriggerRelocation(this, target->GetPositionX(), target->GetPositionY(), target->GetPositionZH(), target->GetOrientation());
            else if (turn)
                SetOrientation(target->GetOrientation());

            return true;
        }
    }

    return false;
}

void AreaTrigger::CalculateRadius(Spell* spell/* = nullptr*/)
{
    Unit* caster = GetCaster();

    if (atInfo.polygon)
        _radius = CalculateRadiusPolygon();
    else
    {
        bool find = false;
        if (atInfo.Radius || atInfo.RadiusTarget)
        {
            if (atInfo.Radius > atInfo.RadiusTarget)
                _radius = atInfo.Radius;
            else
                _radius = atInfo.RadiusTarget;
            find = true;
        }

        if (caster && !find && m_spellInfo)
        {
            for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (m_spellInfo->EffectMask < uint32(1 << j))
                    break;

                if (float r = m_spellInfo->Effects[j]->CalcRadius(GetCaster()))
                    _radius = r * (spell ? spell->m_spellValue->RadiusMod : 1.0f);
            }
        }
    }

    if (caster && m_spellInfo)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RADIUS, _radius, spell);
}

uint32 AreaTrigger::GetCustomEntry() const
{
    return atInfo.customEntry;
}

uint32 AreaTrigger::GetRealEntry() const
{
    return _realEntry;
}

bool AreaTrigger::CheckValidateTargets(Unit* unit, AreaTriggerActionMoment /*actionM*/)
{
    Unit* caster = _caster;

    if (!unit)
        return true;

    for (auto & itr : _actionInfo)
    {
        ActionInfo& action = itr.second;
        // if (!(action.action->moment & actionM)) // Need check all action for all work good
            // continue;

        if (action.action->targetFlags & AT_TARGET_FLAG_FRIENDLY)
            if (!caster || !caster->IsFriendlyTo(unit) || unit->isTotem())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_FRIENDLY unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_HOSTILE)
            if (!caster || !caster->IsHostileTo(unit))
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_HOSTILE unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_VALIDATTACK)
            if (!caster || !caster->IsValidAttackTarget(unit) || unit->isTotem())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_VALIDATTACK unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_OWNER)
            if (unit->GetGUID() != GetCasterGUID())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_OWNER unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_PLAYER)
            if (!unit->IsPlayer())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_PLAYER unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_NOT_PET)
            if (unit->isPet())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_NOT_PET unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_NOT_FULL_HP)
            if (unit->IsFullHealth())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_NOT_FULL_HP unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_GROUP_OR_RAID)
            if (!unit->IsInRaidWith(caster))
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_GROUP_OR_RAID unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_NOT_AURA_TARGET)
            if (GetTargetGuid() && GetTargetGuid() == unit->GetGUID())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_NOT_AURA_TARGET unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAT_IN_FRONT)
            if (!HasInArc(static_cast<float>(M_PI), unit))
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAT_IN_FRONT unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_TARGET_IS_SUMMONER)
            if (Unit* summoner = _caster->GetAnyOwner())
                if (unit->GetGUID() != summoner->GetGUID())
                {
                    // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_TARGET_IS_SUMMONER unit %s", unit->GetGUID().ToString().c_str());
                    continue;
                }

        if (action.action->targetFlags & AT_TARGET_FLAG_NOT_OWNER)
            if (unit->GetGUID() == GetCasterGUID())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_NOT_OWNER unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_NPC_ENTRY)
            if (action.amount != unit->GetEntry() || unit->ToPlayer())
            {
                // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets AT_TARGET_FLAG_NPC_ENTRY unit %s", unit->GetGUID().ToString().c_str());
                continue;
            }

        if (action.action->targetFlags & AT_TARGET_FLAG_SCRIPT)
            if (!_ai->IsValidTarget(_caster, unit, action.action->moment))
                continue;

        if (action.action->hasAura > 0 && !unit->HasAura(action.action->hasAura)
            || action.action->hasAura2 > 0 && !unit->HasAura(action.action->hasAura2)
            || action.action->hasAura3 > 0 && !unit->HasAura(action.action->hasAura3))
        {
            // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets aura > 0 unit %s", unit->GetGUID().ToString().c_str());
            continue;
        }
        if (action.action->hasAura < 0 && unit->HasAura(abs(action.action->hasAura))
            || action.action->hasAura2 < 0 && unit->HasAura(abs(action.action->hasAura2))
            || action.action->hasAura3 < 0 && unit->HasAura(abs(action.action->hasAura3)))
        {
            // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets aura < 0 unit %s", unit->GetGUID().ToString().c_str());
            continue;
        }

        if (action.action->hasspell > 0 && !_caster->HasSpell(action.action->hasspell))
        {
            // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets hasspell > 0 unit %s", unit->GetGUID().ToString().c_str());
            continue;
        }
        if (action.action->hasspell < 0 && _caster->HasSpell(abs(action.action->hasspell)))
        {
            // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CheckValidateTargets hasspell < 0 unit %s", unit->GetGUID().ToString().c_str());
            continue;
        }
        
        if (action.action->minDistance && GetDistance(unit) > action.action->minDistance)
            continue;

        return true;
    }

    return false;
}

AreaTriggerInfo AreaTrigger::GetAreaTriggerInfo() const
{
    return atInfo;
}

void AreaTrigger::GetCollisionPosition(Position &_dest, float dist, float angle)
{
    Position pos;
    pos.Relocate(GetPositionX(), GetPositionY(), GetPositionZ() + 2.0f);

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "GetCollisionPosition coordinates (X: %f Y: %f) ", pos.m_positionX, pos.m_positionY, pos.m_positionZ);
#endif

    float destx = pos.m_positionX + dist * std::cos(angle);
    float desty = pos.m_positionY + dist * std::sin(angle);
    float destz = GetPositionZ() + 2.0f;

    bool col = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ + 0.5f, destx, desty, destz + 0.5f, destx, desty, destz, -0.5f);
    if (!col)
    {
    #ifdef WIN32
        TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "GetCollisionPosition coordinates !col  dest (X: %f Y: %f) ", destx, desty, destz);
    #endif

        dist += 200.0f;
        destx = pos.m_positionX + dist * std::cos(angle);
        desty = pos.m_positionY + dist * std::sin(angle);
        VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ + 0.5f, destx, desty, destz + 0.5f, destx, desty, destz, -0.5f);
    }

#ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "GetCollisionPosition coordinates  dest (X: %f Y: %f) Z %f", destx, desty, destz, GetPositionZ());
#endif

    destx -= (CONTACT_DISTANCE + _radius) * std::cos(angle);
    desty -= (CONTACT_DISTANCE + _radius) * std::sin(angle);
    _dest.Relocate(destx, desty, GetPositionZ());
}

void AreaTrigger::SendReShape(Position const* pos)
{
    if (!pos)
        return;

    AreaTriggerSpline _splineTemp;
    _splineTemp.VerticesPoints.clear();
    float _moveDistanceMax = 0.0f;
    _splineTemp.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
    _splineTemp.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
    _splineTemp.VerticesPoints.emplace_back(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ());
    _splineTemp.VerticesPoints.emplace_back(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ());

    for (size_t i = 0; i < _splineTemp.VerticesPoints.size() - 1; ++i)
        _moveDistanceMax += (_splineTemp.VerticesPoints[i + 1] - _splineTemp.VerticesPoints[i]).length();

    _splineTemp.TimeToTarget = int32(_moveDistanceMax / 10.0f * 800.0f);
    _splineTemp.ElapsedTimeForMovement = uint32(_liveTime * 0.7f);

    WorldPackets::Spells::AreaTriggerReShape rePath;
    rePath.TriggerGUID = GetGUID();
    rePath.Spline = boost::in_place();
    rePath.Spline = _splineTemp;
    _caster->SendMessageToSet(rePath.Write(), true);

    Relocate(pos);
}

void AreaTrigger::MoveTo(Position const * pos)
{
    MoveTo(pos->m_positionX, pos->m_positionY, pos->m_positionZ);
}

void AreaTrigger::MoveTo(Unit * target)
{
    MoveTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
}

void AreaTrigger::MoveTo(float x, float y, float z)
{
    float startX = GetPositionX();
    float startY = GetPositionY();
    float startZ = GetPositionZ();

    _reachedDestination = true;
    _spline.VerticesPoints.clear();
    _spline.VerticesPoints.emplace_back(startX, startY, startZ);
    _spline.VerticesPoints.emplace_back(startX, startY, startZ);
    _spline.VerticesPoints.emplace_back(x, y, z);
    _spline.VerticesPoints.emplace_back(x, y, z);
    InitSplines();
}

void AreaTrigger::SetPolygonVertices(uint32 index, bool editX, float x, bool editY, float y)
{
    if (!atInfo.polygon || atInfo.Polygon.Vertices.size() <= index)
        return;

    if (editX)
    {
        atInfo.Polygon.Vertices[index].Pos.m_positionX = x;
    }
    if (editY)
    {
        atInfo.Polygon.Vertices[index].Pos.m_positionY = y;
    }
}

void AreaTrigger::UpdateScale(uint32 p_time)
{
    if (!_scaleDelay)
        return;

    if (_scaleDelay > p_time)
        _scaleDelay -= p_time;
    else
    {
        _scaleDelay = 0;
        UpdateOverrideScale();
    }
}

void AreaTrigger::UpdateSequence(uint32 p_time)
{
    if (!atInfo.isSequence || !atInfo.sequenceTemplate.oncreated)
        return;

    if (_sequenceDelay > p_time)
        _sequenceDelay -= p_time;
    else
    {
        switch (_sequenceStep)
        {
            case 0:
            {
                _sequenceStep++;
                _sequenceDelay = atInfo.sequenceTemplate.timer1;
                if (atInfo.sequenceTemplate.oncreated)
                {
                    WorldPackets::Spells::AreaTriggerSequence sequence;
                    sequence.TriggerGUID = GetGUID();
                    sequence.SequenceAnimationID = atInfo.sequenceTemplate.animationid;
                    sequence.SequenceEntered = atInfo.sequenceTemplate.entered;
                    _caster->SendMessageToSet(sequence.Write(), true);
                    if (atInfo.sequenceTemplate.animationid == atInfo.sequenceTemplate.animationid1) // Skip first step
                    {
                        _sequenceStep++;
                        _sequenceDelay = atInfo.sequenceTemplate.timer2;
                    }
                }
                break;
            }
            case 1:
            {
                _sequenceStep++;
                _sequenceDelay = atInfo.sequenceTemplate.timer2;
                if (atInfo.sequenceTemplate.animationid1)
                {
                    WorldPackets::Spells::AreaTriggerSequence sequence;
                    sequence.TriggerGUID = GetGUID();
                    sequence.SequenceAnimationID = atInfo.sequenceTemplate.animationid1;
                    sequence.SequenceEntered = atInfo.sequenceTemplate.entered1;
                    _caster->SendMessageToSet(sequence.Write(), true);
                }
                break;
            }
            case 2:
            {
                if (atInfo.sequenceTemplate.cycle)
                {
                    _sequenceStep = 1;
                    _sequenceDelay = atInfo.sequenceTemplate.timer1;
                }
                else
                    _sequenceStep++;

                if (atInfo.sequenceTemplate.animationid2)
                {
                    WorldPackets::Spells::AreaTriggerSequence sequence;
                    sequence.TriggerGUID = GetGUID();
                    sequence.SequenceAnimationID = atInfo.sequenceTemplate.animationid2;
                    sequence.SequenceEntered = atInfo.sequenceTemplate.entered2;
                    _caster->SendMessageToSet(sequence.Write(), true);
                }
                break;
            }
        }
    }
}

void AreaTrigger::SetSphereScale(float mod, uint32 time, bool absolute_change /*=false*/, float scaleMin /*= 0.1f*/, float scaleMax /*= 100.0f*/, bool updateOverride/*= true*/)
{
    if (mod == 0.0f)
        return;

    float radiusMod = _scaleData.OverrideScale[2].floatValue;
    float radiusTargetMod = _scaleData.OverrideScale[4].floatValue;

    if (scaleMax <= 0.0f)
        scaleMax = 100.0f;

    //Server and Client update
    if (updateOverride)
    {
        if (radiusTargetMod >= scaleMax)
            return;

        if (!radiusMod || !radiusTargetMod)
        {
            radiusMod = 1.0f;
            radiusTargetMod = (1.0f * mod) + 1.0f;
        }
        else if (radiusMod && radiusTargetMod)
        {
            radiusMod = _scaleData.OverrideScale[2].floatValue * mod + _scaleData.OverrideScale[2].floatValue;
            radiusTargetMod = _scaleData.OverrideScale[4].floatValue * mod + _scaleData.OverrideScale[4].floatValue;
        }

        if (absolute_change) // use only with absolute values, for example, set radius = 3 yardes, not + 3 yardes
        {
            radiusMod = mod;
            radiusTargetMod = mod;
        }

        _scaleData.OverrideScale[0].integerValue = _liveTime;
        _scaleData.OverrideScale[1].floatValue = 0.0f;
        _scaleData.OverrideScale[2].floatValue = radiusMod;
        _scaleData.OverrideScale[3].floatValue = 1.0f;
        _scaleData.OverrideScale[4].floatValue = radiusTargetMod;
        _scaleData.OverrideScale[5].integerValue = 0x10000000; // mask?
        _scaleData.OverrideScale[6].integerValue = 1;
        _scaleData.timeToTargetScale = time;
        _scaleDelay = time;

        _radius = GetFloatValue(AREATRIGGER_FIELD_BOUNDS_RADIUS_2_D) * radiusTargetMod;

        if (radiusTargetMod <= scaleMin)
        {
            _on_despawn = true;
            SetDuration(0);
        }
    }
    else //Server update
    {
        if (_radius >= scaleMax)
            return;

        _radius += mod;

        if (_radius <= scaleMin)
        {
            _on_despawn = true;
            SetDuration(0);
        }
    }
}

void AreaTrigger::UpdateOverrideScale()
{
    for (uint32 i = 0; i < 7; ++i)
    {
        if (_scaleData.OverrideScale[i].integerValue)
            SetUInt32Value(AREATRIGGER_FIELD_OVERRIDE_SCALE_CURVE + i, _scaleData.OverrideScale[i].integerValue);
        else if (_scaleData.OverrideScale[i].floatValue)
            SetFloatValue(AREATRIGGER_FIELD_OVERRIDE_SCALE_CURVE + i, _scaleData.OverrideScale[i].floatValue);
    }

    if (_scaleData.timeToTargetScale)
        SetUInt32Value(AREATRIGGER_FIELD_TIME_TO_TARGET_SCALE, _scaleData.timeToTargetScale);
}

void AreaTrigger::AI_Initialize()
{
    AI_Destroy();
    AreaTriggerAI* ai = sScriptMgr->GetAreaTriggerAI(this);
    if (!ai)
        ai = new NullAreaTriggerAI(this);

    _ai.reset(ai);
    _ai->OnInitialize();
}

void AreaTrigger::AI_Destroy()
{
    _ai.reset();
}

AreaTriggerAI* AreaTrigger::AI()
{
    return _ai.get();
}

void AreaTrigger::CalculateSplinePosition(Position const& pos, Position const& posMove, Unit* caster)
{
    if (atInfo.isCircle)
    {
        CalculateCyclicPosition(pos, posMove, caster);
        return;
    }

    SetTimeToTarget(_duration);
    _waitTime = atInfo.waitTime;

    Position startPos, endPos;
    std::vector<Position> path{};
    if (_ai->CalculateSpline(&pos, startPos, endPos, path))
    {
        Relocate(startPos); // Set other position
        _spline.VerticesPoints.emplace_back(startPos.GetPositionX(), startPos.GetPositionY(), startPos.GetPositionZ());
        _spline.VerticesPoints.emplace_back(startPos.GetPositionX(), startPos.GetPositionY(), startPos.GetPositionZ());
        
        if (!path.empty())
            for (auto const& _pos : path)
                _spline.VerticesPoints.emplace_back(_pos.GetPositionX(), _pos.GetPositionY(), _pos.GetPositionZ());
            
        _spline.VerticesPoints.emplace_back(endPos.GetPositionX(), endPos.GetPositionY(), endPos.GetPositionZ());
        _spline.VerticesPoints.emplace_back(endPos.GetPositionX(), endPos.GetPositionY(), endPos.GetPositionZ());
        InitSplines();
        return;
    }

    switch (atInfo.moveType)
    {
        case AT_MOVE_TYPE_DEFAULT: // 0
        {
            if (_range != 0.0f)
            {
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                if (GetDistance(posMove) > 0.0f && (posMove.GetPositionX() != 0.0f || posMove.GetPositionY() != 0.0f))
                {
                    _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
                    _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
                }
                else
                {
                    Position nextPos;
                    SimplePosXYRelocationByAngle(nextPos, _range, atInfo.AngleToCaster + frand(atInfo.AnglePointA, atInfo.AnglePointB)/* + GetOrientation()*/);
                    _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
                    _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
                }
            }
            break;
        }
        case AT_MOVE_TYPE_LIMIT_TO_TARGET: // 1
        {
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
            _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
            break;
        }
        case AT_MOVE_TYPE_SPIRAL: // 2
        {
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            float maxDist = _duration / 1000.0f * atInfo.Speed;

            float _nextAngle = atInfo.AngleToCaster ? atInfo.AngleToCaster : 45.0f;
            float _modParam = atInfo.Param ? atInfo.Param : 0.3f;
            float rand = frand(atInfo.AnglePointA, atInfo.AnglePointB);

            for (float a = 0;; a += _nextAngle) // next angel 45
            {
                float rad = a * M_PI / 180.0f;
                float r = 1.0f * exp(rad * _modParam);

                Position nextPos{ m_positionX - r * sin(rad + rand), m_positionY + r * cos(rad + rand), m_positionZ };
                _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());

                maxDist -= (_spline.VerticesPoints[_spline.VerticesPoints.size() - 2] - _spline.VerticesPoints[_spline.VerticesPoints.size() - 1]).length();
                if (maxDist <= 0.0f)
                {
                    _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
                    break;
                }
            }

            //if (Creature* debuger = GetMap()->SummonCreature(950121, { GetPositionX(), GetPositionY(), GetPositionZ() }, nullptr, _duration))
            //{
            //    debuger->SetSpeed(UnitMoveType::MOVE_RUN, atInfo.Speed, true);
            //    debuger->SetWalk(false);
            //    debuger->AI()->SetGUID(GetGUID());
            //}

            //for (auto pos : _spline.VerticesPoints)
            //    GetMap()->SummonCreature(1, { pos.x, pos.y, pos.z }, nullptr, _duration);
            break;
        }
        case AT_MOVE_TYPE_BOOMERANG: // 3
        {
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            float maxDist = _duration / 1000.0f * atInfo.Speed;
            uint8 countStep = uint8(maxDist / 5) * 2;

            for (uint8 i = 1; i <= countStep; i++)
            {
                uint8 step = i >= countStep / 2 ? countStep - i : i;
                float x = m_positionX + step * 5 * std::cos(GetOrientation());
                float y = m_positionY + step * 5 * std::sin(GetOrientation());
                Trinity::NormalizeMapCoord(x);
                Trinity::NormalizeMapCoord(y);
                Position nextPos{ x, y, m_positionZ };
                _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
            }
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            break;
        }
        case AT_MOVE_TYPE_CHAGE_ROTATION: // 4 No WP
            break;
        case AT_MOVE_TYPE_RE_PATH: // 5 only Divine Star
        {
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());

            Position nextPos;
            SimplePosXYRelocationByAngle(nextPos, _range, 0.f/*GetOrientation()*/);
            _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
            _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
            break;
        }
        case AT_MOVE_TYPE_RANDOM: // 6
        {
            float angle = 0.0f;
            if (atInfo.AngleToCaster != 0.0f && m_spellInfo)
            {
                if (atInfo.AnglePointA != 0.0f)
                    angle = atInfo.AngleToCaster += atInfo.AnglePointA * caster->CountAreaObject(m_spellInfo->Id);
                else
                    angle = atInfo.AngleToCaster * caster->CountAreaObject(m_spellInfo->Id);
            }
            else
                angle = urand(0, 6);

            if (_range != 0.0f)
            {
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());

                Position nextPos;
                SimplePosXYRelocationByAngle(nextPos, _range, angle);
                _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
                _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
            }
            break;
        }
        case AT_MOVE_TYPE_ANGLE_TO_CASTER: // 7
        {
            if (_range != 0.0f)
            {
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());

                Position nextPos;
                float angle = caster->GetRelativeAngle(this);
                SimplePosXYRelocationByAngle(nextPos, _range, angle);
                _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
                _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
            }
            break;
        }
        case AT_MOVE_TYPE_RE_PATH_LOS: // 8
        {
            if (_range != 0.0f)
            {
                Position _dest, _destAngle;
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());

                if (GetDistance(posMove) > 0.0f && (posMove.GetPositionX() != 0.0f || posMove.GetPositionY() != 0.0f))
                {
                    GetCollisionPosition(_dest, _range, GetAngle(&posMove));
                    _spline.VerticesPoints.emplace_back(_dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ());
                    _spline.VerticesPoints.emplace_back(_dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ());
                }
                else
                {
                    GetCollisionPosition(_dest, _range, atInfo.AngleToCaster + GetOrientation());
                    _spline.VerticesPoints.emplace_back(_dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ());
                    _spline.VerticesPoints.emplace_back(_dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ());
                }

                float angleToAt = _dest.GetAngle(this);
                _dest.SimplePosXYRelocationByAngle(_destAngle, 2.0f, angleToAt + static_cast<float>(M_PI / 2), true);
                GetCollisionPosition(_destAngle, _range + 10.0f, GetAngle(&_destAngle));

                m_moveAngleLos = _dest.GetAngle(&_destAngle) - static_cast<float>(M_PI / 2);

            #ifdef WIN32
                TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AT_MOVE_TYPE_RE_PATH_LOS _timeToTarget %i _dest (%f %f %f) %f",
                    _spline.TimeToTarget, _dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ(), m_moveAngleLos);
            #endif
            }
            break;
        }
        case AT_MOVE_TYPE_PART_PATH: // 9
        {
            if (_range != 0.0f)
            {
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                if (GetDistance(posMove) > 0.0f && (posMove.GetPositionX() != 0.0f || posMove.GetPositionY() != 0.0f))
                {
                    float angleToMove = GetRelativeAngle(&posMove);
                    if ((pos - posMove).length() > (atInfo.Param ? atInfo.Param : 2.0f))
                    {
                        for (uint8 count = 1; count < ((pos - posMove).length() / (atInfo.Param ? atInfo.Param : 2.0f)); count++)
                        {
                            Position midPos;
                            SimplePosXYRelocationByAngle(midPos, (count * (atInfo.Param ? atInfo.Param : 2.0f) * 1.0f), angleToMove);
                            _spline.VerticesPoints.emplace_back(midPos.GetPositionX(), midPos.GetPositionY(), midPos.GetPositionZ());
                        }
                    }
                    _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
                    _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
                }
                else
                {
                    if (_range > (atInfo.Param ? atInfo.Param : 2.0f))
                    {
                        for (uint8 count = 1; count < (_range / (atInfo.Param ? atInfo.Param : 2.0f)); count++)
                        {
                            Position midPos;
                            SimplePosXYRelocationByAngle(midPos, (count * (atInfo.Param ? atInfo.Param : 2.0f) * 1.0f), atInfo.AngleToCaster);
                            _spline.VerticesPoints.emplace_back(midPos.GetPositionX(), midPos.GetPositionY(), midPos.GetPositionZ());
                        }
                    }
                    Position nextPos;
                    SimplePosXYRelocationByAngle(nextPos, _range, atInfo.AngleToCaster);
                    _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
                    _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
                }
            }
            break;
        }
        case AT_MOVE_TYPE_MOVE_FORWARD: // 10
        {
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());

            Position nextPos;
            SimplePosXYRelocationByAngle(nextPos, _range, 0.f/*GetOrientation()*/);
            _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
            _spline.VerticesPoints.emplace_back(nextPos.GetPositionX(), nextPos.GetPositionY(), nextPos.GetPositionZ());
            break;
        }
        case AT_MOVE_TYPE_RE_PATH_TO_CASTER: // 11
        {
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
            _spline.VerticesPoints.emplace_back(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
            break;
        }
        case AT_MOVE_TYPE_RE_PATH_TO_TARGET: // 12
        {
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
            _spline.VerticesPoints.emplace_back(posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ());
            break;
        }
        default:
            break;
    }
    InitSplines();
}

void AreaTrigger::ReCalculateSplinePosition(bool setReach /*= false*/)
{
    if (setReach)
        _reachedDestination = true;

    switch (atInfo.moveType)
    {
        case AT_MOVE_TYPE_RE_PATH:
        case AT_MOVE_TYPE_RE_PATH_TO_CASTER:
        case AT_MOVE_TYPE_RE_PATH_TO_TARGET:
        {
            if ((*this - *_caster).length() < 3.0f)
            {
                if (GetDuration() > 100)
                    SetDuration(100);
                return;
            }

            float dist = GetExactDist2d(_caster);
            if (dist > 3.0f)
            {
                _spline.VerticesPoints.clear();
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
                _spline.VerticesPoints.emplace_back(_caster->GetPositionX(), _caster->GetPositionY(), _caster->GetPositionZ());
                _spline.VerticesPoints.emplace_back(_caster->GetPositionX(), _caster->GetPositionY(), _caster->GetPositionZ());

                _spline.ElapsedTimeForMovement = _liveTime;
                InitSplines();
            }

            #ifdef WIN32
            TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::UpdateMovement AT_MOVE_TYPE_RE_PATH size %i _timeToTarget %i dist %f", _spline.VerticesPoints.size(), _spline.TimeToTarget, dist);
                DebugVisualizePosition();
            #endif
            break;
        }
        case AT_MOVE_TYPE_RE_PATH_LOS: //8
        {
            Position _dest, _destAngle;

            GetCollisionPosition(_dest, _range, m_moveAngleLos);

            _spline.VerticesPoints.clear();
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(GetPositionX(), GetPositionY(), GetPositionZ());
            _spline.VerticesPoints.emplace_back(_dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ());
            _spline.VerticesPoints.emplace_back(_dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ());

            float angleToAt = _dest.GetAngle(this);
            _dest.SimplePosXYRelocationByAngle(_destAngle, 2.0f, angleToAt + static_cast<float>(M_PI / 2), true);
            GetCollisionPosition(_destAngle, _range + 10.0f, GetAngle(&_destAngle));

            m_moveAngleLos = _dest.GetAngle(&_destAngle) - static_cast<float>(M_PI / 2);

            _spline.ElapsedTimeForMovement = _liveTime;
            InitSplines();

            #ifdef WIN32
            TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "AreaTrigger::UpdateMovement AT_MOVE_TYPE_RE_PATH_LOS size %i _timeToTarget %i _dest (%f %f %f) %f",
                _spline.VerticesPoints.size(), _spline.TimeToTarget, _dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZH(), m_moveAngleLos);
                DebugVisualizePosition();
            #endif
            break;
        }
        default:
            break;
    }
}

void AreaTrigger::CalculateCyclicPosition(Position const& pos, Position const& /*posMove*/, Unit* caster)
{
    if (!atInfo.circleTemplate.CanLoop)
    {
        _CircleData = new AreaTriggerCircle;
        _CircleData->CounterClockwise = atInfo.circleTemplate.CounterClockwise;
        _CircleData->CanLoop = atInfo.circleTemplate.CanLoop;
        _CircleData->Radius = atInfo.circleTemplate.Radius;
        _CircleData->BlendFromRadius = atInfo.circleTemplate.Radius;
        float startAngle = caster->GetAngle(&pos);
        _spline.VerticesPoints.emplace_back(atInfo.circleTemplate.Radius * std::cos(startAngle), atInfo.circleTemplate.Radius * std::sin(startAngle), 0.0f);
        _CircleData->InitialAngle = startAngle;
        _CircleData->PathTarget = boost::in_place();
        _CircleData->PathTarget = caster->GetGUID();
        return;
    }

    Unit* target = caster;
    if (GetTargetGuid() && caster->GetGUID() != GetTargetGuid())
        if (Unit* _target = ObjectAccessor::GetUnit(*this, GetTargetGuid()))
            target = _target;

    float radius = atInfo.circleTemplate.IsDinamicRadius ? target->GetExactDist2d(&pos) : atInfo.circleTemplate.Radius;
    float lenght = 2.0f * static_cast<float>(M_PI) * radius;
    float startAngle = target->GetAngle(&pos);
    float calcLenght = 0.0f;
    float calcAngle = 0.0f;
    float onePartLenght = 1.0f;

    if (atInfo.circleTemplate.RandRevers)
        atInfo.circleTemplate.CounterClockwise = urand(0, 1);

    float angleStep = onePartLenght / radius * (atInfo.circleTemplate.CounterClockwise ? 1.0f : -1.0f);
    _spline.VerticesPoints.clear();

    float posX = atInfo.circleTemplate.HasTarget ? 0.0f : target->GetPositionX();
    float posY = atInfo.circleTemplate.HasTarget ? 0.0f : target->GetPositionY();

    float x = posX + radius * std::cos(startAngle);
    float y = posY + radius * std::sin(startAngle);
    _spline.VerticesPoints.emplace_back(x, y, 0.0f);
    // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CalculateCyclicPosition first calcLenght %f startAngle %f x %f y %f lenght %f", calcLenght, startAngle, x, y, lenght);

    while (lenght > calcLenght)
    {
        if (calcLenght + onePartLenght < lenght)
        {
            calcLenght += onePartLenght;
            calcAngle += angleStep;

            x = posX + radius * std::cos(calcAngle + startAngle);
            y = posY + radius * std::sin(calcAngle + startAngle);
            _spline.VerticesPoints.emplace_back(x, y, 0.0f);
            // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CalculateCyclicPosition calcLenght %f calcAngle %f x %f y %f", calcLenght, calcAngle, x, y);
        }
        else
        {
            float lasPart = lenght - calcLenght;
            calcLenght = lenght;
            calcAngle += lasPart / radius * (atInfo.circleTemplate.CounterClockwise ? 1.0f : -1.0f);

            x = posX + radius * std::cos(calcAngle + startAngle);
            y = posY + radius * std::sin(calcAngle + startAngle);
            _spline.VerticesPoints.emplace_back(x, y, 0.0f);
            // TC_LOG_DEBUG(LOG_FILTER_AREATRIGGER, "CalculateCyclicPosition last calcLenght %f calcAngle %f x %f y %f lasPart %f", calcLenght, calcAngle, x, y, lasPart);
        }
    }

    _CircleData = new AreaTriggerCircle;
    _CircleData->CounterClockwise = atInfo.circleTemplate.CounterClockwise;
    _CircleData->CanLoop = atInfo.circleTemplate.CanLoop;
    _CircleData->Radius = radius;
    _CircleData->BlendFromRadius = radius;
    _CircleData->InitialAngle = startAngle;

    if (atInfo.circleTemplate.HasTarget)
    {
        _CircleData->PathTarget = boost::in_place();
        _CircleData->PathTarget = target->GetGUID();
    }
    if (atInfo.circleTemplate.HasCenterPoint)
    {
        _CircleData->Center = boost::in_place();
        _CircleData->Center = Position(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
    }
    InitSplines();
}

void AreaTrigger::DebugVisualizePosition()
{
    if (!sLog->ShouldLog(LOG_FILTER_AREATRIGGER, LOG_LEVEL_DEBUG))
        return;

    if (Unit* caster = GetCaster())
        caster->SummonCreature(44548, *this, TEMPSUMMON_TIMED_DESPAWN, _spline.TimeToTarget);
}

void AreaTrigger::DebugVisualizePolygon()
{
    if (!sLog->ShouldLog(LOG_FILTER_AREATRIGGER, LOG_LEVEL_DEBUG))
        return;

    if (atInfo.Polygon.Vertices.size() < 3)
        return;

    Unit* caster = GetCaster();
    if (!caster)
        return;

    if (!DebugPolygon.empty())
    {
        for (auto& creature : DebugPolygon)
        {
            if (!creature)
                continue;
            float x = creature->GetFollowDistance() * std::cos(creature->GetFollowAngle() + caster->GetOrientation());
            float y = creature->GetFollowDistance() * std::sin(creature->GetFollowAngle() + caster->GetOrientation());
            creature->Relocate(x + caster->GetPositionX(), y + caster->GetPositionY(), caster->GetPositionZ(), caster->GetOrientation());
            // creature->MonsterMoveWithSpeed(x + caster->GetPositionX(), y + caster->GetPositionY(), caster->GetPositionZ(), 100.0f);
        }
        return;
    }

    uint32 count = 1;
    if (!atInfo.Polygon.VerticesTarget.empty())
    {
        for (auto cur_pt : atInfo.Polygon.VerticesTarget)
        {
            float x_source = cur_pt.Pos.m_positionX;
            float y_source = cur_pt.Pos.m_positionY;
            float angleReal = caster->GetAngle(x_source + caster->GetPositionX(), y_source + caster->GetPositionY());
            float angle = caster->GetAngle(x_source + caster->GetPositionX(), y_source + caster->GetPositionY()) + caster->GetOrientation();
            float dist = sqrt(x_source*x_source + y_source*y_source);
            float x = dist * std::cos(angle);
            float y = dist * std::sin(angle);
            if (Creature* creature = caster->SummonCreature(44548, Position(x + caster->GetPositionX(), y + caster->GetPositionY(), caster->GetPositionZ()), TEMPSUMMON_MANUAL_DESPAWN))
            {
                DebugPolygon.push_back(creature);
                creature->SetLevel(count > STRONG_MAX_LEVEL ? STRONG_MAX_LEVEL : count);
                creature->SetFollowAngle(angleReal);
                creature->SetFollowDistance(dist);
                creature->SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_MIN, 0);
                creature->SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_MAX, 0);
                creature->SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_DELTA, 0);
            }
            count++;
        }
    }
    else
        for (auto cur_pt : atInfo.Polygon.Vertices)
        {
            float x_source = cur_pt.Pos.m_positionX;
            float y_source = cur_pt.Pos.m_positionY;
            float angleReal = caster->GetAngle(x_source + caster->GetPositionX(), y_source + caster->GetPositionY());
            float angle = caster->GetAngle(x_source + caster->GetPositionX(), y_source + caster->GetPositionY()) + caster->GetOrientation();
            float dist = sqrt(x_source*x_source + y_source*y_source);
            float x = dist * std::cos(angle);
            float y = dist * std::sin(angle);
            if (Creature* creature = caster->SummonCreature(44548, Position(x + caster->GetPositionX(), y + caster->GetPositionY(), caster->GetPositionZ()), TEMPSUMMON_MANUAL_DESPAWN))
            {
                DebugPolygon.push_back(creature);
                creature->SetLevel(count > STRONG_MAX_LEVEL ? STRONG_MAX_LEVEL : count);
                creature->SetFollowAngle(angleReal);
                creature->SetFollowDistance(dist);
                creature->SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_MIN, 0);
                creature->SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_MAX, 0);
                creature->SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_DELTA, 0);
            }
            count++;
        }
}


float AreaTrigger::GetProgress() const
{
    if (isMoving())
        return movespline->ComputeProgress();

    return float(_liveTime) / float(GetUInt32Value(AREATRIGGER_FIELD_DURATION));
}
