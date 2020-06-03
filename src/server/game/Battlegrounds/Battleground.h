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

#ifndef __BATTLEGROUND_H
#define __BATTLEGROUND_H

#include "Common.h"
#include "SharedDefines.h"
#include "BattlegroundMap.h"
#include "DBCEnums.h"
#include "Duration.h"
#include "ArenaScore.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "BattlegroundDefines.h"
#include <map>

class SpectatorAddonMsg;
class BattlegroundMap;
class Creature;
class GameObject;
class Group;
class Player;
class Transport;
class Unit;
class WorldObject;
class WorldPacket;
struct BattlegroundScore;
struct PVPDifficultyEntry;
struct WorldSafeLocsEntry;

namespace WorldPackets
{
    namespace Battleground
    {
        class PVPLogData;
        struct BattlegroundPlayerPosition;
    }

    namespace WorldState
    {
        class InitWorldStates;
    }
}

enum BattlegroundMisc
{
    BG_EVENT_START_BATTLE           = 35912,
};

enum BattlegroundSounds
{
    BG_SOUND_START                              = 3439,
    BG_SOUND_NEAR_VICTORY                       = 8456,
    BG_SOUND_FLAG_RESET                         = 8192,

    BG_SOUND_ALLIANCE_WIN                       = 8455,
    BG_SOUND_HORDE_WIN                          = 8454,
    
    BG_SOUND_CAPTURE_POINT_ASSAULT_HORDE        = 8212,
    BG_SOUND_CAPTURE_POINT_ASSAULT_ALLIANCE     = 8174,

    BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE       = 8213,
    BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE    = 8173,

    BG_SOUND_FLAG_PLACED_ALLIANCE               = 8232,
    BG_SOUND_FLAG_PLACED_HORDE                  = 8333,
};

enum BattlegroundQuests
{
    SPELL_WS_QUEST_REWARD           = 43483,
    SPELL_AB_QUEST_REWARD           = 43484,
    SPELL_AV_QUEST_REWARD           = 43475,
    SPELL_AV_QUEST_KILLED_BOSS      = 23658,
    SPELL_EY_QUEST_REWARD           = 43477,
    SPELL_SA_QUEST_REWARD           = 61213,
    SPELL_AB_QUEST_REWARD_4_BASES   = 24061,
    SPELL_AB_QUEST_REWARD_5_BASES   = 24064
};

enum BattlegroundCreatures
{
    BG_CREATURE_ENTRY_A_SPIRITGUIDE      = 13116,           // alliance
    BG_CREATURE_ENTRY_H_SPIRITGUIDE      = 13117,           // horde
};

enum BattlegroundFactions
{
    BG_FACTION_ALLIANCE     = 1732,
    BG_FACTION_HORDE        = 1735,
    BG_FACTION_VILLIAN      = 35,

    BG_MAX_FACTIONS         = 2,
    BF_MAX_FACTIONS         = 3,
};

static uint32 const BgFactions[BG_MAX_FACTIONS] = { BG_FACTION_ALLIANCE, BG_FACTION_HORDE };
static uint32 const BfFactions[BF_MAX_FACTIONS] = { BG_FACTION_ALLIANCE, BG_FACTION_HORDE, BG_FACTION_VILLIAN };

enum BattlegroundSpells
{
    SPELL_BG_FOCUSED_ASSAULT            = 46392,
    SPELL_BG_BRUTAL_ASSAULT             = 46393,

    SPELL_BG_HORDE_FLAG                 = 156618,   // Old Spell: 23333
    SPELL_BG_HORDE_FLAG_DROPPED         = 23334,
    SPELL_BG_HORDE_FLAG_PICKED_UP       = 61266,    ///< Fake Spell - Used as a start timer event
    SPELL_BG_HORDE_GOLD_FLAG            = 35774,
    SPELL_BG_HORDE_GREEN_FLAG           = 35775,

    SPELL_BG_ALLIANCE_FLAG              = 156621,   // Old Spell: 23335
    SPELL_BG_ALLIANCE_FLAG_DROPPED      = 23336,
    SPELL_BG_ALLIANCE_FLAG_PICKED_UP    = 61265,    ///< Fake Spell - Used as a start timer event
    SPELL_BG_ALLIANCE_GOLD_FLAG         = 32724,
    SPELL_BG_ALLIANCE_GREEN_FLAG        = 32725,

    SPELL_WAITING_FOR_RESURRECT         = 2584,
    SPELL_BG_SPIRIT_HEAL_CHANNEL        = 22011,
    SPELL_SPIRIT_HEAL                   = 22012,
    SPELL_RESURRECTION_VISUAL           = 24171,
    SPELL_PET_SUMMONED                  = 6962,
    SPELL_BG_PREPARATION                = 44521,
    SPELL_SPIRIT_HEAL_MANA              = 44535,
    SPELL_BG_RECENTLY_DROPPED_FLAG      = 42792,
    SPELL_BG_AURA_PLAYER_INACTIVE       = 43681,
    SPELL_BG_HONORABLE_DEFENDER_25Y     = 68652,
    SPELL_BG_HONORABLE_DEFENDER_60Y     = 66157,
    SPELL_BG_THE_LAST_STANDING          = 26549,
    SPELL_BG_ARENA_DUMPENING            = 110310,
    SPELL_PVP_RULES_ENABLED             = 134735,
    SPELL_PVP_STATS_TEMPLATE            = 198439,
    
    SPELL_ARENA_PREPARATION             = 32727,
    SPELL_ARENA_PERIODIC_AURA           = 74410,
    SPELL_ENTERING_BATTLEGROUND         = 91318,
    SPELL_RATED_PVP_TRANSFORM_SUPPRESSION = 182306,

    SPELL_BG_SET_FACTION_ALLIANCE       = 195843,
    SPELL_BG_SET_FACTION_HORDE          = 195838,
    
    SPELL_BG_MORPH_FACTION_HORDE        = 193475,    
    SPELL_BG_MORPH_FACTION_ALLIANCE     = 193472,    

    SPELL_PRINCIPLES_OF_WAR             = 197912,
    SPELL_PRINCIPLES_OF_WAR_FROM_DUMMY  = 228696,
    SPELL_DISABLE_ITEM_EFFECTS_IN_PVP   = 198147,

    SPELL_BG_DESERTER                   = 26013,  // Battleground Deserter Spell
    SPELL_BG_CRAVEN                     = 158263, // Arena Deserter Spell
    SPELL_BG_LEVEL_OF_CRAVEN            = 158950, // Hidden spell for apply diminishing to arena deserter duration
};

static Milliseconds const PositionBroadcastUpdate = Seconds(5);

enum BattlegroundTimeIntervals
{
    RESURRECTION_INTERVAL           = 30000,                // ms
    RESURRECTION_INTERVAL_BRAWL     = 20000,                // ms
    RESURRECTION_INTERVAL_HOTMOGU   = 5000,                 // ms
    INVITATION_REMIND_TIME          = 20000,                // ms
    BG_INVITE_ACCEPT_WAIT_TIME      = 90000,                // ms
    ARENA_INVITE_ACCEPT_WAIT_TIME   = 30000,                // ms
    TIME_AUTOCLOSE_BATTLEGROUND     = 120000,               // ms
    MAX_OFFLINE_TIME                = 60,                   // secs
    RESPAWN_ONE_DAY                 = 86400,                // secs
    RESPAWN_IMMEDIATELY             = 0,                    // secs
    BUFF_RESPAWN_TIME               = 180,                  // secs
    BATTLEGROUND_COUNTDOWN_MAX      = 120,                  // secs
    ARENA_COUNTDOWN_MAX             = 60,                   // secs
    ARENA_2V2_COUNTDOWN             = 40,                   // secs
    ARENA_1V1_COUNTDOWN             = 30                    // secs
};

enum BattlegroundBuffObjects
{
    BG_OBJECTID_SPEEDBUFF_ENTRY     = 179871,
    BG_OBJECTID_REGENBUFF_ENTRY     = 179904,
    BG_OBJECTID_BERSERKERBUFF_ENTRY = 179905
};

static uint32 const Buff_Entries[3] = { BG_OBJECTID_SPEEDBUFF_ENTRY, BG_OBJECTID_REGENBUFF_ENTRY, BG_OBJECTID_BERSERKERBUFF_ENTRY };
static Milliseconds const m_messageTimer[4] = { Minutes(2), Minutes(1), Seconds(30), Seconds(0) };

enum BattlegroundStatus
{
    STATUS_NONE         = 0,                                // first status, should mean bg is not instance
    STATUS_WAIT_QUEUE   = 1,                                // means bg is empty and waiting for queue
    STATUS_WAIT_JOIN    = 2,                                // this means, that BG has already started and it is waiting for more players
    STATUS_IN_PROGRESS  = 3,                                // means bg is running
    STATUS_WAIT_LEAVE   = 4                                 // means some faction has won BG and it is ending
};

struct BattlegroundPlayer
{
    time_t OfflineRemoveTime = 0; // for tracking and removing offline players from queue after 5 minutes
    uint32 Team = TEAM_OTHER;
    uint32 PrestigeRank = 0;
    uint32 PrimaryTalentTreeNameIndex = 0;
    int32 ActiveSpec = 0;
    uint8 Role = 0;
};

struct ArenaPlayerInfo
{
    std::string PlayerName;
    std::string PlayerIP;
    uint64 HWId;
    uint8 IPMark;
    uint8 HWIdMark;
    uint8 PlayerID;
    uint32 PlayersCheckedMask;
};

enum BattlegroundWinner
{
    WINNER_HORDE            = 0,
    WINNER_ALLIANCE         = 1,
    WINNER_NONE             = 2
};

enum BattlegroundStartingEvents
{
    BG_STARTING_EVENT_NONE  = 0x00,
    BG_STARTING_EVENT_1     = 0x01,
    BG_STARTING_EVENT_2     = 0x02,
    BG_STARTING_EVENT_3     = 0x04,
    BG_STARTING_EVENT_4     = 0x08
};

enum BattlegroundStartingEventsIds
{
    BG_STARTING_EVENT_FIRST     = 0,
    BG_STARTING_EVENT_SECOND    = 1,
    BG_STARTING_EVENT_THIRD     = 2,
    BG_STARTING_EVENT_FOURTH    = 3,

    BG_STARTING_EVENT_COUNT
};

static uint32 const ArenaBroadcastTexts[BG_STARTING_EVENT_COUNT] = { 15740, 15741, 15739, 15742 };
static uint32 const BattlegroundBroadcastTexts[BG_STARTING_EVENT_COUNT] = { 71789, 71790, 71791, 71792 };

enum BgNodeStatus
{
    NODE_STATUS_NEUTRAL = 0,
    NODE_STATUS_ASSAULT = 1,
    NODE_STATUS_CAPTURE = 2
};

enum BattlegroundPlayerPositionConstants
{
    PLAYER_POSITION_ICON_NONE           = 0,
    PLAYER_POSITION_ICON_HORDE_FLAG     = 1,
    PLAYER_POSITION_ICON_ALLIANCE_FLAG  = 2,

    PLAYER_POSITION_ARENA_SLOT_NONE     = 1,
    PLAYER_POSITION_ARENA_SLOT_1        = 2,
    PLAYER_POSITION_ARENA_SLOT_2        = 3,
    PLAYER_POSITION_ARENA_SLOT_3        = 4,
    PLAYER_POSITION_ARENA_SLOT_4        = 5,
    PLAYER_POSITION_ARENA_SLOT_5        = 6
};

struct CreateBattlegroundData;

class Battleground
{
public:
    Battleground();
    virtual ~Battleground();

    void Update(uint32 diff);

    virtual bool SetupBattleground() { return true; } 
    virtual void Reset();
    virtual void StartingEventCloseDoors() { }
    virtual void StartingEventOpenDoors() { }
    virtual void ResetBGSubclass() { }
    virtual void DestroyGate(Player* /*player*/, GameObject* /*go*/) { }

    virtual bool IsAllNodesConrolledByTeam(uint32 /*team*/) const { return false; }
    bool IsTeamScoreInRange(uint32 team, uint32 minScore, uint32 maxScore) const;
    void StartTimedAchievement(CriteriaTimedTypes type, uint32 entry);

    char const* GetName() const { return m_Name; }
    uint64 GetQueueID() const { return m_QueueID; }

    Seconds CalcArenaCountdown(uint8 joinType) const;

    uint16 GetTypeID(bool GetRandom = false) const;
    uint8 GetBracketId() const { return m_BracketId; }
    uint32 GetInstanceID() const { return m_InstanceID; }
    BattlegroundStatus GetStatus() const { return m_Status; }
    uint32 GetClientInstanceID() const { return m_ClientInstanceID; }
    Milliseconds GetElapsedTime() const { return m_StartTime; }
    Milliseconds GetRemainingTime() const { return m_EndTime; }
    uint32 GetLastResurrectTime() const { return m_LastResurrectTime; }
    uint32 GetMaxPlayers() const { return m_MaxPlayers; }
    uint32 GetMinPlayers() const { return m_MinPlayers; }

    uint32 GetMinLevel() const { return m_LevelMin; }
    uint32 GetMaxLevel() const { return m_LevelMax; }

    uint32 GetMaxPlayersPerTeam() const { return m_MaxPlayersPerTeam; }
    uint32 GetMinPlayersPerTeam() const { return m_MinPlayersPerTeam; }

    Milliseconds GetStartDelayTime() const { return m_StartDelayTime; }
    uint8 GetJoinType() const { return m_JoinType; }
    uint8 GetBrawlJoinType() const { return m_BrawlJoinType; }
    uint32 GetScriptId() const { return ScriptId; }
    uint32 GetBonusHonorFromKill(uint32 kills) const;

    void SetQueueID(uint64 newID) { m_QueueID = newID; }
    void SetTypeID(uint16 TypeID) { m_TypeID = TypeID; }
    void SetRandomTypeID(uint16 TypeID) { m_RandomTypeID = TypeID; }
    void SetBracket(PVPDifficultyEntry const* bracketEntry);
    void SetInstanceID(uint32 InstanceID) { m_InstanceID = InstanceID; }
    void SetStatus(BattlegroundStatus Status) { m_Status = Status; }
    void SetClientInstanceID(uint32 InstanceID) { m_ClientInstanceID = InstanceID; }
    void SetLevelRange(uint32 min, uint32 max) { m_LevelMin = min; m_LevelMax = max; }
    void SetRated(bool state) { m_IsRated = state; }
    void SetSkirmish(bool state) { _isSkirmish = state; }
    void SetTournamentRules(bool state) { _useTournamentRules = state; }
    void SetWargame(bool state) { _isWargame = state; }
    void SetJoinType(uint8 type) { m_JoinType = type; }
    void SetBrawlJoinType(uint8 type) { m_BrawlJoinType = type; }
    void SetWinner(uint8 winner) { m_Winner = winner; }
    void SetArena(bool _arena) { m_IsArena = _arena; }

    void ModifyStartDelayTime(Milliseconds diff) { m_StartDelayTime -= diff; }
    void SetStartDelayTime(Milliseconds Time) { m_StartDelayTime = Time; }

    void AddToBGFreeSlotQueue();
    void RemoveFromBGFreeSlotQueue();

    void DecreaseInvitedCount(uint32 team);
    void IncreaseInvitedCount(uint32 team);
    uint32 GetInvitedCount(uint32 team) const;
    bool HasFreeSlots() const;
    uint32 GetFreeSlotsForTeam(uint32 Team) const;

    bool IsBG() const { return m_IsBG; }
    bool IsArena() const { return m_IsArena; }
    bool IsBattleground() const { return !m_IsArena; }
    bool IsRBG() const { return m_IsRBG; }
    bool IsRated() const { return m_IsRated; }
    bool UseTournamentRules() const { return _useTournamentRules; }
    bool IsSkirmish() const { return m_IsArena && !m_IsRated && !m_IsRBG && !_isBrawl/*_isSkirmish*/; }
    bool IsWargame() const { return _isWargame; }
    bool IsBrawl() const { return _isBrawl; }
    void SetIsBrawl(bool set) { _isBrawl = set; }

    std::map<ObjectGuid, BattlegroundPlayer> const& GetPlayers() const { return _players; }
    uint32 GetPlayerScoresSize() const { return PlayerScores.size(); }
    std::map<ObjectGuid, BattlegroundScore*> const& GetBattlegroundScoreMap() { return PlayerScores; }
    uint32 GetReviveQueueSize() const { return m_ReviveQueue.size(); }

    void AddPlayerToResurrectQueue(ObjectGuid npc_guid, ObjectGuid playerGUID);
    void RemovePlayerFromResurrectQueue(ObjectGuid playerGUID);

    virtual void RelocateDeadPlayers(ObjectGuid guideGuid);

    void StartBattleground();

    GameObject* GetBGObject(uint32 type);
    Creature* GetBGCreature(uint32 type);

    void Initialize(CreateBattlegroundData const* data);

    uint32 GetMapId() const;
    void SetBgMap(BattlegroundMap* map);
    BattlegroundMap* GetBgMap() const;
    BattlegroundMap* FindBgMap() const;

    virtual Position const* GetTeamStartPosition(TeamId teamId);

    virtual void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }
    void SendPacketToTeam(uint32 TeamID, WorldPacket const* packet, Player* sender = nullptr, bool self = true);
    void SendPacketToAll(WorldPacket const* packet);
    void YellToAll(Creature* creature, const char* text, uint32 language);

    template<class Do>
    void BroadcastWorker(Do& _do);

    void PlaySoundToTeam(uint32 SoundID, uint32 TeamID);
    void PlaySoundToAll(uint32 SoundID, ObjectGuid sourceGuid = ObjectGuid::Empty);
    void SendChatMessage(Creature * source, uint8 textId, WorldObject * target = nullptr);
    void CastSpellOnTeam(uint32 SpellID, uint32 TeamID);
    void RemoveAuraOnTeam(uint32 SpellID, uint32 TeamID);
    void RewardHonorToTeam(uint32 Honor, uint32 TeamID);
    void RewardReputationToTeam(uint32 factionIDAlliance, uint32 factionIDHorde, uint32 reputation, uint32 teamID);
    void UpdateWorldState(uint32 field, uint32 value, bool hidden = false);
    void PlayerReward(Player* player, bool isWinner);
    virtual void EndBattleground(uint32 winner);
    bool HandleArenaLogPlayerNames(std::string& Team1, std::string& Team2, std::string Marks, uint32 winner = 0);


    void SendBattleGroundPoints(bool isHorde, int32 teamScores, bool broadcast = true, Player* player = nullptr);

    void BlockMovement(Player* player);

    void PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...);
    void SendBroadcastText(int32 broadcastTextID, ChatMsg type, WorldObject const* unit = nullptr);

    Group* GetBgRaid(uint32 TeamID) const { return TeamID == ALLIANCE ? m_BgRaids[TEAM_ALLIANCE] : m_BgRaids[TEAM_HORDE]; }
    void SetBgRaid(uint32 TeamID, Group* bg_raid);

    void BuildPvPLogDataPacket(WorldPackets::Battleground::PVPLogData& pvpLogData);
    virtual bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true);

    uint32 GetPlayersCountByTeam(uint32 Team) const;
    uint32 GetPlayerScoreByType(Player* player, uint32 type) const;
    uint32 GetAlivePlayersCountByTeam(uint32 Team) const;
    void UpdatePlayersCountByTeam(uint32 Team, bool remove);

    virtual void CheckWinConditions() { }

    void SetGroupForTeam(uint32 Team, uint64 GroupId) { m_GroupIds[MS::Battlegrounds::GetTeamIdByTeam(Team)] = GroupId; }
    uint64 GetGroupIdForTeam(uint32 Team) const { return m_GroupIds[MS::Battlegrounds::GetTeamIdByTeam(Team)]; }
    uint64 GetGroupIdByIndex(uint32 index) const { return m_GroupIds[index]; }
    void SetMatchmakerRating(uint32 Team, uint32 MMR) { m_ArenaTeamMMR[MS::Battlegrounds::GetTeamIdByTeam(Team)] = MMR; }
    uint32 GetMatchmakerRating(uint32 Team) const { return m_ArenaTeamMMR[MS::Battlegrounds::GetTeamIdByTeam(Team)]; }
    uint32 GetMatchmakerRatingByIndex(uint32 index) const { return m_ArenaTeamMMR[index]; }
    void BattlegroundTimedWin(uint32 type = 1);

    virtual void HandleKillPlayer(Player* player, Player* killer);
    virtual void HandleKillUnit(Creature* /*unit*/, Player* /*killer*/) { }
    virtual void HandleStartTimer(TimerType type);

    virtual void UpdateCapturePoint(uint8 type, TeamId teamID, GameObject* node, Player const* player = nullptr, bool initial = false, bool forseCustomAnim = false);
    virtual void EventPlayerDroppedFlag(Player* /*player*/) { }
    virtual void EventPlayerClickedOnFlag(Player* /*player*/, GameObject* /*object*/, bool& canRemove) { }
    virtual void EventPlayerClickedOnFlag(Player* /*player*/, Unit* /*targetUnit*/) { }
    void EventPlayerLoggedIn(Player* player);
    void EventPlayerLoggedOut(Player* player);
    virtual void EventPlayerDamagedGO(Player* /*player*/, GameObject* /*go*/, uint32 /*eventType*/) { }
    virtual void EventPlayerUsedGO(Player* /*player*/, GameObject* /*go*/) { }

    virtual void OnCreatureCreate(Creature* /*creature*/) {}
    virtual void OnCreatureRemove(Creature* /*creature*/) {}
    virtual void OnGameObjectCreate(GameObject* go) {}
    virtual void OnGameObjectRemove(GameObject* go) {}

    void PlayeCapturePointSound(uint8 type, TeamId teamID);

    virtual void DoAction(uint32 /*action*/, ObjectGuid /*var*/) { }

    virtual void HandlePlayerResurrect(Player* /*player*/) { }

    virtual WorldSafeLocsEntry const* GetClosestGraveYard(Player* player);

    virtual void AddPlayer(Player* player);
    virtual void OnPlayerEnter(Player* player);

    void AddMember(ObjectGuid guid, uint32 team);

    void AddOrSetPlayerToCorrectBgGroup(Player* player, uint32 team);

    virtual void RemovePlayerAtLeave(ObjectGuid guid, bool Transport, bool SendPacket);

    void HandleTriggerBuff(ObjectGuid go_guid);

    void AddNameInNameList(uint32 team, ArenaPlayerInfo* Info) { m_nameList[team].push_back(*Info); }

    // TODO: make this protected:
    GuidVector BgObjects;
    GuidVector BgCreatures;

    void SpawnBGObject(uint32 type, uint32 respawntime);

    bool AddObject(uint32 type, uint32 entry, Position pos, Position rotation = { }, uint32 respawnTime = 0, GOState goState = GO_STATE_READY);
    bool AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime = 0, GOState goState = GO_STATE_READY);

    Creature* AddCreature(uint32 entry, uint32 type, uint32 teamval, Position pos, uint32 respawntime = 0, Transport* transport = nullptr);
    Creature* AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime = 0, Transport* transport = nullptr);

    bool DelCreature(uint32 type);
    bool DelObject(uint32 type);

    bool AddSpiritGuide(uint32 type, DBCPosition4D loc, TeamId team);
    bool AddSpiritGuide(uint32 type, Position pos, TeamId team);
    bool AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team);
    int32 GetObjectType(ObjectGuid guid);

    void DoorsOpen(uint32 type1, uint32 type2);
    void DoorsClose(uint32 type1, uint32 type2);
    void DoorOpen(uint32 type);
    void DoorClose(uint32 type);

    const char* GetTrinityString(int32 entry);

    bool HandlePlayerUnderMap(Player* player);

    uint32 GetPlayerTeam(ObjectGuid guid) const;
    uint32 GetOtherTeam(uint32 teamId) const;
    bool IsPlayerInBattleground(ObjectGuid guid) const;

    bool ToBeDeleted() const { return m_SetDeleteThis; }
    void SetDeleteThis() { m_SetDeleteThis = true; }

    int32 m_TeamScores[MAX_TEAMS];
    ArenaTeamScore _arenaTeamScores[MAX_TEAMS];
    uint32 m_lastFlagCaptureTeam;

    void RewardXPAtKill(Player* killer, Player* victim);

    void SetRBG(bool enable) { m_IsRBG = enable; }
    void SetBG(bool enable) { m_IsBG = enable; }

    void _ProcessPlayerPositionBroadcast(Milliseconds diff);
    virtual void GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* /*positions*/) const { }

    static Player* GetPlayer(ObjectGuid guid, bool offlineRemove, const char* context);
    static Player* GetPlayer(std::pair<ObjectGuid, BattlegroundPlayer> pair, const char* context);
    static Player* GetPlayerForTeam(uint32 teamId, std::pair<ObjectGuid, BattlegroundPlayer> pair, const char* context);

    Milliseconds GetCountdownTimer() const { return m_CountdownTimer; }
    void SetCountdownTimer(Milliseconds value) { m_CountdownTimer = value; }

    virtual uint32 GetMaxScore() const { return 0; }
    virtual uint32 GetTeamScore(TeamId /*team*/) const { return 0; }
    virtual bool IsScoreIncremental() const { return true; }

    std::recursive_mutex m_bg_lock;

    void AddSpectator(Player* player);
    void RemoveSpectator(Player* player);
    bool HaveSpectators() { return (!_spectators.empty()); }
    void SendSpectateAddonsMsg(SpectatorAddonMsg& msg);
    void PreventAddingToQueueAgained(bool val) {m_InBGFreeSlotQueue = val;}
    uint8 GetMaxGroupSize() const { return m_maxGroupSize; }
protected:
    void EndNow();
    void PlayerAddedToBGCheckIfBGIsRunning(Player* player);

    void _ProcessOfflineQueue();
    void _ProcessRessurect(uint32 diff);
    void _ProcessProgress(uint32 diff);
    void _ProcessLeave(uint32 diff);
    virtual void _ProcessJoin(uint32 diff);
    virtual void _CheckPositions(uint32 diff);

    std::map<ObjectGuid, BattlegroundScore*> PlayerScores;
    virtual void RemovePlayer(Player* /*player*/, ObjectGuid /*guid*/, uint32 /*team*/) { }

    std::map<ObjectGuid, GuidVector>  m_ReviveQueue;

    uint8 m_Events;

    std::array<uint32, BG_STARTING_EVENT_COUNT> m_broadcastMessages;

    bool   m_BuffChange;
    uint16 m_TypeID;
    std::map<uint32, std::vector<ArenaPlayerInfo>> m_nameList;
    std::map<ObjectGuid, uint32> m_allMembers;
    uint8  m_JoinType;
    uint8  m_BrawlJoinType = 0;
    Position m_TeamStartPos[MAX_TEAMS];
private:
    virtual void PostUpdateImpl(uint32 /* diff */) { }
    virtual void PostUpdateImpl(Milliseconds /*diff*/) { }

    uint16 m_RandomTypeID;
    uint32 m_InstanceID;
    BattlegroundStatus m_Status;
    uint32 m_ClientInstanceID;
    Milliseconds m_StartTime;
    uint32 m_ResetStatTimer;
    uint32 m_ValidStartPositionTimer;
    Milliseconds m_EndTime;
    Milliseconds m_CountdownTimer;
    uint32 m_LastResurrectTime;
    uint8 m_BracketId;
    bool m_InBGFreeSlotQueue;
    bool m_SetDeleteThis;
    bool m_IsArena;
    uint8 m_Winner;
    Milliseconds m_StartDelayTime;
    Milliseconds m_LastPlayerPositionBroadcast;
    bool m_IsRated;
    bool _useTournamentRules;
    bool _isSkirmish;
    bool _isWargame;
    bool m_PrematureCountDown;
    uint32 m_PrematureCountDownTimer;
    char const* m_Name;
    uint64 m_QueueID;
    std::set<ObjectGuid> _spectators;
    GuidVector _resurrectQueue;
    GuidDeque m_OfflineQueue;
    uint32 m_InvitedAlliance;
    uint32 m_InvitedHorde;
    Group* m_BgRaids[MAX_TEAMS];
    uint32 m_PlayersCount[MAX_TEAMS];
    uint64 m_GroupIds[MAX_TEAMS];
    uint32 m_ArenaTeamMMR[MAX_TEAMS];
    uint32 m_LevelMin;
    uint32 m_LevelMax;
    uint32 m_MaxPlayersPerTeam;
    uint32 m_MaxPlayers;
    uint32 m_MinPlayersPerTeam;
    uint32 m_MinPlayers;
    uint32 m_MapId;
    BattlegroundMap* m_Map;
    uint8 m_maxGroupSize;
    uint32 ScriptId;
    bool m_IsRBG;
    bool _isBrawl;
    bool m_IsBG;
    std::map<ObjectGuid, BattlegroundPlayer>  _players;
};

#endif
