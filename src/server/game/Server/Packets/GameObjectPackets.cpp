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

#include "GameObjectPackets.h"

void WorldPackets::GameObject::GameObjectUse::Read()
{
    _worldPacket >> Guid;
}

void WorldPackets::GameObject::GameObjReportUse::Read()
{
    _worldPacket >> Guid;
}

WorldPacket const* WorldPackets::GameObject::GameObjectDespawn::Write()
{
    _worldPacket << ObjectGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::GameObjectUiAction::Write()
{
    _worldPacket << ObjectGUID;
    _worldPacket << ActionID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::PageText::Write()
{
    _worldPacket << GameObjectGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::GameObjectActivateAnimKit::Write()
{
    _worldPacket << ObjectGUID;
    _worldPacket << uint32(AnimKitID);
    _worldPacket.WriteBit(Maintain);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::GoCustomAnim::Write()
{
    _worldPacket << ObjectGUID;
    _worldPacket << CustomAnim;
    _worldPacket.WriteBit(PlayAsDespawn);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::GameObjectResetState::Write()
{
    _worldPacket << ObjectGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::GameObjectPlaySpellVisual::Write()
{
    _worldPacket << ObjectGUID;
    _worldPacket << ActivatorGUID;
    _worldPacket << SpellVisualID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::DestructibleBuildingDamage::Write()
{
    _worldPacket << Target;
    _worldPacket << Caster;
    _worldPacket << Owner;
    _worldPacket << Damage;
    _worldPacket << SpellID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::PlayObjectSound::Write()
{
    _worldPacket << SoundId;
    _worldPacket << SourceObjectGUID;
    _worldPacket << TargetObjectGUID;
    _worldPacket << Pos;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::GameObject::GameObjectSetState::Write()
{
    _worldPacket << ObjectGUID;
    _worldPacket << StateID;

    return &_worldPacket;
}
