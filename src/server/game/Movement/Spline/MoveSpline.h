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

#ifndef TRINITYSERVER_MOVEPLINE_H
#define TRINITYSERVER_MOVEPLINE_H

#include "Spline.h"
#include "MoveSplineInitArgs.h"
#include "MoveSplineInit.h"

namespace WorldPackets
{
    namespace Movement
    {
        class CommonMovement;
        class MonsterMove;
    }
}

namespace Movement
{
    uint32 SelectSpeedType(uint32 moveFlags);

    struct Location : G3D::Vector3
    {
        Location() : orientation(0.0f) { }
        Location(float x, float y, float z, float o) : G3D::Vector3(x, y, z), orientation(o) { }
        Location(G3D::Vector3 const& v) : G3D::Vector3(v), orientation(0.0f) { }
        Location(G3D::Vector3 const& v, float o) : G3D::Vector3(v), orientation(o) { }
        Location(Position pos) : G3D::Vector3(pos.m_positionX, pos.m_positionY, pos.m_positionZ), orientation(pos.m_orientation) { }

        float orientation;
    };

    // MoveSpline represents smooth catmullrom or linear curve and point that moves belong it
    // curve can be cyclic - in this case movement will be cyclic
    // point can have vertical acceleration motion componemt(used in fall, parabolic movement)
    class MoveSpline
    {
        friend class WorldPackets::Movement::CommonMovement;
        friend class WorldPackets::Movement::MonsterMove;
        friend class Movement::MoveSplineInit;

    public:
        typedef Spline<int32> MySpline;
        enum UpdateResult
        {
            Result_None         = 0x01,
            Result_Arrived      = 0x02,
            Result_NextCycle    = 0x04,
            Result_NextSegment  = 0x08,
        };
        friend class PacketBuilder;

        MySpline GetSpline() const { return spline; }

        int32 GetTimePassed() const { return time_passed; }

    protected:
        MySpline        spline;

        FacingInfo      facing;

        uint32          m_Id;

        MoveSplineFlag  splineflags;

        int32           time_passed;
        // currently duration mods are unused, but its _currently_
        float           durationModifier;
        float           nextDurationModifier;
        float           JumpGravity;
        float           initialOrientation;
        int32           SpecialTime;
        int32           point_Idx;
        int32           point_Idx_offset;
        Optional<SpellEffectExtraData> spell_effect_extra;

        void init_spline(const MoveSplineInitArgs& args);

        const MySpline::ControlArray& getPath() const { return spline.getPoints(); }
        void computeParabolicElevation(float& el, float u) const;
        void computeFallElevation(float& el) const;

        UpdateResult _updateState(int32& ms_time_diff);
        int32 next_timestamp() const { return spline.length(point_Idx+1); }
        int32 segment_time_elapsed() const { return next_timestamp()-time_passed; }
        int32 timeElapsed() const { return Duration() - time_passed; }
        int32 timePassed() const { return time_passed; }

        bool walk;
        float _velocity;

    public:
        const MySpline& _Spline() const { return spline; }
        int32 _currentSplineIdx() const { return point_Idx; }
        void _Finalize();
        void _Interrupt() { splineflags.done = true;}

        int32 Duration() const
        {
            if(!spline.empty()) return spline.length();
            return 0;
        }
        void Initialize(const MoveSplineInitArgs&);
        bool Initialized() const { return !spline.empty(); }

        void UpdateVelocity(Unit* owner);

        explicit MoveSpline();

        template<class UpdateHandler>
        void updateState(int32 difftime, UpdateHandler& handler)
        {
            ASSERT(Initialized());

            do
                handler(_updateState(difftime));
            while(difftime > 0);
        }

        void updateState(int32 difftime)
        {
            ASSERT(Initialized());

            do _updateState(difftime);
            while(difftime > 0);
        }

        Location ComputePosition() const;
        float ComputeProgress() const;

        uint32 GetId() const { return m_Id; }
        bool Finalized() const { return splineflags.done; }
        bool isCyclic() const { return splineflags.cyclic; }
        bool isTransportExit() const { return splineflags.hasFlag(MoveSplineFlag::TransportExit); }
        bool isTransportEnter() const { return splineflags.hasFlag(MoveSplineFlag::TransportEnter); }
        G3D::Vector3 FinalDestination() const { return Initialized() ? spline.getPoint(spline.last()) : G3D::Vector3(); }
        G3D::Vector3 CurrentDestination() const { return Initialized() ? spline.getPoint(point_Idx+1) : G3D::Vector3(); }
        int32 currentPathIdx() const;

        bool onTransport;
        bool splineIsFacingOnly;
        ObjectGuid TransportGUID;
        uint8 VehicleSeat           = 255;

        std::string ToString() const;

    };
}
#endif // TRINITYSERVER_MOVEPLINE_H
