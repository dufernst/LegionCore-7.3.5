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

#ifndef _LFGMGR_H
#define _LFGMGR_H

#include "Common.h"
#include "LFG.h"
#include "LFGQueue.h"
#include "LFGGroupData.h"
#include "LFGPlayerData.h"
#include "Packets/BattlegroundPackets.h"


namespace WorldPackets {
    namespace LFG {
        struct ProposalResponse;
    }
}

class Group;
class Player;
class Quest;

namespace lfg
{
enum LfgRoleCheckState : uint8
{
    LFG_ROLECHECK_DEFAULT           = 0,      // Internal use = Not initialized.
    LFG_ROLECHECK_FINISHED          = 1,      // Role check finished
    LFG_ROLECHECK_INITIALITING      = 2,      // + Role check begins
    LFG_ROLECHECK_FAILED_TIMEOUT    = 3,      // + Role Check failed because a group member did not respond.
    LFG_ROLECHECK_WRONG_ROLES       = 4,      // + Role Check failed because your group is not viable.
    LFG_ROLECHECK_ABORTED           = 5,      // ? Your group leader has cancelled the Role Check.
    LFG_ROLECHECK_NO_ROLE           = 6       // Silent
};

enum LfgJoinResult : uint8
{
    LFG_JOIN_OK                             = 0,      // Joined (no client msg)
    LFG_JOIN_FAILED                         = 28,     // Role Check failed because your group is not viable.
    LFG_JOIN_GROUPFULL                      = 29,     // Your group is already full.
    LFG_JOIN_INTERNAL_ERROR                 = 31,     // Internal LFG Error.
    LFG_JOIN_NOT_MEET_REQS                  = 32,     // You do not meet the requirements for the chosen dungeons.
    //LFG_JOIN_PARTY_NOT_MEET_REQS            = 33,     // One or more party members do not meet the requirements for the chosen dungeons
    LFG_JOIN_MIXED_RAID_DUNGEON             = 33,     // You cannot mix dungeons, raids, and random when picking dungeons.
    LFG_JOIN_MULTI_REALM                    = 34,     // The dungeon you chose does not support players from multiple realms.
    LFG_JOIN_DISCONNECTED                   = 35,     // One or more group members are pending invites or disconnected.
    LFG_JOIN_PARTY_INFO_FAILED              = 36,     // Could not retrieve information about some party members.
    LFG_JOIN_DUNGEON_INVALID                = 37,     // One or more dungeons was not valid.
    LFG_JOIN_DESERTER                       = 38,     // You can not queue for dungeons until your deserter debuff wears off.
    LFG_JOIN_PARTY_DESERTER                 = 39,     // One or more party members has a deserter debuff.
    LFG_JOIN_RANDOM_COOLDOWN                = 40,     // You can not queue for random dungeons while on random dungeon cooldown.
    LFG_JOIN_PARTY_RANDOM_COOLDOWN          = 41,     // One or more party members are on random dungeon cooldown.
    LFG_JOIN_TOO_MUCH_MEMBERS               = 42,     // You have too many group members to queue for that.
    LFG_JOIN_USING_BG_SYSTEM                = 43,     // You cannot queue for a dungeon or raid while using battlegrounds or arenas.
    LFG_JOIN_ROLE_CHECK_FAILED_2            = 44,     // The Role Check has failed.
    LFG_JOIN_TOO_FEW_MEMBERS                = 50,     // You do not have enough group members to queue for that.
    LFG_JOIN_FAILED_REASON_TOO_MANY_LFG     = 53,     // You are queued for too many instances.
};

enum LfgOptions
{
    LFG_OPTION_ENABLE_DUNGEON_FINDER             = 0x01,
    LFG_OPTION_ENABLE_RAID_BROWSER               = 0x02,
};

enum LfgGroup
{
    LFG_GROUP_HEROIC_WOD             = 48,
    LFG_GROUP_NORMAL_LEGION          = 52,
    LFG_GROUP_HEROIC_LEGION          = 53,
};

enum LfgQuestRequest
{
    LFG_QUEST_HEROIC_WOD_BRONZE_DAMAGE             = 37212,
    LFG_QUEST_HEROIC_WOD_SILVER_DAMAGE             = 37213,
    LFG_QUEST_HEROIC_WOD_GOLD_DAMAGE               = 37214,
    LFG_QUEST_HEROIC_WOD_BRONZE_HEALER             = 37218,
    LFG_QUEST_HEROIC_WOD_SILVER_HEALER             = 37219,
    LFG_QUEST_HEROIC_WOD_GOLD_HEALER               = 37220,
    LFG_QUEST_HEROIC_WOD_BRONZE_TANK               = 37215,
    LFG_QUEST_HEROIC_WOD_SILVER_TANK               = 37216,
    LFG_QUEST_HEROIC_WOD_GOLD_TANK                 = 37217,
};

enum LFGMgrEnum
{
    LFG_TIME_ROLECHECK                           = 45 * IN_MILLISECONDS,
    LFG_TIME_BOOT                                = 30,
    LFG_TIME_PROPOSAL                            = 45,
    LFG_QUEUEUPDATE_INTERVAL                     = 15 * IN_MILLISECONDS,
    LFG_SPELL_DUNGEON_COOLDOWN                   = 71328,
    LFG_SPELL_DUNGEON_DESERTER                   = 71041,
    LFG_SPELL_LUCK_OF_THE_DRAW                   = 72221,

    LFG_DUNGEON_KICK_VOTES_NEEDED                = 3,
    LFG_RAID_KICK_VOTES_NEEDED                   = 15,
    LFG_SCENARIO_KICK_VOTES_NEEDED               = 2,
};

/// Proposal states
enum LfgProposalState
{
    LFG_PROPOSAL_INITIATING                      = 0,
    LFG_PROPOSAL_FAILED                          = 1,
    LFG_PROPOSAL_SUCCESS                         = 2
};

/// Teleport errors
enum LfgTeleportError
{
    LFG_TELEPORTERROR_OK                         = 0,      // Internal use
    LFG_TELEPORTERROR_PLAYER_DEAD                = 1,
    LFG_TELEPORTERROR_FALLING                    = 2,
    LFG_TELEPORTERROR_IN_VEHICLE                 = 3,
    LFG_TELEPORTERROR_FATIGUE                    = 4,
    LFG_TELEPORTERROR_INVALID_LOCATION           = 6,
    LFG_TELEPORTERROR_CHARMING                   = 8       // FIXME - It can be 7 or 8 (Need proper data)
};

// Forward declaration (just to have all typedef together)
struct LFGDungeonData;
struct LfgReward;
struct LfgRoleCheck;
struct LfgProposal;
struct LfgProposalPlayer;
struct LfgPlayerBoot;

typedef std::map<uint16, LFGQueue> LfgQueueContainer;
typedef std::multimap<uint32, LfgReward const*> LfgRewardContainer;
typedef std::map<uint16, LfgDungeonSet> LfgCachedDungeonContainer;
typedef std::map<ObjectGuid, LfgAnswer> LfgAnswerContainer;
typedef std::map<ObjectGuid, LfgRoleCheck> LfgRoleCheckContainer;
typedef std::map<uint32, LfgProposal> LfgProposalContainer;
typedef std::map<ObjectGuid, LfgProposalPlayer> LfgProposalPlayerContainer;
typedef std::map<ObjectGuid, LfgPlayerBoot> LfgPlayerBootContainer;
typedef std::map<ObjectGuid, LfgGroupData> LfgGroupDataContainer;
typedef std::map<ObjectGuid, std::map<uint32, LfgPlayerData>> LfgPlayerDataContainer;
typedef std::unordered_map<uint32, LFGDungeonData> LFGDungeonContainer;
typedef std::vector<LFGDungeonData*> LFGDungeonVector;
typedef std::map<ObjectGuid, uint8> LfgCTARewardContainer;
typedef std::map<ObjectGuid, uint32> LfgCompletedMaskContainer;
typedef std::map<ObjectGuid, std::set<uint32>> DungeonSet;

struct LfgJoinResultData
{
    explicit LfgJoinResultData(LfgJoinResult _result = LFG_JOIN_OK, LfgRoleCheckState _state = LFG_ROLECHECK_DEFAULT, uint32 _queueId = 0);

    LfgJoinResult result;
    LfgRoleCheckState state;
    LfgLockPartyMap lockmap;
    uint32 queueId;
};

// Data needed by SMSG_LFG_UPDATE_PARTY and SMSG_LFG_UPDATE_PLAYER
struct LfgUpdateData
{
    explicit LfgUpdateData(LfgUpdateType _type = LFG_UPDATETYPE_DEFAULT);
    LfgUpdateData(LfgUpdateType _type, LfgDungeonSet _dungeons);
    LfgUpdateData(LfgUpdateType _type, LfgState _state, LfgDungeonSet ons);

    LfgUpdateType updateType;
    LfgState state;
    LfgDungeonSet dungeons;
};

// Data needed by SMSG_LFG_QUEUE_STATUS
struct LfgQueueStatusData
{
    explicit LfgQueueStatusData(uint32 _dungeonId = 0, int32 _waitTime = -1, int32 _waitTimeAvg = -1, int32 _waitTimeTank = -1, int32 _waitTimeHealer = -1, int32 _waitTimeDps = -1, uint32 _queuedTime = 0, LfgQueueData* _queueInfo = nullptr);

    LfgQueueData* queueInfo;
    uint32 dungeonId;
    int32 waitTime;
    int32 waitTimeAvg;
    int32 waitTimeTank;
    int32 waitTimeHealer;
    int32 waitTimeDps;
    uint32 queuedTime;
};

struct LfgPlayerRewardData
{
    LfgPlayerRewardData(uint32 random, uint32 current, bool _done, LfgReward const* _reward);

    LfgReward const* reward;
    uint32 rdungeonEntry;
    uint32 sdungeonEntry;
    bool done;
};

/// Reward info
struct LfgReward
{
    LfgReward(uint32 _maxLevel = 0, uint32 _firstQuest = 0, uint32 _otherQuest = 0, uint32 _bonusQuestId = 0, uint32 _encounterMask = 0);

    bool RewardPlayer(Player* player, LFGDungeonData const* randomDungeon, uint32 dungeonId) const;

    uint32 maxLevel;
    uint32 firstQuest;
    uint32 otherQuest;
    uint32 bonusQuestId;
    uint32 encounterMask;
};

/// Stores player data related to proposal to join
struct LfgProposalPlayer
{
    LfgProposalPlayer();
    uint8 role;                                            ///< Proposed role
    LfgAnswer accept;                                      ///< Accept status (-1 not answer | 0 Not agree | 1 agree)
    ObjectGuid group;                                      ///< Original group guid. 0 if no original group
};

/// Stores group data related to proposal to join
struct LfgProposal
{
    LfgProposal(uint32 dungeon = 0);

    uint32 id;                                             ///< Proposal Id
    uint32 dungeonId;                                      ///< Dungeon to join
    uint32 queueId{};                                      ///< Queue Id
    LfgProposalState state;                                ///< State of the proposal
    ObjectGuid group;                                      ///< Proposal group (0 if new)
    ObjectGuid leader;                                     ///< Leader guid.
    time_t cancelTime;                                     ///< Time when we will cancel this proposal
    uint32 encounters;                                     ///< Dungeon Encounters
    bool isNew;                                            ///< Determines if it's new group or not
    GuidList queues;                                       ///< Queue Ids to remove/readd
    GuidList showorder;                                    ///< Show order in update window
    LfgProposalPlayerContainer players;                    ///< Players data
};

/// Stores all rolecheck info of a group that wants to join
struct LfgRoleCheck
{
    time_t cancelTime;                                     ///< Time when the rolecheck will fail
    LfgRolesMap roles;                                     ///< Player selected roles
    LfgRoleCheckState state;                               ///< State of the rolecheck
    LfgDungeonSet dungeons;                                ///< Dungeons group is applying for (expanded random dungeons)
    ObjectGuid leader;                                     ///< Leader of the group
    uint32 rDungeonId;                                     ///< Random Dungeon Id.
    uint32 queueId{};                                      ///< Queue Id

    uint32 bgQueueId{};                                    ///< For BG System
    uint8 bgQueueTypeId{};
    WorldPackets::Battleground::IgnorMapInfo ignormap;
    bool isSkirmish = false;
};

/// Stores information of a current vote to kick someone from a group
struct LfgPlayerBoot
{
    LfgAnswerContainer votes;                              ///< Player votes (-1 not answer | 0 Not agree | 1 agree)
    ObjectGuid victim;                                     ///< Player guid to be kicked (can't vote)
    time_t cancelTime;                                     ///< Time left to vote
    std::string reason;                                    ///< kick reason
    uint32 queueId{};                                      ///< Queue Id
    uint8 votesNeeded;
    bool inProgress;                                       ///< Vote in progress
};

struct LfgRoleData
{
    explicit LfgRoleData(LFGDungeonData const* data);
    explicit LfgRoleData(uint32 dungeonId);

    void Init(LFGDungeonData const* data);

    uint8 tanksNeeded;
    uint8 healerNeeded;
    uint8 dpsNeeded;
    uint8 minTanksNeeded;
    uint8 minHealerNeeded;
    uint8 minDpsNeeded;
};

struct LFGDungeonData
{
    LFGDungeonData();
    explicit LFGDungeonData(LFGDungeonsEntry const* _dbc);

    LFGDungeonsEntry const* dbc;
    uint32 id;
    uint32 random_id;
    float x, y, z, o;
    std::string name;
    int16 map;
    uint8 type;
    uint8 expansion;
    uint8 minlevel;
    uint8 maxlevel;
    uint8 difficulty;
    uint8 internalType;
    bool seasonal;
};

class LFGMgr
{
    LFGMgr();
    ~LFGMgr();

public:
    static LFGMgr* instance();
    void Update(uint32 diff);

    void FinishDungeon(ObjectGuid gguid, uint32 dungeonId);
    void LoadRewards();
    void LoadLFGDungeons(bool reload = false);

    bool selectedRandomLfgDungeon(ObjectGuid guid, uint32 queueId);
    bool inLfgDungeonMap(ObjectGuid guid, uint32 map, Difficulty difficulty, uint32& queueId);
    LfgDungeonSet const& GetSelectedDungeons(ObjectGuid guid, uint32 queueId);
    LfgState GetState(ObjectGuid guid, uint32 queueId);
    uint32 GetDungeon(ObjectGuid guid, bool asId = true);
    uint32 GetDungeonMapId(ObjectGuid guid);
    uint8 GetKicksLeft(ObjectGuid gguid);
    void _LoadFromDB(Field* fields, ObjectGuid guid);
    void SetupGroupMember(ObjectGuid guid, ObjectGuid gguid);
    uint32 GetLFGDungeonEntry(uint32 id);
    LFGDungeonData const* GetLFGDungeon(uint32 id);
    LFGDungeonData const* GetLFGDungeon(uint16 scenarioId, uint16 mapId);
    LFGDungeonData const* GetLFGDungeon(uint32 id, uint32 team);
    LFGDungeonData const* GetLFGDungeon(uint32 mapId, Difficulty diff, uint32 team);

    uint8 GetRoles(ObjectGuid guid, uint32 queueId);
    uint32 GetOptions();
    void SetOptions(uint32 options);
    bool isOptionEnabled(uint32 option);
    void Clean();
    std::string DumpQueueInfo(bool full = false);

    ObjectGuid GetLeader(ObjectGuid guid);
    void SetTeam(ObjectGuid guid, uint8 team, uint32 queueId);
    void SetGroup(ObjectGuid guid, ObjectGuid group, uint32 queueId);
    void SetLfgGroup(ObjectGuid guid, ObjectGuid group, uint32 queueId);
    ObjectGuid GetGroup(ObjectGuid guid, uint32 queueId);
    ObjectGuid GetLfgGroup(ObjectGuid guid, uint32 queueId);
    void SetLeader(ObjectGuid gguid, ObjectGuid leader);
    void RemoveGroupData(ObjectGuid guid);
    uint8 RemovePlayerFromGroup(ObjectGuid gguid, ObjectGuid guid);
    void AddPlayerToGroup(ObjectGuid gguid, ObjectGuid guid);
    void ClearState(ObjectGuid guid, uint32 queueId);

    LfgUpdateData GetLfgStatus(ObjectGuid guid, uint32 queueId);
    bool IsSeasonActive(uint32 dungeonId);
    LfgReward const* GetDungeonReward(uint32 dungeon, uint8 level);
    LfgDungeonSet GetRewardableDungeons(uint8 level, uint8 expansion);
    void TeleportPlayer(Player* player, bool out, bool fromOpcode = false);
    void InitBoot(ObjectGuid gguid, ObjectGuid kguid, ObjectGuid vguid, std::string const& reason);
    void UpdateBoot(ObjectGuid gguid, ObjectGuid guid, bool accept);
    void UpdateProposal(WorldPackets::LFG::ProposalResponse response, ObjectGuid RequesterGuid);
    void UpdateRoleCheck(ObjectGuid gguid, ObjectGuid guid = ObjectGuid::Empty, uint8 roles = PLAYER_ROLE_NONE, uint8 partyIndex = 0);
    void SetRoles(ObjectGuid guid, uint8 roles, uint32 queueId);
    void JoinLfg(Player* player, uint8 roles, LfgDungeonSet& dungeons);
    void LeaveLfg(ObjectGuid guid, uint32 queueId = 0);
    WorldPackets::LFG::RideTicket const* GetTicket(ObjectGuid guid, uint32 queueId) const;

    LfgLockMap GetLockedDungeons(ObjectGuid guid);
    void SendLfgPlayerLockInfo(Player* player);
    void SendLfgPartyLockInfo(Player* player);

    LfgState GetOldState(ObjectGuid guid, uint32 queueId);
    uint8 GetQueueTeam(ObjectGuid guid, uint32 queueId);
    uint32 GetQueueId(uint32 dungeonId);
    LFGQueue& GetQueue(ObjectGuid guid, uint32 queueId);
    uint32 GetCompletedMask(ObjectGuid guid);
    void SetCompletedMask(ObjectGuid guid, uint32 mask);
    void RemoveFromQueue(ObjectGuid guid, uint32 queueId);
    void RemoveFromGroupQueue(ObjectGuid guid, uint32 queueId);
    bool HasQueue(ObjectGuid guid);
    bool HasGroupQueue(ObjectGuid guid);
    uint32 GetQueueId(ObjectGuid guid, uint32 dungeonId = 0);

    bool IsLfgGroup(ObjectGuid guid);
    uint8 GetPlayerCount(ObjectGuid guid);
    uint32 AddProposal(LfgProposal& proposal);
    bool AllQueued(GuidList const& check, uint32 queueId);
    static bool CheckGroupRoles(LfgRolesMap& groles, LfgRoleData const& roleData, uint32 &n, bool removeLeaderFlag = true);
    static bool HasIgnore(ObjectGuid guid1, ObjectGuid guid2);
    void SendLfgQueueStatus(ObjectGuid guid, LfgQueueStatusData const& data);
    void SendUpdateStatus(ObjectGuid guid, LfgUpdateData const& updateData, bool Suspended);

    uint8 GetVotesNeededForKick(ObjectGuid gguid);

    void ToggleTesting();
    bool onTest() const;

    void SendLfgBootProposalUpdate(ObjectGuid guid, LfgPlayerBoot const& boot);
    void SendLfgJoinResult(ObjectGuid guid, LfgJoinResultData const& data);
    void SendLfgRoleChosen(ObjectGuid guid, ObjectGuid pguid, uint8 roles);
    void SendLfgRoleCheckUpdate(ObjectGuid guid, LfgRoleCheck const& roleCheck, uint8 partyIndex);
    void SendLfgUpdateParty(ObjectGuid guid, LfgUpdateData const& data);
    void SendLfgUpdatePlayer(ObjectGuid guid, LfgUpdateData const& data, bool Suspended = false);
    void SendLfgUpdateProposal(ObjectGuid guid, LfgProposal const& proposal);
    bool HasPlayerData(ObjectGuid guid, uint32 queueId);
    void StartAllOtherQueue(ObjectGuid guid, uint32 queueId);
    void StopAllOtherQueue(ObjectGuid guid, uint32 queueId);
    void SendLfgUpdateQueue(ObjectGuid guid);

    uint8 GetShortageRolesForQueue(ObjectGuid guid, uint32 dungeonId);
    uint8 GetEligibleRolesForCTA(ObjectGuid guid);

    GuidSet const& GetPlayers(ObjectGuid guid);

    void InitiBattlgroundCheckRoles(Group* group, ObjectGuid playerGuid, uint32 queueid, uint8 roles, uint8 bgQueueTypeId, WorldPackets::Battleground::IgnorMapInfo ignormap, bool isSkirmish = false);


private:
    void SetTicket(ObjectGuid guid, WorldPackets::LFG::RideTicket const& ticket, uint32 queueId);
    uint8 GetTeam(ObjectGuid guid, uint32 queueId);
    void RestoreState(ObjectGuid guid, char const* debugMsg, uint32 queueId);
    void SetDungeon(ObjectGuid guid, uint32 dungeon);
    void SetSelectedDungeons(ObjectGuid guid, LfgDungeonSet const& dungeons, uint32 queueId);
    void DecreaseKicksLeft(ObjectGuid guid);
    void SetState(ObjectGuid guid, LfgState state, uint32 queueId);
    void SetEligibleForCTAReward(ObjectGuid guid, uint8 roles);
    void RemovePlayerData(ObjectGuid guid, uint32 queueId);
    void GetCompatibleDungeons(LfgDungeonSet& dungeons, GuidSet const& players, LfgLockPartyMap& lockMap);
    void _SaveToDB(ObjectGuid guid, uint32 db_guid);
    void SetQueueId(ObjectGuid guid, uint32 queueId);

    void RemoveProposal(LfgProposalContainer::iterator itProposal, LfgUpdateType type);
    void MakeNewGroup(LfgProposal const& proposal);

    LfgDungeonSet const& GetDungeonsByRandom(uint32 randomdungeon);
    LfgType GetDungeonType(uint32 dungeon);

    uint32 m_QueueTimer;                               ///< used to check interval of update
    uint32 m_ShortageCheckTimer;                       ///< used to check interval of shortage check
    uint32 m_lfgProposalId;                            ///< used as internal counter for proposals
    uint32 m_options;                                  ///< Stores config options
    LfgQueueContainer QueuesStore[ALL_TEAMS];          ///< Queues
    LfgCachedDungeonContainer CachedDungeonMapStore;   ///< Stores all dungeons by groupType
    LfgRewardContainer RewardMapStore;                 ///< Stores rewards for random dungeons
    LFGDungeonContainer  LfgDungeonStore;
    LFGDungeonVector  LfgDungeonVStore;
    LfgRoleCheckContainer RoleChecksStore;             ///< Current Role checks
    LfgProposalContainer ProposalsStore;               ///< Current Proposals
    LfgPlayerBootContainer BootsStore;                 ///< Current player kicks
    LfgPlayerDataContainer PlayersStore;               ///< Player data
    LfgGroupDataContainer GroupsStore;                 ///< Group data
    LfgCompletedMaskContainer CompletedMaskStore;      ///< Instance Completed Encounter Mask
    DungeonSet PlayerDungeons;
    DungeonSet GroupDungeons;
    std::recursive_mutex m_lock;
    bool m_Testing = false;

    LfgCTARewardContainer CTARewardStore;              ///< Player selecter roles which were eligible for CTA reward when joining a queue
};

} // namespace lfg

#define sLFGMgr lfg::LFGMgr::instance()
#endif
