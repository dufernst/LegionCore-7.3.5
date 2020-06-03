/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "Cell.h"
#include "CellImpl.h"
#include "ChatTextBuilder.h"
#include "CreatureTextMgr.h"
#include "CreatureTextMgrImpl.h"
#include "DatabaseEnv.h"
#include "GossipDef.h"
#include "GridDefines.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Language.h"
#include "LFGMgr.h"
#include "ObjectDefines.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "QuestData.h"
#include "ScenarioMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SmartAI.h"
#include "SmartScript.h"
#include "SpellMgr.h"
#include "Vehicle.h"

SmartScript::SmartScript()
{
    go = nullptr;
    me = nullptr;
    trigger = nullptr;
    event = nullptr;
    mEventPhase = 0;
    mPathId = 0;
    mTargetStorage = new ObjectListMap();
    mStoredEvents.clear();
    mTextTimer = 0;
    mLastTextID = 0;
    mUseTextTimer = false;
    mTalkerEntry = 0;
    mTemplate = SMARTAI_TEMPLATE_BASIC;
    mScriptType = SMART_SCRIPT_TYPE_CREATURE;
}

SmartScript::~SmartScript()
{
    for (auto& itr : *mTargetStorage)
        delete itr.second;

    delete mTargetStorage;
}

void SmartScript::OnReset()
{
    SetPhase(0);
    ResetBaseObject();
    for (auto& itr : mEvents)
    {
        if (!(itr.event.event_flags & SMART_EVENT_FLAG_DONT_RESET))
        {
            InitTimer(itr);
            itr.runOnce = false;
        }
    }
    ProcessEventsFor(SMART_EVENT_RESET);
    mLastInvoker = ObjectGuid::Empty;
}

void SmartScript::ResetBaseObject()
{
    if (!meOrigGUID.IsEmpty())
    {
        if (Creature* m = HashMapHolder<Creature>::Find(meOrigGUID))
        {
            me = m;
            go = nullptr;
        }
    }
    if (!goOrigGUID.IsEmpty())
    {
        if (GameObject* o = HashMapHolder<GameObject>::Find(goOrigGUID))
        {
            me = nullptr;
            go = o;
        }
    }
    goOrigGUID = ObjectGuid::Empty;
    meOrigGUID = ObjectGuid::Empty;
}

void SmartScript::ProcessEventsFor(SMART_EVENT e, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob)
{
    for (auto& mEvent : mEvents)
    {
        auto eventType = SMART_EVENT(mEvent.GetEventType());
        if (eventType == SMART_EVENT_LINK)//special handling
            continue;

        if (eventType == e/* && (!(*i).event.event_phase_mask || IsInPhase((*i).event.event_phase_mask)) && !((*i).event.event_flags & SMART_EVENT_FLAG_NOT_REPEATABLE && (*i).runOnce)*/)
            ProcessEvent(mEvent, unit, var0, var1, bvar, spell, gob);
    }
}

void SmartScript::ProcessAction(SmartScriptHolder& e, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob)
{
    //calc random
    if (e.GetEventType() != SMART_EVENT_LINK && e.event.event_chance < 100 && e.event.event_chance)
    {
        uint32 rnd = urand(0, 100);
        if (e.event.event_chance <= rnd)
            return;
    }
    e.runOnce = true;//used for repeat check

    if (unit)
        mLastInvoker = unit->GetGUID();

    if (Unit* tempInvoker = GetLastInvoker())
        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: Invoker: %s (guidlow: %u)", tempInvoker->GetName(), tempInvoker->GetGUIDLow());

    switch (e.GetActionType())
    {
        case SMART_ACTION_TALK:
        {
            ObjectList* targets = GetTargets(e, unit);
            Creature* talker = me;
            Player* targetPlayer = nullptr;
            if (targets)
            {
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    if (IsCreature((*itr)))
                    {
                        talker = (*itr)->ToCreature();
                        break;
                    }
                    if (IsPlayer((*itr)))
                    {
                        targetPlayer = (*itr)->ToPlayer();
                        break;
                    }
                }

                delete targets;
            }

            if (!talker)
                break;

            mTalkerEntry = talker->GetEntry();
            mLastTextID = e.action.talk.textGroupID;
            mTextTimer = e.action.talk.duration;
            if (IsPlayer(GetLastInvoker())) // used for $vars in texts and whisper target
                mTextGUID = GetLastInvoker()->GetGUID();
            else if (targetPlayer)
                mTextGUID = targetPlayer->GetGUID();
            else
                mTextGUID = ObjectGuid::Empty;

            mUseTextTimer = true;
            sCreatureTextMgr->SendChat(talker, uint8(e.action.talk.textGroupID), mTextGUID);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_TALK: talker: %s (GuidLow: %u), textGuid: %u",
                talker->GetName(), talker->GetGUIDLow(), mTextGUID.GetCounter());
            break;
        }
        case SMART_ACTION_SIMPLE_TALK:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    if (IsCreature(*itr))
                        sCreatureTextMgr->SendChat((*itr)->ToCreature(), uint8(e.action.talk.textGroupID), IsPlayer(GetLastInvoker()) ? GetLastInvoker()->GetGUID() : ObjectGuid::Empty);
                    else if (IsPlayer(*itr) && me)
                    {
                        Unit* templastInvoker = GetLastInvoker();
                        sCreatureTextMgr->SendChat(me, uint8(e.action.talk.textGroupID), IsPlayer(templastInvoker) ? templastInvoker->GetGUID() : ObjectGuid::Empty, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, (*itr)->ToPlayer());
                    }
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SIMPLE_TALK: talker: %s (GuidLow: %u), textGroupId: %u",
                        (*itr)->GetName(), (*itr)->GetGUIDLow(), uint8(e.action.talk.textGroupID));
                }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_PLAY_EMOTE:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    if (IsUnit(*itr))
                    {
                        (*itr)->ToUnit()->HandleEmoteCommand(e.action.emote.emote);
                        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_PLAY_EMOTE: target: %s (GuidLow: %u), emote: %u",
                            (*itr)->GetName(), (*itr)->GetGUIDLow(), e.action.emote.emote);
                    }
                }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_SOUND:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    if (IsUnit(*itr))
                    {
                        (*itr)->SendPlaySound(e.action.sound.sound, e.action.sound.range > 0);
                        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SOUND: target: %s (GuidLow: %u), sound: %u, onlyself: %u",
                            (*itr)->GetName(), (*itr)->GetGUIDLow(), e.action.sound.sound, e.action.sound.range);
                    }
                }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_SET_FACTION:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    if (IsCreature(*itr))
                    {
                        if (e.action.faction.factionID)
                        {
                            (*itr)->ToCreature()->setFaction(e.action.faction.factionID);
                            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SET_FACTION: Creature entry %u, GuidLow %u set faction to %u",
                                (*itr)->GetEntry(), (*itr)->GetGUIDLow(), e.action.faction.factionID);
                        }
                        else
                        {
                            if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate((*itr)->ToCreature()->GetEntry()))
                            {
                                if ((*itr)->ToCreature()->getFaction() != ci->faction)
                                {
                                    (*itr)->ToCreature()->setFaction(ci->faction);
                                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SET_FACTION: Creature entry %u, GuidLow %u set faction to %u",
                                        (*itr)->GetEntry(), (*itr)->GetGUIDLow(), ci->faction);
                                }
                            }
                        }
                    }
                }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (!IsCreature(*itr))
                    continue;

                if (e.action.morphOrMount.creature || e.action.morphOrMount.model)
                {
                    //set model based on entry from creature_template
                    if (e.action.morphOrMount.creature)
                    {
                        if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(e.action.morphOrMount.creature))
                        {
                            Creature* crea = (*itr)->ToCreature();
                            ASSERT(crea);
                            crea->SetOutfit(sObjectMgr->ChooseDisplayId(0, ci));
                            if (crea->IsMirrorImage())
                            {
                                new MirrorImageUpdate(crea);
                            }
                            else
                            {
                                (*itr)->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
                                
                                uint32 display_id = sObjectMgr->GetCreatureDisplay(crea->GetOutfit());
                                (*itr)->ToCreature()->SetDisplayId(display_id);
                                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL: Creature entry %u, GuidLow %u set displayid to %u",
                                    (*itr)->GetEntry(), (*itr)->GetGUIDLow(), display_id);
                            }
                        }
                    }
                    //if no param1, then use value from param2 (modelId)
                    else
                    {
                        (*itr)->ToCreature()->SetDisplayId(e.action.morphOrMount.model);
                        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL: Creature entry %u, GuidLow %u set displayid to %u",
                            (*itr)->GetEntry(), (*itr)->GetGUIDLow(), e.action.morphOrMount.model);
                    }
                }
                else
                {
                    (*itr)->ToCreature()->DeMorph();
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL: Creature entry %u, GuidLow %u demorphs.",
                        (*itr)->GetEntry(), (*itr)->GetGUIDLow());
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_FAIL_QUEST:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsPlayer(*itr))
                {
                    (*itr)->ToPlayer()->FailQuest(e.action.quest.quest);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_FAIL_QUEST: Player guidLow %u fails quest %u",
                        (*itr)->GetGUIDLow(), e.action.quest.quest);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_ADD_QUEST:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (Player* player = (*itr)->ToPlayer())
                {
                    if (Quest const* q = sQuestDataStore->GetQuestTemplate(e.action.quest.quest))
                    {
                        uint32 questid = q->GetQuestId();
                        if(e.action.quest.check)
                        {
                            if (player->GetQuestStatus(e.action.quest.quest) == e.action.quest.queststate && player->GetQuestStatus(e.action.quest.prequest) == e.action.quest.prequeststate)
                            {
                                player->AddQuest(q, nullptr);
                                if (player->CanCompleteQuest(questid))
                                    player->CompleteQuest(questid);
                            }
                            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_ADD_QUEST: Player guidLow %u add quest %u, queststate %u, prequeststate %u",
                                (*itr)->GetGUIDLow(), e.action.quest.quest, player->GetQuestStatus(e.action.quest.quest), player->GetQuestStatus(e.action.quest.prequest));
                        }
                        else
                        {
                            player->AddQuest(q, nullptr);
                            if (player->CanCompleteQuest(questid))
                                player->CompleteQuest(questid);
                            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_ADD_QUEST: Player guidLow %u add quest %u",
                                (*itr)->GetGUIDLow(), e.action.quest.quest);
                        }
                    }
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_CLEAR_QUEST:
        {

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                    for (auto const& questID : e.action.clearQuest.quest)
                        player->RemoveRewardedQuest(questID);

            delete targets;
            break;
        }
        case SMART_ACTION_COMPLETE_QUEST:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                    for (auto const& questID : e.action.completeQuest.quest)
                        player->CompleteQuest(questID);

            delete targets;
            break;
        }
        case SMART_ACTION_UNLEARN_SPELL:
        {

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                    for (auto const& entryID : e.action.unlearnSpell.spell)
                        player->removeSpell(entryID);

            delete targets;
            break;
        }
        case SMART_ACTION_LEARN_SPELL:
        {

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                    for (auto const& entryID : e.action.learnSpell.spell)
                        player->learnSpell(entryID,false);

            delete targets;
            break;
        }
        case SMART_ACTION_MOD_CURRENCY:
        {

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                    player->ModifyCurrency(e.action.modCurrency.currencyID, e.action.modCurrency.count, true, false);

            delete targets;
            break;
        }
        case SMART_ACTION_SET_REACT_STATE:
        {
            if (!me)
                break;

            ReactStates saveState = me->GetReactState();
            me->SetReactState(ReactStates(e.action.react.state));

            if (uint32 delay = e.action.react.returnStateTimer)
                me->AddDelayedEvent(delay, [this, saveState]() -> void { if (me) me->SetReactState(saveState); });

            if (me->isInCombat() && e.action.react.stopAttack)
                me->AttackStop();

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SET_REACT_STATE: Creature guidLow %u set reactstate %u",
                me->GetGUIDLow(), e.action.react.state);
            break;
        }
        case SMART_ACTION_RANDOM_EMOTE:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            uint32 emotes[SMART_ACTION_PARAM_COUNT];
            emotes[0] = e.action.randomEmote.emote1;
            emotes[1] = e.action.randomEmote.emote2;
            emotes[2] = e.action.randomEmote.emote3;
            emotes[3] = e.action.randomEmote.emote4;
            emotes[4] = e.action.randomEmote.emote5;
            emotes[5] = e.action.randomEmote.emote6;
            uint32 temp[SMART_ACTION_PARAM_COUNT];
            uint32 count = 0;
            for (auto emote : emotes)
            {
                if (emote)
                {
                    temp[count] = emote;
                    ++count;
                }
            }

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    uint32 emote = temp[urand(0, count)];
                    (*itr)->ToUnit()->HandleEmoteCommand(emote);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_RANDOM_EMOTE: Creature guidLow %u handle random emote %u",
                        (*itr)->GetGUIDLow(), emote);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_THREAT_ALL_PCT:
        {
            if (!me)
                break;

            std::list<HostileReference*> const& threatList = me->getThreatManager().getThreatList();
            for (auto i : threatList)
            {
                if (Unit* target = Unit::GetUnit(*me, i->getUnitGuid()))
                {
                    me->getThreatManager().modifyThreatPercent(target, e.action.threatPCT.threatINC ? static_cast<int32>(e.action.threatPCT.threatINC) : -static_cast<int32>(e.action.threatPCT.threatDEC));
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_THREAT_ALL_PCT: Creature guidLow %u modify threat for unit %u, value %i",
                        me->GetGUIDLow(), target->GetGUIDLow(), e.action.threatPCT.threatINC ? static_cast<int32>(e.action.threatPCT.threatINC) : -static_cast<int32>(e.action.threatPCT.threatDEC));
                }
            }
            break;
        }
        case SMART_ACTION_THREAT_SINGLE_PCT:
        {
            if (!me)
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    me->getThreatManager().modifyThreatPercent((*itr)->ToUnit(), e.action.threatPCT.threatINC ? static_cast<int32>(e.action.threatPCT.threatINC) : -static_cast<int32>(e.action.threatPCT.threatDEC));
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_THREAT_SINGLE_PCT: Creature guidLow %u modify threat for unit %u, value %i",
                        me->GetGUIDLow(), (*itr)->GetGUIDLow(), e.action.threatPCT.threatINC ? static_cast<int32>(e.action.threatPCT.threatINC) : -static_cast<int32>(e.action.threatPCT.threatDEC));
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_CALL_AREAEXPLOREDOREVENTHAPPENS:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                // Special handling for vehicles
                if (IsUnit(*itr))
                    if (Vehicle* vehicle = (*itr)->ToUnit()->GetVehicleKit())
                        for (auto& Seat : vehicle->Seats)
                            if (Player* player = ObjectAccessor::FindPlayer(Seat.second.Passenger.Guid))
                                player->AreaExploredOrEventHappens(e.action.quest.quest);

                if (IsPlayer(*itr))
                {
                    (*itr)->ToPlayer()->AreaExploredOrEventHappens(e.action.quest.quest);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_CALL_AREAEXPLOREDOREVENTHAPPENS: Player guidLow %u credited quest %u",
                        (*itr)->GetGUIDLow(), e.action.quest.quest);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_PLAY_SPELL_VISUAL_KIT:
            if (!me)
                break;
            me->SendPlaySpellVisualKit(e.action.visualKit.id, e.action.visualKit.duration);
            break;
        case SMART_ACTION_CAST:
        {
            if(go)
                go->CastSpell(unit, e.action.cast.spell);

            if (!me)
                break;

            if (e.GetTargetType() == SMART_TARGET_POSITION)
            {
                if (e.target.x == 0.0f && e.target.y == 0.0f && e.target.z != 0.0f)
                {
                    float x, y, z;
                    me->GetPosition(x, y, z);
                    z += e.target.z;
                    me->CastSpell(x, y, z, e.action.cast.spell, e.action.cast.flags & SMARTCAST_TRIGGERED ? true : false);
                }
                else if (e.target.x != 0.0f && e.target.y != 0.0f && e.target.z != 0.0f)
                {
                    me->CastSpell(e.target.x, e.target.y, e.target.z, e.action.cast.spell, (e.action.cast.flags & SMARTCAST_TRIGGERED) ? true : false);
                }
                return;
            }

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    if (e.action.cast.flags & SMARTCAST_INTERRUPT_PREVIOUS)
                        me->InterruptNonMeleeSpells(false);

                    if (!(e.action.cast.flags & SMARTCAST_AURA_NOT_PRESENT) || !(*itr)->ToUnit()->HasAura(e.action.cast.spell))
                    {
                        if (e.action.cast.flags & CAST_FORCE_TARGET_SELF)
                            (*itr)->ToUnit()->CastSpell((*itr)->ToUnit(), e.action.cast.spell, (e.action.cast.flags & SMARTCAST_TRIGGERED) != 0);
                        else
                            me->CastSpell((*itr)->ToUnit(), e.action.cast.spell, (e.action.cast.flags & SMARTCAST_TRIGGERED) ? true : false);
                    }
                    else
                        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "Spell %u not casted because it has flag SMARTCAST_AURA_NOT_PRESENT and the target (Guid: %s Entry: %u Type: %u) already has the aura", e.action.cast.spell, (*itr)->GetGUID().ToString().c_str(), (*itr)->GetEntry(), uint32((*itr)->GetTypeId()));
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_CAST:: Creature %u casts spell %u on target %u with castflags %u",
                        me->GetGUIDLow(), e.action.cast.spell, (*itr)->GetGUIDLow(), e.action.cast.flags);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_CAST_CUSTOM:
        {
            if (!me)
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    if (e.action.castCustom.flags & SMARTCAST_INTERRUPT_PREVIOUS)
                        me->InterruptNonMeleeSpells(false);

                    uint32 spellId = e.action.castCustom.spell;
                    float bp0 = e.action.castCustom.damageMin;
                    if (e.action.castCustom.damageMax)
                        bp0 = urand(e.action.castCustom.damageMin, e.action.castCustom.damageMax);
                    float bp1 = e.action.castCustom.bp1;
                    float bp2 = e.action.castCustom.bp2;

                    if (!(e.action.castCustom.flags & SMARTCAST_AURA_NOT_PRESENT) || !(*itr)->ToUnit()->HasAura(spellId))
                    {
                        if (e.action.castCustom.flags & CAST_FORCE_TARGET_SELF)
                            (*itr)->ToUnit()->CastCustomSpell((*itr)->ToUnit(), spellId, bp0 ? &bp0 : nullptr, bp1 ? &bp1 : nullptr, bp2 ? &bp2 : nullptr, e.action.castCustom.flags & SMARTCAST_TRIGGERED ? true : false);
                        else
                            me->CastCustomSpell((*itr)->ToUnit(), spellId, bp0 ? &bp0 : nullptr, bp1 ? &bp1 : nullptr, bp2 ? &bp2 : nullptr, e.action.castCustom.flags & SMARTCAST_TRIGGERED ? true : false);
                    }
                    else
                        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "Spell %u not casted because it has flag SMARTCAST_AURA_NOT_PRESENT and the target (Guid: %s Entry: %u Type: %u) already has the aura", spellId, (*itr)->GetGUID().ToString().c_str(), (*itr)->GetEntry(), uint32((*itr)->GetTypeId()));
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_CAST_CUSTOM:: Creature %u casts spell %u on target %u with castflags %u",
                        me->GetGUIDLow(), spellId, (*itr)->GetGUIDLow(), e.action.castCustom.flags);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_INVOKER_CAST:
        {
            Unit* tempLastInvoker = GetLastInvoker();
            if (!tempLastInvoker)
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    if (e.action.cast.flags & SMARTCAST_INTERRUPT_PREVIOUS)
                        tempLastInvoker->InterruptNonMeleeSpells(false);

                    ObjectGuid targetGUID = (*itr)->GetGUID();
                    uint32 spellID = e.action.cast.spell;
                    bool isTrigger = (e.action.cast.flags & SMARTCAST_TRIGGERED) != 0;
                    if (!(e.action.cast.flags & SMARTCAST_AURA_NOT_PRESENT) || !(*itr)->ToUnit()->HasAura(e.action.cast.spell))
                    {
                        tempLastInvoker->AddDelayedEvent(200, [tempLastInvoker, targetGUID, spellID, isTrigger]() -> void
                        {
                            if (!tempLastInvoker)
                                return;

                            Unit* target = ObjectAccessor::GetUnit(*tempLastInvoker, targetGUID);
                            if (!target)
                                return;

                            tempLastInvoker->CastSpell(target, spellID, isTrigger);
                        });
                    }
                    else
                        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "Spell %u not casted because it has flag SMARTCAST_AURA_NOT_PRESENT and the target (Guid: %s Entry: %u Type: %u) already has the aura", e.action.cast.spell, (*itr)->GetGUID().ToString().c_str(), (*itr)->GetEntry(), uint32((*itr)->GetTypeId()));

                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_INVOKER_CAST: Invoker %u casts spell %u on target %u with castflags %u",
                        tempLastInvoker->GetGUIDLow(), e.action.cast.spell, (*itr)->GetGUIDLow(), e.action.cast.flags);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_ADD_AURA:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    (*itr)->ToUnit()->SetAuraStack(e.action.addAura.spell, (*itr)->ToUnit(), e.action.addAura.stack);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_ADD_AURA: Adding aura %u to unit %u",
                        e.action.addAura.spell, (*itr)->GetGUIDLow());
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_ACTIVATE_GOBJECT:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsGameObject(*itr))
                {
                    // Activate
                    (*itr)->ToGameObject()->SetLootState(GO_READY);
                    (*itr)->ToGameObject()->UseDoorOrButton(0, false, unit);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_ACTIVATE_GOBJECT. Gameobject %u (entry: %u) activated",
                        (*itr)->GetGUIDLow(), (*itr)->GetEntry());
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_RESET_GOBJECT:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsGameObject(*itr))
                {
                    (*itr)->ToGameObject()->ResetDoorOrButton();
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_RESET_GOBJECT. Gameobject %u (entry: %u) reset",
                        (*itr)->GetGUIDLow(), (*itr)->GetEntry());
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_SET_EMOTE_STATE:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    (*itr)->ToUnit()->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, e.action.emote.emote);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SET_EMOTE_STATE. Unit %u set emotestate to %u",
                        (*itr)->GetGUIDLow(), e.action.emote.emote);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_SET_UNIT_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    (*itr)->ToUnit()->SetFlag(UNIT_FIELD_FLAGS, e.action.unitFlag.flag);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SET_UNIT_FLAG. Unit %u added flag %u to UNIT_FIELD_FLAGS",
                        (*itr)->GetGUIDLow(), e.action.unitFlag.flag);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_REMOVE_UNIT_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    (*itr)->ToUnit()->RemoveFlag(UNIT_FIELD_FLAGS, e.action.unitFlag.flag);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_REMOVE_UNIT_FLAG. Unit %u removed flag %u to UNIT_FIELD_FLAGS",
                        (*itr)->GetGUIDLow(), e.action.unitFlag.flag);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_AUTO_ATTACK:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetAutoAttack(e.action.autoAttack.attack != 0);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_AUTO_ATTACK: Creature: %u bool on = %u",
                me->GetGUIDLow(), e.action.autoAttack.attack);
            break;
        }
        case SMART_ACTION_ALLOW_COMBAT_MOVEMENT:
        {
            if (!IsSmart())
                break;

            bool move = e.action.combatMove.move != 0;
            CAST_AI(SmartAI, me->AI())->SetCombatMove(move);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_ALLOW_COMBAT_MOVEMENT: Creature %u bool on = %u",
                me->GetGUIDLow(), e.action.combatMove.move);
            break;
        }
        case SMART_ACTION_SET_EVENT_PHASE:
        {
            if (!GetBaseObject())
                break;

            SetPhase(e.action.setEventPhase.phase);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SET_EVENT_PHASE: Creature %u set event phase %u",
                GetBaseObject()->GetGUIDLow(), e.action.setEventPhase.phase);
            break;
        }
        case SMART_ACTION_INC_EVENT_PHASE:
        {
            if (!GetBaseObject())
                break;

            IncPhase(e.action.incEventPhase.inc);
            DecPhase(e.action.incEventPhase.dec);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_INC_EVENT_PHASE: Creature %u inc event phase by %u, "
                "decrease by %u", GetBaseObject()->GetGUIDLow(), e.action.incEventPhase.inc, e.action.incEventPhase.dec);
            break;
        }
        case SMART_ACTION_EVADE:
        {
            if (!me)
                break;

            me->AI()->EnterEvadeMode();
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_EVADE: Creature %u EnterEvadeMode", me->GetGUIDLow());
            return;
        }
        case SMART_ACTION_BOSS_ANOUNCE:
        {
            if (!me)
                break;

            if(e.action.announce.area && e.action.announce.area != me->GetAreaId())
                break;

            LocaleConstant locale;

            if(e.action.announce.local)
                locale = static_cast<LocaleConstant>(e.action.announce.local);
            else
                locale = sWorld->GetDefaultDbcLocale();

            std::string text = sObjectMgr->GetTrinityString(e.action.announce.tesxid, locale);

            sWorld->SendWorldText(e.action.announce.idsample, text.c_str());

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_BOSS_ANOUNCE: Creature %u Announce", me->GetGUIDLow());
            return;
        }
        case SMART_ACTION_BOSS_EVADE:
        {
            if (!me)
                break;

            ProcessEventsFor(SMART_EVENT_EVADE);

            me->DeleteThreatList();
            me->ClearSaveThreatTarget();
            me->CombatStop(true);
            me->LoadCreaturesAddon();
            me->SetLootRecipient(nullptr);
            me->ResetPlayerDamageReq();
            me->GetMotionMaster()->MoveTargetedHome();

            if (InstanceScript* instance = me->GetInstanceScript())
                if(uint32 _bossId = me->GetBossId())
                    instance->SetBossState(_bossId, NOT_STARTED);

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_BOSS_EVADE: Creature %u EnterEvadeMode", me->GetGUIDLow());
            return;
        }
        case SMART_ACTION_FLEE_FOR_ASSIST:
        {
            if (!me)
                break;

            me->DoFleeToGetAssistance();
            if (e.action.flee.withEmote)
            {
                Trinity::BroadcastTextBuilder builder(me, CHAT_MSG_MONSTER_EMOTE, BROADCAST_TEXT_FLEE_FOR_ASSIST);
                CreatureTextMgr::SendChatPacket(me, builder, CHAT_MSG_MONSTER_EMOTE);
            }

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_FLEE_FOR_ASSIST: Creature %u DoFleeToGetAssistance", me->GetGUIDLow());
            break;
        }
        case SMART_ACTION_CALL_GROUPEVENTHAPPENS:
        {
            if (!unit)
                break;
            if (IsPlayer(unit) && GetBaseObject())
            {
                unit->ToPlayer()->GroupEventHappens(e.action.quest.quest, GetBaseObject());
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_CALL_GROUPEVENTHAPPENS: Player %u, group credit for quest %u",
                    unit->GetGUIDLow(), e.action.quest.quest);
            }
            // Special handling for vehicles
            if (Vehicle* vehicle = unit->GetVehicleKit())
                for (auto& Seat : vehicle->Seats)
                    if (Player* player = ObjectAccessor::FindPlayer(Seat.second.Passenger.Guid))
                        player->GroupEventHappens(e.action.quest.quest, GetBaseObject());
            break;
        }
        case SMART_ACTION_REMOVEAURASFROMSPELL:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (!IsUnit((*itr)))
                    continue;

                if (e.action.removeAura.spell == 0)
                    (*itr)->ToUnit()->RemoveAllAuras();
                else if(e.action.removeAura.stack > 0 || e.action.removeAura.stack < 0)
                {
                    if(Aura* aura = (*itr)->ToUnit()->GetAura(e.action.removeAura.spell))
                        aura->ModStackAmount(e.action.removeAura.stack);
                }
                else
                    (*itr)->ToUnit()->RemoveAurasDueToSpell(e.action.removeAura.spell);

                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_REMOVEAURASFROMSPELL: Unit %u, spell %u",
                    (*itr)->GetGUIDLow(), e.action.removeAura.spell);
            }

            delete targets;
            break;
        }
        case SMART_ACTION_FOLLOW:
        {
            if (!IsSmart())
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit((*itr)))
                {
                    CAST_AI(SmartAI, me->AI())->SetFollow((*itr)->ToUnit(), static_cast<float>(e.action.follow.dist), static_cast<float>(e.action.follow.angle), e.action.follow.credit, e.action.follow.entry, e.action.follow.creditType);
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_FOLLOW: Creature %u following target %u",
                        me->GetGUIDLow(), (*itr)->GetGUIDLow());
                    break;
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_RANDOM_PHASE:
        {
            if (!GetBaseObject())
                break;

            uint32 phases[SMART_ACTION_PARAM_COUNT];
            phases[0] = e.action.randomPhase.phase1;
            phases[1] = e.action.randomPhase.phase2;
            phases[2] = e.action.randomPhase.phase3;
            phases[3] = e.action.randomPhase.phase4;
            phases[4] = e.action.randomPhase.phase5;
            phases[5] = e.action.randomPhase.phase6;
            uint32 temp[SMART_ACTION_PARAM_COUNT];
            uint32 count = 0;
            for (auto phase : phases)
            {
                if (phase > 0)
                {
                    temp[count] = phase;
                    ++count;
                }
            }

            uint32 phase = temp[urand(0, count)];
            SetPhase(phase);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_RANDOM_PHASE: Creature %u sets event phase to %u",
                GetBaseObject()->GetGUIDLow(), phase);
            break;
        }
        case SMART_ACTION_RANDOM_PHASE_RANGE:
        {
            if (!GetBaseObject())
                break;

            uint32 phase = urand(e.action.randomPhaseRange.phaseMin, e.action.randomPhaseRange.phaseMax);
            SetPhase(phase);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_RANDOM_PHASE_RANGE: Creature %u sets event phase to %u",
                GetBaseObject()->GetGUIDLow(), phase);
            break;
        }
        case SMART_ACTION_CALL_KILLEDMONSTER:
        {
            Player* player = nullptr;
            if (me)
                player = me->GetLootRecipient();

            if (me && player)
                player->RewardPlayerAndGroupAtEvent(e.action.killedMonster.creature, player);
            else if (GetBaseObject())
            {
                ObjectList* targets = GetTargets(e, unit);
                if (!targets)
                    break;

                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    // Special handling for vehicles
                    if (IsUnit(*itr))
                        if (Vehicle* vehicle = (*itr)->ToUnit()->GetVehicleKit())
                            for (auto& Seat : vehicle->Seats)
                                if (Player* player2 = ObjectAccessor::FindPlayer(Seat.second.Passenger.Guid))
                                    player2->RewardPlayerAndGroupAtEvent(e.action.killedMonster.creature, player2);

                    if (!IsPlayer(*itr))
                        continue;

                    (*itr)->ToPlayer()->RewardPlayerAndGroupAtEvent(e.action.killedMonster.creature, (*itr)->ToPlayer());
                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_CALL_KILLEDMONSTER: Player %u, Killcredit: %u",
                        (*itr)->GetGUIDLow(), e.action.killedMonster.creature);
                }

                delete targets;
            }
            else if (trigger && IsPlayer(unit))
            {
                unit->ToPlayer()->RewardPlayerAndGroupAtEvent(e.action.killedMonster.creature, unit);
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_CALL_KILLEDMONSTER: (trigger == true) Player %u, Killcredit: %u",
                    unit->GetGUIDLow(), e.action.killedMonster.creature);
            }
            break;
        }
        case SMART_ACTION_SET_INST_DATA:
        {
            WorldObject* obj = GetBaseObject();
            if (!obj)
                obj = unit;

            if (!obj)
                break;

            InstanceScript* instance = obj->GetInstanceScript();
            if (!instance)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Event %u attempt to set instance data without instance script. EntryOrGuid %d", e.GetEventType(), e.entryOrGuid);
                break;
            }

            instance->SetData(e.action.setInstanceData.field, e.action.setInstanceData.data);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_SET_INST_DATA: Field: %u, data: %u",
                e.action.setInstanceData.field, e.action.setInstanceData.data);
            break;
        }
        case SMART_ACTION_SET_INST_DATA64:
        {
            WorldObject* obj = GetBaseObject();
            if (!obj)
                obj = unit;

            if (!obj)
                break;

            InstanceScript* instance = obj->GetInstanceScript();
            if (!instance)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Event %u attempt to set instance data without instance script. EntryOrGuid %d", e.GetEventType(), e.entryOrGuid);
                break;
            }

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            instance->SetGuidData(e.action.setInstanceData64.field, targets->front()->GetGUID());
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_SET_INST_DATA64: Field: %u, data: " UI64FMTD,
                e.action.setInstanceData64.field, targets->front()->GetGUID().GetCounter());

            delete targets;
            break;
        }
        case SMART_ACTION_UPDATE_TEMPLATE:
        {
            if (!me || me->GetEntry() == e.action.updateTemplate.creature)
                break;

            me->UpdateEntry(e.action.updateTemplate.creature, e.action.updateTemplate.team ? HORDE : ALLIANCE);
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_UPDATE_TEMPLATE: Creature %u, Template: %u, Team: %u",
                me->GetGUIDLow(), me->GetEntry(), e.action.updateTemplate.team ? HORDE : ALLIANCE);
            break;
        }
        case SMART_ACTION_DIE:
        {
            if (me && !me->isDead())
            {
                me->Kill(me);
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_DIE: Creature %u", me->GetGUIDLow());
            }
            break;
        }
        case SMART_ACTION_SET_IN_COMBAT_WITH_ZONE:
        {
            if (me)
            {
                me->SetInCombatWithZone();
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_SET_IN_COMBAT_WITH_ZONE: Creature %u", me->GetGUIDLow());
            }
            break;
        }
        case SMART_ACTION_CALL_FOR_HELP:
        {
            if (me)
            {
                me->CallForHelp(static_cast<float>(e.action.callHelp.range));
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_CALL_FOR_HELP: Creature %u", me->GetGUIDLow());
            }
            break;
        }
        case SMART_ACTION_SET_SHEATH:
        {
            if (me)
            {
                me->SetSheath(SheathState(e.action.setSheath.sheath));
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction: SMART_ACTION_SET_SHEATH: Creature %u, State: %u",
                    me->GetGUIDLow(), e.action.setSheath.sheath);
            }
            break;
        }
        case SMART_ACTION_FORCE_DESPAWN:
        {
            if (mScriptType == SMART_SCRIPT_TYPE_EVENTOBJECT)
                event->Remove();

            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetDespawnTime(e.action.forceDespawn.delay + 1);//next tick
            CAST_AI(SmartAI, me->AI())->StartDespawn();
            break;
        }
        case SMART_ACTION_SET_INGAME_PHASE_MASK:
        {
            if (GetBaseObject())
                GetBaseObject()->SetPhaseMask(e.action.ingamePhaseMask.mask, true);
            break;
        }
        case SMART_ACTION_MOUNT_TO_ENTRY_OR_MODEL:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (!IsUnit(*itr))
                    continue;

                if (e.action.morphOrMount.creature || e.action.morphOrMount.model)
                {
                    if (e.action.morphOrMount.creature > 0)
                    {
                        if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(e.action.morphOrMount.creature))
                        {
                            uint32 display_id = sObjectMgr->GetCreatureDisplay(sObjectMgr->ChooseDisplayId(0, cInfo));
                            (*itr)->ToUnit()->Mount(display_id);
                        }
                    }
                    else
                        (*itr)->ToUnit()->Mount(e.action.morphOrMount.model);
                }
                else
                    (*itr)->ToUnit()->Dismount();
            }

            delete targets;
            break;
        }
        case SMART_ACTION_SET_INVINCIBILITY_HP_LEVEL:
        {
            if (!me)
                break;

            SmartAI* ai = CAST_AI(SmartAI, me->AI());

            if (!ai)
                break;

            if (e.action.invincHP.percent)
                ai->SetInvincibilityHpLevel(me->CountPctFromMaxHealth(e.action.invincHP.percent));
            else
                ai->SetInvincibilityHpLevel(e.action.invincHP.minHP);
            break;
        }
        case SMART_ACTION_SET_DATA:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsCreature(*itr))
                    (*itr)->ToCreature()->AI()->SetData(e.action.setData.field, e.action.setData.data);
                else if (IsGameObject(*itr))
                    (*itr)->ToGameObject()->AI()->SetData(e.action.setData.field, e.action.setData.data);
            }

            delete targets;
            break;
        }
        case SMART_ACTION_MOVE_FORWARD:
        {
            if (!me)
                break;

            float x, y, z;
            me->GetClosePoint(x, y, z, me->GetObjectSize() / 3, static_cast<float>(e.action.moveRandom.distance));
            me->GetMotionMaster()->MovePoint(SMART_RANDOM_POINT, x, y, z);
            break;
        }
        case SMART_ACTION_SET_VISIBILITY:
        {
            if (me)
                me->SetVisible(e.action.visibility.state != 0);
            break;
        }
        case SMART_ACTION_SET_ACTIVE:
        {
            if (GetBaseObject())
                GetBaseObject()->setActive(true);
            break;
        }
        case SMART_ACTION_ATTACK_START:
        {
            if (!me)
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    me->AI()->AttackStart((*itr)->ToUnit());
                    break;
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_SUMMON_CREATURE:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                float x, y, z, o;
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    (*itr)->GetPosition(x, y, z, o);
                    x += e.target.x;
                    y += e.target.y;
                    z += e.target.z;
                    o += e.target.o;
                    if (Creature* summon = GetBaseObject()->SummonCreature(e.action.summonCreature.creature, x, y, z, o, static_cast<TempSummonType>(e.action.summonCreature.type), e.action.summonCreature.duration))
                    {
                        if (e.action.summonCreature.attackInvoker)
                            summon->AI()->AttackStart((*itr)->ToUnit());
                        if (e.action.summonCreature.phaseByTarget)
                        {
                            summon->SetPhaseMask((*itr)->GetPhaseMask(), true);
                            summon->SetPhaseId((*itr)->GetPhases(), false);
                        }
                    }
                }

                delete targets;
            }

            if (e.GetTargetType() == SMART_TARGET_RANDOM_POSITION)
            {
                float angle = float(rand_norm())*static_cast<float>(2*M_PI);
                float objSize = me->GetObjectSize();
                float dist = objSize + (e.target.randomPos.range - objSize) * static_cast<float>(rand_norm());
                Position pos;
                if (e.target.randomPos.distance)
                {
                    Position center;
                    me->GetNearPoint2D(center, e.target.randomPos.distance, e.target.randomPos.angle * M_PI / 180.0f);
                    center.SimplePosXYRelocationByAngle(pos, dist, angle);
                }
                else
                    me->GetFirstCollisionPosition(pos, dist, angle);
                GetBaseObject()->SummonCreature(e.action.summonCreature.creature, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), angle, static_cast<TempSummonType>(e.action.summonCreature.type), e.action.summonCreature.duration);
            }

            if (e.GetTargetType() == SMART_TARGET_POSITION)
            {
                float x = e.target.x;
                float y = e.target.y;
                float z = e.target.z;

                if (x == 0.0f && y == 0.0f && z != 0.0f)
                {
                    me->GetPosition(x, y);
                    z += me->GetPositionZ();
                }
                if (Creature* summon = GetBaseObject()->SummonCreature(e.action.summonCreature.creature, x, y, z, e.target.o, static_cast<TempSummonType>(e.action.summonCreature.type), e.action.summonCreature.duration))
                    if (unit && e.action.summonCreature.attackInvoker)
                        summon->AI()->AttackStart(unit);
            }
            break;
        }
        case SMART_ACTION_SUMMON_GO:
        {
            if (!GetBaseObject())
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                float x, y, z, o;
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    if (!IsUnit(*itr))
                        continue;

                    (*itr)->GetPosition(x, y, z, o);
                    x += e.target.x;
                    y += e.target.y;
                    z += e.target.z;
                    o += e.target.o;
                    GetBaseObject()->SummonGameObject(e.action.summonGO.entry, x, y, z, o, 0, 0, 0, 0, e.action.summonGO.despawnTime);
                }

                delete targets;
            }

            if (e.GetTargetType() == SMART_TARGET_RANDOM_POSITION)
            {
                float angle = float(rand_norm())*static_cast<float>(2*M_PI);
                float objSize = me->GetObjectSize();
                float dist = objSize + (e.target.randomPos.range - objSize) * static_cast<float>(rand_norm());
                Position pos;
                if (e.target.randomPos.distance)
                {
                    Position center;
                    me->GetNearPoint2D(center, e.target.randomPos.distance, e.target.randomPos.angle * M_PI / 180.0f);
                    center.SimplePosXYRelocationByAngle(pos, dist, angle);
                }
                else
                    me->GetFirstCollisionPosition(pos, dist, angle);
                GetBaseObject()->SummonGameObject(e.action.summonGO.entry, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), angle, 0, 0, 0, 0, e.action.summonGO.despawnTime);
            }

            if (e.GetTargetType() == SMART_TARGET_POSITION)
                GetBaseObject()->SummonGameObject(e.action.summonGO.entry, e.target.x, e.target.y, e.target.z, e.target.o, 0, 0, 0, 0, e.action.summonGO.despawnTime);
            break;
        }
        case SMART_ACTION_KILL_UNIT:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (!IsUnit(*itr))
                    continue;

                (*itr)->ToUnit()->Kill((*itr)->ToUnit());

                if (e.action.raw.param1)
                    if (Creature* npc = (*itr)->ToCreature())
                        npc->AddDelayedEvent(e.action.raw.param2 ? e.action.raw.param2 : 1000, [npc]() -> void { if (npc) npc->Respawn(); });

                if (e.action.raw.param3 && unit && !(*itr)->ToUnit()->ShouldHideFor(unit->GetGUID()))
                    if (Player* player = unit->ToPlayer())
                        if (!(*itr)->ToUnit()->ShouldHideFor(player->GetGUID()))
                            {
                                (*itr)->ToUnit()->AddToHideList(player->GetGUID());
                                (*itr)->ToUnit()->DestroyForPlayer(player);
                            }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_INSTALL_AI_TEMPLATE:
        {
            InstallTemplate(e);
            break;
        }
        case SMART_ACTION_ADD_ITEM:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (!IsPlayer(*itr))
                    continue;

                (*itr)->ToPlayer()->AddItem(e.action.item.entry, e.action.item.count);
            }

            delete targets;
            break;
        }
        case SMART_ACTION_REMOVE_ITEM:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (!IsPlayer(*itr))
                    continue;

                (*itr)->ToPlayer()->DestroyItemCount(e.action.item.entry, e.action.item.count, true);
            }

            delete targets;
            break;
        }
        case SMART_ACTION_STORE_VARIABLE_DECIMAL:
        {
            if (mStoredDecimals.find(e.action.storeVar.id) != mStoredDecimals.end())
                mStoredDecimals.erase(e.action.storeVar.id);
            mStoredDecimals[e.action.storeVar.id] = e.action.storeVar.number;
            break;
        }
        case SMART_ACTION_STORE_TARGET_LIST:
        {
            ObjectList* targets = GetTargets(e, unit);
            StoreTargetList(targets, e.action.storeTargets.id);
            break;
        }
        case SMART_ACTION_TELEPORT:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsPlayer(*itr))
                    (*itr)->ToPlayer()->TeleportTo(e.action.teleport.mapID, e.target.x, e.target.y, e.target.z, e.target.o);
                else if (IsCreature(*itr))
                    (*itr)->ToCreature()->NearTeleportTo(e.target.x, e.target.y, e.target.z, e.target.o);
            }

            delete targets;
            break;
        }
        case SMART_ACTION_SET_FLY:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetFly(e.action.setFly.fly != 0);
            break;
        }
        case SMART_ACTION_SET_RUN:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetRun(e.action.setRun.run != 0);
            break;
        }
        case SMART_ACTION_SET_SWIM:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetSwim(e.action.setSwim.swim != 0);
            break;
        }
        case SMART_ACTION_WP_START:
        {
            if (!IsSmart())
                break;

            bool run = e.action.wpStart.run != 0;
            uint32 entry = e.action.wpStart.pathID;
            bool repeat = e.action.wpStart.repeat != 0;
            ObjectList* targets = GetTargets(e, unit);
            StoreTargetList(targets, SMART_ESCORT_TARGETS);
            me->SetReactState(static_cast<ReactStates>(e.action.wpStart.reactState));
            CAST_AI(SmartAI, me->AI())->StartPath(run, entry, repeat, unit);

            uint32 quest = e.action.wpStart.quest;
            uint32 DespawnTime = e.action.wpStart.despawnTime;
            CAST_AI(SmartAI, me->AI())->mEscortQuestID = quest;
            CAST_AI(SmartAI, me->AI())->SetDespawnTime(DespawnTime);
            break;
        }
        case SMART_ACTION_WP_PAUSE:
        {
            if (!IsSmart())
                break;

            uint32 delay = e.action.wpPause.delay;
            CAST_AI(SmartAI, me->AI())->PausePath(delay, e.GetEventType() == SMART_EVENT_WAYPOINT_REACHED ? false : true);
            break;
        }
        case SMART_ACTION_WP_STOP:
        {
            if (!IsSmart())
                break;

            uint32 DespawnTime = e.action.wpStop.despawnTime;
            uint32 quest = e.action.wpStop.quest;
            bool fail = e.action.wpStop.fail != 0;
            CAST_AI(SmartAI, me->AI())->StopPath(DespawnTime, quest, fail);
            break;
        }
        case SMART_ACTION_WP_RESUME:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->ResumePath();
            break;
        }
        case SMART_ACTION_SET_ORIENTATION:
        {
            if (!me)
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (e.GetTargetType() == SMART_TARGET_SELF)
                me->SetFacingTo(me->GetHomePosition().GetOrientation());
            else if (e.GetTargetType() == SMART_TARGET_POSITION)
                me->SetFacingTo(e.target.o);
            else if (targets && !targets->empty())
                me->SetFacingToObject(*targets->begin());

            delete targets;
            break;
        }
        case SMART_ACTION_PLAYMOVIE:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (!IsPlayer(*itr))
                    continue;

                (*itr)->ToPlayer()->SendMovieStart(e.action.movie.entry);
            }

            delete targets;
            break;
        }
        case SMART_ACTION_MOVE_TO_POS:
        {
            if (!IsSmart())
                break;

            WorldObject* target = nullptr;

            if (e.GetTargetType() == SMART_TARGET_CREATURE_RANGE || e.GetTargetType() == SMART_TARGET_CREATURE_GUID ||
                e.GetTargetType() == SMART_TARGET_CREATURE_DISTANCE || e.GetTargetType() == SMART_TARGET_GAMEOBJECT_RANGE ||
                e.GetTargetType() == SMART_TARGET_GAMEOBJECT_GUID || e.GetTargetType() == SMART_TARGET_GAMEOBJECT_DISTANCE ||
                e.GetTargetType() == SMART_TARGET_CLOSEST_CREATURE || e.GetTargetType() == SMART_TARGET_CLOSEST_GAMEOBJECT ||
                e.GetTargetType() == SMART_TARGET_OWNER_OR_SUMMONER || e.GetTargetType() == SMART_TARGET_ACTION_INVOKER ||
                e.GetTargetType() == SMART_TARGET_CLOSEST_ENEMY || e.GetTargetType() == SMART_TARGET_CLOSEST_FRIENDLY)
            {
                ObjectList* targets = GetTargets(e, unit);
                if (!targets)
                    break;

                target = targets->front();
                delete targets;
            }

            if (!target)
                me->GetMotionMaster()->MovePoint(e.action.MoveToPos.pointId, e.target.x, e.target.y, e.target.z);
            else
                me->GetMotionMaster()->MovePoint(e.action.MoveToPos.pointId, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
            break;
        }
        case SMART_ACTION_RESPAWN_TARGET:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsCreature(*itr))
                    (*itr)->ToCreature()->Respawn();
                else if (IsGameObject(*itr))
                    (*itr)->ToGameObject()->SetRespawnTime(e.action.RespawnTarget.goRespawnTime);
            }

            delete targets;
            break;
        }
        case SMART_ACTION_CLOSE_GOSSIP:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsPlayer(*itr))
                    (*itr)->ToPlayer()->PlayerTalkClass->SendCloseGossip();

            delete targets;
            break;
        }
        case SMART_ACTION_EQUIP:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (Creature* npc = (*itr)->ToCreature())
                {
                    uint32 slot[3];
                    auto equipId = static_cast<int8>(e.action.equip.entry);
                    if (e.action.equip.entry)
                    {
                        EquipmentInfo const* einfo = sObjectMgr->GetEquipmentInfo(npc->GetEntry(), equipId);
                        if (!einfo)
                        {
                            TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: SMART_ACTION_EQUIP uses non-existent equipment info entry %u", e.action.equip.entry);
                            return;
                        }
                        npc->SetCurrentEquipmentId(e.action.equip.entry);
                        slot[0] = einfo->ItemEntry[0];
                        slot[1] = einfo->ItemEntry[1];
                        slot[2] = einfo->ItemEntry[2];
                    }
                    else
                    {
                        slot[0] = e.action.equip.slot1;
                        slot[1] = e.action.equip.slot2;
                        slot[2] = e.action.equip.slot3;
                    }
                    if (!e.action.equip.mask || e.action.equip.mask & 1)
                        npc->SetVirtualItem(0, slot[0]);
                    if (!e.action.equip.mask || e.action.equip.mask & 2)
                        npc->SetVirtualItem(1, slot[1]);
                    if (!e.action.equip.mask || e.action.equip.mask & 4)
                        npc->SetVirtualItem(2, slot[2]);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_CREATE_TIMED_EVENT:
        {
            SmartEvent ne;
            ne.event_phase_mask = 0;
            ne.type = static_cast<SMART_EVENT>(SMART_EVENT_UPDATE);
            ne.event_chance = e.action.timeEvent.chance;
            if (!ne.event_chance) ne.event_chance = 100;

            ne.minMaxRepeat.min = e.action.timeEvent.min;
            ne.minMaxRepeat.max = e.action.timeEvent.max;
            ne.minMaxRepeat.repeatMin = e.action.timeEvent.repeatMin;
            ne.minMaxRepeat.repeatMax = e.action.timeEvent.repeatMax;

            ne.event_flags = 0;
            if (!ne.minMaxRepeat.repeatMin && !ne.minMaxRepeat.repeatMax)
                ne.event_flags |= SMART_EVENT_FLAG_NOT_REPEATABLE;

            SmartAction ac;
            ac.type = static_cast<SMART_ACTION>(SMART_ACTION_TRIGGER_TIMED_EVENT);
            ac.timeEvent.id = e.action.timeEvent.id;

            SmartScriptHolder ev;
            ev.event = ne;
            ev.event_id = e.action.timeEvent.id;
            ev.target = e.target;
            ev.action = ac;
            InitTimer(ev);
            mStoredEvents.push_back(ev);
            break;
        }
        case SMART_ACTION_TRIGGER_TIMED_EVENT:
            ProcessEventsFor(static_cast<SMART_EVENT>(SMART_EVENT_TIMED_EVENT_TRIGGERED), nullptr, e.action.timeEvent.id);
            break;
        case SMART_ACTION_REMOVE_TIMED_EVENT:
            mRemIDs.push_back(e.action.timeEvent.id);
            break;
        case SMART_ACTION_OVERRIDE_SCRIPT_BASE_OBJECT:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (auto& target : *targets)
            {
                if (IsCreature(target))
                {
                    if (meOrigGUID.IsEmpty())
                        meOrigGUID = me ? me->GetGUID() : ObjectGuid::Empty;
                    if (goOrigGUID.IsEmpty())
                        goOrigGUID = go ? go->GetGUID() : ObjectGuid::Empty;
                    go = nullptr;
                    me = target->ToCreature();
                    break;
                }
                if (IsGameObject(target))
                {
                    if (meOrigGUID.IsEmpty())
                        meOrigGUID = me ? me->GetGUID() : ObjectGuid::Empty;
                    if (goOrigGUID.IsEmpty())
                        goOrigGUID = go ? go->GetGUID() : ObjectGuid::Empty;
                    go = target->ToGameObject();
                    me = nullptr;
                    break;
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_RESET_SCRIPT_BASE_OBJECT:
            ResetBaseObject();
            break;
        case SMART_ACTION_CALL_SCRIPT_RESET:
            OnReset();
            break;
        case SMART_ACTION_SET_RANGED_MOVEMENT:
        {
            if (!IsSmart())
                break;

            auto attackDistance = float(e.action.setRangedMovement.distance);
            auto attackAngle = float(e.action.setRangedMovement.angle) / 180.0f * M_PI;

            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (auto& itr : *targets)
                    if (Creature* target = itr->ToCreature())
                        if (IsSmart(target) && target->getVictim())
                            if (CAST_AI(SmartAI, target->AI())->CanCombatMove())
                                target->GetMotionMaster()->MoveChase(target->getVictim(), attackDistance, attackAngle);

                delete targets;
            }
            break;
        }
        case SMART_ACTION_CALL_TIMED_ACTIONLIST:
        {
            if (e.GetTargetType() == SMART_TARGET_NONE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Entry %d SourceType %u Event %u Action %u is using TARGET_NONE(0) for Script9 target. Please correct target_type in database.", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType());
                break;
            }

            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (auto& itr : *targets)
                {
                    if (Creature* target = itr->ToCreature())
                    {
                        if (IsSmart(target))
                            CAST_AI(SmartAI, target->AI())->SetScript9(e, e.action.timedActionList.id, GetLastInvoker());
                    }
                    else if (GameObject* goTarget = itr->ToGameObject())
                    {
                        if (IsSmartGO(goTarget))
                            CAST_AI(SmartGameObjectAI, goTarget->AI())->SetScript9(e, e.action.timedActionList.id, GetLastInvoker());
                    }
                }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_SET_NPC_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, e.action.unitFlag.flag);

            delete targets;
            break;
        }
        case SMART_ACTION_ADD_NPC_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->SetFlag(UNIT_FIELD_NPC_FLAGS, e.action.unitFlag.flag);

            delete targets;
            break;
        }
        case SMART_ACTION_REMOVE_NPC_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->RemoveFlag(UNIT_FIELD_NPC_FLAGS, e.action.unitFlag.flag);

            delete targets;
            break;
        }
        case SMART_ACTION_CROSS_CAST:
        {
            ObjectList* casters = GetTargets(CreateEvent(SMART_EVENT_UPDATE_IC, 0, 0, 0, 0, 0, SMART_ACTION_NONE, 0, 0, 0, 0, 0, 0, static_cast<SMARTAI_TARGETS>(e.action.cast.targetType), e.action.cast.targetParam1, e.action.cast.targetParam2, e.action.cast.targetParam3, 0), unit);
            if (!casters)
                break;

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
            {
                delete casters; // casters already validated, delete now
                break;
            }

            for (ObjectList::const_iterator itr = casters->begin(); itr != casters->end(); ++itr)
            {
                if (IsUnit(*itr))
                {
                    if (e.action.cast.flags & SMARTCAST_INTERRUPT_PREVIOUS)
                        (*itr)->ToUnit()->InterruptNonMeleeSpells(false);

                    for (ObjectList::const_iterator it = targets->begin(); it != targets->end(); ++it)
                    {
                        if (IsUnit(*it))
                        {
                            if (!(e.action.cast.flags & SMARTCAST_AURA_NOT_PRESENT) || !(*it)->ToUnit()->HasAura(e.action.cast.spell))
                                (*itr)->ToUnit()->CastSpell((*it)->ToUnit(), e.action.cast.spell, e.action.cast.flags & SMARTCAST_TRIGGERED ? true : false);
                            else
                                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "Spell %u not casted because it has flag SMARTCAST_AURA_NOT_PRESENT and the target (Guid: %s Entry: %u Type: %u) already has the aura", e.action.cast.spell, (*it)->GetGUID().ToString().c_str(), (*it)->GetEntry(), uint32((*it)->GetTypeId()));
                        }
                    }
                }
            }

            delete targets;
            delete casters;
            break;
        }
        case SMART_ACTION_CALL_RANDOM_TIMED_ACTIONLIST:
        {
            uint32 actions[SMART_ACTION_PARAM_COUNT];
            actions[0] = e.action.randTimedActionList.entry1;
            actions[1] = e.action.randTimedActionList.entry2;
            actions[2] = e.action.randTimedActionList.entry3;
            actions[3] = e.action.randTimedActionList.entry4;
            actions[4] = e.action.randTimedActionList.entry5;
            actions[5] = e.action.randTimedActionList.entry6;
            uint32 temp[SMART_ACTION_PARAM_COUNT];
            uint32 count = 0;
            for (auto action : actions)
            {
                if (action > 0)
                {
                    temp[count] = action;
                    ++count;
                }
            }

            uint32 id = temp[urand(0, count)];
            if (e.GetTargetType() == SMART_TARGET_NONE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Entry %d SourceType %u Event %u Action %u is using TARGET_NONE(0) for Script9 target. Please correct target_type in database.", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType());
                break;
            }

            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (auto& itr : *targets)
                {
                    if (Creature* target = itr->ToCreature())
                    {
                        if (IsSmart(target))
                            CAST_AI(SmartAI, target->AI())->SetScript9(e, id, GetLastInvoker());
                    }
                    else if (GameObject* goTarget = itr->ToGameObject())
                    {
                        if (IsSmartGO(goTarget))
                            CAST_AI(SmartGameObjectAI, goTarget->AI())->SetScript9(e, id, GetLastInvoker());
                    }
                }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_CALL_RANDOM_RANGE_TIMED_ACTIONLIST:
        {
            uint32 id = urand(e.action.randTimedActionList.entry1, e.action.randTimedActionList.entry2);
            if (e.GetTargetType() == SMART_TARGET_NONE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Entry %d SourceType %u Event %u Action %u is using TARGET_NONE(0) for Script9 target. Please correct target_type in database.", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType());
                break;
            }

            ObjectList* targets = GetTargets(e, unit);
            if (targets)
            {
                for (auto& itr : *targets)
                {
                    if (Creature* target = itr->ToCreature())
                    {
                        if (IsSmart(target))
                            CAST_AI(SmartAI, target->AI())->SetScript9(e, id, GetLastInvoker());
                    }
                    else if (GameObject* goTarget = itr->ToGameObject())
                    {
                        if (IsSmartGO(goTarget))
                            CAST_AI(SmartGameObjectAI, goTarget->AI())->SetScript9(e, id, GetLastInvoker());
                    }
                }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_ACTIVATE_TAXI:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsPlayer(*itr))
                    (*itr)->ToPlayer()->ActivateTaxiPathTo(e.action.taxi.id);

            delete targets;
            break;
        }
        case SMART_ACTION_RANDOM_MOVE:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsCreature((*itr)))
                {
                    if (e.action.moveRandom.distance)
                        (*itr)->ToCreature()->GetMotionMaster()->MoveRandom(static_cast<float>(e.action.moveRandom.distance));
                    else
                        (*itr)->ToCreature()->GetMotionMaster()->MoveIdle();
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_SET_UNIT_FIELD_BYTES_1:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;
            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->SetByteFlag(UNIT_FIELD_BYTES_1, e.action.setunitByte.type, e.action.setunitByte.byte1);

            delete targets;
            break;
        }
        case SMART_ACTION_REMOVE_UNIT_FIELD_BYTES_1:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->RemoveByteFlag(UNIT_FIELD_BYTES_1, e.action.delunitByte.type, e.action.delunitByte.byte1);

            delete targets;
            break;
        }
        case SMART_ACTION_INTERRUPT_SPELL:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->InterruptNonMeleeSpells(e.action.interruptSpellCasting.withDelayed != 0, e.action.interruptSpellCasting.spell_id, e.action.interruptSpellCasting.withInstant != 0);

            delete targets;
            break;
        }
        case SMART_ACTION_SEND_GO_CUSTOM_ANIM:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsGameObject(*itr))
                    (*itr)->ToGameObject()->SendCustomAnim(e.action.sendGoCustomAnim.anim);

            delete targets;
            break;
        }
        case SMART_ACTION_SEND_GO_VISUAL_ID:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            if (gob)
            {
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                    if (IsPlayer(*itr))
                        gob->SendGOPlaySpellVisual(e.action.sendGoCustomAnim.anim, (*itr)->ToPlayer()->GetGUID());
            }
            else
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                    if (IsGameObject(*itr))
                        (*itr)->ToGameObject()->SendGOPlaySpellVisual(e.action.sendGoCustomAnim.anim);

            delete targets;
            break;
        }
        case SMART_ACTION_SET_DYNAMIC_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->SetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, e.action.unitFlag.flag);

            delete targets;
            break;
        }
        case SMART_ACTION_ADD_DYNAMIC_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, e.action.unitFlag.flag);

            delete targets;
            break;
        }
        case SMART_ACTION_REMOVE_DYNAMIC_FLAG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsUnit(*itr))
                    (*itr)->ToUnit()->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, e.action.unitFlag.flag);

            delete targets;
            break;
        }
        case SMART_ACTION_JUMP_TO_POS:
        {
            if (!me)
                break;

            me->GetMotionMaster()->Clear();

            if (e.GetTargetType() == SMART_TARGET_ACTION_INVOKER)
                me->GetMotionMaster()->MoveJump(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), static_cast<float>(e.action.jump.speedxy), static_cast<float>(e.action.jump.speedz));
            else
                me->GetMotionMaster()->MoveJump(e.target.x, e.target.y, e.target.z, static_cast<float>(e.action.jump.speedxy), static_cast<float>(e.action.jump.speedz));
            // TODO: Resume path when reached jump location
            break;
        }
        case SMART_ACTION_MOVE_Z:
        {
            if (!me)
                break;

            me->GetMotionMaster()->Clear();
            if(e.action.movetoz.flymode)
            {
                me->SetCanFly(false);
                me->SetDisableGravity(false);
            }
            else
            {
                me->SetCanFly(true);
                me->SetDisableGravity(true);
            }
            float x,y,z,o;
            me->GetPosition(x, y, z, o);
            x += float(e.action.movetoz.targetX);
            y += float(e.action.movetoz.targetY);
            z += float(e.action.movetoz.targetZ);
            me->GetMotionMaster()->MovePoint(0, x, y, z);
            // TODO: Resume path when reached jump location
            break;
        }
        case SMART_ACTION_SET_KD:
        {
            if (!me)
                break;

            auto map = me->GetMap();
            if (!map || !map->ToInstanceMap())
                break;

            map->ApplyOnEveryPlayer([&](Player* player)
            {
                map->ToInstanceMap()->PermBindAllPlayers(player);
            });

            break;
        }
        case SMART_ACTION_GO_SET_LOOT_STATE:
        {
            ObjectList* targets = GetTargets(e, unit);

            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsGameObject(*itr))
                    (*itr)->ToGameObject()->SetLootState(static_cast<LootState>(e.action.setGoLootState.state));

            delete targets;
            break;
        }
        case SMART_ACTION_SEND_TARGET_TO_TARGET:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            ObjectList* storedTargets = GetTargetList(e.action.sendTargetToTarget.id);
            if (!storedTargets)
            {
                delete targets;
                return;
            }

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsCreature(*itr))
                {
                    if (SmartAI* ai = CAST_AI(SmartAI, (*itr)->ToCreature()->AI()))
                        ai->GetScript()->StoreTargetList(new ObjectList(*storedTargets), e.action.sendTargetToTarget.id);   // store a copy of target list
                    else
                        TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Action target for SMART_ACTION_SEND_TARGET_TO_TARGET is not using SmartAI, skipping");
                }
                else if (IsGameObject(*itr))
                {
                    if (SmartGameObjectAI* ai = CAST_AI(SmartGameObjectAI, (*itr)->ToGameObject()->AI()))
                        ai->GetScript()->StoreTargetList(new ObjectList(*storedTargets), e.action.sendTargetToTarget.id);   // store a copy of target list
                    else
                        TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Action target for SMART_ACTION_SEND_TARGET_TO_TARGET is not using SmartGameObjectAI, skipping");
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_SEND_GOSSIP_MENU:
        {
            if (!GetBaseObject())
                break;

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SEND_GOSSIP_MENU: gossipMenuId %d, gossipNpcTextId %d",
                e.action.sendGossipMenu.gossipMenuId, e.action.sendGossipMenu.gossipNpcTextId);

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                {
                    if (e.action.sendGossipMenu.gossipMenuId)
                        player->PrepareGossipMenu(GetBaseObject(), e.action.sendGossipMenu.gossipMenuId, true);
                    else
                        player->PlayerTalkClass->ClearMenus();

                    player->SEND_GOSSIP_MENU(e.action.sendGossipMenu.gossipNpcTextId, GetBaseObject()->GetGUID());
                }

            delete targets;
            break;
        }
        case SMART_ACTION_SET_HOME_POS:
            {
                if (!me)
                    break;

                if (e.GetTargetType() == SMART_TARGET_SELF)
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                else if (e.GetTargetType() == SMART_TARGET_POSITION)
                    me->SetHomePosition(e.target.x, e.target.y, e.target.z, e.target.o);
                else
                    TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Action target for SMART_ACTION_SET_HOME_POS is not using SMART_TARGET_SELF or SMART_TARGET_POSITION, skipping");

                break;
            }
        case SMART_ACTION_SET_HEALTH_REGEN:
        {
            if (!me || !me->IsCreature())
                break;
            me->setRegeneratingHealth(e.action.setHealthRegen.regenHealth != 0);
            break;
        }
        case SMART_ACTION_SET_ROOT:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (IsCreature(*itr))
                    (*itr)->ToCreature()->SetControlled(e.action.setRoot.root != 0, UNIT_STATE_ROOT);

            delete targets;
            break;
        }
        case SMART_ACTION_SUMMON_CREATURE_GROUP:
        {
            if (!me)
                break;
            std::list<TempSummon*> summonList;
            me->SummonCreatureGroup(e.action.creatureGroup.group, &summonList);

            for (std::list<TempSummon*>::const_iterator itr = summonList.begin(); itr != summonList.end(); ++itr)
                if (unit && e.action.creatureGroup.attackInvoker)
                    (*itr)->AI()->AttackStart(unit);

            break;
        }
        case SMART_ACTION_SET_POWER:
        {
            ObjectList* targets = GetTargets(e, unit);

            if (targets)
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                    if (IsUnit(*itr))
                    {
                        (*itr)->ToUnit()->SetPower(Powers(e.action.power.powerType), e.action.power.newPower);
                        if (e.action.power.max_power)
                            (*itr)->ToUnit()->SetMaxPower(Powers(e.action.power.powerType), e.action.power.newPower);
                    }

            delete targets;
            break;
        }
        case SMART_ACTION_ADD_POWER:
        {
            ObjectList* targets = GetTargets(e, unit);

            if (targets)
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                    if (IsUnit(*itr))
                        (*itr)->ToUnit()->SetPower(Powers(e.action.power.powerType), (*itr)->ToUnit()->GetPower(Powers(e.action.power.powerType)) + e.action.power.newPower);

            delete targets;
            break;
        }
        case SMART_ACTION_REMOVE_POWER:
        {
            ObjectList* targets = GetTargets(e, unit);

            if (targets)
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                    if (IsUnit(*itr))
                        (*itr)->ToUnit()->SetPower(Powers(e.action.power.powerType), (*itr)->ToUnit()->GetPower(Powers(e.action.power.powerType)) - e.action.power.newPower);

            delete targets;
            break;
        }
        case SMART_ACTION_SET_SCENATIO_ID:
        {
            if (!GetBaseObject())
                break;

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SET_SCENATIO_ID: scenarioId %d", e.action.scenario.id);

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                    player->SetScenarioId(e.action.scenario.id);

            delete targets;
            break;
        }
        case SMART_ACTION_UPDATE_ACHIEVEMENT_CRITERIA:
        {
            if (!GetBaseObject())
                break;

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_UPDATE_ACHIEVEMENT_CRITERIA: type %d misc1 %d misc2 %d misc3 %d",
                e.action.achievementCriteria.type, e.action.achievementCriteria.misc1, e.action.achievementCriteria.misc2, e.action.achievementCriteria.misc3);

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                    player->UpdateAchievementCriteria(static_cast<CriteriaTypes>(e.action.achievementCriteria.type), e.action.achievementCriteria.misc1, e.action.achievementCriteria.misc2, e.action.achievementCriteria.misc3, unit, true);

            delete targets;
            break;
        }
        case SMART_ACTION_SUMMON_CONVERSATION:
        {
            if (!GetBaseObject())
                break;

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SUMMON_CONVERSATION: Conversation %d", e.action.conversation.id, e.action.conversation.targetX, e.action.conversation.targetY, e.action.conversation.targetZ);

            if (e.action.conversation.targetX || e.action.conversation.targetY || e.action.conversation.targetZ)
            {
                Position pos;
                pos.Relocate(float(e.action.conversation.targetX), float(e.action.conversation.targetY), float(e.action.conversation.targetZ), 0.0f);

                auto conversation = new Conversation;
                if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), e.action.conversation.id, GetBaseObject()->ToUnit(), nullptr, pos))
                    delete conversation;
            }
            else
            {
                ObjectList* targets = GetTargets(e, unit);
                if (!targets)
                    break;

                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                    if (Player* player = (*itr)->ToPlayer())
                    {
                        Position pos;
                        player->GetPosition(&pos);

                        auto conversation = new Conversation;
                        if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), e.action.conversation.id, player, nullptr, pos))
                            delete conversation;
                    }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_SUMMON_ADD_PLR_PERSONNAL_VISIBILE:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!me || !targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                {
                    if (!e.action.personalOrHideVisibility.visibility)
                        me->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                    else
                    {
                        if (!me->ShouldHideFor(player->GetGUID()))
                        {
                            me->AddToHideList(player->GetGUID());
                            me->DestroyForPlayer(player);
                        }
                    }
                }

            delete targets;
            break;
        }
        case SMART_ACTION_SUMMON_CREATURE_IN_PERS_VISIBILITY:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets && !me)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                Position pos;
                pos = e.action.sumCreaturePV.summmonInNPCPosition ? me->GetPosition() : e.target.x, e.target.y, e.target.z, e.target.o;
                if (Player* player = (*itr)->ToPlayer())
                {
                    if (Creature* summon = player->SummonCreature(e.action.sumCreaturePV.creature, pos, static_cast<TempSummonType>(e.action.sumCreaturePV.type), e.action.sumCreaturePV.duration))
                    {
                        summon->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                        summon->AddDelayedEvent(100, [summon] {
                            summon->SetVisible(true);
                        });
                        if (e.action.sumCreaturePV.attackinvoker)
                            summon->AI()->AttackStart((*itr)->ToUnit());
                        if (e.action.sumCreaturePV.getphases)
                        {
                            summon->SetPhaseMask(GetBaseObject()->GetPhaseMask(), true);
                            summon->SetPhaseId(GetBaseObject()->GetPhases(), false);
                        }
                    }
                }
            }
            delete targets;
            break;
        }
        case SMART_ACTION_SET_HEALTH_IN_PERCENT:
        {
            if (!me)
                break;

            me->SetHealth(me->CountPctFromMaxHealth(e.action.setHpPerc.hpvalue));
            break;
        }
        case SMART_ACTION_SUMMON_ARIATRIGGER:
        {
            if (!GetBaseObject())
                break;

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SUMMON_ARIATRIGGER: AreaTrigger %d", e.action.areatrigger.id, e.action.areatrigger.targetX, e.action.areatrigger.targetY, e.action.areatrigger.targetZ);

            if (e.action.areatrigger.targetX || e.action.areatrigger.targetY || e.action.areatrigger.targetZ)
            {
                Position pos;
                pos.Relocate(float(e.action.areatrigger.targetX), float(e.action.areatrigger.targetY), float(e.action.areatrigger.targetZ), 0.0f);

                auto areaTrigger = new AreaTrigger;
                if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), 0, GetBaseObject()->ToUnit(), nullptr, pos, pos, nullptr, ObjectGuid::Empty, e.action.areatrigger.id))
                    delete areaTrigger;
                else if (e.action.areatrigger.duration)
                    areaTrigger->SetDuration(e.action.areatrigger.duration);
            }
            else
            {
                ObjectList* targets = GetTargets(e, unit);
                if (!targets)
                    break;

                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                    if (Player* player = (*itr)->ToPlayer())
                    {
                        Position pos;
                        player->GetPosition(&pos);

                        auto areaTrigger = new AreaTrigger;
                        if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), 0, GetBaseObject()->ToUnit(), nullptr, pos, pos, nullptr, ObjectGuid::Empty, e.action.areatrigger.id))
                            delete areaTrigger;
                        else if (e.action.areatrigger.duration)
                            areaTrigger->SetDuration(e.action.areatrigger.duration);
                    }

                delete targets;
            }
            break;
        }
        case SMART_ACTION_SUMMON_SCENE:
        {
            if (!GetBaseObject())
                break;

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_SUMMON_SCENE: scene %d duration %u", e.action.scene.id, e.action.scene.duration);

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                {
                    Position pos;
                    player->GetPosition(&pos);
                    uint32 sceneId = e.action.scene.id;

                    player->SendSpellScene(sceneId, nullptr, true, &pos);
                    player->AddDelayedEvent(e.action.scene.duration ? e.action.scene.duration : 60000, [player, sceneId]() -> void
                    {
                        if (!player)
                            return;

                        player->SendSpellScene(sceneId, nullptr, false, nullptr);
                    });
                }

            delete targets;
            break;
        }
        case SMART_ACTION_JOIN_LFG:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                {
                    std::set<uint32> Slot;
                    Slot.insert(e.action.joinLfg.id);
                    sLFGMgr->JoinLfg(player, player->GetSpecializationRoleMaskForGroup(), Slot);
                }
            break;
        }
        case SMART_ACTION_DISABLE_EVADE:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetEvadeDisabled(e.action.disableEvade.disable != 0);
            break;
        }
        case SMART_ACTION_SET_CAN_FLY:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetFlyMode(e.action.flyMode.fly != 0);
            break;
        }
        case SMART_ACTION_PLAY_ANIMKIT:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
            {
                if (IsCreature(*itr))
                {
                    if (e.action.animKit.type == 0)
                        (*itr)->ToCreature()->PlayOneShotAnimKit(e.action.animKit.animKit);
                    else if (e.action.animKit.type == 1)
                        (*itr)->ToCreature()->SetAnimKitId(e.action.animKit.animKit);
                    else if (e.action.animKit.type == 2)
                        (*itr)->ToCreature()->SetMeleeAnimKitId(e.action.animKit.animKit);
                    else if (e.action.animKit.type == 3)
                        (*itr)->ToCreature()->SetMovementAnimKitId(e.action.animKit.animKit);
                    else
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Invalid type for SMART_ACTION_PLAY_ANIMKIT, skipping");
                        break;
                    }

                    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_PLAY_ANIMKIT: target: %s (%s), AnimKit: %u, Type: %u",
                        (*itr)->GetName(), (*itr)->GetGUID().ToString().c_str(), e.action.animKit.animKit, e.action.animKit.type);
                }
            }

            delete targets;
            break;
        }

        case SMART_ACTION_CIRCLE_PATH:
        {
            if (ObjectList* targets = GetTargets(e, unit))
            {
                for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                {
                    if (!IsUnit(*itr))
                        continue;

                    me->GetMotionMaster()->MoveCirclePath((*itr)->ToUnit()->GetPositionX(), (*itr)->ToUnit()->GetPositionY(), (*itr)->ToUnit()->GetPositionZ(), static_cast<float>(e.action.moveCirclePath.radius), e.action.moveCirclePath.clockWise, uint8(e.action.moveCirclePath.stepCount));
                }

                delete targets;
            }

            break;
        }
        case SMART_ACTION_SET_OVERRIDE_ZONE_LIGHT:
        {
            if (me)
                me->GetMap()->SetZoneOverrideLight(e.action.setOverrideZoneLight.zoneId, e.action.setOverrideZoneLight.lightId, e.action.setOverrideZoneLight.fadeTime);
            break;
        }

        case SMART_ACTION_SET_SPEED:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (WorldObject* target : *targets)
                if (Unit* unitTarget = target->ToUnit())
                    unitTarget->SetSpeed(UnitMoveType(e.action.setSpeed.type), e.action.setSpeed.speed);

            delete targets;
            break;
        }
        case SMART_ACTION_IGNORE_PATHFINDING:
        {
            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (WorldObject* target : *targets)
            {
                if (Unit* unitTarget = target->ToUnit())
                {
                    if (e.action.ignorePathfinding.ignore)
                        unitTarget->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                    else
                        unitTarget->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                }
            }

            delete targets;
            break;
        }
        case SMART_ACTION_START_TIMED_ACHIEVEMENT:
        {
            if (!GetBaseObject())
                break;

            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::ProcessAction:: SMART_ACTION_START_TIMED_ACHIEVEMENT: type %d misc1 %d misc2 %d misc3 %d",
                e.action.achievementCriteria.type, e.action.achievementCriteria.misc1, e.action.achievementCriteria.misc2, e.action.achievementCriteria.misc3);

            ObjectList* targets = GetTargets(e, unit);
            if (!targets)
                break;

            for (ObjectList::const_iterator itr = targets->begin(); itr != targets->end(); ++itr)
                if (Player* player = (*itr)->ToPlayer())
                {
                    player->GetAchievementMgr()->StartTimedAchievement(static_cast<CriteriaTimedTypes>(e.action.achievementCriteria.type), e.action.achievementCriteria.misc1);
                    InstanceMap* inst = player->GetMap()->ToInstanceMap();
                    if (uint32 instanceId = inst ? inst->GetInstanceId() : 0)
                        if (Scenario* scenario = sScenarioMgr->GetScenario(instanceId))
                            scenario->GetAchievementMgr().StartTimedAchievement(static_cast<CriteriaTimedTypes>(e.action.achievementCriteria.type), e.action.achievementCriteria.misc1);
                }
            delete targets;
            break;
        }

        default:
            TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript::ProcessAction: Entry %d SourceType %u, Event %u, Unhandled Action type %u", e.entryOrGuid, e.GetScriptType(), e.event_id, e.GetActionType());
            break;
    }

    if (e.link && e.link != e.event_id)
    {
        SmartScriptHolder linked = FindLinkedEvent(e.link);
        if (linked.GetActionType() && linked.GetEventType() == SMART_EVENT_LINK)
            ProcessEvent(linked, unit, var0, var1, bvar, spell, gob);
        else
            TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript::ProcessAction: Entry %d SourceType %u, Event %u, Link Event %u not found or invalid, skipped.", e.entryOrGuid, e.GetScriptType(), e.event_id, e.link);
    }
}

void SmartScript::InstallTemplate(SmartScriptHolder const& e)
{
    if (!GetBaseObject())
        return;
    if (mTemplate)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript::InstallTemplate: Entry %d SourceType %u AI Template can not be set more then once, skipped.", e.entryOrGuid, e.GetScriptType());
        return;
    }
    mTemplate = static_cast<SMARTAI_TEMPLATE>(e.action.installTtemplate.id);
    switch (static_cast<SMARTAI_TEMPLATE>(e.action.installTtemplate.id))
    {
        case SMARTAI_TEMPLATE_CASTER:
            {
                AddEvent(SMART_EVENT_UPDATE_IC, 0, 0, 0, e.action.installTtemplate.param2, e.action.installTtemplate.param3, SMART_ACTION_CAST, e.action.installTtemplate.param1, e.target.raw.param1, 0, 0, 0, 0, SMART_TARGET_VICTIM, 0, 0, 0, 1);
                AddEvent(SMART_EVENT_RANGE, 0, e.action.installTtemplate.param4, 300, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 1, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);
                AddEvent(SMART_EVENT_RANGE, 0, 0, e.action.installTtemplate.param4>10?e.action.installTtemplate.param4-10:0, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 0, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);
                AddEvent(SMART_EVENT_CHECK_DIST_TO_HOME, 0, e.action.installTtemplate.param4, 300, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 1, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);
                AddEvent(SMART_EVENT_CHECK_DIST_TO_HOME, 0, 0, e.action.installTtemplate.param4>10?e.action.installTtemplate.param4-10:0, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 0, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);
                AddEvent(SMART_EVENT_MANA_PCT, 0, e.action.installTtemplate.param5-15>100?100:e.action.installTtemplate.param5+15, 100, 1000, 1000, SMART_ACTION_SET_EVENT_PHASE, 1, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 0);
                AddEvent(SMART_EVENT_MANA_PCT, 0, 0, e.action.installTtemplate.param5, 1000, 1000, SMART_ACTION_SET_EVENT_PHASE, 0, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 0);
                AddEvent(SMART_EVENT_MANA_PCT, 0, 0, e.action.installTtemplate.param5, 1000, 1000, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 1, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 0);
                break;
            }
        case SMARTAI_TEMPLATE_TURRET:
            {
                AddEvent(SMART_EVENT_UPDATE_IC, 0, 0, 0, e.action.installTtemplate.param2, e.action.installTtemplate.param3, SMART_ACTION_CAST, e.action.installTtemplate.param1, e.target.raw.param1, 0, 0, 0, 0, SMART_TARGET_VICTIM, 0, 0, 0, 0);
                AddEvent(SMART_EVENT_JUST_CREATED, 0, 0, 0, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 0, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 0);
                break;
            }
        case SMARTAI_TEMPLATE_CAGED_NPC_PART:
            {
                if (!me)
                    return;
                //store cage as id1
                AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_STORE_TARGET_LIST, 1, 0, 0, 0, 0, 0, SMART_TARGET_CLOSEST_GAMEOBJECT, e.action.installTtemplate.param1, 10, 0, 0);

                 //reset(close) cage on hostage(me) respawn
                AddEvent(SMART_EVENT_UPDATE, SMART_EVENT_FLAG_NOT_REPEATABLE, 0, 0, 0, 0, SMART_ACTION_RESET_GOBJECT, 0, 0, 0, 0, 0, 0, SMART_TARGET_GAMEOBJECT_DISTANCE, e.action.installTtemplate.param1, 5, 0, 0);

                AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_SET_RUN, e.action.installTtemplate.param3, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 0);
                AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_SET_EVENT_PHASE, 1, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 0);

                AddEvent(SMART_EVENT_UPDATE, SMART_EVENT_FLAG_NOT_REPEATABLE, 1000, 1000, 0, 0, SMART_ACTION_MOVE_FORWARD, e.action.installTtemplate.param4, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);
                 //phase 1: give quest credit on movepoint reached
                AddEvent(SMART_EVENT_MOVEMENTINFORM, 0, POINT_MOTION_TYPE, SMART_RANDOM_POINT, 0, 0, SMART_ACTION_SET_DATA, 0, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 1, 0, 0, 1);
                //phase 1: despawn after time on movepoint reached
                AddEvent(SMART_EVENT_MOVEMENTINFORM, 0, POINT_MOTION_TYPE, SMART_RANDOM_POINT, 0, 0, SMART_ACTION_FORCE_DESPAWN, e.action.installTtemplate.param2, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);

                if (sCreatureTextMgr->TextExist(me->GetEntry(), static_cast<uint8>(e.action.installTtemplate.param5)))
                    AddEvent(SMART_EVENT_MOVEMENTINFORM, 0, POINT_MOTION_TYPE, SMART_RANDOM_POINT, 0, 0, SMART_ACTION_TALK, e.action.installTtemplate.param5, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);

                AddEvent(SMART_EVENT_MOVEMENTINFORM, 0, POINT_MOTION_TYPE, SMART_RANDOM_POINT, 0, 0, SMART_ACTION_BOSS_ANOUNCE, e.action.installTtemplate.param5, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 1);
                break;
            }
        case SMARTAI_TEMPLATE_CAGED_GO_PART:
            {
                if (!go)
                    return;
                //store hostage as id1
                AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_STORE_TARGET_LIST, 1, 0, 0, 0, 0, 0, SMART_TARGET_CLOSEST_CREATURE, e.action.installTtemplate.param1, 10, 0, 0);
                //store invoker as id2
                AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_STORE_TARGET_LIST, 2, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, 0);
                //signal hostage
                AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_SET_DATA, 0, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 1, 0, 0, 0);
                //when hostage raeched end point, give credit to invoker
                if (e.action.installTtemplate.param2)
                    AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_CALL_KILLEDMONSTER, e.action.installTtemplate.param1, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 2, 0, 0, 0);
                else
                    AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_CALL_KILLEDMONSTER, e.action.installTtemplate.param1, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 2, 0, 0, 0);
                break;
            }
        case SMARTAI_TEMPLATE_BASIC:
        default:
            return;
    }
}

void SmartScript::AddEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, uint32 phaseMask)
{
    mInstallEvents.push_back(CreateEvent(e, event_flags, event_param1, event_param2, event_param3, event_param4, action, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, t, target_param1, target_param2, target_param3, phaseMask));
}

void SmartScript::SetPathId(uint32 id)
{
    mPathId = id;
}

uint32 SmartScript::GetPathId() const
{
    return mPathId;
}

WorldObject* SmartScript::GetBaseObject()
{
    WorldObject* obj = nullptr;
    if (me)
        obj = me;
    else if (go)
        obj = go;
    else if (event)
        obj = event;
    return obj;
}

bool SmartScript::IsUnit(WorldObject* obj)
{
    return obj && (obj->IsCreature() || obj->IsPlayer());
}

bool SmartScript::IsPlayer(WorldObject* obj)
{
    return obj && obj->IsPlayer();
}

bool SmartScript::IsCreature(WorldObject* obj)
{
    return obj && obj->IsCreature();
}

bool SmartScript::IsGameObject(WorldObject* obj)
{
    return obj && obj->IsGameObject();
}

SmartScriptHolder SmartScript::CreateEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, uint32 phaseMask)
{
    SmartScriptHolder script;
    script.event.type = e;
    script.event.raw.param1 = event_param1;
    script.event.raw.param2 = event_param2;
    script.event.raw.param3 = event_param3;
    script.event.raw.param4 = event_param4;
    script.event.event_phase_mask = phaseMask;
    script.event.event_flags = event_flags;

    script.action.type = action;
    script.action.raw.param1 = action_param1;
    script.action.raw.param2 = action_param2;
    script.action.raw.param3 = action_param3;
    script.action.raw.param4 = action_param4;
    script.action.raw.param5 = action_param5;
    script.action.raw.param6 = action_param6;

    script.target.type = t;
    script.target.raw.param1 = target_param1;
    script.target.raw.param2 = target_param2;
    script.target.raw.param3 = target_param3;

    script.source_type = SMART_SCRIPT_TYPE_CREATURE;
    InitTimer(script);
    return script;
}

ObjectList* SmartScript::GetTargets(SmartScriptHolder const& e, Unit* invoker /*= NULL*/)
{
    Unit* trigger = nullptr;
    if (invoker)
        trigger = invoker;
    else if (Unit* tempLastInvoker = GetLastInvoker())
        trigger = tempLastInvoker;

    auto l = new ObjectList();
    switch (e.GetTargetType())
    {
        case SMART_TARGET_SELF:
            if (GetBaseObject())
                l->push_back(GetBaseObject());
            break;
        case SMART_TARGET_VICTIM:
            if (me && me->getVictim())
                l->push_back(me->getVictim());
            break;
        case SMART_TARGET_HOSTILE_SECOND_AGGRO:
            if (me)
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 1))
                    l->push_back(u);
            break;
        case SMART_TARGET_HOSTILE_LAST_AGGRO:
            if (me)
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_BOTTOMAGGRO, 0))
                    l->push_back(u);
            break;
        case SMART_TARGET_HOSTILE_RANDOM:
            if (me)
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0))
                    l->push_back(u);
            break;
        case SMART_TARGET_HOSTILE_RANDOM_NOT_TOP:
            if (me)
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1))
                    l->push_back(u);
            break;
        case SMART_TARGET_HOSTILE_RANDOM_PLAYER:
            if (me)
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    l->push_back(u);
            break;
        case SMART_TARGET_HOSTILE_RANDOM_NOT_TOP_PLAYER:
            if (me)
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                    l->push_back(u);
            break;
        case SMART_TARGET_HOSTILE_RANDOM_AURA:
            if (me)
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, e.target.spell.topornot, static_cast<float>(e.target.spell.dist), true, e.target.spell.entry))
                    l->push_back(u);
            break;
        case SMART_TARGET_NONE:
        case SMART_TARGET_ACTION_INVOKER:
            if (trigger)
                l->push_back(trigger);
            break;
        case SMART_TARGET_ACTION_INVOKER_VEHICLE:
            if (trigger && trigger->GetVehicle() && trigger->GetVehicle()->GetBase())
                l->push_back(trigger->GetVehicle()->GetBase());
            break;
        case SMART_TARGET_INVOKER_PARTY:
            if (trigger)
            {
                if (Player* player = trigger->ToPlayer())
                {
                    if (Group* group = player->GetGroup())
                    {
                        for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                            if (Player* member = groupRef->getSource())
                                l->push_back(member);
                    }
                    // We still add the player to the list if there is no group. If we do
                    // this even if there is a group (thus the else-check), it will add the
                    // same player to the list twice. We don't want that to happen.
                    else
                        l->push_back(trigger);
                }
            }
            break;
        case SMART_TARGET_CREATURE_RANGE:
        {
            // will always return a valid pointer, even if empty list
            ObjectList* units = GetWorldObjectsInDist(static_cast<float>(e.target.unitRange.maxDist));
            for (ObjectList::const_iterator itr = units->begin(); itr != units->end(); ++itr)
            {
                if (!IsCreature(*itr))
                    continue;

                if (me && me == *itr)
                    continue;

                if (((e.target.unitRange.creature && (*itr)->ToCreature()->GetEntry() == e.target.unitRange.creature) || !e.target.unitRange.creature) && GetBaseObject()->IsInRange(*itr, static_cast<float>(e.target.unitRange.minDist), static_cast<float>(e.target.unitRange.maxDist)))
                    l->push_back(*itr);
            }

            delete units;
            break;
        }
        case SMART_TARGET_CREATURE_DISTANCE:
        {
            // will always return a valid pointer, even if empty list
            ObjectList* units = GetWorldObjectsInDist(static_cast<float>(e.target.unitDistance.dist));
            for (ObjectList::const_iterator itr = units->begin(); itr != units->end(); ++itr)
            {
                if (!IsCreature(*itr))
                    continue;

                if (me && me == *itr)
                    continue;

                if ((e.target.unitDistance.creature && (*itr)->ToCreature()->GetEntry() == e.target.unitDistance.creature) || !e.target.unitDistance.creature)
                    l->push_back(*itr);
            }

            delete units;
            break;
        }
        case SMART_TARGET_GAMEOBJECT_DISTANCE:
        {
            // will always return a valid pointer, even if empty list
            ObjectList* units = GetWorldObjectsInDist(static_cast<float>(e.target.goDistance.dist));
            for (ObjectList::const_iterator itr = units->begin(); itr != units->end(); ++itr)
            {
                if (!IsGameObject(*itr))
                    continue;

                if (go && go == *itr)
                    continue;

                if ((e.target.goDistance.entry && (*itr)->ToGameObject()->GetEntry() == e.target.goDistance.entry) || !e.target.goDistance.entry)
                    l->push_back(*itr);
            }

            delete units;
            break;
        }
        case SMART_TARGET_GAMEOBJECT_RANGE:
        {
            // will always return a valid pointer, even if empty list
            ObjectList* units = GetWorldObjectsInDist(static_cast<float>(e.target.goRange.maxDist));
            for (ObjectList::const_iterator itr = units->begin(); itr != units->end(); ++itr)
            {
                if (!IsGameObject(*itr))
                    continue;

                if (go && go == *itr)
                    continue;

                if (((e.target.goRange.entry && IsGameObject(*itr) && (*itr)->ToGameObject()->GetEntry() == e.target.goRange.entry) || !e.target.goRange.entry) && GetBaseObject()->IsInRange((*itr), static_cast<float>(e.target.goRange.minDist), static_cast<float>(e.target.goRange.maxDist)))
                    l->push_back(*itr);
            }

            delete units;
            break;
        }
        case SMART_TARGET_CREATURE_GUID:
        {
            Creature* target = nullptr;

            if (!trigger && !GetBaseObject())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SMART_TARGET_CREATURE_GUID can not be used without invoker and without entry");
                break;
            }

            target = FindCreatureNear(trigger ? trigger : GetBaseObject(), e.target.unitGUID.guid);

            if (target)
                l->push_back(target);
            break;
        }
        case SMART_TARGET_GAMEOBJECT_GUID:
        {
            GameObject* target = nullptr;

            if (!trigger && !GetBaseObject())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SMART_TARGET_GAMEOBJECT_GUID can not be used without invoker and without entry");
                break;
            }

            target = FindGameObjectNear(trigger ? trigger : GetBaseObject(), e.target.goGUID.guid);

            if (target)
                l->push_back(target);
            break;
        }
        case SMART_TARGET_PLAYER_RANGE:
        {
            // will always return a valid pointer, even if empty list
            ObjectList* units = GetWorldObjectsInDist(static_cast<float>(e.target.playerRange.maxDist));
            if (!units->empty() && GetBaseObject())
                for (ObjectList::const_iterator itr = units->begin(); itr != units->end(); ++itr)
                    if (IsPlayer(*itr) && GetBaseObject()->IsInRange(*itr, static_cast<float>(e.target.playerRange.minDist), static_cast<float>(e.target.playerRange.maxDist)))
                        l->push_back(*itr);

            delete units;
            break;
        }
        case SMART_TARGET_PLAYER_DISTANCE:
        {
            // will always return a valid pointer, even if empty list
            ObjectList* units = GetWorldObjectsInDist(static_cast<float>(e.target.playerDistance.dist));
            for (ObjectList::const_iterator itr = units->begin(); itr != units->end(); ++itr)
                if (IsPlayer(*itr))
                {
                    if(e.target.playerDistance.spell && (*itr)->ToUnit()->HasAura(e.target.playerDistance.spell))
                        l->push_back(*itr);
                    else if(!e.target.playerDistance.spell)
                        l->push_back(*itr);
                }

            delete units;
            break;
        }
        case SMART_TARGET_STORED:
        {
            auto itr = mTargetStorage->find(e.target.stored.id);
            if (itr != mTargetStorage->end())
                l->assign(itr->second->begin(), itr->second->end());

            return l;
        }
        case SMART_TARGET_CLOSEST_CREATURE:
        {
            Creature* target = GetClosestCreatureWithEntry(GetBaseObject(), e.target.closest.entry, static_cast<float>(e.target.closest.dist ? e.target.closest.dist : 100), e.target.closest.dead ? false : true);
            if (target)
                l->push_back(target);
            break;
        }
        case SMART_TARGET_CLOSEST_GAMEOBJECT:
        {
            GameObject* target = GetClosestGameObjectWithEntry(GetBaseObject(), e.target.closest.entry, static_cast<float>(e.target.closest.dist ? e.target.closest.dist : 100));
            if (target)
                l->push_back(target);
            break;
        }
        case SMART_TARGET_CLOSEST_PLAYER:
        {
            if (me)
            {
                Player* target = me->SelectNearestPlayer(static_cast<float>(e.target.playerDistance.dist));
                if (target)
                    l->push_back(target);
            }
            break;
        }
        case SMART_TARGET_OWNER_OR_SUMMONER:
        {
            if (me && me->GetAnyOwner())
                if (Unit* owner = me->GetAnyOwner())
                    l->push_back(owner);
            break;
        }
        case SMART_TARGET_THREAT_LIST:
        {
            if (me)
            {
                std::list<HostileReference*> const& threatList = me->getThreatManager().getThreatList();
                for (auto i : threatList)
                    if (Unit* temp = Unit::GetUnit(*me, i->getUnitGuid()))
                        l->push_back(temp);
            }
            break;
        }
        case SMART_TARGET_CLOSEST_ENEMY:
        {
            if (me)
                if (Unit* target = me->SelectNearestTarget(e.target.closestAttackable.maxDist))
                    l->push_back(target);

            break;
        }
        case SMART_TARGET_CLOSEST_FRIENDLY:
        {
            if (me)
                if (Unit* target = DoFindClosestFriendlyInRange(e.target.closestFriendly.maxDist))
                    l->push_back(target);

            break;
        }
        case SMART_TARGET_TARGETUNIT:
        {
            if (me)
                if (Unit* target = me->GetTargetUnit())
                    l->push_back(target);
            
            break;
        }
        case SMART_TARGET_POSITION:
        case SMART_TARGET_RANDOM_POSITION:
        default:
            break;
    }

    if (l->empty())
    {
        delete l;
        l = nullptr;
    }

    return l;
}

ObjectList* SmartScript::GetWorldObjectsInDist(float dist)
{
    auto targets = new ObjectList();
    WorldObject* obj = GetBaseObject();
    if (obj)
    {
        Trinity::AllWorldObjectsInRange u_check(obj, dist);
        Trinity::WorldObjectListSearcher<Trinity::AllWorldObjectsInRange> searcher(obj, *targets, u_check);
        Trinity::VisitNearbyObject(obj, dist, searcher);
    }
    return targets;
}

void SmartScript::ProcessEvent(SmartScriptHolder& e, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob)
{
    if (!e.active && e.GetEventType() != SMART_EVENT_LINK)
        return;

    if (me && (me->isInCombat() && (e.event.event_flags & SMART_EVENT_FLAG_EVENT_NON_COMBAT)))
        return;

    if ((e.event.event_phase_mask && !IsInPhase(e.event.event_phase_mask)) || ((e.event.event_flags & SMART_EVENT_FLAG_NOT_REPEATABLE) && e.runOnce))
        return;

    ConditionList conds = sConditionMgr->GetConditionsForSmartEvent(e.entryOrGuid, e.event_id, e.source_type);
    ConditionSourceInfo info = ConditionSourceInfo(unit ? unit : GetBaseObject(), GetBaseObject());
    if(!sConditionMgr->IsObjectMeetToConditions(info, conds))
        return;

    switch (e.GetEventType())
    {
        case SMART_EVENT_LINK: //special handling
            ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            break;
        //called from Update tick
        case SMART_EVENT_UPDATE:
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e);
            break;
        case SMART_EVENT_UPDATE_OOC:
            if (me && me->isInCombat())
                return;
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e);
            break;
        case SMART_EVENT_UPDATE_IC:
            if (!me || !me->isInCombat())
                return;
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e);
            break;
        case SMART_EVENT_HEALT_PCT:
        {
            if (!me || (!me->isInCombat() && !(e.event.event_flags & SMART_EVENT_FLAG_ALLOW_EVENT_IN_COMBAT)) || !me->GetMaxHealth())
                return;
            auto perc = static_cast<uint32>(me->GetHealthPct());
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e);
            break;
        }
        case SMART_EVENT_TARGET_HEALTH_PCT:
        {
            if (!me || (!me->isInCombat() && !(e.event.event_flags & SMART_EVENT_FLAG_ALLOW_EVENT_IN_COMBAT)) || !me->getVictim() || !me->getVictim()->GetMaxHealth())
                return;
            auto perc = static_cast<uint32>(me->getVictim()->GetHealthPct());
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e, me->getVictim());
            break;
        }
        case SMART_EVENT_MANA_PCT:
        {
            if (!me || (!me->isInCombat() && !(e.event.event_flags & SMART_EVENT_FLAG_ALLOW_EVENT_IN_COMBAT)) || !me->GetMaxPower(me->getPowerType()))
                return;
            auto perc = uint32(100.0f * me->GetPower(me->getPowerType()) / me->GetMaxPower(me->getPowerType()));
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e);
            break;
        }
        case SMART_EVENT_TARGET_MANA_PCT:
        {
            if (!me || (!me->isInCombat() && !(e.event.event_flags & SMART_EVENT_FLAG_ALLOW_EVENT_IN_COMBAT)) || !me->getVictim() || !me->getVictim()->GetMaxPower(me->getPowerType()))
                return;
            auto perc = uint32(100.0f * me->getVictim()->GetPower(me->getPowerType()) / me->getVictim()->GetMaxPower(me->getPowerType()));
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e, me->getVictim());
            break;
        }
        case SMART_EVENT_RANGE:
        {
            if (!me || !me->isInCombat() || !me->getVictim())
                return;

            if (me->IsInRange(me->getVictim(), static_cast<float>(e.event.minMaxRepeat.min), static_cast<float>(e.event.minMaxRepeat.max)))
            {
                ProcessAction(e, me->getVictim());
                RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            }
            break;
        }
        case SMART_EVENT_TARGET_CASTING:
        {
            if (!me || !me->isInCombat() || !me->getVictim() || !me->getVictim()->IsNonMeleeSpellCast(false, false, true))
                return;
            ProcessAction(e, me->getVictim());
            RecalcTimer(e, e.event.minMax.repeatMin, e.event.minMax.repeatMax);
        }
        case SMART_EVENT_FRIENDLY_HEALTH:
        {
            if (!me || !me->isInCombat())
                return;

            Unit* target = DoSelectLowestHpFriendly(static_cast<float>(e.event.friendlyHealt.radius), e.event.friendlyHealt.hpDeficit);
            if (!target)
                return;
            ProcessAction(e, target);
            RecalcTimer(e, e.event.friendlyHealt.repeatMin, e.event.friendlyHealt.repeatMax);
            break;
        }
        case SMART_EVENT_FRIENDLY_IS_CC:
        {
            if (!me || !me->isInCombat())
                return;

            std::list<Creature*> pList;
            DoFindFriendlyCC(pList, static_cast<float>(e.event.friendlyCC.radius));
            if (pList.empty())
                return;
            ProcessAction(e, *(pList.begin()));
            RecalcTimer(e, e.event.friendlyCC.repeatMin, e.event.friendlyCC.repeatMax);
            break;
        }
        case SMART_EVENT_FRIENDLY_MISSING_BUFF:
        {
            std::list<Creature*> pList;
            DoFindFriendlyMissingBuff(pList, static_cast<float>(e.event.missingBuff.radius), e.event.missingBuff.spell);

            if (pList.empty())
                return;
            ProcessAction(e, *(pList.begin()));
            RecalcTimer(e, e.event.missingBuff.repeatMin, e.event.missingBuff.repeatMax);
            break;
        }
        case SMART_EVENT_HAS_AURA:
        {
            if (!me)
                return;
            uint32 count = me->GetAuraCount(e.event.aura.spell);
            if ((!e.event.aura.count && !count) || (e.event.aura.count && count >= e.event.aura.count))
            {
                ProcessAction(e);
                RecalcTimer(e, e.event.aura.repeatMin, e.event.aura.repeatMax);
            }
            break;
        }
        case SMART_EVENT_TARGET_BUFFED:
        {
            if (!me || !me->getVictim())
                return;
            uint32 count = me->getVictim()->GetAuraCount(e.event.aura.spell);
            if (count < e.event.aura.count)
                return;
            ProcessAction(e);
            RecalcTimer(e, e.event.aura.repeatMin, e.event.aura.repeatMax);
            break;
        }
        case SMART_EVENT_QUEST_ACCEPTED:
        case SMART_EVENT_QUEST_REWARDED:
        {
            if (!me || var0 != e.event.raw.param1)
                return;
            ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            break;
        }
        //no params
        case SMART_EVENT_AGGRO:
        case SMART_EVENT_DEATH:
        case SMART_EVENT_EVADE:
        case SMART_EVENT_REACHED_HOME:
        case SMART_EVENT_CHARMED:
        case SMART_EVENT_CHARMED_TARGET:
        case SMART_EVENT_CORPSE_REMOVED:
        case SMART_EVENT_AI_INIT:
        case SMART_EVENT_TRANSPORT_ADDPLAYER:
        case SMART_EVENT_TRANSPORT_REMOVE_PLAYER:
        case SMART_EVENT_QUEST_OBJ_COPLETETION:
        case SMART_EVENT_QUEST_COMPLETION:
        case SMART_EVENT_QUEST_FAIL:
        case SMART_EVENT_JUST_SUMMONED:
        case SMART_EVENT_RESET:
        case SMART_EVENT_JUST_CREATED:
        case SMART_EVENT_GOSSIP_HELLO:
        case SMART_EVENT_FOLLOW_COMPLETED:
            ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            break;
        case SMART_EVENT_IS_BEHIND_TARGET:
            {
                if (!me)
                    return;

                if (Unit* victim = me->getVictim())
                {
                    if (!victim->HasInArc(static_cast<float>(M_PI), me))
                    {
                        ProcessAction(e, victim);
                        RecalcTimer(e, e.event.behindTarget.cooldownMin, e.event.behindTarget.cooldownMax);
                    }
                }
                break;
            }
        case SMART_EVENT_ON_SPELLCLICK:
            ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            RecalcTimer(e, e.event.spellclick.cooldownMin, e.event.spellclick.cooldownMax);
            break;
        case SMART_EVENT_RECEIVE_EMOTE:
            if (e.event.emote.emote == var0)
            {
                ProcessAction(e, unit);
                RecalcTimer(e, e.event.emote.cooldownMin, e.event.emote.cooldownMax);
            }
            break;
        case SMART_EVENT_KILL:
        {
            if (!me || !unit)
                return;
            if (e.event.kill.playerOnly && !unit->IsPlayer())
                return;
            if (e.event.kill.creature && unit->GetEntry() != e.event.kill.creature)
                return;
            ProcessAction(e, unit);
            RecalcTimer(e, e.event.kill.cooldownMin, e.event.kill.cooldownMax);
            break;
        }
        case SMART_EVENT_SPELLHIT_TARGET:
        case SMART_EVENT_SPELLHIT:
        {
            if (!spell)
                return;
            if ((!e.event.spellHit.spell || spell->Id == e.event.spellHit.spell) &&
                (!e.event.spellHit.school || (spell->GetMisc()->MiscData.SchoolMask & e.event.spellHit.school)))
                {
                    ProcessAction(e, unit, 0, 0, bvar, spell);
                    RecalcTimer(e, e.event.spellHit.cooldownMin, e.event.spellHit.cooldownMax);
                }
            break;
        }
        case SMART_EVENT_OOC_LOS:
        {
            if (!me || me->isInCombat())
                return;
            //can trigger if closer than fMaxAllowedRange
            auto range = static_cast<float>(e.event.los.maxDist);

            //if range is ok and we are actually in LOS
            if (me->IsWithinDistInMap(unit, range) && me->IsWithinLOSInMap(unit))
            {
                //if friendly event&&who is not hostile OR hostile event&&who is hostile
                if ((e.event.los.noHostile && !me->IsHostileTo(unit)) ||
                    (!e.event.los.noHostile && me->IsHostileTo(unit)))
                {
                    ProcessAction(e, unit);
                    RecalcTimer(e, e.event.los.cooldownMin, e.event.los.cooldownMax);
                }
            }
            break;
        }
        case SMART_EVENT_IC_LOS:
        {
            if (!me || !me->isInCombat())
                return;
            //can trigger if closer than fMaxAllowedRange
            auto range = static_cast<float>(e.event.los.maxDist);

            //if range is ok and we are actually in LOS
            if (me->IsWithinDistInMap(unit, range) && me->IsWithinLOSInMap(unit))
            {
                //if friendly event&&who is not hostile OR hostile event&&who is hostile
                if ((e.event.los.noHostile && !me->IsHostileTo(unit)) ||
                    (!e.event.los.noHostile && me->IsHostileTo(unit)))
                {
                    ProcessAction(e, unit);
                    RecalcTimer(e, e.event.los.cooldownMin, e.event.los.cooldownMax);
                }
            }
            break;
        }
        case SMART_EVENT_RESPAWN:
        {
            if (!GetBaseObject())
                return;
            if (e.event.respawn.type == SMART_SCRIPT_RESPAWN_CONDITION_MAP && GetBaseObject()->GetMapId() != e.event.respawn.map)
                return;
            if (e.event.respawn.type == SMART_SCRIPT_RESPAWN_CONDITION_AREA && GetBaseObject()->GetCurrentZoneID() != e.event.respawn.area)
                return;
            ProcessAction(e);
            break;
        }
        case SMART_EVENT_SUMMONED_UNIT:
        {
            if (!IsCreature(unit))
                return;
            if (e.event.summoned.creature && unit->GetEntry() != e.event.summoned.creature)
                return;
            ProcessAction(e, unit);
            RecalcTimer(e, e.event.summoned.cooldownMin, e.event.summoned.cooldownMax);
            break;
        }
        case SMART_EVENT_RECEIVE_HEAL:
        case SMART_EVENT_DAMAGED:
        case SMART_EVENT_DAMAGED_TARGET:
        {
            if (var0 > e.event.minMaxRepeat.max || var0 < e.event.minMaxRepeat.min)
                return;
            ProcessAction(e, unit);
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            break;
        }
        case SMART_EVENT_MOVEMENTINFORM:
        {
            if ((e.event.movementInform.type && var0 != e.event.movementInform.type) || (e.event.movementInform.id && var1 != e.event.movementInform.id))
                return;
            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_TRANSPORT_RELOCATE:
        case SMART_EVENT_WAYPOINT_START:
        {
            if (e.event.waypoint.pathID && var0 != e.event.waypoint.pathID)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_WAYPOINT_REACHED:
        case SMART_EVENT_WAYPOINT_RESUMED:
        case SMART_EVENT_WAYPOINT_PAUSED:
        case SMART_EVENT_WAYPOINT_STOPPED:
        case SMART_EVENT_WAYPOINT_ENDED:
        {
            if (!me || (e.event.waypoint.pointID && var0 != e.event.waypoint.pointID) || (e.event.waypoint.pathID && GetPathId() != e.event.waypoint.pathID))
                return;
            ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_SUMMON_DESPAWNED:
        case SMART_EVENT_INSTANCE_PLAYER_ENTER:
        {
            if (e.event.instancePlayerEnter.team && var0 != e.event.instancePlayerEnter.team)
                return;
            ProcessAction(e, unit, var0);
            RecalcTimer(e, e.event.instancePlayerEnter.cooldownMin, e.event.instancePlayerEnter.cooldownMax);
            break;
        }
        case SMART_EVENT_ACCEPTED_QUEST:
        case SMART_EVENT_REWARD_QUEST:
        {
            if (e.event.quest.quest && var0 != e.event.quest.quest)
                return;
            ProcessAction(e, unit, var0);
            RecalcTimer(e, e.event.quest.cooldownMin, e.event.quest.cooldownMax);
            break;
        }
        case SMART_EVENT_TRANSPORT_ADDCREATURE:
        {
            if (e.event.transportAddCreature.creature && var0 != e.event.transportAddCreature.creature)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_AREATRIGGER_ONTRIGGER:
        {
            if (e.event.areatrigger.id && var0 != e.event.areatrigger.id)
                return;
            ProcessAction(e, unit, var0);
            RecalcTimer(e, e.event.areatrigger.cooldownMin, e.event.areatrigger.cooldownMax);
            break;
        }
        case SMART_EVENT_TEXT_OVER:
        {
            if (var0 != e.event.textOver.textGroupID || (e.event.textOver.creatureEntry && e.event.textOver.creatureEntry != var1))
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_DATA_SET:
        {
            if (e.event.dataSet.id != var0 || e.event.dataSet.value != var1)
                return;
            ProcessAction(e, unit, var0, var1);
            RecalcTimer(e, e.event.dataSet.cooldownMin, e.event.dataSet.cooldownMax);
            break;
        }
        case SMART_EVENT_PASSENGER_REMOVED:
        case SMART_EVENT_PASSENGER_BOARDED:
        {
            if (!unit)
                return;
            ProcessAction(e, unit);
            RecalcTimer(e, e.event.minMax.repeatMin, e.event.minMax.repeatMax);
            break;
        }
        case SMART_EVENT_TIMED_EVENT_TRIGGERED:
        {
            if (e.event.timedEvent.id == var0)
                ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_GOSSIP_SELECT:
        {
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript: Gossip Select:  menu %u action %u", var0, var1);//little help for scripters
            if (e.event.gossip.sender != var0 || e.event.gossip.action != var1)
                return;
            ProcessAction(e, unit, var0, var1);
            RecalcTimer(e, e.event.gossip.cooldownMin, e.event.gossip.cooldownMax);
            break;
        }
        case SMART_EVENT_DUMMY_EFFECT:
        {
            if (e.event.dummy.spell != var0 || e.event.dummy.effIndex != var1)
                return;
            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_GAME_EVENT_START:
        case SMART_EVENT_GAME_EVENT_END:
        {
            if (e.event.gameEvent.gameEventId != var0)
                return;
            ProcessAction(e, nullptr, var0);
            break;
        }
        case SMART_EVENT_GO_STATE_CHANGED:
        {
            if (e.event.goStateChanged.state != var0)
                return;
            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_GO_EVENT_INFORM:
        {
            if (e.event.eventInform.eventId != var0)
                return;
            ProcessAction(e, nullptr, var0);
            break;
        }
        case SMART_EVENT_ACTION_DONE:
        {
            if (e.event.doAction.eventId != var0)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_CHECK_DIST_TO_HOME:
        {
            if (!me || !me->isInCombat() || !me->getVictim())
                return;

            Position const& _homePosition = me->GetHomePosition();
            if (me->GetDistance2d(_homePosition.GetPositionX(), _homePosition.GetPositionY()) > static_cast<float>(e.event.dist.maxDist))
            {
                ProcessAction(e, me->getVictim());
                RecalcTimer(e, e.event.dist.repeatMin, e.event.dist.repeatMax);
            }
            break;
        }
        case SMART_EVENT_EVENTOBJECT_ONTRIGGER:
        case SMART_EVENT_EVENTOBJECT_OFFTRIGGER:
        {
            if (e.event.areatrigger.id && var0 != e.event.areatrigger.id)
                return;
            ProcessAction(e, unit, var0);
            RecalcTimer(e, e.event.areatrigger.cooldownMin, e.event.areatrigger.cooldownMax);
            break;
        }
        case SMART_EVENT_ON_APPLY_OR_REMOVE_AURA:
        {
            if (e.event.applyorremoveaura.spellId && var0 == e.event.applyorremoveaura.spellId && e.event.applyorremoveaura.apply && bvar)
            {
                ProcessAction(e, unit, var0);
                RecalcTimer(e, e.event.applyorremoveaura.cooldown, e.event.applyorremoveaura.cooldown);
            }
            else if (e.event.applyorremoveaura.spellId && var0 == e.event.applyorremoveaura.spellId && !e.event.applyorremoveaura.apply && !bvar)
            {
                ProcessAction(e, unit, var0);
                RecalcTimer(e, e.event.applyorremoveaura.cooldown, e.event.applyorremoveaura.cooldown);
            }
            break;
        }
        case SMART_EVENT_ON_TAXIPATHTO:
        {
            if (e.event.taxipathto.id && var0 != e.event.taxipathto.id)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
		case SMART_EVENT_DISTANCE_CREATURE:
		{
			if (!me)
				return;

			Creature* creature = nullptr;

			if (e.event.distance.guid != 0)
			{
				creature = FindCreatureNear(me, e.event.distance.guid);
				if (!creature)
					return;

				if (!me->IsInRange(creature, 0, static_cast<float>(e.event.distance.dist)))
					return;
			}
			else if (e.event.distance.entry != 0)
			{
				std::list<Creature*> list;
				me->GetCreatureListWithEntryInGrid(list, e.event.distance.entry, static_cast<float>(e.event.distance.dist));

				if (!list.empty())
					creature = list.front();
			}

			if (creature)
				ProcessAction(e);
                RecalcTimer(e, e.event.distance.repeat, e.event.distance.repeat);

			break;
		}
        default:
            TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript::ProcessEvent: Unhandled Event type %u", e.GetEventType());
            break;
    }
}

void SmartScript::ProcessTimedAction(SmartScriptHolder& e, uint32 const& min, uint32 const& max, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob, std::string const& varString)
{
	// We may want to execute action rarely and because of this if condition is not fulfilled the action will be rechecked in a long time
	if (sConditionMgr->IsObjectMeetingSmartEventConditions(e.entryOrGuid, e.event_id, e.source_type, unit, GetBaseObject()))
	{
		ProcessAction(e, unit, var0, var1, bvar, spell, gob);
		RecalcTimer(e, min, max);
	}
	else
		RecalcTimer(e, std::min<uint32>(min, 5000), std::min<uint32>(min, 5000));
}


void SmartScript::InitTimer(SmartScriptHolder& e)
{
    switch (e.GetEventType())
    {
        //set only events which have initial timers
        case SMART_EVENT_UPDATE:
        case SMART_EVENT_UPDATE_IC:
        case SMART_EVENT_UPDATE_OOC:
        case SMART_EVENT_OOC_LOS:
        case SMART_EVENT_IC_LOS:
            RecalcTimer(e, e.event.minMaxRepeat.min, e.event.minMaxRepeat.max);
            break;
		case SMART_EVENT_DISTANCE_CREATURE:
        default:
            e.active = true;
            break;
    }
}
void SmartScript::RecalcTimer(SmartScriptHolder& e, uint32 min, uint32 max)
{
    // min/max was checked at loading!
    if (uint32(min) > uint32(max))
        e.timer = urand(uint32(max), uint32(min));
    else
        e.timer = urand(uint32(min), uint32(max));
    e.active = e.timer ? false : true;
}

void SmartScript::UpdateTimer(SmartScriptHolder& e, uint32 const diff)
{
    if (e.GetEventType() == SMART_EVENT_LINK)
        return;

    if (e.event.event_phase_mask && !IsInPhase(e.event.event_phase_mask))
        return;

    if (e.GetEventType() == SMART_EVENT_UPDATE_IC && (!me || !me->isInCombat()))
        return;

    if (e.GetEventType() == SMART_EVENT_UPDATE_OOC && (me && me->isInCombat()))//can be used with me=NULL (go script)
        return;

    if (e.timer < diff)
    {
        // delay spell cast event if another spell is being casted
        if (e.GetActionType() == SMART_ACTION_CAST || e.GetActionType() == SMART_ACTION_CAST_CUSTOM)
        {
            if (!(e.action.cast.flags & SMARTCAST_INTERRUPT_PREVIOUS))
            {
                if (me && me->HasUnitState(UNIT_STATE_CASTING) && !(e.action.cast.flags & SMARTCAST_TRIGGERED))
                {
                    e.timer = 1;
                    return;
                }
            }
        }

        e.active = true;//activate events with cooldown
        switch (e.GetEventType())//process ONLY timed events
        {
            case SMART_EVENT_UPDATE:
            case SMART_EVENT_UPDATE_OOC:
            case SMART_EVENT_UPDATE_IC:
            case SMART_EVENT_HEALT_PCT:
            case SMART_EVENT_TARGET_HEALTH_PCT:
            case SMART_EVENT_MANA_PCT:
            case SMART_EVENT_TARGET_MANA_PCT:
            case SMART_EVENT_RANGE:
            case SMART_EVENT_TARGET_CASTING:
            case SMART_EVENT_FRIENDLY_HEALTH:
            case SMART_EVENT_FRIENDLY_IS_CC:
            case SMART_EVENT_FRIENDLY_MISSING_BUFF:
            case SMART_EVENT_HAS_AURA:
            case SMART_EVENT_TARGET_BUFFED:
            case SMART_EVENT_IS_BEHIND_TARGET:
            case SMART_EVENT_CHECK_DIST_TO_HOME:
            case SMART_EVENT_DISTANCE_CREATURE:
            {
                ProcessEvent(e);
                if (e.GetScriptType() == SMART_SCRIPT_TYPE_TIMED_ACTIONLIST)
                {
                    e.enableTimed = false; //disable event if it is in an ActionList and was processed once
                    for (auto& i : mTimedActionList)
                    {
                        //find the first event which is not the current one and enable it
                        if (e.entryOrGuid == i.entryOrGuid && i.event_id > e.event_id)
                        {
                            i.enableTimed = true;
                            break;
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    else
        e.timer -= diff;
}

bool SmartScript::CheckTimer(SmartScriptHolder const& e) const
{
    return e.active;
}

void SmartScript::InstallEvents()
{
    if (!mInstallEvents.empty())
    {
        for (auto& mInstallEvent : mInstallEvents)
            mEvents.push_back(mInstallEvent);//must be before UpdateTimers

        mInstallEvents.clear();
    }
}

void SmartScript::RemoveStoredEvent(uint32 id)
{
    if (!mStoredEvents.empty())
    {
        for (auto i = mStoredEvents.begin(); i != mStoredEvents.end(); ++i)
        {
            if (i->event_id == id)
            {
                mStoredEvents.erase(i);
                return;
            }
        }
    }
}

SmartScriptHolder SmartScript::FindLinkedEvent(uint32 link)
{
    if (!mEvents.empty())
        for (auto& mEvent : mEvents)
            if (mEvent.event_id == link)
                return mEvent;

    SmartScriptHolder s;
    return s;
}

void SmartScript::OnUpdate(uint32 const diff)
{
    if ((mScriptType == SMART_SCRIPT_TYPE_CREATURE || mScriptType == SMART_SCRIPT_TYPE_GAMEOBJECT) && !GetBaseObject())
        return;

    InstallEvents();//before UpdateTimers

    for (auto& mEvent : mEvents)
        UpdateTimer(mEvent, diff);

    if (!mStoredEvents.empty())
        for (auto& mStoredEvent : mStoredEvents)
            UpdateTimer(mStoredEvent, diff);

    bool needCleanup = true;
    if (!mTimedActionList.empty())
    {
        for (auto& i : mTimedActionList)
        {
            if (i.enableTimed)
            {
                UpdateTimer(i, diff);
                needCleanup = false;
            }
        }
    }
    if (needCleanup)
        mTimedActionList.clear();

    if (!mRemIDs.empty())
        for (auto& mRemID : mRemIDs)
             RemoveStoredEvent(mRemID);

    if (mUseTextTimer && me)
    {
        if (mTextTimer < diff)
        {
            uint32 textID = mLastTextID;
            mLastTextID = 0;
            uint32 entry = mTalkerEntry;
            mTalkerEntry = 0;
            mTextTimer = 0;
            mUseTextTimer = false;
            ProcessEventsFor(SMART_EVENT_TEXT_OVER, nullptr, textID, entry);
        } else mTextTimer -= diff;
    }
}

void SmartScript::FillScript(SmartAIEventList e, WorldObject* obj, AreaTriggerEntry const* at)
{
    if (e.empty())
    {
        if (obj)
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript: EventMap for Entry %u is empty but is using SmartScript.", obj->GetEntry());
        if (at)
            TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript: EventMap for AreaTrigger %u is empty but is using SmartScript.", at->ID);
        return;
    }
    for (auto& i : e)
    {
        #ifndef TRINITY_DEBUG
            if (i.event.event_flags & SMART_EVENT_FLAG_DEBUG_ONLY)
                continue;
        #endif

        if (i.event.event_flags & SMART_EVENT_FLAG_DIFFICULTY_ALL)//if has instance flag add only if in it
        {
            if (obj && obj->GetMap()->IsDungeon())
            {
                if ((1 << (CreatureTemplate::GetDiffFromSpawn(obj->GetMap()->GetSpawnMode()))) & i.event.event_flags)
                {
                    mEvents.push_back(i);
                }
            }
            continue;
        }
        mEvents.push_back(i);//NOTE: 'world(0)' events still get processed in ANY instance mode
    }
    if (mEvents.empty() && obj)
        TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Entry %u has events but no events added to list because of instance flags.", obj->GetEntry());
    if (mEvents.empty() && at)
        TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: AreaTrigger %u has events but no events added to list because of instance flags. NOTE: triggers can not handle any instance flags.", at->ID);
}

void SmartScript::GetScript()
{
    SmartAIEventList e;
    if (me)
    {
        if(me->GetDBTableGUIDLow())
            e = sSmartScriptMgr->GetScript(-static_cast<int32>(me->GetDBTableGUIDLow()), mScriptType);
        if (e.empty())
            e = sSmartScriptMgr->GetScript(static_cast<int32>(me->GetEntry()), mScriptType);
        FillScript(e, me, nullptr);
    }
    else if (go)
    {
        e = sSmartScriptMgr->GetScript(-static_cast<int32>(go->GetDBTableGUIDLow()), mScriptType);
        if (e.empty())
            e = sSmartScriptMgr->GetScript(static_cast<int32>(go->GetEntry()), mScriptType);
        FillScript(e, go, nullptr);
    }
    else if (event)
    {
        e = sSmartScriptMgr->GetScript(-static_cast<int32>(event->GetDBTableGUIDLow()), mScriptType);
        if (e.empty())
            e = sSmartScriptMgr->GetScript(static_cast<int32>(event->GetEntry()), mScriptType);
        FillScript(e, event, nullptr);
    }
    else if (trigger)
    {
        e = sSmartScriptMgr->GetScript(static_cast<int32>(trigger->ID), mScriptType);
        FillScript(e, nullptr, trigger);
    }
}

void SmartScript::OnInitialize(WorldObject* obj, AreaTriggerEntry const* at)
{
    if (obj)//handle object based scripts
    {
        switch (obj->GetTypeId())
        {
            case TYPEID_UNIT:
                mScriptType = SMART_SCRIPT_TYPE_CREATURE;
                me = obj->ToCreature();
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::OnInitialize: source is Creature %u", me->GetEntry());
                break;
            case TYPEID_GAMEOBJECT:
                mScriptType = SMART_SCRIPT_TYPE_GAMEOBJECT;
                go = obj->ToGameObject();
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::OnInitialize: source is GameObject %u", go->GetEntry());
                break;
            case TYPEID_EVENTOBJECT:
                mScriptType = SMART_SCRIPT_TYPE_EVENTOBJECT;
                event = obj->ToEventObject();
                TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::OnInitialize: source is EventObject %u", event->GetEntry());
                break;
            default:
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "SmartScript::OnInitialize: Unhandled TypeID !WARNING!");
                return;
        }
    } else if (at)
    {
        mScriptType = SMART_SCRIPT_TYPE_AREATRIGGER;
        trigger = at;
        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartScript::OnInitialize: source is AreaTrigger %u", trigger->ID);
    }
    else
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "SmartScript::OnInitialize: !WARNING! Initialized objects are NULL.");
        return;
    }

    GetScript();//load copy of script

    for (auto& mEvent : mEvents)
        InitTimer(mEvent);//calculate timers for first time use

    ProcessEventsFor(SMART_EVENT_AI_INIT);
    InstallEvents();
    ProcessEventsFor(SMART_EVENT_JUST_CREATED);
}

void SmartScript::OnMoveInLineOfSight(Unit* who)
{
    ProcessEventsFor(SMART_EVENT_OOC_LOS, who);

    if (!me)
        return;

    if (!me->getVictim())
        return;

    ProcessEventsFor(SMART_EVENT_IC_LOS, who);

}

/*
void SmartScript::UpdateAIWhileCharmed(const uint32 diff)
{
}

void SmartScript::DoAction(const int32 param)
{
}

uint32 SmartScript::GetData(uint32 id)
{
    return 0;
}

void SmartScript::SetData(uint32 id, uint32 value)
{
}

void SmartScript::SetGUID(uint64 guid, int32 id)
{
}

uint64 SmartScript::GetGUID(int32 id)
{
    return 0;
}

void SmartScript::MovepointStart(uint32 id)
{
}

void SmartScript::SetRun(bool run)
{
}

void SmartScript::SetMovePathEndAction(SMART_ACTION action)
{
}

uint32 SmartScript::DoChat(int8 id, uint64 whisperGuid)
{
    return 0;
}*/
// SmartScript end

Unit* SmartScript::DoSelectLowestHpFriendly(float range, uint32 MinHPDiff)
{
    if (!me)
        return nullptr;

    CellCoord p(Trinity::ComputeCellCoord(me->GetPositionX(), me->GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Unit* unit = nullptr;

    Trinity::MostHPMissingInRange u_check(me, range, MinHPDiff);
    Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(me, unit, u_check);

    cell.Visit(p, Trinity::makeGridVisitor(searcher), *me->GetMap(), *me, range);
    return unit;
}

void SmartScript::DoFindFriendlyCC(std::list<Creature*>& _list, float range)
{
    if (!me)
        return;

    CellCoord p(Trinity::ComputeCellCoord(me->GetPositionX(), me->GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Trinity::FriendlyCCedInRange u_check(me, range);
    Trinity::CreatureListSearcher<Trinity::FriendlyCCedInRange> searcher(me, _list, u_check);

    cell.Visit(p, Trinity::makeGridVisitor(searcher), *me->GetMap(), *me, range);
}

void SmartScript::DoFindFriendlyMissingBuff(std::list<Creature*>& list, float range, uint32 spellid)
{
    if (!me)
        return;

    CellCoord p(Trinity::ComputeCellCoord(me->GetPositionX(), me->GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Trinity::FriendlyMissingBuffInRange u_check(me, range, spellid);
    Trinity::CreatureListSearcher<Trinity::FriendlyMissingBuffInRange> searcher(me, list, u_check);

    cell.Visit(p, Trinity::makeGridVisitor(searcher), *me->GetMap(), *me, range);
}

Unit* SmartScript::DoFindClosestFriendlyInRange(float range)
{
    if (!me)
        return nullptr;

    Unit* unit = nullptr;
    Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(me, me, range);
    Trinity::UnitLastSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(me, unit, u_check);
    Trinity::VisitNearbyObject(me, range, searcher);
    return unit;
}

void SmartScript::StoreTargetList(ObjectList* targets, uint32 id)
{
    if (!targets)
        return;
    if (mTargetStorage->find(id) != mTargetStorage->end())
    {
        // check if already stored
        if ((*mTargetStorage)[id] == targets)
            return;
        delete (*mTargetStorage)[id];
    }
    (*mTargetStorage)[id] = targets;
}

bool SmartScript::IsSmart(Creature* c)
{
    bool smart = true;
    if (c && c->GetAIName() != "SmartAI")
        smart = false;
    if (!me || me->GetAIName() != "SmartAI")
        smart = false;
    if (!smart)
    TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Action target Creature(entry: %u) is not using SmartAI, action skipped to prevent crash.", c ? c->GetEntry() : (me ? me->GetEntry() : 0));
    return smart;
}

bool SmartScript::IsSmartGO(GameObject* g)
{
    bool smart = true;
    if (g && g->GetAIName() != "SmartGameObjectAI")
        smart = false;
    if (!go || go->GetAIName() != "SmartGameObjectAI")
        smart = false;
    if (!smart)
    TC_LOG_ERROR(LOG_FILTER_SQL, "SmartScript: Action target GameObject(entry: %u) is not using SmartGameObjectAI, action skipped to prevent crash.", g ? g->GetEntry() : (go ? go->GetEntry() : 0));
    return smart;
}

ObjectList* SmartScript::GetTargetList(uint32 id)
{
    auto itr = mTargetStorage->find(id);
    if (itr != mTargetStorage->end())
        return (*itr).second;
    return nullptr;
}

GameObject* SmartScript::FindGameObjectNear(WorldObject* searchObject, ObjectGuid::LowType guid) const
{
    GameObject* gameObject = nullptr;
    CellCoord p(Trinity::ComputeCellCoord(searchObject->GetPositionX(), searchObject->GetPositionY()));
    Cell cell(p);
    Trinity::GameObjectWithDbGUIDCheck goCheck(guid);
    Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(searchObject, gameObject, goCheck);
    cell.Visit(p, Trinity::makeGridVisitor(checker), *searchObject->GetMap(), *searchObject, searchObject->GetGridActivationRange());
    return gameObject;
}

Creature* SmartScript::FindCreatureNear(WorldObject* searchObject, ObjectGuid::LowType guid) const
{
    Creature* creature = nullptr;
    CellCoord p(Trinity::ComputeCellCoord(searchObject->GetPositionX(), searchObject->GetPositionY()));
    Cell cell(p);
    Trinity::CreatureWithDbGUIDCheck target_check(guid);
    Trinity::CreatureSearcher<Trinity::CreatureWithDbGUIDCheck> checker(searchObject, creature, target_check);
    cell.Visit(p, Trinity::makeGridVisitor(checker), *searchObject->GetMap(), *searchObject, searchObject->GetGridActivationRange());
    return creature;
}

void SmartScript::SetScript9(SmartScriptHolder& e, uint32 entry)
{
    SmartAIEventList val = sSmartScriptMgr->GetScript(entry, SMART_SCRIPT_TYPE_TIMED_ACTIONLIST);
    if (val.empty())
        return;

    for (auto i = val.begin(); i != val.end(); ++i)
    {
        i->enableTimed = i == val.begin();

        if (e.action.timedActionList.timerType == 1)
            i->event.type = SMART_EVENT_UPDATE_IC;
        else if (e.action.timedActionList.timerType > 1)
            i->event.type = SMART_EVENT_UPDATE;
        InitTimer((*i));
        
        mTimedActionList.push_back(*i);
    }
}

Unit* SmartScript::GetLastInvoker()
{
    return ObjectAccessor::FindUnit(mLastInvoker);
}

void SmartScript::IncPhase(int32 p)
{
    if (p >= 0)
        mEventPhase += static_cast<uint32>(p);
    else
        DecPhase(abs(p));
}

void SmartScript::DecPhase(int32 p)
{
    mEventPhase -= (mEventPhase < static_cast<uint32>(p) ? static_cast<uint32>(p) - mEventPhase : static_cast<uint32>(p));
}

bool SmartScript::IsInPhase(uint32 p) const
{
    return ((1 << (mEventPhase - 1)) & p) != 0;
}

void SmartScript::SetPhase(uint32 p)
{
    mEventPhase = p;
}
