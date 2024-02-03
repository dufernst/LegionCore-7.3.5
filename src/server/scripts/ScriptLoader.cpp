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

#include "ScriptLoader.h"

void AddSpellsScripts();
void AddCommandsScripts();

// Battlegrounds
void AddSC_battleground_seething_shore();
void AddSC_battleground_warsong();
void AddSC_battleground_kotmogu();
void AddSC_battleground_shado_pan();

#ifdef SCRIPTS
void AddWorldScripts();
void AddBattlePayScripts();
void AddDraenorScripts();
void AddEasternKingdomsScripts();
void AddKalimdorScripts();
void AddLegionScripts();
void AddMaelstromScripts();
void AddNorthrendScripts();
void AddOutlandScripts();
void AddPandariaScripts();
void AddOutdoorPvPScripts();
void AddScenarioScripts();
void AddCustomScripts();
#endif

void AddScripts()
{
    AddSpellsScripts();
    AddCommandsScripts();

    // Battleground scripts
    AddSC_battleground_seething_shore();
    AddSC_battleground_warsong();
    AddSC_battleground_kotmogu();
    AddSC_battleground_shado_pan();
    
#ifdef SCRIPTS
    AddWorldScripts();
    AddBattlePayScripts();
    AddDraenorScripts();
    AddEasternKingdomsScripts();
    AddKalimdorScripts();
    AddLegionScripts();
    AddMaelstromScripts();
    AddNorthrendScripts();
    AddOutlandScripts();
    AddPandariaScripts();
    AddOutdoorPvPScripts();
    AddScenarioScripts();
    AddCustomScripts();
#endif
}