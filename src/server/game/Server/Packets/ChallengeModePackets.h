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

#ifndef ChallengeModePacketsWorld_h__
#define ChallengeModePacketsWorld_h__

#include "Packet.h"

namespace WorldPackets
{
    namespace ChallengeMode
    {
        struct ModeAttempt
        {
            struct Member
            {
                ObjectGuid Guid;
                uint32 VirtualRealmAddress = 0;
                uint32 NativeRealmAddress = 0;
                uint32 SpecializationID = 0;
            };

            uint32 InstanceRealmAddress = 0;
            uint32 AttemptID = 0;
            uint32 CompletionTime = 0;
            time_t CompletionDate = time(nullptr);
            uint32 MedalEarned = 0;
            std::vector<Member> Members;
        };

        struct ItemReward
        {
            uint32 ItemID = 0;
            uint32 ItemDisplayID = 0;
            uint32 Quantity = 0;
        };

        struct CurrencyReward
        {
            CurrencyReward(uint32 ID, uint32 count) : CurrencyID(ID), Quantity(count) { }

            uint32 CurrencyID = 0;
            uint32 Quantity = 0;
        };

        struct MapChallengeModeReward
        {
            struct ChallengeModeReward
            {
                std::vector<ItemReward> ItemRewards;
                uint32 Money = 0;
                std::vector<CurrencyReward> CurrencyRewards;
            };

            uint32 MapId = 0;
            std::vector<ChallengeModeReward> Rewards;
        };

        struct ChallengeModeMap
        {
            uint32 MapId = 0;
            uint32 BestCompletionMilliseconds = 0;
            uint32 LastCompletionMilliseconds = 0;
            uint32 CompletedChallengeLevel = 0;
            uint32 ChallengeID = 0;
            time_t BestMedalDate = time(nullptr);
            std::vector<uint16> BestSpecID;
            std::array<uint32, 3> Affixes;
        };

        //< CMSG_CHALLENGE_MODE_REQUEST_MAP_STATS
        //< CMSG_GET_CHALLENGE_MODE_REWARDS
        class Misc final : public ClientPacket
        {
        public:
            Misc(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override { }
        };

        class RequestLeaders final : public ClientPacket
        {
        public:
            RequestLeaders(WorldPacket&& packet) : ClientPacket(CMSG_CHALLENGE_MODE_REQUEST_LEADERS, std::move(packet)) { }

            void Read() override;

            uint32 MapId = 0;
            uint32 ChallengeID = 0;
            time_t LastGuildUpdate = time(nullptr);
            time_t LastRealmUpdate = time(nullptr);
        };

        class RequestLeadersResult final : public ServerPacket
        {
        public:
            RequestLeadersResult() : ServerPacket(SMSG_CHALLENGE_MODE_REQUEST_LEADERS_RESULT, 20 + 8) { }

            WorldPacket const* Write() override;

            uint32 MapID = 0;
            uint32 ChallengeID = 0;
            time_t LastGuildUpdate = time(nullptr);
            time_t LastRealmUpdate = time(nullptr);
            std::vector<ModeAttempt> GuildLeaders;
            std::vector<ModeAttempt> RealmLeaders;
        };

        class Rewards final : public ServerPacket
        {
        public:
            Rewards() : ServerPacket(SMSG_CHALLENGE_MODE_REWARDS, 8) { }

            WorldPacket const* Write() override;

            std::vector<MapChallengeModeReward> MapChallengeModeRewards;
            std::vector<ItemReward> ItemRewards;
        };

        class AllMapStats final : public ServerPacket
        {
        public:
            AllMapStats() : ServerPacket(SMSG_CHALLENGE_MODE_ALL_MAP_STATS, 4) { }

            WorldPacket const* Write() override;

            std::vector<ChallengeModeMap> ChallengeModeMaps;
        };

        class ChallengeModeReset final : public ServerPacket
        {
        public:
            ChallengeModeReset(uint32 mapID) : ServerPacket(SMSG_CHALLENGE_MODE_RESET, 4), MapID(mapID) { }

            WorldPacket const* Write() override;

           uint32 MapID = 0;
        };

        class ChallengeModeStart final : public ServerPacket
        {
        public:
            ChallengeModeStart() : ServerPacket(SMSG_CHALLENGE_MODE_START, 33) { }

            WorldPacket const* Write() override;

            std::array<uint32, 3> Affixes{};
            uint32 MapID = 0;
            int32 ChallengeID = 0;
            uint32 StartedChallengeLevel = 2;
            uint32 DeathCount = 0;
            bool Energized = true;
        };

        class ChallengeModeComplete final : public ServerPacket
        {
        public:
            ChallengeModeComplete() : ServerPacket(SMSG_CHALLENGE_MODE_COMPLETE, 17) { }

            WorldPacket const* Write() override;

            uint32 MapID = 0;
            int32 CompletionMilliseconds = 0;
            int32 StartedChallengeLevel = 0;
            uint32 ChallengeID = 0;
            bool IsCompletedInTimer = false;
        };

        class ChallengeModeNewPlayerRecord final : public ServerPacket
        {
        public:
            ChallengeModeNewPlayerRecord() : ServerPacket(SMSG_CHALLENGE_MODE_NEW_PLAYER_RECORD, 12) { }

            WorldPacket const* Write() override;

            uint32 MapID = 0;
            int32 CompletionMilliseconds = 0;
            int32 StartedChallengeLevel = 0;
        };

        class ChallengeModeMapStatsUpdate final : public ServerPacket
        {
        public:
            ChallengeModeMapStatsUpdate() : ServerPacket(SMSG_CHALLENGE_MODE_MAP_STATS_UPDATE, 12) { }

            WorldPacket const* Write() override;

            ChallengeModeMap Stats{};
        };

        class StartChallengeMode final : public ClientPacket
        {
        public:
            StartChallengeMode(WorldPacket&& packet) : ClientPacket(CMSG_START_CHALLENGE_MODE, std::move(packet)) { }

            void Read() override;

            ObjectGuid GameObjectGUID;
            int32 Slot = 0;
            uint8 Bag = 0;
        };

        class ResetChallengeMode final : public ClientPacket
        {
        public:
            ResetChallengeMode(WorldPacket&& packet) : ClientPacket(CMSG_RESET_CHALLENGE_MODE, std::move(packet)) { }

            void Read() override { }
        };

        class ChallengeModeUpdateDeathCount final : public ServerPacket
        {
        public:
            ChallengeModeUpdateDeathCount() : ServerPacket(SMSG_CHALLENGE_MODE_UPDATE_DEATH_COUNT, 4) { }

            WorldPacket const* Write() override;

            uint32 DeathCount = 0;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::ChallengeMode::ModeAttempt const& modeAttempt);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::ChallengeMode::ItemReward const& itemReward);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::ChallengeMode::MapChallengeModeReward const& mapChallengeModeReward);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::ChallengeMode::ChallengeModeMap const& challengeModeMap);

#endif // ChallengeModePacketsWorld_h__
