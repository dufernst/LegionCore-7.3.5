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

#ifndef ARENA_H
#define ARENA_H

#include "Battleground.h"
#include "Timer.h"
#include "LogsSystem.h"

class Arena : public Battleground
{
protected:
    Arena();
    
    void PostUpdateImpl(uint32 diff) override;
    void _ProcessJoin(uint32 diff) override;

    void AddPlayer(Player* player) override;
    void RemovePlayer(Player* /*player*/, ObjectGuid /*guid*/, uint32 /*team*/) override;

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void UpdateArenaWorldState();

    void HandleKillPlayer(Player* player, Player* killer) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    void SendOpponentSpecialization(uint32 team);

private:
    void RemovePlayerAtLeave(ObjectGuid guid, bool transport, bool sendPacket) override;
    void EndBattleground(uint32 winner) override;
    void CheckWinConditions() override;
    void ApplyDampeningIfNeeded();
    IntervalTimer _dampeningTimer;
    IntervalTimer _winConditionCheckTimer;
    LogsSystem::MainData _logData;
};

#endif // ARENA_H
