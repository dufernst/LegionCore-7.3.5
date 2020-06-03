/*
    Dungeon : Skyreach 97 - 99
    Encounter: Ranjit
*/

#include "skyreach.h"

enum Says
{
    SAY_AGGRO           = 0,
    SAY_DEATH           = 1,
    SAY_KILL_PLAYER     = 2,
    SAY_SPELL_F_WINDS   = 3,
    SAY_WTF             = 4,
    SAY_WARN_LENS       = 5
};

enum Spells
{
    SPELL_SPINNING_BLADE     = 153544,
    SPELL_SPINNING_BLADE_AT  = 153588,
    SPELL_WINDWALL           = 153315,
    SPELL_WINDWALL_AT_1      = 153593,
    SPELL_WINDWALL_AT_2      = 153594,
    SPELL_FAN_OF_BLADES      = 153757,
    SPELL_PIERCING_RUSH      = 165733,
    SPELL_FOUR_WINDS         = 156793,
    SPELL_FOUR_WINDS_AT_1    = 156634,
    SPELL_FOUR_WINDS_AT_2    = 156636,
    SPELL_LENS_FLARE         = 165782
};

enum eEvents
{
    EVENT_SPINNING_BLADE    = 1,
    EVENT_WINDWALL          = 2,
    EVENT_FAN_BLADES        = 3,
    EVENT_PIERCING_RUSH     = 4,
    EVENT_FOUR_WINDS        = 5,
    EVENT_LENS_FLARE        = 6
};

struct boss_ranjit : public BossAI
{
    explicit boss_ranjit(Creature* creature) : BossAI(creature, DATA_RANJIT) {}

    void Reset() override
    {
        events.Reset();
        _Reset();

        me->SetReactState(REACT_AGGRESSIVE);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        _EnterCombat();

        events.RescheduleEvent(EVENT_SPINNING_BLADE, 4000);
        events.RescheduleEvent(EVENT_WINDWALL, 10000);
        events.RescheduleEvent(EVENT_FAN_BLADES, 10000);
        events.RescheduleEvent(EVENT_PIERCING_RUSH, 16000);
        events.RescheduleEvent(EVENT_FOUR_WINDS, 30000);
        events.RescheduleEvent(EVENT_LENS_FLARE, 50000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim->IsPlayer())
            return;

        if (urand(0, 1))
            Talk(SAY_KILL_PLAYER);
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_SPINNING_BLADE)
            DoCast(SPELL_SPINNING_BLADE_AT);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_LENS_FLARE)
            if (target->IsPlayer())
                ZoneTalk(SAY_WARN_LENS, target->GetGUID());
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (me->GetDistance(me->GetHomePosition()) > 35.0f && me->GetPositionZ() <= 184.25f)
        {
            EnterEvadeMode();
            return;
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SPINNING_BLADE:
                DoCast(SPELL_SPINNING_BLADE);
                events.RescheduleEvent(EVENT_SPINNING_BLADE, 26000);
                break;
            case EVENT_WINDWALL:
                DoCast(SPELL_WINDWALL);
                events.RescheduleEvent(EVENT_WINDWALL, 14000);
                break;
            case EVENT_FAN_BLADES:
                DoCast(SPELL_FAN_OF_BLADES);
                events.RescheduleEvent(EVENT_FAN_BLADES, 16000);
                break;
            case EVENT_PIERCING_RUSH:
                DoCast(SPELL_PIERCING_RUSH);
                events.RescheduleEvent(EVENT_PIERCING_RUSH, 60000);
                break;
            case EVENT_FOUR_WINDS:
                Talk(SAY_SPELL_F_WINDS);
                DoCast(SPELL_FOUR_WINDS);
                events.RescheduleEvent(EVENT_FOUR_WINDS, 46000);
                break;
            case EVENT_LENS_FLARE:
                DoCast(SPELL_LENS_FLARE);
                events.RescheduleEvent(EVENT_LENS_FLARE, 50000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//153315, 156793
class spell_ranjit_windwall : public SpellScript
{
    PrepareSpellScript(spell_ranjit_windwall);

    void HandleOnHit()
    {
        if (!GetCaster() || !GetHitUnit() || GetSpellInfo()->Id != SPELL_WINDWALL)
            return;

        if (urand(0, 1))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_WINDWALL_AT_1, true);
        else
            GetCaster()->CastSpell(GetHitUnit(), SPELL_WINDWALL_AT_2, true);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (!GetCaster())
            return;

        if (urand(0, 1))
            GetCaster()->CastSpell(GetCaster(), SPELL_FOUR_WINDS_AT_1, true);
        else
            GetCaster()->CastSpell(GetCaster(), SPELL_FOUR_WINDS_AT_2, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_ranjit_windwall::HandleOnHit);
        OnEffectLaunch += SpellEffectFn(spell_ranjit_windwall::HandleDummy, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
    }
};

void AddSC_boss_ranjit()
{
    RegisterCreatureAI(boss_ranjit);
    RegisterSpellScript(spell_ranjit_windwall);
}
