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

#ifndef __TRINITY_VEHICLE_H
#define __TRINITY_VEHICLE_H

#include "ObjectDefines.h"
#include "VehicleDefines.h"
#include "Unit.h"

struct VehicleEntry;
class Unit;
class VehicleJoinEvent;

class Vehicle : public TransportBase
{
    protected:
        friend bool Unit::CreateVehicleKit(uint32 id, uint32 creatureEntry, uint32 RecAura, bool);
        Vehicle(Unit* unit, VehicleEntry const* vehInfo, uint32 creatureEntry, uint32 recAura);

        friend void Unit::RemoveVehicleKit(bool);
        ~Vehicle();

    public:
        void Install();
        void Uninstall(bool uninstallBeforeDelete = false);
        void Reset(bool evading = false);
        void InstallAllAccessories(bool evading);
        void ApplyAllImmunities();
        void InstallAccessory(VehicleAccessory const* accessory);   //! May be called from scripts

        Unit* GetBase() const;
        VehicleEntry const* GetVehicleInfo() const;
        uint32 GetCreatureEntry() const;

        bool HasEmptySeat(int8 seatId) const;
        Unit* GetPassenger(int8 seatId) const;
        SeatMap::const_iterator GetNextEmptySeat(int8 seatId, bool next) const;
        uint8 GetAvailableSeatCount() const;
        uint32 GetRecAura() const;
        bool CheckCustomCanEnter();
        bool AddPassenger(Unit* passenger, int8 seatId = -1);
        void RemovePassenger(Unit* passenger);
        void RelocatePassengers();
        void RemoveAllPassengers();
        bool IsVehicleInUse();

        bool ArePassengersSpawnedByAI() const;
        void SetPassengersSpawnedByAI(bool passengersSpawnedByAI);

        bool CanBeCastedByPassengers() const;
        void SetCanBeCastedByPassengers(bool canBeCastedByPassengers);

        void SetLastShootPos(Position const& pos);
        Position GetLastShootPos();

        SeatMap Seats;                                      ///< The collection of all seats on the vehicle. Including vacant ones.

        VehicleSeatEntry const* GetSeatForPassenger(Unit const* passenger) const;

        void RemovePendingEventsForPassenger(Unit* passenger);

        void TeleportAccessory(uint32 zoneId);
        void RestoreAccessory();

    protected:
        friend class VehicleJoinEvent;
        uint32 UsableSeatNum;                               ///< Number of seats that match VehicleSeatEntry::UsableByPlayer, used for proper display flags

    private:
        enum Status
        {
            STATUS_NONE,
            STATUS_INSTALLED,
            STATUS_UNINSTALLING,
        };

        SeatMap::iterator GetSeatIteratorForPassenger(Unit* passenger);
        void InitMovementInfoForBase();

        /// This method transforms supplied transport offsets into global coordinates
        void CalculatePassengerPosition(float& x, float& y, float& z, float* o /*= NULL*/) const override;

        /// This method transforms supplied global coordinates into local offsets
        void CalculatePassengerOffset(float& x, float& y, float& z, float* o /*= NULL*/) const override;

        void RemovePendingEvent(VehicleJoinEvent* e);
        void AddPendingEvent(VehicleJoinEvent* e);
        void RemovePendingEventsForSeat(int8 seatId);

        Unit* _me;                                          ///< The underlying unit with the vehicle kit. Can be player or creature.
        VehicleEntry const* _vehicleInfo;                   ///< DBC data for vehicle
        GuidSet vehiclePlayers;
        uint32 _creatureEntry;                              ///< Can be different than the entry of _me in case of players
        Status _status;                                     ///< Internal variable for sanity checks
        Position _lastShootPos;

        uint32 _recAura;                                    ///< aura 296 SPELL_AURA_SET_VEHICLE_ID create vehicle from players.
        bool _isBeingDismissed;
        bool _passengersSpawnedByAI;
        bool _canBeCastedByPassengers;
        bool _canSeat;

        typedef std::list<VehicleJoinEvent*> PendingJoinEventContainer;
        PendingJoinEventContainer _pendingJoinEvents;       ///< Collection of delayed join events for prospective passengers

        std::mutex _lock;
};

class VehicleJoinEvent : public BasicEvent
{
    friend class Vehicle;
    protected:
        VehicleJoinEvent(Vehicle* v, Unit* u);

        ~VehicleJoinEvent();
        bool Execute(uint64, uint32);
        void Abort(uint64);

        C_PTR  ptr;
        Vehicle* vehicle;
        Unit* Target;
        Unit* Passenger;
        SeatMap::iterator Seat;
        ObjectGuid targetGuid;
};

#endif
