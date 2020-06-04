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
Name: learn_commandscript
%Complete: 100
Comment: All learn related commands
Category: commandscripts
EndScriptData */

#include "Chat.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "GlobalFunctional.h"

class learn_commandscript : public CommandScript
{
public:
    learn_commandscript() : CommandScript("learn_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {

        static std::vector<ChatCommand> learnAllCommandTable =
        {
            { "gm",             SEC_GAMEMASTER,     false, &HandleLearnAllGMCommand,            ""},
            { "crafts",         SEC_GAMEMASTER,     false, &HandleLearnAllCraftsCommand,        ""},
            { "default",        SEC_MODERATOR,      false, &HandleLearnAllDefaultCommand,       ""},
            { "lang",           SEC_MODERATOR,      false, &HandleLearnAllLangCommand,          ""},
            { "recipes",        SEC_GAMEMASTER,     false, &HandleLearnAllRecipesCommand,       ""}
        };

        static std::vector<ChatCommand> learnCommandTable =
        {
            { "all",            SEC_ADMINISTRATOR,  false, NULL,                                "",  learnAllCommandTable },
            { "",               SEC_ADMINISTRATOR,  false, &HandleLearnCommand,                 ""}
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "learn",          SEC_MODERATOR,      false, NULL,                                "", learnCommandTable },
            { "unlearn",        SEC_ADMINISTRATOR,  false, &HandleUnLearnCommand,               ""}
        };
        return commandTable;
    }

    static bool HandleLearnCommand(ChatHandler* handler, char const* args)
    {
        Player* targetPlayer = handler->getSelectedPlayer();

        if (!targetPlayer)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spell = handler->extractSpellIdFromLink((char*)args);
        if (!spell || !sSpellMgr->GetSpellInfo(spell))
            return false;

        char const* all = strtok(NULL, " ");
        bool allRanks = all ? (strncmp(all, "all", strlen(all)) == 0) : false;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, handler->GetSession()->GetPlayer()))
        {
            handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!allRanks && targetPlayer->HasSpell(spell))
        {
            if (targetPlayer == handler->GetSession()->GetPlayer())
                handler->SendSysMessage(LANG_YOU_KNOWN_SPELL);
            else
                handler->PSendSysMessage(LANG_TARGET_KNOWN_SPELL, handler->GetNameLink(targetPlayer).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (allRanks)
            targetPlayer->learnSpellHighRank(spell);
        else
            targetPlayer->learnSpell(spell, false);

        uint32 firstSpell = sSpellMgr->GetFirstSpellInChain(spell);
        /*if (GetTalentSpellCost(firstSpell))
            targetPlayer->SendTalentsInfoData(false);*/

        return true;
    }

    static bool HandleLearnAllGMCommand(ChatHandler* handler, char const* /*args*/)
    {
        for (uint32 i = 0; i < sSpellMgr->GetSpellInfoStoreSize(); ++i)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(i);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, handler->GetSession()->GetPlayer(), false))
                continue;

            if (!IsPartOfSkillLine(SKILL_INTERNAL, spellInfo->Id))
                continue;

            handler->GetSession()->GetPlayer()->learnSpell(i, false);
        }

        handler->SendSysMessage(LANG_LEARNING_GM_SKILLS);
        return true;
    }

    static bool HandleLearnAllLangCommand(ChatHandler* handler, char const* /*args*/)
    {
        // skipping UNIVERSAL language (0)
        for (uint8 i = 1; i < LANGUAGE_DESC_COUNT; ++i)
            handler->GetSession()->GetPlayer()->learnSpell(lang_description[i].spell_id, false);

        handler->SendSysMessage(LANG_COMMAND_LEARN_ALL_LANG);
        return true;
    }

    static bool HandleLearnAllDefaultCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        target->learnQuestRewardedSpells();

        handler->PSendSysMessage(LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST, handler->GetNameLink(target).c_str());
        return true;
    }

    static bool HandleLearnAllCraftsCommand(ChatHandler* handler, char const* /*args*/)
    {
        for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
        {
            SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(i);
            if (!skillInfo)
                continue;

            if ((skillInfo->CategoryID == SKILL_CATEGORY_PROFESSION || skillInfo->CategoryID == SKILL_CATEGORY_SECONDARY) && skillInfo->CanLink) // only prof. with recipes have
                HandleLearnSkillRecipesHelper(handler->GetSession()->GetPlayer(), skillInfo->ID);
        }

        handler->SendSysMessage(LANG_COMMAND_LEARN_ALL_CRAFT);
        return true;
    }

    static bool HandleLearnAllRecipesCommand(ChatHandler* handler, char const* args)
    {
        //  Learns all recipes of specified profession and sets skill to max
        //  Example: .learn all_recipes enchanting

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        if (!*args)
            return false;

        std::wstring namePart;

        if (!Utf8toWStr(args, namePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(namePart);

        std::string name;

        SkillLineEntry const* targetSkillInfo = NULL;
        for (uint32 i = 1; i < sSkillLineStore.GetNumRows(); ++i)
        {
            SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(i);
            if (!skillInfo)
                continue;

            if ((skillInfo->CategoryID != SKILL_CATEGORY_PROFESSION && skillInfo->CategoryID != SKILL_CATEGORY_SECONDARY) || !skillInfo->CanLink) // only prof with recipes have set
                continue;

            name = skillInfo->DisplayName[DEFAULT_LOCALE].Str[DEFAULT_LOCALE];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, namePart))
                continue;

            targetSkillInfo = skillInfo;
        }

        if (!targetSkillInfo)
            return false;

        HandleLearnSkillRecipesHelper(target, targetSkillInfo->ID);

        uint16 maxLevel = target->GetPureMaxSkillValue(targetSkillInfo->ID);
        target->SetSkill(targetSkillInfo->ID, target->GetSkillStep(targetSkillInfo->ID), maxLevel, maxLevel);
        handler->PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, name.c_str());
        return true;
    }

    static void HandleLearnSkillRecipesHelper(Player* player, uint32 skillId)
    {
        uint32 classmask = player->getClassMask();

        for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
        {
            SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
            if (!skillLine)
                continue;

            // wrong skill
            if (skillLine->SkillLine != skillId)
                continue;

            // not high rank
            if (skillLine->SupercedesSpell)
                continue;

            // skip racial skills
            if (skillLine->RaceMask != 0)
                continue;

            // skip wrong class skills
            if (skillLine->ClassMask && (skillLine->ClassMask & classmask) == 0)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->Spell);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
                continue;

            player->learnSpell(skillLine->Spell, false);
        }
    }

    static bool HandleUnLearnCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        char const* allStr = strtok(NULL, " ");
        bool allRanks = allStr ? (strncmp(allStr, "all", strlen(allStr)) == 0) : false;

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (allRanks)
            spellId = sSpellMgr->GetFirstSpellInChain (spellId);

        if (target->HasSpell(spellId))
            target->removeSpell(spellId, false, !allRanks);
        else
            handler->SendSysMessage(LANG_FORGET_SPELL);

        /*if (GetTalentSpellCost(spellId))
            target->SendTalentsInfoData(false);*/

        return true;
    }
};

void AddSC_learn_commandscript()
{
    new learn_commandscript();
}
