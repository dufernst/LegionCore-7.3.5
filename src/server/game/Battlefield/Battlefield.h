/*
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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

#ifndef BATTLEFIELD_H_
#define BATTLEFIELD_H_

#include "Utilities/Util.h"
#include "SharedDefines.h"
#include "ZoneScript.h"
#include "WorldPacket.h"
#include "ObjectAccessor.h"

namespace WorldPackets
{
    namespace WorldState
    {
        class InitWorldStates;
    }
}

enum BattlefieldTypes
{
    BATTLEFIELD_WG      = 1,
    BATTLEFIELD_TB      = 21,
};

enum BattlefieldIDs
{
    BATTLEFIELD_BATTLEID_WG     = 1,
    BATTLEFIELD_BATTLEID_TB,
};

enum BattlefieldObjectiveStates
{
    BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL,
    BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE,
};

enum BattlefieldTimers
{
    BATTLEFIELD_OBJECTIVE_UPDATE_INTERVAL        = 1000
};

enum BattlefieldState
{
    BATTLEFIELD_INACTIVE        = 0,
    BATTLEFIELD_WARMUP          = 1,
    BATTLEFIELD_IN_PROGRESS     = 2
};

class Player;
class GameObject;
class WorldPacket;
class Creature;
class Unit;

class Battlefield;
class BfGraveyard;

typedef std::vector<BfGraveyard*> GraveyardVect;
typedef std::map<ObjectGuid, uint32> PlayerTimerMap;

class BfCapturePoint
{
    public:
        BfCapturePoint(Battlefield* bf);

        virtual void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }

        void SendUpdateWorldState(uint32 field, uint32 value);
        void SendUpdateWorldState(WorldStates field, uint32 value);

        void SendObjectiveComplete(uint32 id, ObjectGuid guid);

        virtual bool HandlePlayerEnter(Player* player);
        virtual GuidSet::iterator HandlePlayerLeave(Player* player);

        bool IsInsideObjective(Player* player) const;

        virtual bool Update(uint32 diff);
        virtual void ChangeTeam(TeamId /*oldTeam*/) { }
        virtual void PointCapturedByTeam(TeamId /*teamID*/) { }
        virtual void SendChangePhase();

        bool SetCapturePointData(GameObject* capturePoint);
        GameObject* GetCapturePointGo() { return m_capturePoint; }

        TeamId GetTeamId() { return m_team; }
    protected:
        bool DelCapturePoint();

        GuidSet m_activePlayers[MAX_TEAMS];

        float m_maxValue;
        float m_minValue;
        float m_maxSpeed;
        float m_value;

        TeamId m_team;

        BattlefieldObjectiveStates m_OldState;
        BattlefieldObjectiveStates m_State;

        uint32 m_neutralValuePct;

        Battlefield* m_Bf;

        GameObject* m_capturePoint;
};

class BfGraveyard
{
    public:
        BfGraveyard(Battlefield* Bf);

        void GiveControlTo(TeamId team);
        TeamId GetControlTeamId() { return m_ControlTeam; }

        float GetDistance(Player* player);

        void Initialize(TeamId startcontrol, uint32 gy, uint32 tp);

        void SetSpirit(Creature* spirit, TeamId team);

        void AddPlayer(ObjectGuid playerGUID);

        void RemovePlayer(ObjectGuid playerGUID);

        void Resurrect();

        void RelocateDeadPlayers();

        bool HasNpc(ObjectGuid guid)
        {
            if (!m_SpiritGuide[0] || !m_SpiritGuide[1])
                return false;

            if (!sObjectAccessor->FindUnit(m_SpiritGuide[0]) ||
                !sObjectAccessor->FindUnit(m_SpiritGuide[1]))
                return false;

            return (m_SpiritGuide[0] == guid || m_SpiritGuide[1] == guid);
        }

        bool HasPlayer(ObjectGuid guid) { return m_ResurrectQueue.find(guid) != m_ResurrectQueue.end(); }

        uint32 GetGraveyardId() { return m_GraveyardId; }
        uint32 GetTypeId() const { return m_TypeId; }

    protected:
        TeamId m_ControlTeam;
        uint32 m_GraveyardId;
        uint32 m_TypeId;
        ObjectGuid m_SpiritGuide[MAX_TEAMS];
        GuidSet m_ResurrectQueue;
        Battlefield* m_Bf;
};

class Battlefield : public ZoneScript
{
    friend class BattlefieldMgr;

    public:
        Battlefield();
        virtual ~Battlefield();

        typedef std::map<uint32 /*lowguid */, BfCapturePoint*> BfCapturePointMap;

        virtual bool SetupBattlefield() { return true; }

        virtual void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }

        void SendUpdateWorldState(uint32 field, uint32 value);
        void SendUpdateWorldState(WorldStates field, uint32 value);
        virtual bool Update(uint32 diff);

        void InvitePlayersInZoneToQueue();
        void InvitePlayersInQueueToWar();
        void InvitePlayersInZoneToWar();

        virtual void HandleKill(Player* /*killer*/, Unit* /*killed*/) {};

        uint32 GetTypeId() const { return m_TypeId; }
        uint32 GetZoneId() const { return m_AreaID; }
        uint64 GetQueueID() const { return MAKE_PAIR64(m_BattleId | 0x20000, 0x1F100000); }

        bool IsWarTime() { return m_isActive; }
        int8 GetState() const { return m_isActive ? BATTLEFIELD_IN_PROGRESS : (m_Timer <= m_StartGroupingTimer ? BATTLEFIELD_WARMUP : BATTLEFIELD_INACTIVE); }

        void ToggleBattlefield(bool enable) { m_IsEnabled = enable; }
        bool IsEnabled() { return m_IsEnabled; }

        void KickPlayerFromBattlefield(ObjectGuid guid);

        void HandlePlayerEnterZone(ObjectGuid guid, uint32 zone);
        void HandlePlayerLeaveZone(ObjectGuid guid, uint32 zone);

        virtual uint64 GetData64(uint32 dataId) { return m_Data64[dataId]; }
        virtual void SetData64(uint32 dataId, uint64 value) { m_Data64[dataId] = value; }

        virtual uint32 GetData(uint32 dataId) const { return m_Data32[dataId]; }
        virtual void SetData(uint32 dataId, uint32 value) { m_Data32[dataId] = value; }
        virtual void UpdateData(uint32 index, int32 pad) { m_Data32[index] += pad; }

        TeamId GetDefenderTeam() { return m_DefenderTeam; }
        TeamId GetAttackerTeam() { return TeamId(1 - m_DefenderTeam); }
        void SetDefenderTeam(TeamId team) { m_DefenderTeam = team; }

        Group* GetFreeBfRaid(TeamId TeamId);
        Group* GetGroupPlayer(ObjectGuid guid, TeamId TeamId);
        bool AddOrSetPlayerToCorrectBfGroup(Player* player);

        WorldSafeLocsEntry const * GetClosestGraveYard(Player* player);

        virtual void AddPlayerToResurrectQueue(ObjectGuid npc_guid, ObjectGuid playerGUID);
        void RemovePlayerFromResurrectQueue(ObjectGuid playerGUID);
        void SetGraveyardNumber(uint32 number) { m_GraveyardList.resize(number); }
        BfGraveyard* GetGraveyardById(uint32 id);

        Creature* SpawnCreature(uint32 entry, float x, float y, float z, float o, TeamId team);
        Creature* SpawnCreature(uint32 entry, Position pos, TeamId team);
        GameObject* SpawnGameObject(uint32 entry, Position const pos);
        GameObject* SpawnGameObject(uint32 entry, float x, float y, float z, float o);

        virtual void OnBattleStart() {};
        virtual void OnBattleEnd(bool /*endByTimer*/) {};
        virtual void OnStartGrouping() {};
        virtual void OnPlayerJoinWar(Player* /*player*/) {};
        virtual void OnPlayerLeaveWar(Player* /*player*/);
        virtual void OnPlayerLeaveZone(Player* /*player*/) {};
        virtual void OnPlayerEnterZone(Player* /*player*/) {};

        void SendWarningToAllInZone(uint32 entry);
        void SendWarningToPlayer(Player* player, uint32 entry);
        void SendWorldTextToTeam(uint32 broadcastID, ObjectGuid guid, uint32 arg1, uint32 arg2);

        void PlayerAcceptInviteToQueue(Player* player);
        void PlayerAcceptInviteToWar(Player* player);
        uint32 GetBattleId() { return m_BattleId; }
        void AskToLeaveQueue(Player* player);

        virtual void DoCompleteOrIncrementAchievement(uint32 /*achievement*/, Player* /*player*/, uint8 /*incrementNumber = 1*/) {};

        void SendInitWorldStatesToAll();

        bool IncrementQuest(Player *player, uint32 quest, bool complete = false);

        bool CanFlyIn() { return !m_isActive; }

        void SendAreaSpiritHealerQuery(Player* player, const ObjectGuid & guid);

        void StartBattle();
        void EndBattle(bool endByTimer);

        void HideNpc(Creature* creature);
        void ShowNpc(Creature* creature, bool aggressive);

        GraveyardVect GetGraveyardVector() { return m_GraveyardList; }

        uint32 GetTimer() { return m_Timer; }
        void SetTimer(uint32 timer) { m_Timer = timer; }

        void DoPlaySoundToAll(uint32 SoundID);

        void InvitePlayerToQueue(Player* player);
        void InvitePlayerToWar(Player* player);

        void InitStalker(uint32 entry, float x, float y, float z, float o);

        bool IsOnStartGrouping() const { return m_StartGrouping; }

        static TeamId GetTeamIndexByTeamId(uint32 team) { return team == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
        static Team GetTeamByTeamId(TeamId teamID) { return teamID == TEAM_ALLIANCE ? ALLIANCE : HORDE; }
        static uint32 GetOtherTeamID(TeamId teamID) { return teamID == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE; }

        void SetMap(Map* map) { m_map = map; }
        Map* GetMap() const { return m_map; }

    protected:
        ObjectGuid StalkerGuid;
        uint32 m_Timer;
        bool m_IsEnabled;
        bool m_isActive;
        TeamId m_DefenderTeam;

        BfCapturePointMap m_capturePoints;

        GuidSet m_players[MAX_TEAMS];                      // Players in zone
        GuidSet m_PlayersInQueue[MAX_TEAMS];               // Players in the queue
        GuidSet m_PlayersInWar[MAX_TEAMS];                 // Players in WG combat
        PlayerTimerMap m_InvitedPlayers[MAX_TEAMS];
        PlayerTimerMap m_PlayersWillBeKick[MAX_TEAMS];

        uint32 m_TypeId;                                        // See enum BattlefieldTypes
        uint32 m_BattleId;                                      // BattleID (for packet)
        uint32 m_AreaID;                                        // ZoneID of Wintergrasp = 4197
        uint32 m_MapId;                                         // MapId where is Battlefield
        uint32 m_MaxPlayer;                                     // Maximum number of player that participated to Battlefield
        uint32 m_MinPlayer;                                     // Minimum number of player for Battlefield start
        uint32 m_MinLevel;                                      // Required level to participate at Battlefield
        uint32 m_BattleTime;                                    // Length of a battle
        uint32 m_NoWarBattleTime;                               // Time between two battles
        uint32 m_RestartAfterCrash;                             // Delay to restart Wintergrasp if the server crashed during a running battle.
        uint32 m_TimeForAcceptInvite;
        uint32 m_uiKickDontAcceptTimer;
        WorldLocation KickPosition;                             // Position where players are teleported if they switch to afk during the battle or if they don't accept invitation
        uint32 m_uiKickAfkPlayersTimer;                         // Timer for check Afk in war
        GraveyardVect m_GraveyardList;                          // Vector witch contain the different GY of the battle
        uint32 m_LastResurectTimer;                             // Timer for resurect player every 30 sec

        uint32 m_StartGroupingTimer;                            // Timer for invite players in area 15 minute before start battle
        bool m_StartGrouping;                                   // bool for know if all players in area has been invited
        GuidSet m_Groups[MAX_TEAMS];                            // Contain different raid group

        std::vector<uint64> m_Data64;
        std::vector<uint32> m_Data32;

        void KickAfkPlayers();

        virtual void SendRemoveWorldStates(Player* /*player*/) {}

        void BroadcastPacketToZone(WorldPacket& data) const;
        void BroadcastPacketToQueue(WorldPacket& data) const;
        void BroadcastPacketToWar(WorldPacket& data) const;

        void AddCapturePoint(BfCapturePoint* cp) { m_capturePoints[cp->GetCapturePointGo()->GetEntry()] = cp; }

        BfCapturePoint* GetCapturePoint(uint32 lowguid) const
        {
            Battlefield::BfCapturePointMap::const_iterator itr = m_capturePoints.find(lowguid);
            if (itr != m_capturePoints.end())
                return itr->second;

            return nullptr;
        }

        void RegisterZone(uint32 zoneid);
        bool HasPlayer(Player* player) const;
        void TeamCastSpell(TeamId team, int32 spellId);

        Map* m_map;
};

#endif
