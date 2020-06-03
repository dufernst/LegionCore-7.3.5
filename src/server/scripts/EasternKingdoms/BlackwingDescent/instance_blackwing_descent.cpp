#include "blackwing_descent.h"

#define MAX_ENCOUNTER 6

DoorData const doorData[] =
{
    {GO_INNER_CHAMBER_DOOR,         DATA_MAGMAW,    DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {GO_INNER_CHAMBER_DOOR,         DATA_OMNOTRON,  DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {0,                             0,              DOOR_TYPE_ROOM,     BOUNDARY_NONE},
};

class instance_blackwing_descent : public InstanceMapScript
{
public:
    instance_blackwing_descent() : InstanceMapScript("instance_blackwing_descent", 669) { }

    InstanceScript* GetInstanceScript(InstanceMap* pMap) const
    {
        return new instance_blackwing_descent_InstanceMapScript(pMap);
    }

    struct instance_blackwing_descent_InstanceMapScript : public InstanceScript
    {
        instance_blackwing_descent_InstanceMapScript(Map* pMap) : InstanceScript(pMap) 
        {
            SetBossNumber(MAX_ENCOUNTER);
            LoadDoorData(doorData);
            omnotronHealth          = 0;
            uiBileotron800Active    = 0;
            uiMagmawGUID.Clear();
            uiMagmawHeadGUID.Clear();
            uiMaloriakGUID.Clear();
            uiAberCount             = 18;
            uiChimaeronGUID.Clear();
            uiBileotron800GUID.Clear();
            uiAtramedesGUID.Clear();
            uiInnerChamberDoorGUID.Clear();
            uiNefarianGUID.Clear();
            uiOnyxiaGUID.Clear();
            uiNefarianFloorGUID.Clear();
            for (int32 i = 0; i < 4; ++i)
                m_uiOmnotronGUID[i].Clear();
        }

        void BeforePlayerEnter(Player* player)
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

            switch(creature->GetEntry())
            {
            case NPC_MAGMAW:
                uiMagmawGUID = creature->GetGUID();
                break;
            case NPC_MAGMAW_HEAD:
                uiMagmawHeadGUID = creature->GetGUID();
                break;
            case NPC_OMNOTRON:
                uiOmnotronControllerGUID = creature->GetGUID();
                break;                    
            case NPC_ARCANOTRON:
                m_uiOmnotronGUID[0] = creature->GetGUID();
                break;
            case NPC_ELECTRON:
                m_uiOmnotronGUID[1] = creature->GetGUID();
                break;
            case NPC_MAGMATRON:
                m_uiOmnotronGUID[2] = creature->GetGUID();
                break;
            case NPC_TOXITRON:
                m_uiOmnotronGUID[3] = creature->GetGUID();
                break;
            case NPC_MALORIAK:
                uiMaloriakGUID = creature->GetGUID();
                break;
            case NPC_CHIMAERON:
                uiChimaeronGUID = creature->GetGUID();
                break;
            case NPC_BILE_O_TRON_800:
                uiBileotron800GUID = creature->GetGUID();
                break;
            case NPC_ATRAMEDES:
                uiAtramedesGUID = creature->GetGUID();
                break;
            case NPC_LORD_VICTOR_NEFARIAN:
                uiLordNefarianGUID = creature->GetGUID();
                break;
            case NPC_NEFARIAN:
                uiNefarianGUID = creature->GetGUID();
                break;
            case NPC_ONYXIA:
                uiOnyxiaGUID = creature->GetGUID();
                break;
            }
        }

        void OnGameObjectCreate(GameObject *pGo)
        {
            switch (pGo->GetEntry())
            {
            case GO_INNER_CHAMBER_DOOR:
                AddDoor(pGo, true);
                break;
            case GO_NEFARIAN_FLOOR:
                uiNefarianFloorGUID = pGo->GetGUID();
                break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            switch (type)
            {
            case DATA_MAGMAW:
                return uiMagmawGUID;
            case DATA_MAGMAW_HEAD:
                return uiMagmawHeadGUID;
            case DATA_OMNOTRON:
                return uiOmnotronControllerGUID;
            case DATA_ARCANOTRON:
                return m_uiOmnotronGUID[0];
            case DATA_ELECTRON:
                return m_uiOmnotronGUID[1];
            case DATA_MAGMATRON:
                return m_uiOmnotronGUID[2];
            case DATA_TOXITRON:
                return m_uiOmnotronGUID[3];
            case DATA_MALORIAK:
                return uiMaloriakGUID;
            case DATA_CHIMAERON:
                return uiChimaeronGUID;
            case DATA_BILE_O_TRON_800:
                return uiBileotron800GUID;
            case DATA_ATRAMEDES:
                return uiAtramedesGUID;
            case DATA_INNER_CHAMBER_DOOR:
                return uiInnerChamberDoorGUID;
            case DATA_NEFARIAN:
                return uiNefarianGUID;
            case DATA_ONYXIA:
                return uiOnyxiaGUID;
            case DATA_NEFARIAN_FLOOR:
                return uiNefarianFloorGUID;
            }
                return ObjectGuid::Empty;
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
            case DATA_HEALTH_OMNOTRON_SHARED:
                return omnotronHealth;
                break;
            case DATA_MALORIAK_ABERRATIONS:
                return uiAberCount;
                break;
            case DATA_BILE_O_TRON_800:
                return uiBileotron800Active;
                break;
            }
            return 0;
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
            case DATA_OMNOTRON:
                break;
            case DATA_MAGMAW:
                break;
            case DATA_MALORIAK:
                break;
            case DATA_CHIMAERON:
                break;
            case DATA_ATRAMEDES:
                break;
            case DATA_NEFARIAN:
                if (GameObject* onyxiaPlatform = instance->GetGameObject(uiNefarianFloorGUID))
                    onyxiaPlatform->SetGoState(GO_STATE_READY);
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
            case DATA_HEALTH_OMNOTRON_SHARED:
                omnotronHealth = data;
                break;
            case DATA_MALORIAK_ABERRATIONS:
                uiAberCount = data;
                break;
            case DATA_BILE_O_TRON_800:
                uiBileotron800Active = data;
                break;
            }
        }

        bool CheckRequiredBosses(uint32 bossId, uint32 entry, Player const* player = NULL) const
        {
            if (player && player->isGameMaster())
                return true;

            switch (bossId)
            {
            case DATA_CHIMAERON:
            case DATA_MALORIAK:
            case DATA_ATRAMEDES:
                if (GetBossState(DATA_MAGMAW) != DONE ||
                    GetBossState(DATA_OMNOTRON) != DONE)
                    return false;
                break;
            case DATA_NEFARIAN:
                if (GetBossState(DATA_MAGMAW) != DONE ||
                    GetBossState(DATA_OMNOTRON) != DONE ||
                    GetBossState(DATA_MALORIAK) != DONE ||
                    GetBossState(DATA_CHIMAERON) != DONE ||
                    GetBossState(DATA_ATRAMEDES) != DONE)
                    return false;
            default:   
                break;
            }
            return true;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;

            std::ostringstream saveStream;
            saveStream << "B D " << GetBossSaveData();

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

            if (dataHead1 == 'B' && dataHead2 == 'D')
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }
            } 
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

    private:
        uint32 omnotronHealth;
        uint32 uiAberCount;
        uint32 uiBileotron800Active;
        ObjectGuid uiMagmawGUID;
        ObjectGuid uiMagmawHeadGUID;
        ObjectGuid uiMaloriakGUID;
        ObjectGuid m_uiOmnotronGUID[4];
        ObjectGuid uiOmnotronControllerGUID;
        ObjectGuid uiChimaeronGUID;
        ObjectGuid uiBileotron800GUID;
        ObjectGuid uiAtramedesGUID;
        ObjectGuid uiInnerChamberDoorGUID;
        ObjectGuid uiNefarianGUID;
        ObjectGuid uiOnyxiaGUID;
        ObjectGuid uiNefarianFloorGUID;
        ObjectGuid uiLordNefarianGUID;
        uint32 uiTeamInInstance;
    };
};

void AddSC_instance_blackwing_descent()
{
    new instance_blackwing_descent();
}