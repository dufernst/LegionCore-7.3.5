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

#ifndef LootPackets_h__
#define LootPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "ItemPackets.h"

namespace WorldPackets
{
    namespace Loot
    {
        class LootUnit final : public ClientPacket
        {
        public:
            LootUnit(WorldPacket&& packet) : ClientPacket(CMSG_LOOT_UNIT, std::move(packet)) { }

            void Read() override;

            ObjectGuid Unit;
        };

        struct LootItem
        {
            uint8 Type = 0;
            uint8 UIType = LOOT_ITEM_UI_OWNER;
            uint32 Quantity = 0;
            uint8 LootItemType = 0;
            uint8 LootListID = 0;
            bool CanTradeToTapList = false;
            Item::ItemInstance Loot;
        };

        struct LootCurrency
        {
            uint32 CurrencyID = 0;
            uint32 Quantity = 0;
            uint8 LootListID = 0;
            uint8 UIType = 0;
        };

        class LootResponse final : public ServerPacket
        {
        public:
            LootResponse() : ServerPacket(SMSG_LOOT_RESPONSE, 100) { }

            WorldPacket const* Write() override;

            std::vector<LootItem> Items;
            std::vector<LootCurrency> Currencies;
            ObjectGuid LootObj;
            ObjectGuid Owner;
            uint32 Coins = 0;
            uint8 Threshold = ITEM_QUALITY_UNCOMMON; // Most common value
            uint8 LootMethod = FREE_FOR_ALL;
            uint8 AcquireReason = LOOT_NONE;
            uint8 FailureReason = LOOT_ERROR_NO_LOOT; // Most common value
            bool Acquired = false;
            bool AELooting = false;
        };

        struct LootRequest
        {
            ObjectGuid Object;
            uint8 LootListID = 0;
        };

        class AutoStoreLootItem final : public ClientPacket
        {
        public:
            AutoStoreLootItem(WorldPacket&& packet) : ClientPacket(CMSG_LOOT_ITEM, std::move(packet)) { }

            void Read() override;

            std::vector<LootRequest> Loot;
        };

        class LootRemoved final : public ServerPacket
        {
        public:
            LootRemoved() : ServerPacket(SMSG_LOOT_REMOVED, 30) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            ObjectGuid Owner;
            uint8 LootListID = 0;
        };

        class LootRelease final : public ClientPacket
        {
        public:
            LootRelease(WorldPacket&& packet) : ClientPacket(CMSG_LOOT_RELEASE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Unit;
        };

        class LootMoney final : public ClientPacket
        {
        public:
            LootMoney(WorldPacket&& packet) : ClientPacket(CMSG_LOOT_MONEY, std::move(packet)) { }

            void Read() override { }
        };

        class LootMoneyNotify final : public ServerPacket
        {
        public:
            LootMoneyNotify() : ServerPacket(SMSG_LOOT_MONEY_NOTIFY, 5) { }

            WorldPacket const* Write() override;

            uint32 Money = 0;
            bool SoleLooter = false;
        };

        class StartLootRoll final : public ServerPacket
        {
        public:
            StartLootRoll() : ServerPacket(SMSG_START_LOOT_ROLL, 16 + 33) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            LootItem Item;
            uint32 MapID = 0;
            uint32 RollTime = 0;
            uint8 ValidRolls = 0;
            uint8 Method = 0;
        };

        class CoinRemoved final : public ServerPacket
        {
        public:
            CoinRemoved() : ServerPacket(SMSG_COIN_REMOVED, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
        };

        class LootRoll final : public ClientPacket
        {
        public:
            LootRoll(WorldPacket&& packet) : ClientPacket(CMSG_LOOT_ROLL, std::move(packet)) { }

            void Read() override;

            ObjectGuid LootObj;
            uint8 LootListID = 0;
            uint8 RollType = 0;
        };

        //< SMSG_AE_LOOT_TARGET_ACK
        //< SMSG_SET_LOOT_METHOD_FAILED
        //< SMSG_BONUS_ROLL_EMPTY
        //< SMSG_LOOT_RELEASE_ALL
        class NullSMsg final : public ServerPacket
        {
        public:
            NullSMsg(OpcodeServer opcode) : ServerPacket(opcode, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class LootReleaseResponse final : public ServerPacket
        {
        public:
            LootReleaseResponse() : ServerPacket(SMSG_LOOT_RELEASE, 32) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            ObjectGuid Owner;
        };

        class LootRollsComplete final : public ServerPacket
        {
        public:
            LootRollsComplete() : ServerPacket(SMSG_LOOT_ROLLS_COMPLETE, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            uint8 LootListID = 0;
        };

        class AELootTargets final : public ServerPacket
        {
        public:
            AELootTargets() : ServerPacket(SMSG_AE_LOOT_TARGETS, 4) { }

            WorldPacket const* Write() override;

            uint32 Count = 0;
        };

        class LootList final : public ServerPacket
        {
        public:
            LootList() : ServerPacket(SMSG_LOOT_LIST, 2 * 16 + 2) { }

            WorldPacket const* Write() override;

            ObjectGuid Owner;
            ObjectGuid LootObj;
            Optional<ObjectGuid> Master;
            Optional<ObjectGuid> RoundRobinWinner;
        };

        class SetLootSpecialization final : public ClientPacket
        {
        public:
            SetLootSpecialization(WorldPacket&& packet) : ClientPacket(CMSG_SET_LOOT_SPECIALIZATION, std::move(packet)) { }

            void Read() override;

            uint32 SpecID = 0;
        };

        class LootRollResponse final : public ServerPacket
        {
        public:
            LootRollResponse() : ServerPacket(SMSG_LOOT_ROLL, 50) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            ObjectGuid Player;
            LootItem LootItems;
            int32 Roll = 0;
            uint8 RollType = 0;
            bool Autopassed = false;
        };

        class LootRollWon final : public ServerPacket
        {
        public:
            LootRollWon() : ServerPacket(SMSG_LOOT_ROLL_WON, 30) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            ObjectGuid Player;
            LootItem LootItems;
            int32 Roll = 0;
            uint8 RollType = 0;
            bool Autopassed = false;
        };

        class LootAllPassed final : public ServerPacket
        {
        public:
            LootAllPassed() : ServerPacket(SMSG_LOOT_ALL_PASSED, 30) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            LootItem LootItems;
        };

        class MasterLootCandidateList final : public ServerPacket
        {
        public:
            MasterLootCandidateList() : ServerPacket(SMSG_MASTER_LOOT_CANDIDATE_LIST, 4 + 16) { }

            WorldPacket const* Write() override;

            ObjectGuid LootObj;
            GuidVector Players;
        };

        class LootDisplayToast final : public ServerPacket
        {
        public:
            LootDisplayToast() : ServerPacket(SMSG_DISPLAY_TOAST) { }

            WorldPacket const* Write() override;
            
            Item::ItemInstance Loot;
            uint64 Quantity = 0;
            uint32 Type = 0;
            uint32 SpecID = 0;
            uint32 ItemQuantity = 0;
            uint32 CurrencyID = 0;
            uint32 QuestID = 0;
            uint8 DisplayToastMethod = 0;
            bool Mailed = false;
            bool BonusRoll = false;
        };

        class DoMasterLootRoll final : public ClientPacket
        {
        public:
            DoMasterLootRoll(WorldPacket&& packet) : ClientPacket(CMSG_DO_MASTER_LOOT_ROLL, std::move(packet)) { }

            void Read() override;

            ObjectGuid LootObj;
            uint8 LootListID = 0;
        };

        class CancelMasterLootRoll final : public ClientPacket
        {
        public:
            CancelMasterLootRoll(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_MASTER_LOOT_ROLL, std::move(packet)) { }

            void Read() override;

            ObjectGuid LootObj;
            uint8 LootListID = 0;
        };

        class MasterLootItem final : public ClientPacket
        {
        public:
            MasterLootItem(WorldPacket&& packet) : ClientPacket(CMSG_MASTER_LOOT_ITEM, std::move(packet)) { }

            void Read() override;

            Array<LootRequest, 100> Loot;
            ObjectGuid Target;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Loot::LootItem const& item);
ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Loot::LootRequest& loot);

#endif // LootPackets_h__
