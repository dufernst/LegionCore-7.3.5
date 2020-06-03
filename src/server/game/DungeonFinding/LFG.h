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

#ifndef _LFG_H
#define _LFG_H

#include "Common.h"

namespace lfg
{

enum LFGEnum
{
    LFG_TANKS_NEEDED                             = 1,
    LFG_HEALERS_NEEDED                           = 1,
    LFG_DPS_NEEDED                               = 3
};

enum LfgRoles : uint8
{
    PLAYER_ROLE_NONE                             = 0x00,
    PLAYER_ROLE_LEADER                           = 0x01,
    PLAYER_ROLE_TANK                             = 0x02,
    PLAYER_ROLE_HEALER                           = 0x04,
    PLAYER_ROLE_DAMAGE                           = 0x08,

    ROLE_FULL_MASK = PLAYER_ROLE_LEADER | PLAYER_ROLE_TANK | PLAYER_ROLE_HEALER | PLAYER_ROLE_DAMAGE,
};

enum LfgUpdateType
{
    LFG_UPDATETYPE_DEFAULT                       = 0,      // Internal Use
    LFG_UPDATETYPE_LEADER_UNK1                   = 1,      // FIXME: At group leave
    LFG_UPDATETYPE_ROLECHECK_ABORTED             = 6,
    LFG_UPDATETYPE_ROLECHECK_FAILED              = 7,
    LFG_UPDATETYPE_REMOVED_FROM_QUEUE            = 8,
    LFG_UPDATETYPE_PROPOSAL_FAILED               = 9,  // ERR_LFG_ROLE_CHECK_FAILED_TIMEOUT / ERR_LFG_ROLE_CHECK_FAILED_NOT_VIABLE
    LFG_UPDATETYPE_PROPOSAL_DECLINED             = 10, // ERR_LFG_PROPOSAL_DECLINED_SELF / ERR_LFG_PROPOSAL_DECLINED_PARTY
    LFG_UPDATETYPE_GROUP_FOUND                   = 11,
    LFG_UPDATETYPE_ADDED_TO_QUEUE                = 13,
    LFG_UPDATETYPE_PROPOSAL_BEGIN                = 14,
    LFG_UPDATETYPE_UPDATE_STATUS                 = 15,
    LFG_UPDATETYPE_GROUP_MEMBER_OFFLINE          = 16,
    LFG_UPDATETYPE_GROUP_DISBAND_UNK16           = 17,     // FIXME: Sometimes at group disband
    LFG_UPDATETYPE_JOIN_QUEUE_INITIAL            = 24,
    LFG_UPDATETYPE_JOIN_QUEUE                    = 25,
    LFG_UPDATETYPE_DUNGEON_FINISHED              = 26,
    LFG_UPDATETYPE_PARTY_ROLE_NOT_AVAILABLE      = 43,
    LFG_UPDATETYPE_JOIN_LFG_OBJECT_FAILED        = 45,
    LFG_UPDATETYPE_REMOVED_LEVELUP               = 49,
    LFG_UPDATETYPE_REMOVED_XP_TOGGLE             = 50,
    LFG_UPDATETYPE_REMOVED_FACTION_CHANGE        = 51,
    LFG_UPDATETYPE_PAUSE                         = 54,

    /*
     (5725):    ERR_LFG_LEADER_IS_LFM_S                                 = 568,
     (5726):    ERR_LFG_PENDING                                         = 569,
     (5894):    ERR_LFG_READY_CHECK_FAILED                              = 737,
     (5895):    ERR_LFG_READY_CHECK_FAILED_TIMEOUT                      = 738,
     (5896):    ERR_LFG_GROUP_FULL                                      = 739,
     (5897):    ERR_LFG_NO_LFG_OBJECT                                   = 740,
     (5898):    ERR_LFG_NO_SLOTS_PLAYER                                 = 741,
     (5899):    ERR_LFG_NO_SLOTS_PARTY                                  = 742,
     (5900):    ERR_LFG_NO_SPEC                                         = 743,
     (5901):    ERR_LFG_MISMATCHED_SLOTS                                = 744,
     (5902):    ERR_LFG_MISMATCHED_SLOTS_LOCAL_XREALM                   = 745,
     (5903):    ERR_LFG_PARTY_PLAYERS_FROM_DIFFERENT_REALMS             = 746,
     (5904):    ERR_LFG_MEMBERS_NOT_PRESENT                             = 747,
     (5905):    ERR_LFG_GET_INFO_TIMEOUT                                = 748,
     (5906):    ERR_LFG_INVALID_SLOT                                    = 749,
     (5907):    ERR_LFG_DESERTER_PLAYER                                 = 750,
     (5908):    ERR_LFG_DESERTER_PARTY                                  = 751,
     (5909):    ERR_LFG_DEAD                                            = 752,
     (5910):    ERR_LFG_RANDOM_COOLDOWN_PLAYER                          = 753,
     (5911):    ERR_LFG_RANDOM_COOLDOWN_PARTY                           = 754,
     (5912):    ERR_LFG_TOO_MANY_MEMBERS                                = 755,
     (5913):    ERR_LFG_TOO_FEW_MEMBERS                                 = 756,
     (5914):    ERR_LFG_PROPOSAL_FAILED                                 = 757,
     (5917):    ERR_LFG_NO_SLOTS_SELECTED                               = 760,
     (5918):    ERR_LFG_NO_ROLES_SELECTED                               = 761,
     (5919):    ERR_LFG_ROLE_CHECK_INITIATED                            = 762,
     (5920):    ERR_LFG_READY_CHECK_INITIATED                           = 763,
     (5921):    ERR_LFG_PLAYER_DECLINED_ROLE_CHECK                      = 764,
     (5922):    ERR_LFG_PLAYER_DECLINED_READY_CHECK                     = 765,
     (5923):    ERR_LFG_JOINED_QUEUE                                    = 766,
     (5924):    ERR_LFG_JOINED_FLEX_QUEUE                               = 767,
     (5925):    ERR_LFG_JOINED_RF_QUEUE                                 = 768,
     (5926):    ERR_LFG_JOINED_SCENARIO_QUEUE                           = 769,
     (5927):    ERR_LFG_JOINED_WORLD_PVP_QUEUE                          = 770,
     (5928):    ERR_LFG_JOINED_LIST                                     = 771,
     (5930):    ERR_LFG_LEFT_LIST                                       = 773,
     (5931):    ERR_LFG_ROLE_CHECK_ABORTED                              = 774,
     (5932):    ERR_LFG_READY_CHECK_ABORTED                             = 775,
     (5933):    ERR_LFG_CANT_USE_BATTLEGROUND                           = 776,
     (5934):    ERR_LFG_CANT_USE_DUNGEONS                               = 777,
     (5935):    ERR_LFG_REASON_TOO_MANY_LFG                             = 778,
     */
};

enum LfgState
{
    LFG_STATE_NONE,                                        // Not using LFG / LFR
    LFG_STATE_ROLECHECK,                                   // Rolecheck active
    LFG_STATE_QUEUED,                                      // Queued
    LFG_STATE_PROPOSAL,                                    // Proposal active
    LFG_STATE_BOOT,                                        // Vote kick active
    LFG_STATE_DUNGEON,                                     // In LFG Group, in a Dungeon
    LFG_STATE_FINISHED_DUNGEON,                            // In LFG Group, in a finished Dungeon
    LFG_STATE_RAIDBROWSER,                                 // Using Raid browser
    LFG_STATE_WAITE                                        // Waiting
};

/// Instance lock types
enum LfgLockStatusType
{
    LFG_LOCKSTATUS_OK                            = 0,      // Internal use only
    LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION        = 1,
    LFG_LOCKSTATUS_TOO_LOW_LEVEL                 = 2,
    LFG_LOCKSTATUS_TOO_HIGH_LEVEL                = 3,
    LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE            = 4,
    LFG_LOCKSTATUS_TOO_HIGH_GEAR_SCORE           = 5,
    LFG_LOCKSTATUS_RAID_LOCKED                   = 6,
    LFG_LOCKSTATUS_TARGET_LEVEL_TOO_HIGH         = 7,      // not used in lockedReason
    LFG_LOCKSTATUS_TARGET_LEVEL_TOO_LOW          = 8,      // not used in lockedReason
    LFG_LOCKSTATUS_AREA_NOT_EXPLORED             = 9,
    LFG_LOCKSTATUS_WRONG_FACTION                 = 10,     // LFG_INSTANCE_INVALID_WRONG_FACTION - not used in lockedReason
    LFG_LOCKSTATUS_NOT_COMLETE_CHALANGE          = 11,     // Not cpmplete chalange(scenario)
    LFG_LOCKSTATUS_NOT_HAVE_ARTIFACT             = 14,     // Not have artifact
    LFG_LOCKSTATUS_ATTUNEMENT_TOO_LOW_LEVEL      = 1001,
    LFG_LOCKSTATUS_ATTUNEMENT_TOO_HIGH_LEVEL     = 1002,
    LFG_LOCKSTATUS_QUEST_NOT_COMPLETED           = 1022,
    LFG_LOCKSTATUS_TARGET_LEVEL_HIGH             = 1023,    // need data???
    LFG_LOCKSTATUS_MISSING_ITEM                  = 1025,
    LFG_LOCKSTATUS_WRONG_TIME_RANGE              = 1029,    // not used in lockedReason
    LFG_LOCKSTATUS_WRONG_TIME                    = 1030,    // not used in lockedReason
    LFG_LOCKSTATUS_NOT_IN_SEASON                 = 1031,    // not used in lockedReason
    LFG_LOCKSTATUS_MISSING_ACHIEVEMENT           = 1034,
    LFG_LOCKSTATUS_UNK1                          = 1051,
    LFG_LOCKSTATUS_UNK2                          = 1055,
    LFG_LOCKSTATUS_TEMPORARILY_DISABLED          = 10000,
};

/// Answer state (Also used to check compatibilites)
enum LfgAnswer
{
    LFG_ANSWER_PENDING                           = -1,
    LFG_ANSWER_DENY                              = 0,
    LFG_ANSWER_AGREE                             = 1
};

/// Role Shortage Index (Seems to only use LFG_ROLE_SHORTAGE_RARE on live servers)
enum LfgShortageIndex
{
    LFG_ROLE_SHORTAGE_RARE                       = 0, // Gold icon
    LFG_ROLE_SHORTAGE_UNCOMMON                   = 1, // Silver icon
    LFG_ROLE_SHORTAGE_PLENTIFUL                  = 2, // Bronze icon
    LFG_ROLE_SHORTAGE_MAX
};

struct LockData
{
    LockData();

    uint32 status;
    uint32 currItemLevel;
    uint32 reqItemLevel;
};

typedef std::set<uint32> LfgDungeonSet;
typedef std::map<uint32, LockData> LfgLockMap;
typedef std::map<ObjectGuid, LfgLockMap> LfgLockPartyMap;
typedef std::map<ObjectGuid, uint8> LfgRolesMap;
typedef std::map<ObjectGuid, ObjectGuid> LfgGroupsMap;

std::string ConcatenateDungeons(LfgDungeonSet const& dungeons);
std::string GetRolesString(uint8 roles);
std::string GetStateString(LfgState state);
float GetShortagePercent();

} // namespace lfg

#endif
