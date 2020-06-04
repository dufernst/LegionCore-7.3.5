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
Name: debug_commandscript
%Complete: 100
Comment: All debug related commands
Category: commandscripts
EndScriptData */

#include "BattlegroundMgr.h"
#include "Cell.h"
#include "ChallengeMgr.h"
#include "Chat.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "GroupMgr.h"
#include "LFGListMgr.h"
#include "LFGMgr.h"
#include "LFGQueue.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "OutdoorPvP.h"
#include "Packets/ChatPackets.h"
#include "Packets/InstancePackets.h"
#include "Packets/LfgListPackets.h"
#include "Packets/MiscPackets.h"
#include "PlayerDefines.h"
#include "ScriptMgr.h"
#include "Vehicle.h"
#include <fstream>
#include "Garrison.h"

class debug_commandscript : public CommandScript
{
public:
    debug_commandscript() : CommandScript("debug_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> debugPlayCommandTable =
        {
            { "cinematic",      SEC_MODERATOR,      false, &HandleDebugPlayCinematicCommand,   ""},
            { "movie",          SEC_MODERATOR,      false, &HandleDebugPlayMovieCommand,       ""},
            { "sound",          SEC_MODERATOR,      false, &HandleDebugPlaySoundCommand,       ""}
        };
        static std::vector<ChatCommand> debugSendCommandTable =
        {
            { "buyerror",       SEC_ADMINISTRATOR,  false, &HandleDebugSendBuyErrorCommand,       ""},
            { "channelnotify",  SEC_ADMINISTRATOR,  false, &HandleDebugSendChannelNotifyCommand,  ""},
            { "chatmmessage",   SEC_ADMINISTRATOR,  false, &HandleDebugSendChatMsgCommand,        ""},
            { "compress",       SEC_ADMINISTRATOR,  false, &HandleDebugSendCompressCommand,       ""},
            { "equiperror",     SEC_ADMINISTRATOR,  false, &HandleDebugSendEquipErrorCommand,     ""},
            { "largepacket",    SEC_ADMINISTRATOR,  false, &HandleDebugSendLargePacketCommand,    ""},
            { "multi",          SEC_ADMINISTRATOR,  false, &HandleDebugSendMultCommand,           ""},
            { "opcode",         SEC_ADMINISTRATOR,  false, &HandleDebugSendOpcodeCommand,         ""},
            { "qinvalidmsg",    SEC_ADMINISTRATOR,  false, &HandleDebugSendQuestInvalidMsgCommand, ""},
            { "qpartymsg",      SEC_ADMINISTRATOR,  false, &HandleDebugSendQuestPartyMsgCommand,  ""},
            { "sellerror",      SEC_ADMINISTRATOR,  false, &HandleDebugSendSellErrorCommand,      ""},
            { "setphaseshift",  SEC_ADMINISTRATOR,  false, &HandleDebugSendSetPhaseShiftCommand,  ""},
            { "spellfail",      SEC_ADMINISTRATOR,  false, &HandleDebugSendSpellFailCommand,      ""},
            { "snedLfgListStatus", SEC_ADMINISTRATOR,  false, &HandleDebugSendLFGListStatusUpdate, ""}
        };
        
        static std::vector<ChatCommand> debugGarrisonCommandTable =
        {
            { "generatemissions",   SEC_MODERATOR,  false, &HandleHenerateMissionsCommand,      ""}
        };
        
        static std::vector<ChatCommand> debugCommandTable =
        {
            { "anim",           SEC_GAMEMASTER,     false, &HandleDebugAnimCommand,            ""},
            { "areatriggers",   SEC_ADMINISTRATOR,  false, &HandleDebugAreaTriggersCommand,    ""},
            { "arena",          SEC_REALM_LEADER,   false, &HandleDebugArenaCommand,           ""},
            { "attackpower",    SEC_REALM_LEADER,   false, &HandleDebugModifyAttackpowerCommand,    ""},
            { "backward",       SEC_REALM_LEADER,   false, &HandleDebugMoveBackward,           ""},
            { "bg",             SEC_REALM_LEADER,   false, &HandleDebugBattlegroundCommand,    ""},
            { "ashran",         SEC_REALM_LEADER,   false, &HandleDebugAshranCommand,          ""},
            { "lfg",            SEC_REALM_LEADER,   false, &HandleDebugLFGCommand,             ""},
            { "challenge",      SEC_REALM_LEADER,   false, &HandleDebugChallengeCommand,       ""},
            { "crit",           SEC_REALM_LEADER,   false, &HandleDebugModifyCritChanceCommand,     ""},
            { "entervehicle",   SEC_ADMINISTRATOR,  false, &HandleDebugEnterVehicleCommand,    ""},
            { "getdynamicvalue",SEC_ADMINISTRATOR,  false, &HandleDebugGetDynamicValueCommand, ""},
            { "getitemstate",   SEC_ADMINISTRATOR,  false, &HandleDebugGetItemStateCommand,    ""},
            { "getitemvalue",   SEC_ADMINISTRATOR,  false, &HandleDebugGetItemValueCommand,    ""},
            { "getvalue",       SEC_ADMINISTRATOR,  false, &HandleDebugGetValueCommand,        ""},
            { "haste",          SEC_REALM_LEADER,   false, &HandleDebugModifyHasteCommand,     ""},
            { "hit",            SEC_REALM_LEADER,   false, &HandleDebugModifyHitCommand,       ""},
            { "versality",      SEC_REALM_LEADER,   false, &HandleDebugModifyVersalityCommand, ""},
            { "hostile",        SEC_REALM_LEADER,   false, &HandleDebugHostileRefListCommand,  ""},
            { "itemexpire",     SEC_ADMINISTRATOR,  false, &HandleDebugItemExpireCommand,      ""},
            { "jump",           SEC_ADMINISTRATOR,  false, &HandleDebugMoveJump,               ""},
            { "load_z",         SEC_ADMINISTRATOR,  false, &HandleDebugLoadZ,                  ""},
            { "lootrecipient",  SEC_GAMEMASTER,     false, &HandleDebugGetLootRecipientCommand, ""},
            { "los",            SEC_MODERATOR,      false, &HandleDebugLoSCommand,             ""},
            { "mailstatus",     SEC_ADMINISTRATOR,  false, &HandleSendMailStatus,              ""},
            { "mapinfo",        SEC_ADMINISTRATOR,  false, &HandleDebugGetMapInfoCommand,      ""},
            { "mastery",        SEC_REALM_LEADER,   false, &HandleDebugModifyMasteryCommand,        ""},
            { "mod32value",     SEC_ADMINISTRATOR,  false, &HandleDebugMod32ValueCommand,      ""},
            { "moveflags",      SEC_ADMINISTRATOR,  false, &HandleDebugMoveflagsCommand,       ""},
            { "phase",          SEC_MODERATOR,      false, &HandleDebugPhaseCommand,           ""},
            { "play",           SEC_MODERATOR,      false, NULL,              "", debugPlayCommandTable },
            { "send",           SEC_ADMINISTRATOR,  false, NULL,              "", debugSendCommandTable },
            { "setaurastate",   SEC_ADMINISTRATOR,  false, &HandleDebugSetAuraStateCommand,    ""},
            { "setbit",         SEC_ADMINISTRATOR,  false, &HandleDebugSet32BitCommand,        ""},
            { "setdynamicvalue",SEC_ADMINISTRATOR,  false, &HandleDebugSetDynamicValueCommand, ""},
            { "setitemvalue",   SEC_ADMINISTRATOR,  false, &HandleDebugSetItemValueCommand,    ""},
            { "setvalue",       SEC_ADMINISTRATOR,  false, &HandleDebugSetValueCommand,        ""},
            { "battlepeterror", SEC_ADMINISTRATOR,  false, &HandleDebugBattlePetErrorCommand, ""},
            { "battlepeterequestfailed", SEC_ADMINISTRATOR,  false, &HandleDebugBattlePetRequestFailedCommand, ""},
            { "setvid",         SEC_ADMINISTRATOR,  false, &HandleDebugSetVehicleIdCommand,    ""},
            { "spawnvehicle",   SEC_ADMINISTRATOR,  false, &HandleDebugSpawnVehicleCommand,    ""},
            { "spellpower",     SEC_REALM_LEADER,   false, &HandleDebugModifySpellpowerCommand,     ""},
            { "threat",         SEC_ADMINISTRATOR,  false, &HandleDebugThreatListCommand,      ""},
            { "tradestatus",    SEC_ADMINISTRATOR,  false, &HandleSendTradeStatus,             ""},
            { "update",         SEC_ADMINISTRATOR,  false, &HandleDebugUpdateCommand,          ""},
            { "updatecriteria", SEC_ADMINISTRATOR,  false, &HandleDebugUpdateCriteriaCommand,  ""},
            { "uws",            SEC_ADMINISTRATOR,  false, &HandleDebugUpdateWorldStateCommand, ""},
            { "streamingmovies",SEC_ADMINISTRATOR,  false, &HandleDebugStreamingMoviesCommand,  ""},
            { "movementinfo",   SEC_ADMINISTRATOR,  false, &HandleDebugMovementInfo,           ""},
            { "session",        SEC_ADMINISTRATOR,  false, &HandleDebugSession,                ""},
            { "pvpstatenable",  SEC_ADMINISTRATOR,  false, &HandleDebugPvPStatEnable,          ""},
            { "pvpstatdisable", SEC_ADMINISTRATOR,  false, &HandleDebugPvPStatDisable,         ""},
            { "challengeloot",  SEC_REALM_LEADER,   false, &HandleDebugChallengeLootCommand,   ""},
            { "group",          SEC_REALM_LEADER,   false, &HandleDebugGroupCommand,           ""},
            { "pvpmystic",      SEC_CONFIRMED_GAMEMASTER, false, &HandleDebugPvpMysticCommand, ""},
            { "crash",          SEC_ADMINISTRATOR,  false, &HandleDebugCrash,                  ""},
            { "freeze",         SEC_ADMINISTRATOR,  false, &HandleDebugFreeze,                 ""},
            { "combat",         SEC_ADMINISTRATOR,  false, &HandleDebugPlayerCombat,           ""},
            { "garrison",       SEC_ADMINISTRATOR,  false, nullptr,                             "", debugGarrisonCommandTable },
            { "transport",      SEC_ADMINISTRATOR,  false, &HandleDebugStartTransport,         ""},
            { "setelevel",      SEC_ADMINISTRATOR,  false, &HandleDebugSetEffectiveLevel,      ""},
            { "pvelogs",        SEC_ADMINISTRATOR,  false, &HandleDebugPvELogsCommand,         ""},
            { "setkillpoints",  SEC_GAMEMASTER,     false, &HandleDebugKillPointsCommand,      ""},
            { "abort",          SEC_GAMEMASTER,     false, &HandleDebugAbort,                  ""},
            { "exception",      SEC_GAMEMASTER,     false, &HandleDebugException,              ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "debug",          SEC_MODERATOR,      true,  NULL,                  "", debugCommandTable },
            { "wpgps",          SEC_ADMINISTRATOR,  false, &HandleWPGPSCommand,                ""}
        };
        return commandTable;
    }

    static bool HandleSendTradeStatus(ChatHandler* handler, char const* args)
    {
        // USAGE: .debug play cinematic #cinematicid
        // #cinematicid - ID decimal number from CinemaicSequences.dbc (1st column)
        if (!*args)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = atoi((char*)args);

        //handler->GetSession()->SendTradeStatus(TradeStatus(id));
        return true;
    }

    static bool HandleSendMailStatus(ChatHandler* handler, char const* args)
    {
        // USAGE: .debug play cinematic #cinematicid
        // #cinematicid - ID decimal number from CinemaicSequences.dbc (1st column)
        if (!*args)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = atoi((char*)args);
        uint32 id2 = atoi((char*)args);

        handler->GetSession()->GetPlayer()->SendMailResult(0, MailResponseType(id), MailResponseResult(id2));
        return true;
    }

    static bool HandleDebugPlayCinematicCommand(ChatHandler* handler, char const* args)
    {
        // USAGE: .debug play cinematic #cinematicid
        // #cinematicid - ID decimal number from CinemaicSequences.dbc (1st column)
        if (!*args)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = atoi((char*)args);
        handler->GetSession()->GetPlayer()->SendCinematicStart(id);
        return true;
    }

    static bool HandleDebugPlayMovieCommand(ChatHandler* handler, char const* args)
    {
        // USAGE: .debug play movie #movieid
        // #movieid - ID decimal number from Movie.dbc (1st column)
        if (!*args)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = atoi((char*)args);

        if (!sMovieStore.LookupEntry(id))
        {
            handler->PSendSysMessage(LANG_MOVIE_NOT_EXIST, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->GetSession()->GetPlayer()->SendMovieStart(id);
        return true;
    }

    //Play sound
    static bool HandleDebugPlaySoundCommand(ChatHandler* handler, char const* args)
    {
        // USAGE: .debug playsound #soundid
        // #soundid - ID decimal number from SoundEntries.dbc (1st column)
        if (!*args)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 soundId = atoi((char*)args);
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (handler->GetSession()->GetPlayer()->GetSelection())
            unit->PlayDistanceSound(soundId, handler->GetSession()->GetPlayer());
        else
            unit->PlayDirectSound(soundId, handler->GetSession()->GetPlayer());

        handler->PSendSysMessage(LANG_YOU_HEAR_SOUND, soundId);
        return true;
    }

    static bool HandleDebugSendSpellFailCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* result = strtok((char*)args, " ");
        if (!result)
            return false;

        uint8 failNum = (uint8)atoi(result);
        if (failNum == 0 && *result != '0')
            return false;

        char* fail1 = strtok(NULL, " ");
        uint8 failArg1 = fail1 ? (uint8)atoi(fail1) : 0;

        char* fail2 = strtok(NULL, " ");
        uint8 failArg2 = fail2 ? (uint8)atoi(fail2) : 0;

        WorldPacket data(SMSG_CAST_FAILED, 5);
        data.WriteBit(!failArg2);
        data.WriteBit(!failArg1);
        data << uint32(failNum);                                // problem
        data << uint32(133);                                    // spellId
        if (failArg2)
            data << uint32(failArg2);
        data << uint8(0);                                       // single cast or multi 2.3 (0/1)
        if (failArg1)
            data << uint32(failArg2);

        if (Player* player = handler->GetSession()->GetPlayer())
            player->SendDirectMessage(&data);

        return true;
    }

    static bool HandleDebugSendLFGListStatusUpdate(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        auto status = atoi(const_cast<char*>(args));
        if (!status)
            return false;

        if (auto player = handler->GetSession()->GetPlayer())
            if (auto group = player->GetGroup())
                if (auto entry = sLFGListMgr->GetEntrybyGuidLow(group->GetGUIDLow()))
                    sLFGListMgr->SendLFGListStatusUpdate(entry, handler->GetSession(), true, LFGListStatus(status));

        return true;
    }
    
    static bool HandleDebugSendCompressCommand(ChatHandler* handler, char const* args)
    {
        /*std::ostringstream ss;
        for(int32 i = 0; i < 100; ++i)
            ss << "TEST" << i << " ";

        ss << "FINISH";

        WorldPacket data(SMSG_PRINT_NOTIFICATION, 2 + ss.str().length());
        data.WriteBits(ss.str().length(), 12);
        data.FlushBits();
        data.WriteString(ss.str().c_str());
        //handler->SendDirectMessage(&data);

        WorldPacket buff;
        buff.Compress(handler->GetSession()->GetCompressionStream(), &data);
        handler->SendDirectMessage(&buff);*/
        return true;
    }

    static bool HandleDebugSendMultCommand(ChatHandler* handler, char const* args)
    {
        //uint32 const count = 3;
        //std::string const msg = "TEST TEST TEST III A B C D E F G 1 2 3 4 5 6 7 8 9 0 K O P D S";

        //WorldPacket data(SMSG_MULTIPLE_PACKETS, 100);
        //for (int32 i = 0; i < count; ++i)
        //    data = WorldPackets::Chat::PrintNotification(msg).Write();

        //handler->SendDirectMessage(&data);
        return true;
    }

    static bool HandleDebugSendEquipErrorCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        InventoryResult msg = InventoryResult(atoi(args));
        handler->GetSession()->GetPlayer()->SendEquipError(msg, NULL, NULL);
        return true;
    }

    static bool HandleDebugSendSellErrorCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        SellResult msg = SellResult(atoi(args));
        handler->GetSession()->GetPlayer()->SendSellError(msg, 0, ObjectGuid::Empty);
        return true;
    }

    static bool HandleDebugSendBuyErrorCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        BuyResult msg = BuyResult(atoi(args));
        handler->GetSession()->GetPlayer()->SendBuyError(msg);
        return true;
    }

    static bool HandleDebugSendOpcodeCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        Player* player = NULL;
        if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
            player = handler->GetSession()->GetPlayer();
        else
            player = (Player*)unit;

        if (!unit)
            unit = player;

        std::ifstream ifs("opcode.txt");
        if (ifs.bad())
            return false;

        // remove comments from file
        std::stringstream parsedStream;
        while (!ifs.eof())
        {
            char commentToken[2];
            ifs.get(commentToken[0]);
            if (commentToken[0] == '/' && !ifs.eof())
            {
                ifs.get(commentToken[1]);
                // /* comment
                if (commentToken[1] == '*')
                {
                    while (!ifs.eof())
                    {
                        ifs.get(commentToken[0]);
                        if (commentToken[0] == '*' && !ifs.eof())
                        {
                            ifs.get(commentToken[1]);
                            if (commentToken[1] == '/')
                                break;
                            ifs.putback(commentToken[1]);
                        }
                    }
                    continue;
                }
                // line comment
                if (commentToken[1] == '/')
                {
                    std::string str;
                    getline(ifs, str);
                    continue;
                }
                // regular data
                ifs.putback(commentToken[1]);
            }
            parsedStream.put(commentToken[0]);
        }
        ifs.close();

        uint32 opcode;
        parsedStream >> opcode;

        WorldPacket data(OpcodeServer(opcode), 0);

        while (!parsedStream.eof())
        {
            std::string type;
            parsedStream >> type;

            if (type == "")
                break;

            if (type == "uint8")
            {
                uint16 val1;
                parsedStream >> val1;
                data << uint8(val1);
            }
            else if (type == "uint16")
            {
                uint16 val2;
                parsedStream >> val2;
                data << val2;
            }
            else if (type == "uint32")
            {
                uint32 val3;
                parsedStream >> val3;
                data << val3;
            }
            else if (type == "uint64")
            {
                uint64 val4;
                parsedStream >> val4;
                data << val4;
            }
            else if (type == "float")
            {
                float val5;
                parsedStream >> val5;
                data << val5;
            }
            else if (type == "string")
            {
                std::string val6;
                parsedStream >> val6;
                data << val6;
            }
            else if (type == "appitsguid")
            {
                data << unit->GetGUID();
            }
            else if (type == "appmyguid")
            {
                data << player->GetGUID();
            }
            else if (type == "appgoguid")
            {
                GameObject* obj = handler->GetNearbyGameObject();
                if (!obj)
                {
                    handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, UI64LIT(0));
                    handler->SetSentErrorMessage(true);
                    ifs.close();
                    return false;
                }
                data << obj->GetGUID();
            }
            else if (type == "goguid")
            {
                GameObject* obj = handler->GetNearbyGameObject();
                if (!obj)
                {
                    handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, UI64LIT(0));
                    handler->SetSentErrorMessage(true);
                    ifs.close();
                    return false;
                }
                data << ObjectGuid(obj->GetGUID());
            }
            else if (type == "myguid")
            {
                data << ObjectGuid(player->GetGUID());
            }
            else if (type == "itsguid")
            {
                data << ObjectGuid(unit->GetGUID());
            }
            else if (type == "itspos")
            {
                data << unit->GetPositionX();
                data << unit->GetPositionY();
                data << unit->GetPositionZ();
            }
            else if (type == "mypos")
            {
                data << player->GetPositionX();
                data << player->GetPositionY();
                data << player->GetPositionZ();
            }
            else
            {
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "Sending opcode that has unknown type '%s'", type.c_str());
                break;
            }
        }
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Sending opcode %u", data.GetOpcode());
        data.hexlike();
        player->GetSession()->SendPacket(&data, true);
        handler->PSendSysMessage(LANG_COMMAND_OPCODESENT, data.GetOpcode(), unit->GetName());
        return true;
    }

    static bool HandleDebugUpdateWorldStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        auto cmdArgs = CommandArgs(handler, args, { CommandArgs::ARG_UINT, CommandArgs::ARG_UINT, CommandArgs::ARG_INT });
        if (!cmdArgs.ValidArgs())
            return false;

        handler->GetSession()->GetPlayer()->SendUpdateWorldState(cmdArgs.GetArgUInt(0), cmdArgs.GetArgUInt(1), cmdArgs.GetArgInt(2));
        return true;
    }

    static bool HandleDebugStreamingMoviesCommand(ChatHandler* handler, char const* args)
    {
        char* w = strtok((char*)args, " ");
        if (!w)
            return false;

        WorldPackets::Misc::StreamingMovie movie;
        movie.MovieIDs.push_back((uint32)atoi(w));
        handler->GetSession()->GetPlayer()->SendDirectMessage(movie.Write());
        return true;
    }

    static bool HandleDebugAreaTriggersCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player->isDebugAreaTriggers)
        {
            handler->PSendSysMessage(LANG_DEBUG_AREATRIGGER_ON);
            player->isDebugAreaTriggers = true;
        }
        else
        {
            handler->PSendSysMessage(LANG_DEBUG_AREATRIGGER_OFF);
            player->isDebugAreaTriggers = false;
        }
        return true;
    }

    //Send notification in channel
    static bool HandleDebugSendChannelNotifyCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* name = "test";
        uint8 code = atoi(args);

        /*WorldPacket data(SMSG_CHANNEL_NOTIFY, (1+10));
        data << code;                                           // notify type
        data << name;                                           // channel name
        data << uint32(0);
        data << uint32(0);
        handler->SendDirectMessage(&data);*/
        return true;
    }

    //Send notification in chat
    static bool HandleDebugSendChatMsgCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* msg = "testtest";
        uint8 type = atoi(args);
        WorldPackets::Chat::Chat packet;
        packet.Initialize(CHAT_MSG_IGNORED, LANG_UNIVERSAL, handler->GetSession()->GetPlayer(), handler->GetSession()->GetPlayer(), msg);
        if (Player* player = handler->GetSession()->GetPlayer())
            player->SendDirectMessage(packet.Write());
        return true;
    }

    static bool HandleDebugSendQuestPartyMsgCommand(ChatHandler* handler, char const* args)
    {
        uint8 msg = atol((char*)args);
        handler->GetSession()->GetPlayer()->SendPushToPartyResponse(handler->GetSession()->GetPlayer(), msg);
        return true;
    }

    static bool HandleDebugGetLootRecipientCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* target = handler->getSelectedCreature();
        if (!target)
            return false;

        handler->PSendSysMessage("Loot recipient for creature %s (GUID %u, DB GUID %u) is %s", target->GetName(), target->GetGUID().GetGUIDLow(), target->GetDBTableGUIDLow(), target->hasLootRecipient() ? (target->GetLootRecipient() ? target->GetLootRecipient()->GetName() : "offline") : "no loot recipient");
        return true;
    }

    static bool HandleDebugSendQuestInvalidMsgCommand(ChatHandler* handler, char const* args)
    {
        uint32 msg = atol((char*)args);
        handler->GetSession()->GetPlayer()->SendCanTakeQuestResponse(msg, NULL);
        return true;
    }

    static bool HandleDebugGetItemStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // std::string itemState = args;

        // ItemUpdateState state = ITEM_UNCHANGED;
        // bool listQueue = false;
        // bool checkAll = false;

        // if (itemState == "unchanged")
            // state = ITEM_UNCHANGED;
        // else if (itemState == "changed")
            // state = ITEM_CHANGED;
        // else if (itemState == "new")
            // state = ITEM_NEW;
        // else if (itemState == "removed")
            // state = ITEM_REMOVED;
        // else if (itemState == "queue")
            // listQueue = true;
        // else if (itemState == "check_all")
            // checkAll = true;
        // else
            // return false;

        // Player* player = handler->getSelectedPlayer();
        // if (!player)
            // player = handler->GetSession()->GetPlayer();

        // if (!listQueue && !checkAll)
        // {
            // itemState = "The player has the following " + itemState + " items: ";
            // handler->SendSysMessage(itemState.c_str());
            // for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; ++i)
            // {
                // if (i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                    // continue;

                // if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                // {
                    // if (Bag* bag = item->ToBag())
                    // {
                        // for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                            // if (Item* item2 = bag->GetItemByPos(j))
                                // if (item2->GetState() == state)
                                    // handler->PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item2->GetSlot(), item2->GetGUID().GetGUIDLow(), item2->GetOwnerGUID().GetGUIDLow());
                    // }
                    // else if (item->GetState() == state)
                        // handler->PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUID().GetGUIDLow(), item->GetOwnerGUID().GetGUIDLow());
                // }
            // }
        // }

        // if (listQueue)
        // {
            // std::vector<Item*>& updateQueue = player->GetItemUpdateQueue();
            // for (size_t i = 0; i < updateQueue.size(); ++i)
            // {
                // Item* item = updateQueue[i];
                // if (!item)
                    // continue;

                // Bag* container = item->GetContainer();
                // uint8 bagSlot = container ? container->GetSlot() : uint8(INVENTORY_SLOT_BAG_0);

                // std::string st;
                // switch (item->GetState())
                // {
                    // case ITEM_UNCHANGED:
                        // st = "unchanged";
                        // break;
                    // case ITEM_CHANGED:
                        // st = "changed";
                        // break;
                    // case ITEM_NEW:
                        // st = "new";
                        // break;
                    // case ITEM_REMOVED:
                        // st = "removed";
                        // break;
                // }

                // handler->PSendSysMessage("bag: %d slot: %d guid: %d - state: %s", bagSlot, item->GetSlot(), item->GetGUID().GetGUIDLow(), st.c_str());
            // }
            // if (updateQueue.empty())
                // handler->PSendSysMessage("The player's updatequeue is empty");
        // }

        // if (checkAll)
        // {
            // bool error = false;
            // std::vector<Item*>& updateQueue = player->GetItemUpdateQueue();
            // for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; ++i)
            // {
                // if (i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                    // continue;

                // Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                // if (!item)
                    // continue;

                // if (item->GetSlot() != i)
                // {
                    // handler->PSendSysMessage("Item with slot %d and guid %d has an incorrect slot value: %d", i, item->GetGUID().GetGUIDLow(), item->GetSlot());
                    // error = true;
                    // continue;
                // }

                // if (item->GetOwnerGUID() != player->GetGUID())
                // {
                    // handler->PSendSysMessage("The item with slot %d and itemguid %d does have non-matching owner guid (%d) and player guid (%d) !", item->GetSlot(), item->GetGUID().GetGUIDLow(), item->GetOwnerGUID().GetGUIDLow(), player->GetGUID().GetGUIDLow());
                    // error = true;
                    // continue;
                // }

                // if (Bag* container = item->GetContainer())
                // {
                    // handler->PSendSysMessage("The item with slot %d and guid %d has a container (slot: %d, guid: %d) but shouldn't!", item->GetSlot(), item->GetGUID().GetGUIDLow(), container->GetSlot(), container->GetGUID().GetGUIDLow());
                    // error = true;
                    // continue;
                // }

                // if (item->IsInUpdateQueue())
                // {
                    // uint16 qp = item->GetQueuePos();
                    // if (qp > updateQueue.size())
                    // {
                        // handler->PSendSysMessage("The item with slot %d and guid %d has its queuepos (%d) larger than the update queue size! ", item->GetSlot(), item->GetGUID().GetGUIDLow(), qp);
                        // error = true;
                        // continue;
                    // }

                    // if (updateQueue[qp] == NULL)
                    // {
                        // handler->PSendSysMessage("The item with slot %d and guid %d has its queuepos (%d) pointing to NULL in the queue!", item->GetSlot(), item->GetGUID().GetGUIDLow(), qp);
                        // error = true;
                        // continue;
                    // }

                    // if (updateQueue[qp] != item)
                    // {
                        // handler->PSendSysMessage("The item with slot %d and guid %d has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", item->GetSlot(), item->GetGUID().GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUID().GetGUIDLow());
                        // error = true;
                        // continue;
                    // }
                // }
                // else if (item->GetState() != ITEM_UNCHANGED)
                // {
                    // handler->PSendSysMessage("The item with slot %d and guid %d is not in queue but should be (state: %d)!", item->GetSlot(), item->GetGUID().GetGUIDLow(), item->GetState());
                    // error = true;
                    // continue;
                // }

                // if (Bag* bag = item->ToBag())
                // {
                    // for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                    // {
                        // Item* item2 = bag->GetItemByPos(j);
                        // if (!item2)
                            // continue;

                        // if (item2->GetSlot() != j)
                        // {
                            // handler->PSendSysMessage("The item in bag %d and slot %d (guid: %d) has an incorrect slot value: %d", bag->GetSlot(), j, item2->GetGUID().GetGUIDLow(), item2->GetSlot());
                            // error = true;
                            // continue;
                        // }

                        // if (item2->GetOwnerGUID() != player->GetGUID())
                        // {
                            // handler->PSendSysMessage("The item in bag %d at slot %d and with itemguid %d, the owner's guid (%d) and the player's guid (%d) don't match!", bag->GetSlot(), item2->GetSlot(), item2->GetGUID().GetGUIDLow(), item2->GetOwnerGUID().GetGUIDLow(), player->GetGUID().GetGUIDLow());
                            // error = true;
                            // continue;
                        // }

                        // Bag* container = item2->GetContainer();
                        // if (!container)
                        // {
                            // handler->PSendSysMessage("The item in bag %d at slot %d with guid %d has no container!", bag->GetSlot(), item2->GetSlot(), item2->GetGUID().GetGUIDLow());
                            // error = true;
                            // continue;
                        // }

                        // if (container != bag)
                        // {
                            // handler->PSendSysMessage("The item in bag %d at slot %d with guid %d has a different container(slot %d guid %d)!", bag->GetSlot(), item2->GetSlot(), item2->GetGUID().GetGUIDLow(), container->GetSlot(), container->GetGUID().GetGUIDLow());
                            // error = true;
                            // continue;
                        // }

                        // if (item2->IsInUpdateQueue())
                        // {
                            // uint16 qp = item2->GetQueuePos();
                            // if (qp > updateQueue.size())
                            // {
                                // handler->PSendSysMessage("The item in bag %d at slot %d having guid %d has a queuepos (%d) larger than the update queue size! ", bag->GetSlot(), item2->GetSlot(), item2->GetGUID().GetGUIDLow(), qp);
                                // error = true;
                                // continue;
                            // }

                            // if (updateQueue[qp] == NULL)
                            // {
                                // handler->PSendSysMessage("The item in bag %d at slot %d having guid %d has a queuepos (%d) that points to NULL in the queue!", bag->GetSlot(), item2->GetSlot(), item2->GetGUID().GetGUIDLow(), qp);
                                // error = true;
                                // continue;
                            // }

                            // if (updateQueue[qp] != item2)
                            // {
                                // handler->PSendSysMessage("The item in bag %d at slot %d having guid %d has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", bag->GetSlot(), item2->GetSlot(), item2->GetGUID().GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUID().GetGUIDLow());
                                // error = true;
                                // continue;
                            // }
                        // }
                        // else if (item2->GetState() != ITEM_UNCHANGED)
                        // {
                            // handler->PSendSysMessage("The item in bag %d at slot %d having guid %d is not in queue but should be (state: %d)!", bag->GetSlot(), item2->GetSlot(), item2->GetGUID().GetGUIDLow(), item2->GetState());
                            // error = true;
                            // continue;
                        // }
                    // }
                // }
            // }

            // for (size_t i = 0; i < updateQueue.size(); ++i)
            // {
                // Item* item = updateQueue[i];
                // if (!item)
                    // continue;

                // if (item->GetOwnerGUID() != player->GetGUID())
                // {
                    // handler->PSendSysMessage("queue(%u): For the item with guid %d, the owner's guid (%d) and the player's guid (%d) don't match!", i, item->GetGUID().GetGUIDLow(), item->GetOwnerGUID().GetGUIDLow(), player->GetGUID().GetGUIDLow());
                    // error = true;
                    // continue;
                // }

                // if (item->GetQueuePos() != i)
                // {
                    // handler->PSendSysMessage("queue(%u): For the item with guid %d, the queuepos doesn't match it's position in the queue!", i, item->GetGUID().GetGUIDLow());
                    // error = true;
                    // continue;
                // }

                // if (item->GetState() == ITEM_REMOVED)
                    // continue;

                // Item* test = player->GetItemByPos(item->GetBagSlot(), item->GetSlot());

                // if (test == NULL)
                // {
                    // handler->PSendSysMessage("queue(%u): The bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have any item at that position!", i, item->GetBagSlot(), item->GetSlot(), item->GetGUID().GetGUIDLow());
                    // error = true;
                    // continue;
                // }

                // if (test != item)
                // {
                    // handler->PSendSysMessage("queue(%u): The bag(%d) and slot(%d) values for the item with guid %d are incorrect, an item which guid is %d is there instead!", i, item->GetBagSlot(), item->GetSlot(), item->GetGUID().GetGUIDLow(), test->GetGUID().GetGUIDLow());
                    // error = true;
                    // continue;
                // }
            // }
            // if (!error)
                // handler->SendSysMessage("All OK!");
        // }

        return true;
    }

    static bool HandleDebugBattlegroundCommand(ChatHandler* /*handler*/, char const* /*args*/)
    {
        sBattlegroundMgr->ToggleTesting();
        return true;
    }

    static bool HandleDebugAshranCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (OutdoorPvP* pvp = player->GetOutdoorPvP())
            pvp->HandleBFMGREntryInviteResponse(true, player);

        return true;
    }

    static bool HandleDebugChallengeCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* _player = handler->GetSession()->GetPlayer();
        if (!_player)
            return false;

        if (Item* item = _player->GetItemByEntry(138019))
        {
            if (Group* group = _player->GetGroup())
            {
                WorldPackets::Instance::ChangePlayerDifficultyResult result;
                result.CooldownReason = 2813862382;
                result.Result = 5;
                group->BroadcastPacket(result.Write(), true);

                group->m_challengeOwner = _player->GetGUID();
                group->m_challengeItem = item->GetGUID();

                WorldPackets::Instance::ChangePlayerDifficultyResult result2;
                result2.Result = 11;
                result2.InstanceMapID = _player->GetMapId();
                result2.DifficultyRecID = DIFFICULTY_MYTHIC_KEYSTONE;

                for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    if (Player* player = itr->getSource())
                    {
                        player->SetDungeonDifficultyID(DIFFICULTY_MYTHIC_KEYSTONE);
                        player->SendDungeonDifficulty();
                        player->SendDirectMessage(result2.Write());
                        player->TeleportTo(player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TELE_TO_SEAMLESS);
                    }
                }
            }
        }
        else
            handler->SendSysMessage("Mythic key 138019 not found");

        // if (InstanceScript* instance = player->GetInstanceScript())
            // if (Challenge* challenge = instance->GetChallenge())
                // challenge->CreateChallenge(player);

        return true;
    }

    static bool HandleDebugLFGCommand(ChatHandler* handler, char const* /*args*/)
    {
        sLFGMgr->ToggleTesting();
        sWorld->setBoolConfig(CONFIG_LFG_DEBUG_JOIN, sLFGMgr->onTest());

        std::ostringstream ss;
        ss << "LFG MODE NOW IS: " << (sLFGMgr->onTest() ? "debug" : "normal");
        handler->SendSysMessage(ss.str().c_str());
        return true;
    }

    static bool HandleDebugArenaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (player->InBattleground())
            return false;

        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(MS::Battlegrounds::BattlegroundTypeId::ArenaAll);
        if (!bg)
            return false;

        uint8 jointype = MS::Battlegrounds::GetJoinTypeByBracketSlot(atoi((char*)args));
        uint16 bgTypeId = bg->GetTypeID();
        uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(bgTypeId, jointype);
        BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);

        PVPDifficultyEntry const* bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
        if (!bracketEntry)
            return false;

        bg->SetRated(true);

        sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(1700, jointype, bgQueueTypeId, bgTypeId, bracketEntry->RangeIndex));

        WorldPackets::Battleground::IgnorMapInfo ignore;
        GroupQueueInfo* ginfo = bgQueue.AddGroup(player, nullptr, bgTypeId, bracketEntry, jointype, true, false, ignore, 1500);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->RangeIndex);

        WorldPackets::Battleground::BattlefieldStatusQueued queued;
        sBattlegroundMgr->BuildBattlegroundStatusQueued(&queued, bg, player, player->AddBattlegroundQueueId(bgQueueTypeId), ginfo->JoinTime, avgTime, ginfo->JoinType, true);
        player->SendDirectMessage(queued.Write());

        sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(1500, jointype, bgQueueTypeId, bgTypeId, bracketEntry->RangeIndex));

        return true;
    }

    static bool HandleDebugThreatListCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* target = handler->getSelectedCreature();
        if (!target || target->isTotem() || target->isPet())
            return false;

        std::list<HostileReference*>& threatList = target->getThreatManager().getThreatList();
        std::list<HostileReference*>::iterator itr;
        uint32 count = 0;
        handler->PSendSysMessage("Threat list of %s (guid %u)", target->GetName(), target->GetGUID().GetGUIDLow());
        for (itr = threatList.begin(); itr != threatList.end(); ++itr)
        {
            Unit* unit = (*itr)->getTarget();
            if (!unit)
                continue;
            ++count;
            handler->PSendSysMessage("   %u.   %s   (guid %u)  - threat %f", count, unit->GetName(), unit->GetGUID().GetGUIDLow(), (*itr)->getThreat());
        }
        handler->SendSysMessage("End of threat list.");
        return true;
    }

    static bool HandleDebugHostileRefListCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();
        HostileReference* ref = target->getHostileRefManager().getFirst();
        uint32 count = 0;
        handler->PSendSysMessage("Hostil reference list of %s (guid %u)", target->GetName(), target->GetGUID().GetGUIDLow());
        while (ref)
        {
            if (Unit* unit = ref->getSource()->getOwner())
            {
                ++count;
                handler->PSendSysMessage("   %u.   %s   (guid %u)  - threat %f", count, unit->GetName(), unit->GetGUID().GetGUIDLow(), ref->getThreat());
            }
            ref = ref->next();
        }
        handler->SendSysMessage("End of hostil reference list.");
        return true;
    }

    static bool HandleDebugSetVehicleIdCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target || target->GetTypeId() != TYPEID_UNIT || !target->IsVehicle())
            return false;

        if (!args)
            return false;

        char* i = strtok((char*)args, " ");
        if (!i)
            return false;

        uint32 vehicle_id = (uint32)atoi(i);
        VehicleEntry const * entry = sVehicleStore.LookupEntry(vehicle_id);
        if (!entry)
        {
            handler->PSendSysMessage("No such vehicle id");
            return false;
        }

        if (target->GetVehicleKit())
            target->RemoveVehicleKit();

        if (!target->CreateVehicleKit(entry->ID, target->GetEntry()))
        {
            handler->PSendSysMessage("Can't create vehicle kit id %u.", vehicle_id);
            return false;
        }

        if (target->GetVehicleKit())
            target->GetVehicleKit()->Reset();

        handler->PSendSysMessage("Vehicle id set to %u", vehicle_id);
        return true;
    }

    static bool HandleDebugEnterVehicleCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)// || !target->IsVehicle())
            return false;

        if (!args)
            return false;

        char* i = strtok((char*)args, " ");
        if (!i)
            return false;

        char* j = strtok(NULL, " ");

        int32 entry = (int32)atoi(i);
        int8 seatId = j ? (int8)atoi(j) : -1;

        if (entry == -1)
            target->EnterVehicle(handler->GetSession()->GetPlayer(), seatId);
        else if (!entry)
            handler->GetSession()->GetPlayer()->EnterVehicle(target, seatId);
        else
        {
            Creature* passenger = NULL;
            Trinity::AllCreaturesOfEntryInRange check(handler->GetSession()->GetPlayer(), entry, 20.0f);
            Trinity::CreatureSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(handler->GetSession()->GetPlayer(), passenger, check);
            Trinity::VisitNearbyObject(handler->GetSession()->GetPlayer(), 30.0f, searcher);
            if (!passenger || passenger == target)
                return false;
            passenger->EnterVehicle(target, seatId);
        }

        handler->PSendSysMessage("Unit %u entered vehicle %d", entry, (int32)seatId);
        return true;
    }

    static bool HandleDebugSpawnVehicleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* e = strtok((char*)args, " ");
        char* i = strtok(NULL, " ");

        if (!e)
            return false;

        uint32 entry = (uint32)atoi(e);

        float x, y, z, o = handler->GetSession()->GetPlayer()->GetOrientation();
        handler->GetSession()->GetPlayer()->GetClosePoint(x, y, z, handler->GetSession()->GetPlayer()->GetObjectSize());

        if (!i)
            return handler->GetSession()->GetPlayer()->SummonCreature(entry, x, y, z, o) != nullptr;

        uint32 id = (uint32)atoi(i);

        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(entry);

        if (!ci)
            return false;

        VehicleEntry const* ve = sVehicleStore.LookupEntry(id);

        if (!ve)
            return false;

        Creature* v = new Creature;

        Map* map = handler->GetSession()->GetPlayer()->GetMap();

        if (!v->Create(sObjectMgr->GetGenerator<HighGuid::Vehicle>()->Generate(), map, handler->GetSession()->GetPlayer()->GetPhaseMask(), entry, id, handler->GetSession()->GetPlayer()->GetTeam(), x, y, z, o))
        {
            delete v;
            return false;
        }

        map->AddToMap(v->ToCreature());

        return true;
    }

    static bool HandleDebugSendLargePacketCommand(ChatHandler* handler, char const* /*args*/)
    {
        const char* stuffingString = "This is a dummy string to push the packet's size beyond 128000 bytes. ";
        std::ostringstream ss;
        while (ss.str().size() < 128000)
            ss << stuffingString;
        handler->SendSysMessage(ss.str().c_str());
        return true;
    }

    static bool HandleDebugSendSetPhaseShiftCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* t = strtok((char*)args, " ");
        char* p = strtok(nullptr, " ");
        char* w = strtok(nullptr, " ");
        char* s = strtok(nullptr, " ");
        if (!t)
            return false;

        std::vector<WorldPackets::Misc::PhaseShiftDataPhase> phaseIds;
        std::vector<uint16> visibleMapIDs;
        std::vector<uint16> uiWorldMapAreaIDSwaps;
        std::vector<uint16> preloadMapIDs;

        visibleMapIDs.emplace_back(static_cast<uint32>(atoi(t)));

        if (p)
            phaseIds.emplace_back(static_cast<uint32>(atoi(p)));

        if (w)
            uiWorldMapAreaIDSwaps.emplace_back(static_cast<uint32>(atoi(w)));

        if (s)
            preloadMapIDs.emplace_back(static_cast<uint32>(atoi(s)));

        handler->GetSession()->SendSetPhaseShift(phaseIds, visibleMapIDs, uiWorldMapAreaIDSwaps, preloadMapIDs);
        return true;
    }

    static bool HandleDebugGetItemValueCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* e = strtok((char*)args, " ");
        char* f = strtok(NULL, " ");

        if (!e || !f)
            return false;

        uint32 guid = strtoull(e, nullptr, 10);
        uint32 index = (uint32)atoi(f);

        Item* i = handler->GetSession()->GetPlayer()->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(guid));

        if (!i)
            return false;

        if (index >= i->GetValuesCount())
            return false;

        uint32 value = i->GetUInt32Value(index);

        handler->PSendSysMessage("Item " UI64FMTD ": value at %u is %u", guid, index, value);

        return true;
    }

    static bool HandleDebugSetItemValueCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* e = strtok((char*)args, " ");
        char* f = strtok(NULL, " ");
        char* g = strtok(NULL, " ");

        if (!e || !f || !g)
            return false;

        uint32 guid = strtoull(e, nullptr, 10);
        uint32 index = (uint32)atoi(f);
        uint32 value = (uint32)atoi(g);

        Item* i = handler->GetSession()->GetPlayer()->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(guid));

        if (!i)
            return false;

        if (index >= i->GetValuesCount())
            return false;

        i->SetUInt32Value(index, value);

        return true;
    }

    static bool HandleDebugItemExpireCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* e = strtok((char*)args, " ");
        if (!e)
            return false;

        uint32 guid = strtoull(e, nullptr, 10);

        Item* i = handler->GetSession()->GetPlayer()->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(guid));

        if (!i)
            return false;

        handler->GetSession()->GetPlayer()->DestroyItem(i->GetBagSlot(), i->GetSlot(), true);
        sScriptMgr->OnItemExpire(handler->GetSession()->GetPlayer(), i->GetTemplate());

        return true;
    }

    //show animation
    static bool HandleDebugAnimCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 animId = atoi((char*)args);
        handler->GetSession()->GetPlayer()->HandleEmoteCommand(animId);
        return true;
    }

    static bool HandleDebugLoSCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (Unit* unit = handler->getSelectedUnit())
            handler->PSendSysMessage("Unit %s (GuidLow: %u) is %sin LoS", unit->GetName(), unit->GetGUID().GetGUIDLow(), handler->GetSession()->GetPlayer()->IsWithinLOSInMap(unit) ? "" : "not ");
        return true;
    }

    static bool HandleDebugSetAuraStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Unit* unit = handler->getSelectedUnit();
        if (!unit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 state = atoi((char*)args);
        if (!state)
        {
            // reset all states
            for (int i = 1; i <= 32; ++i)
                unit->ModifyAuraState(AuraStateType(i), false);
            return true;
        }

        unit->ModifyAuraState(AuraStateType(abs(state)), state > 0);
        return true;
    }

    static bool HandleDebugBattlePetErrorCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* x = strtok((char*)args, " ");
        if (!x)
            return false;

        //if (Player* target = handler->getSelectedObject()->ToPlayer())
        //    target->GetSession()->SendBattlePetError(atoul(x), atoul(strtok(nullptr, " ")));

        return true;
    }

    static bool HandleDebugBattlePetRequestFailedCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* x = strtok((char*)args, " ");
        if (!x)
            return false;

        if (Player* target = handler->getSelectedObject()->ToPlayer())
            target->GetSession()->SendPetBattleRequestFailed(atoul(x));

        return true;
    }

    static bool HandleDebugSetValueCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* x = strtok((char*)args, " ");
        char* y = strtok(NULL, " ");
        char* z = strtok(NULL, " ");

        if (!x || !y)
            return false;

        WorldObject* target = handler->getSelectedObject();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = target->GetGUID();

        uint32 field = (uint32)atoi(x);
        if (field >= target->GetValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, field, guid.GetGUIDLow(), target->GetValuesCount());
            return false;
        }

        bool isInt32 = true;
        if (z)
            isInt32 = atoi(z) != 0;

        if (isInt32)
        {
            uint32 value = (uint32)atoi(y);
            target->SetUInt32Value(field, value);
            handler->PSendSysMessage(LANG_SET_UINT_FIELD, guid.GetGUIDLow(), field, value);
        }
        else
        {
            float value = (float)atof(y);
            target->SetFloatValue(field, value);
            handler->PSendSysMessage(LANG_SET_FLOAT_FIELD, guid.GetGUIDLow(), field, value);
        }

        return true;
    }

    static bool HandleDebugSetDynamicValueCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* x = strtok((char*)args, " ");
        char* y = strtok(NULL, " ");
        char* z = strtok(NULL, " ");

        if (!x || !y || !z)
            return false;

        WorldObject* target = handler->getSelectedObject();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = target->GetGUID();

        uint32 opcode = (uint32)atoi(x);
        if (opcode >= target->GetDynamicValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, opcode, guid.GetGUIDLow(), target->GetDynamicValuesCount());
            return false;
        }

        uint32 offs = (uint32)atoi(y);
        if (offs >= 32)
        {
            handler->PSendSysMessage("Dynamic field index must be less than %u.", 32);
            return  false;
        }

        uint32 value = (uint32)atoi(z);
        target->SetDynamicValue(opcode, offs, value);
        handler->PSendSysMessage("Dynamic value of field %u at offset %u set to %u", opcode, offs, value);

        return true;
    }

    static bool HandleDebugGetValueCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* x = strtok((char*)args, " ");
        char* z = strtok(NULL, " ");

        if (!x)
            return false;

        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = target->GetGUID();

        uint32 opcode = (uint32)atoi(x);
        if (opcode >= target->GetValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, opcode, guid.GetGUIDLow(), target->GetValuesCount());
            return false;
        }

        bool isInt32 = true;
        if (z)
            isInt32 = atoi(z) != 0;

        if (isInt32)
        {
            uint32 value = target->GetUInt32Value(opcode);
            handler->PSendSysMessage(LANG_GET_UINT_FIELD, guid.GetGUIDLow(), opcode, value);
        }
        else
        {
            float value = target->GetFloatValue(opcode);
            handler->PSendSysMessage(LANG_GET_FLOAT_FIELD, guid.GetGUIDLow(), opcode, value);
        }

        return true;
    }

    static bool HandleDebugGetDynamicValueCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* x = strtok((char*)args, " ");
        char* y = strtok(NULL, " ");

        if (!x || !y)
            return false;

        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = target->GetGUID();

        uint32 opcode = (uint32)atoi(x);
        if (opcode >= target->GetDynamicValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, opcode, guid.GetGUIDLow(), target->GetDynamicValuesCount());
            return false;
        }

        uint32 offs = (uint32)atoi(y);
        if (offs >= 32)
        {
            handler->PSendSysMessage("Dynamic field index must be less than %u.", 32);
            return  false;
        }

        uint32 value = target->GetDynamicValue(opcode, offs);
        handler->PSendSysMessage("Unit %u has dynamic value %u at field %u offset %u", guid.GetGUIDLow(), value, opcode, offs);

        return true;
    }

    static bool HandleDebugGetMapInfoCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        Map* map = player->GetMap();
        if (!map)
            return false;

        MapEntry const* mapEntry = map->GetEntry();
        handler->PSendSysMessage("MapId: %u MapName: %s Difficulty: %u Instance Id: %u",
            mapEntry->ID, mapEntry->MapName->Get(0), map->GetDifficultyID(), map->GetInstanceId());

        return true;
    }

    static bool HandleDebugMod32ValueCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* x = strtok((char*)args, " ");
        char* y = strtok(NULL, " ");

        if (!x || !y)
            return false;

        uint32 opcode = (uint32)atoi(x);
        int value = atoi(y);

        if (opcode >= handler->GetSession()->GetPlayer()->GetValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, opcode, handler->GetSession()->GetPlayer()->GetGUID().GetGUIDLow(), handler->GetSession()->GetPlayer()->GetValuesCount());
            return false;
        }

        int currentValue = (int)handler->GetSession()->GetPlayer()->GetUInt32Value(opcode);

        currentValue += value;
        handler->GetSession()->GetPlayer()->SetUInt32Value(opcode, (uint32)currentValue);

        handler->PSendSysMessage(LANG_CHANGE_32BIT_FIELD, opcode, currentValue);

        return true;
    }

    static bool HandleDebugUpdateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 updateIndex;
        uint32 value;

        char* index = strtok((char*)args, " ");

        Unit* unit = handler->getSelectedUnit();
        if (!unit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!index)
            return true;

        updateIndex = atoi(index);
        //check updateIndex
        if (unit->GetTypeId() == TYPEID_PLAYER)
        {
            if (updateIndex >= PLAYER_FIELD_END)
                return true;
        }
        else if (updateIndex >= UNIT_END)
            return true;

        char* val = strtok(NULL, " ");
        if (!val)
        {
            value = unit->GetUInt32Value(updateIndex);

            handler->PSendSysMessage(LANG_UPDATE, unit->GetGUID().GetGUIDLow(), updateIndex, value);
            return true;
        }

        value = atoi(val);

        handler->PSendSysMessage(LANG_UPDATE_CHANGE, unit->GetGUID().GetGUIDLow(), updateIndex, value);

        unit->SetUInt32Value(updateIndex, value);

        return true;
    }

    static bool HandleDebugSet32BitCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        WorldObject* target = handler->getSelectedObject();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* x = strtok((char*)args, " ");
        char* y = strtok(NULL, " ");

        if (!x || !y)
            return false;

        uint32 opcode = (uint32)atoi(x);
        uint32 val = (uint32)atoi(y);
        if (val > 32)                                         //uint32 = 32 bits
            return false;

        uint32 value = val ? 1 << (val - 1) : 0;
        target->SetUInt32Value(opcode,  value);

        handler->PSendSysMessage(LANG_SET_32BIT_FIELD, opcode, value);
        return true;
    }

    static bool HandleDebugMovementInfo(ChatHandler* handler, char const* /*args*/)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        //target->m_movementInfo.OutDebug();

        return true;
    }

    static bool HandleDebugMoveflagsCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        if (!*args)
        {
            //! Display case
            handler->PSendSysMessage(LANG_MOVEFLAGS_GET, target->GetUnitMovementFlags(), target->GetExtraUnitMovementFlags());
        }
        else
        {
            char* mask1 = strtok((char*)args, " ");
            if (!mask1)
                return false;

            char* mask2 = strtok(NULL, " \n");

            uint32 moveFlags = (uint32)atoi(mask1);
            target->SetUnitMovementFlags(moveFlags);

            if (mask2)
            {
                uint32 moveFlagsExtra = uint32(atoi(mask2));
                target->SetExtraUnitMovementFlags(moveFlagsExtra);
            }

            target->SendMovementFlagUpdate();
            handler->PSendSysMessage(LANG_MOVEFLAGS_SET, target->GetUnitMovementFlags(), target->GetExtraUnitMovementFlags());
        }

        return true;
    }

    static bool HandleWPGPSCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        TC_LOG_INFO(LOG_FILTER_SQL_DEV, "(@PATH, XX, %.3f, %.3f, %.5f, 0, 0, 0, 100, 0),", player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

        handler->PSendSysMessage("Waypoint SQL written to SQL Developer log");
        return true;
    }

    static bool HandleDebugPhaseCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        player->GetPhaseMgr().SendDebugReportToPlayer(handler->GetSession()->GetPlayer());
        return true;
    }

    static bool HandleDebugMoveJump(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        WorldObject* target = handler->getSelectedObject();
        if (!target || !target->ToUnit())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* cx        = strtok((char*)args, " ");
        char* cy        = strtok(NULL, " ");
        char* cz        = strtok(NULL, " ");
        char* cspeedXY  = strtok(NULL, " ");
        char* cspeedZ   = strtok(NULL, " ");

        if (!cx || !cy || !cz || !cspeedXY || !cspeedZ)
            return false;

        float x         = (float)atof(cx);
        float y         = (float)atof(cy);
        float z         = (float)atof(cz);
        float speedXY   = (float)atof(cspeedXY);
        float speedZ    = (float)atof(cspeedZ);

        target->ToUnit()->GetMotionMaster()->MoveJump(x, y,z, speedXY, speedZ);
        return true;
    }

    static bool HandleDebugMoveBackward(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        WorldObject* target = handler->getSelectedObject();
        if (!target || !target->ToUnit())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* cx        = strtok((char*)args, " ");
        char* cy        = strtok(NULL, " ");
        char* cz        = strtok(NULL, " ");

        if (!cx || !cy || !cz)
            return false;

        float x         = (float)atof(cx);
        float y         = (float)atof(cy);
        float z         = (float)atof(cz);

        //target->ToUnit()->GetMotionMaster()->MoveBackward(0, x, y,z);
        return true;
    }

    static bool HandleDebugLoadZ(ChatHandler* handler, char const* args)
    {
        for (GameObjectDataContainer::iterator itr = sObjectMgr->_gameObjectDataStore.begin(); itr != sObjectMgr->_gameObjectDataStore.end(); ++itr)
        {
            GameObjectData data = itr->second;

            if (!data.posZ)
            {
                Map* map = sMapMgr->FindMap(data.mapid, 0);

                if (!map)
                    map = sMapMgr->CreateMap(data.mapid, handler->GetSession()->GetPlayer());

                if (map)
                {
                    float newPosZ = map->GetHeight(data.posX, data.posY, MAX_HEIGHT, true);

                    if (newPosZ && newPosZ != -200000.0f)
                        WorldDatabase.PExecute("UPDATE gameobject SET position_z = %f WHERE guid = %u", newPosZ, itr->first);
                }
            }
        }

        return true;
    }

    static bool HandleDebugModifySpellpowerCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        char* cval = strtok((char*)args, " ");

        if (!cval)
            return false;

        int32 Value = (int32)atoi(cval);
        player->ApplySpellPowerBonus(Value, true);
        return true;
    }

    static bool HandleDebugModifyAttackpowerCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        char* cval = strtok((char*)args, " ");

        if (!cval)
            return false;

        int32 Value = (int32)atoi(cval);
        player->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(Value), true);
        player->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(Value), true);
        return true;
    }

    static bool HandleDebugModifyCritChanceCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        char* cval = strtok((char*)args, " ");

        if (!cval)
            return false;

        int32 Value = (int32)atoi(cval);
        player->SetStatFloatValue(PLAYER_FIELD_CRIT_PERCENTAGE, Value);
        player->SetStatFloatValue(PLAYER_FIELD_OFFHAND_CRIT_PERCENTAGE, Value);
        player->SetStatFloatValue(PLAYER_FIELD_RANGED_CRIT_PERCENTAGE, Value);
        player->SetFloatValue(PLAYER_FIELD_SPELL_CRIT_PERCENTAGE, Value);

        return true;
    }
    static bool HandleDebugModifyHasteCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        char* cval = strtok((char*)args, " ");

        if (!cval)
            return false;

        int32 Value = (int32)atoi(cval);
        player->ApplyRatingMod(CR_HASTE_MELEE, Value, true);
        player->ApplyRatingMod(CR_HASTE_RANGED, Value, true);
        player->ApplyRatingMod(CR_HASTE_SPELL, Value, true);
        return true;
    }
    static bool HandleDebugModifyHitCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        char* cval = strtok((char*)args, " ");

        if (!cval)
            return false;

        int32 Value = (int32)atoi(cval);
        player->ApplyRatingMod(CR_HIT_MELEE, Value, true);
        player->ApplyRatingMod(CR_HIT_RANGED, Value, true);
        player->ApplyRatingMod(CR_HIT_SPELL, Value, true);
        return true;
    }
    static bool HandleDebugModifyVersalityCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        char* cval = strtok((char*) args, " ");

        if (!cval)
            return false;

        int32 Value = (int32) atoi(cval);
        player->SetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_VERSATILITY_DAMAGE_DONE, player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_VERSATILITY_DAMAGE_DONE) + Value);
        player->SetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_VERSATILITY_HEALING_DONE, player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_VERSATILITY_HEALING_DONE) + Value);
        player->SetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_VERSATILITY_DAMAGE_TAKEN, player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_VERSATILITY_DAMAGE_TAKEN) + Value);
        player->UpdateVersality();
        return true;
    }
    static bool HandleDebugModifyMasteryCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        char* cval = strtok((char*)args, " ");

        if (!cval)
            return false;

        int32 Value = (int32)atoi(cval);
        player->ApplyRatingMod(CR_MASTERY, Value, true);
        return true;
    }

    static bool HandleDebugUpdateCriteriaCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        Tokenizer tokens(args, ' ');
        if (tokens.empty())
            return false;

        uint32 criteriaType = 0;
        uint32 miscValue1 = 0;
        uint32 miscValue2 = 0;
        uint32 miscValue3 = 0;
        Unit* unit = NULL;
        for (uint32 i = 0; i < tokens.size(); ++i)
        {
            switch (i)
            {
                case 0:
                    criteriaType = std::atoi(tokens[i]);
                    break;
                case 1:
                    miscValue1 = std::atoi(tokens[i]);
                    break;
                case 2:
                    miscValue2 = std::atoi(tokens[i]);
                    break;
                case 3:
                    miscValue3 = std::atoi(tokens[i]);
                    break;
                case 4:
                    unit = player;
                    break;
            }
        }

        player->UpdateAchievementCriteria(CriteriaTypes(criteriaType), miscValue1, miscValue2, miscValue3, unit, true);
        return true;
    }

    static bool HandleDebugSession(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        char* caccId = strtok((char*)args, " ");
        int32 accId = caccId ? (int32)atoi(caccId) : 0;

        if (!accId && player)
            accId = player->GetSession()->GetAccountId();

        if (!accId)
            return false;

        if (WorldSessionPtr sess = sWorld->FindSession(accId))
            handler->PSendSysMessage("Session find for accountID %u in World", accId);

        sMapMgr->FindSessionInAllMaps(accId, handler);
        return true;
    }

    static bool HandleDebugPvpMysticCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
            player = handler->GetSession()->GetPlayer();

        handler->PSendSysMessage("Pvp mystic Count = %u", sWorld->GetPvPMysticCount());
        return true;
    }

    static bool HandleDebugPvPStatEnable(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
            player = handler->GetSession()->GetPlayer();

        player->AddAura(SPELL_PRINCIPLES_OF_WAR, player);
        player->EnablePvpRules(false);
        return true;
    }

    static bool HandleDebugPvPStatDisable(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
            player = handler->GetSession()->GetPlayer();

        player->RemoveAurasDueToSpell(SPELL_PRINCIPLES_OF_WAR);
        return true;
    }

    static bool HandleDebugChallengeLootCommand(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Generate Oplote Loot run");

        sChallengeMgr->GenerateOploteLoot(true);
        return true;
    }

    static bool HandleDebugGroupCommand(ChatHandler* handler, char const* args)
    {
        uint8 count = 1;

        if (atoi((char*)args))
            count = atoi((char*)args);

        handler->PSendSysMessage("HandleDebugGroupCommand count %u", count);

        if (Player* player = handler->GetSession()->GetPlayer())
        {
            Group* group = player->GetGroup();
            if (!group)
            {
                group = new Group;
                group->Create(player);
                sGroupMgr->AddGroup(group);
            }
            count -= group->GetMembersCount();

            player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);
            uint8 subgroup = group->GetMemberGroup(player->GetGUID());
            player->SetGroup(group, subgroup);
            player->SetPartyType(group->GetGroupCategory(), GROUP_TYPE_NORMAL);
            player->ResetGroupUpdateSequenceIfNeeded(group);

            uint8 memberCounter = 1;
            if (QueryResult result = CharacterDatabase.PQuery("SELECT guid, name, class FROM characters WHERE `name` != '' AND online = 0 LIMIT %u", count))
            {
                do
                {
                    Field* fields = result->Fetch();
                    uint64 guid = fields[0].GetUInt64();
                    std::string Name = fields[1].GetString();
                    uint8 Class = fields[2].GetUInt8();

                    handler->PSendSysMessage("HandleDebugGroupCommand load guid %u", fields[0].GetUInt32());

                    if (fields[0].GetUInt32() == player->GetGUIDLow())
                        continue;

                    if (group->AddMysteryMember(guid, Name, Class, lfg::LfgRoles::PLAYER_ROLE_DAMAGE, 0, true))
                        memberCounter++;

                    if (memberCounter == MAX_GROUP_SIZE)
                    {
                        if (!group->isRaidGroup())
                            group->ConvertToRaid();
                    }
                } while (result->NextRow());
            }

            group->SendUpdate();
        }
        return true;
    }

    static bool HandleDebugCrash(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Kicking out server!");
        *(int*) 0 = 0;
        return true;
    }

    static bool HandleDebugAbort(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Abort out server!");
        abort();
        return true;
    }

    static bool HandleDebugException(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Exception out server!");
        throw std::runtime_error("Test exception out server");
        return true;
    }

    static bool HandleDebugFreeze(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Start freeze server!");
        uint64 counter = 0;
        while (true) { counter++; }
        return true;
    }

    static bool HandleDebugPlayerCombat(ChatHandler* handler, char const* args)
    {
        if (Player* player = handler->GetSession()->GetPlayer())
        {
            HostileRefManager& refManager = player->getHostileRefManager();
            HostileReference* ref = refManager.getFirst();

            if (!ref)
            {
                handler->PSendSysMessage("Attackers is Empty!");
                return true;
            }

            while (ref)
            {
                if (Unit* unit = ref->getSource()->getOwner())
                    if (Creature* cre = unit->ToCreature())
                        handler->PSendSysMessage("Entry: %u, GUID: " UI64FMTD ", X: %f, Y: %f, Z: %f, MapID: %u", cre->GetEntry(), cre->ToCreature()->GetDBTableGUIDLow(), cre->GetPositionX(), cre->GetPositionY(), cre->GetPositionZ(), cre->GetMapId());

                ref = ref->next();
            }
        }
        return true;
    }

    static bool HandleHenerateMissionsCommand(ChatHandler* handler, char const* args)
    {
        auto player = handler->getSelectedPlayer();
        if (!player)
            player = handler->GetSession()->GetPlayer();

        if (auto garrison = player->GetGarrison())
            garrison->GenerateRandomMission();

        handler->PSendSysMessage("Garrison: called GenerateRandomMission");
        return true;
    }

    static bool HandleDebugStartTransport(ChatHandler* handler, char const* args)
    {
        char* centry = strtok((char*)args, " ");
        uint32 entry = centry ? (int32)atoi(centry) : 0;

        if (auto player = handler->GetSession()->GetPlayer())
            if (Transport* transport = sTransportMgr->GetTransport(player->GetMap(), entry))
                transport->EnableMovement(true);

        handler->PSendSysMessage("Start shio %u", entry);
        return true;
    }

    static bool HandleDebugSetEffectiveLevel(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* clevel = strtok((char*)args, " ");
        char* cilevelMax = strtok(NULL, " ");
        char* cilevelMin = strtok(NULL, " ");
        uint32 level = clevel ? (int32)atoi(clevel) : 0;
        uint32 ilevelMax = cilevelMax ? (int32)atoi(cilevelMax) : 0;
        uint32 ilevelMin = cilevelMin ? (int32)atoi(cilevelMin) : 0;

        if (auto player = handler->GetSession()->GetPlayer())
            player->RescaleAllForTimeWalk(level, ilevelMax, ilevelMin);

        handler->PSendSysMessage("SetEffectiveLevel level %u ilevelMax %u ilevelMin %u", level, ilevelMax, ilevelMin);
        return true;
    }

    static bool HandleDebugPvELogsCommand(ChatHandler* handler, char const* /*args*/)
    {
        //auto player = handler->GetSession()->GetPlayer();

        //LogsSystem::MainData data;

        //data.RealmID = realm.Id.Realm;
        //data.MapID = player->GetMapId();

        //data.Guild = boost::in_place();
        //data.Guild->GuildID = 9;
        //data.Guild->GuildFaction = player->GetTeamId();
        //data.Guild->GuildName = player->GetGuildName();

        //data.Encounter = boost::in_place();
        //data.Encounter->EncounterID = 1704;
        //data.Encounter->Expansion = CURRENT_EXPANSION;
        //data.Encounter->DifficultyID = 16;
        //data.Encounter->StartTime = time(nullptr);
        //data.Encounter->CombatDuration = 7 * MINUTE;
        //data.Encounter->EndTime = data.Encounter->StartTime + data.Encounter->CombatDuration;
        //data.Encounter->Success = true;
        //data.Encounter->DeadCount = 2;

        //LogsSystem::RosterData rooster;
        //rooster.GuidLow = player->GetGUIDLow();
        //rooster.Name = player->GetName();
        //rooster.Level = player->getLevel();
        //rooster.Class = player->getClass();
        //rooster.SpecID = player->GetSpecializationId();
        //rooster.ItemLevel = player->GetAverageItemLevelEquipped();
        //data.Rosters.push_back(rooster);

        //TC_LOG_ERROR(LOG_FILTER_GENERAL, " %s ", data.Serealize().c_str());
        return true;
    }

    static bool HandleDebugKillPointsCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* cpoint = strtok((char*)args, " ");
        uint32 point = cpoint ? (int32)atoi(cpoint) : 0;

        if (auto player = handler->GetSession()->GetPlayer())
            player->m_killPoints = point;

        handler->PSendSysMessage("KillPoints point %u", point);
        return true;
    }
};

void AddSC_debug_commandscript()
{
    new debug_commandscript();
}
