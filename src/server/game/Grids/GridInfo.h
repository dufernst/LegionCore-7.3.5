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

#ifndef TRINITY_GRIDInfo_H
#define TRINITY_GRIDInfo_H

#include "Timer.h"

class GridInfo final
{
public:
    GridInfo();
    explicit GridInfo(time_t expiry, bool unload = true);

    const TimeTracker& getTimeTracker() const;
    bool getUnloadLock() const;
    void setUnloadReferenceLock(bool on);
    void incUnloadActiveLock();
    void decUnloadActiveLock();
    void setTimer(const TimeTracker& pTimer);
    void ResetTimeTracker(time_t interval);

    void UpdateTimeTracker(time_t diff);

private:
    TimeTracker i_timer;

    uint16 i_unloadActiveLockCount;                     // lock from active object spawn points (prevent clone loading)
    uint8 i_unloadReferenceLock;                        // lock from instance map copy
    uint8 i_unloadExplicitLock;                         // explicit config setting
};

#endif
