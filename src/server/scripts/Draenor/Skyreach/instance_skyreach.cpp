/*
    Dungeon : Skyreach 97 - 99
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "skyreach.h"

DoorData const doorData[] =
{
    {GO_RANJIT_ENTER_DOOR,       DATA_RANJIT,        DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_RANJIT_EXIT_DOOR,        DATA_RANJIT,        DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_ARAKNATH_ENTER_DOOR_1,   DATA_ARAKNATH,      DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_ARAKNATH_ENTER_DOOR_2,   DATA_ARAKNATH,      DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_ARAKNATH_EXIT_DOOR_1,    DATA_ARAKNATH,      DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_ARAKNATH_EXIT_DOOR_2,    DATA_ARAKNATH,      DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_RUKHRAN_ENTER_DOOR,      DATA_RUKHRAN,       DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_RUKHRAN_EXIT_DOOR,       DATA_RUKHRAN,       DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_VIRYX_DOOR,              DATA_VIRYX,         DOOR_TYPE_ROOM,       BOUNDARY_NONE},
};

class instance_skyreach : public InstanceMapScript
{
public:
    instance_skyreach() : InstanceMapScript("instance_skyreach", 1209) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_skyreach_InstanceMapScript(map);
    }

    struct instance_skyreach_InstanceMapScript : public InstanceScript
    {
        instance_skyreach_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        ObjectGuid ArakkoanCacheGUID;
        
        void Initialize()
        {
            LoadDoorData(doorData);

            ArakkoanCacheGUID.Clear();
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;
            
            switch (type)
            {
                case DATA_RUKHRAN:
                    if (state == DONE)
                    {
                        if (GameObject* cache = instance->GetGameObject(ArakkoanCacheGUID))
                            cache->SetRespawnTime(cache->GetRespawnDelay());
                    }
                    break;
                case DATA_VIRYX:
                    if (state == DONE)
                    {
                        if (Creature* reshad = instance->SummonCreature(NPC_RESHAD, reshadPos[0]))
                            reshad->GetMotionMaster()->MovePoint(1, reshadPos[1]);
                    }
                    break;
            }

            return true;
        }

        void OnCreatureCreate(Creature* creature)
        {
            /* switch (creature->GetEntry())
            {
                case :
                    break;
            } */
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_RANJIT_ENTER_DOOR:
                case GO_RANJIT_EXIT_DOOR:
                case GO_ARAKNATH_ENTER_DOOR_1:
                case GO_ARAKNATH_ENTER_DOOR_2:
                case GO_ARAKNATH_EXIT_DOOR_1:
                case GO_ARAKNATH_EXIT_DOOR_2:
                case GO_RUKHRAN_ENTER_DOOR:
                case GO_RUKHRAN_EXIT_DOOR:
                case GO_VIRYX_DOOR:
                    AddDoor(go, true);
                    break;
                case GO_CACHE_ARAKKOAN_TREASURES:
                    ArakkoanCacheGUID = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            /* switch (type)
            {
                case :   
                    return ;
            } */
            return ObjectGuid::Empty;
        }

        void SetData(uint32 type, uint32 data)
        {
            /* switch (type)
            {
                case :
                    break;
                default:
                    break;
            } */
        }

        uint32 GetData(uint32 type) const
        {
            return 0;
        }

        /* void Update(uint32 diff) 
        {
            // Challenge
            InstanceScript::Update(diff);
        } */
    };
};

void AddSC_instance_skyreach()
{
    new instance_skyreach();
}