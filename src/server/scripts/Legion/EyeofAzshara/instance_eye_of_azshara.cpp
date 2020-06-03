/*
    Dungeon : Eye of Azshara 100-110
*/

#include "Group.h"
#include "eye_of_azshara.h"

Position const azsharaPos = { -3485.11f, 4386.22f, -5.58f };

std::vector<Position> azsharaLightningPos =
{
    {-3658.50f, 4541.43f, -1.11f},
    {-3542.42f, 4548.56f, -0.43f},
    {-3493.93f, 4520.60f, -0.43f},
    {-3456.24f, 4454.83f, -0.12f},
    {-3383.79f, 4469.41f,  0.35f},
    {-3355.67f, 4396.12f, -0.38f},
    {-3352.58f, 4318.04f,  0.98f},
    {-3353.09f, 4258.99f, -0.28f},
    {-3584.86f, 4297.77f, -1.52f},
    {-3623.71f, 4332.40f, -3.22f},
    {-3610.39f, 4370.81f, -0.22f},
    {-3593.22f, 4392.55f, -0.16f},
    {-3591.55f, 4435.23f, -0.42f},
    {-3624.21f, 4474.72f, -0.26f},
    {-3665.91f, 4495.70f, -1.00f}
};

DoorData const doorData[] =
{
    {GO_DEEPBEARD_DOOR,      DATA_DEEPBEARD,      DOOR_TYPE_ROOM,      BOUNDARY_NONE},
    {0,                      0,                   DOOR_TYPE_ROOM,      BOUNDARY_NONE} // END
};

class instance_eye_of_azshara : public InstanceMapScript
{
public:
    instance_eye_of_azshara() : InstanceMapScript("instance_eye_of_azshara", 1456) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_eye_of_azshara_InstanceMapScript(map);
    }

    struct instance_eye_of_azshara_InstanceMapScript : public InstanceScript
    {
        instance_eye_of_azshara_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        ObjectGuid SerpentrixGUID;
        ObjectGuid kingDeepbeardGUID;
        ObjectGuid bubbleGUID;
        ObjectGuid AzsharaGUID;
        ObjectGuid NagasContainerGUID[4];

        bool StartEvent = false;
        bool StormActive = false;
        uint8 NagasCount = 0;
        uint16 shelterTimer = 0;
        uint32 CheckBossTimer = 0;
        uint32 WindsTimer = 0;
        uint32 WindsDisableTimer = 0;
        uint32 PlayerCount = 0;
        uint32 StormTimer = 0;
        uint32 TempestTimer = 0;

        void Initialize() override
        {
            LoadDoorData(doorData);

            shelterTimer = 2000;
            CheckBossTimer = 2000;
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_WRATH_OF_AZSHARA:
                    AzsharaGUID = creature->GetGUID(); 
                    break;
                case NPC_KING_DEEPBEARD:
                    kingDeepbeardGUID = creature->GetGUID();
                    break;
                case NPC_SERPENTRIX:
                    SerpentrixGUID = creature->GetGUID();
                    break;
                case NPC_MYSTIC_SSAVEH:
                case NPC_RITUALIST_LESHA:
                case NPC_CHANNELER_VARISZ:
                case NPC_BINDER_ASHIOI:
                    NagasContainerGUID[NagasCount++] = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_DEEPBEARD_DOOR:
                    AddDoor(go, true);
                    break;
                case GO_AZSHARA_BUBBLE:
                    bubbleGUID = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            if (type == DATA_WRATH_OF_AZSHARA && state == DONE)
            {
                WindsTimer = 0;
                StormTimer = 0;
            }

            DoEventCreatures();

            if (state == DONE)
                ChangeWeather();

            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case ACTION_1:
                    WindsTimer = 10000;
                    break;
                default:
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case NPC_SERPENTRIX:
                    return SerpentrixGUID;
                case NPC_KING_DEEPBEARD:   
                    return kingDeepbeardGUID;
                case NPC_WRATH_OF_AZSHARA:
                    return AzsharaGUID;
            }
            return ObjectGuid::Empty;
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_WIND_ACTIVE:
                    return WindsDisableTimer;
                default:
                    return 0;
            }
        }

        bool CheckBossDone()
        {
            uint8 bossDiedCount = 0;

            for (uint8 i = 0; i < DATA_WRATH_OF_AZSHARA; ++i)
                if (GetBossState(i) == DONE)
                    ++bossDiedCount;

            if (!WindsTimer && bossDiedCount >= 2)
                WindsTimer = 60 * IN_MILLISECONDS;

            if (!StormActive && bossDiedCount >= 3)
            {
                StormActive = true;
                StormTimer = 10 * IN_MILLISECONDS;
            }

            if (bossDiedCount < 4)
            {
                TempestTimer = 2000;
                return false;
            }
            else
                TempestTimer = 0;

            return true;
        }

        void DoEventCreatures()
        {
            if (!CheckBossDone())
                return;

            if (auto boss = instance->GetCreature(AzsharaGUID))
            {
                if (auto bubble = instance->GetGameObject(bubbleGUID))
                    bubble->Delete();

                boss->SetVisible(true);
                boss->AI()->ZoneTalk(0); // "THE STORM AWAKENS"
            }

            for (uint8 i = 0; i < 4; ++i)
                if (auto naga = instance->GetCreature(NagasContainerGUID[i]))
                    naga->AI()->DoAction(true);   
        }

        void OnPlayerEnter(Player* player) override
        {
            if (!StartEvent)
            {
                if (++PlayerCount == 5)
                {
                    StartEvent = true;

                    if (Group *g = player->GetGroup())
                        if (Player* leader = ObjectAccessor::GetPlayer(*player, g->GetLeaderGUID()))
                            if (Creature* target = leader->FindNearestCreature(100216, 50.0f, true))
                                target->AI()->Talk(0);
                }
            }

            ChangeWeather();
        }

        void ChangeWeather()
        {
            uint8 bossDiedCount = 0;

            for (uint8 i = 0; i <= DATA_WRATH_OF_AZSHARA; ++i)
                if (GetBossState(i) == DONE)
                    ++bossDiedCount;

            switch (bossDiedCount)
            {
                case 1:
                    DoCastSpellOnPlayers(SPELL_SKYBOX_RAIN);
                    break;
                case 2:
                    DoCastSpellOnPlayers(SPELL_SKYBOX_WIND);
                    break;
                case 3:
                    DoCastSpellOnPlayers(SPELL_SKYBOX_LIGHTNING);
                    break;
                case 4:
                    DoCastSpellOnPlayers(SPELL_SKYBOX_HURRICANE);
                    break;
                case 5:
                    for (auto spellId : {SPELL_SKYBOX_RAIN, SPELL_SKYBOX_WIND, SPELL_SKYBOX_LIGHTNING, SPELL_SKYBOX_HURRICANE})
                        DoRemoveAurasDueToSpellOnPlayers(spellId);
                    break;
                default:
                    break;
            }
        }

        void Update(uint32 diff) override
        {
            if (CheckBossTimer)
            {
                if (CheckBossTimer <= diff)
                {
                    CheckBossTimer = 0;
                    DoEventCreatures();
                }
                else
                    CheckBossTimer -= diff;            
            }

            return;

            if (WindsTimer)
            {
                if (WindsTimer <= diff)
                {
                    WindsDisableTimer = 0;

                    if (auto azshara = instance->GetCreature(AzsharaGUID))
                    {
                        instance->ApplyOnEveryPlayer([&](Player* player)
                        {
                            if (player->isAlive() && !player->isGameMaster() && !player->HasAura(197134)) //Shelter
                                player->CastSpell(player, 191797, true); //Violent Winds
                        });

                        if (azshara->HealthBelowPct(11))
                            WindsTimer = 10000;
                        else
                            WindsTimer = 90 * IN_MILLISECONDS;

                        WindsDisableTimer = 8000;
                    }
                }
                else
                    WindsTimer -= diff;            
            }

            if (WindsDisableTimer)
            {
                if (WindsDisableTimer <= diff)
                {
                    WindsDisableTimer = 0;
                    DoRemoveAurasDueToSpellOnPlayers(191797);
                }
                else
                    WindsDisableTimer -= diff;
            }

            if (StormTimer)
            {
                if (StormTimer <= diff)
                {
                    StormTimer = 0;
                    std::list<ObjectGuid> playerGUIDList;
                    instance->ApplyOnEveryPlayer([&](Player* player) { playerGUIDList.push_back(player->GetGUID()); });
                    Trinity::Containers::RandomResizeList(playerGUIDList, 1);

                    if (!playerGUIDList.empty())
                    {
                        if (auto player = ObjectAccessor::GetObjectInMap(playerGUIDList.front(), instance, (Player*)nullptr))
                            if (player->GetCurrentAreaID() == 8040 || player->GetCurrentAreaID() == 8084)
                                player->CastSpell(player, 192796, true); //Lightning Strikes
                    }

                    if (auto boss = instance->GetCreature(AzsharaGUID))
                    {
                        if (boss->HealthBelowPct(11))
                            StormTimer = 2000;
                        else
                            StormTimer = 5000;
                    }
                }
                else
                    StormTimer -= diff;
            }

            if (shelterTimer <= diff)
            {
                instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (player->GetCurrentAreaID() == 8083)
                    {
                        if (!player->GetMap()->IsOutdoors(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ()))
                        {
                            if (!player->HasAura(197134))
                                player->CastSpell(player, 197134, true);
                        }
                        else
                        {
                            if (player->HasAura(197134))
                                player->RemoveAurasDueToSpell(197134);
                        }
                        if (player->HasAura(191797))
                            player->RemoveAurasDueToSpell(191797);
                    }
                    else if (player->HasAura(197134))
                        player->RemoveAurasDueToSpell(197134);
                });
                shelterTimer = 2 * IN_MILLISECONDS;
            }
            else
                shelterTimer -= diff;

            if (TempestTimer)
            {
                if (TempestTimer <= diff)
                {
                    TempestTimer = 2000;

                    instance->ApplyOnEveryPlayer([&](Player* player)
                    {
                        if (player->IsPointInBox(azsharaPos, azsharaLightningPos, player->GetPosition()))
                            player->CastSpell(player, 199944, true);
                        else if (player->GetDistance(azsharaPos) < 300.0f)
                            player->RemoveAurasDueToSpell(199944);
                    });
                }
                else
                    TempestTimer -= diff;
            }
        }
    };
};

void AddSC_instance_eye_of_azshara()
{
    new instance_eye_of_azshara();
}
