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
Name: reload_commandscript
%Complete: 100
Comment: All reload related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "TicketMgr.h"
#include "MapManager.h"
#include "DisableMgr.h"
#include "LFGMgr.h"
#include "AuctionHouseMgr.h"
#include "CreatureTextMgr.h"
#include "SmartAI.h"
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "Chat.h"
#include "WaypointManager.h"
#include "WardenMgr.h"
#include "ScriptSystem.h"
#include "GuildMgr.h"
#include "WordFilterMgr.h"
#include "BattlegroundMgr.h"
#include "CharacterData.h"
#include "AreaTriggerData.h"
#include "ConversationData.h"
#include "QuestData.h"
#include "GossipData.h"
#include "ScriptsData.h"

class reload_commandscript : public CommandScript
{
public:
    reload_commandscript() : CommandScript("reload_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> reloadAllCommandTable =
        {
            { "achievement", SEC_ADMINISTRATOR,  true,  &HandleReloadAllAchievementCommand, ""},
            { "area",       SEC_ADMINISTRATOR,  true,  &HandleReloadAllAreaCommand,       ""},
            { "gossips",    SEC_ADMINISTRATOR,  true,  &HandleReloadAllGossipsCommand,    ""},
            { "item",       SEC_ADMINISTRATOR,  true,  &HandleReloadAllItemCommand,       ""},
            { "locales",    SEC_ADMINISTRATOR,  true,  &HandleReloadAllLocalesCommand,    ""},
            { "loot",       SEC_ADMINISTRATOR,  true,  &HandleReloadAllLootCommand,       ""},
            { "npc",        SEC_ADMINISTRATOR,  true,  &HandleReloadAllNpcCommand,        ""},
            { "quest",      SEC_ADMINISTRATOR,  true,  &HandleReloadAllQuestCommand,      ""},
            { "scripts",    SEC_ADMINISTRATOR,  true,  &HandleReloadAllScriptsCommand,    ""},
            { "spell",      SEC_ADMINISTRATOR,  true,  &HandleReloadAllSpellCommand,      ""},
            { "",           SEC_ADMINISTRATOR,  true,  &HandleReloadAllCommand,           ""}
        };
        static std::vector<ChatCommand> reloadCommandTable =
        {
            { "auctions",                     SEC_ADMINISTRATOR, true,  &HandleReloadAuctionsCommand,                   ""},
            { "access_requirement",           SEC_ADMINISTRATOR, true,  &HandleReloadAccessRequirementCommand,          ""},
            { "achievement_criteria_data",    SEC_ADMINISTRATOR, true,  &HandleReloadAchievementCriteriaDataCommand,    ""},
            { "achievement_reward",           SEC_ADMINISTRATOR, true,  &HandleReloadAchievementRewardCommand,          ""},
            { "all",                          SEC_ADMINISTRATOR, true,  NULL,                          "", reloadAllCommandTable },
            { "areatrigger_data",             SEC_ADMINISTRATOR, true,  &HandleReloadAreaTriggerDataCommand,            ""},
            { "areatrigger_questender",       SEC_ADMINISTRATOR, true,  &HandleReloadQuestAreaTriggersCommand,          ""},
            { "areatrigger_tavern",           SEC_ADMINISTRATOR, true,  &HandleReloadAreaTriggerTavernCommand,          ""},
            { "areatrigger_teleport",         SEC_ADMINISTRATOR, true,  &HandleReloadAreaTriggerTeleportCommand,        ""},
            { "bad_word",                     SEC_ADMINISTRATOR, true,  &HandleReloadBadWordCommand,                    ""},
            { "bad_senteces",                 SEC_ADMINISTRATOR, true,  &HandleReloadBadSentencesCommand,               ""},
            { "autobroadcast",                SEC_ADMINISTRATOR, true,  &HandleReloadAutobroadcastCommand,              ""},
            { "command",                      SEC_ADMINISTRATOR, true,  &HandleReloadCommandCommand,                    ""},
            { "conditions",                   SEC_ADMINISTRATOR, true,  &HandleReloadConditions,                        ""},
            { "config",                       SEC_ADMINISTRATOR, true,  &HandleReloadConfigCommand,                     ""},
            { "conversation",                 SEC_ADMINISTRATOR, true,  &HandleReloadConversation,                      ""},
            { "creature_area",                SEC_ADMINISTRATOR, true,  &HandleReloadCreatureArea,                      ""},
            { "creature_text",                SEC_ADMINISTRATOR, true,  &HandleReloadCreatureText,                      ""},
            { "creature_questender",          SEC_ADMINISTRATOR, true,  &HandleReloadCreatureQuestInvRelationsCommand,  ""},
            { "creature_linked_respawn",      SEC_GAMEMASTER,    true,  &HandleReloadLinkedRespawnCommand,              ""},
            { "creature_loot_template",       SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesCreatureCommand,      ""},
            { "creature_onkill_reputation",   SEC_ADMINISTRATOR, true,  &HandleReloadOnKillReputationCommand,           ""},
            { "creature_queststarter",        SEC_ADMINISTRATOR, true,  &HandleReloadCreatureQuestRelationsCommand,     ""},
            { "creature_template",            SEC_ADMINISTRATOR, true,  &HandleReloadCreatureTemplateCommand,           ""},
            { "creature_template_outfits",    SEC_ADMINISTRATOR, true,  &HandleReloadCreatureTemplateOutfitsCommand,    ""},
            { "deathmatch_products",          SEC_ADMINISTRATOR, true,  &HandleReloadDeathMatchProductsCommand,    ""},
            //{ "db_script_string",             SEC_ADMINISTRATOR, true,  &HandleReloadDbScriptStringCommand,            ""},
            { "disables",                     SEC_ADMINISTRATOR, true,  &HandleReloadDisablesCommand,                   ""},
            { "disenchant_loot_template",     SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesDisenchantCommand,    ""},
            { "event_scripts",                SEC_ADMINISTRATOR, true,  &HandleReloadEventScriptsCommand,               ""},
            { "fishing_loot_template",        SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesFishingCommand,       ""},
            { "game_graveyard_zone",          SEC_ADMINISTRATOR, true,  &HandleReloadGameGraveyardZoneCommand,          ""},
            { "game_tele",                    SEC_ADMINISTRATOR, true,  &HandleReloadGameTeleCommand,                   ""},
            { "gameobject_questender",        SEC_ADMINISTRATOR, true,  &HandleReloadGOQuestInvRelationsCommand,        ""},
            { "gameobject_loot_template",     SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesGameobjectCommand,    ""},
            { "gameobject_queststarter",      SEC_ADMINISTRATOR, true,  &HandleReloadGOQuestRelationsCommand,           ""},
            { "gameobject_scripts",           SEC_ADMINISTRATOR, true,  &HandleReloadGameObjectScriptsCommand,          ""},
            { "gm_tickets",                   SEC_ADMINISTRATOR, true,  &HandleReloadGMTicketsCommand,                  ""},
            { "gossip_menu",                  SEC_ADMINISTRATOR, true,  &HandleReloadGossipMenuCommand,                 ""},
            { "gossip_menu_option",           SEC_ADMINISTRATOR, true,  &HandleReloadGossipMenuOptionCommand,           ""},
            { "guild_rewards",                SEC_ADMINISTRATOR, true,  &HandleReloadGuildRewardsCommand,               ""},
            { "item_enchantment_template",    SEC_ADMINISTRATOR, true,  &HandleReloadItemEnchantementsCommand,          ""},
            { "item_loot_template",           SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesItemCommand,          ""},
            { "lfg_dungeon_rewards",          SEC_ADMINISTRATOR, true,  &HandleReloadLfgRewardsCommand,                 ""},
            { "letter_analog",                SEC_ADMINISTRATOR, true,  &HandleReloadLetterAnalogCommand,               ""},
            { "locales_achievement_reward",   SEC_ADMINISTRATOR, true,  &HandleReloadLocalesAchievementRewardCommand,   ""},
            { "locales_creature",             SEC_ADMINISTRATOR, true,  &HandleReloadLocalesCreatureCommand,            ""},
            { "locales_gameobject",           SEC_ADMINISTRATOR, true,  &HandleReloadLocalesGameobjectCommand,          ""},
            { "locales_gossip_menu_option",   SEC_ADMINISTRATOR, true,  &HandleReloadLocalesGossipMenuOptionCommand,    ""},
            { "locales_page_text",            SEC_ADMINISTRATOR, true,  &HandleReloadLocalesPageTextCommand,            ""},
            { "locales_points_of_interest",   SEC_ADMINISTRATOR, true,  &HandleReloadLocalesPointsOfInterestCommand,    ""},
            { "locales_quest",                SEC_ADMINISTRATOR, true,  &HandleReloadLocalesQuestCommand,               ""},
            { "mail_level_reward",            SEC_ADMINISTRATOR, true,  &HandleReloadMailLevelRewardCommand,            ""},
            { "mail_loot_template",           SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesMailCommand,          ""},
            { "milling_loot_template",        SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesMillingCommand,       ""},
            { "npc_spellclick_spells",        SEC_ADMINISTRATOR, true,  &HandleReloadSpellClickSpellsCommand,           ""},
            { "npc_trainer",                  SEC_ADMINISTRATOR, true,  &HandleReloadNpcTrainerCommand,                 ""},
            { "npc_vendor",                   SEC_ADMINISTRATOR, true,  &HandleReloadNpcVendorCommand,                  ""},
            { "page_text",                    SEC_ADMINISTRATOR, true,  &HandleReloadPageTextsCommand,                  ""},
            { "phasedefinitions",             SEC_ADMINISTRATOR, true,  &HandleReloadPhaseDefinitionsCommand,           ""},
            { "pickpocketing_loot_template",  SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesPickpocketingCommand, ""},
            { "points_of_interest",           SEC_ADMINISTRATOR, true,  &HandleReloadPointsOfInterestCommand,           ""},
            { "prospecting_loot_template",    SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesProspectingCommand,   ""},
            { "quest_end_scripts",            SEC_ADMINISTRATOR, true,  &HandleReloadQuestEndScriptsCommand,            ""},
            { "quest_poi",                    SEC_ADMINISTRATOR, true,  &HandleReloadQuestPOICommand,                   ""},
            { "quest_start_scripts",          SEC_ADMINISTRATOR, true,  &HandleReloadQuestStartScriptsCommand,          ""},
            { "quest_template",               SEC_ADMINISTRATOR, true,  &HandleReloadQuestTemplateCommand,              ""},
            { "reference_loot_template",      SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesReferenceCommand,     ""},
            { "reserved_name",                SEC_ADMINISTRATOR, true,  &HandleReloadReservedNameCommand,               ""},
            { "reputation_reward_rate",       SEC_ADMINISTRATOR, true,  &HandleReloadReputationRewardRateCommand,       ""},
            { "reputation_spillover_template", SEC_ADMINISTRATOR, true,  &HandleReloadReputationRewardRateCommand,       ""},
            { "script_waypoint",              SEC_ADMINISTRATOR, true,  &HandleReloadScriptWaypointCommand,             ""},
            { "skill_discovery_template",     SEC_ADMINISTRATOR, true,  &HandleReloadSkillDiscoveryTemplateCommand,     ""},
            { "skill_extra_item_template",    SEC_ADMINISTRATOR, true,  &HandleReloadSkillExtraItemTemplateCommand,     ""},
            { "skill_fishing_base_level",     SEC_ADMINISTRATOR, true,  &HandleReloadSkillFishingBaseLevelCommand,      ""},
            { "skinning_loot_template",       SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesSkinningCommand,      ""},
            { "smart_scripts",                SEC_ADMINISTRATOR, true,  &HandleReloadSmartScripts,                      ""},
            { "spell_required",               SEC_ADMINISTRATOR, true,  &HandleReloadSpellRequiredCommand,              ""},
            { "spell_area",                   SEC_ADMINISTRATOR, true,  &HandleReloadSpellAreaCommand,                  ""},
            { "spell_bonus_data",             SEC_ADMINISTRATOR, true,  &HandleReloadSpellBonusesCommand,               ""},
            { "spell_group",                  SEC_ADMINISTRATOR, true,  &HandleReloadSpellGroupsCommand,                ""},
            { "spell_learn_spell",            SEC_ADMINISTRATOR, true,  &HandleReloadSpellLearnSpellCommand,            ""},
            { "spell_loot_template",          SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesSpellCommand,         ""},
            { "spell_linked_spell",           SEC_ADMINISTRATOR, true,  &HandleReloadSpellLinkedSpellCommand,           ""},
            { "spell_pet_auras",              SEC_ADMINISTRATOR, true,  &HandleReloadSpellPetAurasCommand,              ""},
            { "spell_proc_event",             SEC_ADMINISTRATOR, true,  &HandleReloadSpellProcEventCommand,             ""},
            { "spell_proc",                   SEC_ADMINISTRATOR, true,  &HandleReloadSpellProcsCommand,                 ""},
            { "spell_scripts",                SEC_ADMINISTRATOR, true,  &HandleReloadSpellScriptsCommand,               ""},
            { "spell_target_position",        SEC_ADMINISTRATOR, true,  &HandleReloadSpellTargetPositionCommand,        ""},
            { "spell_threats",                SEC_ADMINISTRATOR, true,  &HandleReloadSpellThreatsCommand,               ""},
            { "spell_group_stack_rules",      SEC_ADMINISTRATOR, true,  &HandleReloadSpellGroupStackRulesCommand,       ""},
            { "trinity_string",               SEC_ADMINISTRATOR, true,  &HandleReloadTrinityStringCommand,              ""},
            { "warden_data",                  SEC_ADMINISTRATOR, true,  &HandleReloadWardenDataCommand,                 ""},
            { "waypoint_scripts",             SEC_ADMINISTRATOR, true,  &HandleReloadWpScriptsCommand,                  ""},
            { "waypoint_data",                SEC_ADMINISTRATOR, true,  &HandleReloadWpCommand,                         ""},
            { "vehicle_template_accessory",   SEC_ADMINISTRATOR, true,  &HandleReloadVehicleTemplateAccessoryCommand,   ""},
            { "world_visible_distance",       SEC_ADMINISTRATOR, true,  &HandleReloadWorldVisibleDistanceCommand,       ""},
            { "creature_summon_groups",       SEC_ADMINISTRATOR, true,  &HandleReloadSummonGroups,                      ""},
            { "donate_vendor",                SEC_ADMINISTRATOR, true,  &HandleReloadNpcDonateVendorCommand,            ""},
            { "world_rate_info",              SEC_ADMINISTRATOR, true,  &HandleReloadWorldRateInfoCommand,              ""},
            { "scenario_data",                SEC_ADMINISTRATOR, true,  &HandleReloadScenarioDataCommand,               ""},
            { "pvp_reward",                   SEC_ADMINISTRATOR, true,  &HandleReloadPvpRewardCommand,                  ""},
            { "world_loot_template",          SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesWorldCommand,         ""},
            { "zone_loot_template",           SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesZoneCommand,          ""},
            { "luck_loot_template",           SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesLuckCommand,         ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "reload",         SEC_ADMINISTRATOR,  true,  NULL,                 "", reloadCommandTable }
        };
        return commandTable;
    }

    //reload commands
    static bool HandleReloadGMTicketsCommand(ChatHandler* /*handler*/, const char* /*args*/)
    {
        sTicketMgr->LoadTickets();
        return true;
    }

    static bool HandleReloadAllCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadSkillFishingBaseLevelCommand(handler, "");

        HandleReloadAllAchievementCommand(handler, "");
        HandleReloadAllAreaCommand(handler, "");
        HandleReloadAllLootCommand(handler, "");
        HandleReloadAllNpcCommand(handler, "");
        HandleReloadAllQuestCommand(handler, "");
        HandleReloadAllSpellCommand(handler, "");
        HandleReloadAllItemCommand(handler, "");
        HandleReloadAllGossipsCommand(handler, "");
        HandleReloadAllLocalesCommand(handler, "");

        HandleReloadAccessRequirementCommand(handler, "");
        HandleReloadMailLevelRewardCommand(handler, "");
        HandleReloadCommandCommand(handler, "");
        HandleReloadReservedNameCommand(handler, "");
        HandleReloadTrinityStringCommand(handler, "");
        HandleReloadGameTeleCommand(handler, "");

        HandleReloadVehicleTemplateAccessoryCommand(handler, "");

        HandleReloadAutobroadcastCommand(handler, "");
        HandleReloadBadWordCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllAchievementCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadAchievementCriteriaDataCommand(handler, "");
        HandleReloadAchievementRewardCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllAreaCommand(ChatHandler* handler, const char* /*args*/)
    {
        //HandleReloadQuestAreaTriggersCommand(handler, ""); -- reloaded in HandleReloadAllQuestCommand
        HandleReloadAreaTriggerTeleportCommand(handler, "");
        HandleReloadAreaTriggerTavernCommand(handler, "");
        HandleReloadGameGraveyardZoneCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllLootCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables...");
        LoadLootTables();
        handler->SendGlobalGMSysMessage("DB tables `*_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadAllNpcCommand(ChatHandler* handler, const char* args)
    {
        if (*args != 'a')                                          // will be reloaded from all_gossips
        HandleReloadNpcTrainerCommand(handler, "a");
        HandleReloadNpcVendorCommand(handler, "a");
        HandleReloadPointsOfInterestCommand(handler, "a");
        HandleReloadSpellClickSpellsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllQuestCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadQuestAreaTriggersCommand(handler, "a");
        HandleReloadQuestPOICommand(handler, "a");
        HandleReloadQuestTemplateCommand(handler, "a");

        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Quests Relations...");
        sQuestDataStore->LoadQuestRelations();
        handler->SendGlobalGMSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
        return true;
    }

    static bool HandleReloadAllScriptsCommand(ChatHandler* handler, const char* /*args*/)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->PSendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Scripts...");
        HandleReloadGameObjectScriptsCommand(handler, "a");
        HandleReloadEventScriptsCommand(handler, "a");
        HandleReloadQuestEndScriptsCommand(handler, "a");
        HandleReloadQuestStartScriptsCommand(handler, "a");
        HandleReloadSpellScriptsCommand(handler, "a");
        handler->SendGlobalGMSysMessage("DB tables `*_scripts` reloaded.");
        HandleReloadDbScriptStringCommand(handler, "a");
        HandleReloadWpScriptsCommand(handler, "a");
        HandleReloadWpCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadSkillDiscoveryTemplateCommand(handler, "a");
        HandleReloadSkillExtraItemTemplateCommand(handler, "a");
        HandleReloadSpellRequiredCommand(handler, "a");
        HandleReloadSpellAreaCommand(handler, "a");
        HandleReloadSpellGroupsCommand(handler, "a");
        HandleReloadSpellLearnSpellCommand(handler, "a");
        HandleReloadSpellLinkedSpellCommand(handler, "a");
        HandleReloadSpellProcEventCommand(handler, "a");
        HandleReloadSpellProcsCommand(handler, "a");
        HandleReloadSpellBonusesCommand(handler, "a");
        HandleReloadSpellTargetPositionCommand(handler, "a");
        HandleReloadSpellThreatsCommand(handler, "a");
        HandleReloadSpellGroupStackRulesCommand(handler, "a");
        HandleReloadSpellPetAurasCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllGossipsCommand(ChatHandler* handler, const char* args)
    {
        HandleReloadGossipMenuCommand(handler, "a");
        HandleReloadGossipMenuOptionCommand(handler, "a");
        if (*args != 'a')                                          // already reload from all_scripts
        HandleReloadPointsOfInterestCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllItemCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadPageTextsCommand(handler, "a");
        HandleReloadItemEnchantementsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllLocalesCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadLocalesAchievementRewardCommand(handler, "a");
        HandleReloadLocalesCreatureCommand(handler, "a");
        HandleReloadLocalesGameobjectCommand(handler, "a");
        HandleReloadLocalesGossipMenuOptionCommand(handler, "a");
        HandleReloadLocalesPageTextCommand(handler, "a");
        HandleReloadLocalesPointsOfInterestCommand(handler, "a");
        HandleReloadLocalesQuestCommand(handler, "a");
        return true;
    }

    static bool HandleReloadConfigCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading config settings...");
        sLog->_checkLock = true;
        sWorld->LoadConfigSettings(true);
        sMapMgr->InitializeVisibilityDistanceInfo();
        sLog->_checkLock = false;
        handler->SendGlobalGMSysMessage("World config settings reloaded.");
        return true;
    }

    static bool HandleReloadAccessRequirementCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Access Requirement definitions...");
        sObjectMgr->LoadAccessRequirements();
        handler->SendGlobalGMSysMessage("DB table `access_requirement` reloaded.");
        return true;
    }

    static bool HandleReloadAchievementCriteriaDataCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Additional Achievement Criteria Data...");
        sAchievementMgr->LoadAchievementCriteriaData();
        handler->SendGlobalGMSysMessage("DB table `achievement_criteria_data` reloaded.");
        return true;
    }

    static bool HandleReloadAchievementRewardCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Achievement Reward Data...");
        sAchievementMgr->LoadRewards();
        handler->SendGlobalGMSysMessage("DB table `achievement_reward` reloaded.");
        return true;
    }

    static bool HandleReloadAreaTriggerTavernCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Tavern Area Triggers...");
        sAreaTriggerDataStore->LoadTavernAreaTriggers();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_tavern` reloaded.");
        return true;
    }

    static bool HandleReloadAreaTriggerTeleportCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading AreaTrigger teleport definitions...");
        sAreaTriggerDataStore->LoadAreaTriggerTeleports();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_teleport` reloaded.");
        return true;
    }

    static bool HandleReloadAutobroadcastCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Autobroadcasts...");
        sWorld->LoadAutobroadcasts();
        handler->SendGlobalGMSysMessage("DB table `autobroadcast` reloaded.");
        return true;
    }

    static bool HandleReloadCommandCommand(ChatHandler* handler, const char* /*args*/)
    {
        handler->SetLoadCommandTable(true);
        handler->SendGlobalGMSysMessage("DB table `command` will be reloaded at next chat command use.");
        return true;
    }

    static bool HandleReloadOnKillReputationCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading creature award reputation definitions...");
        sObjectMgr->LoadReputationOnKill();
        handler->SendGlobalGMSysMessage("DB table `creature_onkill_reputation` reloaded.");
        return true;
    }

    static bool HandleReloadCreatureTemplateCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Tokenizer entries(std::string(args), ' ');

        for (Tokenizer::const_iterator itr = entries.begin(); itr != entries.end(); ++itr)
        {
            uint32 entry = uint32(atoi(*itr));

            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_TEMPLATE);
            stmt->setUInt32(0, entry);
            PreparedQueryResult result = WorldDatabase.Query(stmt);

            if (!result)
            {
                handler->PSendSysMessage(LANG_COMMAND_CREATURETEMPLATE_NOTFOUND, entry);
                continue;
            }

            CreatureTemplate* cInfo = const_cast<CreatureTemplate*>(sObjectMgr->GetCreatureTemplate(entry));
            if (!cInfo)
            {
                handler->PSendSysMessage(LANG_COMMAND_CREATURESTORAGE_NOTFOUND, entry);
                continue;
            }

            TC_LOG_INFO(LOG_FILTER_GENERAL, "Reloading creature template entry %u", entry);

            Field* fields = result->Fetch();

            uint8 index = 0;
            cInfo->GossipMenuId       = fields[index++].GetUInt32();
            cInfo->minlevel           = fields[index++].GetUInt8();
            cInfo->maxlevel           = fields[index++].GetUInt8();
            cInfo->faction            = fields[index++].GetUInt16();
            cInfo->npcflag            = fields[index++].GetUInt32();
            cInfo->speed_walk         = fields[index++].GetFloat();
            cInfo->speed_run          = fields[index++].GetFloat();
            cInfo->scale              = fields[index++].GetFloat();
            cInfo->dmgschool          = fields[index++].GetUInt8();
            cInfo->dmg_multiplier     = fields[index++].GetFloat();
            cInfo->baseattacktime     = fields[index++].GetUInt32();
            cInfo->rangeattacktime    = fields[index++].GetUInt32();
            cInfo->unit_class         = fields[index++].GetUInt8();
            cInfo->unit_flags         = fields[index++].GetUInt32();
            cInfo->unit_flags2        = fields[index++].GetUInt32();
            cInfo->dynamicflags       = fields[index++].GetUInt32();
            cInfo->trainer_type       = fields[index++].GetUInt8();
            cInfo->trainer_spell      = fields[index++].GetUInt32();
            cInfo->trainer_class      = fields[index++].GetUInt8();
            cInfo->trainer_race       = fields[index++].GetUInt8();
            cInfo->lootid             = fields[index++].GetUInt32();
            cInfo->pickpocketLootId   = fields[index++].GetUInt32();
            cInfo->SkinLootId         = fields[index++].GetUInt32();

            for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
                cInfo->resistance[i] = fields[index++].GetUInt16();

            cInfo->spells[0]          = fields[index++].GetUInt32();
            cInfo->spells[1]          = fields[index++].GetUInt32();
            cInfo->spells[2]          = fields[index++].GetUInt32();
            cInfo->spells[3]          = fields[index++].GetUInt32();
            cInfo->spells[4]          = fields[index++].GetUInt32();
            cInfo->spells[5]          = fields[index++].GetUInt32();
            cInfo->spells[6]          = fields[index++].GetUInt32();
            cInfo->spells[7]          = fields[index++].GetUInt32();
            cInfo->PetSpellDataId     = fields[index++].GetUInt32();
            cInfo->VehicleId          = fields[index++].GetUInt32();
            cInfo->mingold            = fields[index++].GetUInt32();
            cInfo->maxgold            = fields[index++].GetUInt32();
            cInfo->AIName             = fields[index++].GetString();
            cInfo->MovementType       = fields[index++].GetUInt8();
            cInfo->InhabitType        = fields[index++].GetUInt8();
            cInfo->HoverHeight        = fields[index++].GetFloat();
            cInfo->ModManaExtra       = fields[index++].GetFloat();
            cInfo->ModArmor           = fields[index++].GetFloat();
            cInfo->RegenHealth        = fields[index++].GetBool();
            cInfo->MechanicImmuneMask = fields[index++].GetUInt32();
            cInfo->flags_extra        = fields[index++].GetUInt32();
            cInfo->ScriptID           = sObjectMgr->GetScriptId(fields[index++].GetCString());

            sObjectMgr->CheckCreatureTemplate(cInfo);
        }

        handler->SendGlobalGMSysMessage("Creature template reloaded.");
        return true;
    }

    static bool HandleReloadCreatureTemplateOutfitsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading Creature Outfits... (`creature_template_outfits`)");
        sObjectMgr->LoadCreatureOutfits();
        handler->SendGlobalGMSysMessage("DB table `creature_template_outfits` reloaded.");
        return true;
    }
    static bool HandleReloadCreatureQuestRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading Quests Relations... (`creature_queststarter`)");
        sQuestDataStore->LoadCreatureQuestRelations();
        handler->SendGlobalGMSysMessage("DB table `creature_queststarter` (creature quest givers) reloaded.");
        return true;
    }

    static bool HandleReloadLinkedRespawnCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading Linked Respawns... (`creature_linked_respawn`)");
        sObjectMgr->LoadLinkedRespawn();
        handler->SendGlobalGMSysMessage("DB table `creature_linked_respawn` (creature linked respawns) reloaded.");
        return true;
    }

    static bool HandleReloadCreatureQuestInvRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading Quests Relations... (`creature_questender`)");
        sQuestDataStore->LoadCreatureInvolvedRelations();
        handler->SendGlobalGMSysMessage("DB table `creature_questender` (creature quest takers) reloaded.");
        return true;
    }

    static bool HandleReloadGossipMenuCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `gossip_menu` Table!");
        sGossipDataStore->LoadGossipMenu();
        handler->SendGlobalGMSysMessage("DB table `gossip_menu` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadGossipMenuOptionCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `gossip_menu_option` Table!");
        sGossipDataStore->LoadGossipMenuItems();
        handler->SendGlobalGMSysMessage("DB table `gossip_menu_option` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadGuildRewardsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `guild_rewards` Table!");
        sGuildMgr->LoadGuildRewards();
        handler->SendGlobalGMSysMessage("DB table `guild_rewards` reloaded.");
        return true;
    }

    static bool HandleReloadGOQuestRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading Quests Relations... (`gameobject_queststarter`)");
        sQuestDataStore->LoadGameobjectQuestRelations();
        handler->SendGlobalGMSysMessage("DB table `gameobject_queststarter` (gameobject quest givers) reloaded.");
        return true;
    }

    static bool HandleReloadGOQuestInvRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading Quests Relations... (`gameobject_questender`)");
        sQuestDataStore->LoadGameobjectInvolvedRelations();
        handler->SendGlobalGMSysMessage("DB table `gameobject_questender` (gameobject quest takers) reloaded.");
        return true;
    }

    static bool HandleReloadQuestAreaTriggersCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Quest Area Triggers...");
        sAreaTriggerDataStore->LoadQuestAreaTriggers();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_questender` (quest area triggers) reloaded.");
        return true;
    }

    static bool HandleReloadQuestTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Quest Templates...");
        sQuestDataStore->LoadQuests();
        handler->SendGlobalGMSysMessage("DB table `quest_template` (quest definitions) reloaded.");

        /// dependent also from `gameobject` but this table not reloaded anyway
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading GameObjects for quests...");
        sQuestDataStore->LoadGameObjectForQuests();
        handler->SendGlobalGMSysMessage("Data GameObjects for quests reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesCreatureCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`creature_loot_template`)");
        LoadLootTemplates_Creature();
        LootTemplates_Creature.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `creature_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesDisenchantCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`disenchant_loot_template`)");
        LoadLootTemplates_Disenchant();
        LootTemplates_Disenchant.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `disenchant_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesFishingCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`fishing_loot_template`)");
        LoadLootTemplates_Fishing();
        LootTemplates_Fishing.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `fishing_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesGameobjectCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`gameobject_loot_template`)");
        LoadLootTemplates_Gameobject();
        LootTemplates_Gameobject.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `gameobject_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesItemCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`item_loot_template`)");
        LoadLootTemplates_Item();
        LootTemplates_Item.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `item_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesMillingCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`milling_loot_template`)");
        LoadLootTemplates_Milling();
        LootTemplates_Milling.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `milling_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesPickpocketingCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`pickpocketing_loot_template`)");
        LoadLootTemplates_Pickpocketing();
        LootTemplates_Pickpocketing.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `pickpocketing_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesProspectingCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`prospecting_loot_template`)");
        LoadLootTemplates_Prospecting();
        LootTemplates_Prospecting.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `prospecting_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesMailCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`mail_loot_template`)");
        LoadLootTemplates_Mail();
        LootTemplates_Mail.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `mail_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesReferenceCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`reference_loot_template`)");
        LoadLootTemplates_Reference();
        handler->SendGlobalGMSysMessage("DB table `reference_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesSkinningCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`skinning_loot_template`)");
        LoadLootTemplates_Skinning();
        LootTemplates_Skinning.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `skinning_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`spell_loot_template`)");
        LoadLootTemplates_Spell();
        LootTemplates_Spell.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `spell_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesWorldCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`world_loot_template`)");
        LoadLootTemplates_World();
        LootTemplates_World.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `world_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesZoneCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`zone_loot_template`)");
        LoadLootTemplates_World();
        LootTemplates_Zone.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `zone_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesLuckCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Loot Tables... (`luck_loot_template`)");
        LoadLootTemplates_World();
        LootTemplates_Luck.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `luck_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadTrinityStringCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading trinity_string Table!");
        sObjectMgr->LoadTrinityStrings();
        handler->SendGlobalGMSysMessage("DB table `trinity_string` reloaded.");
        return true;
    }

    static bool HandleReloadWardenDataCommand(ChatHandler* handler, const char* /*args*/)
    {
        // TODO: remove later
        if (!sWorld->getBoolConfig(CONFIG_WARDEN_ENABLED))
        {
            handler->SendSysMessage("Warden system disabled by config - reloading warden_action skipped.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading warden_overrides Table!");
        _wardenMgr->LoadWardenOverrides();
        handler->SendGlobalGMSysMessage("DB table `warden_overrides` reloaded.");
        return true;
    }

    static bool HandleReloadNpcTrainerCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `npc_trainer` Table!");
        sObjectMgr->LoadTrainerSpell();
        handler->SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
        return true;
    }

    static bool HandleReloadNpcVendorCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `npc_vendor` Table!");
        sObjectMgr->LoadVendors();
        handler->SendGlobalGMSysMessage("DB table `npc_vendor` reloaded.");
        return true;
    }

    static bool HandleReloadPointsOfInterestCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `points_of_interest` Table!");
        sQuestDataStore->LoadPointsOfInterest();
        handler->SendGlobalGMSysMessage("DB table `points_of_interest` reloaded.");
        return true;
    }

    static bool HandleReloadQuestPOICommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Quest POI ..." );
        sQuestDataStore->LoadQuestPOI();
        handler->SendGlobalGMSysMessage("DB Table `quest_poi` and `quest_poi_points` reloaded.");
        return true;
    }

    static bool HandleReloadSpellClickSpellsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `npc_spellclick_spells` Table!");
        sObjectMgr->LoadNPCSpellClickSpells();
        handler->SendGlobalGMSysMessage("DB table `npc_spellclick_spells` reloaded.");
        return true;
    }

    static bool HandleReloadReservedNameCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading ReservedNames... (`reserved_name`)");
        sCharacterDataStore->LoadReservedPlayersNames();
        handler->SendGlobalGMSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
        return true;
    }

    static bool HandleReloadReputationRewardRateCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `reputation_reward_rate` Table!" );
        sObjectMgr->LoadReputationRewardRate();
        handler->SendGlobalSysMessage("DB table `reputation_reward_rate` reloaded.");
        return true;
    }

    static bool HandleReloadReputationSpilloverTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `reputation_spillover_template` Table!" );
        sObjectMgr->LoadReputationSpilloverTemplate();
        handler->SendGlobalSysMessage("DB table `reputation_spillover_template` reloaded.");
        return true;
    }

    static bool HandleReloadSkillDiscoveryTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Skill Discovery Table...");
        LoadSkillDiscoveryTable();
        handler->SendGlobalGMSysMessage("DB table `skill_discovery_template` (recipes discovered at crafting) reloaded.");
        return true;
    }

    static bool HandleReloadSkillExtraItemTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Skill Extra Item Table...");
        LoadSkillExtraItemTable();
        handler->SendGlobalGMSysMessage("DB table `skill_extra_item_template` (extra item creation when crafting) reloaded.");
        return true;
    }

    static bool HandleReloadSkillFishingBaseLevelCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Skill Fishing base level requirements...");
        sObjectMgr->LoadFishingBaseSkillLevel();
        handler->SendGlobalGMSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
        return true;
    }

    static bool HandleReloadSpellAreaCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading SpellArea Data...");
        sSpellMgr->LoadSpellAreas();
        handler->SendGlobalGMSysMessage("DB table `spell_area` (spell dependences from area/quest/auras state) reloaded.");
        return true;
    }

    static bool HandleReloadSpellRequiredCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Required Data... ");
        sSpellMgr->LoadSpellRequired();
        handler->SendGlobalGMSysMessage("DB table `spell_required` reloaded.");
        return true;
    }

    static bool HandleReloadSpellGroupsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Groups...");
        sSpellMgr->LoadSpellGroups();
        handler->SendGlobalGMSysMessage("DB table `spell_group` (spell groups) reloaded.");
        return true;
    }

    static bool HandleReloadSpellLearnSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Learn Spells...");
        sSpellMgr->LoadSpellLearnSpells();
        handler->SendGlobalGMSysMessage("DB table `spell_learn_spell` reloaded.");
        return true;
    }

    static bool HandleReloadSpellLinkedSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Linked Spells...");
        sSpellMgr->LoadSpellLinked();
        sSpellMgr->LoadTalentSpellLinked();
        sSpellMgr->LoadSpellConcatenateAura();
        handler->SendGlobalGMSysMessage("DB table `spell_linked_spell` reloaded.");
        return true;
    }

    static bool HandleReloadSpellProcEventCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Proc Event conditions...");
        sSpellMgr->LoadSpellProcEvents();
        sSpellMgr->LoadSpellPrcoCheck();
        sSpellMgr->LoadSpellTriggered();
        sSpellMgr->LoadSpellVisual();
        sSpellMgr->LoadSpellPendingCast();
        handler->SendGlobalGMSysMessage("DB table `spell_proc_event` (spell proc trigger requirements) reloaded.");
        return true;
    }

    static bool HandleReloadSpellProcsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Proc conditions and data...");
        sSpellMgr->LoadSpellProcs();
        handler->SendGlobalGMSysMessage("DB table `spell_proc` (spell proc conditions and data) reloaded.");
        return true;
    }

    static bool HandleReloadSpellBonusesCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Bonus Data...");
        sSpellMgr->LoadSpellBonusess();
        handler->SendGlobalGMSysMessage("DB table `spell_bonus_data` (spell damage/healing coefficients) reloaded.");
        return true;
    }

    static bool HandleReloadSpellTargetPositionCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell target coordinates...");
        sSpellMgr->LoadSpellTargetPositions();
        handler->SendGlobalGMSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
        return true;
    }

    static bool HandleReloadSpellThreatsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Aggro Spells Definitions...");
        sSpellMgr->LoadSpellThreats();
        handler->SendGlobalGMSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
        return true;
    }

    static bool HandleReloadSpellGroupStackRulesCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell Group Stack Rules...");
        sSpellMgr->LoadSpellGroupStackRules();
        handler->SendGlobalGMSysMessage("DB table `spell_group_stack_rules` (spell stacking definitions) reloaded.");
        return true;
    }

    static bool HandleReloadSpellPetAurasCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Spell pet auras...");
        sSpellMgr->LoadSpellPetAuras();
        handler->SendGlobalGMSysMessage("DB table `spell_pet_auras` reloaded.");
        return true;
    }

    static bool HandleReloadPageTextsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Page Texts...");
        sObjectMgr->LoadPageTexts();
        handler->SendGlobalGMSysMessage("DB table `page_texts` reloaded.");
        return true;
    }

    static bool HandleReloadItemEnchantementsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Item Random Enchantments Table...");
        LoadRandomEnchantmentsTable();
        handler->SendGlobalGMSysMessage("DB table `item_enchantment_template` reloaded.");
        return true;
    }

    static bool HandleReloadGameObjectScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Scripts from `gameobject_scripts`...");

        sScriptDataStore->LoadGameObjectScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `gameobject_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadEventScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Scripts from `event_scripts`...");

        sScriptDataStore->LoadEventScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `event_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadWpScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Scripts from `waypoint_scripts`...");

        sScriptDataStore->LoadWaypointScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `waypoint_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadWpCommand(ChatHandler* handler, const char* args)
    {
        if (*args != 'a')
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Waypoints data from 'waypoints_data'");

        sWaypointMgr->Load();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB Table 'waypoint_data' reloaded.");

        return true;
    }

    static bool HandleReloadQuestEndScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Scripts from `quest_end_scripts`...");

        sScriptDataStore->LoadQuestEndScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `quest_end_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadQuestStartScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Scripts from `quest_start_scripts`...");

        sScriptDataStore->LoadQuestStartScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `quest_start_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadSpellScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Scripts from `spell_scripts`...");

        sScriptDataStore->LoadSpellScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `spell_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadDbScriptStringCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Script strings from `db_script_string`...");
        sScriptDataStore->LoadDbScriptStrings();
        handler->SendGlobalGMSysMessage("DB table `db_script_string` reloaded.");
        return true;
    }

    static bool HandleReloadGameGraveyardZoneCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Graveyard-zone links...");

        sObjectMgr->LoadGraveyardZones();

        handler->SendGlobalGMSysMessage("DB table `game_graveyard_zone` reloaded.");

        return true;
    }

    static bool HandleReloadGameTeleCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Game Tele coordinates...");

        sObjectMgr->LoadGameTele();

        handler->SendGlobalGMSysMessage("DB table `game_tele` reloaded.");

        return true;
    }

    static bool HandleReloadDisablesCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading disables table...");
        DisableMgr::LoadDisables();
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Checking quest disables...");
        DisableMgr::CheckQuestDisables();
        handler->SendGlobalGMSysMessage("DB table `disables` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesAchievementRewardCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Locales Achievement Reward Data...");
        sAchievementMgr->LoadRewardLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_achievement_reward` reloaded.");
        return true;
    }

    static bool HandleReloadLfgRewardsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading lfg dungeon rewards...");
        sLFGMgr->LoadRewards();
        handler->SendGlobalGMSysMessage("DB table `lfg_dungeon_rewards` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesCreatureCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Locales Creature ...");
        sObjectMgr->LoadCreatureLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_creature` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesGameobjectCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Locales Gameobject ... ");
        sObjectMgr->LoadGameObjectLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_gameobject` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesGossipMenuOptionCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Locales Gossip Menu Option ... ");
        sGossipDataStore->LoadGossipMenuItemsLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_gossip_menu_option` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesPageTextCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Locales Page Text ... ");
        sObjectMgr->LoadPageTextLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_page_text` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesPointsOfInterestCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Locales Points Of Interest ... ");
        sQuestDataStore->LoadPointOfInterestLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_points_of_interest` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesQuestCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Locales Quest ... ");
        sQuestDataStore->LoadQuestTemplateLocale();
        sQuestDataStore->LoadQuestObjectivesLocale();
        handler->SendGlobalGMSysMessage("DB table `locales_quest` reloaded.");
        return true;
    }

    static bool HandleReloadMailLevelRewardCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Player level dependent mail rewards...");
        sObjectMgr->LoadMailLevelRewards();
        handler->SendGlobalGMSysMessage("DB table `mail_level_reward` reloaded.");
        return true;
    }

    static bool HandleReloadAuctionsCommand(ChatHandler* handler, const char* /*args*/)
    {
        ///- Reload dynamic data tables from the database
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Auctions...");
        sAuctionMgr->LoadAuctionItems();
        sAuctionMgr->LoadAuctions();
        handler->SendGlobalGMSysMessage("Auctions reloaded.");
        return true;
    }

    static bool HandleReloadConditions(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Conditions...");
        sConditionMgr->LoadConditions(true);
        handler->SendGlobalGMSysMessage("Conditions reloaded.");
        return true;
    }

    static bool HandleReloadCreatureText(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Creature Texts...");
        sCreatureTextMgr->LoadCreatureTexts();
        handler->SendGlobalGMSysMessage("Creature Texts reloaded.");
        return true;
    }

    static bool HandleReloadSmartScripts(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading Smart Scripts...");
        sSmartScriptMgr->LoadSmartAIFromDB();
        handler->SendGlobalGMSysMessage("Smart Scripts reloaded.");
        return true;
    }

    static bool HandleReloadVehicleTemplateAccessoryCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Reloading vehicle_template_accessory table...");
        sObjectMgr->LoadVehicleTemplateAccessories();
        handler->SendGlobalGMSysMessage("Vehicle template accessories reloaded.");
        return true;
    }

    static bool HandleReloadScriptWaypointCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Reloading script_waypoint table...");
        sScriptSystemMgr->LoadScriptWaypoints();
        handler->SendGlobalGMSysMessage("script_waypoint table reloaded.");
        return true;
    }

    static bool HandleReloadPhaseDefinitionsCommand(ChatHandler* handler, const char* /*args*/)    
    {    
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Reloading phase_definitions table...");    
        sObjectMgr->LoadPhaseDefinitions();    
        sWorld->UpdatePhaseDefinitions();    
        handler->SendGlobalGMSysMessage("Phase Definitions reloaded.");    
        return true;
    }

    static bool HandleReloadCreatureArea(ChatHandler* handler, const char* args)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Updating Creature Area...");

        QueryResult result;

        if (!*args)
            return false;

        char* mapIdStr = strtok((char*) args, " ");
        uint32 mapId = uint32(atoi(mapIdStr));
        result = WorldDatabase.PQuery("SELECT guid, map, position_x, position_y, position_z FROM creature WHERE map = %u", mapId);

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Updated 0 creature area.");
            return true;
        }

        SQLTransaction trans = WorldDatabase.BeginTransaction();

        do
        {
            Field* fields = result->Fetch();

            uint32 guid  = fields[0].GetUInt32();
            uint32 mapId = fields[1].GetUInt32();
            float  posX  = fields[2].GetFloat();
            float  poxY  = fields[3].GetFloat();
            float  posZ  = fields[4].GetFloat();

            uint32 zoneId = 0, areaId = 0;
            sMapMgr->GetZoneAndAreaId(zoneId, areaId, mapId, posX, poxY, posZ);

            std::ostringstream outCreatureAreaStream;
            outCreatureAreaStream << "REPLACE INTO creature_area (`guid`, `zone`, `area`) VALUES (" << guid << ", " << zoneId << ", " << areaId << ");";
            trans->Append(outCreatureAreaStream.str().c_str());
        }
        while (result->NextRow());

        WorldDatabase.CommitTransaction(trans);

        handler->SendGlobalGMSysMessage("Creature Areas Updated.");
        return true;
    }

    static bool HandleReloadLetterAnalogCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Re-Loading Letter Analogs...");
        sWordFilterMgr->LoadLetterAnalogs();
        handler->SendGlobalGMSysMessage("DB table `letter_analog` reloaded.");
        return true;
    }

    static bool HandleReloadBadWordCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Re-Loading Bad Words...");
        sWordFilterMgr->LoadBadWords();
        handler->SendGlobalGMSysMessage("DB table `bad_word` reloaded.");
        return true;
    }
    
    static bool HandleReloadBadSentencesCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,"Re-Loading Bad Sentences...");
        sWordFilterMgr->LoadBadSentences();
        handler->SendGlobalGMSysMessage("DB table `bad_senteces` reloaded.");
        return true;
    }

    static bool HandleReloadAreaTriggerDataCommand(ChatHandler* handler, const char* /*args*/)
    {
        sAreaTriggerDataStore->LoadAreaTriggerActionsAndData();
        handler->SendGlobalGMSysMessage("DB tables `areatrigger_data` and `areatrigger_actions` reloaded.");
        return true;
    }

    static bool HandleReloadWorldVisibleDistanceCommand(ChatHandler* handler, const char* /*args*/)
    {
        sObjectMgr->LoadWorldVisibleDistance();
        handler->SendGlobalGMSysMessage("DB tables `world_visible_distance` reloaded.");
        return true;
    }

    static bool HandleReloadWorldRateInfoCommand(ChatHandler* handler, const char* /*args*/)
    {
        sObjectMgr->LoadWorldRateInfo();
        handler->SendGlobalGMSysMessage("DB tables `world_rate_info` reloaded.");
        return true;
    }

    static bool HandleReloadConversation(ChatHandler* handler, const char* /*args*/)
    {
        sConversationDataStore->LoadConversationData();
        handler->SendGlobalGMSysMessage("DB tables `conversation_data` and `conversation_creature` reloaded.");
        return true;
    }

    static bool HandleReloadSummonGroups(ChatHandler* handler, const char* /*args*/)
    {
        sObjectMgr->LoadTempSummons();
        handler->SendGlobalGMSysMessage("DB tables `creature_summon_groups` reloaded.");
        return true;
    }

    static bool HandleReloadNpcDonateVendorCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `store_products` Table!");
        sObjectMgr->LoadDonateVendors();
        handler->SendGlobalGMSysMessage("DB table `store_products` reloaded.");
        return true;
    }
    
    static bool HandleReloadDeathMatchProductsCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `deathmatch_products` Table!");
        sObjectMgr->LoadDeathMatchStore();
        handler->SendGlobalGMSysMessage("DB table `deathmatch_products` reloaded.");
        return true;
    }

    static bool HandleReloadScenarioDataCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading scenario_data...");
        sObjectMgr->LoadScenarioData();
        handler->SendGlobalGMSysMessage("DB table `scenario_data`reloaded.");
        return true;
    }

    static bool HandleReloadPvpRewardCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading pvp_reward...");
        sBattlegroundMgr->LoadPvpRewards();
        handler->SendGlobalGMSysMessage("DB table `pvp_reward`reloaded.");
        return true;
    }

    static bool HandleReloadWorldQuestCommand(ChatHandler* handler, const char* /*args*/)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading World Quest...");
        sQuestDataStore->LoadWorldQuestTemplates();
        handler->SendGlobalGMSysMessage("DB table `world_quest` reloaded.");
        return true;
    }
};

void AddSC_reload_commandscript()
{
    new reload_commandscript();
}
