#include "grim_batol.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_DEATH   = 2,
    SAY_GALE    = 3,
    SAY_ADDS    = 4
};

enum Spells
{
    SPELL_ENFEEBLING_BLOW           = 75789,
    SPELL_BINDING_SHADOWS           = 79466,
    SPELL_BINDING_SHADOWS_AURA      = 75861,
    SPELL_SIPHON_ESSENSE            = 75755,
    SPELL_UMBRAL_MENDING            = 75763,
    SPELL_SHADOW_GALE_SPEED_TRIGGER = 75675,
    SPELL_SHADOW_GALE_SPEED         = 75694,
    SPELL_SHADOW_GALE               = 75664,
    SPELL_SHADOW_GALE_DMG           = 75692,
    SPELL_TWILIGHT_CORRUPTION       = 75520,
    SPELL_TWILIGHT_CORRUPTION_DMG   = 75566,
    SPELL_SUMMON_TWILIGHT_HATCHLING = 91058,
    SPELL_SPAWN_FACELESS            = 75704,
    SPELL_TWILIGHT_PORTAL_VISUAL    = 95716,
    SPELL_TWILIGHT_BLAST_TRIGGER    = 76192,
    SPELL_TWILIGHT_BLAST_DMG        = 76194,
    SPELL_SHIELD_OF_NIGHTMARES      = 75809
};

enum Adds
{
    NPC_FACELESS_CORRUPTOR          = 40600,
    NPC_FACELESS_PORTAL_STALKER     = 44314,
    NPC_ALEXSTRASZA_EGG             = 40486,
    NPC_TWILIGHT_HATCHLING          = 39388,
    NPC_SHADOW_GALE_STALKER         = 40567
};

enum Events
{
    EVENT_ENFEEBLING_BLOW           = 1,
    EVENT_BINDING_SHADOWS           = 2,
    EVENT_ADDS                      = 3,
    EVENT_SIPHON_ESSENSE            = 4,
    EVENT_UMBRAL_MENDING            = 5,
    EVENT_REMOVE_TWILIGHT_PORTAL    = 6,
    EVENT_SHADOW_GALE               = 7,
    EVENT_TWILIGHT_CORRUPTION       = 8,
    EVENT_CORRUPTOR_MOVE            = 9,
    EVENT_SHIELD_OF_NIGHTMARES      = 10
};

enum Points
{
    POINT_EGG   = 1
};

const Position erudaxportalPos = { -641.515f, -827.8f, 235.5f, 3.069f };

const Position neareggPos[2] = 
{
    { -719.67f, -839.64f, 232.43f, 0.0f },
    { -730.63f, -864.46f, 232.44f, 4.71f }
};

const Position shadowgalePos[3] = 
{
    { -745.07f, -845.16f, 232.41f, 0.0f },
    { -724.05f, -823.47f, 232.41f, 0.0f },
    { -741.81f, -819.44f, 232.41f, 0.0f }
};

const Position eggPos[23] =
{
    {-720.21f, -875.28f, 232.56f, 4.59f},
    {-728.52f, -875.78f, 232.55f, 3.33f},
    {-736.79f, -877.43f, 232.58f, 3.33f},
    {-748.38f, -876.34f, 233.23f, 2.87f},
    {-757.47f, -873.80f, 233.23f, 2.87f},
    {-749.23f, -871.13f, 233.23f, 0.30f},
    {-740.28f, -870.06f, 232.58f, 6.17f},
    {-731.70f, -868.19f, 232.47f, 0.00f},
    {-752.09f, -860.58f, 232.54f, 3.13f},
    {-747.30f, -779.60f, 233.23f, 1.92f},
    {-740.44f, -778.07f, 233.23f, 0.22f},
    {-729.57f, -778.52f, 232.57f, 0.11f},
    {-719.22f, -770.13f, 233.44f, 0.72f},
    {-722.09f, -780.99f, 232.55f, 4.45f},
    {-735.99f, -786.08f, 232.57f, 3.05f},
    {-751.92f, -788.38f, 233.23f, 3.33f},
    {-745.56f, -785.41f, 233.23f, 0.43f},
    {-757.08f, -780.37f, 233.23f, 2.68f},
    {-746.64f, -770.96f, 233.23f, 0.95f},
    {-731.53f, -770.02f, 236.14f, 0.10f},
    {-730.15f, -885.09f, 235.96f, 5.93f},
    {-717.82f, -887.81f, 233.95f, 6.05f},
    {-751.01f, -886.38f, 234.87f, 3.27f}
};

struct boss_erudax : public BossAI
{
    explicit boss_erudax(Creature* creature) : BossAI(creature, DATA_ERUDAX) {}

    bool isCorruption;

    void Reset() override
    {
        _Reset();
        summons.DespawnAll();
        DespawnAllSummons();
        events.Reset();

        isCorruption = false;

        for (uint8 i = 0; i < 23; ++i)
            me->SummonCreature(NPC_ALEXSTRASZA_EGG, eggPos[i]);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        if (summon->GetEntry() == NPC_FACELESS_CORRUPTOR)
            summon->AI()->DoZoneInCombat();
    }

    void DespawnAllSummons()
    {
        std::list<Creature*> list;
        list.clear();
        me->GetCreatureListWithEntryInGrid(list, 39388, 200.0f);
        if (!list.empty())
            for (auto& cre : list)
                cre->DespawnOrUnsummon();
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        summons.Despawn(summon);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);
        me->SummonCreature(NPC_FACELESS_PORTAL_STALKER, erudaxportalPos, TEMPSUMMON_MANUAL_DESPAWN);
        events.RescheduleEvent(EVENT_SHADOW_GALE, urand(25000, 26000));
        events.RescheduleEvent(EVENT_ENFEEBLING_BLOW, urand(4000, 6000));
        events.RescheduleEvent(EVENT_BINDING_SHADOWS, urand(9000, 11000));
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        summons.DespawnAll();
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (CheckHomeDistToEvade(diff, 100.0f))
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SHADOW_GALE:
                Talk(SAY_GALE);
                DoCast(SPELL_SHADOW_GALE);
                events.RescheduleEvent(EVENT_ADDS, 16500);

                if (auto ShadowGaleTrigger = me->SummonCreature(NPC_SHADOW_GALE_STALKER, shadowgalePos[urand(0, 2)]))
                    ShadowGaleTrigger->CastSpell(ShadowGaleTrigger, SPELL_SHADOW_GALE_SPEED_TRIGGER, false);
                break;
            case EVENT_REMOVE_TWILIGHT_PORTAL:
                if (auto FacelessPortalStalker = me->FindNearestCreature(NPC_FACELESS_PORTAL_STALKER, 100.0f))
                    FacelessPortalStalker->RemoveAurasDueToSpell(SPELL_TWILIGHT_PORTAL_VISUAL);
                break;
            case EVENT_ADDS:
                if (auto ShadowGaleTrigger = me->FindNearestCreature(NPC_SHADOW_GALE_STALKER, 100.0f))
                    ShadowGaleTrigger->DespawnOrUnsummon();

                if (auto FacelessPortalStalker = me->FindNearestCreature(NPC_FACELESS_PORTAL_STALKER, 100.0f))
                {
                    FacelessPortalStalker->CastSpell(FacelessPortalStalker, SPELL_TWILIGHT_PORTAL_VISUAL, true);

                    if (me->GetMap()->GetDifficultyID() != DIFFICULTY_NORMAL)
                    {
                        ZoneTalk(5);
                        me->SummonCreature(NPC_FACELESS_CORRUPTOR, -632.56f, -832.335f, 236.22f, 3.21002f, TEMPSUMMON_MANUAL_DESPAWN, 0);
                        me->SummonCreature(NPC_FACELESS_CORRUPTOR, -632.7311f, -824.0012f, 236.24501f, 3.17075f, TEMPSUMMON_MANUAL_DESPAWN, 0);
                    }
                    else
                        me->CastSpell(FacelessPortalStalker, SPELL_SPAWN_FACELESS, true);
                }

                Talk(SAY_ADDS);
                events.RescheduleEvent(EVENT_SHIELD_OF_NIGHTMARES, urand(4000, 7000));
                events.RescheduleEvent(EVENT_REMOVE_TWILIGHT_PORTAL, 7000);
                events.RescheduleEvent(EVENT_SHADOW_GALE, urand(40000, 44000));
                break;
            case EVENT_ENFEEBLING_BLOW:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_ENFEEBLING_BLOW, false);

                events.RescheduleEvent(EVENT_ENFEEBLING_BLOW, urand(20000, 30000));
                break;
            case EVENT_BINDING_SHADOWS:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_BINDING_SHADOWS, false);

                events.RescheduleEvent(EVENT_BINDING_SHADOWS, urand(20000, 30000));
                break;
            case EVENT_SHIELD_OF_NIGHTMARES:
                DoCast(SPELL_SHIELD_OF_NIGHTMARES);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_erudax_faceless_corruptor : public ScriptedAI
{
    explicit npc_erudax_faceless_corruptor(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        std::list<Creature*> eggs;
        me->GetCreatureListWithEntryInGrid(eggs, NPC_ALEXSTRASZA_EGG, 200.0f);
        Trinity::Containers::RandomResizeList(eggs, 1);
        if (!eggs.empty())
        {
            for (auto& egg : eggs)
            {
                me->StopAttack(true);
                me->GetMotionMaster()->MovePoint(POINT_EGG, egg->GetPositionX() - 4.0f, egg->GetPositionY() - 4.0f, egg->GetPositionZ());
                egg->AI()->DoZoneInCombat();
            }
        }
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        events.RescheduleEvent(EVENT_UMBRAL_MENDING, urand(15000, 20000));
        events.RescheduleEvent(EVENT_SIPHON_ESSENSE, urand(5000, 7000));
    }

    void JustDied(Unit* /*killer*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        me->DespawnOrUnsummon();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (id)
            {
            case POINT_EGG:
                DoCast(SPELL_TWILIGHT_CORRUPTION);
                me->SetReactState(REACT_AGGRESSIVE, 10000);
                break;
            }
        }
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
            case EVENT_UMBRAL_MENDING:
                if (auto erudax = Unit::GetCreature(*me, instance->GetGuidData(DATA_ERUDAX)))
                    DoCast(erudax, SPELL_UMBRAL_MENDING, false);

                events.RescheduleEvent(EVENT_UMBRAL_MENDING, urand(15000, 20000));
                break;
            case EVENT_SIPHON_ESSENSE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_SIPHON_ESSENSE, false);

                events.RescheduleEvent(EVENT_SIPHON_ESSENSE, urand(7000, 10000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_alexstrasza_egg : public ScriptedAI
{
    explicit npc_alexstrasza_egg(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->setRegeneratingHealth(false);
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void JustDied(Unit* /*killer*/) override
    {
        DoCast(SPELL_SUMMON_TWILIGHT_HATCHLING);
        me->DespawnOrUnsummon(1000);
    }

    void JustSummoned(Creature* summon) override
    {
        if (!instance)
            return;

        if (summon->GetEntry() == NPC_TWILIGHT_HATCHLING)
            if (auto _erudax = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ERUDAX)))
                if (auto target = _erudax->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    summon->AI()->AttackStart(target);
    }
};

struct npc_shadow_gale_stalker : public ScriptedAI
{
    explicit npc_shadow_gale_stalker(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }
};

struct npc_erudax_twilight_hatchling : public ScriptedAI
{
    explicit npc_erudax_twilight_hatchling(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void Reset() override
    {
        me->SetCanFly(true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon();
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        DoCast(SPELL_TWILIGHT_BLAST_TRIGGER);
    }
};

class spell_erudax_shadow_gale : public SpellScript
{
    PrepareSpellScript(spell_erudax_shadow_gale);

    void HandleScript()
    {
        if (!GetCaster() || !GetHitUnit())
            return;

        if (GetHitUnit()->FindNearestCreature(NPC_SHADOW_GALE_STALKER, 5.0f))
            SetHitDamage(0);
    }

    void Register() override
    {
        BeforeHit += SpellHitFn(spell_erudax_shadow_gale::HandleScript);
    }
};

void AddSC_boss_erudax()
{
    RegisterCreatureAI(boss_erudax);
    RegisterCreatureAI(npc_erudax_faceless_corruptor);
    RegisterCreatureAI(npc_alexstrasza_egg);
    RegisterCreatureAI(npc_shadow_gale_stalker);
    RegisterCreatureAI(npc_erudax_twilight_hatchling);
    RegisterSpellScript(spell_erudax_shadow_gale);
}