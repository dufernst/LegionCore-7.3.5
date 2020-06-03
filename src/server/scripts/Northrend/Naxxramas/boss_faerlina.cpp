/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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

#include "naxxramas.h"

enum Yells
{
    SAY_GREET       = -1533009,
    SAY_AGGRO_1     = -1533010,
    SAY_AGGRO_2     = -1533011,
    SAY_AGGRO_3     = -1533012,
    SAY_AGGRO_4     = -1533013,
    SAY_SLAY_1      = -1533014,
    SAY_SLAY_2      = -1533015,
    SAY_DEATH       = -1533016
};

enum Spells
{
    SPELL_POISON_BOLT_VOLLEY    = 28796,
    H_SPELL_POISON_BOLT_VOLLEY  = 54098,
    SPELL_RAIN_OF_FIRE          = 28794,
    H_SPELL_RAIN_OF_FIRE        = 54099,
    SPELL_FRENZY                = 28798,
    H_SPELL_FRENZY              = 54100,
    SPELL_WIDOWS_EMBRACE        = 28732,
    H_SPELL_WIDOWS_EMBRACE      = 54097,

    // add spell
    SPELL_FIREBALL              = 54095,
    H_SPELL_FIREBALL            = 54096
};

#define SPELL_WIDOWS_EMBRACE_HELPER RAID_MODE(SPELL_WIDOWS_EMBRACE, H_SPELL_WIDOWS_EMBRACE)
#define SPELL_FIREBALL_HELPER RAID_MODE(SPELL_FIREBALL, H_SPELL_FIREBALL)

enum Events
{
    EVENT_POISON    = 1,
    EVENT_FIRE      = 2,
    EVENT_FRENZY    = 3,
    // adds
    EVENT_FIREBALL  = 1
};

#define DATA_FRENZY_DISPELS 1

class boss_faerlina : public CreatureScript
{
    public:
        boss_faerlina() : CreatureScript("boss_faerlina") { }

        struct boss_faerlinaAI : public BossAI
        {
            boss_faerlinaAI(Creature* creature) : BossAI(creature, BOSS_FAERLINA),
                _frenzyDispels(0), _introDone(false), _delayFrenzy(false)
            {}


            void EnterCombat(Unit* /*who*/) override
            {
                _EnterCombat();
                DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3, SAY_AGGRO_4), me);
                events.ScheduleEvent(EVENT_POISON, urand(10000, 15000));
                events.ScheduleEvent(EVENT_FIRE, urand(6000, 18000));
                events.ScheduleEvent(EVENT_FRENZY, urand(60000, 80000));
            }

            void Reset() override
            {
                _Reset();
                _delayFrenzy = false;
                _frenzyDispels = 0;
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (!_introDone && who->GetTypeId() == TYPEID_PLAYER)
                {
                    DoScriptText(SAY_GREET, me);
                    _introDone = true;
                }

                BossAI::MoveInLineOfSight(who);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                if (!urand(0, 2))
                    DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                DoScriptText(SAY_DEATH, me);
            }

            void SpellHit(Unit* caster, const SpellInfo* spell) override
            {
                if (spell->Id == SPELL_WIDOWS_EMBRACE || spell->Id == H_SPELL_WIDOWS_EMBRACE)
                {
                    // TODO : Add Text
                    ++_frenzyDispels;
                    _delayFrenzy = true;
                    me->Kill(caster);
                }
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_FRENZY_DISPELS)
                    return _frenzyDispels;

                return 0;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->IsNonMeleeSpellCast(false))
                    return;

                if (_delayFrenzy && !me->HasAura(SPELL_WIDOWS_EMBRACE_HELPER))
                {
                    _delayFrenzy = false;
                    DoCast(me, RAID_MODE(SPELL_FRENZY, H_SPELL_FRENZY), true);
                }

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_POISON:
                            if (!me->HasAura(SPELL_WIDOWS_EMBRACE_HELPER))
                                DoCastAOE(RAID_MODE(SPELL_POISON_BOLT_VOLLEY, H_SPELL_POISON_BOLT_VOLLEY));
                            events.ScheduleEvent(EVENT_POISON, urand(8000, 15000));
                            break;
                        case EVENT_FIRE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, RAID_MODE(SPELL_RAIN_OF_FIRE, H_SPELL_RAIN_OF_FIRE));
                            events.ScheduleEvent(EVENT_FIRE, urand(6000, 18000));
                            break;
                        case EVENT_FRENZY:
                            // TODO : Add Text
                            if (!me->HasAura(SPELL_WIDOWS_EMBRACE_HELPER))
                                DoCast(me, RAID_MODE(SPELL_FRENZY, H_SPELL_FRENZY));
                            else
                                _delayFrenzy = true;

                            events.ScheduleEvent(EVENT_FRENZY, urand(60000, 80000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            uint32 _frenzyDispels;
            bool _introDone;
            bool _delayFrenzy;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_faerlinaAI(creature);
        }
};

class mob_faerlina_add : public CreatureScript
{
    public:
        mob_faerlina_add() : CreatureScript("mob_faerlina_add") { }

        struct mob_faerlina_addAI : public ScriptedAI
        {
            mob_faerlina_addAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_BIND, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_FIREBALL, 3000);
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (instance)
                    if (Creature* faerlina = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FAERLINA)))
                        DoCast(faerlina, SPELL_WIDOWS_EMBRACE);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_FIREBALL:
                        DoCast(SPELL_FIREBALL_HELPER);
                        events.ScheduleEvent(EVENT_FIREBALL, urand (3000, 5000));
                        break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_faerlina_addAI(creature);
        }
};

class achievement_momma_said_knock_you_out : public AchievementCriteriaScript
{
    public:
        achievement_momma_said_knock_you_out() : AchievementCriteriaScript("achievement_momma_said_knock_you_out") { }

        bool OnCheck(Player* /*source*/, Unit* target) override
        {
            return target && !target->GetAI()->GetData(DATA_FRENZY_DISPELS);
        }
};

void AddSC_boss_faerlina()
{
    new boss_faerlina();
    new mob_faerlina_add();
    new achievement_momma_said_knock_you_out();
}
