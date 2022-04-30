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

#ifndef _OBJECT_H
#define _OBJECT_H

#include "Common.h"
#include "UpdateFields.h"
#include "UpdateData.h"
#include "ObjectDefines.h"
#include "GridDefines.h"
#include "Map.h"
#include "MovementInfo.h"

enum TempSummonType : uint8
{
    TEMPSUMMON_TIMED_OR_DEAD_DESPAWN       = 1,             // despawns after a specified time OR when the creature disappears
    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN     = 2,             // despawns after a specified time OR when the creature dies
    TEMPSUMMON_TIMED_DESPAWN               = 3,             // despawns after a specified time
    TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT = 4,             // despawns after a specified time after the creature is out of combat
    TEMPSUMMON_CORPSE_DESPAWN              = 5,             // despawns instantly after death
    TEMPSUMMON_CORPSE_TIMED_DESPAWN        = 6,             // despawns after a specified time after death
    TEMPSUMMON_DEAD_DESPAWN                = 7,             // despawns when the creature disappears
    TEMPSUMMON_MANUAL_DESPAWN              = 8              // despawns when UnSummon() is called
};

enum SummonerType
{
    SUMMONER_TYPE_CREATURE      = 0,
    SUMMONER_TYPE_GAMEOBJECT    = 1,
    SUMMONER_TYPE_MAP           = 2
};

enum PhaseMasks
{
    PHASEMASK_NORMAL   = 0x00000001,
    PHASEMASK_ANYWHERE = 0xFFFFFFFF
};

enum NotifyFlags
{
    NOTIFY_NONE                     = 0x00,
    NOTIFY_AI_RELOCATION            = 0x01,
    NOTIFY_VISIBILITY_CHANGED       = 0x02,
    NOTIFY_ALL                      = 0xFF
};

class WorldPacket;
class UpdateData;
class ByteBuffer;
class WorldSession;
class Creature;
class Player;
class InstanceScript;
class Item;
class GameObject;
class TempSummon;
class Vehicle;
class CreatureAI;
class ZoneScript;
class Unit;
class Transport;

/// Key for AccessRequirement
struct AccessRequirementKey
{
    AccessRequirementKey(int32 mapId, uint8 difficulty, uint16 dungeonId);

    bool operator<(AccessRequirementKey const& rhs) const;

private:
    int32 _mapId;
    uint8 _difficulty;
    uint16 _dungeonId;
};

struct ObjectInvisibility final
{
    ObjectInvisibility(InvisibilityType t, int32 a) : type(t) , amount(a) { }

    InvisibilityType type;
    int32 amount;
};

typedef std::unordered_map<Player*, UpdateData> UpdateDataMapType;

namespace UpdateMask
{
    typedef uint32 BlockType;

    enum DynamicFieldChangeType : uint16
    {
        UNCHANGED               = 0,
        VALUE_CHANGED           = 0x7FFF,
        VALUE_AND_SIZE_CHANGED  = 0x8000
    };

    inline std::size_t GetBlockCount(std::size_t bitCount)
    {
        using BitsPerBlock = std::integral_constant<std::size_t, sizeof(BlockType) * 8>;
        return (bitCount + BitsPerBlock::value - 1) / BitsPerBlock::value;
    }

    inline std::size_t EncodeDynamicFieldChangeType(std::size_t blockCount, DynamicFieldChangeType changeType, uint8 updateType)
    {
        return blockCount | ((changeType & VALUE_AND_SIZE_CHANGED) * ((3 - updateType /*this part evaluates to 0 if update type is not VALUES*/) / 3));
    }

    template<typename T>
    void SetUpdateBit(T* data, std::size_t bitIndex)
    {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "Type used for SetUpdateBit data arg is not an unsigned integer");
        using BitsPerBlock = std::integral_constant<std::size_t, sizeof(T) * 8>;
        data[bitIndex / BitsPerBlock::value] |= T(1) << (bitIndex % BitsPerBlock::value);
    }
}

// Helper class used to iterate object dynamic fields while interpreting them as a structure instead of raw int array
template<class T>
class DynamicFieldStructuredView
{
public:
    explicit DynamicFieldStructuredView(std::vector<uint32> const& data) : _data(data) { }

    T const* begin() const
    {
        return reinterpret_cast<T const*>(_data.data());
    }

    T const* end() const
    {
        return reinterpret_cast<T const*>(_data.data() + _data.size());
    }

    std::size_t size() const
    {
        using BlockCount = std::integral_constant<uint16, sizeof(T) / sizeof(uint32)>;
        return _data.size() / BlockCount::value;
    }

    T const * operator[](int n) const
    {
        if (size() <= n)
            return nullptr;

        using BlockCount = std::integral_constant<uint16, sizeof(T) / sizeof(uint32)>;
        return reinterpret_cast<T const*>(_data.data() + BlockCount::value*n);
    }

private:
    std::vector<uint32> const& _data;
};

class Object
{
    public:
        virtual ~Object();

        bool IsInWorld() const;
        bool IsDelete() const;
        bool IsPreDelete() const;
        bool IsUpdated() const;

        virtual void AddToWorld();
        virtual void RemoveFromWorld();
        virtual void Clear();
        virtual uint32 GetSize();

        ObjectGuid const& GetGUID() const { return GetGuidValue(OBJECT_FIELD_GUID); }
        uint32 GetGUIDLow() const { return GetGUID().GetGUIDLow(); }
        uint32 GetGUIDHigh() const { return GetGUID().GetGUIDHigh(); }
        uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY_ID); }
        void SetEntry(uint32 entry) { SetUInt32Value(OBJECT_FIELD_ENTRY_ID, entry); }

        float GetObjectScale() const { return GetFloatValue(OBJECT_FIELD_SCALE); }
        virtual void SetObjectScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE, scale); }

        TypeID GetTypeId() const { return m_objectTypeId; }
        bool isType(uint16 mask) const { return (mask & m_objectType) != 0; }

        virtual void BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        void SendUpdateToPlayer(Player* player);

        void BuildValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        void BuildOutOfRangeUpdateBlock(UpdateData* data) const;

        virtual void DestroyForPlayer(Player* target) const;

        int32 GetInt32Value(uint16 index) const;
        uint32 GetUInt32Value(uint16 index) const;
        uint64 GetUInt64Value(uint16 index) const;
        float GetFloatValue(uint16 index) const;
        uint8 GetByteValue(uint16 index, uint8 offset) const;
        uint16 GetUInt16Value(uint16 index, uint8 offset) const;
        ObjectGuid const& GetGuidValue(uint16 index) const;
        void SetInt32Value(uint16 index, int32 value);
        void UpdateInt32Value(uint16 index, int32 value);
        void SetUInt32Value(uint16 index, uint32 value);
        void UpdateUInt32Value(uint16 index, uint32 value);
        void SetUInt64Value(uint16 index, uint64 value);
        void SetFloatValue(uint16 index, float value);
        void SetByteValue(uint16 index, uint8 offset, uint8 value);
        void SetUInt16Value(uint16 index, uint8 offset, uint16 value, bool update = true);
        void SetInt16Value(uint16 index, uint8 offset, int16 value) { SetUInt16Value(index, offset, static_cast<uint16>(value)); }
        void SetGuidValue(uint16 index, ObjectGuid const& value);
        void SetStatFloatValue(uint16 index, float value);
        void SetStatInt32Value(uint16 index, int32 value);

        bool AddGuidValue(uint16 index, ObjectGuid const& value);
        bool RemoveGuidValue(uint16 index, ObjectGuid const& value);

        void ApplyModUInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModUInt16Value(uint16 index, uint8 offset, int16 val, bool apply);
        void ApplyModPositiveFloatValue(uint16 index, float val, bool apply);
        void ApplyModSignedFloatValue(uint16 index, float val, bool apply);
        void ApplyPercentModFloatValue(uint16 index, float val, bool apply);

        void SetFlag(uint16 index, uint32 newFlag);
        void RemoveFlag(uint16 index, uint32 oldFlag);
        void ToggleFlag(uint16 index, uint32 flag);
        bool HasFlag(uint16 index, uint32 flag) const;
        void ApplyModFlag(uint16 index, uint32 flag, bool apply);

        void SetByteFlag(uint16 index, uint8 offset, uint8 newFlag);
        void RemoveByteFlag(uint16 index, uint8 offset, uint8 newFlag);
        void ToggleByteFlag(uint16 index, uint8 offset, uint8 flag);
        bool HasByteFlag(uint16 index, uint8 offset, uint8 flag) const;

        void SetFlag64(uint16 index, uint64 newFlag);
        void RemoveFlag64(uint16 index, uint64 oldFlag);
        void ToggleFlag64(uint16 index, uint64 flag);
        bool HasFlag64(uint16 index, uint64 flag) const;
        void ApplyModFlag64(uint16 index, uint64 flag, bool apply);

        void ClearUpdateMask(bool remove);

        uint16 GetValuesCount() const { return m_valuesCount; }
        uint16 GetDynamicValuesCount() const { return _dynamicValuesCount; }

        std::vector<uint32> const& GetDynamicValues(uint16 index) const;
        uint32 GetDynamicValue(uint16 index, uint16 offset) const;
        void AddDynamicValue(uint16 index, uint32 value);
        void RemoveDynamicValue(uint16 index, uint32 value);
        void ClearDynamicValue(uint16 index);
        void SetDynamicValue(uint16 index, uint16 offset, uint32 value);

        template<class T>
        DynamicFieldStructuredView<T> GetDynamicStructuredValues(uint16 index) const
        {
            static_assert(std::is_standard_layout<T>::value && std::is_trivially_destructible<T>::value, "T used for Object::SetDynamicStructuredValue<T> is not a trivially destructible standard layout type");
            using BlockCount = std::integral_constant<uint16, sizeof(T) / sizeof(uint32)>;
            ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
            std::vector<uint32> const& values = _dynamicValues[index];
            ASSERT((values.size() % BlockCount::value) == 0, "Dynamic field value count must exactly fit into structure");
            return DynamicFieldStructuredView<T>(values);
        }

        template<class T>
        T const* GetDynamicStructuredValue(uint16 index, uint16 offset) const
        {
            static_assert(std::is_standard_layout<T>::value && std::is_trivially_destructible<T>::value, "T used for Object::SetDynamicStructuredValue<T> is not a trivially destructible standard layout type");
            using BlockCount = std::integral_constant<uint16, sizeof(T) / sizeof(uint32)>;
            ASSERT(index < _dynamicValuesCount || PrintIndexError(index, false));
            std::vector<uint32> const& values = _dynamicValues[index];
            ASSERT((values.size() % BlockCount::value) == 0, "Dynamic field value count must exactly fit into structure");
            if (offset * BlockCount::value >= values.size())
                return nullptr;
            return reinterpret_cast<T const*>(&values[offset * BlockCount::value]);
        }

        template<class T>
        void SetDynamicStructuredValue(uint16 index, uint16 offset, T const* value)
        {
            static_assert(std::is_standard_layout<T>::value && std::is_trivially_destructible<T>::value, "T used for Object::SetDynamicStructuredValue<T> is not a trivially destructible standard layout type");
            using BlockCount = std::integral_constant<uint16, sizeof(T) / sizeof(uint32)>;
            SetDynamicValue(index, (offset + 1) * BlockCount::value - 1, 0); // reserve space
            for (uint16 i = 0; i < BlockCount::value; ++i)
                SetDynamicValue(index, offset * BlockCount::value + i, *(reinterpret_cast<uint32 const*>(value) + i));
        }

        template<class T>
        void AddDynamicStructuredValue(uint16 index, T const* value)
        {
            static_assert(std::is_standard_layout<T>::value && std::is_trivially_destructible<T>::value, "T used for Object::SetDynamicStructuredValue<T> is not a trivially destructible standard layout type");
            using BlockCount = std::integral_constant<uint16, sizeof(T) / sizeof(uint32)>;
            std::vector<uint32> const& values = _dynamicValues[index];
            auto offset = uint16(values.size() / BlockCount::value);
            SetDynamicValue(index, (offset + 1) * BlockCount::value - 1, 0); // reserve space
            for (uint16 i = 0; i < BlockCount::value; ++i)
                SetDynamicValue(index, offset * BlockCount::value + i, *(reinterpret_cast<uint32 const*>(value) + i));
        }

        virtual bool hasQuest(uint32 /* quest_id */) const { return false; }
        virtual bool hasInvolvedQuest(uint32 /* quest_id */) const { return false; }
        virtual void BuildUpdate(UpdateDataMapType&) {}

        void BuildFieldsUpdate(Player*, UpdateDataMapType &) const;

        void SetFieldNotifyFlag(uint16 flag) { _fieldNotifyFlags |= flag; }
        void RemoveFieldNotifyFlag(uint16 flag) { _fieldNotifyFlags &= ~flag; }

        // FG: some hacky helpers
        void ForceValuesUpdateAtIndex(uint32);
        
        bool IsPlayer() const { return GetTypeId() == TYPEID_PLAYER; }
        Player* ToPlayer() { if (IsPlayer()) return reinterpret_cast<Player*>(this); return nullptr; }
        Player const* ToPlayer() const { if (IsPlayer()) return reinterpret_cast<Player const*>(this); return nullptr; }

        bool IsCreature() const { return GetTypeId() == TYPEID_UNIT; }
        Creature* ToCreature() { if (IsCreature()) return reinterpret_cast<Creature*>(this); return nullptr; }
        Creature const* ToCreature() const { if (IsCreature()) return reinterpret_cast<Creature const*>(this); return nullptr; }

        bool IsUnit() const { return isType(TYPEMASK_UNIT); }
        Unit* ToUnit() { if (IsUnit()) return reinterpret_cast<Unit*>(this); return nullptr; }
        Unit const* ToUnit() const { if (IsUnit()) return reinterpret_cast<Unit const*>(this); return nullptr; }

        bool IsGameObject() const { return GetTypeId() == TYPEID_GAMEOBJECT; }
        GameObject* ToGameObject() { if (IsGameObject()) return reinterpret_cast<GameObject*>(this); return nullptr; }
        GameObject const* ToGameObject() const { if (IsGameObject()) return reinterpret_cast<GameObject const*>(this); return nullptr; }

        bool IsCorpse() const { return GetTypeId() == TYPEID_CORPSE; }
        Corpse* ToCorpse() { if (IsCorpse()) return reinterpret_cast<Corpse*>(this); return nullptr; }
        Corpse const* ToCorpse() const { if (IsCorpse()) return reinterpret_cast<Corpse const*>(this); return nullptr; }

        bool IsItem() const { return GetTypeId() == TYPEID_ITEM; }
        Item* ToItem() { if (IsItem()) return reinterpret_cast<Item*>(this); return nullptr; }
        Item const* ToItem() const { if (IsItem()) return reinterpret_cast<Item const*>(this); return nullptr; }

        bool IsDynObject() const { return GetTypeId() == TYPEID_DYNAMICOBJECT; }
        DynamicObject* ToDynObject() { if (IsDynObject()) return reinterpret_cast<DynamicObject*>(this); return nullptr; }
        DynamicObject const* ToDynObject() const { if (IsDynObject()) return reinterpret_cast<DynamicObject const*>(this); return nullptr; }

        bool IsAreaTrigger() const { return GetTypeId() == TYPEID_AREATRIGGER; }
        AreaTrigger* ToAreaTrigger() { if (IsAreaTrigger()) return reinterpret_cast<AreaTrigger*>(this); return nullptr; }
        AreaTrigger const* ToAreaTrigger() const { if (IsAreaTrigger()) return reinterpret_cast<AreaTrigger const*>(this); return nullptr; }

        bool IsEventObject() const { return GetTypeId() == TYPEID_EVENTOBJECT; }
        EventObject* ToEventObject() { if (IsEventObject()) return reinterpret_cast<EventObject*>(this); return nullptr; }
        EventObject const* ToEventObject() const { if (IsEventObject()) return reinterpret_cast<EventObject const*>(this); return nullptr; }

        WorldObject* ToWorldObject() { return reinterpret_cast<WorldObject*>(this); }
        WorldObject const* ToWorldObject() const { return reinterpret_cast<WorldObject const*>(this); }

        Map* m_currMap;                                    //current object's Map location

        virtual void SetMap(Map* map);
        virtual void ResetMap();
        Map* GetMap() const { return m_currMap; }
        Map* FindMap() const { return m_currMap; }
        //used to check all object's GetMap() calls when object is not in world!

        void SetDelete() { m_delete = true; }
        void SetPreDelete() { m_preDelete = true; }
        void SetObjectUpdated(bool update = false) { m_objectUpdated = update; }

        bool MaxVisible;

        uint8 GetSpawnMode() const { return m_spawnMode; }

        bool m_Teleports;
        std::atomic<bool> m_isUpdate;

    protected:
        Object();

        void _InitValues();
        void _Create(ObjectGuid const& guid);
        std::string _ConcatFields(uint16 startIndex, uint16 size) const;
        void _LoadIntoDataField(std::string const& data, uint32 startOffset, uint32 count);

        uint32 GetUpdateFieldData(Player const* target, uint32*& flags) const;
        uint32 GetDynamicUpdateFieldData(Player const* target, uint32*& flags) const;

        void BuildMovementUpdate(ByteBuffer * data, uint16 flags) const;
        virtual void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const;
        virtual void BuildDynamicValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const;

        uint8 m_spawnMode;

        uint16 m_objectType;

        TypeID m_objectTypeId;
        uint16 m_updateFlag;

        union
        {
            int32  *m_int32Values;
            uint32 *m_uint32Values;
            float  *m_floatValues;
        };

        std::vector<uint32>* _dynamicValues;

        std::vector<uint8> _changesMask;
        std::vector<UpdateMask::DynamicFieldChangeType> _dynamicChangesMask;
        std::vector<uint8>* _dynamicChangesArrayMask;

        uint16 m_valuesCount;
        uint16 _dynamicValuesCount;

        uint16 _fieldNotifyFlags;

        virtual void AddToObjectUpdateIfNeeded();

        std::atomic<bool> m_objectUpdated;

        std::atomic<uint64> m_inWorld;
        std::atomic<bool> m_delete;
        std::atomic<bool> m_preDelete;
        std::atomic<bool> m_needLock;
        std::recursive_mutex m_update_lock;

    private:
        // for output helpfull error messages from asserts
        bool PrintIndexError(uint32 index, bool set) const;
        Object(const Object&) = delete;                              // prevent generation copy constructor
        Object& operator=(Object const&) = delete;                   // prevent generation assigment operator
};

typedef cyber_ptr<WorldObject> C_PTR;

class WorldObject : public Object, public WorldLocation
{
    protected:
        explicit WorldObject(bool isWorldObject); //note: here it means if it is in grid object list or world object list
    public:
        virtual ~WorldObject();

        virtual void Update (uint32 /*time_diff*/) { }

        void Clear();

        void Relocate(float x, float y, float z, float orientation) override;
        void Relocate(float x, float y, float z) override;
        void Relocate(float x, float y) override;
        void Relocate(const Position &pos) override;
        void Relocate(const Position* pos) override;

        void SetOrientation(float orientation);

        virtual void RemoveFromWorld();

        void GetNearPoint2D(float &x, float &y, float distance, float absAngle, bool allowObjectSize = true) const;
        void GetNearPoint2D(Position &pos, float distance, float angle, bool allowObjectSize = true) const;
        void GetNearPoint(WorldObject const* searcher, float &x, float &y, float &z, float searcher_size, float distance2d, float absAngle) const;
        void GetClosePoint(float& x, float& y, float& z, float size, float distance2d = 0, float angle = 0) const;
        void MovePosition(Position &pos, float dist, float angle);
        void GetNearPosition(Position& pos, float dist, float angle);
        void MovePositionToFirstCollision(Position &pos, float dist, float angle);
        void MovePositionToTransportCollision(Position &pos, float dist, float angle);
        void GetFirstCollisionPosition(Position& pos, float dist, float angle);
        void MovePositionToCollisionBetween(Position &pos, float distMin, float distMax, float angle);
        void GetCollisionPositionBetween(Position& pos, float distMin, float distMax, float angle);
        void GetRandomNearPosition(Position& pos, float radius);
        void GetContactPoint(const WorldObject* obj, float& x, float& y, float& z, float distance2d = CONTACT_DISTANCE) const;
        void GenerateCollisionNonDuplicatePoints(std::list<Position>& randPosList, uint8 maxPoint, float randMin, float randMax, float minDist);

        float GetObjectSize() const;
        void UpdateGroundPositionZ(float x, float y, float &z) const;
        void UpdateAllowedPositionZ(float x, float y, float &z) const;
        virtual bool IsInWater() const { return false; }
        virtual bool IsUnderWater() const { return false; }

        void GetRandomPoint(const Position &srcPos, float distance, float &rand_x, float &rand_y, float &rand_z) const;
        void GetRandomPoint(Position const& srcPos, float distance, Position& pos) const;

        uint32 GetInstanceId() const { return m_InstanceId; }
        bool InInstance() const { return m_currMap && m_currMap->Instanceable(); }

        virtual void SetPhaseMask(uint32 newPhaseMask, bool update);
        uint32 GetPhaseMask() const { return m_phaseMask; }
        bool RemovePhase(uint32 PhaseID);
        bool InSamePhase(WorldObject const* obj) const;
        bool InSamePhase(uint32 phasemask) const { return (GetPhaseMask() & phasemask) != 0; }

        virtual void SetPhaseId(std::set<uint32> const& newPhaseId, bool update);
        bool HasPhaseId(uint32 PhaseID) const;
        std::set<uint32> const& GetPhases() const;
        bool InSamePhaseId(WorldObject const* obj) const;
        bool InSamePhaseId(std::set<uint32> const& phase, bool otherIsPlayer) const;
        void RebuildTerrainSwaps();
        void RebuildWorldMapAreaSwaps();
        std::set<uint32> const& GetTerrainSwaps() const { return _terrainSwaps; }
        std::set<uint32> const& GetWorldMapAreaSwaps() const { return _worldMapAreaSwaps; }
        bool IsInTerrainSwap(uint32 terrainSwap) const { return _terrainSwaps.find(terrainSwap) != _terrainSwaps.end(); }

        void setIgnorePhaseIdCheck(bool apply)  { m_ignorePhaseIdCheck = apply; }
        bool IgnorePhaseId() const { return m_ignorePhaseIdCheck; }

        bool m_zoneForce;
        uint32 m_zoneId;
        uint32 m_oldZoneId;
        uint32 m_areaId;
        uint32 m_oldAreaId;

        uint32 GetCurrentAreaID() const { return m_areaId; }
        uint32 GetCurrentZoneID() const { return m_zoneId; }
        uint32 GetOldAreaID() const { return m_oldAreaId; }
        uint32 GetOldZoneID() const { return m_oldZoneId; }
        uint32 GetZoneId() const;
        uint32 GetPZoneId() const;
        uint32 GetAreaId() const;
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid) const;

        InstanceScript* GetInstanceScript();

        const char* GetName() const { return m_name.c_str(); }
        void SetName(std::string const& newname) { m_name=newname; }

        virtual const char* GetNameForLocaleIdx(LocaleConstant /*locale_idx*/) const { return GetName(); }

        float GetDistance(const WorldObject* obj) const;
        float GetDistance(Position const& pos) const;
        float GetDistance(float x, float y, float z) const;
        float GetDistance2d(const WorldObject* obj) const;
        float GetDistance2d(float x, float y) const;
        float GetDistanceZ(const WorldObject* obj) const;
        bool IsSelfOrInSameMap(const WorldObject* obj) const;
        bool IsInMap(const WorldObject* obj) const;
        bool IsWithinDist3d(float x, float y, float z, float dist, bool allowObjectSize = true) const;
        bool IsWithinDist3d(const Position* pos, float dist, bool allowObjectSize = true) const;
        bool IsWithinDist2d(float x, float y, float dist) const;
        bool IsWithinDist2d(const Position* pos, float dist) const;
        bool IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D = true, bool ignoreObjectSize = false) const; // use only if you will sure about placing both object at same map
        bool IsWithinDistInMap(WorldObject const* obj, float dist2compare, bool is3D = true, bool ignoreObjectSize = false) const;
        bool IsWithinLOS(float x, float y, float z) const;
        bool IsWithinLOSInMap(const WorldObject* obj) const;
        bool GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D = true) const;
        bool IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D = true) const;
        bool IsInRange2d(float x, float y, float minRange, float maxRange) const;
        bool IsInRange3d(float x, float y, float z, float minRange, float maxRange) const;
        bool isInFront(WorldObject const* target, float arc = M_PI) const;
        bool isInBack(WorldObject const* target, float arc = M_PI) const;

        float GetWaterOrGroundLevel(float x, float y, float z, float* ground = nullptr, bool swim = false) const;
        float GetWaterOrGroundLevel(Position pos) const { return GetWaterOrGroundLevel(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ()); }
        float GetHeight(float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const;

        bool IsInBetweenShift(const Position* obj1, const Position* obj2, float size, float shift, float angleShift) const;
        bool IsInBetween(const Position* obj1, const Position* obj2, float size = 0) const;
        bool IsInBetween(const WorldObject* obj1, float x2, float y2, float size = 0) const;

        bool IsInAxe(WorldObject const* obj1, WorldObject const* obj2, float size = 0) const;
        bool IsInAxe(WorldObject const* obj, float width, float range) const;

        virtual void CleanupsBeforeDelete(bool finalCleanup = true);  // used in destructor or explicitly before mass creature delete to remove cross-references to already deleted units

        virtual void SendMessageToSet(WorldPacket const* data, bool self, GuidUnorderedSet const& ignoredList = GuidUnorderedSet());
        virtual void SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, GuidUnorderedSet const& ignoredList = GuidUnorderedSet());
        virtual void SendMessageToSet(WorldPacket const* data, Player const* skipped_rcvr, GuidUnorderedSet const& ignoredList = GuidUnorderedSet());

        virtual uint8 getLevelForTarget(WorldObject const* /*target*/) const { return 1; }

        void Talk(std::string const& text, ChatMsg msgType, Language language, float textRange, WorldObject const* target);
        void Talk(uint32 textId, ChatMsg msgType, float textRange, WorldObject const* target);
        void MonsterSay(const char* text, uint32 language, ObjectGuid TargetGuid);
        void MonsterYell(const char* text, uint32 language, ObjectGuid TargetGuid);
        void MonsterTextEmote(const char* text, ObjectGuid TargetGuid, bool IsBossEmote = false);
        void MonsterWhisper(const char* text, ObjectGuid receiver, bool IsBossWhisper = false);
        void MonsterSay(int32 textId, uint32 language, ObjectGuid TargetGuid);
        void MonsterYell(int32 textId, uint32 language, ObjectGuid TargetGuid);
        void MonsterTextEmote(int32 textId, ObjectGuid TargetGuid, bool IsBossEmote = false);
        void MonsterWhisper(int32 textId, ObjectGuid receiver, bool IsBossWhisper = false);
        void MonsterYellToZone(int32 textId, uint32 language, ObjectGuid TargetGuid);

        void PlayDistanceSound(uint32 soundID, Player* target = nullptr);
        void PlayDirectSound(uint32 sound_id, Player* target = nullptr);

        void SendObjectDeSpawnAnim(ObjectGuid guid);

        virtual void SaveRespawnTime() {}
        void AddObjectToRemoveList();

        float GetGridActivationRange() const;
        float GetVisibilityRange() const;
        float GetVisibilityCombatLog() const;
        float GetSightRange(const WorldObject* target = nullptr) const;
        bool canSeeOrDetect(WorldObject const* obj, bool ignoreStealth = false, bool distanceCheck = false) const;

        void SetVisible(bool x);

        FlaggedValuesArray<int32, uint32, StealthType, TOTAL_STEALTH_TYPES> m_stealth;
        FlaggedValuesArray<int32, uint32, StealthType, TOTAL_STEALTH_TYPES> m_stealthDetect;

        FlaggedValuesArray<int32, uint64, InvisibilityType, TOTAL_INVISIBILITY_TYPES> m_invisibility;
        FlaggedValuesArray<int32, uint64, InvisibilityType, TOTAL_INVISIBILITY_TYPES> m_invisibilityDetect;

        FlaggedValuesArray<int32, uint32, ServerSideVisibilityType, TOTAL_SERVERSIDE_VISIBILITY_TYPES> m_serverSideVisibility;
        FlaggedValuesArray<int32, uint32, ServerSideVisibilityType, TOTAL_SERVERSIDE_VISIBILITY_TYPES> m_serverSideVisibilityDetect;

        bool IsRWVisibility();
        float GetRWVisibility();
        void SetRWVisibilityRange(float rwvisible);
        bool m_rwVisibility;
        float m_rwVisibilityRange;
        virtual bool DistanceCheck() { return true; }

        // Low Level Packets
        void SendPlaySound(uint32 Sound, bool OnlySelf);

        void SetMap(Map* map) override;
        void ResetMap() override;
        //this function should be removed in nearest time...
        Map const* GetBaseMap() const;

        void SetZoneScript();
        ZoneScript* GetZoneScript() const { return m_zoneScript; }

        TempSummon* SummonCreature(uint32 id, const Position &pos, TempSummonType spwtype = TEMPSUMMON_MANUAL_DESPAWN, uint32 despwtime = 0, int32 vehId = 0, ObjectGuid viewerGuid = ObjectGuid::Empty, GuidUnorderedSet* viewersList = nullptr) const;
        TempSummon* SummonCreature(uint32 id, const Position &pos, ObjectGuid targetGuid, TempSummonType spwtype, uint32 despwtime, uint32 spellId = 0, SummonPropertiesEntry const* properties = nullptr) const;
        TempSummon* SummonCreature(uint32 id, float x, float y, float z, float ang = 0, TempSummonType spwtype = TEMPSUMMON_MANUAL_DESPAWN, uint32 despwtime = 0, ObjectGuid viewerGuid = ObjectGuid::Empty, GuidUnorderedSet* viewersList = nullptr);
        TempSummon* SummonCreature(uint32 id, TempSummonType spwtype = TEMPSUMMON_DEAD_DESPAWN, uint32 despwtime = 0);

        GameObject* SummonGameObject(uint32 entry, Position pos, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime, ObjectGuid viewerGuid = ObjectGuid::Empty, GuidUnorderedSet* viewersList = nullptr, bool hasCreator = true);
        GameObject* SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime, ObjectGuid viewerGuid = ObjectGuid::Empty, GuidUnorderedSet* viewersList = nullptr, bool hasCreator = true);
        Creature*   SummonTrigger(float x, float y, float z, float ang, uint32 dur, CreatureAI* (*GetAI)(Creature*) = nullptr);

        void GetAttackableUnitListInRange(std::list<Unit*> &list, float fMaxSearchRange, bool aliveOnly = true) const;
        void GetAreaTriggersWithEntryInRange(std::list<AreaTrigger*>& list, uint32 entry, ObjectGuid casterGuid, float fMaxSearchRange) const;
        Creature*   FindNearestCreature(uint32 entry, float range, bool alive = true) const;
        GameObject* FindNearestGameObject(uint32 entry, float range) const;
        Player*     FindNearestPlayer(float range, bool alive = true);
        GameObject* FindNearestGameObjectOfType(GameobjectTypes type, float range) const;

        void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& lList, uint32 uiEntry, float fMaxSearchRange) const;
        void GetNearGameObjectListInGrid(std::list<GameObject*>& lList, float maxSearchRange) const;
        void GetCreatureListWithEntryInGrid(std::list<Creature*>& lList, uint32 uiEntry, float fMaxSearchRange) const;
        void GetCreatureListInGrid(std::list<Creature*>& lList, float fMaxSearchRange) const;
        void GetAreaTriggerListWithEntryInGrid(std::list<AreaTrigger*>& atList, uint32 uiEntry, float fMaxSearchRange) const;
        void GetPlayerListInGrid(std::list<Player*>& lList, float fMaxSearchRange) const;
        void GetAliveCreatureListWithEntryInGrid(std::list<Creature*>& lList, uint32 uiEntry, float fMaxSearchRange) const;
        void GetCorpseCreatureInGrid(std::list<Creature*>& lList, float fMaxSearchRange) const;

        void GetGameObjectListWithEntryInGridAppend(std::list<GameObject*>& lList, uint32 uiEntry, float fMaxSearchRange) const;
        void GetCreatureListWithEntryInGridAppend(std::list<Creature*>& lList, uint32 uiEntry, float fMaxSearchRange) const;

        void DestroyForNearbyPlayers();
        virtual void UpdateObjectVisibility(bool forced = true);
        void BuildUpdate(UpdateDataMapType&) override;
        void DestroyForPlayer(Player* target) const override;

        bool isActiveObject() const { return m_isActive; }
        void setActive(bool isActiveObject);
        void SetWorldObject(bool apply);
        void SetTratsport(Transport* transport, Unit* owner = nullptr);

        Position GetPosition() const override;
        void GetPosition(float& x, float& y, Transport* transport = nullptr) const override;
        void GetPosition(float& x, float& y, float& z, Transport* transport = nullptr) const override;
        void GetPosition(float& x, float& y, float& z, float& o, Transport* transport = nullptr) const override;
        void GetPosition(Position* pos, Transport* transport = nullptr) const override;

        bool IsPermanentWorldObject() const { return m_isWorldObject; }
        bool IsWorldObject() const;

        uint32  LastUsedScriptID;

        // Transports
        Transport* GetTransport() const;
        float GetTransOffsetX() const;
        float GetTransOffsetY() const;
        float GetTransOffsetZ() const;
        float GetTransOffsetO() const;
        Position const& GetTransOffset() const;
        uint32 GetTransTime() const;
        int8 GetTransSeat() const;
        virtual ObjectGuid GetTransGUID()   const;
        void SetTransport(Transport* t);

        MovementInfo m_movementInfo;
        bool m_deleted;
        float GetDistanceToZOnfall();

        virtual float GetStationaryX() const { return GetPositionX(); }
        virtual float GetStationaryY() const { return GetPositionY(); }
        virtual float GetStationaryZ() const { return GetPositionZ(); }
        virtual float GetStationaryO() const { return GetOrientation(); }

        //template<class NOTIFIER> void VisitNearbyObject(const float &radius, NOTIFIER &notifier) const;
        template<class NOTIFIER>
        void VisitNearbyObject(const float &radius, NOTIFIER &notifier) const
        {
            if (IsInWorld())
                GetMap()->VisitAll(GetPositionX(), GetPositionY(), radius, notifier);
        }

        template<class NOTIFIER> void VisitNearbyGridObject(const float &radius, NOTIFIER &notifier) const;
        template<class NOTIFIER> void VisitNearbyWorldObject(const float &radius, NOTIFIER &notifier) const;

        virtual uint16 GetAIAnimKitId() const { return 0; }
        virtual uint16 GetMovementAnimKitId() const { return 0; }
        virtual uint16 GetMeleeAnimKitId() const { return 0; }

        // Personal visibility system
        bool MustBeVisibleOnlyForSomePlayers() const { return !_visibilityPlayerList.empty(); }
        void GetMustBeVisibleForPlayersList(GuidUnorderedSet& playerList) { playerList = _visibilityPlayerList; }
        void ClearVisibleOnlyForSomePlayers()  { _visibilityPlayerList.clear(); }

        bool IsInPersonnalVisibilityList(ObjectGuid const& guid) const;
        void AddPlayerInPersonnalVisibilityList(ObjectGuid  const& guid) { _visibilityPlayerList.insert(guid); }
        void AddPlayersInPersonnalVisibilityList(GuidUnorderedSet const& viewerList);
        void RemovePlayerFromPersonnalVisibilityList(ObjectGuid  const& guid) { _visibilityPlayerList.erase(guid); }

        bool HideForSomePlayers() const { return !_hideForGuid.empty(); }
        void AddToHideList(ObjectGuid  const& guid) { _hideForGuid.insert(guid); }
        bool ShouldHideFor(ObjectGuid const& guid) const { return _hideForGuid.find(guid) != _hideForGuid.end();  };

        void setTransport(Transport* t) { m_transport = t; }

        //!  Get or Init cyber ptr.
        C_PTR get_ptr();

        uint32 GetCustomData() const { return m_customData; }
        void SetCustomData(uint32 data) { m_customData = data; }

        // transports
        Transport* m_transport;

    protected:
        std::string m_name;
        bool m_isActive;
        const bool m_isWorldObject;
        ZoneScript* m_zoneScript;

        //these functions are used mostly for Relocate() and Corpse/Player specific stuff...
        //use them ONLY in LoadFromDB()/Create() funcs and nowhere else!
        //mapId/instanceId should be set in SetMap() function!
        void SetLocationMapId(uint32 _mapId) { m_mapId = _mapId; }
        void SetLocationInstanceId(uint32 _instanceId) { m_InstanceId = _instanceId; }

        virtual bool IsNeverVisible(WorldObject const* seer = nullptr) const { return !IsInWorld(); }
        virtual bool IsAlwaysVisibleFor(WorldObject const* /*seer*/) const { return false; }
        virtual bool IsInvisibleDueToDespawn() const { return false; }
        //difference from IsAlwaysVisibleFor: 1. after distance check; 2. use owner or charmer as seer
        virtual bool IsAlwaysDetectableFor(WorldObject const* /*seer*/) const { return false; }

    private:
        C_PTR ptr;

        //uint32 m_mapId;                                     // object at map with map_id
        uint32 m_InstanceId;                                // in map copy with instance id
        uint32 m_phaseMask;                                 // in area phase state
        std::set<uint32> m_phaseId;                         // special phase. It's new generation phase, when we should check id.
        std::vector<bool> m_phaseBit;
        bool m_ignorePhaseIdCheck;                          // like gm mode.
        std::set<uint32> _terrainSwaps;
        std::set<uint32> _worldMapAreaSwaps;

        GuidUnorderedSet _visibilityPlayerList;
        GuidUnorderedSet _hideForGuid;

        virtual bool _IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D, bool ignoreObjectSize = false) const;
        virtual bool CanNeverSee2(WorldObject const* obj) const { return false; }
        virtual bool CanNeverSee(WorldObject const* obj) const { return GetMap() != obj->GetMap() || !InSamePhase(obj); }
        virtual bool CanAlwaysSee(WorldObject const* /*obj*/) const { return false; }
        bool CanDetect(WorldObject const* obj, bool ignoreStealth) const;
        bool CanDetectInvisibilityOf(WorldObject const* obj) const;
        bool CanDetectStealthOf(WorldObject const* obj) const;

        uint32 m_customData{};
};

namespace Trinity
{
    // Binary predicate to sort WorldObjects based on the distance to a reference WorldObject
    class ObjectDistanceOrderPred
    {
            const WorldObject* m_refObj;
            const bool m_ascending;
        public:
            ObjectDistanceOrderPred(const WorldObject* pRefObj, bool ascending = true);
            bool operator()(const WorldObject* pLeft, const WorldObject* pRight) const;
    };

    // Binary predicate to sort WorldObjects based on the distance to a reference WorldObject
    class GuidValueSorterPred
    {
            const bool m_ascending;
        public:
            GuidValueSorterPred(bool ascending = true);
            bool operator()(const WorldObject* pLeft, const WorldObject* pRight) const;
    };
}

#endif
