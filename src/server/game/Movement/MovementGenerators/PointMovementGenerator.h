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

#ifndef TRINITY_POINTMOVEMENTGENERATOR_H
#define TRINITY_POINTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "FollowerReference.h"
#include "PathGenerator.h"
#include "Spell.h"

template<class T>
class PointMovementGenerator : public MovementGeneratorMedium< T, PointMovementGenerator<T> >
{
    public:
        PointMovementGenerator(uint32 _id, float _x, float _y, float _z, bool _generatePath, float _speed = 0.0f, float _angle = -1000.0f) :
            id(_id), i_x(_x), i_y(_y), i_z(_z), speed(_speed), angle(_angle), i_recalculateSpeed(false), m_generatePath(_generatePath)
    {
    }

        void DoInitialize(T &);
        void DoFinalize(T &);
        void DoReset(T &);
        bool DoUpdate(T &, const uint32 &);

        void MovementInform(T &);

        void unitSpeedChanged() override { i_recalculateSpeed = true; }

        MovementGeneratorType GetMovementGeneratorType() override { return POINT_MOTION_TYPE; }

        bool GetDestination(float& x, float& y, float& z) const { x=i_x; y=i_y; z=i_z; return true; }
    private:
        uint32 id;
        float i_x, i_y, i_z;
        float speed;
        float angle;
        bool i_recalculateSpeed;
        bool i_restartMove = false;
        bool m_generatePath;
};

class AssistanceMovementGenerator : public PointMovementGenerator<Creature>
{
    public:
        AssistanceMovementGenerator(float _x, float _y, float _z) :
            PointMovementGenerator<Creature>(0, _x, _y, _z, true) {}

        MovementGeneratorType GetMovementGeneratorType() override { return ASSISTANCE_MOTION_TYPE; }
        void DoFinalize(Unit &);
};

// Does almost nothing - just doesn't allows previous movegen interrupt current effect.
class EffectMovementGenerator : public MovementGenerator
{
    public:
        explicit EffectMovementGenerator(uint32 Id, float _x, float _y, float _z, DelayCastEvent *e = nullptr) : m_Id(Id),
        i_x(_x), i_y(_y), i_z(_z), m_event(e)        {}

        void Initialize(Unit &) override {}
        void Finalize(Unit &unit) override;
        void Reset(Unit &) override {}
        bool Update(Unit &u, const uint32&) override;
        MovementGeneratorType GetMovementGeneratorType() override { return EFFECT_MOTION_TYPE; }
    private:
        uint32 m_Id;
        float i_x, i_y, i_z;
        DelayCastEvent *m_event;
};

// Charge Movement Generator
class ChargeMovementGenerator : public MovementGenerator
{
    public:
        explicit ChargeMovementGenerator(uint32 Id, float _x, float _y, float _z, float _speed = 0.0f, uint32 _triggerspellId = 0, PathGenerator* _path = nullptr) :
        m_Id(Id), triggerspellId(_triggerspellId), i_x(_x), i_y(_y), i_z(_z), speed(_speed), i_recalculateSpeed(false), i_recalculateTravel(false), i_targetReached(false), i_path(_path)
        { }
        ~ChargeMovementGenerator() { delete i_path; }

        void Initialize(Unit &) override;
        void Finalize(Unit &unit) override;
        void Reset(Unit &) override {}
        bool Update(Unit &u, const uint32&) override;
        MovementGeneratorType GetMovementGeneratorType() override { return POINT_MOTION_TYPE; }
        bool GetDestination(float& x, float& y, float& z) const { x=i_x; y=i_y; z=i_z; return true; }
        bool IsReachable() const { return (i_path) ? (i_path->GetPathType() & PATHFIND_NORMAL) : true; }
    private:
        uint32 m_Id;
        uint32 triggerspellId;
        float i_x, i_y, i_z;
        float speed;
        bool i_recalculateSpeed;
        bool i_recalculateTravel : 1;
        bool i_targetReached : 1;
        PathGenerator* i_path;
        void _setTargetLocation(Unit &);
};

#endif

