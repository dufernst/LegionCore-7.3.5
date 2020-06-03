/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#include "Warden.h"
#include "WardenTimer.h"

WardenTimer::WardenTimer(uint32 time, uint32 reqLevel, bool reqPlayerInWorld) : _initTime(time), _enabled(false), _reqPlayer(reqPlayerInWorld), _reqLevel(reqLevel) 
{
    _currentTime = _initTime;
}

bool WardenTimer::CheckConditions(Player* player)
{
    if (!_reqPlayer)
        return true;

    if (!player || !player->IsInWorld() || player->isBeingLoaded() || player->IsBeingTeleported())
        return false;

    if (_reqLevel && player->getLevel() < _reqLevel)
        return false;

    return true;
}