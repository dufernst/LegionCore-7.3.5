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

#ifndef TRINITY_OBJECTGRIDLOADER_H
#define TRINITY_OBJECTGRIDLOADER_H

#include "Define.h"
#include "GridDefines.h"
#include "Cell.h"

namespace Trinity {

struct ObjectGridLoader final
{
    static void LoadN(NGrid const &grid, Map *map, Cell cell);
};

//Stop the creatures before unloading the NGrid
struct ObjectGridStoper final
{
    void Visit(CreatureMapType &m);

    template <typename NotInterested>
    void Visit(NotInterested &) { }
    std::mutex _lock;
};

// Move the foreign creatures back to respawn positions before unloading the NGrid
struct ObjectGridEvacuator final
{
    void Visit(CreatureMapType &m);
    void Visit(GameObjectMapType &m);

    template <typename NotInterested>
    void Visit(NotInterested &) { }
};

// Clean up and remove from world
struct ObjectGridCleaner final
{
    template <typename AnyMapType>
    void Visit(AnyMapType &m);
};

// Delete objects before deleting NGrid
struct ObjectGridUnloader final
{
    template <typename AnyMapType>
    void Visit(AnyMapType &m);
    std::mutex _lock;
};

} // namespace Trinity

#endif
