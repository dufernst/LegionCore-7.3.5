
#ifndef WORLD_STATE_MGR_H
#define WORLD_STATE_MGR_H

#include "Common.h"
#include "WorldState.h"

inline void AddToWorldState(std::vector<WorldState*>** worldStates, WorldState* state, Player* player);

class Player;

class WorldStateMgr
{
public:
    WorldStateMgr();
    ~WorldStateMgr();

    void Update(uint32 diff);

    static WorldStateMgr& Instance();
    void Initialize();

    void LoadTemplatesFromDB();
    void LoadTemplatesFromObjectTemplateDB();
    void LoadTemplatesFromDBC();
    void LoadFromDB();

    void SaveToDB();

    void CreateWorldStatesIfNeed();

    void CreateInstanceState(uint32 mapID, uint32 instanceID);
    void DeleteInstanceState(uint32 instanceID);

    void MapUpdate(Map* map);

    WorldState* CreateWorldState(WorldStateTemplate const* stateTemplate, uint32 instanceID = 0, uint32 value = std::numeric_limits<uint32>().max());
    WorldState* CreateWorldState(uint32 variableID, uint32 instanceID = 0, uint32 value = std::numeric_limits<uint32>().max());
    WorldState* GetWorldState(uint32 variableID, uint32 instanceID = 0);
    WorldState* GetWorldState(WorldStateTemplate const* stateTemplate, uint32 instanceID = 0);
    void DeleteWorldState(WorldState* state);
    WorldStateTemplate const* FindTemplate(uint32 variableID);
    WorldStateTemplate* AddTemplate(uint32 variableID, uint32 type, uint32 _condition, uint32 flags, uint32 defaultValue);

    WorldState* AddWorldState(uint32 variableID, uint32 instanceID, uint32 type, uint32 flags, uint32 value);

    uint32 GetWorldStateValue(uint32 variableID, WorldObject* object = nullptr);
    void SetWorldState(uint32 variableID, uint32 instanceID, uint32 value, bool hidden = false);

    std::vector<WorldState*>* GetInitWorldStates(Player* player);

    static bool IsFitToCondition(Player* player, WorldState const* state);

private:
    WorldStateTemplateV _worldStateTemplates;
    WorldStateTemplateMap _worldStateTemplatesMap[WorldStatesData::Types::Max];
    WorldStateMap _worldState;
    WorldStateMapV _worldStateV;
    WorldStateInstanceMap _worldStateInstance;
    uint32 m_nextSave;
};

#define sWorldStateMgr WorldStateMgr::Instance()

#endif
