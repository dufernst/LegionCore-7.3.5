/*====================
======================*/

#include "stormstout_brewery.h"

enum Spells
{
    SPELL_GROUND_POUND  = 106807,
    SPELL_GOING_BANANAS  = 106651,
    SPELL_BAREL_EXPLOSION = 106769,
    SPELL_FORCECAST_BARREL_DROP = 122385
};

struct boss_ook_ook : public BossAI
{
    explicit boss_ook_ook(Creature* creature) : BossAI(creature, DATA_OOK_OOK) {}

    uint32 groundtimer;
    bool fbuff, sbuff, lbuff;

    void Reset() override
    {
        _Reset();
        fbuff = false;
        sbuff = false;
        lbuff = false;
        groundtimer = 0;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        groundtimer = 5000;
    }

    void JustDied(Unit* /*killer*/) override
    {
        std::list<Creature*> bouncer;
        me->GetCreatureListWithEntryInGrid(bouncer, 56849, 200.0f);
        if (!bouncer.empty())
            for (auto& cre_bouncer : bouncer)
                cre_bouncer->DespawnOrUnsummon();

        _JustDied();
    }

    void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType dmgType) override
    {
        if (HealthBelowPct(90) && !fbuff)
        {
            fbuff = true;
            DoCast(SPELL_GOING_BANANAS);
            return;
        }
        else if (HealthBelowPct(60) && !sbuff)
        {
            sbuff = true;
            DoCast(SPELL_GOING_BANANAS);
            return;
        }
        else if (HealthBelowPct(30) && !lbuff)
        {
            lbuff = true;
            DoCast(SPELL_GOING_BANANAS);
            return;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (groundtimer <= diff)
        {
            DoCast(SPELL_GROUND_POUND);
            groundtimer = 10000;
        }
        else
            groundtimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct npc_barrel : public ScriptedAI
{
    npc_barrel(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        me->GetMotionMaster()->MovePoint(100, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (id != 100)
            return;

        float x = 0, y = 0;
        GetPositionWithDistInOrientation(me, 5.0f, me->GetOrientation(), x, y);

        me->GetMotionMaster()->MovePoint(100, x, y, me->GetPositionZ());
    }

    bool CheckIfAgainstWall()
    {
        float x = 0, y = 0;
        GetPositionWithDistInOrientation(me, 5.0f, me->GetOrientation(), x, y);

        if (!me->IsWithinLOS(x, y, me->GetPositionZ()))
            return true;

        return false;
    }

    bool CheckIfAgainstUnit()
    {
        if (me->SelectNearbyTarget(NULL, 1.0f))
            return true;

        return false;
    }

    void DoExplode()
    {
        if (auto barrel = me->GetVehicleKit())
            barrel->RemoveAllPassengers();

        me->Kill(me);
        DoCast(me, SPELL_BAREL_EXPLOSION, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (CheckIfAgainstWall() || CheckIfAgainstUnit())
            DoExplode();
    }
};

class spell_ook_ook_barrel_ride : public AuraScript
{
    PrepareAuraScript(spell_ook_ook_barrel_ride);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (auto barrelBase = target)
            barrelBase->GetMotionMaster()->MoveIdle();
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_ook_ook_barrel_ride::OnApply, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

class spell_ook_ook_barrel : public AuraScript
{
    PrepareAuraScript(spell_ook_ook_barrel);

    bool CheckIfAgainstWall(Unit* caster)
    {
        float x = caster->GetPositionX() + (2 * cos(caster->GetOrientation()));
        float y = caster->GetPositionY() + (2 * sin(caster->GetOrientation()));

        if (!caster->IsWithinLOS(x, y, caster->GetPositionZ()))
            return true;

        return false;
    }

    bool CheckIfAgainstUnit(Unit* caster)
    {
        if (caster->SelectNearbyTarget(NULL, 1.0f))
            return true;

        return false;
    }

    void OnUpdate(uint32 diff, AuraEffect* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (CheckIfAgainstWall(caster) || CheckIfAgainstUnit(caster))
        {
            if (auto barrel = caster->GetVehicle())
            {
                barrel->RemoveAllPassengers();

                if (auto barrelBase = barrel->GetBase())
                {
                    barrelBase->CastSpell(barrelBase, SPELL_BAREL_EXPLOSION, true);
                    barrelBase->Kill(barrelBase);
                }
            }

            caster->CastSpell(caster, SPELL_FORCECAST_BARREL_DROP, true);
            caster->RemoveAurasDueToSpell(GetSpellInfo()->Id);
        }
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_ook_ook_barrel::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

void AddSC_boss_ook_ook()
{
    RegisterCreatureAI(boss_ook_ook);
    RegisterCreatureAI(npc_barrel);
    //RegisterAuraScript(spell_ook_ook_barrel_ride);
    //RegisterAuraScript(spell_ook_ook_barrel);
}
