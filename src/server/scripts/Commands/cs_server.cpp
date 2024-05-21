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
Name: server_commandscript
%Complete: 100
Comment: All server related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "Config.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "GitRevision.h"
#include "Anticheat.h"

#include <regex>

class server_commandscript : public CommandScript
{
public:
    server_commandscript() : CommandScript("server_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> serverIdleRestartCommandTable =
        {
            { "cancel",         SEC_ADMINISTRATOR,  true,  &HandleServerShutDownCancelCommand,      ""},
            { ""   ,            SEC_ADMINISTRATOR,  true,  &HandleServerIdleRestartCommand,         ""}
        };

        static std::vector<ChatCommand> serverIdleShutdownCommandTable =
        {
            { "cancel",         SEC_ADMINISTRATOR,  true,  &HandleServerShutDownCancelCommand,      ""},
            { ""   ,            SEC_ADMINISTRATOR,  true,  &HandleServerIdleShutDownCommand,        ""}
        };

        static std::vector<ChatCommand> serverRestartCommandTable =
        {
            { "cancel",         SEC_ADMINISTRATOR,  true,  &HandleServerShutDownCancelCommand,      ""},
            { "force",          SEC_ADMINISTRATOR,  true,  &HandleServerForceRestartCommand,        ""},
            { ""   ,            SEC_ADMINISTRATOR,  true,  &HandleServerRestartCommand,             ""}
        };

        static std::vector<ChatCommand> serverShutdownCommandTable =
        {
            { "cancel",         SEC_ADMINISTRATOR,  true,  &HandleServerShutDownCancelCommand,      ""},
            { "force",          SEC_ADMINISTRATOR,  true,  &HandleServerForceShutDownCommand,       ""},
            { ""   ,            SEC_ADMINISTRATOR,  true,  &HandleServerShutDownCommand,            ""}
        };

        static std::vector<ChatCommand> serverSetCommandTable =
        {
            { "difftime",       SEC_CONSOLE,        true,  &HandleServerSetDiffTimeCommand,         ""},
            { "loglevel",       SEC_CONSOLE,        true,  &HandleServerSetLogLevelCommand,         ""},
            { "motd",           SEC_ADMINISTRATOR,  true,  &HandleServerSetMotdCommand,             ""},
            { "closed",         SEC_ADMINISTRATOR,  true,  &HandleServerSetClosedCommand,           ""}
        };

        static std::vector<ChatCommand> serverCommandTable =
        {
            { "corpses",         SEC_GAMEMASTER,     true,  &HandleServerCorpsesCommand,             ""},
            { "anticheatReload", SEC_ADMINISTRATOR,  true,  &HandleReloadAnticheatCommand,           ""},
            { "exit",            SEC_CONSOLE,        true,  &HandleServerExitCommand,                ""},
            { "idlerestart",     SEC_ADMINISTRATOR,  true,  nullptr,                                 "", serverIdleRestartCommandTable },
            { "idleshutdown",    SEC_ADMINISTRATOR,  true,  nullptr,                                 "", serverIdleShutdownCommandTable },
            { "info",            SEC_PLAYER,         true,  &HandleServerInfoCommand,                ""},
            { "motd",            SEC_PLAYER,         true,  &HandleServerMotdCommand,                ""},
            { "plimit",          SEC_ADMINISTRATOR,  true,  &HandleServerPLimitCommand,              ""},
            { "restart",         SEC_ADMINISTRATOR,  true,  nullptr,                                 "", serverRestartCommandTable },
            { "shutdown",        SEC_ADMINISTRATOR,  true,  nullptr,                                 "", serverShutdownCommandTable },
            { "set",             SEC_ADMINISTRATOR,  true,  nullptr,                                 "", serverSetCommandTable }
        };

         static std::vector<ChatCommand> commandTable =
        {
            { "server",         SEC_ADMINISTRATOR,  true,  nullptr,                                  "", serverCommandTable }
        };
        return commandTable;
    }

    // Triggering corpses expire check in world
    static bool HandleServerCorpsesCommand(ChatHandler* /*handler*/, char const* /*args*/)
    {
        sObjectAccessor->RemoveOldCorpses();
        return true;
    }

    static bool HandleReloadAnticheatCommand(ChatHandler* /*handler*/, char const* /*args*/)
    {
        sAnticheatMgr->LoadFromDB();
        return true;
    }

    static bool HandleServerInfoCommand(ChatHandler* handler, char const* /*args*/)
    {
        uint32 playersNum           = sWorld->GetPlayerCount();
        uint32 maxPlayersNum        = sWorld->GetMaxPlayerCount();
        uint32 activeClientsNum     = sWorld->GetActiveSessionCount();
        uint32 queuedClientsNum     = sWorld->GetQueuedSessionCount();
        uint32 maxActiveClientsNum  = sWorld->GetMaxActiveSessionCount();
        uint32 maxQueuedClientsNum  = sWorld->GetMaxQueuedSessionCount();
        std::string uptime          = secsToTimeString(sWorld->GetUptime());
        uint32 updateTime           = sWorld->GetUpdateTime();
        uint32 updateTimeMap        = 0;
        uint32 updateSessionTime    = 0;
        if (auto const& session = handler->GetSession())
        {
            if (Player* player = session->GetPlayer())
            {
                if (player->GetMap() && player->GetMap()->CanCreatedZone())
                {
                    updateTimeMap = player->GetMap()->GetUpdateTime();
                    updateSessionTime = player->GetMap()->GetSessionTime();
                }
                else if (Map* map = sMapMgr->FindBaseNonInstanceMap(player->GetMapId()))
                {
                    updateTimeMap = map->GetUpdateTime();
                    updateSessionTime = map->GetSessionTime();
                }
            }
        }

        handler->PSendSysMessage(GitRevision::GetFullVersion());
        handler->PSendSysMessage(LANG_CONNECTED_USERS, activeClientsNum, maxActiveClientsNum, queuedClientsNum, maxQueuedClientsNum);
        handler->PSendSysMessage(LANG_UPTIME, uptime.c_str());
        handler->PSendSysMessage("World delay: %u ms", updateTime);
        handler->PSendSysMessage("Map delay: %u ms diff %u", updateTimeMap, sWorld->getIntConfig(CONFIG_INTERVAL_MAPUPDATE));
        handler->PSendSysMessage("Session delay: %u ms diff %u", updateSessionTime, sWorld->getIntConfig(CONFIG_INTERVAL_MAP_SESSION_UPDATE));

        // Can't use sWorld->ShutdownMsg here in case of console command
        if (sWorld->IsShuttingDown())
            handler->PSendSysMessage(LANG_SHUTDOWN_TIMELEFT, secsToTimeString(sWorld->GetShutDownTimeLeft()).c_str());

        return true;
    }
    // Display the 'Message of the day' for the realm
    static bool HandleServerMotdCommand(ChatHandler* handler, char const* /*args*/)
    {
        std::string motd;
        for (std::string const& line : sWorld->GetMotd())
            motd += line;
        handler->PSendSysMessage(LANG_MOTD_CURRENT, motd.c_str());
        return true;
    }

    static bool HandleServerPLimitCommand(ChatHandler* handler, char const* args)
    {
        if (*args)
        {
            char* paramStr = strtok((char*)args, " ");
            if (!paramStr)
                return false;

            int32 limit = strlen(paramStr);

            if (strncmp(paramStr, "player", limit) == 0)
                sWorld->SetPlayerSecurityLimit(SEC_PLAYER);
            else if (strncmp(paramStr, "moderator", limit) == 0)
                sWorld->SetPlayerSecurityLimit(SEC_MODERATOR);
            else if (strncmp(paramStr, "gamemaster", limit) == 0)
                sWorld->SetPlayerSecurityLimit(SEC_GAMEMASTER);
            else if (strncmp(paramStr, "administrator", limit) == 0)
                sWorld->SetPlayerSecurityLimit(SEC_ADMINISTRATOR);
            else if (strncmp(paramStr, "reset", limit) == 0)
            {
                sWorld->SetPlayerAmountLimit(sConfigMgr->GetIntDefault("PlayerLimit", 100));
                sWorld->LoadDBAllowedSecurityLevel();
            }
            else
            {
                int32 value = atoi(paramStr);
                if (value < 0)
                    sWorld->SetPlayerSecurityLimit(AccountTypes(-value));
                else
                    sWorld->SetPlayerAmountLimit(uint32(value));
            }
        }

        uint32 playerAmountLimit = sWorld->GetPlayerAmountLimit();
        AccountTypes allowedAccountType = sWorld->GetPlayerSecurityLimit();
        char const* secName = "";
        switch (allowedAccountType)
        {
            case SEC_PLAYER:
                secName = "Player";
                break;
            case SEC_MODERATOR:
                secName = "Moderator";
                break;
            case SEC_GAMEMASTER:
                secName = "Gamemaster";
                break;
            case SEC_ADMINISTRATOR:
                secName = "Administrator";
                break;
            default:
                secName = "<unknown>";
                break;
        }
        handler->PSendSysMessage("Player limits: amount %u, min. security level %s.", playerAmountLimit, secName);

        return true;
    }

    static bool HandleServerShutDownCancelCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (uint32 timer = sWorld->ShutdownCancel())
            handler->PSendSysMessage(LANG_SHUTDOWN_CANCELLED, timer);

        return true;
    }

    static bool IsOnlyUser(WorldSession* mySession)
    {
        // if mySession is null then the shutdown command was issued from console (local or remote), always shutdown in this case
        if (!mySession)
            return true;

        // check if there is any session connected from a different address
        std::string myAddr = mySession ? mySession->GetRemoteAddress() : "";
        SessionMap const& sessions = sWorld->GetAllSessions();
        for (SessionMap::value_type const& session : sessions)
            if (session.second && myAddr != session.second->GetRemoteAddress())
                return false;
        return true;
    }

    static bool HandleServerShutDownCommand(ChatHandler* handler, char const* args)
    {
        return ShutdownServer(handler, args, 0, SHUTDOWN_EXIT_CODE);
    }

    static bool HandleServerRestartCommand(ChatHandler* handler, char const* args)
    {
        return ShutdownServer(handler, args, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE);
    }

    static bool HandleServerForceShutDownCommand(ChatHandler* handler, char const* args)
    {
        return ShutdownServer(handler, args, SHUTDOWN_MASK_FORCE, SHUTDOWN_EXIT_CODE);
    }

    static bool HandleServerForceRestartCommand(ChatHandler* handler, char const* args)
    {
        return ShutdownServer(handler, args, SHUTDOWN_MASK_FORCE | SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE);
    }

    static bool HandleServerIdleShutDownCommand(ChatHandler* handler, char const* args)
    {
        return ShutdownServer(handler, args, SHUTDOWN_MASK_IDLE, SHUTDOWN_EXIT_CODE);
    }

    static bool HandleServerIdleRestartCommand(ChatHandler* handler, char const* args)
    {
        return ShutdownServer(handler, args, SHUTDOWN_MASK_RESTART | SHUTDOWN_MASK_IDLE, RESTART_EXIT_CODE);
    }

    // Exit the realm
    static bool HandleServerExitCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->SendSysMessage(LANG_COMMAND_EXIT);
        World::StopNow(SHUTDOWN_EXIT_CODE);
        return true;
    }

    // Define the 'Message of the day' for the realm
    static bool HandleServerSetMotdCommand(ChatHandler* handler, char const* args)
    {
        sWorld->SetMotd(args);
        handler->PSendSysMessage(LANG_MOTD_NEW, args);
        return true;
    }

    // Set whether we accept new clients
    static bool HandleServerSetClosedCommand(ChatHandler* handler, char const* args)
    {
        if (strncmp(args, "on", 3) == 0)
        {
            handler->SendSysMessage(LANG_WORLD_CLOSED);
            sWorld->SetClosed(true);
            return true;
        }
        if (strncmp(args, "off", 4) == 0)
        {
            handler->SendSysMessage(LANG_WORLD_OPENED);
            sWorld->SetClosed(false);
            return true;
        }
        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // Set the level of logging
    static bool HandleServerSetLogLevelCommand(ChatHandler* /*handler*/, char const* args)
    {
        if (!*args)
            return false;

        char* type = strtok((char*)args, " ");
        char* name = strtok(nullptr, " ");
        char* level = strtok(nullptr, " ");

        if (!type || !name || !level || *name == '\0' || *level == '\0' || (*type != 'a' && *type != 'l'))
            return false;

        sLog->SetLogLevel(name, level, *type == 'l');
        return true;
    }

    // set diff time record interval
    static bool HandleServerSetDiffTimeCommand(ChatHandler* /*handler*/, char const* args)
    {
        if (!*args)
            return false;

        char* newTimeStr = strtok((char*)args, " ");
        if (!newTimeStr)
            return false;

        int32 newTime = atoi(newTimeStr);
        if (newTime < 0)
            return false;

        sWorld->SetRecordDiffInterval(newTime);
        printf("Record diff every %u ms\n", newTime);

        return true;
    }

private:
    static bool ParseExitCode(char const* exitCodeStr, int32& exitCode)
    {
        exitCode = atoi(exitCodeStr);

        // Handle atoi() errors
        if (exitCode == 0 && (exitCodeStr[0] != '0' || exitCodeStr[1] != '\0'))
            return false;

        // Exit code should be in range of 0-125, 126-255 is used
        // in many shells for their own return codes and code > 255
        // is not supported in many others
        if (exitCode < 0 || exitCode > 125)
            return false;

        return true;
    }

    static bool ShutdownServer(ChatHandler* handler, char const* args, uint32 shutdownMask, int32 defaultExitCode)
    {
        if (!*args)
            return false;

        // #delay [#exit_code] [reason]
        int32 delay = 0;
        char* delayStr = strtok((char*)args, " ");
        if (!delayStr)
            return false;

        if (isNumeric(delayStr))
        {
            delay = atoi(delayStr);
            // Prevent interpret wrong arg value as 0 secs shutdown time
            if ((delay == 0 && (delayStr[0] != '0' || delayStr[1] != '\0')) || delay < 0)
                return false;
        }
        else
        {
            delay = TimeStringToSecs(std::string(delayStr));

            if (delay == 0)
                return false;
        }

        char* exitCodeStr = nullptr;

        char reason[256] = { 0 };

        while (char* nextToken = strtok(nullptr, " "))
        {
            if (isNumeric(nextToken))
                exitCodeStr = nextToken;
            else
            {
                strcat(reason, nextToken);
                if (char* remainingTokens = strtok(nullptr, "\0"))
                {
                    strcat(reason, " ");
                    strcat(reason, remainingTokens);
                }
                break;
            }
        }

        int32 exitCode = defaultExitCode;
        if (exitCodeStr)
            if (!ParseExitCode(exitCodeStr, exitCode))
                return false;

        if (delay < (int32)sWorld->getIntConfig(CONFIG_FORCE_SHUTDOWN_THRESHOLD) && !(shutdownMask & SHUTDOWN_MASK_FORCE) && !IsOnlyUser(handler->GetSession()))
        {
            delay = (int32)sWorld->getIntConfig(CONFIG_FORCE_SHUTDOWN_THRESHOLD);
            handler->PSendSysMessage(LANG_SHUTDOWN_DELAYED, delay);
        }

        sWorld->ShutdownServ(delay, shutdownMask, static_cast<uint8>(exitCode),  std::string(reason));

        return true;
    }
};

void AddSC_server_commandscript()
{
    new server_commandscript();
}
