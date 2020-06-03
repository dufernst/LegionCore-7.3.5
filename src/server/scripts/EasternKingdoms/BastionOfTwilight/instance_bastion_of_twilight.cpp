#include "bastion_of_twilight.h"

#define MAX_ENCOUNTER 5

static const DoorData doordata[] = 
{
    {DOOR_HALFUS_ENTRANCE,  DATA_HALFUS,            DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {DOOR_HALFUS_EXIT,      DATA_HALFUS,            DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {DOOR_THER_ENTRANCE,    DATA_VALIONA_THERALION, DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {DOOR_THER_EXIT,        DATA_VALIONA_THERALION, DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {DOOR_COUNCIL_ENTRANCE, DATA_COUNCIL,           DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {DOOR_COUNCIL_EXIT,     DATA_COUNCIL,           DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {DOOR_CHOGALL_ENTRANCE, DATA_CHOGALL,           DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_SINESTRA_DOOR,      DATA_SINESTRA,          DOOR_TYPE_ROOM,     BOUNDARY_NONE}           
};

class instance_bastion_of_twilight : public InstanceMapScript
{
public:
    instance_bastion_of_twilight() : InstanceMapScript("instance_bastion_of_twilight", 671) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_bastion_of_twilight_InstanceMapScript(map);
    }

    struct instance_bastion_of_twilight_InstanceMapScript: public InstanceScript
    {
        instance_bastion_of_twilight_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_ENCOUNTER);
            LoadDoorData(doordata);
            uiWyrmbreakerGUID.Clear();
            uiValionaGUID.Clear();
            uiTheralionGUID.Clear();
            uiFeludiusGUID.Clear();
            uiArionGUID.Clear();
            uiIgnaciousGUID.Clear();
            uiTerrastraGUID.Clear();
            uiMonstrosityGUID.Clear();
            uiWhelpCageGUID.Clear();
            uiValionaTheralionHealth = 0;
            uiChogallGUID.Clear();
            uiChogallFloorGUID.Clear();
            memset(m_uiDialogs, 0, sizeof(m_uiDialogs));
        }

        void OnPlayerEnter(Player* player)
        {
            if (!uiTeamInInstance)
                uiTeamInInstance = player->GetTeam();
        }

        void OnCreatureCreate(Creature* creature)
        {
            if (!uiTeamInInstance)
            {
                Map::PlayerList const &players = instance->GetPlayers();
                if (!players.isEmpty())
                    if (Player* player = players.begin()->getSource())
                        uiTeamInInstance = player->GetTeam();
            }

            switch (creature->GetEntry())
            {
                case NPC_HALFUS_WYRMBREAKER:
                    uiWyrmbreakerGUID = creature->GetGUID();
                    break;
                case NPC_VALIONA:
                    uiValionaGUID = creature->GetGUID();
                    break;
                case NPC_THERALION:
                    uiTheralionGUID = creature->GetGUID();
                    break;
                case NPC_FELUDIUS:
                    uiFeludiusGUID = creature->GetGUID();
                    break;
                case NPC_IGNACIOUS:
                    uiIgnaciousGUID = creature->GetGUID();
                    break;
                case NPC_ARION:
                    uiArionGUID = creature->GetGUID();
                    break;
                case NPC_TERRASTRA:
                    uiTerrastraGUID = creature->GetGUID();
                    break;
                case NPC_MONSTROSITY:
                    uiMonstrosityGUID = creature->GetGUID();
                    break;
                case NPC_CHOGALL:
                    uiChogallGUID = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* pGo)
        {
            switch (pGo->GetEntry())
            {
                case GO_WHELP_CAGE:
                    uiWhelpCageGUID = pGo->GetGUID();
                    break;
                case DOOR_HALFUS_ENTRANCE:
                    uiDoorHalfusEntranceGUID = pGo->GetGUID();
                    AddDoor(pGo, true);
                    break;
                case DOOR_HALFUS_EXIT:
                    uiDoorHalfusExitGUID = pGo->GetGUID();
                    AddDoor(pGo, true);
                    break;
                case DOOR_THER_ENTRANCE:
                    uiDoorTherEntranceGUID = pGo->GetGUID();
                    AddDoor(pGo, true);
                    break;
                case DOOR_THER_EXIT:
                    uiDoorTherExitGUID = pGo->GetGUID();
                    AddDoor(pGo, true);
                    break;
                case DOOR_COUNCIL_ENTRANCE:
                    uiDoorCouncilEntranceGUID = pGo->GetGUID();
                    AddDoor(pGo, true);
                    break;
                case DOOR_COUNCIL_EXIT:
                    uiDoorCouncilExitGUID = pGo->GetGUID();
                    AddDoor(pGo, true);
                    break;
                case DOOR_CHOGALL_ENTRANCE:
                    uiDoorChogallEntranceGUID = pGo->GetGUID();
                    AddDoor(pGo, true);
                    break;
                case GO_SINESTRA_DOOR:
                    AddDoor(pGo, true);
                    break;
                case GO_CHOGALL_FLOOR:
                    uiChogallFloorGUID = pGo->GetGUID();
                    if (GetBossState(DATA_CHOGALL) == DONE && (instance->GetDifficultyID() == DIFFICULTY_10_HC || instance->GetDifficultyID() == DIFFICULTY_25_HC))
                        HandleGameObject(uiChogallFloorGUID, true);
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 identifier) const
        {
            switch (identifier)
            {
                case DATA_HALFUS:
                    return uiWyrmbreakerGUID;
                case DATA_VALIONA:
                    return uiValionaGUID;
                case DATA_THERALION:
                    return uiTheralionGUID;
                case DATA_FELUDIUS:
                    return uiFeludiusGUID;
                case DATA_IGNACIOUS:
                    return uiIgnaciousGUID;
                case DATA_ARION:
                    return uiArionGUID;
                case DATA_TERRASTRA:
                    return uiTerrastraGUID;
                case DATA_MONSTROSITY:
                    return uiMonstrosityGUID;
                case DATA_WHELP_CAGE:
                    return uiWhelpCageGUID;
                case DATA_CHOGALL:
                    return uiChogallGUID;
            }
            return ObjectGuid::Empty;
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_HEALTH_VALIONA_THERALION:
                    return uiValionaTheralionHealth;
                case DATA_DLG_ENTRANCE:
                    return m_uiDialogs[0];
                case DATA_DLG_HALFUS:
                    return m_uiDialogs[1];
                case DATA_DLG_VALIONA_THERALION:
                    return m_uiDialogs[2];
                case DATA_DLG_COUNCIL_1:
                    return m_uiDialogs[3];
                case DATA_DLG_COUNCIL_2:
                    return m_uiDialogs[4];
                case DATA_DLG_COUNCIL_3:
                    return m_uiDialogs[5];
                case DATA_DLG_CHOGALL:
                    return m_uiDialogs[6];
                case DATA_DLG_SINESTRA:
                    return m_uiDialogs[7];
            }
            return 0;
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case DATA_HALFUS:
                    break;
                case DATA_VALIONA_THERALION:
                    break;
                case DATA_COUNCIL:
                    break;
                case DATA_CHOGALL:
                    if (state == DONE)
                    {
                        if (GetBossState(DATA_CHOGALL) == DONE && (instance->GetDifficultyID() == DIFFICULTY_10_HC || instance->GetDifficultyID() == DIFFICULTY_25_HC))
                            HandleGameObject(uiChogallFloorGUID, true);
                    }
                    break;
                case DATA_SINESTRA:
                    break;
                default:
                    break;
            }
            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_HEALTH_VALIONA_THERALION:
                    uiValionaTheralionHealth = data;
                    break;
                case DATA_DLG_ENTRANCE:
                    m_uiDialogs[0] = data;
                    SaveToDB();
                    break;
                case DATA_DLG_HALFUS:
                    m_uiDialogs[1] = data;
                    SaveToDB();
                    break;
                case DATA_DLG_VALIONA_THERALION:
                    m_uiDialogs[2] = data;
                    SaveToDB();
                    break;
                case DATA_DLG_COUNCIL_1:
                    m_uiDialogs[3] = data;
                    SaveToDB();
                    break;
                case DATA_DLG_COUNCIL_2:
                    m_uiDialogs[4] = data;
                    SaveToDB();
                    break;
                case DATA_DLG_COUNCIL_3:
                    m_uiDialogs[5] = data;
                    SaveToDB();
                    break;
                case DATA_DLG_CHOGALL:
                    m_uiDialogs[6] = data;
                    SaveToDB();
                    break;
                case DATA_DLG_SINESTRA:
                    m_uiDialogs[7] = data;
                    SaveToDB();
                    break;
            }
        }

        bool CheckRequiredBosses(uint32 bossId, uint32 entry, Player const* player = NULL) const
        {
            if (player && player->isGameMaster())
                return true;

            switch (bossId)
            {
                case DATA_VALIONA_THERALION:
                    if (GetBossState(DATA_HALFUS) != DONE)
                        return false;
                    break;
                case DATA_COUNCIL:
                    if (GetBossState(DATA_VALIONA_THERALION) != DONE)
                        return false;
                    break;
                case DATA_CHOGALL:
                    if (GetBossState(DATA_COUNCIL) != DONE)
                        return false;
                    break;
                case DATA_SINESTRA:
                    if (GetBossState(DATA_CHOGALL) != DONE)
                        return false;
                    break;
                default:   
                    break;
            }
            return true;
        }

        std::string GetDialogSaveData()
        {
            std::ostringstream saveStream;
            for (uint8 i = 0; i < 8; i++)
                saveStream << (uint32)m_uiDialogs[i] << " ";
            return saveStream.str();
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;

            std::ostringstream saveStream;
            saveStream << "B T " << GetBossSaveData() << GetDialogSaveData();

            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
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

            if (dataHead1 == 'B' && dataHead2 == 'T')
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }
                for (uint8 i = 0; i < 8; i++)
                {
                    uint32 tmpDlg;
                    loadStream >> tmpDlg;
                    if (tmpDlg != DONE)
                        tmpDlg = NOT_STARTED;
                    m_uiDialogs[i] = tmpDlg;
                }
            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        private:
            ObjectGuid uiWyrmbreakerGUID;
            uint32 uiTeamInInstance;
            uint32 m_uiEncounter[MAX_ENCOUNTER];
            uint32 m_uiDialogs[8];
            ObjectGuid uiValionaGUID;
            ObjectGuid uiTheralionGUID;
            ObjectGuid uiFeludiusGUID;
            ObjectGuid uiArionGUID;
            ObjectGuid uiIgnaciousGUID;
            ObjectGuid uiTerrastraGUID;
            ObjectGuid uiMonstrosityGUID;
            uint32 uiValionaTheralionHealth;
            ObjectGuid uiChogallGUID;
            ObjectGuid uiWhelpCageGUID;
            ObjectGuid uiDoorHalfusEntranceGUID;
            ObjectGuid uiDoorHalfusExitGUID;
            ObjectGuid uiDoorTherEntranceGUID;
            ObjectGuid uiDoorTherExitGUID;
            ObjectGuid uiDoorCouncilEntranceGUID;
            ObjectGuid uiDoorCouncilExitGUID;
            ObjectGuid uiDoorChogallEntranceGUID;
            ObjectGuid uiChogallFloorGUID;
    };
};
void AddSC_instance_bastion_of_twilight()
{
    new instance_bastion_of_twilight();
}