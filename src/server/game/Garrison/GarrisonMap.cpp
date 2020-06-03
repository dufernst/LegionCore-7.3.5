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

#include "GarrisonMap.h"
#include "Garrison.h"
#include "ObjectAccessor.h"
#include "ObjectGridLoader.h"
#include "GameObject.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"

GarrisonMap::GarrisonMap(uint32 id, time_t expiry, uint32 instanceId, Map* parent, ObjectGuid const& owner) : InstanceMap(id, expiry, instanceId, DIFFICULTY_NORMAL, parent), _owner(owner)
{
    GarrisonMap::InitVisibilityDistance();
}

Garrison* GarrisonMap::GetGarrison()
{
    if (_garrison)
        return _garrison;

    if (Player* owner = ObjectAccessor::FindPlayer(_owner))
        _garrison = owner->GetGarrisonPtr();

    return _garrison;
}

void GarrisonMap::InitVisibilityDistance()
{
    //init visibility distance for instances
    m_VisibleDistance = sWorld->GetMaxVisibleDistanceInBG();
    m_VisibilityNotifyPeriod = World::GetVisibilityNotifyPeriodInBGArenas();
}

bool GarrisonMap::AddPlayerToMap(Player* player, bool initPlayer /*= true*/)
{
    if (player->GetGUID() == _owner)
        _garrison = player->GetGarrisonPtr();

    return InstanceMap::AddPlayerToMap(player, initPlayer);
}

bool GarrisonMap::onEnsureGridLoaded(NGrid* grid, Cell const& _cell)
{
    if (!_garrison)
        return false;

    if (!_garrison->_plots.empty())
    {
        Cell i_cell = _cell;
        i_cell.data.Part.cell_y = 0;
        for (uint32 x = 0; x < MAX_NUMBER_OF_CELLS; ++x)
        {
            i_cell.data.Part.cell_x = x;
            for (uint32 y = 0; y < MAX_NUMBER_OF_CELLS; ++y)
            {
                i_cell.data.Part.cell_y = y;

                CellCoord cellCoord = i_cell.GetCellCoord();
                for (auto& plot : _garrison->_plots)
                {
                    Position const& spawn = plot.second.PacketInfo.PlotPos;
                    CellCoord const spawnCoord = Trinity::ComputeCellCoord(spawn.GetPositionX(), spawn.GetPositionY());
                    if (cellCoord != spawnCoord)
                        continue;

                    if (GameObject* go = plot.second.CreateGameObject(this, _garrison->GetFaction(), _garrison))
                        AddToMap(go);
                }
            }
        }
    }
    return true;
}