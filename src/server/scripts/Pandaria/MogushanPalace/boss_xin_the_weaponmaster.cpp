/*===============
================*/

#include "mogu_shan_palace.h"

enum eEvents
{
    EVENT_RING_OF_FIRE = 1,
    EVENT_HEURT = 2,
    EVENT_INCITING_ROAR = 3,
    EVENT_SWORD_THROWER = 4,
    EVENT_SWORD_THROWER_STOP = 5,
    EVENT_AXES_ACTIVATE = 6,
    EVENT_AXES_DESACTIVATE = 7,
    EVENT_SUMMON_RING_OF_FIRE = 1,
    EVENT_UNSUMMON = 2,
    EVENT_SUMMON_RING_TRIGGER = 3
};

enum eActions
{
    ACTION_ACTIVATE
};

struct boss_xin_the_weaponmaster : public BossAI
{
    explicit boss_xin_the_weaponmaster(Creature* creature) : BossAI(creature, DATA_XIN_THE_WEAPONMASTER), summons(me) {}

    SummonList summons;
    bool onegem;
    bool twogem;

    void Reset() override
    {
        if (instance)
            instance->SetData(TYPE_ACTIVATE_SWORD, 0);

        onegem = true;
        twogem = true;

        summons.DespawnAll();
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_RING_OF_FIRE, 3000);
        events.RescheduleEvent(EVENT_HEURT, urand(10000, 15000));
        events.RescheduleEvent(EVENT_INCITING_ROAR, urand(15000, 25000));
        events.RescheduleEvent(EVENT_SWORD_THROWER, 30000);
        events.RescheduleEvent(EVENT_AXES_ACTIVATE, 15000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (me->HealthBelowPctDamaged(66, damage) && onegem)
        {
            onegem = false;
            me->SummonCreature(CREATURE_LAUNCH_SWORD, otherPos[2]);
        }

        if (me->HealthBelowPctDamaged(33, damage) && twogem)
        {
            twogem = false;
            me->SummonCreature(CREATURE_LAUNCH_SWORD, otherPos[3]);
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);
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
            case EVENT_RING_OF_FIRE:
                if (instance)
                    instance->SetData(TYPE_ACTIVATE_ANIMATED_STAFF, 0);

                events.RescheduleEvent(EVENT_RING_OF_FIRE, 20000);
                break;
            case EVENT_HEURT:
                DoCast(119684);
                events.RescheduleEvent(EVENT_HEURT, urand(10000, 15000));
                break;
            case EVENT_INCITING_ROAR:
                DoCast(122959);
                events.RescheduleEvent(EVENT_INCITING_ROAR, 30000);
                break;
            case EVENT_SWORD_THROWER:
                if (instance)
                    instance->SetData(TYPE_ACTIVATE_SWORD, 1);

                events.RescheduleEvent(EVENT_SWORD_THROWER_STOP, 10000);
                break;
            case EVENT_SWORD_THROWER_STOP:
                if (instance)
                    instance->SetData(TYPE_ACTIVATE_SWORD, 0);

                events.RescheduleEvent(EVENT_SWORD_THROWER, 20000);
                break;
            case EVENT_AXES_ACTIVATE:
                if (instance)
                    instance->SetData(TYPE_ACTIVATE_ANIMATED_AXE, 1);

                events.RescheduleEvent(EVENT_AXES_DESACTIVATE, 10000);
                break;
            case EVENT_AXES_DESACTIVATE:
                if (instance)
                    instance->SetData(TYPE_ACTIVATE_ANIMATED_AXE, 0);

                events.RescheduleEvent(EVENT_AXES_ACTIVATE, 15000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_animated_staff : public ScriptedAI
{
    explicit mob_animated_staff(Creature* creature) : ScriptedAI(creature)
    {
        me->SetDisplayId(42195);
        me->SetVirtualItem(0, 76364);
        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
        instance = me->GetInstanceScript();
    }

    EventMap events;
    InstanceScript* instance;
    float _x;
    float _y;
    float point;

    void Reset() override
    {
        _x = 0.f;
        _y = 0.f;
        point = 0.f;

        me->AddAura(130966, me);

        Position home = me->GetHomePosition();
        me->GetMotionMaster()->MovePoint(0, home);
    }

    void DoAction(const int32 action) override
    {
        switch (action)
        {
        case ACTION_ACTIVATE:
            me->RemoveAura(130966);
            events.RescheduleEvent(EVENT_SUMMON_RING_OF_FIRE, 500);
            break;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == 61499)
        {
            summoned->setFaction(14);
            summoned->SetReactState(REACT_PASSIVE);
            summoned->AddAura(119544, summoned);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SUMMON_RING_OF_FIRE:
            {
                events.RescheduleEvent(EVENT_UNSUMMON, 9000);
                std::list<Unit*> units;
                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (player->isAlive() && !player->isGameMaster())
                        units.push_back(player);
                });

                if (!units.empty())
                {
                    if (auto target = Trinity::Containers::SelectRandomContainerElement(units))
                    {
                        me->GetMotionMaster()->MovePoint(0, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                        point = 0.0f;
                        _x = target->GetPositionX();
                        _y = target->GetPositionY();
                    }
                }
                events.RescheduleEvent(EVENT_SUMMON_RING_TRIGGER, 100);
                break;
            }
            case EVENT_UNSUMMON:
                EnterEvadeMode();
                break;
            case EVENT_SUMMON_RING_TRIGGER:
                if (point >= 11)
                {
                    if (TempSummon* tmp = me->SummonCreature(61499, _x, _y, me->GetMap()->GetHeight(_x, _y, me->GetPositionZ()), 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10000))
                    {
                        tmp->RemoveAura(119544);
                        tmp->CastSpell(tmp, 119590, false);
                    }
                    return;
                }

                float x = _x + 5.0f * cos(point * M_PI / 5);
                float y = _y + 5.0f * sin(point * M_PI / 5);
                me->SummonCreature(61499, x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ()), 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10000);
                ++point;
                events.RescheduleEvent(EVENT_SUMMON_RING_TRIGGER, 400);
                break;
            }
        }
    }
};

class OnlyTriggerInFrontPredicate
{
    public:
        OnlyTriggerInFrontPredicate(Unit* caster) : _caster(caster) {}

        bool operator()(WorldObject* target)
        {
            return target->GetEntry() != 59481 || !_caster->isInFront(target, M_PI / 5) || target->GetGUID() == _caster->GetGUID();
        }

    private:
        Unit* _caster;
};

class spell_dart : public SpellScript
{
    PrepareSpellScript(spell_dart);

    void SelectTarget(std::list<WorldObject*>& targetList)
    {
        Unit* caster = GetCaster();

        if (!caster)
            return;

        if (targetList.empty())
        {
            FinishCast(SPELL_FAILED_NO_VALID_TARGETS);
            return;
        }

        //Select the two targets.
        std::list<WorldObject*> targets = targetList;
        targetList.remove_if(OnlyTriggerInFrontPredicate(caster));

        //See if we intersect with any players.
        for (std::list<WorldObject*>::iterator object = targets.begin(); object != targets.end(); ++object)
        {
            if ((*object)->IsPlayer())
            {
                for (std::list<WorldObject*>::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                {
                    if ((*object)->IsInBetween(caster, *itr, 2.0f))
                    {
                        const SpellInfo* damageSpell = sSpellMgr->GetSpellInfo(SPELL_THROW_DAMAGE);
                        caster->DealDamage((*object)->ToPlayer(), damageSpell->Effects[0]->BasePoints, 0, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, damageSpell);
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dart::SelectTarget, EFFECT_0, TARGET_SRC_CASTER);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dart::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dart::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dart::SelectTarget, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
    }
};

void AddSC_boss_xin_the_weaponmaster()
{
    RegisterCreatureAI(boss_xin_the_weaponmaster);
    RegisterCreatureAI(mob_animated_staff);
    RegisterSpellScript(spell_dart);
}
