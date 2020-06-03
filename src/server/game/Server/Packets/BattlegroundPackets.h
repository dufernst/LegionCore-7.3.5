/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#ifndef BattlegroundPackets_h__
#define BattlegroundPackets_h__

#include "Packet.h"
#include "Packets/LFGPackets.h"
#include "BattlegroundDefines.h" // remove this shit -_-

namespace WorldPackets
{
    namespace Battleground
    {
        class PVPSeason final : public ServerPacket
        {
        public:
            PVPSeason() : ServerPacket(SMSG_PVP_SEASON, 8) { }

            WorldPacket const* Write() override;

            uint32 PreviousSeason = 0;
            uint32 CurrentSeason = 0;
        };

        struct BattlefieldStatusHeader
        {
            LFG::RideTicket Ticket;
            uint64 QueueID = 0;
            uint32 InstanceID = 0;
            uint8 RangeMax = 0;
            uint8 RangeMin = 0;
            uint8 TeamSize = 0;
            bool RegisteredMatch = false;
            bool TournamentRules = false;
        };

        struct BracketInfo
        {
            uint32 PersonalRating = 0;
            uint32 Ranking = 0;
            uint32 SeasonPlayed = 0;
            uint32 SeasonWon = 0;
            uint32 WeeklyPlayed = 0;
            uint32 WeeklyWon = 0;
            uint32 BestWeeklyRating = 0;
            uint32 BestSeasonRating = 0;
            uint32 BestWeeklyLastRating = 0;
        };

        class RatedInfo final : public ServerPacket
        {
        public:
            RatedInfo() : ServerPacket(SMSG_RATED_BATTLEFIELD_INFO, 44 * MS::Battlegrounds::BracketType::Max) { }

            WorldPacket const* Write() override;

            BracketInfo Info[MS::Battlegrounds::BracketType::Max] = { };
        };

        class ListClient final : public ClientPacket
        {
        public:
            ListClient(WorldPacket&& packet) : ClientPacket(CMSG_BATTLEFIELD_LIST, std::move(packet)) { }

            void Read() override;

            uint32 ListID = 0;
        };

        class Port final : public ClientPacket
        {
        public:
            Port(WorldPacket&& packet) : ClientPacket(CMSG_BATTLEFIELD_PORT, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket Ticket;
            bool AcceptedInvite = false;
        };

        struct IgnorMapInfo
        {
            IgnorMapInfo() { map[0] = 0, map[1] = 0; }
            uint32 map[2];
        };

        class Join final : public ClientPacket
        {
        public:
            Join(WorldPacket&& packet) : ClientPacket(CMSG_BATTLEMASTER_JOIN, std::move(packet)) { }

            void Read() override;

            uint64 QueueID = 0;
            IgnorMapInfo BlacklistMap;
            uint8 RolesMask = 0;
            bool JoinAsGroup = false;
        };

        class JoinArena final : public ClientPacket
        {
        public:
            JoinArena(WorldPacket&& packet) : ClientPacket(CMSG_BATTLEMASTER_JOIN_ARENA, std::move(packet)) { }

            void Read() override;

            uint8 TeamSizeIndex = 0;
            uint8 Roles = 0;
        };

        class PVPLogData final : public ServerPacket
        {
        public:
            PVPLogData() : ServerPacket(SMSG_PVP_LOG_DATA, 8) { }

            WorldPacket const* Write() override;

            struct RatingData
            {
                int32 Prematch[MAX_TEAMS] = { };
                int32 Postmatch[MAX_TEAMS] = { };
                int32 PrematchMMR[MAX_TEAMS] = { };
            };

            struct HonorData
            {
                uint32 HonorKills = 0;
                uint32 Deaths = 0;
                uint32 ContributionPoints = 0;
            };

            struct PlayerData
            {
                std::vector<int32> Stats;
                Optional<HonorData> Honor;
                Optional<uint32> PreMatchRating;
                Optional<uint32> PreMatchMMR;
                Optional<int32> RatingChange;
                Optional<int32> MmrChange;
                ObjectGuid PlayerGUID;
                uint32 Kills = 0;
                uint32 DamageDone = 0;
                uint32 HealingDone = 0;
                uint32 PrimaryTalentTreeNameIndex = 0;
                uint32 Race = 0;
                uint32 PrestigeRank = 0;
                int32 PrimaryTalentTree = 0;
                uint8 Faction = 0;
                bool IsInWorld = false;
            };

            std::vector<PlayerData> Players;
            Optional<RatingData> Ratings;
            Optional<uint8> Winner;
            int8 PlayerCount[MAX_TEAMS] = { };
        };

        class AreaSpiritHealerQuery final : public ClientPacket
        {
        public:
            AreaSpiritHealerQuery(WorldPacket&& packet) : ClientPacket(CMSG_AREA_SPIRIT_HEALER_QUERY, std::move(packet)) { }

            void Read() override;

            ObjectGuid HealerGuid;
        };

        class AreaSpiritHealerQueue final : public ClientPacket
        {
        public:
            AreaSpiritHealerQueue(WorldPacket&& packet) : ClientPacket(CMSG_AREA_SPIRIT_HEALER_QUEUE, std::move(packet)) { }

            void Read() override;

            ObjectGuid HealerGuid;
        };

        class AreaSpiritHealerTime final : public ServerPacket
        {
        public:
            AreaSpiritHealerTime() : ServerPacket(SMSG_AREA_SPIRIT_HEALER_TIME, 25) { }

            WorldPacket const* Write() override;

            ObjectGuid HealerGuid;
            uint32 TimeLeft = 0;
        };

        class ReportPvPPlayerAFKResult final : public ServerPacket
        {
        public:
            ReportPvPPlayerAFKResult() : ServerPacket(SMSG_REPORT_PVP_PLAYER_AFK_RESULT, 16 + 1 + 1 + 1) { }

            WorldPacket const* Write() override;

            enum ResultCode : uint8
            {
                PVP_REPORT_AFK_SUCCESS = 0,
                PVP_REPORT_AFK_GENERIC_FAILURE = 1,
                PVP_REPORT_AFK_SYSTEM_ENABLED = 5,
                PVP_REPORT_AFK_SYSTEM_DISABLED = 6
            };

            ObjectGuid Offender;
            uint8 NumPlayersIHaveReported = 0;
            uint8 NumBlackMarksOnOffender = 0;
            uint8 Result = PVP_REPORT_AFK_GENERIC_FAILURE;
        };

        class BattlefieldList final : public ServerPacket
        {
        public:
            BattlefieldList() : ServerPacket(SMSG_BATTLEFIELD_LIST, 25) { }

            WorldPacket const* Write() override;

            ObjectGuid BattlemasterGuid;
            std::vector<uint32> Battlefields;
            uint32 BattlemasterListID = 0;
            uint8 MaxLevel = 0;
            uint8 MinLevel = 0;
            bool PvpAnywhere = false;
            bool HasWinToday = false;
        };

        class PVPOptionsEnabled final : public ServerPacket
        {
        public:
            PVPOptionsEnabled() : ServerPacket(SMSG_PVP_OPTIONS_ENABLED, 6) { }

            WorldPacket const* Write() override;

            bool RatedArenas = false;
            bool ArenaSkirmish = false;
            bool PugBattlegrounds = false;
            bool WargameBattlegrounds = false;
            bool WargameArenas = false;
            bool RatedBattlegrounds = false;
        };

        class RequestPVPRewardsResponse final : public ServerPacket
        {
        public:
            RequestPVPRewardsResponse() : ServerPacket(SMSG_REQUEST_PVP_REWARDS_RESPONSE, 40 * 4) { }

            WorldPacket const* Write() override;
            
            LFG::ShortageReward RandomBGRewards;
            LFG::ShortageReward RatedBGRewards;
            LFG::ShortageReward ArenaSkirmishRewards;
            LFG::ShortageReward ArenaRewards2v2;
            LFG::ShortageReward ArenaRewards3v3;
            LFG::ShortageReward BrawlRewardsBattleground;
            LFG::ShortageReward BrawlRewardsArena;

            bool HasRatedBGWinToday = false;
            bool HasArenaSkirmishWinToday = false;
            bool HasArena2v2WinToday = false;
            bool HasArena3v3WinToday = false;
            bool HasBrawlBattlegroundWinToday = false;
            bool HasBrawlArenaWinToday = false;
        };

        struct BattlegroundPlayerPosition
        {
            TaggedPosition<Position::XY> Pos;
            ObjectGuid Guid;
            uint8 IconID = 0;
            uint8 ArenaSlot = 0;
        };

        class PlayerPositions final : public ServerPacket
        {
        public:
            PlayerPositions() : ServerPacket(SMSG_BATTLEGROUND_PLAYER_POSITIONS, 25) { }

            WorldPacket const* Write() override;

            std::vector<BattlegroundPlayerPosition> FlagCarriers;
        };

        class BattlefieldStatusNone final : public ServerPacket
        {
        public:
            BattlefieldStatusNone() : ServerPacket(SMSG_BATTLEFIELD_STATUS_NONE, 25) { }

            WorldPacket const* Write() override;

            LFG::RideTicket Ticket;
        };

        class BattlefieldStatusNeedConfirmation final : public ServerPacket
        {
        public:
            BattlefieldStatusNeedConfirmation() : ServerPacket(SMSG_BATTLEFIELD_STATUS_NEED_CONFIRMATION, 25) { }

            WorldPacket const* Write() override;

            BattlefieldStatusHeader Header;
            uint32 Mapid = 0;
            uint32 Timeout = 0;
            uint8 Role = 0;
        };

        class BattlefieldStatusFailed final : public ServerPacket
        {
        public:
            BattlefieldStatusFailed() : ServerPacket(SMSG_BATTLEFIELD_STATUS_FAILED, 25) { }

            WorldPacket const* Write() override;

            LFG::RideTicket Ticket;
            ObjectGuid ClientID;
            uint64 QueueID = 0;
            uint32 Reason = 0;
        };

        class PlayerJoined final : public ServerPacket
        {
        public:
            PlayerJoined(ObjectGuid guid) : ServerPacket(SMSG_BATTLEGROUND_PLAYER_JOINED, 20), Guid(guid) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
        };

        class PlayerLeft final : public ServerPacket
        {
        public:
            PlayerLeft(ObjectGuid guid) : ServerPacket(SMSG_BATTLEGROUND_PLAYER_LEFT, 20), Guid(guid) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
        };

        class Points final : public ServerPacket
        {
        public:
            Points() : ServerPacket(SMSG_BATTLEGROUND_POINTS, 3) { }

            WorldPacket const* Write() override;

            uint16 BgPoints = 0;
            bool Team = false; // 0 - alliance; 1 - horde
        };

        class Init final : public ServerPacket
        {
        public:
            Init(uint16 maxPoints) : ServerPacket(SMSG_BATTLEGROUND_INIT, 2 + 4), MaxPoints(maxPoints) { }

            WorldPacket const* Write() override;

            uint32 ServerTime = getMSTime();
            uint16 MaxPoints = 0;
        };

        struct BattlegroundCapturePointInfoData
        {
            ObjectGuid Guid;
            TaggedPosition<Position::XY> Pos;
            uint32 CaptureTime = 0;
            uint32 CaptureTotalDuration = 0;
            int8 NodeState = NODE_STATE_NONE;
        };

        class BattlegroundCapturePointInfo final : public ServerPacket
        {
        public:
            BattlegroundCapturePointInfo() : ServerPacket(SMSG_BATTLEGROUND_CAPTURE_POINT_INFO, 16 + 8 + 1 + 4 + 4) { }

            WorldPacket const* Write() override;

            BattlegroundCapturePointInfoData Info;
        };

        class MapObjectivesInit final : public ServerPacket
        {
        public:
            MapObjectivesInit() : ServerPacket(SMSG_MAP_OBJECTIVES_INIT, 25) { }

            WorldPacket const* Write() override;

            std::vector<BattlegroundCapturePointInfoData> CapturePointInfo;
        };

        class StatusWaitForGroups final : public ServerPacket
        {
        public:
            StatusWaitForGroups() : ServerPacket(SMSG_BATTLEFIELD_STATUS_WAIT_FOR_GROUPS, 12) { }

            WorldPacket const* Write() override;

            BattlefieldStatusHeader Header;
            uint32 Mapid = 0;
            uint32 Timeout = 0;
            uint8 TotalPlayers[MAX_TEAMS] = { };
            uint8 AwaitingPlayers[MAX_TEAMS] = { };
        };

        class BattlefieldStatusQueued final : public ServerPacket
        {
        public:
            BattlefieldStatusQueued() : ServerPacket(SMSG_BATTLEFIELD_STATUS_QUEUED, 11) { }

            WorldPacket const* Write() override;

            BattlefieldStatusHeader Header;
            uint32 AverageWaitTime = 0;
            uint32 WaitTime = 0;
            bool AsGroup = false;
            bool SuspendedQueue = false;
            bool EligibleForMatchmaking = false;
        };

        class BattlefieldStatusActive final : public ServerPacket
        {
        public:
            BattlefieldStatusActive() : ServerPacket(SMSG_BATTLEFIELD_STATUS_ACTIVE, 14) { }

            WorldPacket const* Write() override;

            BattlefieldStatusHeader Header;
            Milliseconds StartTimer = Milliseconds(0);
            Milliseconds ShutdownTimer = Milliseconds(0);
            uint32 Mapid = 0;
            bool ArenaFaction = false;
            bool LeftEarly = false;
        };

        class ConquestFormulaConstants final : public ServerPacket
        {
        public:
            ConquestFormulaConstants() : ServerPacket(SMSG_CONQUEST_FORMULA_CONSTANTS, 20) { }

            WorldPacket const* Write() override;

            uint32 PvpMinCPPerWeek = 0;
            uint32 PvpMaxCPPerWeek = 0;
            float PvpCPBaseCoefficient = 0.0f;
            float PvpCPExpCoefficient = 0.0f;
            float PvpCPNumerator = 0.0f;
        };

        //< SMSG_BATTLEGROUND_INFO_THROTTLED
        class NullSMsg final : public ServerPacket
        {
        public:
            NullSMsg(OpcodeServer opcode) : ServerPacket(opcode) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        //< CMSG_REQUEST_BATTLEFIELD_STATUS
        //< CMSG_REQUEST_CONQUEST_FORMULA_CONSTANTS
        //< CMSG_REQUEST_RATED_BATTLEFIELD_INFO
        //< CMSG_GET_PVP_OPTIONS_ENABLED
        //< CMSG_BATTLEFIELD_LEAVE
        //< CMSG_PVP_LOG_DATA
        class NullCmsg final : public ClientPacket
        {
        public:
            NullCmsg(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override { }
        };

        class ArenaPrepOpponentSpecializations final : public ServerPacket
        {
        public:
            ArenaPrepOpponentSpecializations() : ServerPacket(SMSG_ARENA_PREP_OPPONENT_SPECIALIZATIONS, 4) { }

            WorldPacket const* Write() override;

            struct OpponentSpecData
            {
                int32 SpecializationID = 0;
                int32 Unk = 0;
                ObjectGuid Guid;
            };

            std::vector<OpponentSpecData> Data;
        };

        class ArenaCrowdControlSpells final : public ServerPacket
        {
        public:
            ArenaCrowdControlSpells() : ServerPacket(SMSG_ARENA_CROWD_CONTROL_SPELLS, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid PlayerGuid;
            int32 CrowdControlSpellID = 0;
        };

        class JoinSkirmish final : public ClientPacket
        {
        public:
            JoinSkirmish(WorldPacket&& packet) : ClientPacket(CMSG_BATTLEMASTER_JOIN_SKIRMISH, std::move(packet)) { }

            void Read() override;

            uint8 RolesMask = 0;
            uint8 Bracket = 0;
            bool JoinAsGroup = false;
            bool UnkBool = false;
        };

        class HearthAndResurrect final : public ClientPacket
        {
        public:
            HearthAndResurrect(WorldPacket&& packet) : ClientPacket(CMSG_HEARTH_AND_RESURRECT, std::move(packet)) { }

            void Read() override { }
        };

        class JoinRatedBattleground final : public ClientPacket
        {
        public:
            JoinRatedBattleground(WorldPacket&& packet) : ClientPacket(CMSG_JOIN_RATED_BATTLEGROUND, std::move(packet)) { }

            void Read() override { }
        };

        class RequestPvpBrawlInfo final : public ClientPacket
        {
        public:
            RequestPvpBrawlInfo(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_PVP_BRAWL_INFO, std::move(packet)) { }

            void Read() override { }
        };

        class RequestPvpBrawlInfoResponse final : public ServerPacket
        {
        public:
            RequestPvpBrawlInfoResponse() : ServerPacket(SMSG_REQUEST_PVP_BRAWL_INFO_RESPONSE, 12 + 6 + 1) { }

            WorldPacket const* Write() override;

            uint32 TimeToEnd = 0;
            uint32 BattlegroundTypeId = 0;
            uint32 BrawlType = 0;
            std::string Name;
            std::string Description;
            std::string Description2;
            bool IsActive = false;
        };

        class BattlemasterJoinBrawl final : public ClientPacket
        {
        public:
            BattlemasterJoinBrawl(WorldPacket&& packet) : ClientPacket(CMSG_BATTLEMASTER_JOIN_BRAWL, std::move(packet)) { }

            void Read() override;
            
            uint8 RolesMask = 0;
        };

        class ReportPvPPlayerAFK final : public ClientPacket
        {
        public:
            ReportPvPPlayerAFK(WorldPacket&& packet) : ClientPacket(CMSG_REPORT_PVP_PLAYER_AFK, std::move(packet)) { }

            void Read() override;

            ObjectGuid Offender;
        };

        class RequestPVPRewards final : public ClientPacket
        {
        public:
            RequestPVPRewards(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_PVP_REWARDS, std::move(packet)) { }

            void Read() override { }
        };

        class SetCemetryPreferrence final : public ClientPacket
        {
        public:
            SetCemetryPreferrence(WorldPacket&& packet) : ClientPacket(CMSG_SET_PREFERRED_CEMETERY, std::move(packet)) { }

            void Read() override;

            uint32 GraveyardID = 0;
        };

        class AcceptWargameInvite final : public ClientPacket
        {
        public:
            AcceptWargameInvite(WorldPacket&& packet) : ClientPacket(CMSG_ACCEPT_WARGAME_INVITE, std::move(packet)) { }

            void Read() override;

            ObjectGuid OpposingPartyMember;
            uint64 QueueID = 0;
            bool Accept = false;
        };

        class StartWargame final : public ClientPacket
        {
        public:
            StartWargame(WorldPacket&& packet) : ClientPacket(CMSG_START_WAR_GAME, std::move(packet)) { }

            void Read() override;

            ObjectGuid OpposingPartyMember;
            uint64 QueueID = 0;
            uint32 OpposingPartyMemberVirtualRealmAddress = 0;
            uint16 UnkShort = 0;
            bool TournamentRules = false;
        };

        class CheckWargameEntry final : public ServerPacket
        {
        public:
            CheckWargameEntry() : ServerPacket(SMSG_CHECK_WARGAME_ENTRY, 16 + 8 + 8 + 4 + 1 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid OpposingPartyBnetAccountID;
            ObjectGuid OpposingPartyMember;
            uint64 QueueID = 0;
            uint32 TimeoutSeconds = 0;
            uint32 RealmID = 0;
            uint16 UnkShort = 0;
            uint8 OpposingPartyUserServer = 0;
            bool TournamentRules = false;
        };

        class WargameRequestSuccessfullySentToOpponent final : public ServerPacket
        {
        public:
            WargameRequestSuccessfullySentToOpponent() : ServerPacket(SMSG_WARGAME_REQUEST_SUCCESSFULLY_SENT_TO_OPPONENT, 6) { }

            WorldPacket const* Write() override;

            Optional<uint32> UnkInt2;
            Optional<uint32> UnkInt3;
            uint32 UnkInt = 0;
        };

        class RequstCrowdControlSpell final : public ClientPacket
        {
        public:
            RequstCrowdControlSpell(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_CROWD_CONTROL_SPELL, std::move(packet)) { }

            void Read() override;

            ObjectGuid PlayerGuid;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Battleground::BattlefieldStatusHeader const& header);

#endif // BattlegroundPackets_h__
