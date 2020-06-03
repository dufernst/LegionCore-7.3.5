/*
    Dungeon : Court of Stars 110
*/

#include "court_of_stars.h"

DoorData const doorData[] =
{
    {GO_GERDO_DOOR,           DATA_CAPTAIN_GERDO,       DOOR_TYPE_PASSAGE,     BOUNDARY_NONE},
    {GO_MELANDRUS_DOOR_1,     DATA_TALIXAE,             DOOR_TYPE_PASSAGE,     BOUNDARY_NONE},
    {GO_MELANDRUS_DOOR_2,     DATA_MELANDRUS_EVENT,     DOOR_TYPE_PASSAGE,     BOUNDARY_NONE},
    {GO_MELANDRUS_DOOR_2,     DATA_MELANDRUS,           DOOR_TYPE_ROOM,        BOUNDARY_NONE}
};

class instance_court_of_stars : public InstanceMapScript
{
public:
    instance_court_of_stars() : InstanceMapScript("instance_court_of_stars", 1571) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_court_of_stars_InstanceMapScript(map);
    }

    struct instance_court_of_stars_InstanceMapScript : public InstanceScript
    {
        instance_court_of_stars_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        WorldLocation loc_res_pla;

        ObjectGuid guidEvent;
        ObjectGuid guidEventSave;
        ObjectGuid GerdoGUID;

        std::list<ObjectGuid> beaconsGUID;
        
        void Initialize() override
        {
            LoadDoorData(doorData);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_PATROL_CAPTAIN_GERDO:
                    GerdoGUID = creature->GetGUID();
                    break;
                case NPC_GERDO_ARCANE_BEACON:
                    beaconsGUID.push_back(creature->GetGUID());
                    break;
                case 107435:    
                    if (!guidEvent)
                        if (urand(0, 100) >= 70)
                            guidEvent = creature->GetGUID(); 
                    guidEventSave = creature->GetGUID(); 
                    break;
            } 
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_GERDO_DOOR:
                case GO_MELANDRUS_DOOR_1:
                case GO_MELANDRUS_DOOR_2:
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
            
            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_BEACON_ACTIVATE:
                    if (Creature* gerdo = instance->GetCreature(GerdoGUID))
                    {
                        for (std::list<ObjectGuid>::iterator itr = beaconsGUID.begin(); itr != beaconsGUID.end(); ++itr)
                            if (Creature* beacon = instance->GetCreature(*itr))
                                if (beacon && !beacon->HasAura(210257)) //SPELL_BEACON_DISABLED
                                    beacon->CastSpell(beacon, 210435, true, 0, 0, gerdo->GetGUID()); //SPELL_SUM_VIGILANT_NIGHTWATCH
                    }
                    break;
                default:
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case 107435:   
                    return !guidEvent ? guidEventSave : guidEvent;
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
            loc_res_pla.SetMapId(1571);

            uint32 graveyardId = 5432;

            if (GetBossState(DATA_TALIXAE) == DONE)
                graveyardId = 5483;
            else if (GetBossState(DATA_CAPTAIN_GERDO) == DONE)
                graveyardId = 5502;

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

void AddSC_instance_court_of_stars()
{
    new instance_court_of_stars();
}