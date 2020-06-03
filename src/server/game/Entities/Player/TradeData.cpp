/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#include "TradeData.h"
#include "Player.h"
#include "Random.h"
#include "TradePackets.h"

TradeData::TradeData(Player* player, Player* trader): _player(player), _trader(trader), _money(0), _spell(0), _clientStateIndex(1), _serverStateIndex(1), _accepted(false), _acceptProccess(false)
{
    memset(_items, 0, TRADE_SLOT_COUNT * sizeof(ObjectGuid));
}

Player* TradeData::GetTrader() const
{
    return _trader;
}

TradeData* TradeData::GetTraderData() const
{
    return _trader->GetTradeData();
}

Item* TradeData::GetItem(TradeSlots slot) const
{
    return _items[slot] ? _player->GetItemByGuid(_items[slot]) : nullptr;
}

bool TradeData::HasItem(ObjectGuid itemGuid) const
{
    for (auto _item : _items)
        if (_item == itemGuid)
            return true;

    return false;
}

TradeSlots TradeData::GetTradeSlotForItem(ObjectGuid itemGuid) const
{
    for (uint8 i = 0; i < TRADE_SLOT_COUNT; ++i)
        if (_items[i] == itemGuid)
            return TradeSlots(i);

    return TRADE_SLOT_INVALID;
}

Item* TradeData::GetSpellCastItem() const
{
    return _spellCastItem ? _player->GetItemByGuid(_spellCastItem) : nullptr;
}

bool TradeData::HasSpellCastItem() const
{
    return _spellCastItem;
}

uint64 TradeData::GetMoney() const
{
    return _money;
}

void TradeData::SetItem(TradeSlots slot, Item* item)
{
    ObjectGuid itemGuid = item ? item->GetGUID() : ObjectGuid::Empty;

    if (!GetTraderData() || _items[slot] == itemGuid)
        return;

    _items[slot] = itemGuid;

    SetAccepted(false);
    GetTraderData()->SetAccepted(false);
    UpdateServerStateIndex();

    Update();

    if (slot == TRADE_SLOT_NONTRADED)
        GetTraderData()->SetSpell(0);

    SetSpell(0);
}

uint32 TradeData::GetSpell() const
{
    return _spell;
}

void TradeData::SetSpell(uint32 spell_id, Item* castItem /*= nullptr*/)
{
    ObjectGuid itemGuid = castItem ? castItem->GetGUID() : ObjectGuid::Empty;

    if (_spell == spell_id && _spellCastItem == itemGuid)
        return;

    _spell = spell_id;
    _spellCastItem = itemGuid;

    SetAccepted(false);
    GetTraderData()->SetAccepted(false);
    UpdateServerStateIndex();

    Update(true);
    Update(false);
}

void TradeData::SetMoney(uint64 money)
{
    if (_money == money)
        return;

    _money = money;

    if (!_player->HasEnoughMoney(money))
    {
        WorldPackets::Trade::TradeStatus info;
        info.Status = TRADE_STATUS_FAILED;
        info.BagResult = EQUIP_ERR_NOT_ENOUGH_MONEY;
        _player->GetSession()->SendTradeStatus(info);
        return;
    }

    SetAccepted(false);
    GetTraderData()->SetAccepted(false);
    UpdateServerStateIndex();

    Update(true);
}

bool TradeData::IsAccepted() const
{
    return _accepted;
}

void TradeData::Update(bool forTarget /*= true*/)
{
    if (forTarget)
        _trader->GetSession()->SendUpdateTrade(true);
    else
        _player->GetSession()->SendUpdateTrade(false);
}

void TradeData::SetAccepted(bool state, bool crosssend /*= false*/)
{
    _accepted = state;

    if (!state)
    {
        WorldPackets::Trade::TradeStatus info;
        info.Status = TRADE_STATUS_UNACCEPTED;
        if (crosssend)
            _trader->GetSession()->SendTradeStatus(info);
        else
            _player->GetSession()->SendTradeStatus(info);
    }
}

bool TradeData::IsInAcceptProcess() const
{
    return _acceptProccess;
}

void TradeData::SetInAcceptProcess(bool state)
{
    _acceptProccess = state;
}

uint32 TradeData::GetClientStateIndex() const
{
    return _clientStateIndex;
}

void TradeData::UpdateClientStateIndex()
{
    ++_clientStateIndex;
}

uint32 TradeData::GetServerStateIndex() const
{
    return _serverStateIndex;
}

void TradeData::UpdateServerStateIndex()
{
    _serverStateIndex = rand32();
}
