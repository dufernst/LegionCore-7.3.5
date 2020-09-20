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

/* ScriptData
Name: lookup_commandscript
%Complete: 100
Comment: All lookup related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "AccountMgr.h"
#include "GameEventMgr.h"
#include "GuildMgr.h"
#include "ObjectMgr.h"
#include "QuestData.h"
#include "DatabaseEnv.h"

class lookup_commandscript : public CommandScript
{
public:
    lookup_commandscript() : CommandScript("lookup_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> lookupPlayerCommandTable =
        {
            { "ip",             SEC_GAMEMASTER,     true,  &HandleLookupPlayerIpCommand,        ""},
            { "account",        SEC_GAMEMASTER,     true,  &HandleLookupPlayerEmailCommand,     ""},
            { "email",          SEC_GAMEMASTER,     true,  &HandleLookupPlayerEmailCommand,     ""}
        };
        static std::vector<ChatCommand> lookupCommandTable =
        {
            { "area",           SEC_MODERATOR,      true,  &HandleLookupAreaCommand,            ""},
            { "creature",       SEC_ADMINISTRATOR,  true,  &HandleLookupCreatureCommand,        ""},
            { "event",          SEC_GAMEMASTER,     true,  &HandleLookupEventCommand,           ""},
            { "faction",        SEC_ADMINISTRATOR,  true,  &HandleLookupFactionCommand,         ""},
            { "item",           SEC_ADMINISTRATOR,  true,  &HandleLookupItemCommand,            ""},
            { "itemset",        SEC_ADMINISTRATOR,  true,  &HandleLookupItemSetCommand,         ""},
            { "guild",          SEC_GAMEMASTER,     true,  &HandleLookupGuildCommand,           ""},
            { "object",         SEC_ADMINISTRATOR,  true,  &HandleLookupObjectCommand,          ""},
            { "quest",          SEC_ADMINISTRATOR,  true,  &HandleLookupQuestCommand,           ""},
            { "player",         SEC_GAMEMASTER,     true,  NULL,                                "", lookupPlayerCommandTable },
            { "skill",          SEC_ADMINISTRATOR,  true,  &HandleLookupSkillCommand,           ""},
            { "spell",          SEC_ADMINISTRATOR,  true,  &HandleLookupSpellCommand,           ""},
            { "taxinode",       SEC_ADMINISTRATOR,  true,  &HandleLookupTaxiNodeCommand,        ""},
            { "tele",           SEC_MODERATOR,      true,  &HandleLookupTeleCommand,            ""},
            { "title",          SEC_GAMEMASTER,     true,  &HandleLookupTitleCommand,           ""},
            { "map",            SEC_ADMINISTRATOR,  true,  &HandleLookupMapCommand,             ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "lookup",         SEC_ADMINISTRATOR,  true,  NULL,                                "", lookupCommandTable }
        };
        return commandTable;
    }

    static bool HandleLookupAreaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        // Search in AreaTable.dbc
        for (AreaTableEntry const* areaEntry : sAreaTableStore)
        {
            std::string name = areaEntry->ZoneName->Str[sObjectMgr->GetDBCLocaleIndex()];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wNamePart))
                continue;

            if (maxResults && count++ == maxResults)
            {
                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                return true;
            }

            // send area in "id - [name]" format
            std::ostringstream ss;
            if (handler->GetSession())
                ss << areaEntry->ID << " - |cffffffff|Harea:" << areaEntry->ID << "|h[" << name << "]|h|r";
            else
                ss << areaEntry->ID << " - " << name;

            handler->SendSysMessage(ss.str().c_str());

            if (!found)
                found = true;
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOAREAFOUND);

        return true;
    }

    static bool HandleLookupCreatureCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        CreatureTemplateContainerMap const* ctc = sObjectMgr->GetCreatureTemplates();
        for (auto &v : *ctc)
        {
            uint32 id = v.second.Entry;
            uint8 localeIndex = handler->GetSessionDbLocaleIndex();
            if (CreatureLocale const* creatureLocale = sObjectMgr->GetCreatureLocale(id))
            {
                if (creatureLocale->Name[0].size() > localeIndex && !creatureLocale->Name[0][localeIndex].empty())
                {
                    std::string name = creatureLocale->Name[0][localeIndex];

                    if (Utf8FitTo(name, wNamePart))
                    {
                        if (maxResults && count++ == maxResults)
                        {
                            handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                            return true;
                        }

                        if (handler->GetSession())
                            handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str());
                        else
                            handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str());

                        if (!found)
                            found = true;

                        continue;
                    }
                }
            }

            std::string name = v.second.Name[0];
            if (name.empty())
                continue;

            if (Utf8FitTo(name, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str());
                else
                    handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str());

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOCREATUREFOUND);

        return true;
    }

    static bool HandleLookupEventCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();
        GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr->GetActiveEventList();

        for (uint32 id = 0; id < events.size(); ++id)
        {
            GameEventData const& eventData = events[id];

            std::string descr = eventData.description;
            if (descr.empty())
                continue;

            if (Utf8FitTo(descr, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                char const* active = activeEvents.find(id) != activeEvents.end() ? handler->GetTrinityString(LANG_ACTIVE) : "";

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_EVENT_ENTRY_LIST_CHAT, id, id, eventData.description.c_str(), active);
                else
                    handler->PSendSysMessage(LANG_EVENT_ENTRY_LIST_CONSOLE, id, eventData.description.c_str(), active);

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_NOEVENTFOUND);

        return true;
    }

    static bool HandleLookupFactionCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // Can be NULL at console call
        Player* target = handler->getSelectedPlayer();

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower (wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        for (uint32 id = 0; id < sFactionStore.GetNumRows(); ++id)
        {
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(id))
            {
                FactionState const* factionState = target ? target->GetReputationMgr().GetState(factionEntry) : NULL;

                std::string name = factionEntry->Name->Str[LOCALE_enUS];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                    continue;

                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                // send faction in "id - [faction] rank reputation [visible] [at war] [own team] [unknown] [invisible] [inactive]" format
                // or              "id - [faction] [no reputation]" format
                std::ostringstream ss;
                if (handler->GetSession())
                    ss << id << " - |cffffffff|Hfaction:" << id << "|h[" << name << "]|h|r";
                else
                    ss << id << " - " << name;

                if (factionState) // and then target != NULL also
                {
                    uint32 index = target->GetReputationMgr().GetReputationRankStrIndex(factionEntry);
                    std::string rankName = handler->GetTrinityString(index);

                    ss << ' ' << rankName << "|h|r (" << target->GetReputationMgr().GetReputation(factionEntry) << ')';

                    if (factionState->Flags & FACTION_FLAG_VISIBLE)
                        ss << handler->GetTrinityString(LANG_FACTION_VISIBLE);
                    if (factionState->Flags & FACTION_FLAG_AT_WAR)
                        ss << handler->GetTrinityString(LANG_FACTION_ATWAR);
                    if (factionState->Flags & FACTION_FLAG_PEACE_FORCED)
                        ss << handler->GetTrinityString(LANG_FACTION_PEACE_FORCED);
                    if (factionState->Flags & FACTION_FLAG_HIDDEN)
                        ss << handler->GetTrinityString(LANG_FACTION_HIDDEN);
                    if (factionState->Flags & FACTION_FLAG_INVISIBLE_FORCED)
                        ss << handler->GetTrinityString(LANG_FACTION_INVISIBLE_FORCED);
                    if (factionState->Flags & FACTION_FLAG_INACTIVE)
                        ss << handler->GetTrinityString(LANG_FACTION_INACTIVE);
                }
                else
                    ss << handler->GetTrinityString(LANG_FACTION_NOREPUTATION);

                handler->SendSysMessage(ss.str().c_str());

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_FACTION_NOTFOUND);
        return true;
    }

    static bool HandleLookupItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in `item_template`
        ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
        for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            if (auto name = itr->second.GetName()->Str[handler->GetSessionDbLocaleIndex()])
            {
                if (Utf8FitTo(name, wNamePart))
                {
                    if (maxResults && count++ == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    if (handler->GetSession())
                        handler->PSendSysMessage(LANG_ITEM_LIST_CHAT, itr->second.GetId(), itr->second.GetId(), name);
                    else
                        handler->PSendSysMessage(LANG_ITEM_LIST_CONSOLE, itr->second.GetId(), name);

                    if (!found)
                        found = true;

                    continue;
                }
            }

            std::string name = itr->second.GetName()->Str[handler->GetSessionDbLocaleIndex()];
            if (name.empty())
                continue;

            if (Utf8FitTo(name, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_ITEM_LIST_CHAT, itr->second.GetId(), itr->second.GetId(), name.c_str());
                else
                    handler->PSendSysMessage(LANG_ITEM_LIST_CONSOLE, itr->second.GetId(), name.c_str());

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOITEMFOUND);

        return true;
    }

    static bool HandleLookupItemSetCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in ItemSet.dbc
        for (ItemSetEntry const* set : sItemSetStore)
        {
            std::string name = set->Name->Str[sObjectMgr->GetDBCLocaleIndex()];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wNamePart))
                continue;

            if (maxResults && count++ == maxResults)
            {
                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                return true;
            }

            // send item set in "id - [namedlink locale]" format
            if (handler->GetSession())
                handler->PSendSysMessage(LANG_ITEMSET_LIST_CHAT, set->ID, set->ID, name.c_str(), "");
            else
                handler->PSendSysMessage(LANG_ITEMSET_LIST_CONSOLE, set->ID, name.c_str(), "");

            if (!found)
                found = true;
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOITEMSETFOUND);

        return true;
    }

    static bool HandleLookupObjectCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplates();
        for (GameObjectTemplateContainer::const_iterator itr = gotc->begin(); itr != gotc->end(); ++itr)
        {
            uint8 localeIndex = handler->GetSessionDbLocaleIndex();
            if (GameObjectLocale const* objectLocalte = sObjectMgr->GetGameObjectLocale(itr->second.entry))
            {
                if (objectLocalte->Name.size() > localeIndex && !objectLocalte->Name[localeIndex].empty())
                {
                    std::string name = objectLocalte->Name[localeIndex];

                    if (Utf8FitTo(name, wNamePart))
                    {
                        if (maxResults && count++ == maxResults)
                        {
                            handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                            return true;
                        }

                        if (handler->GetSession())
                            handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, itr->second.entry, itr->second.entry, name.c_str());
                        else
                            handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, itr->second.entry, name.c_str());

                        if (!found)
                            found = true;

                        continue;
                    }
                }
            }

            std::string name = itr->second.name;
            if (name.empty())
                continue;

            if (Utf8FitTo(name, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, itr->second.entry, itr->second.entry, name.c_str());
                else
                    handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, itr->second.entry, name.c_str());

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);

        return true;
    }

    static bool HandleLookupQuestCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL at console call
        Player* target = handler->getSelectedPlayer();

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        QuestMap const& qTemplates = sQuestDataStore->GetQuestTemplates();
        for (QuestMap::const_iterator iter = qTemplates.begin(); iter != qTemplates.end(); ++iter)
        {
            Quest* qInfo = iter->second;

            int localeIndex = handler->GetSessionDbLocaleIndex();
            if (localeIndex >= 0)
            {
                uint8 ulocaleIndex = uint8(localeIndex);
                if (QuestTemplateLocale const* questLocale = sQuestDataStore->GetQuestLocale(qInfo->GetQuestId()))
                {
                    if (questLocale->LogTitle.size() > ulocaleIndex && !questLocale->LogTitle[ulocaleIndex].empty())
                    {
                        std::string title = questLocale->LogTitle[ulocaleIndex];

                        if (Utf8FitTo(title, wNamePart))
                        {
                            if (maxResults && count++ == maxResults)
                            {
                                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                                return true;
                            }

                            char const* statusStr = "";

                            if (target)
                            {
                                QuestStatus status = target->GetQuestStatus(qInfo->GetQuestId());

                                switch (status)
                                {
                                    case QUEST_STATUS_COMPLETE:
                                        statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                                        break;
                                    case QUEST_STATUS_INCOMPLETE:
                                        statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
                                        break;
                                    case QUEST_STATUS_REWARDED:
                                        statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                                        break;
                                    default:
                                        break;
                                }
                            }

                            if (handler->GetSession())
                                handler->PSendSysMessage(LANG_QUEST_LIST_CHAT, qInfo->GetQuestId(), qInfo->GetQuestId(), qInfo->Level, qInfo->MinLevel, qInfo->MaxScalingLevel, title.c_str(), statusStr);
                            else
                                handler->PSendSysMessage(LANG_QUEST_LIST_CONSOLE, qInfo->GetQuestId(), title.c_str(), statusStr);

                            if (!found)
                                found = true;

                            continue;
                        }
                    }
                }
            }

            std::string title = qInfo->LogTitle;
            if (title.empty())
                continue;

            if (Utf8FitTo(title, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                char const* statusStr = "";

                if (target)
                {
                    QuestStatus status = target->GetQuestStatus(qInfo->GetQuestId());

                    switch (status)
                    {
                        case QUEST_STATUS_COMPLETE:
                            statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                            break;
                        case QUEST_STATUS_INCOMPLETE:
                            statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
                            break;
                        case QUEST_STATUS_REWARDED:
                            statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                            break;
                        default:
                            break;
                    }
                }

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_QUEST_LIST_CHAT, qInfo->GetQuestId(), qInfo->GetQuestId(), qInfo->Level, title.c_str(), statusStr);
                else
                    handler->PSendSysMessage(LANG_QUEST_LIST_CONSOLE, qInfo->GetQuestId(), title.c_str(), statusStr);

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOQUESTFOUND);

        return true;
    }

    static bool HandleLookupSkillCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL in console call
        Player* target = handler->getSelectedPlayer();

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in SkillLine.dbc
        for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); id++)
        {
            SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(id);
            if (skillInfo)
            {
                std::string name = skillInfo->DisplayName[DEFAULT_LOCALE].Str[DEFAULT_LOCALE];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                    continue;

                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                char valStr[50] = "";
                char const* knownStr = "";
                if (target && target->HasSkill(id))
                {
                    knownStr = handler->GetTrinityString(LANG_KNOWN);
                    uint32 curValue = target->GetPureSkillValue(id);
                    uint32 maxValue  = target->GetPureMaxSkillValue(id);
                    uint32 permValue = target->GetSkillPermBonusValue(id);
                    uint32 tempValue = target->GetSkillTempBonusValue(id);

                    char const* valFormat = handler->GetTrinityString(LANG_SKILL_VALUES);
                    snprintf(valStr, 50, valFormat, curValue, maxValue, permValue, tempValue);
                }

                // send skill in "id - [namedlink locale]" format
                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_SKILL_LIST_CHAT, id, id, name.c_str(), "", knownStr, valStr);
                else
                    handler->PSendSysMessage(LANG_SKILL_LIST_CONSOLE, id, name.c_str(), "", knownStr, valStr);

                if (!found)
                    found = true;
            }
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOSKILLFOUND);

        return true;
    }

    static bool HandleLookupSpellCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL at console call
        Player* target = handler->getSelectedPlayer();

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in Spell.dbc
        uint32 StoreSize = sSpellMgr->GetSpellInfoStoreSize();
        for (uint32 id = 0; id < StoreSize; ++id)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
            if (spellInfo)
            {
                std::string name = spellInfo->SpellName;
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                    continue;

                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                bool known = target && target->HasSpell(id);
                bool learn = (spellInfo->Effects[0]->Effect == SPELL_EFFECT_LEARN_SPELL);

                SpellInfo const* learnSpellInfo = sSpellMgr->GetSpellInfo(spellInfo->Effects[0]->TriggerSpell);

                uint32 talentCost =0;// GetTalentSpellCost(id);

                bool talent = (talentCost > 0);
                bool passive = spellInfo->IsPassive();
                bool active = target && target->HasAura(id);

                // unit32 used to prevent interpreting uint8 as char at output
                // find rank of learned spell for learning spell, or talent rank
                uint32 rank = talentCost ? talentCost : learn && learnSpellInfo ? learnSpellInfo->GetRank() : spellInfo->GetRank();

                // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
                std::ostringstream ss;
                if (handler->GetSession())
                    ss << id << " - |cffffffff|Hspell:" << id << "|h[" << name;
                else
                    ss << id << " - " << name;

                // include rank in link name
                if (rank)
                    ss << handler->GetTrinityString(LANG_SPELL_RANK) << rank;

                if (handler->GetSession())
                    ss << "]|h|r";

                if (talent)
                    ss << handler->GetTrinityString(LANG_TALENT);
                if (passive)
                    ss << handler->GetTrinityString(LANG_PASSIVE);
                if (learn)
                    ss << handler->GetTrinityString(LANG_LEARN);
                if (known)
                    ss << handler->GetTrinityString(LANG_KNOWN);
                if (active)
                    ss << handler->GetTrinityString(LANG_ACTIVE);

                handler->SendSysMessage(ss.str().c_str());

                if (!found)
                    found = true;
            }
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOSPELLFOUND);

        return true;
    }

    static bool HandleLookupTaxiNodeCommand(ChatHandler* handler, const char * args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);
        int32 locale = handler->GetSessionDbcLocale();

        // Search in TaxiNodes.dbc
        for (TaxiNodesEntry const* nodeEntry : sTaxiNodesStore)
        {
            std::string name = nodeEntry->Name->Str[locale];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wNamePart))
                continue;

            if (maxResults && count++ == maxResults)
            {
                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                return true;
            }

            // send taxinode in "id - [name] (Map:m X:x Y:y Z:z)" format
            if (handler->GetSession())
                handler->PSendSysMessage(LANG_TAXINODE_ENTRY_LIST_CHAT, nodeEntry->ID, nodeEntry->ID, name.c_str(), "",
                nodeEntry->ContinentID, nodeEntry->Pos.X, nodeEntry->Pos.Y, nodeEntry->Pos.Z);
            else
                handler->PSendSysMessage(LANG_TAXINODE_ENTRY_LIST_CONSOLE, nodeEntry->ID, name.c_str(), "",
                nodeEntry->ContinentID, nodeEntry->Pos.X, nodeEntry->Pos.Y, nodeEntry->Pos.Z);

            if (!found)
                found = true;
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOTAXINODEFOUND);

        return true;
    }

    // Find tele in game_tele order by name
    static bool HandleLookupTeleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(LANG_COMMAND_TELE_PARAMETER);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char const* str = strtok((char*)args, " ");
        if (!str)
            return false;

        std::string namePart = str;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        std::ostringstream reply;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);
        bool limitReached = false;

        GameTeleContainer const & teleMap = sObjectMgr->GetGameTeleMap();
        for (GameTeleContainer::const_iterator itr = teleMap.begin(); itr != teleMap.end(); ++itr)
        {
            GameTele const* tele = &itr->second;

            if (tele->wnameLow.find(wNamePart) == std::wstring::npos)
                continue;

            if (maxResults && count++ == maxResults)
            {
                limitReached = true;
                break;
            }

            if (handler->GetSession())
                reply << "  |cffffffff|Htele:" << itr->first << "|h[" << tele->name << "]|h|r\n";
            else
                reply << "  " << itr->first << ' ' << tele->name << "\n";
        }

        if (reply.str().empty())
            handler->SendSysMessage(LANG_COMMAND_TELE_NOLOCATION);
        else
            handler->PSendSysMessage(LANG_COMMAND_TELE_LOCATION, reply.str().c_str());

        if (limitReached)
            handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);

        return true;
    }

    static bool HandleLookupTitleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL in console call
        Player* target = handler->getSelectedPlayer();

        // title name have single string arg for player name
        char const* targetName = target ? target->GetName() : "NAME";

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        uint32 counter = 0;                                     // Counter for figure out that we found smth.
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        for (CharTitlesEntry const* titleInfo : sCharTitlesStore)
        {
            std::string name = titleInfo->Name->Str[sObjectMgr->GetDBCLocaleIndex()];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wNamePart))
                continue;

            if (maxResults && counter == maxResults)
            {
                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                return true;
            }

            char const* knownStr = target && target->HasTitle(titleInfo) ? handler->GetTrinityString(LANG_KNOWN) : "";

            char const* activeStr = target && target->GetUInt32Value(PLAYER_FIELD_PLAYER_TITLE) == titleInfo->MaskID
                ? handler->GetTrinityString(LANG_ACTIVE)
                : "";

            char titleNameStr[80];
            snprintf(titleNameStr, 80, name.c_str(), targetName);

            // send title in "id (idx:idx) - [namedlink locale]" format
            if (handler->GetSession())
                handler->PSendSysMessage(LANG_TITLE_LIST_CHAT, titleInfo->ID, titleInfo->MaskID, titleInfo->ID, titleNameStr, "", knownStr, activeStr);
            else
                handler->PSendSysMessage(LANG_TITLE_LIST_CONSOLE, titleInfo->ID, titleInfo->MaskID, titleNameStr, "", knownStr, activeStr);

            ++counter;
        }

        if (counter == 0)  // if counter == 0 then we found nth
            handler->SendSysMessage(LANG_COMMAND_NOTITLEFOUND);

        return true;
    }

    static bool HandleLookupMapCommand(ChatHandler* /*handler*/, char const* args)
    {
        if (!*args)
            return false;
        /*
        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;

        // search in Map.dbc
        for (uint32 id = 0; id < sMapStore.GetNumRows(); id++)
        {
            MapEntry const* MapInfo = sMapStore.LookupEntry(id);
            if (MapInfo)
            {
                uint8 locale = handler->GetSession() ? handler->GetSession()->GetSessionDbcLocale() : sWorld->GetDefaultDbcLocale();

                std::string name = MapInfo->name[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                {
                    locale = LOCALE_enUS;
                    for (; locale < TOTAL_LOCALES; locale++)
                    {
                        if (handler->GetSession() && locale == handler->GetSession()->GetSessionDbcLocale())
                            continue;

                        name = MapInfo->name[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    // send map in "id - [name][Continent][Instance/Battleground/Arena][Raid reset time:][Heroic reset time:][Mountable]" format
                    std::ostringstream ss;

                    if (handler->GetSession())
                        ss << id << " - |cffffffff|Hmap:" << id << "|h[" << name << ']';
                    else // console
                        ss << id << " - [" << name << ']';

                    if (MapInfo->IsContinent())
                        ss << handler->GetTrinityString(LANG_CONTINENT);

                    switch (MapInfo->InstanceType)
                    {
                        case MAP_INSTANCE:      ss << handler->GetTrinityString(LANG_INSTANCE);      break;
                        case MAP_BATTLEGROUND:  ss << handler->GetTrinityString(LANG_BATTLEGROUND);  break;
                        case MAP_ARENA:         ss << handler->GetTrinityString(LANG_ARENA);         break;
                    }

                    if (MapInfo->IsRaid())
                        ss << handler->GetTrinityString(LANG_RAID);

                    if (MapInfo->SupportsHeroicMode())
                        ss << handler->GetTrinityString(LANG_HEROIC);

                    uint32 ResetTimeRaid = MapInfo->resetTimeRaid;

                    std::string ResetTimeRaidStr;
                    if (ResetTimeRaid)
                        ResetTimeRaidStr = secsToTimeString(ResetTimeRaid, true, false);

                    uint32 ResetTimeHeroic = MapInfo->resetTimeHeroic;
                    std::string ResetTimeHeroicStr;
                    if (ResetTimeHeroic)
                        ResetTimeHeroicStr = secsToTimeString(ResetTimeHeroic, true, false);

                    if (MapInfo->IsMountAllowed())
                        ss << handler->GetTrinityString(LANG_MOUNTABLE);

                    if (ResetTimeRaid && !ResetTimeHeroic)
                        handler->PSendSysMessage(ss.str().c_str(), ResetTimeRaidStr.c_str());
                    else if (!ResetTimeRaid && ResetTimeHeroic)
                        handler->PSendSysMessage(ss.str().c_str(), ResetTimeHeroicStr.c_str());
                    else if (ResetTimeRaid && ResetTimeHeroic)
                        handler->PSendSysMessage(ss.str().c_str(), ResetTimeRaidStr.c_str(), ResetTimeHeroicStr.c_str());
                    else
                        handler->SendSysMessage(ss.str().c_str());

                    if (!found)
                        found = true;
                }
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOMAPFOUND);
        */
        return true;
    }

    static bool HandleLookupPlayerIpCommand(ChatHandler* handler, char const* args)
    {
        std::string ip;
        int32 limit;
        char* limitStr;

        Player* target = handler->getSelectedPlayer();
        if (!*args)
        {
            // NULL only if used from console
            if (!target || target == handler->GetSession()->GetPlayer())
                return false;

            ip = target->GetSession()->GetRemoteAddress();
            limit = -1;
        }
        else
        {
            ip = strtok((char*)args, " ");
            limitStr = strtok(NULL, " ");
            limit = limitStr ? atoi(limitStr) : -1;
        }

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_IP);
        stmt->setString(0, ip);

        uint32 accountId = handler->GetSession()->GetAccountId();
        LoginDatabase.CallBackQuery(stmt, [accountId, limit](PreparedQueryResult result) -> void
        {
            if (WorldSessionPtr sess = sWorld->FindSession(accountId))
                sess->LookupPlayerSearchCommand(result, limit);
        });

        return true;
    }

    static bool HandleLookupPlayerEmailCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string email = strtok((char*)args, " ");
        char* limitStr = strtok(NULL, " ");
        int32 limit = limitStr ? atoi(limitStr) : -1;

        // the account name and email address are the same since the switch to bnet accounts
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_LIST_BY_NAME);
        stmt->setString(0, email);

        uint32 accountId = handler->GetSession()->GetAccountId();
        LoginDatabase.CallBackQuery(stmt, [accountId, limit](PreparedQueryResult result) -> void
        {
            if (WorldSessionPtr sess = sWorld->FindSession(accountId))
                sess->LookupPlayerSearchCommand(result, limit);
        });

        return true;
    }   
    
    static bool HandleLookupGuildCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        ObjectGuid::LowType guildId = atoi(strtok((char*)args, ""));
        if (!guildId)
            return false;
        
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))
            handler->PSendSysMessage("Found guild with name: '%s' by id = %d", guild->GetName().c_str(), guildId);
        else
            handler->PSendSysMessage("Don't found any guild with id %d", guildId);

        return true;
    }
};

void AddSC_lookup_commandscript()
{
    new lookup_commandscript();
}
