/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "MovementPackets.h"
#include "MoveSpline.h"
#include "MoveSplineFlag.h"
#include "MovementTypedefs.h"

ByteBuffer& operator<<(ByteBuffer& data, MovementInfo& movementInfo)
{
    data << movementInfo.Guid;
    data << movementInfo.MoveTime;
    data << movementInfo.Pos.PositionXYZOStream();
    data << movementInfo.pitch;
    data << movementInfo.splineElevation;

    data << static_cast<uint32>(movementInfo.RemoveForcesIDs.size());
    data << movementInfo.MoveIndex;

    for (ObjectGuid const& guid : movementInfo.RemoveForcesIDs)
        data << guid;

    data.WriteBits(movementInfo.MoveFlags[0], 30);
    data.WriteBits(movementInfo.MoveFlags[1], 18);

    data.WriteBit(movementInfo.hasTransportData);
    data.WriteBit(movementInfo.hasFallData);
    data.WriteBit(movementInfo.hasSpline);
    data.WriteBit(movementInfo.HeightChangeFailed);
    data.WriteBit(movementInfo.RemoteTimeValid);

    data.FlushBits();

    if (movementInfo.hasTransportData)
        data << movementInfo.transport;

    if (movementInfo.hasFallData)
    {
        data << movementInfo.fall.fallTime;
        data << movementInfo.fall.JumpVelocity;

        data.WriteBit(movementInfo.fall.hasFallDirection);
        data.FlushBits();
        if (movementInfo.fall.hasFallDirection)
        {
            data << movementInfo.fall.Direction;
            data << movementInfo.fall.HorizontalSpeed;
        }
    }

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, MovementInfo& movementInfo)
{
    movementInfo.MoveTime = getMSTime();
    data >> movementInfo.Guid;
    data >> movementInfo.ClientMoveTime;
    data >> movementInfo.Pos.PositionXYZOStream();
    data >> movementInfo.pitch;
    data >> movementInfo.splineElevation;

    movementInfo.RemoveForcesIDs.reserve(data.read<uint32>());
    data >> movementInfo.MoveIndex;

    for (ObjectGuid& guid : movementInfo.RemoveForcesIDs)
        data >> guid;

    movementInfo.MoveFlags[0] = data.ReadBits(30);
    movementInfo.MoveFlags[1] = data.ReadBits(18);

    movementInfo.hasTransportData = data.ReadBit();
    movementInfo.hasFallData = data.ReadBit();
    movementInfo.hasSpline = data.ReadBit();

    movementInfo.HeightChangeFailed = data.ReadBit();
    movementInfo.RemoteTimeValid = data.ReadBit();

    if (movementInfo.hasTransportData)
        data >> movementInfo.transport;

    if (movementInfo.hasFallData)
    {
        data >> movementInfo.fall.fallTime;
        data >> movementInfo.fall.JumpVelocity;
        movementInfo.fall.lastTimeUpdate = getMSTime();

        movementInfo.fall.hasFallDirection = data.ReadBit();
        if (movementInfo.fall.hasFallDirection)
        {
            data >> movementInfo.fall.Direction;
            data >> movementInfo.fall.HorizontalSpeed;
        }
    }
    else
        movementInfo.fall.lastTimeUpdate = 0;

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, MovementInfo::TransportInfo& transportInfo)
{
    data >> transportInfo.Guid;
    data >> transportInfo.Pos.PositionXYZOStream();
    data >> transportInfo.VehicleSeatIndex;
    data >> transportInfo.MoveTime;

    bool hasPrevTime = data.ReadBit();
    bool hasVehicleId = data.ReadBit();

    if (hasPrevTime)
        data >> transportInfo.PrevMoveTime;

    if (hasVehicleId)
        data >> transportInfo.VehicleRecID;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, MovementInfo::TransportInfo const& transportInfo)
{
    bool hasPrevTime = transportInfo.PrevMoveTime != 0;
    bool hasVehicleId = transportInfo.VehicleRecID != 0;

    data << transportInfo.Guid;
    data << transportInfo.Pos.PositionXYZOStream();
    data << transportInfo.VehicleSeatIndex;
    data << transportInfo.MoveTime;

    data.WriteBit(hasPrevTime);
    data.WriteBit(hasVehicleId);
    data.FlushBits();

    if (hasPrevTime)
        data << transportInfo.PrevMoveTime;

    if (hasVehicleId)
        data << transportInfo.VehicleRecID;

    return data;
}

void WorldPackets::Movement::ClientPlayerMovement::Read()
{
    _worldPacket >> movementInfo;
}

ByteBuffer& WorldPackets::operator<<(ByteBuffer& data, Movement::MonsterSplineFilterKey const& monsterSplineFilterKey)
{
    data << monsterSplineFilterKey.Idx;
    data << monsterSplineFilterKey.Speed;

    return data;
}

ByteBuffer& WorldPackets::operator<<(ByteBuffer& data, Movement::MonsterSplineFilter const& monsterSplineFilter)
{
    data << static_cast<uint32>(monsterSplineFilter.FilterKeys.size());
    data << monsterSplineFilter.BaseSpeed;
    data << monsterSplineFilter.StartOffset;
    data << monsterSplineFilter.DistToPrevFilterKey;
    data << monsterSplineFilter.AddedToStart;
    for (Movement::MonsterSplineFilterKey const& filterKey : monsterSplineFilter.FilterKeys)
        data << filterKey;

    data.WriteBits(monsterSplineFilter.FilterFlags, 2);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Movement::MonsterSplineSpellEffectExtraData const& spellEffectExtraData)
{
    data << spellEffectExtraData.TargetGUID;
    data << uint32(spellEffectExtraData.SpellVisualID);
    data << uint32(spellEffectExtraData.ProgressCurveID);
    data << uint32(spellEffectExtraData.ParabolicCurveID);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Movement::MovementForce const& movementForce)
{
    data << movementForce.ID;
    data << movementForce.Direction;
    data << movementForce.TransportPosition;
    data << movementForce.TransportID;
    data << movementForce.Magnitude;
    data.WriteBits(movementForce.Type, 2);
    data.FlushBits();

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Movement::MovementForce& movementForce)
{
    data >> movementForce.ID;
    data >> movementForce.Direction;
    data >> movementForce.TransportPosition;
    data >> movementForce.TransportID;
    data >> movementForce.Magnitude;
    data.ResetBitReader();
    movementForce.Type = data.ReadBits(2);

    return data;
}

ByteBuffer& WorldPackets::operator<<(ByteBuffer& data, Movement::MovementSpline const& movementSpline)
{
    data << movementSpline.Flags;
    data << movementSpline.AnimTier;
    data << movementSpline.TierTransStartTime;
    data << movementSpline.Elapsed;
    data << movementSpline.MoveTime;
    data << movementSpline.JumpGravity;
    data << movementSpline.SpecialTime;
    data << movementSpline.Mode;
    data << movementSpline.VehicleExitVoluntary;
    data << movementSpline.TransportGUID;
    data << movementSpline.VehicleSeat;
    data.WriteBits(movementSpline.Face, 2);
    data.WriteBits(movementSpline.Points.size(), 16);
    data.WriteBits(movementSpline.PackedDeltas.size(), 16);
    data.WriteBit(movementSpline.SplineFilter.is_initialized());
    data.WriteBit(movementSpline.SpellEffectExtraData.is_initialized());
    data.FlushBits();

    if (movementSpline.SplineFilter)
        data << *movementSpline.SplineFilter;

    switch (movementSpline.Face)
    {
        case MONSTER_MOVE_FACING_SPOT:
            data << movementSpline.FaceSpot;
            break;
        case MONSTER_MOVE_FACING_TARGET:
            data << movementSpline.FaceDirection;
            data << movementSpline.FaceGUID;
            break;
        case MONSTER_MOVE_FACING_ANGLE:
            data << movementSpline.FaceDirection;
            break;
        default:
            break;
    }

    for (auto const& pos : movementSpline.Points)
        data << pos;

    for (auto const& pos : movementSpline.PackedDeltas)
        data << pos;

    if (movementSpline.SpellEffectExtraData)
        data << *movementSpline.SpellEffectExtraData;

    return data;
}

ByteBuffer& WorldPackets::operator<<(ByteBuffer& data, Movement::MovementMonsterSpline const& movementMonsterSpline)
{
    data << movementMonsterSpline.ID;
    data << movementMonsterSpline.Destination;
    data.WriteBit(movementMonsterSpline.CrzTeleport);
    data.WriteBits(movementMonsterSpline.StopDistanceTolerance, 3);

    data << movementMonsterSpline.Move;

    return data;
}

void WorldPackets::Movement::CommonMovement::WriteCreateObjectSplineDataBlock(::Movement::MoveSpline const& moveSpline, ByteBuffer& data)
{
    data << uint32(moveSpline.GetId());

    auto dest = G3D::Vector3::zero();
    if (!moveSpline.isCyclic())
        dest = moveSpline.FinalDestination();

    data << dest.x << dest.y << dest.z;

    bool hasSplineMove = data.WriteBit(!moveSpline.Finalized() && !moveSpline.splineIsFacingOnly);
    data.FlushBits();

    if (hasSplineMove)
    {
        data << moveSpline.splineflags.raw();
        data << moveSpline.timePassed();
        data << moveSpline.Duration();
        data << moveSpline.durationModifier;
        data << moveSpline.nextDurationModifier;
        data.WriteBits(moveSpline.facing.type, 2);
        bool HasJumpGravity = data.WriteBit(moveSpline.splineflags.parabolic || moveSpline.splineflags.animation);
        bool HasSpecialTime = data.WriteBit(moveSpline.splineflags.parabolic && moveSpline.SpecialTime < moveSpline.Duration());
        data.WriteBits(moveSpline.getPath().size(), 16);
        data.WriteBits(uint8(moveSpline.spline.mode()), 2);
        data.WriteBit(false);                                                       // HasSplineFilter
        data.WriteBit(moveSpline.spell_effect_extra.is_initialized());
        data.FlushBits();

        //if (HasSplineFilterKey)
        //{
        //    data << uint32(FilterKeysCount);
        //    for (var i = 0; i < FilterKeysCount; ++i)
        //    {
        //        data << float(In);
        //        data << float(Out);
        //    }

        //    data.WriteBits(FilterFlags, 2);
        //    data.FlushBits();
        //}

        switch (moveSpline.facing.type)
        {
            case MONSTER_MOVE_FACING_SPOT:
            {
                // FaceSpot
                data << float(moveSpline.facing.f.x);
                data << float(moveSpline.facing.f.y);
                data << float(moveSpline.facing.f.z);
                break;
            }
            case MONSTER_MOVE_FACING_TARGET:
                data << moveSpline.facing.target;   // FaceGUID
                break;
            case MONSTER_MOVE_FACING_ANGLE:
                data << moveSpline.facing.angle;    // FaceDirection
                break;
            default:
                break;
        }

        if (HasJumpGravity)
            data << float(moveSpline.JumpGravity);

        if (HasSpecialTime)
            data << uint32(moveSpline.SpecialTime);

        data.append(moveSpline.getPath().data(), moveSpline.getPath().size());

        if (moveSpline.spell_effect_extra)
        {
            data << moveSpline.spell_effect_extra->Target;
            data << uint32(moveSpline.spell_effect_extra->SpellVisualId);
            data << uint32(moveSpline.spell_effect_extra->ProgressCurveId);
            data << uint32(moveSpline.spell_effect_extra->ParabolicCurveId);
        }
    }
}

void WorldPackets::Movement::MonsterMove::InitializeSplineData(::Movement::MoveSpline const& moveSpline)
{
    SplineData.ID = moveSpline.m_Id;
    auto& movementSpline = SplineData.Move;

    auto splineFlags = moveSpline.splineflags;
    splineFlags.enter_cycle = moveSpline.isCyclic();
    movementSpline.Flags = uint32(splineFlags & uint32(~::Movement::MoveSplineFlag::Mask_No_Monster_Move));
    movementSpline.Face = moveSpline.facing.type;
    movementSpline.FaceDirection = moveSpline.facing.angle;
    movementSpline.FaceGUID = moveSpline.facing.target;
    movementSpline.FaceSpot = Position(moveSpline.facing.f.x, moveSpline.facing.f.y, moveSpline.facing.f.z);

    if (splineFlags.animation)
    {
        movementSpline.AnimTier = splineFlags.getAnimationId();
        movementSpline.TierTransStartTime = moveSpline.SpecialTime;
    }

    movementSpline.MoveTime = moveSpline.Duration();

    if (splineFlags.parabolic)
    {
        movementSpline.JumpGravity = moveSpline.JumpGravity;
        movementSpline.SpecialTime = moveSpline.SpecialTime;
    }

    if (splineFlags.fadeObject)
        movementSpline.SpecialTime = moveSpline.SpecialTime;

    if (moveSpline.spell_effect_extra)
    {
        movementSpline.SpellEffectExtraData = boost::in_place();
        movementSpline.SpellEffectExtraData->TargetGUID = moveSpline.spell_effect_extra->Target;
        movementSpline.SpellEffectExtraData->SpellVisualID = moveSpline.spell_effect_extra->SpellVisualId;
        movementSpline.SpellEffectExtraData->ProgressCurveID = moveSpline.spell_effect_extra->ProgressCurveId;
        movementSpline.SpellEffectExtraData->ParabolicCurveID = moveSpline.spell_effect_extra->ParabolicCurveId;
    }

    auto const& spline = moveSpline.spline;
    auto const& array = spline.getPoints();

    if (splineFlags & ::Movement::MoveSplineFlag::UncompressedPath)
    {
        if (!splineFlags.cyclic)
        {
            uint32 count = spline.getPointCount() - 3;
            for (uint32 i = 0; i < count; ++i)
                movementSpline.Points.emplace_back(array[i + 2].x, array[i + 2].y, array[i + 2].z);
        }
        else
        {
            uint32 count = spline.getPointCount() - 3;
            movementSpline.Points.emplace_back(array[1].x, array[1].y, array[1].z);
            for (uint32 i = 0; i < count; ++i)
                movementSpline.Points.emplace_back(array[i + 1].x, array[i + 1].y, array[i + 1].z);
        }
    }
    else
    {
        uint32 lastIdx = spline.getPointCount() - 3;
        auto realPath = &spline.getPoint(1);

        movementSpline.Points.emplace_back(realPath[lastIdx].x, realPath[lastIdx].y, realPath[lastIdx].z);

        if (lastIdx > 1)
        {
            auto middle = (realPath[0] + realPath[lastIdx]) / 2.f;
            for (uint32 i = 1; i < lastIdx; ++i)
            {
                auto delta = middle - realPath[i];
                movementSpline.PackedDeltas.emplace_back(delta.x, delta.y, delta.z);
            }
        }
    }
}

WorldPacket const* WorldPackets::Movement::MonsterMove::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << Pos;
    _worldPacket << SplineData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveSplineSetSpeed::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << Speed;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveSetSpeed::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << SequenceIndex;
    _worldPacket << Speed;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveUpdateSpeed::Write()
{
    _worldPacket << *movementInfo;
    _worldPacket << Speed;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveSplineSetFlag::Write()
{
    _worldPacket << MoverGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveSetFlag::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << SequenceIndex;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveUpdate::Write()
{
    _worldPacket << *movementInfo;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::TransferPending::Write()
{
    _worldPacket << MapID;
    _worldPacket << OldMapPosition;
    _worldPacket.WriteBit(Ship.is_initialized());
    _worldPacket.WriteBit(TransferSpellID.is_initialized());
    _worldPacket.FlushBits();

    if (Ship)
    {
        _worldPacket << Ship->ID;
        _worldPacket << Ship->OriginMapID;
    }

    if (TransferSpellID)
        _worldPacket << *TransferSpellID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::TransferAborted::Write()
{
    _worldPacket << uint32(MapID);
    _worldPacket << uint8(Arg);
    _worldPacket << int32(MapDifficultyXConditionID);
    _worldPacket.WriteBits(TransfertAbort, 5);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::NewWorld::Write()
{
    _worldPacket << int32(MapID);
    _worldPacket << Pos;
    _worldPacket << uint32(Reason);
    _worldPacket << MovementOffset;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveTeleport::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << uint32(SequenceIndex);
    _worldPacket << Pos;
    _worldPacket << float(Facing);
    _worldPacket << uint8(PreloadWorld);

    _worldPacket.WriteBit(TransportGUID.is_initialized());
    _worldPacket.WriteBit(Vehicle.is_initialized());
    _worldPacket.FlushBits();

    if (Vehicle)
    {
        _worldPacket << Vehicle->VehicleSeatIndex;
        _worldPacket.WriteBit(Vehicle->VehicleExitVoluntary);
        _worldPacket.WriteBit(Vehicle->VehicleExitTeleport);
        _worldPacket.FlushBits();
    }

    if (TransportGUID)
        _worldPacket << *TransportGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveUpdateTeleport::Write()
{
    _worldPacket << *movementInfo;

    _worldPacket << static_cast<int32>(MovementForces.size());
    _worldPacket.WriteBit(WalkSpeed.is_initialized());
    _worldPacket.WriteBit(RunSpeed.is_initialized());
    _worldPacket.WriteBit(RunBackSpeed.is_initialized());
    _worldPacket.WriteBit(SwimSpeed.is_initialized());
    _worldPacket.WriteBit(SwimBackSpeed.is_initialized());
    _worldPacket.WriteBit(FlightSpeed.is_initialized());
    _worldPacket.WriteBit(FlightBackSpeed.is_initialized());
    _worldPacket.WriteBit(TurnRate.is_initialized());
    _worldPacket.WriteBit(PitchRate.is_initialized());
    _worldPacket.FlushBits();

    for (MovementForce const& force : MovementForces)
        _worldPacket << force;

    if (WalkSpeed)
        _worldPacket << *WalkSpeed;

    if (RunSpeed)
        _worldPacket << *RunSpeed;

    if (RunBackSpeed)
        _worldPacket << *RunBackSpeed;

    if (SwimSpeed)
        _worldPacket << *SwimSpeed;

    if (SwimBackSpeed)
        _worldPacket << *SwimBackSpeed;

    if (FlightSpeed)
        _worldPacket << *FlightSpeed;

    if (FlightBackSpeed)
        _worldPacket << *FlightBackSpeed;

    if (TurnRate)
        _worldPacket << *TurnRate;

    if (PitchRate)
        _worldPacket << *PitchRate;

    return &_worldPacket;
}

void WorldPackets::Movement::MoveTeleportAck::Read()
{
    _worldPacket >> MoverGUID;
    _worldPacket >> AckIndex;
    _worldPacket >> ClientMoveTime;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Movement::MovementAck& ack)
{
    data >> ack.movementInfo;
    data >> ack.AckIndex;

    return data;
}

void WorldPackets::Movement::MovementAckMessage::Read()
{
    _worldPacket >> Ack;
}

void WorldPackets::Movement::MovementSpeedAck::Read()
{
    _worldPacket >> Ack;
    _worldPacket >> Speed;
}

void WorldPackets::Movement::MoveRemoveMovementForceAck::Read()
{
    _worldPacket >> Ack;
    _worldPacket >> TriggerGUID;
}

void WorldPackets::Movement::MoveApplyMovementForceAck::Read()
{
    _worldPacket >> Ack;
    _worldPacket >> MovementForceData;
}

void WorldPackets::Movement::SetActiveMover::Read()
{
    _worldPacket >> ActiveMover;
}

WorldPacket const* WorldPackets::Movement::MoveSetActiveMover::Write()
{
    _worldPacket << MoverGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveUpdateKnockBack::Write()
{
    _worldPacket << *movementInfo;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveSetCollisionHeight::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << uint32(SequenceIndex);
    _worldPacket << MsgData.Height;
    _worldPacket << MsgData.Scale;
    _worldPacket << uint32(MountDisplayID);
    _worldPacket << int32(ScaleDuration);
    _worldPacket.WriteBits(MsgData.Reason, 2);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveUpdateCollisionHeight::Write()
{
    _worldPacket << *movementInfo;
    _worldPacket << float(Height);
    _worldPacket << float(Scale);

    return &_worldPacket;
}

void WorldPackets::Movement::MoveSetCollisionHeightAck::Read()
{
    _worldPacket >> Data;
    _worldPacket >> MsgData.Height;
    _worldPacket >> MountDisplayID;
    MsgData.Reason = UpdateCollisionHeightReason(_worldPacket.ReadBits(2));
}

void WorldPackets::Movement::MoveTimeSkipped::Read()
{
    _worldPacket >> MoverGUID;
    _worldPacket >> TimeSkipped;
}

void WorldPackets::Movement::SummonResponse::Read()
{
    _worldPacket >> SummonerGUID;
    Accept = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Movement::ControlUpdate::Write()
{
    _worldPacket << Guid;
    _worldPacket.WriteBit(On);
    // _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Movement::MoveSplineDone::Read()
{
    _worldPacket >> movementInfo;
    _worldPacket >> SplineID;
}

WorldPacket const* WorldPackets::Movement::MoveUpdateRemoveMovementForce::Write()
{
    _worldPacket << *movementInfo;
    _worldPacket << TriggerGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveUpdateApplyMovementForce::Write()
{
    _worldPacket << *movementInfo;
    _worldPacket << MovementForceData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveSetCompoundState::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << static_cast<int32>(StateChanges.size());
    for (auto& v : StateChanges)
    {
        _worldPacket << v.MessageID;
        _worldPacket << v.SequenceIndex;

        _worldPacket.WriteBit(v.Speed.is_initialized());
        _worldPacket.WriteBit(v.KnockBack.is_initialized());
        _worldPacket.WriteBit(v.VehicleRecID.is_initialized());
        _worldPacket.WriteBit(v.CollisionHeight.is_initialized());
        _worldPacket.WriteBit(v.MovementForceData.is_initialized());
        _worldPacket.WriteBit(v.Unknown.is_initialized());
        _worldPacket.FlushBits();

        if (v.Speed)
            _worldPacket << *v.Speed;

        if (v.KnockBack)
        {
            _worldPacket << v.KnockBack->HorzSpeed;
            _worldPacket << v.KnockBack->Direction;
            _worldPacket << v.KnockBack->InitVertSpeed;
        }

        if (v.VehicleRecID)
            _worldPacket << *v.VehicleRecID;

        if (v.CollisionHeight)
        {
            _worldPacket << v.CollisionHeight->Height;
            _worldPacket << v.CollisionHeight->Scale;
            _worldPacket.WriteBits(v.CollisionHeight->Reason, 2);
            _worldPacket.FlushBits();
        }

        if (v.Unknown)
            _worldPacket << *v.Unknown;

        if (v.MovementForceData)
            _worldPacket << *v.MovementForceData;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::FlightSplineSync::Write()
{
    _worldPacket << Guid;
    _worldPacket << SplineDist;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::AdjustSplineDuration::Write()
{
    _worldPacket << Unit;
    _worldPacket << Scale;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Movement::MoveKnockBackSpeeds const& speeds)
{
    data << float(speeds.HorzSpeed);
    data << float(speeds.VertSpeed);

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Movement::MoveKnockBackSpeeds& speeds)
{
    data >> speeds.HorzSpeed;
    data >> speeds.VertSpeed;

    return data;
}

WorldPacket const* WorldPackets::Movement::MoveKnockBack::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << SequenceIndex;
    _worldPacket << Direction;
    _worldPacket << Speeds;

    return &_worldPacket;
}

void WorldPackets::Movement::MoveKnockBackAck::Read()
{
    _worldPacket >> Ack;
    if (_worldPacket.ReadBit())
    {
        Speeds = boost::in_place();
        _worldPacket >> *Speeds;
    }
}

WorldPacket const* WorldPackets::Movement::MoveApplyMovementForce::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << SequenceIndex;
    _worldPacket << Force;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveRemoveMovementForce::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << SequenceIndex;
    _worldPacket << TriggerGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::TimeSyncRequest::Write()
{
    _worldPacket << SequenceIndex;

    return &_worldPacket;
}

void WorldPackets::Movement::TimeSyncResponse::Read()
{
    _worldPacket >> SequenceIndex;
    _worldPacket >> ClientTime;
}

void WorldPackets::Movement::DiscardedTimeSyncAcks::Read()
{
    _worldPacket >> MaxSequenceIndex;
}

void WorldPackets::Movement::TimeSyncResponseDropped::Read()
{
    _worldPacket >> SequenceIndexFirst;
    _worldPacket >> SequenceIndexLast;
}

void WorldPackets::Movement::TimeSyncResponseFailed::Read()
{
    _worldPacket >> SequenceIndex;
}

WorldPacket const* WorldPackets::Movement::SuspendToken::Write()
{
    _worldPacket << uint32(SequenceIndex);
    _worldPacket.WriteBits(Reason, 2);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Movement::SuspendTokenResponse::Read()
{
    _worldPacket >> SequenceIndex;
}

WorldPacket const* WorldPackets::Movement::ResumeToken::Write()
{
    _worldPacket << uint32(SequenceIndex);
    _worldPacket.WriteBits(Reason, 2);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Movement::MoveSkipTime::Write()
{
    _worldPacket << MoverGUID;
    _worldPacket << SkippedTime;

    return &_worldPacket;
}
