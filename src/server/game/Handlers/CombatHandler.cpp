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

#include "Common.h"
#include "WorldSession.h"
#include "ObjectAccessor.h"
#include "Vehicle.h"
#include "CombatPackets.h"

void WorldSession::HandleAttackSwing(WorldPackets::Combat::AttackSwing& packet)
{
    Unit* pEnemy = ObjectAccessor::GetUnit(*_player, packet.Victim);

    if (!pEnemy)
    {
        // stop attack state at client
        GetPlayer()->SendMeleeAttackStop(nullptr);
        return;
    }

    if (!_player->IsValidAttackTarget(pEnemy))
    {
        // stop attack state at client
        GetPlayer()->SendMeleeAttackStop(pEnemy);
        return;
    }

    //! Client explicitly checks the following before sending CMSG_ATTACK_SWING packet,
    //! so we'll place the same check here. Note that it might be possible to reuse this snippet
    //! in other places as well.
    if (Vehicle* vehicle = _player->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(_player);
        if (!seat)
            return;
        if (!(seat->Flags & VEHICLE_SEAT_FLAG_CAN_ATTACK))
        {
            GetPlayer()->SendMeleeAttackStop(pEnemy);
            return;
        }
    }

    _player->Attack(pEnemy, true);
}

void WorldSession::HandleAttackStop(WorldPackets::Combat::AttackStop& /*recvData*/)
{
    GetPlayer()->AttackStop();
}

void WorldSession::HandleSetSheathed(WorldPackets::Combat::SetSheathed& packet)
{
    if (packet.CurrentSheathState >= MAX_SHEATH_STATE)
        return;

    GetPlayer()->SetSheath(SheathState(packet.CurrentSheathState));
}