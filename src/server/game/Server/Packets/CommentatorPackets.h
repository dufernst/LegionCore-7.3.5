#ifndef CommentatorPackets_h__
#define CommentatorPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace Commentator
    {
        struct ServerSpec
        {
            uint32 Realm;
            uint16 Server;
            uint8 Type;
        };

        struct CommentatorSpellChargeEntry
        {
            uint32 Unk;
            uint32 Unk2;
        };

        struct SpellChargeEntry
        {
            uint32 Unk;
            uint32 Unk2;
            uint32 Unk3;
            uint32 Unk4;
        };

        struct SpellHistoryEntry
        {
            uint32 Unk;
            uint32 Unk2;
            uint32 Unk3;
            uint32 Unk4;
            uint32 Unk5;
            bool Unk6;
            bool Unk7;
            bool Unk8;
        };

        struct CommentatorPlayer
        {
            ObjectGuid Guid;
            ServerSpec UserServer;
        };

        struct CommentatorPlayerData
        {
            ObjectGuid PlayerGUID;
            uint32 DamageDone;
            uint32 DamageTaken;
            uint32 HealingDone;
            uint32 HealingTaken;
            uint32 SpecID;
            uint16 Kills;
            uint16 Deaths;
            uint8 FactionIndex;
            std::list<CommentatorSpellChargeEntry> CommentatorSpellChargeEntries;
            std::list<SpellChargeEntry> SpellChargeEntries;
            std::list<SpellHistoryEntry> SpellHistoryEntries;
        };

        struct CommentatorTeam
        {
            ObjectGuid Guid;
            std::list<CommentatorPlayer> Players;
        };

        struct CommentatorInstance
        {
            uint32 MapID;
            ServerSpec WorldServer;
            uint64 InstanceID;
            uint32 Status;
            CommentatorTeam Teams[2];
        };

        struct CommentatorMap
        {
            uint32 TeamSize;
            uint32 MinLevelRange;
            uint32 MaxLevelRange;
            int32 DifficultyID;
            std::list<CommentatorInstance> Instances;
        };

        class CommentatorEnable final : public ClientPacket
        {
            public:
                CommentatorEnable(WorldPacket&& packet) : ClientPacket(CMSG_COMMENTATOR_ENABLE, std::move(packet)) { }

                void Read() override;

                uint32 Enable;
        };

        class CommentatorGetMapInfo final : public ClientPacket
        {
            public:
                CommentatorGetMapInfo(WorldPacket&& packet) : ClientPacket(CMSG_COMMENTATOR_GET_MAP_INFO, std::move(packet)) { }

                void Read() override;

                std::string PlayerName;
        };

        class CommentatorGetPlayerInfo final : public ClientPacket
        {
            public:
                CommentatorGetPlayerInfo(WorldPacket&& packet) : ClientPacket(CMSG_COMMENTATOR_GET_PLAYER_INFO, std::move(packet)) { }

                void Read() override;

                ServerSpec WorldServer;
                uint32 MapID;
        };

        class CommentatorEnterInstance final : public ClientPacket
        {
            public:
                CommentatorEnterInstance(WorldPacket&& packet) : ClientPacket(CMSG_COMMENTATOR_ENTER_INSTANCE, std::move(packet)) { }

                void Read() override;

                uint32 Enable;
        };

        class CommentatorExitInstance final : public ClientPacket
        {
            public:
                CommentatorExitInstance(WorldPacket&& packet) : ClientPacket(CMSG_COMMENTATOR_EXIT_INSTANCE, std::move(packet)) { }

                void Read() override {}
        };

        // CMSG_COMMENTATOR_START_WARGAME - not supported
        // other possible undefined commentator packets (CMSG) - not supported

        class CommentatorMapInfo final : public ServerPacket
        {
            public:
                CommentatorMapInfo() : ServerPacket(SMSG_COMMENTATOR_MAP_INFO, 16 + 1) { }

                WorldPacket const* Write() override;

                uint64 PlayerInstanceID;
                std::list<CommentatorMap> Maps;
        };

        class CommentatorPlayerInfo final : public ServerPacket
        {
            public:
                CommentatorPlayerInfo() : ServerPacket(SMSG_COMMENTATOR_PLAYER_INFO, 16 + 1) { }

                WorldPacket const* Write() override;

                uint32 MapID;
                std::list<CommentatorPlayerData> PlayerInfo;
                uint64 InstanceID;
                ServerSpec WorldServer;
        };

        class CommentatorStateChanged final : public ServerPacket
        {
            public:
                CommentatorStateChanged() : ServerPacket(SMSG_COMMENTATOR_STATE_CHANGED, 16 + 1) { }

                WorldPacket const* Write() override;

                ObjectGuid Guid;
                bool Enable;
        };

        // other possible undefined commentator packets (SMSG) - not supported
    }
}

#endif // CommentatorPackets_h__
