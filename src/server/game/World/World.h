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

/// \addtogroup world The World
/// @{
/// \file

#ifndef __WORLD_H
#define __WORLD_H

#include "Common.h"
#include "Realm/Realm.h"
#include "ObjectGuid.h"
#include "Timer.h"
#include "SharedDefines.h"
#include "DatabaseEnvFwd.h"
#include "Opcodes.h"
#include "QueryCallbackProcessor.h"
#include "WorldPacket.h"
#include "Threading/LockedQueue.h"
#include <safe_ptr.h>
#include <atomic>
#include "Util.h"

class Object;
class WorldPacket;
class WorldSession;
class Player;
class WorldSocket;
class SystemMgr;

// ServerMessages.dbc
enum ServerMessageType
{
    SERVER_MSG_SHUTDOWN_TIME          = 1,
    SERVER_MSG_RESTART_TIME           = 2,
    SERVER_MSG_STRING                 = 3,
    SERVER_MSG_SHUTDOWN_CANCELLED     = 4,
    SERVER_MSG_RESTART_CANCELLED      = 5,
    SERVER_MSG_BG_SHUTDOWN_TIME       = 6,
    SERVER_MSG_BG_RESTART_TIME        = 7,
    SERVER_MSG_INSTANCE_SHUTDOWN_TIME = 8,
    SERVER_MSG_INSTANCE_RESTART_TIME  = 9,
    SERVER_MSG_CONTENT_READY          = 10,
    SERVER_MSG_TICKET_SERVICED_SOON   = 11,
    SERVER_MSG_WAIT_TIME_UNAVAILABLE  = 12,
    SERVER_MSG_TICKET_WAIT_TIME       = 13,
};

enum ShutdownMask
{
    SHUTDOWN_MASK_RESTART = 1,
    SHUTDOWN_MASK_IDLE    = 2,
};

enum ShutdownExitCode
{
    SHUTDOWN_EXIT_CODE = 0,
    ERROR_EXIT_CODE    = 1,
    RESTART_EXIT_CODE  = 2,
};

/// Timers for different object refresh rates
enum WorldTimers
{
    WUPDATE_AUCTIONS,
    WUPDATE_UPTIME,
    WUPDATE_CORPSES,
    WUPDATE_EVENTS,
    WUPDATE_CLEANDB,
    WUPDATE_AUTOBROADCAST,
    WUPDATE_MAILBOXQUEUE,
    WUPDATE_DELETECHARS,
    WUPDATE_PINGDB,
    WUPDATE_GUILDSAVE,
    WUPDATE_BLACKMARKET,
    WUPDATE_AHBOT,
    WUPDATE_DONATE_AND_SERVICES,

    WUPDATE_COUNT
};

/// Configuration elements
enum WorldBoolConfigs
{
    CONFIG_DURABILITY_LOSS_IN_PVP = 0,
    CONFIG_ADDON_CHANNEL,
    CONFIG_ALLOW_PLAYER_COMMANDS,
    CONFIG_CLEAN_CHARACTER_DB,
    CONFIG_GRID_UNLOAD,
    CONFIG_ALLOW_TWO_SIDE_ACCOUNTS,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_CALENDAR,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_MAIL,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFG,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFR,
    CONFIG_ALLOW_TWO_SIDE_WHO_LIST,
    CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND,
    CONFIG_ALLOW_TWO_SIDE_TRADE,
    CONFIG_ALL_TAXI_PATHS,
    CONFIG_INSTANT_TAXI,
    CONFIG_INSTANCE_IGNORE_LEVEL,
    CONFIG_INSTANCE_IGNORE_RAID,
    CONFIG_CAST_UNSTUCK,
    CONFIG_GM_LOG_TRADE,
    CONFIG_ALLOW_GM_GROUP,
    CONFIG_ALLOW_GM_FRIEND,
    CONFIG_GM_LOWER_SECURITY,
    CONFIG_SKILL_PROSPECTING,
    CONFIG_SKILL_MILLING,
    CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY,
    CONFIG_WEATHER,
    CONFIG_QUEST_IGNORE_RAID,
    CONFIG_DETECT_POS_COLLISION,
    CONFIG_RESTRICTED_LFG_CHANNEL,
    CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL,
    CONFIG_TALENTS_INSPECTING,
    CONFIG_CHAT_FAKE_MESSAGE_PREVENTING,
    CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP,
    CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE,
    CONFIG_DEATH_BONES_WORLD,
    CONFIG_DEATH_BONES_BG_OR_ARENA,
    CONFIG_DECLINED_NAMES_USED,
    CONFIG_BATTLEGROUND_CAST_DESERTER,
    CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE,
    CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY,
    CONFIG_BG_XP_FOR_KILL,
    CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS,
    CONFIG_ARENA_QUEUE_ANNOUNCER_ENABLE,
    CONFIG_ARENA_QUEUE_ANNOUNCER_PLAYERONLY,
    CONFIG_ARENA_SEASON_IN_PROGRESS,
    CONFIG_ARENA_LOG_EXTENDED_INFO,
    CONFIG_ARENA_DEATHMATCH,
    CONFIG_OFFHAND_CHECK_AT_SPELL_UNLEARN,
    CONFIG_VMAP_INDOOR_CHECK,
    CONFIG_PET_LOS,
    CONFIG_START_ALL_SPELLS,
    CONFIG_START_ALL_EXPLORED,
    CONFIG_START_ALL_REP,
    CONFIG_ALWAYS_MAXSKILL,
    CONFIG_PVP_TOKEN_ENABLE,
    CONFIG_NO_RESET_TALENT_COST,
    CONFIG_SHOW_KICK_IN_WORLD,
    CONFIG_CHATLOG_CHANNEL,
    CONFIG_CHATLOG_WHISPER,
    CONFIG_CHATLOG_SYSCHAN,
    CONFIG_CHATLOG_PARTY,
    CONFIG_CHATLOG_RAID,
    CONFIG_CHATLOG_GUILD,
    CONFIG_CHATLOG_PUBLIC,
    CONFIG_CHATLOG_ADDON,
    CONFIG_CHATLOG_BGROUND,
    CONFIG_AUTOBROADCAST,
    CONFIG_ALLOW_TICKETS,
    CONFIG_DROP_DUNGEON_ONLY_X1,
    CONFIG_DBC_ENFORCE_ITEM_ATTRIBUTES,
    CONFIG_PRESERVE_CUSTOM_CHANNELS,
    CONFIG_PDUMP_NO_PATHS,
    CONFIG_PDUMP_NO_OVERWRITE,
    CONFIG_QUEST_IGNORE_AUTO_ACCEPT,
    CONFIG_QUEST_IGNORE_AUTO_COMPLETE,
    CONFIG_WARDEN_ENABLED,
    CONFIG_WINTERGRASP_ENABLE,
    CONFIG_TOL_BARAD_ENABLE,
    CONFIG_GUILD_LEVELING_ENABLED,
    CONFIG_LIMIT_WHO_ONLINE,
    CONFIG_ANNOUNCE_BAN,
    CONFIG_ANNOUNCE_MUTE,
    CONFIG_SPELL_FORBIDDEN,
    CONFIG_DISABLE_RESTART,
    CONFIG_DISABLE_NEW_ONLINE,
    CONFIG_DISABLE_DONATELOADING,
    CONFIG_WORD_FILTER_ENABLE,
    CONFIG_SHARE_ENABLE,
    CONFIG_IPSET_ENABLE,
    CONFIG_ARCHAEOLOGY_ENABLED,
    CONFIG_ENABLE_MMAPS,
    CONFIG_LFG_DEBUG_JOIN,
    CONFIG_LFG_FORCE_MINPLAYERS,
    CONFIG_CHECK_MT_SESSION,
    CONFIG_BLACKMARKET_ENABLED,
    CONFIG_FEATURE_SYSTEM_BPAY_STORE_ENABLED,
    CONFIG_DISABLE_GARE_UPGRADE,
    CONFIG_RESPAWN_FROM_PLAYER_ENABLED,
    CONFIG_PET_BATTLES,
    CONFIG_DONATE_ON_TESTS,
    CONFIG_WORLD_QUEST,
    CONFIG_CHALLENGE_ENABLED,
    CONFIG_CROSSFACTIONBG,
    CONFIG_RESTRUCT_GUID,
    CONFIG_PLAYER_AURA_SCAN_COMMAND,
    CONFIG_ANTI_FLOOD_LFG,
    CONFIG_ANTI_FLOOD_PM,
    CONFIG_ANTI_FLOOD_HWID_BANS_ALLOW,
    CONFIG_ANTI_FLOOD_HWID_MUTE_ALLOW,
    CONFIG_ANTI_FLOOD_HWID_KICK_ALLOW,
    CONFIG_ANTICHEAT_ENABLED,
    CONFIG_ANTICHEAT_ANTI_MULTI_JUMP_ENABLED,
    CONFIG_ANTICHEAT_ANTI_SPEED_HACK_ENABLED,
    CONFIG_ANTICHEAT_USE_INTERPOLATION,
    CONFIG_ANTICHEAT_ANTI_WALL_CLIMB_ENABLED,
    CONFIG_ANTICHEAT_ANTI_WATER_WALK_ENABLED,
    CONFIG_ANTICHEAT_NOTIFY_CHEATERS,
    CONFIG_ANTICHEAT_LOG_DATA,
    CONFIG_ANTICHEAT_DETAIL_LOG,
    CONFIG_OBLITERUM_LEVEL_ENABLE,
    CONFIG_PVP_LEVEL_ENABLE,
    CONFIG_PARAGON_ENABLE,
    CONFIG_ARTIFACT_TIER_ENABLE,
    CONFIG_PLAYER_CONTROL_GUARDIAN_PETS,
    CONFIG_PLAYER_UNLIMITED_LEGION_LEGENDARIES,
    CONFIG_PLAYER_ALLOW_PVP_TALENTS_ALL_THE_TIME,
    CONFIG_GAIN_HONOR_GUARD,
    CONFIG_GAIN_HONOR_ELITE,
    BOOL_CONFIG_VALUE_COUNT
};

enum WorldFloatConfigs
{
    CONFIG_GROUP_XP_DISTANCE = 0,
    CONFIG_MAX_RECRUIT_A_FRIEND_DISTANCE,
    CONFIG_SIGHT_MONSTER,
    CONFIG_SIGHT_GUARDER,
    CONFIG_LISTEN_RANGE_SAY,
    CONFIG_LISTEN_RANGE_TEXTEMOTE,
    CONFIG_LISTEN_RANGE_YELL,
    CONFIG_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS,
    CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS,
    CONFIG_THREAT_RADIUS,
    CONFIG_CHANCE_OF_GM_SURVEY,
    CONFIG_ARCHAEOLOGY_RARE_BASE_CHANCE,
    CONFIG_ARCHAEOLOGY_RARE_MAXLEVEL_CHANCE,
    CONFIG_CAP_KILLPOINTS,
    CONFIG_CAP_KILL_CREATURE_POINTS,
    FLOAT_CONFIG_VALUE_COUNT
};

enum WorldIntConfigs
{
    CONFIG_COMPRESSION = 0,
    CONFIG_INTERVAL_SAVE,
    CONFIG_INTERVAL_GRIDCLEAN,
    CONFIG_INTERVAL_MAPUPDATE,
    CONFIG_INTERVAL_INSTANCE_UPDATE,
    CONFIG_INTERVAL_PVP_MAP_UPDATE,
    CONFIG_INTERVAL_MAP_SESSION_UPDATE,
    CONFIG_INTERVAL_OBJECT_UPDATE,
    CONFIG_INTERVAL_CHANGEWEATHER,
    CONFIG_INTERVAL_DISCONNECT_TOLERANCE,
    CONFIG_PORT_WORLD,
    CONFIG_PORT_INSTANCE,
    CONFIG_SOCKET_TIMEOUTTIME,
    CONFIG_SESSION_ADD_DELAY,
    CONFIG_GAME_TYPE,
    CONFIG_REALM_ZONE,
    CONFIG_STRICT_PLAYER_NAMES,
    CONFIG_STRICT_CHARTER_NAMES,
    CONFIG_STRICT_PET_NAMES,
    CONFIG_MIN_PLAYER_NAME,
    CONFIG_MIN_CHARTER_NAME,
    CONFIG_MIN_PET_NAME,
    CONFIG_CHARACTER_CREATING_DISABLED,
    CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK,
    CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK,
    CONFIG_CHARACTERS_PER_ACCOUNT,
    CONFIG_CHARACTERS_PER_REALM,
    CONFIG_HEROIC_CHARACTERS_PER_REALM,
    CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER,
    CONFIG_DEMON_HUNTERS_PER_REALM,
    CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_DEMON_HUNTER,
    CONFIG_SKIP_CINEMATICS,
    CONFIG_MAX_PLAYER_LEVEL,
    CONFIG_MIN_DUALSPEC_LEVEL,
    CONFIG_START_PLAYER_LEVEL,
    CONFIG_START_PLAYER_MONEY,
    CONFIG_CURRENCY_RESET_HOUR,
    CONFIG_CURRENCY_RESET_DAY,
    CONFIG_CURRENCY_RESET_INTERVAL,
    CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL,
    CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL_DIFFERENCE,
    CONFIG_MAX_SPELL_CASTS_IN_CHAIN,
    CONFIG_INSTANCE_RESET_TIME_HOUR,
    CONFIG_INSTANCE_DAILY_RESET,
    CONFIG_INSTANCE_HALF_WEEK_RESET,
    CONFIG_INSTANCE_WEEKLY_RESET,
    CONFIG_INSTANCE_UNLOAD_DELAY,
    CONFIG_MAX_PRIMARY_TRADE_SKILL,
    CONFIG_MIN_PETITION_SIGNS,
    CONFIG_GM_LOGIN_STATE,
    CONFIG_GM_VISIBLE_STATE,
    CONFIG_GM_ACCEPT_TICKETS,
    CONFIG_GM_CHAT,
    CONFIG_GM_WHISPERING_TO,
    CONFIG_GM_LEVEL_IN_GM_LIST,
    CONFIG_GM_LEVEL_IN_WHO_LIST,
    CONFIG_START_GM_LEVEL,
    CONFIG_GROUP_VISIBILITY,
    CONFIG_MAIL_DELIVERY_DELAY,
    CONFIG_UPTIME_UPDATE,
    CONFIG_SKILL_CHANCE_ORANGE,
    CONFIG_SKILL_CHANCE_YELLOW,
    CONFIG_SKILL_CHANCE_GREEN,
    CONFIG_SKILL_CHANCE_GREY,
    CONFIG_SKILL_CHANCE_MINING_STEPS,
    CONFIG_SKILL_CHANCE_SKINNING_STEPS,
    CONFIG_SKILL_GAIN_CRAFTING,
    CONFIG_SKILL_GAIN_GATHERING,
    CONFIG_MAX_OVERSPEED_PINGS,
    CONFIG_ANTI_FLOOD_COUNT_OF_MESSAGES,
    CONFIG_ANTI_FLOOD_HWID_BANS_COUNT,
    CONFIG_COMPLAINTS_REQUIRED,
    CONFIG_COMPLAINTS_PENALTY1,
    CONFIG_COMPLAINTS_PENALTY2,
    CONFIG_COMPLAINTS_PENALTY3,
    CONFIG_CHATFLOOD_MESSAGE_COUNT,
    CONFIG_CHATFLOOD_MESSAGE_DELAY,
    CONFIG_CHATFLOOD_MUTE_TIME,
    CONFIG_EVENT_ANNOUNCE,
    CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY,
    CONFIG_CREATURE_FAMILY_FLEE_DELAY,
    CONFIG_WORLD_BOSS_LEVEL_DIFF,
    CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF,
    CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF,
    CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY,
    CONFIG_CHAT_STRICT_LINK_CHECKING_KICK,
    CONFIG_CHAT_CHANNEL_LEVEL_REQ,
    CONFIG_CHAT_WHISPER_LEVEL_REQ,
    CONFIG_CHAT_SAY_LEVEL_REQ,
    CONFIG_TRADE_LEVEL_REQ,
    CONFIG_TICKET_LEVEL_REQ,
    CONFIG_AUCTION_LEVEL_REQ,
    CONFIG_MAIL_LEVEL_REQ,
    CONFIG_MAIL_GOLD_LEVEL_REQ,
    CONFIG_CORPSE_DECAY_NORMAL,
    CONFIG_CORPSE_DECAY_RARE,
    CONFIG_CORPSE_DECAY_ELITE,
    CONFIG_CORPSE_DECAY_RAREELITE,
    CONFIG_CORPSE_DECAY_WORLDBOSS,
    CONFIG_DEATH_SICKNESS_LEVEL,
    CONFIG_INSTANT_LOGOUT,
    CONFIG_DISABLE_BREATHING,
    CONFIG_BATTLEGROUND_INVITATION_TYPE,
    CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER,
    CONFIG_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH,
    CONFIG_ARENA_MAX_RATING_DIFFERENCE,
    CONFIG_ARENA_RATING_DISCARD_TIMER,
    CONFIG_ARENA_RATED_UPDATE_TIMER,
    CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS,
    CONFIG_ARENA_SEASON_ID,
    CONFIG_ARENA_START_RATING,
    CONFIG_ARENA_START_PERSONAL_RATING,
    CONFIG_ARENA_START_MATCHMAKER_RATING,
    CONFIG_MAX_WHO,
    CONFIG_HONOR_AFTER_DUEL,
    CONFIG_PVP_TOKEN_MAP_TYPE,
    CONFIG_PVP_TOKEN_ID,
    CONFIG_PVP_TOKEN_COUNT,
    CONFIG_INTERVAL_LOG_UPDATE,
    CONFIG_MIN_LOG_UPDATE,
    CONFIG_ENABLE_SINFO_LOGIN,
    CONFIG_PLAYER_ALLOW_COMMANDS,
    CONFIG_NUMTHREADS,
    CONFIG_MAP_NUMTHREADS,
    CONFIG_LOGDB_CLEARINTERVAL,
    CONFIG_LOGDB_CLEARTIME,
    CONFIG_CLIENTCACHE_VERSION,
    CONFIG_HOTFIX_CACHE_VERSION,
    CONFIG_GUILD_EVENT_LOG_COUNT,
    CONFIG_GUILD_NEWS_LOG_COUNT,
    CONFIG_GUILD_BANK_EVENT_LOG_COUNT,
    CONFIG_RANDOM_BG_RESET_HOUR,
    CONFIG_CHARDELETE_KEEP_DAYS,
    CONFIG_CHARDELETE_METHOD,
    CONFIG_CHARDELETE_MIN_LEVEL,
    CONFIG_AUTOBROADCAST_CENTER,
    CONFIG_AUTOBROADCAST_INTERVAL,
    CONFIG_MAX_RESULTS_LOOKUP_COMMANDS,
    CONFIG_DB_PING_INTERVAL,
    CONFIG_PRESERVE_CUSTOM_CHANNEL_DURATION,
    CONFIG_PERSISTENT_CHARACTER_CLEAN_FLAGS,
    CONFIG_LFG_OPTIONSMASK,
    CONFIG_LFG_SHORTAGE_CHECK_INTERVAL,
    CONFIG_LFG_SHORTAGE_PERCENT,
    CONFIG_LFG_MAX_QUEUES,
    CONFIG_MAX_INSTANCES_PER_HOUR,
    CONFIG_NEW_ANTICHEAT_MODE,
    CONFIG_WARDEN_CLIENT_RESPONSE_DELAY,
    CONFIG_WARDEN_CLIENT_CHECK_HOLDOFF,
    CONFIG_WARDEN_BASIC_SECURITY_LEVEL_REQ,
    CONFIG_WARDEN_MAX_SECURITY_LEVEL_REQ,
    CONFIG_AHBOT_UPDATE_INTERVAL,
    CONFIG_WINTERGRASP_PLR_MAX,
    CONFIG_WINTERGRASP_PLR_MIN,
    CONFIG_WINTERGRASP_PLR_MIN_LVL,
    CONFIG_WINTERGRASP_BATTLETIME,
    CONFIG_WINTERGRASP_NOBATTLETIME,
    CONFIG_WINTERGRASP_RESTART_AFTER_CRASH,
    CONFIG_TOL_BARAD_PLR_MAX,
    CONFIG_TOL_BARAD_PLR_MIN,
    CONFIG_TOL_BARAD_PLR_MIN_LVL,
    CONFIG_TOL_BARAD_BATTLETIME,
    CONFIG_TOL_BARAD_NOBATTLETIME,
    CONFIG_GUILD_SAVE_INTERVAL,
    CONFIG_GUILD_MAX_LEVEL,
    CONFIG_GUILD_UNDELETABLE_LEVEL,
    CONFIG_GUILD_DAILY_XP_CAP,
    CONFIG_GUILD_WEEKLY_REP_CAP,
    CONFIG_AUTO_SERVER_RESTART_HOUR,
    CONFIG_WORD_FILTER_MUTE_DURATION,
    CONFIG_MAX_SKILL_VALUE,
    CONFIG_TRANSFER_GOLD_LIMIT,
    CONFIG_BLACKMARKET_MAXAUCTIONS,
    CONFIG_BLACKMARKET_UPDATE_PERIOD,
    CONFIG_RESPAWN_FROM_PLAYER_COUNT,
    CONFIG_LOG_GOLD_FROM,
    CONFIG_ARTIFACT_RESEARCH_TIMER,
    CONFIG_CHALLENGE_KEY_RESET,
    CONFIG_WORLD_QUEST_RESET_TIME_HOUR,
    CONFIG_WORLD_QUEST_HOURLY_RESET,
    CONFIG_WORLD_QUEST_DAILY_RESET,
    CONFIG_INVASION_POINT_RESET,
    CONFIG_ARTIFACT_KNOWLEDGE_CAP,
    CONFIG_ARTIFACT_KNOWLEDGE_START,
    CONFIG_WORLD_QUEST_MIN_ITEMLEVEL,
    CONFIG_WORLD_QUEST_ITEMLEVEL_CAP,
    CONFIG_WORLD_SESSION_EXPIRE_TIME,
    CONFIG_WORLD_PLAYER_COMMAND_TIMER,
    CONFIG_PLAYER_INVISIBLE_STATUS_COMMAND,
    CONFIG_ARENA_1V1_COUNTDOWN,
    CONFIG_ARENA_2V2_COUNTDOWN,
    CONFIG_ARENA_3V3_COUNTDOWN,
    CONFIG_MAX_PRESTIGE_LEVEL,
    CONFIG_SIZE_CELL_FOR_PULL,
    CONFIG_ANTICHEAT_MAX_ALLOWED_DESYNC,
    CONFIG_ANTICHEAT_GM_ANNOUNCE_MASK,
    CONFIG_MAX_ITEM_LEVEL,
    CONFIG_OBLITERUM_LEVEL_START,
    CONFIG_OBLITERUM_LEVEL_MIN,
    CONFIG_OBLITERUM_LEVEL_MAX,
    CONFIG_ITEM_LEGENDARY_LIMIT,
    CONFIG_ITEM_LEGENDARY_LEVEL,
    CONFIG_CHALLENGE_LEVEL_LIMIT,
    CONFIG_CHALLENGE_LEVEL_MAX,
    CONFIG_CHALLENGE_LEVEL_STEP,
    CONFIG_CHALLENGE_ADD_ITEM,
    CONFIG_CHALLENGE_ADD_ITEM_TYPE,
    CONFIG_CHALLENGE_ADD_ITEM_COUNT,
    CONFIG_CHALLENGE_MANUAL_AFFIX1,
    CONFIG_CHALLENGE_MANUAL_AFFIX2,
    CONFIG_CHALLENGE_MANUAL_AFFIX3,
    CONFIG_PVP_ACTIVE_SEASON,
    CONFIG_PVP_ACTIVE_STEP,
    CONFIG_WEIGHTED_MYTHIC_KEYSTONE,
    CONFIG_PLAYER_AFK_TIMEOUT,
    CONFIG_PLAYER_LEGION_LEGENDARY_EQUIP_COUNT,
    CONFIG_LEGION_ENABLED_PATCH,
    INT_CONFIG_VALUE_COUNT
};

/// Server rates
enum Rates
{
    RATE_HEALTH = 0,
    RATE_POWER_MANA,
    RATE_POWER_RAGE_INCOME,
    RATE_POWER_RAGE_LOSS,
    RATE_POWER_RUNICPOWER_INCOME,
    RATE_POWER_RUNICPOWER_LOSS,
    RATE_POWER_FOCUS,
    RATE_POWER_ENERGY,
    RATE_SKILL_DISCOVERY,
    RATE_DROP_ITEM_POOR,
    RATE_DROP_ITEM_NORMAL,
    RATE_DROP_ITEM_UNCOMMON,
    RATE_DROP_ITEM_RARE,
    RATE_DROP_ITEM_EPIC,
    RATE_DROP_ITEM_LEGENDARY,
    RATE_DROP_ITEM_ARTIFACT,
    RATE_DROP_ITEM_REFERENCED,
    RATE_DROP_ITEM_REFERENCED_AMOUNT,
    RATE_DROP_CURRENCY,
    RATE_DROP_CURRENCY_AMOUNT,
    RATE_DROP_MONEY,
    RATE_DROP_ZONE_LOOT,
    RATE_XP_KILL,
    RATE_XP_QUEST,
    RATE_XP_GUILD_MODIFIER,
    RATE_XP_EXPLORE,
    RATE_XP_GATHERING,
    RATE_REPAIRCOST,
    RATE_REPUTATION_GAIN,
    RATE_REPUTATION_LOWLEVEL_KILL,
    RATE_REPUTATION_LOWLEVEL_QUEST,
    RATE_REPUTATION_RECRUIT_A_FRIEND_BONUS,
    RATE_CREATURE_NORMAL_HP,
    RATE_CREATURE_ELITE_ELITE_HP,
    RATE_CREATURE_ELITE_RAREELITE_HP,
    RATE_CREATURE_ELITE_WORLDBOSS_HP,
    RATE_CREATURE_ELITE_RARE_HP,
    RATE_CREATURE_NORMAL_DAMAGE,
    RATE_CREATURE_ELITE_ELITE_DAMAGE,
    RATE_CREATURE_ELITE_RAREELITE_DAMAGE,
    RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE,
    RATE_CREATURE_ELITE_RARE_DAMAGE,
    RATE_CREATURE_NORMAL_SPELLDAMAGE,
    RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE,
    RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE,
    RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE,
    RATE_CREATURE_ELITE_RARE_SPELLDAMAGE,
    RATE_CREATURE_AGGRO,
    RATE_REST_INGAME,
    RATE_REST_OFFLINE_IN_TAVERN_OR_CITY,
    RATE_REST_OFFLINE_IN_WILDERNESS,
    RATE_DAMAGE_FALL,
    RATE_AUCTION_TIME,
    RATE_AUCTION_DEPOSIT,
    RATE_AUCTION_CUT,
    RATE_HONOR,
    RATE_HONOR_QB,
    RATE_MINING_AMOUNT,
    RATE_MINING_NEXT,
    RATE_TALENT,
    RATE_CORPSE_DECAY_LOOTED,
    RATE_TARGET_POS_RECALCULATION_RANGE,
    RATE_DURABILITY_LOSS_ON_DEATH,
    RATE_DURABILITY_LOSS_DAMAGE,
    RATE_DURABILITY_LOSS_PARRY,
    RATE_DURABILITY_LOSS_ABSORB,
    RATE_DURABILITY_LOSS_BLOCK,
    RATE_MOVESPEED,
    RATE_ONLINE,
    RATE_DONATE,
    MAX_RATES
};

enum RealmZone
{
    REALM_ZONE_UNKNOWN       = 0,                           // any language
    REALM_ZONE_DEVELOPMENT   = 1,                           // any language
    REALM_ZONE_UNITED_STATES = 2,                           // extended-Latin
    REALM_ZONE_OCEANIC       = 3,                           // extended-Latin
    REALM_ZONE_LATIN_AMERICA = 4,                           // extended-Latin
    REALM_ZONE_TOURNAMENT_5  = 5,                           // basic-Latin at create, any at login
    REALM_ZONE_KOREA         = 6,                           // East-Asian
    REALM_ZONE_TOURNAMENT_7  = 7,                           // basic-Latin at create, any at login
    REALM_ZONE_ENGLISH       = 8,                           // extended-Latin
    REALM_ZONE_GERMAN        = 9,                           // extended-Latin
    REALM_ZONE_FRENCH        = 10,                          // extended-Latin
    REALM_ZONE_SPANISH       = 11,                          // extended-Latin
    REALM_ZONE_RUSSIAN       = 12,                          // Cyrillic
    REALM_ZONE_TOURNAMENT_13 = 13,                          // basic-Latin at create, any at login
    REALM_ZONE_TAIWAN        = 14,                          // East-Asian
    REALM_ZONE_TOURNAMENT_15 = 15,                          // basic-Latin at create, any at login
    REALM_ZONE_CHINA         = 16,                          // East-Asian
    REALM_ZONE_CN1           = 17,                          // basic-Latin at create, any at login
    REALM_ZONE_CN2           = 18,                          // basic-Latin at create, any at login
    REALM_ZONE_CN3           = 19,                          // basic-Latin at create, any at login
    REALM_ZONE_CN4           = 20,                          // basic-Latin at create, any at login
    REALM_ZONE_CN5           = 21,                          // basic-Latin at create, any at login
    REALM_ZONE_CN6           = 22,                          // basic-Latin at create, any at login
    REALM_ZONE_CN7           = 23,                          // basic-Latin at create, any at login
    REALM_ZONE_CN8           = 24,                          // basic-Latin at create, any at login
    REALM_ZONE_TOURNAMENT_25 = 25,                          // basic-Latin at create, any at login
    REALM_ZONE_TEST_SERVER   = 26,                          // any language
    REALM_ZONE_TOURNAMENT_27 = 27,                          // basic-Latin at create, any at login
    REALM_ZONE_QA_SERVER     = 28,                          // any language
    REALM_ZONE_CN9           = 29,                          // basic-Latin at create, any at login
    REALM_ZONE_TEST_SERVER_2 = 30,                          // any language
    REALM_ZONE_CN10          = 31,                          // basic-Latin at create, any at login
    REALM_ZONE_CTC           = 32,
    REALM_ZONE_CNC           = 33,
    REALM_ZONE_CN1_4         = 34,                          // basic-Latin at create, any at login
    REALM_ZONE_CN2_6_9       = 35,                          // basic-Latin at create, any at login
    REALM_ZONE_CN3_7         = 36,                          // basic-Latin at create, any at login
    REALM_ZONE_CN5_8         = 37                           // basic-Latin at create, any at login
};

enum ServerWorldStates
{
    WS_CURRENCY_RESET_TIME              = 20001,                      // Next currency reset time
    WS_WEEKLY_RESET_TIME                = 20002,                      // Next weekly reset time
    WS_BG_DAILY_RESET_TIME              = 20003,                      // Next daily BG reset time
    WS_AUTO_SERVER_RESTART_TIME         = 20005,                      // Next server restart time
    WS_INSTANCE_DAILY_RESET_TIME        = 20006,                      // Next daily Instance restart time
    WS_INSTANCE_HALF_WEEK_RESET_TIME    = 20007,                      // Next daily Instance restart time
    WS_INSTANCE_WEEKLY_RESET_TIME       = 20008,                      // Next weekly Instance restart time
    WS_CHALLENGE_KEY_RESET_TIME         = 20015,                      // Reset time for Challenge key
    WS_CHALLENGE_AFFIXE1_RESET_TIME     = 20016,                      // Challenge Affixe 1
    WS_CHALLENGE_AFFIXE2_RESET_TIME     = 20017,                      // Challenge Affixe 2
    WS_CHALLENGE_AFFIXE3_RESET_TIME     = 20018,                      // Challenge Affixe 3
    WS_WORLDQUEST_HOURLY_RESET_TIME     = 20019,                      // World quest every 6 hours reset time
    WS_WORLDQUEST_DAILY_RESET_TIME      = 20020,                      // World quest every day reset time
    WS_CHALLENGE_LAST_RESET_TIME        = 20021,                      // Last reset time for Challenge key
    WS_BAN_WAVE_TIME                    = 20022,                      // Next banwave time
    WS_CURRENT_ARTIFACT_KNOWLEDGE       = 20023,                      // Current Artifact Knowledge
    WS_INVASION_POINT_RESET_TIME        = 20024,                      // World quest every 2 hours reset time
    WS_SCORE_CALL_OF_THE_SCARAB_ALLINCE = 20025,                      // Alliance Score for Holiday "Call of the Scarab".
    WS_SCORE_CALL_OF_THE_SCARAB_HORDE   = 20026                       // Horde Score for Holiday "Call of the Scarab".
};

/// Storage class for commands issued for delayed execution
struct CliCommandHolder
{
    typedef void Print(void*, const char*);
    typedef void CommandFinished(void*, bool success);

    void* m_callbackArg;
    char *m_command;
    Print* m_print;

    CommandFinished* m_commandFinished;

    CliCommandHolder(void* callbackArg, const char* command, Print* zprint, CommandFinished* commandFinished);

    ~CliCommandHolder();
};

typedef std::shared_ptr<WorldSession> WorldSessionPtr;
typedef std::unordered_map<uint32, WorldSessionPtr> SessionMap;

struct CharacterInfo
{
    ObjectGuid::LowType Guid;
    std::string Name;
    uint32 GuildId = 0;
    uint32 AccountId = 0;
    uint32 BnetAccountId = 0;
    uint16 ZoneId = 0;
    uint16 SpecId = 0;
    uint8 Class = 0;
    uint8 Race = 0;
    uint8 Sex = 0;
    uint8 Level = 0;
    uint8 RankId = 0;
    bool IsDeleted = false;
};

struct GlobalMessageData
{
    GlobalMessageData(WorldPacket const* _packet, WorldSession* _self, uint32 _team);
    WorldPacket packet;
    WorldSession* self;
    uint32 team;
};

/// The World
class World
{
    public:
        static std::atomic<uint32> m_worldLoopCounter;
        static uint64 SendSize[OPCODE_COUNT];
        static uint64 SendCount[OPCODE_COUNT];

        static World* instance();

        WorldSessionPtr FindSession(uint32 id) const;
        void AddSession(WorldSessionPtr s);
        void AddInstanceSocket(std::weak_ptr<WorldSocket> sock, uint64 connectToKey);
        void SendAutoBroadcast();
        bool RemoveSession(uint32 id);
        /// Get the number of current active sessions
        void UpdateMaxSessionCounters();
        const SessionMap& GetAllSessions() const;
        uint32 GetActiveAndQueuedSessionCount() const;
        uint32 GetActiveSessionCount() const;
        uint32 GetActiveSessionCountDiff() const;
        uint32 GetQueuedSessionCount() const;
        uint32 GetMaxQueuedSessionCount() const;
        uint32 GetMaxActiveSessionCount() const;
        uint32 GetPlayerCount() const;
        uint32 GetMaxPlayerCount() const;
        void IncreasePlayerCount();
        void DecreasePlayerCount();
        uint32 GetSessionCount() const;
        uint32 GetMaxSessionCount() const;
        void IncreaseSessionCount();
        void DecreaseSessionCount();

        Player* FindPlayerInZone(uint32 zone);

        /// Deny clients?
        bool IsClosed() const;

        /// Close world
        void SetClosed(bool val);

        void ProcessMailboxQueue();
        void Transfer();

        uint32 GetPvPMysticCount() const { return m_pvpMysticCount; }
        void AddPvPMysticCount() { m_pvpMysticCount++; }

        /// Security level limitations
        AccountTypes GetPlayerSecurityLimit() const { return m_allowedSecurityLevel; }
        void SetPlayerSecurityLimit(AccountTypes sec);
        void LoadDBAllowedSecurityLevel();

        /// Active session server limit
        void SetPlayerAmountLimit(uint32 limit) { m_playerLimit = limit; }
        uint32 GetPlayerAmountLimit() const { return m_playerLimit; }

        //player Queue
        typedef std::list<WorldSessionPtr> Queue;
        void AddQueuedPlayer(WorldSessionPtr);
        bool RemoveQueuedPlayer(WorldSessionPtr session);
        int32 GetQueuePos(WorldSessionPtr);
        bool HasRecentlyDisconnected(WorldSessionPtr);

        /// \todo Actions on m_allowMovement still to be implemented
        /// Is movement allowed?
        bool getAllowMovement() const { return m_allowMovement; }
        /// Allow/Disallow object movements
        void SetAllowMovement(bool allow) { m_allowMovement = allow; }

        /// Set a new Message of the Day
        void SetMotd(std::string motd);
        /// Get the current Message of the Day
        StringVector const& GetMotd() const;

        /// Set the string for new characters (first login)
        void SetNewCharString(std::string const& str) { m_newCharString = str; }
        /// Get the string for new characters (first login)
        std::string const& GetNewCharString() const { return m_newCharString; }

        LocaleConstant GetDefaultDbcLocale() const { return m_defaultDbcLocale; }

        /// Get the path where data (dbc, maps) are stored on disk
        std::string GetDataPath() const { return m_dataPath; }

        /// When server started?
        time_t const& GetStartTime() const { return m_startTime; }
        /// What time is it?
        time_t const& GetGameTime() const { return m_gameTime; }
        /// Uptime (in secs)
        uint32 GetUptime() const { return uint32(m_gameTime - m_startTime); }
        /// Update time
        uint32 GetUpdateTime() const { return m_updateTime; }
        void SetRecordDiffInterval(int32 t) { if (t >= 0) m_int_configs[CONFIG_INTERVAL_LOG_UPDATE] = static_cast<uint32>(t); }

        /// Next daily quests and random bg reset time
        time_t GetNextDailyQuestsResetTime() const { return m_NextDailyQuestReset; }
        time_t GetNextWeeklyResetTime() const { return m_NextWeeklyReset; }
        time_t GetNextRandomBGResetTime() const { return m_NextRandomBGReset; }

        /// Get the maximum skill level a player can reach
        uint16 GetConfigMaxSkillValue() const
        {
            return getIntConfig(CONFIG_MAX_SKILL_VALUE);
        }

        void SetInitialWorldSettings();
        void LoadConfigSettings(bool reload = false);

        void SendWorldText(int32 string_id, ...);
        void SendGlobalText(const char* text, WorldSession* self);
        void SendGMText(int32 string_id, ...);
        void SendGlobalMessage(WorldPacket const* packet, WorldSession* self = nullptr, uint32 team = 0);
        void SendGlobalGMMessage(WorldPacket const* packet, WorldSession* self = nullptr, uint32 team = 0);
        bool SendZoneMessage(uint32 zone, WorldPacket const* packet, WorldSession* self = nullptr, uint32 team = 0);
        void SendZoneText(uint32 zone, const char *text, WorldSession* self = nullptr, uint32 team = 0);
        void SendServerMessage(ServerMessageType type, const char *text = "", Player* player = nullptr);
        void UpdateGlobalMessage();

        /// Are we in the middle of a shutdown?
        bool IsShuttingDown() const { return m_ShutdownTimer > 0; }
        uint32 GetShutDownTimeLeft() const { return m_ShutdownTimer; }
        void ShutdownServ(uint32 time, uint32 options, uint8 exitcode);
        void ShutdownCancel();
        void ShutdownMsg(bool show = false, Player* player = nullptr);
        static uint8 GetExitCode() { return m_ExitCode; }
        static void StopNow(uint8 exitcode) { m_stopEvent = true; m_ExitCode = exitcode; }
        static bool IsStopped() { return m_stopEvent; }

        void Update(uint32 diff);

        void UpdateSessions(uint32 diff);

        void setRate(Rates rate, float value);
        float getRate(Rates rate) const;

        void setBoolConfig(WorldBoolConfigs index, bool value);
        bool getBoolConfig(WorldBoolConfigs index) const;
        void setFloatConfig(WorldFloatConfigs index, float value);
        float getFloatConfig(WorldFloatConfigs index) const;
        void setIntConfig(WorldIntConfigs index, uint32 value);
        uint32 getIntConfig(WorldIntConfigs index) const;

        void setWorldState(uint32 index, uint32 value);
        uint32 getWorldState(uint32 index) const;
        void LoadWorldStates();

        /// Are we on a "Player versus Player" server?
        bool IsPvPRealm() const;
        bool IsFFAPvPRealm() const;

        void KickAll();
        void KickAllLess(AccountTypes sec);
        BanReturn BanAccount(BanMode mode, std::string nameOrIP, std::string duration, std::string reason, std::string author, bool queued = false);
        bool RemoveBanAccount(BanMode mode, std::string nameOrIP);
        BanReturn BanCharacter(std::string name, std::string duration, std::string reason, std::string author);
        bool RemoveBanCharacter(std::string name);
        void MuteAccount(uint32 accountId, int64 duration, std::string reason, std::string author, WorldSession * session = nullptr);

        void FlagAccount(uint32 accountId, uint32 duration, std::string reason, std::string author);
        void BanFlaggedAccounts();

        // for max speed access
        static float GetMaxVisibleDistanceOnContinents()    { return m_MaxVisibleDistanceOnContinents; }
        static float GetMaxVisibleDistanceInInstances()     { return m_MaxVisibleDistanceInInstances;  }
        float GetMaxVisibleDistanceInBG() const { return m_MaxVisibleDistanceInBG; }
        float GetMaxVisibleDistanceInArenas() const { return m_MaxVisibleDistanceInArenas; }

        static int32 GetVisibilityNotifyPeriodOnContinents(){ return m_visibility_notify_periodOnContinents; }
        static int32 GetVisibilityNotifyPeriodInInstances() { return m_visibility_notify_periodInInstances;  }
        static int32 GetVisibilityNotifyPeriodInBGArenas()  { return m_visibility_notify_periodInBGArenas;   }

        float GetVisibilityRelocationLowerLimit() const { return m_visibilityRelocationLowerLimit; }
        float GetVisibilityRelocationLowerLimitC() const { return m_visibilityRelocationLowerLimitC; }
        uint32 GetVisibilityAINotifyDelay() const { return m_visibilityAINotifyDelay; }

        static float Relocation_UpdateUnderwateLimit;
        static float ZoneUpdateDistanceRangeLimit;

        void ProcessCliCommands();
        void QueueCliCommand(CliCommandHolder* commandHolder) { cliCmdQueue.add(commandHolder); }

        void ForceGameEventUpdate();

        void UpdateRealmCharCount(uint32 accid);

        // used World DB version
        void LoadDBVersion();
        char const* GetDBVersion() const { return m_DBVersion.c_str(); }

        void RecordTimeDiff(const char * text, ...);

        void LoadAutobroadcasts();

        void UpdateAreaDependentAuras();

        void ProcessStartEvent();
        void ProcessStopEvent();
        bool GetEventKill() const { return isEventKillStart; }

        bool isEventKillStart;

        CharacterInfo const* GetCharacterInfo(ObjectGuid const& guid) const;
        CharacterInfo const* GetCharacterInfo(std::string name) const;
        void AddCharacterInfo(ObjectGuid::LowType guid, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level, uint32 accountId, uint8 zoneId = 0, uint8 rankId = 0, uint32 guildId = 0, uint32 specid = 0);
        void UpdateCharacterInfo(ObjectGuid const& guid, std::string const& name, uint8 gender = GENDER_NONE, uint8 race = RACE_NONE);
        void UpdateCharacterAccount(uint32 guid, uint32 BnetAccountId);
        void UpdateCharacterInfoLevel(ObjectGuid::LowType guid, uint8 level);
        void UpdateCharacterInfoDeleted(ObjectGuid::LowType guid, bool deleted, std::string const* name = nullptr);
        void DeleteCharacterNameData(ObjectGuid::LowType guid);
        void UpdateCharacterNameDataZoneGuild(ObjectGuid::LowType guid, uint16 zoneId, uint16 guildId, uint8 rankId);

        uint32 GetCleaningFlags() const;
        void SetCleaningFlags(uint32 flags);
        void ResetEventSeasonalQuests(uint16 event_id);
        std::string GetRealmName();
        std::string GetTrimmedRealmName();
        void UpdatePhaseDefinitions();

        bool CheckCharacterName(std::string name);
        void AddCharacterName(std::string name, CharacterInfo* nameData);
        void DeleteCharName(std::string name);

        time_t getInstanceResetTime(uint32 resetTime);
        time_t getNextInstanceDailyReset();
        time_t getNextInstanceWeeklyReset();
        time_t getNextChallengeKeyReset();

        uint32 GetRealmId() const;

        std::map<ObjectGuid, Player*> deletedPlayres;

        std::string m_serverTimeTZ;
        std::string m_gameTimeTZ;

    protected:
        void _UpdateGameTime();
        // callback for UpdateRealmCharacters
        void _UpdateRealmCharCount(PreparedQueryResult resultCharCount);

        void InitDailyQuestResetTime();
        void InitWeeklyResetTime();
        void InitRandomBGResetTime();
        void InitCurrencyResetTime();
        void InitInstanceDailyResetTime();
        void InitInstanceWeeklyResetTime();
        void InitChallengeKeyResetTime();
        void InitWorldQuestHourlyResetTime();
        void InitWorldQuestDailyResetTime();
        void InitInvasionPointResetTime();
        void InitBanWaveTime();
        void ResetDailyQuests();
        void ResetWeekly();
        void ResetRandomBG();
        void ResetCurrencyWeekCap();
        void InitServerAutoRestartTime();
        void AutoRestartServer();
        void InstanceDailyResetTime();
        void InstanceWeeklyResetTime();
        void ChallengeKeyResetTime();
        void ResetLootCooldown();
        void WorldQuestHourlyResetTime();
        void WorldQuestDailyResetTime();
        void InvasionPointResetTime();
        void StartBanWave();

    private:
        World();
        ~World();

        static uint8 m_ExitCode;
        uint32 m_ShutdownTimer;
        uint32 m_ShutdownMask;

        uint32 m_CleaningFlags;

        bool m_isClosed;

        time_t m_startTime;
        time_t m_gameTime;
        IntervalTimer m_timers[WUPDATE_COUNT];
        time_t mail_timer;
        time_t mail_timer_expires;
        time_t blackmarket_timer;
        uint32 m_updateTime, m_updateTimeSum;
        uint32 m_updateTimeCount;
        uint32 m_currentTime;

        SessionMap m_sessions;
        typedef std::unordered_map<uint32, time_t> DisconnectMap;
        DisconnectMap m_disconnects;
        uint32 m_maxActiveSessionCount;
        uint32 m_maxQueuedSessionCount;
        uint32 m_PlayerCount;
        uint32 m_MaxPlayerCount;
        uint32 m_sessionCount;
        uint32 m_maxSessionCount;

        std::string m_newCharString;
        std::string m_realmName;
        std::string m_trimmedRealmName;

        float rate_values[MAX_RATES];
        uint32 m_int_configs[INT_CONFIG_VALUE_COUNT];
        bool m_bool_configs[BOOL_CONFIG_VALUE_COUNT];
        float m_float_configs[FLOAT_CONFIG_VALUE_COUNT];
        typedef std::map<uint32, uint32> WorldStatesMap;
        WorldStatesMap m_worldstates;
        uint32 m_playerLimit;
        AccountTypes m_allowedSecurityLevel;
        LocaleConstant m_defaultDbcLocale;                     // from config for one from loaded DBC locales
        bool m_allowMovement;
        StringVector _motd;
        std::string m_dataPath;

        // for max speed access
        static float m_MaxVisibleDistanceOnContinents;
        static float m_MaxVisibleDistanceInInstances;
        static float m_MaxVisibleDistanceInBG;
        static float m_MaxVisibleDistanceInArenas;

        static int32 m_visibility_notify_periodOnContinents;
        static int32 m_visibility_notify_periodInInstances;
        static int32 m_visibility_notify_periodInBGArenas;

        static float m_visibilityRelocationLowerLimit;
        static float m_visibilityRelocationLowerLimitC;
        static int32 m_visibilityAINotifyDelay;

        // CLI command holder to be thread safe
        LockedQueue<CliCommandHolder*> cliCmdQueue;

        // next daily quests and random bg reset time
        time_t m_NextDailyQuestReset;
        time_t m_NextWeeklyReset;
        time_t m_NextRandomBGReset;
        time_t m_NextCurrencyReset;
        time_t m_NextServerRestart;
        time_t m_NextInstanceDailyReset;
        time_t m_NextInstanceWeeklyReset;
        time_t m_NextChallengeKeyReset;
        time_t m_NextWorldQuestHourlyReset;
        time_t m_NextWorldQuestDailyReset;
        time_t m_NextInvasionPointReset;
        time_t m_NextBanWaveTime;

        //Player Queue
        Queue m_QueuedPlayer;
        uint32 m_pvpMysticCount;

        // sessions that are added async
        void AddSession_(WorldSessionPtr s);
        LockedQueue<WorldSessionPtr> addSessQueue;
        
        void ProcessLinkInstanceSocket(std::pair<std::weak_ptr<WorldSocket>, uint64> linkInfo);
        LockedQueue<std::pair<std::weak_ptr<WorldSocket>, uint64>> _linkSocketQueue;

        // used versions
        std::string m_DBVersion;

        std::list<std::string> m_Autobroadcasts;

        std::vector<CharacterInfo*> _characterInfoStore;
        void LoadCharacterNameData();

        std::unordered_map<std::string, CharacterInfo*> nameMap;

        void ProcessQueryCallbacks();
        QueryCallbackProcessor _queryProcessor;

        std::queue<GlobalMessageData> _messageQueue;
        sf::contention_free_shared_mutex< > _messageQueueLock;
};

extern Realm realm;
uint32 GetVirtualRealmAddress();

#define sWorld World::instance()
#endif
/// @}
