/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "ReactorAI.h"
#include "CreatureAIImpl.h"
#include "CreatureTextMgr.h"

int ReactorAI::Permissible(const Creature* creature)
{
    if (creature->isCivilian() || creature->IsNeutralToAll())
        return PERMIT_BASE_REACTIVE;

    return PERMIT_BASE_NO;
}

void ReactorAI::MoveInLineOfSight(Unit*) {}

void ReactorAI::InitializeAI()
{
    for (auto& spell : *me->CreatureSpells)
    {
        if (SpellInfo const* sInfo = sSpellMgr->GetSpellInfo(spell.second.SpellID))
            if (sInfo->GetMaxRange(false) >= 30.0f && sInfo->GetMaxRange(false) > me->GetAttackDist() && (sInfo->AttributesCu[0] & SPELL_ATTR0_CU_DIRECT_DAMAGE) && !sInfo->IsTargetingAreaCast())
                if (!sInfo->IsPositive())
                    me->SetAttackDist(sInfo->GetMaxRange(false));
    }

    CreatureTexts = sCreatureTextMgr->GetTextGroup(me->GetEntry(), TEXT_GROUP_TIMER);
    CreatureCombatTexts = sCreatureTextMgr->GetTextGroup(me->GetEntry(), TEXT_GROUP_COMBAT_TIMER);

    CreatureAI::InitializeAI();

    if (me->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        return;

    if (auto faction = me->getFactionTemplateEntry())
        if (faction->ID == 28 || faction->ID == 31)
        {
            auto movementType = me->GetDefaultMovementType();
            if (movementType < MAX_DB_MOTION_TYPE && movementType != WAYPOINT_MOTION_TYPE)
                me->GetMotionMaster()->MoveRandom(urand(30, 40));
        }
}

void ReactorAI::Reset()
{
    events.Reset();
    spellCasts.Reset();
    textCombatEvents.Reset();

    for (auto& spell : *me->CreatureSpells)
        if (AISpellInfo[spell.second.SpellID].condition == AICOND_EVADE)
            if (AISpellInfo[spell.second.SpellID].target == AITARGET_SELF)
                me->CastSpell(me, spell.second.SpellID, false);
}

void ReactorAI::UpdateAI(uint32 diff)
{
    if (!me->IsInWorld() || me->isDead() || me->m_cleanupDone)
        return;

    if (!me->isInCombat())
    {
        for (auto& event : textEvents)
        {
            event.second.Update(diff);
            if (uint32 eventId = event.second.ExecuteEvent())
            {
                Player* textTarget = ObjectAccessor::GetPlayer(*me, event.first);
                if (!textTarget)
                {
                    event.second.ScheduleEvent(eventId, 500);
                    continue;
                }
                auto* text = &(*CreatureTexts)[eventId - 1];
                if (me->GetDistance(textTarget) > sCreatureTextMgr->GetRangeForChatType(text->type))
                {
                    event.second.ScheduleEvent(eventId, 500);
                    continue;
                }
                sCreatureTextMgr->SendText(me, text, event.first);
                uint32 nextEventId = eventId + 1;
                if (eventId < CreatureTexts->size()) // use eventId is true, not replace it!!!
                {
                    auto* nextText = &(*CreatureTexts)[eventId]; // use eventId is true, not replace it!!!
                    event.second.ScheduleEvent(nextEventId, nextText->MinTimer);
                }
            }
        }
    }

    // update i_victimGuid if me->getVictim() !=0 and changed
    if (!UpdateVictim())
        return;

    events.Update(diff);
    spellCasts.Update(diff);

    if (me->isInCombat())
        textCombatEvents.Update(diff);

    if (me->getVictim() && me->getVictim()->HasCrowdControlAura(me))
    {
        me->InterruptNonMeleeSpells(false);
        return;
    }

    if (IsInControl() || me->HasUnitState(UNIT_STATE_CASTING))
        return;

    while (uint32 eventId = events.ExecuteEvent())
    {
        switch (eventId)
        {
        case EVENT_1:
            if (me->HealthBelowPct(15) && !me->HasSearchedAssistance())
            {
                events.ScheduleEvent(eventId, 30000);
                me->DoFleeToGetAssistance();
                return;
            }
            else
                events.ScheduleEvent(eventId, 500);
            break;
        }
    }

    if (me->isInCombat())
    {
        if (uint32 eventId = textCombatEvents.ExecuteEvent())
        {
            auto* text = &(*CreatureCombatTexts)[eventId - 1];
            sCreatureTextMgr->SendText(me, text);
            uint32 nextEventId = eventId + 1;
            if (eventId < CreatureCombatTexts->size()) // use eventId is true, not replace it!!!
            {
                auto* nextText = &(*CreatureCombatTexts)[eventId]; // use eventId is true, not replace it!!!
                textCombatEvents.ScheduleEvent(nextEventId, nextText->MinTimer);
            }
        }
    }

    if (uint32 spellId = spellCasts.ExecuteEvent())
    {
        bool isCasted = false;
        if (SpellInfo const* sInfo = sSpellMgr->GetSpellInfo(spellId))
        {
            if (sInfo->CanAutoCast(me, me->getVictim()))
            {
                if (CreatureSpell const* spellData = me->GetCreatureSpell(spellId))
                    if (spellData->Text)
                        sCreatureTextMgr->SendText(me, spellData->Text, me->getVictim() ? me->getVictim()->GetGUID() : ObjectGuid::Empty);
                DoCast(spellId);
                isCasted = true;
            }
        }

        if (isCasted)
            spellCasts.ScheduleEvent(spellId, AISpellInfo[spellId].cooldown + rand() % AISpellInfo[spellId].cooldown);
        else
            spellCasts.ScheduleEvent(spellId, 500);
    }
    else
        DoMeleeAttackIfReady();
}

void ReactorAI::JustDied(Unit* killer)
{
    if (!me->IsInWorld() || me->m_cleanupDone)
        return;

    if (me->isElite() || roll_chance_i(25)) // Prevent spamm text
        TalkAuto(TEXT_GROUP_DIE, killer->GetGUID());

    for (auto& spell : *me->CreatureSpells)
        if (AISpellInfo[spell.second.SpellID].condition == AICOND_DIE)
            if (SpellInfo const* sInfo = sSpellMgr->GetSpellInfo(spell.second.SpellID))
                if (sInfo->CanAutoCast(me, killer))
                    me->CastSpell(killer, spell.second.SpellID, true);
}

void ReactorAI::EnterCombat(Unit* who)
{
    AttackedBy(who);

    if (me->isElite() || roll_chance_i(25)) // Prevent spamm text
        TalkAuto(TEXT_GROUP_COMBAT, who->GetGUID());
    if (CreatureCombatTexts)
    {
        textCombatEvents.Reset();
        auto const& text = CreatureCombatTexts->begin();
        textCombatEvents.ScheduleEvent(EVENT_1, text->MinTimer);
    }
    me->CastStop();
    for (auto& spell : *me->CreatureSpells)
    {
        if (AISpellInfo[spell.second.SpellID].condition == AICOND_AGGRO)
        {
            if (SpellInfo const* sInfo = sSpellMgr->GetSpellInfo(spell.second.SpellID))
                if (sInfo->CanAutoCast(me, who))
                    me->CastSpell(who, spell.second.SpellID, false);
        }
        else if (AISpellInfo[spell.second.SpellID].condition == AICOND_COMBAT)
            spellCasts.ScheduleEvent(spell.second.SpellID, AISpellInfo[spell.second.SpellID].cooldown + rand() % AISpellInfo[spell.second.SpellID].cooldown);
    }
    if (me->m_CanCallAssistance)
        events.ScheduleEvent(EVENT_1, 500);
}

void ReactorAI::AddClientVisibility(ObjectGuid guid)
{
    return;
    if (CreatureTexts)
    {
        auto const& text = CreatureTexts->begin();
        EventMap& event = textEvents[guid];
        event.ScheduleEvent(EVENT_1, text->MinTimer);
    }
}

void ReactorAI::RemoveClientVisibility(ObjectGuid guid)
{
    return;
    if (CreatureTexts)
        textEvents.erase(guid);
}

void ReactorAI::AttackedBy(Unit * who)
{
    if (me->HasUnitState(UNIT_STATE_FLEEING))
        return;

    if (!who->IsPlayer())
        if (auto faction = me->getFactionTemplateEntry())
            if (faction->ID == 28 || faction->Friend[1] == 28)
            {
                me->GetMotionMaster()->MoveFleeing(who);
                return;
            }
}
