/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "AccountMgr.h"
#include "Common.h"
#include "Item.h"
#include "ItemPackets.h"
#include "Language.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerDefines.h"
#include "SocialMgr.h"
#include "Spell.h"
#include "TradeData.h"
#include "TradePackets.h"
#include "World.h"
#include "WorldSession.h"

void WorldSession::SendTradeStatus(WorldPackets::Trade::TradeStatus& packet)
{
    if (!_player)
        return;

    packet.Clear();
    Player* trader = _player->GetTrader();
    packet.PartnerIsSameBnetAccount = trader && trader->GetSession()->GetAccountId() == GetAccountId();
    SendPacket(packet.Write());
}

void WorldSession::HandleIgnoreTrade(WorldPackets::Trade::NullCmsg& /*packet*/)
{
    WorldPackets::Trade::TradeStatus info;
    info.Status = TRADE_STATUS_PLAYER_IGNORED;
    SendTradeStatus(info);
}

void WorldSession::HandleBusyTrade(WorldPackets::Trade::NullCmsg& /*packet*/) { }

void WorldSession::SendUpdateTrade(bool traderData /*= true*/)
{
    TradeData* ViewTrade = traderData ? _player->GetTradeData()->GetTraderData() : _player->GetTradeData();

    WorldPackets::Trade::TradeUpdated packet;
    packet.WhichPlayer = traderData;
    packet.ClientStateIndex = ViewTrade->GetClientStateIndex();
    packet.CurrentStateIndex = ViewTrade->GetServerStateIndex();
    packet.Gold = ViewTrade->GetMoney();
    packet.ProposedEnchantment = ViewTrade->GetSpell();

    for (uint8 i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = ViewTrade->GetItem(TradeSlots(i)))
        {
            WorldPackets::Trade::TradeUpdated::TradeItem tradeItem;
            tradeItem.Slot = i;
            tradeItem.Item.Initialize(item);
            tradeItem.StackCount = item->GetCount();
            tradeItem.GiftCreator = item->GetGuidValue(ITEM_FIELD_GIFT_CREATOR);
            if (!item->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_WRAPPED))
            {
                tradeItem.Unwrapped = boost::in_place();
                tradeItem.Unwrapped->EnchantID = item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT);
                tradeItem.Unwrapped->OnUseEnchantmentID = item->GetEnchantmentId(USE_ENCHANTMENT_SLOT);
                tradeItem.Unwrapped->Creator = item->GetGuidValue(ITEM_FIELD_CREATOR);
                tradeItem.Unwrapped->Charges = item->GetSpellCharges();
                tradeItem.Unwrapped->Lock = item->GetTemplate()->GetLockID() != 0;
                tradeItem.Unwrapped->MaxDurability = item->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY);
                tradeItem.Unwrapped->Durability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

                uint8 g = 0;
                for (ItemDynamicFieldGems const& gemData : item->GetGems())
                {
                    if (gemData.ItemId)
                    {
                        WorldPackets::Item::ItemGemData gem;
                        gem.Slot = g;
                        gem.Item.Initialize(&gemData);
                        tradeItem.Unwrapped->Gems.push_back(gem);
                    }
                    ++g;
                }
            }

            packet.Items.push_back(tradeItem);
        }
    }

    SendPacket(packet.Write());
}

void WorldSession::moveItems(Item* myItems[], Item* hisItems[])
{
    Player* trader = _player->GetTrader();
    if (!trader)
        return;

    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        ItemPosCountVec traderDst;
        ItemPosCountVec playerDst;
        bool traderCanTrade = (myItems[i] == nullptr || trader->CanStoreItem(NULL_BAG, NULL_SLOT, traderDst, myItems[i], false) == EQUIP_ERR_OK);
        bool playerCanTrade = (hisItems[i] == nullptr || _player->CanStoreItem(NULL_BAG, NULL_SLOT, playerDst, hisItems[i], false) == EQUIP_ERR_OK);
        if (traderCanTrade && playerCanTrade)
        {
            // Ok, if trade item exists and can be stored
            // If we trade in both directions we had to check, if the trade will work before we actually do it
            // A roll back is not possible after we stored it
            if (myItems[i])
            {
                // logging
                TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "partner storing: %u", myItems[i]->GetGUIDLow());
                if (!AccountMgr::IsPlayerAccount(_player->GetSession()->GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
                {
                    sLog->outCommand(_player->GetSession()->GetAccountId(), "GM %s (Account: %u) trade: %s (Entry: %d Count: %u) to player: %s (Account: %u)",
                        _player->GetName(), _player->GetSession()->GetAccountId(),
                        myItems[i]->GetTemplate()->GetName()->Str[_player->GetSession()->GetSessionDbLocaleIndex()], myItems[i]->GetEntry(), myItems[i]->GetCount(),
                        trader->GetName(), trader->GetSession()->GetAccountId());
                }

                // adjust time (depends on /played)
                if (myItems[i]->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_BOP_TRADEABLE))
                    myItems[i]->SetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME, trader->GetTotalPlayedTime()-(_player->GetTotalPlayedTime()-myItems[i]->GetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME)));
                // store
                trader->MoveItemToInventory(traderDst, myItems[i], true, true);
            }
            if (hisItems[i])
            {
                // logging
                TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "player storing: %u", hisItems[i]->GetGUIDLow());
                if (!AccountMgr::IsPlayerAccount(trader->GetSession()->GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
                {
                    sLog->outCommand(trader->GetSession()->GetAccountId(), "GM %s (Account: %u) trade: %s (Entry: %d Count: %u) to player: %s (Account: %u)",
                        trader->GetName(), trader->GetSession()->GetAccountId(),
                        hisItems[i]->GetTemplate()->GetName()->Str[trader->GetSession()->GetSessionDbLocaleIndex()], hisItems[i]->GetEntry(), hisItems[i]->GetCount(),
                        _player->GetName(), _player->GetSession()->GetAccountId());
                }

                // adjust time (depends on /played)
                if (hisItems[i]->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_BOP_TRADEABLE))
                    hisItems[i]->SetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME, _player->GetTotalPlayedTime()-(trader->GetTotalPlayedTime()-hisItems[i]->GetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME)));
                // store
                _player->MoveItemToInventory(playerDst, hisItems[i], true, true);
            }
        }
        else
        {
            // in case of fatal error log error message
            // return the already removed items to the original owner
            if (myItems[i])
            {
                if (!traderCanTrade)
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "trader can't store item: %u", myItems[i]->GetGUIDLow());
                if (_player->CanStoreItem(NULL_BAG, NULL_SLOT, playerDst, myItems[i], false) == EQUIP_ERR_OK)
                    _player->MoveItemToInventory(playerDst, myItems[i], true, true);
                else
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "player can't take item back: %u", myItems[i]->GetGUIDLow());
            }
            // return the already removed items to the original owner
            if (hisItems[i])
            {
                if (!playerCanTrade)
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "player can't store item: %u", hisItems[i]->GetGUIDLow());
                if (trader->CanStoreItem(NULL_BAG, NULL_SLOT, traderDst, hisItems[i], false) == EQUIP_ERR_OK)
                    trader->MoveItemToInventory(traderDst, hisItems[i], true, true);
                else
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "trader can't take item back: %u", hisItems[i]->GetGUIDLow());
            }
        }
    }
}

//==============================================================

static void setAcceptTradeMode(TradeData* myTrade, TradeData* hisTrade, Item* *myItems, Item* *hisItems)
{
    myTrade->SetInAcceptProcess(true);
    hisTrade->SetInAcceptProcess(true);

    // store items in local list and set 'in-trade' flag
    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (Item* item = myTrade->GetItem(TradeSlots(i)))
        {
            TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "player trade item %u bag: %u slot: %u", item->GetGUIDLow(), item->GetBagSlot(), item->GetSlot());
            //Can return NULL
            myItems[i] = item;
            myItems[i]->SetInTrade();
        }

        if (Item* item = hisTrade->GetItem(TradeSlots(i)))
        {
            TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "partner trade item %u bag: %u slot: %u", item->GetGUIDLow(), item->GetBagSlot(), item->GetSlot());
            hisItems[i] = item;
            hisItems[i]->SetInTrade();
        }
    }
}

static void clearAcceptTradeMode(TradeData* myTrade, TradeData* hisTrade)
{
    myTrade->SetInAcceptProcess(false);
    hisTrade->SetInAcceptProcess(false);
}

static void clearAcceptTradeMode(Item* *myItems, Item* *hisItems)
{
    // clear 'in-trade' flag
    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (myItems[i])
            myItems[i]->SetInTrade(false);
        if (hisItems[i])
            hisItems[i]->SetInTrade(false);
    }
}

void WorldSession::HandleAcceptTrade(WorldPackets::Trade::AcceptTrade& acceptTrade)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    TradeData* myTrade = player->m_trade;
    if (!myTrade)
        return;

    Player* trader = myTrade->GetTrader();

    TradeData* hisTrade = trader->m_trade;
    if (!hisTrade)
        return;

    Item* myItems[TRADE_SLOT_TRADED_COUNT]  = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    Item* hisItems[TRADE_SLOT_TRADED_COUNT] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    // set before checks for propertly undo at problems (it already set in to client)
    myTrade->SetAccepted(true);

    WorldPackets::Trade::TradeStatus info;
    // if (hisTrade->GetServerStateIndex() != acceptTrade.StateIndex)
    // {
        // info.Status = TRADE_STATUS_STATE_CHANGED;
        // SendTradeStatus(info);
        // myTrade->SetAccepted(false);
        // return;
    // }

    if (!player->IsWithinDistInMap(trader, TRADE_DISTANCE, false))
    {
        info.Status = TRADE_STATUS_TOO_FAR_AWAY;
        SendTradeStatus(info);
        myTrade->SetAccepted(false);
        return;
    }

    // not accept case incorrect money amount
    if (!player->HasEnoughMoney(myTrade->GetMoney()))
    {
        info.Status = TRADE_STATUS_FAILED;
        info.BagResult = EQUIP_ERR_NOT_ENOUGH_MONEY;
        SendTradeStatus(info);
        myTrade->SetAccepted(false, true);
        return;
    }

    // not accept case incorrect money amount
    if (!trader->HasEnoughMoney(hisTrade->GetMoney()))
    {
        info.Status = TRADE_STATUS_FAILED;
        info.BagResult = EQUIP_ERR_NOT_ENOUGH_MONEY;
        trader->GetSession()->SendTradeStatus(info);
        hisTrade->SetAccepted(false, true);
        return;
    }

    if (player->GetMoney() >= uint64(MAX_MONEY_AMOUNT) - hisTrade->GetMoney())
    {
        info.Status = TRADE_STATUS_FAILED;
        info.BagResult = EQUIP_ERR_TOO_MUCH_GOLD;
        SendTradeStatus(info);
        myTrade->SetAccepted(false, true);
        return;
    }

    if (trader->GetMoney() >= uint64(MAX_MONEY_AMOUNT) - myTrade->GetMoney())
    {
        info.Status = TRADE_STATUS_FAILED;
        info.BagResult = EQUIP_ERR_TOO_MUCH_GOLD;
        trader->GetSession()->SendTradeStatus(info);
        hisTrade->SetAccepted(false, true);
        return;
    }

    // not accept if some items now can't be trade (cheating)
    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (Item* item = myTrade->GetItem(TradeSlots(i)))
        {
            if (!item->CanBeTraded(false, true))
            {
                info.Status = TRADE_STATUS_CANCELLED;
                SendTradeStatus(info);
                return;
            }

            if (item->IsBindedNotWith(trader))
            {
                info.Status = TRADE_STATUS_FAILED;
                info.BagResult = EQUIP_ERR_TRADE_BOUND_ITEM;
                SendTradeStatus(info);
                return;
            }
        }

        if (Item* item = hisTrade->GetItem(TradeSlots(i)))
        {
            if (!item->CanBeTraded(false, true))
            {
                info.Status = TRADE_STATUS_CANCELLED;
                SendTradeStatus(info);
                return;
            }
        }
    }

    if (hisTrade->IsAccepted())
    {
        setAcceptTradeMode(myTrade, hisTrade, myItems, hisItems);

        Spell* my_spell = nullptr;
        SpellCastTargets my_targets;
        my_targets.SetCaster(player);

        Spell* his_spell = nullptr;
        SpellCastTargets his_targets;
        his_targets.SetCaster(player);

        // not accept if spell can't be casted now (cheating)
        if (uint32 my_spell_id = myTrade->GetSpell())
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(my_spell_id);
            Item* castItem = myTrade->GetSpellCastItem();

            if (!spellEntry || !hisTrade->GetItem(TRADE_SLOT_NONTRADED) || (myTrade->HasSpellCastItem() && !castItem))
            {
                clearAcceptTradeMode(myTrade, hisTrade);
                clearAcceptTradeMode(myItems, hisItems);

                myTrade->SetSpell(0);
                return;
            }

            TriggerCastData triggerData;
            triggerData.triggerFlags = TRIGGERED_FULL_MASK;
            triggerData.castItem = castItem;

            my_spell = new Spell(player, spellEntry, triggerData);
            my_targets.SetTradeItemTarget(player);
            my_spell->m_targets = my_targets;

            SpellCastResult res = my_spell->CheckCast(true);
            if (res != SPELL_CAST_OK)
            {
                my_spell->SendCastResult(res);

                clearAcceptTradeMode(myTrade, hisTrade);
                clearAcceptTradeMode(myItems, hisItems);

                delete my_spell;
                myTrade->SetSpell(0);
                return;
            }
        }

        // not accept if spell can't be casted now (cheating)
        if (uint32 his_spell_id = hisTrade->GetSpell())
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(his_spell_id);
            Item* castItem = hisTrade->GetSpellCastItem();

            if (!spellEntry || !myTrade->GetItem(TRADE_SLOT_NONTRADED) || (hisTrade->HasSpellCastItem() && !castItem))
            {
                delete my_spell;
                hisTrade->SetSpell(0);

                clearAcceptTradeMode(myTrade, hisTrade);
                clearAcceptTradeMode(myItems, hisItems);
                return;
            }

            TriggerCastData triggerData;
            triggerData.triggerFlags = TRIGGERED_FULL_MASK;
            triggerData.castItem = castItem;

            his_spell = new Spell(trader, spellEntry, triggerData);
            his_targets.SetTradeItemTarget(trader);
            his_spell->m_targets = his_targets;

            SpellCastResult res = his_spell->CheckCast(true);
            if (res != SPELL_CAST_OK)
            {
                his_spell->SendCastResult(res);

                clearAcceptTradeMode(myTrade, hisTrade);
                clearAcceptTradeMode(myItems, hisItems);

                delete my_spell;
                delete his_spell;

                hisTrade->SetSpell(0);
                return;
            }
        }

        // inform partner client
        info.Status = TRADE_STATUS_ACCEPTED;
        trader->GetSession()->SendTradeStatus(info);

        WorldPackets::Trade::TradeStatus myCanCompleteInfo, hisCanCompleteInfo;
        uint32 ItemIDHis = 0;
        uint32 ItemIDMy = 0;
        hisCanCompleteInfo.BagResult = trader->CanStoreItems(myItems, TRADE_SLOT_TRADED_COUNT, ItemIDHis);
        myCanCompleteInfo.BagResult = _player->CanStoreItems(hisItems, TRADE_SLOT_TRADED_COUNT, ItemIDMy);
        hisCanCompleteInfo.ItemID = ItemIDHis;
        myCanCompleteInfo.ItemID = ItemIDMy;

        clearAcceptTradeMode(myItems, hisItems);

        // in case of missing space report error
        if (myCanCompleteInfo.BagResult != EQUIP_ERR_OK)
        {
            clearAcceptTradeMode(myTrade, hisTrade);

            myCanCompleteInfo.Status = TRADE_STATUS_FAILED;
            trader->GetSession()->SendTradeStatus(myCanCompleteInfo);
            myCanCompleteInfo.FailureForYou = true;
            SendTradeStatus(myCanCompleteInfo);
            myTrade->SetAccepted(false);
            hisTrade->SetAccepted(false);
            delete my_spell;
            delete his_spell;
            return;
        }
        if (hisCanCompleteInfo.BagResult != EQUIP_ERR_OK)
        {
            clearAcceptTradeMode(myTrade, hisTrade);

            hisCanCompleteInfo.Status = TRADE_STATUS_FAILED;
            SendTradeStatus(hisCanCompleteInfo);
            hisCanCompleteInfo.FailureForYou = true;
            trader->GetSession()->SendTradeStatus(hisCanCompleteInfo);
            myTrade->SetAccepted(false);
            hisTrade->SetAccepted(false);
            delete my_spell;
            delete his_spell;
            return;
        }

        // execute trade: 1. remove
        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
        {
            if (myItems[i])
            {
                myItems[i]->SetGuidValue(ITEM_FIELD_GIFT_CREATOR, player->GetGUID());
                player->MoveItemFromInventory(myItems[i], true);
            }
            if (hisItems[i])
            {
                hisItems[i]->SetGuidValue(ITEM_FIELD_GIFT_CREATOR, trader->GetGUID());
                trader->MoveItemFromInventory(hisItems[i], true);
            }
        }

        // execute trade: 2. store
        moveItems(myItems, hisItems);

        // logging money
        if (sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
        {
            if (!AccountMgr::IsPlayerAccount(player->GetSession()->GetSecurity()) && myTrade->GetMoney() > 0)
            {
                sLog->outCommand(player->GetSession()->GetAccountId(), "GM %s (Account: %u) give money (Amount: " UI64FMTD ") to player: %s (Account: %u)",
                    player->GetName(), player->GetSession()->GetAccountId(),
                    myTrade->GetMoney(),
                    trader->GetName(), trader->GetSession()->GetAccountId());
            }
            if (!AccountMgr::IsPlayerAccount(trader->GetSession()->GetSecurity()) && hisTrade->GetMoney() > 0)
            {
                sLog->outCommand(trader->GetSession()->GetAccountId(), "GM %s (Account: %u) give money (Amount: " UI64FMTD ") to player: %s (Account: %u)",
                    trader->GetName(), trader->GetSession()->GetAccountId(),
                    hisTrade->GetMoney(),
                    player->GetName(), player->GetSession()->GetAccountId());
            }
        }

        if(myTrade->GetMoney() >= sWorld->getIntConfig(CONFIG_LOG_GOLD_FROM))
            TC_LOG_DEBUG(LOG_FILTER_GOLD, "Trade: From player %s GUID %u Account: %u give money (Amount: " UI64FMTD ") to player: %s GUID %u Account: %u",
                player->GetName(), player->GetGUIDLow(), player->GetSession()->GetAccountId(), myTrade->GetMoney(), trader->GetName(), trader->GetGUIDLow(), trader->GetSession()->GetAccountId());

        if(hisTrade->GetMoney() >= sWorld->getIntConfig(CONFIG_LOG_GOLD_FROM))
            TC_LOG_DEBUG(LOG_FILTER_GOLD, "Trade: From player %s GUID %u Account: %u give money (Amount: " UI64FMTD ") to player: %s GUID %u Account: %u",
                trader->GetName(), trader->GetGUIDLow(), trader->GetSession()->GetAccountId(), hisTrade->GetMoney(), player->GetName(), player->GetGUIDLow(), player->GetSession()->GetAccountId());

        // update money
        player->ModifyMoney(-int64(myTrade->GetMoney()));
        player->ModifyMoney(hisTrade->GetMoney());
        trader->ModifyMoney(-int64(hisTrade->GetMoney()));
        trader->ModifyMoney(myTrade->GetMoney());

        if (my_spell)
            my_spell->prepare(&my_targets);

        if (his_spell)
            his_spell->prepare(&his_targets);

        // cleanup
        clearAcceptTradeMode(myTrade, hisTrade);
        delete player->m_trade;
        player->m_trade = nullptr;
        delete trader->m_trade;
        trader->m_trade = nullptr;

        // desynchronized with the other saves here (SaveInventoryAndGoldToDB() not have own transaction guards)
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        player->SaveInventoryAndGoldToDB(trans);
        trader->SaveInventoryAndGoldToDB(trans);
        CharacterDatabase.CommitTransaction(trans);

        info.Status = TRADE_STATUS_COMPLETE;
        trader->GetSession()->SendTradeStatus(info);
        SendTradeStatus(info);
    }
    else
    {
        info.Status = TRADE_STATUS_ACCEPTED;
        trader->GetSession()->SendTradeStatus(info);
    }
}

void WorldSession::HandleUnacceptTrade(WorldPackets::Trade::NullCmsg& /*packet*/)
{
    if (TradeData* myTrade = _player->GetTradeData())
        myTrade->SetAccepted(false, true);
}

void WorldSession::HandleBeginTrade(WorldPackets::Trade::NullCmsg& /*packet*/)
{
    TradeData* myTrade = _player->m_trade;
    if (!myTrade)
        return;

    WorldPackets::Trade::TradeStatus info;
    info.Status = TRADE_STATUS_INITIATED;
    myTrade->GetTrader()->GetSession()->SendTradeStatus(info);
    SendTradeStatus(info);
}

void WorldSession::SendCancelTrade()
{
    if (PlayerRecentlyLoggedOut() || PlayerLogout())
        return;

    WorldPackets::Trade::TradeStatus info;
    info.Status = TRADE_STATUS_CANCELLED;
    SendTradeStatus(info);
}

void WorldSession::HandleCancelTrade(WorldPackets::Trade::NullCmsg& /*packet*/)
{
    if (Player* player = GetPlayer())
        player->TradeCancel(true);
}

void WorldSession::HandleInitiateTrade(WorldPackets::Trade::InitiateTrade& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (player->m_trade)
        return;

    WorldPackets::Trade::TradeStatus info;

    if (!player->isAlive())
    {
        info.Status = TRADE_STATUS_DEAD;
        SendTradeStatus(info);
        return;
    }

    if (player->HasUnitState(UNIT_STATE_STUNNED))
    {
        info.Status = TRADE_STATUS_STUNNED;
        SendTradeStatus(info);
        return;
    }

    if (isLogingOut())
    {
        info.Status = TRADE_STATUS_LOGGING_OUT;
        SendTradeStatus(info);
        return;
    }

    if (player->isInFlight())
    {
        info.Status = TRADE_STATUS_TOO_FAR_AWAY;
        SendTradeStatus(info);
        return;
    }

    if (player->getLevel() < sWorld->getIntConfig(CONFIG_TRADE_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_TRADE_REQ), sWorld->getIntConfig(CONFIG_TRADE_LEVEL_REQ));
        return;
    }

    Player* pOther = ObjectAccessor::FindPlayer(packet.Guid);

    if (!pOther)
    {
        info.Status = TRADE_STATUS_NO_TARGET;
        SendTradeStatus(info);
        return;
    }

    if (pOther == player || pOther->m_trade)
    {
        info.Status = TRADE_STATUS_PLAYER_BUSY;
        SendTradeStatus(info);
        return;
    }

    if (!pOther->isAlive())
    {
        info.Status = TRADE_STATUS_TARGET_DEAD;
        SendTradeStatus(info);
        return;
    }

    if (pOther->isInFlight())
    {
        info.Status = TRADE_STATUS_TOO_FAR_AWAY;
        SendTradeStatus(info);
        return;
    }

    if (pOther->HasUnitState(UNIT_STATE_STUNNED))
    {
        info.Status = TRADE_STATUS_TARGET_STUNNED;
        SendTradeStatus(info);
        return;
    }

    if (pOther->GetSession()->isLogingOut())
    {
        info.Status = TRADE_STATUS_TARGET_LOGGING_OUT;
        SendTradeStatus(info);
        return;
    }

    if (pOther->GetSocial()->HasIgnore(player->GetGUID()))
    {
        info.Status = TRADE_STATUS_PLAYER_IGNORED;
        SendTradeStatus(info);
        return;
    }

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_TRADE) && pOther->GetTeam() != player->GetTeam() || pOther->HasFlag(PLAYER_FIELD_PLAYER_FLAGS_EX, PLAYER_FLAGS_EX_MERCENARY_MODE) || player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS_EX, PLAYER_FLAGS_EX_MERCENARY_MODE))
    {
        info.Status = TRADE_STATUS_WRONG_FACTION;
        SendTradeStatus(info);
        return;
    }

    if (!pOther->IsWithinDistInMap(player, TRADE_DISTANCE, false))
    {
        info.Status = TRADE_STATUS_TOO_FAR_AWAY;
        SendTradeStatus(info);
        return;
    }

    if (pOther->getLevel() < sWorld->getIntConfig(CONFIG_TRADE_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_TRADE_OTHER_REQ), sWorld->getIntConfig(CONFIG_TRADE_LEVEL_REQ));
        return;
    }

    player->m_trade = new TradeData(player, pOther);
    pOther->m_trade = new TradeData(pOther, player);

    info.Status = TRADE_STATUS_PROPOSED;
    info.Partner = player->GetGUID();
    pOther->GetSession()->SendTradeStatus(info);
}

void WorldSession::HandleSetTradeGold(WorldPackets::Trade::SetTradeGold& packet)
{
    TradeData* myTrade = _player->GetTradeData();
    if (!myTrade)
        return;

    myTrade->UpdateClientStateIndex();
    myTrade->SetMoney(packet.Coinage);
}

void WorldSession::HandleSetTradeCurrency(WorldPackets::Trade::SetTradeCurrency& packet)
{
    TradeData* myTrade = _player->GetTradeData();
    if (!myTrade)
        return;

    myTrade->UpdateClientStateIndex();
}

void WorldSession::HandleSetTradeItem(WorldPackets::Trade::SetTradeItem& packet)
{
    TradeData* myTrade = _player->GetTradeData();
    if (!myTrade)
        return;

    WorldPackets::Trade::TradeStatus info;

    if (packet.TradeSlot >= TRADE_SLOT_COUNT)
    {
        info.Status = TRADE_STATUS_CANCELLED;
        SendTradeStatus(info);
        return;
    }

    // check cheating, can't fail with correct client operations
    Item* item = _player->GetItemByPos(packet.PackSlot, packet.ItemSlotInPack);
    if (!item || (packet.TradeSlot != TRADE_SLOT_NONTRADED && !item->CanBeTraded(false, true)))
    {
        info.Status = TRADE_STATUS_CANCELLED;
        SendTradeStatus(info);
        return;
    }

    // prevent place single item into many trade slots using cheating and client bugs
    if (myTrade->HasItem(item->GetGUID()))
    {
        // cheating attempt
        info.Status = TRADE_STATUS_CANCELLED;
        SendTradeStatus(info);
        return;
    }

    myTrade->UpdateClientStateIndex();

    if (packet.TradeSlot != TRADE_SLOT_NONTRADED && item->IsBindedNotWith(myTrade->GetTrader()))
    {
        info.Status = TRADE_STATUS_NOT_ON_TAPLIST;
        info.TradeSlot = packet.TradeSlot;
        SendTradeStatus(info);
        return;
    }

    myTrade->SetItem(TradeSlots(packet.TradeSlot), item);
}

void WorldSession::HandleClearTradeItem(WorldPackets::Trade::ClearTradeItem& packet)
{
    TradeData* myTrade = _player->m_trade;
    if (!myTrade)
        return;

    myTrade->UpdateClientStateIndex();

    if (packet.TradeSlot >= TRADE_SLOT_COUNT)
        return;

    myTrade->SetItem(TradeSlots(packet.TradeSlot), nullptr);
}
