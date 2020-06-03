/*==============
==============*/

#include "shadopan_monastery.h"

enum eSpells
{
    SPELL_KILL_GUARDIANS            = 114927,

    SPELL_INVOKE_LIGHTNING          = 106984,
    SPELL_CHARGING_SOUL             = 110945,
    
    SPELL_OVERCHARGED_SOUL          = 110852,
    SPELL_OVERCHARGED_SOUL_DAMAGE   = 111129,

    SPELL_LIGHTNING_SHIELD          = 123496,
    SPELL_STATIC_FIELD              = 106923,

    SPELL_LIGHTNING_BREATH          = 102573,
    SPELL_MAGNETIC_SHROUD           = 107140
};

enum eEvents
{
    EVENT_INVOKE_LIGHTNING  = 1,
    EVENT_OVERCHARGED_SOUL  = 2,

    EVENT_STATIC_FIELD      = 3,
    EVENT_LIGHTNING_BREATH  = 4,
    EVENT_MAGNETIC_SHROUD   = 5
};

enum eActions
{
    ACTION_INTRO                = 1,
    ACTION_GU_P_3               = 2,
    
    ACTION_AZURE_SERPENT_P_1    = 3,
    ACTION_AZURE_SERPENT_P_2    = 4,
    ACTION_AZURE_SERPENT_RESET  = 5
};

enum ePhases
{
    PHASE_ONE   = 1,
    PHASE_TWO   = 2,
    PHASE_THREE = 3
};

struct boss_gu_cloudstrike : public BossAI
{
    boss_gu_cloudstrike(Creature* creature) : BossAI(creature, DATA_GU_CLOUDSTRIKE) {}

    uint8 phase;

    void Reset() override
    {
        _Reset();
        phase = 1;
        me->SetReactState(REACT_AGGRESSIVE);
        me->RemoveAurasDueToSpell(SPELL_CHARGING_SOUL);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (phase == 1 && me->HealthBelowPctDamaged(50, damage))
        {
            phase = 2;
            events.CancelEventGroup(PHASE_ONE);
            events.RescheduleEvent(EVENT_OVERCHARGED_SOUL, 2500, PHASE_TWO);
            DoCast(SPELL_CHARGING_SOUL);
            if (auto azureSerpent = instance->instance->GetCreature(instance->GetGuidData(NPC_AZURE_SERPENT)))
                if (azureSerpent->isAlive())
                    azureSerpent->AI()->DoAction(ACTION_AZURE_SERPENT_P_2);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGNETIC_SHROUD);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_GU_P_3)
        {
            DoCast(me, SPELL_OVERCHARGED_SOUL, true);
            me->RemoveAurasDueToSpell(SPELL_CHARGING_SOUL);
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_INVOKE_LIGHTNING, urand(5000, 10000));

        if (auto azureSerpent = me->SummonCreature(56754, 3726.13f, 2677.06f, 773.01f, 0, TEMPSUMMON_CORPSE_DESPAWN))
        {
            azureSerpent->SetInCombatWithZone();
            azureSerpent->SetFacingTo(3.625f);
        }
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
        case EVENT_INVOKE_LIGHTNING:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                DoCast(target, SPELL_INVOKE_LIGHTNING, false);

            events.RescheduleEvent(EVENT_INVOKE_LIGHTNING, urand(5000, 10000));
            break;
        case EVENT_OVERCHARGED_SOUL:
            DoCast(SPELL_OVERCHARGED_SOUL_DAMAGE);
            events.RescheduleEvent(EVENT_OVERCHARGED_SOUL, 2500);
            break;
        default:
            break;
        }
        DoMeleeAttackIfReady();
    }
};

Position azureSerpentPositions[4] =
{
    {3835.01f, 2906.63f, 753.33f},
    {3850.37f, 2738.14f, 814.84f},
    {3758.79f, 2692.08f, 778.60f},
    {3736.37f, 2680.89f, 778.60f}
};

struct npc_azure_serpent : public ScriptedAI
{
    explicit npc_azure_serpent(Creature* creature) : ScriptedAI(creature)
    {
        me->SetCanFly(true);
        me->SetDisableGravity(true);
    }

    EventMap events;
    uint8 phase;

    void Reset() override
    {
        phase = 1;
        me->setActive(true);
        me->SetReactState(REACT_PASSIVE);
        events.Reset();
        me->AddAura(SPELL_LIGHTNING_SHIELD, me);
        me->SetSpeed(MOVE_FLIGHT, 5.0f);
        DoZoneInCombat();
        events.RescheduleEvent(EVENT_STATIC_FIELD, 5000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (auto gu = me->GetAnyOwner())
            gu->GetAI()->DoAction(ACTION_GU_P_3);
    }

    void DoAction(const int32 action) override
    {
        switch (action)
        {
        case ACTION_AZURE_SERPENT_P_2:
            events.CancelEventGroup(PHASE_ONE);
            events.RescheduleEvent(EVENT_MAGNETIC_SHROUD, urand(10000, 15000), PHASE_TWO);
            events.RescheduleEvent(EVENT_LIGHTNING_BREATH, urand(2500, 7500), PHASE_TWO);
            me->RemoveAurasDueToSpell(SPELL_LIGHTNING_SHIELD);
            break;
        }
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
        case EVENT_STATIC_FIELD:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                DoCast(target, SPELL_STATIC_FIELD, false);

            events.RescheduleEvent(EVENT_STATIC_FIELD, 10000);
            break;
        case EVENT_MAGNETIC_SHROUD:
            DoCast(SPELL_MAGNETIC_SHROUD);
            events.RescheduleEvent(EVENT_MAGNETIC_SHROUD, urand(10000, 15000));
            break;
        case EVENT_LIGHTNING_BREATH:
            if (auto target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                DoCast(target, SPELL_LIGHTNING_BREATH, false);

            events.RescheduleEvent(EVENT_LIGHTNING_BREATH, urand(7500, 12500));
            break;
        }
    }
};

class AreaTrigger_at_gu_intro : public AreaTriggerScript
{
    public:
        AreaTrigger_at_gu_intro() : AreaTriggerScript("at_gu_intro") {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/, bool apply) override
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                if (auto gu = instance->instance->GetCreature(instance->GetGuidData(NPC_GU_CLOUDSTRIKE)))
                    gu->AI()->DoAction(ACTION_INTRO);

            return false;
        }
};

class OnlyGuardianPredicate
{
    public:
        OnlyGuardianPredicate() {}

        bool operator()(WorldObject* target)
        {
            return target->GetEntry() != NPC_GUARDIAN;
        }
};

class spell_kill_guardians : public SpellScript
{
    PrepareSpellScript(spell_kill_guardians);

    void SelectTarget(std::list<WorldObject*>& targetList)
    {
        targetList.remove_if(OnlyGuardianPredicate());
        _targetList = targetList;
    }

    void KillTarget(std::list<WorldObject*>& targetList)
    {
        targetList = _targetList;

        for (std::list<WorldObject*>::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
            if (Creature* target = (*itr)->ToCreature())
                target->DespawnOrUnsummon(2000);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kill_guardians::SelectTarget, EFFECT_0, TARGET_SRC_CASTER);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kill_guardians::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kill_guardians::KillTarget, EFFECT_1, TARGET_SRC_CASTER);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kill_guardians::KillTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
    }

    std::list<WorldObject*> _targetList;
};

class spell_overcharged_soul_damage : public SpellScript
{
    PrepareSpellScript(spell_overcharged_soul_damage);

    void ChangeDamage(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        SetHitDamage(25000 / caster->GetHealthPct());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_overcharged_soul_damage::ChangeDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

void AddSC_boss_gu_cloudstrike()
{
    RegisterCreatureAI(boss_gu_cloudstrike);
    RegisterCreatureAI(npc_azure_serpent);
    RegisterSpellScript(spell_kill_guardians);
    RegisterSpellScript(spell_overcharged_soul_damage);
    //new AreaTrigger_at_gu_intro();
}
