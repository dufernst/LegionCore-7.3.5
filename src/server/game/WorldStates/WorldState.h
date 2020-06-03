
#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include "Common.h"

namespace WorldStatesData
{
    enum Limits : uint16
    {
        Begin = 1000,
        End = 20000
    };

    enum Types : uint8
    {
        Custom = 0,
        World = 1,
        Weekly = 2,
        Event = 3,
        Map = 4,
        Zone = 5,
        Area = 6,
        Battlegound = 7,
        CapturePoint = 8,
        DestructibleObject = 9,
        Max
    };

    enum Flags : uint8
    {
        Initialized = 0,
        Active = 1,
        Saved = 2,
        Expired = 3,
        Updated = 4,
        Deleted = 7,
        Neutral = 8,
        Alliance = 9,
        Horde = 10,
        InitialState = 16,
        PassiaveAtCreate = 17,
        NotExpireable = 18,
        CustomFormat = 24,
        CustomGlobal = 25,
        CustomHidden = 26,
        CustomX = 27,
    };

    enum InitialValue : uint8
    {
        Remove = 0,
        Add = 1,
    };
}

typedef cds::container::FeldmanHashSet< cds::gc::HP, ObjectGuid, ObjectGuidHashAccessor > GuidHashSet;

struct WorldStateTemplate
{
    WorldStateTemplate(uint32 variableID = 0, uint32 type = 0, uint32 _condition = 0, uint32 flags = 0, uint32 defaultValue = 0);

    uint32 VariableID;
    uint32 VariableType;
    uint32 ConditionID;
    uint32 Flags;
    uint32 DefaultValue;

    bool IsGlobal() const;
    bool HasFlag(WorldStatesData::Flags flag) const;
};

struct WorldState
{
    WorldState(uint32 variableID = 0, uint32 type = 0, uint32 flags = 0, uint32 value = 0, WorldStateTemplate const* stateTemplate = nullptr);

    bool IsGlobal() const;

    void Initialize();

    void Reload();

    void AddClient(ObjectGuid const& guid);
    bool HasClient(ObjectGuid const& guid);
    void RemoveClient(ObjectGuid const& guid);

    WorldStateTemplate const* GetTemplate() const;

    void AddFlag(WorldStatesData::Flags flag);
    void RemoveFlag(WorldStatesData::Flags flag);
    bool HasFlag(WorldStatesData::Flags flag) const;

    void SetValue(uint32 value, bool hidden);

    WorldStateTemplate const* StateTemplate;
    GuidHashSet ClientGuids;
    ObjectGuid LinkedGuid;
    uint32 VariableID;
    uint32 Type;
    uint32 Flags;
    uint32 Value;
    uint32 ConditionID;
    uint32 InstanceID;
    bool Hidden;
};

typedef std::vector<WorldStateTemplate*> WorldStateTemplateV;
typedef std::map<uint32 /*variableID*/, WorldStateTemplate> WorldStateTemplateMap;
typedef std::map<uint32 /*variableID*/, WorldState> WorldStateMap;
typedef std::map<uint32 /*instanceID*/, WorldStateMap> WorldStateInstanceMap;
typedef std::vector<WorldState*> WorldStateMapV;

#endif
