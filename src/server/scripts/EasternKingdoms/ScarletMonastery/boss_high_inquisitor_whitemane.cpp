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
    //Whitemane says
    SAY_WH_AGGRO                 = 0,
    SAY_WH_WARN_RESSURECT        = 1,
    SAY_WH_RESSURECT             = 2,
    SAY_WH_DEATH                 = 3,
    
    //Durand says
    SAY_D_INTRO         = 0,
    SAY_D_AGGRO         = 1,
    SAY_D_DEATH         = 2,
};

enum Spells
{
    //Whitemanes Spells
    SPELL_HOLYSMITE              = 114848,
    SPELL_POWERWORDSHIELD        = 127399,
    SPELL_DEEPSLEEP              = 9256,
    SPELL_SCARLET_RESURRECTION   = 9232,
    SPELL_MASS_RESURRECTION      = 113134,
    SPELL_DOMINATE_MIND          = 130857,

    SPELL_ACHIEV_CREDIT          = 132022,
    
    //Durand
    SPELL_FLASH_OF_STEEL         = 115629,
    SPELL_FEIGN_DEATH            = 29266,
    SPELL_FURIOUS_RESOLVE        = 115876,
    SPELL_SUICIDE_NO_BLOOD       = 42278,
};

enum Events
{
    EVENT_HOLYSMITE             = 1,
    EVENT_POWERWORDSHIELD       = 2,
    EVENT_MASS_RESURRECTION     = 3,
    EVENT_DOMINATE_MIND         = 4, //Challenge mode only
    
    EVENT_FLASH_OF_STEEL        = 5,
};

const Position judPos[13] =
{
    {752.32f, 546.028f, 12.80f, 0.59f},
    {753.15f, 550.778f, 12.80f, 0.89f},
    {755.33f, 544.255f, 12.80f, 1.23f},
    {756.57f, 550.286f, 12.80f, 6.21f},
    {758.66f, 544.073f, 12.80f, 5.86f},
    {758.92f, 548.054f, 12.80f, 4.01f},
    {761.47f, 546.904f, 12.80f, 1.22f},
    {763.87f, 543.892f, 12.80f, 5.07f},
    {765.29f, 546.938f, 12.80f, 3.43f},
    {767.08f, 551.016f, 12.80f, 1.09f},
    {767.94f, 543.682f, 12.80f, 0.01f},
    {768.95f, 547.529f, 12.80f, 3.29f},
    {770.43f, 549.938f, 12.80f, 1.65f},
};

//3977
class boss_high_inquisitor_whitemane : public CreatureScript
{
public:
    boss_high_inquisitor_whitemane() : CreatureScript("boss_high_inquisitor_whitemane") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_high_inquisitor_whitemaneAI (creature);
    }

    struct boss_high_inquisitor_whitemaneAI : public BossAI
    {
        boss_high_inquisitor_whitemaneAI(Creature* creature) : BossAI(creature, DATA_WHITEMANE)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        
        uint8 JudCount = 0;
        bool Resurrect = false;
        bool MassRes = false;
        bool ResurrectCheck = false;

        void Reset() override
        {
            _Reset();
            JudCount = 0;
            Resurrect = false;
            MassRes = false;
            ResurrectCheck = false;
            me->SetReactState(REACT_DEFENSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            for (uint8 i = 0; i < 13; i++)
                me->SummonCreature(NPC_SCARLET_JUDICATOR, judPos[i]);

            me->SummonCreature(NPC_DURAND, 747.96f, 605.97f, 15.07f, -0.04f);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(SAY_WH_AGGRO);
            _EnterCombat();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            events.RescheduleEvent(EVENT_HOLYSMITE, 5000);
            events.RescheduleEvent(EVENT_POWERWORDSHIELD, 26000);
            events.RescheduleEvent(EVENT_MASS_RESURRECTION, 12000);
            if (GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                events.RescheduleEvent(EVENT_DOMINATE_MIND, 14000);
        }

        void EnterEvadeMode() override
        {
            BossAI::EnterEvadeMode();
            me->NearTeleportTo(me->GetHomePosition());
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if ((damage >= me->GetHealth() && !Resurrect) || ResurrectCheck)
            {
                damage = 0;
                me->SetHealth(me->CountPctFromMaxHealth(50));
            }

            if (me->HealthBelowPct(51) && !Resurrect)
            {
                Resurrect = true;
                ResurrectCheck = true;
                me->StopAttack(false, true);
                DoCast(me, SPELL_DEEPSLEEP, true);
                if (auto durand = instance->instance->GetCreature(instance->GetGuidData(DATA_DURAND)))
                {
                    me->SetWalk(true);
                    me->GetMotionMaster()->MovePoint(1, durand->GetPositionX(), durand->GetPositionY(), durand->GetPositionZ());
                }
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
                if (id == 1)
                {
                    DoCast(SPELL_SCARLET_RESURRECTION);
                    Talk(SAY_WH_RESSURECT);
                    ResurrectCheck = false;
                    me->SetReactState(REACT_AGGRESSIVE, 2500);
                }
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            Talk(SAY_WH_DEATH);
            instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_ACHIEV_CREDIT, 0, 0, me);

            if (GetDifficultyID() != DIFFICULTY_MYTHIC_KEYSTONE)
                if (auto durand = instance->instance->GetCreature(instance->GetGuidData(DATA_DURAND)))
                    durand->AI()->DoCast(SPELL_SUICIDE_NO_BLOOD);
        }

        void DoAction(const int32 action) override
        {
            if (action == ACTION_1)
                me->SetInCombatWithZone();

            if (action == ACTION_2)
                JudCount++;
        }

        void SpellFinishCast(const SpellInfo* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_MASS_RESURRECTION:
                    MassRes = true;
                    break;
            }
        }

        bool AllowAchieve()
        {
            return JudCount >= 13;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || ResurrectCheck)
                return;

            if (CheckHomeDistToEvade(diff, 70.0f))
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HOLYSMITE:
                        if (auto pTarget = me->getVictim())
                            DoCast(pTarget, SPELL_HOLYSMITE);
                        events.RescheduleEvent(EVENT_HOLYSMITE, 1000);
                        break;
                    case EVENT_POWERWORDSHIELD:
                        DoCast(me, SPELL_POWERWORDSHIELD, true);
                        events.RescheduleEvent(EVENT_POWERWORDSHIELD, 15000);
                        break;
                    case EVENT_MASS_RESURRECTION:
                        if (!MassRes)
                        {
                            Talk(SAY_WH_WARN_RESSURECT);
                            DoCast(SPELL_MASS_RESURRECTION);
                            events.RescheduleEvent(EVENT_MASS_RESURRECTION, 22000);
                        }
                        break;
                    case EVENT_DOMINATE_MIND:
                        if (auto pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                            DoCast(pTarget, SPELL_DOMINATE_MIND);
                        events.RescheduleEvent(EVENT_DOMINATE_MIND, 18000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

//60040
class npc_commander_durand : public CreatureScript
{
public:
    npc_commander_durand() : CreatureScript("npc_commander_durand") {}

    struct npc_commander_durandAI : public ScriptedAI
    {
        npc_commander_durandAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        bool intro = true;
        bool fakeDeath = false;
        bool sayDeath = false;

        void Reset() override
        {
            fakeDeath = false;
            sayDeath = false;
            me->RemoveAllAuras();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(SAY_D_AGGRO);
            events.RescheduleEvent(EVENT_FLASH_OF_STEEL, 10000);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (!fakeDeath)
                {
                    fakeDeath = true;
                    me->StopAttack();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    DoCast(SPELL_FEIGN_DEATH);
                    if (me->isAlive())
                        me->RemoveAurasAllDots();

                    if (!sayDeath)
                    {
                        sayDeath = true;
                        Talk(SAY_D_DEATH);

                        if (auto Wh = instance->instance->GetCreature(instance->GetGuidData(DATA_WHITEMANE)))
                            Wh->AI()->DoAction(ACTION_1);
                    }
                }
            }
        }

        void DoAction(const int32 action) override
        {
            if (intro)
            {
                intro = false;
                Talk(SAY_D_INTRO);
            }
        }

        void SpellHit(Unit* attacker, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_SCARLET_RESURRECTION)
            {
                me->RemoveAura(SPELL_FEIGN_DEATH);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetHealth(me->GetMaxHealth());
                attacker->CastSpell(attacker, SPELL_FURIOUS_RESOLVE, true);
                me->SetReactState(REACT_AGGRESSIVE);
                fakeDeath = false;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || fakeDeath)
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FLASH_OF_STEEL:
                        if (auto pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                            DoCast(pTarget, SPELL_FLASH_OF_STEEL);
                        events.RescheduleEvent(EVENT_FLASH_OF_STEEL, 26000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_commander_durandAI (creature);
    }
};

//58605
class npc_scarlet_judicator : public CreatureScript
{
public:
    npc_scarlet_judicator() : CreatureScript("npc_scarlet_judicator") {}

    struct npc_scarlet_judicatorAI : public ScriptedAI
    {
        npc_scarlet_judicatorAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override
        {
            DoCast(SPELL_FEIGN_DEATH);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void SpellHit(Unit* attacker, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_MASS_RESURRECTION)
            {
                me->RemoveAura(SPELL_FEIGN_DEATH);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 120.0f);
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (auto Wh = me->GetAnyOwner())
                Wh->GetAI()->DoAction(ACTION_2);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_judicatorAI (creature);
    }
};

//7496
class at_enter_durand_room : public AreaTriggerScript
{
public:
    at_enter_durand_room() : AreaTriggerScript("at_enter_durand_room") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (pPlayer->isGameMaster())
            return false;
        
        if (InstanceScript* pInstance = pPlayer->GetInstanceScript())
        {
            if (auto durand = pInstance->instance->GetCreature(pInstance->GetGuidData(DATA_DURAND)))
                durand->AI()->DoAction(true);
        }
        return true;
    }
};

typedef boss_high_inquisitor_whitemane::boss_high_inquisitor_whitemaneAI WhitemaneAI;

class achievement_and_stay_dead : public AchievementCriteriaScript
{
public:
    achievement_and_stay_dead() : AchievementCriteriaScript("achievement_and_stay_dead") { }

    bool OnCheck(Player* source, Unit* target)
    {
        if (!target)
            return false;

        if (WhitemaneAI* whitemaneAI = CAST_AI(WhitemaneAI, target->GetAI()))
            return whitemaneAI->AllowAchieve();

        return false;
    }
};

void AddSC_boss_high_inquisitor_whitemane()
{
    new boss_high_inquisitor_whitemane();
    new npc_commander_durand();
    new npc_scarlet_judicator();
    new at_enter_durand_room();
    //new achievement_and_stay_dead();
}
