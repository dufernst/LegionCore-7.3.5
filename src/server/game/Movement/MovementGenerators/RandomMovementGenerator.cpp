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

#include "Creature.h"
#include "MapManager.h"
#include "RandomMovementGenerator.h"
#include "ObjectAccessor.h"
#include "Map.h"
#include "Util.h"
#include "CreatureGroups.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"

#define RUNNING_CHANCE_RANDOMMV 5                                  //will be "1 / RUNNING_CHANCE_RANDOMMV"

template<>
void RandomMovementGenerator<Creature>::_setRandomLocation(Creature& creature)
{
    float respX, respY, respZ, respO /*, travelDistZ*/;
    creature.GetHomePosition(respX, respY, respZ, respO);

    // For 2D/3D system selection
    bool is_air_ok = creature.CanFly();

    const float angle = float(rand_norm()) * static_cast<float>(M_PI*2.0f);
    const float range = float(frand(rand_norm(), frand(0.8f, 1.0f)) * wander_distance * (is_air_ok ? 4.0f : 2.0f));

    const float distanceX = range * cos(angle);
    const float distanceY = range * sin(angle);

    float destX = respX + distanceX;
    float destY = respY + distanceY;
    float destZ = creature.GetPositionZ();


    if (creature.GetTransport())
    {
        Position pos;
        creature.GetFirstCollisionPosition(pos, range, angle);
        destX = pos.m_positionX;
        destY = pos.m_positionY;
        destZ = pos.m_positionZ;
    }

    creature.UpdateAllowedPositionZ(destX, destY, destZ);

    // prevent invalid coordinates generation
    Trinity::NormalizeMapCoord(destX);
    Trinity::NormalizeMapCoord(destY);

    /*travelDistZ = distanceX*distanceX + distanceY*distanceY;

    if (is_air_ok)                                          // 3D system above ground and above water (flying mode)
    {
        // Limit height change
        const float distanceZ = float(rand_norm()) * sqrtf(travelDistZ)/2.0f;
        destZ = respZ + distanceZ;
        float levelZ = creature->GetWaterOrGroundLevel(destX, destY, destZ-2.0f);

        // Problem here, we must fly above the ground and water, not under. Let's try on next tick
        if (levelZ >= destZ)
            return;
    }
    //else if (is_water_ok)                                 // 3D system under water and above ground (swimming mode)
    else                                                    // 2D only
    {
        // 10.0 is the max that vmap high can check (MAX_CAN_FALL_DISTANCE)
        travelDistZ = travelDistZ >= 100.0f ? 10.0f : sqrtf(travelDistZ);

        // The fastest way to get an accurate result 90% of the time.
        // Better result can be obtained like 99% accuracy with a ray light, but the cost is too high and the code is too long.
        destZ = creature.GetHeight(destX, destY, respZ+travelDistZ-2.0f, false);

        if (fabs(destZ - respZ) > travelDistZ)              // Map check
        {
            // Vmap Horizontal or above
            destZ = creature.GetHeight(destX, destY, respZ - 2.0f, true);

            if (fabs(destZ - respZ) > travelDistZ)
            {
                // Vmap Higher
                destZ = creature.GetHeight(destX, destY, respZ+travelDistZ-2.0f, true);

                // let's forget this bad coords where a z cannot be find and retry at next tick
                if (fabs(destZ - respZ) > travelDistZ)
                    return;
            }
        }
    }*/

    PathGenerator path(&creature);

    if (!path.CalculatePath(destX, destY, destZ) || path.GetPathType() & PATHFIND_NOPATH)
    {
        i_nextMoveTime.Reset(urand(500, 1500));
        return;
    }

    creature.AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);

    Movement::MoveSplineInit init(creature);
    if (creature.GetTransport())
        init.DisableTransportPathTransformations();
    init.MovebyPath(path.GetPath());
    init.SetWalk((irand(0, RUNNING_CHANCE_RANDOMMV) > 0) ? true : false);
    int32 traveltime = init.Launch();

    if (is_air_ok)
        i_nextMoveTime.Reset(0);
    else
        i_nextMoveTime.Reset(traveltime + urand(500, 2000));

    //Call for creature group update
    if (creature.GetFormation() && creature.GetFormation()->getLeader() == &creature)
        creature.GetFormation()->LeaderMoveTo(destX, destY, destZ);
}

template<>
void RandomMovementGenerator<Creature>::DoInitialize(Creature &creature)
{
    if (!creature.isAlive() || creature.isDead(true) || !creature.CanFreeMove())
        return;

    if (!wander_distance)
        wander_distance = creature.GetRespawnRadius();

    creature.AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
    i_nextMoveTime.Reset(urand(1000, 5000));
}

template<>
void RandomMovementGenerator<Creature>::DoReset(Creature &creature)
{
    Initialize(creature);
}

template<>
void RandomMovementGenerator<Creature>::DoFinalize(Creature &creature)
{
    creature.ClearUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
    creature.SetWalk(false);
}

template<>
bool RandomMovementGenerator<Creature>::DoUpdate(Creature &creature, const uint32 diff)
{
    if (creature.HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    {
        i_nextMoveTime.Reset(0);  // Expire the timer
        creature.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    if (creature.movespline->Finalized())
    {
        i_nextMoveTime.Update(diff);
        if (i_nextMoveTime.Passed())
        {
            _setRandomLocation(creature);
            if (roll_chance_i(5)) // Random stop move for some time
            {
                creature.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
                i_nextMoveTime.Reset(urand(1000, 5000));
            }
        }
    }
    return true;
}

template<>
bool RandomMovementGenerator<Creature>::GetResetPos(Creature &creature, float& x, float& y, float& z)
{
    float radius;
    creature.GetRespawnPosition(x, y, z, nullptr, &radius);

    // use current if in range
    if (creature.IsWithinDist2d(x,y,radius))
        creature.GetPosition(x,y,z);

    return true;
}
