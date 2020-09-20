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

#include "AccountMgr.h"
#include "BattlegroundMgr.h"
#include "CharmInfo.h"
#include "Chat.h"
#include "DB2Stores.h"
#include "GlobalFunctional.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "MovementGenerator.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PlayerDefines.h"
#include "QuestData.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "TargetedMovementGenerator.h"
#include "WeatherMgr.h"
#include "WordFilterMgr.h"
#include <fstream>
#include <boost/locale/encoding_utf.hpp>

class misc_commandscript : public CommandScript
{
public:
    misc_commandscript() : CommandScript("misc_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> badWordCommandTable =
        {
            { "add",                SEC_ADMINISTRATOR,      true,  &HandleBadWordAddCommand,            ""},
            { "remove",             SEC_ADMINISTRATOR,      true,  &HandleBadWordRemoveCommand,         ""},
            { "list",               SEC_ADMINISTRATOR,      true,  &HandleBadWordListCommand,           ""}
        };
        static std::vector<ChatCommand> wordFilterCommandTable =
        {
            { "badword",            SEC_ADMINISTRATOR,      true,  NULL,                                "", badWordCommandTable },
            { "mod",                SEC_ADMINISTRATOR,      true,  &HandleWordFilterModCommand,         ""}
        };
        static std::vector<ChatCommand> groupCommandTable =
        {
            { "leader",         SEC_ADMINISTRATOR,          false,  &HandleGroupLeaderCommand,          ""},
            { "disband",        SEC_ADMINISTRATOR,          false,  &HandleGroupDisbandCommand,         ""},
            { "remove",         SEC_ADMINISTRATOR,          false,  &HandleGroupRemoveCommand,          ""}
        };
        static std::vector<ChatCommand> petCommandTable =
        {
            { "create",             SEC_GAMEMASTER,         false, &HandleCreatePetCommand,             ""},
            { "learn",              SEC_GAMEMASTER,         false, &HandlePetLearnCommand,              ""},
            { "unlearn",            SEC_GAMEMASTER,         false, &HandlePetUnlearnCommand,            ""}
        };
        static std::vector<ChatCommand> sendCommandTable =
        {
            { "items",              SEC_ADMINISTRATOR,      true,  &HandleSendItemsCommand,             ""},
            { "mail",               SEC_MODERATOR,          true,  &HandleSendMailCommand,              ""},
            { "message",            SEC_ADMINISTRATOR,      true,  &HandleSendMessageCommand,           ""},
            { "money",              SEC_ADMINISTRATOR,      true,  &HandleSendMoneyCommand,             ""}
        };
        static std::vector<ChatCommand> auraCommandTable =
        {
            { "amount",             SEC_ADMINISTRATOR,      true,  &HandleAuraAmountCommand,            ""},
            { "duration",           SEC_ADMINISTRATOR,      true,  &HandleAuraDurationCommand,          ""},
            { "charges",            SEC_ADMINISTRATOR,      true,  &HandleAuraChargesCommand,           ""},
            { "",                   SEC_ADMINISTRATOR,      true,  &HandleAuraCommand,                  ""}
        };
        static std::vector<ChatCommand> areatriggerCommandTable =
        {
            { "add",                SEC_ADMINISTRATOR,      true,  &HandleAreaTriggerCommand,           ""},
            { "castaction",         SEC_ADMINISTRATOR,      true,  &HandleCastActionCommand,            ""},
            { "",                   SEC_ADMINISTRATOR,      true,  &HandleAreaTriggerCommand,           ""}
        };
        static std::vector<ChatCommand> HandleBGTable =
        {
            { "start",              SEC_ADMINISTRATOR,      true,  &HandleBGStartCommand,               ""},
            { "finishwin",          SEC_ADMINISTRATOR,      true,  &HandleBGFinishCommand,              ""},
            { "setarena",           SEC_ADMINISTRATOR,      true,  &HandleBGSetArenaCommand,            ""},
            { "setskirmish",        SEC_ADMINISTRATOR,      true,  &HandleBGSetSkirmishCommand,         ""},
            { "setrbg",             SEC_ADMINISTRATOR,      true,  &HandleBGSetRBGCommand,              ""},
            { "setbg",              SEC_ADMINISTRATOR,      true,  &HandleBGSetBGCommand,               ""}
        };
        static std::vector<ChatCommand> WorldQuestTable =
        {
            { "add",                SEC_GAMEMASTER,         true,  &HandleWorldQuestAdd,                ""},
            { "clear",              SEC_GAMEMASTER,         true,  &HandleWorldQuestClear,              ""},
            { "invasionpoint",      SEC_GAMEMASTER,         true,  &HandleInvasionPointQuest,            ""},
            { "",                   SEC_GAMEMASTER,         true,  &HandleWorldQuest,                   ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "dev",                SEC_ADMINISTRATOR,      false, &HandleDevCommand,                   ""},
            { "gps",                SEC_ADMINISTRATOR,      false, &HandleGPSCommand,                   ""},
            { "aura",               SEC_ADMINISTRATOR,      false, NULL,                                "", auraCommandTable },
            { "unaura",             SEC_ADMINISTRATOR,      false, &HandleUnAuraCommand,                ""},
            { "appear",             SEC_MODERATOR,          false, &HandleAppearCommand,                ""},
            { "summon",             SEC_MODERATOR,          false, &HandleSummonCommand,                ""},
            { "groupsummon",        SEC_MODERATOR,          false, &HandleGroupSummonCommand,           ""},
            { "commands",           SEC_PLAYER,             true,  &HandleCommandsCommand,              ""},
            { "die",                SEC_ADMINISTRATOR,      false, &HandleDieCommand,                   ""},
            { "revive",             SEC_ADMINISTRATOR,      true,  &HandleReviveCommand,                ""},
            { "dismount",           SEC_PLAYER,             false, &HandleDismountCommand,              ""},
            { "guid",               SEC_GAMEMASTER,         false, &HandleGUIDCommand,                  ""},
            { "help",               SEC_PLAYER,             true,  &HandleHelpCommand,                  ""},
            { "itemmove",           SEC_GAMEMASTER,         false, &HandleItemMoveCommand,              ""},
            { "cooldown",           SEC_ADMINISTRATOR,      false, &HandleCooldownCommand,              ""},
            { "distance",           SEC_ADMINISTRATOR,      false, &HandleGetDistanceCommand,           ""},
            { "recall",             SEC_MODERATOR,          false, &HandleRecallCommand,                ""},
            { "save",               SEC_PLAYER,             false, &HandleSaveCommand,                  ""},
            { "saveall",            SEC_MODERATOR,          true,  &HandleSaveAllCommand,               ""},
            { "kick",               SEC_GAMEMASTER,         true,  &HandleKickPlayerCommand,            ""},
            { "start",              SEC_PLAYER,             false, &HandleStartCommand,                 ""},
            { "linkgrave",          SEC_ADMINISTRATOR,      false, &HandleLinkGraveCommand,             ""},
            { "neargrave",          SEC_ADMINISTRATOR,      false, &HandleNearGraveCommand,             ""},
            { "showarea",           SEC_ADMINISTRATOR,      false, &HandleShowAreaCommand,              ""},
            { "hidearea",           SEC_ADMINISTRATOR,      false, &HandleHideAreaCommand,              ""},
            { "additem",            SEC_ADMINISTRATOR,      false, &HandleAddItemCommand,               ""},
            { "additemmod",         SEC_ADMINISTRATOR,      false, &HandleAddItemModCommand,            ""},
            { "additemset",         SEC_ADMINISTRATOR,      false, &HandleAddItemSetCommand,            ""},
            { "bank",               SEC_ADMINISTRATOR,      false, &HandleBankCommand,                  ""},
            { "wchange",            SEC_ADMINISTRATOR,      false, &HandleChangeWeather,                ""},
            { "setskill",           SEC_ADMINISTRATOR,      false, &HandleSetSkillCommand,              ""},
            { "pinfo",              SEC_GAMEMASTER,         true,  &HandlePInfoCommand,                 ""},
            { "phaseinfo",          SEC_GAMEMASTER,         true,  &HandlePhaseInfoCommand,             ""},
            { "respawn",            SEC_ADMINISTRATOR,      false, &HandleRespawnCommand,               ""},
            { "send",               SEC_MODERATOR,          true,  NULL,                                "", sendCommandTable },
            { "pet",                SEC_GAMEMASTER,         false, NULL,                                "", petCommandTable },
            { "mute",               SEC_MODERATOR,          true,  &HandleMuteCommand,                  ""},
            { "unmute",             SEC_MODERATOR,          true,  &HandleUnmuteCommand,                ""},
            { "movegens",           SEC_ADMINISTRATOR,      false, &HandleMovegensCommand,              ""},
            { "cometome",           SEC_ADMINISTRATOR,      false, &HandleComeToMeCommand,              ""},
            { "damage",             SEC_ADMINISTRATOR,      false, &HandleDamageCommand,                ""},
            { "combatstop",         SEC_GAMEMASTER,         true,  &HandleCombatStopCommand,            ""},
            { "flusharenapoints",   SEC_ADMINISTRATOR,      false, &HandleFlushArenaPointsCommand,      ""},
            { "repairitems",        SEC_GAMEMASTER,         true,  &HandleRepairitemsCommand,           ""},
            { "freeze",             SEC_MODERATOR,          false, &HandleFreezeCommand,                ""},
            { "unfreeze",           SEC_MODERATOR,          false, &HandleUnFreezeCommand,              ""},
            { "listfreeze",         SEC_MODERATOR,          false, &HandleListFreezeCommand,            ""},
            { "group",              SEC_ADMINISTRATOR,      false, NULL,                                "", groupCommandTable },
            { "possess",            SEC_ADMINISTRATOR,      false, &HandlePossessCommand,                ""},
            { "unpossess",          SEC_ADMINISTRATOR,      false, &HandleUnPossessCommand,              ""},
            { "bindsight",          SEC_ADMINISTRATOR,      false, &HandleBindSightCommand,              ""},
            { "unbindsight",        SEC_ADMINISTRATOR,      false, &HandleUnbindSightCommand,            ""},
            { "playall",            SEC_GAMEMASTER,         false, &HandlePlayAllCommand,                ""},
            { "abortReason",        SEC_GAMEMASTER,         false, &HandleTransferAbortReasonCommand,    ""},
            { "selectfaction",      SEC_ADMINISTRATOR,      false, &HandleSelectFactionCommand,          ""},
            { "outItemTemplate",    SEC_ADMINISTRATOR,      false, &HandleOutItemTemplateCommand,        ""},
            { "wordfilter",         SEC_ADMINISTRATOR,      false, NULL,                                "", wordFilterCommandTable },
            { "head",               SEC_ADMINISTRATOR,      true,  &HandleCharDisplayHeadCommand,       ""},
            { "shoulders",          SEC_ADMINISTRATOR,      true,  &HandleCharDisplayShouldersCommand,  ""},
            { "chest",              SEC_ADMINISTRATOR,      true,  &HandleCharDisplayChestCommand,      ""},
            { "waist",              SEC_ADMINISTRATOR,      true,  &HandleCharDisplayWaistCommand,      ""},
            { "legs",               SEC_ADMINISTRATOR,      true,  &HandleCharDisplayLegsCommand,       ""},
            { "feet",               SEC_ADMINISTRATOR,      true,  &HandleCharDisplayFeetCommand,       ""},
            { "wrists",             SEC_ADMINISTRATOR,      true,  &HandleCharDisplayWristsCommand,     ""},
            { "hands",              SEC_ADMINISTRATOR,      true,  &HandleCharDisplayHandsCommand,      ""},
            { "back",               SEC_ADMINISTRATOR,      true,  &HandleCharDisplayBackCommand,       ""},
            { "mainhand",           SEC_ADMINISTRATOR,      true,  &HandleCharDisplayMainhandCommand,   ""},
            { "offhand",            SEC_ADMINISTRATOR,      true,  &HandleCharDisplayOffhandCommand,    ""},
            { "ranged",             SEC_ADMINISTRATOR,      true,  &HandleCharDisplayRangedCommand,     ""},
            { "tabard",             SEC_ADMINISTRATOR,      true,  &HandleCharDisplayTabardCommand,     ""},
            { "shirt",              SEC_ADMINISTRATOR,      true,  &HandleCharDisplayShirtCommand,      ""},
            { "itemspec",           SEC_ADMINISTRATOR,      false, &HandleItemSpecCommand,              ""},
            { "setscenario",        SEC_ADMINISTRATOR,      false, &HandleSetScenarioCommand,           ""},
            { "conversation",       SEC_ADMINISTRATOR,      false, &HandleConversationCommand,          ""},
            { "areatrigger",        SEC_ADMINISTRATOR,      false, NULL,                                "", areatriggerCommandTable },
            { "adddupe",            SEC_ADMINISTRATOR,      false, &HandleAddDupeCommand,               ""},
            { "bg",                 SEC_ADMINISTRATOR,      false, NULL,                                "", HandleBGTable },
            { "pvpReward",          SEC_ADMINISTRATOR,      false, &HandlePvpRewardCommand,             ""},
            { "addmythickey",       SEC_GAMEMASTER,         false, &HandleAddMythicKeyCommand,          ""},
            { "worldquest",         SEC_GAMEMASTER,         false, NULL,                                "", WorldQuestTable },
            { "deltransmogbyitem",  SEC_GAMEMASTER,         false, &HandleDelTransmog,                  ""},
            { "sendscene",          SEC_GAMEMASTER,         false, &HandleSendSpellScene,               ""}
        };
        return commandTable;
    }

    static bool HandleBadWordAddCommand(ChatHandler* handler, char const* args)
    {
        std::string badWord = args;

        const uint8 maxWordSize = 3;
        if (badWord.size() <= maxWordSize)
        {
            handler->PSendSysMessage("The word '%s' is incorrect! The word length must be greater than %u.", badWord.c_str(), maxWordSize);
            return false;
        }

        if (!sWordFilterMgr->AddBadWord(badWord, true))
        {
            handler->PSendSysMessage("The word '%s' is exist!", badWord.c_str());
            return false;
        }

        handler->PSendSysMessage("The word '%s' is added to 'bad_word'.", badWord.c_str());
        return true;
    }

    static bool HandleBadWordRemoveCommand(ChatHandler* handler, char const* args)
    {
        std::string badWord = args;

        if (!sWordFilterMgr->RemoveBadWord(args, true))
        {
            handler->PSendSysMessage("The word '%s' is not exist!", badWord.c_str());
            return false;
        }

        handler->PSendSysMessage("The word '%s' is removed from 'bad_word'.", badWord.c_str());
        return true;
    }

    static bool HandleBadWordListCommand(ChatHandler* handler, char const* args)
    {
        WordFilterMgr::BadWordMap const& badWords = sWordFilterMgr->GetBadWords();

        if (badWords.empty())
        {
            handler->PSendSysMessage("empty.");
            return true;
        }

        std::string strBadWordList;
        uint16 addressesSize = 0;
        const uint16 maxAddressesSize = 255; // !uint8

        for (WordFilterMgr::BadWordMap::const_iterator it = badWords.begin(); it != badWords.end(); ++it)
        {
            strBadWordList.append(boost::locale::conv::utf_to_utf<char>(*it));

            if ((*it) != (*badWords.rbegin()))
                strBadWordList.append(", ");
            else
                strBadWordList.append(".");

            // send
            if (addressesSize >= maxAddressesSize)
            {
                handler->PSendSysMessage("Bad words: %s", strBadWordList.c_str());
                strBadWordList.clear();
                addressesSize = 0;
            }

            addressesSize += (*it).size();
        }

        return true;
    }

    static bool HandleWordFilterModCommand(ChatHandler* handler, char const* args)
    {
        std::string argstr = (char*)args;
        if (argstr == "on")
        {
            sWorld->setBoolConfig(CONFIG_WORD_FILTER_ENABLE, true);
            handler->PSendSysMessage("WordFilter: mod is enabled");
            return true;
        }
        if (argstr == "off")
        {
            sWorld->setBoolConfig(CONFIG_WORD_FILTER_ENABLE, false);
            handler->PSendSysMessage("WordFilter: mod is disabled");
            return true;
        }

        std::string strModIs = sWorld->getBoolConfig(CONFIG_WORD_FILTER_ENABLE) ? "enabled" : "disabled";
        handler->PSendSysMessage("WordFilter: mod is now %s.", strModIs.c_str());

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);

        return true;
    }

    static bool HandleDevCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            if (handler->GetSession()->GetPlayer()->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_DEVELOPER))
                handler->GetSession()->SendNotification(LANG_DEV_ON);
            else
                handler->GetSession()->SendNotification(LANG_DEV_OFF);
            return true;
        }

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_DEVELOPER);
            handler->GetSession()->SendNotification(LANG_DEV_ON);
            return true;
        }

        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_DEVELOPER);
            handler->GetSession()->SendNotification(LANG_DEV_OFF);
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleGPSCommand(ChatHandler* handler, char const* args)
    {
        WorldObject* object = nullptr;
        if (*args)
        {
            ObjectGuid guid = handler->extractGuidFromLink((char*)args);
            if (!guid.IsEmpty())
                object = (WorldObject*)ObjectAccessor::GetObjectByTypeMask(*handler->GetSession()->GetPlayer(), guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT);

            if (!object)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            object = handler->getSelectedUnit();

            if (!object)
            {
                handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        CellCoord cellCoord = Trinity::ComputeCellCoord(object->GetPositionX(), object->GetPositionY());
        Cell cell(cellCoord);

        uint32 zoneId, areaId;
        object->GetZoneAndAreaId(zoneId, areaId);
        uint32 mapId = object->GetMapId();

        MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
        AreaTableEntry const* zoneEntry = sAreaTableStore.LookupEntry(zoneId);
        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaId);

        float zoneX = object->GetPositionX();
        float zoneY = object->GetPositionY();

        sDB2Manager.Map2ZoneCoordinates(zoneX, zoneY, zoneId);

        Map const* map = object->GetMap();
        float groundZ = map->GetHeight(object->GetPhases(), object->GetPositionX(), object->GetPositionY(), MAX_HEIGHT);
        DynamicTreeCallback dCallback;
        float floorZ = map->GetHeight(object->GetPhases(), object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), true, DEFAULT_HEIGHT_SEARCH, &dCallback);
        float vmapZ = map->GetVmapHeight(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ());
        GridCoord gridCoord = Trinity::ComputeGridCoord(object->GetPositionX(), object->GetPositionY());

        int TileX = (MAX_NUMBER_OF_GRIDS - 1) - gridCoord.x_coord;
        int TileY = (MAX_NUMBER_OF_GRIDS - 1) - gridCoord.y_coord;

        uint32 haveMap = Map::ExistMap(mapId, TileX, TileY) ? 1 : 0;
        uint32 haveVMap = Map::ExistVMap(mapId, TileX, TileY) ? 1 : 0;
        const char* AreaName = areaEntry ? areaEntry->AreaName->Str[sObjectMgr->GetDBCLocaleIndex()] : "<unknown>";

        if (haveVMap)
        {
            uint32 mogpFlags;
            int32 adtId, rootId, groupId;
            if(map->GetAreaInfo(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), mogpFlags, adtId, rootId, groupId))
            {
                if(WMOAreaTableEntry const* wmoEntry = sDB2Manager.GetWMOAreaTableEntryByTripple(rootId, adtId, groupId))
                    if(std::strlen(wmoEntry->AreaName->Str[LOCALE_enUS]) > 0)
                        AreaName = wmoEntry->AreaName->Str[LOCALE_enUS];

                handler->PSendSysMessage("WMO rootId %i adtId %i groupId %i mogpFlags %u", rootId, adtId, groupId, mogpFlags);
            }

            if (map->IsOutdoors(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ()))
                handler->PSendSysMessage("You are outdoors");
            else
                handler->PSendSysMessage("You are indoors");
        }
        else
            handler->PSendSysMessage("no VMAP available for area info");

        handler->PSendSysMessage(LANG_MAP_POSITION,
            object->GetMapId(), (mapEntry ? mapEntry->MapName->Str[sObjectMgr->GetDBCLocaleIndex()] : "<unknown>"),
            zoneId, (zoneEntry ? zoneEntry->AreaName->Str[sObjectMgr->GetDBCLocaleIndex()] : "<unknown>"),
            areaId, AreaName,
            object->GetPhaseMask(),
            object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation(),
            cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), object->GetInstanceId(),
            zoneX, zoneY, groundZ, floorZ, haveMap, haveVMap);

        handler->PSendSysMessage("Diffyculty %i spawnmask %i vmapZ %f GetPositionH %f GetPositionZH %f TileX %i TileY %i thread %u",
        map->GetDifficultyID(), map->GetSpawnMode(), vmapZ, object->GetPositionH(), object->GetPositionZH(), TileX, TileY, std::this_thread::get_id());

        if (object->m_movementInfo.transport.Guid)
            handler->PSendSysMessage("Transport position: %s Guid: %s", object->m_movementInfo.transport.Pos.ToString().c_str(), object->m_movementInfo.transport.Guid.ToString().c_str());

        LiquidData liquidStatus;
        ZLiquidStatus status = map->getLiquidStatus(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), MAP_ALL_LIQUIDS, &liquidStatus);

        if (status)
            handler->PSendSysMessage(LANG_LIQUID_STATUS, liquidStatus.level, liquidStatus.depth_level, liquidStatus.entry, liquidStatus.type_flags, status);

        if (!object->GetTerrainSwaps().empty())
        {
            std::stringstream ss;
            for (uint32 swap : object->GetTerrainSwaps())
                ss << swap << " ";
            handler->PSendSysMessage("Target's active terrain swaps: %s", ss.str().c_str());
        }
        if (!object->GetWorldMapAreaSwaps().empty())
        {
            std::stringstream ss;
            for (uint32 swap : object->GetWorldMapAreaSwaps())
                ss << swap << " ";
            handler->PSendSysMessage("Target's active world map area swaps: %s", ss.str().c_str());
        }

        if (!object->m_movementInfo.transport.Guid.IsEmpty())
            handler->PSendSysMessage("Transport pos: (%f, %f, %f)", object->m_movementInfo.transport.Pos.GetPositionX(), object->m_movementInfo.transport.Pos.GetPositionY(), object->m_movementInfo.transport.Pos.GetPositionZ());

        if (dCallback.go)
            handler->PSendSysMessage("GameObject Entry %u DisplayId %u", dCallback.go->GetEntry(), dCallback.go->GetDisplayId());

        return true;
    }

    static bool HandleAuraCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            if (Aura* aura = Aura::TryRefreshStackOrCreate(spellInfo, MAX_EFFECT_MASK, target, target))
                aura->ApplyForTargets();

        return true;
    }

    static bool HandleAuraDurationCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (!spellId)
            return false;
        
        char const* dduration = strtok(NULL, " ");
        int32 duration = 1000;

        if (dduration)
            duration = strtol(dduration, NULL, 10);

        if (!duration)
            target->RemoveAurasDueToSpell(spellId);
        else
        {
            if (Aura* aura = target->GetAura(spellId))
                aura->SetDuration(duration);
            else
                return false;
        }

        return true;
    }

    static bool HandleAuraAmountCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        char const* effectStr = strtok(NULL, " ");
        char const* amountStr = strtok(NULL, " ");

        uint8 effectId = effectStr ? atoi(effectStr) : 0;
        int32 amount = amountStr ? atoi(amountStr) : 0;

        if (AuraEffect* effect = target->GetAuraEffect(spellId, effectId))
            effect->SetAmount(amount);

        return true;
    }

    static bool HandleAuraChargesCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (!spellId)
            return false;
        
        char const* ccharges = strtok(NULL, " ");
        int32 charges = 1;

        if (ccharges)
            charges = strtol(ccharges, NULL, 10);

        if (!charges)
            target->RemoveAurasDueToSpell(spellId);
        else
        {
            if (Aura* aura = target->GetAura(spellId))
                aura->SetCharges(charges);
            else
                return false;
        }

        return true;
    }

    static bool HandleUnAuraCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string argstr = args;
        if (argstr == "all")
        {
            target->RemoveAllAuras();
            return true;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        target->RemoveAurasDueToSpell(spellId);

        return true;
    }
    // Teleport to Player
    static bool HandleAppearCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        Player* _player = handler->GetSession()->GetPlayer();
        if (target == _player || targetGuid == _player->GetGUID())
        {
            handler->SendSysMessage(LANG_CANT_TELEPORT_SELF);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            std::string chrNameLink = handler->playerLink(targetName);

            Map* map = target->GetMap();
            if (map->IsBattlegroundOrArena())
            {
                // only allow if gm mode is on
                if (!_player->isGameMaster())
                {
                    handler->PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, chrNameLink.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                // if both players are in different bgs
                if (_player->GetBattlegroundId() && _player->GetBattlegroundId() != target->GetBattlegroundId())
                    _player->LeaveBattleground(false);
                // Note: should be changed so _player gets no Deserter debuff

                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                _player->SetBattlegroundId(target->GetBattlegroundId(), target->GetBattlegroundTypeId());
                // remember current position as entry point for return at bg end teleportation
                if (!_player->GetMap()->IsBattlegroundOrArena())
                    _player->SetBattlegroundEntryPoint();
            }
            else if (map->IsDungeon())
            {
                // we have to go to instance, and can go to player only if:
                //   1) we are in his group (either as leader or as member)
                //   2) we are not bound to any group and have GM mode on
                if (_player->GetGroup())
                {
                    // we are in group, we can go only if we are in the player group
                    if (_player->GetGroup() != target->GetGroup())
                    {
                        handler->PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY, chrNameLink.c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }
                else
                {
                    // we are not in group, let's verify our GM mode
                    if (!_player->isGameMaster())
                    {
                        handler->PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM, chrNameLink.c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }

                // if the player or the player's group is bound to another instance
                // the player will not be bound to another one
                InstancePlayerBind* bind = _player->GetBoundInstance(target->GetMapId(), target->GetDifficultyID(map->GetEntry()));
                if (!bind)
                {
                    Group* group = _player->GetGroup();
                    // if no bind exists, create a solo bind
                    InstanceGroupBind* gBind = group ? group->GetBoundInstance(target) : NULL;                // if no bind exists, create a solo bind
                    if (!gBind)
                        if (InstanceSave* save = sInstanceSaveMgr->GetInstanceSave(target->GetInstanceId()))
                            _player->BindToInstance(save, !save->CanReset());
                }

                if (map->IsRaid())
                    _player->SetRaidDifficultyID(target->GetRaidDifficultyID());
                else
                    _player->SetDungeonDifficultyID(target->GetDungeonDifficultyID());
            }

            handler->PSendSysMessage(LANG_APPEARING_AT, chrNameLink.c_str());

            // stop flight if need
            if (_player->isInFlight())
            {
                _player->GetMotionMaster()->MovementExpired();
                _player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            // to point to see at target with same orientation
            float x, y, z;
            target->GetContactPoint(_player, x, y, z);

            _player->TeleportTo(target->GetMapId(), x, y, z, _player->GetAngle(target), TELE_TO_GM_MODE);
            _player->SetPhaseMask(target->GetPhaseMask(), true);
        }
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            std::string nameLink = handler->playerLink(targetName);

            handler->PSendSysMessage(LANG_APPEARING_AT, nameLink.c_str());

            // to point where player stay (if loaded)
            float x, y, z, o;
            uint32 map;
            bool in_flight;
            if (!Player::LoadPositionFromDB(map, x, y, z, o, in_flight, targetGuid))
                return false;

            // stop flight if need
            if (_player->isInFlight())
            {
                _player->GetMotionMaster()->MovementExpired();
                _player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            _player->TeleportTo(map, x, y, z, _player->GetOrientation());
        }

        return true;
    }
    // Summon Player
    static bool HandleSummonCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        Player* _player = handler->GetSession()->GetPlayer();
        if (target == _player || targetGuid == _player->GetGUID())
        {
            handler->PSendSysMessage(LANG_CANT_TELEPORT_SELF);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            std::string nameLink = handler->playerLink(targetName);
            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            if (target->IsBeingTeleported())
            {
                handler->PSendSysMessage(LANG_IS_TELEPORTED, nameLink.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            Map* map = handler->GetSession()->GetPlayer()->GetMap();

            if (map->IsBattlegroundOrArena())
            {
                // only allow if gm mode is on
                if (!_player->isGameMaster())
                {
                    handler->PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, nameLink.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                // if both players are in different bgs
                if (target->GetBattlegroundId() && handler->GetSession()->GetPlayer()->GetBattlegroundId() != target->GetBattlegroundId())
                    target->LeaveBattleground(false);
                // Note: should be changed so target gets no Deserter debuff

                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                target->SetBattlegroundId(handler->GetSession()->GetPlayer()->GetBattlegroundId(), handler->GetSession()->GetPlayer()->GetBattlegroundTypeId());
                // remember current position as entry point for return at bg end teleportation
                if (!target->GetMap()->IsBattlegroundOrArena())
                    target->SetBattlegroundEntryPoint();
            }
            else if (map->IsDungeon())
            {
                Map* map = target->GetMap();

                if (map->Instanceable() && map->GetInstanceId() != map->GetInstanceId())
                    target->UnbindInstance(map->GetInstanceId(), target->GetDungeonDifficultyID(), true);

                // we are in instance, and can summon only player in our group with us as lead
                if (!handler->GetSession()->GetPlayer()->GetGroup() || !target->GetGroup() ||
                    (target->GetGroup()->GetLeaderGUID() != handler->GetSession()->GetPlayer()->GetGUID()) ||
                    (handler->GetSession()->GetPlayer()->GetGroup()->GetLeaderGUID() != handler->GetSession()->GetPlayer()->GetGUID()))
                    // the last check is a bit excessive, but let it be, just in case
                {
                    handler->PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, nameLink.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }

            handler->PSendSysMessage(LANG_SUMMONING, nameLink.c_str(), "");
            if (handler->needReportToTarget(target))
                ChatHandler(target).PSendSysMessage(LANG_SUMMONED_BY, handler->playerLink(_player->GetName()).c_str());

            // stop flight if need
            if (target->isInFlight())
            {
                target->GetMotionMaster()->MovementExpired();
                target->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                target->SaveRecallPosition();

            // before GM
            float x, y, z;
            handler->GetSession()->GetPlayer()->GetClosePoint(x, y, z, target->GetObjectSize());
            target->TeleportTo(handler->GetSession()->GetPlayer()->GetMapId(), x, y, z, target->GetOrientation());
            target->SetPhaseMask(handler->GetSession()->GetPlayer()->GetPhaseMask(), true);
        }
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            std::string nameLink = handler->playerLink(targetName);

            handler->PSendSysMessage(LANG_SUMMONING, nameLink.c_str(), handler->GetTrinityString(LANG_OFFLINE));

            // in point where GM stay
            Player::SavePositionInDB(handler->GetSession()->GetPlayer()->GetMapId(),
                handler->GetSession()->GetPlayer()->GetPositionX(),
                handler->GetSession()->GetPlayer()->GetPositionY(),
                handler->GetSession()->GetPlayer()->GetPositionZ(),
                handler->GetSession()->GetPlayer()->GetOrientation(),
                handler->GetSession()->GetPlayer()->GetZoneId(),
                targetGuid);
        }

        return true;
    }
    // Summon group of player
    static bool HandleGroupSummonCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        Group* group = target->GetGroup();

        std::string nameLink = handler->GetNameLink(target);

        if (!group)
        {
            handler->PSendSysMessage(LANG_NOT_IN_GROUP, nameLink.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        Map* gmMap = handler->GetSession()->GetPlayer()->GetMap();
        bool toInstance = gmMap->Instanceable();

        // we are in instance, and can summon only player in our group with us as lead
        if (toInstance && (
            !handler->GetSession()->GetPlayer()->GetGroup() || (group->GetLeaderGUID() != handler->GetSession()->GetPlayer()->GetGUID()) ||
            (handler->GetSession()->GetPlayer()->GetGroup()->GetLeaderGUID() != handler->GetSession()->GetPlayer()->GetGUID())))
            // the last check is a bit excessive, but let it be, just in case
        {
            handler->SendSysMessage(LANG_CANNOT_SUMMON_TO_INST);
            handler->SetSentErrorMessage(true);
            return false;
        }

        for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* player = itr->getSource();

            if (!player || player == handler->GetSession()->GetPlayer() || !player->GetSession())
                continue;

            // check online security
            if (handler->HasLowerSecurity(player, ObjectGuid::Empty))
                return false;

            std::string plNameLink = handler->GetNameLink(player);

            if (player->IsBeingTeleported() == true)
            {
                handler->PSendSysMessage(LANG_IS_TELEPORTED, plNameLink.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (toInstance)
            {
                Map* playerMap = player->GetMap();

                if (playerMap->Instanceable() && playerMap->GetInstanceId() != gmMap->GetInstanceId())
                {
                    // cannot summon from instance to instance
                    handler->PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, plNameLink.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }

            handler->PSendSysMessage(LANG_SUMMONING, plNameLink.c_str(), "");
            if (handler->needReportToTarget(player))
                ChatHandler(player).PSendSysMessage(LANG_SUMMONED_BY, handler->GetNameLink().c_str());

            // stop flight if need
            if (player->isInFlight())
            {
                player->GetMotionMaster()->MovementExpired();
                player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                player->SaveRecallPosition();

            // before GM
            float x, y, z;
            handler->GetSession()->GetPlayer()->GetClosePoint(x, y, z, player->GetObjectSize());
            player->TeleportTo(handler->GetSession()->GetPlayer()->GetMapId(), x, y, z, player->GetOrientation());
        }

        return true;
    }

    static bool HandleCommandsCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->ShowHelpForCommand(handler->getCommandTable(), "");
        return true;
    }

    static bool HandleDieCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* target = handler->getSelectedUnit();

        if (!target || !handler->GetSession()->GetPlayer()->GetSelection())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            if (handler->HasLowerSecurity((Player*)target, ObjectGuid::Empty, false))
                return false;
        }

        if (target->isAlive())
        {
            handler->GetSession()->GetPlayer()->DealDamage(target, target->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }

        return true;
    }

    static bool HandleReviveCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid))
            return false;

        if (target)
        {
            target->ResurrectPlayer(!AccountMgr::IsPlayerAccount(target->GetSession()->GetSecurity()) ? 1.0f : 0.5f);
            target->SpawnCorpseBones();
            target->SaveToDB();
        }
        else
            // will resurrected at login without corpse
            sObjectAccessor->ConvertCorpseForPlayer(targetGuid);

        return true;
    }

    static bool HandleDismountCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        // If player is not mounted, so go out :)
        if (!player->IsMounted())
        {
            handler->SendSysMessage(LANG_CHAR_NON_MOUNTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->isInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->Dismount();
        player->RemoveAurasByType(SPELL_AURA_MOUNTED);
        return true;
    }

    static bool HandleGUIDCommand(ChatHandler* handler, char const* /*args*/)
    {
        ObjectGuid guid = handler->GetSession()->GetPlayer()->GetSelection();

        if (!guid)
        {
            handler->SendSysMessage(LANG_NO_SELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_OBJECT_GUID, guid.GetGUIDLow(), guid.GetGUIDHigh());
        return true;
    }

    static bool HandleHelpCommand(ChatHandler* handler, char const* args)
    {
        char const* cmd = strtok((char*)args, " ");
        if (!cmd)
        {
            handler->ShowHelpForCommand(handler->getCommandTable(), "help");
            handler->ShowHelpForCommand(handler->getCommandTable(), "");
        }
        else
        {
            if (!handler->ShowHelpForCommand(handler->getCommandTable(), cmd))
                handler->SendSysMessage(LANG_NO_HELP_CMD);
        }

        return true;
    }
    // move item to other slot
    static bool HandleItemMoveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* param1 = strtok((char*)args, " ");
        if (!param1)
            return false;

        char const* param2 = strtok(NULL, " ");
        if (!param2)
            return false;

        uint8 srcSlot = uint8(atoi(param1));
        uint8 dstSlot = uint8(atoi(param2));

        if (srcSlot == dstSlot)
            return true;

        if (!handler->GetSession()->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0, srcSlot, true))
            return false;

        if (!handler->GetSession()->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0, dstSlot, false))
            return false;

        uint16 src = ((INVENTORY_SLOT_BAG_0 << 8) | srcSlot);
        uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | dstSlot);

        handler->GetSession()->GetPlayer()->SwapItem(src, dst);

        return true;
    }

    static bool HandleCooldownCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            if (WorldSession* _session = handler->GetSession())
                target = _session->GetPlayer();

            if (!target)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        std::string nameLink = handler->GetNameLink(target);
        std::string argstr = args;

        if (!*args)
        {
            target->RemoveAllSpellCooldown();
            handler->PSendSysMessage(LANG_REMOVEALL_COOLDOWN, nameLink.c_str());
        }
        else if (argstr == "s") // self
        {
            if (WorldSession* _session = handler->GetSession())
                if (Player* plr = _session->GetPlayer())
                {
                    plr->RemoveAllSpellCooldown();
                    handler->PSendSysMessage(LANG_REMOVEALL_COOLDOWN, "self");
                }
        }
        else
        {
            // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
            uint32 spellIid = handler->extractSpellIdFromLink((char*) args);
            if (!spellIid)
                return false;

            if (!sSpellMgr->GetSpellInfo(spellIid))
            {
                handler->PSendSysMessage(LANG_UNKNOWN_SPELL, target == handler->GetSession()->GetPlayer() ? handler->GetTrinityString(LANG_YOU) : nameLink.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            target->RemoveSpellCooldown(spellIid, true);
            handler->PSendSysMessage(LANG_REMOVE_COOLDOWN, spellIid, target == handler->GetSession()->GetPlayer() ? handler->GetTrinityString(LANG_YOU) : nameLink.c_str());
        }
        return true;
    }

    static bool HandleGetDistanceCommand(ChatHandler* handler, char const* args)
    {
        WorldObject* obj = NULL;

        if (*args)
        {
            ObjectGuid guid = handler->extractGuidFromLink((char*)args);
            if (guid)
                obj = (WorldObject*)ObjectAccessor::GetObjectByTypeMask(*handler->GetSession()->GetPlayer(), guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);

            if (!obj)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            obj = handler->getSelectedUnit();

            if (!obj)
            {
                handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        handler->PSendSysMessage(LANG_DISTANCE, handler->GetSession()->GetPlayer()->GetDistance(obj), handler->GetSession()->GetPlayer()->GetDistance2d(obj), handler->GetSession()->GetPlayer()->GetExactDist(obj), handler->GetSession()->GetPlayer()->GetExactDist2d(obj));
        return true;
    }
    // Teleport player to last position
    static bool HandleRecallCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        if (target->IsBeingTeleported())
        {
            handler->PSendSysMessage(LANG_IS_TELEPORTED, handler->GetNameLink(target).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (target->isInFlight())
        {
            target->GetMotionMaster()->MovementExpired();
            target->CleanupAfterTaxiFlight();
        }

        target->TeleportTo(target->m_recallLoc);
        return true;
    }

    static bool HandleSaveCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        // save GM account without delay and output message
        if (!AccountMgr::IsPlayerAccount(handler->GetSession()->GetSecurity()))
        {
            if (Player* target = handler->getSelectedPlayer())
                target->SaveToDB();
            else
                player->SaveToDB();
            handler->SendSysMessage(LANG_PLAYER_SAVED);
            return true;
        }

        // save if the player has last been saved over 20 seconds ago
        uint32 saveInterval = sWorld->getIntConfig(CONFIG_INTERVAL_SAVE);
        if (saveInterval == 0 || (saveInterval > 5 * IN_MILLISECONDS && player->GetSaveTimer() <= saveInterval - 5 * IN_MILLISECONDS))
        {
            player->SaveToDB();
            handler->SendSysMessage(LANG_PLAYER_SAVED);
        }

        return true;
    }

    // Save all players in the world
    static bool HandleSaveAllCommand(ChatHandler* handler, char const* /*args*/)
    {
        sObjectAccessor->SaveAllPlayers();
        handler->SendSysMessage(LANG_PLAYERS_SAVED);
        return true;
    }

    // kick player
    static bool HandleKickPlayerCommand(ChatHandler* handler, char const* args)
    {
        Player* target = NULL;
        std::string playerName;
        if (!handler->extractPlayerTarget((char*)args, &target, NULL, &playerName))
            return false;

        if (handler->GetSession() && target == handler->GetSession()->GetPlayer())
        {
            handler->SendSysMessage(LANG_COMMAND_KICKSELF);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        if (sWorld->getBoolConfig(CONFIG_SHOW_KICK_IN_WORLD))
            sWorld->SendWorldText(LANG_COMMAND_KICKMESSAGE, playerName.c_str());
        else
            handler->PSendSysMessage(LANG_COMMAND_KICKMESSAGE, playerName.c_str());

        target->GetSession()->KickPlayer();

        return true;
    }

    static bool HandleStartCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (player->isInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->isInCombat())
        {
            handler->SendSysMessage(LANG_YOU_IN_COMBAT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->isDead() || player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        {
            if (auto instance = player->GetInstanceScript())
            {
                if (instance->IsEncounterInProgress())
                {
                    handler->SendSysMessage(LANG_YOU_IN_COMBAT);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            // if player is dead and stuck, send ghost to graveyard
            player->RepopAtGraveyard();
            return true;
        }

        // cast spell Stuck
        player->CastSpell(player, 7355, false);
        return true;
    }

    static bool HandleLinkGraveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* px = strtok((char*)args, " ");
        if (!px)
            return false;

        uint32 graveyardId = uint32(atoi(px));

        uint32 team;

        char* px2 = strtok(NULL, " ");

        if (!px2)
            team = 0;
        else if (strncmp(px2, "horde", 6) == 0)
            team = HORDE;
        else if (strncmp(px2, "alliance", 9) == 0)
            team = ALLIANCE;
        else
            return false;

        WorldSafeLocsEntry const* graveyard = sWorldSafeLocsStore.LookupEntry(graveyardId);

        if (!graveyard)
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, graveyardId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        uint32 zoneId = player->GetZoneId();

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(zoneId);
        if (!areaEntry || areaEntry->ParentAreaID !=0)
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDWRONGZONE, graveyardId, zoneId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (sObjectMgr->AddGraveYardLink(graveyardId, zoneId, team))
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDLINKED, graveyardId, zoneId);
        else
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDALRLINKED, graveyardId, zoneId);

        return true;
    }

    static bool HandleNearGraveCommand(ChatHandler* handler, char const* args)
    {
        uint32 team;

        size_t argStr = strlen(args);

        if (!*args)
            team = 0;
        else if (strncmp((char*)args, "horde", argStr) == 0)
            team = HORDE;
        else if (strncmp((char*)args, "alliance", argStr) == 0)
            team = ALLIANCE;
        else
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 zone_id = player->GetZoneId();

        WorldSafeLocsEntry const* graveyard = sObjectMgr->GetClosestGraveYard(
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), team);

        if (graveyard)
        {
            uint32 graveyardId = graveyard->ID;

            GraveYardData const* data = sObjectMgr->FindGraveYardData(graveyardId, zone_id);
            if (!data)
            {
                handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDERROR, graveyardId);
                handler->SetSentErrorMessage(true);
                return false;
            }

            team = data->team;

            std::string team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_NOTEAM);

            if (team == 0)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
            else if (team == HORDE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
            else if (team == ALLIANCE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNEAREST, graveyardId, team_name.c_str(), zone_id);
        }
        else
        {
            std::string team_name;

            if (team == 0)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
            else if (team == HORDE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
            else if (team == ALLIANCE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

            if (team == ~uint32(0))
                handler->PSendSysMessage(LANG_COMMAND_ZONENOGRAVEYARDS, zone_id);
            else
                handler->PSendSysMessage(LANG_COMMAND_ZONENOGRAFACTION, zone_id, team_name.c_str());
        }

        return true;
    }

    static bool HandleShowAreaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(atoi((char*)args));
        if (!areaEntry)
            return false;

        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 area = areaEntry->ID;
        int32 offset = area / 32;
        uint32 val = uint32((1 << (area % 32)));

        if (area<0 || offset >= PLAYER_EXPLORED_ZONES_SIZE)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 currFields = playerTarget->GetUInt32Value(PLAYER_FIELD_EXPLORED_ZONES + offset);
        playerTarget->SetUInt32Value(PLAYER_FIELD_EXPLORED_ZONES + offset, uint32((currFields | val)));

        handler->SendSysMessage(LANG_EXPLORE_AREA);
        return true;
    }

    static bool HandleHideAreaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(atoi((char*)args));
        if (!areaEntry)
            return false;

        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 area = areaEntry->ID;
        int32 offset = area / 32;

        if (offset >= PLAYER_EXPLORED_ZONES_SIZE)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 val = uint32((1 << (area % 32)));
        uint32 currFields = playerTarget->GetUInt32Value(PLAYER_FIELD_EXPLORED_ZONES + offset);
        playerTarget->SetUInt32Value(PLAYER_FIELD_EXPLORED_ZONES + offset, uint32((currFields ^ val)));

        handler->SendSysMessage(LANG_UNEXPLORE_AREA);
        return true;
    }

    static bool HandleAddItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 itemId = 0;

        if (args[0] == '[')                                        // [name] manual form
        {
            char const* itemNameStr = strtok((char*)args, "]");

            if (itemNameStr && itemNameStr[0])
            {
                std::string itemName = itemNameStr+1;
                WorldDatabase.EscapeString(itemName);

                PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_ITEM_TEMPLATE_BY_NAME);
                stmt->setString(0, itemName);
                PreparedQueryResult result = WorldDatabase.Query(stmt);

                if (!result)
                {
                    handler->PSendSysMessage(LANG_COMMAND_COULDNOTFIND, itemNameStr+1);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                itemId = result->Fetch()->GetUInt32();
            }
            else
                return false;
        }
        else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
        {
            char const* id = handler->extractKeyFromLink((char*)args, "Hitem");
            if (!id)
                return false;
            itemId = uint32(atol(id));
        }

        char const* ccount = strtok(NULL, " ");

        int32 count = 1;

        if (ccount)
            count = strtol(ccount, NULL, 10);

        if (count == 0)
            count = 1;
        
        char const* bonus = strtok(NULL, " ");

        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        TC_LOG_DEBUG(LOG_FILTER_GENERAL, handler->GetTrinityString(LANG_ADDITEM), itemId, count);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Subtract
        if (count < 0)
        {
            playerTarget->DestroyItemCount(itemId, -count, true, false);
            handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, handler->GetNameLink(playerTarget).c_str());
            return true;
        }

        // Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        InventoryResult msg = playerTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
            count -= noSpaceForCount;

        if (count == 0 || dest.empty())                         // can't add any
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::vector<uint32> bonusListIDs = sObjectMgr->GetItemBonusTree(itemId, player->GetMap()->GetDifficultyLootItemContext(), player->getLevel());
        if (bonus)
        {
            Tokenizer BonusListID(bonus, ':');
            for (char const* token : BonusListID)
                bonusListIDs.push_back(atol(token));
        }

        Item* item = playerTarget->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId), GuidSet(), bonusListIDs);
        if (count > 0 && item)
        {
            player->SendNewItem(item, count, false, true);
            if (player != playerTarget)
                playerTarget->SendNewItem(item, count, true, false);
        }
        // remove binding (let GM give it to another player later)
        if (player == playerTarget)
            for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
                if (Item* item1 = player->GetItemByPos(itr->pos))
                    item1->SetBinding(false);

        if (noSpaceForCount > 0)
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

        return true;
    }

    static bool HandleAddItemModCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;
        char* mod = strtok(NULL, " ");
        if (!mod)
            return false;
        uint32 itemId = uint32(atol(id));
        int32 count = 1;
        uint32 modId = uint32(atol(mod));

        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Subtract
        if (count < 0)
        {
            playerTarget->DestroyItemCount(itemId, -count, true, false);
            handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, handler->GetNameLink(playerTarget).c_str());
            return true;
        }

        // Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        InventoryResult msg = playerTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
            count -= noSpaceForCount;

        if (count == 0 || dest.empty())                         // can't add any
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Item* item = playerTarget->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId), GuidSet(), sObjectMgr->GetItemBonusTree(itemId, modId, player->getLevel()));

        // remove binding (let GM give it to another player later)
        if (player == playerTarget)
            for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
                if (Item* item1 = player->GetItemByPos(itr->pos))
                    item1->SetBinding(false);

        if (count > 0 && item)
        {
            player->SendNewItem(item, count, false, true);
            if (player != playerTarget)
                playerTarget->SendNewItem(item, count, true, false);
        }

        if (noSpaceForCount > 0)
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

        return true;
    }

    static bool HandleAddItemSetCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* id = handler->extractKeyFromLink((char*)args, "Hitemset"); // number or [name] Shift-click form |color|Hitemset:itemset_id|h[name]|h|r
        if (!id)
            return false;

        uint32 itemSetId = atol(id);

        // prevent generation all items with itemset field value '0'
        if (itemSetId == 0)
        {
            handler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemSetId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        TC_LOG_DEBUG(LOG_FILTER_GENERAL, handler->GetTrinityString(LANG_ADDITEMSET), itemSetId);

        bool found = false;
        ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
        for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            if (itr->second.GetItemSet() == itemSetId)
            {
                found = true;
                ItemPosCountVec dest;
                InventoryResult msg = playerTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itr->second.GetId(), 1);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = playerTarget->StoreNewItem(dest, itr->second.GetId(), true);

                    // remove binding (let GM give it to another player later)
                    if (player == playerTarget)
                        item->SetBinding(false);

                    player->SendNewItem(item, 1, false, true);
                    if (player != playerTarget)
                        playerTarget->SendNewItem(item, 1, true, false);
                }
                else
                {
                    player->SendEquipError(msg, NULL, NULL, itr->second.GetId());
                    handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itr->second.GetId(), 1);
                }
            }
        }

        if (!found)
        {
            handler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemSetId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleBankCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->GetSession()->SendShowBank(handler->GetSession()->GetPlayer()->GetGUID());
        return true;
    }

    static bool HandleChangeWeather(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // Weather is OFF
        if (!sWorld->getBoolConfig(CONFIG_WEATHER))
        {
            handler->SendSysMessage(LANG_WEATHER_DISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // *Change the weather of a cell
        char const* px = strtok((char*)args, " ");
        char const* py = strtok(NULL, " ");

        if (!px || !py)
            return false;

        uint32 type = uint32(atoi(px));                         //0 to 3, 0: fine, 1: rain, 2: snow, 3: sand
        float grade = float(atof(py));                          //0 to 1, sending -1 is instand good weather

        Player* player = handler->GetSession()->GetPlayer();
        Weather* weather = player->GetMap()->GetOrGenerateZoneDefaultWeather(player->GetZoneId());
        if (!weather)
        {
            handler->SendSysMessage(LANG_NO_WEATHER);
            handler->SetSentErrorMessage(true);
            return false;
        }

        weather->SetWeather(WeatherType(type), grade);

        return true;
    }

    static bool HandleSetSkillCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hskill:skill_id|h[name]|h|r
        char const* skillStr = handler->extractKeyFromLink((char*)args, "Hskill");
        if (!skillStr)
            return false;

        char const* levelStr = strtok(NULL, " ");
        if (!levelStr)
            return false;

        char const* maxPureSkill = strtok(NULL, " ");

        int32 skill = atoi(skillStr);
        if (skill <= 0)
        {
            handler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 level = uint32(atol(levelStr));

        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        SkillLineEntry const* skillLine = sSkillLineStore.LookupEntry(skill);
        if (!skillLine)
        {
            handler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string tNameLink = handler->GetNameLink(target);

        if (!target->GetSkillValue(skill))
        {
            handler->PSendSysMessage(LANG_SET_SKILL_ERROR, tNameLink.c_str(), skill, skillLine->DisplayName[DEFAULT_LOCALE].Str[DEFAULT_LOCALE]);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint16 max = maxPureSkill ? atol (maxPureSkill) : target->GetPureMaxSkillValue(skill);

        if (level <= 0 || level > max || max <= 0)
            return false;

        target->SetSkill(skill, target->GetSkillStep(skill), level, max);
        handler->PSendSysMessage(LANG_SET_SKILL, skill, skillLine->DisplayName[DEFAULT_LOCALE].Str[DEFAULT_LOCALE], tNameLink.c_str(), level, max);

        return true;
    }
    static bool HandlePhaseInfoCommand(ChatHandler* handler, char const* args)
    {
        if (Player* player = handler->getSelectedPlayer())
            handler->PSendSysMessage(player->GetPhaseMgr().GetPhaseIdString().c_str());

        if (Unit* target = handler->getSelectedUnit())
        {
            std::set<uint32> const& phases = target->GetPhases();
            std::ostringstream ss;
            for (uint32 phaseId : phases)
                ss << phaseId << ' ';

            handler->PSendSysMessage("GetPhases : %s", ss.str().c_str());
        }

        return true;
    }
    // show info of player
    static bool HandlePInfoCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;

        ObjectGuid parseGUID = ObjectGuid::Create<HighGuid::Player>(atol((char*)args));

        if (ObjectMgr::GetPlayerNameByGUID(parseGUID, targetName))
        {
            target = sObjectMgr->GetPlayerByLowGUID(parseGUID.GetGUIDLow());
            targetGuid = parseGUID;
        }
        else if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        uint32 accId            = 0;
        uint32 money            = 0;
        uint32 totalPlayerTime  = 0;
        uint32 totalAccountTime = 0;
        uint8 level             = 0;
        uint32 latency          = 0;
        uint8 race;
        uint8 Class;
        int64 muteTime          = 0;
        int64 banTime           = -1;
        uint32 mapId;
        uint32 areaId;
        uint32 phase            = 0;
        std::string lastLogin   = handler->GetTrinityString(LANG_ERROR);

        // get additional information from Player object
        if (target)
        {
            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            accId             = target->GetSession()->GetAccountId();
            money             = target->GetMoney();
            totalPlayerTime   = target->GetTotalPlayedTime();
            level             = target->getLevel();
            latency           = target->GetSession()->GetLatency();
            race              = target->getRace();
            Class             = target->getClass();
            muteTime          = target->GetSession()->m_muteTime;
            mapId             = target->GetMapId();
            areaId            = target->GetAreaId();
            phase             = target->GetPhaseMask();
            totalAccountTime  = target->GetTotalAccountTime();
            lastLogin = "Online";
        }
        // get additional information from DB
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_PINFO);
            stmt->setUInt64(0, targetGuid.GetGUIDLow());
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (!result)
                return false;

            Field* fields     = result->Fetch();
            totalPlayerTime = fields[0].GetUInt32();
            level             = fields[1].GetUInt8();
            money             = fields[2].GetUInt32();
            accId             = fields[3].GetUInt32();
            race              = fields[4].GetUInt8();
            Class             = fields[5].GetUInt8();
            mapId             = fields[6].GetUInt16();
            areaId            = fields[7].GetUInt16();
            lastLogin         = TimeToTimestampStr(time_t(fields[8].GetUInt32()));
        }

        std::string userName    = handler->GetTrinityString(LANG_ERROR);
        std::string eMail       = handler->GetTrinityString(LANG_ERROR);
        std::string lastIp      = handler->GetTrinityString(LANG_ERROR);
        uint32 security         = 0;

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO);
        stmt->setInt32(0, int32(realm.Id.Realm));
        stmt->setUInt32(1, accId);
        PreparedQueryResult result = LoginDatabase.Query(stmt);

        if (result)
        {
            Field* fields = result->Fetch();
            userName      = fields[0].GetString();
            security      = fields[1].GetUInt8();
            eMail         = fields[2].GetString();
            muteTime      = fields[5].GetUInt64();

            if (handler->GetSession()->GetSecurity() <= 3)
                eMail = "IT`S SECRET!";

            if (!handler->GetSession() || handler->GetSession()->GetSecurity() >= AccountTypes(security))
            {
                lastIp = fields[3].GetString();
                // lastLogin = fields[4].GetString();

                uint32 ip = inet_addr(lastIp.c_str());
#if TRINITY_ENDIAN == BIGENDIAN
                EndianConvertReverse(ip);
#endif

                PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_IP2NATION_COUNTRY);

                stmt->setUInt32(0, ip);

                PreparedQueryResult result2 = WorldDatabase.Query(stmt);

                if (result2)
                {
                    Field* fields2 = result2->Fetch();
                    lastIp.append(" (");
                    lastIp.append(fields2[0].GetString());
                    lastIp.append(")");
                }
            }
            else
            {
                lastIp = "-";
                lastLogin = "-";
               
            }
        }

        std::string nameLink = handler->playerLink(targetName);

        handler->PSendSysMessage(LANG_PINFO_ACCOUNT, (target ? "" : handler->GetTrinityString(LANG_OFFLINE)), nameLink.c_str(), targetGuid.GetGUIDLow(), userName.c_str(), accId, accId, eMail.c_str(), security, lastIp.c_str(), lastLogin.c_str(), latency);

        std::string bannedby = "unknown";
        std::string banreason = "";

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO_BANS);
        stmt->setUInt32(0, accId);
        PreparedQueryResult result2 = LoginDatabase.Query(stmt);
        if (!result2)
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_BANS);
            stmt->setUInt64(0, targetGuid.GetGUIDLow());
            result2 = CharacterDatabase.Query(stmt);
        }

        if (result2)
        {
            Field* fields = result2->Fetch();
            banTime       = int64(fields[1].GetBool() ? 0 : fields[0].GetUInt32());
            bannedby      = fields[2].GetString();
            banreason     = fields[3].GetString();
        }

        if (muteTime > 0)
            handler->PSendSysMessage(LANG_PINFO_MUTE, secsToTimeString(muteTime - time(NULL), true).c_str());

        if (banTime >= 0)
            handler->PSendSysMessage(LANG_PINFO_BAN, banTime > 0 ? secsToTimeString(banTime - time(NULL), true).c_str() : "permanently", bannedby.c_str(), banreason.c_str());

        std::string raceStr, ClassStr;
        switch (race)
        {
            case RACE_HUMAN:
                raceStr = "Human";
                break;
            case RACE_ORC:
                raceStr = "Orc";
                break;
            case RACE_DWARF:
                raceStr = "Dwarf";
                break;
            case RACE_NIGHTELF:
                raceStr = "Night Elf";
                break;
            case RACE_UNDEAD_PLAYER:
                raceStr = "Undead";
                break;
            case RACE_TAUREN:
                raceStr = "Tauren";
                break;
            case RACE_GNOME:
                raceStr = "Gnome";
                break;
            case RACE_TROLL:
                raceStr = "Troll";
                break;
            case RACE_BLOODELF:
                raceStr = "Blood Elf";
                break;
            case RACE_DRAENEI:
                raceStr = "Draenei";
                break;
            case RACE_GOBLIN:
                raceStr = "Goblin";
                break;
            case RACE_WORGEN:
                raceStr = "Worgen";
                break;
            case RACE_PANDAREN_ALLIANCE:
                raceStr = "Pandaren alliance";
                break;
            case RACE_PANDAREN_HORDE:
                raceStr = "Pandaren horde";
                break;
            case RACE_PANDAREN_NEUTRAL:
                raceStr = "Pandaren neutral";
                break;
        }

        switch (Class)
        {
            case CLASS_WARRIOR:
                ClassStr = "Warrior";
                break;
            case CLASS_PALADIN:
                ClassStr = "Paladin";
                break;
            case CLASS_HUNTER:
                ClassStr = "Hunter";
                break;
            case CLASS_ROGUE:
                ClassStr = "Rogue";
                break;
            case CLASS_PRIEST:
                ClassStr = "Priest";
                break;
            case CLASS_DEATH_KNIGHT:
                ClassStr = "Death Knight";
                break;
            case CLASS_SHAMAN:
                ClassStr = "Shaman";
                break;
            case CLASS_MAGE:
                ClassStr = "Mage";
                break;
            case CLASS_WARLOCK:
                ClassStr = "Warlock";
                break;
            case CLASS_MONK:
                ClassStr = "Monk";
                break;
            case CLASS_DRUID:
                ClassStr = "Druid";
                break;
            
        }

        std::string timeStr = secsToTimeString(totalPlayerTime, true, true);
        uint32 gold = money /GOLD;
        uint32 silv = (money % GOLD) / SILVER;
        uint32 copp = (money % GOLD) % SILVER;
        handler->PSendSysMessage(LANG_PINFO_LEVEL, raceStr.c_str(), ClassStr.c_str(), timeStr.c_str(), level, gold, silv, copp);
        if (totalAccountTime)
        {
            std::string timeAccoutnStr = secsToTimeString(totalAccountTime, true, true);
            handler->PSendSysMessage("TotalAccountTime: %s", timeAccoutnStr.c_str());
        }

        // Add map, zone, subzone and phase to output
        std::string areaName = "<unknown>";
        std::string zoneName = "";

        MapEntry const* map = sMapStore.LookupEntry(mapId);

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId);
        if (area)
        {
            areaName = area->ZoneName->Str[sObjectMgr->GetDBCLocaleIndex()];

            AreaTableEntry const* zone = sAreaTableStore.LookupEntry(area->ParentAreaID);
            if (zone)
                zoneName = zone->ZoneName->Str[sObjectMgr->GetDBCLocaleIndex()];
        }

        if (target)
        {
            if (!zoneName.empty())
                handler->PSendSysMessage(LANG_PINFO_MAP_ONLINE, map->MapName->Str[sObjectMgr->GetDBCLocaleIndex()], zoneName.c_str(), areaName.c_str(), phase);
            else
                handler->PSendSysMessage(LANG_PINFO_MAP_ONLINE, map->MapName->Str[sObjectMgr->GetDBCLocaleIndex()], areaName.c_str(), "<unknown>", phase);
        }
        else
           handler->PSendSysMessage(LANG_PINFO_MAP_OFFLINE, map->MapName->Str[sObjectMgr->GetDBCLocaleIndex()], areaName.c_str());
       
       // info about guild
       
        ObjectGuid::LowType guildId = target ? target->GetGuildId() : Player::GetGuildIdFromDB(targetGuid);
        if (!guildId)
        {
            handler->PSendSysMessage("Target don't have a guild!");
            return true;
        }
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        {
            std::string GuildLeader = (guild->GetLeaderGUID() == targetGuid ? "Yes" : "No");
            handler->PSendSysMessage("Target have guild: '%s' (id: %d). GuildLeader: %s", guild->GetName().c_str(), guildId, GuildLeader.c_str());
        }

        return true;
    }

    static bool HandleRespawnCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        // accept only explicitly selected target (not implicitly self targeting case)
        Unit* target = handler->getSelectedUnit();
        if (player->GetSelection() && target)
        {
            if (target->GetTypeId() != TYPEID_UNIT || target->isPet())
            {
                handler->SendSysMessage(LANG_SELECT_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (target->isDead())
                target->ToCreature()->Respawn(false);
            return true;
        }

        CellCoord p(Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY()));
        Cell cell(p);
        cell.SetNoCreate();

        Trinity::RespawnDo u_do;
        Trinity::WorldObjectWorker<Trinity::RespawnDo> worker(player, u_do);

        cell.Visit(p, Trinity::makeGridVisitor(worker), *player->GetMap(), *player, player->GetGridActivationRange());

        return true;
    }
    // mute player for some times
    static bool HandleMuteCommand(ChatHandler* handler, char const* args)
    {
        char* nameStr;
        char* delayStr;
        handler->extractOptFirstArg((char*)args, &nameStr, &delayStr);
        if (!delayStr)
            return false;

        char const* muteReason = strtok(NULL, "\r");
        std::string muteReasonStr = "No reason";
        if (muteReason != NULL)
            muteReasonStr = muteReason;

        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget(nameStr, &target, &targetGuid, &targetName))
            return false;

        uint32 accountId = target ? target->GetSession()->GetAccountId() : ObjectMgr::GetPlayerAccountIdByGUID(targetGuid);

        // find only player from same account if any
        if (!target)
            if (WorldSessionPtr session = sWorld->FindSession(accountId))
                target = session->GetPlayer();

        int64 notSpeakTime = (int64) atoi(delayStr);
        
        // must have strong lesser security level
        if (handler->HasLowerSecurity (target, targetGuid, true))
            return false;

        // If the account is muted add time mute
        int64 activemute = 0;
        QueryResult mutresult = LoginDatabase.PQuery("SELECT mutetime FROM account WHERE id = %u", accountId);
        if (mutresult)
        {
            do
            {
                Field* fields = mutresult->Fetch();
                activemute = fields[0].GetUInt64();
            } while (mutresult->NextRow());
        }
        int64 muteTime;

        // Target is online, mute will be in effect right away.
        if (activemute > time(NULL) && notSpeakTime > 0)
        {
            if(notSpeakTime < 4294967295)
            {
                muteTime = notSpeakTime * MINUTE;
                LoginDatabase.PQuery("UPDATE account SET mutetime = mutetime + %u WHERE id = %u", muteTime, accountId);
                if (target)
                    target->GetSession()->m_muteTime = target->GetSession()->m_muteTime + muteTime;
            }
        }
        else
        {
            if(notSpeakTime < 0)
                muteTime = 4294967295;
            else
                muteTime = time(NULL) + notSpeakTime * MINUTE;
            LoginDatabase.PQuery("UPDATE account SET mutetime = " UI64FMTD " WHERE id = %u", uint64(muteTime), accountId);
            if (target)
                target->GetSession()->m_muteTime = muteTime;
        }

        WorldDatabase.EscapeString(muteReasonStr);

        if (target)
            ChatHandler(target).PSendSysMessage(LANG_YOUR_CHAT_DISABLED, notSpeakTime, muteReasonStr.c_str());

        LoginDatabase.PQuery("INSERT INTO account_muted VALUES (%u, UNIX_TIMESTAMP(), UNIX_TIMESTAMP() + %u, '%s', '%s', 1)", accountId, notSpeakTime*60, handler->GetSession()->GetPlayer()->GetName(), muteReasonStr.c_str());

        std::string nameLink = handler->playerLink(targetName);

        handler->PSendSysMessage(target ? LANG_YOU_DISABLE_CHAT : LANG_COMMAND_DISABLE_CHAT_DELAYED, nameLink.c_str(), notSpeakTime, muteReasonStr.c_str());

        if (sWorld->getBoolConfig(CONFIG_ANNOUNCE_MUTE))
        {
            std::string announce;

            announce = "The character '";
            if (nameStr)  announce += nameStr;
            announce += "' was muted for ";
            if (delayStr) announce += delayStr;
            announce += " minutes by the character '";
            announce += handler->GetSession()? handler->GetSession()->GetPlayerName(): "";
            announce += "'. The reason is: ";
            announce += muteReasonStr;

            char buff[2048];
            sprintf(buff, handler->GetTrinityString(LANG_SYSTEMMESSAGE), announce.c_str());
            sWorld->SendServerMessage(SERVER_MSG_STRING, buff);
        }

        sWorld->SendGMText(27000, nameLink.c_str(), notSpeakTime, handler->GetSession()->GetPlayerName().c_str(), muteReasonStr.c_str());
        return true;
    }

    // unmute player
    static bool HandleUnmuteCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        uint32 accountId = target ? target->GetSession()->GetAccountId() : ObjectMgr::GetPlayerAccountIdByGUID(targetGuid);

        // find only player from same account if any
        if (!target)
            if (WorldSessionPtr session = sWorld->FindSession(accountId))
                target = session->GetPlayer();

        // must have strong lesser security level
        if (handler->HasLowerSecurity (target, targetGuid, true))
            return false;

        if (target)
        {
            if (target->CanSpeak())
            {
                handler->SendSysMessage(LANG_CHAT_ALREADY_ENABLED);
                handler->SetSentErrorMessage(true);
                return false;
            }

            target->GetSession()->m_muteTime = 0;
        }

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
        stmt->setInt64(0, 0);
        stmt->setUInt32(1, accountId);
        LoginDatabase.Execute(stmt);

        if (target)
            ChatHandler(target).PSendSysMessage(LANG_YOUR_CHAT_ENABLED);

        std::string nameLink = handler->playerLink(targetName);

        handler->PSendSysMessage(LANG_YOU_ENABLE_CHAT, nameLink.c_str());

        sWorld->SendGMText(27001, nameLink.c_str(), handler->GetSession()->GetPlayerName().c_str());

        return true;
    }


    static bool HandleMovegensCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_MOVEGENS_LIST, (unit->GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), unit->GetGUID().GetGUIDLow());

        MotionMaster* motionMaster = unit->GetMotionMaster();
        float x, y, z;
        motionMaster->GetDestination(x, y, z);

        for (uint8 i = 0; i < MAX_MOTION_SLOT; ++i)
        {
            MovementGenerator* movementGenerator = motionMaster->GetMotionSlot(i);
            if (!movementGenerator)
            {
                handler->SendSysMessage("Empty");
                continue;
            }

            switch (movementGenerator->GetMovementGeneratorType())
            {
                case IDLE_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_IDLE);
                    break;
                case RANDOM_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_RANDOM);
                    break;
                case WAYPOINT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_WAYPOINT);
                    break;
                case ANIMAL_RANDOM_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_ANIMAL_RANDOM);
                    break;
                case CONFUSED_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_CONFUSED);
                    break;
                case CHASE_MOTION_TYPE:
                {
                    Unit* target = NULL;
                    if (unit->GetTypeId() == TYPEID_PLAYER)
                        target = static_cast<ChaseMovementGenerator<Player> const*>(movementGenerator)->GetTarget();
                    else
                        target = static_cast<ChaseMovementGenerator<Creature> const*>(movementGenerator)->GetTarget();

                    if (!target)
                        handler->SendSysMessage(LANG_MOVEGENS_CHASE_NULL);
                    else if (target->GetTypeId() == TYPEID_PLAYER)
                        handler->PSendSysMessage(LANG_MOVEGENS_CHASE_PLAYER, target->GetName(), target->GetGUID().GetGUIDLow());
                    else
                        handler->PSendSysMessage(LANG_MOVEGENS_CHASE_CREATURE, target->GetName(), target->GetGUID().GetGUIDLow());
                    break;
                }
                case FOLLOW_MOTION_TYPE:
                {
                    Unit* target = NULL;
                    if (unit->GetTypeId() == TYPEID_PLAYER)
                        target = static_cast<FollowMovementGenerator<Player> const*>(movementGenerator)->GetTarget();
                    else
                        target = static_cast<FollowMovementGenerator<Creature> const*>(movementGenerator)->GetTarget();

                    if (!target)
                        handler->SendSysMessage(LANG_MOVEGENS_FOLLOW_NULL);
                    else if (target->GetTypeId() == TYPEID_PLAYER)
                        handler->PSendSysMessage(LANG_MOVEGENS_FOLLOW_PLAYER, target->GetName(), target->GetGUID().GetGUIDLow());
                    else
                        handler->PSendSysMessage(LANG_MOVEGENS_FOLLOW_CREATURE, target->GetName(), target->GetGUID().GetGUIDLow());
                    break;
                }
                case HOME_MOTION_TYPE:
                {
                    if (unit->GetTypeId() == TYPEID_UNIT)
                        handler->PSendSysMessage(LANG_MOVEGENS_HOME_CREATURE, x, y, z);
                    else
                        handler->SendSysMessage(LANG_MOVEGENS_HOME_PLAYER);
                    break;
                }
                case FLIGHT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_FLIGHT);
                    break;
                case POINT_MOTION_TYPE:
                {
                    handler->PSendSysMessage(LANG_MOVEGENS_POINT, x, y, z);
                    break;
                }
                case FLEEING_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_FEAR);
                    break;
                case DISTRACT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_DISTRACT);
                    break;
                case EFFECT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_EFFECT);
                    break;
                default:
                    handler->PSendSysMessage(LANG_MOVEGENS_UNKNOWN, movementGenerator->GetMovementGeneratorType());
                    break;
            }
        }
        return true;
    }
    /*
    ComeToMe command REQUIRED for 3rd party scripting library to have access to PointMovementGenerator
    Without this function 3rd party scripting library will get linking errors (unresolved external)
    when attempting to use the PointMovementGenerator
    */
    static bool HandleComeToMeCommand(ChatHandler* handler, char const* args)
    {
        char const* newFlagStr = strtok((char*)args, " ");
        if (!newFlagStr)
            return false;

        Creature* caster = handler->getSelectedCreature();
        if (!caster)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        caster->GetMotionMaster()->MovePoint(0, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

        return true;
    }

    static bool HandleDamageCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Unit* target = handler->getSelectedUnit();
        Player* player = handler->GetSession()->GetPlayer();
        if (!target || !player->GetSelection())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            if (handler->HasLowerSecurity((Player*)target, ObjectGuid::Empty, false))
                return false;
        }

        if (!target->isAlive())
            return true;

        char* damageStr = strtok((char*)args, " ");
        if (!damageStr)
            return false;

        int32 damage_int = atoi((char*)damageStr);
        if (damage_int <= 0)
            return true;

        uint32 damage = damage_int;

        char* schoolStr = strtok((char*)NULL, " ");

        // flat melee damage without resistence/etc reduction
        if (!schoolStr)
        {
            player->DealDamage(target, damage, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            if (target != player)
                player->SendAttackStateUpdate (HITINFO_AFFECTS_VICTIM, target, SPELL_SCHOOL_MASK_NORMAL, damage, 0, 0, VICTIMSTATE_HIT, 0);
            return true;
        }

        uint32 school = schoolStr ? atoi((char*)schoolStr) : SPELL_SCHOOL_NORMAL;
        if (school >= MAX_SPELL_SCHOOL)
            return false;

        SpellSchoolMask schoolmask = SpellSchoolMask(1 << school);

        if (Unit::IsDamageReducedByArmor(schoolmask))
            damage = player->CalcArmorReducedDamage(player, target, damage, nullptr);

        char* spellStr = strtok((char*)NULL, " ");

        // melee damage by specific school
        if (!spellStr)
        {
            uint32 absorb = 0;
            uint32 resist = 0;

            player->CalcAbsorbResist(target, schoolmask, SPELL_DIRECT_DAMAGE, damage, &absorb, &resist);

            if (damage <= absorb + resist)
                return true;

            damage -= absorb + resist;

            player->DealDamageMods(target, damage, &absorb);
            player->DealDamage(target, damage, NULL, DIRECT_DAMAGE, schoolmask, NULL, false);
            player->SendAttackStateUpdate (HITINFO_AFFECTS_VICTIM, target, schoolmask, damage, absorb, resist, VICTIMSTATE_HIT, 0);
            return true;
        }

        // non-melee damage

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellid = handler->extractSpellIdFromLink((char*)args);
        auto const& spellInfo = sSpellMgr->GetSpellInfo(spellid);
        if (!spellid || !spellInfo)
            return false;

        player->SendSpellNonMeleeDamageLog(target, spellid, damage, schoolmask, 0, 0, false, 0, spellInfo->GetSpellXSpellVisualId(target), false);
        return true;
    }

    static bool HandleCombatStopCommand(ChatHandler* handler, char const* args)
    {
        Player* target = nullptr;
        Creature* creature = handler->getSelectedUnit()->ToCreature();

        if (creature && creature->isInCombat())
        {
            creature->AI()->EnterEvadeMode();
            return true;
        }

        if (args && strlen(args) > 0)
        {
            std::string name(args);
            target = sObjectAccessor->FindPlayerByName(name);
            if (!target)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (!target)
        {
            if (!handler->extractPlayerTarget((char*)args, &target))
                return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        target->CombatStop();
        target->getHostileRefManager().deleteReferences();
        return true;
    }

    static bool HandleFlushArenaPointsCommand(ChatHandler* /*handler*/, char const* /*args*/)
    {
        //sArenaTeamMgr->DistributeArenaPoints();
        return true;
    }

    static bool HandleRepairitemsCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        // Repair items
        target->DurabilityRepairAll(false, 0, false);

        handler->PSendSysMessage(LANG_YOU_REPAIR_ITEMS, handler->GetNameLink(target).c_str());
        if (handler->needReportToTarget(target))
            ChatHandler(target).PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, handler->GetNameLink().c_str());

        return true;
    }

    // Send mail by command
    static bool HandleSendMailCommand(ChatHandler* handler, char const* args)
    {
        // format: name "subject text" "mail text"
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        char* tail1 = strtok(NULL, "");
        if (!tail1)
            return false;

        char const* msgSubject = handler->extractQuotedArg(tail1);
        if (!msgSubject)
            return false;

        char* tail2 = strtok(NULL, "");
        if (!tail2)
            return false;

        char const* msgText = handler->extractQuotedArg(tail2);
        if (!msgText)
            return false;

        // msgSubject, msgText isn't NUL after prev. check
        std::string subject = msgSubject;
        std::string text    = msgText;

        // from console show not existed sender
        MailSender sender(MAIL_NORMAL, handler->GetSession() ? handler->GetSession()->GetPlayer()->GetGUID().GetGUIDLow() : 0, MAIL_STATIONERY_GM);

        //- TODO: Fix poor design
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        MailDraft(subject, text)
            .SendMailTo(trans, MailReceiver(target, targetGuid.GetGUIDLow()), sender);

        CharacterDatabase.CommitTransaction(trans);

        std::string nameLink = handler->playerLink(targetName);
        handler->PSendSysMessage(LANG_MAIL_SENT, nameLink.c_str());
        return true;
    }
    // Send items by mail
    static bool HandleSendItemsCommand(ChatHandler* handler, char const* args)
    {
        // format: name "subject text" "mail text" item1[:count1] item2[:count2] ... item12[:count12]
        Player* receiver;
        ObjectGuid receiverGuid;
        std::string receiverName;
        if (!handler->extractPlayerTarget((char*)args, &receiver, &receiverGuid, &receiverName))
            return false;

        char* tail1 = strtok(NULL, "");
        if (!tail1)
            return false;

        char const* msgSubject = handler->extractQuotedArg(tail1);
        if (!msgSubject)
            return false;

        char* tail2 = strtok(NULL, "");
        if (!tail2)
            return false;

        char const* msgText = handler->extractQuotedArg(tail2);
        if (!msgText)
            return false;

        // msgSubject, msgText isn't NUL after prev. check
        std::string subject = msgSubject;
        std::string text    = msgText;

        // extract items
        typedef std::pair<uint32, uint32> ItemPair;
        typedef std::list< ItemPair > ItemPairs;
        ItemPairs items;

        // get all tail string
        char* tail = strtok(NULL, "");

        // get from tail next item str
        while (char* itemStr = strtok(tail, " "))
        {
            // and get new tail
            tail = strtok(NULL, "");

            // parse item str
            char const* itemIdStr = strtok(itemStr, ":");
            char const* itemCountStr = strtok(NULL, " ");

            uint32 itemId = atoi(itemIdStr);
            if (!itemId)
                return false;

            ItemTemplate const* item_proto = sObjectMgr->GetItemTemplate(itemId);
            if (!item_proto)
            {
                handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
                handler->SetSentErrorMessage(true);
                return false;
            }

            uint32 itemCount = itemCountStr ? atoi(itemCountStr) : 1;
            if (itemCount < 1 || (item_proto->GetMaxCount() > 0 && itemCount > uint32(item_proto->GetMaxCount())))
            {
                handler->PSendSysMessage(LANG_COMMAND_INVALID_ITEM_COUNT, itemCount, itemId);
                handler->SetSentErrorMessage(true);
                return false;
            }

            while (itemCount > item_proto->GetMaxStackSize())
            {
                items.push_back(ItemPair(itemId, item_proto->GetMaxStackSize()));
                itemCount -= item_proto->GetMaxStackSize();
            }

            items.push_back(ItemPair(itemId, itemCount));

            if (items.size() > MAX_MAIL_ITEMS)
            {
                handler->PSendSysMessage(LANG_COMMAND_MAIL_ITEMS_LIMIT, MAX_MAIL_ITEMS);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        // from console show not existed sender
        MailSender sender(MAIL_NORMAL, handler->GetSession() ? handler->GetSession()->GetPlayer()->GetGUID().GetGUIDLow() : 0, MAIL_STATIONERY_GM);

        // fill mail
        MailDraft draft(subject, text);

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        for (ItemPairs::const_iterator itr = items.begin(); itr != items.end(); ++itr)
        {
            if (Item* item = Item::CreateItem(itr->first, itr->second, handler->GetSession() ? handler->GetSession()->GetPlayer() : 0))
            {
                item->SaveToDB(trans);                               // save for prevent lost at next mail load, if send fail then item will deleted
                draft.AddItem(item);
            }
        }

        draft.SendMailTo(trans, MailReceiver(receiver, receiverGuid.GetGUIDLow()), sender);
        CharacterDatabase.CommitTransaction(trans);

        std::string nameLink = handler->playerLink(receiverName);
        handler->PSendSysMessage(LANG_MAIL_SENT, nameLink.c_str());
        return true;
    }
    /// Send money by mail
    static bool HandleSendMoneyCommand(ChatHandler* handler, char const* args)
    {
        /// format: name "subject text" "mail text" money

        Player* receiver;
        ObjectGuid receiverGuid;
        std::string receiverName;
        if (!handler->extractPlayerTarget((char*)args, &receiver, &receiverGuid, &receiverName))
            return false;

        char* tail1 = strtok(NULL, "");
        if (!tail1)
            return false;

        char* msgSubject = handler->extractQuotedArg(tail1);
        if (!msgSubject)
            return false;

        char* tail2 = strtok(NULL, "");
        if (!tail2)
            return false;

        char* msgText = handler->extractQuotedArg(tail2);
        if (!msgText)
            return false;

        char* moneyStr = strtok(NULL, "");
        int32 money = moneyStr ? atoi(moneyStr) : 0;
        if (money <= 0)
            return false;

        // msgSubject, msgText isn't NUL after prev. check
        std::string subject = msgSubject;
        std::string text    = msgText;

        // from console show not existed sender
        MailSender sender(MAIL_NORMAL, handler->GetSession() ? handler->GetSession()->GetPlayer()->GetGUID().GetGUIDLow() : 0, MAIL_STATIONERY_GM);

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        MailDraft(subject, text)
            .AddMoney(money)
            .SendMailTo(trans, MailReceiver(receiver, receiverGuid.GetGUIDLow()), sender);

        CharacterDatabase.CommitTransaction(trans);

        std::string nameLink = handler->playerLink(receiverName);
        handler->PSendSysMessage(LANG_MAIL_SENT, nameLink.c_str());
        return true;
    }
    /// Send a message to a player in game
    static bool HandleSendMessageCommand(ChatHandler* handler, char const* args)
    {
        /// - Find the player
        Player* player;
        if (!handler->extractPlayerTarget((char*)args, &player))
            return false;

        char* msgStr = strtok(NULL, "");
        if (!msgStr)
            return false;

        ///- Check that he is not logging out.
        if (player->GetSession()->isLogingOut())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        /// - Send the message
        // Use SendAreaTriggerMessage for fastest delivery.
        //player->GetSession()->SendAreaTriggerMessage("%s", msgStr);
        //player->GetSession()->SendAreaTriggerMessage("|cffff0000[Message from administrator]:|r");

        // Confirmation message
        std::string nameLink = handler->GetNameLink(player);
        handler->PSendSysMessage(LANG_SENDMESSAGE, nameLink.c_str(), msgStr);

        return true;
    }

    static bool HandleCreatePetCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creatureTarget = handler->getSelectedCreature();

        if (!creatureTarget || creatureTarget->isPet() || creatureTarget->GetTypeId() == TYPEID_PLAYER)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* creatrueTemplate = sObjectMgr->GetCreatureTemplate(creatureTarget->GetEntry());
        // Creatures with family 0 crashes the server
        if (!creatrueTemplate->Family)
        {
            handler->PSendSysMessage("This creature cannot be tamed. (family id: 0).");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetPetGUID())
        {
            handler->PSendSysMessage("You already have a pet");
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Everything looks OK, create new pet
        Pet* pet = new Pet(player, HUNTER_PET);
        if (!pet->CreateBaseAtCreature(creatureTarget))
        {
            delete pet;
            handler->PSendSysMessage("Error 1");
            return false;
        }

        creatureTarget->setDeathState(JUST_DIED);
        creatureTarget->RemoveCorpse();
        creatureTarget->SetHealth(0); // just for nice GM-mode view

        pet->SetGuidValue(UNIT_FIELD_CREATED_BY, player->GetGUID());
        pet->SetUInt32Value(UNIT_FIELD_FACTION_TEMPLATE, player->getFaction());

        if (!pet->InitStatsForLevel(creatureTarget->getLevel()))
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "InitStatsForLevel() in EffectTameCreature failed! Pet deleted.");
            handler->PSendSysMessage("Error 2");
            delete pet;
            return false;
        }

        // prepare visual effect for levelup
        pet->SetLevel(creatureTarget->getLevel()-1);

        pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
        // this enables pet details window (Shift+P)
        pet->InitPetCreateSpells();
        pet->SetFullHealth();

        pet->GetMap()->AddToMap(pet->ToCreature());

        // visual effect for levelup
        pet->SetLevel(creatureTarget->getLevel());
        pet->SetEffectiveLevel(creatureTarget->GetEffectiveLevel());

        player->SetMinion(pet, true);
        pet->SavePetToDB();
        player->PetSpellInitialize();

        return true;
    }

    static bool HandlePetLearnCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        Pet* pet = player->GetPet();

        if (!pet)
        {
            handler->PSendSysMessage("You have no pet");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (!spellId || !sSpellMgr->GetSpellInfo(spellId))
            return false;

        // Check if pet already has it
        if (pet->HasSpell(spellId))
        {
            handler->PSendSysMessage("Pet already has spell: %u", spellId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Check if spell is valid
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo))
        {
            handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spellId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pet->learnSpell(spellId);

        handler->PSendSysMessage("Pet has learned spell %u", spellId);
        return true;
    }

    static bool HandlePetUnlearnCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        Pet* pet = player->GetPet();
        if (!pet)
        {
            handler->PSendSysMessage("You have no pet");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (pet->HasSpell(spellId))
            pet->removeSpell(spellId);
        else
            handler->PSendSysMessage("Pet doesn't have that spell");

        return true;
    }

    static bool HandleFreezeCommand(ChatHandler* handler, char const* args)
    {
        std::string name;
        Player* player;
        char const* TargetName = strtok((char*)args, " "); // get entered name
        if (!TargetName) // if no name entered use target
        {
            player = handler->getSelectedPlayer();
            if (player) //prevent crash with creature as target
            {
                name = player->GetName();
                normalizePlayerName(name);
            }
        }
        else // if name entered
        {
            name = TargetName;
            normalizePlayerName(name);
            player = sObjectAccessor->FindPlayerByName(name);
        }

        if (!player)
        {
            handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
            return true;
        }

        if (player == handler->GetSession()->GetPlayer())
        {
            handler->SendSysMessage(LANG_COMMAND_FREEZE_ERROR);
            return true;
        }

        if(player->GetSession()->GetSecurity() > handler->GetSession()->GetSecurity())
        {
            handler->PSendSysMessage("This player have higher GM level, than your GM level!");
            return true;
        }
        
        // effect
        if (player && (player != handler->GetSession()->GetPlayer()))
        {
            handler->PSendSysMessage(LANG_COMMAND_FREEZE, name.c_str());

            // stop combat + make player unattackable + duel stop + stop some spells
            player->setFaction(35);
            player->CombatStop();
            if (player->IsNonMeleeSpellCast(true))
                player->InterruptNonMeleeSpells(true);
            player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            // if player class = hunter || warlock remove pet if alive
            if ((player->getClass() == CLASS_HUNTER) || (player->getClass() == CLASS_WARLOCK))
            {
                if (Pet* pet = player->GetPet())
                {
                    pet->SavePetToDB();
                 // not let dismiss dead pet
                 if (pet && pet->isAlive())
                    player->RemovePet(pet);
                }
            }

            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(9454))
                if (Aura* aura = Aura::TryRefreshStackOrCreate(spellInfo, MAX_EFFECT_MASK, player, player))
                    aura->ApplyForTargets();

            // save player
            player->SaveToDB();
        }

        return true;
    }

    static bool HandleUnFreezeCommand(ChatHandler* handler, char const*args)
    {
        std::string name;
        Player* player;
        char* targetName = strtok((char*)args, " "); // Get entered name

        if (targetName)
        {
            name = targetName;
            normalizePlayerName(name);
            player = sObjectAccessor->FindPlayerByName(name);
        }
        else // If no name was entered - use target
        {
            player = handler->getSelectedPlayer();
            if (player)
                name = player->GetName();
        }

        if (player)
        {
            handler->PSendSysMessage(LANG_COMMAND_UNFREEZE, name.c_str());

            // Reset player faction + allow combat + allow duels
            player->setFactionForRace(player->getRace());
            player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            // Remove Freeze spell (allowing movement and spells)
            player->RemoveAurasDueToSpell(9454);

            // Save player
            player->SaveToDB();
        }
        else
        {
            if (targetName)
            {
                // Check for offline players
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_GUID_BY_NAME);
                stmt->setString(0, name);
                PreparedQueryResult result = CharacterDatabase.Query(stmt);

                if (!result)
                {
                    handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                    return true;
                }

                // If player found: delete his freeze aura
                Field* fields = result->Fetch();
                uint32 lowGuid = fields[0].GetUInt32();

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_AURA_FROZEN);
                stmt->setUInt64(0, lowGuid);
                CharacterDatabase.Execute(stmt);

                handler->PSendSysMessage(LANG_COMMAND_UNFREEZE, name.c_str());
                return true;
            }
            handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
            return true;
        }

        return true;
    }

    static bool HandleListFreezeCommand(ChatHandler* handler, char const* /*args*/)
    {
        // Get names from DB
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_AURA_FROZEN);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (!result)
        {
            handler->SendSysMessage(LANG_COMMAND_NO_FROZEN_PLAYERS);
            return true;
        }

        // Header of the names
        handler->PSendSysMessage(LANG_COMMAND_LIST_FREEZE);

        // Output of the results
        do
        {
            Field* fields = result->Fetch();
            std::string player = fields[0].GetString();
            handler->PSendSysMessage(LANG_COMMAND_FROZEN_PLAYERS, player.c_str());
        }
        while (result->NextRow());

        return true;
    }

    static bool HandleGroupLeaderCommand(ChatHandler* handler, char const* args)
    {
        Player* player = NULL;
        Group* group = NULL;
        ObjectGuid guid;
        char* nameStr = strtok((char*)args, " ");

        if (handler->GetPlayerGroupAndGUIDByName(nameStr, player, group, guid))
            if (group && group->GetLeaderGUID() != guid)
            {
                group->ChangeLeader(guid);
                group->SendUpdate();
            }

            return true;
    }

    static bool HandleGroupDisbandCommand(ChatHandler* handler, char const* args)
    {
        Player* player = NULL;
        Group* group = NULL;
        ObjectGuid guid;
        char* nameStr = strtok((char*)args, " ");

        if (handler->GetPlayerGroupAndGUIDByName(nameStr, player, group, guid))
            if (group)
                group->Disband();

        return true;
    }

    static bool HandleGroupRemoveCommand(ChatHandler* handler, char const* args)
    {
        Player* player = NULL;
        Group* group = NULL;
        ObjectGuid guid;
        char* nameStr = strtok((char*)args, " ");

        if (handler->GetPlayerGroupAndGUIDByName(nameStr, player, group, guid, true))
            if (group)
                group->RemoveMember(guid);

        return true;
    }

    static bool HandlePlayAllCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 soundId = atoi((char*)args);
        handler->GetSession()->GetPlayer()->PlayDirectSound(soundId, NULL);
        handler->PSendSysMessage(LANG_COMMAND_PLAYED_TO_ALL, soundId);
        return true;
    }

    static bool HandlePossessCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;

        handler->GetSession()->GetPlayer()->CastSpell(unit, 530, true);
        return true;
    }

    static bool HandleUnPossessCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            unit = handler->GetSession()->GetPlayer();

        unit->RemoveCharmAuras();

        return true;
    }

    static bool HandleBindSightCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;

        handler->GetSession()->GetPlayer()->CastSpell(unit, 6277, true);
        return true;
    }

    static bool HandleUnbindSightCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (player->isPossessing())
            return false;

        player->StopCastingBindSight();
        return true;
    }

    static bool HandleSelectFactionCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->GetSession()->GetPlayer()->ShowNeutralPlayerFactionSelectUI();
        return true;
    }

    static bool HandleOutItemTemplateCommand(ChatHandler* handler, char const* /*args*/)
    {
        //std::ofstream file("item_template.sql");
        //for (uint32 i = 0; i < sItemStore.GetNumRows(); ++i)
        //{
        //    if (const ItemEntry* itemEntry = sItemStore.LookupEntry(i))
        //        if (const ItemSparseEntry* entry = sItemSparseStore.LookupEntry(i))
        //        {
        //            file << "REPLACE INTO item_template VALUES (" << itemEntry->ID << ", " << itemEntry->Class << ", " << itemEntry->SubClass << ", " << itemEntry->SoundOverrideSubclass
        //                << ", \"" << entry->Name1 << "\", " << sDB2Manager.GetItemDisplayId(itemEntry->ID, 0) << ", " << entry->Quality << ", " << entry->Flags[0] << ", " << entry->Flags[1] << ", " << entry->PriceRandomValue
        //                << ", " << entry->PriceVariance << ", " << entry->VendorStackCount << ", " << entry->BuyPrice << ", " << entry->SellPrice << ", " << entry->InventoryType << ", " << entry->AllowableClass
        //                << ", " << entry->AllowableRace << ", " << entry->ItemLevel << ", " << entry->RequiredLevel << ", " << entry->RequiredSkill << ", " << entry->RequiredSkillRank
        //                << ", " << entry->RequiredSpell << ", " << entry->RequiredHonorRank << ", " << entry->RequiredCityRank << ", " << entry->RequiredReputationFaction << ", " << entry->RequiredReputationRank
        //                << ", " << entry->MaxCount << ", " << entry->Stackable << ", " << entry->ContainerSlots;

        //                for (uint8 i = 0; i < 10; ++i)
        //                    file << ", " << entry->ItemStatType[i] << ", " << entry->ItemStatValue[i] << ", " << entry->StatPercentEditor[i] << ", " << entry->StatPercentageOfSocket[i];

        //                file << ", " << entry->ScalingStatDistribution << ", " << entry->DamageType << ", " << entry->Delay << ", " << entry->ItemRange;

        //                //ToDo: new field
        //                //for (uint8 i = 0; i < 5; ++i)
        //                //    file << ", " << entry->SpellId[i] << ", " << entry->SpellTrigger[i] << ", " << entry->SpellCharges[i] << ", " << entry->SpellCooldown[i] << ", " << entry->SpellCategory[i]
        //                //              << ", " << entry->SpellCategoryCooldown[i];

        //                file << ", " << entry->Bonding << ", \"" << entry->Description << "\", " << entry->PageText << ", " << entry->LanguageID << ", " << entry->PageMaterial << ", " << entry->StartQuest
        //                    << ", " << entry->LockID << ", " << entry->Material << ", " << entry->Sheath << ", " << entry->RandomProperty << ", " << entry->RandomSuffix << ", " << entry->ItemSet
        //                    << ", " << 0 << ", " << entry->Area << ", " << entry->Map << ", " << entry->BagFamily << ", " << entry->TotemCategory
        //                    << ", " << 0 << ", " << 0 << ", " << 0 << ", " << 0 << ", " << 0 << ", " << 0
        //                    << ", " << entry->SocketBonus << ", " << entry->GemProperties << ", " << entry->QualityModifier << ", " << entry->DurationInInventory << ", " << entry->ItemLimitCategory
        //                    << ", " << entry->HolidayID << ", " << entry->DmgVariance << ", " << entry->CurrencySubstitutionID << ", " << entry->CurrencySubstitutionCount << ", " << 0 << ", " << 16135 << ");\n";
        //        }
        //}
        //file.close();
        return true;
    }
    static bool HandleCharDisplayMainhandCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);

        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, EQUIPMENT_SLOT_MAINHAND))
        {
            pl->m_vis->m_visMainhand = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayHeadCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId); 
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, EQUIPMENT_SLOT_HEAD))
        {
            pl->m_vis->m_visHead = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayShouldersCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, EQUIPMENT_SLOT_SHOULDERS))
        {
            pl->m_vis->m_visShoulders = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayChestCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, EQUIPMENT_SLOT_CHEST))
        {
            pl->m_vis->m_visChest = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayWaistCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 5))
        {
            pl->m_vis->m_visWaist = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayLegsCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 6))
        {
            pl->m_vis->m_visLegs = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayFeetCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 7))
        {
            pl->m_vis->m_visFeet = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayWristsCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 8))
        {
            pl->m_vis->m_visWrists = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayHandsCommand(ChatHandler* handler, const char* args) //РЎСѓРєРё, РїРѕРєР°Р¶РёС‚Рµ СЃРІРѕРё СЂСѓРєРё!
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 9))
        {
            pl->m_vis->m_visHands = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayBackCommand(ChatHandler* handler,const char* args) //РЎСѓРєРё, РїРѕРєР°Р¶РёС‚Рµ СЃРІРѕРё СЂСѓРєРё!
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 14))
        {
            pl->m_vis->m_visBack = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayOffhandCommand(ChatHandler* handler, const char* args) //РЎСѓРєРё, РїРѕРєР°Р¶РёС‚Рµ СЃРІРѕРё СЂСѓРєРё!
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 16))
        {
            pl->m_vis->m_visOffhand = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayRangedCommand(ChatHandler* handler, const char* args) //РЎСѓРєРё, РїРѕРєР°Р¶РёС‚Рµ СЃРІРѕРё СЂСѓРєРё!
    {
        if (!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*) args, "Hitem");
        if (!cId)
            return false;

        uint32 newItem = (uint32) atol(cId);
        Player* pl = handler->GetSession()->GetPlayer();

        if (!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, 17))
        {
            pl->m_vis->m_visRanged = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayTabardCommand(ChatHandler* handler, const char* args) //РЎСѓРєРё, РїРѕРєР°Р¶РёС‚Рµ СЃРІРѕРё СЂСѓРєРё!
    {
        if(!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*)args,"Hitem");
        if(!cId)
            return false;

        uint32 newItem = (uint32)atol(cId);

        Player* pl = handler->GetSession()->GetPlayer();

        if(!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, EQUIPMENT_SLOT_TABARD))
        {
            pl->m_vis->m_visTabard = newItem;
            return true;
        }
        return false;
    }

    static bool HandleCharDisplayShirtCommand(ChatHandler* handler, const char* args) //РЎСѓРєРё, РїРѕРєР°Р¶РёС‚Рµ СЃРІРѕРё СЂСѓРєРё!
    {
        if(!*args)
            return false;
        char* cId = handler->extractKeyFromLink((char*)args,"Hitem");
        if(!cId)
            return false;

        uint32 newItem = (uint32)atol(cId);

        Player* pl = handler->GetSession()->GetPlayer();

        if(!pl->m_vis)
            pl->m_vis = new Visuals;

        if (pl->HandleChangeSlotModel(newItem, EQUIPMENT_SLOT_BODY))
        {
            pl->m_vis->m_visShirt = newItem;
            return true;
        }
        return false;
    }

    static bool HandleItemSpecCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 itemId = atol((char*)args);

        if (itemId == 0)
        {
            handler->PSendSysMessage("Item not set");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (std::vector<ItemSpecOverrideEntry const*> const* itemSpecOverrides = sDB2Manager.GetItemSpecOverrides(itemId))
            for (ItemSpecOverrideEntry const* itemSpecOverride : *itemSpecOverrides)
                handler->PSendSysMessage("ItemSpecsList %u", itemSpecOverride->SpecID);

        ItemSparseEntry const* sparse = sItemSparseStore.LookupEntry(itemId);
        ItemEntry const* db2Data = sItemStore.LookupEntry(itemId);

        ItemSpecStats itemSpecStats(db2Data, sparse);
        handler->PSendSysMessage("ItemSpecStats ItemSpecStatCount %u", itemSpecStats.ItemSpecStatCount);

        /*if (itemSpecStats.ItemSpecStatCount || itemSpecStats.ItemSpecPrimaryStat != -1)
        {
            for (uint32 i = 0; i < sItemSpecStore.GetNumRows(); ++i)
            {
                if (ItemSpecEntry const* itemSpec = sItemSpecStore.LookupEntry(i))
                {
                    if (itemSpecStats.ItemType != itemSpec->ItemType)
                        continue;

                    bool hasPrimary = false;
                    if (itemSpecStats.ItemSpecPrimaryStat == itemSpec->PrimaryStat)
                        hasPrimary = true;

                    bool hasSecondary = itemSpec->SecondaryStat == ITEM_SPEC_STAT_NONE || itemSpecStats.ItemSpecStatCount == 0;
                    for (uint32 i = 0; i < itemSpecStats.ItemSpecStatCount; ++i)
                    {
                        if (itemSpecStats.ItemSpecStatTypes[i] == itemSpec->SecondaryStat)
                            hasSecondary = true;
                    }

                    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "ItemSpecStats SpecID %u hasPrimary %u, hasSecondary %u ItemSpecPrimaryStat %i PrimaryStat %i", itemSpec->SpecID, hasPrimary, hasSecondary, itemSpecStats.ItemSpecPrimaryStat, itemSpec->PrimaryStat);

                    if (!hasPrimary || !hasSecondary)
                        continue;

                    handler->PSendSysMessage("ItemSpecStats SpecID %u hasPrimary %u, hasSecondary %u", itemSpec->SpecID, hasPrimary, hasSecondary);
                }
            }
        }*/

        handler->PSendSysMessage("end");
        return true;
    }

    static bool HandleTransferAbortReasonCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint8 reasonId = atoi((char*)args);

        handler->GetSession()->GetPlayer()->SendTransferAborted(0, TransferAbortReason(reasonId));
        return true;
    }

    static bool HandleSetScenarioCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 scenarioId = atoi((char*)args);

        handler->GetSession()->GetPlayer()->SetScenarioId(scenarioId);

        handler->PSendSysMessage("HandleSetScenarioCommand scenarioId %u GetScenarioId %u", scenarioId, handler->GetSession()->GetPlayer()->GetScenarioId());

        return true;
    }

    static bool HandleConversationCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 conversationId = atoi((char*)args);

        Conversation* conversation = new Conversation;
        if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), conversationId, player, NULL, *player))
            delete conversation;

        handler->PSendSysMessage("HandleConversationCommand conversationId %u", conversationId);

        return true;
    }

    static bool HandleCastActionCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 spellID = atoi((char*)args);

        std::list<AreaTrigger*> list;
        player->GetAreaObjectList(list, spellID);
        if(!list.empty())
        {
            for (std::list<AreaTrigger*>::iterator itr = list.begin(); itr != list.end(); ++itr)
            {
                if(AreaTrigger* areaObj = (*itr))
                    areaObj->CastAction();
            }
        }

        handler->PSendSysMessage("HandleCastActionCommand spellID %u", spellID);
        return true;
    }

    static bool HandleAreaTriggerCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 AreaTriggerID = atoi((char*)args);

        AreaTrigger* areaTrigger = new AreaTrigger;
        if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), AreaTriggerID, (Unit*)player, NULL, (player->GetPosition()), (player->GetPosition()), NULL, ObjectGuid::Empty, 0))
        {
            handler->PSendSysMessage("HandleAreaTriggerCommand AreaTrigger %u Error", AreaTriggerID);
            delete areaTrigger;
        }
        else
            handler->PSendSysMessage("HandleAreaTriggerCommand AreaTrigger %u Spawn", AreaTriggerID);

        return true;
    }

    static bool HandleAddDupeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 itemId = 0;

        char const* id = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!id)
            return false;
        itemId = uint32(atol(id));

        char const* ccount = strtok(NULL, " ");

        int32 count = 1;

        if (ccount)
            count = strtol(ccount, NULL, 10);

        if (count == 0)
            count = 1;

        Player* player = handler->GetSession()->GetPlayer();

        TC_LOG_DEBUG(LOG_FILTER_GENERAL, handler->GetTrinityString(LANG_ADDITEM), itemId, count);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Subtract
        if (count < 0)
        {
            player->DestroyItemCount(itemId, -count, true, false);
            handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, handler->GetNameLink(player).c_str());
            return true;
        }

        // check space and find places
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
        Item* item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
        msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
        item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
        msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
        item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

        if (count == 0 || dest.empty())                         // can't add any
        {
            handler->SetSentErrorMessage(true);
            return false;
        }


        if (count > 0 && item)
            player->SendNewItem(item, count, false, true);

        return true;
    }

    static bool HandleBGStartCommand(ChatHandler* handler, char const* args)
    {
        bool sourceWin;

        if (!*args)
            sourceWin = 0;
        else
            sourceWin = 1;

        Player* player = handler->GetSession()->GetPlayer();
        if (Battleground* bg = player->GetBattleground())
        {
            bg->StartingEventOpenDoors();
            bg->SetStatus(STATUS_IN_PROGRESS);
            bg->SetStartDelayTime(m_messageTimer[BG_STARTING_EVENT_FOURTH]);

            // Add Spectator Data
            Map::PlayerList const& playerList = player->GetMap()->GetPlayers();

            if (!playerList.isEmpty())
                for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                    if (Player* _player = itr->getSource())
                    {
                        if (auto group = _player->GetGroup())
                            if (group->GetLeaderGUID() == _player->GetGUID())
                                sBattlegroundMgr->AddSpectatorData(_player->GetInstanceId(), _player->GetGUID());
                    }
        }
        else
            handler->PSendSysMessage("HandleBGStartCommand: You not in BG");

        return true;
    }

    static bool HandleBGFinishCommand(ChatHandler* handler, char const* args)
    {
        bool sourceWin;

        if (!*args)
            sourceWin = 0;
        else
            sourceWin = 1;

        Player* player = handler->GetSession()->GetPlayer();
        if (Battleground* bg = player->GetBattleground())
        {
            bg->SetStatus(STATUS_IN_PROGRESS);

            if (!sourceWin)
                bg->EndBattleground(player->GetBGTeam());
            else
                bg->EndBattleground(bg->GetOtherTeam(player->GetBGTeam()));
        }
        else
            handler->PSendSysMessage("HandleBGFinishCommand: You not in BG");

        return true;
    }

    static bool HandleBGSetArenaCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (Battleground* bg = player->GetBattleground())
        {
            bg->SetSkirmish(false);
            bg->SetArena(true);
            bg->SetRated(true);
        }
        else
            handler->PSendSysMessage("HandleBGSetArenaCommand: You not in BG");

        return true;
    }

    static bool HandleBGSetSkirmishCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (Battleground* bg = player->GetBattleground())
            bg->SetSkirmish(true);
        else
            handler->PSendSysMessage("HandleBGSetSkirmishCommand: You not in BG");

        return true;
    }

    static bool HandleBGSetRBGCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (Battleground* bg = player->GetBattleground())
            bg->SetRBG(true);
        else
            handler->PSendSysMessage("HandleBGSetRBGCommand: You not in BG");

        return true;
    }

    static bool HandleBGSetBGCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (Battleground* bg = player->GetBattleground())
            bg->SetBG(true);
        else
            handler->PSendSysMessage("HandleBGSetBGCommand: You not in BG");

        return true;
    }

    static bool HandlePvpRewardCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 rewardPackID = atoi((char*)args);

        player->DeliveryRewardPack(rewardPackID);

        handler->PSendSysMessage("HandleAreaTriggerCommand: rewardPackID %u", rewardPackID);

        return true;
    }

    static bool HandleAddMythicKeyCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* mapStr = strtok((char*)args, " ");
        char* levelStr = strtok(NULL, " ");
        char* affix1Str = strtok(NULL, " ");
        char* affix2Str = strtok(NULL, " ");
        char* affix3Str = strtok(NULL, " ");

        uint32 itemId = 138019;
        int32 count = 1;
        uint32 mapID = mapStr ? uint32(atol(mapStr)) : 0;
        uint32 level = levelStr ? uint32(atol(levelStr)) : 2;
        uint32 affix1 = affix1Str ? uint32(atol(affix1Str)) : 0;
        uint32 affix2 = affix2Str ? uint32(atol(affix2Str)) : 0;
        uint32 affix3 = affix3Str ? uint32(atol(affix3Str)) : 0;

        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Subtract
        if (count < 0)
        {
            playerTarget->DestroyItemCount(itemId, -count, true, false);
            handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, handler->GetNameLink(playerTarget).c_str());
            return true;
        }

        // Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        InventoryResult msg = playerTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
            count -= noSpaceForCount;

        if (count == 0 || dest.empty())                         // can't add any
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Item* item = playerTarget->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

        // remove binding (let GM give it to another player later)
        if (player == playerTarget)
            for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
                if (Item* item1 = player->GetItemByPos(itr->pos))
                    item1->SetBinding(false);

        if (!item)
            return false;

        if (MapChallengeModeEntry const* _challengeEntry = sDB2Manager.GetChallengeModeByMapID(mapID))
            item->SetModifier(ITEM_MODIFIER_CHALLENGE_ID, _challengeEntry->ID);

        item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_LEVEL, level);
        item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_1, affix1);
        item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_2, affix2);
        item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_3, affix3);

        item->SetState(ITEM_CHANGED, player);

        playerTarget->UpdateChallengeKey(item);
        playerTarget->m_challengeKeyInfo.Affix = affix1;
        playerTarget->m_challengeKeyInfo.Affix1 = affix2;
        playerTarget->m_challengeKeyInfo.Affix2 = affix3;
        playerTarget->m_challengeKeyInfo.needUpdate = false;
        playerTarget->m_challengeKeyInfo.needSave = true;

        if (count > 0 && item)
        {
            player->SendNewItem(item, count, false, true);
            if (player != playerTarget)
                playerTarget->SendNewItem(item, count, true, false);
        }

        if (noSpaceForCount > 0)
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

        return true;
    }

    static bool HandleWorldQuest(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Generate World Quest Update run");

        sQuestDataStore->GenerateWorldQuestUpdate();
        return true;
    }

    static bool HandleInvasionPointQuest(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Generate Invasion Point Update run");

        sQuestDataStore->GenerateInvasionPointUpdate();
        return true;
    }

    static bool HandleWorldQuestClear(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Clea World Quest");

        sQuestDataStore->ClearWorldQuest();
        return true;
    }

    static bool HandleWorldQuestAdd(ChatHandler* handler, char const* args)
    {
        uint32 QuestId = 0;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;
        QuestId = uint32(atol(id));

        if (!QuestId)
            return false;
        handler->PSendSysMessage("Add %u World Quest", QuestId);

        sQuestDataStore->GenerateNewWorldQuest(QuestId);
        return true;
    }

    static bool HandleDelTransmog(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;
        
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;
        
        if (target)
        {
            handler->PSendSysMessage("Player is online! Error");
            return false;
        }
        
        uint32 account = ObjectMgr::GetPlayerAccountIdByGUID(targetGuid);

        uint32 itemId = 0;

        char const* id = strtok(NULL, " ");
        if (!id)
            return false;
        itemId = uint32(atol(id));

        if (!itemId)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        for (uint32 transmog : sDB2Manager.GetAllTransmogsByItemId(itemId))
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_TRANSMOG);
            stmt->setUInt32(0, account);
            stmt->setUInt32(1, transmog);
            trans->Append(stmt);
        }
        CharacterDatabase.CommitTransaction(trans);
        handler->PSendSysMessage("Deleted successfully!");
        return true;
    }

    static bool HandleSendSpellScene(ChatHandler* handler, char const* args)
    {
        uint32 SceneId = 0;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;
        SceneId = uint32(atol(id));

        if (!SceneId)
            return false;
        handler->PSendSysMessage("SendSpellScene %u", SceneId);

        Player* player = handler->GetSession()->GetPlayer();
        Position pos;
        player->GetPosition(&pos);
        player->SendSpellScene(SceneId, nullptr, true, &pos);
        return true;
    }

};

void AddSC_misc_commandscript()
{
    new misc_commandscript();
}
