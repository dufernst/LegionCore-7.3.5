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
Name: ban_commandscript
%Complete: 100
Comment: All ban related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "AccountMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "GlobalFunctional.h"

class ban_commandscript : public CommandScript
{
public:
    ban_commandscript() : CommandScript("ban_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> unbanCommandTable =
        {
            { "account",        SEC_ADMINISTRATOR,  true,  &HandleUnBanAccountCommand,          ""},
            { "character",      SEC_ADMINISTRATOR,  true,  &HandleUnBanCharacterCommand,        ""},
            { "playeraccount",  SEC_ADMINISTRATOR,  true,  &HandleUnBanAccountByCharCommand,    ""},
            { "ip",             SEC_ADMINISTRATOR,  true,  &HandleUnBanIPCommand,               ""}
        };
        static std::vector<ChatCommand> banlistCommandTable =
        {
            { "account",        SEC_ADMINISTRATOR,  true,  &HandleBanListAccountCommand,        ""},
            { "character",      SEC_ADMINISTRATOR,  true,  &HandleBanListCharacterCommand,      ""},
            { "ip",             SEC_ADMINISTRATOR,  true,  &HandleBanListIPCommand,             ""}
        };
        static std::vector<ChatCommand> baninfoCommandTable =
        {
            { "account",        SEC_ADMINISTRATOR,  true,  &HandleBanInfoAccountCommand,        ""},
            { "character",      SEC_ADMINISTRATOR,  true,  &HandleBanInfoCharacterCommand,      ""},
            { "ip",             SEC_ADMINISTRATOR,  true,  &HandleBanInfoIPCommand,             ""}
        };
        static std::vector<ChatCommand> banCommandTable =
        {
            { "account",        SEC_ADMINISTRATOR,  true,  &HandleBanAccountCommand,            ""},
            { "playeraccount",  SEC_ADMINISTRATOR,  true,  &HandleBanCharacterCommand,          ""},
            { "character",      SEC_ADMINISTRATOR,  true,  &HandleBanAccountByCharCommand,      ""},
            { "ip",             SEC_ADMINISTRATOR,  true,  &HandleBanIPCommand,                 ""},
            { "hwidbyacc",      SEC_ADMINISTRATOR,  true,  &HandleBanHwidCommand,               ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "ban",            SEC_ADMINISTRATOR,  true,  NULL,                                "", banCommandTable },
            { "baninfo",        SEC_ADMINISTRATOR,  true,  NULL,                                "", baninfoCommandTable },
            { "banlist",        SEC_ADMINISTRATOR,  true,  NULL,                                "", banlistCommandTable },
            { "unban",          SEC_ADMINISTRATOR,  true,  NULL,                                "", unbanCommandTable }
        };
        return commandTable;
    }

    static bool HandleBanAccountCommand(ChatHandler* handler, char const* args)
    {
        return HandleBanHelper(BAN_ACCOUNT, args, handler);
    }

    static bool HandleBanCharacterCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* nameStr = strtok((char*)args, " ");
        if (!nameStr)
            return false;

        std::string name = nameStr;

        char* durationStr = strtok(NULL, " ");
        if (!durationStr || !atoi(durationStr))
            return false;

        char* reasonStr = strtok(NULL, "");
        if (!reasonStr)
            return false;

        if (!normalizePlayerName(name))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        switch (sWorld->BanCharacter(name, durationStr, reasonStr, handler->GetSession() ? handler->GetSession()->GetPlayerName().c_str() : ""))
        {
            case BAN_SUCCESS:
            {
                if (atoi(durationStr) > 0)
                    handler->PSendSysMessage(LANG_BAN_YOUBANNED, name.c_str(), secsToTimeString(TimeStringToSecs(durationStr), true).c_str(), reasonStr);
                else
                    handler->PSendSysMessage(LANG_BAN_YOUPERMBANNED, name.c_str(), reasonStr);
                break;
            }
            case BAN_NOTFOUND:
            {
                handler->PSendSysMessage(LANG_BAN_NOTFOUND, "character", name.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }
            default:
                break;
        }

         if (sWorld->getBoolConfig(CONFIG_ANNOUNCE_BAN))
         {
            std::string announce;

            announce = "The character '";
            announce += name.c_str();
            announce += "' was banned for ";
            announce += durationStr;
            announce += " by the character '";
            announce += handler->GetSession() ? handler->GetSession()->GetPlayerName().c_str() : "";
            announce += "'. The reason is: ";
            announce += reasonStr;

            char buff[2048];
            sprintf(buff, handler->GetTrinityString(LANG_SYSTEMMESSAGE), announce.c_str());
            sWorld->SendServerMessage(SERVER_MSG_STRING, buff);
         }

        return true;
    }

    static bool HandleBanAccountByCharCommand(ChatHandler* handler, char const* args)
    {
        return HandleBanHelper(BAN_CHARACTER, args, handler);
    }

    static bool HandleBanIPCommand(ChatHandler* handler, char const* args)
    {
        return HandleBanHelper(BAN_IP, args, handler);
    }

    static bool HandleBanHelper(BanMode mode, char const* args, ChatHandler* handler)
    {
        if (!*args)
            return false;

        char* cnameOrIP = strtok((char*)args, " ");
        if (!cnameOrIP)
            return false;

        std::string nameOrIP = cnameOrIP;

        char* durationStr = strtok(NULL, " ");
        if (!durationStr || !atoi(durationStr))
            return false;

        char* reasonStr = strtok(NULL, "");
        if (!reasonStr)
            return false;

        switch (mode)
        {
            case BAN_ACCOUNT:
                if (!Utf8ToUpperOnlyLatin(nameOrIP))
                {
                    handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, nameOrIP.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                break;
            case BAN_CHARACTER:
                if (!normalizePlayerName(nameOrIP))
                {
                    handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                break;
            case BAN_IP:
                if (!IsIPAddress(nameOrIP.c_str()))
                    return false;
                break;
        }

        if (mode == BAN_CHARACTER)
        {
            // Add queue for mod Decency points
            ObjectGuid charGuid = ObjectMgr::GetPlayerGUIDByName(nameOrIP);
            CharacterDatabase.PQuery("insert into `character_reward` (`owner_guid`,`type`)value ('%u','13');", charGuid.GetGUIDLow());
        }

        switch (sWorld->BanAccount(mode, nameOrIP, durationStr, reasonStr, handler->GetSession() ? handler->GetSession()->GetPlayerName().c_str() : ""))
        {
            case BAN_SUCCESS:
                if (atoi(durationStr) > 0)
                {
                    handler->PSendSysMessage(LANG_BAN_YOUBANNED, nameOrIP.c_str(), secsToTimeString(TimeStringToSecs(durationStr), true).c_str(), reasonStr);
                    sWorld->SendGMText(27002, nameOrIP.c_str(), secsToTimeString(TimeStringToSecs(durationStr), true).c_str(), handler->GetSession()->GetPlayerName().c_str(), reasonStr);
                }
                else
                    handler->PSendSysMessage(LANG_BAN_YOUPERMBANNED, nameOrIP.c_str(), reasonStr);
                break;
            case BAN_SYNTAX_ERROR:
                return false;
            case BAN_NOTFOUND:
                switch (mode)
                {
                    default:
                        handler->PSendSysMessage(LANG_BAN_NOTFOUND, "account", nameOrIP.c_str());
                        break;
                    case BAN_CHARACTER:
                        handler->PSendSysMessage(LANG_BAN_NOTFOUND, "character", nameOrIP.c_str());
                        break;
                    case BAN_IP:
                        handler->PSendSysMessage(LANG_BAN_NOTFOUND, "ip", nameOrIP.c_str());
                        break;
                }
                handler->SetSentErrorMessage(true);
                return false;
        }

        if (sWorld->getBoolConfig(CONFIG_ANNOUNCE_BAN))
        {
            std::string announce;

            if (mode == BAN_CHARACTER)
                announce = "The character '";
            else if (mode == BAN_IP)
                announce = "The IP '";
            else
                announce = "Account '";
            announce += nameOrIP.c_str();
            announce += "' was banned for ";
            announce += durationStr;
            announce += " by the character '";
            announce += handler->GetSession() ? handler->GetSession()->GetPlayerName().c_str() : "";
            announce += "'. The reason is: ";
            announce += reasonStr;

            char buff[2048];
            sprintf(buff, handler->GetTrinityString(LANG_SYSTEMMESSAGE), announce.c_str());
            sWorld->SendServerMessage(SERVER_MSG_STRING, buff);
        }

        return true;
    }

    static bool HandleBanInfoAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* nameStr = strtok((char*)args, "");
        if (!nameStr)
            return false;

        std::string accountName = nameStr;
        if (!Utf8ToUpperOnlyLatin(accountName))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accountId = AccountMgr::GetId(accountName);
        if (!accountId)
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            return true;
        }

        return HandleBanInfoHelper(accountId, accountName.c_str(), handler);
    }

    static bool HandleBanInfoHelper(uint32 accountId, char const* accountName, ChatHandler* handler)
    {
        uint32 account = handler->GetSession()->GetAccountId();
        LoginDatabase.CallBackQuery(Trinity::StringFormat("SELECT FROM_UNIXTIME(bandate), unbandate-bandate, active, unbandate, banreason, bannedby FROM account_banned WHERE id = '%u' ORDER BY bandate ASC", accountId).c_str(), [account, accountName](QueryResult result) -> void
        {
            WorldSessionPtr sess = sWorld->FindSession(account);
            if (!sess)
                return;

            ChatHandler chH = ChatHandler(&*sess);
            if (!result)
            {
                chH.PSendSysMessage(LANG_BANINFO_NOACCOUNTBAN, accountName);
                return;
            }

            chH.PSendSysMessage(LANG_BANINFO_BANHISTORY, accountName);
            do
            {
                Field* fields = result->Fetch();

                time_t unbanDate = time_t(fields[3].GetUInt32());
                bool active = false;
                if (fields[2].GetBool() && (fields[1].GetUInt64() == uint64(0) || unbanDate >= time(NULL)))
                    active = true;
                bool permanent = (fields[1].GetUInt64() == uint64(0));
                std::string banTime = permanent ? chH.GetTrinityString(LANG_BANINFO_INFINITE) : secsToTimeString(fields[1].GetUInt64(), true);
                chH.PSendSysMessage(LANG_BANINFO_HISTORYENTRY,
                    fields[0].GetCString(), banTime.c_str(), active ? chH.GetTrinityString(LANG_BANINFO_YES) : chH.GetTrinityString(LANG_BANINFO_NO), fields[4].GetCString(), fields[5].GetCString());
            }
            while (result->NextRow());
        });

        return true;
    }

    static bool HandleBanInfoCharacterCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string name(args);
        Player* target = sObjectAccessor->FindPlayerByName(name);
        ObjectGuid targetGuid;

        if (!target)
        {
            targetGuid = ObjectMgr::GetPlayerGUIDByName(name);
            if (!targetGuid)
            {
                handler->PSendSysMessage(LANG_BANINFO_NOCHARACTER);
                return false;
            }

        }
        else
            targetGuid = target->GetGUID();
       

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_BANINFO);
        stmt->setUInt64(0, targetGuid.GetGUIDLow());
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (!result)
        {
            handler->PSendSysMessage(LANG_CHAR_NOT_BANNED, name.c_str());
            return true;
        }

        handler->PSendSysMessage(LANG_BANINFO_BANHISTORY, name.c_str());
        do
        {
            Field* fields = result->Fetch();
            time_t unbanDate = time_t(fields[3].GetUInt32());
            bool active = false;
            if (fields[2].GetUInt8() && (!fields[1].GetUInt32() || unbanDate >= time(NULL)))
                active = true;
            bool permanent = (fields[1].GetUInt32() == uint32(0));
            std::string banTime = permanent ? handler->GetTrinityString(LANG_BANINFO_INFINITE) : secsToTimeString(fields[1].GetUInt32(), true);
            handler->PSendSysMessage(LANG_BANINFO_HISTORYENTRY,
                fields[0].GetCString(), banTime.c_str(), active ? handler->GetTrinityString(LANG_BANINFO_YES) : handler->GetTrinityString(LANG_BANINFO_NO), fields[4].GetCString(), fields[5].GetCString());
        }
        while (result->NextRow());

        return true;
    }

    static bool HandleBanInfoIPCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* ipStr = strtok((char*)args, "");
        if (!ipStr)
            return false;

        if (!IsIPAddress(ipStr))
            return false;

        std::string IP = ipStr;

        LoginDatabase.EscapeString(IP);

        uint32 accountId = handler->GetSession()->GetAccountId();
        LoginDatabase.CallBackQuery(Trinity::StringFormat("SELECT ip, FROM_UNIXTIME(bandate), FROM_UNIXTIME(unbandate), unbandate-UNIX_TIMESTAMP(), banreason, bannedby, unbandate-bandate FROM ip_banned WHERE ip = '%s'", IP.c_str()).c_str(), [accountId, handler](QueryResult result) -> void
        {
            WorldSessionPtr sess = sWorld->FindSession(accountId);
            if (!sess)
                return;

            ChatHandler chH = ChatHandler(&*sess);

            if (!result)
            {
                chH.PSendSysMessage(LANG_BANINFO_NOIP);
                return;
            }

            Field* fields = result->Fetch();
            bool permanent = !fields[6].GetUInt64();
            chH.PSendSysMessage(LANG_BANINFO_IPENTRY,
                fields[0].GetCString(), fields[1].GetCString(), permanent ? chH.GetTrinityString(LANG_BANINFO_NEVER) : fields[2].GetCString(),
                permanent ? chH.GetTrinityString(LANG_BANINFO_INFINITE) : secsToTimeString(fields[3].GetUInt64(), true).c_str(), fields[4].GetCString(), fields[5].GetCString());

        });


        return true;
    }

    static bool HandleBanListAccountCommand(ChatHandler* handler, char const* args)
    {
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_EXPIRED_IP_BANS);
        LoginDatabase.Execute(stmt);

        char* filterStr = strtok((char*)args, " ");
        std::string filter = filterStr ? filterStr : "";

        PreparedQueryResult result;

        if (filter.empty())
        {
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BANNED_ALL);
        }
        else
        {
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BANNED_BY_USERNAME);
            stmt->setString(0, filter);
        }

        uint32 accountId = handler->GetSession()->GetAccountId();
        LoginDatabase.CallBackQuery(stmt, [accountId](PreparedQueryResult result) -> void
        {
            if (WorldSessionPtr sess = sWorld->FindSession(accountId))
                sess->BanListHelper(result);
        });

        return true;
    }

    static bool HandleBanListCharacterCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* filterStr = strtok((char*)args, " ");
        if (!filterStr)
            return false;

        std::string filter(filterStr);
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_BY_NAME_FILTER);
        stmt->setString(0, filter);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (!result)
        {
            handler->PSendSysMessage(LANG_BANLIST_NOCHARACTER);
            return true;
        }

        handler->PSendSysMessage(LANG_BANLIST_MATCHINGCHARACTER);

        // Chat short output
        if (handler->GetSession())
        {
            do
            {
                Field* fields = result->Fetch();
                PreparedStatement* stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_BANNED_NAME);
                stmt2->setUInt64(0, fields[0].GetUInt64());
                PreparedQueryResult banResult = CharacterDatabase.Query(stmt2);
                if (banResult)
                    handler->PSendSysMessage("%s", (*banResult)[0].GetCString());
            }
            while (result->NextRow());
        }
        // Console wide output
        else
        {
            handler->SendSysMessage(LANG_BANLIST_CHARACTERS);
            handler->SendSysMessage(" =============================================================================== ");
            handler->SendSysMessage(LANG_BANLIST_CHARACTERS_HEADER);
            do
            {
                handler->SendSysMessage("-------------------------------------------------------------------------------");

                Field* fields = result->Fetch();

                std::string char_name = fields[1].GetString();

                PreparedStatement* stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_BANINFO_LIST);
                stmt2->setUInt64(0, fields[0].GetUInt64());
                PreparedQueryResult banInfo = CharacterDatabase.Query(stmt2);
                if (banInfo)
                {
                    Field* banFields = banInfo->Fetch();
                    do
                    {
                        time_t timeBan = time_t(banFields[0].GetUInt32());
                        tm tmBan;
                        localtime_r(&timeBan, &tmBan);

                        if (banFields[0].GetUInt32() == banFields[1].GetUInt32())
                        {
                            handler->PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                                char_name.c_str(), tmBan.tm_year%100, tmBan.tm_mon+1, tmBan.tm_mday, tmBan.tm_hour, tmBan.tm_min,
                                banFields[2].GetCString(), banFields[3].GetCString());
                        }
                        else
                        {
                            time_t timeUnban = time_t(banFields[1].GetUInt32());
                            tm tmUnban;
                            localtime_r(&timeUnban, &tmUnban);
                            handler->PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                                char_name.c_str(), tmBan.tm_year%100, tmBan.tm_mon+1, tmBan.tm_mday, tmBan.tm_hour, tmBan.tm_min,
                                tmUnban.tm_year%100, tmUnban.tm_mon+1, tmUnban.tm_mday, tmUnban.tm_hour, tmUnban.tm_min,
                                banFields[2].GetCString(), banFields[3].GetCString());
                        }
                    }
                    while (banInfo->NextRow());
                }
            }
            while (result->NextRow());
            handler->SendSysMessage(" =============================================================================== ");
        }

        return true;
    }

    static bool HandleBanListIPCommand(ChatHandler* handler, char const* args)
    {
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_EXPIRED_IP_BANS);
        LoginDatabase.Execute(stmt);

        char* filterStr = strtok((char*)args, " ");
        std::string filter = filterStr ? filterStr : "";
        LoginDatabase.EscapeString(filter);

        PreparedQueryResult result;

        if (filter.empty())
        {
            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_IP_BANNED_ALL);
            result = LoginDatabase.Query(stmt);
        }
        else
        {
            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_IP_BANNED_BY_IP);
            stmt->setString(0, filter);
            result = LoginDatabase.Query(stmt);
        }

        if (!result)
        {
            handler->PSendSysMessage(LANG_BANLIST_NOIP);
            return true;
        }

        handler->PSendSysMessage(LANG_BANLIST_MATCHINGIP);
        // Chat short output
        if (handler->GetSession())
        {
            do
            {
                Field* fields = result->Fetch();
                handler->PSendSysMessage("%s", fields[0].GetCString());
            }
            while (result->NextRow());
        }
        // Console wide output
        else
        {
            handler->SendSysMessage(LANG_BANLIST_IPS);
            handler->SendSysMessage(" ===============================================================================");
            handler->SendSysMessage(LANG_BANLIST_IPS_HEADER);
            do
            {
                handler->SendSysMessage("-------------------------------------------------------------------------------");
                Field* fields = result->Fetch();
                time_t timeBan = time_t(fields[1].GetUInt32());
                tm tmBan;
                localtime_r(&timeBan, &tmBan);
                if (fields[1].GetUInt32() == fields[2].GetUInt32())
                {
                    handler->PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                        fields[0].GetCString(), tmBan.tm_year%100, tmBan.tm_mon+1, tmBan.tm_mday, tmBan.tm_hour, tmBan.tm_min,
                        fields[3].GetCString(), fields[4].GetCString());
                }
                else
                {
                    time_t timeUnban = time_t(fields[2].GetUInt32());
                    tm tmUnban;
                    localtime_r(&timeUnban, &tmUnban);
                    handler->PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                        fields[0].GetCString(), tmBan.tm_year%100, tmBan.tm_mon+1, tmBan.tm_mday, tmBan.tm_hour, tmBan.tm_min,
                        tmUnban.tm_year%100, tmUnban.tm_mon+1, tmUnban.tm_mday, tmUnban.tm_hour, tmUnban.tm_min,
                        fields[3].GetCString(), fields[4].GetCString());
                }
            }
            while (result->NextRow());

            handler->SendSysMessage(" ===============================================================================");
        }

        return true;
    }

    static bool HandleUnBanAccountCommand(ChatHandler* handler, char const* args)
    {
        return HandleUnBanHelper(BAN_ACCOUNT, args, handler);
    }

    static bool HandleUnBanCharacterCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* nameStr = strtok((char*)args, " ");
        if (!nameStr)
            return false;

        std::string name = nameStr;

        if (!normalizePlayerName(name))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sWorld->RemoveBanCharacter(name))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleUnBanAccountByCharCommand(ChatHandler* handler, char const* args)
    {
        return HandleUnBanHelper(BAN_CHARACTER, args, handler);
    }

    static bool HandleUnBanIPCommand(ChatHandler* handler, char const* args)
    {
        return HandleUnBanHelper(BAN_IP, args, handler);
    }

    static bool HandleUnBanHelper(BanMode mode, char const* args, ChatHandler* handler)
    {
        if (!*args)
            return false;

        char* nameOrIPStr = strtok((char*)args, " ");
        if (!nameOrIPStr)
            return false;

        std::string nameOrIP = nameOrIPStr;

        switch (mode)
        {
            case BAN_ACCOUNT:
                if (!Utf8ToUpperOnlyLatin(nameOrIP))
                {
                    handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, nameOrIP.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                break;
            case BAN_CHARACTER:
                if (!normalizePlayerName(nameOrIP))
                {
                    handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                break;
            case BAN_IP:
                if (!IsIPAddress(nameOrIP.c_str()))
                    return false;
                break;
        }

        if (sWorld->RemoveBanAccount(mode, nameOrIP))
            handler->PSendSysMessage(LANG_UNBAN_UNBANNED, nameOrIP.c_str());
        else
            handler->PSendSysMessage(LANG_UNBAN_ERROR, nameOrIP.c_str());

        return true;
    }

    static bool HandleBanHwidCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* accName = strtok((char*)args, " ");
        if (!accName)
            return false;

        std::string account = accName;

        char* reasonStr = strtok(NULL, "");
        if (!reasonStr)
            return false;

        if (!Utf8ToUpperOnlyLatin(account))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME);
        stmt->setString(0, account);
        PreparedQueryResult resultAccounts = LoginDatabase.Query(stmt);
        if (!resultAccounts)
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        do
        {
            Field* fieldsAccount = resultAccounts->Fetch();
            uint32 accountid = fieldsAccount[0].GetUInt32();
            uint64 hwid = fieldsAccount[1].GetUInt64();
            LoginDatabase.PQuery("REPLACE INTO hwid_penalties VALUES (" UI64FMTD ", -1, \"%s\")", hwid, reasonStr);
            handler->PSendSysMessage("Successfully ban accid %u by hwid", accountid);
        } while (resultAccounts->NextRow());
        return true;
    }
};

void AddSC_ban_commandscript()
{
    new ban_commandscript();
}
