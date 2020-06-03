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

#ifndef TRINITYSERVER_MOVESPLINEINIT_ARGS_H
#define TRINITYSERVER_MOVESPLINEINIT_ARGS_H

#include "MoveSplineFlag.h"
#include "ObjectGuid.h"

namespace Movement
{
    typedef std::vector<G3D::Vector3> PointsArray;

    struct FacingInfo
    {
        struct
        {
            float x, y, z;
        } f;

        ObjectGuid target;
        MonsterMoveType type;
        float angle;

        FacingInfo(float o) : type(MONSTER_MOVE_NORMAL), angle(o) { f.x = f.y = f.z = 0.0f; }
        FacingInfo(ObjectGuid t) : target(t), type(MONSTER_MOVE_NORMAL), angle(0) { f.x = f.y = f.z = 0.0f; }
        FacingInfo(): type(MONSTER_MOVE_NORMAL), angle(0) { f.x = f.y = f.z = 0.0f; }
    };

    struct SpellEffectExtraData
    {
        ObjectGuid Target;
        uint32 SpellVisualId = 0;
        uint32 ProgressCurveId = 0;
        uint32 ParabolicCurveId = 0;
    };

    struct MoveSplineInitArgs
    {
        MoveSplineInitArgs(size_t path_capacity = 16);

        Optional<SpellEffectExtraData> spellEffectExtra;
        PointsArray path;
        FacingInfo facing;
        MoveSplineFlag flags;
        int32 path_Idx_offset;
        float velocity;
        float parabolic_amplitude;
        float time_perc;
        uint32 splineId;
        float initialOrientation;
        bool walk;
        bool HasVelocity;
        bool TransformForTransport;

        /** Returns true to show that the arguments were configured correctly and MoveSpline initialization will succeed. */
        bool Validate() const;
    private:
        bool _checkPathLengths() const;
    };
}

#endif // TRINITYSERVER_MOVESPLINEINIT_ARGS_H
