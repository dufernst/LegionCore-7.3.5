
#include "blackrock_caverns.h"

//todo: реализовать касты щенков, берсерк при убийстве ранти, взрыв magma split
enum Spells
{
    SPELL_BERSERK               = 82395,
    SPELL_BERSERKER_CHARGE      = 76030,
    SPELL_FLAMEBREAK            = 76032,
    SPELL_MAGMA_SPLIT           = 76031,
    SPELL_TERRIFYING_ROAR       = 76028
};

enum events
{
    EVENT_MAGMA_SPLIT           = 1,
    EVENT_TERRIFYING_ROAR       = 2,
    EVENT_FLAMEBREAK            = 3,
    EVENT_BERSERKER_CHARGE      = 4
};

enum Adds
{
    NPC_SPOT    = 40011,
    NPC_BUSTER  = 40013,
    NPC_LUCKY   = 40008,
    NPC_RUNTY   = 40015
};

const Position pupsPos[2] = 
{
    {116.70f, 572.05f, 76.45f, 0.04f},
    {119.81f, 588.90f, 76.35f, 6.22f}
};

class boss_beauty : public CreatureScript
{
 public:
    boss_beauty() : CreatureScript("boss_beauty") {}
 
    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_beautyAI (pCreature);
    }
 
    struct boss_beautyAI : public ScriptedAI
    {
        boss_beautyAI(Creature* c) : ScriptedAI(c), summons(me)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            instance = (InstanceScript*)c->GetInstanceScript();
        }
 
        InstanceScript* instance;
        EventMap events;
        SummonList summons;

        void Reset() override
        {
            summons.DespawnAll();
            events.Reset();
            instance->SetData(DATA_BEAUTY, NOT_STARTED);
        }
 
        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_BERSERKER_CHARGE, 1000);
            events.RescheduleEvent(EVENT_FLAMEBREAK, 15000);
            events.RescheduleEvent(EVENT_MAGMA_SPLIT, 18000);
            events.RescheduleEvent(EVENT_TERRIFYING_ROAR, 30000);
            DoZoneInCombat();
            if (instance)
                instance->SetData(DATA_BEAUTY, IN_PROGRESS);
        }
 
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (me->GetDistance(me->GetHomePosition()) > 60.0f)
            {
                EnterEvadeMode();
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BERSERKER_CHARGE:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_BERSERKER_CHARGE);
                    events.RescheduleEvent(EVENT_BERSERKER_CHARGE, 10000);
                    break;
                case EVENT_FLAMEBREAK:
                    DoCast(me, SPELL_FLAMEBREAK);
                    events.RescheduleEvent(EVENT_FLAMEBREAK, 15000);
                    break;
                case EVENT_MAGMA_SPLIT:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_MAGMA_SPLIT);
                    events.RescheduleEvent(EVENT_MAGMA_SPLIT, 18000);
                    break;
                case EVENT_TERRIFYING_ROAR:
                    DoCast(SPELL_TERRIFYING_ROAR);
                    events.RescheduleEvent(EVENT_TERRIFYING_ROAR, 30000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
 
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            summons.Despawn(summon);
        }

        void JustDied(Unit* /*killer*/) override
        {
            summons.DespawnAll();
            if (instance)
                instance->SetData(DATA_BEAUTY, DONE);
        }
    };
};
 
class spell_defiled_earth_rager_meteor : public SpellScript
{
    PrepareSpellScript(spell_defiled_earth_rager_meteor);

    uint8 targetsCount = 0;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targetsCount = targets.size();
    }

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        if (targetsCount)
            SetHitDamage(GetHitDamage() / targetsCount);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_defiled_earth_rager_meteor::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_defiled_earth_rager_meteor::HandleDamage, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

void AddSC_boss_beauty()
{
    new boss_beauty();
    RegisterSpellScript(spell_defiled_earth_rager_meteor);
}
