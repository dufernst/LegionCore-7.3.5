/*==============
==============*/

#include "siege_of_the_niuzoa_temple.h"

enum Yells
{
    SAY_AGGRO,
    SAY_FLIGHT,
    SAY_WIPE,
    SAY_DEATH,
    SAY_SLAY,
    EMOTE_WINDS
};

enum Spells
{
    SPELL_HURL_BRICK        = 121762,
    SPELL_CAUSTIC_PITCH     = 121443,
    SPELL_QUICK_DRY_RESIN   = 121447,
    SPELL_GUSTING_WINDS     = 121282,
    SPELL_ENCASED_IN_RESIN  = 121448,
    SPELL_SCREEN_EFFECT     = 122063
};

enum Events
{
    EVENT_HAUL_BRICK        = 1,
    EVENT_CAUSTIC_PITCH,
    EVENT_QUICK_DRY_RESIN,
    EVENT_SWITCH_PHASE,
    EVENT_GUSTING_WINDS,
    EVENT_FLY_OTHER_END,
    EVENT_LAND,
    EVENT_RE_ENGAGE,

    EVENT_GROUP_MOVEMENT    = 1,
    EVENT_GROUP_COMBAT
};

enum Other
{
    ACTION_INTERRUPT = 1,
    NPC_CHUM_KIU = 64517
};

struct boss_wing_leader_neronok : public BossAI
{
    explicit boss_wing_leader_neronok(Creature * creature) : BossAI(creature, DATA_NERONOK) {}

    uint8 phase;
    ObjectGuid victimGuid;
    bool interrupted;
    bool pct_70;
    bool pct_40;
    bool winds;

    void Reset() override
    {
        pct_70 = false;
        pct_40 = false;
        winds = false;
        interrupted = false;
        SetCombatMovement(false);
        events.Reset();
        me->SetCanFly(false);
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetSpeed(MOVE_FLIGHT, 3.5f);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);
        _Reset();
    }

    void ClearDebuffs()
    {
        if (auto trigger = me->GetAreaObject(SPELL_GUSTING_WINDS))
            trigger->SetDuration(0);

        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_QUICK_DRY_RESIN);
        std::list<DynamicObject *> dList;
        me->GetDynObjectList(dList, 121443);

        for (auto itr : dList)
            itr->SetDuration(0);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_INTERRUPT)
        {
            interrupted = true;
            events.CancelEvent(EVENT_GUSTING_WINDS);
            if (auto trigger = me->GetAreaObject(SPELL_GUSTING_WINDS))
                trigger->SetDuration(0);
        }
    }

    void AttackStart(Unit* target) override
    {
        if (!target)
            return;

        if (me->Attack(target, true))
            DoStartNoMovement(target);
    }

    void DamageTaken(Unit* /*who*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(70) && !pct_70)
        {
            pct_70 = true;
            interrupted = false;
            winds = true;
            Talk(SAY_FLIGHT);
            Talk(EMOTE_WINDS);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->SetCanFly(true);
            events.CancelEventGroup(EVENT_GROUP_COMBAT);
            events.CancelEventGroup(EVENT_GROUP_MOVEMENT);
            me->GetMotionMaster()->MoveTakeoff(0, 1808.063f, 5251.847f, 142.6636f);
            events.RescheduleEvent(EVENT_LAND, 5000);
            events.RescheduleEvent(EVENT_RE_ENGAGE, 6000);
        }

        if (me->HealthBelowPct(40) && !pct_40)
        {
            winds = false;
            pct_40 = true;
            interrupted = false;
            Talk(SAY_FLIGHT);
            Talk(EMOTE_WINDS);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->SetCanFly(true);
            events.CancelEventGroup(EVENT_GROUP_COMBAT);
            events.CancelEventGroup(EVENT_GROUP_MOVEMENT);
            me->GetMotionMaster()->MoveTakeoff(0, 1888.998f, 5177.15f, 143.3173f);
            events.RescheduleEvent(EVENT_LAND, 5000);
            events.RescheduleEvent(EVENT_RE_ENGAGE, 6000);
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim && victim->IsPlayer())
            Talk(SAY_SLAY);
    }

    void JustReachedHome() override
    {
        Talk(SAY_WIPE);
        _JustReachedHome();
    }

    void EnterEvadeMode() override
    {
        ClearDebuffs();
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*who*/) override
    {
        ClearDebuffs();
        Talk(SAY_DEATH);
        _JustDied();

        me->SummonCreature(NPC_CHUM_KIU, 1851.226f, 5214.163f, 131.2519f, 4.03f);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        events.RescheduleEvent(EVENT_HAUL_BRICK, 1000, EVENT_GROUP_COMBAT);
        events.RescheduleEvent(EVENT_CAUSTIC_PITCH, 3000, EVENT_GROUP_COMBAT);
        events.RescheduleEvent(EVENT_QUICK_DRY_RESIN, 8000, EVENT_GROUP_COMBAT);
        _EnterCombat();
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
            case EVENT_HAUL_BRICK:
                if (auto victim = me->getVictim())
                    if (!victim->IsWithinMeleeRange(me))
                        DoCast(victim, SPELL_HURL_BRICK, false);

                events.RescheduleEvent(EVENT_HAUL_BRICK, 2000, EVENT_GROUP_COMBAT);
                break;
            case EVENT_CAUSTIC_PITCH:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_CAUSTIC_PITCH, true);

                events.RescheduleEvent(EVENT_CAUSTIC_PITCH, 10000, EVENT_GROUP_COMBAT);
                break;
            case EVENT_QUICK_DRY_RESIN:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_QUICK_DRY_RESIN, false);

                events.RescheduleEvent(EVENT_QUICK_DRY_RESIN, 8000, EVENT_GROUP_COMBAT);
                break;
            case EVENT_GUSTING_WINDS:
                if (interrupted)
                    break;
                if (winds)
                    DoCast(me, SPELL_GUSTING_WINDS, false);
                else
                    DoCast(me, 121284, false);

                events.RescheduleEvent(EVENT_GUSTING_WINDS, 10000, EVENT_GROUP_COMBAT);
                break;
            case EVENT_LAND:
                me->GetMotionMaster()->MoveLand(0, { me->GetPositionX(), me->GetPositionY(), 131.1689f });
                break;
            case EVENT_RE_ENGAGE:
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                {
                    AttackStart(victim);
                    victimGuid = victim->GetGUID();
                    me->SetTarget(victimGuid);
                }

                events.RescheduleEvent(EVENT_HAUL_BRICK, 1000, EVENT_GROUP_COMBAT);
                events.RescheduleEvent(EVENT_CAUSTIC_PITCH, 3000, EVENT_GROUP_COMBAT);
                events.RescheduleEvent(EVENT_QUICK_DRY_RESIN, 8000, EVENT_GROUP_COMBAT);
                events.RescheduleEvent(EVENT_GUSTING_WINDS, 2000, EVENT_GROUP_COMBAT);
                break;

            default:
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 121447
class spell_quick_dry_resin : public AuraScript
{
    PrepareAuraScript(spell_quick_dry_resin)

    void PeriodicTick(AuraEffect const * aurEff)
    {
        Unit* target = GetUnitOwner();
        if (!target)
            return;

        int32 powerAmt = target->GetPower(POWER_ALTERNATE) + 2;

        if (powerAmt >= 100)
        {
            target->RemoveAurasDueToSpell(aurEff->GetId());
            target->RemoveAurasDueToSpell(SPELL_SCREEN_EFFECT);
            target->CastSpell(target, SPELL_ENCASED_IN_RESIN, true);
            return;
        }
        else if (powerAmt >= 80)
        {
            if (!target->HasAura(SPELL_SCREEN_EFFECT))
                target->CastSpell(target, SPELL_SCREEN_EFFECT, true);
        }
        else if (target->HasAura(SPELL_SCREEN_EFFECT))
            target->RemoveAurasDueToSpell(SPELL_SCREEN_EFFECT);

        target->SetPower(POWER_ALTERNATE, powerAmt);
    }

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        // NEED FIX PROC FLAG.

        Unit* owner = GetTarget();
        if (!owner)
            return;
        if (owner->GetPower(POWER_ALTERNATE) <= 10)
            owner->RemoveAurasDueToSpell(121447);
        else
            owner->ModifyPower(POWER_ALTERNATE, -10);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_quick_dry_resin::OnProc, EFFECT_0, SPELL_AURA_ENABLE_ALT_POWER);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_quick_dry_resin::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 121282
class spell_neronok_gusting_winds : public AuraScript
{
    PrepareAuraScript(spell_neronok_gusting_winds);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* owner = GetCaster();
        if (!owner)
            return;

        owner->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
        owner->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, false);
    }

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* owner = GetCaster();
        if (!owner)
            return;

        owner->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
        owner->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);

        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_CANCEL
            && GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
            return;

        if (auto creature = owner->ToCreature())
            creature->AI()->DoAction(ACTION_1);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectRemoveFn(spell_neronok_gusting_winds::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_neronok_gusting_winds::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_boss_wing_leader_neronok()
{
    RegisterCreatureAI(boss_wing_leader_neronok);
    RegisterAuraScript(spell_quick_dry_resin);
    RegisterAuraScript(spell_neronok_gusting_winds);
}
