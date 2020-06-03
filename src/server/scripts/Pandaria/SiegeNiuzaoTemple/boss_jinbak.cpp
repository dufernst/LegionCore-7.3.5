/*==============
==============*/

#include "siege_of_the_niuzoa_temple.h"

enum eSpells
{
    SPELL_SUMMON_GLOBULE        = 119990,
    SPELL_DETONATE              = 120001,
    SPELL_DETONATE_ANIMATION    = 120002,

    SPELL_SAP_PUDDLE            = 119939,
    SPELL_VISUAL_SHIELD         = 131628,
    SPELL_SAP_RESIDUE           = 119941,
    SPELL_GROW                  = 120865
};

enum eEvents
{
    EVENT_GROW      = 1,
    EVENT_DEAL_DOT,
    EVENT_GLOBUES,
    EVENT_DETONATE,
    EVENT_COSMETIC_LASER
};

struct boss_jinbak : public BossAI
{
    explicit boss_jinbak(Creature* creature) : BossAI(creature, DATA_JINBAK) {}

    void Reset() override
    {
        _Reset();
        events.Reset();

        me->SummonCreature(61613, 1529.359497f, 5163.071289f, 158.893372f, 1.240147f);
        DoCast(120095);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();

        events.RescheduleEvent(EVENT_GROW, 4000);
        events.RescheduleEvent(EVENT_DETONATE, 35000);
        events.RescheduleEvent(EVENT_GLOBUES, 25000);

        if (instance)
            instance->SetBossState(DATA_JINBAK, IN_PROGRESS);

        me->RemoveAllAuras();
        Talk(1);

        if (auto puddle = me->FindNearestCreature(61613, 100.0f, true))
        {
            puddle->RemoveAllAuras();
            puddle->CastSpell(puddle, 119939, false);
        }
    }

    void JustReachedHome() override
    {
        if (instance)
            instance->SetBossState(DATA_JINBAK, FAIL);

        summons.DespawnAll();
        me->SummonCreature(61613, 1529.359497f, 5163.071289f, 158.893372f, 1.240147f);
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim && !victim->IsPlayer())
            return;

        Talk(5);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_GLOBUES:
                ZoneTalk(3);
                me->SummonCreature(61623, 1559.777100f, 5154.710938f, 161.472107f);
                me->SummonCreature(61623, 1497.709595f, 5169.799316f, 161.033157f);
                me->SummonCreature(61623, 1514.540039f, 5185.884766f, 160.270462f);
                events.RescheduleEvent(EVENT_GLOBUES, 40000);
                break;
            case EVENT_DETONATE:
                if (auto puddle = me->FindNearestCreature(61613, 100.0f, true))
                    me->CastSpell(puddle, 120001);

                Talk(4);
                events.RescheduleEvent(EVENT_DETONATE, 35000);
                break;
            case EVENT_GROW:
                if (auto puddle = me->FindNearestCreature(61613, 100.0f, true))
                {
                    puddle->AddAura(120865, puddle);

                    if (puddle->HasAura(120865))
                        puddle->SetAuraStack(120865, puddle, puddle->GetAura(120865)->GetStackAmount() + 1);

                    events.RescheduleEvent(EVENT_GROW, 1000);
                }
                break;
            }
            DoMeleeAttackIfReady();
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();

        if (instance)
        {
            instance->SetBossState(DATA_JINBAK, DONE);
            instance->DoRemoveAurasDueToSpellOnPlayers(119941);
            instance->DoRemoveAurasDueToSpellOnPlayers(120938);
        }

        Talk(2);
        summons.DespawnAll();

        if (auto objg = me->FindNearestGameObject(213174, 600.0f))
            objg->Delete();

        me->SummonCreature(62795, 1524.239990f, 5309.060059f, 185.227005f, 4.759360f, TEMPSUMMON_MANUAL_DESPAWN, 0);
    }
};

struct npc_globue : public ScriptedAI
{
    explicit npc_globue(Creature* creature) : ScriptedAI(creature)
    {
        me->AddUnitState(UNIT_STATE_CANNOT_AUTOATTACK);
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void Reset() override
    {
        me->GetMotionMaster()->MovePoint(1, 1529.432495f, 5163.758301f, 158.892502f);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (auto paddle = me->FindNearestCreature(61613, 100, true))
        {
            float preradius = me->GetExactDist2d(me->GetPositionX(), me->GetPositionY());
            float width = 5.0f;

            float radius = width + preradius;
            float modifiedRadius = (me->GetObjectSize()) + radius;

            if (paddle->HasAura(SPELL_GROW))
            {
                if (auto aura = paddle->GetAura(SPELL_GROW))
                {
                    if (me->IsWithinDistInMap(paddle, modifiedRadius, true))
                    {
                        aura->SetStackAmount(aura->GetStackAmount() + 1);
                        me->DespawnOrUnsummon(100);
                    }
                }
            }
        }
    }
};

struct npc_puddle : public ScriptedAI
{
    explicit npc_puddle(Creature* creature) : ScriptedAI(creature)
    {
        me->SetObjectScale(0.4f);
        me->SetDisplayId(38497);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
    }

    EventMap events;

    bool Used;

    void Reset() override
    {
        events.Reset();
        Used = false;

        me->CastSpell(me, SPELL_SAP_PUDDLE, true);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (auto* pl = me->SelectNearestPlayer(1.0f))
        {
            if (pl->IsWithinDistInMap(me, 1.0f, true) && !Used)
            {
                bool Used = true;
                events.RescheduleEvent(EVENT_GROW, 2000);
                events.RescheduleEvent(EVENT_CHARGE, 4000);
            }
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CHARGE:
                Used = false;
                break;
            case EVENT_GROW:
                if (me->HasAura(SPELL_GROW))
                {
                    float stacks = me->GetAura(SPELL_GROW)->GetStackAmount() * 2;

                    if (auto pl = me->SelectNearestPlayer(1.0f + stacks))
                    {
                        if (pl->IsWithinDistInMap(me, 1.0f + stacks, true) && !Used)
                        {
                            me->AddAura(119941, pl);
                            me->AddAura(120938, pl);

                            if (me->HasAura(SPELL_GROW))
                                if (auto aura = me->GetAura(SPELL_GROW))
                                    aura->Remove();
                        }
                    }
                }
                break;
            }
        }
    }
};

//119941
class spell_sap_residue : public SpellScript
{
    PrepareSpellScript(spell_sap_residue);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        float dist = 5.0f;

        if (AuraEffect* eff = caster->GetAuraEffect(SPELL_GROW, EFFECT_0))
            dist *= eff->GetAmount() / 100.0f;

        if (!targets.empty())
            targets.remove_if([&caster, &dist](const WorldObject * unit) { return unit->GetExactDist2d(caster) > dist; });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sap_residue::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

class spell_jinbak_detonate : public AuraScript
{
    PrepareAuraScript(spell_jinbak_detonate);

    void OnTick(AuraEffect const *aurEff)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        uint32 stackAmount = std::max<uint32>(1, target->GetAuraCount(SPELL_GROW));
        float damage = (target->GetMap()->IsHeroic() ? 25500 : 8500) * stackAmount;
        target->CastCustomSpell(target, 120002, &damage, 0, 0, true);
        target->RemoveAurasDueToSpell(SPELL_GROW);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_jinbak_detonate::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

void AddSC_boss_jinbak()
{
    RegisterCreatureAI(boss_jinbak);
    RegisterCreatureAI(npc_globue);
    RegisterCreatureAI(npc_puddle);
    RegisterSpellScript(spell_sap_residue);
    RegisterAuraScript(spell_jinbak_detonate);
}