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

#include "TalentPackets.h"

WorldPacket const* WorldPackets::Talent::UpdateTalentData::Write()
{
    _worldPacket << Info.ActiveGroup;
    _worldPacket << Info.ActiveSpecID;
    _worldPacket << static_cast<uint32>(Info.TalentGroups.size());

    for (auto const& talentGroupInfo : Info.TalentGroups)
    {
        _worldPacket << talentGroupInfo.SpecID;

        _worldPacket << static_cast<uint32>(talentGroupInfo.TalentIDs.size());
        _worldPacket << static_cast<uint32>(talentGroupInfo.PvPTalentIDs.size());

        for (uint16 talentID : talentGroupInfo.TalentIDs)
            _worldPacket << talentID;

        for (uint16 talentID : talentGroupInfo.PvPTalentIDs)
            _worldPacket << talentID;
    }

    return &_worldPacket;
}

void WorldPackets::Talent::LearnTalent::Read()
{
    Talents.resize(_worldPacket.ReadBits(6));
    for (uint16& talent : Talents)
        _worldPacket >> talent;
}

void WorldPackets::Talent::LearnPvpTalents::Read()
{
    TalentIDs.resize(_worldPacket.ReadBits(6));
    for (uint16& talent : TalentIDs)
        _worldPacket >> talent;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Talent::LearnTalentFailedData const& failed)
{
    data.WriteBits(static_cast<uint32>(failed.Reason), 4);
    data.FlushBits();
    data << failed.SpecID;
    data << static_cast<int32>(failed.TalentIDs.size());
    for (uint16 const& id : failed.TalentIDs)
        data << id;

    return data;
}

WorldPacket const* WorldPackets::Talent::LearnTalentFailed::Write()
{
    _worldPacket << Talent;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Talent::LearnPvPTalentFailed::Write()
{
    _worldPacket << PvPTalent;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Talent::GlyphBinding const& glyphBinding)
{
    data << uint32(glyphBinding.SpellID);
    data << uint16(glyphBinding.GlyphID);
    return data;
}

WorldPacket const* WorldPackets::Talent::ActiveGlyphs::Write()
{
    _worldPacket << static_cast<uint32>(Glyphs.size());
    for (GlyphBinding const& glyph : Glyphs)
        _worldPacket << glyph;

    _worldPacket.WriteBit(IsFullUpdate);
    _worldPacket.FlushBits();

    return &_worldPacket;
}
