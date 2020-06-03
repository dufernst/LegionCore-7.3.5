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

#ifndef __TRINITY_VEHICLEDEFINES_H
#define __TRINITY_VEHICLEDEFINES_H

struct VehicleSeatEntry;

enum VehicleFlags
{
    VEHICLE_FLAG_NO_STRAFE                       = 0x00000001,           // Sets MOVEFLAG2_NO_STRAFE
    VEHICLE_FLAG_NO_JUMPING                      = 0x00000002,           // Sets MOVEFLAG2_NO_JUMPING
    VEHICLE_FLAG_FULLSPEEDTURNING                = 0x00000004,           // Sets MOVEFLAG2_FULLSPEEDTURNING
    VEHICLE_FLAG_UNK1                            = 0x00000008,
    VEHICLE_FLAG_ALLOW_PITCHING                  = 0x00000010,           // Sets MOVEFLAG2_ALLOW_PITCHING
    VEHICLE_FLAG_FULLSPEEDPITCHING               = 0x00000020,           // Sets MOVEFLAG2_FULLSPEEDPITCHING
    VEHICLE_FLAG_CUSTOM_PITCH                    = 0x00000040,           // If set use pitchMin and pitchMax from DBC, otherwise pitchMin = -pi/2, pitchMax = pi/2
    VEHICLE_FLAG_UNK2                            = 0x00000080,
    VEHICLE_FLAG_UNK3                            = 0x00000100,
    VEHICLE_FLAG_UNK4                            = 0x00000200,           // Vehicle is accessory?
    VEHICLE_FLAG_ADJUST_AIM_ANGLE                = 0x00000400,           // Lua_IsVehicleAimAngleAdjustable
    VEHICLE_FLAG_ADJUST_AIM_POWER                = 0x00000800,           // Lua_IsVehicleAimPowerAdjustable
    VEHICLE_FLAG_UNK5                            = 0x00001000,
    VEHICLE_FLAG_UNK6                            = 0x00002000,
    VEHICLE_FLAG_UNK7                            = 0x00004000,
    VEHICLE_FLAG_UNK8                            = 0x00008000,
    VEHICLE_FLAG_UNK9                            = 0x00010000,
    VEHICLE_FLAG_UNK10                           = 0x00020000,
    VEHICLE_FLAG_UNK11                           = 0x00040000,
    VEHICLE_FLAG_UNK12                           = 0x00080000,
    VEHICLE_FLAG_UNK13                           = 0x00100000,
    VEHICLE_FLAG_FIXED_POSITION                  = 0x00200000,           // Used for vehicles that have a fixed position, such as cannons
    VEHICLE_FLAG_DISABLE_SWITCH                  = 0x00400000,           // Can't change seats, VEHICLE_ID = 335 chopper
    VEHICLE_FLAG_UNK15                           = 0x00800000,
    VEHICLE_FLAG_UNK16                           = 0x01000000,
    VEHICLE_FLAG_UNK17                           = 0x02000000,
    VEHICLE_FLAG_UNK18                           = 0x04000000,
    VEHICLE_FLAG_UNK19                           = 0x08000000,
    VEHICLE_FLAG_BATTLEFIELD_ICON                = 0x10000000,           // Vehicle not dismissed after eject passenger?
    VEHICLE_FLAG_UNK21                           = 0x20000000,
    VEHICLE_FLAG_UNK22                           = 0x40000000,
    VEHICLE_FLAG_UNK23                           = 0x80000000,
};

enum VehicleSpells
{
    VEHICLE_SPELL_RIDE_HARDCODED                 = 46598,
    VEHICLE_SPELL_PARACHUTE                      = 45472
};

struct PassengerInfo
{
    ObjectGuid Guid;
    bool IsUnselectable;
    bool IsGravityDisabled;

    void Reset()
    {
        Guid.Clear();
        IsUnselectable = false;
        IsGravityDisabled = false;
    }
};

struct VehicleSeat
{
    explicit VehicleSeat(VehicleSeatEntry const* seatInfo) : SeatInfo(seatInfo) { Passenger.Reset(); }
    VehicleSeatEntry const* SeatInfo;
    PassengerInfo Passenger;
    Unit* unit{};
};

struct VehicleAccessory
{
    VehicleAccessory(uint32 entry, int8 seatId, bool isMinion, uint8 summonType, uint32 summonTime, Position pos) :
        AccessoryEntry(entry), IsMinion(isMinion), SummonTime(summonTime), SeatId(seatId), SummonedType(summonType), Pos(pos) { }

    uint32 AccessoryEntry;
    bool IsMinion;
    uint32 SummonTime;
    int8 SeatId;
    uint8 SummonedType;
    Position Pos;
};

typedef std::vector<VehicleAccessory> VehicleAccessoryList;
typedef std::map<ObjectGuid::LowType, VehicleAccessoryList> VehicleAccessoryContainer;
typedef std::map<uint32, VehicleAccessoryList> VehicleAccessoryTemplateContainer;
typedef std::map<int8, VehicleSeat> SeatMap;

struct VehicleAttachmentOffset
{
    VehicleAttachmentOffset(uint32 entry, int8 seatId, Position pos) : Pos(pos), Entry(entry), SeatId(seatId) { }

    Position Pos;
    uint32 Entry;
    int8 SeatId;
};

typedef std::vector<VehicleAttachmentOffset> VehicleAttachmentOffsetList;
typedef std::map<uint32, VehicleAttachmentOffsetList> VehicleAttachmentOffsetContainer;

class TransportBase
{
protected:
    TransportBase() { }
    virtual ~TransportBase() { }

public:
    /// This method transforms supplied transport offsets into global coordinates
    virtual void CalculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) const = 0;

    /// This method transforms supplied global coordinates into local offsets
    virtual void CalculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) const = 0;

    static void CalculatePassengerPosition(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
    {
        float inx = x, iny = y, inz = z;
        if (o)
            *o = Position::NormalizeOrientation(transO + *o);

        x = transX + inx * std::cos(transO) - iny * std::sin(transO);
        y = transY + iny * std::cos(transO) + inx * std::sin(transO);
        z = transZ + inz;
    }

    static void CalculatePassengerOffset(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
    {
        if (o)
            *o = Position::NormalizeOrientation(*o - transO);

        z -= transZ;
        y -= transY;    // y = searchedY * std::cos(o) + searchedX * std::sin(o)
        x -= transX;    // x = searchedX * std::cos(o) + searchedY * std::sin(o + pi)
        float inx = x, iny = y;
        y = (iny - inx * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
        x = (inx + iny * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
    }
};
#endif
