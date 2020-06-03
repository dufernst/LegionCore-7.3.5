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

#include "InspectPackets.h"
#include "Item.h"

void WorldPackets::Inspect::Inspect::Read()
{
    _worldPacket >> Target;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Inspect::InspectEnchantData const& enchantData)
{
    data << uint32(enchantData.Id);
    data << uint8(enchantData.Index);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Inspect::InspectItemData const& itemData)
{
    data << itemData.CreatorGUID;
    data << itemData.Index;
    data << itemData.Item;
    data.WriteBit(itemData.Usable);
    data.WriteBits(itemData.Enchants.size(), 4);
    data.WriteBits(itemData.Gems.size(), 2);
    data.FlushBits();

    for (auto const& gem : itemData.Gems)
        data << gem;

    for (auto const& enchant : itemData.Enchants)
        data << enchant;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Inspect::InspectGuildData const& guildData)
{
    data << guildData.GuildGUID;
    data << int32(guildData.NumGuildMembers);
    data << int32(guildData.AchievementPoints);

    return data;
}

WorldPackets::Inspect::InspectItemData::InspectItemData(::Item const* item, uint8 index)
{
    CreatorGUID = item->GetGuidValue(ITEM_FIELD_CREATOR);

    Item.Initialize(item);
    Index = index;
    Usable = true; /// @todo

    for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
        if (uint32 enchId = item->GetEnchantmentId(EnchantmentSlot(i)))
            Enchants.emplace_back(enchId, i);

    uint8 i = 0;
    for (ItemDynamicFieldGems const& gemData : item->GetGems())
    {
        if (gemData.ItemId)
        {
            Item::ItemGemData gem;
            gem.Slot = i;
            gem.Item.Initialize(&gemData);
            Gems.push_back(gem);
        }
        ++i;
    }
}

WorldPacket const* WorldPackets::Inspect::InspectResult::Write()
{
    _worldPacket << InspecteeGUID;

    _worldPacket << static_cast<uint32>(Items.size());
    _worldPacket << static_cast<uint32>(Glyphs.size());
    _worldPacket << static_cast<uint32>(Talents.size());
    _worldPacket << static_cast<uint32>(PvPTalents.size());

    _worldPacket << int32(ClassID);
    _worldPacket << int32(SpecializationID);
    _worldPacket << int32(GenderID);

    for (uint16 const& glyph : Glyphs)
        _worldPacket << glyph;

    for (uint16 const& talent : Talents)
        _worldPacket << talent;

    for (uint16 const& talent : PvPTalents)
        _worldPacket << talent;

    _worldPacket.FlushBits();
    _worldPacket.WriteBit(GuildData.is_initialized());

    for (auto const& item : Items)
        _worldPacket << item;

    if (GuildData)
        _worldPacket << *GuildData;

    return &_worldPacket;
}

void WorldPackets::Inspect::RequestHonorStats::Read()
{
    _worldPacket >> TargetGUID;
}

WorldPacket const* WorldPackets::Inspect::InspectHonorStats::Write()
{
    _worldPacket << PlayerGUID;
    _worldPacket << uint8(LifetimeMaxRank);
    _worldPacket << uint16(YesterdayHK);
    _worldPacket << uint16(TodayHK);
    _worldPacket << uint32(LifetimeHK);

    return &_worldPacket;
}

void WorldPackets::Inspect::InspectPVPRequest::Read()
{
    _worldPacket >> InspectTarget;
    _worldPacket >> InspectRealmAddress;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Inspect::PVPBracketData const& bracket)
{
    data << int32(bracket.Rating);
    data << int32(bracket.Rank);
    data << int32(bracket.WeeklyPlayed);
    data << int32(bracket.WeeklyWon);
    data << int32(bracket.SeasonPlayed);
    data << int32(bracket.SeasonWon);
    data << int32(bracket.WeeklyBestRating);
    data << int32(bracket.Unk710);
    data << uint8(bracket.Bracket);

    return data;
}

WorldPacket const* WorldPackets::Inspect::InspectPVPResponse::Write()
{
    _worldPacket << ClientGUID;

    _worldPacket.WriteBits(Bracket.size(), 3);
    _worldPacket.FlushBits();

    for (size_t i = 0; i < Bracket.size(); ++i)
        _worldPacket << Bracket[i];

    return &_worldPacket;
}

void WorldPackets::Inspect::QueryInspectAchievements::Read()
{
    _worldPacket >> Guid;
}
