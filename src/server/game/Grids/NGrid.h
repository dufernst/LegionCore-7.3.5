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

#ifndef TRINITY_NGRID_H
#define TRINITY_NGRID_H

/** NGrid is nothing more than a wrapper of the Grid with an NxN cells
 */

#include "Grid.h"
#include "GridDefines.h"
#include "GridInfo.h"

#define DEFAULT_VISIBILITY_NOTIFY_PERIOD 1000

enum GridState
{
    GRID_STATE_INVALID,
    GRID_STATE_ACTIVE,
    GRID_STATE_IDLE,
    GRID_STATE_REMOVAL,
    MAX_GRID_STATE
};

class NGrid final
{
public:
    NGrid(int32 x, int32 y, time_t expiry, bool unload = true);
    Grid& GetGrid(const uint32 x, const uint32 y);
    Grid const& GetGrid(const uint32 x, const uint32 y) const;

    GridState GetGridState() const;
    void SetGridState(GridState s);

    int32 getX() const;
    int32 getY() const;

    bool isGridObjectDataLoaded() const;
    void setGridObjectDataLoaded(bool pLoaded);

    GridInfo& getGridInfo();

    const TimeTracker& getTimeTracker() const;

    bool getUnloadLock() const;
    void setUnloadReferenceLock(bool on);
    void incUnloadActiveLock();
    void decUnloadActiveLock();

    void ResetTimeTracker(time_t interval);
    void UpdateTimeTracker(time_t diff);

    // Visit all Grids (cells) in NGrid (grid)
    template <typename Visitor>
    void VisitAllGrids(Visitor &&visitor)
    {
        for (auto &cell : i_cells)
            cell.Visit(visitor);
    }

    // Visit a single Grid (cell) in NGrid (grid)
    template <typename Visitor>
    void VisitGrid(const uint32 x, const uint32 y, Visitor &&visitor)
    {
        GetGrid(x, y).Visit(visitor);
    }

    template <typename T>
    std::size_t GetWorldObjectCountInNGrid() const
    {
        std::size_t count = 0;
        for (auto &cell : i_cells)
            count += cell.GetWorldObjectCountInGrid<T>();
        return count;
    }

private:
    GridInfo i_GridInfo;
    int32 i_x;
    int32 i_y;
    GridState i_cellstate;
    Grid i_cells[MAX_NUMBER_OF_CELLS * MAX_NUMBER_OF_CELLS];
    bool i_GridObjectDataLoaded;
};

#endif
