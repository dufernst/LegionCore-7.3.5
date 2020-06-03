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

#ifndef LFGPackets_h__
#define LFGPackets_h__

#include "Packet.h"
#include "PacketUtilities.h"
#include "LFGPacketsCommon.h"

namespace lfg
{
    struct LockData;
}

namespace WorldPackets
{
    namespace LFG
    {
        struct BlackList
        {
            void Initialize(std::map<uint32, lfg::LockData> const& lock, ObjectGuid const& = ObjectGuid::Empty);

            struct BlackListInfo
            {
                BlackListInfo() = default;
                BlackListInfo(uint32 slot, uint32 reason, int32 subReason1, int32 subReason2);

                uint32 Slot = 0;
                uint32 Reason = 0;
                int32 SubReason1 = 0;
                int32 SubReason2 = 0;
            };

            Optional<ObjectGuid> PlayerGuid;
            std::vector<BlackListInfo> Slots;
        };

        struct BootInfo
        {
            ObjectGuid Target;
            uint32 TotalVotes = 0;
            uint32 BootVotes = 0;
            uint32 TimeLeft = 0;
            uint32 VotesNeeded = 0;
            std::string Reason;
            bool VoteInProgress = false;
            bool VotePassed = false;
            bool MyVoteCompleted = false;
            bool MyVote = false;
        };

        struct ShortageReward
        {
            ShortageReward() = default;
            void Initialize(::Quest const* quest = nullptr, Player* player = nullptr);

            struct PlayerQuestRewardItem
            {
                PlayerQuestRewardItem(uint32 itemID, uint32 quantity = 1) : ItemID(itemID), Quantity(quantity) { }

                uint32 ItemID = 0;
                uint32 Quantity = 0;
            };

            struct PlayerQuestRewardCurrency
            {
                PlayerQuestRewardCurrency(uint32 currency, uint32 quantity = 1) : CurrencyID(currency), Quantity(quantity) { }

                uint32 CurrencyID = 0;
                uint32 Quantity = 0;
            };

            std::vector<PlayerQuestRewardItem> Item;
            std::vector<PlayerQuestRewardCurrency> Currency;
            std::vector<PlayerQuestRewardCurrency> BonusCurrency;
            Optional<uint32> RewardSpellID;
            Optional<uint32> UnkInt2;
            Optional<uint32> UnkInt3;
            Optional<uint32> RewardHonor;
            uint32 Mask = 0;
            uint32 RewardMoney = 0;
            uint32 RewardXP = 0;
        };

        struct PlayerDungeonInfo
        {
            std::vector<ShortageReward> ShortageRewards;
            ShortageReward Reward;
            uint32 Slot = 0;
            uint32 CompletionQuantity = 0;
            uint32 CompletionLimit = 0;
            uint32 CompletionCurrencyID = 0;
            uint32 SpecificQuantity = 0;
            uint32 SpecificLimit = 0;
            uint32 OverallQuantity = 0;
            uint32 OverallLimit = 0;
            uint32 PurseWeeklyQuantity = 0;
            uint32 PurseWeeklyLimit = 0;
            uint32 PurseQuantity = 0;
            uint32 PurseLimit = 0;
            uint32 Quantity = 0;
            uint32 CompletedMask = 0;
            uint32 EncounterMask = 0;
            bool FirstReward = false;
            bool ShortageEligible = false;
        };

        class PlayerInfo final : public ServerPacket
        {
        public:
            PlayerInfo() : ServerPacket(SMSG_LFG_PLAYER_INFO, 5 + 4) { }

            WorldPacket const* Write() override;

            std::vector<PlayerDungeonInfo> Dungeon;
            BlackList BlackListMap;
        };

        class JoinResult final : public ServerPacket
        {
        public:
            JoinResult() : ServerPacket(SMSG_LFG_JOIN_RESULT, 4 + 28 + 1 + 1) { }

            WorldPacket const* Write() override;

            std::vector<BlackList> Slots;
            RideTicket Ticket;
            uint8 Result = 0;
            uint8 ResultDetail = 0;
        };

        class QueueStatusUpdate final : public ServerPacket
        {
        public:
            QueueStatusUpdate() : ServerPacket(SMSG_LFG_UPDATE_STATUS, 28 + 1 + 1 + 1 + 4 + 4 + 4 + 5 + 2) { }

            WorldPacket const* Write() override;

            RideTicket Ticket;
            GuidVector SuspendedPlayers;
            std::vector<uint32> Slots;
            uint32 RequestedRoles = 0;
            uint8 SubType = 0;
            uint8 Reason = 0;
            bool IsParty = false;
            bool NotifyUI = false;
            bool Joined = false;
            bool LfgJoined = false;
            bool Queued = false;
            bool Unused = false;
        };

        class LockInfoRequest final : public ClientPacket
        {
        public:
            LockInfoRequest(WorldPacket&& packet) : ClientPacket(CMSG_DF_GET_SYSTEM_INFO, std::move(packet)) { }

            void Read() override;

            bool Player = false;
            uint8 PartyIndex = 0;
        };

        class QueueStatus final : public ServerPacket
        {
        public:
            QueueStatus() : ServerPacket(SMSG_LFG_QUEUE_STATUS, 28 + 4 + 4 + 4 + 4 + 1 + 4) { }

            WorldPacket const* Write() override;

            RideTicket Ticket;
            uint32 AvgWaitTimeByRole[3] = {};
            uint8 LastNeeded[3] = {};
            uint32 Slot = 0;
            uint32 AvgWaitTime = 0;
            uint32 QueuedTime = 0;
            uint32 AvgWaitTimeMe = 0;
        };

        class ProposalUpdate final : public ServerPacket
        {
        public:
            ProposalUpdate() : ServerPacket(SMSG_LFG_PROPOSAL_UPDATE, 28 + 8 + 4 + 4 + 1 + 1 + 4 + 1 + 1 + 4) { }

            WorldPacket const* Write() override;

            struct ProposalUpdatePlayer
            {
                uint32 Roles = 0;
                bool Me = false;
                bool SameParty = false;
                bool MyParty = false;
                bool Responded = false;
                bool Accepted = false;
            };

            std::vector<ProposalUpdatePlayer> Players;
            RideTicket Ticket;
            uint64 InstanceID = 0;
            uint32 ProposalID = 0;
            uint32 Slot = 0;
            uint32 CompletedMask = 0;
            uint32 EncounterMask = 0;
            uint8 State = 0;
            bool ValidCompletedMask = false;
            bool ProposalSilent = false;
            bool IsRequeue = false;
        };

        class PlayerReward final : public ServerPacket
        {
        public:
            PlayerReward() : ServerPacket(SMSG_LFG_PLAYER_REWARD, 4 + 4 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            struct PlayerRewards
            {
                uint32 RewardItem = 0;
                uint32 RewardItemQuantity = 0;
                uint32 BonusCurrency = 0;
                bool IsCurrency = false;
            };

            std::vector<PlayerRewards> Players;
            uint32 ActualSlot = 0;
            uint32 QueuedSlot = 0;
            uint32 RewardMoney = 0;
            uint32 AddedXP = 0;
        };

        class LfgJoin final : public ClientPacket
        {
        public:
            LfgJoin(WorldPacket&& packet) : ClientPacket(CMSG_DF_JOIN, std::move(packet)) { }

            void Read() override;

            std::set<uint32> Slot;
            uint32 Roles = 0;
            uint8 PartyIndex = 0;
            bool QueueAsGroup = false;
            bool UnkBit = false;
        };

        class RoleCheckUpdate final : public ServerPacket
        {
        public:
            RoleCheckUpdate() : ServerPacket(SMSG_LFG_ROLE_CHECK_UPDATE, 4 + 4 + 1 + 1 + 8 + 1 + 1 + 4) { }

            WorldPacket const* Write() override;

            struct CheckUpdateMember
            {
                ObjectGuid Guid;
                uint32 RolesDesired = 0;
                uint8 Level = 0;
                bool RoleCheckComplete = false;
            };

            std::vector<CheckUpdateMember> Members;
            std::vector<uint32> JoinSlots;
            uint64 BgQueueID = 0;
            uint32 GroupFinderActivityID = 0;
            uint8 PartyIndex = 0;
            uint8 RoleCheckStatus = 0;
            bool IsBeginning = false;
            bool IsRequeue = false;
        };

        class RoleChosen final : public ServerPacket
        {
        public:
            RoleChosen() : ServerPacket(SMSG_ROLE_CHOSEN, 16 + 4 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid Player;
            uint32 RoleMask = 0;
            bool Accepted = false;
        };

        class PartyInfo final : public ServerPacket
        {
        public:
            PartyInfo() : ServerPacket(SMSG_LFG_PARTY_INFO, 4) { }

            WorldPacket const* Write() override;

            std::vector<BlackList> Player;
        };

        class BootPlayer final : public ServerPacket
        {
        public:
            BootPlayer() : ServerPacket(SMSG_LFG_BOOT_PLAYER, 2 + 1 + 1 + 1 + 16 + 4 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            BootInfo Info;
        };

        class LfgBootPlayerVote final : public ClientPacket
        {
        public:
            LfgBootPlayerVote(WorldPacket&& packet) : ClientPacket(CMSG_DF_BOOT_PLAYER_VOTE, std::move(packet)) { }

            void Read() override;

            bool Vote = false;
        };

        struct ProposalResponse
        {
            RideTicket Ticket;
            uint64 InstanceID = 0;
            uint32 ProposalID = 0;
            bool Accepted = false;
        };

        class LfgProposalResponse final : public ClientPacket
        {
        public:
            LfgProposalResponse(WorldPacket&& packet) : ClientPacket(CMSG_DF_PROPOSAL_RESPONSE, std::move(packet)) { }

            void Read() override;

            ProposalResponse Data;
        };

        class TeleportDenied final : public ServerPacket
        {
        public:
            TeleportDenied(uint8 reason) : ServerPacket(SMSG_LFG_TELEPORT_DENIED, 1), Reason(reason) { }

            WorldPacket const* Write() override;

            uint8 Reason = 0;
        };

        class LfgTeleport final : public ClientPacket
        {
        public:
            LfgTeleport(WorldPacket&& packet) : ClientPacket(CMSG_DF_TELEPORT, std::move(packet)) { }

            void Read() override;

            bool TeleportOut = false;
        };

        class LfgCompleteRoleCheck final : public ClientPacket
        {
        public:
            LfgCompleteRoleCheck(WorldPacket&& packet) : ClientPacket(CMSG_DF_SET_ROLES, std::move(packet)) { }

            void Read() override;

            uint32 RolesDesired = 0;
            uint8 PartyIndex = 0;
        };

        class LfgLeave final : public ClientPacket
        {
        public:
            LfgLeave(WorldPacket&& packet) : ClientPacket(CMSG_DF_LEAVE, std::move(packet)) { }

            void Read() override;

            RideTicket Ticket;
        };

        class BonusFactionID final : public ClientPacket
        {
        public:
            BonusFactionID(WorldPacket&& packet) : ClientPacket(CMSG_SET_LFG_BONUS_FACTION_ID, std::move(packet)) { }

            void Read() override;

            uint32 FactionID = 0;
        };

        //< CMSG_DF_GET_JOIN_STATUS
        class NullCmsg final : public ClientPacket
        {
        public:
            NullCmsg(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override { }
        };

        //< SMSG_LFG_DISABLED
        class NullSmsg final : public ServerPacket
        {
        public:
            NullSmsg(OpcodeServer opcode) : ServerPacket(opcode, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class LFGOfferContinue final : public ServerPacket
        {
        public:
            LFGOfferContinue(uint32 slot) : ServerPacket(SMSG_LFG_OFFER_CONTINUE, 4), Slot(slot) { }

            WorldPacket const* Write() override;

            uint32 Slot = 0;
        };

        class SlotInvalid final : public ServerPacket
        {
        public:
            SlotInvalid() : ServerPacket(SMSG_LFG_SLOT_INVALID, 12) { }

            WorldPacket const* Write() override;
            
            uint32 Reason = 0;
            int32 SubReason1 = 0;
            int32 SubReason2 = 0;
        };

        class CompleteReadyCheck final : public ClientPacket
        {
        public:
            CompleteReadyCheck(WorldPacket&& packet) : ClientPacket(CMSG_DF_READY_CHECK_RESPONSE, std::move(packet)) { }

            void Read() override;

            uint8 PartyIndex = 0;
            bool IsReady = false;
        };
        
        class ReadyCheckResult final : public ServerPacket
        {
        public:
            ReadyCheckResult() : ServerPacket(SMSG_LFG_READY_CHECK_RESULT, 17) { }

            WorldPacket const* Write() override;
            
            ObjectGuid PlayerGuid;
            bool IsReady = false;
        };

        class ReadyCheckUpdate final : public ServerPacket
        {
        public:
            ReadyCheckUpdate() : ServerPacket(SMSG_LFG_READY_CHECK_UPDATE, 4 + 8 + 3) { }

            WorldPacket const* Write() override;
            
            struct UpdateData
            {
                ObjectGuid PlayerGuid;
                bool IsReady = false;
                bool UnkBit = false;
            };

            std::vector<UpdateData> Data;
            uint64 UnkLong = 0;
            uint8 UnkByte = 0;
            uint8 UnkByte2 = 0;
            bool IsCompleted = false;
        };

        class SetFastLaunchResult final : public ServerPacket
        {
        public:
            SetFastLaunchResult() : ServerPacket(SMSG_SET_DF_FAST_LAUNCH_RESULT, 1) { }

            WorldPacket const* Write() override;
            
            bool Set = false;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LFG::BootInfo const& boot);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LFG::ShortageReward const& reward);


#endif // LFGPackets_h__
