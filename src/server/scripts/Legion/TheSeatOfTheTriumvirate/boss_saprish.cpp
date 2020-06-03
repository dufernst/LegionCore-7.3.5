/*
    The Seat of the Triumvirate: Saprish [heroic & mythic]

*/

#include "the_seat_of_the_triumvirate.h"
#include "ScriptedCreature.h"
#include "AreaTriggerAI.h"

enum Says
{
    SAY_AGGRO                   = 0,
    SAY_DEATH                   = 1,
    SAY_KILL                    = 2,
    SAY_GHOSTSTRIKE             = 3,
    SAY_GHOSTSTRIKE_ANNOUNCE    = 4,

    //conversation controller
    SAY_0                       = 0,
    SAY_1                       = 1,
    SAY_2                       = 2
};

enum Spells
{
    //Saprish
    SPELL_HUNTERS_RUSH          = 247145,
    SPELL_GHOSTSTRIKE_MARKED    = 247245,
    SPELL_GHOSTSTRIKE_CHARGE    = 247246,
    SPELL_DETONATE_TRAPS        = 247206,
    SPELL_DETONATE_TRAPS_DAM    = 247207,
    SPELL_TRAP_AT               = 245903,
    SPELL_TRAP_JUST_STUN        = 245873,
    SPELL_TRAP_MISSLE_TRIGGER   = 247197,
    SPELL_TRAP_ONENTER_STUN     = 246026,
    SPELL_TRAPS_TARG_SELECTOR   = 247175,
    SPELL_CHARGE                = 85293,
    SPELL_SUM_STAFF             = 248480,
    SPELL_SUM_WALKER            = 249955,
    //darkfang
    SPELL_SHADOW_POUNCE         = 245741,
    SPELL_SHADOWSTEP_SELECT     = 245800,
    SPELL_SHADOWSTEP            = 245801,
    SPELL_SHADOWSTEP_DAMAGE     = 245802,
    SPELL_SHADOWSTEP_DAMAGE_P   = 245803,
    SPELL_SHADOWSTEP_DOT        = 245806,
    //shadewing
    SPELL_SWOOP                 = 248830,
    SPELL_SWOOP_DAMAGE          = 248829,
    SPELL_SCREAM                = 248831,

    //rift
    SPELL_RIFT_VISUAL           = 246258,
    SPELL_RIFT_VISUAL_ACTIVE    = 248144,
    SPELL_RIFT_MISSLE           = 246859,
    SPELL_RIFT_MISSLE_DAMAGE    = 246859,
    SPELL_RIFT_VOID_FRAGMENT    = 250188,
    //rift warden
    SPELL_DARKENED_REMNANT      = 248128,
    SPELL_STYGIAN_BLAST         = 248133,

    //Christmas
    SPELL_CHRISTMAS_CAP         = 254068
};

enum eEvents
{
    //saprish
    EVENT_TRAPS                  = 1,
    EVENT_DETONATE_TRAPS         = 2, 
    EVENT_GHOSTSTRIKE_PRECAST    = 3, 
    EVENT_GHOSTSTRIKE_CHARGE     = 4,
    
    //darkfang
    EVENT_POUNCE                 = 1,
    EVENT_JUMPTO                 = 2,
    EVENT_AOE                    = 3,

    //shadewing
    EVENT_SWOOP                  = 1,
    EVENT_SCREAM                 = 2,

    //portal events
    EVENT_PORT1                  = 1,
    EVENT_PORT2                  = 2,
    EVENT_PORT3                  = 3,
    EVENT_PORT4                  = 4,

    // rift
    EVENT_M                      = 1
};

enum actions
{
    START_PORTALS_EVENT     = 0,
    PORTAL_EVENT_DONE       = 1
};

Position const ccPos = { 5858.62f, 10603.29f, 39.22f, 1.27f };

Position const portalPos[4] =
{
    { 5762.47f, 10691.43f, 5.26f, 3.85f },
    { 5647.66f, 10678.40f, 5.02f, 1.69f },
    { 6023.37f, 10580.29f, 21.0f, 4.90f },
    { 5570.29f, 10590.59f, 5.79f, 0.74f }
};

Position const petPos[2] =
{
    { 5910.95f, 10697.99f, 13.66f, 4.0f }, //cat
    { 5918.35f, 10693.65f, 13.66f, 4.0f }  //myth
};

//122316
class boss_saprish : public CreatureScript
{
public:
    boss_saprish() : CreatureScript("boss_saprish") {}

    struct boss_saprishAI : public BossAI
    {
        boss_saprishAI(Creature* creature) : BossAI(creature, DATA_SAPRISH)
        {
            me->SetVisible(false);
            if (instance->GetData(SAPRISH_PORTALS) == DONE)
                me->SetVisible(true);
        }

        uint32 CheckPets = 6500;

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case START_PORTALS_EVENT:
                SummonController();
                break;
            case PORTAL_EVENT_DONE:
                PetsVisible(true);
                me->SetVisible(true);
                instance->DoCastSpellOnPlayers(246880);
                if (Creature* port = me->FindNearestCreature(NPC_SAPRISH_EVENTPORTAL, 22.0f, true))
                    port->CastSpell(port, SPELL_RIFT_VISUAL_ACTIVE, true);
                break;
            }
        }

        void JustReachedHome() override
        {
            PetsVisible(true);
        }

        void PetsVisible(bool apply)
        {
            if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                if (Creature* shadewing = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_SHADEWING)))
                    shadewing->SetVisible(apply);
            if (Creature* darkfang = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_DARKFANG)))
                darkfang->SetVisible(apply);
        }

        void PetsCombat()
        {
            if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                if (Creature * shadewing = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_SHADEWING)))
                    if (shadewing->IsAIEnabled)
                        shadewing->AI()->DoZoneInCombat();
            if (Creature * darkfang = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_DARKFANG)))
                if (darkfang->IsAIEnabled)
                    darkfang->AI()->DoZoneInCombat();
        }

        void Traps()
        {
            uint8 count = urand(3, 6);
            for (uint8 i = 1; i <= count; ++i)
            {
                me->AddDelayedEvent(10 * i, [this]() -> void
                {
                    Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 50.0f, true);
                    if (!target)
                        return;

                    Position pos(target->GetPosition());
                    me->MovePosition(pos, frand(4, 9), frand(3, 8));
                    me->SummonCreature(124427, pos);
                });
            }
        }

        void Reset() override 
        {
            _Reset();
            PetsVisible(false);
            me->RemoveAurasDueToSpell(SPELL_HUNTERS_RUSH);

            if (sGameEventMgr->IsActiveEvent(2))
                DoCast(SPELL_CHRISTMAS_CAP);
            else
            {
                if (me->HasAura(SPELL_CHRISTMAS_CAP))
                    me->RemoveAura(SPELL_CHRISTMAS_CAP);
            }
        }

        void EnterCombat(Unit* /*who*/) override 
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            PetsCombat();
            events.RescheduleEvent(EVENT_TRAPS, 8000);
            events.RescheduleEvent(EVENT_DETONATE_TRAPS, 15000);
            events.RescheduleEvent(EVENT_GHOSTSTRIKE_PRECAST, 20000);
        }

        void KilledUnit(Unit* unit)
        {
            if (unit->ToPlayer())
                Talk(SAY_KILL);
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker == me)
                return;

            if (Creature* shadewing = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_SHADEWING)))
            {
                if (damage >= me->GetHealth())
                    shadewing->Kill(shadewing, true);
                else
                    shadewing->DealDamage(shadewing, damage);
            }
            if (Creature* darkfang = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_DARKFANG)))
            {
                if (damage >= me->GetHealth())
                    darkfang->Kill(darkfang, true);
                else
                    darkfang->DealDamage(darkfang, damage);
            }
        }

        void JustDied(Unit* /*killer*/) override 
        {
            _JustDied();
            Talk(SAY_DEATH);
            me->AddDelayedEvent(2000, [this] {
                DoCast(SPELL_SUM_STAFF);
            });
            
            if (Creature* portal = me->FindNearestCreature(NPC_SAPRISH_EVENTPORTAL, 100.0f, true))
                portal->DespawnOrUnsummon(500);
        }

        void SummonController() 
        {
            me->SummonCreature(NPC_SAPRISH_CONV_CONTROL, ccPos);
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_GHOSTSTRIKE_MARKED)
                events.RescheduleEvent(EVENT_GHOSTSTRIKE_CHARGE, 6000);
        }

        void GhostStrikeAnnounce()
        {
            Map::PlayerList const& players = me->GetMap()->GetPlayers();

            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                    if (player->HasAura(SPELL_GHOSTSTRIKE_MARKED))
                        Talk(SAY_GHOSTSTRIKE_ANNOUNCE, player->GetGUID());
            }
        }

        void UpdateAI(uint32 diff) override 
        {
            if (!UpdateVictim())
                return;

            if (CheckHomeDistToEvade(diff, 70.0f))
                return;
            
            if (me->isInCombat())
            {
                if (CheckPets <= diff)
                {
                    CheckPets = 6500;
                    PetsCombat();
                }
                else CheckPets -= diff;
            }

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_TRAPS:
                    Traps();
                    events.RescheduleEvent(EVENT_TRAPS, 16000);
                    break;
                case EVENT_DETONATE_TRAPS:
                    DoCast(me, SPELL_DETONATE_TRAPS, true);
                    events.RescheduleEvent(EVENT_DETONATE_TRAPS, 20000);
                    break;
                case EVENT_GHOSTSTRIKE_PRECAST:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                    {
                        me->CastSpell(target, SPELL_GHOSTSTRIKE_MARKED);
                        GhostStrikeAnnounce();
                    }

                    events.RescheduleEvent(EVENT_GHOSTSTRIKE_PRECAST, 35000);
                    break;
                case EVENT_GHOSTSTRIKE_CHARGE:
                        me->AddDelayedEvent(500, [this] {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, SPELL_GHOSTSTRIKE_MARKED))
                            {
                                DoCast(target, SPELL_CHARGE);
                                target->RemoveAura(SPELL_GHOSTSTRIKE_MARKED);
                                Talk(SAY_GHOSTSTRIKE);
                                if (me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC)
                                    target->DealDamage(target, urand(165747, 182748));
                                else
                                    target->DealDamage(target, urand(415160, 484839));
                            }
                        });
                        me->AddDelayedEvent(1500, [this] {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, SPELL_GHOSTSTRIKE_MARKED))
                            {
                                DoCast(target, SPELL_CHARGE);
                                target->RemoveAura(SPELL_GHOSTSTRIKE_MARKED);
                                if (me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC)
                                    target->DealDamage(target, urand(165747, 182748));
                                else
                                    target->DealDamage(target, urand(415160, 484839));
                            }
                        });
                        me->AddDelayedEvent(1500, [this] {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, SPELL_GHOSTSTRIKE_MARKED))
                            {
                                DoCast(target, SPELL_CHARGE);
                                target->RemoveAura(SPELL_GHOSTSTRIKE_MARKED);
                                me->AddAura(SPELL_HUNTERS_RUSH, me);
                                if (me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC)
                                    target->DealDamage(target, urand(165747, 182748));
                                else
                                    target->DealDamage(target, urand(415160, 484839));
                            }
                        });
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_saprishAI (creature);
    }
};


//124276
class npc_saprish_conv_control : public CreatureScript
{
public:
    npc_saprish_conv_control() : CreatureScript("npc_saprish_conv_control") {}

    struct npc_saprish_conv_controlAI : public ScriptedAI
    {
        npc_saprish_conv_controlAI(Creature* creature) : ScriptedAI(creature) 
        {
            instance = me->GetInstanceScript();
            eventportalclosed = 0;
            portsummoned = 0;
            me->SetDisableGravity(true);
            eventact = false;
            if (!me->isAlive())
                if (Creature* saprish = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH)))
                    saprish->SetVisible(true);
            if (instance->GetBossState(DATA_SAPRISH) == DONE || instance->GetData(SAPRISH_PORTALS) == DONE)
                me->DespawnOrUnsummon();
        }

        EventMap events;

        InstanceScript* instance;

        uint8 eventportalclosed = 0;
        uint8 portsummoned = 0;
        bool eventact = false;

        void StartEvent()
        {
            if (!eventact)
            {
                if (instance)
                {
                    if (instance->GetData(SAPRISH_PORTALS) != DONE)
                    {
                        eventact = true;
                        instance->SetData(SAPRISH_PORTALS, IN_PROGRESS);
                        instance->DoCastSpellOnPlayers(246876);
                        events.RescheduleEvent(EVENT_PORT1, 1000);
                    }
                }
            }
        }

        void DoAction(int32 const action) override
        {
            if (action == 1)
                if (instance)
                    if (instance->GetBossState(DATA_SAPRISH) != DONE)
                        StartEvent();
        }

        void SaprishCallsConv()
        {
            if (instance)
            {
                if (portsummoned == 2)
                    instance->DoCastSpellOnPlayers(246878);
                else if (portsummoned == 3)
                    instance->DoCastSpellOnPlayers(246879);
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            switch (summon->GetEntry())
            {
            case NPC_SAPRISH_EVENTPORTAL:
                eventportalclosed++;
                events.RescheduleEvent(urand(EVENT_PORT2, EVENT_PORT4), 1000);
                break;
            }
            if (eventportalclosed == 3)
            {
                if (instance)
                {
                    instance->SetData(SAPRISH_PORTALS, DONE);
                    if (Creature* saprish = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH)))
                    {
                        saprish->GetAI()->DoAction(PORTAL_EVENT_DONE);
                        me->Kill(me);
                    }
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_PORT1:
                    if (portsummoned == 0)
                    {
                        portsummoned++;
                        me->SummonCreature(NPC_SAPRISH_EVENTPORTAL, portalPos[0]);
                        ZoneTalk(SAY_0);
                    }
                    break;
                case EVENT_PORT2:
                    if (portsummoned >= 1)
                    {
                        portsummoned++;
                        me->SummonCreature(NPC_SAPRISH_EVENTPORTAL, portalPos[1]);
                        ZoneTalk(SAY_0);
                        SaprishCallsConv();
                    }
                    break;
                case EVENT_PORT3:
                    if (portsummoned >= 1)
                    {
                        portsummoned++;
                        me->SummonCreature(NPC_SAPRISH_EVENTPORTAL, portalPos[2]);
                        ZoneTalk(SAY_2);
                        SaprishCallsConv();
                    }
                    break;
                case EVENT_PORT4:
                    if (portsummoned >= 1)
                    {
                        portsummoned++;
                        me->SummonCreature(NPC_SAPRISH_EVENTPORTAL, portalPos[3]);
                        ZoneTalk(SAY_1);
                        SaprishCallsConv();
                    }
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_conv_controlAI(creature);
    }
};

//123767
class npc_saprish_portal_event : public CreatureScript
{
public:
    npc_saprish_portal_event() : CreatureScript("npc_saprish_portal_event") {}

    struct npc_saprish_portal_eventAI : public ScriptedAI
    {
        npc_saprish_portal_eventAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);
            me->AddAura(SPELL_RIFT_MISSLE, me);
            DoCast(me, SPELL_RIFT_VISUAL, true);
            CheckTimer = 1000;
        }

        EventMap events;
        SummonList summons;
        bool saprishfound;
        uint32 CheckTimer = 1000;

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                events.Reset();
                summons.DespawnEntry(125981);
                me->CombatStop(false);
                break;
            }
        }

        void JustSummoned(Creature* summoned) override
        {
            summons.Summon(summoned);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, urand(8000, 12000));
        }

        void IsSummonedBy(Unit* summoner) override
        {
            DoCast(me, SPELL_RIFT_VISUAL_ACTIVE, true);
            float position_x = me->GetPositionX() + frand(2, 5);
            float position_y = me->GetPositionY() + frand(3, 5);

            if (Creature* ward = me->SummonCreature(NPC_SAPRISH_RIFT_WARDEN, position_x, position_y, me->GetPositionZ(), 0.0f))
                ward->SetReactState(REACT_AGGRESSIVE);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            switch (summon->GetEntry())
            {
            case NPC_SAPRISH_RIFT_WARDEN:
                me->Kill(me);
                break;
            }
        }

        void UpdateAI(uint32 diff) override 
        {
            if (!me->isInCombat())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    DoCast(me, SPELL_RIFT_VOID_FRAGMENT, true);
                    events.RescheduleEvent(EVENT_1, 12000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_portal_eventAI(creature);
    }
};

//122571
class npc_saprish_rift_warden : public CreatureScript
{
public:
    npc_saprish_rift_warden() : CreatureScript("npc_saprish_rift_warden") {}

    struct npc_saprish_rift_wardenAI : public ScriptedAI
    {
        npc_saprish_rift_wardenAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset() override
        {
            events.Reset();
            if (Creature* portal = me->FindNearestCreature(NPC_SAPRISH_EVENTPORTAL, 50.0f, true))
                portal->AI()->DoAction(1);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, urand(8000, 12000));
            events.RescheduleEvent(EVENT_2, urand(13000, 16000));
            if (Creature* portal = me->FindNearestCreature(NPC_SAPRISH_EVENTPORTAL, 15.0f, true))
                portal->SetInCombatWithZone();
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
                case EVENT_1:
                    DoCast(SPELL_DARKENED_REMNANT);
                    events.RescheduleEvent(EVENT_1, urand(25000, 30000));
                    break;
                case EVENT_2:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 30.0f, true))
                        DoCast(target, SPELL_STYGIAN_BLAST);
                    events.RescheduleEvent(EVENT_2, urand(20000, 25000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_rift_wardenAI(creature);
    }
};

//124427
class npc_saprish_void_trap : public CreatureScript
{
public:
    npc_saprish_void_trap() : CreatureScript("npc_saprish_void_trap") {}

    struct npc_saprish_void_trapAI : public ScriptedAI
    {
        npc_saprish_void_trapAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            me->AddDelayedEvent(1500, [this] {
                DoCast(me, SPELL_TRAP_AT);
            });
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_DETONATE_TRAPS)
            {
                me->RemoveAllAreaObjects();
                DoCast(SPELL_DETONATE_TRAPS_DAM);
            }
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_DETONATE_TRAPS_DAM)
                me->DespawnOrUnsummon(500);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                DoCast(me, SPELL_TRAP_ONENTER_STUN, true);
                me->DespawnOrUnsummon(500);
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_void_trapAI(creature);
    }
};

//122319
class npc_saprish_darkfang : public CreatureScript
{
public:
    npc_saprish_darkfang() : CreatureScript("npc_saprish_darkfang") {}

    struct npc_saprish_darkfangAI : public ScriptedAI
    {
        npc_saprish_darkfangAI(Creature* creature) : ScriptedAI(creature) 
        {
            instance = me->GetInstanceScript();
            me->SetVisible(false);
            if (instance->GetData(SAPRISH_PORTALS) == DONE)
                me->SetVisible(true);
        }

        EventMap events;

        InstanceScript* instance;

        void Reset() override
        {
            Position home = me->GetHomePosition();
            me->NearTeleportTo(home);
            events.Reset();
            me->SetControlled(0, UNIT_STATE_NOT_MOVE);
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_END, me);
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_END, me);
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker == me)
                return;

            if (Creature* shadewing = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_SHADEWING)))
            {
                if (damage >= me->GetHealth())
                    shadewing->Kill(shadewing, true);
                else
                    shadewing->DealDamage(shadewing, damage);
            }
            if (Creature* saprish = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH)))
            {
                if (damage >= me->GetHealth())
                    saprish->Kill(saprish, true);
                else
                    saprish->DealDamage(saprish, damage);
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_POUNCE, 1500);
            events.RescheduleEvent(EVENT_JUMPTO, 3000);
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_START, me);
                if (auto saprish = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH)))
                {
                    me->AddDelayedCombat(500, [this, saprish]() -> void
                    {
                        if (auto hp = saprish->GetMaxHealth())
                        {
                            me->SetMaxHealth(hp);
                            me->SetHealth(hp);
                        }
                    });
                }
            }
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_SHADOWSTEP_DAMAGE)
                me->SetControlled(0, UNIT_STATE_NOT_MOVE);
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_SHADOWSTEP_DAMAGE_P)
            {
                if (Creature* saprish = me->FindNearestCreature(NPC_SAPRISH, 200.0f, true))
                    me->AddAura(SPELL_HUNTERS_RUSH, saprish);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (instance->GetBossState(DATA_SAPRISH) != IN_PROGRESS)
                EnterEvadeMode();

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_POUNCE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_SHADOW_POUNCE);
                    events.RescheduleEvent(EVENT_POUNCE, 11000);
                    break;
                case EVENT_JUMPTO:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                    {
                        me->SetControlled(1, UNIT_STATE_NOT_MOVE);
                        DoCast(SPELL_SHADOWSTEP_SELECT);
                    }
                    events.RescheduleEvent(EVENT_JUMPTO, 11000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_darkfangAI(creature);
    }
};

//125340
class npc_saprish_shadewing : public CreatureScript
{
public:
    npc_saprish_shadewing() : CreatureScript("npc_saprish_shadewing") {}

    struct npc_saprish_shadewingAI : public ScriptedAI
    {
        npc_saprish_shadewingAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            me->SetVisible(false);
            if (instance->GetData(SAPRISH_PORTALS) == DONE)
                me->SetVisible(true);
        }

        EventMap events;

        InstanceScript* instance;

        void Reset() override
        {
            Position home = me->GetHomePosition();
            me->NearTeleportTo(home);
            events.Reset();
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_END, me);
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_END, me);
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_SWOOP, 3000);
            if (instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_START, me);
                if (auto saprish = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH)))
                {
                    me->AddDelayedCombat(500, [this, saprish]() -> void
                    {
                        if (auto hp = saprish->GetMaxHealth())
                        {
                            me->SetMaxHealth(hp);
                            me->SetHealth(hp);
                        }
                    });
                }
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker == me)
                return;

            if (Creature* darkfang = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_DARKFANG)))
            {
                if (damage >= me->GetHealth())
                    darkfang->Kill(darkfang, true);
                else
                    darkfang->DealDamage(darkfang, damage);
            }
            if (Creature* saprish = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH)))
            {
                if (damage >= me->GetHealth())
                    saprish->Kill(saprish, true);
                else
                    saprish->DealDamage(saprish, damage);
            }
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_SWOOP)
            {
                me->AddDelayedEvent(1500, [this] {
                    DoCast(SPELL_SWOOP_DAMAGE);
                });
            }
            if (spellInfo->Id == SPELL_SWOOP_DAMAGE)
                DoCast(SPELL_SCREAM);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (instance->GetBossState(DATA_SAPRISH) != IN_PROGRESS)
                EnterEvadeMode();

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SWOOP:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_SWOOP);

                    events.RescheduleEvent(EVENT_SWOOP, urand(13000,15000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_shadewingAI(creature);
    }
};

//125131
class npc_saprish_staff : public CreatureScript
{
public:
    npc_saprish_staff() : CreatureScript("npc_saprish_staff") {}

    struct npc_saprish_staffAI : public ScriptedAI
    {
        npc_saprish_staffAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            DoCast(SPELL_SUM_WALKER);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == 249967)
            {
                me->DespawnOrUnsummon(500);
            }
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_staffAI(creature);
    }
};

//125840
class npc_saprish_locus_walker_after_boss : public CreatureScript
{
public:
    npc_saprish_locus_walker_after_boss() : CreatureScript("npc_saprish_locus_walker_after_boss") {}

    struct npc_saprish_locus_walker_after_bossAI : public ScriptedAI
    {
        npc_saprish_locus_walker_after_bossAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;

        void IsSummonedBy(Unit* summoner) override
        {
            Talk(0);
            if (Creature* staff = me->FindNearestCreature(125131, 30.0f, true))
                me->GetMotionMaster()->MoveFollow(staff, 2.0f, 2.0f);

            me->AddDelayedEvent(2500, [this] {
                DoCast(me, 249967);
            });

            me->AddDelayedEvent(4000, [this] {
                if (instance)
                {
                    Conversation* conversation = new Conversation;
                    if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 5639, me, NULL, *me))
                        delete conversation;
                }
            });

            me->AddDelayedEvent(38000, [this] {
                me->DespawnOrUnsummon();
            });
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_saprish_locus_walker_after_bossAI(creature);
    }
};

//245801
class spell_ravaging_darkness_teleport : public SpellScript
{
    PrepareSpellScript(spell_ravaging_darkness_teleport);

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        caster->SetControlled(1, UNIT_STATE_NOT_MOVE);
        caster->CastSpell(caster, SPELL_SHADOWSTEP_DAMAGE);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_ravaging_darkness_teleport::HandleAfterCast);
    }
};

//245802
class spell_ravaging_darkness_pereodic : public SpellScriptLoader
{
public:
    spell_ravaging_darkness_pereodic() : SpellScriptLoader("spell_ravaging_darkness_pereodic") { }

    class spell_ravaging_darkness_pereodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ravaging_darkness_pereodic_AuraScript)

        void HandleTriggerSpell(AuraEffect const* /*aurEff*/)
        {
            auto caster = GetCaster();
            if (!caster)
                return;

            caster->CastSpell(caster, SPELL_SHADOWSTEP_DOT, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_ravaging_darkness_pereodic_AuraScript::HandleTriggerSpell, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_ravaging_darkness_pereodic_AuraScript();
    }
};

//10798 and 1079800 custom
struct at_saprish_void_trap : AreaTriggerAI
{
    explicit at_saprish_void_trap(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    void OnUnitEnter(Unit* unit) override
    {
        auto caster = at->GetCaster();
        if (!caster)
            return;

        if (caster->IsAIEnabled)
            caster->GetAI()->DoAction(ACTION_1);
    }
};

//1542  
class eventobject_sott_saprish_portal_event : public EventObjectScript
{
public:
    eventobject_sott_saprish_portal_event() : EventObjectScript("eventobject_sott_saprish_portal_event") {}

    bool IsTriggerMeets(Player* player, EventObject* eo) override
    {
        if (!player->isGameMaster())
        {
            if (InstanceScript* instance = eo->GetInstanceScript())
            {
                if ((instance->GetBossState(DATA_ZURAAL) == DONE))
                {
                    if (instance->GetData(SAPRISH_PORTALS) != IN_PROGRESS || instance->GetData(SAPRISH_PORTALS) != DONE)
                    {
                        if (auto cc = instance->instance->GetCreature(instance->GetGuidData(NPC_SAPRISH_CONV_CONTROL)))
                            if (cc->IsAIEnabled)
                                cc->AI()->DoAction(1);
                        return true;
                    }
                }
            }
        }

        return false;
    }
};

void AddSC_boss_saprish()
{
    new boss_saprish();
    new npc_saprish_conv_control();
    new npc_saprish_portal_event();
    new npc_saprish_rift_warden();
    new npc_saprish_void_trap();
    new npc_saprish_darkfang();
    new npc_saprish_shadewing();
    new npc_saprish_staff();
    new npc_saprish_locus_walker_after_boss();
    new spell_ravaging_darkness_pereodic();
    new eventobject_sott_saprish_portal_event();
    RegisterSpellScript(spell_ravaging_darkness_teleport);
    RegisterAreaTriggerAI(at_saprish_void_trap);
}