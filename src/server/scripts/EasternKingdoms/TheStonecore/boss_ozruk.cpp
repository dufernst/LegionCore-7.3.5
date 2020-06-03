#include "the_stonecore.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_SKILL   = 2,
    SAY_DEATH   = 3
};
enum Spells
{
    SPELL_GROUND_SLAM       = 78903,
    SPELL_SHATTER           = 78807,
    SPELL_PARALYZE          = 92426,
    SPELL_PARALYZE_DMG      = 94661,
    SPELL_ENRAGE            = 80467,
    SPELL_SHIELD            = 78835,
    SPELL_BULWARK           = 78939
};

enum Events
{
    EVENT_GROUND_SLAM   = 1,
    EVENT_SHATTER       = 2,
    EVENT_PARALYZE      = 3,
    EVENT_SHIELD        = 4,
    EVENT_BULWARK       = 5
};

struct boss_ozruk : public BossAI
{
    explicit boss_ozruk(Creature* creature) : BossAI(creature, DATA_OZRUK) {}

    bool bEnrage;

    void Reset() override
    {
        _Reset();
        bEnrage = false;
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);
        DoCast(92428);
        events.RescheduleEvent(EVENT_BULWARK, 6000);
        events.RescheduleEvent(EVENT_SHIELD, 11000);
        events.RescheduleEvent(EVENT_PARALYZE, 18000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(20) && !bEnrage)
        {
            DoCast(SPELL_ENRAGE);
            bEnrage = true;
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
            case EVENT_GROUND_SLAM:
                DoCast(SPELL_GROUND_SLAM);
                break;
            case EVENT_SHATTER:
                DoCast(SPELL_SHATTER);
                break;
            case EVENT_SHIELD:
                DoCast(SPELL_SHIELD);
                events.RescheduleEvent(EVENT_GROUND_SLAM, 2000);
                events.RescheduleEvent(EVENT_SHIELD, 23000);
                break;
            case EVENT_BULWARK:
                ZoneTalk(4);
                DoCast(SPELL_BULWARK);
                events.RescheduleEvent(EVENT_BULWARK, 15000);
                break;
            case EVENT_PARALYZE:
                if (IsHeroic())
                    DoCast(SPELL_PARALYZE);

                events.RescheduleEvent(EVENT_SHATTER, 3000);
                events.RescheduleEvent(EVENT_PARALYZE, 21000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//92426
class spell_paralyze : public AuraScript
{
    PrepareAuraScript(spell_paralyze);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        target->CastSpell(target, 94661, true);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_paralyze::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_boss_ozruk()
{
    RegisterCreatureAI(boss_ozruk);
    RegisterAuraScript(spell_paralyze);
}