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
#include "Log.h"
#include "ObjectMgr.h"
#include "Vehicle.h"
#include "Unit.h"
#include "Util.h"
#include "WorldPacket.h"
#include "ScriptMgr.h"
#include "CreatureAI.h"
#include "ZoneScript.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "MoveSplineInit.h"
#include "EventProcessor.h"
#include "Player.h"
#include "Battleground.h"

Vehicle::Vehicle(Unit* unit, VehicleEntry const* vehInfo, uint32 creatureEntry, uint32 recAura) :
    UsableSeatNum(0), _me(unit), _vehicleInfo(vehInfo), _creatureEntry(creatureEntry), _status(STATUS_NONE),
    _recAura(recAura), _isBeingDismissed(false), _passengersSpawnedByAI(false), _canBeCastedByPassengers(false),
    _canSeat(true)
{
    for (uint32 i = 0; i < MAX_VEHICLE_SEATS; ++i)
    {
        if (uint32 seatId = _vehicleInfo->SeatID[i])
            if (VehicleSeatEntry const* veSeat = sVehicleSeatStore.LookupEntry(seatId))
            {
                Seats.insert(std::make_pair(i, VehicleSeat(veSeat)));
                if (veSeat->CanEnterOrExit() || veSeat->IsUsableByOverride())
                    ++UsableSeatNum;
            }
    }

    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(_recAura))
    {
        if (_me->IsPlayer() && !spellInfo->_IsPositiveSpell())
            _canSeat = false;

        // TODO: temp disabled Flying Legion Disc - bugged
        if (spellInfo->Id == 234740)
            _canSeat = false;
    }

    if (UsableSeatNum && unit->CanVehicleAI() && _canSeat)
        _me->SetFlag(UNIT_FIELD_NPC_FLAGS, (_me->IsPlayer() ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));
    else
        _me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, (_me->IsPlayer() ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));

    InitMovementInfoForBase();
    objectCountInWorld[uint8(HighGuid::Vehicle)]++;
}

Vehicle::~Vehicle()
{
    /// @Uninstall must be called before this.
    //ASSERT(_status == STATUS_UNINSTALLING);
    //for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        //ASSERT(!itr->second.Passenger);
    objectCountInWorld[uint8(HighGuid::Vehicle)]--;
}

/**
 * @fn void Vehicle::Install()
 *
 * @brief Initializes power type for vehicle. Nothing more.
 *
 * @author Machiavelli
 * @date 17-2-2013
 */

void Vehicle::Install()
{
    if (Creature* creature = _me->ToCreature())
    {
        if (PowerDisplayEntry const* powerDisplay = sPowerDisplayStore.LookupEntry(_vehicleInfo->PowerDisplayID[0]))
        {
            _me->setPowerType(Powers(powerDisplay->ActualType));
            _me->SetMaxPower(Powers(powerDisplay->ActualType), _me->GetCreatePowers(Powers(powerDisplay->ActualType)));
            _me->SetPower(Powers(powerDisplay->ActualType), _me->GetPowerForReset(Powers(powerDisplay->ActualType), powerDisplay->ID));
        }
        else
        {
            Powers powerType = POWER_ENERGY;
            for (uint32 i = 0; i < MAX_SPELL_CONTROL_BAR; ++i)
            {
                uint32 spellId = i < CREATURE_MAX_SPELLS ? creature->m_templateSpells[i] : 0;
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo)
                    continue;

                if (!spellInfo->IsPowerActive(0))
                    continue;

                if (SpellPowerEntry const* power = spellInfo->GetPowerInfo(0))
                    powerType = static_cast<Powers>(power->PowerType);
                break;
            }
            _me->setPowerType(powerType);
            _me->SetMaxPower(powerType, _me->GetCreatePowers(powerType));
            _me->SetPower(powerType, _me->GetPowerForReset(powerType));
        }
    }

    _status = STATUS_INSTALLED;
    if (GetBase()->IsCreature())
        sScriptMgr->OnInstall(this);
}

void Vehicle::InstallAllAccessories(bool evading)
{
    if (ArePassengersSpawnedByAI())
        return;

    if (GetBase()->IsPlayer() || !evading)
        RemoveAllPassengers();   // We might have aura's saved in the DB with now invalid casters - remove

    VehicleAccessoryList const* accessories = sObjectMgr->GetVehicleAccessoryList(this);
    if (!accessories)
        return;

    for (const auto& accessorie : *accessories)
        if (!evading || accessorie.IsMinion)  // only install minions on evade mode
            InstallAccessory(&accessorie);
}

/**
* @fn void Vehicle::Uninstall()
*
* @brief Removes all passengers and sets status to STATUS_UNINSTALLING.
*           No new passengers can be added to the vehicle after this call.
*
* @author Machiavelli
* @date 17-2-2013
*/

void Vehicle::Uninstall(bool uninstallBeforeDelete)
{
    /// @Prevent recursive uninstall call. (Bad script in OnUninstall/OnRemovePassenger/PassengerBoarded hook.)
    if (_status == STATUS_UNINSTALLING && !GetBase()->HasUnitTypeMask(UNIT_MASK_MINION))
    {
        TC_LOG_ERROR(LOG_FILTER_VEHICLES, "Vehicle GuidLow: %u, Entry: %u attempts to uninstall, but already has STATUS_UNINSTALLING! "
            "Check Uninstall/PassengerBoarded script hooks for errors.", _me->GetGUIDLow(), _me->GetEntry());
        return;
    }
    _status = STATUS_UNINSTALLING;

    if (uninstallBeforeDelete)
        _isBeingDismissed = true;

    TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Vehicle::Uninstall Entry: %u, GuidLow: %u", _creatureEntry, _me->GetGUIDLow());
    RemoveAllPassengers();

    if (GetBase()->IsCreature())
        sScriptMgr->OnUninstall(this);
}

/**
* @fn void Vehicle::Reset(bool evading )
*
* @brief Reapplies immunities and reinstalls accessories. Only has effect for creatures.
*
* @author Machiavelli
* @date 17-2-2013
*
* @param evading true if called from CreatureAI::EnterEvadeMode
*/

void Vehicle::Reset(bool evading /*= false*/)
{
    if (GetBase()->IsPlayer())
    {
        InstallAllAccessories(evading);
        return;
    }

    if (!GetBase()->IsCreature())
        return;

    TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Vehicle::Reset (Entry: %u, GuidLow: %u, DBGuid: %u)", GetCreatureEntry(), _me->GetGUIDLow(), _me->ToCreature()->GetDBTableGUIDLow());

    ApplyAllImmunities();
    InstallAllAccessories(evading);

    // Set or remove correct flags based on available seats. Will overwrite db data (if wrong).
    if (UsableSeatNum && _me->CanVehicleAI() && _canSeat)
        _me->SetFlag(UNIT_FIELD_NPC_FLAGS, (_me->IsPlayer() ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));
    else
        _me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, (_me->IsPlayer() ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));

    sScriptMgr->OnReset(this);
}

/**
* @fn void Vehicle::ApplyAllImmunities()
*
* @brief Applies specific immunities that cannot be set in DB.
*
* @author Machiavelli
* @date 17-2-2013
*/

void Vehicle::ApplyAllImmunities()
{
    // This couldn't be done in DB, because some spells have MECHANIC_NONE

    // Vehicles should be immune on Knockback ...
    if (GetVehicleInfo()->ID != 2059)
    {
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
    }

    // Mechanical units & vehicles ( which are not Bosses, they have own immunities in DB ) should be also immune on healing ( exceptions in switch below )
    if (_me->ToCreature() && _me->ToCreature()->GetCreatureTemplate()->Type == CREATURE_TYPE_MECHANICAL && !_me->ToCreature()->isWorldBoss())
    {
        // Heal & dispel ...
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_HEAL, true);
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_HEAL_PCT, true);
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_DISPEL, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_PERIODIC_HEAL, true);

        // ... Shield & Immunity grant spells ...
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_SCHOOL_IMMUNITY, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_UNATTACKABLE, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_SCHOOL_ABSORB, true);
        _me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SHIELD, true);
        _me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_MAGICAL_IMMUNITY, true);

        // ... Resistance, Split damage, Change stats ...
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_DAMAGE_SHIELD, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_SPLIT_DAMAGE_PCT, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_RESISTANCE, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STAT, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, true);
    }

    // Different immunities for vehicles goes below
    switch (GetVehicleInfo()->ID)
    {
        // code below prevents a bug with movable cannons
    case 160: // Strand of the Ancients
    case 244: // Wintergrasp
    case 510: // Isle of Conquest
        _me->SetControlled(true, UNIT_STATE_ROOT);
        // why we need to apply this? we can simple add immunities to slow mechanic in DB
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_DECREASE_SPEED, true);
        break;
    case 158:
        _me->ApplySpellImmune(0, IMMUNITY_ID, 33786, true);
        break;
    default:
        break;
    }
}

/**
 * @fn void Vehicle::RemoveAllPassengers()
 *
 * @brief Removes all current and pending passengers from the vehicle.
 *
 * @author Machiavelli
 * @date 17-2-2013
 */

void Vehicle::RemoveAllPassengers()
{
    TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Vehicle::RemoveAllPassengers. Entry: %u, GuidLow: %u", _creatureEntry, _me->GetGUIDLow());

    /// Setting to_Abort to true will cause @VehicleJoinEvent::Abort to be executed on next @Unit::UpdateEvents call
    /// This will properly "reset" the pending join process for the passenger.
    {
        /// Update vehicle pointer in every pending join event - Abort may be called after vehicle is deleted
        Vehicle* eventVehicle = _status != STATUS_UNINSTALLING ? this : nullptr;

        _lock.lock();
        while (!_pendingJoinEvents.empty())
        {
            VehicleJoinEvent* e = _pendingJoinEvents.front();
            e->to_Abort = true;
            e->vehicle = eventVehicle;
            _pendingJoinEvents.pop_front();
        }
        _lock.unlock();
    }

    // Passengers always cast an aura with SPELL_AURA_CONTROL_VEHICLE on the vehicle
    // We just remove the aura and the unapply handler will make the target leave the vehicle.
    // We don't need to iterate over Seats
    _me->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE);

    // Sometime aura do not work, so we iterate to be sure that every passengers have been removed
    // We need a copy because passenger->_ExitVehicle() may modify the Seats list
    SeatMap tempSeatMap = Seats;
    for (auto& itr : tempSeatMap)
    {
        if (itr.second.Passenger.Guid)
        {
            if (Unit* passenger = ObjectAccessor::FindUnit(itr.second.Passenger.Guid))
                passenger->_ExitVehicle();

            itr.second.Passenger.Guid.Clear();
        }
    }
}

/**
 * @fn bool Vehicle::HasEmptySeat(int8 seatId) const
 *
 * @brief Checks if vehicle's seat specified by 'seatId' is empty.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param seatId Identifier for the seat.
 *
 * @return true if empty seat, false if not.
 */

bool Vehicle::HasEmptySeat(int8 seatId) const
{
    auto seat = Seats.find(seatId);
    if (seat == Seats.end())
        return false;
    return !seat->second.Passenger.Guid;
}

/**
 * @fn Unit* Vehicle::GetPassenger(int8 seatId) const
 *
 * @brief Gets a passenger on specified seat.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param seatId Seat to look on.
 *
 * @return null if it not found, else pointer to passenger if in world
 */

Unit* Vehicle::GetPassenger(int8 seatId) const
{
    auto seat = Seats.find(seatId);
    if (seat == Seats.end())
        return nullptr;

    return ObjectAccessor::GetUnit(*GetBase(), seat->second.Passenger.Guid);
}

/**
 * @fn SeatMap::const_iterator Vehicle::GetNextEmptySeat(int8 seatId, bool next) const
 *
 * @brief Gets the next empty seat based on current seat.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param seatId Identifier for the current seat.
 * @param next   true if iterating forward, false means iterating backwards.
 *
 * @return The next empty seat.
 */

SeatMap::const_iterator Vehicle::GetNextEmptySeat(int8 seatId, bool next) const
{
    auto seat = Seats.find(seatId);
    if (seat == Seats.end())
        return seat;

    while (seat->second.Passenger.Guid || (!seat->second.SeatInfo->CanEnterOrExit() && !seat->second.SeatInfo->IsUsableByOverride()))
    {
        if (next)
        {
            if (++seat == Seats.end())
                seat = Seats.begin();
        }
        else
        {
            if (seat == Seats.begin())
                seat = Seats.end();
            --seat;
        }

        // Make sure we don't loop indefinetly
        if (seat->first == seatId)
            return Seats.end();
    }

    return seat;
}

/**
 * @fn void Vehicle::InstallAccessory(uint32 entry, int8 seatId, bool minion, uint8 type,
 *     uint32 summonTime)
 *
 * @brief Installs an accessory.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param entry      The NPC entry of accessory.
 * @param seatId     Identifier for the seat to add the accessory to.
 * @param minion     true if accessory considered a 'minion'. Implies that the accessory will despawn when the vehicle despawns.
 *                   Essentially that it has no life without the vehicle. Their fates are bound.
 * @param type       See enum @SummonType.
 * @param summonTime Time after which the minion is despawned in case of a timed despawn @type specified.
 */

void Vehicle::InstallAccessory(VehicleAccessory const* as)
{
    /// @Prevent adding accessories when vehicle is uninstalling. (Bad script in OnUninstall/OnRemovePassenger/PassengerBoarded hook.)
    if (_status == STATUS_UNINSTALLING)
    {
        TC_LOG_ERROR(LOG_FILTER_VEHICLES, "Vehicle (GuidLow: %u, DB GUID: %u, Entry: %u) attempts to install accessory (Entry: %u) on seat %d with STATUS_UNINSTALLING! "
            "Check Uninstall/PassengerBoarded script hooks for errors.", _me->GetGUIDLow(),
            (_me->IsCreature() ? _me->ToCreature()->GetDBTableGUIDLow() : _me->GetGUIDLow()), GetCreatureEntry(), as->AccessoryEntry, as->SeatId);
        return;
    }

    TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Vehicle (GuidLow: %u, DB Guid: %u, Entry %u): installing accessory (Entry: %u) on seat: %d RecAura %u SummonedType %u",
        _me->GetGUIDLow(), uint32(_me->IsCreature() ? _me->ToCreature()->GetDBTableGUIDLow() : _me->GetGUIDLow()), GetCreatureEntry(),
        as->AccessoryEntry, as->SeatId, GetRecAura(), as->SummonedType);

    Map* map = _me->FindMap();
    if (!map)
        return;

    // For correct initialization accessory should set owner 
    TempSummon* accessory = map->SummonCreature(as->AccessoryEntry, *_me, nullptr, as->SummonTime, _me, ObjectGuid::Empty, 0, GetRecAura() ? 0 : -1);

    //ASSERT(accessory);
    if (!accessory)
        return;

    accessory->SetTempSummonType(TempSummonType(as->SummonedType));
    if (as->IsMinion)
        accessory->AddUnitTypeMask(UNIT_MASK_ACCESSORY);

    accessory->m_movementInfo.transport.Pos = as->Pos;

    // Force enter for force vehicle aura - 296
    if (GetRecAura())
        accessory->EnterVehicle(_me, -1);
    else
        _me->HandleSpellClick(accessory, as->SeatId);

    /// If for some reason adding accessory to vehicle fails it will unsummon in
    /// @VehicleJoinEvent::Abort
}

Unit* Vehicle::GetBase() const
{
    return _me;
}

VehicleEntry const* Vehicle::GetVehicleInfo() const
{
    return _vehicleInfo;
}

uint32 Vehicle::GetCreatureEntry() const
{
    return _creatureEntry;
}

uint32 Vehicle::GetRecAura() const
{
    return _recAura;
}

bool Vehicle::IsVehicleInUse()
{
    return Seats.begin() != Seats.end();
}

bool Vehicle::ArePassengersSpawnedByAI() const
{
    return _passengersSpawnedByAI;
}

void Vehicle::SetPassengersSpawnedByAI(bool passengersSpawnedByAI)
{
    _passengersSpawnedByAI = passengersSpawnedByAI;
}

bool Vehicle::CanBeCastedByPassengers() const
{
    return _canBeCastedByPassengers;
}

void Vehicle::SetCanBeCastedByPassengers(bool canBeCastedByPassengers)
{
    _canBeCastedByPassengers = canBeCastedByPassengers;
}

void Vehicle::SetLastShootPos(Position const& pos)
{
    _lastShootPos.Relocate(pos);
}

Position Vehicle::GetLastShootPos()
{
    return _lastShootPos;
}

bool Vehicle::CheckCustomCanEnter()
{
    switch (GetCreatureEntry())
    {
    case 56682: // Keg in Stormstout Brewery
    case 46185: // Sanitron
    case 25460: //Amazing Flying Carpet. VehID 317
    case 33513: //368
    case 33386: //360
    case 63872: //2341
    case 80578: //Q34462
        return true;
    default:
        break;
    }

    return false;
}

/**
 * @fn bool Vehicle::AddPassenger(Unit* unit, int8 seatId)
 *
 * @brief Attempts to add a passenger to the vehicle on 'seatId'.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param [in,out] The prospective passenger.
 * @param seatId        Identifier for the seat. Value of -1 indicates the next available seat.
 *
 * @return true if it succeeds, false if it fails.
 */

bool Vehicle::AddPassenger(Unit* unit, int8 seatId)
{
    /// @Prevent adding passengers when vehicle is uninstalling. (Bad script in OnUninstall/OnRemovePassenger/PassengerBoarded hook.)
    if (_status == STATUS_UNINSTALLING)
    {
        if (unit)
            TC_LOG_ERROR(LOG_FILTER_VEHICLES, "Passenger GuidLow: %u, Entry: %u, attempting to board vehicle GuidLow: %u, Entry: %u during uninstall! SeatId: %d",
                unit->GetGUID().GetGUIDLow(), unit->GetEntry(), _me->GetGUID().GetGUIDLow(), _me->GetEntry(), static_cast<int32>(seatId));
        return false;
    }

    if (!unit)
        return false;

    TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Unit %s scheduling enter vehicle (entry: %u, vehicleId: %u, guid: %u (dbguid: %u) on seat %d",
        unit->GetName(), _me->GetEntry(), _vehicleInfo->ID, _me->GetGUID().GetGUIDLow(), (_me->IsCreature() ? _me->ToCreature()->GetGUIDLow() : 0), static_cast<int32>(seatId));

    // The seat selection code may kick other passengers off the vehicle.
    // While the validity of the following may be arguable, it is possible that when such a passenger
    // exits the vehicle will dismiss. That's why the actual adding the passenger to the vehicle is scheduled
    // asynchronously, so it can be cancelled easily in case the vehicle is uninstalled meanwhile.
    SeatMap::iterator seat;
    auto* e = new VehicleJoinEvent(this, unit);
    unit->m_Events.AddEvent(e, unit->m_Events.CalculateTime(0));

    if (seatId < 0) // no specific seat requirement
    {
        for (seat = Seats.begin(); seat != Seats.end(); ++seat)
            if (!seat->second.Passenger.Guid && (seat->second.SeatInfo->CanEnterOrExit() || seat->second.SeatInfo->IsUsableByOverride() || CheckCustomCanEnter()))
                break;

        if (seat == Seats.end()) // no available seat
        {
            e->to_Abort = true;
            return false;
        }

        unit->waitOnSeat = true;
        e->Seat = seat;
        AddPendingEvent(e);
    }
    else
    {
        seat = Seats.find(seatId);
        if (seat == Seats.end())
        {
            e->to_Abort = true;
            return false;
        }

        unit->waitOnSeat = true;
        e->Seat = seat;
        AddPendingEvent(e);
        if (seat->second.Passenger.Guid)
        {
            Unit* passenger = ObjectAccessor::GetUnit(*GetBase(), seat->second.Passenger.Guid);
            //ASSERT(passenger);
            if (passenger)
                passenger->ExitVehicle();
        }

        //ASSERT(!seat->second.Passenger);
        if (seat->second.Passenger.Guid)
            return false;
    }
    if (seat->second.SeatInfo->Flags && !(seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING))
        if (!(_me->ToCreature() && _me->ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_VEHICLE_ATTACKABLE_PASSENGERS) &&
            !(unit->ToCreature() && unit->ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_VEHICLE_ATTACKABLE_PASSENGERS))
            unit->AddUnitState(UNIT_STATE_ONVEHICLE);

    if (Player* player = _me->ToPlayer())
        player->AddToExtraLook(unit->GetGUID());

    if (Player* player = unit->ToPlayer())
    {
        player->AddToExtraLook(_me->GetGUID());
        for (auto& seat : Seats)
            if (seat.second.Passenger.Guid)
            {
                player->AddToExtraLook(seat.second.Passenger.Guid);
                if (Player* passenger = ObjectAccessor::GetPlayer(*_me, seat.second.Passenger.Guid))
                    passenger->AddToExtraLook(unit->GetGUID());
            }
    }

    return true;
}

/**
 * @fn void Vehicle::RemovePassenger(Unit* unit)
 *
 * @brief Removes the passenger from the vehicle.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param [in,out] unit The passenger to remove.
 */

void Vehicle::RemovePassenger(Unit* unit)
{
    if (unit->GetVehicle() != this)
        return;

    auto seat = GetSeatIteratorForPassenger(unit);
    //ASSERT(seat != Seats.end());
    if (seat == Seats.end())
        return;

    TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Unit %s exit vehicle entry %u id %u dbguid %u seat %d",
        unit->GetName(), _me->GetEntry(), _vehicleInfo->ID, _me->GetGUIDLow(), static_cast<int32>(seat->first));

    if (seat->second.SeatInfo->CanEnterOrExit() && ++UsableSeatNum && _me->CanVehicleAI() && _canSeat)
        _me->SetFlag(UNIT_FIELD_NPC_FLAGS, (_me->IsPlayer() ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));

    if (seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_DISABLE_GRAVITY && !seat->second.Passenger.IsGravityDisabled)
        unit->SetDisableGravity(false);

    if (seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE && !seat->second.Passenger.IsUnselectable)
        unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    seat->second.Passenger.Reset();

    unit->ClearUnitState(UNIT_STATE_ONVEHICLE);

    if (_me->IsCreature() && unit->IsPlayer())
    {
        if (seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_CAN_CONTROL)
            _me->RemoveCharmedBy(unit);
        // else if (seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_UNK2)
        // {
            // unit->ToPlayer()->SetClientControl(unit, true);
            // unit->ToPlayer()->SetViewpoint(_me, false);
            // unit->ToPlayer()->SetClientControl(_me, false);
        // }
    }

    if (_me->IsInWorld())
        unit->m_movementInfo.transport.Reset();

    // only for flyable vehicles
    if (unit->IsFlying())
        _me->CastSpell(unit, VEHICLE_SPELL_PARACHUTE, true);

    if (_me->IsCreature())
    {
        if (_me->ToCreature()->IsAIEnabled)
            _me->ToCreature()->AI()->PassengerBoarded(unit, seat->first, false);

        if (_me->IsInWorld())
            sScriptMgr->OnRemovePassenger(this, unit);
    }

    if (Player* player = _me->ToPlayer())
        player->RemoveFromExtraLook(unit->GetGUID());

    if (Player* player = unit->ToPlayer())
    {
        player->RemoveFromExtraLook(_me->GetGUID());
        for (auto& seat : Seats)
            if (seat.second.Passenger.Guid)
            {
                player->RemoveFromExtraLook(seat.second.Passenger.Guid);
                if (Player* passenger = ObjectAccessor::GetPlayer(*_me, seat.second.Passenger.Guid))
                    passenger->RemoveFromExtraLook(unit->GetGUID());
            }
    }
}

/**
 * @fn void Vehicle::RelocatePassengers()
 *
 * @brief Relocate passengers. Must be called after m_base::Relocate
 *
 * @author Machiavelli
 * @date 17-2-2013
 */

void Vehicle::RelocatePassengers()
{
    // not sure that absolute position calculation is correct, it must depend on vehicle pitch angle
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
    {
        if (Unit* passenger = ObjectAccessor::GetUnit(*GetBase(), itr->second.Passenger.Guid))
        {
            if (!passenger->IsInWorld())
                continue;

            float px, py, pz, po;
            passenger->m_movementInfo.transport.Pos.GetPosition(px, py, pz, po);
            CalculatePassengerPosition(px, py, pz, &po);
            passenger->UpdatePosition(px, py, pz, po);
        }
    }
}

/**
 * @fn void Vehicle::InitMovementInfoForBase()
 *
 * @brief Sets correct MovementFlags2 based on VehicleFlags from DBC.
 *
 * @author Machiavelli
 * @date 17-2-2013
 */

void Vehicle::InitMovementInfoForBase()
{
    uint32 vehicleFlags = GetVehicleInfo()->Flags;

    if (vehicleFlags & VEHICLE_FLAG_NO_STRAFE)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_NO_STRAFE);

    if (vehicleFlags & VEHICLE_FLAG_NO_JUMPING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);

    if (vehicleFlags & VEHICLE_FLAG_FULLSPEEDTURNING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_FULL_SPEED_TURNING);

    if (vehicleFlags & VEHICLE_FLAG_ALLOW_PITCHING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_ALWAYS_ALLOW_PITCHING);

    if (vehicleFlags & VEHICLE_FLAG_FULLSPEEDPITCHING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_FULL_SPEED_PITCHING);

    if (vehicleFlags & VEHICLE_FLAG_FIXED_POSITION)
    {
        _me->RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING);
        _me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
    }
}

void Vehicle::CalculatePassengerPosition(float& x, float& y, float& z, float* o /*= NULL*/) const
{
    TransportBase::CalculatePassengerPosition(x, y, z, o, GetBase()->GetPositionX(), GetBase()->GetPositionY(), GetBase()->GetPositionZ(), GetBase()->GetOrientation());
}

void Vehicle::CalculatePassengerOffset(float& x, float& y, float& z, float* o /*= NULL*/) const
{
    TransportBase::CalculatePassengerOffset(x, y, z, o, GetBase()->GetPositionX(), GetBase()->GetPositionY(), GetBase()->GetPositionZ(), GetBase()->GetOrientation());
}

/**
 * @fn VehicleSeatEntry const* Vehicle::GetSeatForPassenger(Unit* passenger)
 *
 * @brief Returns information on the seat of specified passenger, represented by the format in VehicleSeat.dbc
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param [in,out] The passenger for which we check the seat info.
 *
 * @return null if passenger not found on vehicle, else the DBC record for the seat.
 */

VehicleSeatEntry const* Vehicle::GetSeatForPassenger(Unit const* passenger) const
{
    if (!_me->IsInWorld() || (_me->ToCreature() && _me->ToCreature()->IsDespawn()))
        return nullptr;

    for (const auto& seat : Seats)
        if (seat.second.Passenger.Guid == passenger->GetGUID())
            return seat.second.SeatInfo;

    return nullptr;
}

/**
 * @fn SeatMap::iterator Vehicle::GetSeatIteratorForPassenger(Unit* passenger)
 *
 * @brief Gets seat iterator for specified passenger.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param [in,out] passenger Passenger to look up.
 *
 * @return The seat iterator for specified passenger if it's found on the vehicle. Otherwise Seats.end() (invalid iterator).
 */

SeatMap::iterator Vehicle::GetSeatIteratorForPassenger(Unit* passenger)
{
    for (auto itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second.Passenger.Guid == passenger->GetGUID())
            return itr;

    return Seats.end();
}

/**
 * @fn uint8 Vehicle::GetAvailableSeatCount() const
 *
 * @brief Gets the available seat count.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @return The available seat count.
 */

uint8 Vehicle::GetAvailableSeatCount() const
{
    uint8 ret = 0;
    for (const auto& seat : Seats)
        if (!seat.second.Passenger.Guid && (seat.second.SeatInfo->CanEnterOrExit() || seat.second.SeatInfo->IsUsableByOverride()))
            ++ret;

    return ret;
}

/**
 * @fn void Vehicle::RemovePendingEvent(VehicleJoinEvent* e)
 *
 * @brief Removes @VehicleJoinEvent objects from pending join event store.
 *        This method only removes it after it's executed or aborted to prevent leaving
 *        pointers to deleted events.
 *
 * @author Shauren
 * @date 22-2-2013
 *
 * @param [in] e The VehicleJoinEvent* to remove from pending event store.
 */

void Vehicle::RemovePendingEvent(VehicleJoinEvent* /*e*/)
{
    _lock.lock();
    _pendingJoinEvents.clear();
    _lock.unlock();
}

void Vehicle::AddPendingEvent(VehicleJoinEvent* e)
{
    _lock.lock();
    _pendingJoinEvents.push_back(e);
    _lock.unlock();
}

/**
 * @fn void Vehicle::RemovePendingEventsForSeat(uint8 seatId)
 *
 * @brief Removes any pending events for given seatId. Executed when a @VehicleJoinEvent::Execute is called
 *
 * @author Machiavelli
 * @date 23-2-2013
 *
 * @param seatId Identifier for the seat.
 */

void Vehicle::RemovePendingEventsForSeat(int8 seatId)
{
    _lock.lock();
    for (auto itr = _pendingJoinEvents.begin(); itr != _pendingJoinEvents.end();)
    {
        if ((*itr)->Seat->first == seatId)
        {
            (*itr)->to_Abort = true;
            _pendingJoinEvents.erase(itr++);
        }
        else
            ++itr;
    }
    _lock.unlock();
}

/**
 * @fn void Vehicle::RemovePendingEventsForSeat(uint8 seatId)
 *
 * @brief Removes any pending events for given passenger. Executed when vehicle control aura is removed while adding passenger is in progress
 *
 * @author Shauren
 * @date 13-2-2013
 *
 * @param passenger Unit that is supposed to enter the vehicle.
 */

void Vehicle::RemovePendingEventsForPassenger(Unit* passenger)
{
    _lock.lock();
    for (auto itr = _pendingJoinEvents.begin(); itr != _pendingJoinEvents.end();)
    {
        if ((*itr)->Passenger == passenger)
        {
            (*itr)->to_Abort = true;
            _pendingJoinEvents.erase(itr++);
        }
        else
            ++itr;
    }
    _lock.unlock();
}

void Vehicle::TeleportAccessory(uint32 zoneId)
{
    for (auto& seat : Seats)
    {
        if (seat.second.Passenger.Guid)
        {
            if (Creature* passenger = ObjectAccessor::GetCreature(*_me, seat.second.Passenger.Guid))
            {
                passenger->m_Teleports = true;
                passenger->CleanupBeforeTeleport();
                passenger->ResetMap();
                seat.second.unit = passenger;
            }
            else if (Player* player = ObjectAccessor::GetPlayer(*_me, seat.second.Passenger.Guid))
            {
                if (!player->IsChangeMap() && _me->GetCurrentZoneID() != player->GetCurrentZoneID())
                    player->UpdateZone(_me->GetCurrentZoneID(), _me->GetCurrentAreaID());
            }
        }
    }
}

void Vehicle::RestoreAccessory()
{
    if (!_me->IsInWorld() || (_me->ToCreature() && _me->ToCreature()->IsDespawn()))
        return;

    for (auto& seat : Seats)
    {
        if (seat.second.Passenger.Guid)
        {
            if (Unit* passenger = seat.second.unit)
            {
                if (passenger->ToCreature())
                {
                    passenger->SetMap(_me->GetMap());
                    _me->GetMap()->AddToMap(passenger->ToCreature());
                    seat.second.unit = nullptr;
                    passenger->m_Teleports = false;
                }
            }
        }
    }
}

VehicleJoinEvent::VehicleJoinEvent(Vehicle* v, Unit* u) : vehicle(v), Target(nullptr), Passenger(u), Seat(v->Seats.end()), targetGuid(v->GetBase() ? v->GetBase()->GetGUID() : ObjectGuid::Empty)
{
    if (v->GetBase())
        ptr = v->GetBase()->get_ptr();
}

VehicleJoinEvent::~VehicleJoinEvent()
{
    Object* obj = ptr.get();
    if (!obj)
        return;
    Target = obj->ToUnit();
    if (!Target)
        return;
    vehicle = Target->GetVehicleKit();
    if (!vehicle)
        return;

    vehicle->RemovePendingEvent(this);
}

/**
 * @fn bool VehicleJoinEvent::Execute(uint64, uint32)
 *
 * @brief Actually adds the passenger @Passenger to vehicle @Target.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param parameter1 Unused
 * @param parameter2 Unused.
 *
 * @return true, cannot fail.
 *
 */

bool VehicleJoinEvent::Execute(uint64, uint32)
{
    if (!Passenger->IsInWorld())
        return false;
    Target = ObjectAccessor::GetUnit(*Passenger, targetGuid);

    if (!Target || !Target->IsInWorld())
        return false;

    vehicle = Target->GetVehicleKit();

    if (!vehicle)
        return false;

    if (!vehicle->GetRecAura() && !Target->HasAuraTypeWithCaster(SPELL_AURA_CONTROL_VEHICLE, Passenger->GetGUID()))
        return false;

    Player* player = Passenger->ToPlayer();
    vehicle->RemovePendingEventsForSeat(Seat->first);
    vehicle->RemovePendingEventsForPassenger(Passenger);

    bool newTPos = true;
    if (Passenger->m_movementInfo.transport.Pos.m_positionX != 0.0f || Passenger->m_movementInfo.transport.Pos.m_positionY != 0.0f || Passenger->m_movementInfo.transport.Pos.m_positionZ != 0.0f || Passenger->m_movementInfo.transport.Pos.m_orientation != 0.0f)
        newTPos = false;

    Passenger->m_vehicle = vehicle;

    Seat->second.Passenger.Guid = Passenger->GetGUID();
    Seat->second.Passenger.IsUnselectable = Passenger->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    Seat->second.Passenger.IsGravityDisabled = Passenger->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);

    if (Seat->second.SeatInfo->CanEnterOrExit())
    {
        //ASSERT(vehicle->UsableSeatNum);
        if (!vehicle->UsableSeatNum)
            return false;
        --(vehicle->UsableSeatNum);
        if (!vehicle->UsableSeatNum)
        {
            if (Target->IsPlayer())
                Target->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE);
            else
                Target->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }
    }

    Passenger->InterruptNonMeleeSpells(false);
    Passenger->RemoveAurasByType(SPELL_AURA_MOUNTED);

    auto veSeat = Seat->second.SeatInfo;
    if (player)
    {
        // drop flag
        if (Battleground* bg = player->GetBattleground())
            bg->EventPlayerDroppedFlag(player);

        player->StopCastingCharm();
        player->StopCastingBindSight();
        player->SendOnCancelExpectedVehicleRideAura();
        if (!(veSeat->FlagsB & VEHICLE_SEAT_FLAG_B_KEEP_PET))
            player->UnsummonPetTemporaryIfAny();
    }

    if (veSeat->Flags & VEHICLE_SEAT_FLAG_HIDE_PASSENGER)
        if (!(Target->IsCreature() && Target->ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_VEHICLE_ATTACKABLE_PASSENGERS))
        {
            if (Seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE)
                Passenger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            Passenger->AddUnitState(UNIT_STATE_ONVEHICLE);
        }

    if (newTPos)
        Passenger->m_movementInfo.transport.Pos.SetPosition(veSeat->AttachmentOffset);
    Passenger->m_movementInfo.transport.MoveTime = 0; // 1 for player
    Passenger->m_movementInfo.transport.VehicleSeatIndex = Seat->first;
    Passenger->m_movementInfo.transport.Guid = Target->GetGUID();
    Passenger->m_movementInfo.transport.VehicleRecID = vehicle->GetVehicleInfo()->ID;

    // Hackfix
    switch (veSeat->ID)
    {
    case 10882:
        Passenger->m_movementInfo.transport.Pos.m_positionX = 15.0f;
        Passenger->m_movementInfo.transport.Pos.m_positionY = 0.0f;
        Passenger->m_movementInfo.transport.Pos.m_positionZ = 30.0f;
        break;
    default:
        break;
    }

    if (Target->IsCreature() && player)
    {
        if (Seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_CAN_CONTROL
            && !Target->SetCharmedBy(Passenger, CHARM_TYPE_VEHICLE))     // SMSG_CLIENT_CONTROL
        {
            //ASSERT(false);
            return false;
        }
        // if (Seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_UNK2 && Seat->second.SeatInfo->Flags & VEHICLE_SEAT_FLAG_CAN_CONTROL)
        // {
            // Passenger->Dismount();
            // player->SetClientControl(Target, true);
            // player->SetMover(Target);
            // player->SetViewpoint(Target, true);
        // }
    }

    Passenger->SendBreakTarget(Target);                      // SMSG_BREAK_TARGET
    Passenger->SetDisableGravity(true, true);
    Passenger->SetControlled(true, UNIT_STATE_ROOT);         // SMSG_FORCE_ROOT - In some cases we send SMSG_MOVE_SPLINE_ROOT here (for creatures)

    // also adds MOVEMENTFLAG_ROOT


    Position offset(veSeat->AttachmentOffset.X, veSeat->AttachmentOffset.Y, veSeat->AttachmentOffset.Z);

    if (offset.m_positionX == 0.0f && offset.m_positionY == 0.0f && offset.m_positionZ == 0.0f)
        if (VehicleAttachmentOffset const* attachmentOffset = sObjectMgr->GetVehicleAttachmentOffset(vehicle, Seat->first))
            offset = attachmentOffset->Pos;

    Movement::MoveSplineInit init(*Passenger);
    init.DisableTransportPathTransformations();
    init.MoveTo(offset.m_positionX, offset.m_positionY, offset.m_positionZ, false, true);
    init.SetFacing(0.0f);
    init.SetTransportEnter();
    init.Launch();

    //not we could install accessory
    if (Creature *c = Passenger->ToCreature())
    {
        if (c->onVehicleAccessoryInit())
        {
            // Before add to map call initialization accasorys if it has.
            if (c->GetVehicleKit())
                c->GetVehicleKit()->Reset();

            // and after initialization we finally could see
            c->SetVehicleAccessoryInit(false);
            Passenger->UpdateObjectVisibility(true); // not need, buged visible data when player move to vehicle
        }
    }

    if (Target->IsCreature())
    {
        if (Target->ToCreature()->IsAIEnabled)
            Target->ToCreature()->AI()->PassengerBoarded(Passenger, Seat->first, true);

        sScriptMgr->OnAddPassenger(vehicle, Passenger, Seat->first);

        // Actually quite a redundant hook. Could just use OnAddPassenger and check for unit typemask inside script.
        if (Passenger->HasUnitTypeMask(UNIT_MASK_ACCESSORY))
            sScriptMgr->OnInstallAccessory(vehicle, Passenger->ToCreature());
    }

    if (player)
        player->SendMovementSetCollisionHeight(player->GetCollisionHeight(true), 2); // Force update collision

    return true;
}

/**
 * @fn void VehicleJoinEvent::Abort(uint64)
 *
 * @brief Aborts the event. Implies that unit @Passenger will not be boarding vehicle @Target after all.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param parameter1 Unused
 */

void VehicleJoinEvent::Abort(uint64)
{
    Object *obj = ptr.get();
    Unit *targetBase = obj ? obj->ToUnit() : nullptr; // Faster then ObjectAccessor::GetUnit

    /// Check if the Vehicle was already uninstalled, in which case all auras were removed already
    //if (Target)
    //if (Unit* targetBase = ObjectAccessor::GetUnit(*Passenger, targetGuid))
    if (targetBase)
    {
        //TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Passenger GuidLow: %u, Entry: %u, board on vehicle GuidLow: %u, Entry: %u SeatId: %d cancelled",
            //Passenger->GetGUIDLow(), Passenger->GetEntry(), Target->GetBase()->GetGUIDLow(), Target->GetBase()->GetEntry(), (int32)Seat->first);

        /// @SPELL_AURA_CONTROL_VEHICLE auras can be applied even when the passenger is not (yet) on the vehicle.
        /// When this code is triggered it means that something went wrong in @Vehicle::AddPassenger, and we should remove
        /// the aura manually.
        //if(Unit* targetBase = Target->GetBase())
            //if (targetBase->IsInWorld())
        targetBase->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE, Passenger->GetGUID());
    }
    else if (Passenger)
        TC_LOG_DEBUG(LOG_FILTER_VEHICLES, "Passenger GuidLow: %u, Entry: %u, board on uninstalled vehicle SeatId: %d cancelled", Passenger->GetGUIDLow(), Passenger->GetEntry(), static_cast<int32>(Seat->first));
    else
        TC_LOG_ERROR(LOG_FILTER_VEHICLES, " WARNING!!! VehicleJoinEvent Abort with non existen Passanger");

    if (Passenger && Passenger->IsInWorld() && Passenger->HasUnitTypeMask(UNIT_MASK_ACCESSORY))
        Passenger->ToCreature()->DespawnOrUnsummon();
}