#include "Map.h"
#include "WorldStateMgr.h"
#include "Player.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "CellImpl.h"
#include "BattlegroundMgr.h"
#include "ScriptMgr.h"
#include "DB2Stores.h"

void AddToWorldState(std::vector<WorldState*>** worldStates, WorldState* state, Player* player)
{
    if (!*worldStates)
        *worldStates = new std::vector<WorldState*>;

    if (player)
        state->AddClient(player->GetGUID());

    (*worldStates)->push_back(state);
}

WorldStateMgr::WorldStateMgr()
{
    m_nextSave = sWorld->getIntConfig(CONFIG_INTERVAL_SAVE);
}

WorldStateMgr::~WorldStateMgr()
{
}

WorldStateMgr& WorldStateMgr::Instance()
{
    static WorldStateMgr instance;
    return instance;
}

void WorldStateMgr::Update(uint32 diff)
{
    if (_worldState.empty())
        return;

    if (m_nextSave > 0)
    {
        if (diff >= m_nextSave)
        {
            for (auto itr = _worldState.begin(); itr != _worldState.end(); ++itr)
                if (itr->second.HasFlag(WorldStatesData::Flags::Deleted))
                    _worldState.erase(itr++);

            SaveToDB();
        }
        else
            m_nextSave -= diff;
    }
}

void WorldStateMgr::Initialize()
{
    _worldStateTemplates.clear();

    for (uint8 i = WorldStatesData::Types::Custom; i < WorldStatesData::Types::Max; i++)
        _worldStateTemplatesMap[i].clear();

    LoadTemplatesFromDB();

    // For activeted this need all bg and other converted to new system, who can do?
    // LoadTemplatesFromObjectTemplateDB();
    // LoadTemplatesFromDBC();

    LoadFromDB();

    AddTemplate(WorldStates::WS_PVP_ARENA_ENABLED, WorldStatesData::Types::World, 0, 1 << WorldStatesData::Flags::InitialState, sWorld->getBoolConfig(CONFIG_ARENA_SEASON_IN_PROGRESS));
    AddTemplate(WorldStates::WS_ARENA_SEASON_ID, WorldStatesData::Types::World, 0, 1 << WorldStatesData::Flags::InitialState, sWorld->getIntConfig(CONFIG_ARENA_SEASON_ID));
    AddTemplate(WorldStates::WS_RATED_BG_ENABLED, WorldStatesData::Types::World, 0, 1 << WorldStatesData::Flags::InitialState, sWorld->getBoolConfig(CONFIG_ARENA_SEASON_IN_PROGRESS));
}

WorldStateTemplate const* WorldStateMgr::FindTemplate(uint32 variableID)
{
    if (_worldStateTemplates.empty() || _worldStateTemplates.size() <= variableID)
        return nullptr;

    return _worldStateTemplates[variableID];
}

WorldStateTemplate* WorldStateMgr::AddTemplate(uint32 variableID, uint32 type, uint32 _condition, uint32 flags, uint32 defaultValue)
{
    if (_worldStateTemplates.size() <= variableID)
        _worldStateTemplates.resize(variableID + 1, nullptr);

    auto pair = _worldStateTemplatesMap[type].insert({ variableID, WorldStateTemplate(variableID, type, _condition, flags, defaultValue) });
    _worldStateTemplates[variableID] = &(*pair.first).second;
    return _worldStateTemplates[variableID];
}

WorldState* WorldStateMgr::AddWorldState(uint32 variableID, uint32 instanceID, uint32 type, uint32 flags, uint32 value)
{
    WorldStateTemplate const* stateTemplate = FindTemplate(variableID);
    if (!stateTemplate)
        return nullptr;

    if (stateTemplate->IsGlobal())
    {
        if (_worldStateV.size() <= variableID)
            _worldStateV.resize(variableID + 1, nullptr);

        WorldState& ws = _worldState[variableID];
        ws.VariableID = variableID;
        ws.Type = type;
        ws.Flags = flags;
        ws.Value = value;
        ws.InstanceID = instanceID;
        ws.StateTemplate = stateTemplate;
        _worldStateV[variableID] = &ws;
        return _worldStateV[variableID];
    }

    if (!instanceID)
        return nullptr;

    auto& stateMap = _worldStateInstance[instanceID];
    WorldState& ws = stateMap[variableID];
    ws.VariableID = variableID;
    ws.Type = type;
    ws.Flags = flags;
    ws.Value = value;
    ws.InstanceID = instanceID;
    ws.StateTemplate = stateTemplate;
    return &ws;
}

void WorldStateMgr::LoadTemplatesFromDBC()
{
    uint32 count = 0;

    for (WorldStateUIEntry const* wsEntry : sWorldStateUIStore)
    {
        if (!wsEntry)
            continue;

        uint32 stateId = wsEntry->StateVariable;
        if (!stateId)
            continue;

        uint8 type;
        uint32 condition;

        std::string extendedUI = wsEntry->ExtendedUI->Str[sWorld->GetDefaultDbcLocale()];
        if (!extendedUI.empty() && !strcmp(extendedUI.c_str(), "CAPTUREPOINT"))
        {
            type = WorldStatesData::Types::CapturePoint;
            condition = 0;
            if (wsEntry->ExtendedUIStateVariable[0])
                AddTemplate(wsEntry->ExtendedUIStateVariable[0], type, condition, (1 << WorldStatesData::Flags::InitialState), 0);
            if (wsEntry->ExtendedUIStateVariable[1])
                AddTemplate(wsEntry->ExtendedUIStateVariable[1], type, condition, (1 << WorldStatesData::Flags::InitialState), 0);
            if (wsEntry->ExtendedUIStateVariable[2])
                AddTemplate(wsEntry->ExtendedUIStateVariable[2], type, condition, (1 << WorldStatesData::Flags::InitialState), 0);
        }
        else if (wsEntry->MapID && !wsEntry->AreaID)
        {
            type = WorldStatesData::Types::Map;
            condition = wsEntry->MapID;
        }
        else if (wsEntry->AreaID)
        {
            type = WorldStatesData::Types::Zone;
            condition = wsEntry->AreaID;
        }
        else if (!wsEntry->MapID && !wsEntry->AreaID)
        {
            type = WorldStatesData::Types::Event;
            condition = wsEntry->PhaseUseFlags;   // Phase currently
        }
        else
        {
            TC_LOG_INFO(LOG_FILTER_GENERAL, "WorldStateMgr::LoadTemplatesFromDBC unhandled template %u!", stateId);
            continue;
        }

        AddTemplate(stateId, type, condition, (1 << WorldStatesData::Flags::InitialState), 0);

        // parse linked worldstates here
        std::string message = wsEntry->String->Str[sWorld->GetDefaultDbcLocale()];
        if (!message.empty())
        {
            auto pos1 = message.find_first_of("%");
            auto pos2 = message.find_first_of("w", pos1);

            while (pos1 != std::string::npos && pos2 != std::string::npos)
            {
                pos1 = message.find_first_of("%");
                pos2 = message.find_first_of("w", pos1);
                if ((pos2 - pos1) == 5)
                {
                    std::string digits = message.substr(pos1 + 1, 4);
                    uint32 linkedId = atoi(digits.c_str());
                    if (linkedId)
                        AddTemplate(linkedId, type, condition, (1 << WorldStatesData::Flags::InitialState), 0);
                }
                message.erase(0, pos2 + 1);
            }
        }

        ++count;
    }
    TC_LOG_INFO(LOG_FILTER_GENERAL, ">> Loaded static DBC templates for %u WorldStates", count);
}

void WorldStateMgr::LoadTemplatesFromDB()
{
    //                                         0             1       2              3        4
    auto result = WorldDatabase.Query("SELECT `VariableID`, `Type`, `ConditionID`, `Flags`, `DefaultValue` FROM `worldstate_template`");
    if (!result)
        return;

    do
    {
        auto fields = result->Fetch();
        auto variableID = fields[0].GetUInt32();
        auto type = fields[1].GetUInt32();

        AddTemplate(variableID, fields[1].GetUInt32(), fields[2].GetUInt32(), fields[3].GetUInt32(), fields[4].GetUInt32());
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_GENERAL, "%s >> Loaded %u templates", __FUNCTION__, _worldStateTemplates.size());
}

void WorldStateMgr::LoadTemplatesFromObjectTemplateDB()
{
    uint32 count = 0;
    if (auto result = WorldDatabase.PQuery("SELECT `entry` FROM `gameobject_template` WHERE `type` = %u", GAMEOBJECT_TYPE_CONTROL_ZONE))
    {
        do
        {
            auto fields = result->Fetch();

            uint32 goEntry = fields[0].GetUInt32();

            GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(goEntry);
            if (!goInfo)
                continue;

            if (goInfo->controlZone.worldState1)
                AddTemplate(goInfo->controlZone.worldState1, WorldStatesData::Types::CapturePoint, goEntry, (1 << WorldStatesData::Flags::Active), WorldStatesData::InitialValue::Add);

            if (goInfo->controlZone.worldstate2)
                AddTemplate(goInfo->controlZone.worldstate2, WorldStatesData::Types::CapturePoint, goEntry, (1 << WorldStatesData::Flags::Active), 50);

            if (goInfo->controlZone.worldstate3)
                AddTemplate(goInfo->controlZone.worldstate3, WorldStatesData::Types::CapturePoint, goEntry, (1 << WorldStatesData::Flags::Active), goInfo->controlZone.neutralPercent);

            if (goInfo->controlZone.speedWorldState1)
                AddTemplate(goInfo->controlZone.speedWorldState1, WorldStatesData::Types::CapturePoint, goEntry, (1 << WorldStatesData::Flags::Active), 0);

            if (goInfo->controlZone.speedWorldState2)
                AddTemplate(goInfo->controlZone.speedWorldState2, WorldStatesData::Types::CapturePoint, goEntry, (1 << WorldStatesData::Flags::Active), 0);

            ++count;
        }
        while (result->NextRow());
    }

    if (auto result = WorldDatabase.PQuery("SELECT `entry` FROM `gameobject_template` WHERE `type` = %u", GAMEOBJECT_TYPE_NEW_FLAG))
    {
        do
        {
            auto fields = result->Fetch();

            uint32 goEntry = fields[0].GetUInt32();

            GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(goEntry);
            if (!goInfo)
                continue;

            if (goInfo->newflag.worldState1)
                AddTemplate(goInfo->newflag.worldState1, WorldStatesData::Types::CapturePoint, goEntry, (1 << WorldStatesData::Flags::Active), WorldStatesData::InitialValue::Add);

            ++count;
        }
        while (result->NextRow());
    }

    if (auto result = WorldDatabase.PQuery("SELECT `entry` FROM `gameobject_template` WHERE `type` = %u", GAMEOBJECT_TYPE_CAPTURE_POINT))
    {
        do
        {
            auto fields = result->Fetch();

            uint32 goEntry = fields[0].GetUInt32();

            GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(goEntry);
            if (!goInfo)
                continue;

            if (goInfo->capturePoint.worldState1)
                AddTemplate(goInfo->capturePoint.worldState1, WorldStatesData::Types::CapturePoint, goEntry, (1 << WorldStatesData::Flags::Active), WorldStatesData::InitialValue::Add);

            ++count;
        }
        while (result->NextRow());
    }

    TC_LOG_INFO(LOG_FILTER_GENERAL, ">> Loaded static templates for %u GAMEOBJECT_TYPE_CAPTURE_POINT linked WorldStates", count);
}

void WorldStateMgr::LoadFromDB()
{
    _worldState.clear();

    //                                             0             1            2           3       4
    auto result = CharacterDatabase.Query("SELECT `VariableID`, `Type`, `ConditionID`, `Flags`, `Value` FROM `worldstate_data`");
    if (!result)
        return;

    do
    {
        auto fields = result->Fetch();

        auto variableID = fields[0].GetUInt32();
        auto type = fields[1].GetUInt32();
        auto conditionID = fields[2].GetUInt32();
        auto flags = fields[3].GetUInt32();
        auto value = fields[4].GetUInt32();

        if (auto* stateTemplate = FindTemplate(variableID))
        {
            if (auto state = GetWorldState(stateTemplate))
                state->SetValue(value, false);
            else
                AddWorldState(variableID, 0, type, flags, value);
        }
        else if (type == WorldStatesData::Types::Custom)
            AddWorldState(variableID, 0, type, flags, value);

    } while (result->NextRow());
}

void WorldStateMgr::CreateWorldStatesIfNeed()
{
    for (auto itr : _worldStateTemplatesMap[WorldStatesData::Types::World])
        if (itr.second.HasFlag(WorldStatesData::Flags::InitialState))
            if (!GetWorldState(&itr.second))
                CreateWorldState(&itr.second, 0, itr.second.DefaultValue);

    for (auto itr : _worldStateTemplatesMap[WorldStatesData::Types::Weekly])
        if (itr.second.HasFlag(WorldStatesData::Flags::InitialState))
            if (!GetWorldState(&itr.second))
                CreateWorldState(&itr.second, 0, itr.second.DefaultValue);

    // for (auto itr : _worldStateTemplatesMap[WorldStatesData::Types::Zone])
        // if (itr.second.HasFlag(WorldStatesData::Flags::InitialState))
            // if (!GetWorldState(&itr.second))
                // CreateWorldState(&itr.second, 0, itr.second.DefaultValue);

    // for (auto itr : _worldStateTemplatesMap[WorldStatesData::Types::Area])
        // if (itr.second.HasFlag(WorldStatesData::Flags::InitialState))
            // if (!GetWorldState(&itr.second))
                // CreateWorldState(&itr.second, 0, itr.second.DefaultValue);
}

void WorldStateMgr::CreateInstanceState(uint32 mapID, uint32 instanceID)
{
    for (auto itr : _worldStateTemplatesMap[WorldStatesData::Types::Map])
        if (itr.second.HasFlag(WorldStatesData::Flags::InitialState) && itr.second.ConditionID == mapID)
            CreateWorldState(&itr.second, instanceID, itr.second.DefaultValue);

    for (auto itr : _worldStateTemplatesMap[WorldStatesData::Types::Battlegound])
        if (itr.second.HasFlag(WorldStatesData::Flags::InitialState) && itr.second.ConditionID == mapID)
            CreateWorldState(&itr.second, instanceID, itr.second.DefaultValue);
}

void WorldStateMgr::SaveToDB()
{
    m_nextSave = sWorld->getIntConfig(CONFIG_INTERVAL_SAVE);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    for (auto iter = _worldState.begin(); iter != _worldState.end(); ++iter)
        if (!iter->second.HasFlag(WorldStatesData::Flags::Saved))
        {
            auto state = &iter->second;

            auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_WORLD_STATE_BY_STATE_INSTANCE);
            stmt->setUInt32(0, state->VariableID);
            trans->Append(stmt);

            if (!state->HasFlag(WorldStatesData::Flags::Deleted))
            {
                state->AddFlag(WorldStatesData::Flags::Saved);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_WORLD_STAT);
                stmt->setUInt32(0, state->VariableID);
                stmt->setUInt32(1, state->Type);
                stmt->setUInt32(2, state->ConditionID);
                stmt->setUInt32(3, state->Flags);
                stmt->setUInt32(4, state->Value);
                trans->Append(stmt);
            }
        }

    CharacterDatabase.CommitTransaction(trans);
}

void WorldStateMgr::DeleteWorldState(WorldState* state)
{
    if (!state)
        return;

    auto trans = CharacterDatabase.BeginTransaction();
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_WORLD_STATE_BY_STATE_INSTANCE);
    stmt->setUInt32(0, state->VariableID);
    trans->Append(stmt);
    CharacterDatabase.CommitTransaction(trans);

    for (auto iter = _worldState.begin(); iter != _worldState.end(); ++iter)
    {
        if (&iter->second != state)
            continue;

        iter->second.RemoveFlag(WorldStatesData::Flags::Active);
        iter->second.AddFlag(WorldStatesData::Flags::Deleted);
        break;
    }
}

void WorldStateMgr::MapUpdate(Map* /*map*/)
{
}

bool WorldStateMgr::IsFitToCondition(Player* player, WorldState const* state)
{
    if (!player || !state)
        return false;

    if (state->HasFlag(WorldStatesData::Flags::Deleted))
        return false;

    return sConditionMgr->IsObjectMeetToConditions(player, sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_WORLD_STATE, state->VariableID));
}

uint32 WorldStateMgr::GetWorldStateValue(uint32 variableID, WorldObject* object)
{
    uint32 instanceID = 0;
    if (object && object->InInstance())
        instanceID = object->GetInstanceId();

    if (auto state = GetWorldState(variableID, instanceID))
        return state->Value;

    return std::numeric_limits<uint32>().max();
}


void WorldStateMgr::SetWorldState(uint32 variableID, uint32 instanceID, uint32 value, bool hidden /*= false*/)
{
    if (auto state = GetWorldState(variableID, instanceID))
    {
        state->SetValue(value, hidden);
        return;
    }

    CreateWorldState(variableID, instanceID, value);
}

WorldState* WorldStateMgr::CreateWorldState(uint32 variableID, uint32 instanceID, uint32 value)
{
    if (variableID == 0)
        return nullptr;

    return CreateWorldState(FindTemplate(variableID), instanceID, value);
}

WorldState* WorldStateMgr::CreateWorldState(WorldStateTemplate const* stateTemplate, uint32 instanceID, uint32 value)
{
    if (!stateTemplate)
        return nullptr;

    if (stateTemplate->IsGlobal() && instanceID > 0)
        return nullptr;

    sScriptMgr->OnWorldStateCreate(stateTemplate->VariableID, value, stateTemplate->VariableType);

    if (auto state = GetWorldState(stateTemplate, instanceID))
    {
        state->SetValue(value, false);
        return state;
    }

    auto state = AddWorldState(stateTemplate->VariableID, instanceID, stateTemplate->VariableType, 0, value);
    if (!state)
        return nullptr;

    if (value != std::numeric_limits<uint32>().max())
        state->SetValue(value, false);
    else
        state->RemoveFlag(WorldStatesData::Flags::Saved);

    if (!state->HasFlag(WorldStatesData::Flags::PassiaveAtCreate))
        state->AddFlag(WorldStatesData::Flags::Active);

    return state;
}

WorldState* WorldStateMgr::GetWorldState(uint32 variableID, uint32 instanceID)
{
    return GetWorldState(FindTemplate(variableID), instanceID);
}

WorldState* WorldStateMgr::GetWorldState(WorldStateTemplate const* stateTemplate, uint32 instanceID)
{
    if (!stateTemplate)
        return nullptr;

    if (stateTemplate->IsGlobal())
    {
        if (stateTemplate->VariableID >= _worldStateV.size())
            return nullptr;

        return _worldStateV[stateTemplate->VariableID];
    }

    if (!instanceID)
        return nullptr;

    auto iter = _worldStateInstance.find(instanceID);
    if (iter == _worldStateInstance.end())
        return nullptr;

    return Trinity::Containers::MapGetValuePtr(iter->second, stateTemplate->VariableID);
}

void WorldStateMgr::DeleteInstanceState(uint32 instanceID)
{
    if (instanceID == 0)
        return;

    _worldStateInstance.erase(instanceID);
}

std::vector<WorldState*>* WorldStateMgr::GetInitWorldStates(Player* player)
{
    uint32 instanceID = player->InInstance() ? player->GetInstanceId() : 0;

    std::vector<WorldState*>* states = nullptr;

    for (auto itr = _worldState.begin(); itr != _worldState.end(); ++itr)
    {
        auto state = &itr->second;
        if (!state || state->HasFlag(WorldStatesData::Flags::Deleted))
            continue;

        if ((state->HasFlag(WorldStatesData::Flags::InitialState) || state->HasFlag(WorldStatesData::Flags::Active)) && IsFitToCondition(player, state))
            AddToWorldState(&states, state, player);
    }

    if (instanceID)
    {
        for (auto itr = _worldState.begin(); itr != _worldState.end(); ++itr)
        {
            auto state = &itr->second;
            if (!state || state->HasFlag(WorldStatesData::Flags::Deleted))
                continue;

            if ((state->HasFlag(WorldStatesData::Flags::InitialState) || state->HasFlag(WorldStatesData::Flags::Active)) && IsFitToCondition(player, state))
                AddToWorldState(&states, state, player);
        }
    }
    return states;
}
