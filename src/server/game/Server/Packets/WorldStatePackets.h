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

#ifndef WorldStatePackets_h__
#define WorldStatePackets_h__

#include "Packet.h"

namespace WorldPackets
{
    namespace WorldState
    {
        class InitWorldStates final : public ServerPacket
        {
        public:
            InitWorldStates() : ServerPacket(SMSG_INIT_WORLD_STATES, 12) { }
            
            WorldPacket const* Write() override;

            struct WorldStateInfo
            {
                WorldStateInfo(WorldStates variableID, int32 value) : VariableID(variableID), Value(value) { }
                WorldStateInfo(uint32 variableID, int32 value) : VariableID(variableID), Value(value) { }

                uint32 VariableID;
                int32 Value;
            };
            
            std::vector<WorldStateInfo> Worldstates;
            uint32 AreaID = 0;
            uint32 SubareaID = 0;
            uint32 MapID = 0;
        };

        class UpdateWorldState final : public ServerPacket
        {
        public:
            UpdateWorldState() : ServerPacket(SMSG_UPDATE_WORLD_STATE, 9) { }

            WorldPacket const* Write() override;

            int32 Value = 0;
            WorldStates VariableID = WS_NONE;
            bool Hidden = false;
        };

        struct ElaspedTimer
        {
            ElaspedTimer() { }
            ElaspedTimer(uint32 timerID, time_t currentDuration) : TimerID(timerID), CurrentDuration(currentDuration) { }

            uint32 TimerID = 0;
            time_t CurrentDuration = time_t(0);
        };
        
        class StartElapsedTimer final : public ServerPacket
        {
        public:
            StartElapsedTimer() : ServerPacket(SMSG_START_ELAPSED_TIMER, 8) { }

            WorldPacket const* Write() override;

            ElaspedTimer Timer;
        };

        class StartElapsedTimers final : public ServerPacket
        {
        public:
            StartElapsedTimers() : ServerPacket(SMSG_START_ELAPSED_TIMERS, 4) { }

            WorldPacket const* Write() override;

            std::vector<ElaspedTimer> Timers;
        };
    }
}

#endif // WorldStatePackets_h__
