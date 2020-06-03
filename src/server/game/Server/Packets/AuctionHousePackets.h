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

#ifndef AuctionHousePackets_h__
#define AuctionHousePackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "ItemPackets.h"
#include "MailPackets.h"

struct AuctionEntry;

namespace WorldPackets
{
    namespace AuctionHouse
    {
        struct AuctionItem
        {
            std::vector<Item::ItemEnchantData> Enchantments;
            std::vector<Item::ItemGemData> Gems;
            Item::ItemInstance Item;
            ObjectGuid Owner;
            ObjectGuid ItemGuid;
            ObjectGuid OwnerAccountID;
            ObjectGuid Bidder;
            uint64 MinBid = 0;
            uint64 MinIncrement = 0;
            uint64 BuyoutPrice = 0;
            uint64 BidAmount = 0;
            uint32 EndTime = 0;
            uint32 Flags = 0;
            int32 Count = 0;
            int32 Charges = 0;
            int32 AuctionItemID = 0;
            int32 DurationLeft = 0;
            uint8 DeleteReason = 0;
            bool CensorServerSideInfo = false;
            bool CensorBidInfo = false;
        };

        struct AuctionOwnerNotification
        {
            void Initialize(AuctionEntry const* auction, ::Item const* item);

            int32 AuctionItemID = 0;
            uint64 BidAmount = 0;
            Item::ItemInstance Item;
        };

        struct AuctionBidderNotification
        {
            void Initialize(AuctionEntry const* auction, ::Item const* item);

            int32 AuctionItemID = 0;
            ObjectGuid Bidder;
            Item::ItemInstance Item;
        };

        class AuctionHelloRequest final : public ClientPacket
        {
        public:
            AuctionHelloRequest(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_HELLO_REQUEST, std::move(packet)) { }

            void Read() override;
            ObjectGuid Guid;
        };

        class AuctionHelloResponse final : public ServerPacket
        {
        public:
            AuctionHelloResponse() : ServerPacket(SMSG_AUCTION_HELLO_RESPONSE, 1 + 16) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            bool OpenForBusiness = true;
        };

        class AuctionCommandResult final : public ServerPacket
        {
        public:
            AuctionCommandResult() : ServerPacket(SMSG_AUCTION_COMMAND_RESULT, 32 + 16 + 16) { }

            void InitializeAuction(AuctionEntry* auction);

            WorldPacket const* Write() override;
            
            ObjectGuid Guid;
            uint64 MinIncrement = 0;
            uint64 Money = 0;
            uint32 AuctionItemID = 0;
            uint32 Command = 0;
            int32 ErrorCode = 0;
            int32 BagResult = 0;
        };

        class AuctionSellItem final : public ClientPacket
        {
        public:
            struct AuctionItemForSale
            {
                ObjectGuid Guid;
                uint32 UseCount = 0;
            };
            
            AuctionSellItem(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_SELL_ITEM, std::move(packet)) { }

            void Read() override;

            ObjectGuid Auctioneer;
            uint64 BuyoutPrice = 0;
            uint64 MinBid = 0;
            uint32 RunTime = 0;
            Array<AuctionItemForSale, 32> Items;
        };

        class AuctionPlaceBid final : public ClientPacket
        {
        public:
            AuctionPlaceBid(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_PLACE_BID, std::move(packet)) { }

            void Read() override;

            ObjectGuid Auctioneer;
            uint64 BidAmount = 0;
            int32 AuctionItemID = 0;
        };

        class AuctionListBidderItems final : public ClientPacket
        {
        public:
            AuctionListBidderItems(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_LIST_BIDDER_ITEMS, std::move(packet)) { }

            void Read() override;

            uint32 Offset = 0;
            std::vector<uint32> AuctionItemIDs;
            ObjectGuid Auctioneer;
        };

        class AuctionRemoveItem final : public ClientPacket
        {
        public:
            AuctionRemoveItem(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_REMOVE_ITEM, std::move(packet)) { }

            void Read() override;

            ObjectGuid Auctioneer;
            int32 AuctionItemID = 0;
        };

        class AuctionReplicateItems final : public ClientPacket
        {
        public:
            AuctionReplicateItems(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_REPLICATE_ITEMS, std::move(packet)) { }

            void Read() override;

            ObjectGuid Auctioneer;
            int32 Count = 0;
            int32 ChangeNumberGlobal = 0;
            int32 ChangeNumberCursor = 0;
            int32 ChangeNumberTombstone = 0;
        };

        class AuctionListPendingSales final : public ClientPacket
        {
        public:
            AuctionListPendingSales(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_LIST_PENDING_SALES, std::move(packet)) { }

            void Read() override { }
        };

        class AuctionListItemsResult final : public ServerPacket
        {
        public:
            AuctionListItemsResult() : ServerPacket(SMSG_AUCTION_LIST_ITEMS_RESULT, 150) { }

            WorldPacket const* Write() override;

            std::vector<AuctionItem> Items;
            uint32 DesiredDelay = 0;
            uint32 TotalCount = 0;
            bool OnlyUsable = true;
        };

        class AuctionListOwnerItemsResult final : public ServerPacket
        {
        public:
            AuctionListOwnerItemsResult() : ServerPacket(SMSG_AUCTION_LIST_OWNER_ITEMS_RESULT, 149) { }

            WorldPacket const* Write() override;

            uint32 DesiredDelay = 0;
            uint32 TotalCount = 0;
            std::vector<AuctionItem> Items;
        };

        class AuctionListBidderItemsResult final : public ServerPacket
        {
        public:
            AuctionListBidderItemsResult() : ServerPacket(SMSG_AUCTION_LIST_BIDDER_ITEMS_RESULT, 149) { }

            WorldPacket const* Write() override;

            uint32 DesiredDelay = 0;
            uint32 TotalCount = 0;
            std::vector<AuctionItem> Items;
        };

        class AuctionListOwnerItems final : public ClientPacket
        {
        public:
            AuctionListOwnerItems(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_LIST_OWNER_ITEMS, std::move(packet)) { }

            void Read() override;

            ObjectGuid Auctioneer;
            uint32 Offset = 0;
        };

        class AuctionListItems final : public ClientPacket
        {
        public:
            AuctionListItems(WorldPacket&& packet) : ClientPacket(CMSG_AUCTION_LIST_ITEMS, std::move(packet)) { }

            void Read() override;
            
            struct Sort
            {
                uint8 Type = 0;
                uint8 Direction = 0;
            };

            struct ClassFilter
            {
                struct SubClassFilter
                {
                    int32 ItemSubclass = 0;
                    uint32 InvTypeMask = 0;
                };

                int32 ItemClass = 0;
                Array<SubClassFilter, 31> SubClassFilters;
            };
            
            std::vector<Sort> DataSort;
            Array<ClassFilter, 7> ClassFilters;
            Array<uint8, BATTLE_PET_SPECIES_MAX_ID / 8 + 1> KnownPets;
            ObjectGuid Auctioneer;
            uint32 Offset = 0;
            int32 Quality = 0;
            std::string Name;
            uint8 MinLevel = 1;
            uint8 SortCount = 0;
            uint8 MaxLevel = 100;
            int8 MaxPetLevel = 0;
            bool ExactMatch = true;
            bool OnlyUsable = false;
        };

        class AuctionListPendingSalesResult final : public ServerPacket
        {
        public:
            AuctionListPendingSalesResult() : ServerPacket(SMSG_AUCTION_LIST_PENDING_SALES_RESULT, 140) { }

            WorldPacket const* Write() override;

            std::vector<Mail::MailListEntry> Mails;
            int32 TotalNumRecords = 0;
        };

        class AuctionClosedNotification final : public ServerPacket
        {
        public:
            AuctionClosedNotification() : ServerPacket(SMSG_AUCTION_CLOSED_NOTIFICATION, 45) { }

            WorldPacket const* Write() override;

            AuctionOwnerNotification Info;
            float ProceedsMailDelay = 0.0f;
            bool Sold = true;
        };

        class AuctionOwnerBidNotification final : public ServerPacket
        {
        public:
            AuctionOwnerBidNotification() : ServerPacket(SMSG_AUCTION_OWNER_BID_NOTIFICATION, 62) { }

            WorldPacket const* Write() override;

            AuctionOwnerNotification Info;
            ObjectGuid Bidder;
            uint64 MinIncrement = 0;
        };

        class AuctionWonNotification final : public ServerPacket
        {
        public:
            AuctionWonNotification() : ServerPacket(SMSG_AUCTION_WON_NOTIFICATION, 46) { }

            WorldPacket const* Write() override;

            AuctionBidderNotification Info;
        };

        class AuctionOutBidNotification final : public ServerPacket
        {
        public:
            AuctionOutBidNotification() : ServerPacket(SMSG_AUCTION_OUTBID_NOTIFICATION, 62) { }

            WorldPacket const* Write() override;

            AuctionBidderNotification Info;
            uint64 BidAmount = 0;
            uint64 MinIncrement = 0;
        };

        class AuctionReplicateResponse final : public ServerPacket
        {
        public:
            AuctionReplicateResponse() : ServerPacket(SMSG_AUCTION_REPLICATE_RESPONSE, 165) { }

            WorldPacket const* Write() override;

            uint32 ChangeNumberCursor = 0;
            uint32 ChangeNumberGlobal = 0;
            uint32 DesiredDelay = 0;
            uint32 ChangeNumberTombstone = 0;
            uint32 Result = 0;
            std::vector<AuctionItem> Items;
        };
    }
}

#endif // AuctionHousePackets_h__
