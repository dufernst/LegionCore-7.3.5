/*
    Dungeon : Violet Hold Legion 100-110
*/

#include "ScriptMgr.h"
#include "violet_hold_legion.h"

class instance_violet_hold_legion : public InstanceMapScript
{
public:
    instance_violet_hold_legion() : InstanceMapScript("instance_violet_hold_legion", 1544) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_violet_hold_legion_InstanceMapScript(map);
    }

    struct instance_violet_hold_legion_InstanceMapScript : public InstanceScript
    {
        instance_violet_hold_legion_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        ObjectGuid uiKaahrj;
        ObjectGuid uiMillificent;
        ObjectGuid uiFesterface;
        ObjectGuid uiShivermaw;
        ObjectGuid uiAnubesset;
        ObjectGuid uiSaelorn;
        ObjectGuid uiThalena;
        ObjectGuid uiMalgath;
        ObjectGuid uiBetrug;

        ObjectGuid uiKaahrjCell;
        ObjectGuid uiMillificentCell;
        ObjectGuid uiFesterfaceCell;
        ObjectGuid uiShivermawCell;
        ObjectGuid uiAnubessetCell;
        ObjectGuid uiSaelornCell;
        ObjectGuid uiThalenaCell;

        ObjectGuid uiMainDoor;
        ObjectGuid uiActivationCrystal[4];
        ObjectGuid uiSinclari;

        std::set<ObjectGuid> trashMobs;

        uint8 uiMainEventPhase;
        uint8 uiWaveCount;
        uint8 uiFirstBoss;
        uint8 uiSecondBoss;
        uint8 uiRemoveNpc;
        uint8 uiDoorIntegrity;
        uint8 uiCountActivationCrystals;

        uint16 m_auiEncounter[MAX_ENCOUNTER];

        uint32 uiActivationTimer;

        bool bActive;
        bool bWiped;
        bool defenseless;

        std::string str_data;

        void Initialize() override
        {
            uiKaahrj.Clear();
            uiMillificent.Clear();
            uiFesterface.Clear();
            uiShivermaw.Clear();
            uiAnubesset.Clear();
            uiSaelorn.Clear();
            uiThalena.Clear();
            uiMalgath.Clear();
            uiBetrug.Clear();
            
            uiKaahrjCell.Clear();
            uiMillificentCell.Clear();
            uiFesterfaceCell.Clear();
            uiShivermawCell.Clear();
            uiAnubessetCell.Clear();
            uiSaelornCell.Clear();
            uiThalenaCell.Clear();

            uiMainDoor.Clear();
            uiSinclari.Clear();

            trashMobs.clear();

            uiWaveCount = 0;
            uiFirstBoss = 0;
            uiSecondBoss = 0;
            uiRemoveNpc = 0;
            uiCountActivationCrystals = 0;

            uiDoorIntegrity = 100;
            uiActivationTimer = 5000;

            bActive = false;
            defenseless = true;
            uiMainEventPhase = NOT_STARTED;

            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_SHADOW_BEAST:
                    SetGuidData(DATA_ADD_TRASH_MOB, creature->GetGUID());
                    break;
                case NPC_LIEUTENANT_SINCLARI:
                    uiSinclari = creature->GetGUID();
                    break;
                case NPC_MINDFLAYER_KAAHRJ:
                    uiKaahrj = creature->GetGUID();
                    break;
                case NPC_MILLIFICENT_MANASTORM:
                    uiMillificent = creature->GetGUID();
                    break;
                case NPC_FESTERFACE:
                    uiFesterface = creature->GetGUID();
                    break;
                case NPC_SHIVERMAW:
                    uiShivermaw = creature->GetGUID();
                    break;
                case NPC_ANUBESSET:
                    uiAnubesset = creature->GetGUID();
                    break;
                case NPC_SAELORN:
                    uiSaelorn = creature->GetGUID();
                    break;
                case NPC_PRINCESS_THALENA:
                    uiThalena = creature->GetGUID();
                    break;
                case NPC_LORD_MALGATH:
                    uiMalgath = creature->GetGUID();
                    break;
                case NPC_FEL_LORD_BETRUG:
                    uiBetrug = creature->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_MAIN_DOOR:
                    uiMainDoor = go->GetGUID();
                    break;
                case GO_KAAHRJ_DOOR:
                    uiKaahrjCell = go->GetGUID();
                    break;
                case GO_MILLIFICENT_DOOR:
                    uiMillificentCell = go->GetGUID();
                    break;
                case GO_FESTERFACE_DOOR:
                    uiFesterfaceCell = go->GetGUID();
                    break;
                case GO_SHIVERMAW_DOOR:
                    uiShivermawCell = go->GetGUID();
                    break;
                case GO_ANUBESSET_DOOR:
                    uiAnubessetCell = go->GetGUID();
                    break;
                case GO_SAELORN_DOOR:
                    uiSaelornCell = go->GetGUID();
                    break;
                case GO_THALENA_DOOR:
                    uiThalenaCell = go->GetGUID();
                    break;
                case GO_ACTIVATION_CRYSTAL:
                    go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (uiCountActivationCrystals < 4)
                        uiActivationCrystal[uiCountActivationCrystals++] = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void CreatureDies(Creature* creature, Unit* /*killer*/) override
        {
            if (creature->GetEntry() == NPC_SHADOW_BEAST)
                SetGuidData(DATA_DEL_TRASH_MOB, creature->GetGUID());
        }

        bool IsEncounterInProgress() const override
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)
                    return true;

            return false;
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            if (state == DONE)
            {
                if (type != DATA_BETRUG)
                {
                    if (GetData(DATA_WAVE_COUNT) == 6)
                    {
                        SetData(DATA_1ST_BOSS_EVENT, DONE);
                        SetData(DATA_WAVE_COUNT, 7);
                    }
                    else if (GetData(DATA_WAVE_COUNT) == 12)
                    {
                        SetData(DATA_2ND_BOSS_EVENT, DONE);
                        SetData(DATA_WAVE_COUNT, 13);
                    }
                }
                else
                {
                    SetData(DATA_BETRUG_EVENT, DONE);
                }
            }
            
            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_1ST_BOSS_EVENT:
                    m_auiEncounter[0] = data;
                    if (data == DONE)
                        SaveToDB();
                    break;
                case DATA_2ND_BOSS_EVENT:
                    m_auiEncounter[1] = data;
                    if (data == DONE)
                        SaveToDB();
                    break;
                case DATA_BETRUG_EVENT:
                    m_auiEncounter[2] = data;
                    if (data == DONE)
                    {
                        SetData(DATA_MAIN_DOOR, GO_STATE_ACTIVE);
                        uiMainEventPhase = DONE;
                        SaveToDB();
                    }
                    break;
                case DATA_START_BOSS_ENCOUNTER:
                    switch (uiWaveCount)
                    {
                        case 6:
                            StartBossEncounter(uiFirstBoss);
                            break;
                        case 12:
                            StartBossEncounter(uiSecondBoss);
                            break;
                    }
                    break;
                case DATA_WAVE_COUNT:
                    uiWaveCount = data;
                    bActive = true;
                    break;
                case DATA_REMOVE_NPC:
                    uiRemoveNpc = data;
                    break;
                case DATA_MAIN_DOOR:
                    if (GameObject* pMainDoor = instance->GetGameObject(uiMainDoor))
                        pMainDoor->SetGoState(static_cast<GOState>(data));
                    break;
                case DATA_DOOR_INTEGRITY:
                    uiDoorIntegrity = data;
                    defenseless = false;
                    DoUpdateWorldState(WorldStates::WORLD_STATE_VH_PRISON_STATE, uiDoorIntegrity);
                    break;
                case DATA_MAIN_EVENT_PHASE:
                    uiMainEventPhase = data;
                    if (data == IN_PROGRESS) // Start event
                    {
                        SetData(DATA_MAIN_DOOR, GO_STATE_READY);
                        uiWaveCount = 1;
                        bActive = true;
                        for (int i = 0; i < 4; ++i)
                            if (GameObject* crystal = instance->GetGameObject(uiActivationCrystal[i]))
                            {
                                crystal->EnableOrDisableGo(false);
                                crystal->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                            }
                        uiRemoveNpc = 0; // might not have been reset after a wipe on a boss.
                    }
                    break;
                default:
                    break;
            }
        }

        void SetGuidData(uint32 type, ObjectGuid data) override
        {
            switch (type)
            {
                case DATA_ADD_TRASH_MOB:
                    trashMobs.insert(data);
                    break;
                case DATA_DEL_TRASH_MOB:
                    trashMobs.erase(data);
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_MAIN_EVENT_PHASE:
                    return uiMainEventPhase;
                case DATA_1ST_BOSS_EVENT:
                    return m_auiEncounter[0];
                case DATA_2ND_BOSS_EVENT:
                    return m_auiEncounter[1];
                case DATA_BETRUG_EVENT:
                    return m_auiEncounter[2];
                case DATA_DOOR_INTEGRITY:
                    return uiDoorIntegrity;
                case DATA_REMOVE_NPC:
                    return uiRemoveNpc;
                case DATA_WAVE_COUNT:
                    return uiWaveCount;
                case DATA_FIRST_BOSS:
                    return uiFirstBoss;
                case DATA_SECOND_BOSS:
                    return uiSecondBoss;
            }

            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_SINCLARI:
                    return uiSinclari;
                case DATA_KAAHRJ:
                    return uiKaahrj;
                case DATA_MILLIFICENT:
                    return uiMillificent;
                case DATA_FESTERFACE:
                    return uiFesterface;
                case DATA_SHIVERMAW:
                    return uiShivermaw;
                case DATA_ANUBESSET:
                    return uiAnubesset;
                case DATA_SAELORN:
                    return uiSaelorn;
                case DATA_THALENA:
                    return uiThalena;
                case DATA_BETRUG:
                    return uiBetrug;
                case DATA_MAIN_DOOR:
                    return uiMainDoor;
                //Door Data
                case DATA_KAAHRJ_CELL:
                    return uiKaahrjCell;
                case DATA_MILLIFICENT_CELL:
                    return uiMillificentCell;
                case DATA_FESTERFACE_CELL:
                    return uiFesterfaceCell;
                case DATA_SHIVERMAW_CELL:
                    return uiShivermawCell;
                case DATA_ANUBESSET_CELL:
                    return uiAnubessetCell;
                case DATA_SAELORN_CELL:
                    return uiSaelornCell;
                case DATA_THALENA_CELL:
                    return uiThalenaCell;
            }
            return ObjectGuid::Empty;
        }

        void SpawnPortal()
        {
            if (Creature* pSinclari = instance->GetCreature(uiSinclari))
                pSinclari->SummonCreature(NPC_TELEPORTATION_PORTAL, PortalLocation[urand(0,4)]);
        }

        void StartBossEncounter(uint8 uiBoss, bool bForceRespawn = true)
        {
            Creature* pBoss = nullptr;

            switch (uiBoss)
            {
                case DATA_KAAHRJ:
                    HandleGameObject(uiKaahrjCell, bForceRespawn);
                    pBoss = instance->GetCreature(uiKaahrj);
                    break;
                case DATA_MILLIFICENT:
                    HandleGameObject(uiMillificentCell, bForceRespawn);
                    pBoss = instance->GetCreature(uiMillificent);
                    break;
                case DATA_FESTERFACE:
                    HandleGameObject(uiFesterfaceCell, bForceRespawn);
                    pBoss = instance->GetCreature(uiFesterface);
                    break;
                case DATA_SHIVERMAW:
                    HandleGameObject(uiShivermawCell, bForceRespawn);
                    pBoss = instance->GetCreature(uiShivermaw);
                    break;
                case DATA_ANUBESSET:
                    HandleGameObject(uiAnubessetCell, bForceRespawn);
                    pBoss = instance->GetCreature(uiAnubesset);
                    break;
                case DATA_SAELORN:
                    HandleGameObject(uiSaelornCell, bForceRespawn);
                    pBoss = instance->GetCreature(uiSaelorn);
                    break;
                case DATA_THALENA:
                    HandleGameObject(uiThalenaCell, bForceRespawn);
                    pBoss = instance->GetCreature(uiThalena);
                    break;
            }

            // generic boss state changes
            if (pBoss)
            {
                if (pBoss->isDead())
                {
                    // respawn but avoid to be looted again
                    pBoss->Respawn();
                    pBoss->AI()->DoAction(ACTION_REMOVE_LOOT);
                }

                if (!bForceRespawn)
                {
                    pBoss->SetReactState(REACT_PASSIVE);
                    pBoss->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
                    pBoss->SetHomePosition(pBoss->GetHomePosition().GetPositionX(), pBoss->GetHomePosition().GetPositionY(), pBoss->GetHomePosition().GetPositionZ(), pBoss->GetHomePosition().GetOrientation());
                    pBoss->NearTeleportTo(pBoss->GetHomePosition().GetPositionX(), pBoss->GetHomePosition().GetPositionY(), pBoss->GetHomePosition().GetPositionZ(), pBoss->GetHomePosition().GetOrientation());
                    uiWaveCount = 0;
                    if (Creature* betrug = instance->GetCreature(uiBetrug))
                    {
                        betrug->SetReactState(REACT_PASSIVE);
                        betrug->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
                        betrug->SetHomePosition(betrug->GetHomePosition().GetPositionX(), betrug->GetHomePosition().GetPositionY(), betrug->GetHomePosition().GetPositionZ(), betrug->GetHomePosition().GetOrientation());
                        betrug->NearTeleportTo(betrug->GetHomePosition().GetPositionX(), betrug->GetHomePosition().GetPositionY(), betrug->GetHomePosition().GetPositionZ(), 2.99f);
                        betrug->SetVisible(false);
                    }
                }
                else
                {
                    pBoss->SetHomePosition(centrPos);
                    pBoss->GetMotionMaster()->MovePoint(1, bossStartMove[uiBoss]);
                    if (uiBoss == 1) //manastorm event
                        pBoss->AI()->DoAction(1);
                    else
                    {
                        pBoss->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC|UNIT_FLAG_NON_ATTACKABLE);
                        pBoss->SetReactState(REACT_AGGRESSIVE);
                    }
                }
            }
        }

        void AddWave()
        {
            DoUpdateWorldState(WorldStates::WORLD_STATE_VH, 1);

            switch (uiWaveCount)
            {
                case 6:
                    if (uiFirstBoss == 0)
                        uiFirstBoss = urand(0, 6);
                    if (Creature* pSinclari = instance->GetCreature(uiSinclari))
                    {
                        if (Creature* malgath = pSinclari->SummonCreature(NPC_LORD_MALGATH, MiddleRoomSaboLoc, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                            malgath->CastSpell(malgath, SPELL_FEL_SHIELD, true);
                    }
                    break;
                case 12:
                    if (uiSecondBoss == 0)
                        do
                        {
                            uiSecondBoss = urand(0, 6);
                        } while (uiSecondBoss == uiFirstBoss);
                    if (Creature* pSinclari = instance->GetCreature(uiSinclari))
                    {
                        if (Creature* malgath = pSinclari->SummonCreature(NPC_LORD_MALGATH, MiddleRoomSaboLoc, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                            malgath->CastSpell(malgath, SPELL_FEL_SHIELD, true);
                    }
                    break;
                case 18:
                {
                    if (Creature* pSinclari = instance->GetCreature(uiSinclari))
                    {
                        if (Creature* malgath = pSinclari->SummonCreature(NPC_LORD_MALGATH, MiddleRoomSaboLoc, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                            malgath->CastSpell(malgath, SPELL_FEL_SHIELD, true);
                    }
                    break;
                }
                case 1:
                {
                    SetData(DATA_MAIN_DOOR, GO_STATE_READY);
                    DoUpdateWorldState(WorldStates::WORLD_STATE_VH_PRISON_STATE, 100);
                    // no break
                }
                default:
                    SpawnPortal();
                    break;
            }
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "V H " << (uint16)m_auiEncounter[0]
                << ' ' << (uint16)m_auiEncounter[1]
                << ' ' << (uint16)m_auiEncounter[2]
                << ' ' << (uint16)uiFirstBoss
                << ' ' << (uint16)uiSecondBoss;

            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
        }

        void Load(const char* in) override
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;
            uint16 data0, data1, data2, data3, data4;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3 >> data4;

            if (dataHead1 == 'V' && dataHead2 == 'H')
            {
                m_auiEncounter[0] = data0;
                m_auiEncounter[1] = data1;
                m_auiEncounter[2] = data2;

                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (m_auiEncounter[i] == IN_PROGRESS)
                        m_auiEncounter[i] = NOT_STARTED;

                uiFirstBoss = uint8(data3);
                uiSecondBoss = uint8(data4);
            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        bool CheckWipe()
        {
            Map::PlayerList const &players = instance->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                Player* player = itr->getSource();
                if (player->isGameMaster())
                    continue;

                if (player->isAlive())
                    return false;
            }

            return true;
        }

        void Reset_Event()
        {
            uiMainEventPhase = NOT_STARTED;
            uiDoorIntegrity = 100;
            SetData(DATA_REMOVE_NPC, 1);
            StartBossEncounter(uiFirstBoss, false);
            StartBossEncounter(uiSecondBoss, false);
            SetData(DATA_MAIN_DOOR, GO_STATE_ACTIVE);
            SetData(DATA_WAVE_COUNT, 0);
            DoUpdateWorldState(WorldStates::WORLD_STATE_VH, 0);

            for (int i = 0; i < 4; ++i)
                if (GameObject* crystal = instance->GetGameObject(uiActivationCrystal[i]))
                    crystal->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);

            std::set<ObjectGuid> tempMobs(trashMobs);
            for (auto itr = tempMobs.begin(); itr != tempMobs.end(); ++itr)
            {
                if (Creature* creature = instance->GetCreature(*itr))
                    if (creature && creature->isAlive())
                        creature->DespawnOrUnsummon();
            }

            trashMobs.clear();
            
            if (Creature* pSinclari = instance->GetCreature(uiSinclari))
            {
                pSinclari->SetVisible(true);
                std::list<Creature*> GuardList;
                pSinclari->GetCreatureListWithEntryInGrid(GuardList, NPC_VIOLET_HOLD_GUARD, 40.0f);
                if (!GuardList.empty())
                {
                    for (std::list<Creature*>::const_iterator itr = GuardList.begin(); itr != GuardList.end(); ++itr)
                    {
                        if (Creature* pGuard = *itr)
                        {
                            pGuard->SetVisible(true);
                            pGuard->SetReactState(REACT_AGGRESSIVE);
                            pGuard->GetMotionMaster()->MovePoint(1, pGuard->GetHomePosition());
                        }
                    }
                }
                pSinclari->GetMotionMaster()->MovePoint(1, pSinclari->GetHomePosition());
                pSinclari->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                pSinclari->AI()->DoAction(true);
            }
        }

        void Update(uint32 diff) override
        {
            if (!instance->HavePlayers())
                return;

            // portals should spawn if other portal is dead and doors are closed
            if (bActive && uiMainEventPhase == IN_PROGRESS)
            {
                if (uiActivationTimer <= diff)
                {
                    AddWave();
                    bActive = false;
                    // 30 seconds waiting time after each boss fight
                    uiActivationTimer = (uiWaveCount == 6 || uiWaveCount == 12) ? 30000 : 5000;
                } else uiActivationTimer -= diff;
            }

            // if main event is in progress and players have wiped then reset instance
            //if (uiMainEventPhase == IN_PROGRESS && CheckWipe())
            //    Reset_Event();
            
            if (!GetData(DATA_DOOR_INTEGRITY) && uiMainEventPhase == IN_PROGRESS)
            {
                uiMainEventPhase = NOT_STARTED;
                Reset_Event();
            }
        }

        void ActivateCrystal()
        {
            // just to make things easier we'll get the gameobject from the map
            GameObject* invoker = instance->GetGameObject(uiActivationCrystal[0]);
            if (!invoker)
                return;

            // the orb
            TempSummon* trigger = invoker->SummonCreature(NPC_DEFENSE_SYSTEM, MiddleRoomSaboLoc, TEMPSUMMON_MANUAL_DESPAWN, 0);
            if (!trigger)
                return;

            // visuals
            trigger->CastSpell(trigger, SPELL_ARCANE_LIGHTNING, true, 0, 0, trigger->GetGUID());

            // Kill all mobs registered with SetGuidData(ADD_TRASH_MOB)
            std::set<ObjectGuid> tempMobs(trashMobs);
            for (auto itr = tempMobs.begin(); itr != tempMobs.end(); ++itr)
            {
                Creature* creature = instance->GetCreature(*itr);
                if (creature && creature->isAlive())
                {
                    if (creature->GetEntry() == NPC_LORD_MALGATH)
                        creature->SetHealth(creature->GetHealth() * 0.7f);
                    else
                        trigger->Kill(creature);
                }
            }
        }

        void ProcessEvent(WorldObject* /*go*/, uint32 uiEventId) override
        {
            switch (uiEventId)
            {
                case EVENT_ACTIVATE_CRYSTAL:
                    ActivateCrystal();
                    break;
            }
        }
    };
};

void AddSC_instance_violet_hold_legion()
{
    new instance_violet_hold_legion();
}