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
#include "scarlet_halls.h"

enum Says
{
    SAY_AGGRO_1         = 0,
    SAY_AGGRO_2         = 1,
    SAY_DOG_DIED        = 3,
    SAY_DOG_HELP        = 4,
    SAY_PHASE_2         = 8,
    SAY_LOW_HP          = 9,
    SAY_DOG_ATTACK_ME   = 10,
    SAY_DEATH           = 11,
    SAY_EVADE           = 12,
    
    SAY_DOG_AGGRO   = 0,
};

enum Spells
{
    SPELL_CALL_DOG_1        = 114259,
    SPELL_CALL_DOG_2        = 114390,
    SPELL_CALL_DOG_3        = 114260,
    SPELL_BLOODY_RAGE       = 116140,
    SPELL_PIERCING_THROW    = 114004,
    SPELL_DEATH_BLOSSOM     = 114241,
    //Dogs
    SPELL_HUNGRY_DOG        = 111578,
    SPELL_DOG_CLEAVE        = 116222,
};

enum Achieve
{
    ACHIEVECOMPLETED,
};

enum eEvents
{
    EVENT_AGGRO             = 1,
    EVENT_DOG_HELP          = 2,
    EVENT_PIERCING_THROW    = 3,
    EVENT_DEATH_BLOSSOM     = 4,
    EVENT_FINISH            = 5,
    EVENT_DOGS_ATTACK       = 6,
    EVENT_DOGS_RAGE         = 7,
    EVENT_DIE               = 8,
};

const Position DogPos[12] = 
{
    {1027.34f, 508.01f, 13.48f, 3.06f}, //Reset Pos
    {1026.08f, 509.57f, 13.48f, 3.20f},
    {1006.87f, 495.92f, 13.48f, 1.43f},
    {1003.86f, 495.85f, 13.48f, 1.41f},
    {1005.37f, 497.18f, 13.48f, 1.48f},
    {1027.41f, 511.02f, 13.48f, 3.03f},
    {1015.11f, 531.17f, 13.48f, 4.68f}, //On Aggro Pos
    {991.53f,  519.19f, 13.48f, 6.27f},
    {991.55f,  515.67f, 13.48f, 6.20f},
    {992.17f,  517.41f, 13.48f, 6.23f},
    {1011.64f, 531.27f, 13.48f, 4.68f},
    {1013.34f, 530.67f, 13.48f, 4.74f},
};

const Position dogsPoint[1] =
{
    {1053.22f, 509.70f, 6.20f},
};

class boss_houndmaster_braun : public CreatureScript
{
public:
    boss_houndmaster_braun() : CreatureScript("boss_houndmaster_braun") { }

    struct boss_houndmaster_braunAI : public BossAI
    {
        boss_houndmaster_braunAI(Creature* creature) : BossAI(creature, DATA_BRAUN), summons(me)
        {
            instance = me->GetInstanceScript();
            dognotkilled = true;
        }

        InstanceScript* instance;
        SummonList summons;
        ObjectGuid dogGUID[4];
        uint32 dogcount;
        uint32 health;
        uint8 dogId;

        bool DogEvents;
        bool LowHpSay;
        bool DeathEvent;
        bool dognotkilled;

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
            me->SetReactState(REACT_AGGRESSIVE);
            dogId = 0;
            dogcount = 0;
            health = 90;
            DogEvents = true;
            LowHpSay = false;
            DeathEvent = false;
            dognotkilled = true;

            for (uint8 i = 0; i < 6; i++)
                me->SummonCreature(NPC_OBEDIENT_HOUND, DogPos[i]);
            _Reset();
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
            case ACHIEVECOMPLETED:
                return dognotkilled;
            }
            return 0;
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            dogcount++;

            switch (dogcount)
            {
                case 2:
                    dogGUID[0] = summon->GetGUID();
                    break;
                case 5:
                    dogGUID[1] = summon->GetGUID();
                    break;
                case 10:
                    dogGUID[2] = summon->GetGUID();
                    break;
                case 12:
                    dogGUID[3] = summon->GetGUID();
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(SAY_AGGRO_1);
            events.RescheduleEvent(EVENT_AGGRO, 5000);
            events.RescheduleEvent(EVENT_PIERCING_THROW, 12000);
            events.RescheduleEvent(EVENT_DEATH_BLOSSOM, 18000);
            _EnterCombat();
        }

        void EnterEvadeMode()
        {
            Talk(SAY_EVADE);
            BossAI::EnterEvadeMode();
        }

        void JustDied(Unit* /*killer*/)
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/)
        {
            Talk(SAY_DOG_DIED);
            if (me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC)
                dognotkilled = false;
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
            {
                if (!attacker->ToCreature())
                    if (attacker->ToPet() || attacker->ToPlayer())
                        damage = 0;

                if (!DeathEvent)
                {
                    DeathEvent = true;
                    events.RescheduleEvent(EVENT_FINISH, 0);
                }
            }

            if (HealthBelowPct(health) && DogEvents)
            {
                health -= 10;
                Talk(SAY_DOG_HELP);

                switch (health)
                {
                    case 70:
                        dogId = 1;
                        break;
                    case 60:
                        dogId = 2;
                        break;
                    case 50:
                        dogId = 3;
                        break;
                }
                if (Creature* dog = me->GetCreature(*me, dogGUID[dogId]))
                {
                    dog->AI()->Talk(SAY_DOG_AGGRO);
                    dog->AI()->DoAction(2);
                }
            }

            if (me->HealthBelowPct(50) && DogEvents)
            {
                DogEvents = false;
                events.CancelEvent(EVENT_DOG_HELP);
                Talk(SAY_PHASE_2);
                DoCast(SPELL_BLOODY_RAGE);
            }

            if (me->HealthBelowPct(10) && !LowHpSay)
            {
                LowHpSay = true;
                Talk(SAY_LOW_HP);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->GetDistance(me->GetHomePosition()) > 20.0f)
            {
                EnterEvadeMode();
                return;
            }

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_AGGRO:
                        Talk(SAY_AGGRO_2);
                        for (uint8 i = 6; i < 12; i++)
                            me->SummonCreature(NPC_OBEDIENT_HOUND, DogPos[i]);
                        break;
                    case EVENT_DIE:
                        me->Kill(me);
                        break;
                    case EVENT_PIERCING_THROW:
                        DoCast((SelectTarget(SELECT_TARGET_RANDOM, 0, 50, true)), SPELL_PIERCING_THROW);
                        events.RescheduleEvent(EVENT_PIERCING_THROW, 12000);
                        break;
                    case EVENT_DEATH_BLOSSOM:
                        DoCast((SelectTarget(SELECT_TARGET_RANDOM, 0, 50, true)), SPELL_DEATH_BLOSSOM);
                        events.RescheduleEvent(EVENT_DEATH_BLOSSOM, 18000);
                        break;
                    case EVENT_FINISH:
                        events.CancelEvent(EVENT_PIERCING_THROW);
                        events.CancelEvent(EVENT_DEATH_BLOSSOM);
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->MoveTargetedHome();
                        Talk(SAY_DOG_ATTACK_ME);
                        EntryCheckPredicate pred(NPC_OBEDIENT_HOUND);
                        summons.DoAction(1, pred);
                        events.RescheduleEvent(EVENT_DIE, 5000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_houndmaster_braunAI (creature);
    }
};

class npc_obedient_hound : public CreatureScript
{
public:
    npc_obedient_hound() : CreatureScript("npc_obedient_hound") { }

    struct npc_obedient_houndAI : public ScriptedAI
    {
        npc_obedient_houndAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;
        EventMap events;

        bool aggro;

        void Reset()
        {
            aggro = false;
            me->SetHomePosition(dogsPoint[0]);
        }

        void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth() && !aggro)
                damage = 0;
        }

        void DoAction(const int32 action)
        {
            if (action == 1)
                events.RescheduleEvent(EVENT_DOGS_RAGE, 1000);
            
            if (action == 2)
            {
                aggro = true;
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 60.0f);
                events.RescheduleEvent(EVENT_DOGS_ATTACK, 0);
            }
        }

        void JustReachedHome()
        {
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DOGS_ATTACK:
                        DoCast((SelectTarget(SELECT_TARGET_RANDOM, 0, 50, true)), SPELL_HUNGRY_DOG, true);
                        break;
                    case EVENT_DOGS_RAGE:
                        DoCast(SPELL_DOG_CLEAVE);
                        me->GetMotionMaster()->MovePoint(1, dogsPoint[0]);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->DespawnOrUnsummon(20000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_obedient_houndAI (creature);
    }
};

class npc_braun_scarlet_guardian : public CreatureScript
{
public:
    npc_braun_scarlet_guardian() : CreatureScript("npc_braun_scarlet_guardian") { }

    struct npc_braun_scarlet_guardianAI : public ScriptedAI
    {
        npc_braun_scarlet_guardianAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            SetCombatMovement(false);
        }

        InstanceScript* instance;

        void Reset()
        {
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
        return new npc_braun_scarlet_guardianAI (creature);
    }
};

// 20983 criteria
class achievement_humane_society : public AchievementCriteriaScript
{
public:
    achievement_humane_society() : AchievementCriteriaScript("achievement_humane_society") { }

    bool OnCheck(Player* /*player*/, Unit* target) override
    {
        if (!target)
            return false;

        if (Creature* braun = target->ToCreature())
            if (braun->AI()->GetData(ACHIEVECOMPLETED))
                return true;

        return false;
    }
};

void AddSC_boss_houndmaster_braun()
{
    new boss_houndmaster_braun();
    new npc_obedient_hound();
    new npc_braun_scarlet_guardian();
    new achievement_humane_society();
}
