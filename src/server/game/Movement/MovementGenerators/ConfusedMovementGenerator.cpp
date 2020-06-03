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

#include "Unit.h"
#include "Creature.h"
#include "MapManager.h"
#include "ConfusedMovementGenerator.h"
#include "VMapFactory.h"
#include "MoveSplineInit.h"
#include "Player.h"
#include "MoveSpline.h"

#define WALK_SPEED_YARDS_PER_SECOND 2.45f

template<class T>
void ConfusedMovementGenerator<T>::DoInitialize(T &unit)
{
    unit.GetPosition(i_x, i_y, i_z, unit.GetTransport());
    unit.StopMoving();
    unit.SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
    unit.AddUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
    i_duration = unit.GetTimeForSpline();
    unit.SetSpeed(MOVE_WALK, 1.f, true);
    speed = WALK_SPEED_YARDS_PER_SECOND;
    moving_to_start = false;
}

template<class T>
void ConfusedMovementGenerator<T>::DoReset(T &unit)
{
    i_nextMoveTime.Reset(0);
    unit.AddUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
    unit.StopMoving();
}

template<class T>
bool ConfusedMovementGenerator<T>::DoUpdate(T &unit, const uint32 &diff)
{
    if (unit.HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
        return true;

    i_nextMoveTime.Update(diff);
    if(i_duration >= diff)
        i_duration -= diff;

    if (!moving_to_start)
    {
        float max_accuracyMiliYard = 150 * speed;
        float min_accuracyMiliYard = -20 * speed;
        PathGenerator path(&unit);

        if (path.CalculatePath(i_x, i_y, i_z))
        {
            float len = 0;
            for (size_t i = 1; i < path.GetPath().size(); ++i)
            {
                G3D::Vector3 node = path.GetPath()[i];
                G3D::Vector3 prev = path.GetPath()[i-1];
                float xd = node.x - prev.x;
                float yd = node.y - prev.y;
                float zd = node.z - prev.z;
                len += sqrtf(xd*xd + yd*yd + zd*zd);
            }

            float timeSpent = float(i_duration) - len * 1000.f / speed;

            if (timeSpent <= max_accuracyMiliYard && timeSpent >= min_accuracyMiliYard)
            {
                moving_to_start = true;
                unit.AddUnitState(UNIT_STATE_CONFUSED_MOVE);
                unit.UpdateAllowedPositionZ(i_x, i_y, i_z);

                Movement::MoveSplineInit init(unit);
                init.MovebyPath(path.GetPath());
                init.SetWalk(true);
                init.Launch();
                return true;
            }
        }
    }
    else
        return true;

    // waiting for next move
    if (i_nextMoveTime.Passed() && unit.movespline->Finalized())
    {
        {
            // start moving
            unit.AddUnitState(UNIT_STATE_CONFUSED_MOVE);

            float const wander_distance = 5;
            float x, y, z = 0.0f;
            unit.GetPosition(x, y, z, unit.GetTransport());
            x += (wander_distance * static_cast<float>(rand_norm()) - wander_distance/2);
            y += (wander_distance * static_cast<float>(rand_norm()) - wander_distance/2);

            Trinity::NormalizeMapCoord(x);
            Trinity::NormalizeMapCoord(y);

            unit.UpdateAllowedPositionZ(x, y, z);

            if (z <= INVALID_HEIGHT)
                i_z = unit.GetHeight(x, y, MAX_HEIGHT) + 2.0f;

            PathGenerator path(&unit);
            path.SetPathLengthLimit(30.0f);
            path.SetUseStraightPath(false);

            if (!unit.IsWithinLOS(x, y, z) || !path.CalculatePath(x, y, z) || path.GetPathType() & PATHFIND_NOPATH)
            {
                i_nextMoveTime.Reset(urand(200, 500));
                return true;
            }

            Movement::MoveSplineInit init(unit);
            if (unit.GetTransport())
                init.DisableTransportPathTransformations();
            init.MovebyPath(path.GetPath());
            init.SetWalk(true);
            init.Launch();
        }
    }

    return true;
}

template<>
void ConfusedMovementGenerator<Player>::DoFinalize(Player &unit)
{
    unit.RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
    unit.ClearUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
    unit.StopMoving();
}

template<>
void ConfusedMovementGenerator<Creature>::DoFinalize(Creature &unit)
{
    unit.RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
    unit.ClearUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
    if (unit.getVictim())
        unit.SetTarget(unit.getVictim()->GetGUID());
}

template void ConfusedMovementGenerator<Player>::DoInitialize(Player &player);
template void ConfusedMovementGenerator<Creature>::DoInitialize(Creature &creature);
template void ConfusedMovementGenerator<Player>::DoReset(Player &player);
template void ConfusedMovementGenerator<Creature>::DoReset(Creature &creature);
template bool ConfusedMovementGenerator<Player>::DoUpdate(Player &player, const uint32 &diff);
template bool ConfusedMovementGenerator<Creature>::DoUpdate(Creature &creature, const uint32 &diff);

