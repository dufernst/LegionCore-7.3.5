/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Unit.h"
#include "Transport.h"
#include "Vehicle.h"
#include "MovementPackets.h"

namespace Movement
{
    int32 MoveSplineInit::Launch()
    {
        MoveSpline& move_spline = *unit.movespline;
        bool transport = !unit.GetTransGUID().IsEmpty();

        Location real_position(unit.GetPositionX(), unit.GetPositionY(), unit.GetPositionZ(), unit.GetOrientation());
        // Elevators also use MOVEMENTFLAG_ONTRANSPORT but we do not keep track of their position changes
        //if (unit.GetTransGUID())
        if (unit.m_movementInfo.transport.Guid) //for vehicle too
            real_position = unit.GetTransOffset();

        // there is a big chance that current position is unknown if current state is not finalized, need compute it
        // this also allows calculate spline position and update map position in much greater intervals
        // Don't compute for transport movement if the unit is in a motion between two transports
        if (!move_spline.Finalized() && move_spline.onTransport == transport)
            real_position = move_spline.ComputePosition();

        // should i do the things that user should do? - no.
        if (args.path.empty())
            return 0;

        // correct first vertex
        args.path[0] = real_position;
        args.initialOrientation = real_position.orientation;
        move_spline.onTransport = transport;

        uint32 moveFlags = unit.m_movementInfo.GetMovementFlags();
        if (args.walk)
            moveFlags |= MOVEMENTFLAG_WALKING;
        else
            moveFlags &= ~MOVEMENTFLAG_WALKING;

        if (!args.flags.backward)
            moveFlags = (moveFlags & ~MOVEMENTFLAG_BACKWARD) | MOVEMENTFLAG_FORWARD;
        else
            moveFlags = (moveFlags & ~MOVEMENTFLAG_FORWARD) | MOVEMENTFLAG_BACKWARD;

        if (!args.HasVelocity)
            args.velocity = unit.GetSpeed(UnitMoveType(SelectSpeedType(moveFlags)));

        if (!args.Validate())
            return 0;

        if (moveFlags & MOVEMENTFLAG_ROOT)
            moveFlags &= ~MOVEMENTFLAG_MASK_MOVING;

        unit.m_movementInfo.SetMovementFlags(moveFlags);
        move_spline.Initialize(args);

        WorldPackets::Movement::MonsterMove packet;
        packet.MoverGUID = unit.GetGUID();
        packet.Pos = Position(real_position.x, real_position.y, real_position.z, real_position.orientation);
        packet.InitializeSplineData(move_spline);
        if (transport)
        {
            packet.SplineData.Move.TransportGUID = unit.GetTransGUID();
            packet.SplineData.Move.VehicleSeat = unit.GetTransSeat();
        }
        unit.SendMessageToSet(packet.Write(), true);

        //blizz-hack.
        //on retail if creature has loop emote and start run we remove emote, else client crash at getting object create.
        //ToDo: more reseach.
        if(unit.GetUInt32Value(UNIT_FIELD_EMOTE_STATE))
            unit.SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);

        return move_spline.Duration();
    }

    void MoveSplineInit::Stop(bool force /*= false*/, bool stopAttack /*= false*/)
    {
        MoveSpline& move_spline = *unit.movespline;

        bool transport = !unit.GetTransGUID().IsEmpty();
        Location loc;

        if (stopAttack && !transport)
        {
            Position const* pos = &unit;
            loc.x = pos->GetPositionX();
            loc.y = pos->GetPositionY();
            loc.z = pos->GetPositionZ();
            loc.orientation = unit.GetOrientation();

            args.flags = MoveSplineFlag::Done;
            move_spline.Initialize(args);

            WorldPackets::Movement::MonsterMove packet;
            packet.MoverGUID = unit.GetGUID();
            packet.Pos = Position(loc.x, loc.y, loc.z, loc.orientation);
            packet.SplineData.ID = move_spline.GetId();
            packet.SplineData.StopDistanceTolerance = 1;
            packet.SplineData.Move.Face = 3;
            packet.SplineData.Move.FaceDirection = unit.GetOrientation();
            unit.SendMessageToSet(packet.Write(), true);
            return;
        }

        if (force)
        {
            args.flags = MoveSplineFlag::Done;
            unit.m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FORWARD);
            move_spline.Initialize(args);
            return;
        }

        // No need to stop if we are not moving
        if (move_spline.Finalized())
            return;

        if (move_spline.onTransport == transport)
            loc = move_spline.ComputePosition();
        else
        {
            Position const* pos;
            if (!transport)
                pos = &unit;
            else
                pos = &unit.m_movementInfo.transport.Pos;

            loc.x = pos->GetPositionX();
            loc.y = pos->GetPositionY();
            loc.z = pos->GetPositionZ();
            loc.orientation = unit.GetOrientation();
        }

        args.flags = MoveSplineFlag::Done;
        unit.m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FORWARD);
        move_spline.onTransport = transport;
        move_spline.Initialize(args);

        WorldPackets::Movement::MonsterMove packet;
        packet.MoverGUID = unit.GetGUID();
        packet.Pos = Position(loc.x, loc.y, loc.z, loc.orientation);
        packet.SplineData.ID = move_spline.GetId();
        packet.SplineData.StopDistanceTolerance = 2;

        if (transport)
        {
            packet.SplineData.Move.TransportGUID = unit.GetTransGUID();
            packet.SplineData.Move.VehicleSeat = unit.GetTransSeat();
        }

        unit.SendMessageToSet(packet.Write(), true);
    }

    MoveSplineInit::MoveSplineInit(Unit& m) : unit(m)
    {
        args.splineId = splineIdGen.NewId();
        // Elevators also use MOVEMENTFLAG_ONTRANSPORT but we do not keep track of their position changes
        args.TransformForTransport = !unit.GetTransGUID().IsEmpty();
        // mix existing state into new
        args.flags.canSwim = unit.CanSwim();
        args.walk = unit.HasUnitMovementFlag(MOVEMENTFLAG_WALKING);
        args.flags.flying = unit.HasUnitMovementFlag(MovementFlags(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY));
        args.flags.smoothGroundPath = true; // enabled by default, CatmullRom mode or client config "pathSmoothing" will disable this
        args.flags.steering = unit.HasFlag(UNIT_FIELD_NPC_FLAGS2, UNIT_NPC_FLAG2_STEERING);
    }

    void MoveSplineInit::SetFacing(Unit const* target)
    {
        args.facing.type = MONSTER_MOVE_FACING_TARGET;
        args.facing.angle = unit.GetAngle(target);
        args.facing.target = target->GetGUID();
    }

    void MoveSplineInit::SetFacing(float angle)
    {
        args.facing.type = MONSTER_MOVE_FACING_ANGLE;
        if (args.TransformForTransport)
        {
            if (Unit* vehicle = unit.GetVehicleBase())
                angle -= vehicle->GetOrientation();
            else if (Transport* transport = unit.GetTransport())
                angle -= transport->GetOrientation();
        }

        args.facing.angle = G3D::wrap(angle, 0.f, static_cast<float>(G3D::twoPi()));
    }

    void MoveSplineInit::MoveTo(G3D::Vector3 const& dest, bool generatePath, bool forceDestination)
    {
        if (generatePath)
        {
            PathGenerator path(&unit);
            path.CalculatePath(dest.x, dest.y, dest.z, forceDestination);
            MovebyPath(path.GetPath());
        }
        else
        {
            args.path_Idx_offset = 0;
            args.path.resize(2);
            TransportPathTransform transform(unit, args.TransformForTransport);
            args.path[1] = transform(dest);
        }
    }

    void MoveSplineInit::SetFall()
    {
        args.flags.EnableFalling();
        args.flags.fallingSlow = unit.HasUnitMovementFlag(MOVEMENTFLAG_FEATHER_FALL);
    }

    PointsArray& MoveSplineInit::Path()
    {
        return args.path;
    }

    void MoveSplineInit::PushPath(Position pos)
    {
        return args.path.push_back(G3D::Vector3(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ()));
    }

    TransportPathTransform::TransportPathTransform(Unit& owner, bool transformForTransport): _owner(owner), _transformForTransport(transformForTransport)
    {
    }

    G3D::Vector3 TransportPathTransform::operator()(G3D::Vector3 input)
    {
        if (_transformForTransport)
        {
            if (TransportBase* transport = _owner.GetVehicle())
                transport->CalculatePassengerOffset(input.x, input.y, input.z, nullptr);
        }
        return input;
    }
}
