#include "Group.h"
#include "LFGMgr.h"
#include "return_to_karazhan.h"
#include "ScenarioMgr.h"

DoorData const doorData[] =
{
    {GO_OPERA_SCENE_LEFT_DOOR,  DATA_OPERA_HALL,        DOOR_TYPE_PASSAGE,      BOUNDARY_NONE},
    {GO_OPERA_SCENE_RIGHT_DOOR, DATA_OPERA_HALL,        DOOR_TYPE_ROOM,         BOUNDARY_NONE},
    {GO_OPERA_DOOR_TO_MOROES,   DATA_OPERA_HALL,        DOOR_TYPE_PASSAGE,      BOUNDARY_NONE},
    {GO_MOROES_DOOR_1,          DATA_MOROES,            DOOR_TYPE_ROOM,         BOUNDARY_NONE},
    {GO_MOROES_DOOR_2,          DATA_MOROES,            DOOR_TYPE_ROOM,         BOUNDARY_NONE},
    {GO_CURATOR_DOOR,           DATA_CURATOR,           DOOR_TYPE_PASSAGE,      BOUNDARY_NONE},
    {GO_MEDIVH_DOOR_1,          DATA_SHADE_OF_MEDIVH,   DOOR_TYPE_ROOM,         BOUNDARY_NONE},
    {GO_MEDIVH_DOOR_2,          DATA_SHADE_OF_MEDIVH,   DOOR_TYPE_PASSAGE,      BOUNDARY_NONE},
    {GO_VIZADUUM_DOOR_1,        DATA_VIZADUUM,          DOOR_TYPE_ROOM,         BOUNDARY_NONE},
};

class instance_return_to_karazhan : public InstanceMapScript
{
public:
    instance_return_to_karazhan() : InstanceMapScript("instance_return_to_karazhan", 1651) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_return_to_karazhan_InstanceMapScript(map);
    }

    struct instance_return_to_karazhan_InstanceMapScript : public InstanceScript
    {
        instance_return_to_karazhan_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        std::list<ObjectGuid> operaEncountersGUID;
        std::list<ObjectGuid> operaDecorGUID[4];
        std::list<ObjectGuid> otherGUIDcontainer;
        std::list<ObjectGuid> operaPatronGUIDList;

        ObjectGuid operaVelumGUID;
        ObjectGuid groupGuid;
        ObjectGuid phase2Door[3];

        uint8 OperaHallIntro;
        uint32 OperaHallScene;
        uint32 dataPH2Door;
        uint32 dungeonId;

        WorldLocation loc_res_pla;

        uint32 BonusBossTimer;

        void Initialize() override
        {
            LoadDoorData(doorData);

            OperaHallIntro = 0;
            OperaHallScene = 0;
            dataPH2Door = 0;
            dungeonId = 0;

            BonusBossTimer = 480000; // 8 minutes

            AddDelayedEvent(500, [this]() -> void
            {
                SetData(DATA_PHASE2_DOOR, DONE);
            });
        }

        void OnPlayerEnter(Player* player) override
        {
            if (groupGuid.IsEmpty())
            {
                groupGuid = player->GetGroup() ? player->GetGroup()->GetGUID() : ObjectGuid::Empty;

                if (!groupGuid.IsEmpty())
                    dungeonId = sLFGMgr->GetDungeon(groupGuid);
            }
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_GALINDRE:
                case NPC_ELFYRA:
                case NPC_MRRGRIA:
                case NPC_TOE_KNEE:
                case NPC_COGGLESTON:
                case NPC_LUMINORE:
                case NPC_BABBLET:
                case NPC_CAULDRONS:
                    operaEncountersGUID.push_back(creature->GetGUID());
                    break;
                case NPC_CAGED_ASSISTANT:
                case NPC_HOZEN_CAGE:
                    operaDecorGUID[3].push_back(creature->GetGUID());
                    break;
                case NPC_VIZADUUM_THE_WATCHER:
                    otherGUIDcontainer.push_back(creature->GetGUID());
                    break;
                case NPC_SKELETAL_USHER:
                case NPC_GHOSTLY_UNDERSTUDY:
                case NPC_SPECTRAL_PATRON:
                case NPC_SPECTRAL_PATRON_2:
                case NPC_PHANTOM_CREW:
                case NPC_BACKUP_SINGER:
                {
                    if (creature->GetPositionZ() <= 95.0f && GetBossState(DATA_OPERA_HALL) != DONE)
                    {
                        operaPatronGUIDList.push_back(creature->GetGUID());
                        creature->SetReactState(REACT_DEFENSIVE);
                        creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

                        if (creature->GetEntry() == NPC_SPECTRAL_PATRON_2 || creature->GetEntry() == NPC_BACKUP_SINGER)
                            creature->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_DANCESPECIAL);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void SummonedCreatureDespawn(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case NPC_CAGED_ASSISTANT:
                case NPC_HOZEN_CAGE:
                case NPC_GALINDRE:
                case NPC_ELFYRA:
                case NPC_COGGLESTON:
                case NPC_LUMINORE:
                case NPC_BABBLET:
                case NPC_CAULDRONS:
                    operaEncountersGUID.remove(creature->GetGUID());
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_OPERA_SCENE_LEFT_DOOR:
                case GO_OPERA_SCENE_RIGHT_DOOR:
                case GO_MOROES_DOOR_1:
                case GO_MOROES_DOOR_2:
                case GO_CURATOR_DOOR:
                case GO_MEDIVH_DOOR_1:
                case GO_MEDIVH_DOOR_2:
                case GO_VIZADUUM_DOOR_1:
                case GO_OPERA_DOOR_TO_MOROES:
                    AddDoor(go, true);
                    break;
                case GO_OPERA_DECOR_WIKKET_1:
                case GO_OPERA_DECOR_WIKKET_2:
                case GO_OPERA_DECOR_WIKKET_3:
                    operaDecorGUID[0].push_back(go->GetGUID());
                    break;
                case GO_OPERA_DECOR_WESTFALL_1:
                case GO_OPERA_DECOR_WESTFALL_2:
                case GO_OPERA_DECOR_WESTFALL_3:
                case GO_OPERA_DECOR_WESTFALL_4:
                case GO_OPERA_DECOR_WESTFALL_5:
                    operaDecorGUID[1].push_back(go->GetGUID());
                    break;
                case GO_OPERA_DECOR_BEAUTIFUL_BEAST_1:
                case GO_OPERA_DECOR_BEAUTIFUL_BEAST_2:
                case GO_OPERA_DECOR_BEAUTIFUL_BEAST_3:
                case GO_OPERA_DECOR_BEAUTIFUL_BEAST_4:
                case GO_OPERA_DECOR_BEAUTIFUL_BEAST_5:
                    operaDecorGUID[2].push_back(go->GetGUID());
                    break;
                case GO_OPERA_SCENE_VELUM:
                    operaVelumGUID = go->GetGUID();
                    break;
                case GO_GHOST_TRAP:
                    otherGUIDcontainer.push_back(go->GetGUID());
                    break;
                case GO_DOOR_INSTANCE_PH2_1:
                    phase2Door[0] = go->GetGUID();
                    break;
                case GO_DOOR_INSTANCE_PH2_2:
                    phase2Door[1] = go->GetGUID();
                    break;
                case GO_DOOR_INSTANCE_PH2_3:
                    phase2Door[2] = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case DATA_OPERA_HALL:
                {
                    if (operaEncountersGUID.empty())
                        break;

                    switch (state)
                    {
                        case NOT_STARTED:
                        {
                            ActionListGUIDManager(ACTION_ENCOUNTER_DESPAWN, ACTION_1);

                            AddDelayedEvent(10000, [this]() -> void
                            {
                                SetData(DATA_OPERA_HALL_INTRO, IN_PROGRESS);
                            });
                            break;
                        }
                        case IN_PROGRESS:
                        {
                            if (OperaHallScene != DATA_OPERA_HALL_WESTFALL)
                                ActionListGUIDManager(ACTION_ZONE_IN_COMBAT);
                            break;
                        }
                        case DONE:
                        {
                            if (!operaDecorGUID[3].empty())
                            {
                                for (auto const& guid : operaDecorGUID[3])
                                {
                                    if (auto decor = instance->GetCreature(guid))
                                    {
                                        if (decor->GetEntry() == NPC_HOZEN_CAGE)
                                            decor->SetAnimKitId(9779);

                                        if (decor->GetEntry() == NPC_CAGED_ASSISTANT)
                                        {
                                            Position pos;
                                            decor->GetNearPosition(pos, 5.0f, 0.0f);

                                            AddDelayedEvent(2000, [this, decor, pos]() -> void
                                            {
                                                decor->GetMotionMaster()->MovePoint(1, pos);
                                            });
                                        }
                                    }
                                }
                            }
                            for (auto const& guid : operaPatronGUIDList)
                            {
                                if (auto patron = instance->GetCreature(guid))
                                {
                                    if (patron->GetEntry() == NPC_SPECTRAL_PATRON_2 || patron->GetEntry() == NPC_BACKUP_SINGER)
                                        patron->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_ONESHOT_NONE);

                                    patron->SetReactState(REACT_AGGRESSIVE);
                                    patron->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                                }
                            }
                            DoUpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER, 1957);
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
                case DATA_MOROES:
                {
                    if (state == NOT_STARTED)
                    {
                        for (auto const& guid : otherGUIDcontainer)
                            if (GameObject* go = instance->GetGameObject(guid))
                                if (go->GetEntry() == GO_GHOST_TRAP)
                                    go->Respawn();
                    }
                    if (state == DONE)
                    {
                        instance->SummonCreature(NPC_MEDIVH, moroesScenePos[0]);
                        instance->SummonCreature(NPC_NIELAS_ARAN, moroesScenePos[1]);
                    }
                    break;
                }
                case DATA_VIZADUUM:
                {
                    if (state == DONE)
                    {
                        if (Creature* khadgar = instance->SummonCreature(NPC_ARCHMAGE_KHADGAR_VIZADUUM, khadgarEventPos[0]))
                            khadgar->CastSpell(khadgar, 229645, true);

                        if (Creature* medivh = instance->SummonCreature(NPC_MEDIVH_VIZADUUM, khadgarEventPos[1]))
                            medivh->CastSpell(medivh, 229651, true);

                        if (Creature* portal = instance->SummonCreature(NPC_DEMONIC_PORTAL, endPortalPos))
                            portal->CastSpell(portal, 229591, true);
                    }
                    break;
                }
                default:
                    break;
            }

            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_OPERA_HALL_INTRO:
                {
                    switch (data)
                    {
                        case SPECIAL:
                        {
                            if (!OperaHallScene)
                            {
                                OperaHallScene = urand(DATA_OPERA_HALL_WIKKET, DATA_OPERA_HALL_BEAUTIFUL_BEAST); // 1-3
                            //> DEBUG!!!
                                //OperaHallScene = DATA_OPERA_HALL_BEAUTIFUL_BEAST;
                            //< DEBUG!!!
                                BindPlayer();
                                SaveToDB();
                            }

                            if (!operaDecorGUID[OperaHallScene - 1].empty())
                                for (auto const& guid : operaDecorGUID[OperaHallScene - 1])
                                    if (GameObject* decor = instance->GetGameObject(guid))
                                        decor->SetRespawnTime(86400);
                            break;
                        }
                        case IN_PROGRESS:
                        {
                            switch (OperaHallScene)
                            {
                                case DATA_OPERA_HALL_WIKKET:
                                {
                                    if (OperaHallIntro != IN_PROGRESS)
                                    {
                                        for (uint8 i = 0; i < 10; i++)
                                            instance->SummonCreature(i < 5 ? NPC_CAGED_ASSISTANT : NPC_HOZEN_CAGE, operaSpawnPos[i]);

                                        //Boss Summons
                                        instance->SummonCreature(NPC_GALINDRE, operaSpawnPos[10]);
                                        instance->SummonCreature(NPC_ELFYRA, operaSpawnPos[12]);
                                    }
                                    else
                                    {
                                        //Boss Summons
                                        instance->SummonCreature(NPC_GALINDRE, operaSpawnPos[11]);
                                        instance->SummonCreature(NPC_ELFYRA, operaSpawnPos[13]);
                                    }
                                    break;
                                }
                                case DATA_OPERA_HALL_WESTFALL:
                                {
                                    //Boss Summons
                                    if (Creature* boss = instance->SummonCreature(NPC_MRRGRIA, operaSpawnPos[14]))
                                    {
                                        for (uint8 i = 19; i < 22; i++)
                                            boss->SummonCreature(NPC_SHORELINE_TIDESPEAKER, operaSpawnPos[i]);
                                    }
                                    if (Creature* boss = instance->SummonCreature(NPC_TOE_KNEE, operaSpawnPos[15]))
                                    {
                                        for (uint8 i = 16; i < 19; i++)
                                            boss->SummonCreature(NPC_GANG_RUFFIAN, operaSpawnPos[i]);
                                    }
                                    break;
                                }
                                case DATA_OPERA_HALL_BEAUTIFUL_BEAST:
                                {
                                    //Boss Summons
                                    instance->SummonCreature(NPC_COGGLESTON, operaSpawnPos[34]);
                                    instance->SummonCreature(NPC_LUMINORE, operaSpawnPos[35]);
                                    instance->SummonCreature(NPC_BABBLET, operaSpawnPos[36]);
                                    if (Creature* boss = instance->SummonCreature(NPC_CAULDRONS, operaSpawnPos[37]))
                                    {
                                        boss->SummonCreature(NPC_BELLA, operaSpawnPos[38]);
                                        // boss->SummonCreature(NPC_ADEM, operaSpawnPos[39]);
                                        boss->SummonCreature(NPC_BRUTE, operaSpawnPos[39]);
                                    }
                                    break;
                                }
                                default:
                                    break;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    if (data == IN_PROGRESS && OperaHallIntro != IN_PROGRESS)
                    {
                        OperaHallIntro = data;
                        HandleGameObject(operaVelumGUID, true);
                    }
                    break;
                }
                case DATA_OPERA_HALL_WIKKET:
                    if (data == SPECIAL)
                        ActionListGUIDManager(ACTION_WIKKET_INTRO_END);
                    break;
                case DATA_PHASE2_DOOR:
                    for (uint8 i = 0; i < 3; i++)
                        HandleGameObject(phase2Door[i], dataPH2Door == DONE ? true : false);
                    break;
                case DATA_TIMER_BONUS_BOSS:
                    BonusBossTimer += 5*60000;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_OPERA_HALL_INTRO:
                    return OperaHallIntro;
                case DATA_OPERA_HALL_SCENE:
                    return OperaHallScene;
                case DATA_TIMER_BONUS_BOSS:
                    return BonusBossTimer;
            }
            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            for (auto const& guid : operaEncountersGUID)
                if (Creature* creature = instance->GetCreature(guid))
                    if (creature->GetEntry() == type)
                        return guid;

            for (auto const& guid : otherGUIDcontainer)
            {
                if (Creature* creature = instance->GetCreature(guid))
                    if (creature->GetEntry() == type)
                        return guid;

                if (GameObject* go = instance->GetGameObject(guid))
                    if (go->GetEntry() == type)
                        return guid;
            }

            return ObjectGuid::Empty;
        }

        void ProcessEvent(WorldObject* obj, uint32 eventId) override
        {
            switch (eventId)
            {
                case 56130:
                    if (dataPH2Door != DONE)
                    {
                        if (instance->GetDifficultyID() == DIFFICULTY_HEROIC || instance->GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON)
                        {
                            dataPH2Door = DONE;
                            SaveToDB();
                        }
                        SetData(DATA_PHASE2_DOOR, DONE);
                    }
                    break;
                default:
                    break;
            }
        }

        void ActionListGUIDManager(uint8 action, uint8 DoActionId = 0)
        {
            if (operaEncountersGUID.empty())
                return;

            for (auto const& guid : operaEncountersGUID)
            {
                if (Creature* creature = instance->GetCreature(guid))
                {
                    switch (action)
                    {
                        case ACTION_WIKKET_INTRO_END:
                        {
                            creature->SetReactState(REACT_DEFENSIVE);
                            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1);

                            if (creature->GetEntry() == NPC_ELFYRA)
                                if (Creature* veh = creature->GetVehicleCreatureBase())
                                {
                                    veh->GetVehicleKit()->RemoveAllPassengers();
                                    veh->DespawnOrUnsummon(100);
                                }
                            break;
                        }
                        case ACTION_ZONE_IN_COMBAT:
                            creature->AI()->DoZoneInCombat();
                            break;
                        case ACTION_ENCOUNTER_DESPAWN:
                            creature->AI()->DoAction(DoActionId);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        void BindPlayer()
        {
            Map::PlayerList const& players = instance->GetPlayers();
            if (!players.isEmpty())
            {
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    if (Player* pPlayer = itr->getSource())
                        instance->ToInstanceMap()->PermBindAllPlayers(pPlayer);
                }
            }
        }

        std::string GetSaveData() override
        {
            std::ostringstream saveStream;
            saveStream << "R K " << GetBossSaveData() << OperaHallScene << ' ' << dataPH2Door;

            OUT_SAVE_INST_DATA_COMPLETE;
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

            char dataHead1, dataHead2;

            std::istringstream loadStream(data);
            loadStream >> dataHead1 >> dataHead2;

            if (dataHead1 == 'R' && dataHead2 == 'K')
            {
                for (uint32 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }

                loadStream >> OperaHallScene >> dataPH2Door;
            }
            else
                OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        WorldLocation* GetClosestGraveYard(float x, float y, float z) override
        {
            // Init data
            loc_res_pla.Relocate(x, y, z);
            loc_res_pla.SetMapId(1651);

            uint32 graveyardId = dungeonId == 1474 ? 5906 : 5905;

            if (instance->GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
            {
                if (Scenario* progress = sScenarioMgr->GetScenario(instance->GetInstanceId()))
                {
                    if (progress->GetScenarioId() == 1309) //Lower
                    {
                        graveyardId = 5905;
                    }
                    else if (progress->GetScenarioId() == 1308) //Upper
                    {
                        if (GetBossState(DATA_MANA_DEVOURER) == DONE)
                            graveyardId = 5824;
                        else if (GetBossState(DATA_SHADE_OF_MEDIVH) == DONE)
                            graveyardId = 5844;
                        else if (GetBossState(DATA_CURATOR) == DONE)
                            graveyardId = 5819;
                        else
                            graveyardId = 5906;
                    }
                }
            }
            else if (instance->GetDifficultyID() == DIFFICULTY_HEROIC || instance->GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || dungeonId == 1474)
            {
                if (GetBossState(DATA_MANA_DEVOURER) == DONE)
                    graveyardId = 5824;
                else if (GetBossState(DATA_SHADE_OF_MEDIVH) == DONE)
                    graveyardId = 5844;
                else if (GetBossState(DATA_CURATOR) == DONE && GetBossState(DATA_NIGHTBANE) == SPECIAL)
                    graveyardId = dungeonId == 1474 ? 5906 : 5905;
                else if (GetBossState(DATA_CURATOR) == DONE)
                    graveyardId = 5819;
            }

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
            if (BonusBossTimer)
            {
                if (BonusBossTimer <= diff)
                    BonusBossTimer = 0;
                else
                    BonusBossTimer -= diff;
            }
        }
    };
};

void AddSC_instance_return_to_karazhan()
{
    new instance_return_to_karazhan();
}