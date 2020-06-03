/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#ifndef _LFGPLAYERDATA_H
#define _LFGPLAYERDATA_H

#include "LFG.h"
#include "Packets/LFGPacketsCommon.h"

namespace lfg
{
class LfgPlayerData
{
public:
    LfgPlayerData();
    ~LfgPlayerData();

    void SetTicket(WorldPackets::LFG::RideTicket const& ticket);
    WorldPackets::LFG::RideTicket const& GetTicket() const;
    void SetState(LfgState state);
    void RestoreState();
    void SetTeam(uint8 team);
    void SetGroup(ObjectGuid group);
    void SetLfgGroup(ObjectGuid group);
    void ClearState();
    void SetRoles(uint8 roles);
    void SetSelectedDungeons(const LfgDungeonSet& dungeons);
    LfgState GetState() const;
    LfgState GetOldState() const;
    uint8 GetTeam() const;
    ObjectGuid GetGroup() const;
    ObjectGuid GetLfgGroup() const;
    uint8 GetRoles() const;
    LfgDungeonSet const& GetSelectedDungeons() const;

private:
    WorldPackets::LFG::RideTicket m_Ticket;            ///< Join ticket
    LfgDungeonSet m_SelectedDungeons;                  ///< Selected Dungeons when joined LFG
    ObjectGuid m_Group;                                ///< Original group of player when joined LFG
    ObjectGuid m_LfgGroup;
    LfgState m_State;                                  ///< State if group in LFG
    LfgState m_OldState;                               ///< Old State - Used to restore state after failed Rolecheck/Proposal
    uint8 m_Team;                                      ///< Player team - determines the queue to join
    uint8 m_Roles;                                     ///< Roles the player selected when joined LFG
    std::recursive_mutex m_lock;
};

} // namespace lfg

#endif
