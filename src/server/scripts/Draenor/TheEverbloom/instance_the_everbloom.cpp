/*
    Dungeon : The Everbloom 100
*/

#include "the_everbloom.h"

DoorData const doorData[] =
{
    {GO_WITHERBARK_DOOR_1,       DATA_WITHERBARK,         DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_WITHERBARK_DOOR_2,       DATA_WITHERBARK,         DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_YALNU_DOOR,              DATA_ARCHMAGE_SOL,       DOOR_TYPE_PASSAGE,    BOUNDARY_NONE}
};

class instance_the_everbloom : public InstanceMapScript
{
public:
    instance_the_everbloom() : InstanceMapScript("instance_the_everbloom", 1279) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_the_everbloom_InstanceMapScript(map);
    }

    struct instance_the_everbloom_InstanceMapScript : public InstanceScript
    {
        instance_the_everbloom_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        std::map<uint32, ObjectGuid> protectorsGUIDconteiner;
        std::list<ObjectGuid> yalnuGUIDconteiner;
        ObjectGuid yalnuGUIDdoor;

        void Initialize()
        {
            LoadDoorData(doorData);
            yalnuGUIDdoor.Clear();
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case DATA_PROTECTORS:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            for (std::map<uint32, ObjectGuid>::iterator itr = protectorsGUIDconteiner.begin(); itr != protectorsGUIDconteiner.end(); ++itr)
                                if (auto protectors = instance->GetCreature(itr->second))
                                    protectors->AI()->DespawnOnRespawn(THREE_SECONDS);
                            break;
                        case IN_PROGRESS:
                            for (std::map<uint32, ObjectGuid>::iterator itr = protectorsGUIDconteiner.begin(); itr != protectorsGUIDconteiner.end(); ++itr)
                                if (auto protectors = instance->GetCreature(itr->second))
                                    protectors->AI()->DoZoneInCombat(protectors, 150.0f);
                            break;
                        case DONE:
                            if (auto dulhu = instance->GetCreature(protectorsGUIDconteiner[NPC_DULHU]))
                                dulhu->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                            break;
                    }
                    break;
                }
                case DATA_YALNU:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                        {
                            for (std::list<ObjectGuid>::iterator itr = yalnuGUIDconteiner.begin(); itr != yalnuGUIDconteiner.end(); ++itr)
                                if (auto yalnuTrash = instance->GetCreature(*itr))
                                    yalnuTrash->DespawnOrUnsummon();
                            break;
                        }
                    }
                    break;
                }
            }

            return true;
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_LIFE_WARDEN_GOLA:
                case NPC_EARTHSHAPER_TELU:
                case NPC_DULHU:
                    protectorsGUIDconteiner[creature->GetEntry()] = creature->GetGUID();
                    break;
                case NPC_COLOSSAL_BLOW:
                case NPC_VICIOUS_MANDRAGORA:
                case NPC_GNARLED_ANCIENT:
                case NPC_SWIFT_SPROUTLING:
                case NPC_ENTANGLEMENT:
                case NPC_ENTANGLEMENT_PLR:
                    yalnuGUIDconteiner.push_back(creature->GetGUID());
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_WITHERBARK_DOOR_1:
                case GO_WITHERBARK_DOOR_2:
                case GO_YALNU_DOOR:
                    AddDoor(go, true);
                    break;
                case GO_YALNU_VISUAL_DOOR:
                    yalnuGUIDdoor = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void SetData(uint32 type, uint32 data) override {}

        ObjectGuid GetGuidData(uint32 type) const
        {
            std::map<uint32, ObjectGuid>::const_iterator itr = protectorsGUIDconteiner.find(type);
            if (itr != protectorsGUIDconteiner.end())
                return itr->second;

            return ObjectGuid::Empty;
        }

        uint32 GetData(uint32 type) const
        {
            return 0;
        }
    };
};

void AddSC_instance_the_everbloom()
{
    new instance_the_everbloom();
}