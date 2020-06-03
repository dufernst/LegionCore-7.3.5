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

#include "mogu_shan_vault.h"

enum spells
{
    SPELL_SPIRIT_BOLT           = 121224,
    SPELL_GROUND_SLAM           = 121087,
    SPELL_PETRIFICATION         = 125090,
    SPELL_PETRIFIED             = 125092,
    SPELL_FULLY_PETRIFIED       = 115877,
    SPELL_MONSTROUS_BITE        = 125096,
    SPELL_SUNDERING_BITE        = 116970,
    SPELL_PROTECTIVE_FENZY      = 116982,
    SPELL_SHATTERING_STONE      = 116977,
    SPELL_FOCUSED_ASSAULT       = 116990
};

class mob_cursed_mogu_sculpture : public CreatureScript
{
    public:
        mob_cursed_mogu_sculpture() : CreatureScript("mob_cursed_mogu_sculture") {}

        struct mob_cursed_mogu_sculptureAI : public ScriptedAI
        {
            mob_cursed_mogu_sculptureAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;
            uint32 spiritBoltTimer;
            uint32 groundSlamTimer;

            void Reset() override
            {
                spiritBoltTimer = urand(10000, 40000);
                groundSlamTimer = urand(40000, 60000);

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);

                if (me->GetEntry() == 61334)
                {
                    me->AddAura(120613, me); // Pose
                    me->AddAura(120661, me); // Bronze
                }
                else if (me->GetEntry() == 61989)
                {
                    me->AddAura(120650, me); // Pose
                    me->AddAura(120663, me); // Pierre
                }
            }

            void EnterCombat(Unit* attacker) override
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveAurasDueToSpell(120661);
                me->RemoveAurasDueToSpell(120613);
                me->RemoveAurasDueToSpell(120650);
                me->RemoveAurasDueToSpell(120663);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (spiritBoltTimer <= diff)
                {
                    if (Unit* target = me->SelectNearestTarget(5.0f))
                        if (!target->IsFriendlyTo(me))
                            me->CastSpell(target, SPELL_SPIRIT_BOLT, true);
                    spiritBoltTimer = urand(10000, 30000);
                }
                else
                    spiritBoltTimer -= diff;
                 
                if (groundSlamTimer <= diff)
                {
                    if (Unit* target = me->SelectNearestTarget(5.0f))
                        if (!target->IsFriendlyTo(me))
                            me->CastSpell(target, SPELL_GROUND_SLAM, true);
                    groundSlamTimer = urand(40000, 60000);
                }
                else
                    groundSlamTimer -= diff;

                DoMeleeAttackIfReady();
            }
            
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_cursed_mogu_sculptureAI(creature);
        }
};

float quilenNewY[2] = {1170.0f, 1240.0f};

enum enormousQuilenEvent
{
    EVENT_MONSTROUS_BITE = 1,
    EVENT_NEXT_MOVEMENT  = 2
};

class mob_enormous_stone_quilen : public CreatureScript
{
    public:
        mob_enormous_stone_quilen() : CreatureScript("mob_enormous_stone_quilen") {}

        struct mob_enormous_stone_quilenAI : public ScriptedAI
        {
            mob_enormous_stone_quilenAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                prevPosition = 1;

                if (me->GetPositionX() > 3900)
                    prevPosition = 2;

                nextMovementTimer = 0;
                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(prevPosition, me->GetPositionX(), quilenNewY[prevPosition - 1], me->GetPositionZ());
            }

            InstanceScript* pInstance;
            EventMap events;
            uint32 nextMovementTimer;
            uint8 prevPosition;

            void Reset() override
            {
                events.RescheduleEvent(EVENT_MONSTROUS_BITE, urand (3000, 5000));
            }

            void JustReachedHome() override
            {
                prevPosition = 1;

                if (me->GetPositionX() > 3900)
                    prevPosition = 2;

                nextMovementTimer = 0;
                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(prevPosition, me->GetPositionX(), quilenNewY[prevPosition - 1], me->GetPositionZ());
            }

            void MovementInform(uint32 typeId, uint32 pointId) override
            {
                if (typeId != POINT_MOTION_TYPE)
                    return;

                if (me->isInCombat())
                    return;

                prevPosition = pointId;
                nextMovementTimer = 500;
            }

            void EnterCombat(Unit* attacker) override
            {
                me->SetWalk(false);
                //me->AddAura(SPELL_PETRIFICATION, me);
            }

            void UpdateAI(uint32 diff) override
            {
                if (nextMovementTimer)
                {
                    if (nextMovementTimer <= diff)
                    {
                        me->GetMotionMaster()->MovePoint(prevPosition == 2 ? 1: 2, me->GetPositionX(), quilenNewY[prevPosition == 2 ? 0: 1], me->GetPositionZ());
                        nextMovementTimer = 0;
                    }
                    else
                        nextMovementTimer -= diff;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MONSTROUS_BITE:
                        {
                            if (Unit* target = me->SelectNearestTarget(5.0f))
                            {
                                if (!target->IsFriendlyTo(me))
                                    me->CastSpell(target, SPELL_MONSTROUS_BITE, true);
                            }
                            events.RescheduleEvent(EVENT_MONSTROUS_BITE, urand(6000, 8000));
                            break;
                        }
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_enormous_stone_quilenAI(creature);
        }
};

enum eQuilenEvents
{
    EVENT_SUNDERING_BITE    = 1,
    EVENT_SHATTERING_STONE  = 2,
    EVENT_fOCUSED_ASSAULT   = 3
};

class mob_stone_quilen : public CreatureScript
{
    public:
        mob_stone_quilen() : CreatureScript("mob_stone_quilen") {}

        struct mob_stone_quilenAI : public ScriptedAI
        {
            mob_stone_quilenAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset() override
            {
                events.Reset();

                events.RescheduleEvent(EVENT_SUNDERING_BITE,   urand (5000, 6000));
                events.RescheduleEvent(EVENT_SHATTERING_STONE, urand (10000, 12000));
                events.RescheduleEvent(EVENT_fOCUSED_ASSAULT,  urand (500, 5000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (!me->HasAura(SPELL_PROTECTIVE_FENZY) && me->HealthBelowPct(10))
                    me->CastSpell(me, SPELL_PROTECTIVE_FENZY, true);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUNDERING_BITE:
                        {
                            if (Unit* target = me->SelectNearestTarget(5.0f))
                                me->CastSpell(target, SPELL_SUNDERING_BITE, true);

                            events.RescheduleEvent(EVENT_SUNDERING_BITE,   urand (5000, 6000));
                            break;
                        }
                        case EVENT_SHATTERING_STONE:
                        {
                            if (Unit* target = me->SelectNearestTarget(5.0f))
                                me->CastSpell(target, SPELL_SHATTERING_STONE, true);

                            events.RescheduleEvent(EVENT_SHATTERING_STONE, urand (10000, 12000));
                            break;
                        }
                        case EVENT_fOCUSED_ASSAULT:
                        {
                            if (Unit* target = me->SelectNearestTarget(5.0f))
                                me->AddAura(SPELL_FOCUSED_ASSAULT, target);

                            events.RescheduleEvent(EVENT_fOCUSED_ASSAULT,  urand (500, 5000));
                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_stone_quilenAI(creature);
        }
};
enum eSkullCharger
{
    SPELL_TROLL_RUSH    = 116606,
    EVENT_TROLL_RUSH    = 1,
};

class mob_zandalari_skullcharger : public CreatureScript
{
    public:
        mob_zandalari_skullcharger() : CreatureScript("mob_zandalari_skullcharger") {}

        struct mob_zandalari_skullchargerAI : public ScriptedAI
        {
            mob_zandalari_skullchargerAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset() override
            {
                events.Reset();

                events.RescheduleEvent(EVENT_TROLL_RUSH, urand (5000, 6000));
            }

            void JustReachedHome() override
            {

            }

            void MovementInform(uint32 typeId, uint32 pointId) override
            {

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
                        case EVENT_TROLL_RUSH:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST))
                            {
                                me->CastSpell(target, SPELL_TROLL_RUSH, true);
                                me->GetMotionMaster()->MoveChase(target);
                            }
                            events.RescheduleEvent(EVENT_TROLL_RUSH,   urand (5000, 6000));
                            break;
                        }

                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_zandalari_skullchargerAI(creature);
        }
};

class spell_mogu_petrification : public SpellScriptLoader
{
    public:
        spell_mogu_petrification() : SpellScriptLoader("spell_mogu_petrification") { }

        class spell_mogu_petrification_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mogu_petrification_AuraScript);

            uint32 stack;

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetTarget())
                    {
                        if (target->HasAura(SPELL_PETRIFIED))
                        {
                            stack = GetTarget()->GetAura(SPELL_PETRIFIED)->GetStackAmount();

                            if (stack >= 100)
                                target->AddAura(SPELL_FULLY_PETRIFIED, target);
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_mogu_petrification_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAPPLY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mogu_petrification_AuraScript();
        }
};

void AddSC_mogu_shan_vault()
{
    new mob_cursed_mogu_sculpture();
    new mob_enormous_stone_quilen();
    new mob_stone_quilen();
    new mob_zandalari_skullcharger();
    new spell_mogu_petrification();
}