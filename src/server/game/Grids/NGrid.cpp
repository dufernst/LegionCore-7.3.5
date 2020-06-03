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


#include "NGrid.h"
#include "GridDefines.h"
#include "Timer.h"
#include "Util.h"
#include "GridInfo.h"

NGrid::NGrid(int32 x, int32 y, time_t expiry, bool unload): i_GridInfo(expiry, unload), i_x(x), i_y(y)
                                                            , i_cellstate(GRID_STATE_INVALID), i_GridObjectDataLoaded(false)
{
}

Grid& NGrid::GetGrid(const uint32 x, const uint32 y)
{
    return i_cells[x * MAX_NUMBER_OF_CELLS + y];
}

Grid const& NGrid::GetGrid(const uint32 x, const uint32 y) const
{
    return i_cells[x * MAX_NUMBER_OF_CELLS + y];
}

GridState NGrid::GetGridState() const
{
    return i_cellstate;
}

void NGrid::SetGridState(GridState s)
{
    i_cellstate = s;
}

int32 NGrid::getX() const
{
    return i_x;
}

int32 NGrid::getY() const
{
    return i_y;
}

bool NGrid::isGridObjectDataLoaded() const
{
    return i_GridObjectDataLoaded;
}

void NGrid::setGridObjectDataLoaded(bool pLoaded)
{
    i_GridObjectDataLoaded = pLoaded;
}

GridInfo& NGrid::getGridInfo()
{
    return i_GridInfo;
}

const TimeTracker& NGrid::getTimeTracker() const
{
    return i_GridInfo.getTimeTracker();
}

bool NGrid::getUnloadLock() const
{
    return i_GridInfo.getUnloadLock();
}

void NGrid::setUnloadReferenceLock(bool on)
{
    i_GridInfo.setUnloadReferenceLock(on);
}

void NGrid::incUnloadActiveLock()
{
    i_GridInfo.incUnloadActiveLock();
}

void NGrid::decUnloadActiveLock()
{
    i_GridInfo.decUnloadActiveLock();
}

void NGrid::ResetTimeTracker(time_t interval)
{
    i_GridInfo.ResetTimeTracker(interval);
}

void NGrid::UpdateTimeTracker(time_t diff)
{
    i_GridInfo.UpdateTimeTracker(diff);
}
