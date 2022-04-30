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

#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "CellImpl.h"
#include "CharmInfo.h"
#include "ChatTextBuilder.h"
#include "ChatPackets.h"
#include "Common.h"
#include "Creature.h"
#include "GameObjectPackets.h"
#include "Garrison.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "MMapFactory.h"
#include "MMapManager.h"
#include "MiscPackets.h"
#include "MovementPackets.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "SharedDefines.h"
#include "SpellPackets.h"
#include "TargetedMovementGenerator.h"
#include "TemporarySummon.h"
#include "Totem.h"
#include "Unit.h"
#include "UpdateData.h"
#include "UpdateFieldFlags.h"
#include "Util.h"
#include "Vehicle.h"
#include "VMapFactory.h"
#include "WaypointMovementGenerator.h"
#include "World.h"
#include "WorldPacket.h"

Object::Object()
{
    m_objectTypeId              = TYPEID_OBJECT;
    m_objectType                = TYPEMASK_OBJECT;
    m_updateFlag                = UPDATEFLAG_NONE;

    m_uint32Values              = nullptr;
    _dynamicValues              = nullptr;
    _dynamicChangesArrayMask    = nullptr;
    m_valuesCount               = 0;
    _dynamicValuesCount         = 0;
    _fieldNotifyFlags           = UF_FLAG_DYNAMIC;

    m_inWorld                   = 0;
    m_objectUpdated             = false;
    m_currMap                   = nullptr;
    m_delete                    = false;
    m_preDelete                 = false;
    m_needLock                  = false;
    MaxVisible                  = false;
    m_Teleports                 = false;
    m_isUpdate                  = false;

    m_spawnMode                 = 0;
}

uint32 Object::GetSize()
{
    uint32 size = m_valuesCount * sizeof(uint32);
    size += _dynamicValuesCount * sizeof(std::vector<uint32>);
    size += _dynamicValuesCount * sizeof(std::vector<uint8>);
    size += m_valuesCount * sizeof(std::vector<uint8>);
    size += _dynamicValuesCount * sizeof(std::vector<UpdateMask::DynamicFieldChangeType>);
    return size;
}

WorldObject::~WorldObject()
{
    // this may happen because there are many !create/delete
    if (IsWorldObject() && m_currMap)
    {
        if (IsCorpse())
        {
            TC_LOG_FATAL(LOG_FILTER_GENERAL, "Object::~Object Corpse guid=" UI64FMTD ", type=%d, entry=%u deleted but still in map!!", GetGUID().GetCounter(), ToCorpse()->GetType(), GetEntry());
            ASSERT(false);
        }
        WorldObject::ResetMap();
    }
}

AccessRequirementKey::AccessRequirementKey(int32 mapId, uint8 difficulty, uint16 dungeonId): _mapId(mapId), _difficulty(difficulty), _dungeonId(dungeonId)
{
}

bool AccessRequirementKey::operator<(AccessRequirementKey const& rhs) const
{
    return std::tie(_mapId, _difficulty, _dungeonId) < std::tie(rhs._mapId, rhs._difficulty, rhs._dungeonId);
}

Object::~Object()
{
    if (IsInWorld())
    {
        TC_LOG_FATAL(LOG_FILTER_GENERAL, "Object::~Object - guid=" UI64FMTD ", typeid=%d, entry=%u deleted but still in world!!", GetGUID().GetCounter(), GetTypeId(), GetEntry());
        if (isType(TYPEMASK_ITEM))
            TC_LOG_FATAL(LOG_FILTER_GENERAL, "Item slot %u",ToItem()->GetSlot());
        //ASSERT(false);
        Object::RemoveFromWorld();
    }

    if (m_objectUpdated)
    {
        TC_LOG_FATAL(LOG_FILTER_GENERAL, "Object::~Object - guid=" UI64FMTD ", typeid=%d, entry=%u deleted but still in update list!!", GetGUID().GetCounter(), GetTypeId(), GetEntry());
        //ASSERT(false);
    }

    delete[] m_uint32Values;
    m_uint32Values = nullptr;

    delete[] _dynamicValues;
    _dynamicValues = nullptr;

    delete[] _dynamicChangesArrayMask;
    _dynamicChangesArrayMask = nullptr;
}

void Object::_InitValues()
{
    m_uint32Values = new uint32[m_valuesCount];
    memset(m_uint32Values, 0, m_valuesCount * sizeof(uint32));

    _changesMask.resize(m_valuesCount);
    _dynamicChangesMask.resize(_dynamicValuesCount);
    if (_dynamicValuesCount)
    {
        _dynamicValues = new std::vector<uint32>[_dynamicValuesCount];
        _dynamicChangesArrayMask = new std::vector<uint8>[_dynamicValuesCount];
    }

    m_objectUpdated = false;
}

void Object::_Create(ObjectGuid const& guid)
{
    if (!m_uint32Values) _InitValues();

    SetGuidValue(OBJECT_FIELD_GUID, guid);
    SetUInt16Value(OBJECT_FIELD_TYPE, 0, m_objectType);
    if (m_objectType & TYPEMASK_PLAYER)
        SetUInt16Value(OBJECT_FIELD_TYPE, 1, 1);
}

std::string Object::_ConcatFields(uint16 startIndex, uint16 size) const
{
    std::ostringstream ss;
    for (uint16 index = 0; index < size; ++index)
        ss << GetUInt32Value(index + startIndex) << ' ';
    return ss.str();
}

bool Object::IsInWorld() const
{
    return m_inWorld == 1;
}

bool Object::IsDelete() const
{
    return m_delete;
}

bool Object::IsPreDelete() const
{
    return m_preDelete;
}

bool Object::IsUpdated() const
{
    return m_objectUpdated;
}

void Object::AddToWorld()
{
    if (IsInWorld())
        return;

    ASSERT(m_uint32Values);

    m_inWorld = 1;

    // synchronize values mirror with values array (changes will send in updatecreate opcode any way
    ClearUpdateMask(true);
}

void Object::RemoveFromWorld()
{
    if (!IsInWorld())
        return;

    m_inWorld = 0;

    // if we remove from world then sending changes not required
    // ClearUpdateMask(true);
    if (m_objectUpdated)
        m_objectUpdated = false;
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    //if (!IsInWorld())
    //    return;

    if (!target)
        return;

    uint8  updateType = UPDATETYPE_CREATE_OBJECT;
    uint16 flags      = m_updateFlag;

    if ((flags & UPDATEFLAG_SELF) != 0)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "BuildCreateUpdateBlockForPlayer start UPDATEFLAG_SELF wrong flags %u for object GUID %s", flags, GetGUID().ToString().c_str());
        return;
    }
    if ((flags & UPDATEFLAG_HAS_WORLDEFFECTID) != 0 && !ToGameObject())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "BuildCreateUpdateBlockForPlayer start UPDATEFLAG_HAS_WORLDEFFECTID wrong flags %u for object GUID %s", flags, GetGUID().ToString().c_str());
        return;
    }
    if ((flags & UPDATEFLAG_ANIMKITS) != 0 && !ToWorldObject()->GetAIAnimKitId() && !ToWorldObject()->GetMovementAnimKitId() && !ToWorldObject()->GetMeleeAnimKitId())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "BuildCreateUpdateBlockForPlayer start UPDATEFLAG_ANIMKITS wrong flags %u for object GUID %s", flags, GetGUID().ToString().c_str());
        return;
    }

    /** lower flag1 **/
    if (target == this)                                      // building packet for yourself
        flags |= UPDATEFLAG_SELF;

    switch (GetGUID().GetHigh())
    {
        case HighGuid::Player:
        case HighGuid::Pet:
        case HighGuid::DynamicObject:
        case HighGuid::AreaTrigger:
            updateType = UPDATETYPE_CREATE_OBJECT;
            break;
        case HighGuid::Corpse:
        case HighGuid::Conversation:
            updateType = UPDATETYPE_CREATE_OBJECT2;
            break;
        case HighGuid::Creature:
        case HighGuid::Vehicle:
            if (ToUnit()->ToTempSummon() && ToUnit()->ToTempSummon()->GetSummonerGUID().IsPlayer())
                updateType = UPDATETYPE_CREATE_OBJECT2;
            break;
        case HighGuid::GameObject:
            if (ToGameObject()->GetOwnerGUID().IsPlayer())
                updateType = UPDATETYPE_CREATE_OBJECT2;
            break;
        default:
            break;
    }

    if (auto worldObject = dynamic_cast<WorldObject const*>(this))
    {
        if (!(flags & UPDATEFLAG_LIVING))
            if (!worldObject->m_movementInfo.transport.Guid.IsEmpty())
                flags |= UPDATEFLAG_GO_TRANSPORT_POSITION;

        if (worldObject->GetAIAnimKitId() || worldObject->GetMovementAnimKitId() || worldObject->GetMeleeAnimKitId())
            flags |= UPDATEFLAG_ANIMKITS;
    }

    if (!(flags & UPDATEFLAG_LIVING))
        if (auto worldObject = dynamic_cast<WorldObject const*>(this))
            if (!worldObject->m_movementInfo.transport.Guid.IsEmpty())
                flags |= UPDATEFLAG_GO_TRANSPORT_POSITION;

    if (flags & UPDATEFLAG_STATIONARY_POSITION)
    {
        // UPDATETYPE_CREATE_OBJECT2 for some gameobject types...
        if (isType(TYPEMASK_GAMEOBJECT))
        {
            switch (ToGameObject()->GetGoType())
            {
                case GAMEOBJECT_TYPE_TRAP:
                case GAMEOBJECT_TYPE_DUEL_ARBITER:
                case GAMEOBJECT_TYPE_FLAGSTAND:
                case GAMEOBJECT_TYPE_FLAGDROP:
                    updateType = UPDATETYPE_CREATE_OBJECT2;
                    break;
                default:
                    break;
            }
        }
    }

    if (Unit const* unit = ToUnit())
        if (unit->getVictim())
            flags |= UPDATEFLAG_HAS_TARGET;

    if ((flags & UPDATEFLAG_SELF) != 0 && !ToPlayer())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "BuildCreateUpdateBlockForPlayer end UPDATEFLAG_SELF wrong flags %u for object GUID %s", flags, GetGUID().ToString().c_str());
        return;
    }
    if ((flags & UPDATEFLAG_HAS_WORLDEFFECTID) != 0 && !ToGameObject())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "BuildCreateUpdateBlockForPlayer end UPDATEFLAG_HAS_WORLDEFFECTID wrong flags %u for object GUID %s", flags, GetGUID().ToString().c_str());
        return;
    }
    if ((flags & UPDATEFLAG_ANIMKITS) != 0 && !ToWorldObject()->GetAIAnimKitId() && !ToWorldObject()->GetMovementAnimKitId() && !ToWorldObject()->GetMeleeAnimKitId())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "BuildCreateUpdateBlockForPlayer end UPDATEFLAG_ANIMKITS wrong flags %u for object GUID %s", flags, GetGUID().ToString().c_str());
        return;
    }

    ByteBuffer buf(0x400);
    buf << uint8(updateType);
    buf << GetGUID();
    buf << uint8(m_objectTypeId);

    BuildMovementUpdate(&buf, flags);
    BuildValuesUpdate(updateType, &buf, target);
    BuildDynamicValuesUpdate(updateType, &buf, target);
    data->AddUpdateBlock(buf);
}

void Object::SendUpdateToPlayer(Player* player)
{
    // send create update to player
    UpdateData upd(player->GetMapId());
    WorldPacket packet;

    if (player->HaveAtClient(static_cast<WorldObject*>(this)))
        BuildValuesUpdateBlockForPlayer(&upd, player);
    else
        BuildCreateUpdateBlockForPlayer(&upd, player);

    if (upd.BuildPacket(&packet))
        player->GetSession()->SendPacket(&packet);
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    if (!IsInWorld())
        return;

    ByteBuffer buf(500);

    buf << uint8(UPDATETYPE_VALUES);
    buf << GetGUID();

    BuildValuesUpdate(UPDATETYPE_VALUES, &buf, target);
    BuildDynamicValuesUpdate(UPDATETYPE_VALUES, &buf, target);

    if (buf.size() > 10000000) // Prevent overflow
        return;

    data->AddUpdateBlock(buf);
}

void Object::BuildOutOfRangeUpdateBlock(UpdateData* data) const
{
    data->AddOutOfRangeGUID(GetGUID());
}

void Object::DestroyForPlayer(Player* target) const
{
    ASSERT(target);

    UpdateData updateData(target->GetMapId());
    BuildOutOfRangeUpdateBlock(&updateData);
    WorldPacket packet;
    if (updateData.BuildPacket(&packet))
        target->SendDirectMessage(&packet);
}

void Object::BuildMovementUpdate(ByteBuffer* data, uint16 flags) const
{
    bool NoBirthAnim = false;
    bool EnablePortals = false;
    bool PlayHoverAnim = (flags & UPDATEFLAG_PLAY_HOVER_ANIM) != 0;
    bool HasMovementUpdate = (flags & UPDATEFLAG_LIVING) != 0;
    bool HasMovementTransport = (flags & UPDATEFLAG_GO_TRANSPORT_POSITION) != 0;
    bool Stationary = (flags & UPDATEFLAG_STATIONARY_POSITION) != 0;
    bool CombatVictim = (flags & UPDATEFLAG_HAS_TARGET) != 0 && ToUnit() && ToUnit()->getVictim();
    bool ServerTime = (flags & UPDATEFLAG_TRANSPORT) != 0;
    bool VehicleCreate = (flags & UPDATEFLAG_VEHICLE) != 0 && ToUnit() && ToUnit()->GetVehicleKit();
    bool AnimKitCreate = (flags & UPDATEFLAG_ANIMKITS) != 0;
    bool Rotation = (flags & UPDATEFLAG_ROTATION) != 0 && ToGameObject();
    bool HasAreaTrigger = (flags & UPDATEFLAG_AREA_TRIGGER) != 0 && ToAreaTrigger();
    bool HasGameObject = (flags & UPDATEFLAG_HAS_WORLDEFFECTID) != 0;
    bool ThisIsYou = (flags & UPDATEFLAG_SELF) != 0;
    bool SmoothPhasing = false;
    bool SceneObjCreate = false;
    bool PlayerCreateData = IsPlayer() && ToUnit()->GetPowerIndex(POWER_RUNES) != MAX_POWERS;

    std::vector<uint32> const* PauseTimes = nullptr;
    uint32 PauseTimesCount = 0;
    if (GameObject const* go = ToGameObject())
    {
        if (StaticTransport const* staticTransport = go->ToStaticTransport())
        {
            PauseTimes = staticTransport->GetGOValue()->Transport.StopFrames;
            PauseTimesCount = PauseTimes->size();
        }
    }

    data->WriteBit(NoBirthAnim);
    data->WriteBit(EnablePortals);
    data->WriteBit(PlayHoverAnim);
    data->WriteBit(HasMovementUpdate);
    data->WriteBit(HasMovementTransport);
    data->WriteBit(Stationary);
    data->WriteBit(CombatVictim);
    data->WriteBit(ServerTime);
    data->WriteBit(VehicleCreate);
    data->WriteBit(AnimKitCreate);
    data->WriteBit(Rotation);
    data->WriteBit(HasAreaTrigger);
    data->WriteBit(HasGameObject);
    data->WriteBit(SmoothPhasing);
    data->WriteBit(ThisIsYou);
    data->WriteBit(SceneObjCreate);
    data->WriteBit(PlayerCreateData);
    data->FlushBits();

    if (HasMovementUpdate)
    {
        Unit const* unit = ToUnit();
        uint32 movementFlags = unit->m_movementInfo.GetMovementFlags();
        uint32 movementFlagsExtra = unit->m_movementInfo.GetExtraMovementFlags();
        // these break update packet
        if (IsCreature())
            movementFlags &= MOVEMENTFLAG_MASK_CREATURE_ALLOWED;
        else
        {
            if (movementFlags & (MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY))
                movementFlags &= ~(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR | MOVEMENTFLAG_FEATHER_FALL);
            if ((movementFlagsExtra & MOVEMENTFLAG2_IGNORE_MOVEMENT_FORCES/*MOVEMENTFLAG2_INTERPOLATED_TURNING*/) == 0)
                movementFlags &= ~MOVEMENTFLAG_FALLING;
        }

        bool HasFallDirection = unit->HasUnitMovementFlag(MOVEMENTFLAG_FALLING);
        bool HasFall = HasFallDirection || movementFlagsExtra & MOVEMENTFLAG2_IGNORE_MOVEMENT_FORCES/*MOVEMENTFLAG2_INTERPOLATED_TURNING*/;
        bool HasSpline = unit->IsSplineEnabled();

        *data << GetGUID();                                             // MoverGUID
        if (unit->m_movementInfo.ClientMoveTime)
            *data << uint32(unit->m_movementInfo.MoveTime);
        else
        {
            *data << uint32(getMSTime() + 1000);                       // time / counter
            const_cast<Unit*>(unit)->m_movementInfo.ChangePosition(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetOrientation());
        }
        *data << float(unit->GetPositionX());
        *data << float(unit->GetPositionY());
        *data << float(unit->GetPositionZ());
        *data << float(G3D::fuzzyEq(unit->GetOrientation(), 0.0f) ? 0.0f : Position::NormalizeOrientation(unit->GetOrientation()));
        *data << float(unit->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING) || unit->m_movementInfo.HasExtraMovementFlag(MOVEMENTFLAG2_ALWAYS_ALLOW_PITCHING) ? Position::NormalizePitch(unit->m_movementInfo.pitch) : 0.0f);
        *data << float(/*unit->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION) ? unit->m_movementInfo.splineElevation : */0.0f);

        *data << uint32(unit->m_movementInfo.RemoveForcesIDs.size());
        *data << uint32(0);                                             // MoveIndex

        for (ObjectGuid const& guid : unit->m_movementInfo.RemoveForcesIDs)
            *data << guid;

        data->WriteBits(movementFlags, 30);
        data->WriteBits(movementFlagsExtra, 18);
        data->WriteBit(!unit->m_movementInfo.transport.Guid.IsEmpty()); // HasTransport
        data->WriteBit(HasFall);                                        // HasFall
        data->WriteBit(HasSpline);                                      // HasSpline - marks that the unit uses spline movement
        data->WriteBit(unit->m_movementInfo.HeightChangeFailed);
        data->WriteBit(unit->m_movementInfo.RemoteTimeValid);

        if (!unit->m_movementInfo.transport.Guid.IsEmpty())
            *data << unit->m_movementInfo.transport;

        if (HasFall)
        {
            *data << uint32(unit->m_movementInfo.fall.fallTime);                   // Time
            *data << float(unit->m_movementInfo.fall.JumpVelocity);                // JumpVelocity

            if (data->WriteBit(HasFallDirection))
            {
                *data << unit->m_movementInfo.fall.Direction;                      // Direction
                *data << float(unit->m_movementInfo.fall.HorizontalSpeed);         // Speed
            }
        }

        *data << float(unit->GetSpeed(MOVE_WALK));
        *data << float(unit->GetSpeed(MOVE_RUN));
        *data << float(unit->GetSpeed(MOVE_RUN_BACK));
        *data << float(unit->GetSpeed(MOVE_SWIM));
        *data << float(unit->GetSpeed(MOVE_SWIM_BACK));
        *data << float(unit->GetSpeed(MOVE_FLIGHT));
        *data << float(unit->GetSpeed(MOVE_FLIGHT_BACK));
        *data << float(unit->GetSpeed(MOVE_TURN_RATE));
        *data << float(unit->GetSpeed(MOVE_PITCH_RATE));

        *data << static_cast<uint32>(unit->m_movementInfo.Forces.size());

        data->WriteBit(HasSpline);
        data->FlushBits();

        for (auto const& force : unit->m_movementInfo.Forces)
            *data << force.second;

        if (HasSpline)
            WorldPackets::Movement::CommonMovement::WriteCreateObjectSplineDataBlock(*unit->movespline, *data);
    }

    *data << PauseTimesCount;

    if (Stationary)
    {
        auto self = static_cast<WorldObject const*>(this);

        *data << float(self->GetStationaryX());
        *data << float(self->GetStationaryY());
        *data << float(self->GetStationaryZ());
        *data << float(Position::NormalizeOrientation(self->GetStationaryO()));
    }

    if (CombatVictim)
        *data << ToUnit()->getVictim()->GetGUID();                      // CombatVictim

    if (ServerTime)
    {
        GameObject const* go = ToGameObject();
        /** @TODO Use IsTransport() to also handle type 11 (TRANSPORT)
            Currently grid objects are not updated if there are no nearby players,
            this causes clients to receive different PathProgress
            resulting in players seeing the object in a different position
        */
        if (go && go->ToTransport()) // PathProgress
            *data << uint32(go->GetPathProgress());
        else
            *data << uint32(getMSTime()); // ServerTime
    }

    if (VehicleCreate)
    {
        Unit const* unit = ToUnit();
        *data << uint32(unit->GetVehicleKit()->GetVehicleInfo()->ID); // RecID
        *data << float(Position::NormalizeOrientation(unit->GetOrientation())); // InitialRawFacing
    }

    if (AnimKitCreate)
    {
        WorldObject const* obj = ToWorldObject();
        *data << uint16(obj->GetAIAnimKitId());                        // AiID
        *data << uint16(obj->GetMovementAnimKitId());                  // MovementID
        *data << uint16(obj->GetMeleeAnimKitId());                     // MeleeID
    }

    if (Rotation && ToGameObject())
        *data << uint64(ToGameObject()->GetPackedWorldRotation());

    if (PauseTimesCount)
        data->append(PauseTimes->data(), PauseTimes->size());

    if (HasMovementTransport)
        *data << static_cast<WorldObject const*>(this)->m_movementInfo.transport;

    if (HasAreaTrigger)
    {
        data->FlushBits();
        AreaTrigger const* t = ToAreaTrigger();
        ASSERT(t);

        if (t->GetAreaTriggerInfo().ElapsedTime)
            *data << uint32(t->GetAreaTriggerInfo().ElapsedTime);       // Elapsed Time Ms
        else
            *data << uint32(1);                                         // Elapsed Time Ms

        *data << t->GetAreaTriggerInfo().RollPitchYaw1X;
        *data << t->GetAreaTriggerInfo().RollPitchYaw1Y;
        *data << t->GetAreaTriggerInfo().RollPitchYaw1Z;

        data->WriteBit(t->GetAreaTriggerInfo().HasAbsoluteOrientation); // HasAbsoluteOrientation
        data->WriteBit(t->GetAreaTriggerInfo().HasDynamicShape);        // HasDynamicShape
        data->WriteBit(t->GetAreaTriggerInfo().HasAttached);            // HasAttached is have transport, not complete
        data->WriteBit(t->GetAreaTriggerInfo().HasFaceMovementDir);     // HasFaceMovementDir
        data->WriteBit(t->GetAreaTriggerInfo().HasFollowsTerrain);      // HasFollowsTerrain
        data->WriteBit(false);                                          // unkbit
        data->WriteBit(t->HasTargetRollPitchYaw());                     // HasTargetRollPitchYaw
        data->WriteBit(t->GetAreaTriggerInfo().ScaleCurveID);           // HasScaleCurveID
        data->WriteBit(t->GetAreaTriggerInfo().MorphCurveID);           // HasMorphCurveID
        data->WriteBit(t->GetAreaTriggerInfo().FacingCurveID);          // HasFacingCurveID
        data->WriteBit(t->GetAreaTriggerInfo().MoveCurveID);            // hasMoveCurveID
        data->WriteBit(t->GetAreaTriggerInfo().sequenceTemplate.oncreated); // HasSequenceAnimation
        data->WriteBit(t->GetAreaTriggerInfo().sequenceTemplate.oncreated); // HasSequenceEntered
        data->WriteBit(t->GetAreaTriggerInfo().sequenceTemplate.oncreated); // HasSequenceElapsedMs
        data->WriteBit(t->GetVisualScale() != 0.0f);                    // HasAreaTriggerSphere
        data->WriteBit(t->GetAreaTriggerInfo().hasAreaTriggerBox);      // HasAreaTriggerBox
        data->WriteBit(t->HasPolygon());                                // AreaTriggerPolygon
        data->WriteBit(t->GetAreaTriggerCylinder());                    // areaTriggerCylinder
        data->WriteBit(t->HasSpline());                                 // areaTriggerSpline
        data->WriteBit(t->HasCircleData());                             // areaTriggerCircle

         if (t->GetAreaTriggerInfo().sequenceTemplate.oncreated)
            data->WriteBit(t->GetAreaTriggerInfo().sequenceTemplate.entered);

        if (t->HasSpline())
            *data << t->GetSplineInfo();

        if (t->HasTargetRollPitchYaw())
        {
            *data << t->GetAreaTriggerInfo().TargetRollPitchYawX;
            *data << t->GetAreaTriggerInfo().TargetRollPitchYawY;
            *data << t->GetAreaTriggerInfo().TargetRollPitchYawZ;
        }

        if (t->GetAreaTriggerInfo().ScaleCurveID)
            *data << uint32(t->GetAreaTriggerInfo().ScaleCurveID);

        if (t->GetAreaTriggerInfo().MorphCurveID)
            *data << uint32(t->GetAreaTriggerInfo().MorphCurveID);

        if (t->GetAreaTriggerInfo().FacingCurveID)
            *data << uint32(t->GetAreaTriggerInfo().FacingCurveID);

        if (t->GetAreaTriggerInfo().MoveCurveID)
            *data << uint32(t->GetAreaTriggerInfo().MoveCurveID);

        if (t->GetAreaTriggerInfo().sequenceTemplate.oncreated)
            *data << uint32(t->GetAreaTriggerInfo().sequenceTemplate.animationid); // SequenceAnimationID

        if (t->GetAreaTriggerInfo().sequenceTemplate.oncreated)
            *data << uint32(1); // SequenceElapsedMs

        if (t->GetVisualScale() != 0.0f)                                        // areaTriggerSphere
        {
            *data << t->GetVisualScale();                           // Radius
            *data << t->GetVisualScale(true);                       // RadiusTarget
        }

        if (t->GetAreaTriggerInfo().hasAreaTriggerBox)
        {
            //*data << Vector3(Extents);                instead of field below oO
            //*data << Vector3(ExtentsTarget);
            *data << t->GetAreaTriggerInfo().Radius;                    // Radius
            *data << t->GetAreaTriggerInfo().Polygon.Height;            // Height
            *data << t->GetAreaTriggerInfo().LocationZOffset;           // LocationZOffset
            *data << t->GetAreaTriggerInfo().RadiusTarget;              // RadiusTarget
            *data << t->GetAreaTriggerInfo().Polygon.HeightTarget;      // HeightTarget
            *data << t->GetAreaTriggerInfo().LocationZOffsetTarget;     // LocationZOffsetTarget
        }

        if (t->HasPolygon())
            *data << t->GetAreaTriggerInfo().Polygon;

        if (t->GetAreaTriggerCylinder())                                // areaTriggerCylinder
        {
            *data << t->GetAreaTriggerInfo().Radius;                    // Radius
            *data << t->GetAreaTriggerInfo().RadiusTarget;              // RadiusTarget
            *data << t->GetAreaTriggerInfo().Polygon.Height;            // Height
            *data << t->GetAreaTriggerInfo().Polygon.HeightTarget;      // HeightTarget
            *data << t->GetAreaTriggerInfo().LocationZOffset;           // Float4
            *data << t->GetAreaTriggerInfo().LocationZOffsetTarget;     // LocationZOffsetTarget
        }

        if (t->HasCircleData())
            *data << *t->GetCircleData();
    }

    if (HasGameObject)
    {
        if (GameObject const* go = ToGameObject())
            *data << uint32(go->GetGOInfo()->visualData[0].WorldEffectID);
        else
            *data << uint32(0);

        data->WriteBit(false); // bit8
        //if (bit8)
            //*data << uint32(Int1);
    }

    //if (SmoothPhasing)
    //{
    //    data->WriteBit(ReplaceActive);
    //    data->WriteBit(HasReplaceObjectt);
    //    if (HasReplaceObject)
    //        *data << ObjectGuid(ReplaceObject);
    //}

    //if (SceneObjCreate)
    //{
    //    var CliSceneLocalScriptData = packet.ReadBit("CliSceneLocalScriptData", index);
    //    var PetBattleFullUpdate = packet.ReadBit("PetBattleFullUpdate", index);

    //    if (CliSceneLocalScriptData)
    //        packet.ReadWoWString("Data", packet.ReadBits(7), index);

    //    if (PetBattleFullUpdate)
    //      *data <<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleFullUpdate const& update);

    if (PlayerCreateData)
    {
        auto HasSceneInstanceIDs = false;
        auto HasRuneState = ToUnit()->GetPowerIndex(POWER_RUNES) != MAX_POWERS;

        data->WriteBit(HasSceneInstanceIDs);
        data->WriteBit(HasRuneState);
        data->FlushBits();

        //if (HasSceneInstanceIDs)
        //{
        //    *data << uint32(SceneInstanceIDs.size());
        //    for (std::size_t i = 0; i < SceneInstanceIDs.size(); ++i)
        //        *data << uint32(SceneInstanceIDs[i]);
        //}

        if (HasRuneState)
        {
            auto player = ToPlayer();
            auto baseCd = float(player->GetRuneBaseCooldown());
            auto maxRunes = uint32(player->GetMaxPower(POWER_RUNES));

            *data << uint8((1 << maxRunes) - 1);
            *data << uint8(player->GetRunesState());
            *data << uint32(maxRunes);
            for (uint32 i = 0; i < maxRunes; ++i)
                *data << uint8((baseCd - float(player->GetRuneCooldown(i))) / baseCd * 255);
        }
    }
}

void Object::BuildValuesUpdate(uint8 updateType, ByteBuffer* data, Player* target) const
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
        if (_fieldNotifyFlags & flags[index] || (updateType == UPDATETYPE_VALUES ? _changesMask[index] : m_uint32Values[index]) && flags[index] & visibleFlag)
        {
            UpdateMask::SetUpdateBit(data->contents() + maskPos, index);
            *data << m_uint32Values[index];
        }
    }
}

void Object::BuildDynamicValuesUpdate(uint8 updateType, ByteBuffer *data, Player* target) const
{
    if (!target)
        return;

    std::size_t blockCount = UpdateMask::GetBlockCount(_dynamicValuesCount);

    uint32* flags = nullptr;
    uint32 visibleFlag = GetDynamicUpdateFieldData(target, flags);

    *data << uint8(blockCount);
    std::size_t maskPos = data->wpos();
    data->resize(data->size() + blockCount * sizeof(UpdateMask::BlockType));

    for (uint16 index = 0; index < _dynamicValuesCount; ++index)
    {
        std::vector<uint32> const& values = _dynamicValues[index];

        if (_fieldNotifyFlags & flags[index] ||
            (updateType == UPDATETYPE_VALUES ? _dynamicChangesMask[index] != UpdateMask::UNCHANGED : !values.empty()) && flags[index] & visibleFlag)
        {
            if (index == PLAYER_DYNAMIC_FIELD_ARENA_COOLDOWNS && updateType == UPDATETYPE_VALUES)
            {
                uint32 pTimeSyncClient = target->GetTimeSyncClient();
                uint32 cooldown = values[6] - values[4];

                _dynamicValues[index][4] = pTimeSyncClient;
                _dynamicValues[index][6] = pTimeSyncClient + cooldown;
            }

            UpdateMask::SetUpdateBit(data->contents() + maskPos, index);

            std::size_t arrayBlockCount = UpdateMask::GetBlockCount(values.size());
            *data << uint16(UpdateMask::EncodeDynamicFieldChangeType(arrayBlockCount, _dynamicChangesMask[index], updateType));
            if (_dynamicChangesMask[index] == UpdateMask::VALUE_AND_SIZE_CHANGED && updateType == UPDATETYPE_VALUES)
                *data << uint32(values.size());

            std::size_t arrayMaskPos = data->wpos();
            data->resize(data->size() + arrayBlockCount * sizeof(UpdateMask::BlockType));
            for (std::size_t v = 0; v < values.size(); ++v)
            {
                if (updateType != UPDATETYPE_VALUES || _dynamicChangesArrayMask[index][v])
                {
                    UpdateMask::SetUpdateBit(data->contents() + arrayMaskPos, v);
                    *data << uint32(values[v]);
                }
            }
        }
    }
}

void Object::AddToObjectUpdateIfNeeded()
{
    if (IsInWorld() && !m_objectUpdated)
    {
        if (Map* map = GetMap())
            map->AddUpdateObject(this);
        m_objectUpdated = true;
    }
}

void Object::ClearUpdateMask(bool /*remove*/)
{
    std::lock_guard<std::recursive_mutex> _update_lock(m_update_lock);

    memset(_changesMask.data(), 0, _changesMask.size());
    _dynamicChangesMask.assign(_dynamicChangesMask.size(), UpdateMask::UNCHANGED);
    for (uint32 i = 0; i < _dynamicValuesCount; ++i)
        memset(_dynamicChangesArrayMask[i].data(), 0, _dynamicChangesArrayMask[i].size());

    if (m_objectUpdated)
        m_objectUpdated = false;
}

void Object::BuildFieldsUpdate(Player* player, UpdateDataMapType& data_map) const
{
    auto iter = data_map.find(player);

    if (iter == data_map.end())
    {
        auto p = data_map.emplace(player, UpdateData(player->GetMapId()));
        ASSERT(p.second);
        iter = p.first;
    }

    BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);
}

uint32 Object::GetUpdateFieldData(Player const* target, uint32*& flags) const
{
    uint32 visibleFlag = UF_FLAG_PUBLIC;

    if (target == this)
        visibleFlag |= UF_FLAG_PRIVATE;

    // This function assumes updatefield index is always valid
    switch (GetTypeId())
    {
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            flags = ItemUpdateFieldFlags;
            if (IsItem() && ToItem()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER | UF_FLAG_ITEM_OWNER;
            break;
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
        {
            Player* plr = ToUnit()->GetCharmerOrOwnerPlayerOrPlayerItself();
            flags = UnitUpdateFieldFlags;
            if (ToUnit()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;

            if (HasFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO))
                if (ToUnit()->HasAuraTypeWithCaster(SPELL_AURA_EMPATHY, target->GetGUID()))
                    visibleFlag |= UF_FLAG_SPECIAL_INFO;

            if (plr && plr->IsInSameRaidWith(target))
                visibleFlag |= UF_FLAG_PARTY_MEMBER;
            break;
        }
        case TYPEID_GAMEOBJECT:
            flags = GameObjectUpdateFieldFlags;
            if (ToGameObject()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            break;
        case TYPEID_DYNAMICOBJECT:
            flags = DynamicObjectUpdateFieldFlags;
            if (ToDynObject()->GetCasterGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            break;
        case TYPEID_CORPSE:
            flags = CorpseUpdateFieldFlags;
            if (ToCorpse()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            break;
        case TYPEID_AREATRIGGER:
            flags = AreaTriggerUpdateFieldFlags;
            if (ToAreaTrigger()->GetCasterGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            break;
        case TYPEID_SCENEOBJECT:
            flags = SceneObjectUpdateFieldFlags;
            break;
        case TYPEID_CONVERSATION:
            flags = ConversationUpdateFieldFlags;
            break;
        default:
            break;
    }
    return visibleFlag;
}

uint32 Object::GetDynamicUpdateFieldData(Player const* target, uint32*& flags) const
{
    uint32 visibleFlag = UF_FLAG_PUBLIC;

    if (target == this)
        visibleFlag |= UF_FLAG_PRIVATE;

    switch (GetTypeId())
    {
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            flags = ItemDynamicFieldFlags;
            if (IsItem() && ToItem()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER | UF_FLAG_ITEM_OWNER;
            break;
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
        {
            visibleFlag |= UF_FLAG_UNIT_ALL;

            Player* plr = ToUnit()->GetCharmerOrOwnerPlayerOrPlayerItself();
            flags = UnitDynamicFieldFlags;
            if (ToUnit()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;

            if (HasFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO))
                if (ToUnit()->HasAuraTypeWithCaster(SPELL_AURA_EMPATHY, target->GetGUID()))
                    visibleFlag |= UF_FLAG_SPECIAL_INFO;

            if (plr && plr->IsInSameRaidWith(target))
                visibleFlag |= UF_FLAG_PARTY_MEMBER;
            break;
        }
        case TYPEID_GAMEOBJECT:
            flags = GameObjectDynamicUpdateFieldFlags;
            break;
        case TYPEID_CONVERSATION:
            flags = ConversationDynamicFieldFlags;
            break;
        default:
            flags = nullptr;
            break;
    }

    return visibleFlag;
}

void Object::_LoadIntoDataField(std::string const& data, uint32 startOffset, uint32 count)
{
    if (data.empty())
        return;

    Tokenizer tokens(data, ' ', count);

    if (tokens.size() != count)
        return;

    for (uint32 index = 0; index < count; ++index)
    {
        m_uint32Values[startOffset + index] = atoul(tokens[index]);
        _changesMask[startOffset + index] = 1;
    }
}

void Object::SetInt32Value(uint16 index, int32 value)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (m_int32Values[index] != value)
    {
        m_int32Values[index] = value;
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::UpdateInt32Value(uint16 index, int32 value)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    m_int32Values[index] = value;
    _changesMask[index] = 1;
}

void Object::SetUInt32Value(uint16 index, uint32 value)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (m_uint32Values[index] != value)
    {
        m_uint32Values[index] = value;
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::UpdateUInt32Value(uint16 index, uint32 value)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    m_uint32Values[index] = value;
    _changesMask[index] = 1;
}

void Object::SetUInt64Value(uint16 index, uint64 value)
{
    // ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, true));
    if (!(index + 1 < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (*reinterpret_cast<uint64*>(&m_uint32Values[index]) != value)
    {
        m_uint32Values[index] = PAIR64_LOPART(value);
        m_uint32Values[index + 1] = PAIR64_HIPART(value);
        _changesMask[index] = 1;
        _changesMask[index + 1] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

bool Object::AddGuidValue(uint16 index, ObjectGuid const& value)
{
    // ASSERT(index + 3 < m_valuesCount || PrintIndexError(index, true));
    if (!(index + 3 < m_valuesCount || PrintIndexError(index, true)))
        return false;

    if (!value.IsEmpty() && reinterpret_cast<ObjectGuid*>(&m_uint32Values[index])->IsEmpty())
    {
        *reinterpret_cast<ObjectGuid*>(&m_uint32Values[index]) = value;
        _changesMask[index] = 1;
        _changesMask[index + 1] = 1;
        _changesMask[index + 2] = 1;
        _changesMask[index + 3] = 1;

        AddToObjectUpdateIfNeeded();
        return true;
    }

    return false;
}

bool Object::RemoveGuidValue(uint16 index, ObjectGuid const& value)
{
    // ASSERT(index + 3 < m_valuesCount || PrintIndexError(index, true));
    if (!(index + 3 < m_valuesCount || PrintIndexError(index, true)))
        return false;

    if (!value.IsEmpty() && *reinterpret_cast<ObjectGuid*>(&m_uint32Values[index]) == value)
    {
        reinterpret_cast<ObjectGuid*>(&m_uint32Values[index])->Clear();
        _changesMask[index] = 1;
        _changesMask[index + 1] = 1;
        _changesMask[index + 2] = 1;
        _changesMask[index + 3] = 1;

        AddToObjectUpdateIfNeeded();
        return true;
    }

    return false;
}

void Object::SetFloatValue(uint16 index, float value)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (m_floatValues[index] != value)
    {
        m_floatValues[index] = value;
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::SetByteValue(uint16 index, uint8 offset, uint8 value)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (offset > 3)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[index] >> offset * 8) != value)
    {
        m_uint32Values[index] &= ~uint32(uint32(0xFF) << offset * 8);
        m_uint32Values[index] |= uint32(uint32(value) << offset * 8);
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value, bool update)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (offset > 1)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if (uint16(m_uint32Values[index] >> offset * 16) != value)
    {
        m_uint32Values[index] &= ~uint32(uint32(0xFFFF) << offset * 16);
        m_uint32Values[index] |= uint32(uint32(value) << offset * 16);
        _changesMask[index] = 1;

        if (update)
            AddToObjectUpdateIfNeeded();
    }
}

void Object::SetGuidValue(uint16 index, ObjectGuid const& value)
{
    // ASSERT(index + 3 < m_valuesCount || PrintIndexError(index, true));
    if (!(index + 3 < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (*reinterpret_cast<ObjectGuid*>(&m_uint32Values[index]) != value)
    {
        *reinterpret_cast<ObjectGuid*>(&m_uint32Values[index]) = value;
        _changesMask[index] = 1;
        _changesMask[index + 1] = 1;
        _changesMask[index + 2] = 1;
        _changesMask[index + 3] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::SetStatFloatValue(uint16 index, float value)
{
    if (value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value(uint16 index, int32 value)
{
    if (value < 0)
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += apply ? val : -val;
    if (cur < 0)
        cur = 0;
    SetUInt32Value(index, cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetInt32Value(index);
    cur += apply ? val : -val;
    SetInt32Value(index, cur);
}

void Object::ApplyModUInt16Value(uint16 index, uint8 offset, int16 val, bool apply)
{
    int16 cur = GetUInt16Value(index, offset);
    cur += apply ? val : -val;
    if (cur < 0)
        cur = 0;
    SetUInt16Value(index, offset, cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += apply ? val : -val;
    SetFloatValue(index, cur);
}

void Object::ApplyPercentModFloatValue(uint16 index, float val, bool apply)
{
    float value = GetFloatValue(index);
    ApplyPercentModFloatVar(value, val, apply);
    SetFloatValue(index, value);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += apply ? val : -val;
    if (cur < 0)
        cur = 0;
    SetFloatValue(index, cur);
}

void Object::SetFlag(uint16 index, uint32 newFlag)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    uint32 oldval = m_uint32Values[index];
    uint32 newval = oldval | newFlag;

    if (oldval != newval)
    {
        m_uint32Values[index] = newval;
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::RemoveFlag(uint16 index, uint32 oldFlag)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    ASSERT(m_uint32Values);

    uint32 oldval = m_uint32Values[index];
    uint32 newval = oldval & ~oldFlag;

    if (oldval != newval)
    {
        m_uint32Values[index] = newval;
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::ToggleFlag(uint16 index, uint32 flag)
{
    if (HasFlag(index, flag))
        RemoveFlag(index, flag);
    else
        SetFlag(index, flag);
}

bool Object::HasFlag(uint16 index, uint32 flag) const
{
    if (index >= m_valuesCount && !PrintIndexError(index, false))
        return false;

    return (m_uint32Values[index] & flag) != 0;
}

void Object::ApplyModFlag(uint16 index, uint32 flag, bool apply)
{
    if (apply) SetFlag(index, flag); else RemoveFlag(index, flag);
}

void Object::SetByteFlag(uint16 index, uint8 offset, uint8 newFlag)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (offset > 3)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if (!(uint8(m_uint32Values[index] >> offset * 8) & newFlag))
    {
        m_uint32Values[index] |= uint32(uint32(newFlag) << offset * 8);
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::RemoveByteFlag(uint16 index, uint8 offset, uint8 oldFlag)
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    if (!(index < m_valuesCount || PrintIndexError(index, true)))
        return;

    if (offset > 3)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[index] >> offset * 8) & oldFlag)
    {
        m_uint32Values[index] &= ~uint32(uint32(oldFlag) << offset * 8);
        _changesMask[index] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

void Object::ToggleByteFlag(uint16 index, uint8 offset, uint8 flag)
{
    if (HasByteFlag(index, offset, flag))
        RemoveByteFlag(index, offset, flag);
    else
        SetByteFlag(index, offset, flag);
}

bool Object::HasByteFlag(uint16 index, uint8 offset, uint8 flag) const
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    if (!(index < m_valuesCount || PrintIndexError(index, false)))
        return false;

    ASSERT(offset < 4);
    return (reinterpret_cast<uint8*>(&m_uint32Values[index])[offset] & flag) != 0;
}

void Object::SetFlag64(uint16 index, uint64 newFlag)
{
    uint64 oldval = GetUInt64Value(index);
    uint64 newval = oldval | newFlag;
    SetUInt64Value(index, newval);
}

void Object::RemoveFlag64(uint16 index, uint64 oldFlag)
{
    uint64 oldval = GetUInt64Value(index);
    uint64 newval = oldval & ~oldFlag;
    SetUInt64Value(index, newval);
}

void Object::ToggleFlag64(uint16 index, uint64 flag)
{
    if (HasFlag64(index, flag))
        RemoveFlag64(index, flag);
    else
        SetFlag64(index, flag);
}

bool Object::HasFlag64(uint16 index, uint64 flag) const
{
    // ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    if (!(index < m_valuesCount || PrintIndexError(index, false)))
        return false;

    return (GetUInt64Value(index) & flag) != 0;
}

void Object::ApplyModFlag64(uint16 index, uint64 flag, bool apply)
{
    if (apply) SetFlag64(index, flag); else RemoveFlag64(index, flag);
}

std::vector<uint32> const& Object::GetDynamicValues(uint16 index) const
{
    // ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
    if (!(index < _dynamicValuesCount || PrintIndexError(index, false)))
        std::vector<uint32>();
    return _dynamicValues[index];
}

uint32 Object::GetDynamicValue(uint16 index, uint16 offset) const
{
    // ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
    if (!(index < _dynamicValuesCount || PrintIndexError(index, false)))
        return 0;

    if (offset >= _dynamicValues[index].size())
        return 0;
    return _dynamicValues[index][offset];
}

void Object::AddDynamicValue(uint16 index, uint32 value)
{
    // ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
    if (!(index < _dynamicValuesCount || PrintIndexError(index, false)))
        return;

    SetDynamicValue(index, _dynamicValues[index].size(), value);
}

void Object::RemoveDynamicValue(uint16 index, uint32 value)
{
    // ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
    if (!(index < _dynamicValuesCount || PrintIndexError(index, false)))
        return;

    // TODO: Research if this is blizzlike to just set value to 0
    std::vector<uint32>& values = _dynamicValues[index];
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (values[i] == value)
        {
            values[i] = 0;
            _dynamicChangesMask[index] = UpdateMask::VALUE_CHANGED;
            _dynamicChangesArrayMask[index][i] = 1;

            AddToObjectUpdateIfNeeded();
        }
    }
}

void Object::ClearDynamicValue(uint16 index)
{
    // ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
    if (!(index < _dynamicValuesCount || PrintIndexError(index, false)))
        return;

    if (!_dynamicValues[index].empty())
    {
        std::lock_guard<std::recursive_mutex> _update_lock(m_update_lock);
        _dynamicValues[index].clear();
        _dynamicChangesMask[index] = UpdateMask::VALUE_AND_SIZE_CHANGED;
        _dynamicChangesArrayMask[index].clear();

        AddToObjectUpdateIfNeeded();
    }
}

void Object::SetDynamicValue(uint16 index, uint16 offset, uint32 value)
{
    // ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
    if (!(index < _dynamicValuesCount || PrintIndexError(index, false)))
        return;

    std::lock_guard<std::recursive_mutex> _update_lock(m_update_lock);

    UpdateMask::DynamicFieldChangeType changeType = UpdateMask::VALUE_CHANGED;
    std::vector<uint32>& values = _dynamicValues[index];
    if (values.size() <= offset)
    {
        values.resize(offset + 1);
        changeType = UpdateMask::VALUE_AND_SIZE_CHANGED;
    }

    if (_dynamicChangesArrayMask[index].size() <= offset)
        _dynamicChangesArrayMask[index].resize((offset / 32 + 1) * 32);

    if (values[offset] != value || changeType == UpdateMask::VALUE_AND_SIZE_CHANGED)
    {
        values[offset] = value;
        _dynamicChangesMask[index] = changeType;
        _dynamicChangesArrayMask[index][offset] = 1;

        AddToObjectUpdateIfNeeded();
    }
}

uint32 Object::GetUInt32Value(uint16 index) const
{
    if (!m_uint32Values)
        return uint32(0);
    if (index < m_valuesCount)
        return m_uint32Values[index];
    if (PrintIndexError(index, false))
        return m_uint32Values[index];
    return uint32(0);
}

int32 Object::GetInt32Value(uint16 index) const
{
    //ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    if (index < m_valuesCount || PrintIndexError(index, false))
        return m_int32Values[index];
    return int32(0);
}

uint64 Object::GetUInt64Value(uint16 index) const
{
    //ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, false));
    if (index + 1 < m_valuesCount || PrintIndexError(index, false))
        return *reinterpret_cast<uint64*>(&m_uint32Values[index]);
    return uint64(0);
}

float Object::GetFloatValue(uint16 index) const
{
    //ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    if (index < m_valuesCount || PrintIndexError(index, false))
        return m_floatValues[index];
    return float(0);
}

uint8 Object::GetByteValue(uint16 index, uint8 offset) const
{
    //ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    //ASSERT(offset < 4);
    if ((index < m_valuesCount || PrintIndexError(index, false)) && offset < 4)
        return *(reinterpret_cast<uint8*>(&m_uint32Values[index]) + offset);
    return uint8(0);
}

uint16 Object::GetUInt16Value(uint16 index, uint8 offset) const
{
    //ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    //ASSERT(offset < 2);
    if ((index < m_valuesCount || PrintIndexError(index, false)) && offset < 2)
        return *(reinterpret_cast<uint16*>(&m_uint32Values[index]) + offset);
    return uint16(0);
}

ObjectGuid const& Object::GetGuidValue(uint16 index) const
{
    if (!this)
        return ObjectGuid::Empty;

    // ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, false));
    if (index + 1 < m_valuesCount || PrintIndexError(index, false))
        return *reinterpret_cast<ObjectGuid*>(&m_uint32Values[index]);
    return ObjectGuid::Empty;
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    TC_LOG_ERROR(LOG_FILTER_GENERAL, "Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u", set ? "set value to" : "get value from", index, m_valuesCount, GetTypeId(), m_objectType);

    // ASSERT must fail after function call
    return false;
}

void Object::SetMap(Map* map)
{
    if (!map)
        return;
    if (m_currMap == map)
        return;
    if (m_currMap)
        return;

    m_currMap = map;
}

void Object::ResetMap()
{
    m_currMap = nullptr;
}

WorldObject::WorldObject(bool isWorldObject): LastUsedScriptID(0), m_transport(nullptr), m_name(""), m_isActive(false), m_isWorldObject(isWorldObject), m_zoneScript(nullptr), m_InstanceId(0), m_phaseMask(PHASEMASK_NORMAL), m_ignorePhaseIdCheck(false)
{
    m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE | GHOST_VISIBILITY_GHOST);
    m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
    m_deleted = false;
    m_rwVisibility = false;
    m_rwVisibilityRange = 0.0f;
    m_zoneForce = false;
    m_zoneId = 0;
    m_oldZoneId = 0;
    m_areaId = 0;
    m_oldAreaId = 0;
}

void WorldObject::SetWorldObject(bool on)
{
    if (!IsInWorld())
        return;

    GetMap()->AddObjectToSwitchList(this, on);
}

void WorldObject::SetTratsport(Transport* transport, Unit* owner)
{
    if (!transport)
    {
        if (owner)
        {
            float x, y, z = 0.0f;
            owner->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);
            if (Unit* unit = ToUnit())
                unit->NearTeleportTo(x, y, z, owner->GetOrientation());
        }
        return;
    }

    // This object must be added to transport before adding to map for the client to properly display it
    transport->AddPassenger(this);

    if (owner)
    {
        float x, y, z, o;
        owner->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);
        Position pos(x, y, z, owner->GetOrientation());
        transport->CalculatePassengerOffset(x, y, z, &o);
        m_movementInfo.transport.Pos.Relocate(x, y, z, o);

        if (Unit* unit = ToUnit())
            unit->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), owner->GetOrientation());
    }
    else
    {
        float x, y, z, o;
        GetPosition(x, y, z, o);
        transport->CalculatePassengerOffset(x, y, z, &o);
        m_movementInfo.transport.Pos.Relocate(x, y, z, o);
    }
}

Position WorldObject::GetPosition() const
{
    return Position::GetPosition();
}

void WorldObject::GetPosition(float& x, float& y, Transport* transport) const
{
    if (transport)
    {
        x = m_movementInfo.transport.Pos.GetPositionX();
        y = m_movementInfo.transport.Pos.GetPositionY();
        return;
    }
    Position::GetPosition(x, y);
}

void WorldObject::GetPosition(float& x, float& y, float& z, Transport* transport) const
{
    if (transport)
    {
        x = m_movementInfo.transport.Pos.GetPositionX();
        y = m_movementInfo.transport.Pos.GetPositionY();
        z = m_movementInfo.transport.Pos.GetPositionZH();
        return;
    }
    Position::GetPosition(x, y, z);
}

void WorldObject::GetPosition(float& x, float& y, float& z, float& o, Transport* transport) const
{
    if (transport)
    {
        x = m_movementInfo.transport.Pos.GetPositionX();
        y = m_movementInfo.transport.Pos.GetPositionY();
        z = m_movementInfo.transport.Pos.GetPositionZH();
        o = m_movementInfo.transport.Pos.GetOrientation();
        return;
    }
    Position::GetPosition(x, y, z, o);
}

void WorldObject::GetPosition(Position* pos, Transport* transport) const
{
    if (transport)
    {
        if (pos)
            pos->Relocate(m_movementInfo.transport.Pos);
        return;
    }
    Position::GetPosition(pos);
}

void WorldObject::Relocate(float x, float y, float z, float orientation)
{
    if(!Trinity::IsValidMapCoord(x, y, z))
        return;

    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    m_orientation = orientation;

    m_movementInfo.ChangePosition(x, y, z, orientation);
    m_movementInfo.UpdateTime(getMSTime());
    /*if (Transport* t = GetTransport())
    {
        t->CalculatePassengerOffset(x, y, z);
        m_movementInfo.t_pos.x = x;
        m_movementInfo.t_pos.y = y;
        m_movementInfo.t_pos.z = z;
    }*/
}

void WorldObject::Relocate(float x, float y, float z)
{
    Relocate(x, y, z, GetOrientation());
}

void WorldObject::Relocate(float x, float y)
{
    Relocate(x, y, m_positionZ, GetOrientation());
}

void WorldObject::Relocate(const Position &pos)
{
    Relocate(pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation);
}

void WorldObject::Relocate(const Position* pos)
{
    Relocate(pos->m_positionX, pos->m_positionY, pos->m_positionZ, pos->m_orientation);
}

void WorldObject::SetOrientation(float orientation)
{
    m_orientation = orientation;

    if (Unit* unit = ToUnit())
        unit->m_movementInfo.ChangeOrientation(orientation);
}

Transport* WorldObject::GetTransport() const
{
    return m_transport;
}

float WorldObject::GetTransOffsetX() const
{
    return m_movementInfo.transport.Pos.GetPositionX();
}

float WorldObject::GetTransOffsetY() const
{
    return m_movementInfo.transport.Pos.GetPositionY();
}

float WorldObject::GetTransOffsetZ() const
{
    return m_movementInfo.transport.Pos.GetPositionZ();
}

float WorldObject::GetTransOffsetO() const
{
    return m_movementInfo.transport.Pos.GetOrientation();
}

Position const& WorldObject::GetTransOffset() const
{
    return m_movementInfo.transport.Pos;
}

uint32 WorldObject::GetTransTime() const
{
    return m_movementInfo.transport.MoveTime;
}

int8 WorldObject::GetTransSeat() const
{
    return m_movementInfo.transport.VehicleSeatIndex;
}

void WorldObject::SetTransport(Transport* t)
{
    m_transport = t;
}

float WorldObject::GetDistanceToZOnfall()
{
    Position pos;
    GetFirstCollisionPosition(pos, PET_FOLLOW_DIST, 0.f);
    auto zNow = pos.m_positionX;
    if (auto lastUpdateTime = m_movementInfo.fall.lastTimeUpdate)
        zNow = pos.m_positionZ - Movement::computeFallElevation(Movement::MSToSec(getMSTime() - lastUpdateTime), false) - 5.0f;
    return zNow - GetHeight(pos.m_positionX, pos.m_positionY, MAX_HEIGHT, true);
}

bool WorldObject::IsWorldObject() const
{
    if (m_isWorldObject)
        return true;

    if (ToCreature() && ToCreature()->m_isTempWorldObject)
        return true;

    return false;
}

void WorldObject::setActive(bool on)
{
    if (m_isActive == on)
        return;

    if (IsPlayer())
        return;

    m_isActive = on;

    if (!IsInWorld())
        return;

    Map* map = FindMap();
    if (!map)
        return;

    if (on)
    {
        if (IsCreature())
            map->AddToActive(ToCreature());
        else if (IsDynObject())
            map->AddToActive(ToDynObject());
        else if (IsGameObject())
            map->AddToActive(ToGameObject());
    }
    else
    {
        if (IsCreature())
            map->RemoveFromActive(this->ToCreature());
        else if (IsDynObject())
            map->RemoveFromActive(ToDynObject());
        else if (IsGameObject())
            map->RemoveFromActive(ToGameObject());
    }
}

void WorldObject::CleanupsBeforeDelete(bool /*finalCleanup*/)
{
    if (IsInWorld())
        RemoveFromWorld();

    m_phaseId.clear();
    m_phaseBit.clear();
    _terrainSwaps.clear();
    _worldMapAreaSwaps.clear();
    _visibilityPlayerList.clear();
    _hideForGuid.clear();

    if (Transport* transport = GetTransport())
        transport->RemovePassenger(this);
}

uint32 WorldObject::GetZoneId() const
{
    Map* map = GetMap();
    if (!map || map->IsMapUnload())
        return m_zoneId;

    return map->GetZoneId(m_positionX, m_positionY, m_positionZ);
}

uint32 WorldObject::GetPZoneId() const
{
    return sDB2Manager.GetParentZoneOrSelf(m_zoneId);
}

uint32 WorldObject::GetAreaId() const
{
    Map* map = GetMap();
    if (!map || map->IsMapUnload())
        return m_areaId;

    return map->GetAreaId(m_positionX, m_positionY, m_positionZ);
}

void WorldObject::GetZoneAndAreaId(uint32& zoneid, uint32& areaid) const
{
    Map* map = GetMap();
    if (!map || map->IsMapUnload())
    {
        zoneid = m_zoneId;
        areaid = m_areaId;
        return;
    }

    map->GetZoneAndAreaId(zoneid, areaid, m_positionX, m_positionY, m_positionZ);
}

InstanceScript* WorldObject::GetInstanceScript()
{
    Map* map = GetMap();
    if (!map)
        return nullptr;
    return map->IsDungeon() ? dynamic_cast<InstanceMap*>(map)->GetInstanceScript() : nullptr;
}

float WorldObject::GetDistance(const WorldObject* obj) const
{
    float d = GetExactDist(obj) - GetObjectSize() - obj->GetObjectSize();
    return d > 0.0f ? d : 0.0f;
}

float WorldObject::GetDistance(Position const& pos) const
{
    float d = GetExactDist(&pos) - GetObjectSize();
    return d > 0.0f ? d : 0.0f;
}

float WorldObject::GetDistance(float x, float y, float z) const
{
    float d = GetExactDist(x, y, z) - GetObjectSize();
    return d > 0.0f ? d : 0.0f;
}

float WorldObject::GetDistance2d(const WorldObject* obj) const
{
    float d = GetExactDist2d(obj) - GetObjectSize() - obj->GetObjectSize();
    return d > 0.0f ? d : 0.0f;
}

float WorldObject::GetDistance2d(float x, float y) const
{
    float d = GetExactDist2d(x, y) - GetObjectSize();
    return d > 0.0f ? d : 0.0f;
}

float WorldObject::GetDistanceZ(const WorldObject* obj) const
{
    float dz = fabs(GetPositionZH() - obj->GetPositionZH());
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = dz - sizefactor;
    return dist > 0 ? dist : 0;
}

bool WorldObject::IsSelfOrInSameMap(const WorldObject* obj) const
{
    if (this == obj)
        return true;
    return IsInMap(obj);
}

bool WorldObject::IsInMap(const WorldObject* obj) const
{
    if (obj)
        return IsInWorld() && obj->IsInWorld() && GetMap() == obj->GetMap();
    return false;
}

bool WorldObject::IsWithinDist3d(float x, float y, float z, float dist, bool allowObjectSize) const
{
    return IsInDist(x, y, z, dist + (allowObjectSize ? GetObjectSize() : 0.0f));
}

bool WorldObject::IsWithinDist3d(const Position* pos, float dist, bool allowObjectSize) const
{
    return IsInDist(pos, dist + (allowObjectSize ? GetObjectSize() : 0.0f));
}

bool WorldObject::IsWithinDist2d(float x, float y, float dist) const
{
    return IsInDist2d(x, y, dist + GetObjectSize());
}

bool WorldObject::IsWithinDist2d(const Position* pos, float dist) const
{
    return IsInDist2d(pos, dist + GetObjectSize());
}

bool WorldObject::IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D, bool ignoreObjectSize /*=false*/) const
{
    return obj && _IsWithinDist(obj, dist2compare, is3D, ignoreObjectSize);
}

bool WorldObject::IsWithinDistInMap(WorldObject const* obj, float dist2compare, bool is3D, bool ignoreObjectSize /*=false*/) const
{
    return obj && IsInMap(obj) && InSamePhase(obj) && _IsWithinDist(obj, dist2compare, is3D, ignoreObjectSize);
}

bool WorldObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D, bool ignoreObjectSize /*=false*/) const
{
    float targetObjectSize = obj->GetObjectSize();

    if (IsAreaTrigger() && obj->IsUnit())
    {
        if (!ToAreaTrigger()->GetAreaTriggerInfo().WithObjectSize)
        {
            if (obj->IsPlayer())
                targetObjectSize = 0.0f;
            else
                targetObjectSize = obj->ToUnit()->GetModelSize();
        }
    }

    float sizefactor = ignoreObjectSize ? 0.0f : GetObjectSize() + targetObjectSize;
    float maxdist = dist2compare + sizefactor;

    if (m_transport && obj->GetTransport() && obj->GetTransport()->GetGUIDLow() == m_transport->GetGUIDLow())
    {
        float dtx = m_movementInfo.transport.Pos.m_positionX - obj->m_movementInfo.transport.Pos.m_positionX;
        float dty = m_movementInfo.transport.Pos.m_positionY - obj->m_movementInfo.transport.Pos.m_positionY;
        float disttsq = dtx * dtx + dty * dty;
        if (is3D)
        {
            float dtz = m_movementInfo.transport.Pos.m_positionZ - obj->m_movementInfo.transport.Pos.m_positionZ;
            disttsq += dtz * dtz;
        }
        return disttsq < maxdist * maxdist;
    }

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if (is3D)
    {
        float dz = GetPositionZH() - obj->GetPositionZH();
        distsq += dz*dz;
    }

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinLOSInMap(const WorldObject* obj) const
{
    if (!IsInMap(obj))
        return false;

    float ox, oy, oz;
    if (GetTransport() && GetTransport() == obj->GetTransport())
    {
        if (Transport* transport = GetTransport())
        {
            obj->GetPosition(ox, oy, oz, transport);
            Position pos;
            GetPosition(&pos, transport);
            if (GameObjectModel* _model = transport->m_model)
            {
                G3D::Vector3 pos1(pos.m_positionX, pos.m_positionY, pos.m_positionZ + 2.f);
                G3D::Vector3 pos2(ox, oy, oz + 2.f);
                transport->CalculatePassengerPosition(pos1.x, pos1.y, pos1.z);
                transport->CalculatePassengerPosition(pos2.x, pos2.y, pos2.z);
                return _model->isInLineOfSight(pos1, pos2, GetPhases(), IsPlayer());
            }
        }
    }
    obj->GetPosition(ox, oy, oz);
    return IsWithinLOS(ox, oy, oz);
}

bool WorldObject::IsWithinLOS(float ox, float oy, float oz) const
{
    if (IsInWorld())
    {
        if (Transport* transport = GetTransport())
        {
            Position pos;
            GetPosition(&pos, transport);
            if (GameObjectModel* _model = transport->m_model)
            {
                G3D::Vector3 pos1(pos.m_positionX, pos.m_positionY, pos.m_positionZ + 2.f);
                G3D::Vector3 pos2(ox, oy, oz + 2.f);
                transport->CalculatePassengerPosition(pos1.x, pos1.y, pos1.z);
                // transport->CalculatePassengerPosition(pos2.x, pos2.y, pos2.z);
                return _model->isInLineOfSight(pos1, pos2, GetPhases(), IsPlayer());
            }
        }
        if (!GetMap())
            return false;
        return GetMap()->isInLineOfSight(GetPositionX(), GetPositionY(), GetPositionZH() + 2.f, ox, oy, oz + 2.f, GetPhases());
    }

    return true;
}

float WorldObject::GetWaterOrGroundLevel(float x, float y, float z, float* ground /*= NULL*/, bool /*swim = false*/) const
{
    if (Transport* transport = GetTransport())
    {
        if (GameObjectModel* _model = transport->m_model)
        {
            float ground_z = _model->getHeight(x, y, z, DEFAULT_HEIGHT_SEARCH, GetPhases(), IsPlayer());
            if (ground_z == -G3D::finf() || ground_z == G3D::finf())
                return VMAP_INVALID_HEIGHT_VALUE;

            if (ground)
                *ground = ground_z;
            return ground_z;
        }
    }

    if (!GetMap())
        return VMAP_INVALID_HEIGHT_VALUE;
    return GetMap()->GetWaterOrGroundLevel(GetPhases(), x, y, z, ground);
}

float WorldObject::GetHeight(float x, float y, float z, bool vmap /*= true*/, float maxSearchDist /*= DEFAULT_HEIGHT_SEARCH*/) const
{
    if (Transport* transport = GetTransport())
    {
        if (GameObjectModel* _model = transport->m_model)
        {
            float ground_z = _model->getHeight(x, y, z, maxSearchDist, GetPhases(), IsPlayer());
            if (ground_z == -G3D::finf() || ground_z == G3D::finf())
                return VMAP_INVALID_HEIGHT_VALUE;
            return ground_z;
        }
    }

    if (!GetMap())
        return VMAP_INVALID_HEIGHT_VALUE;
    return GetMap()->GetHeight(GetPhases(), x, y, z, vmap, maxSearchDist);
}

bool WorldObject::GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D /* = true */) const
{
    float dx1 = GetPositionX() - obj1->GetPositionX();
    float dy1 = GetPositionY() - obj1->GetPositionY();
    float distsq1 = dx1*dx1 + dy1*dy1;
    if (is3D)
    {
        float dz1 = GetPositionZH() - obj1->GetPositionZH();
        distsq1 += dz1*dz1;
    }

    float dx2 = GetPositionX() - obj2->GetPositionX();
    float dy2 = GetPositionY() - obj2->GetPositionY();
    float distsq2 = dx2*dx2 + dy2*dy2;
    if (is3D)
    {
        float dz2 = GetPositionZH() - obj2->GetPositionZH();
        distsq2 += dz2*dz2;
    }

    return distsq1 < distsq2;
}

bool WorldObject::IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D /* = true */) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if (is3D)
    {
        float dz = GetPositionZH() - obj->GetPositionZH();
        distsq += dz*dz;
    }

    float sizefactor = GetObjectSize() + obj->GetObjectSize();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange2d(float x, float y, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float distsq = dx*dx + dy*dy;

    float sizefactor = GetObjectSize();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange3d(float x, float y, float z, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZH() - z;
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetObjectSize();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInBetween(const Position* obj1, const Position* obj2, float size) const
{
    if (!obj1 || !obj2)
        return false;

    float dist = GetExactDist2d(obj1->GetPositionX(), obj1->GetPositionY());

    // not using sqrt() for performance
    if ((dist * dist) >= obj1->GetExactDist2dSq(obj2->GetPositionX(), obj2->GetPositionY()))
        return false;

    if (!size)
        size = GetObjectSize() / 2.f;

    float angle = obj1->GetAngle(obj2);

    // not using sqrt() for performance
    return (size * size) >= GetExactDist2dSq(obj1->GetPositionX() + cos(angle) * dist, obj1->GetPositionY() + sin(angle) * dist);
}

bool WorldObject::IsInBetweenShift(const Position* obj1, const Position* obj2, float size, float shift, float angleShift) const
{
    if (!obj1 || !obj2)
        return false;

    angleShift += obj1->GetOrientation();
    float destx = obj1->GetPositionX() + shift * std::cos(angleShift);
    float desty = obj1->GetPositionY() + shift * std::sin(angleShift);

    float dist = GetExactDist2d(destx, desty);

    // not using sqrt() for performance
    if ((dist * dist) >= obj1->GetExactDist2dSq(obj2->GetPositionX(), obj2->GetPositionY()))
        return false;

    if (!size)
        size = GetObjectSize() / 2.f;

    float angle = obj1->GetAngle(obj2);

    // not using sqrt() for performance
    return (size * size) >= GetExactDist2dSq(destx + cos(angle) * dist, desty + sin(angle) * dist);
}

bool WorldObject::IsInBetween(const WorldObject* obj1, float x2, float y2, float size) const
{
    if (!obj1)
        return false;

    float dist = GetExactDist2d(obj1->GetPositionX(), obj1->GetPositionY());

    // not using sqrt() for performance
    if ((dist * dist) >= obj1->GetExactDist2dSq(x2, y2))
        return false;

    if (!size)
        size = GetObjectSize() / 2.f;

    float angle = obj1->GetAngle(x2, y2);

    // not using sqrt() for performance
    return (size * size) >= GetExactDist2dSq(obj1->GetPositionX() + std::cos(angle) * dist, obj1->GetPositionY() + std::sin(angle) * dist);
}

bool WorldObject::IsInAxe(const WorldObject* obj1, const WorldObject* obj2, float size) const
{
    if (!obj1 || !obj2)
        return false;

    if (!size)
        size = GetObjectSize() / 2.f;

    float dist = GetExactDist2d(obj1->GetPositionX(), obj1->GetPositionY());
    float angle = obj1->GetAngle(obj2);
    return (size * size) >= GetExactDist2dSq(obj1->GetPositionX() + cos(angle) * dist, obj1->GetPositionY() + sin(angle) * dist);
}

bool WorldObject::IsInAxe(WorldObject const* obj, float width, float range) const
{
    if (obj == nullptr)
        return false;

    float dist = GetExactDist2d(obj->GetPositionX(), obj->GetPositionY());
    float x = obj->GetPositionX() + range * cos(obj->GetOrientation());
    float y = obj->GetPositionY() + range * sin(obj->GetOrientation());

    if ((dist * dist) >= obj->GetExactDist2dSq(x, y))
        return false;

    if (!width)
        width = GetObjectSize() / 2.f;

    float angle = obj->GetAngle(x, y);

    return (width * width) >= GetExactDist2dSq(obj->GetPositionX() + cos(angle) * dist, obj->GetPositionY() + sin(angle) * dist);
}

bool WorldObject::isInFront(WorldObject const* target, float arc) const
{
    return HasInArc(arc, target);
}

bool WorldObject::isInBack(WorldObject const* target, float arc) const
{
    return !HasInArc(2.f * M_PI - arc, target);
}

void WorldObject::GetRandomPoint(const Position &pos, float distance, float &rand_x, float &rand_y, float &rand_z) const
{
    if (!distance)
    {
        pos.GetPosition(rand_x, rand_y, rand_z);
        return;
    }

    // angle to face `obj` to `this`
    float angle = static_cast<float>(rand_norm())*static_cast<float>(2 * M_PI);
    float new_dist = static_cast<float>(rand_norm()) + static_cast<float>(rand_norm());
    new_dist = distance * (new_dist > 1 ? new_dist - 2 : new_dist);

    rand_x = pos.m_positionX + new_dist * std::cos(angle);
    rand_y = pos.m_positionY + new_dist * std::sin(angle);
    rand_z = pos.m_positionZ;

    Trinity::NormalizeMapCoord(rand_x);
    Trinity::NormalizeMapCoord(rand_y);
    UpdateGroundPositionZ(rand_x, rand_y, rand_z);            // update to LOS height if available
}

void WorldObject::GetRandomPoint(Position const& srcPos, float distance, Position& pos) const
{
    float x, y, z;
    GetRandomPoint(srcPos, distance, x, y, z);
    pos.Relocate(x, y, z, GetOrientation());
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float &z) const
{
    float new_z = GetHeight(x, y, z + 2.0f, true);
    if (new_z > INVALID_HEIGHT)
        z = new_z + 0.08f;                                   // just to be sure that we are not a few pixel under the surface
}

void WorldObject::UpdateAllowedPositionZ(float x, float y, float &z) const
{
    float _offset = GetPositionH() < 2.0f ? 2.0f : 0.0f; // For find correct position Z
    bool isFalling = m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);

    switch (GetTypeId())
    {
        case TYPEID_UNIT:
        {
            Unit* victim = ToCreature()->getVictim();
            if (victim)
            {
                // anyway creature move to victim for thinly Z distance (shun some VMAP wrong ground calculating)
                if (fabs(GetPositionZ() - victim->GetPositionZ()) < 2.5f)
                    return;
            }
            // non fly unit don't must be in air
            // non swim unit must be at ground (mostly speedup, because it don't must be in water and water level check less fast
            if (!ToCreature()->CanFly())
            {
                bool canSwim = ToCreature()->CanSwim();
                float ground_z = z;
                float max_z = canSwim
                    ? GetWaterOrGroundLevel(x, y, z + _offset, &ground_z, !ToUnit()->HasAuraType(SPELL_AURA_WATER_WALK))
                    : ((ground_z = GetHeight(x, y, z + _offset, true)));

                if (isFalling) // Allowed point in air if we falling
                    if ((z - max_z) > 2.0f)
                        return;

                max_z += GetPositionH();
                ground_z += GetPositionH();
                if (max_z > INVALID_HEIGHT)
                {
                    if (z > max_z && !IsInWater())
                        z = max_z;
                    else if (z < ground_z)
                        z = ground_z;
                }
            }
            else
            {
                float ground_z = GetHeight(x, y, z + _offset, true);
                ground_z += GetPositionH();
                if (z < ground_z)
                    z = ground_z;
            }
            break;
        }
        case TYPEID_PLAYER:
        {
            // for server controlled moves playr work same as creature (but it can always swim)
            if (!ToPlayer()->CanFly())
            {
                float ground_z = z;
                float max_z = GetWaterOrGroundLevel(x, y, z + _offset, &ground_z, !ToUnit()->HasAuraType(SPELL_AURA_WATER_WALK));
                max_z += GetPositionH();

                if (isFalling) // Allowed point in air if we falling
                    if ((z - max_z) > 2.0f)
                        return;

                ground_z += GetPositionH();
                if (max_z > INVALID_HEIGHT)
                {
                    if (z > max_z && !IsInWater())
                        z = max_z;
                    else if (z < ground_z)
                        z = ground_z;
                }
            }
            else
            {
                float ground_z = GetHeight(x, y, z + _offset, true);
                ground_z += GetPositionH();
                if (z < ground_z)
                    z = ground_z;
            }
            break;
        }
        default:
        {
            float ground_z = GetHeight(x, y, z + _offset, true);
            ground_z += GetPositionH();

            if (isFalling) // Allowed point in air if we falling
                if ((z - ground_z) > 2.0f)
                    return;

            if (ground_z > INVALID_HEIGHT)
                z = ground_z;
            break;
        }
    }
}

float WorldObject::GetGridActivationRange() const
{
    if (m_Teleports)
        return 0.0f;

    if (IsPlayer())
        if (Map* map = GetMap())
            return map->GetVisibilityRange(m_zoneId, m_areaId);

    if (ToCreature())
        return ToCreature()->m_SightDistance;

    if (MaxVisible)
        return MAX_VISIBILITY_DISTANCE;

    return 0.0f;
}

float WorldObject::GetVisibilityRange() const
{
    if (isActiveObject() && !IsPlayer())
        return MAX_VISIBILITY_DISTANCE;

    if (MaxVisible)
        return MAX_VISIBILITY_DISTANCE;

    if (Map* map = GetMap())
        return map->GetVisibilityRange(m_zoneId, m_areaId);

    return MAX_VISIBILITY_DISTANCE;
}

float WorldObject::GetVisibilityCombatLog() const
{
    if (GetMap() && GetMap()->IsDungeon())
        return 120.0f;

    return SIGHT_RANGE_UNIT;
}

float WorldObject::GetSightRange(const WorldObject* target) const
{
    if (!this || !GetMap())
        return 0.0f;

    if (ToUnit())
    {
        if (IsPlayer())
        {
            if (target)
            {
                if (target->MaxVisible)
                    return GLOBAL_VISIBILITY_DISTANCE;
                if (target->isActiveObject() && !target->IsPlayer())
                    return MAX_VISIBILITY_DISTANCE;
            }
            return GetMap()->GetVisibilityRange(m_zoneId, m_areaId);
        }
        if (ToCreature())
            return ToCreature()->m_SightDistance;
        return SIGHT_RANGE_UNIT;
    }

    return 0.0f;
}

void WorldObject::SetVisible(bool x)
{
    if (!x)
        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_GAMEMASTER);
    else
        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_PLAYER);

    UpdateObjectVisibility();
}

bool WorldObject::canSeeOrDetect(WorldObject const* obj, bool ignoreStealth, bool distanceCheck) const
{
    if (this == obj || m_Teleports && obj->m_Teleports)
        return true;

    if (GetGUID().IsPlayer())
    {
        if (obj->MustBeVisibleOnlyForSomePlayers())
        {
            Player const* thisPlayer = ToPlayer();
            if (!thisPlayer)
                return false;

            Group const* group = thisPlayer->GetGroup();
            if (!obj->IsInPersonnalVisibilityList(thisPlayer->GetGUID()) && (!group || !obj->IsInPersonnalVisibilityList(group->GetGUID())))
                return false;

            if (thisPlayer->IsSpectator())
                return false;
        }

        if (obj->HideForSomePlayers() && obj->ShouldHideFor(GetGUID()))
            return false;
    }

    if (GetGUID().IsPlayer() && obj->IsGameObject() && obj->ToGameObject()->IsPersonal())
    {
        Player const* thisPlayer = ToPlayer();
        if (thisPlayer && const_cast<Player*>(thisPlayer)->IsPlayerLootCooldown(obj->GetEntry(), TYPE_GO, GetMap()->GetDifficultyID()))
            return false;
        if (thisPlayer && const_cast<Player*>(thisPlayer)->IsPlayerLootCooldown(obj->GetGUIDLow(), TYPE_GUID, GetMap()->GetDifficultyID()))
            return false;
    }

    if (obj->IsNeverVisible(this) || CanNeverSee(obj) || obj->CanNeverSee2(this))
        return false;

    if (obj->IsAlwaysVisibleFor(this) || CanAlwaysSee(obj))
        return true;

    bool corpseVisibility = false;
    if (distanceCheck)
    {
        bool corpseCheck = false;
        bool onArena = false;   //on arena we have always see all

        if (Player const* thisPlayer = ToPlayer())
        {
            if (thisPlayer->HaveExtraLook(obj->GetGUID()))
                return true;

            //not see befor enter vehicle.
            if (Creature const* creature = obj->ToCreature())
                if (creature->onVehicleAccessoryInit())
                    return false;

            onArena = thisPlayer->InArena();

            if (thisPlayer->isDead() && thisPlayer->GetHealth() > 0 && // Cheap way to check for ghost state
                !(obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) & m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) & GHOST_VISIBILITY_GHOST))
            {
                if (Corpse* corpse = thisPlayer->GetCorpse())
                {
                    corpseCheck = true;
                    if (corpse->IsWithinDist(thisPlayer, GetSightRange(obj), false))
                        if (corpse->IsWithinDist(obj, GetSightRange(obj), false))
                            corpseVisibility = true;
                }
            }
        }

        WorldObject const* viewpoint = this;
        if (Player const* player = this->ToPlayer())
            viewpoint = player->GetViewpoint();

        if (!viewpoint)
            viewpoint = this;

        if (!corpseCheck && !onArena && !viewpoint->IsWithinDist(obj, GetSightRange(obj), false))
            return false;
    }

    // GM visibility off or hidden NPC
    if (!obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM))
    {
        // Stop checking other things for GMs
        if (m_serverSideVisibilityDetect.GetValue(SERVERSIDE_VISIBILITY_GM))
            return true;
    }
    else
        return m_serverSideVisibilityDetect.GetValue(SERVERSIDE_VISIBILITY_GM) >= obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM);

    // Ghost players, Spirit Healers, and some other NPCs
    if (!corpseVisibility && !(obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) & m_serverSideVisibilityDetect.GetValue(SERVERSIDE_VISIBILITY_GHOST)))
    {
        // Alive players can see dead players in some cases, but other objects can't do that
        if (Player const* thisPlayer = ToPlayer())
        {
            if (Player const* objPlayer = obj->ToPlayer())
            {
                if (thisPlayer->GetTeam() != objPlayer->GetTeam() || !thisPlayer->IsGroupVisibleFor(objPlayer))
                    return false;
            }
            else
                return false;
        }
        else
            return false;
    }

    if (obj->IsInvisibleDueToDespawn())
        return false;

    if (!CanDetect(obj, ignoreStealth))
        return false;

    // if (ToPlayer() && obj->ToPlayer())
        // if (m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL) > 0)
            // return (m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL) == 1 && obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL) == 1) || obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL) == 0 || m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL) == obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL);

    return true;
}

bool WorldObject::CanDetect(WorldObject const* obj, bool ignoreStealth) const
{
    const WorldObject* seer = this;

    // Pets don't have detection, they use the detection of their masters
    if (const Unit* thisUnit = ToUnit())
        if (Unit* controller = thisUnit->GetCharmerOrOwner())
            seer = controller;

    if (obj->IsAlwaysDetectableFor(seer))
        return true;

    if (!ignoreStealth && !seer->CanDetectInvisibilityOf(obj))
        return false;

    if (!ignoreStealth && !seer->CanDetectStealthOf(obj))
        return false;

    return true;
}

bool WorldObject::CanDetectInvisibilityOf(WorldObject const* obj) const
{
    uint64 mask = obj->m_invisibility.GetFlags() & m_invisibilityDetect.GetFlags();

    // Check for not detected types
    if (mask != obj->m_invisibility.GetFlags())
        return false;

    if (obj->ToUnit())
        if ((m_invisibility.GetFlags() & obj->m_invisibilityDetect.GetFlags()) != m_invisibility.GetFlags())
        {
            if (obj->m_invisibility.GetFlags() != 0 || !IsUnit() || !ToUnit()->HasAuraType(SPELL_AURA_SEE_WHILE_INVISIBLE))
                return false;
        }

    for (uint8 i = 0; i < TOTAL_INVISIBILITY_TYPES; ++i)
    {
        if (!(mask & (SI64LIT(1) << i)))
            continue;

        int32 objInvisibilityValue = obj->m_invisibility.GetValue(InvisibilityType(i));
        int32 ownInvisibilityDetectValue = m_invisibilityDetect.GetValue(InvisibilityType(i));

        // Too low value to detect
        if (ownInvisibilityDetectValue < objInvisibilityValue)
            return false;
    }

    return true;
}

bool WorldObject::CanDetectStealthOf(WorldObject const* obj) const
{
    // Combat reach is the minimal distance (both in front and behind),
    //   and it is also used in the range calculation.
    // One stealth point increases the visibility range by 0.3 yard.

    if (!obj->m_stealth.GetFlags())
        return true;

    float distance = GetExactDist(obj);
    float combatReach = 0.0f;

    if (IsUnit())
        combatReach = ToUnit()->GetCombatReach();

//     if (distance < combatReach)
//         return true;
// 
//     if (Player const* player = ToPlayer())
//         if(player->HaveAtClient(obj) && distance < (ATTACK_DISTANCE * 2))
//             return true;

    if (!HasInArc(M_PI, obj))
        return false;

    GameObject const* go = ToGameObject();
    for (uint32 i = 0; i < TOTAL_STEALTH_TYPES; ++i)
    {
        if (!(obj->m_stealth.GetFlags() & 1 << i))
            continue;

        if (IsUnit())
            if (ToUnit()->HasAuraTypeWithMiscvalue(SPELL_AURA_DETECT_STEALTH, i))
                return true;

        // Starting points
        int32 detectionValue = 30;

        // Level difference: 5 point / level, starting from level 1.
        // There may be spells for this and the starting points too, but
        // not in the DBCs of the client.
        detectionValue += int32(getLevelForTarget(obj) - 1) * 5;

        // Apply modifiers
        detectionValue += m_stealthDetect.GetValue(StealthType(i));
        if (go)
            if (Unit* owner = go->GetOwner())
                detectionValue -= int32(owner->getLevelForTarget(this) - 1) * 5;

        detectionValue -= obj->m_stealth.GetValue(StealthType(i));

        // Calculate max distance
        float visibilityRange = float(detectionValue) * 0.3f + combatReach;

        if (visibilityRange > MAX_PLAYER_STEALTH_DETECT_RANGE)
            visibilityRange = MAX_PLAYER_STEALTH_DETECT_RANGE;

        if (distance > visibilityRange)
            return false;
    }

    return true;
}

bool WorldObject::IsInPersonnalVisibilityList(ObjectGuid const& guid) const
{
    return _visibilityPlayerList.find(guid) != _visibilityPlayerList.end();
}

void WorldObject::AddPlayersInPersonnalVisibilityList(GuidUnorderedSet const& viewerList)
{
    for (auto guid : viewerList)
    {
        if (!guid.IsPlayer())
            continue;

        _visibilityPlayerList.insert(guid);
    }
}

void WorldObject::SendPlaySound(uint32 soundKitID, bool OnlySelf)
{
    WorldPackets::Misc::PlaySound  sound;
    sound.SoundKitID = soundKitID;
    sound.SourceObjectGuid = GetGUID();

    if (OnlySelf && IsPlayer())
        this->ToPlayer()->SendDirectMessage(sound.Write());
    else
        SendMessageToSet(sound.Write(), true); // ToSelf ignored in this case
}

void Object::ForceValuesUpdateAtIndex(uint32 i)
{
    _changesMask[i] = 1;
    AddToObjectUpdateIfNeeded();
}

void WorldObject::Talk(std::string const& text, ChatMsg msgType, Language language, float textRange, WorldObject const* target)
{
    Trinity::CustomChatTextBuilder builder(this, msgType, text, language, target);
    Trinity::LocalizedPacketDo<Trinity::CustomChatTextBuilder> localizer(builder);
    Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::CustomChatTextBuilder> > worker(this, textRange, localizer);

    CellCoord p = Trinity::ComputeCellCoord(GetPositionX(), GetPositionY());
    Cell cell(p);
    cell.SetNoCreate();
    cell.Visit(p, Trinity::makeWorldVisitor(worker), *GetMap(), *this, textRange);
}

void WorldObject::Talk(uint32 textId, ChatMsg msgType, float textRange, WorldObject const* target)
{
    auto bct = sBroadcastTextStore.LookupEntry(textId);
    if (!bct)
        return;

    auto unit = ToUnit();
    if (!unit)
        return;

    Trinity::BroadcastTextBuilder builder(unit, msgType, textId, target);
    Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> localizer(builder);
    Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> > worker(this, textRange, localizer);

    auto p = Trinity::ComputeCellCoord(GetPositionX(), GetPositionY());
    Cell cell(p);
    cell.SetNoCreate();
    cell.Visit(p, Trinity::makeWorldVisitor(worker), *GetMap(), *this, textRange);

    uint32 fSound = [bct, unit]() -> uint32
    {
        uint8 gender = GENDER_NONE;
        if (CreatureDisplayInfoEntry const* creatureDisplay = sCreatureDisplayInfoStore.LookupEntry(unit->GetDisplayId()))
            gender = creatureDisplay->Gender;
        if (gender == GENDER_NONE)
            gender = unit->getGender();

        return gender == GENDER_FEMALE ? bct->SoundEntriesID[1] : bct->SoundEntriesID[0];
    }();

    if (fSound)
        sCreatureTextMgr->SendNonChatPacket(unit, WorldPackets::Misc::PlaySound(GetGUID(), fSound).Write(), msgType, ObjectGuid::Empty, TEXT_RANGE_NORMAL, Team(unit->IsPlayer() ? unit->ToPlayer()->GetTeam() : TEAM_OTHER), false);
}

void WorldObject::MonsterSay(const char* text, uint32 language, ObjectGuid TargetGuid)
{
    Talk(text, CHAT_MSG_MONSTER_SAY, static_cast<Language>(language), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), ObjectAccessor::GetObjectInWorld(TargetGuid, static_cast<Unit*>(nullptr)));
}

void WorldObject::MonsterSay(int32 textId, uint32 /*language*/, ObjectGuid TargetGuid)
{
    Talk(textId, CHAT_MSG_MONSTER_SAY, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), ObjectAccessor::GetObjectInWorld(TargetGuid, static_cast<Unit*>(nullptr)));
}

void WorldObject::MonsterYell(const char* text, uint32 language, ObjectGuid TargetGuid)
{
    Talk(text, CHAT_MSG_MONSTER_YELL, static_cast<Language>(language), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_YELL), ObjectAccessor::GetObjectInWorld(TargetGuid, static_cast<Unit*>(nullptr)));
}

void WorldObject::MonsterYell(int32 textId, uint32 /*language*/, ObjectGuid TargetGuid)
{
    Talk(textId, CHAT_MSG_MONSTER_YELL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_YELL), ObjectAccessor::GetObjectInWorld(TargetGuid, static_cast<Unit*>(nullptr)));
}

void WorldObject::MonsterYellToZone(int32 textId, uint32 /*language*/, ObjectGuid TargetGuid)
{
    auto unit = ToUnit();
    if (!unit)
        return;

    Trinity::BroadcastTextBuilder builder(unit, CHAT_MSG_MONSTER_YELL, textId, ObjectAccessor::GetObjectInWorld(TargetGuid, static_cast<Unit*>(nullptr)));
    Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> say_do(builder);

    uint32 zoneid = m_zoneId;

    GetMap()->ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->m_zoneId == zoneid)
            say_do(player);
    });
}

void WorldObject::MonsterTextEmote(const char* text, ObjectGuid TargetGuid, bool IsBossEmote)
{
    Talk(text, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), ObjectAccessor::GetObjectInWorld(TargetGuid, static_cast<Unit*>(nullptr)));
}

void WorldObject::MonsterTextEmote(int32 textId, ObjectGuid TargetGuid, bool IsBossEmote)
{
    Talk(textId, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), ObjectAccessor::GetObjectInWorld(TargetGuid, static_cast<Unit*>(nullptr)));
}

void WorldObject::MonsterWhisper(const char* text, ObjectGuid receiver, bool IsBossWhisper)
{
    Player* player = ObjectAccessor::FindPlayer(receiver);
    if (!player || !player->GetSession())
        return;

    LocaleConstant locale = player->GetSession()->GetSessionDbLocaleIndex();
    WorldPackets::Chat::Chat packet;
    packet.Initialize(IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, this, player, text, 0, "", locale);
    player->SendDirectMessage(packet.Write());
}

void WorldObject::MonsterWhisper(int32 textId, ObjectGuid receiver, bool IsBossWhisper)
{
    Player* player = ObjectAccessor::FindPlayer(receiver);
    if (!player || !player->GetSession())
        return;

    LocaleConstant localeConstant = player->GetSession()->GetSessionDbLocaleIndex();
    WorldPackets::Chat::Chat packet;
    packet.Initialize(IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, this, player, sObjectMgr->GetTrinityString(textId, localeConstant), 0, "", localeConstant);
    player->SendDirectMessage(packet.Write());
}

void WorldObject::SendMessageToSet(WorldPacket const* data, bool self, GuidUnorderedSet const& ignoredList /*= GuidUnorderedSet()*/)
{
    if (IsInWorld())
        SendMessageToSetInRange(data, GetVisibilityRange(), self, ignoredList);
}

void WorldObject::SendMessageToSetInRange(WorldPacket const* data, float dist, bool /*self*/, GuidUnorderedSet const& ignoredList /*= GuidUnorderedSet()*/)
{
    Trinity::MessageDistDeliverer notifier(this, data, dist, false, nullptr, ignoredList);
    Trinity::VisitNearbyWorldObject(this, dist, notifier);
}

void WorldObject::SendMessageToSet(WorldPacket const* data, Player const* skipped_rcvr, GuidUnorderedSet const& ignoredList /*= GuidUnorderedSet()*/)
{
    Trinity::MessageDistDeliverer notifier(this, data, GetVisibilityRange(), false, skipped_rcvr, ignoredList);
    Trinity::VisitNearbyWorldObject(this, GetVisibilityRange(), notifier);
}

void WorldObject::SendObjectDeSpawnAnim(ObjectGuid guid)
{
    SendMessageToSet(WorldPackets::GameObject::GameObjectDespawn(guid).Write(), true);
}

void WorldObject::SetMap(Map* map)
{
    if (m_currMap == map) // command add npc: first create, than loadfromdb
        return;

    /* bool inWorldOrIsCorpse = (!IsInWorld() || IsCorpse());
    ASSERT(map);
    ASSERT(inWorldOrIsCorpse);
    if (m_currMap)
    {
        TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::SetMap: obj %u new map %u %u, old map %u %u", (uint32)GetTypeId(), map->GetId(), map->GetInstanceId(), m_currMap->GetId(), m_currMap->GetInstanceId());
        ASSERT(false);
    } */

    m_spawnMode = map->GetSpawnMode();
    m_currMap = map;
    m_mapId = map->GetId();
    m_InstanceId = map->GetInstanceId();
    if (IsWorldObject())
        m_currMap->AddWorldObject(this);
}

void WorldObject::ResetMap()
{
    if (!m_currMap)
        return;
    // ASSERT(!IsInWorld());
    if (IsWorldObject())
        m_currMap->RemoveWorldObject(this);
    m_currMap = nullptr;
}

Map const* WorldObject::GetBaseMap() const
{
    if (!m_currMap)
        return nullptr;

    ASSERT(m_currMap);
    return m_currMap->GetParent();
}

void WorldObject::AddObjectToRemoveList()
{
    ASSERT(m_uint32Values);

    Map* map = FindMap();
    if (!map)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Object (TypeId: %u Entry: %u GUID: %u) at attempt add to move list not have valid map (Id: %u).", GetTypeId(), GetEntry(), GetGUIDLow(), GetMapId());
        return;
    }

    map->AddObjectToRemoveList(this);
}

void WorldObject::SetZoneScript()
{
    if (Map* map = FindMap())
    {
        if (map->IsDungeon() || map->IsGarrison())
            m_zoneScript = reinterpret_cast<ZoneScript*>(map->ToInstanceMap()->GetInstanceScript());
        else if (!map->IsBattlegroundOrArena())
        {
            if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(m_zoneId))
                m_zoneScript = bf;
            else
                m_zoneScript = sOutdoorPvPMgr->GetZoneScript(m_zoneId);
        }
    }
}

TempSummon* WorldObject::SummonCreature(uint32 entry, const Position &pos, ObjectGuid targetGuid, TempSummonType spwtype, uint32 duration, uint32 spellId /*= 0*/, SummonPropertiesEntry const* properties /*= NULL*/) const
{
    if (Map* map = FindMap())
    {
        if (!ToUnit())
        {
            std::list<Creature*> creatures;
            GetAliveCreatureListWithEntryInGrid(creatures, entry, 110.0f);
            if (creatures.size() > (map->GetInstanceId() ? 100 : 50))
                return nullptr;
        }
        if (TempSummon* summon = map->SummonCreature(entry, pos, properties, duration, IsUnit() ? (Unit*)this : nullptr, targetGuid, spellId))
        {
            summon->SetTempSummonType(spwtype);
            return summon;
        }
    }

    return nullptr;
}

TempSummon* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang, TempSummonType spwtype, uint32 despwtime, ObjectGuid viewerGuid, GuidUnorderedSet* viewersList)
{
    if (!x && !y && !z)
    {
        GetClosePoint(x, y, z, GetObjectSize());
        ang = GetOrientation();
    }
    Position pos;
    pos.Relocate(x, y, z, ang);
    return SummonCreature(id, pos, spwtype, despwtime, 0, viewerGuid, viewersList);
}

TempSummon* WorldObject::SummonCreature(uint32 id, TempSummonType spwtype, uint32 despwtime)
{
    return SummonCreature(id, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation(), spwtype, despwtime);
}

TempSummon* WorldObject::SummonCreature(uint32 entry, const Position &pos, TempSummonType spwtype, uint32 duration, int32 vehId, ObjectGuid viewerGuid, GuidUnorderedSet* viewersList) const
{
    if (Map* map = FindMap())
    {
        if (!ToUnit())
        {
            std::list<Creature*> creatures;
            GetAliveCreatureListWithEntryInGrid(creatures, entry, 110.0f);
            if (creatures.size() > (map->GetInstanceId() ? 100 : 50))
                return nullptr;
        }
        if (TempSummon* summon = map->SummonCreature(entry, pos, nullptr, duration, IsUnit() ? (Unit*)this : nullptr, ObjectGuid::Empty, 0, vehId, viewerGuid, viewersList))
        {
            summon->SetTempSummonType(spwtype);
            return summon;
        }
    }

    return nullptr;
}

GameObject* WorldObject::SummonGameObject(uint32 entry, Position pos, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime, ObjectGuid viewerGuid /*= ObjectGuid::Empty*/, GuidUnorderedSet* viewersList /*= nullptr*/, bool hasCreator /*= true*/)
{
    return SummonGameObject(entry, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), ang, rotation0, rotation1, rotation2, rotation3, respawnTime, viewerGuid, viewersList, hasCreator);
}

GameObject* WorldObject::SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime, ObjectGuid viewerGuid, GuidUnorderedSet* viewersList, bool hasCreator /*= true*/)
{
    if (!IsInWorld())
        return nullptr;

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if (!goinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject template %u not found in database!", entry);
        return nullptr;
    }
    Map* map = GetMap();
    GameObject* go = sObjectMgr->IsStaticTransport(entry) ? new StaticTransport : new GameObject;
    if (!go->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), entry, map, GetPhaseMask(), Position(x, y, z, ang), G3D::Quat(rotation0, rotation1, rotation2, rotation3), 100, GO_STATE_READY))
    {
        delete go;
        return nullptr;
    }
    go->SetPhaseId(GetPhases(), false);
    go->SetTratsport(GetTransport());
    go->SetRespawnTime(respawnTime);

    /// @TODO comes from GarrBuildingDoodadSet.db2
    //if (_isGarrisonPlotObject)
    //    go->SetDynamicValue(GAMEOBJECT_DYNAMIC_ENABLE_DOODAD_SETS, 0, 1);

    // If we summon go by creature at despown - we will see deleted go.
    // If we summon go by creature with ownership in some cases we couldn't use it
    if (hasCreator && (IsPlayer() || IsCreature() && !respawnTime)) //not sure how to handle this
        ToUnit()->AddGameObject(go);
    else
        go->SetSpawnedByDefault(false);

    if (!viewerGuid.IsEmpty())
        go->AddPlayerInPersonnalVisibilityList(viewerGuid);

    if (viewersList)
        go->AddPlayersInPersonnalVisibilityList(*viewersList);

    map->AddToMap(go);

    // if (IsCreature())
        // if (Creature* owner = ((Creature*)this))
            // if (owner->IsAIEnabled)
                // owner->AI()->JustSummonedGO(go);

    return go;
}

Creature* WorldObject::SummonTrigger(float x, float y, float z, float ang, uint32 duration, CreatureAI* (*GetAI)(Creature*))
{
    TempSummonType summonType = duration == 0 ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Creature* summon = SummonCreature(WORLD_TRIGGER, x, y, z, ang, summonType, duration);
    if (!summon)
        return nullptr;

    //summon->SetName(GetName());
    if (IsPlayer() || IsCreature())
    {
        summon->setFaction(ToUnit()->getFaction());
        summon->SetLevel(ToUnit()->getLevel());
        summon->SetEffectiveLevel(ToUnit()->GetEffectiveLevel());
    }

    if (GetAI)
        summon->AIM_Initialize(GetAI(summon));
    return summon;
}

void WorldObject::GetAttackableUnitListInRange(std::list<Unit*> &list, float fMaxSearchRange, bool aliveOnly/* = true*/) const
{
    CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Trinity::AnyUnitInObjectRangeCheck u_check(this, fMaxSearchRange, aliveOnly);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(this, list, u_check);

    cell.Visit(p, Trinity::makeWorldVisitor(searcher), *GetMap(), *this, fMaxSearchRange);
    cell.Visit(p, Trinity::makeGridVisitor(searcher), *GetMap(), *this, fMaxSearchRange);
}

void WorldObject::GetAreaTriggersWithEntryInRange(std::list<AreaTrigger*>& list, uint32 entry, ObjectGuid casterGuid, float fMaxSearchRange) const
{
    Trinity::AreaTriggerWithEntryInObjectRangeCheck checker(this, entry, casterGuid, fMaxSearchRange);
    Trinity::AreaTriggerListSearcher<Trinity::AreaTriggerWithEntryInObjectRangeCheck> searcher(this, list, checker);
    Trinity::VisitNearbyObject(this, fMaxSearchRange, searcher);
}

Creature* WorldObject::FindNearestCreature(uint32 entry, float range, bool alive) const
{
    Creature* creature = nullptr;
    Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck checker(*this, entry, alive, range);
    Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(this, creature, checker);
    Trinity::VisitNearbyObject(this, range, searcher);
    return creature;
}

GameObject* WorldObject::FindNearestGameObject(uint32 entry, float range) const
{
    GameObject* go = nullptr;
    Trinity::NearestGameObjectEntryInObjectRangeCheck checker(*this, entry, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(this, go, checker);
    Trinity::VisitNearbyGridObject(this, range, searcher);
    return go;
}

Player* WorldObject::FindNearestPlayer(float range, bool /*alive*/)
{
    Player* player = nullptr;
    Trinity::AnyPlayerInObjectRangeCheck check(this, GetVisibilityRange());
    Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(this, player, check);
    Trinity::VisitNearbyWorldObject(this, range, searcher);
    return player;
}

GameObject* WorldObject::FindNearestGameObjectOfType(GameobjectTypes type, float range) const
{
    GameObject* go = nullptr;
    Trinity::NearestGameObjectTypeInObjectRangeCheck checker(*this, type, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectTypeInObjectRangeCheck> searcher(this, go, checker);
    Trinity::VisitNearbyGridObject(this, range, searcher);
    return go;
}

void WorldObject::GetGameObjectListWithEntryInGrid(std::list<GameObject*>& gameobjectList, uint32 entry, float maxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::AllGameObjectsWithEntryInRange check(this, entry, maxSearchRange);
    Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInRange> searcher(this, gameobjectList, check);

    cell.Visit(pair, Trinity::makeGridVisitor(searcher), *this->GetMap(), *this, maxSearchRange);
}

void WorldObject::GetNearGameObjectListInGrid(std::list<GameObject*>& lList, float maxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::NearestGameObjectCheck check(*this);
    Trinity::GameObjectListSearcher<Trinity::NearestGameObjectCheck> searcher(this, lList, check);
    cell.Visit(pair, Trinity::makeGridVisitor(searcher), *this->GetMap(), *this, maxSearchRange);
}

void WorldObject::GetCreatureListWithEntryInGrid(std::list<Creature*>& creatureList, uint32 entry, float maxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::AllCreaturesOfEntryInRange check(this, entry, maxSearchRange);
    Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(this, creatureList, check);

    cell.Visit(pair, Trinity::makeGridVisitor(searcher), *this->GetMap(), *this, maxSearchRange);
}

void WorldObject::GetCreatureListInGrid(std::list<Creature*>& creatureList, float maxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::AllCreaturesInRange check(this, maxSearchRange);
    Trinity::CreatureListSearcher<Trinity::AllCreaturesInRange> searcher(this, creatureList, check);

    cell.Visit(pair, Trinity::makeGridVisitor(searcher), *this->GetMap(), *this, maxSearchRange);
}

void WorldObject::GetAreaTriggerListWithEntryInGrid(std::list<AreaTrigger*>& atList, uint32 entry, float maxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::AllAreaTriggeresOfEntryInRange check(this, entry, maxSearchRange);
    Trinity::AreaTriggerListSearcher<Trinity::AllAreaTriggeresOfEntryInRange> searcher(this, atList, check);

    cell.Visit(pair, Trinity::makeGridVisitor(searcher), *this->GetMap(), *this, maxSearchRange);
}

void WorldObject::GetPlayerListInGrid(std::list<Player*>& playerList, float maxSearchRange) const
{
    Trinity::AnyPlayerInObjectRangeCheck checker(this, maxSearchRange);
    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(this, playerList, checker);
    Trinity::VisitNearbyWorldObject(this, maxSearchRange, searcher);
}

void WorldObject::GetGameObjectListWithEntryInGridAppend(std::list<GameObject*>& gameobjectList, uint32 entry, float maxSearchRange) const
{
    std::list<GameObject*> tempList;
    GetGameObjectListWithEntryInGrid(tempList, entry, maxSearchRange);
    gameobjectList.sort();
    tempList.sort();
    gameobjectList.merge(tempList);
}

void WorldObject::GetCreatureListWithEntryInGridAppend(std::list<Creature*>& creatureList, uint32 entry, float maxSearchRange) const
{
    std::list<Creature*> tempList;
    GetCreatureListWithEntryInGrid(tempList, entry, maxSearchRange);
    creatureList.sort();
    tempList.sort();
    creatureList.merge(tempList);
}

void WorldObject::GetAliveCreatureListWithEntryInGrid(std::list<Creature*>& creatureList, uint32 entry, float maxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::AllAliveCreaturesOfEntryInRange check(this, entry, maxSearchRange);
    Trinity::CreatureListSearcher<Trinity::AllAliveCreaturesOfEntryInRange> searcher(this, creatureList, check);

    cell.Visit(pair, Trinity::makeGridVisitor(searcher), *this->GetMap(), *this, maxSearchRange);
}

void WorldObject::GetCorpseCreatureInGrid(std::list<Creature*>& creatureList, float maxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::SearchCorpseCreatureCheck check(this, maxSearchRange);
    Trinity::CreatureListSearcher<Trinity::SearchCorpseCreatureCheck> searcher(this, creatureList, check);

    cell.Visit(pair, Trinity::makeGridVisitor(searcher), *this->GetMap(), *this, maxSearchRange);
}

void WorldObject::GetNearPoint2D(float &x, float &y, float distance2d, float absAngle, bool allowObjectSize) const
{
    x = GetPositionX() + (distance2d + (allowObjectSize ? GetObjectSize() : 0.0f)) * std::cos(absAngle);
    y = GetPositionY() + (distance2d + (allowObjectSize ? GetObjectSize() : 0.0f)) * std::sin(absAngle);

    Trinity::NormalizeMapCoord(x);
    Trinity::NormalizeMapCoord(y);
}

void WorldObject::GetNearPoint2D(Position &pos, float distance2d, float angle, bool allowObjectSize) const
{
    angle += GetOrientation();
    float x = GetPositionX() + (distance2d + (allowObjectSize ? GetObjectSize() : 0.0f)) * std::cos(angle);
    float y = GetPositionY() + (distance2d + (allowObjectSize ? GetObjectSize() : 0.0f)) * std::sin(angle);

    Trinity::NormalizeMapCoord(x);
    Trinity::NormalizeMapCoord(y);

    pos.m_positionX = x;
    pos.m_positionY = y;
    pos.m_positionZ = GetPositionZ();

    float ground = pos.m_positionZ;

    UpdateAllowedPositionZ(x, y, ground);

    if (fabs(pos.m_positionZ - ground) < 6)
        pos.m_positionZ = ground;
}

void WorldObject::GetNearPoint(WorldObject const* searcher, float &x, float &y, float &z, float searcher_size, float distance2d, float absAngle) const
{
    GetNearPoint2D(x, y, distance2d + searcher_size, absAngle);
    z = GetPositionZH() + (searcher ? searcher->GetPositionH() : 0.0f);

    if (!searcher)
        UpdateAllowedPositionZ(x, y, z);
    else if (!searcher->ToCreature() || !searcher->GetMap()->Instanceable())
        searcher->UpdateAllowedPositionZ(x, y, z);

    // if detection disabled, return first point
    if (!sWorld->getBoolConfig(CONFIG_DETECT_POS_COLLISION))
        return;

    // return if the point is already in LoS
    if (IsWithinLOS(x, y, z))
        return;

    // remember first point
    float first_x = x;
    float first_y = y;
    float first_z = z;

    // loop in a circle to look for a point in LoS using small steps
    for (float angle = float(M_PI) / 8; angle < float(M_PI) * 2; angle += float(M_PI) / 8)
    {
        GetNearPoint2D(x, y, distance2d + searcher_size, absAngle + angle);
        z = GetPositionZ() + 2.0f;
        UpdateAllowedPositionZ(x, y, z);
        if (IsWithinLOS(x, y, z))
            return;
    }

    // still not in LoS, give up and return first position found
    x = first_x;
    y = first_y;
    z = first_z;
}

void WorldObject::GetClosePoint(float& x, float& y, float& z, float size, float distance2d, float angle) const
{
    // angle calculated from current orientation
    GetNearPoint(nullptr, x, y, z, size, distance2d, GetOrientation() + angle);
}

void WorldObject::MovePosition(Position &pos, float dist, float angle)
{
    angle += GetOrientation();
    float destx = pos.m_positionX + dist * std::cos(angle);
    float desty = pos.m_positionY + dist * std::sin(angle);

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty))
    {
        // TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::MovePosition invalid coordinates X: %f and Y: %f were passed!", destx, desty);
        return;
    }

    float ground = GetHeight(destx, desty, MAX_HEIGHT, true);
    float floor = GetHeight(destx, desty, pos.m_positionZ, true);
    float destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;

    float step = dist / 10.0f;

    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes
        if (fabs(pos.m_positionZ - destz) > 6)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            ground = GetHeight(destx, desty, MAX_HEIGHT, true);
            floor = GetHeight(destx, desty, pos.m_positionZ, true);
            destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;
        }
        // we have correct destz now
        else
        {
            pos.Relocate(destx, desty, destz);
            break;
        }
    }

    Trinity::NormalizeMapCoord(pos.m_positionX);
    Trinity::NormalizeMapCoord(pos.m_positionY);
    UpdateGroundPositionZ(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
    pos.SetOrientation(GetOrientation());
}

void WorldObject::GetNearPosition(Position& pos, float dist, float angle)
{
    GetPosition(&pos);
    MovePosition(pos, dist, angle);
}

void WorldObject::MovePositionToFirstCollision(Position &pos, float dist, float angle)
{
    angle += GetOrientation();
    if (!IsInWater())
        pos.m_positionZ += 2.0f;
    float destx = pos.m_positionX + dist * std::cos(angle);
    float desty = pos.m_positionY + dist * std::sin(angle);

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty))
    {
        // TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::MovePositionToFirstCollision invalid coordinates X: %f and Y: %f were passed!", destx, desty);
        return;
    }

    bool isFalling = m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
    float ground = GetHeight(destx, desty, MAX_HEIGHT, true);
    float floor = GetHeight(destx, desty, pos.m_positionZ, true);
    float destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;

    if (IsInWater()) // In water not allow change Z to ground
    {
        if (pos.m_positionZ > destz)
            destz = pos.m_positionZ;
    }

    bool _checkZ = true;
    if (isFalling) // Allowed point in air if we falling
    {
        float z_now = m_movementInfo.fall.lastTimeUpdate ? (pos.m_positionZ - Movement::computeFallElevation(Movement::MSToSec(getMSTime() - m_movementInfo.fall.lastTimeUpdate), false) - 5.0f) : pos.m_positionZ;
        if ((z_now - ground) > 10.0f)
        {
            destz = z_now;
            _checkZ = false;
        }
    }

    float step = dist / 10.0f;
    for (uint8 j = 0; j < 10; ++j)
    {
        if (fabs(pos.m_positionZ - destz) > 6 && _checkZ)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            ground = GetHeight(destx, desty, MAX_HEIGHT, true);
            floor = GetHeight(destx, desty, pos.m_positionZ, true);
            destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;
        }
        else
            break;
    }

    bool col = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ + 0.5f, destx, desty, destz + 0.5f, destx, desty, destz, -0.5f);
    // collision occurred
    if (col)
    {
        // move back a bit
        destx -= CONTACT_DISTANCE * std::cos(angle);
        desty -= CONTACT_DISTANCE * std::sin(angle);
        dist = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
    }
    else
        destz -= 0.5f;

    // check dynamic collision
    col = GetMap()->getObjectHitPos(GetPhases(), IsPlayer(), pos.m_positionX, pos.m_positionY, pos.m_positionZ + 0.5f, destx, desty, destz + 0.5f, destx, desty, destz, -0.5f);

    // Collided with a gameobject
    if (col)
    {
        destx -= CONTACT_DISTANCE * std::cos(angle);
        desty -= CONTACT_DISTANCE * std::sin(angle);
        dist = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
    }
    else
        destz -= 0.5f;

    step = dist / 10.0f;
    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes
        if (fabs(pos.m_positionZ - destz) > 6 && _checkZ)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            ground = GetHeight(destx, desty, MAX_HEIGHT, true);
            floor = GetHeight(destx, desty, pos.m_positionZ, true);
            destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;
        }
        // we have correct destz now
        else
        {
            pos.Relocate(destx, desty, destz);
            break;
        }
    }

    Trinity::NormalizeMapCoord(pos.m_positionX);
    Trinity::NormalizeMapCoord(pos.m_positionY);
    UpdateAllowedPositionZ(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
    pos.SetOrientation(GetOrientation());
}

void WorldObject::MovePositionToTransportCollision(Position &pos, float dist, float angle)
{
    Transport* transport = GetTransport();
    if (!transport)
    {
        // TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::MovePositionToTransportCollision invalid transport");
        return;
    }

    GameObjectModel* _model = transport->m_model;
    if (!_model)
    {
        // TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::MovePositionToTransportCollision invalid transport GameObjectModel");
        return;
    }

    angle += GetOrientation();

    transport->CalculatePassengerPosition(pos.m_positionX, pos.m_positionY, pos.m_positionZ);

    float destx = pos.m_positionX + dist * std::cos(angle);
    float desty = pos.m_positionY + dist * std::sin(angle);

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty))
    {
        // TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::MovePositionToTransportCollision invalid coordinates X: %f and Y: %f were passed!", destx, desty);
        return;
    }

    float floor = GetHeight(destx, desty, pos.m_positionZ + 2.0f, true);
    float destz = floor;

    if (destz == VMAP_INVALID_HEIGHT_VALUE)
    {
        float step = dist / 10.0f;
        for (uint8 j = 0; j < 10; ++j)
        {
            // find correct Z
            if (destz == VMAP_INVALID_HEIGHT_VALUE)
            {
                destx -= step * std::cos(angle);
                desty -= step * std::sin(angle);
                floor = GetHeight(destx, desty, pos.m_positionZ + 2.0f, true);
                destz = floor;
            }
            else
                break;
        }
    }

    if (destz == VMAP_INVALID_HEIGHT_VALUE)
    {
        TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::MovePositionToTransportCollision invalid destz: %f", destz);
        return;
    }

    G3D::Vector3 pos1(pos.m_positionX, pos.m_positionY, pos.m_positionZ + 2.0f);
    G3D::Vector3 pos2(destx, desty, destz + 2.0f);
    G3D::Vector3 resultPos;

    bool col = _model->getObjectHitPos(GetPhases(), IsPlayer(), pos1, pos2, resultPos, -0.5f);
    destx = resultPos.x;
    desty = resultPos.y;
    destz = resultPos.z;

    // Collided with a transport
    if (col)
    {
        dist = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
        if (dist > CONTACT_DISTANCE)
        {
            destx -= CONTACT_DISTANCE * std::cos(angle);
            desty -= CONTACT_DISTANCE * std::sin(angle);
            dist = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
        }
    }
    else
        destz -= 2.0f;

    float step = dist / 10.0f;
    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes
        if (fabs(pos.m_positionZ - destz) > 6)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            floor = GetHeight(destx, desty, pos.m_positionZ, true);
            destz = floor;
        }
        // we have correct destz now
        else
        {
            pos.Relocate(destx, desty, destz);
            break;
        }
    }

    Trinity::NormalizeMapCoord(pos.m_positionX);
    Trinity::NormalizeMapCoord(pos.m_positionY);
    UpdateAllowedPositionZ(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
    pos.SetOrientation(GetOrientation());
}

void WorldObject::GetFirstCollisionPosition(Position& pos, float dist, float angle)
{
    if (Transport* transport = GetTransport())
    {
        GetPosition(&pos, transport);
        MovePositionToTransportCollision(pos, dist, angle);
        return;
    }
    GetPosition(&pos);
    MovePositionToFirstCollision(pos, dist, angle);
}

void WorldObject::MovePositionToCollisionBetween(Position &pos, float distMin, float distMax, float angle)
{
    angle += GetOrientation();
    pos.m_positionZ += 2.0f;

    float tempDestx = pos.m_positionX + distMin * std::cos(angle);
    float tempDesty = pos.m_positionY + distMin * std::sin(angle);

    float destx = pos.m_positionX + distMax * std::cos(angle);
    float desty = pos.m_positionY + distMax * std::sin(angle);

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty))
    {
        // TC_LOG_FATAL(LOG_FILTER_GENERAL, "WorldObject::MovePositionToCollisionBetween invalid coordinates X: %f and Y: %f were passed!", destx, desty);
        return;
    }

    float ground = GetHeight(destx, desty, MAX_HEIGHT, true);
    float floor = GetHeight(destx, desty, pos.m_positionZ, true);
    float destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;

    bool col = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), tempDestx, tempDesty, pos.m_positionZ + 0.5f, destx, desty, destz + 0.5f, destx, desty, destz, -0.5f);

    // collision occurred
    if (col)
    {
        // move back a bit
        destx -= CONTACT_DISTANCE * std::cos(angle);
        desty -= CONTACT_DISTANCE * std::sin(angle);
        distMax = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
    }

    // check dynamic collision
    col = GetMap()->getObjectHitPos(GetPhases(), IsPlayer(), tempDestx, tempDesty, pos.m_positionZ + 0.5f, destx, desty, destz + 0.5f, destx, desty, destz, -0.5f);

    // Collided with a gameobject
    if (col)
    {
        destx -= CONTACT_DISTANCE * std::cos(angle);
        desty -= CONTACT_DISTANCE * std::sin(angle);
        distMax = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
    }

    float step = distMax / 10.0f;

    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes
        if (fabs(pos.m_positionZ - destz) > 6)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            ground = GetHeight(destx, desty, MAX_HEIGHT, true);
            floor = GetHeight(destx, desty, pos.m_positionZ, true);
            destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;
        }
        // we have correct destz now
        else
        {
            pos.Relocate(destx, desty, destz);
            break;
        }
    }

    Trinity::NormalizeMapCoord(pos.m_positionX);
    Trinity::NormalizeMapCoord(pos.m_positionY);
    UpdateAllowedPositionZ(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
    pos.SetOrientation(GetOrientation());
}

void WorldObject::GetCollisionPositionBetween(Position& pos, float distMin, float distMax, float angle)
{
    GetPosition(&pos);
    MovePositionToCollisionBetween(pos, distMin, distMax, angle);
}

void WorldObject::GetRandomNearPosition(Position& pos, float radius)
{
    GetPosition(&pos);
    MovePosition(pos, radius * static_cast<float>(rand_norm()), static_cast<float>(rand_norm()) * static_cast<float>(2 * M_PI));
}

void WorldObject::GetContactPoint(const WorldObject* obj, float& x, float& y, float& z, float distance2d) const
{
    // angle to face `obj` to `this` using distance includes size of `obj`
    GetNearPoint(obj, x, y, z, obj->GetObjectSize(), distance2d, GetAngle(obj));
}

void WorldObject::GenerateCollisionNonDuplicatePoints(std::list<Position>& randPosList, uint8 maxPoint, float randMin, float randMax, float minDist)
{
    Position pos;
    bool badPos = false;
    uint8 traiCount = 0;

    while (randPosList.size() < maxPoint)
    {
        badPos = false;
        GetFirstCollisionPosition(pos, frand(randMin, randMax), frand(0.0f, 6.28f));
        ++traiCount;

        for (auto _pos : randPosList)
        {
            if (pos.GetExactDist(&_pos) <= minDist)
            {
                badPos = true;
                break;
            }
        }

        if (!badPos || traiCount > (maxPoint * 10))
            randPosList.push_back(pos);
    }
}

float WorldObject::GetObjectSize() const
{
    return m_valuesCount > UNIT_FIELD_COMBAT_REACH ? m_floatValues[UNIT_FIELD_COMBAT_REACH] : DEFAULT_WORLD_OBJECT_SIZE;
}

void WorldObject::SetPhaseMask(uint32 newPhaseMask, bool update)
{
    m_phaseMask = newPhaseMask;

    if (update && IsInWorld())
        UpdateObjectVisibility();
}

void WorldObject::PlayDistanceSound(uint32 soundID, Player* target /*= nullptr*/)
{
    WorldPackets::GameObject::PlayObjectSound objectSound;
    objectSound.SourceObjectGUID = GetGUID();
    if (target)
        objectSound.TargetObjectGUID = target->GetGUID();
    objectSound.Pos = GetPosition();
    objectSound.SoundId = soundID;

    if (target)
        target->SendDirectMessage(objectSound.Write());
    else
        SendMessageToSet(objectSound.Write(), true);
}

void WorldObject::PlayDirectSound(uint32 soundKitID, Player* target /*= NULL*/)
{
    WorldPackets::Misc::PlaySound  sound;
    sound.SoundKitID = soundKitID;
    sound.SourceObjectGuid = GetGUID();

    if (target)
        target->SendDirectMessage(sound.Write());
    else
        SendMessageToSet(sound.Write(), true);
}

void WorldObject::DestroyForNearbyPlayers()
{
    if (!IsInWorld())
        return;

    std::list<Player*> targets;
    Trinity::AnyPlayerInObjectRangeCheck check(this, GetVisibilityRange(), false);
    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(this, targets, check);
    Trinity::VisitNearbyWorldObject(this, GetVisibilityRange(), searcher);
    for (std::list<Player*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
    {
        Player* player = *iter;

        if (player == this)
            continue;

        if (!player->HaveAtClient(this))
            continue;

        if (player->IsOnVehicle() || IsUnit() && ToUnit()->IsOnVehicle())
            if (player->HaveExtraLook(GetGUID()))
                continue;

        if (IsUnit() && ToUnit()->GetCharmerGUID() == player->GetGUID()) // TODO: this is for puppet
            continue;

        DestroyForPlayer(player);
        player->RemoveClient(GetGUID());
        player->GetVignetteMgr().OnWorldObjectDisappear(this);
    }
}

void WorldObject::UpdateObjectVisibility(bool /*forced*/)
{
    //updates object's visibility for nearby players
    Trinity::VisibleChangesNotifier notifier(*this);
    Trinity::VisitNearbyWorldObject(this, GetVisibilityRange(), notifier);
}

struct WorldObjectChangeAccumulator
{
    UpdateDataMapType& i_updateDatas;
    WorldObject& i_object;
    GuidSet plr_list;

    WorldObjectChangeAccumulator(WorldObject &obj, UpdateDataMapType &d) : i_updateDatas(d), i_object(obj) { }

    void Visit(PlayerMapType &m)
    {
        for (auto &source : m)
        {
            BuildPacket(source);

            if (!source->GetSharedVisionList().empty())
                for (auto &player : source->GetSharedVisionList())
                    BuildPacket(player);
        }
    }

    void Visit(CreatureMapType &m)
    {
        for (auto &source : m)
        {
            if (!source->GetSharedVisionList().empty())
                for (auto &player : source->GetSharedVisionList())
                    BuildPacket(player);
        }
    }

    void Visit(DynamicObjectMapType &m)
    {
        for (auto &source : m)
        {
            auto const guid = source->GetCasterGUID();
            if (!guid.IsPlayer())
                continue;

            // Caster may be NULL if DynObj is in removelist
            auto const caster = ObjectAccessor::FindPlayer(guid);
            if (caster && caster->GetGuidValue(PLAYER_FIELD_FARSIGHT_OBJECT) == source->GetGUID())
                BuildPacket(caster);
        }
    }

    void BuildPacket(Player* player)
    {
        if (!player) // http://pastebin.com/RmVRfpHS
            return;

        // Only send update once to a player
        if (plr_list.find(player->GetGUID()) == plr_list.end() && player->HaveAtClient(&i_object))
        {
            i_object.BuildFieldsUpdate(player, i_updateDatas);
            plr_list.insert(player->GetGUID());
        }
    }

    template <typename NotInterested>
    void Visit(NotInterested &) { }
};

void WorldObject::BuildUpdate(UpdateDataMapType& data_map)
{
    CellCoord p = Trinity::ComputeCellCoord(GetPositionX(), GetPositionY());
    Cell cell(p);
    cell.SetNoCreate();
    WorldObjectChangeAccumulator notifier(*this, data_map);

    //we must build packets for all visible players
    cell.Visit(p, Trinity::makeWorldVisitor(notifier), *GetMap(), *this, GetVisibilityRange());

    ClearUpdateMask(false);
}

void WorldObject::DestroyForPlayer(Player* target) const
{
    Object::DestroyForPlayer(target);
}

ObjectGuid WorldObject::GetTransGUID() const
{
    if (GetTransport())
        return GetTransport()->GetGUID();
    return ObjectGuid::Empty;
}

//! if someone has phaseID but enother has empty - not see any! YES! NOT SEE! FIX2. WHY SHOULD? 
//      FOR SUPPORT OLD STYLE NEED ALLOW TO SEE. FOR SUPER HIDE PHASE ALL SHOULD HAVE SOME PHASEIDs
//! If some have 1 2 enother has 1 = see each other.
//! ir some have 1 2 enorther has 3 - not see.
//! if some has ignorePhase id - see each.
bool WorldObject::InSamePhaseId(std::set<uint32> const& phase, bool otherIsPlayer) const
{
    if (IgnorePhaseId())
        return true;

    if (IsPlayer() && otherIsPlayer)
        return true;

    if (phase.empty() && m_phaseId.empty())
        return true;

    if (IsPlayer() && phase.empty())
        return true;

    if (otherIsPlayer && m_phaseId.empty())
        return true;

    //! speed up case. should be done in any way. 
    // As iteration not check empty data but it should be done.
    if (phase.empty() && !m_phaseId.empty() || !phase.empty() && m_phaseId.empty())
        return false;

    //! check target phases
    for (auto PhaseID : phase)
    {
        if (PhaseID >= m_phaseBit.size())
            continue;

        if (m_phaseBit[PhaseID])
            return true;
    }
    return false;
}

std::set<uint32> const& WorldObject::GetPhases() const
{
    return m_phaseId;
}

bool WorldObject::InSamePhaseId(WorldObject const* obj) const
{
    return obj->IgnorePhaseId() || InSamePhaseId(obj->GetPhases(), obj->IsPlayer());
}

bool WorldObject::InSamePhase(WorldObject const* obj) const
{
    auto isPlayer = IsPlayer();
    auto isObjPlayer = obj->IsPlayer();

    if (!isPlayer && IsUnit())
        if (auto owner = ToUnit()->GetAnyOwner())
            isPlayer = owner->IsPlayer();

    if (!isObjPlayer && obj->IsUnit())
        if (auto owner = obj->ToUnit()->GetAnyOwner())
            isObjPlayer = owner->IsPlayer();

    return InSamePhase(obj->GetPhaseMask()) && (isPlayer && isObjPlayer || InSamePhaseId(obj));
}

bool WorldObject::RemovePhase(uint32 PhaseID)
{
    if (PhaseID >= m_phaseBit.size() || !m_phaseBit[PhaseID])
        return false;

    m_phaseBit[PhaseID] = false;

    return m_phaseId.erase(PhaseID);
}

void WorldObject::SetPhaseId(std::set<uint32> const& newPhaseId, bool /*update*/)
{
    m_phaseBit.clear();
    for (auto PhaseID : newPhaseId)
    {
        if (PhaseID >= m_phaseBit.size())
            m_phaseBit.resize(PhaseID+1, false);

        m_phaseBit[PhaseID] = true;
    }
    m_phaseId = newPhaseId;
};

bool WorldObject::HasPhaseId(uint32 PhaseID) const
{
    if (PhaseID >= m_phaseBit.size())
        return false;
    return m_phaseBit[PhaseID];
}

C_PTR WorldObject::get_ptr()
{
    if (ptr.numerator && ptr.numerator->ready)
        return ptr.shared_from_this();

    ptr.InitParent(this);
    ASSERT(ptr.numerator);  // It's very bad. If it hit nothing work.
    return ptr.shared_from_this();
}

void WorldObject::RebuildTerrainSwaps()
{
    // Clear all terrain swaps, will be rebuilt below
    // Reason for this is, multiple phases can have the same terrain swap, we should not remove the swap if another phase still use it
    _terrainSwaps.clear();
    /*ConditionList conditions;

    // Check all applied phases for terrain swap and add it only once
    for (uint32 phaseId : _phases)
    {
        std::list<uint32>& swaps = sObjectMgr->GetPhaseTerrainSwaps(phaseId);

        for (uint32 swap : swaps)
        {
            // only add terrain swaps for current map
            MapEntry const* mapEntry = sMapStore.LookupEntry(swap);
            if (!mapEntry || mapEntry->ParentMapID != int32(GetMapId()))
                continue;

            conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_TERRAIN_SWAP, swap);

            if (sConditionMgr->IsObjectMeetToConditions(this, conditions))
                _terrainSwaps.insert(swap);
        }
    }

    // get default terrain swaps, only for current map always
    std::list<uint32>& mapSwaps = sObjectMgr->GetDefaultTerrainSwaps(GetMapId());

    for (uint32 swap : mapSwaps)
    {
        conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_TERRAIN_SWAP, swap);

        if (sConditionMgr->IsObjectMeetToConditions(this, conditions))
            _terrainSwaps.insert(swap);
    }*/

    // online players have a game client with world map display
    if (IsPlayer())
        RebuildWorldMapAreaSwaps();
}

void WorldObject::RebuildWorldMapAreaSwaps()
{
    // Clear all world map area swaps, will be rebuilt below
    _worldMapAreaSwaps.clear();

    // get ALL default terrain swaps, if we are using it (condition is true)
    // send the worldmaparea for it, to see swapped worldmaparea in client from other maps too, not just from our current
    /*TerrainPhaseInfo defaults = sObjectMgr->GetDefaultTerrainSwapStore();
    for (TerrainPhaseInfo::const_iterator itr = defaults.begin(); itr != defaults.end(); ++itr)
    {
        for (uint32 swap : itr->second)
        {
            ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_TERRAIN_SWAP, swap);
            if (sConditionMgr->IsObjectMeetToConditions(this, conditions))
            {
                for (uint32 map : sObjectMgr->GetTerrainWorldMaps(swap))
                    _worldMapAreaSwaps.insert(map);
            }
        }
    }

    // Check all applied phases for world map area swaps
    for (uint32 phaseId : _phases)
    {
        std::list<uint32>& swaps = sObjectMgr->GetPhaseTerrainSwaps(phaseId);

        for (uint32 swap : swaps)
        {
            // add world map swaps for ANY map

            ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_TERRAIN_SWAP, swap);

            if (sConditionMgr->IsObjectMeetToConditions(this, conditions))
            {
                for (uint32 map : sObjectMgr->GetTerrainWorldMaps(swap))
                    _worldMapAreaSwaps.insert(map);
            }
        }
    }*/
}

template<class NOTIFIER>
void WorldObject::VisitNearbyGridObject(const float &radius, NOTIFIER &notifier) const
{
    if (IsInWorld())
        GetMap()->VisitGrid(GetPositionX(), GetPositionY(), radius, notifier);
}

template<class NOTIFIER>
void WorldObject::VisitNearbyWorldObject(const float &radius, NOTIFIER &notifier) const
{
    if (IsInWorld())
        GetMap()->VisitWorld(GetPositionX(), GetPositionY(), radius, notifier);
}

void WorldObject::Clear()
{
    Object::Clear();

    m_phaseId.clear();
    m_phaseBit.clear();
    _terrainSwaps.clear();
    _worldMapAreaSwaps.clear();
    _visibilityPlayerList.clear();
    _hideForGuid.clear();
}

void WorldObject::RemoveFromWorld()
{
    if (!IsInWorld())
        return;

    DestroyForNearbyPlayers();
    Object::RemoveFromWorld();
}

void WorldObject::SetRWVisibilityRange(float rwvisible)
{
    if (rwvisible <= 0.0f)
    {
        m_rwVisibility = false;
        m_rwVisibilityRange = 0.0f;
        return;
    }
    m_rwVisibility = true;
    m_rwVisibilityRange = rwvisible;
}

bool WorldObject::IsRWVisibility()
{
    return m_rwVisibility;
}

float WorldObject::GetRWVisibility()
{
    return m_rwVisibilityRange;
}

void Object::Clear()
{
    if (IsInWorld())
        Object::RemoveFromWorld();

    delete[] m_uint32Values;
    m_uint32Values = nullptr;

    delete[] _dynamicValues;
    _dynamicValues = nullptr;

    delete[] _dynamicChangesArrayMask;
    _dynamicChangesArrayMask = nullptr;

    _changesMask.clear();
    _dynamicChangesMask.clear();
}

Trinity::ObjectDistanceOrderPred::ObjectDistanceOrderPred(const WorldObject* pRefObj, bool ascending) : m_refObj(pRefObj), m_ascending(ascending)
{
}

bool Trinity::ObjectDistanceOrderPred::operator()(const WorldObject* pLeft, const WorldObject* pRight) const
{
    return m_ascending ? m_refObj->GetDistanceOrder(pLeft, pRight) : !m_refObj->GetDistanceOrder(pLeft, pRight);
}

Trinity::GuidValueSorterPred::GuidValueSorterPred(bool ascending) : m_ascending(ascending)
{
}

bool Trinity::GuidValueSorterPred::operator()(const WorldObject* pLeft, const WorldObject* pRight) const
{
    if (!pLeft->IsInWorld() || !pRight->IsInWorld())
        return false;

    return m_ascending ? pLeft->GetGUIDLow() < pRight->GetGUIDLow() : pLeft->GetGUIDLow() > pRight->GetGUIDLow();
}
