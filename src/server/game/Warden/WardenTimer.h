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

#ifndef _WARDEN_TIMERS_H
#define _WARDEN_TIMERS_H

class WardenTimer
{
    public:
        WardenTimer(uint32 time, uint32 reqLevel, bool reqPlayerInWorld);
        ~WardenTimer() {}

        void SetCurrentTime(uint32 time) { _currentTime = time; }
        uint32 GetCurrentTime() { return _currentTime; }
        uint32 GetInitTime() { return _initTime; }
        void Reset() { _currentTime = _initTime; }
        void Start() { _enabled = true; }
        void Stop() { _enabled = false; }
        bool Active() { return _enabled; }
        bool Expired(uint32 diff) { return _currentTime <= diff; }
        void Continue(uint32 diff) { _currentTime -= diff; }
        bool CheckConditions(Player* player);

    private:
        uint32 _initTime;
        uint32 _currentTime;
        bool _enabled;
        bool _reqPlayer;
        uint32 _reqLevel;
};

#endif
