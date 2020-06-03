#include "baradin_hold.h"

DoorData const doorData[] =
{
    {GO_TOLBARAD_DOOR_2,    DATA_ARGALOTH,  DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_CELL_DOOR,          DATA_OCCUTHAR,  DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_TOLBARAD_DOOR_1,    DATA_ALIZABAL,  DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {0,                     0,              DOOR_TYPE_ROOM,       BOUNDARY_NONE}, // END
};

class instance_baradin_hold : public InstanceMapScript
{
    public:
        instance_baradin_hold() : InstanceMapScript("instance_baradin_hold", 757) { }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_baradin_hold_InstanceMapScript(map);
        }

        struct instance_baradin_hold_InstanceMapScript : public InstanceScript
        {
            instance_baradin_hold_InstanceMapScript(Map* map) : InstanceScript(map)
            {
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doorData);
            }
            
            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;
                
                return true;
            }

            void OnGameObjectCreate(GameObject* pGo)
            {
                switch (pGo->GetEntry())
                {
                    case GO_TOLBARAD_DOOR_2:
                    case GO_CELL_DOOR:
                    case GO_TOLBARAD_DOOR_1:
                        AddDoor(pGo, true);
                        break;
                }
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << "B H " << GetBossSaveData();

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

                if (dataHead1 == 'B' && dataHead2 == 'H')
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

        };
};

void AddSC_instance_baradin_hold()
{
    new instance_baradin_hold();
}
