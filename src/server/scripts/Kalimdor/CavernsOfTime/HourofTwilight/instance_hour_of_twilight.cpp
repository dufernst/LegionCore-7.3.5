#include "hour_of_twilight.h"

#define MAX_ENCOUNTER 3

static const DoorData doordata[] = 
{
    {GO_ICEWALL_1, DATA_ARCURION,   DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_ICEWALL_2, DATA_ARCURION,   DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {GO_GATE,      DATA_BENEDICTUS, DOOR_TYPE_ROOM,     BOUNDARY_NONE}
};

class instance_hour_of_twilight : public InstanceMapScript
{
    public:
        instance_hour_of_twilight() : InstanceMapScript("instance_hour_of_twilight", 940) { }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_hour_of_twilight_InstanceMapScript(map);
        }

        struct instance_hour_of_twilight_InstanceMapScript : public InstanceScript
        {
            instance_hour_of_twilight_InstanceMapScript(Map* map) : InstanceScript(map)
            {
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doordata);
                
                uiAsira.Clear();
            }
            
            void OnCreatureCreate(Creature* pCreature)
            {
                switch (pCreature->GetEntry())
                {
                    case NPC_ASIRA:
                        uiAsira = pCreature->GetGUID();
                        if (GetBossState(DATA_ASIRA)==DONE)
                            pCreature->SummonCreature(NPC_LIFE_WARDEN_2, drakePos);
                        break;
                    default:
                        break;
                }                
            }

            void OnGameObjectCreate(GameObject* pGo)
            {
                switch (pGo->GetEntry())
                {
                    case GO_ICEWALL_1:
                    case GO_ICEWALL_2:
                    case GO_GATE:
                        AddDoor(pGo, true);
                        break;
                    default:
                        break;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                return true;
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::string str_data;

                std::ostringstream saveStream;
                saveStream << "H o T " << GetBossSaveData();

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

                char dataHead1, dataHead2, dataHead3;

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2 >> dataHead3;

                if (dataHead1 == 'H' && dataHead2 == 'o' && dataHead3 == 'T')
                {
                    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }
                } else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

            private:
                ObjectGuid uiAsira;
        };
};

void AddSC_instance_hour_of_twilight()
{
    new instance_hour_of_twilight();
}