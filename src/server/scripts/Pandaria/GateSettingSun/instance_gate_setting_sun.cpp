/*==============
==============*/

#include "gate_setting_sun.h"

DoorData const doorData[] =
{
    {GO_KIPTILAK_WALLS,                     DATA_KIPTILAK,              DOOR_TYPE_ROOM,         BOUNDARY_E   },
    {GO_KIPTILAK_WALLS,                     DATA_KIPTILAK,              DOOR_TYPE_ROOM,         BOUNDARY_N   },
    {GO_KIPTILAK_WALLS,                     DATA_KIPTILAK,              DOOR_TYPE_ROOM,         BOUNDARY_S   },
    {GO_KIPTILAK_WALLS,                     DATA_KIPTILAK,              DOOR_TYPE_ROOM,         BOUNDARY_W   },
    {GO_KIPTILAK_EXIT_DOOR,                 DATA_KIPTILAK,              DOOR_TYPE_PASSAGE,      BOUNDARY_N   },
    {GO_RIMAK_AFTER_DOOR,                   DATA_RIMOK,                 DOOR_TYPE_ROOM,         BOUNDARY_S   },
    {GO_RAIGONN_DOOR,                       DATA_RAIGONN,               DOOR_TYPE_ROOM,         BOUNDARY_NE  },
    {GO_RAIGONN_AFTER_DOOR,                 DATA_RAIGONN,               DOOR_TYPE_PASSAGE,      BOUNDARY_E   },
    {0,                                     0,                          DOOR_TYPE_ROOM,         BOUNDARY_NONE} // END
};

class instance_gate_setting_sun : public InstanceMapScript
{
public:
    instance_gate_setting_sun() : InstanceMapScript("instance_gate_setting_sun", 962) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_gate_setting_sun_InstanceMapScript(map);
    }

    struct instance_gate_setting_sun_InstanceMapScript : public InstanceScript
    {
        ObjectGuid kiptilakGuid;
        ObjectGuid gadokGuid;
        ObjectGuid rimokGuid;
        ObjectGuid raigonnGuid;
        ObjectGuid raigonWeakGuid;

        ObjectGuid firstDoorGuid;
        ObjectGuid fireSignalGuid;

        ObjectGuid wallCGuid;
        ObjectGuid portalTempGadokGuid;

        uint32 cinematicTimer;
        uint8 cinematicEventProgress;

        GuidList bombarderGuids;
        GuidList bombStalkerGuids;
        GuidList mantidBombsGUIDs;
        GuidList rimokAddGenetarorsGUIDs;
        GuidList artilleryGUIDs;
        GuidList secondaryDoorGUIDs;

        uint32 dataStorage[MAX_DATA];

        instance_gate_setting_sun_InstanceMapScript(Map* map) : InstanceScript(map) {}

        void Initialize() override
        {
            SetBossNumber(EncounterCount);
            LoadDoorData(doorData);

            kiptilakGuid.Clear();
            gadokGuid.Clear();
            rimokGuid.Clear();
            raigonnGuid.Clear();
            raigonWeakGuid.Clear();
            
            firstDoorGuid.Clear();

            cinematicTimer          = 0;
            cinematicEventProgress  = 0;

            wallCGuid.Clear();
            portalTempGadokGuid.Clear();

            memset(dataStorage, 0, MAX_DATA * sizeof(uint32));

            bombarderGuids.clear();
            bombStalkerGuids.clear();
            mantidBombsGUIDs.clear();
            rimokAddGenetarorsGUIDs.clear();
            artilleryGUIDs.clear();
            secondaryDoorGUIDs.clear();
        }

        void OnDestroy(InstanceMap* pMap)
        {
            if (Creature* weakSpot = instance->GetCreature(GetGuidData(NPC_WEAK_SPOT)))
                weakSpot->_ExitVehicle();
        }

        void OnPlayerEnter(Player* player) override
        {
            if (GetData(DATA_BRASIER_CLICKED) == NOT_STARTED)
                player->SetPhaseMask(1, true);
            else
                player->SetPhaseMask(2, true);
        }

        void OnPlayerLeave(Player* player) override
        {
            player->SetPhaseMask(1, true);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_KIPTILAK:          kiptilakGuid    = creature->GetGUID();                  return;
                case NPC_GADOK:             gadokGuid       = creature->GetGUID();                  return;
                case NPC_RIMOK:             rimokGuid       = creature->GetGUID();                  return;
                case NPC_RAIGONN:           raigonnGuid     = creature->GetGUID();                  return;
                case NPC_KRITHUK_BOMBARDER: bombarderGuids.push_back(creature->GetGUID());          return;
                case NPC_BOMB_STALKER:      bombStalkerGuids.push_back(creature->GetGUID());        return;
                case NPC_ADD_GENERATOR:     rimokAddGenetarorsGUIDs.push_back(creature->GetGUID()); return;
                case NPC_ARTILLERY:         artilleryGUIDs.push_back(creature->GetGUID());          return;
                default:                                                                            return;
            }
        }

        void OnCreatureRemove(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_KRITHUK_BOMBARDER:
                    for (GuidList::iterator it = bombarderGuids.begin(); it != bombarderGuids.end(); ++it)
                    {
                        if (*it == creature->GetGUID())
                        {
                            bombarderGuids.erase(it);
                            break;
                        }
                    }
                    break;
                default:                                                                            return;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_KIPTILAK_ENTRANCE_DOOR:
                    firstDoorGuid = go->GetGUID();
                    break;
                case GO_SIGNAL_FIRE:
                    fireSignalGuid = go->GetGUID();
                    break;
                case GO_KIPTILAK_WALLS:
                case GO_KIPTILAK_EXIT_DOOR:
                case GO_RIMAK_AFTER_DOOR:
                case GO_RAIGONN_AFTER_DOOR:
                    AddDoor(go, true);
                    return;
                case GO_KIPTILAK_MANTID_BOMBS:
                    mantidBombsGUIDs.push_back(go->GetGUID());
                    return;
                case GO_GREATDOOR_SECOND_DOOR:
                    secondaryDoorGUIDs.push_back(go->GetGUID());
                    HandleGameObject(go->GetGUID(), true, go);
                    return;
                case GO_WALL_C:
                    wallCGuid = go->GetGUID();
                    return;
                case GO_PORTAL_TEMP_GADOK:
                    portalTempGadokGuid = go->GetGUID();
                    return;
                default:
                    return;
            }
        }

        bool SetBossState(uint32 id, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case DATA_KIPTILAK:
                {
                    if (state == DONE)
                        for (GuidList::iterator itr = mantidBombsGUIDs.begin(); itr != mantidBombsGUIDs.end(); ++itr)
                            if (auto bomb = instance->GetGameObject(*itr))
                                bomb->SetPhaseMask(32768, true); // Set Invisible
                    break;
                }
                case DATA_GADOK:
                {
                    if (auto portal = instance->GetGameObject(portalTempGadokGuid))
                        portal->SetPhaseMask(state == IN_PROGRESS ? 4 : 3, true);
                    break;
                }
                case DATA_RIMOK:
                {
                    uint8 generatorsCount = 0;

                    for (GuidList::iterator itr = secondaryDoorGUIDs.begin(); itr != secondaryDoorGUIDs.end(); ++itr)
                        HandleGameObject(*itr, state != DONE);

                    for (GuidList::iterator itr = rimokAddGenetarorsGUIDs.begin(); itr != rimokAddGenetarorsGUIDs.end(); ++itr)
                    {
                        if (auto generator = instance->GetCreature(*itr))
                        {
                            if (generator->AI())
                            {
                                // There is 7 add generators, the middle one spawn saboteur
                                if (state == IN_PROGRESS && (++generatorsCount == 4))
                                    generator->AI()->DoAction(SPECIAL);
                                else
                                    generator->AI()->DoAction(state);
                            }
                        }
                    }
                    break;
                }
                case DATA_RAIGONN:
                {
                    for (GuidList::iterator itr = artilleryGUIDs.begin(); itr != artilleryGUIDs.end(); ++itr)
                    {
                        if (auto artillery = instance->GetCreature(*itr))
                        {
                            artillery->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE, state != IN_PROGRESS);
                            artillery->ApplyModFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK, state == IN_PROGRESS);
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            if (state == DONE && id != DATA_RAIGONN)
            {
                if (InstanceMap* im = instance->ToInstanceMap())
                {
                    InstanceScript* instance = im->GetInstanceScript();
                    int8 bossval = 0;
                    for (uint8 n = DATA_KIPTILAK; n <= DATA_RIMOK; n++)
                    {
                        if (instance->GetBossState(n) == DONE)
                            bossval++;
                        else
                            break;
                    }

                    if (bossval == 3)
                    {
                        if (Creature* raigonn = instance->instance->GetCreature(raigonnGuid))
                            raigonn->SetVisible(true);
                    }
                }
            }
            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_OPEN_FIRST_DOOR:
                {
                    if (dataStorage[type] == DONE)
                        return;

                    HandleGameObject(firstDoorGuid, true);

                    dataStorage[type] = data;
                    
                    if (GameObject* firstDoor = instance->GetGameObject(firstDoorGuid))
                    {

                        if (Creature* trigger = firstDoor->SummonTrigger(firstDoor->GetPositionX(), firstDoor->GetPositionY(), firstDoor->GetPositionZ(), 0, 500))
                        {
                            std::list<Creature*> defensorList;
                            GetCreatureListWithEntryInGrid(defensorList, trigger, 65337, 20.0f);

                            trigger->CastSpell(trigger, 115456); // Explosion

                            for (std::list<Creature*>::iterator itr = defensorList.begin(); itr != defensorList.end(); ++itr)
                            {
                                uint8 random = rand() % 2;

                                float posX = random ? 814.0f:  640.0f;
                                float posY = random ? 2102.0f: 2112.0f;
                                (*itr)->KnockbackFrom(posX, posY, 25.0f, 20.0f);
                                (*itr)->DespawnOrUnsummon(1000);
                            }
                        }
                    }
                    break;
                }
                case DATA_BRASIER_CLICKED:
                {
                    if (dataStorage[type] == DONE)
                        return;

                    if (GetBossState(DATA_GADOK) != DONE)
                        return;

                    Map::PlayerList const &PlayerList = instance->GetPlayers();
                    for (Map::PlayerList::const_iterator it = PlayerList.begin(); it != PlayerList.end(); ++it)
                    {
                        if (auto player = it->getSource())
                        {
                            player->SendCinematicStart(CINEMATIC_SETTING_SUN);
                            player->SetPhaseMask(2, true);
                            player->NearTeleportTo(1370.0f, 2283.6f, 402.328f, 2.70f);
                        }
                    }

                    cinematicTimer = 100;
                    dataStorage[type] = data;
                    break;
                }
                default:
                    if (type < MAX_DATA)
                        dataStorage[type] = data;
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_OPEN_FIRST_DOOR:
                default:
                    if (type < MAX_DATA)
                        return dataStorage[type];
                    break;
            }

            return 0;
        }

        void SetGuidData(uint32 type, ObjectGuid value) override
        {
            switch (type)
            {
                case NPC_WEAK_SPOT:
                    raigonWeakGuid = value;
                    break;
                default:
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case NPC_KIPTILAK:              return kiptilakGuid;
                case NPC_GADOK:                 return gadokGuid;
                case NPC_RIMOK:                 return rimokGuid;
                case NPC_RAIGONN:               return raigonnGuid;
                case NPC_WEAK_SPOT:             return raigonWeakGuid;
                case DATA_RANDOM_BOMBARDER:     return Trinity::Containers::SelectRandomContainerElement(bombarderGuids);
                case DATA_RANDOM_BOMB_STALKER:  return Trinity::Containers::SelectRandomContainerElement(bombStalkerGuids);
            }

            return ObjectGuid::Empty;
        }

        void doEventCinematic()
        {
            switch(cinematicEventProgress)
            {
                case 0:
                    // On allume le brasier & la meche
                    cinematicTimer = 6000;
                    break;
                case 1:
                    if (auto go = instance->GetGameObject(fireSignalGuid))
                        go->UseDoorOrButton();
                    cinematicTimer = 5000;
                    break;
                case 2:
                    if (auto go = instance->GetGameObject(wallCGuid))
                        go->ModifyHealth(-100000);
                    cinematicTimer = 0;
                    break;
            }

            ++cinematicEventProgress;
        }

        void Update(uint32 diff) override
        {
            // Challenge
            InstanceScript::Update(diff);

            if (cinematicTimer)
            {
                if (cinematicTimer <= diff)
                    doEventCinematic();
                else
                    cinematicTimer -= diff;
            }
        }
    };
};

void AddSC_instance_gate_setting_sun()
{
    new instance_gate_setting_sun();
}
