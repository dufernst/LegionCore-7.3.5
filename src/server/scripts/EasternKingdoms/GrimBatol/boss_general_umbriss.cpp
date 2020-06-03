#include "grim_batol.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_ADDS    = 4
};

enum Spells
{
    SPELL_BLEEDING_WOUND        = 74846,
    SPELL_GROUND_SIEGE_DUMMY    = 74636,
    SPELL_GROUND_SIEGE          = 74634,
    SPELL_BLITZ                 = 74670,
    SPELL_BLITZ_DMG             = 74675,
    SPELL_FRENZY                = 74853,
    SPELL_CLAW_PUNCTURE         = 76507,
    SPELL_MODGUD_MALICE         = 74699,
    SPELL_MODGUD_MALICE_AURA    = 90170,
    SPELL_MODGUD_MALADY         = 74837
};

enum Events
{
    EVENT_BLEEDING_WOUND    = 1,
    EVENT_GROUND_SIEGE      = 2,
    EVENT_BLITZ             = 3,
    EVENT_CLAW_PUNCTURE     = 4,
    EVENT_ADDS              = 5
};

enum Adds
{
    NPC_GROUND_SIEGE_STALKER    = 40030,
    NPC_BLITZ_STALKER           = 40040,
    NPC_MALIGNANT_TROGG         = 39984,
    NPC_TROGG_DWELLER           = 45467
};

const Position troggPos[2]=
{
    { -722.15f, -442.53f, 268.77f, 0.54f },
    { -702.22f, -450.9f, 268.77f, 1.34f }
};

struct boss_general_umbriss : public BossAI
{
    explicit boss_general_umbriss(Creature* creature) : BossAI(creature, DATA_GENERAL_UMBRISS) {}

    ObjectGuid tempGUID;
    bool bEnrage;

    void Reset() override
    {
        _Reset();
        bEnrage = false;
        summons.DespawnAll();
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        events.RescheduleEvent(EVENT_ADDS, 30000);
        events.RescheduleEvent(EVENT_GROUND_SIEGE, 10000);
        events.RescheduleEvent(EVENT_BLEEDING_WOUND, 5000);
        events.RescheduleEvent(EVENT_BLITZ, 23000);
        Talk(SAY_AGGRO);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        summons.DespawnAll();
    }

    void KilledUnit(Unit* victim)
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        switch (summon->GetEntry())
        {
        case NPC_MALIGNANT_TROGG:
        case NPC_TROGG_DWELLER:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                summon->AI()->AttackStart(target);
            break;
        }
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        summons.Despawn(summon);
    }

    void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType dmgType) override
    {
        if (HealthBelowPct(30) && !bEnrage)
        {
            bEnrage = true;
            DoCast(SPELL_FRENZY);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (CheckHomeDistToEvade(diff, 60.0f))
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_BLEEDING_WOUND:
                if (auto victim = me->getVictim())
                    if (victim->HealthBelowPct(91))
                        DoCast(victim, SPELL_BLEEDING_WOUND, false);

                events.RescheduleEvent(EVENT_BLEEDING_WOUND, 25000);
                break;
            case EVENT_GROUND_SIEGE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    if (auto stalker = me->SummonCreature(NPC_GROUND_SIEGE_STALKER, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 6000))
                        DoCast(stalker, SPELL_GROUND_SIEGE);

                events.RescheduleEvent(EVENT_GROUND_SIEGE, 18000);
                break;
            case EVENT_BLITZ:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    if (auto blitz = me->SummonCreature(NPC_BLITZ_STALKER, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 6000))
                        DoCast(blitz, SPELL_BLITZ, false);

                events.RescheduleEvent(EVENT_BLITZ, 23000);
                break;
            case EVENT_ADDS:
                if (!bEnrage)
                {
                    for (uint8 i = 0; i < 3; i++)
                        me->SummonCreature(NPC_TROGG_DWELLER, troggPos[0]);

                    me->SummonCreature(NPC_MALIGNANT_TROGG, troggPos[1]);
                    Talk(SAY_ADDS);
                }

                events.RescheduleEvent(EVENT_ADDS, 45000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_malignant_trogg : public ScriptedAI
{
    explicit npc_malignant_trogg(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        DoCast(SPELL_MODGUD_MALICE);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        events.RescheduleEvent(EVENT_CLAW_PUNCTURE, 5000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoCast(me, SPELL_MODGUD_MALADY, true);

        if (IsHeroic())
            DoCast(me, SPELL_MODGUD_MALICE_AURA, true);
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
            case EVENT_CLAW_PUNCTURE:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_CLAW_PUNCTURE, false);

                events.RescheduleEvent(EVENT_CLAW_PUNCTURE, urand(5000, 10000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_umbriss_trogg_dweller : public ScriptedAI
{
    explicit npc_umbriss_trogg_dweller(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void EnterCombat(Unit* /*attacker*/) override
    {
        events.RescheduleEvent(EVENT_CLAW_PUNCTURE, 5000);
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
            case EVENT_CLAW_PUNCTURE:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_CLAW_PUNCTURE, false);

                events.RescheduleEvent(EVENT_CLAW_PUNCTURE, urand(5000, 10000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//74846
class spell_bleeding_wound : public AuraScript
{
    PrepareAuraScript(spell_bleeding_wound);

    void HandleTick(AuraEffect const* aurEff)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (target->HealthAbovePct(89))
            target->RemoveAura(SPELL_BLEEDING_WOUND);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_bleeding_wound::HandleTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

void AddSC_boss_general_umbriss()
{
    RegisterCreatureAI(boss_general_umbriss);
    RegisterCreatureAI(npc_malignant_trogg);
    RegisterCreatureAI(npc_umbriss_trogg_dweller);
    RegisterAuraScript(spell_bleeding_wound);
}
