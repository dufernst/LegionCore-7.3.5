#include "throne_of_the_tides.h"
#include "Group.h"
#include "LFGMgr.h"

#define GOSSIP_READY "We are ready!"

enum ScriptTexts
{
    SAY_INTRO_1     = 0,
    SAY_INTRO_2     = 1,
    SAY_INTRO_3_1   = 2,
    SAY_PHASE_2_1   = 3,
    SAY_50          = 4,
    SAY_25          = 5,
    SAY_INTRO_3_2   = 6,
    SAY_PHASE_2_2   = 7,
    SAY_PHASE_3_1   = 8,
    SAY_PHASE_3_2   = 9,
    SAY_DEATH       = 10,
    SAY_KILL        = 11
};

enum Spells
{
    SPELL_BLIGHT_OF_OZUMAT_SELF         = 83585,
    SPELL_BLIGHT_OF_OZUMAT_TRIGGER      = 83518,
    SPELL_BLIGHT_OF_OZUMAT_MISSILE      = 83506,
    SPELL_BLIGHT_OF_OZUMAT_SUMMON_1     = 83524,
    SPELL_BLIGHT_OF_OZUMAT_SUMMON_2     = 83606,
    SPELL_BLIGHT_OF_OZUMAT_DUMMY        = 83672,
    SPELL_BLIGHT_OF_OZUMAT_AURA         = 83525,
    SPELL_BLIGHT_OF_OZUMAT_DMG          = 83561,
    SPELL_BLIGHT_OF_OZUMAT_AOE          = 83607,
    SPELL_BLIGHT_OF_OZUMAT_AOE_DMG      = 83608,
    SPELL_TIDAL_SURGE                   = 76133,
    SPELL_REMOVE_TIDAL_SURGE            = 83909,
        
    // Vicious Mindslasher
    SPELL_BRAIN_SPIKE                   = 83915,
    SPELL_VEIL_OF_SHADOW                = 83926,
    SPELL_SHADOW_BOLT                   = 83914,

    // Unyielding Behemoth
    SPELL_BLIGHT_SPRAY                  = 83985,

    // Faceless Sapper
    SPELL_ENTANGLING_GRASP              = 83463,

    SPELL_ENCOUNTER_COMPLETE            = 95673
}; 

enum Events
{
    EVENT_INTRO_2               = 1,
    EVENT_INTRO_3_2             = 2,
    EVENT_SUMMON_MURLOC         = 3,
    EVENT_SUMMON_BEHEMOTH       = 4,
    EVENT_SUMMON_MINDLASHER     = 5,
    EVENT_SUMMON_SAPPER         = 6,
    EVENT_SUMMON_BEAST          = 7,
    EVENT_BLIGHT_OF_OZUMAT      = 8,
    EVENT_PLAYER_CHECK          = 9,
    EVENT_SHADOW_BOLT           = 10,
    EVENT_BRAIN_SPIKE           = 11,
    EVENT_VEIL_OF_SHADOW        = 12,
    EVENT_BLIGHT_SPRAY          = 13,
    EVENT_PHASE_2_2             = 14,
    EVENT_SUMMON_OZUMAT         = 15
};

enum Adds
{
    NPC_BOSS_OZUMAT             = 44566,
    NPC_DEEP_MURLOC_INVADER     = 44658,
    NPC_VICIOUS_MINDLASHER      = 44715,
    NPC_UNYIELDING_BEHEMOTH     = 44648,
    NPC_FACELESS_SAPPER         = 44752,
    NPC_BLIGHT_BEAST            = 44841,
    NPC_BLIGHT_OF_OZUMAT_1      = 44801,
    NPC_BLIGHT_OF_OZUMAT_2      = 44834
};

enum Actions
{
    ACTION_NEPTULON_START_EVENT = 1,
    ACTION_NEPTULON_START       = 2
};

enum Achievement
{
    SPELL_KILL_OZUMAT   = 95673
};

const Position spawnPos[5] = 
{
    {-142.48f, 950.78f, 231.05f, 1.88f},
    {-126.62f, 1015.55f, 230.37f, 4.48f},
    {-171.65f, 1006.13f, 230.67f, 5.90f},
    {-162.53f, 966.55f, 229.43f, 0.65f},
    {-110.35f, 981.47f, 229.90f, 2.83f}
};

class npc_neptulon : public CreatureScript
{
    public:
        npc_neptulon() : CreatureScript("npc_neptulon") {}

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<npc_neptulonAI>(creature);
        }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (InstanceScript* instance = creature->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_COMMANDER_ULTHOK) != DONE || instance->GetBossState(DATA_OZUMAT) == IN_PROGRESS || instance->GetBossState(DATA_OZUMAT) == DONE)
                    return false;

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_READY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
                player->SEND_GOSSIP_MENU(1, creature->GetGUID());
            }
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
        {
            if (InstanceScript* instance = creature->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_COMMANDER_ULTHOK) != DONE || instance->GetBossState(DATA_OZUMAT) == IN_PROGRESS || instance->GetBossState(DATA_OZUMAT) == DONE)
                    return false;

                player->PlayerTalkClass->ClearMenus();
                player->CLOSE_GOSSIP_MENU();
                creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                instance->SetBossState(DATA_OZUMAT, IN_PROGRESS);
                creature->AI()->DoAction(ACTION_NEPTULON_START);
            }
            return true;
        }

        struct npc_neptulonAI : public ScriptedAI
        {
            npc_neptulonAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;
            EventMap events;
            SummonList summons;
            uint32 uiMindLasherCount;
            uint32 uiSapperCount;
            bool bActive;
            bool b50;
            bool b25;

            void Reset() override
            {
                bActive = false;
                b50 = false;
                b25 = false;
                uiMindLasherCount = 0;
                uiSapperCount = 0;
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->AddUnitState(UNIT_STATE_ROOT);
                events.Reset();
                summons.DespawnAll();
                me->SetHealth(me->GetMaxHealth());
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (instance)
                    if (instance->GetBossState(DATA_OZUMAT) != DONE)
                        instance->SetBossState(DATA_OZUMAT, NOT_STARTED);
            }
            
            void EnterEvadeMode() override
            {
                me->RemoveAura(76952);
                ScriptedAI::EnterEvadeMode();
            }

            void DoAction(int32 const action) override
            {
                if (action == ACTION_NEPTULON_START_EVENT)
                {
                    me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    bActive = true;
                    Talk(SAY_INTRO_1);
                    events.RescheduleEvent(EVENT_INTRO_2, 7000);
                }
                else if (action == ACTION_NEPTULON_START)
                {
                    bActive = true;
                    Talk(SAY_INTRO_3_1);
                    events.RescheduleEvent(EVENT_INTRO_3_2, 30000);
                    events.RescheduleEvent(EVENT_SUMMON_MURLOC, urand(5000, 8000));
                    events.RescheduleEvent(EVENT_SUMMON_MINDLASHER, 10000);
                    events.RescheduleEvent(EVENT_SUMMON_BEHEMOTH, 20000);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                    DoCast(76952);
                }
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDies(Creature* creature, Unit* /*killer*/) override
            {
                summons.Despawn(creature);

                if (creature->GetEntry() == NPC_VICIOUS_MINDLASHER)
                {
                    uiMindLasherCount++;
                    if (uiMindLasherCount >= 3)
                    {
                        Talk(SAY_PHASE_2_2);
                        me->RemoveAura(76952);
                        events.CancelEvent(EVENT_SUMMON_MURLOC);
                        events.RescheduleEvent(EVENT_PHASE_2_2, 10000);
                        events.RescheduleEvent(EVENT_SUMMON_SAPPER, 8000);
                        events.RescheduleEvent(EVENT_SUMMON_BEAST, 14000);
                        events.RescheduleEvent(EVENT_BLIGHT_OF_OZUMAT, urand(9000, 11000));
                        
                        if (auto ozumat_vehicle = me->FindNearestCreature(45030, 200.0f))
                            ozumat_vehicle->AI()->DoAction(ACTION_1);
                    }
                    else
                        events.RescheduleEvent(EVENT_SUMMON_MINDLASHER, urand(10000, 15000));
                }
                else if (creature->GetEntry() == NPC_FACELESS_SAPPER)
                {
                    uiSapperCount++;

                    if (uiSapperCount >= 3)
                    {
                        Talk(SAY_PHASE_3_2);
                        events.RescheduleEvent(EVENT_SUMMON_OZUMAT, 10000);
                    }
                }
                if (creature->GetEntry() == NPC_BOSS_OZUMAT)
                    CompleteEncounter();
            }

            void CompleteEncounter()
            {
                if (instance)
                {
                    // Achievement
                    instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 26063);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                    instance->instance->ApplyOnEveryPlayer([&](Player* player)
                    {
                        if (Group* group = player->GetGroup())
                        {
                            if (sLFGMgr->GetQueueId(1146))
                                sLFGMgr->FinishDungeon(player->GetGroup()->GetGUID(), 1146);

                            if (player->GetGuildId() && group->IsGuildGroup(player->GetGuildGUID(), true, true))
                                group->UpdateGuildAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_KILL_OZUMAT, 0, 0, NULL, me);
                        }
                    });

                    me->GetMap()->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, SPELL_ENCOUNTER_COMPLETE, me, me); 
                    instance->SetBossState(DATA_OZUMAT, DONE);
                    events.CancelEvent(EVENT_16);
                }

                DoCast(SPELL_REMOVE_TIDAL_SURGE);
                EnterEvadeMode();
                me->DespawnOrUnsummon(3000);
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType /*dmgType*/) override
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    Talk(SAY_DEATH);
                    EnterEvadeMode();
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!bActive)
                    return;

                if (me->HealthBelowPct(50) && !b50)
                {
                    b50 = true;
                    Talk(SAY_50);
                }

                if (me->HealthBelowPct(25) && !b25)
                {
                    b25 = true;
                    Talk(SAY_25);
                }

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_INTRO_2:
                        Talk(SAY_INTRO_2);
                        bActive = false;
                        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case EVENT_INTRO_3_2:
                        Talk(SAY_INTRO_3_2);
                        break;
                    case EVENT_PHASE_2_2:
                        Talk(SAY_PHASE_2_1);
                        break;
                    case EVENT_SUMMON_MINDLASHER:
                        if (auto mindlasher = me->SummonCreature(NPC_VICIOUS_MINDLASHER, spawnPos[urand(0, 1)]))
                        {
                            mindlasher->AddThreat(me, 1.0f);
                            mindlasher->AI()->AttackStart(me);
                        }
                        break;
                    case EVENT_SUMMON_BEHEMOTH:
                        if (auto behemoth = me->SummonCreature(NPC_UNYIELDING_BEHEMOTH, spawnPos[urand(0, 1)]))
                        {
                            behemoth->AddThreat(me, 1.0f);
                            behemoth->AI()->AttackStart(me);
                        }
                        break;
                    case EVENT_SUMMON_SAPPER:
                        for (uint8 i = 2; i < 5; i++)
                            if (auto sapper = me->SummonCreature(NPC_FACELESS_SAPPER, spawnPos[i]))
                                sapper->CastSpell(me, SPELL_ENTANGLING_GRASP, false);
                        break;
                    case EVENT_SUMMON_BEAST:
                        if (auto beast = me->SummonCreature(NPC_BLIGHT_BEAST, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
                            if (auto target = GetRandomPlayer())
                                beast->AI()->AttackStart(target);

                        events.RescheduleEvent(EVENT_SUMMON_BEAST, urand(15000, 24000));
                        break;
                    case EVENT_SUMMON_MURLOC:
                        for (uint8 i = 0; i < 5; i++)
                        {
                            if (auto murloc = me->SummonCreature(NPC_DEEP_MURLOC_INVADER, spawnPos[urand(0, 1)]))
                            {
                                murloc->AddThreat(me, 1.0f);
                                murloc->AI()->AttackStart(me);
                            }
                        }
                        events.RescheduleEvent(EVENT_SUMMON_MURLOC, urand(10000, 17000));
                        break;
                    case EVENT_BLIGHT_OF_OZUMAT:
                        if (auto target = GetRandomPlayer())
                            DoCast(target, SPELL_BLIGHT_OF_OZUMAT_MISSILE, false);

                        events.RescheduleEvent(EVENT_BLIGHT_OF_OZUMAT, urand(10000, 18000));
                        break;
                    case EVENT_SUMMON_OZUMAT:
                    {
                        std::list<Creature*> list;
                        list.clear();
                        me->GetCreatureListWithEntryInGrid(list, 45030, 200.0f);
                        me->GetCreatureListWithEntryInGrid(list, 40655, 200.0f);
                        if (!list.empty())
                            for (auto& cre : list)
                                cre->DespawnOrUnsummon();

                        Talk(SAY_PHASE_3_1);
                        ZoneTalk(12);
                        DoCast(SPELL_BLIGHT_OF_OZUMAT_SUMMON_2);
                        DoCast(SPELL_TIDAL_SURGE);
                        me->SummonCreature(NPC_BOSS_OZUMAT, -92.70f, 916.18f, 273.14f, 2.23f);
                        me->SetFacingTo(5.320109f);
                        events.RescheduleEvent(EVENT_16, 1000);
                        break;
                    }
                    case EVENT_16:
                        DoCast(84037);
                        events.RescheduleEvent(EVENT_16, 1000);
                        break;
                    }
                }
            }         

            Player* GetRandomPlayer()
            {
                std::list<Player*> AliveList;
                Map::PlayerList const &pPlayerList = me->GetMap()->GetPlayers();
                if (!pPlayerList.isEmpty())
                    for (Map::PlayerList::const_iterator itr = pPlayerList.begin(); itr != pPlayerList.end(); ++itr)
                        if (itr->getSource()->isAlive())
                            AliveList.push_back(itr->getSource());

                if (!AliveList.empty())
                {
                    std::list<Player*>::const_iterator itr = AliveList.begin();
                    std::advance(itr, rand() % AliveList.size());
                    return (*itr);
                }
                return NULL;
            }

            bool isPlayerAlive()
            {
                Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();

                if (!PlayerList.isEmpty())
                    for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                        if (itr->getSource()->isAlive())
                            return true;

                return false;
            };
        };        
};

struct npc_vicious_mindslasher : public ScriptedAI
{
    explicit npc_vicious_mindslasher(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (IsHeroic())
            events.RescheduleEvent(EVENT_VEIL_OF_SHADOW, urand(10000, 15000));

        events.RescheduleEvent(EVENT_BRAIN_SPIKE, urand(6000, 10000));
        events.RescheduleEvent(EVENT_SHADOW_BOLT, 2000);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        if (instance)
            if (auto neptulon = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_NEPTULON)))
                neptulon->AI()->Talk(SAY_KILL);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_BRAIN_SPIKE:
                DoCast(SPELL_BRAIN_SPIKE);
                events.RescheduleEvent(EVENT_BRAIN_SPIKE, urand(15000, 20000));
                break;
            case EVENT_VEIL_OF_SHADOW:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_VEIL_OF_SHADOW, false);

                events.RescheduleEvent(EVENT_VEIL_OF_SHADOW, urand(13000, 20000));
                break;
            case EVENT_SHADOW_BOLT:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_SHADOW_BOLT, false);

                events.RescheduleEvent(EVENT_SHADOW_BOLT, 2000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_unyielding_behemoth : public ScriptedAI
{
    explicit npc_unyielding_behemoth(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        if (instance)
            if (auto neptulon = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_NEPTULON)))
                neptulon->AI()->Talk(SAY_KILL);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_BLIGHT_SPRAY, urand(8000, 12000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_BLIGHT_SPRAY:
                DoCast(SPELL_BLIGHT_SPRAY);
                events.RescheduleEvent(EVENT_BLIGHT_SPRAY, urand(15000, 23000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_faceless_sapper : public ScriptedAI
{
    explicit npc_faceless_sapper(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }
};

struct npc_blight_of_ozumat : public ScriptedAI
{
    explicit npc_blight_of_ozumat(Creature* creature) : ScriptedAI(creature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        DoCast(SPELL_BLIGHT_OF_OZUMAT_AURA);
    }
};

struct npc_ozumat_vehicle : public ScriptedAI
{
    explicit npc_ozumat_vehicle(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
        {
            me->SummonCreature(40655, -74.125f, 852.818f, 340.5083f, 2.073103f, TEMPSUMMON_MANUAL_DESPAWN, 0);
        }
    }
};

struct npc_ozumat_passanger : public ScriptedAI
{
    explicit npc_ozumat_passanger(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!me->HasAura(46598))
            if (auto owner = me->GetAnyOwner())
                DoCast(owner, 46598, true);
    }
};

struct npc_ozumat : public ScriptedAI
{
    explicit npc_ozumat(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        DoCast(SPELL_BLIGHT_OF_OZUMAT_DUMMY);

        if (auto vehicle = me->FindNearestCreature(44581, 100.0f, true))
            DoCast(vehicle, 83119, true);
    }
};

struct npc_blight_of_ozumat_final : public ScriptedAI
{
    explicit npc_blight_of_ozumat_final(Creature* creature) : ScriptedAI(creature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        DoCast(SPELL_BLIGHT_OF_OZUMAT_AOE);
    }
};

class at_tott_ozumat : public AreaTriggerScript
{
    public:
        at_tott_ozumat() : AreaTriggerScript("at_tott_ozumat") {}

        bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/, bool /*enter*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
            {
                if (instance->GetData(DATA_NEPTULON_EVENT) != DONE && instance->GetBossState(DATA_OZUMAT) != IN_PROGRESS && instance->GetBossState(DATA_OZUMAT) != DONE)
                {
                    instance->SetData(DATA_NEPTULON_EVENT, DONE);

                    if (auto neptulon = ObjectAccessor::GetCreature(*player, instance->GetGuidData(DATA_NEPTULON)))
                        neptulon->AI()->DoAction(ACTION_NEPTULON_START_EVENT);
                }
            }
            return true;
        }
};

void AddSC_boss_ozumat()
{
    new npc_neptulon();
    new at_tott_ozumat();
    RegisterCreatureAI(npc_vicious_mindslasher);
    RegisterCreatureAI(npc_unyielding_behemoth);
    RegisterCreatureAI(npc_faceless_sapper);
    RegisterCreatureAI(npc_blight_of_ozumat);
    RegisterCreatureAI(npc_ozumat);
    RegisterCreatureAI(npc_blight_of_ozumat_final);
    RegisterCreatureAI(npc_ozumat_vehicle);
    RegisterCreatureAI(npc_ozumat_passanger);
}