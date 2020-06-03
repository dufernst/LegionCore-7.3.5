/*
    Dungeon : The Arcway 100-110
*/

#include "the_arcway.h"

DoorData const doorData[] =
{
    {GO_DOOR_9,                 DATA_IVANYR,          DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_DOOR_D,                 DATA_IVANYR,          DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_DOOR_G,                 DATA_IVANYR,          DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_CORSTILAX_DOOR_2,       DATA_CORSTILAX,       DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_DOOR_3,                 DATA_CORSTILAX,       DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_DOOR_4,                 DATA_CORSTILAX,       DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_DOOR_B,                 DATA_NALTIRA,         DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_DOOR_K,                 DATA_NALTIRA,         DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_DOOR_A,                 DATA_NALTIRA,         DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
};

class instance_the_arcway : public InstanceMapScript
{
public:
    instance_the_arcway() : InstanceMapScript("instance_the_arcway", 1516) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_the_arcway_InstanceMapScript(map);
    }

    struct instance_the_arcway_InstanceMapScript : public InstanceScript
    {
        instance_the_arcway_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        WorldLocation loc_res_pla;

        ObjectGuid VandrosGUID;

        bool onInitEnterState;
        bool TwoSay;
        uint8 RandTeleport;
        uint32 TwoSayTimer;

        void Initialize() override
        {
            LoadDoorData(doorData);

            onInitEnterState = false;
            TwoSay = false;
            TwoSayTimer = 0;
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_ADVISOR_VANDROS:
                    VandrosGUID = creature->GetGUID(); 
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_CORSTILAX_DOOR_1:
                case GO_CORSTILAX_DOOR_2:
                case GO_DOOR_A:
                case GO_DOOR_D:
                case GO_DOOR_G:
                case GO_DOOR_9:
                case GO_DOOR_3:
                case GO_DOOR_4:
                case GO_DOOR_B:
                case GO_DOOR_K:
                    AddDoor(go, true);
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            DoEventCreatures();

            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_RAND_TELEPORT:
                    RandTeleport = data;
                    break;
                default:
                    break;
            }
        }

        /* ObjectGuid GetGuidData(uint32 type) const
        {
            switch (type)
            {
                case NPC_:   
                    return GUID;
            }
            return ObjectGuid::Empty;
        } */

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_RAND_TELEPORT:
                    return RandTeleport;
            }
            return 0;
        }

        void OnPlayerEnter(Player* player) override
        {
            if (onInitEnterState)
                return;

            onInitEnterState = true;

            AddDelayedEvent(100, [=] () -> void
            {
                DoEventCreatures();
            });
        }

        bool checkBossDone()
        {
            for (uint8 i = 0; i < DATA_VANDROS; i++)
                if (GetBossState(i) != DONE)
                    return false;

            return true;
        }

        void DoEventCreatures()
        {
            if (!checkBossDone())
                return;

            if (Creature* vandros = instance->GetCreature(VandrosGUID))
            {
                vandros->AI()->ZoneTalk(0);
                vandros->SetReactState(REACT_AGGRESSIVE);
                vandros->SetVisible(true);
                TwoSay = true;
                TwoSayTimer = 20000;
            }
        }

        WorldLocation* GetClosestGraveYard(float x, float y, float z) override
        {
            // Init data
            loc_res_pla.Relocate(x, y, z);
            loc_res_pla.SetMapId(1516);

            uint32 graveyardId = 5194;

            if (checkBossDone())
                graveyardId = 5194;
            else if (GetBossState(DATA_XAKAL) == DONE)
                graveyardId = 5875;
            else if (GetBossState(DATA_NALTIRA) == DONE)
                graveyardId = 5876;
            else if (GetBossState(DATA_CORSTILAX) == DONE)
                graveyardId = 5877;
            else if (GetBossState(DATA_IVANYR) == DONE)
                graveyardId = 5878;

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

        void Update(uint32 diff) override
        {
            // Challenge
            InstanceScript::Update(diff);

            if (!TwoSay)
                return;
            
            if (TwoSayTimer <= diff)
            {
                if (Creature* vandros = instance->GetCreature(VandrosGUID))
                    vandros->AI()->ZoneTalk(1); 
                TwoSay = false;
            }
            else 
                TwoSayTimer -= diff;
        }
    };
};

void AddSC_instance_the_arcway()
{
    new instance_the_arcway();
}