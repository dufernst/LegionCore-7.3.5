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

#ifndef GuildPackets_h__
#define GuildPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "Guild.h"
#include "ItemPackets.h"

namespace WorldPackets
{
    namespace Guild
    {
        struct GuildBankLogEntry
        {
            Optional<uint64> Money;
            Optional<int32> ItemID;
            Optional<int32> Count;
            Optional<int8> OtherTab;
            ObjectGuid PlayerGUID;
            uint32 TimeOffset = 0;
            int8 EntryType = 0;
        };

        class QueryGuildInfo final : public ClientPacket
        {
        public:
            QueryGuildInfo(WorldPacket&& packet) : ClientPacket(CMSG_QUERY_GUILD_INFO, std::move(packet)) { }

            void Read() override;

            ObjectGuid PlayerGuid;
            ObjectGuid GuildGuid;
        };

        class QueryGuildInfoResponse final : public ServerPacket
        {
        public:
            QueryGuildInfoResponse() : ServerPacket(SMSG_QUERY_GUILD_INFO_RESPONSE, 16 + 1) { }

            struct GuildInfo
            {
                ObjectGuid GuildGUID;
                uint32 VirtualRealmAddress = 0;
                std::string GuildName;

                struct GuildInfoRank
                {
                    GuildInfoRank(uint32 id, uint32 order, std::string const& name);

                    uint32 RankID;
                    uint32 RankOrder;
                    std::string RankName;

                    bool operator<(GuildInfoRank const& right) const;
                };

                std::set<GuildInfoRank> Ranks;

                int32 EmblemStyle = -1;
                int32 EmblemColor = -1;
                int32 BorderStyle = -1;
                int32 BorderColor = -1;
                int32 BackgroundColor = -1;
            };

            WorldPacket const* Write() override;

            ObjectGuid GuildGuid;
            Optional<GuildInfo> Info;
        };

        class GuildGetRoster final : public ClientPacket
        {
        public:
            GuildGetRoster(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_GET_ROSTER, std::move(packet)) { }

            void Read() override { }
        };

        struct GuildRosterProfessionData
        {
            int32 DbID = 0;
            int32 Rank = 0;
            int32 Step = 0;
        };

        struct GuildRosterMemberData
        {
            GuildRosterProfessionData ProfessionData[2] = {};
            ObjectGuid Guid;
            int64 WeeklyXP = 0;
            int64 TotalXP = 0;
            int32 RankID = 0;
            int32 AreaID = 0;
            int32 PersonalAchievementPoints = 0;
            int32 GuildReputation = 0;
            int32 GuildRepToCap = 0;
            uint32 VirtualRealmAddress = 0;
            float LastSave = 0.0f;
            std::string Name;
            std::string Note;
            std::string OfficerNote;
            uint8 Status = 0;
            uint8 Level = 0;
            uint8 ClassID = 0;
            uint8 Gender = 0;
            bool Authenticated = false;
            bool SorEligible = false;
        };

        class GuildRoster final : public ServerPacket
        {
        public:
            GuildRoster() : ServerPacket(SMSG_GUILD_ROSTER) { }

            WorldPacket const* Write() override;

            std::vector<GuildRosterMemberData> MemberData;
            std::string WelcomeText;
            std::string InfoText;
            uint32 CreateDate = 0;
            int32 NumAccounts = 0;
            int32 GuildFlags = 0;
        };

        class GuildRosterUpdate final : public ServerPacket
        {
        public:
            GuildRosterUpdate() : ServerPacket(SMSG_GUILD_ROSTER_UPDATE) { }

            WorldPacket const* Write() override;

            std::vector<GuildRosterMemberData> MemberData;
        };

        class GuildUpdateMotdText final : public ClientPacket
        {
        public:
            GuildUpdateMotdText(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_UPDATE_MOTD_TEXT, std::move(packet)) { }

            void Read() override;

            std::string MotdText;
        };

        class GuildCommandResult final : public ServerPacket
        {
        public:
            GuildCommandResult() : ServerPacket(SMSG_GUILD_COMMAND_RESULT) { }

            WorldPacket const* Write() override;

            int32 Result = 0;
            int32 Command = 0;
            std::string Name;
        };

        class AcceptGuildInvite final : public ClientPacket
        {
        public:
            AcceptGuildInvite(WorldPacket&& packet) : ClientPacket(CMSG_ACCEPT_GUILD_INVITE, std::move(packet)) { }

            void Read() override { }
        };

        class GuildDeclineInvitation final : public ClientPacket
        {
        public:
            GuildDeclineInvitation(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DECLINE_INVITATION, std::move(packet)) { }

            void Read() override { }
        };

        class AutoDeclineGuildInvites final : public ClientPacket
        {
        public:
            AutoDeclineGuildInvites(WorldPacket&& packet) : ClientPacket(CMSG_DECLINE_GUILD_INVITES, std::move(packet)) { }

            void Read() override;

            bool Allow = false;
        };

        class GuildInviteByName final : public ClientPacket
        {
        public:
            GuildInviteByName(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_INVITE_BY_NAME, std::move(packet)) { }

            void Read() override;

            std::string Name;
        };

        class GuildInvite final : public ServerPacket
        {
        public:
            GuildInvite() : ServerPacket(SMSG_GUILD_INVITE, 68) { }

            WorldPacket const* Write() override;

            ObjectGuid GuildGUID;
            ObjectGuid OldGuildGUID;
            uint32 AchievementPoints = 0;
            int32 EmblemColor = -1;
            int32 EmblemStyle = -1;
            int32 BorderStyle = -1;
            int32 BorderColor = -1;
            int32 Background = -1;
            uint32 GuildVirtualRealmAddress = 0;
            uint32 OldGuildVirtualRealmAddress = 0;
            uint32 InviterVirtualRealmAddress = 0;
            std::string InviterName;
            std::string GuildName;
            std::string OldGuildName;
        };

        class GuildEventPresenceChange final : public ServerPacket
        {
        public:
            GuildEventPresenceChange() : ServerPacket(SMSG_GUILD_EVENT_PRESENCE_CHANGE, 16 + 4 + 1 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            uint32 VirtualRealmAddress = 0;
            std::string Name;
            bool Mobile = false;
            bool LoggedOn = false;
        };

        class GuildEventMotd final : public ServerPacket
        {
        public:
            GuildEventMotd() : ServerPacket(SMSG_GUILD_EVENT_MOTD, 1) { }

            WorldPacket const* Write() override;

            std::string MotdText;
        };

        class GuildEventPlayerJoined final : public ServerPacket
        {
        public:
            GuildEventPlayerJoined() : ServerPacket(SMSG_GUILD_EVENT_PLAYER_JOINED, 21) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            std::string Name;
            uint32 VirtualRealmAddress = 0;
        };

        class GuildEventRankChanged final : public ServerPacket
        {
        public:
            GuildEventRankChanged() : ServerPacket(SMSG_GUILD_EVENT_RANK_CHANGED, 4) { }

            WorldPacket const* Write() override;

            int32 RankID = 0;
        };

        class GuildEventRanksUpdated final : public ServerPacket
        {
        public:
            GuildEventRanksUpdated() : ServerPacket(SMSG_GUILD_EVENT_RANKS_UPDATED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class GuildEventBankMoneyChanged final : public ServerPacket
        {
        public:
            GuildEventBankMoneyChanged() : ServerPacket(SMSG_GUILD_EVENT_BANK_MONEY_CHANGED, 8) { }

            WorldPacket const* Write() override;

            uint64 Money = 0;
        };

        class GuildEventDisbanded final : public ServerPacket
        {
        public:
            GuildEventDisbanded() : ServerPacket(SMSG_GUILD_EVENT_DISBANDED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class GuildEventPlayerLeft final : public ServerPacket
        {
        public:
            GuildEventPlayerLeft() : ServerPacket(SMSG_GUILD_EVENT_PLAYER_LEFT, 43) { }

            WorldPacket const* Write() override;

            ObjectGuid LeaverGUID;
            ObjectGuid RemoverGUID;
            uint32 LeaverVirtualRealmAddress = 0;
            uint32 RemoverVirtualRealmAddress = 0;
            std::string LeaverName;
            std::string RemoverName;
            bool Removed = false;
        };

        class GuildEventNewLeader final : public ServerPacket
        {
        public:
            GuildEventNewLeader() : ServerPacket(SMSG_GUILD_EVENT_NEW_LEADER, 43) { }

            WorldPacket const* Write() override;

            ObjectGuid NewLeaderGUID;
            ObjectGuid OldLeaderGUID;
            uint32 NewLeaderVirtualRealmAddress = 0;
            uint32 OldLeaderVirtualRealmAddress = 0;
            std::string NewLeaderName;
            std::string OldLeaderName;
            bool SelfPromoted = false;
        };

        class GuildEventTabAdded final : public ServerPacket
        {
        public:
            GuildEventTabAdded() : ServerPacket(SMSG_GUILD_EVENT_TAB_ADDED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class GuildEventTabModified final : public ServerPacket
        {
        public:
            GuildEventTabModified() : ServerPacket(SMSG_GUILD_EVENT_TAB_MODIFIED, 6) { }

            WorldPacket const* Write() override;

            int32 Tab = 0;
            std::string Icon;
            std::string Name;
        };

        class GuildEventTabTextChanged final : public ServerPacket
        {
        public:
            GuildEventTabTextChanged() : ServerPacket(SMSG_GUILD_EVENT_TAB_TEXT_CHANGED, 4) { }

            WorldPacket const* Write() override;

            int32 Tab = 0;
        };

        class GuildEventBankContentsChanged final : public ServerPacket
        {
        public:
            GuildEventBankContentsChanged() : ServerPacket(SMSG_GUILD_EVENT_BANK_CONTENTS_CHANGED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        struct GuildBankItemInfo
        {
            std::vector<Item::ItemGemData> SocketEnchant;
            Item::ItemInstance Item;
            int32 Slot = 0;
            int32 Count = 0;
            int32 EnchantmentID = 0;
            int32 Charges = 0;
            int32 OnUseEnchantmentID = 0;
            int32 Flags = 0;
            bool Locked = false;
        };

        struct GuildBankTabInfo
        {
            int32 TabIndex = 0;
            std::string Name;
            std::string Icon;
        };

        class GuildBankQueryTab final : public ClientPacket
        {
        public:
            GuildBankQueryTab(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_QUERY_TAB, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint8 TabId = 0;
            bool FullUpdate = false;
        };

        class GuildBankQueryResults final : public ServerPacket
        {
        public:
            GuildBankQueryResults() : ServerPacket(SMSG_GUILD_BANK_QUERY_RESULTS, 8 + 4 + 4 + 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            std::vector<GuildBankItemInfo> ItemInfo;
            std::vector<GuildBankTabInfo> TabInfo;
            uint64 Money = 0;
            int32 WithdrawalsRemaining = 0;
            int32 Tab = 0;
            bool FullUpdate = false;
        };

        class GuildBankSwapItemsLegacy final : public ClientPacket
        {
        public:
            GuildBankSwapItemsLegacy(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_SWAP_ITEMS_LEGACY, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            int32 StackCount = 0;
            int32 BankItemCount = 0;
            uint32 ItemID = 0;
            uint32 ItemID1 = 0;
            uint8 ToSlot = 0;
            uint8 BankSlot = 0;
            uint8 BankSlot1 = 0;
            uint8 BankTab = 0;
            uint8 BankTab1 = 0;
            uint8 ContainerSlot = 0;
            uint8 ContainerItemSlot = 0;
            bool AutoStore = false;
            bool BankOnly = false;
        };

        class GuildBankSwapItems final : public ClientPacket
        {
        public:
            GuildBankSwapItems(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint8 BankTab = 0;
            uint8 BankSlot = 0;
            uint8 PlayerSlot = 0;
            uint8 PlayerBag = 0;
            bool HasBag = false;
        };

        class GuildBankSwapItemsAuto final : public ClientPacket
        {
        public:
            GuildBankSwapItemsAuto(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint8 BankTab = 0;
            uint8 BankSlot = 0;
        };

        class GuildBankSwapItemsCount final : public ClientPacket
        {
        public:
            GuildBankSwapItemsCount(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint32 StackCount = 0;
            uint8 BankTab = 0;
            uint8 BankSlot = 0;
            uint8 PlayerSlot = 0;
            uint8 PlayerBag = 0;
            bool HasBag = false;
        };

        class GuildBankSwapItemsBankBank final : public ClientPacket
        {
        public:
            GuildBankSwapItemsBankBank(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint8 BankTab = 0;
            uint8 BankSlot = 0;
            uint8 NewBankTab = 0;
            uint8 NewBankSlot = 0;
        };

        class GuildBankSwapItemsBankBankCount final : public ClientPacket
        {
        public:
            GuildBankSwapItemsBankBankCount(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint32 StackCount = 0;
            uint8 BankTab = 0;
            uint8 BankSlot = 0;
            uint8 NewBankTab = 0;
            uint8 NewBankSlot = 0;
        };

        struct LFGuildRecruitData
        {
            ObjectGuid RecruitGUID;
            int32 RecruitVirtualRealm = 0;
            int32 CharacterClass = 0;
            int32 CharacterGender = 0;
            int32 CharacterLevel = 0;
            int32 ClassRoles = 0;
            int32 PlayStyle = 0;
            int32 Availability = 0;
            int32 SecondsSinceCreated = 0;
            int32 SecondsUntilExpiration = 0;
            std::string Name;
            std::string Comment;
        };

        class LFGuildRecruits final : public ServerPacket
        {
        public:
            LFGuildRecruits() : ServerPacket(SMSG_LF_GUILD_RECRUITS) { }

            WorldPacket const* Write() override;

            std::vector<LFGuildRecruitData> Recruits;
            time_t UpdateTime = time_t(0);
        };

        struct GuildPostData
        {
            int32 PlayStyle = 0;
            int32 Availability = 0;
            int32 ClassRoles = 0;
            int32 LevelRange = 0;
            int32 SecondsRemaining = 0;
            std::string Comment;
            bool Active = false;
        };

        class LFGuildPost final : public ServerPacket
        {
        public:
            LFGuildPost() : ServerPacket(SMSG_LF_GUILD_POST, 4) { }

            WorldPacket const* Write() override;

            Optional<GuildPostData> Post;
        };

        struct LFGuildBrowseData
        {
            ObjectGuid GuildGUID;
            uint32 GuildVirtualRealm = 0;
            int32 GuildMembers = 0;
            int32 GuildAchievementPoints = 0;
            int32 PlayStyle = 0;
            int32 Availability = 0;
            int32 ClassRoles = 0;
            int32 LevelRange = 0;
            int32 EmblemStyle = -1;
            int32 EmblemColor = -1;
            int32 BorderStyle = -1;
            int32 BorderColor = -1;
            int32 Background = -1;
            std::string GuildName;
            std::string Comment;
            int8 Cached = 0;
            int8 MembershipRequested = 0;
        };

        class LFGuildBrowseResponse final : public ServerPacket
        {
        public:
            LFGuildBrowseResponse() : ServerPacket(SMSG_LF_GUILD_BROWSE, 4) { }

            WorldPacket const* Write() override;

            std::vector<LFGuildBrowseData> Browses;
        };

        struct LFGuildApplicationData
        {
            ObjectGuid GuildGUID;
            int32 GuildVirtualRealm = 0;
            int32 ClassRoles = 0;
            int32 PlayStyle = 0;
            int32 Availability = 0;
            int32 SecondsSinceCreated = 0;
            std::string GuildName;
            std::string Comment;
        };

        class LFGuildApplication final : public ServerPacket
        {
        public:
            LFGuildApplication() : ServerPacket(SMSG_LF_GUILD_APPLICATIONS, 4 + 4) { }

            WorldPacket const* Write() override;

            std::vector<LFGuildApplicationData> Applications;
            int32 NumRemaining = 0;
        };

        class GuildReputationReactionChanged final : public ServerPacket
        {
        public:
            GuildReputationReactionChanged() : ServerPacket(SMSG_GUILD_REPUTATION_REACTION_CHANGED, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid MemberGUID;
        };

        class GuildChallengeUpdateRequest final : public ClientPacket
        {
        public:
            GuildChallengeUpdateRequest(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_CHALLENGE_UPDATE_REQUEST, std::move(packet)) { }

            void Read() override { }
        };

        class GuildChallengeUpdated final : public ServerPacket
        {
        public:
            GuildChallengeUpdated() : ServerPacket(SMSG_GUILD_CHALLENGE_UPDATE, 4 * 4 * 6) { }

            WorldPacket const* Write() override;

            int32 CurrentCount[6] = { };
            int32 MaxCount[6] = { };
            int32 Gold[6] = { };
            int32 MaxLevelGold[6] = { };
        };

        class GuildChallengeCompleted final : public ServerPacket
        {
        public:
            GuildChallengeCompleted() : ServerPacket(SMSG_GUILD_CHALLENGE_COMPLETED, 4 * 4) { }

            WorldPacket const* Write() override;

            int32 ChallengeType = 0;
            int32 CurrentCount = 0;
            int32 MaxCount = 0;
            int32 GoldAwarded = 0;
        };

        class LFGuildSetGuildPost final : public ClientPacket
        {
        public:
            LFGuildSetGuildPost(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_SET_GUILD_POST, std::move(packet)) { }

            void Read() override;

            uint32 Availability = 0;
            uint32 PlayStyle = 0;
            uint32 ClassRoles = 0;
            uint32 LevelRange = 0;
            std::string Comment;
            bool Active = false;
        };

        class GuildBankDepositMoney final : public ClientPacket
        {
        public:
            GuildBankDepositMoney(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_DEPOSIT_MONEY, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint64 Money = 0;
        };

        class GuildBankWithdrawMoney final : public ClientPacket
        {
        public:
            GuildBankWithdrawMoney(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_WITHDRAW_MONEY, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint64 Money = 0;
        };

        class GuildPermissionsQuery final : public ClientPacket
        {
        public:
            GuildPermissionsQuery(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_PERMISSIONS_QUERY, std::move(packet)) { }

            void Read() override { }
        };

        class GuildPermissionsQueryResults final : public ServerPacket
        {
        public:
            struct GuildRankTabPermissions
            {
                int32 Flags = 0;
                int32 WithdrawItemLimit = 0;
            };

            GuildPermissionsQueryResults() : ServerPacket(SMSG_GUILD_PERMISSIONS_QUERY_RESULTS, 20) { }

            WorldPacket const* Write() override;

            std::vector<GuildRankTabPermissions> Tab;
            int32 NumTabs = 0;
            int32 WithdrawGoldLimit = 0;
            int32 Flags = 0;
            uint32 RankID = 0;
        };

        class GuildBankLogQuery final : public ClientPacket
        {
        public:
            GuildBankLogQuery(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_LOG_QUERY, std::move(packet)) { }

            void Read() override;

            int32 TabId = 0;
        };

        class GuildBankLogQueryResults final : public ServerPacket
        {
        public:
            GuildBankLogQueryResults() : ServerPacket(SMSG_GUILD_BANK_LOG_QUERY_RESULTS, 25) { }

            WorldPacket const* Write() override;

            std::vector<GuildBankLogEntry> Entry;
            Optional<uint64> WeeklyBonusMoney;
            int32 TabId = 0;
        };

        class GuildBankTextQuery final : public ClientPacket
        {
        public:
            GuildBankTextQuery(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_TEXT_QUERY, std::move(packet)) { }

            void Read() override;

            int32 TabId = 0;
        };

        class GuildBankTextQueryResult final : public ServerPacket
        {
        public:
            GuildBankTextQueryResult() : ServerPacket(SMSG_GUILD_BANK_TEXT_QUERY_RESULT, 5) { }

            WorldPacket const* Write() override;

            int32 TabId = 0;
            std::string Text;
        };

        class GuildBankSetTabText final : public ClientPacket
        {
        public:
            GuildBankSetTabText(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_SET_TAB_TEXT, std::move(packet)) { }

            void Read() override;

            int32 TabId = 0;
            std::string TabText;
        };

        class GuildBankRemainingWithdrawMoneyQuery final : public ClientPacket
        {
        public:
            GuildBankRemainingWithdrawMoneyQuery(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_REMAINING_WITHDRAW_MONEY_QUERY, std::move(packet)) { }

            void Read() override { }
        };

        class GuildBankRemainingWithdrawMoney final : public ServerPacket
        {
        public:
            GuildBankRemainingWithdrawMoney() : ServerPacket(SMSG_GUILD_BANK_REMAINING_WITHDRAW_MONEY, 8) { }

            WorldPacket const* Write() override;

            int64 RemainingWithdrawMoney = 0;
        };

        struct GuildEventEntry
        {
            ObjectGuid PlayerGUID;
            ObjectGuid OtherGUID;
            uint32 TransactionDate = 0;
            uint8 TransactionType = 0;
            uint8 RankID = 0;
        };

        class GuildEventLogQuery final : public ClientPacket
        {
        public:
            GuildEventLogQuery(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_EVENT_LOG_QUERY, std::move(packet)) { }

            void Read() override { }
        };

        class GuildEventLogQueryResults final : public ServerPacket
        {
        public:
            GuildEventLogQueryResults() : ServerPacket(SMSG_GUILD_EVENT_LOG_QUERY_RESULTS, 4) { }

            WorldPacket const* Write() override;

            std::vector<GuildEventEntry> Entry;
        };

        class GuildQueryNews final : public ClientPacket
        {
        public:
            GuildQueryNews(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_QUERY_NEWS, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
        };

        struct GuildNewsEvent
        {
            Optional<Item::ItemInstance> Item;
            GuidList MemberList;
            ObjectGuid MemberGuid;
            int32 Data[2] = {};
            int32 Id = 0;
            uint32 CompletedDate = 0;
            int32 Type = 0;
            int32 Flags = 0;
        };

        class GuildNews final : public ServerPacket
        {
        public:
            GuildNews() : ServerPacket(SMSG_GUILD_NEWS, 25) { }

            WorldPacket const* Write() override;

            std::vector<GuildNewsEvent> NewsEvents;
        };

        class GuildNewsUpdateSticky final : public ClientPacket
        {
        public:
            GuildNewsUpdateSticky(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_NEWS_UPDATE_STICKY, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
            int32 NewsID = 0;
            bool Sticky = false;
        };

        class GuildSetRankPermissions final : public ClientPacket
        {
        public:
            GuildSetRankPermissions(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_SET_RANK_PERMISSIONS, std::move(packet)) { }

            void Read() override;

            int32 TabFlags[GUILD_BANK_MAX_TABS] = {};
            int32 TabWithdrawItemLimit[GUILD_BANK_MAX_TABS] = {};
            int32 RankID = 0;
            int32 RankOrder = 0;
            int32 WithdrawGoldLimit = 0;
            uint32 Flags = 0;
            uint32 OldFlags = 0;
            std::string RankName;
        };

        class GuildAddRank final : public ClientPacket
        {
        public:
            GuildAddRank(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_ADD_RANK, std::move(packet)) { }

            void Read() override;

            std::string Name;
            int32 RankOrder = 0;
        };

        class GuildAssignMemberRank final : public ClientPacket
        {
        public:
            GuildAssignMemberRank(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_ASSIGN_MEMBER_RANK, std::move(packet)) { }

            void Read() override;

            ObjectGuid Member;
            int32 RankOrder = 0;
        };

        class GuildDeleteRank final : public ClientPacket
        {
        public:
            GuildDeleteRank(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DELETE_RANK, std::move(packet)) { }

            void Read() override;

            int32 RankOrder = 0;
        };

        class GuildGetRanks final : public ClientPacket
        {
        public:
            GuildGetRanks(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_GET_RANKS, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
        };

        struct GuildRankData
        {
            uint32 TabFlags[GUILD_BANK_MAX_TABS] = { };
            uint32 TabWithdrawItemLimit[GUILD_BANK_MAX_TABS] = { };
            uint32 RankID = 0;
            uint32 RankOrder = 0;
            uint32 Flags = 0;
            uint32 WithdrawGoldLimit = 0;
            std::string RankName;
        };

        class GuildRanks final : public ServerPacket
        {
        public:
            GuildRanks() : ServerPacket(SMSG_GUILD_RANKS, 4) { }

            WorldPacket const* Write() override;

            std::vector<GuildRankData> Ranks;
        };

        class GuildSendRankChange final : public ServerPacket
        {
        public:
            GuildSendRankChange() : ServerPacket(SMSG_GUILD_SEND_RANK_CHANGE, 43) { }

            WorldPacket const* Write() override;

            ObjectGuid Other;
            ObjectGuid Officer;
            uint32 RankID = 0;
            bool Promote = false;
        };

        class GuildShiftRank final : public ClientPacket
        {
        public:
            GuildShiftRank(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_SHIFT_RANK, std::move(packet)) { }

            void Read() override;

            int32 RankOrder = 0;
            bool ShiftUp = false;
        };

        class GuildUpdateInfoText final : public ClientPacket
        {
        public:
            GuildUpdateInfoText(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_UPDATE_INFO_TEXT, std::move(packet)) { }

            void Read() override;

            std::string InfoText;
        };

        class GuildSetMemberNote final : public ClientPacket
        {
        public:
            GuildSetMemberNote(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_SET_MEMBER_NOTE, std::move(packet)) { }

            void Read() override;

            ObjectGuid NoteeGUID;
            std::string Note;
            bool IsPublic = false;          ///< 0 == Officer, 1 == Public
        };

        class GuildMemberUpdateNote final : public ServerPacket
        {
        public:
            GuildMemberUpdateNote() : ServerPacket(SMSG_GUILD_MEMBER_UPDATE_NOTE, 21) { }

            WorldPacket const* Write() override;

            ObjectGuid Member;
            std::string Note;
            bool IsPublic = false;          ///< 0 == Officer, 1 == Public
        };

        class GuildMemberDailyReset final : public ServerPacket
        {
        public:
            GuildMemberDailyReset() : ServerPacket(SMSG_GUILD_MEMBER_DAILY_RESET, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class GuildDelete final : public ClientPacket
        {
        public:
            GuildDelete(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DELETE, std::move(packet)) { }

            void Read() override { }
        };

        class GuildDemoteMember final : public ClientPacket
        {
        public:
            GuildDemoteMember(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DEMOTE_MEMBER, std::move(packet)) { }

            void Read() override;

            ObjectGuid Demotee;
        };

        class GuildPromoteMember final : public ClientPacket
        {
        public:
            GuildPromoteMember(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_PROMOTE_MEMBER, std::move(packet)) { }

            void Read() override;

            ObjectGuid Promotee;
        };

        class GuildOfficerRemoveMember : public ClientPacket
        {
        public:
            GuildOfficerRemoveMember(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_OFFICER_REMOVE_MEMBER, std::move(packet)) { }

            void Read() override;

            ObjectGuid Removee;
        };

        class GuildLeave final : public ClientPacket
        {
        public:
            GuildLeave(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_LEAVE, std::move(packet)) { }

            void Read() override { }
        };

        class RequestGuildPartyState final : public ClientPacket
        {
        public:
            RequestGuildPartyState(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_GUILD_PARTY_STATE, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
        };

        class GuildPartyState final : public ServerPacket
        {
        public:
            GuildPartyState() : ServerPacket(SMSG_GUILD_PARTY_STATE, 15) { }

            WorldPacket const* Write() override;

            float GuildXPEarnedMult = 0.0f;
            int32 NumMembers = 0;
            int32 NumRequired = 0;
            bool InGuildParty = false;
        };

        class RequestGuildRewardsList final : public ClientPacket
        {
        public:
            RequestGuildRewardsList(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_GUILD_REWARDS_LIST, std::move(packet)) { }

            void Read() override;

            uint32 CurrentVersion = 0;
        };

        struct GuildRewardItem
        {
            std::vector<uint32> AchievementsRequired;
            uint64 Cost = 0;
            uint32 ItemID = 0;
            uint32 Unk4 = 0;
            uint64 RaceMask = 0;
            int32 MinGuildLevel = 0;
            int32 MinGuildRep = 0;
        };

        class GuildRewardList final : public ServerPacket
        {
        public:
            GuildRewardList() : ServerPacket(SMSG_GUILD_REWARD_LIST, 8) { }

            WorldPacket const* Write() override;

            std::vector<GuildRewardItem> RewardItems;
            uint32 Version = 0;
        };

        class GuildBankActivate final : public ClientPacket
        {
        public:
            GuildBankActivate(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_ACTIVATE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            bool FullUpdate = false;
        };

        class GuildBankBuyTab final : public ClientPacket
        {
        public:
            GuildBankBuyTab(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_BUY_TAB, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint8 BankTab = 0;
        };

        class GuildBankUpdateTab final : public ClientPacket
        {
        public:
            GuildBankUpdateTab(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_UPDATE_TAB, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint8 BankTab = 0;
            std::string Name;
            std::string Icon;
        };

        class GuildSetGuildMaster final : public ClientPacket
        {
        public:
            GuildSetGuildMaster(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_SET_GUILD_MASTER, std::move(packet)) { }

            void Read() override;

            std::string NewMasterName;
        };

        class PlayerSaveGuildEmblem final : public ServerPacket
        {
        public:
            PlayerSaveGuildEmblem() : ServerPacket(SMSG_PLAYER_SAVE_GUILD_EMBLEM, 4) { }

            WorldPacket const* Write() override;

            int32 Error = 0;
        };

        class QueryMemberRecipes final : public ClientPacket
        {
        public:
            QueryMemberRecipes(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_QUERY_MEMBER_RECIPES, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildMember;
            ObjectGuid GuildGUID;
            uint32 SkillLineID = 0;
        };

        class QueryGuildMembersForRecipe final : public ClientPacket
        {
        public:
            QueryGuildMembersForRecipe(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_QUERY_MEMBERS_FOR_RECIPE, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
            uint32 SkillLineID = 0;
            uint32 SpellID = 0;
            uint32 UniqueBit = 0;
        };

        class QueryGuildMembersForRecipeReponse final : public ServerPacket
        {
        public:
            QueryGuildMembersForRecipeReponse() : ServerPacket(SMSG_GUILD_MEMBERS_WITH_RECIPE, 12) { }

            WorldPacket const* Write() override;

            GuidList Member;
            uint32 SpellID = 0;
            uint32 SkillLineID = 0;
        };
        
        class GuildMemberRecipes final : public ServerPacket
        {
        public:
            GuildMemberRecipes() : ServerPacket(SMSG_GUILD_MEMBER_RECIPES, 16 + 12 + 300) { }

            WorldPacket const* Write() override;

            ObjectGuid Member;
            uint32 SkillLineID = 0;
            uint32 SkillRank = 0;
            uint32 SkillStep = 0;
            std::array<uint8, KNOW_RECIPES_MASK_SIZE> SkillLineBitArray;
        };

        class GuildInviteDeclined final : public ServerPacket
        {
        public:
            GuildInviteDeclined() : ServerPacket(SMSG_GUILD_INVITE_DECLINED, 8) { }

            WorldPacket const* Write() override;

            uint32 VirtualRealmAddress = 0;
            std::string Name;
            bool AutoDecline = false;
        };

        class GuildFlaggedForRename final : public ServerPacket
        {
        public:
            GuildFlaggedForRename() : ServerPacket(SMSG_GUILD_FLAGGED_FOR_RENAME, 1) { }

            WorldPacket const* Write() override;

            bool FlagSet = true;
        };

        class GuildChangeNameResult final : public ServerPacket
        {
        public:
            GuildChangeNameResult() : ServerPacket(SMSG_GUILD_CHANGE_NAME_RESULT, 1) { }

            WorldPacket const* Write() override;

            bool Success = true;
        };

        class GuildNameChanged final : public ServerPacket
        {
        public:
            GuildNameChanged() : ServerPacket(SMSG_GUILD_NAME_CHANGED, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid GuildGUID;
            std::string GuildName;
        };

        class QueryRecipes final : public ClientPacket
        {
        public:
            QueryRecipes(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_QUERY_RECIPES, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
        };

        class LFGuildAddRecruit final : public ClientPacket
        {
        public:
            LFGuildAddRecruit(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_ADD_RECRUIT, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
            int32 Availability = 0;
            int32 ClassRoles = 0;
            int32 PlayStyle = 0;
            std::string Comment;
        };

        class LFGuildBrowse final : public ClientPacket
        {
        public:
            LFGuildBrowse(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_BROWSE, std::move(packet)) { }

            void Read() override;

            uint32 CharacterLevel = 0;
            uint32 Availability = 0;
            uint32 ClassRoles = 0;
            uint32 PlayStyle = 0;
        };

        class LFGuildRemoveRecruit final : public ClientPacket
        {
        public:
            LFGuildRemoveRecruit(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_REMOVE_RECRUIT, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
        };

        class LFGuildGetRecruits final : public ClientPacket
        {
        public:
            LFGuildGetRecruits(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_GET_RECRUITS, std::move(packet)) { }

            void Read() override;

            uint32 LastUpdate = 0;
        };

        class LFGuildGetGuildPost final : public ClientPacket
        {
        public:
            LFGuildGetGuildPost(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_GET_GUILD_POST, std::move(packet)) { }

            void Read() override { }
        };

        class LFGuildGetApplications final : public ClientPacket
        {
        public:
            LFGuildGetApplications(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_GET_APPLICATIONS, std::move(packet)) { }

            void Read() override { }
        };

        class LFGuildDeclineRecruit final : public ClientPacket
        {
        public:
            LFGuildDeclineRecruit(WorldPacket&& packet) : ClientPacket(CMSG_LF_GUILD_DECLINE_RECRUIT, std::move(packet)) { }

            void Read() override;

            ObjectGuid RecruitGUID;
        };

        class SaveGuildEmblem final : public ClientPacket
        {
        public:
            SaveGuildEmblem(WorldPacket&& packet) : ClientPacket(CMSG_SAVE_GUILD_EMBLEM, std::move(packet)) { }

            void Read() override;

            ObjectGuid Vendor;
            int32 BStyle = 0;
            int32 EStyle = 0;
            int32 BColor = 0;
            int32 EColor = 0;
            int32 Bg = 0;
        };

        class GuildChangeNameRequest final : public ClientPacket
        {
        public:
            GuildChangeNameRequest(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_CHANGE_NAME_REQUEST, std::move(packet)) { }

            void Read() override;

            std::string NewName;
        };

        class ReplaceGuildMaster final : public ClientPacket
        {
        public:
            ReplaceGuildMaster(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_REPLACE_GUILD_MASTER, std::move(packet)) { }

            void Read() override { }
        };

        class GuildSetAchievementTracking final : public ClientPacket
        {
        public:
            GuildSetAchievementTracking(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_SET_ACHIEVEMENT_TRACKING, std::move(packet)) { }

            void Read() override;

            std::set<uint32> AchievementIDs;
        };

        class GuildAutoDeclineInvitation final : public ClientPacket
        {
        public:
            GuildAutoDeclineInvitation(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_AUTO_DECLINE_INVITATION, std::move(packet)) { }

            void Read() override { }
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterProfessionData const& rosterProfessionData);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterMemberData const& rosterMemberData);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRankData const& rankData);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRewardItem const& rewardItem);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildNewsEvent const& newsEvent);

#endif // GuildPackets_h__
