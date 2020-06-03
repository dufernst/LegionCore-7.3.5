/*==============
==============*/

#include "shadopan_monastery.h"

enum eSpells
{
    SPELL_CORRUPTED                 = 131530,

    SPELL_RISING_HATE               = 107356,
    SPELL_RING_OF_MALICE            = 131521,
    SPELL_SHA_BLAST                 = 114999,
    SPELL_SUMMON_GRIPPING_HATRED    = 115002,

    // Gripping Hatred
    SPELL_GRIP_OF_HATE              = 115010,
    SPELL_POOL_OF_SHADOWS           = 112929,
    
    SPELL_ACHIEV_CREDIT             = 123095
};

enum eEvents
{
    EVENT_RISING_HATE               = 1,
    EVENT_RING_OF_MALICE            = 2,
    EVENT_SHA_BLAST                 = 3,
    EVENT_SUMMON_GRIPPING_HATRED    = 4,

    EVENT_GRIP_OF_HATE              = 5
};

struct boss_taran_zhu : public BossAI
{
    explicit boss_taran_zhu(Creature* creature) : BossAI(creature, DATA_TARAN_ZHU) {}

    void Reset() override
    {
        _Reset();
        summons.DespawnAll();
        me->AddAura(SPELL_CORRUPTED, me);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (instance)
            instance->SetBossState(DATA_TARAN_ZHU, IN_PROGRESS);

        events.RescheduleEvent(EVENT_RISING_HATE, urand(25000, 35000));
        events.RescheduleEvent(EVENT_RING_OF_MALICE, urand(7500, 12500));
        events.RescheduleEvent(EVENT_SHA_BLAST, urand(2500, 5000));
        events.RescheduleEvent(EVENT_SUMMON_GRIPPING_HATRED, urand(10000, 15000));
    }

    void DamageDealt(Unit* target, uint32& damage, DamageEffectType /*damageType*/) override
    {
        if (auto player = target->ToPlayer())
        {
            uint32 newPower = player->GetPower(POWER_ALTERNATE) + std::floor(damage / 1000.0f);
            player->SetPower(POWER_ALTERNATE, newPower > 100 ? 100 : newPower);
        }
    }

    void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;

            if (instance)
                instance->SetBossState(DATA_TARAN_ZHU, DONE);

            me->setFaction(35);
            me->SetFullHealth();
            me->RemoveAurasDueToSpell(SPELL_CORRUPTED);
            DoCast(SPELL_ACHIEV_CREDIT);
            me->GetMap()->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, SPELL_ACHIEV_CREDIT, me, who);

            if (checkaura() && IsHeroic())
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 124979);
        }
    }

    bool checkaura()
    {
        auto const& players = me->GetMap()->GetPlayers();
        if (!players.isEmpty())
        {
            for (auto const& itr : players)
            {
                if (auto player = itr.getSource())
                {
                    if (!player->HasAura(107217))
                        return false;
                }
            }
        }
        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        events.Update(diff);

        switch (events.ExecuteEvent())
        {
        case EVENT_RISING_HATE:
            DoCast(SPELL_RISING_HATE);
            events.RescheduleEvent(EVENT_RISING_HATE, urand(10000, 15000));
            break;
        case EVENT_RING_OF_MALICE:
            DoCast(me, SPELL_RING_OF_MALICE, true);
            events.RescheduleEvent(EVENT_SHA_BLAST, urand(2000, 4000));
            events.RescheduleEvent(EVENT_RING_OF_MALICE, urand(27500, 32500));
            break;
        case EVENT_SHA_BLAST:
            if (!me->HasAura(SPELL_RING_OF_MALICE))
                if (auto target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                    DoCast(target, SPELL_SHA_BLAST, false);

            events.RescheduleEvent(EVENT_SHA_BLAST, urand(2500, 5000));
            break;
        case EVENT_SUMMON_GRIPPING_HATRED:
            DoCast(SPELL_SUMMON_GRIPPING_HATRED);
            events.RescheduleEvent(EVENT_SUMMON_GRIPPING_HATRED, urand(20000, 30000));
            break;
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_gripping_hatred : public ScriptedAI
{
    explicit npc_gripping_hatred(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        DoCast(me, SPELL_POOL_OF_SHADOWS, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!me->HasUnitState(UNIT_STATE_CASTING))
            DoCast(me, SPELL_GRIP_OF_HATE, false);
    }
};

class spell_taran_zhu_hate : public AuraScript
{
    PrepareAuraScript(spell_taran_zhu_hate);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (target->GetPower(POWER_ALTERNATE) >= 100)
        {
            if (!target->HasAura(SPELL_HAZE_OF_HATE))
            {
                target->CastSpell(target, SPELL_HAZE_OF_HATE, true);
                target->CastSpell(target, SPELL_HAZE_OF_HATE_VISUAL, true);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_taran_zhu_hate::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

class spell_taran_zhu_meditation : public AuraScript
{
    PrepareAuraScript(spell_taran_zhu_meditation);

    void OnRemove(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            target->SetPower(POWER_ALTERNATE, 0);
            target->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE);
            target->RemoveAurasDueToSpell(SPELL_HAZE_OF_HATE_VISUAL);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_taran_zhu_meditation::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_taran_zhu_grip_of_hate : public SpellScript
{
    PrepareSpellScript(spell_taran_zhu_grip_of_hate);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        Unit* target = GetHitUnit();
        if (!target)
            return;

        target->CastSpell(caster, GetSpellInfo()->Effects[effIndex]->BasePoints, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_taran_zhu_grip_of_hate::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_boss_taran_zhu()
{
    RegisterCreatureAI(boss_taran_zhu);
    RegisterCreatureAI(npc_gripping_hatred);
    RegisterAuraScript(spell_taran_zhu_hate);
    RegisterAuraScript(spell_taran_zhu_meditation);
    RegisterSpellScript(spell_taran_zhu_grip_of_hate);
}
