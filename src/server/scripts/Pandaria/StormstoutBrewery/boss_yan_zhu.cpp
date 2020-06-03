/*====================
======================*/

#include "stormstout_brewery.h"

enum eSpells
{
    SPELL_BLACKOUT      = 106851,
    SPELL_BREW_BOLT     = 114548,
    SPELL_BLOAT         = 106546,
    SPELL_CARBONATION   = 115003,
    SPELL_BUBBLE_SHIELD = 106563,
    SPELL_FORCE_VEHICLE = 46598,
    SPELL_YEASTY_BREW   = 114932,
    SPELL_SUDSY_BREW    = 114933,
    SPELL_FIZZY_BREW    = 114934,
    SPELL_BLOATING_BREW = 114929,
    SPELL_BUBBLING_BREW = 114931,
    SPELL_BLACKOUT_DRUNK = 106857,
    SPELL_GUSHING_BREW  = 106549
};

enum eEvents
{
    EVENT_BLOAT = 1,
    EVENT_BLACKOUT,
    EVENT_BUBBLE_SHIELD,
    EVENT_CARBONATION
};

enum eCreatures
{
    NPC_BUBBLE_SHIELD = 65522
};

const Position posBubble[5] =
{
    { -699.861f, 1181.42f, 169.234f, 4.74175f },
    { -718.137f, 1149.96f, 169.217f, 0.636328f },
    { -686.656f, 1167.27f, 169.212f, 3.40519f },
    { -689.035f, 1175.61f, 169.216f, 3.99264f },
    { -716.102f, 1177.32f, 169.234f, 5.6062f },
};

struct boss_yan_zhu : public BossAI
{
    explicit boss_yan_zhu(Creature* creature) : BossAI(creature, DATA_YAN_ZHU)
    {
        SetCombatMovement(false);
    }

    ObjectGuid bubbleGuid;
    uint32 AOEdmg;

    void Reset() override
    {
        _Reset();
        events.Reset();
        summons.DespawnAll();
        me->SetReactState(REACT_AGGRESSIVE);
        AOEdmg = 0;

        instance->DoRemoveAurasDueToSpellOnPlayers(106851);
        instance->DoRemoveAurasDueToSpellOnPlayers(114386);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLACKOUT_DRUNK);
        me->RemoveAllAurasExceptType(SPELL_AURA_DUMMY);
        me->AddAura(114930, me);
        me->AddAura(114929, me);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        AOEdmg = 2000;

        if (me->HasAura(SPELL_BUBBLING_BREW))
        {
            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            events.RescheduleEvent(EVENT_BUBBLE_SHIELD, 6000, 14000);
        }

        events.RescheduleEvent(EVENT_BLACKOUT, 7000);
        events.RescheduleEvent(EVENT_BLOAT, 8000);
        events.RescheduleEvent(EVENT_CARBONATION, urand(9000, 17000));
    }

    void JustSummoned(Creature* sum) override
    {
        summons.Summon(sum);

        switch (sum->GetEntry())
        {
        case 59799:
            bubbleGuid.Clear();
            bubbleGuid = sum->GetGUID();
            break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
            return;

        events.Update(diff);

        if (AOEdmg <= diff)
        {
            AOEdmg = 2000;

            if (auto victim = me->getVictim())
                if (!me->IsWithinMeleeRange(victim))
                    DoCast(me, 114548, false);
        }
        else
            AOEdmg -= diff;


        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_BLOAT:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.f, true))
                {
                    DoCast(target, SPELL_BLOAT, false);
                    events.RescheduleEvent(EVENT_BLOAT, urand(10000, 18000));
                }
                else
                    events.RescheduleEvent(EVENT_BLOAT, 1000);
                break;
            case EVENT_BLACKOUT:
                DoCast(SPELL_BLACKOUT);
                events.RescheduleEvent(EVENT_BLACKOUT, 8000);
                break;
            case EVENT_CARBONATION:
                events.RescheduleEvent(EVENT_CARBONATION, urand(35000, 48000));
                DoCast(SPELL_CARBONATION);

                for (uint8 i = 0; i < 5; i++)
                    me->SummonCreature(59799, posBubble[i], TEMPSUMMON_TIMED_DESPAWN, 20000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 114386
class spell_carbonation : public SpellScript
{
    PrepareSpellScript(spell_carbonation);

    void SelectTargets(std::list<WorldObject*>&targets)
    {
        targets.remove_if([](WorldObject* object) -> bool
        {
            Unit* unit = object->ToUnit();
            if (!unit)
                return true;

            if (unit->HasAura(114459))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_carbonation::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_carbonation::SelectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 106851
class spell_blackout_brew : public AuraScript
{
    PrepareAuraScript(spell_blackout_brew);

    void OnPeriodic(AuraEffect const* aurEff)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (target->isMoving() || target->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
        {
            if (GetStackAmount() - 1 <= 0)
                Remove(AURA_REMOVE_BY_EXPIRE);
            else
                SetStackAmount(GetStackAmount() - 1);
        }
        else
        {
            if (GetStackAmount() + 1 >= 10)
            {
                target->AddAura(SPELL_BLACKOUT_DRUNK, target);
                Remove(AURA_REMOVE_BY_EXPIRE);
            }
            else
                SetStackAmount(GetStackAmount() + 1);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_blackout_brew::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

void AddSC_boss_yan_zhu()
{
    RegisterCreatureAI(boss_yan_zhu);
    RegisterSpellScript(spell_carbonation);
    RegisterAuraScript(spell_blackout_brew);
}
