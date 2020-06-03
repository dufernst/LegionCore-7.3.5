#include "deadmines.h"
#include "Packets/MiscPackets.h"

#define MAX_ENCOUNTER 6

static const DoorData doordata[] = 
{
    {GO_FACTORY_DOOR,   DATA_GLUBTOK,   DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_MAST_ROOM_DOOR, DATA_HELIX,     DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_HEAVY_DOOR,     DATA_HELIX,     DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_FOUNDRY_DOOR,   DATA_FOEREAPER, DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_HEAVY_DOOR_2,   DATA_FOEREAPER, DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {0, 0, DOOR_TYPE_ROOM, BOUNDARY_NONE},
};

class instance_deadmines : public InstanceMapScript
{
    public:
        instance_deadmines() : InstanceMapScript("instance_deadmines", 36) {}

        InstanceScript* GetInstanceScript(InstanceMap* pMap) const
        {
            return new instance_deadmines_InstanceMapScript(pMap);
        }

        struct instance_deadmines_InstanceMapScript : public InstanceScript
        {
            instance_deadmines_InstanceMapScript(Map* pMap) : InstanceScript(pMap) 
            {
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doordata);
                
                uiGlubtokGUID.Clear();
                uiHelixGUID.Clear();
                uiOafGUID.Clear();
                uiFoereaperGUID.Clear();
                uiAdmiralGUID.Clear();
                uiCaptainGUID.Clear();

                GoblinTeleporterGUID.Clear();
                HeavyDoorGUID.Clear();
                HeavyDoor2GUID.Clear();
                TeamInInstance = 0;

                State = CANNON_NOT_USED;
                uiVanessaEvent = 0;
                TeamInInstance = 0;
            };

            void OnPlayerEnter(Player* player)
            {
                if (!TeamInInstance)
                    TeamInInstance = player->GetTeam();
            }

            void OnCreatureCreate(Creature *pCreature)
            {
                if (!TeamInInstance)
                {
                    Map::PlayerList const &players = instance->GetPlayers();
                    if (!players.isEmpty())
                        if (Player* player = players.begin()->getSource())
                            TeamInInstance = player->GetTeam();
                }

                switch (pCreature->GetEntry())
                {
                    case NPC_GLUBTOK:
                        uiGlubtokGUID = pCreature->GetGUID();
                        break;
                    case NPC_HELIX_GEARBREAKER:
                        uiHelixGUID = pCreature->GetGUID();
                        break;
                    case NPC_LUMBERING_OAF:
                        uiOafGUID = pCreature->GetGUID();
                        break;
                    case NPC_FOE_REAPER_5000:
                        uiFoereaperGUID = pCreature->GetGUID();
                        break;
                    case NPC_ADMIRAL_RIPSNARL:
                        uiAdmiralGUID = pCreature->GetGUID();
                        break;
                    case NPC_CAPTAIN_COOKIE:
                        uiCaptainGUID = pCreature->GetGUID();
                        break;
                    case NPC_KAGTHA:
                        if (TeamInInstance == ALLIANCE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_SLINKY_SHARPSHIV:
                        if (TeamInInstance == ALLIANCE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_MISS_MAYHEM:
                        if (TeamInInstance == ALLIANCE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_MAYHEM_REAPER:
                        if (TeamInInstance == ALLIANCE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_HAND_ASSASIN:
                        if (TeamInInstance == ALLIANCE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_HORATIO_LAINE:
                        if (TeamInInstance == HORDE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_DEFENDER:
                        if (TeamInInstance == HORDE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_INVESTIGATOR:
                        if (TeamInInstance == HORDE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                    case NPC_CRIME_SCENE_BOT:
                        if (TeamInInstance == HORDE)
                            pCreature->SetPhaseMask(2, true);
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* pGo)
            {
                switch(pGo->GetEntry())
                {
                    case GO_HEAVY_DOOR:
                        HeavyDoorGUID = pGo->GetGUID();
                        AddDoor(pGo, true);
                        break;
                    case GO_HEAVY_DOOR_2:
                        HeavyDoor2GUID = pGo->GetGUID();
                        AddDoor(pGo, true);
                        break;
                    case GO_FACTORY_DOOR:
                    case GO_MAST_ROOM_DOOR:
                    case GO_FOUNDRY_DOOR:
                        AddDoor(pGo, true);
                        break;
                    case GO_GOBLIN_TELEPORTER:
                        GoblinTeleporterGUID = pGo->GetGUID();
                        break;
                    case GO_IRONCLAD_DOOR:
                        IronCladDoorGUID = pGo->GetGUID();
                        break;
                    case GO_DEFIAS_CANNON:
                        DefiasCannonGUID = pGo->GetGUID();
                        break;
                    case GO_DOOR_LEVER:
                        DoorLeverGUID = pGo->GetGUID();
                        break;
                }
            }

            void ShootCannon()
            {
                if (GameObject *pDefiasCannon = instance->GetGameObject(DefiasCannonGUID))
                {
                    pDefiasCannon->SetGoState(GO_STATE_ACTIVE);
                    DoPlaySound(pDefiasCannon, SOUND_CANNONFIRE);
                }
            }

            void BlastOutDoor()
            {
                if (GameObject *pIronCladDoor = instance->GetGameObject(IronCladDoorGUID))
                {
                    pIronCladDoor->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                    DoPlaySound(pIronCladDoor, SOUND_DESTROYDOOR);
                }
            }

            void DoPlaySound(GameObject* unit, uint32 soundKitID)
            {
                unit->SendMessageToSet(WorldPackets::Misc::PlaySound(unit->GetGUID(), soundKitID).Write(), false);
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_CANNON_EVENT:
                        State = data;
                        if (data == CANNON_BLAST_INITIATED)
                        {
                            ShootCannon();
                            BlastOutDoor();
                        }    
                        break;
                    case DATA_VANESSA_EVENT:
                        uiVanessaEvent = data;
                        if (data == DONE)
                            SaveToDB();
                        break;
                }
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_VANESSA_EVENT)
                    return uiVanessaEvent;
                if (type == DATA_TEAM_IN_INSTANCE)
                    return TeamInInstance;
                return 0;
            }

            ObjectGuid GetGuidData(uint32 data) const
            {
                switch (data)
                {
                    case DATA_GLUBTOK:
                        return uiGlubtokGUID;
                    case DATA_HELIX:
                        return uiHelixGUID;
                    case DATA_OAF:
                        return uiOafGUID;
                    case DATA_FOEREAPER:
                        return uiFoereaperGUID;
                    case DATA_ADMIRAL:
                        return uiAdmiralGUID;
                }
                return ObjectGuid::Empty;
            }
            
            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;
                    
                switch (type)
                {
                case DATA_HELIX:
                    if (state == IN_PROGRESS)
                    {
                        if (GameObject* go = instance->GetGameObject(HeavyDoorGUID))
                            go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    else if (state == DONE)
                        if (GameObject* go = instance->GetGameObject(HeavyDoorGUID))
                            go->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                case DATA_FOEREAPER:
                    if (state == IN_PROGRESS)
                    {
                        if (GameObject* go = instance->GetGameObject(HeavyDoor2GUID))
                            go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    else if (state == DONE)
                        if (GameObject* go = instance->GetGameObject(HeavyDoor2GUID))
                            go->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                }

                return true;
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::string str_data;
                std::ostringstream saveStream;
                saveStream << "D M " << GetBossSaveData() << State << " " << uiVanessaEvent << " ";
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

                if (dataHead1 == 'D' && dataHead2 == 'M')
                {

                    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }

                    loadStream >> State;

                    if (State == CANNON_BLAST_INITIATED)
                        if (GameObject *pIronCladDoor = instance->GetGameObject(IronCladDoorGUID))
                            pIronCladDoor->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);

                    loadStream >> uiVanessaEvent;
                    if (uiVanessaEvent != DONE)
                        uiVanessaEvent = NOT_STARTED;

                }
                else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

        private:
            uint32 TeamInInstance;

            ObjectGuid uiGlubtokGUID;
            ObjectGuid uiHelixGUID;
            ObjectGuid uiOafGUID;
            ObjectGuid uiFoereaperGUID;
            ObjectGuid uiAdmiralGUID;
            ObjectGuid uiCaptainGUID;

            ObjectGuid FactoryDoorGUID;
            ObjectGuid FoundryDoorGUID;
            ObjectGuid MastRoomDoorGUID;
            ObjectGuid GoblinTeleporterGUID;
            ObjectGuid HeavyDoorGUID;
            ObjectGuid HeavyDoor2GUID;
            ObjectGuid DefiasPirate1GUID;
            ObjectGuid DefiasPirate2GUID;
            ObjectGuid DefiasCompanionGUID;
            ObjectGuid IronCladDoorGUID;
            ObjectGuid DefiasCannonGUID;
            ObjectGuid DoorLeverGUID;

            uint32 State;
            uint32 uiVanessaEvent;
        };
};

void AddSC_instance_deadmines()
{
    new instance_deadmines();
}