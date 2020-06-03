/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#include "MapObject.h"

MapObject::MapObject()
{
    _moveState = MAP_OBJECT_CELL_MOVE_NONE;
    _newPosition.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
}

Cell const& MapObject::GetCurrentCell() const
{
    return _currentCell;
}

void MapObject::SetCurrentCell(Cell const& cell)
{
    _currentCell = cell;
}

void MapObject::SetNewCellPosition(float x, float y, float z, float o)
{
    _moveState = MAP_OBJECT_CELL_MOVE_ACTIVE;
    _newPosition.Relocate(x, y, z, o);
}
