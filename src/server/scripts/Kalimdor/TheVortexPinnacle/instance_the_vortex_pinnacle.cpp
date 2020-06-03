#include "the_vortex_pinnacle.h"

#define MAX_ENCOUNTER 3

class instance_the_vortex_pinnacle : public InstanceMapScript
{
    public:
        instance_the_vortex_pinnacle() : InstanceMapScript("instance_the_vortex_pinnacle", 657) { }
        
        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_the_vortex_pinnacle_InstanceMapScript(map);
        }

        struct instance_the_vortex_pinnacle_InstanceMapScript: public InstanceScript
        {
            instance_the_vortex_pinnacle_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

            ObjectGuid uiGrandVizierErtanGUID;
            ObjectGuid uiAltairusGUID;
            ObjectGuid uiAsaadGUID;
            uint8 OrbsCount;

            void Initialize()
            {
                SetBossNumber(MAX_ENCOUNTER);
                uiGrandVizierErtanGUID.Clear();
                uiAltairusGUID.Clear();
                uiAsaadGUID.Clear();
                OrbsCount = 0;
            }

            void OnPlayerEnter(Player* player)
            {
                player->SendUpdateWorldState(5649, OrbsCount);
            }

            void OnCreatureCreate(Creature* pCreature)
            {
                switch(pCreature->GetEntry())
                {
                    case NPC_GRAND_VIZIER_ERTAN:
                        uiGrandVizierErtanGUID = pCreature->GetGUID();
                        break;
                    case NPC_ALTAIRUS:
                        uiAltairusGUID = pCreature->GetGUID();
                        break;
                    case NPC_ASAAD:
                        uiAsaadGUID = pCreature->GetGUID();
                        break;
                }
            }

            void SetData(uint32 data, uint32 value)
            {
                switch (data)
                {
                    case DATA_ORB:
                        OrbsCount = value;
                        break;
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_ORB:
                        return OrbsCount;
                }
    
                return 0;
            }

            ObjectGuid GetGuidData(uint32 identifier) const
            {
                switch(identifier)
                {
                    case DATA_ERTAN:
                        return uiGrandVizierErtanGUID;
                    case DATA_ALTAIRUS:
                        return uiAltairusGUID;
                    case DATA_ASAAD:
                        return uiAsaadGUID;
                }
                return ObjectGuid::Empty;
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::string str_data;
                std::ostringstream saveStream;
                saveStream << "V P" << GetBossSaveData(); 
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

                if (dataHead1 == 'V' && dataHead2 == 'P')
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

void AddSC_instance_the_vortex_pinnacle()
{
    new instance_the_vortex_pinnacle();
}
