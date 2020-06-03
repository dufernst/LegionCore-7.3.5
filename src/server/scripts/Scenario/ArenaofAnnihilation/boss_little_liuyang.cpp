/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "arena_of_annihilation.h"

enum Says
{
    SAY_WARN_WALL   = 0,
};

enum Spells
{
    SPELL_GIANT_HUI     = 122213,
    SPELL_FIREBALL      = 123958,
    SPELL_FLAMELINE     = 123960,
    SPELL_FLAME_WALL    = 123966,
    SPELL_FLAME_SHIELD  = 131666,
};

enum Events
{
    EVENT_POINT_HOME    = 1,
    EVENT_FIREBALL      = 2,
    EVENT_FLAMELINE     = 3,
    EVENT_FLAME_WALL    = 4,
};

class boss_little_liuyang : public CreatureScript
{
public:
    boss_little_liuyang() : CreatureScript("boss_little_liuyang") { }

    struct boss_little_liuyangAI : public BossAI
    {
        boss_little_liuyangAI(Creature* creature) : BossAI(creature, DATA_LIUYANG)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override
        {
            events.Reset();
            SetCombatMovement(false);
            me->SetReactState(REACT_PASSIVE);
            events.RescheduleEvent(EVENT_POINT_HOME, 2000);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_FIREBALL, 1000);
            events.RescheduleEvent(EVENT_FLAMELINE, 30000);
            //events.RescheduleEvent(EVENT_FLAME_WALL, 40000);
        }

        void EnterEvadeMode() override
        {
            _Reset();
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                    pGo->SetGoState(GO_STATE_READY);
                DoCast(SPELL_GIANT_HUI);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_AGGRESSIVE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_POINT_HOME:
                        if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                            pGo->SetGoState(GO_STATE_ACTIVE);
                        me->GetMotionMaster()->MovePoint(1, centerPos);
                        break;
                    case EVENT_FIREBALL:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_FIREBALL);
                        events.RescheduleEvent(EVENT_FIREBALL, 2000);
                        break;
                    case EVENT_FLAMELINE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            DoCast(target, SPELL_FLAMELINE);
                        events.RescheduleEvent(EVENT_FLAMELINE, 24000);
                        break;
                    case EVENT_FLAME_WALL:
                        Talk(SAY_WARN_WALL);
                        DoCast(SPELL_FLAME_WALL);
                        events.RescheduleEvent(EVENT_FLAME_WALL, 30000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_little_liuyangAI (creature);
    }
};

class spell_liuyang_flameline : public SpellScriptLoader
{
    public:
        spell_liuyang_flameline() : SpellScriptLoader("spell_liuyang_flameline") { }

        class spell_liuyang_flameline_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_liuyang_flameline_AuraScript);

            Position savePos;
            void OnTick(AuraEffect const* aurEff)
            {
                if(Unit* caster = GetCaster())
                {
                    float distance = aurEff->GetTickNumber() * 2;
                    float angle = caster->GetOrientation();
                    float destx = caster->GetPositionX() + distance * std::cos(angle);
                    float desty = caster->GetPositionY() + distance * std::sin(angle);
                    caster->CastSpell(destx, desty, caster->GetPositionZ(), 123959, true);
                    if(aurEff->GetTickNumber() >= 15)
                        GetAura()->Remove();
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_liuyang_flameline_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_liuyang_flameline_AuraScript();
        }
};

void AddSC_boss_little_liuyang()
{
    new boss_little_liuyang();
    new spell_liuyang_flameline();
}
