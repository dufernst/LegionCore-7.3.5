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
#include "WorldSession.h"

void WorldSession::HandleScenePlaybackCanceled(WorldPackets::Scene::SceneInstance& packet)
{
    SendPacket(WorldPackets::Scene::CancelScene(packet.SceneInstanceID).Write());

    if (_player)
        _player->SceneCompleted(packet.SceneInstanceID);
}

void WorldSession::HandleScenePlaybackComplete(WorldPackets::Scene::SceneInstance& packet)
{
    if(_player)
        _player->SceneCompleted(packet.SceneInstanceID);
}

void WorldSession::HandleSceneTriggerEvent(WorldPackets::Scene::SceneTriggerEvent& packet)
{
    if(_player)
        _player->TrigerScene(packet.SceneInstanceID, packet.Event);
}
