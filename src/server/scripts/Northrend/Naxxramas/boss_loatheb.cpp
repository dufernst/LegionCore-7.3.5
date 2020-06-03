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
    EMOTE_AURA_BLOCKING                                     = -1533143,
    EMOTE_AURA_WANE                                         = -1533144,
    EMOTE_AURA_FADING                                       = -1533145
};
enum Spells
{
    SPELL_NECROTIC_AURA                                    = 55593,
    SPELL_SUMMON_SPORE                                     = 29234,
    SPELL_DEATHBLOOM                                       = 29865,
    H_SPELL_DEATHBLOOM                                     = 55053,
    SPELL_INEVITABLE_DOOM                                  = 29204,
    H_SPELL_INEVITABLE_DOOM                                = 55052,
    SPELL_FUNGAL_CREEP                                     = 29232
};

enum Events
{
    EVENT_NONE,
    EVENT_AURA,
    EVENT_BLOOM,
    EVENT_DOOM,
    EVENT_EMOTE_WANE,
    EVENT_EMOTE_FADE
};

enum Achievements
{
    ACHIEVEMENT_SPORE_LOSER_10            =2182,
    ACHIEVEMENT_SPORE_LOSER_25            =2183,
};
class boss_loatheb : public CreatureScript
{
public:
    boss_loatheb() : CreatureScript("boss_loatheb") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_loathebAI (creature);
    }

    struct boss_loathebAI : public BossAI
    {
        boss_loathebAI(Creature* c) : BossAI(c, BOSS_LOATHEB) {}

        bool bSporeKilled;

        void Reset() override
        {
            _Reset();
            bSporeKilled = false;
        }

        void EnterCombat(Unit * /*who*/) override
        {
            _EnterCombat();
            events.ScheduleEvent(EVENT_AURA, 10000);
            events.ScheduleEvent(EVENT_BLOOM, 5000);
            events.ScheduleEvent(EVENT_DOOM, 120000);
        }

        void DoAction(int32 const /*param*/) override
        {
            bSporeKilled = true;
        }

        void JustDied(Unit* /*Killer*/) override
        {
            _JustDied();

            if (instance && !bSporeKilled)
            {
                instance->DoCompleteAchievement(RAID_MODE(ACHIEVEMENT_SPORE_LOSER_10, ACHIEVEMENT_SPORE_LOSER_25));
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FUNGAL_CREEP);
            }
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
                    case EVENT_AURA:
                        DoCastAOE(SPELL_NECROTIC_AURA);
                        DoScriptText(EMOTE_AURA_BLOCKING, me);
                        events.ScheduleEvent(EVENT_AURA, 20000);
                        events.ScheduleEvent(EVENT_EMOTE_WANE, 12000);
                        events.ScheduleEvent(EVENT_EMOTE_FADE, 17000);
                        break;
                    case EVENT_BLOOM:
                        // TODO : Add missing text
                        DoCastAOE(SPELL_SUMMON_SPORE, true);
                        DoCastAOE(RAID_MODE(SPELL_DEATHBLOOM, H_SPELL_DEATHBLOOM));
                        events.ScheduleEvent(EVENT_BLOOM, 30000);
                        break;
                    case EVENT_DOOM:
                        DoCastAOE(RAID_MODE(SPELL_INEVITABLE_DOOM, H_SPELL_INEVITABLE_DOOM));
                        events.ScheduleEvent(EVENT_DOOM, events.GetTimer() < 5*60000 ? 30000 : 15000);
                        break;
                    case EVENT_EMOTE_WANE:
                        DoScriptText(EMOTE_AURA_WANE, me);
                        break;
                    case EVENT_EMOTE_FADE:
                        DoScriptText(EMOTE_AURA_FADING, me);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

};

void AddSC_boss_loatheb()
{
    new boss_loatheb();
}
