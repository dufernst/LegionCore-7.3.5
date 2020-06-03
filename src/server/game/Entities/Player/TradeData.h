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

#ifndef TradeData_h__
#define TradeData_h__

#include "ObjectGuid.h"

enum TradeSlots
{
    TRADE_SLOT_COUNT            = 7,
    TRADE_SLOT_TRADED_COUNT     = 6,
    TRADE_SLOT_NONTRADED        = 6,
    TRADE_SLOT_INVALID          = -1,
};

class Item;
class Player;

class TradeData
{
public:
    TradeData(Player* player, Player* trader);

    Player* GetTrader() const;
    TradeData* GetTraderData() const;

    Item* GetItem(TradeSlots slot) const;
    bool HasItem(ObjectGuid itemGuid) const;
    TradeSlots GetTradeSlotForItem(ObjectGuid itemGuid) const;
    void SetItem(TradeSlots slot, Item* item);

    uint32 GetSpell() const;
    void SetSpell(uint32 spell_id, Item* castItem = nullptr);

    Item*  GetSpellCastItem() const;
    bool HasSpellCastItem() const;

    uint64 GetMoney() const;
    void SetMoney(uint64 money);

    bool IsAccepted() const;
    void SetAccepted(bool state, bool crosssend = false);

    bool IsInAcceptProcess() const;
    void SetInAcceptProcess(bool state);

    uint32 GetClientStateIndex() const;
    void UpdateClientStateIndex();

    uint32 GetServerStateIndex() const;
    void UpdateServerStateIndex();
    void Update(bool for_trader = true);

private:
    Player* _player;
    Player* _trader;
    ObjectGuid _spellCastItem;
    ObjectGuid _items[TRADE_SLOT_COUNT];
    uint64 _money;
    uint32 _spell;
    uint32 _clientStateIndex;
    uint32 _serverStateIndex;
    bool _accepted;
    bool _acceptProccess;
};

#endif // TradeData_h__
