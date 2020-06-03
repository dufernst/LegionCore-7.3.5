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

#ifndef LfgListPackets_h__
#define LfgListPackets_h__

#include "Packet.h"
#include "LFGPacketsCommon.h"

namespace WorldPackets
{
    namespace LfgList
    {
        struct LFGListBlacklist
        {
            uint32 ActivityID = 0;
            uint32 Reason = 0;
        };

        struct ApplicationToGroup
        {
            LFG::RideTicket ApplicationTicket;
            uint32 ActivityID = 0;
            std::string Comment;
            uint8 Role = 0;
        };

        struct ListRequest
        {
            ListRequest() { }

            Optional<uint32> QuestID;
            uint32 ActivityID = 0;
            uint32 HonorLevel = 0;
            float ItemLevel = 0.0f;
            std::string GroupName;
            std::string Comment;
            std::string VoiceChat;
            bool AutoAccept = false;
            bool PrivateGroup = false;
        };

        struct MemberInfo
        {
            MemberInfo() { }
            MemberInfo(uint8 classID, uint8 role) : ClassID(classID), Role(role) { }

            uint8 ClassID = CLASS_NONE;
            uint8 Role = 0;
        };

        struct ListSearchResult
        {
            LFG::RideTicket ApplicationTicket;
            ListRequest JoinRequest;
            std::vector<MemberInfo> Members;
            GuidList BNetFriendsGuids;
            GuidList NumCharFriendsGuids;
            GuidList NumGuildMateGuids;
            ObjectGuid UnkGuid1;
            ObjectGuid UnkGuid2;
            ObjectGuid UnkGuid3;
            ObjectGuid UnkGuid4;
            ObjectGuid UnkGuid5;
            uint32 VirtualRealmAddress = 0;
            uint32 CompletedEncounters = 0;
            uint32 Age = 0;
            uint32 ResultID = 0;
            uint8 ApplicationStatus = 0;
        };

        struct ApplicantStruct
        {
            ApplicantStruct() { }
            ApplicantStruct(ObjectGuid playerGUID, uint8 role) : PlayerGUID(playerGUID), Role(role) { }

            ObjectGuid PlayerGUID;
            uint8 Role = 0;
        };

        struct ApplicantMember
        {
            ApplicantMember() { }

            struct ACStatInfo
            {
                uint32 UnkInt4 = 0;
                uint32 UnkInt5 = 0;
            };

            std::list<ACStatInfo> AcStat;
            ObjectGuid PlayerGUID;
            uint32 VirtualRealmAddress = 0;
            uint32 Level = 0;
            uint32 HonorLevel = 0;
            float ItemLevel = 0.0f;
            uint8 PossibleRoleMask = 0;
            uint8 SelectedRoleMask = 0;
        };

        struct ApplicantInfo
        {
            std::vector<ApplicantMember> Member;
            LFG::RideTicket ApplicantTicket;
            ObjectGuid ApplicantPartyLeader;
            std::string Comment;
            uint8 ApplicationStatus = 0;
            bool Listed = false;
        };

        class LfgListApplyToGroup final : public ClientPacket
        {
        public:
            LfgListApplyToGroup(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_APPLY_TO_GROUP, std::move(packet)) { }

            void Read() override;

            ApplicationToGroup application;
        };

        class LfgListCancelApplication final : public ClientPacket
        {
        public:
            LfgListCancelApplication(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_CANCEL_APPLICATION, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicantTicket;
        };

        class LfgListDeclineApplicant final : public ClientPacket
        {
        public:
            LfgListDeclineApplicant(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_DECLINE_APPLICANT, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
        };

        class LfgListInviteApplicant final : public ClientPacket
        {
        public:
            LfgListInviteApplicant(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_INVITE_APPLICANT, std::move(packet)) { }

            void Read() override;

            std::list<ApplicantStruct> Applicant;
            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
        };

        class LfgListUpdateRequest final : public ClientPacket
        {
        public:
            LfgListUpdateRequest(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_UPDATE_REQUEST, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket Ticket;
            ListRequest UpdateRequest;
        };

        class LfgListGetStatus final : public ClientPacket
        {
        public:
            LfgListGetStatus(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_GET_STATUS, std::move(packet)) { }

            void Read() override { }
        };

        class LfgListInviteResponse final : public ClientPacket
        {
        public:
            LfgListInviteResponse(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_INVITE_RESPONSE, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicantTicket;
            bool Accept = false;
        };

        class LfgListJoin final : public ClientPacket
        {
        public:
            LfgListJoin(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_JOIN, std::move(packet)) { }

            void Read() override;

            ListRequest Request;
        };

        class LfgListLeave final : public ClientPacket
        {
        public:
            LfgListLeave(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_LEAVE, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicationTicket;
        };

        class LfgListSearch final : public ClientPacket
        {
        public:
            LfgListSearch(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_SEARCH, std::move(packet)) { }

            void Read() override;

            std::vector<LFGListBlacklist> Blacklist;
            GuidVector Guids;
            int32 CategoryID = 0;
            int32 SearchTerms = 0;
            int32 Filter = 0;
            int32 PreferredFilters = 0;
            std::string LanguageSearchFilter;
        };

        class RequestLfgListBlacklist final : public ClientPacket
        {
        public:
            RequestLfgListBlacklist(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_LFG_LIST_BLACKLIST, std::move(packet)) { }

            void Read() override { }
        };

        class LfgListApplicationUpdate final : public ServerPacket
        {
        public:
            LfgListApplicationUpdate() : ServerPacket(SMSG_LFG_LIST_APPLICATION_UPDATE, 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            std::vector<ApplicantInfo> Applicants;
            LFG::RideTicket ApplicationTicket;
            uint32 UnkInt = 0;
        };

        class LfgListApplyToGroupResponce final : public ServerPacket
        {
        public:
            LfgListApplyToGroupResponce() : ServerPacket(SMSG_LFG_LIST_APPLY_TO_GROUP_RESPONCE, 28 + 28 + 4 + 4 + 1 + 1 + 150) { }

            WorldPacket const* Write() override;

            ListSearchResult SearchResult;
            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
            uint32 InviteExpireTimer = 0;
            uint8 Status = 0;
            uint8 Role = 0;
            uint8 ApplicationStatus = 0;
        };

        class LfgListInviteApplicantResponse final : public ServerPacket
        {
        public:
            LfgListInviteApplicantResponse() : ServerPacket(SMSG_LFG_LIST_INVITE_APPLICANT_RESPONSE, 28 + 4 + 1) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicationTicket;
            uint32 Timer = 0;
            uint8 Status = 0;
        };

        class LfgListJoinResult final : public ServerPacket
        {
        public:
            LfgListJoinResult() : ServerPacket(SMSG_LFG_LIST_JOIN_RESULT, 28 + 1 + 1) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicationTicket;
            uint8 Status = 0;
            uint8 Result = 0;
        };

        class LfgListSearchResults final : public ServerPacket
        {
        public:
            LfgListSearchResults() : ServerPacket(SMSG_LFG_LIST_SEARCH_RESULTS, 6) { }

            WorldPacket const* Write() override;

            std::vector<ListSearchResult> SearchResults;
            uint16 AppicationsCount = 0;
        };

        class LfgListSearchStatus final : public ServerPacket
        {
        public:
            LfgListSearchStatus() : ServerPacket(SMSG_LFG_LIST_SEARCH_STATUS, 30) { }

            WorldPacket const* Write() override;

            LFG::RideTicket Ticket;
            uint8 Status = 0;
            bool UnkBit = false;
        };

        class LfgListGroupInviteResponce final : public ServerPacket
        {
        public:
            LfgListGroupInviteResponce() : ServerPacket(SMSG_LFG_LIST_INVITE_RESPONCE, 28 + 28 + 4 + 4 + 1 + 1) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
            uint32 InviteExpireTimer = 0;
            uint8 Status = 0;
            uint8 Role = 0;
            uint8 ApplicationStatus = 0;
        };

        class LfgListUpdateBlacklist final : public ServerPacket
        {
        public:
            LfgListUpdateBlacklist() : ServerPacket(SMSG_LFG_LIST_UPDATE_BLACKLIST, 4) { }

            WorldPacket const* Write() override;

            std::vector<LFGListBlacklist> Blacklist;
        };

        class LfgListUpdateStatus final : public ServerPacket
        {
        public:
            LfgListUpdateStatus() : ServerPacket(SMSG_LFG_LIST_UPDATE_STATUS, 28 + 1 + 1 + 4 + 4 + 2 + 2 + 2) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicationTicket;
            ListRequest Request;
            uint32 ExpirationTime = 0;
            uint8 Status = 0;
            bool Listed = false;
        };

        struct LfgListSearchResult
        {
            std::vector<MemberInfo> Members;
            LFG::RideTicket ApplicationTicket;
            ListRequest JoinRequest;
            Optional<ObjectGuid> LeaderGuid;
            Optional<ObjectGuid> UnkGuid;
            Optional<ObjectGuid> UnkGuid2;
            Optional<ObjectGuid> UnkGuid3;
            Optional<uint32> VirtualRealmAddress;
            Optional<uint32> UnkInt2;
            uint32 UnkInt = 0;
            bool UnkBIt = false;
            bool UnkBIt2 = false;
            bool UnkBIt3 = false;
            bool UnkBIt4 = false;
            bool UnkBit96 = false;
        };

        class LfgListSearchResultUpdate final : public ServerPacket
        {
        public:
            LfgListSearchResultUpdate() : ServerPacket(SMSG_LFG_LIST_SEARCH_RESULT_UPDATE, 4) { }

            WorldPacket const* Write() override;

            Array<LfgListSearchResult, 50> ResultUpdate;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LfgList::LFGListBlacklist const& blackList);
ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::LfgList::LFGListBlacklist& blackList);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LfgList::ListSearchResult const& listSearch);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LfgList::MemberInfo const& memberInfo);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LfgList::ListRequest const& join);
ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::LfgList::ListRequest& join);

#endif // LfgListPackets_h__
