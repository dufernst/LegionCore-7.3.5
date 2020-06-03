/*==============
==============*/

#include "shadopan_monastery.h"

enum eSpells
{
    SPELL_SMOKE_BLADES          = 106826,
    SPELL_SHA_SPIKE             = 106871,
    SPELL_DISORIENTING_SMASH    = 106872,
    SPELL_PARTING_SMOKE         = 127576,
    SPELL_ENRAGE                = 130196,
    
    SPELL_ICE_TRAP              = 110610,
    SPELL_EXPLOSION             = 106966
};

enum eEvents
{
    EVENT_SMOKE_BLADES          = 1,
    EVENT_SHA_SPIKE             = 2,
    EVENT_DISORIENTING_SMASH    = 3
};

struct boss_sha_of_violence : public BossAI
{
    explicit boss_sha_of_violence(Creature* creature) : BossAI(creature, DATA_SHA_VIOLENCE) {}

    bool enrageDone;

    void Reset() override
    {
        _Reset();
        enrageDone = false;
        summons.DespawnAll();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (instance)
            instance->SetBossState(DATA_SHA_VIOLENCE, IN_PROGRESS);

        events.RescheduleEvent(EVENT_SMOKE_BLADES, urand(25000, 35000));
        events.RescheduleEvent(EVENT_SHA_SPIKE, urand(10000, 20000));
        events.RescheduleEvent(EVENT_DISORIENTING_SMASH, urand(20000, 30000));
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
        summon->CastSpell(summon, SPELL_ICE_TRAP, true);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (!enrageDone && me->HealthBelowPctDamaged(20, damage))
        {
            DoCast(me, SPELL_ENRAGE, true);
            enrageDone = true;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        summons.DespawnAll();
        _JustDied();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        switch (events.ExecuteEvent())
        {
        case EVENT_SMOKE_BLADES:
            DoCast(SPELL_SMOKE_BLADES);
            events.RescheduleEvent(EVENT_SMOKE_BLADES, urand(25000, 35000));
            break;
        case EVENT_SHA_SPIKE:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                DoCast(target, SPELL_SHA_SPIKE, false);

            events.RescheduleEvent(EVENT_SHA_SPIKE, urand(10000, 20000));
            break;
        case EVENT_DISORIENTING_SMASH:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                DoCast(target, SPELL_DISORIENTING_SMASH, false);

            events.RescheduleEvent(EVENT_DISORIENTING_SMASH, urand(20000, 30000));
            break;
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_sha_of_violence()
{
    RegisterCreatureAI(boss_sha_of_violence);
}
