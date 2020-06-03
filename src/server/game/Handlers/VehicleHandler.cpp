/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "WorldSession.h"
#include "Opcodes.h"
#include "Vehicle.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "VehiclePackets.h"

void WorldSession::HandleMoveDismissVehicle(WorldPackets::Vehicle::MoveDismissVehicle& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (player->GetCharmGUID().IsEmpty())
        return;

    Unit* vehicle_base = player->GetVehicleBase();
    if (!vehicle_base)
        return;

    player->ValidateMovementInfo(&packet.Status);
    vehicle_base->m_movementInfo = packet.Status;
    player->ExitVehicle();
}

void WorldSession::HandleRequestVehiclePrevSeat(WorldPackets::Vehicle::RequestVehiclePrevSeat& /*packet*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetVehicleBase())
        return;

    VehicleSeatEntry const* seat = player->GetVehicle()->GetSeatForPassenger(player);
    if (!seat || !seat->CanSwitchFromSeat())
        return;

    player->ChangeSeat(-1, false);
}

void WorldSession::HandleRequestVehicleNextSeat(WorldPackets::Vehicle::RequestVehicleNextSeat& /*packet*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetVehicleBase())
        return;

    VehicleSeatEntry const* seat = player->GetVehicle()->GetSeatForPassenger(player);
    if (!seat || !seat->CanSwitchFromSeat())
        return;

    player->ChangeSeat(-1, true);
}

void WorldSession::HandleMoveChangeVehicleSeats(WorldPackets::Vehicle::MoveChangeVehicleSeats& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Unit* vehicle_base = player->GetVehicleBase();
    if (!vehicle_base)
        return;

    VehicleSeatEntry const* seat = player->GetVehicle()->GetSeatForPassenger(player);
    if (!seat || !seat->CanSwitchFromSeat())
        return;

    player->ValidateMovementInfo(&packet.Status);

    if (vehicle_base->GetGUID() != packet.Status.Guid)
        return;

    vehicle_base->m_movementInfo = packet.Status;

    if (packet.DstVehicle.IsEmpty())
        player->ChangeSeat(-1, packet.DstSeatIndex != 255);
    else if (Unit* vehUnit = ObjectAccessor::GetUnit(*player, packet.DstVehicle))
        if (Vehicle* vehicle = vehUnit->GetVehicleKit())
            if (vehicle->HasEmptySeat(packet.DstSeatIndex))
                vehUnit->HandleSpellClick(player, int8(packet.DstSeatIndex));
}

void WorldSession::HandleRequestVehicleSwitchSeat(WorldPackets::Vehicle::RequestVehicleSwitchSeat& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Unit* vehicleBase = player->GetVehicleBase();
    if (!vehicleBase)
        return;

    VehicleSeatEntry const* seat = player->GetVehicle()->GetSeatForPassenger(player);
    if (!seat || !seat->CanSwitchFromSeat())
        return;

    if (vehicleBase->GetGUID() == packet.Vehicle)
        player->ChangeSeat(int8(packet.SeatIndex));
    else if (Unit* vehUnit = ObjectAccessor::GetUnit(*player, packet.Vehicle))
        if (Vehicle* vehicle = vehUnit->GetVehicleKit())
            if (vehicle->HasEmptySeat(int8(packet.SeatIndex)))
                vehUnit->HandleSpellClick(player, int8(packet.SeatIndex));
}

void WorldSession::HandleRideVehicleInteract(WorldPackets::Vehicle::RideVehicleInteract& packet)
{
    if (Player* player = ObjectAccessor::FindPlayer(packet.Vehicle))
    {
        if (!player->GetVehicleKit() || !player->IsInRaidWith(_player) || !player->IsWithinDistInMap(_player, INTERACTION_DISTANCE) || player->InArena())
            return;

        _player->EnterVehicle(player);
    }
}

void WorldSession::HandleSetVehicleRecId(WorldPackets::Vehicle::MoveSetVehicleRecIdAck& packet)
{
    Player* player = GetPlayer();

    if (!player)
        return;

    player->ValidateMovementInfo(&packet.Data.movementInfo);

    // try find vehicle
    if (Vehicle* v = _player->GetVehicleKit())
    {
        if (v->GetVehicleInfo()->ID != packet.VehicleRecID)
            return;
    }

    // movementInfo? need update time and relocate?
    // update time
    packet.Data.movementInfo.MoveTime = getMSTime();
    // relocate
    RelocateMover(packet.Data.movementInfo);
    player->UpdateFallInformationIfNeed(packet.Data.movementInfo, packet.GetOpcode());

    WorldPackets::Vehicle::SetVehicleRecID setVehicleRec;
    setVehicleRec.VehicleGUID = player->GetGUID();
    setVehicleRec.VehicleRecID = packet.VehicleRecID;
    player->SendMessageToSet(setVehicleRec.Write(), false);
}

void WorldSession::HandleEjectPassenger(WorldPackets::Vehicle::EjectPassenger& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Vehicle* vehicle = player->GetVehicleKit();
    if (!vehicle)
        return;

    if (packet.Passenger.IsPlayer())
    {
        Player* player_ = ObjectAccessor::FindPlayer(packet.Passenger);
        if (!player_)
            return;

        if (!player_->IsOnVehicle(vehicle->GetBase()))
            return;

        if (VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(player_))
            if (seat->IsEjectable())
                player_->ExitVehicle();
    }
    else if (packet.Passenger.IsCreature())
    {
        Unit* unit = ObjectAccessor::GetUnit(*player, packet.Passenger);
        if (!unit)
            return;

        if (!unit->IsOnVehicle(vehicle->GetBase()))
            return;

        if (unit->GetEntry() == 89715)
            return;

        if (VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(unit))
            if (seat->IsEjectable())
                unit->ExitVehicle();
    }
}

void WorldSession::HandleRequestVehicleExit(WorldPackets::Vehicle::RequestVehicleExit& /*packet*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (Vehicle* vehicle = player->GetVehicle())
    {
        if (VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(player))
        {
            if (seat->CanEnterOrExit())
                player->ExitVehicle();
        }
    }
}
