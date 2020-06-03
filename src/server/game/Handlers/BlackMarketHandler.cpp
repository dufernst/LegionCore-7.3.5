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

#include "BlackMarketPackets.h"
#include "BlackMarketMgr.h"
#include "DatabaseEnv.h"

void WorldSession::HandleBlackMarketOpen(WorldPackets::BlackMarket::BlackMarketOpen& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetNPCIfCanInteractWith(packet.Guid, UNIT_NPC_FLAG_BLACK_MARKET))
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);


    WorldPackets::BlackMarket::BlackMarketOpenResult response;
    response.Guid = packet.Guid;
    response.Enable = sBlackMarketMgr->IsEnabled();
    SendPacket(response.Write());
}

void WorldSession::HandleBlackMarketRequestItems(WorldPackets::BlackMarket::BlackMarketRequestItems& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!sBlackMarketMgr->IsEnabled())
        return;

    if (!player->GetNPCIfCanInteractWith(packet.Guid, UNIT_NPC_FLAG_BLACK_MARKET))
        return;

    WorldPackets::BlackMarket::BlackMarketRequestItemsResult result;
    sBlackMarketMgr->BuildItemsResponse(result, player);
    SendPacket(result.Write());
}

void WorldSession::HandleBlackMarketBidOnItem(WorldPackets::BlackMarket::BlackMarketBidOnItem& packet)
{
    if (!sBlackMarketMgr->IsEnabled())
        return;

    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetNPCIfCanInteractWith(packet.Guid, UNIT_NPC_FLAG_BLACK_MARKET);
    if (!unit)
        return;

    BlackMarketEntry* entry = sBlackMarketMgr->GetAuctionByID(packet.MarketID);
    if (!entry)
    {
        SendBlackMarketBidOnItemResult(ERR_BMAH_ITEM_NOT_FOUND, packet.MarketID, packet.Item);
        return;
    }

    if (entry->GetBidder() == player->GetGUIDLow())
    {
        SendBlackMarketBidOnItemResult(ERR_BMAH_ALREADY_BID, packet.MarketID, packet.Item);
        return;
    }

    if (!entry->ValidateBid(packet.BidAmount))
    {
        SendBlackMarketBidOnItemResult(ERR_BMAH_HIGHER_BID, packet.MarketID, packet.Item);
        return;
    }

    if (!player->HasEnoughMoney(packet.BidAmount))
    {
        SendBlackMarketBidOnItemResult(ERR_BMAH_NOT_ENOUGH_MONEY, packet.MarketID, packet.Item);
        return;
    }

    if (entry->GetSecondsRemaining() <= 0)
    {
        SendBlackMarketBidOnItemResult(ERR_BMAH_DATABASE_ERROR, packet.MarketID, packet.Item);
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    sBlackMarketMgr->SendAuctionOutbidMail(entry, trans);
    entry->PlaceBid(packet.BidAmount, player, trans);
    CharacterDatabase.CommitTransaction(trans);
    SendBlackMarketBidOnItemResult(ERR_BMAH_OK, packet.MarketID, packet.Item);
}

void WorldSession::SendBlackMarketBidOnItemResult(int32 result, int32 marketId, WorldPackets::Item::ItemInstance& item)
{
    WorldPackets::BlackMarket::BlackMarketBidOnItemResult packet;

    packet.MarketID = marketId;
    packet.Item = item;
    packet.Result = result;

    SendPacket(packet.Write());
}

void WorldSession::SendBlackMarketWonNotification(BlackMarketEntry const* entry, Item const* item)
{
    WorldPackets::BlackMarket::BlackMarketWon packet;
    packet.MarketID = entry->GetMarketId();
    packet.Item.Initialize(item);
    packet.RandomPropertiesID = item->GetItemRandomPropertyId();

    SendPacket(packet.Write());
}

void WorldSession::SendBlackMarketOutbidNotification(BlackMarketTemplate const* templ)
{
    WorldPackets::BlackMarket::BlackMarketOutbid packet;
    packet.MarketID = templ->MarketID;
    packet.Item = templ->Item;
    packet.RandomPropertiesID = templ->Item.RandomPropertiesID;

    SendPacket(packet.Write());
}
