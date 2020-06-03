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

/* ScriptData
SDName: Boss_Exarch_Maladaar
SD%Complete: 95
SDComment: Most of event implemented, some adjustments to timers remain and possibly make some better code for switching his dark side in to better "images" of player.
SDCategory: Auchindoun, Auchenai Crypts
EndScriptData */

/* ContentData
mob_stolen_soul
boss_exarch_maladaar
mob_avatar_of_martyred
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

#define SPELL_MOONFIRE          37328
#define SPELL_FIREBALL          37329
#define SPELL_MIND_FLAY         37330
#define SPELL_HEMORRHAGE        37331
#define SPELL_FROSTSHOCK        37332
#define SPELL_CURSE_OF_AGONY    37334
#define SPELL_MORTAL_STRIKE     37335
#define SPELL_FREEZING_TRAP     37368
#define SPELL_HAMMER_OF_JUSTICE 37369

class mob_stolen_soul : public CreatureScript
{
public:
    mob_stolen_soul() : CreatureScript("mob_stolen_soul") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_stolen_soulAI (creature);
    }

    struct mob_stolen_soulAI : public ScriptedAI
    {
        mob_stolen_soulAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 myClass;
        uint32 Class_Timer;

        void Reset() override
        {
            Class_Timer = 1000;
        }

        void EnterCombat(Unit* /*who*/) override {}

        void SetMyClass(uint8 myclass)
        {
            myClass = myclass;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (Class_Timer <= diff)
            {
                switch (myClass)
                {
                    case CLASS_WARRIOR:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_MORTAL_STRIKE, false);

                        Class_Timer = 6000;
                        break;
                    case CLASS_PALADIN:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_HAMMER_OF_JUSTICE, false);

                        Class_Timer = 6000;
                        break;
                    case CLASS_HUNTER:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_FREEZING_TRAP, false);

                        Class_Timer = 20000;
                        break;
                    case CLASS_ROGUE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_HEMORRHAGE, false);

                        Class_Timer = 10000;
                        break;
                    case CLASS_PRIEST:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_MIND_FLAY, false);

                        Class_Timer = 5000;
                        break;
                    case CLASS_SHAMAN:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_FROSTSHOCK, false);

                        Class_Timer = 8000;
                        break;
                    case CLASS_MAGE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_FIREBALL, false);

                        Class_Timer = 5000;
                        break;
                    case CLASS_WARLOCK:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_CURSE_OF_AGONY, false);

                        Class_Timer = 20000;
                        break;
                    case CLASS_DRUID:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_MOONFIRE, false);

                        Class_Timer = 10000;
                        break;
                }
            }
            else
                Class_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

enum Says
{
    SAY_INTRO = 0,
    SAY_SUMMON,
    SAY_AGGRO,
    SAY_ROAR,
    SAY_SOUL_CLEAVE,
    SAY_SLAY,
    SAY_DEATH
};

#define SPELL_RIBBON_OF_SOULS       32422
#define SPELL_SOUL_SCREAM           32421

#define SPELL_STOLEN_SOUL           32346
#define SPELL_STOLEN_SOUL_VISUAL    32395

#define SPELL_SUMMON_AVATAR         32424

#define ENTRY_STOLEN_SOUL           18441

class boss_exarch_maladaar : public CreatureScript
{
public:
    boss_exarch_maladaar() : CreatureScript("boss_exarch_maladaar") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_exarch_maladaarAI (creature);
    }

    struct boss_exarch_maladaarAI : public ScriptedAI
    {
        boss_exarch_maladaarAI(Creature* creature) : ScriptedAI(creature)
        {
            HasTaunted = false;
        }

        uint32 soulmodel;
        ObjectGuid soulholder;
        uint8 soulclass;

        uint32 Fear_timer;
        uint32 Ribbon_of_Souls_timer;
        uint32 StolenSoul_Timer;

        bool HasTaunted;
        bool Avatar_summoned;

        void Reset() override
        {
            soulmodel = 0;
            soulholder.Clear();
            soulclass = 0;

            Fear_timer = 15000 + rand()% 5000;
            Ribbon_of_Souls_timer = 5000;
            StolenSoul_Timer = 25000 + rand()% 10000;

            Avatar_summoned = false;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!HasTaunted && me->IsWithinDistInMap(who, 150.0f))
            {
                Talk(SAY_INTRO);
                HasTaunted = true;
            }

            ScriptedAI::MoveInLineOfSight(who);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(SAY_AGGRO);
        }

        void JustSummoned(Creature* summoned) override
        {
            if (summoned->GetEntry() == ENTRY_STOLEN_SOUL)
            {
                //SPELL_STOLEN_SOUL_VISUAL has shapeshift effect, but not implemented feature in Trinity for this spell.
                summoned->CastSpell(summoned, SPELL_STOLEN_SOUL_VISUAL, false);
                summoned->SetDisplayId(soulmodel);
                summoned->setFaction(me->getFaction());

                if (Unit* target = Unit::GetUnit(*me, soulholder))
                {
                    CAST_AI(mob_stolen_soul::mob_stolen_soulAI, summoned->AI())->SetMyClass(soulclass);
                    summoned->AI()->AttackStart(target);
                }
            }
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            if (rand()%2)
                return;

            Talk(SAY_SLAY);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            //When Exarch Maladar is defeated D'ore appear.
            me->SummonCreature(19412, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 600000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (!Avatar_summoned && HealthBelowPct(25))
            {
                if (me->IsNonMeleeSpellCast(false))
                    me->InterruptNonMeleeSpells(true);

                Talk(SAY_SUMMON);

                DoCast(SPELL_SUMMON_AVATAR);
                Avatar_summoned = true;
                StolenSoul_Timer = 15000 + rand()% 15000;
            }

            if (StolenSoul_Timer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                {
                    if (target->IsPlayer())
                    {
                        if (me->IsNonMeleeSpellCast(false))
                            me->InterruptNonMeleeSpells(true);

                        uint32 i = urand(1, 2);
                        if (i == 1)
                            Talk(SAY_ROAR);
                        else
                            Talk(SAY_SOUL_CLEAVE);

                        soulmodel = target->GetDisplayId();
                        soulholder = target->GetGUID();
                        soulclass = target->getClass();

                        DoCast(target, SPELL_STOLEN_SOUL);
                        me->SummonCreature(ENTRY_STOLEN_SOUL, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);

                        StolenSoul_Timer = 20000 + rand()% 10000;
                    }
                    else
                        StolenSoul_Timer = 1000;
                }
            }
            else
                StolenSoul_Timer -= diff;

            if (Ribbon_of_Souls_timer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_RIBBON_OF_SOULS);

                Ribbon_of_Souls_timer = 5000 + (rand()%20 * 1000);
            }
            else
                Ribbon_of_Souls_timer -= diff;

            if (Fear_timer <= diff)
            {
                DoCast(me, SPELL_SOUL_SCREAM);
                Fear_timer = 15000 + rand()% 15000;
            }
            else
                Fear_timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

#define SPELL_AV_MORTAL_STRIKE          16856
#define SPELL_AV_SUNDER_ARMOR           16145

class mob_avatar_of_martyred : public CreatureScript
{
public:
    mob_avatar_of_martyred() : CreatureScript("mob_avatar_of_martyred") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_avatar_of_martyredAI (creature);
    }

    struct mob_avatar_of_martyredAI : public ScriptedAI
    {
        mob_avatar_of_martyredAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 Mortal_Strike_timer;

        void Reset() override
        {
            Mortal_Strike_timer = 10000;
        }

        void EnterCombat(Unit* /*who*/) override {}

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (Mortal_Strike_timer <= diff)
            {
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_AV_MORTAL_STRIKE, false);

                Mortal_Strike_timer = urand(10, 30) * 1000;
            }
            else
                Mortal_Strike_timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_exarch_maladaar()
{
    new boss_exarch_maladaar();
    new mob_avatar_of_martyred();
    new mob_stolen_soul();
}
