#include "grim_batol.h"

class instance_grim_batol : public InstanceMapScript
{
public:
    instance_grim_batol() : InstanceMapScript("instance_grim_batol", 670) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_grim_batol_InstanceMapScript(map);
    }

    struct instance_grim_batol_InstanceMapScript : public InstanceScript
    {
        instance_grim_batol_InstanceMapScript(Map* map) : InstanceScript(map) {}
        
        ObjectGuid uiGeneralUmbrissGUID;
        ObjectGuid uiForgemasterThrongusGUID;
        ObjectGuid uiDrahgaShadowburnerGUID;
        ObjectGuid uiErudaxGUID;
        
        void Initialize() override
        {
            SetBossNumber(MAX_ENCOUNTER);
                
            uiGeneralUmbrissGUID.Clear();
            uiForgemasterThrongusGUID.Clear();
            uiDrahgaShadowburnerGUID.Clear();
            uiErudaxGUID.Clear();
        }     
        
        void OnCreatureCreate(Creature* creature) override
        {
            switch(creature->GetEntry())
            {
                case NPC_GENERAL_UMBRISS:
                    uiGeneralUmbrissGUID = creature->GetGUID();
                    break;
                case NPC_FORGEMASTER_THRONGUS:
                    uiForgemasterThrongusGUID = creature->GetGUID();
                    break;
                case NPC_DRAHGA_SHADOWBURNER:
                    uiDrahgaShadowburnerGUID = creature->GetGUID();
                    break;
                case NPC_ERUDAX:
                    uiErudaxGUID = creature->GetGUID();
                    break;
            }
        }
        
        ObjectGuid GetGuidData(uint32 identifier) const
        {
            switch(identifier)
            {
                case DATA_GENERAL_UMBRISS:
                    return uiGeneralUmbrissGUID;
                case DATA_FORGEMASTER_THRONGUS:
                    return uiForgemasterThrongusGUID;
                case DATA_DRAHGA_SHADOWBURNER:
                    return uiDrahgaShadowburnerGUID;
                case DATA_ERUDAX:
                    return uiErudaxGUID;
            }
            return ObjectGuid::Empty;
        }
        
        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "G B " << GetBossSaveData();

            OUT_SAVE_INST_DATA_COMPLETE;
            return saveStream.str();
        }

        void Load(const char* in) override
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

            if (dataHead1 == 'G' && dataHead2 == 'B')
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
    };
};

void AddSC_instance_grim_batol()
{
    new instance_grim_batol();
}
