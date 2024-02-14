/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#include "World.h"

 // This is where scripts' loading functions should be declared:
 // world

//garrison
void AddSC_garrison_general();
void AddSC_garrison_instance();

void AddSC_areatrigger_scripts();
void AddSC_emerald_dragons();
void AddSC_generic_creature();
void AddSC_go_scripts();
void AddSC_guards();
void AddSC_item_scripts();
void AddSC_npc_professions();
void AddSC_npc_innkeeper();
void AddSC_npcs_special();
void AddSC_npc_taxi();
void AddSC_achievement_scripts();
void AddSC_challenge_scripts();
void AddSC_player_special_scripts();
void AddSC_fireworks_spectacular();
void AddSC_custom_events();
void AddSC_scene_scripts();

void AddSC_petbattle_abilities();
void AddSC_PetBattlePlayerScript();
void AddSC_npc_PetBattleTrainer();

// player
void AddSC_chat_log();

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddWorldScripts()
{
    AddSC_garrison_general();
    AddSC_garrison_instance();

    AddSC_areatrigger_scripts();
    AddSC_emerald_dragons();
    AddSC_generic_creature();
    AddSC_go_scripts();
    AddSC_guards();
    AddSC_item_scripts();
    AddSC_npc_professions();
    AddSC_npc_innkeeper();
    AddSC_npcs_special();
    AddSC_npc_taxi();
    AddSC_achievement_scripts();
    AddSC_challenge_scripts();
    AddSC_player_special_scripts();
    AddSC_fireworks_spectacular();
    AddSC_custom_events();
    AddSC_scene_scripts();

    AddSC_petbattle_abilities();
    AddSC_PetBattlePlayerScript();
    AddSC_npc_PetBattleTrainer();

    AddSC_chat_log(); // location: scripts\World\chat_log.cpp
}