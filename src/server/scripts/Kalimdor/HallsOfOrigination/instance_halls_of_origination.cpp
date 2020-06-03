#include "halls_of_origination.h"

#define MAX_ENCOUNTER 7

static const DoorData doorData[] =
{
    {GO_DOOR_ULDUM_14,              DATA_TEMPLE_GUARDIAN_ANHUUR, DOOR_TYPE_ROOM,    BOUNDARY_NONE},
    {GO_ANHUUR_BRIDGE,              DATA_TEMPLE_GUARDIAN_ANHUUR, DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {GO_DOOR_ULDUM_15,              DATA_TEMPLE_GUARDIAN_ANHUUR, DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {GO_ANHUUR_ELEVATOR,            DATA_TEMPLE_GUARDIAN_ANHUUR, DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {GO_VAULT_OF_LIGHTS_ENTR_DOOR,  DATA_ANRAPHET,               DOOR_TYPE_PASSAGE, BOUNDARY_NONE},
    {0,                0,                                        DOOR_TYPE_ROOM,    BOUNDARY_NONE} // END
};

class instance_halls_of_origination : public InstanceMapScript
{
    public:
        instance_halls_of_origination() : InstanceMapScript("instance_halls_of_origination", 644) {}

        InstanceScript* GetInstanceScript(InstanceMap *map) const
        {
            return new instance_halls_of_origination_InstanceMapScript(map);
        }

        struct instance_halls_of_origination_InstanceMapScript: public InstanceScript
        {
            instance_halls_of_origination_InstanceMapScript(InstanceMap *map) : InstanceScript(map)
            {
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doorData);
                uiTempleGuardianAnhuurGUID.Clear();
                uiEarthragerPtahGUID.Clear();
                uiAnraphetGUID.Clear();
                uiIsisetGUID.Clear();
                uiAmmunaeGUID.Clear();
                uiSeteshGUID.Clear();
                uiRajhGUID.Clear();
                uiBrannGUID.Clear();

                uiWardensDone = 0;

                uiOriginationElevatorGUID.Clear();
                uiAnhuurBridgeGUID.Clear();
                uiAnraphetEntranceDoorGUID.Clear();
                uiAnraphetBossDoorGUID.Clear();
            }

            void OnPlayerEnter(Player* player)
            {
                if (!uiTeamInInstance)
                    uiTeamInInstance = player->GetTeam();
            }

            void OnCreatureCreate(Creature* pCreature)
            {
                if (!uiTeamInInstance)
                {
                    Map::PlayerList const &players = instance->GetPlayers();
                    if (!players.isEmpty())
                        if (Player* player = players.begin()->getSource())
                            uiTeamInInstance = player->GetTeam();
                }

                switch (pCreature->GetEntry())
                {
                    case NPC_TEMPLE_GUARDIAN_ANHUUR:
                        uiTempleGuardianAnhuurGUID = pCreature->GetGUID();
                        break;
                    case NPC_EARTHRAGER_PTAH:
                        uiEarthragerPtahGUID = pCreature->GetGUID();
                        break;
                    case NPC_ANRAPHET:
                        uiAnraphetGUID = pCreature->GetGUID();
                        if (uiWardensDone >= 4)
                        {
                            pCreature->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                            pCreature->SetHomePosition(-203.93f, 368.71f, 75.92f, pCreature->GetOrientation());
                            //DoTeleportTo(-203.93f, 368.71f, 75.92f);
                            pCreature->GetMotionMaster()->MovePoint(0, -203.93f, 368.71f, 75.92f);
                        }
                        break;
                    case NPC_ISISET:
                        uiIsisetGUID = pCreature->GetGUID();
                        break;
                    case NPC_AMMUNAE:
                        uiAmmunaeGUID = pCreature->GetGUID();
                        break;
                    case NPC_SETESH:
                        uiSeteshGUID = pCreature->GetGUID();
                        break;
                    case NPC_RAJH:
                        uiRajhGUID = pCreature->GetGUID();
                        break;
                    case NPC_BRANN_BRONZEBEARD:
                        uiBrannGUID = pCreature->GetGUID();
                        if (GetBossState(DATA_ANRAPHET) == DONE)
                            pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry()) 
                {
                    case GO_ORIGINATION_ELEVATOR:
                        uiOriginationElevatorGUID = go->GetGUID();
                        break;
                    case GO_ANHUUR_BRIDGE:
                    case GO_ANHUUR_ELEVATOR:
                    case GO_DOOR_ULDUM_14:
                    case GO_DOOR_ULDUM_15:                                           
                        AddDoor(go, true);
                        break;
                    case GO_VAULT_OF_LIGHTS_ENTR_DOOR:
                        uiAnraphetEntranceDoorGUID = go->GetGUID();
                        break;
                    case GO_VAULT_OF_LIGHTS_BOSS_DOOR:
                        uiAnraphetBossDoorGUID = go->GetGUID();
                        if (uiWardensDone >= 4)
                            go->SetGoState(GO_STATE_ACTIVE);
                        break;
                }
            }

            ObjectGuid GetGuidData(uint32 identifier) const
            {
                switch(identifier)
                {
                    case DATA_TEMPLE_GUARDIAN_ANHUUR:
                        return uiTempleGuardianAnhuurGUID;
                    case DATA_EARTHRAGER_PTAH:
                        return uiEarthragerPtahGUID;
                    case DATA_ANRAPHET:
                        return uiAnraphetGUID;
                    case DATA_ISISET:
                        return uiIsisetGUID;
                    case DATA_AMMUNAE:
                        return uiAmmunaeGUID;
                    case DATA_SETESH:
                        return uiSeteshGUID;
                    case DATA_RAJH:
                        return uiRajhGUID;
                    case DATA_BRANN:
                        return uiBrannGUID;
                    case DATA_ANRAPHET_ENTRANCE_DOOR:
                        return uiAnraphetEntranceDoorGUID;
                    case DATA_ANRAPHET_BOSS_DOOR:
                        return uiAnraphetBossDoorGUID;
                }
                return ObjectGuid::Empty;
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;
                
                if (type == DATA_EARTHRAGER_PTAH)
                {
                    if (state == DONE)
                    {
                        Map::PlayerList const& players = instance->GetPlayers();
                        if (!players.isEmpty())
                            if (Player* player = players.begin()->getSource())
                                if (player->HasAura(SPELL_RIDE_VEHICLE))
                                    DoCompleteAchievement(ACHIEV_STRAW_THAT_BROKE_CAMELS);
                    }
                }

                return true;
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_WARDENS:

                        uiWardensDone += data;
                        
                        if (uiWardensDone == 4)
                        {
                            DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, SPELL_FASTER_THAN_LIGHT);
                            HandleGameObject(uiAnraphetBossDoorGUID, true);
                            if (auto pAnraphet = instance->GetCreature(uiAnraphetGUID))
                                pAnraphet->AI()->DoAction(1);
                        }
                            
                        switch (uiWardensDone)
                        {
                            case 1:
                            case 2:
                            case 3:
                            case 4:
                                if (auto pBrann = instance->GetCreature(uiBrannGUID))
                                    pBrann->AI()->DoAction(uiWardensDone);
                                break;
                        }
                        break;
                }
                SaveToDB();
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << "H O" << GetBossSaveData() << uiWardensDone << " "; 

                OUT_SAVE_INST_DATA_COMPLETE;
                return saveStream.str();
            }

            void Load(const char* in)
            {
                if (!in)
                {
                    OUT_LOAD_INST_DATA_FAIL;
                    return;
                }

                OUT_LOAD_INST_DATA(in);

                char dataHead1, dataHead2;

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2;

                if (dataHead1 == 'H' && dataHead2 == 'O')
                {
                    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }
                    uint32 wardens = 0;
                    loadStream >> wardens;
                    //uiWardensDone = wardens;
                    if (wardens > 4) wardens = 4;
                    SetData(DATA_WARDENS, wardens);
                }else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

            private:
                uint32 uiTeamInInstance;
                ObjectGuid uiTempleGuardianAnhuurGUID;
                ObjectGuid uiEarthragerPtahGUID;
                ObjectGuid uiAnraphetGUID;
                ObjectGuid uiIsisetGUID;
                ObjectGuid uiAmmunaeGUID;
                ObjectGuid uiSeteshGUID;
                ObjectGuid uiRajhGUID;
                ObjectGuid uiBrannGUID;

                uint32 uiWardensDone;

                ObjectGuid uiOriginationElevatorGUID;
                ObjectGuid uiAnhuurBridgeGUID;
                ObjectGuid uiAnraphetEntranceDoorGUID;
                ObjectGuid uiAnraphetBossDoorGUID;

        };
};

void AddSC_instance_halls_of_origination()
{
    new instance_halls_of_origination();
}
