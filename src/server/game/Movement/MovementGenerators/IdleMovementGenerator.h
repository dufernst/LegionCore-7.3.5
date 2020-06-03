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

#ifndef TRINITY_IDLEMOVEMENTGENERATOR_H
#define TRINITY_IDLEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class IdleMovementGenerator : public MovementGenerator
{
    public:

        void Initialize(Unit &) override;
        void Finalize(Unit &) override{  }
        void Reset(Unit &) override;
        bool Update(Unit &, const uint32&) override { return true; }
        MovementGeneratorType GetMovementGeneratorType() override { return IDLE_MOTION_TYPE; }
};

extern IdleMovementGenerator si_idleMovement;

class RotateMovementGenerator : public MovementGenerator
{
    public:
        explicit RotateMovementGenerator(uint32 time, RotateDirection direction, bool _repeat = false) : m_duration(time), m_maxDuration(time), m_direction(direction), repeat(_repeat) {}

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Reset(Unit& owner) override { Initialize(owner); }
        bool Update(Unit& owner, const uint32& time_diff) override;
        MovementGeneratorType GetMovementGeneratorType() override { return ROTATE_MOTION_TYPE; }

    private:
        uint32 m_duration, m_maxDuration;
        RotateDirection m_direction;
        bool repeat;
};

class DistractMovementGenerator : public MovementGenerator
{
    public:
        explicit DistractMovementGenerator(uint32 timer) : m_timer(timer) {}

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Reset(Unit& owner) override { Initialize(owner); }
        bool Update(Unit& owner, const uint32& time_diff) override;
        MovementGeneratorType GetMovementGeneratorType() override { return DISTRACT_MOTION_TYPE; }

    private:
        uint32 m_timer;
};

class AssistanceDistractMovementGenerator : public DistractMovementGenerator
{
    public:
        AssistanceDistractMovementGenerator(uint32 timer) :
            DistractMovementGenerator(timer) {}

        MovementGeneratorType GetMovementGeneratorType() override { return ASSISTANCE_DISTRACT_MOTION_TYPE; }
        void Finalize(Unit& unit) override;
};

#endif

