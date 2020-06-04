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
Name: titles_commandscript
%Complete: 100
Comment: All titles related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"

class titles_commandscript : public CommandScript
{
public:
    titles_commandscript() : CommandScript("titles_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> titlesSetCommandTable =
        {
            { "mask",           SEC_GAMEMASTER,     false, &HandleTitlesSetMaskCommand,        ""}
        };
        static std::vector<ChatCommand> titlesCommandTable =
        {
            { "add",            SEC_GAMEMASTER,     false, &HandleTitlesAddCommand,            ""},
            { "current",        SEC_GAMEMASTER,     false, &HandleTitlesCurrentCommand,        ""},
            { "remove",         SEC_GAMEMASTER,     false, &HandleTitlesRemoveCommand,         ""},
            { "set",            SEC_GAMEMASTER,     false, NULL,              "", titlesSetCommandTable }
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "titles",         SEC_GAMEMASTER,     false, NULL,                 "", titlesCommandTable }
        };
        return commandTable;
    }

    static bool HandleTitlesCurrentCommand(ChatHandler* handler, const char* args)
    {
        // number or [name] Shift-click form |color|Htitle:title_id|h[name]|h|r
        char* id_p = handler->extractKeyFromLink((char*)args, "Htitle");
        if (!id_p)
            return false;

        int32 id = atoi(id_p);
        if (id <= 0)
        {
            handler->PSendSysMessage(LANG_INVALID_TITLE_ID, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(id);
        if (!titleInfo)
        {
            handler->PSendSysMessage(LANG_INVALID_TITLE_ID, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string tNameLink = handler->GetNameLink(target);

        target->SetTitle(titleInfo);                            // to be sure that title now known
        target->SetUInt32Value(PLAYER_FIELD_PLAYER_TITLE, titleInfo->MaskID);

        handler->PSendSysMessage(LANG_TITLE_CURRENT_RES, id, titleInfo->Name->Str[sObjectMgr->GetDBCLocaleIndex()], tNameLink.c_str());

        return true;
    }

    static bool HandleTitlesAddCommand(ChatHandler* handler, const char* args)
    {
        // number or [name] Shift-click form |color|Htitle:title_id|h[name]|h|r
        char* id_p = handler->extractKeyFromLink((char*)args, "Htitle");
        if (!id_p)
            return false;

        int32 id = atoi(id_p);
        if (id <= 0)
        {
            handler->PSendSysMessage(LANG_INVALID_TITLE_ID, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(id);
        if (!titleInfo)
        {
            handler->PSendSysMessage(LANG_INVALID_TITLE_ID, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string tNameLink = handler->GetNameLink(target);

        char const* targetName = target->GetName();
        char titleNameStr[80];
        snprintf(titleNameStr, 80, titleInfo->Name->Str[sObjectMgr->GetDBCLocaleIndex()], targetName);

        target->SetTitle(titleInfo);
        handler->PSendSysMessage(LANG_TITLE_ADD_RES, id, titleNameStr, tNameLink.c_str());

        return true;
    }

    static bool HandleTitlesRemoveCommand(ChatHandler* handler, const char* args)
    {
        // number or [name] Shift-click form |color|Htitle:title_id|h[name]|h|r
        char* id_p = handler->extractKeyFromLink((char*)args, "Htitle");
        if (!id_p)
            return false;

        int32 id = atoi(id_p);
        if (id <= 0)
        {
            handler->PSendSysMessage(LANG_INVALID_TITLE_ID, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(id);
        if (!titleInfo)
        {
            handler->PSendSysMessage(LANG_INVALID_TITLE_ID, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->SetTitle(titleInfo, true);

        std::string tNameLink = handler->GetNameLink(target);

        char const* targetName = target->GetName();
        char titleNameStr[80];
        snprintf(titleNameStr, 80, titleInfo->Name->Str[sObjectMgr->GetDBCLocaleIndex()], targetName);

        handler->PSendSysMessage(LANG_TITLE_REMOVE_RES, id, titleNameStr, tNameLink.c_str());

        if (!target->HasTitle(target->GetInt32Value(PLAYER_FIELD_PLAYER_TITLE)))
        {
            target->SetUInt32Value(PLAYER_FIELD_PLAYER_TITLE, 0);
            handler->PSendSysMessage(LANG_CURRENT_TITLE_RESET, tNameLink.c_str());
        }

        return true;
    }

    //Edit Player KnownTitles
    static bool HandleTitlesSetMaskCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint64 titles = 0;

        sscanf((char*)args, UI64FMTD, &titles);

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        uint64 titles2 = titles;

        for (uint32 i = 1; i < sCharTitlesStore.GetNumRows(); ++i)
            if (CharTitlesEntry const* tEntry = sCharTitlesStore.LookupEntry(i))
                titles2 &= ~(uint64(1) << tEntry->MaskID);

        titles &= ~titles2;                                     // remove not existed titles

        target->SetUInt64Value(PLAYER_FIELD_KNOWN_TITLES, titles);
        handler->SendSysMessage(LANG_DONE);

        if (!target->HasTitle(target->GetInt32Value(PLAYER_FIELD_PLAYER_TITLE)))
        {
            target->SetUInt32Value(PLAYER_FIELD_PLAYER_TITLE, 0);
            handler->PSendSysMessage(LANG_CURRENT_TITLE_RESET, handler->GetNameLink(target).c_str());
        }

        return true;
    }
};

void AddSC_titles_commandscript()
{
    new titles_commandscript();
}
