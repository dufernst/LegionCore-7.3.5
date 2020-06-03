/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#ifndef ObjectGuid_h__
#define ObjectGuid_h__

#include "ByteBuffer.h"
#include "ObjectDefines.h"
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <vector>

enum TypeID : uint8
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7,
    TYPEID_AREATRIGGER   = 8,
    TYPEID_SCENEOBJECT   = 9,
    TYPEID_CONVERSATION  = 10,
    TYPEID_EVENTOBJECT   = 11,

    NUM_CLIENT_OBJECT_TYPES
};

#define TRADE_DISTANCE                      11.11f

enum TypeMask
{
    TYPEMASK_OBJECT         = 0x0001,
    TYPEMASK_ITEM           = 0x0002,
    TYPEMASK_CONTAINER      = 0x0004,   //MoP: TYPEMASK_CONTAINER      = TYPEMAST_BAG | TYPEMASK_ITEM,                       // TYPEMASK_ITEM | 0x0004
    TYPEMASK_UNIT           = 0x0008,
    TYPEMASK_PLAYER         = 0x0010,
    TYPEMASK_GAMEOBJECT     = 0x0020,
    TYPEMASK_DYNAMICOBJECT  = 0x0040,
    TYPEMASK_CORPSE         = 0x0080,
    TYPEMASK_AREATRIGGER    = 0x0100,
    TYPEMASK_SCENEOBJECT    = 0x0200,
    TYPEMASK_CONVERSATION   = 0x0400,
    TYPEMASK_EVENTOBJECT    = 0x0800,
    TYPEMASK_SEER           = TYPEMASK_PLAYER | TYPEMASK_UNIT | TYPEMASK_DYNAMICOBJECT
};

enum class HighGuid
{
    Null            = 0,
    Uniq            = 1,
    Player          = 2,
    Item            = 3,
    WorldTransaction = 4,
    StaticDoor      = 5,
    Transport       = 6,
    Conversation    = 7,
    Creature        = 8,
    Vehicle         = 9,
    Pet             = 10,
    GameObject      = 11,
    DynamicObject   = 12,
    AreaTrigger     = 13,
    Corpse          = 14,
    LootObject      = 15,
    SceneObject     = 16,
    Scenario        = 17,
    AIGroup         = 18,
    DynamicDoor     = 19,
    ClientActor     = 20,
    Vignette        = 21,
    CallForHelp     = 22,
    AIResource      = 23,
    AILock          = 24,
    AILockTicket    = 25,
    ChatChannel     = 26,
    Party           = 27,
    Guild           = 28,
    WowAccount      = 29,
    BNetAccount     = 30,
    GMTask          = 31,
    MobileSession   = 32,
    RaidGroup       = 33,
    Mail            = 35,
    WebObj          = 36,
    LFGObject       = 37,
    LFGList         = 38,
    UserRouter      = 39,
    PVPQueueGroup   = 40,
    UserClient      = 41,
    PetBattle       = 42,
    UniqUserClient  = 43,
    BattlePet       = 44,
    CommerceObj     = 45,
    ClientSession   = 46,
    Cast            = 47,
    Spell           = 48,
    EventObject     = 49,

    Count,
};

class ObjectGuid;
class GuidFormat;

#pragma pack(push, 1)

class ObjectGuid
{
    friend std::ostream& operator<<(std::ostream& stream, ObjectGuid const& guid);
    friend ByteBuffer& operator<<(ByteBuffer& buf, ObjectGuid const& guid);
    friend ByteBuffer& operator>>(ByteBuffer& buf, ObjectGuid& guid);
    friend class GuidFormat;

    public:
        static ObjectGuid const Empty;
        static ObjectGuid const TradeItem;

        typedef uint64 LowType;

        template<HighGuid type>
        static ObjectGuid Create(LowType counter);

        template<HighGuid type>
        static ObjectGuid Create(uint16 mapId, uint32 entry, LowType counter, uint8 subType = 0);

        template<HighGuid type>
        static ObjectGuid Create(LowType counter, uint8 subType);

        ObjectGuid();
        ObjectGuid(ObjectGuid const&) = default;

        std::vector<uint8> GetRawValue() const;
        void SetRawValue(std::vector<uint8> const& guid);
        void SetRawValue(uint64 high, uint64 low);
        void Clear();

        HighGuid GetHigh() const;
        uint32 GetRealmId() const;
        uint32 GetMapId() const;
        uint32 GetEntry() const;
        uint8 GetSubType() const;
        LowType GetCounter() const;

        uint64 GetHighPart() const;
        uint64 GetLowPart() const;
        uint32 GetGUIDLow() const;
        uint32 GetGUIDHigh() const;

        static LowType GetMaxCounter(HighGuid /*high*/);

        uint32 GetMaxCounter() const;

        bool IsEmpty() const;
        bool IsCreature() const;
        bool IsPet() const;
        bool IsVehicle() const;
        bool IsCreatureOrPet() const;
        bool IsCreatureOrVehicle() const;
        bool IsCreatureOrPetOrVehicle() const;
        bool IsAnyTypeCreature() const;
        bool IsPlayer() const;
        bool IsUnit() const;
        bool IsItem() const;
        bool IsGameObject() const;
        bool IsDynamicObject() const;
        bool IsCorpse() const;
        bool IsAreaTrigger() const;
        bool IsLoot() const;
        bool IsMOTransport() const;
        bool IsAnyTypeGameObject() const;
        bool IsParty() const;
        bool IsGuild() const;
        bool IsSceneObject() const;
        bool IsConversation() const;
        bool IsSpell() const;
        bool IsCast() const;
        bool IsEvent() const;

        static TypeID GetTypeId(HighGuid high);
        TypeID GetTypeId() const;

        bool operator!() const;
        bool operator==(ObjectGuid const& guid) const;
        bool operator!=(ObjectGuid const& guid) const;
        bool operator<(ObjectGuid const& guid) const;
        bool operator<=(ObjectGuid const& guid) const;
        bool operator>(ObjectGuid const& guid) const;
        bool operator>=(ObjectGuid const& guid) const;
        operator bool() const;
        operator int() const;

        static char const* GetTypeName(HighGuid high);
        char const* GetTypeName() const;
        std::string ToString() const;
        std::size_t GetHash() const;

        bool HasMapId() const;

    private:

        static bool HasMapId(HighGuid high);
        static bool HasEntry(HighGuid high);
        bool HasEntry() const;

        ObjectGuid(uint64 high, uint64 low);
        explicit ObjectGuid(uint32 const&) = delete;                 // no implementation, used to catch wrong type assignment

        uint64 _low;
        uint64 _high;
};

#pragma pack(pop)

// Some Shared defines
typedef std::set<ObjectGuid> GuidSet;
typedef std::list<ObjectGuid> GuidList;
typedef std::deque<ObjectGuid> GuidDeque;
typedef std::vector<ObjectGuid> GuidVector;
typedef std::unordered_set<ObjectGuid> GuidUnorderedSet;
typedef std::map<uint32, GuidSet> GuidSetInMap;
typedef std::map<uint32, GuidList> GuidListInMap;
typedef std::map<uint32, GuidVector> GuidVectorInMap;
typedef std::set<uint32> Uint32Set;
typedef std::list<uint32> Uint32List;
typedef std::vector<uint32> Uint32Vector;

template<HighGuid high>
class ObjectGuidGenerator
{
    public:
        explicit ObjectGuidGenerator(ObjectGuid::LowType start = UI64LIT(1)) : _nextGuid(start) { }

        void Set(uint64 val) { _nextGuid = val; }
        ObjectGuid::LowType Generate();
        uint32 GenerateLow();
        ObjectGuid::LowType GetNextAfterMaxUsed() const { return _nextGuid; }

    private:
        uint64 _nextGuid;
};

ByteBuffer& operator<<(ByteBuffer& buf, ObjectGuid const& guid);
ByteBuffer& operator>>(ByteBuffer& buf, ObjectGuid& guid);
std::ostream& operator<<(std::ostream& stream, ObjectGuid const& guid);

namespace std
{
template<>
struct hash<ObjectGuid>
{
    size_t operator()(ObjectGuid const& key) const noexcept
    {
        return key.GetHash();
    }
};
}

#endif // ObjectGuid_h__
