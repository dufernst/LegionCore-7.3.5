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

#include "scarlet_monastery.h"

enum Says
{
    SAY_AGGRO             = 0,
    SAY_CRUSADER          = 1,
    SAY_EVICT_SOUL        = 2,
    SAY_WARN_SOUL         = 3,
    SAY_WARN_EMPOWERING   = 4,
    SAY_EMPOWERING        = 5,
    SAY_DEATH             = 6,
};

enum Spells
{
    SPELL_SPIRIT_GALE               = 115289,
    SPELL_RAISE_FALLEN_CRUSADER     = 115139,
    SPELL_EVICT_SOUL                = 115297,
    SPELL_SUMMON_EMPOWERING_SPIRITS = 115147,
    
    //Trash
    SPELL_EVICT_SOUL_VISUAL         = 115304,
    SPELL_EVICT_SOUL_FOCUS_TARGET   = 115309,
    SPELL_SOUL_REND                 = 115306,
    
    SPELL_EMPOWERING_SPIRIT         = 115157,
    SPELL_EMPOWER_ZOMBIE_RIDE       = 115239,
    SPELL_EMPOWER_ZOMBIE_VISUAL     = 115249,
    SPELL_ZOMBIE_TRANSFORM          = 115258,

    SPELL_MIND_ROT                  = 115143,
    SPELL_PERMANENT_FEIGN_DEATH     = 115163,
};

enum eEvents
{
    EVENT_SPIRIT_GALE       = 1,
    EVENT_FALLEN_CRUSADER   = 2,
    EVENT_EVICT_SOUL        = 3,
    EVENT_SUMMON_SPIRITS    = 4,
};

//59789
class boss_thalnos_the_soulrender : public CreatureScript
{
public:
    boss_thalnos_the_soulrender() : CreatureScript("boss_thalnos_the_soulrender") {}

    struct boss_thalnos_the_soulrenderAI : public BossAI
    {
        boss_thalnos_the_soulrenderAI(Creature* creature) : BossAI(creature, DATA_THALNOS) {}

        uint8 EmpZombi = 0;

        void Reset()
        {
            _Reset();
            EmpZombi = 0;
        }

        void EnterCombat(Unit* /*who*/) 
        {
            Talk(SAY_AGGRO);
            _EnterCombat();
            events.RescheduleEvent(EVENT_SPIRIT_GALE, 0);
            events.RescheduleEvent(EVENT_FALLEN_CRUSADER, 8000);
            events.RescheduleEvent(EVENT_EVICT_SOUL, 25000);
            events.RescheduleEvent(EVENT_SUMMON_SPIRITS, 46000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            DoZoneInCombat(summon, 60.0f);

            if (summon->GetEntry() == NPC_EVICTED_SOUL)
            {
                if (Unit* pTarget = summon->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                {
                    summon->CastSpell(summon, SPELL_SOUL_REND, true);
                    summon->AddThreat(pTarget, 150.0f);
                    summon->AI()->AttackStart(pTarget);
                    pTarget->CastSpell(summon, SPELL_EVICT_SOUL_VISUAL, true);
                    summon->CastSpell(pTarget, SPELL_EVICT_SOUL_FOCUS_TARGET, true);
                }
            }
        }

        void JustDied(Unit* /*Killer*/)
        {
            Talk(SAY_DEATH);
            _JustDied();

            if (Creature* zombie = instance->instance->GetCreature(instance->GetGuidData(NPC_EMPOWERED_ZOMBIE)))
                zombie->DespawnOrUnsummon();
        }

        bool AllowAchieve()
        {
            return EmpZombi >= 3;
        }

        void DoAction(const int32 action)
        {
            if (action == 1)
                EmpZombi++;

            if (action == 2)
                EmpZombi--;
        }

        void UpdateAI(uint32 diff)
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
                    case EVENT_SPIRIT_GALE:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_SPIRIT_GALE);
                        events.RescheduleEvent(EVENT_SPIRIT_GALE, 8000);
                        break;
                    case EVENT_FALLEN_CRUSADER:
                        Talk(SAY_CRUSADER);
                        DoCast(SPELL_RAISE_FALLEN_CRUSADER);
                        events.RescheduleEvent(EVENT_FALLEN_CRUSADER, 60000);
                        break;
                    case EVENT_EVICT_SOUL:
                        Talk(SAY_EVICT_SOUL);
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        {
                            Talk(SAY_WARN_SOUL, pTarget->GetGUID());
                            DoCast(pTarget, SPELL_EVICT_SOUL);
                        }
                        events.RescheduleEvent(EVENT_EVICT_SOUL, 41000);
                        break;
                    case EVENT_SUMMON_SPIRITS:
                        Talk(SAY_WARN_EMPOWERING);
                        DoCast(SPELL_SUMMON_EMPOWERING_SPIRITS);
                        Talk(SAY_EMPOWERING);
                        events.RescheduleEvent(EVENT_SUMMON_SPIRITS, 60000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_thalnos_the_soulrenderAI (creature);
    }
};

//59893
class npc_empowering_spirit : public CreatureScript
{
public:
    npc_empowering_spirit() : CreatureScript("npc_empowering_spirit") {}

    struct npc_empowering_spiritAI : public ScriptedAI
    {
        npc_empowering_spiritAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
            me->SetSpeed(MOVE_RUN, 0.6f);
            me->SetSpeed(MOVE_WALK, 0.6f);
        }

        bool RideVehicle = false;
        uint32 moveTimer = 0;

        void Reset() {}

        void IsSummonedBy(Unit* owner)
        {
            DoCast(SPELL_EMPOWERING_SPIRIT);
            moveTimer = 500;
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_EMPOWERING_SPIRIT)
            {
                me->AddThreat(target, 15000.0f);
                AttackStart(target);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || RideVehicle)
                return;

            if (moveTimer)
            {
                if (moveTimer <= diff)
                {
                    if (auto target = me->getVictim())
                        if (target->IsCreature() && target->HasAura(SPELL_EMPOWERING_SPIRIT))
                        {
                            if (me->GetDistance(target) <= 5.0f)
                            {
                                moveTimer = 0;
                                RideVehicle = true;
                                me->StopAttack();
                                DoCast(target, SPELL_EMPOWER_ZOMBIE_RIDE, true);
                                target->CastSpell(target, SPELL_ZOMBIE_TRANSFORM, true);
                                me->DespawnOrUnsummon(500);
                            }
                            else
                            {
                                moveTimer = 500;
                                me->GetMotionMaster()->MoveChase(target);
                            }
                        }
                }
                else
                    moveTimer -= diff;
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_empowering_spiritAI(creature);
    }
};

//59884
class npc_fallen_crusader : public CreatureScript
{
public:
    npc_fallen_crusader() : CreatureScript("npc_fallen_crusader") {}

    struct npc_fallen_crusaderAI : public ScriptedAI
    {
        npc_fallen_crusaderAI(Creature* creature) : ScriptedAI(creature) {}

        bool feignDeath = false;

        void Reset() 
        {
            DoCast(SPELL_MIND_ROT);
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->DespawnOrUnsummon(1000);
            }
        }

        void DamageTaken(Unit* /*who*/, uint32& damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (!feignDeath)
                {
                    feignDeath = true;
                    me->AttackStop();
                    DoCast(me, SPELL_PERMANENT_FEIGN_DEATH, true);
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fallen_crusaderAI (creature);
    }
};

//59930
class npc_empowered_zombie : public CreatureScript
{
public:
    npc_empowered_zombie() : CreatureScript("npc_empowered_zombie") { }

    struct npc_empowered_zombieAI : public ScriptedAI
    {
        npc_empowered_zombieAI(Creature* creature) : ScriptedAI(creature) 
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 checkTimer = 1000;

        void Reset() {}

        void IsSummonedBy(Unit* owner)
        {
            DoZoneInCombat(me, 100.0f);

            if (auto thalnos = instance->instance->GetCreature(instance->GetGuidData(DATA_THALNOS)))
                thalnos->AI()->DoAction(1);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (auto thalnos = instance->instance->GetCreature(instance->GetGuidData(DATA_THALNOS)))
                thalnos->AI()->DoAction(2);
        }

        void EnterEvadeMode()
        {
            ScriptedAI::EnterEvadeMode();
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (checkTimer)
            {
                if (checkTimer <= diff)
                {
                    if (instance->GetBossState(DATA_THALNOS) != IN_PROGRESS)
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    checkTimer = 1000;
                }
                else
                    checkTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_empowered_zombieAI (creature);
    }
};

typedef boss_thalnos_the_soulrender::boss_thalnos_the_soulrenderAI ThalnosAI;

class achievement_empowered_spiritualist : public AchievementCriteriaScript
{
public:
    achievement_empowered_spiritualist() : AchievementCriteriaScript("achievement_empowered_spiritualist") { }

    bool OnCheck(Player* source, Unit* target)
    {
        if (!target)
            return false;

        if (ThalnosAI* thalnosAI = CAST_AI(ThalnosAI, target->GetAI()))
            return thalnosAI->AllowAchieve();

        return false;
    }
};

void AddSC_boss_thalnos_the_soulrender()
{
    new boss_thalnos_the_soulrender();
    new npc_empowering_spirit();
    new npc_fallen_crusader();
    new npc_empowered_zombie();
    //new achievement_empowered_spiritualist();
}
