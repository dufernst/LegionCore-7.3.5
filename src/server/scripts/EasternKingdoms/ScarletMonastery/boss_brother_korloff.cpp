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
    SAY_AGGRO                   = 0,
    SAY_AGGRO_2                 = 1,
    SAY_EVADE                   = 2, // Evade?
    SAY_DEATH                   = 3,
};

enum Spells
{
    SPELL_RISING_FLAME          = 114410,
    SPELL_FLYING_KICK           = 114487,
    SPELL_BLAZING_FISTS         = 114807,
    SPELL_SCORCHED_EARTH        = 114460,
    SPELL_SCORCHED_EARTH_SUM    = 114463,

    //Achiev
    SPELL_BURNING_MAN_1         = 125852,
    SPELL_BURNING_MAN_2         = 125844,
};

enum Events
{
    EVENT_FLYING_KICK           = 1,
    EVENT_BLAZING_FISTS         = 2
};

const Position dummyPos[6] =
{
    {896.24f, 649.75f, 10.20f, 4.67f},
    {896.53f, 562.22f, 10.39f, 1.61f}, 
    {899.97f, 562.03f, 10.47f, 1.68f},
    {903.60f, 562.01f, 10.55f, 1.55f},
    {899.69f, 649.56f, 10.23f, 4.66f},
    {903.32f, 649.54f, 10.26f, 4.65f},
};

//59223
class boss_brother_korloff : public CreatureScript
{
public:
    boss_brother_korloff() : CreatureScript("boss_brother_korloff") {}

    struct boss_brother_korloffAI : public BossAI
    {
        boss_brother_korloffAI(Creature* creature) : BossAI(creature, DATA_KORLOFF)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        bool lowHp = false;
        uint8 dummieFire = 0; // achiev
        uint32 healthPct = 0;

        void Reset()
        {
            _Reset();
            lowHp = false;
            healthPct = 91;
            dummieFire = 0;
            me->RemoveAllAuras();

            for(uint8 i = 0; i < 6; i++)
                me->SummonCreature(NPC_TRAINING_DUMMY, dummyPos[i]);
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            events.RescheduleEvent(EVENT_FLYING_KICK, 10000);
            events.RescheduleEvent(EVENT_BLAZING_FISTS, 20000);
        }

        void EnterEvadeMode()
        {
            BossAI::EnterEvadeMode();
            Talk(SAY_EVADE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BURNING_MAN_2);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BURNING_MAN_2);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPct(healthPct))
            {
                healthPct = healthPct < 11 ? 0 : 10;
                DoCast(me, SPELL_RISING_FLAME, true);
            }

            if (damage >= me->GetHealth() && !lowHp)
            {
                lowHp = true;
                DoCast(SPELL_SCORCHED_EARTH);
            }
        }

        void DoAction(const int32 action)
        {
            ++dummieFire;
        }

        bool AllowAchieve()
        {
            return dummieFire >= 6;
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
                    case EVENT_FLYING_KICK:
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            DoCast(pTarget, SPELL_FLYING_KICK);
                        events.RescheduleEvent(EVENT_FLYING_KICK, 26000);
                        break;
                    case EVENT_BLAZING_FISTS:
                        if (Unit* pTarget = me->getVictim())
                            DoCast(pTarget, SPELL_BLAZING_FISTS);
                        events.RescheduleEvent(EVENT_BLAZING_FISTS, 30000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_brother_korloffAI (creature);
    }
};

//64446
class npc_korloff_training_dummy : public CreatureScript
{
public:
    npc_korloff_training_dummy() : CreatureScript("npc_korloff_training_dummy") {}

    struct npc_korloff_training_dummyAI : public Scripted_NoMovementAI
    {
        npc_korloff_training_dummyAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;
        bool burn = false;

        void SpellHit(Unit* attacker, const SpellInfo* spell)
        {
            if ((spell->Id == SPELL_BURNING_MAN_1 || spell->Id == SPELL_BURNING_MAN_2) && !burn)
            {
                burn = true;

                if (attacker->IsCreature())
                    if (attacker->ToCreature()->GetEntry() == NPC_BROTHER_KORLOFF)
                        attacker->ToCreature()->AI()->DoAction(true);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korloff_training_dummyAI (creature);
    }
};

//114460
class spell_korloff_scorched_earth : public AuraScript
{
    PrepareAuraScript(spell_korloff_scorched_earth);

    void OnTick(AuraEffect const* aurEff)
    {
        if (GetCaster()->isMoving())
            GetCaster()->CastSpell(GetCaster(), SPELL_SCORCHED_EARTH_SUM, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_korloff_scorched_earth::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

typedef boss_brother_korloff::boss_brother_korloffAI KorloffAI;

class achievement_burning_man : public AchievementCriteriaScript
{
public:
    achievement_burning_man() : AchievementCriteriaScript("achievement_burning_man") { }

    bool OnCheck(Player* source, Unit* target)
    {
        if (!target)
            return false;

        if (KorloffAI* korloffAI = CAST_AI(KorloffAI, target->GetAI()))
            return korloffAI->AllowAchieve();

        return false;
    }
};

void AddSC_boss_brother_korloff()
{
    new boss_brother_korloff();
    new npc_korloff_training_dummy();
    RegisterAuraScript(spell_korloff_scorched_earth);
    //new achievement_burning_man();
}
