/*
    Dungeon : Dark Heart Thicket 100-110
*/

#include "darkheart_thicket.h"

DoorData const doorData[] =
{
    {GO_GLAIDALIS_FIRE_DOOR,     DATA_GLAIDALIS,     DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_DRESARON_FIRE_DOOR,      DATA_DRESARON,      DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_OAKHEART_DOOR,           DATA_OAKHEART,      DOOR_TYPE_ROOM,     BOUNDARY_NONE},
};

class instance_darkheart_thicket : public InstanceMapScript
{
public:
    instance_darkheart_thicket() : InstanceMapScript("instance_darkheart_thicket", 1466) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_darkheart_thicket_InstanceMapScript(map);
    }

    struct instance_darkheart_thicket_InstanceMapScript : public InstanceScript
    {
        instance_darkheart_thicket_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        WorldLocation loc_res_pla;

        ObjectGuid MalfurionGUID;
        ObjectGuid MalfurionCageGUID;
        ObjectGuid InvisDoorGUID;
        ObjectGuid DresaronGUID;

        void Initialize() override
        {
            LoadDoorData(doorData);
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;
            
            return true;
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_MALFURION_STORMRAGE:    
                    MalfurionGUID = creature->GetGUID();
                    break;
                case NPC_DRESARON:
                    DresaronGUID = creature->GetGUID();
                    break;
                case NPC_NIGHTMARE_BINDINGS:
                    MalfurionCageGUID = creature->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_GLAIDALIS_FIRE_DOOR:
                case GO_DRESARON_FIRE_DOOR:
                case GO_OAKHEART_DOOR:
                    AddDoor(go, true);
                    break;
                case GO_GLAIDALIS_INVIS_DOOR:
                    InvisDoorGUID = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            /*switch (type)
            {
                default:
                    break;
            }*/
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case NPC_MALFURION_STORMRAGE:   
                    return MalfurionGUID;
                case NPC_DRESARON:
                    return DresaronGUID;
                case NPC_NIGHTMARE_BINDINGS:
                    return MalfurionCageGUID;
                case GO_GLAIDALIS_INVIS_DOOR:
                    return InvisDoorGUID;
            }
            return ObjectGuid::Empty;
        }

        uint32 GetData(uint32 type) const override
        {
            return 0;
        }

        WorldLocation* GetClosestGraveYard(float x, float y, float z) override
        {
            loc_res_pla.Relocate(x, y, z);
            loc_res_pla.SetMapId(1466);

            uint32 graveyardId = 5334;

            if (GetBossState(DATA_GLAIDALIS) == DONE)
                graveyardId = 5874;

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

        /* void Update(uint32 diff) 
        {
            // Challenge
            InstanceScript::Update(diff);
        } */
    };
};

void AddSC_instance_darkheart_thicket()
{
    new instance_darkheart_thicket();
}