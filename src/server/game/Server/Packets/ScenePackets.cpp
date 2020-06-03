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

#include "ScenePackets.h"

void WorldPackets::Scene::SceneInstance::Read()
{
    _worldPacket >> SceneInstanceID;
}

WorldPacket const* WorldPackets::Scene::CancelScene::Write()
{
    _worldPacket << SceneInstanceID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Scene::PlayScene::Write()
{
    _worldPacket << SceneID;
    _worldPacket << PlaybackFlags;
    _worldPacket << SceneInstanceID;
    _worldPacket << SceneScriptPackageID;
    _worldPacket << TransportGUID;
    _worldPacket << Pos;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Scene::PetBattleRound::Write()
{
    _worldPacket << SceneObjectGUID;
    _worldPacket << MsgData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Scene::SceneObjectFinalRound::Write()
{
    _worldPacket << SceneObjectGUID;
    _worldPacket << MsgData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Scene::PetBattleFinished::Write()
{
    _worldPacket << SceneObjectGUID;

    return &_worldPacket;
}

void WorldPackets::Scene::SceneTriggerEvent::Read()
{
    uint32 len = _worldPacket.ReadBits(6);
    _worldPacket >> SceneInstanceID;
    Event = _worldPacket.ReadString(len);
}

WorldPacket const* WorldPackets::Scene::SceneObjectPetBattleInitialUpdate::Write()
{
    _worldPacket << SceneObjectGUID;
    _worldPacket << MsgData;

    return &_worldPacket;
}
