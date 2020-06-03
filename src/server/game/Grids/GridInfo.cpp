/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "GridInfo.h"

inline GridInfo::GridInfo() : i_timer(0), i_unloadActiveLockCount(0), i_unloadReferenceLock(0), i_unloadExplicitLock(0)
{
}

GridInfo::GridInfo(time_t expiry, bool unload) : i_timer(expiry), i_unloadActiveLockCount(0), i_unloadReferenceLock(0), i_unloadExplicitLock(unload ? 0 : 1)
{
}

const TimeTracker& GridInfo::getTimeTracker() const
{
    return i_timer;
}

bool GridInfo::getUnloadLock() const
{
    return i_unloadActiveLockCount || i_unloadReferenceLock || i_unloadExplicitLock;
}

void GridInfo::setUnloadReferenceLock(bool on)
{
    if (!this) // https://pastebin.com/7aVfGk6a
        return;

    i_unloadReferenceLock = on ? 1 : 0;
}

void GridInfo::incUnloadActiveLock()
{
    ++i_unloadActiveLockCount;
}

void GridInfo::decUnloadActiveLock()
{
    if (i_unloadActiveLockCount)
        --i_unloadActiveLockCount;
}

void GridInfo::setTimer(const TimeTracker& pTimer)
{
    i_timer = pTimer;
}

void GridInfo::ResetTimeTracker(time_t interval)
{
    i_timer.Reset(interval);
}

void GridInfo::UpdateTimeTracker(time_t diff)
{
    i_timer.Update(diff);
}
