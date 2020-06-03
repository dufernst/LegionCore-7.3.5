/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "ObjectGuid.h"
#include "World.h"
#include <iomanip>
#include "Hash.h"

namespace
{
    struct GuidTypeNames
    {
        char const* Values[uint32(HighGuid::Count)];

        GuidTypeNames();
    } Names;

    GuidTypeNames::GuidTypeNames()
    {
#define SET_GUID_NAME(type) Values[uint32(HighGuid::type)] = #type;

        SET_GUID_NAME(Null);
        SET_GUID_NAME(Uniq);
        SET_GUID_NAME(Player);
        SET_GUID_NAME(Item);
        SET_GUID_NAME(WorldTransaction);
        SET_GUID_NAME(StaticDoor);
        SET_GUID_NAME(Transport);
        SET_GUID_NAME(Conversation);
        SET_GUID_NAME(Creature);
        SET_GUID_NAME(Vehicle);
        SET_GUID_NAME(Pet);
        SET_GUID_NAME(GameObject);
        SET_GUID_NAME(DynamicObject);
        SET_GUID_NAME(AreaTrigger);
        SET_GUID_NAME(Corpse);
        SET_GUID_NAME(LootObject);
        SET_GUID_NAME(SceneObject);
        SET_GUID_NAME(Scenario);
        SET_GUID_NAME(AIGroup);
        SET_GUID_NAME(DynamicDoor);
        SET_GUID_NAME(ClientActor);
        SET_GUID_NAME(Vignette);
        SET_GUID_NAME(CallForHelp);
        SET_GUID_NAME(AIResource);
        SET_GUID_NAME(AILock);
        SET_GUID_NAME(AILockTicket);
        SET_GUID_NAME(ChatChannel);
        SET_GUID_NAME(Party);
        SET_GUID_NAME(Guild);
        SET_GUID_NAME(WowAccount);
        SET_GUID_NAME(BNetAccount);
        SET_GUID_NAME(GMTask);
        SET_GUID_NAME(MobileSession);
        SET_GUID_NAME(RaidGroup);
        SET_GUID_NAME(Mail);
        SET_GUID_NAME(WebObj);
        SET_GUID_NAME(LFGObject);
        SET_GUID_NAME(LFGList);
        SET_GUID_NAME(UserRouter);
        SET_GUID_NAME(PVPQueueGroup);
        SET_GUID_NAME(UserClient);
        SET_GUID_NAME(PetBattle);
        SET_GUID_NAME(UniqUserClient);
        SET_GUID_NAME(BattlePet);
        SET_GUID_NAME(CommerceObj);
        SET_GUID_NAME(ClientSession);
        SET_GUID_NAME(Spell);
        SET_GUID_NAME(EventObject);

#undef SET_GUID_NAME
    }
}


TypeID ObjectGuid::GetTypeId(HighGuid high)
{
    switch (high)
    {
    case HighGuid::Item: return TYPEID_ITEM;
    case HighGuid::Creature:
    case HighGuid::Pet:
    case HighGuid::Vehicle: return TYPEID_UNIT;
    case HighGuid::Player: return TYPEID_PLAYER;
    case HighGuid::GameObject:
    case HighGuid::Transport: return TYPEID_GAMEOBJECT;
    case HighGuid::DynamicObject: return TYPEID_DYNAMICOBJECT;
    case HighGuid::Corpse: return TYPEID_CORPSE;
    case HighGuid::AreaTrigger: return TYPEID_AREATRIGGER;
    case HighGuid::SceneObject: return TYPEID_SCENEOBJECT;
    case HighGuid::Conversation: return TYPEID_CONVERSATION;
    case HighGuid::EventObject: return TYPEID_EVENTOBJECT;
    default:
        return TYPEID_OBJECT;
    }
}

TypeID ObjectGuid::GetTypeId() const
{
    return GetTypeId(GetHigh());
}

bool ObjectGuid::operator!() const
{
    return IsEmpty();
}

bool ObjectGuid::operator==(ObjectGuid const& guid) const
{
    return _low == guid._low && _high == guid._high;
}

bool ObjectGuid::operator!=(ObjectGuid const& guid) const
{
    return !(*this == guid);
}

bool ObjectGuid::operator<(ObjectGuid const& guid) const
{
    if (_high < guid._high)
        return true;
    if (_high > guid._high)
        return false;
    return _low < guid._low;
}

bool ObjectGuid::operator<=(ObjectGuid const& guid) const
{
    if (_high < guid._high)
        return true;
    if (_high > guid._high)
        return false;
    return _low <= guid._low;
}

bool ObjectGuid::operator>(ObjectGuid const& guid) const
{
    if (_high > guid._high)
        return false;
    if (_high < guid._high)
        return true;
    return _low > guid._low;
}

bool ObjectGuid::operator>=(ObjectGuid const& guid) const
{
    if (_high > guid._high)
        return true;
    if (_high < guid._high)
        return false;
    return _low >= guid._low;
}

ObjectGuid::operator bool() const
{
    return !IsEmpty();
}

ObjectGuid::operator int() const
{
    //static_assert(false, "ooo");
    return _low;
}

char const* ObjectGuid::GetTypeName(HighGuid high)
{
    if (high >= HighGuid::Count)
        return "<unknown>";
    return Names.Values[uint32(high)];
}

char const* ObjectGuid::GetTypeName() const
{
    return !IsEmpty() ? GetTypeName(GetHigh()) : "None";
}

std::string ObjectGuid::ToString() const
{
    std::ostringstream str;
    str << "GUID Full: 0x" << std::hex << std::setw(16) << std::setfill('0') << _low << std::dec;
    str << " Type: " << GetTypeName();
    if (HasEntry())
        str << (IsPet() ? " Pet number: " : " Entry: ") << GetEntry() << " ";

    str << " Low: " << GetCounter();
    return str.str();
}

std::size_t ObjectGuid::GetHash() const
{
    std::size_t hashVal = 0;
    Trinity::hash_combine(hashVal, _low);
    Trinity::hash_combine(hashVal, _high);
    return hashVal;
}

bool ObjectGuid::HasMapId(HighGuid high)
{
    switch (high)
    {
    case HighGuid::Conversation:
    case HighGuid::Creature:
    case HighGuid::Vehicle:
    case HighGuid::Pet:
    case HighGuid::GameObject:
    case HighGuid::DynamicObject:
    case HighGuid::AreaTrigger:
    case HighGuid::Corpse:
    case HighGuid::LootObject:
    case HighGuid::SceneObject:
    case HighGuid::Scenario:
    case HighGuid::AIGroup:
    case HighGuid::DynamicDoor:
    case HighGuid::Vignette:
    case HighGuid::CallForHelp:
    case HighGuid::AIResource:
    case HighGuid::AILock:
    case HighGuid::AILockTicket:
    case HighGuid::Cast:
    case HighGuid::EventObject:
        return true;
    default:
        return false;
    }
}

bool ObjectGuid::HasEntry(HighGuid high)
{
    switch (high)
    {
    case HighGuid::GameObject:
    case HighGuid::Creature:
    case HighGuid::Pet:
    case HighGuid::Vehicle:
    default:
        return true;
    }
}

ObjectGuid::ObjectGuid(): _low(0), _high(0) { }

void ObjectGuid::SetRawValue(uint64 high, uint64 low)
{
    _high = high;
    _low = low;
}

void ObjectGuid::Clear()
{
    _high = 0;
    _low = 0;
}

HighGuid ObjectGuid::GetHigh() const
{
    return HighGuid((_high >> 58) & 0x3F);
}

uint32 ObjectGuid::GetRealmId() const
{
    return uint32((_high >> 42) & 0x1FFF);
}

uint32 ObjectGuid::GetMapId() const
{
    return uint32((_high >> 29) & 0x1FFF);
}

uint32 ObjectGuid::GetEntry() const
{
    return uint32((_high >> 6) & 0x7FFFFF);
}

uint8 ObjectGuid::GetSubType() const
{
    return uint8(_high & 0x3F);
}

ObjectGuid::LowType ObjectGuid::GetCounter() const
{
    return _low & UI64LIT(0x000000FFFFFFFFFF);
}

uint64 ObjectGuid::GetHighPart() const
{
    return _high;
}

uint64 ObjectGuid::GetLowPart() const
{
    return _low;
}

uint32 ObjectGuid::GetGUIDLow() const
{
    return PAIR64_LOPART(_low);
}

uint32 ObjectGuid::GetGUIDHigh() const
{
    return PAIR64_HIPART(_high);
}

ObjectGuid::LowType ObjectGuid::GetMaxCounter(HighGuid)
{
    return UI64LIT(0xFFFFFFFFFF);
}

uint32 ObjectGuid::GetMaxCounter() const
{
    return GetMaxCounter(GetHigh());
}

bool ObjectGuid::IsEmpty() const
{
    return _low == 0 && _high == 0;
}

bool ObjectGuid::IsCreature() const
{
    return GetHigh() == HighGuid::Creature;
}

bool ObjectGuid::IsPet() const
{
    return GetHigh() == HighGuid::Pet;
}

bool ObjectGuid::IsVehicle() const
{
    return GetHigh() == HighGuid::Vehicle;
}

bool ObjectGuid::IsCreatureOrPet() const
{
    return IsCreature() || IsPet();
}

bool ObjectGuid::IsCreatureOrVehicle() const
{
    return IsCreature() || IsVehicle();
}

bool ObjectGuid::IsCreatureOrPetOrVehicle() const
{
    return IsCreature() || IsVehicle() || IsPet();
}

bool ObjectGuid::IsAnyTypeCreature() const
{
    return IsCreature() || IsPet() || IsVehicle();
}

bool ObjectGuid::IsPlayer() const
{
    return !IsEmpty() && GetHigh() == HighGuid::Player;
}

bool ObjectGuid::IsUnit() const
{
    return IsAnyTypeCreature() || IsPlayer();
}

bool ObjectGuid::IsItem() const
{
    return GetHigh() == HighGuid::Item;
}

bool ObjectGuid::IsGameObject() const
{
    return GetHigh() == HighGuid::GameObject;
}

bool ObjectGuid::IsDynamicObject() const
{
    return GetHigh() == HighGuid::DynamicObject;
}

bool ObjectGuid::IsCorpse() const
{
    return GetHigh() == HighGuid::Corpse;
}

bool ObjectGuid::IsAreaTrigger() const
{
    return GetHigh() == HighGuid::AreaTrigger;
}

bool ObjectGuid::IsLoot() const
{
    return GetHigh() == HighGuid::LootObject;
}

bool ObjectGuid::IsMOTransport() const
{
    return GetHigh() == HighGuid::Transport;
}

bool ObjectGuid::IsAnyTypeGameObject() const
{
    return IsGameObject() || IsMOTransport();
}

bool ObjectGuid::IsParty() const
{
    return GetHigh() == HighGuid::Party;
}

bool ObjectGuid::IsGuild() const
{
    return GetHigh() == HighGuid::Guild;
}

bool ObjectGuid::IsSceneObject() const
{
    return GetHigh() == HighGuid::SceneObject;
}

bool ObjectGuid::IsConversation() const
{
    return GetHigh() == HighGuid::Conversation;
}

bool ObjectGuid::IsSpell() const
{
    return GetHigh() == HighGuid::Spell;
}

bool ObjectGuid::IsCast() const
{
    return GetHigh() == HighGuid::Cast;
}

bool ObjectGuid::IsEvent() const
{
    return GetHigh() == HighGuid::EventObject;
}

bool ObjectGuid::HasMapId() const
{
    return HasMapId(GetHigh());
}

bool ObjectGuid::HasEntry() const
{
    return HasEntry(GetHigh());
}

ObjectGuid::ObjectGuid(uint64 high, uint64 low): _low(low), _high(high) { }

std::vector<uint8> ObjectGuid::GetRawValue() const
{
    std::vector<uint8> raw(16);
    memcpy(raw.data(), this, sizeof(*this));
    return raw;
}

void ObjectGuid::SetRawValue(std::vector<uint8> const& guid)
{
    ASSERT(guid.size() == sizeof(*this));
    memcpy(this, guid.data(), sizeof(*this));
}

ByteBuffer& operator<<(ByteBuffer& buf, ObjectGuid const& guid)
{
    uint8 lowMask = 0;
    uint8 highMask = 0;
    buf.FlushBits();
    std::size_t pos = buf.wpos();
    buf << uint8(lowMask);
    buf << uint8(highMask);

    uint8 packed[8];
    if (size_t packedSize = ByteBuffer::PackUInt64(guid._low, &lowMask, packed))
        buf.append(packed, packedSize);
    if (size_t packedSize = ByteBuffer::PackUInt64(guid._high, &highMask, packed))
        buf.append(packed, packedSize);

    buf.put(pos, lowMask);
    buf.put(pos + 1, highMask);

    return buf;
}

template<HighGuid high>
ObjectGuid::LowType ObjectGuidGenerator<high>::Generate()
{
    if (_nextGuid >= ObjectGuid::GetMaxCounter(high) - 1)
    {
        //TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "%s guid overflow!! Can't continue, shutting down server. ", ObjectGuid::GetTypeName(high));
        World::StopNow(ERROR_EXIT_CODE);
    }

    ObjectAccessor::SetGuidSize(high, _nextGuid + 1);

    return _nextGuid++;
}

template<HighGuid high>
uint32 ObjectGuidGenerator<high>::GenerateLow()
{
    if (_nextGuid >= ObjectGuid::GetMaxCounter(high) - 1)
    {
        //TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "%s guid overflow!! Can't continue, shutting down server. ", ObjectGuid::GetTypeName(high));
        World::StopNow(ERROR_EXIT_CODE);
    }

    return static_cast<uint32>(_nextGuid++);
}

ByteBuffer& operator>>(ByteBuffer& buf, ObjectGuid& guid)
{
    uint8 lowMask, highMask;
    buf >> lowMask >> highMask;
    buf.ReadPackedUInt64(lowMask, guid._low);
    buf.ReadPackedUInt64(highMask, guid._high);
    return buf;
}

std::ostream& operator<<(std::ostream& stream, ObjectGuid const& guid)
{
    std::ostringstream tmp;
    tmp << std::hex << std::setw(16) << std::setfill('0') << guid._high << std::setw(16) << std::setfill('0') << guid._low;
    stream << tmp.str();
    return stream;
}

class GuidFormat
{
public:
    static ObjectGuid Global(HighGuid type, ObjectGuid::LowType counter)
    {
        return ObjectGuid(uint64(uint64(type) << 58), counter);
    }

    static ObjectGuid RealmSpecific(HighGuid type, ObjectGuid::LowType counter)
    {
        return ObjectGuid(uint64(uint64(type) << 58 | uint64(realm.Id.Realm) << 42), counter);
    }

    static ObjectGuid MapSpecific(HighGuid type, uint8 subType, uint16 mapId, uint32 serverId, uint32 entry, ObjectGuid::LowType counter)
    {
        return ObjectGuid(uint64((uint64(type) << 58) | (uint64(realm.Id.Realm & 0x1FFF) << 42) | (uint64(mapId & 0x1FFF) << 29) | (uint64(entry & 0x7FFFFF) << 6) | (uint64(subType) & 0x3F)),
            uint64((uint64(serverId & 0xFFFFFF) << 40) | (counter & UI64LIT(0xFFFFFFFFFF))));
    }
};

#define GLOBAL_GUID_CREATE(highguid) template<> ObjectGuid ObjectGuid::Create<highguid>(LowType counter) { return GuidFormat::Global(highguid, counter); }
#define REALM_GUID_CREATE(highguid) template<> ObjectGuid ObjectGuid::Create<highguid>(LowType counter) { return GuidFormat::RealmSpecific(highguid, counter); }
#define MAP_GUID_CREATE(highguid) template<> ObjectGuid ObjectGuid::Create<highguid>(uint16 mapId, uint32 entry, LowType counter, uint8 subType) { return GuidFormat::MapSpecific(highguid, subType, mapId, 0, entry, counter); }
#define GLOBAL_GUID_WITH_SUBTYPE(highguid) template<> ObjectGuid ObjectGuid::Create<highguid>(LowType counter, uint8 subType) { return GuidFormat::MapSpecific(highguid, subType, 0, 0, 0, counter); }

GLOBAL_GUID_WITH_SUBTYPE(HighGuid::Party)
GLOBAL_GUID_CREATE(HighGuid::Uniq)
GLOBAL_GUID_CREATE(HighGuid::WowAccount)
GLOBAL_GUID_CREATE(HighGuid::BNetAccount)
GLOBAL_GUID_CREATE(HighGuid::GMTask)
GLOBAL_GUID_CREATE(HighGuid::RaidGroup)
GLOBAL_GUID_CREATE(HighGuid::Spell)
GLOBAL_GUID_CREATE(HighGuid::Mail)
GLOBAL_GUID_CREATE(HighGuid::UserRouter)
GLOBAL_GUID_CREATE(HighGuid::PVPQueueGroup)
GLOBAL_GUID_CREATE(HighGuid::UserClient)
GLOBAL_GUID_CREATE(HighGuid::BattlePet)
GLOBAL_GUID_CREATE(HighGuid::PetBattle)
GLOBAL_GUID_CREATE(HighGuid::LFGList)
GLOBAL_GUID_CREATE(HighGuid::LFGObject)
REALM_GUID_CREATE(HighGuid::Player)
REALM_GUID_CREATE(HighGuid::Item)   // This is not exactly correct, there are 2 more unknown parts in highguid: (high >> 10 & 0xFF), (high >> 18 & 0xFFFFFF)
REALM_GUID_CREATE(HighGuid::Transport)
REALM_GUID_CREATE(HighGuid::Guild)
MAP_GUID_CREATE(HighGuid::Conversation)
MAP_GUID_CREATE(HighGuid::Creature)
MAP_GUID_CREATE(HighGuid::Vehicle)
MAP_GUID_CREATE(HighGuid::Pet)
MAP_GUID_CREATE(HighGuid::GameObject)
MAP_GUID_CREATE(HighGuid::DynamicObject)
MAP_GUID_CREATE(HighGuid::AreaTrigger)
MAP_GUID_CREATE(HighGuid::Corpse)
MAP_GUID_CREATE(HighGuid::LootObject)
MAP_GUID_CREATE(HighGuid::SceneObject)
MAP_GUID_CREATE(HighGuid::Scenario)
MAP_GUID_CREATE(HighGuid::AIGroup)
MAP_GUID_CREATE(HighGuid::DynamicDoor)
MAP_GUID_CREATE(HighGuid::Vignette)
MAP_GUID_CREATE(HighGuid::CallForHelp)
MAP_GUID_CREATE(HighGuid::AIResource)
MAP_GUID_CREATE(HighGuid::AILock)
MAP_GUID_CREATE(HighGuid::AILockTicket)
MAP_GUID_CREATE(HighGuid::Cast)
MAP_GUID_CREATE(HighGuid::EventObject)
MAP_GUID_CREATE(HighGuid::PVPQueueGroup)

ObjectGuid const ObjectGuid::Empty = ObjectGuid();
ObjectGuid const ObjectGuid::TradeItem = ObjectGuid::Create<HighGuid::Uniq>(uint64(10));

template<HighGuid type>
ObjectGuid ObjectGuid::Create(LowType /*counter*/)
{
    static_assert(type == HighGuid::Count, "This guid type cannot be constructed using Create(LowType counter).");
}

template<HighGuid type>
ObjectGuid ObjectGuid::Create(uint16 /*mapId*/, uint32 /*entry*/, LowType /*counter*/, uint8 /*subType = 0*/)
{
    static_assert(type == HighGuid::Count, "This guid type cannot be constructed using Create(uint16 mapId, uint32 entry, LowType counter, uint8 subType).");
}

template<HighGuid type>
ObjectGuid ObjectGuid::Create(LowType /*counter*/, uint8 /*subType = 0*/)
{
    static_assert(type == HighGuid::Count, "This guid type cannot be constructed using Create(LowType counter, uint8 subType).");
}

template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Player>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Creature>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Pet>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Vehicle>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Item>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::GameObject>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::DynamicObject>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Corpse>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::LootObject>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::AreaTrigger>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Transport>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::BattlePet>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::PetBattle>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Conversation>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Cast>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::EventObject>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Scenario>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::Vignette>::Generate();
template ObjectGuid::LowType ObjectGuidGenerator<HighGuid::LFGList>::Generate();
template uint32 ObjectGuidGenerator<HighGuid::LFGObject>::GenerateLow();
