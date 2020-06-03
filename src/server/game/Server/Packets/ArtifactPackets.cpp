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

#include "ArtifactPackets.h"

WorldPacket const* WorldPackets::Artifact::XpGain::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << Xp;

    return &_worldPacket;
}

void WorldPackets::Artifact::AddPower::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    Powers.resize(_worldPacket.read<uint32>());
    for (auto& v : Powers)
    {
        _worldPacket >> v.PowerID;
        _worldPacket >> v.Rank;
    }
}

void WorldPackets::Artifact::ConfirmRespec::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> NpcGUID;
}

void WorldPackets::Artifact::SetAppearance::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    _worldPacket >> AppearanceID;
}

WorldPacket const* WorldPackets::Artifact::ForgeOpenResult::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << GameObjectGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Artifact::ArtifactKnowledge::Write()
{
    _worldPacket << int32(ArtifactCategoryID);
    _worldPacket << int8(KnowledgeLevel);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Artifact::RespecResult::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << NpcGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Artifact::ArtifactTraitsRefunded::Write()
{
    _worldPacket << Guid;
    _worldPacket << UnkInt;
    _worldPacket << UnkInt2;

    return &_worldPacket;
}

void WorldPackets::Artifact::ArtifactAddRelicTalent::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    _worldPacket >> SlotIndex;
    _worldPacket >> TalentIndex;
}

void WorldPackets::Artifact::ArtifactAttuneSocketedRelic::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    _worldPacket >> RelicSlotIndex;
}

void WorldPackets::Artifact::ArtifactAttunePreviewdRelic::Read()
{
    _worldPacket >> RelicGUID;
    _worldPacket >> GameObjectGUID;
}

WorldPacket const* WorldPackets::Artifact::ArtifactAttuneSocketedRelicData::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << Result;

    return &_worldPacket;
}
