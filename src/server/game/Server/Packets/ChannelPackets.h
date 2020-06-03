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

#ifndef ChannelPackets_h__
#define ChannelPackets_h__

#include "Packet.h"
#include "Channel.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace Channel
    {
        class ChannelListResponse final : public ServerPacket
        {
        public:
            struct ChannelPlayer
            {
                ChannelPlayer(ObjectGuid const& guid, uint32 realm, uint8 flags) : Guid(guid), VirtualRealmAddress(realm), Flags(flags) { }

                ObjectGuid Guid;
                uint32 VirtualRealmAddress;
                uint8 Flags;
            };

            ChannelListResponse() : ServerPacket(SMSG_CHANNEL_LIST, 4 + 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            std::vector<ChannelPlayer> _Members;
            std::string _Channel;
            uint32 _ChannelFlags = 0;
            bool _Display = false;
        };

        class ChannelNotify final : public ServerPacket
        {
        public:
            ChannelNotify() : ServerPacket(SMSG_CHANNEL_NOTIFY, 16 * 3 + 4 * 3 + 4 * 2 + 1 * 3) { }

            WorldPacket const* Write() override;
            
            ObjectGuid SenderGuid;
            ObjectGuid SenderAccountID;
            ObjectGuid TargetGuid;
            uint32 SenderVirtualRealm = 0;
            uint32 TargetVirtualRealm = 0;
            int32 ChatChannelID = 0;
            std::string Sender;
            std::string _Channel;
            uint8 Type = 0;
            uint8 OldFlags = 0;
            uint8 NewFlags = 0;
        };

        class ChannelNotifyJoined final : public ServerPacket
        {
        public:
            ChannelNotifyJoined() : ServerPacket(SMSG_CHANNEL_NOTIFY_JOINED, 50) { }

            WorldPacket const* Write() override;

            std::string ChannelWelcomeMsg;
            int32 ChatChannelID = 0;
            int32 InstanceID = 0;
            uint32 _ChannelFlags = 0;
            std::string _Channel;
        };

        class ChannelNotifyLeft final : public ServerPacket
        {
        public:
            ChannelNotifyLeft() : ServerPacket(SMSG_CHANNEL_NOTIFY_LEFT, 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            std::string Channel;
            int32 ChatChannelID = 0;
            bool Suspended = false;
        };

        class UserlistAdd final : public ServerPacket
        {
        public:
            UserlistAdd() : ServerPacket(SMSG_USERLIST_ADD, 30) { }

            WorldPacket const* Write() override;

            ObjectGuid AddedUserGUID;
            uint32 _ChannelFlags = CHANNEL_FLAG_NONE; ///< @see enum ChannelFlags
            uint8 UserFlags = MEMBER_FLAG_NONE;
            int32 ChannelID = 0;
            std::string ChannelName;
        };

        class UserlistRemove final : public ServerPacket
        {
        public:
            UserlistRemove() : ServerPacket(SMSG_USERLIST_REMOVE, 30) { }

            WorldPacket const* Write() override;

            ObjectGuid RemovedUserGUID;
            uint32 _ChannelFlags = CHANNEL_FLAG_NONE; ///< @see enum ChannelFlags
            uint32 ChannelID = 0;
            std::string ChannelName;
        };

        class UserlistUpdate final : public ServerPacket
        {
        public:
            UserlistUpdate() : ServerPacket(SMSG_USERLIST_UPDATE, 30) { }

            WorldPacket const* Write() override;

            ObjectGuid UpdatedUserGUID;
            uint32 _ChannelFlags = CHANNEL_FLAG_NONE; ///< @see enum ChannelFlags
            uint8 UserFlags = MEMBER_FLAG_NONE;
            int32 ChannelID = 0;
            std::string ChannelName;
        };

        class ChannelPlayerCommand final : public ClientPacket
        {
        public:
            ChannelPlayerCommand(WorldPacket&& packet);

            void Read() override;

            std::string ChannelName;
            std::string Name;
        };

        class JoinChannel final : public ClientPacket
        {
        public:
            JoinChannel(WorldPacket&& packet) : ClientPacket(CMSG_CHAT_JOIN_CHANNEL, std::move(packet)) { }

            void Read() override;

            std::string Password;
            std::string ChannelName;
            bool CreateVoiceSession = false;
            int32 ChatChannelId = 0;
            bool Internal = false;
        };

        class LeaveChannel final : public ClientPacket
        {
        public:
            LeaveChannel(WorldPacket&& packet) : ClientPacket(CMSG_CHAT_LEAVE_CHANNEL, std::move(packet)) { }

            void Read() override;

            int32 ZoneChannelID = 0;
            std::string ChannelName;
        };
    }
}

#endif // ChannelPackets_h__
