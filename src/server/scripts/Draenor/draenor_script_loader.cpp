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

 // This is where scripts' loading functions should be declared:
void AddSC_wod_dark_portal();
void AddSC_wod_frostfire_ridge();
void AddSC_wod_shadowmoon_valley();

void AddSC_instance_iron_docks();       // Iron Docks
void AddSC_boss_fleshrender_nokgar();
void AddSC_boss_grimrail_enforcers();
void AddSC_boss_oshir();
void AddSC_boss_skulloc();

void AddSC_instance_upper_blackrock_spire();   // Upper Blackrock Spire
void AddSC_boss_orebender_gorashan();
void AddSC_boss_kyrak();
void AddSC_boss_commander_tharbek();
void AddSC_boss_ragewing_untamed();
void AddSC_boss_warlord_zaela();
void AddSC_upper_blackrock_spire();

void AddSC_instance_shadowmoon_burial_grounds();   // Shadowmoon Burial Grounds
void AddSC_boss_sadana_bloodfury();
void AddSC_boss_nhallish();
void AddSC_boss_bonemaw();
void AddSC_boss_nerzhul();

void AddSC_instance_bloodmaul_slag_mines(); // Bloodmaul Slag Mines
void AddSC_boss_slave_watcher_crushto();
void AddSC_boss_forgemaster_gogduh();
void AddSC_boss_roltall();
void AddSC_boss_gugrokk();
void AddSC_bloodmaul_slag_mines();

void AddSC_instance_skyreach(); // Skyreach
void AddSC_boss_ranjit();
void AddSC_boss_araknath();
void AddSC_boss_rukhran();
void AddSC_boss_high_sage_viryx();

void AddSC_instance_the_everbloom(); // The Everbloom
void AddSC_boss_witherbark();
void AddSC_boss_ancient_protectors();
void AddSC_boss_xeritac();
void AddSC_boss_archmage_sol();
void AddSC_boss_yalnu();

void AddSC_instance_auchindoun();
void AddSC_auchindoun();
void AddSC_boss_azzakel();
void AddSC_boss_teronogor();
void AddSC_boss_nyami();
void AddSC_boss_kaathar();

void AddSC_instance_highmaul(); // Raid: Highmaul
void AddSC_boss_twin_ogron();
void AddSC_boss_brackenspore();
void AddSC_boss_imperator_margok();
void AddSC_boss_kargath_bladefist();
void AddSC_boss_koragh();
void AddSC_boss_tectus();
void AddSC_boss_the_butcher();
void AddSC_highmaul();

void AddSC_instance_blackrock_foundry(); // Raid: Blackrock Foundry

void AddSC_instance_hellfire_citadel();  // Raid: Hellfire Citadel

void AddSC_world_bossess_draenor();
void AddSC_edge_of_reality();

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddDraenorScripts()
{
    AddSC_wod_dark_portal();
    AddSC_wod_frostfire_ridge();
    AddSC_wod_shadowmoon_valley();

    AddSC_instance_iron_docks();       // Iron Docks
    AddSC_boss_fleshrender_nokgar();
    AddSC_boss_grimrail_enforcers();
    AddSC_boss_oshir();
    AddSC_boss_skulloc();

    AddSC_instance_upper_blackrock_spire();   // Upper Blackrock Spire
    AddSC_boss_orebender_gorashan();
    AddSC_boss_kyrak();
    AddSC_boss_commander_tharbek();
    AddSC_boss_ragewing_untamed();
    AddSC_boss_warlord_zaela();
    AddSC_upper_blackrock_spire();

    AddSC_instance_shadowmoon_burial_grounds();   // Shadowmoon Burial Grounds
    AddSC_boss_sadana_bloodfury();
    AddSC_boss_nhallish();
    AddSC_boss_bonemaw();
    AddSC_boss_nerzhul();

    AddSC_instance_bloodmaul_slag_mines(); //< Bloodmaul Slag Mines
    AddSC_boss_slave_watcher_crushto();
    AddSC_boss_forgemaster_gogduh();
    AddSC_boss_roltall();
    AddSC_boss_gugrokk();
    AddSC_bloodmaul_slag_mines();

    AddSC_instance_skyreach(); // Skyreach
    AddSC_boss_ranjit();
    AddSC_boss_araknath();
    AddSC_boss_rukhran();
    AddSC_boss_high_sage_viryx();

    AddSC_instance_the_everbloom(); // The Everbloom
    AddSC_boss_witherbark();
    AddSC_boss_ancient_protectors();
    AddSC_boss_xeritac();
    AddSC_boss_archmage_sol();
    AddSC_boss_yalnu();

    AddSC_instance_auchindoun();
    AddSC_auchindoun();
    AddSC_boss_azzakel();
    AddSC_boss_teronogor();
    AddSC_boss_nyami();
    AddSC_boss_kaathar();

    AddSC_instance_highmaul(); // Raid: Highmaul
    AddSC_boss_twin_ogron();
    AddSC_boss_brackenspore();
    AddSC_boss_imperator_margok();
    AddSC_boss_kargath_bladefist();
    AddSC_boss_koragh();
    AddSC_boss_tectus();
    AddSC_boss_the_butcher();
    AddSC_highmaul();

    AddSC_instance_blackrock_foundry(); // Raid: Blackrock Foundry

    AddSC_instance_hellfire_citadel();  // Raid: Hellfire Citadel

    AddSC_world_bossess_draenor();
    AddSC_edge_of_reality();
}