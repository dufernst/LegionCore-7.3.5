/*
    Dungeon : Seat Of The Triumvirate 110
*/

#include "the_seat_of_the_triumvirate.h"

DoorData const doorData[] =
{
    { GO_WALL_AE,           DATA_ALLERIA1,       DOOR_TYPE_PASSAGE,     BOUNDARY_NONE },
    { GO_DOOR_LURA_1,       DATA_ALLERIA2,       DOOR_TYPE_PASSAGE,     BOUNDARY_NONE },
    { GO_DOOR_LURA_2,       DATA_LURA,           DOOR_TYPE_ROOM,        BOUNDARY_NONE },
    { 0,                    0,                   DOOR_TYPE_ROOM,        BOUNDARY_NONE }
};

class instance_seat_of_the_triumvirate : public InstanceMapScript
{
public:
    instance_seat_of_the_triumvirate() : InstanceMapScript("instance_seat_of_the_triumvirate", 1753) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_seat_of_the_triumvirate_InstanceMapScript(map);
    }

    struct instance_seat_of_the_triumvirate_InstanceMapScript : public InstanceScript
    {
        instance_seat_of_the_triumvirate_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        WorldLocation loc_res_pla;

        ObjectGuid ZuraalGUID;
        ObjectGuid SaprishGUID;
        ObjectGuid NezharGUID;
        ObjectGuid LuraGUID;
        ObjectGuid darkfangGUID;
        ObjectGuid shadewingGUID;
        ObjectGuid portEvent;
        uint32 luraIntro{};
        uint32 saprishPortals{};
        bool convers = false;

        void Initialize() override
        {
            LoadDoorData(doorData);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_ZURAAL:
                    ZuraalGUID = creature->GetGUID();
                    break;
                case NPC_SAPRISH:
                    SaprishGUID = creature->GetGUID();
                    break;
                case NPC_NEZHAR:
                    NezharGUID = creature->GetGUID();
                    break;
                case NPC_LURA:
                    LuraGUID = creature->GetGUID();
                    break;
                case NPC_SAPRISH_CONV_CONTROL:
                    portEvent = creature->GetGUID();
                    break;
                case NPC_SAPRISH_SHADEWING:
                    shadewingGUID = creature->GetGUID();
                    break;
                case NPC_SAPRISH_DARKFANG:
                    darkfangGUID = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
            case GO_WALL_AE:
            case GO_DOOR_LURA_1:
            case GO_DOOR_LURA_2:
                AddDoor(go, true);
                break;
            default:
                break;
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
            case LURA_INTRO_ADDS:
                luraIntro = data;
                SaveToDB();
                break;
            case SAPRISH_PORTALS:
                saprishPortals = data;
                SaveToDB();
                break;
            default:
                break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
            case LURA_INTRO_ADDS:
                return luraIntro;
            case SAPRISH_PORTALS:
                return saprishPortals;
            default:
                return 0;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
            case NPC_ZURAAL:
                return ZuraalGUID;
            case NPC_SAPRISH:
                return SaprishGUID;
            case NPC_NEZHAR:
                return NezharGUID;
            case NPC_LURA:
                return LuraGUID;
            case NPC_SAPRISH_CONV_CONTROL:
                return portEvent;
            case NPC_SAPRISH_SHADEWING:
                return shadewingGUID;
            case NPC_SAPRISH_DARKFANG:
                return darkfangGUID;
            }
            return ObjectGuid::Empty;
        }

        void CreatureDies(Creature* creature, Unit* /*killer*/) override
        {
            switch (creature->GetEntry())
            {
            case NPC_ZURAAL:
                DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 61071);
                break;
            case NPC_SAPRISH:
                DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 61070);
                break;
            case NPC_NEZHAR:
                DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 61069);
                break;
            case NPC_LURA:
                DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 61068);
                DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 58793);
                break;
            }
        }

        std::string GetSaveData() override
        {
            std::ostringstream saveStream;
            saveStream << "S O T " << GetBossSaveData() << luraIntro << saprishPortals;
            return saveStream.str();
        }

        void Load(const char* data) override
        {
            if (!data)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(data);

            char dataHead1, dataHead2, dataHead3;

            std::istringstream loadStream(data);
            loadStream >> dataHead1 >> dataHead2 >> dataHead3;

            if (dataHead1 == 'S' && dataHead2 == 'O' && dataHead3 == 'T')
            {
                for (uint32 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }
                loadStream >> saprishPortals >> luraIntro;
            }
            else
                OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;

        }

        void OnPlayerEnter(Player* player) override
        {
            if (!convers)
            {
                player->AddDelayedEvent(3500, [player, this]() -> void
                {
                    if (InstanceScript* instance = player->GetInstanceScript())
                        if (instance->GetBossState(DATA_ZURAAL) != DONE)
                        {
                            convers = true;
                            Conversation* conversation = new Conversation;
                            if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 6054, player, NULL, *player))
                                delete conversation;
                        }
                });
            }
        }

        WorldLocation* GetClosestGraveYard(float x, float y, float z) override
        {
            loc_res_pla.Relocate(x, y, z);
            loc_res_pla.SetMapId(1753);

            uint32 graveyardId = 6113;

            if (GetBossState(DATA_NEZHAR) == DONE)
                graveyardId = 6115;
            else if (GetBossState(DATA_SAPRISH) == DONE)
                graveyardId = 6114;

            if (graveyardId)
            {
                if (WorldSafeLocsEntry const* gy = sWorldSafeLocsStore.LookupEntry(graveyardId))
                {
                    loc_res_pla.Relocate(gy->Loc.X, gy->Loc.Y, gy->Loc.Z);
                    loc_res_pla.SetMapId(gy->MapID);
                }
            }
            return &loc_res_pla;
        }

        void Update(uint32 diff) override {}
    };
};

void AddSC_instance_seat_of_the_triumvirate()
{
    new instance_seat_of_the_triumvirate();
}
