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

#define SAY_AGGRO           RAND(-1533109, -1533110, -1533111)
#define SAY_SLAY            -1533112
#define SAY_TAUNT           RAND(-1533113, -1533114, -1533115, -1533116, -1533117)
#define SAY_DEATH           -1533118

#define SPELL_SPELL_DISRUPTION  29310
#define SPELL_DECREPIT_FEVER    RAID_MODE(29998, 55011)
#define SPELL_PLAGUE_CLOUD      29350

enum Emotes
{
    EMOTE_TELEPORT          = -1533136,
    EMOTE_RETURN            = -1533137
};
enum Events
{
    EVENT_NONE,
    EVENT_DISRUPT,
    EVENT_FEVER,
    EVENT_ERUPT,
    EVENT_PHASE,
};

enum Phases
{
    PHASE_FIGHT = 1,
    PHASE_DANCE,
};

enum Achievements
{
    ACHIEV_THE_SAFETY_DANCE_10       = 1996,
    ACHIEV_THE_SAFETY_DANCE_25       = 2139
};
class boss_heigan : public CreatureScript
{
public:
    boss_heigan() : CreatureScript("boss_heigan") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_heiganAI (pCreature);
    }

    struct boss_heiganAI : public BossAI
    {
        boss_heiganAI(Creature *c) : BossAI(c, BOSS_HEIGAN) {}

        uint32 eruptSection;
        uint32 uiCheckAchievTimer;
        bool eruptDirection;
        bool bAchievSavety;
        Phases phase;

        void Reset() override
        {
            _Reset();
            bAchievSavety = true;
            uiCheckAchievTimer = 1*IN_MILLISECONDS;
        }

        void KilledUnit(Unit* /*Victim*/) override
        {
            if (!(rand()%5))
                DoScriptText(SAY_SLAY, me);
        }

        void JustDied(Unit* /*Killer*/) override
        {
            _JustDied();
            DoScriptText(SAY_DEATH, me);

            if (bAchievSavety)
                if  (InstanceScript* instance = me->GetInstanceScript())
                    instance->DoCompleteAchievement(RAID_MODE(ACHIEV_THE_SAFETY_DANCE_10, ACHIEV_THE_SAFETY_DANCE_25));
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            DoScriptText(SAY_AGGRO, me);
            EnterPhase(PHASE_FIGHT);
        }

        void EnterPhase(Phases newPhase)
        {
            phase = newPhase;
            events.Reset();
            eruptSection = 3;
            if (phase == PHASE_FIGHT)
            {
                me->GetMotionMaster()->MoveChase(me->SelectVictim());
                events.ScheduleEvent(EVENT_DISRUPT, urand(10000, 25000));
                events.ScheduleEvent(EVENT_FEVER, urand(15000, 20000));
                events.ScheduleEvent(EVENT_PHASE, 90000);
                events.ScheduleEvent(EVENT_ERUPT, 15000);
            }
            else
            {
                me->GetMotionMaster()->MoveIdle();
                float x, y, z, o;
                me->GetHomePosition(x, y, z, o);
                me->NearTeleportTo(x, y, z, o);
                DoCastAOE(SPELL_PLAGUE_CLOUD);
                events.ScheduleEvent(EVENT_PHASE, 45000);
                events.ScheduleEvent(EVENT_ERUPT, 8000);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || !CheckInRoom())
                return;

            if (uiCheckAchievTimer <= diff)
            {
                if (Map *instance = me->GetInstanceScript()->instance)
                {
                    Map::PlayerList const &PlayerList = instance->GetPlayers();
                        if (!PlayerList.isEmpty())
                            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                if (i->getSource()->isDead())
                                    bAchievSavety = false;
                }
                uiCheckAchievTimer = 1*IN_MILLISECONDS;
            } else
                uiCheckAchievTimer -= diff;
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DISRUPT:
                        DoCastAOE(SPELL_SPELL_DISRUPTION);
                        events.ScheduleEvent(EVENT_DISRUPT, urand(5000, 10000));
                        break;
                    case EVENT_FEVER:
                        DoCastAOE(SPELL_DECREPIT_FEVER);
                        events.ScheduleEvent(EVENT_FEVER, urand(20000, 25000));
                        break;
                    case EVENT_PHASE:
                        EnterPhase(phase == PHASE_FIGHT ? PHASE_DANCE : PHASE_FIGHT);
                        if (phase == PHASE_FIGHT)
                            DoScriptText(EMOTE_RETURN, me);
                        else
                            DoScriptText(EMOTE_TELEPORT, me);
                        break;
                    case EVENT_ERUPT:
                        instance->SetData(DATA_HEIGAN_ERUPT, eruptSection);

                        if (eruptSection == 0)
                            eruptDirection = true;
                        else if (eruptSection == 3)
                            eruptDirection = false;

                        eruptDirection ? ++eruptSection : --eruptSection;

                        events.ScheduleEvent(EVENT_ERUPT, phase == PHASE_FIGHT ? 10000 : 3333);
                        break;
                }
            }

            DoMeleeAttackIfReady();

            EnterEvadeIfOutOfCombatArea(diff);
        }
    };

};

void AddSC_boss_heigan()
{
    new boss_heigan();
}
