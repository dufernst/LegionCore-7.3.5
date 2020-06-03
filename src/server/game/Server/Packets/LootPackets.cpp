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

#include "LootPackets.h"

void WorldPackets::Loot::LootUnit::Read()
{
    _worldPacket >> Unit;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Loot::LootItem const& item)
{
    data.WriteBits(item.Type, 2);
    data.WriteBits(item.UIType, 3);
    data.WriteBit(item.CanTradeToTapList);
    data.FlushBits();

    data << item.Loot;
    data << item.Quantity;
    data << item.LootItemType;
    data << item.LootListID;

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Loot::LootRequest& loot)
{
    data >> loot.Object;
    data >> loot.LootListID;

    return data;
}

WorldPacket const* WorldPackets::Loot::LootResponse::Write()
{
    _worldPacket << Owner;
    _worldPacket << LootObj;
    _worldPacket << FailureReason;
    _worldPacket << AcquireReason;
    _worldPacket << LootMethod;
    _worldPacket << Threshold;

    _worldPacket << Coins;

    _worldPacket << static_cast<uint32>(Items.size());
    _worldPacket << static_cast<uint32>(Currencies.size());

    _worldPacket.WriteBit(Acquired);
    _worldPacket.WriteBit(AELooting);
    _worldPacket.FlushBits();

    for (LootItem const& item : Items)
        _worldPacket << item;

    for (LootCurrency const& currency : Currencies)
    {
        _worldPacket << currency.CurrencyID;
        _worldPacket << currency.Quantity;
        _worldPacket << currency.LootListID;
        _worldPacket.WriteBits(currency.UIType, 3);
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}

void WorldPackets::Loot::AutoStoreLootItem::Read()
{
    Loot.resize(_worldPacket.read<uint32>());
    for (auto& loot : Loot)
        _worldPacket >> loot;
}

WorldPacket const* WorldPackets::Loot::LootRemoved::Write()
{
    _worldPacket << Owner;
    _worldPacket << LootObj;
    _worldPacket << LootListID;

    return &_worldPacket;
}

void WorldPackets::Loot::LootRelease::Read()
{
    _worldPacket >> Unit;
}

WorldPacket const* WorldPackets::Loot::LootMoneyNotify::Write()
{
    _worldPacket << Money;
    _worldPacket.WriteBit(SoleLooter);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::CoinRemoved::Write()
{
    _worldPacket << LootObj;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::StartLootRoll::Write()
{
    _worldPacket << LootObj;
    _worldPacket << MapID;
    _worldPacket << RollTime;
    _worldPacket << Method;
    _worldPacket << ValidRolls;
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootRollsComplete::Write()
{
    _worldPacket << LootObj;
    _worldPacket << LootListID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::AELootTargets::Write()
{
    _worldPacket << Count;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootRollResponse::Write()
{
    _worldPacket << LootObj;
    _worldPacket << Player;
    _worldPacket << LootItems;
    _worldPacket << Roll;
    _worldPacket << RollType;
    _worldPacket.WriteBit(Autopassed);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootRollWon::Write()
{
    _worldPacket << LootObj;
    _worldPacket << Player;
    _worldPacket << Roll;
    _worldPacket << RollType;
    _worldPacket << LootItems;

    _worldPacket << Autopassed;
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootAllPassed::Write()
{
    _worldPacket << LootObj;
    _worldPacket << LootItems;

    return &_worldPacket;
}

void WorldPackets::Loot::LootRoll::Read()
{
    _worldPacket >> LootObj;
    _worldPacket >> LootListID;
    _worldPacket >> RollType;
}

WorldPacket const* WorldPackets::Loot::LootReleaseResponse::Write()
{
    _worldPacket << LootObj;
    _worldPacket << Owner;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootList::Write()
{
    _worldPacket << Owner;
    _worldPacket << LootObj;
    _worldPacket.WriteBit(Master.is_initialized());
    _worldPacket.WriteBit(RoundRobinWinner.is_initialized());
    _worldPacket.FlushBits();

    if (Master)
        _worldPacket << *Master;

    if (RoundRobinWinner)
        _worldPacket << *RoundRobinWinner;

    return &_worldPacket;
}

void WorldPackets::Loot::SetLootSpecialization::Read()
{
    _worldPacket >> SpecID;
}

WorldPacket const* WorldPackets::Loot::MasterLootCandidateList::Write()
{
    _worldPacket << LootObj;
    _worldPacket << static_cast<uint32>(Players.size());
    for (ObjectGuid const& i : Players)
        _worldPacket << i;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootDisplayToast::Write()
{
    _worldPacket << Quantity;
    _worldPacket << DisplayToastMethod;

    _worldPacket << QuestID;
    _worldPacket.FlushBits();

    _worldPacket.WriteBit(BonusRoll);

    _worldPacket.WriteBits(Type, 2);

    if (Type == uint32(ToastType::ITEM))
    {
        _worldPacket.WriteBit(Mailed);
        _worldPacket << Loot;
        _worldPacket << SpecID;
        _worldPacket << ItemQuantity;
    }

    if (Type == uint32(ToastType::CURRENCY))
        _worldPacket << CurrencyID;

    return &_worldPacket;
}

void WorldPackets::Loot::DoMasterLootRoll::Read()
{
    _worldPacket >> LootObj;
    _worldPacket >> LootListID;
}

void WorldPackets::Loot::CancelMasterLootRoll::Read()
{
    _worldPacket >> LootObj;
    _worldPacket >> LootListID;
}

void WorldPackets::Loot::MasterLootItem::Read()
{
    Loot.resize(_worldPacket.read<uint32>());
    _worldPacket >> Target;
    for (auto& loot : Loot)
        _worldPacket >> loot;
}
