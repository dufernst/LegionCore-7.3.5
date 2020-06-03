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

#ifndef TicketPackets_h__
#define TicketPackets_h__

#include "Packet.h"
#include "LFGPacketsCommon.h"

namespace WorldPackets
{
    namespace Ticket
    {
        struct SupportTicketHeader
        {
            TaggedPosition<Position::XYZ> Pos;
            int32 MapID = 0;
            float Facing = 0.0f;
        };

        class Complaint final : public ClientPacket
        {
        public:
            Complaint(WorldPacket&& packet) : ClientPacket(CMSG_COMPLAINT, std::move(packet)) { }

            void Read() override;

            enum SupportSpamType
            {
                SUPPORT_SPAM_TYPE_MAIL     = 0,
                SUPPORT_SPAM_TYPE_CHAT     = 1,
                SUPPORT_SPAM_TYPE_CALENDAR = 2
            };

            struct ComplaintOffender
            {
                ObjectGuid PlayerGuid;
                uint32 RealmAddress = 0;
                uint32 TimeSinceOffence = 0;
            };

            struct ComplaintChat
            {
                uint32 Command = 0;
                uint32 ChannelID = 0;
                std::string MessageLog;
            };

            ComplaintOffender Offender;
            ComplaintChat Chat;
            ObjectGuid EventGuid;
            ObjectGuid InviteGuid;
            uint32 MailID = 0;
            uint8 ComplaintType = 0;
        };

        class ComplaintResult final : public ServerPacket
        {
        public:
            ComplaintResult() : ServerPacket(SMSG_COMPLAINT_RESULT, 9) { }

            WorldPacket const* Write() override;

            uint32 ComplaintType = 0;
            uint8 Result = 0;
        };

        class SupportTicketSubmitBug final : public ClientPacket
        {
        public:
            SupportTicketSubmitBug(WorldPacket&& packet) : ClientPacket(CMSG_SUPPORT_TICKET_SUBMIT_BUG, std::move(packet)) { }

            void Read() override;

            SupportTicketHeader Header;
            std::string Note;
        };

        class GMTicketGetSystemStatus final : public ClientPacket
        {
        public:
            GMTicketGetSystemStatus(WorldPacket&& packet) : ClientPacket(CMSG_GM_TICKET_GET_SYSTEM_STATUS, std::move(packet)) { }

            void Read() override { }
        };

        class GMTicketAcknowledgeSurvey final : public ClientPacket
        {
        public:
            GMTicketAcknowledgeSurvey(WorldPacket&& packet) : ClientPacket(CMSG_GM_TICKET_ACKNOWLEDGE_SURVEY, std::move(packet)) { }

            void Read() override;

            int32 CaseID = 0;
        };

        class GMTicketGetCaseStatus final : public ClientPacket
        {
        public:
            GMTicketGetCaseStatus(WorldPacket&& packet) : ClientPacket(CMSG_GM_TICKET_GET_CASE_STATUS, std::move(packet)) { }

            void Read() override { }
        };

        class GMTicketCaseStatus final : public ServerPacket
        {
        public:

            GMTicketCaseStatus() : ServerPacket(SMSG_GM_TICKET_CASE_STATUS, 12) { }

            WorldPacket const* Write() override;

            struct GMTicketCase
            {
                int32 CaseID = 0;
                int32 CaseOpened = 0;
                int32 CaseStatus = 0;
                int16 CfgRealmID = 0;
                int64 CharacterID = 0;
                int32 WaitTimeOverrideMinutes = 0;
                std::string Url;
                std::string WaitTimeOverrideMessage;
            };

            std::vector<GMTicketCase> Cases;
        };

        class GMTicketSystemStatus final : public ServerPacket
        {
        public:
            GMTicketSystemStatus() : ServerPacket(SMSG_GM_TICKET_SYSTEM_STATUS, 4) { }

            WorldPacket const* Write() override;

            int32 Status = 0;
        };

        class SupportTicketSubmitComplaint final : public ClientPacket
        {
        public:

            SupportTicketSubmitComplaint(WorldPacket&& packet) : ClientPacket(CMSG_SUPPORT_TICKET_SUBMIT_COMPLAINT, std::move(packet)) { }

            void Read() override;

            struct SupportTicketChatLine
            {
                SupportTicketChatLine(ByteBuffer& data);
                SupportTicketChatLine(uint32 timestamp, std::string  text);

                uint32 Timestamp = 0;
                std::string Text;
            };

            struct SupportTicketChatLog
            {
                std::vector<SupportTicketChatLine> Lines;
                Optional<uint32> ReportLineIndex;
            };

            struct SupportTicketMailInfo
            {
                int32 MailID = 0;
                std::string MailSubject;
                std::string MailBody;
            };

            struct SupportTicketCalendarEventInfo
            {
                uint64 EventID;
                uint64 InviteID;
                std::string EventTitle;
            };

            struct SupportTicketPetInfo
            {
                ObjectGuid PetID;
                std::string PetName;
            };

            struct SupportTicketGuildInfo
            {
                ObjectGuid GuildID;
                std::string GuildName;
            };

            struct SupportTicketLFGListSearchResult
            {
                LFG::RideTicket RideTicket;
                ObjectGuid LastTitleAuthorGuid;
                ObjectGuid LastDescriptionAuthorGuid;
                ObjectGuid LastVoiceChatAuthorGuid;
                ObjectGuid ListingCreatorGuid;
                ObjectGuid Unknown735;
                uint32 GroupFinderActivityID = 0;
                std::string Title;
                std::string Description;
                std::string VoiceChat;
            };

            struct SupportTicketLFGListApplicant
            {
                LFG::RideTicket RideTicket;
                std::string Comment;
            };
            
            Optional<SupportTicketMailInfo> MailInfo;
            Optional<SupportTicketCalendarEventInfo> CalenderInfo;
            Optional<SupportTicketPetInfo> PetInfo;
            Optional<SupportTicketGuildInfo> GuildInfo;
            Optional<SupportTicketLFGListSearchResult> LFGListSearchResult;
            Optional<SupportTicketLFGListApplicant> LFGListApplicant;
            SupportTicketHeader Header;
            SupportTicketChatLog ChatLog;
            ObjectGuid TargetCharacterGUID;
            std::string Note;
            uint8 ComplaintType = 0;
        };

        class SupportTicketSubmitSuggestion final : public ClientPacket
        {
        public:
            SupportTicketSubmitSuggestion(WorldPacket&& packet) : ClientPacket(CMSG_SUPPORT_TICKET_SUBMIT_SUGGESTION, std::move(packet)) { }

            void Read() override;

            SupportTicketHeader Header;
            std::string Note;
        };
    }
}

#endif // TicketPackets_h__
