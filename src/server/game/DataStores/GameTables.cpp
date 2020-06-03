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
 
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

#include "GameTables.h"
#include "Timer.h"
#include "Log.h"
#include "Util.h"
#include "StringFormat.h"

GameTable<GtArmorMitigationByLvlEntry>      sArmorMitigationByLvlGameTable;
GameTable<GtBarberShopCostBaseEntry>        sBarberShopCostBaseGameTable;
GameTable<GtBaseMPEntry>                    sBaseMPGameTable;
GameTable<GtCombatRatingsEntry>             sCombatRatingsGameTable;
GameTable<GtCombatRatingsMultByILvl>        sCombatRatingsMultByILvlGameTable;
GameTable<GtHpPerStaEntry>                  sHpPerStaGameTable;
GameTable<GtItemSocketCostPerLevelEntry>    sItemSocketCostPerLevelGameTable;
GameTable<GtNpcDamageByClassEntry>          sNpcDamageByClassGameTable[MAX_EXPANSIONS];
GameTable<GtNpcManaCostScalerEntry>         sNpcManaCostScalerGameTable;
GameTable<GtNpcTotalHpEntry>                sNpcTotalHpGameTable[MAX_EXPANSIONS];
GameTable<GtSpellScalingEntry>              sSpellScalingGameTable;
GameTable<GtXpEntry>                        sXpGameTable;
GameTable<GtHonorLevelEntry>                sHonorLevelGameTable;
GameTable<GtArtifactLevelXPEntry>           sArtifactLevelXPGameTable;
GameTable<GtArtifactKnowledgeMultiplierEntry> sArtifactKnowledgeMultiplierGameTable;
GameTable<GtBattlePetXPEntry>               sBattlePetXPTable;
GameTable<GtBattlePetTypeDamageModEntry>    sBattlePetTypeDamageModTable;
GameTable<GtChallengeModeDamageEntry>       sChallengeModeDamageTable;
GameTable<GtChallengeModeHealthEntry>       sChallengeModeHealthTable;

template<class T>
uint32 LoadGameTable(StringVector& errors, GameTable<T>& storage, boost::filesystem::path const& path)
{
    std::ifstream stream(path.string());
    if (!stream)
    {
        errors.push_back(Trinity::StringFormat("GameTable file %s cannot be opened.", path.string().c_str()));
        return 0;
    }

    std::string headers;
    if (!std::getline(stream, headers))
    {
        errors.push_back(Trinity::StringFormat("GameTable file %s is empty.", path.string().c_str()));
        return 0;
    }

    boost::algorithm::trim(headers);
    Tokenizer columnDefs(headers, '\t', 0, false);

    if (columnDefs.size() != sizeof(T) / sizeof(float))
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "GameTable '%s' has different count of columns " SZFMTD " than expected by size of C++ structure (" SZFMTD ").", path.string().c_str(), columnDefs.size(), sizeof(T) / sizeof(float));
        // errors.push_back(Trinity::StringFormat("GameTable '%s' has different count of columns " SZFMTD " than expected by size of C++ structure (" SZFMTD ").", path.string().c_str(), columnDefs.size(), sizeof(T) / sizeof(float)));
    // ASSERT(columnDefs.size() - 1 == sizeof(T) / sizeof(float),
        // "GameTable '%s' has different count of columns " SZFMTD " than expected by size of C++ structure (" SZFMTD ").",
        // path.string().c_str(), columnDefs.size() - 1, sizeof(T) / sizeof(float));

    std::vector<T> data;
    data.emplace_back(); // row id 0, unused

    std::string line;
    while (std::getline(stream, line))
    {
        boost::algorithm::trim(line);
        Tokenizer values(line, '\t', columnDefs.size());
        if (values.empty())
            break;

        // make end point just after last nonempty token
        auto end = values.begin() + values.size() - 1;
        while (!strlen(*end) && end != values.begin())
            --end;

        if (values.begin() == end)
            break;

        ++end;

        // if (std::distance(values.begin(), end) > columnDefs.size()) //Don`t load last empty tab on nix
            // break;

        ASSERT(std::distance(values.begin(), end) == columnDefs.size(), SZFMTD " == " SZFMTD, std::distance(values.begin(), end), columnDefs.size());

        data.emplace_back();
        auto row = reinterpret_cast<float*>(&data.back());
        for (auto itr = values.begin(); itr != end; ++itr)
            *row++ = strtof(*itr, nullptr);
    }

    storage.SetData(std::move(data));
    return 1;
}

void LoadGameTables(std::string const& dataPath)
{
    uint32 oldMSTime = getMSTime();

    boost::filesystem::path gtPath(dataPath);
    gtPath /= "gt";

    StringVector bad_gt_files;
    uint32 gameTableCount = 0;

#define LOAD_GT(store, file) gameTableCount += LoadGameTable(bad_gt_files, store, gtPath / file)

    LOAD_GT(sArmorMitigationByLvlGameTable, "ArmorMitigationByLvl.txt");
    LOAD_GT(sBarberShopCostBaseGameTable, "BarberShopCostBase.txt");
    LOAD_GT(sBaseMPGameTable, "BaseMp.txt");
    LOAD_GT(sCombatRatingsGameTable, "CombatRatings.txt");
    LOAD_GT(sCombatRatingsMultByILvlGameTable, "CombatRatingsMultByILvl.txt");
    LOAD_GT(sItemSocketCostPerLevelGameTable, "ItemSocketCostPerLevel.txt");
    LOAD_GT(sHpPerStaGameTable, "HpPerSta.txt");
    LOAD_GT(sNpcDamageByClassGameTable[0], "NpcDamageByClass.txt");
    LOAD_GT(sNpcDamageByClassGameTable[1], "NpcDamageByClassExp1.txt");
    LOAD_GT(sNpcDamageByClassGameTable[2], "NpcDamageByClassExp2.txt");
    LOAD_GT(sNpcDamageByClassGameTable[3], "NpcDamageByClassExp3.txt");
    LOAD_GT(sNpcDamageByClassGameTable[4], "NpcDamageByClassExp4.txt");
    LOAD_GT(sNpcDamageByClassGameTable[5], "NpcDamageByClassExp5.txt");
    LOAD_GT(sNpcDamageByClassGameTable[6], "NpcDamageByClassExp6.txt");
    LOAD_GT(sNpcManaCostScalerGameTable, "NPCManaCostScaler.txt");
    LOAD_GT(sNpcTotalHpGameTable[0], "NpcTotalHp.txt");
    LOAD_GT(sNpcTotalHpGameTable[1], "NpcTotalHpExp1.txt");
    LOAD_GT(sNpcTotalHpGameTable[2], "NpcTotalHpExp2.txt");
    LOAD_GT(sNpcTotalHpGameTable[3], "NpcTotalHpExp3.txt");
    LOAD_GT(sNpcTotalHpGameTable[4], "NpcTotalHpExp4.txt");
    LOAD_GT(sNpcTotalHpGameTable[5], "NpcTotalHpExp5.txt");
    LOAD_GT(sNpcTotalHpGameTable[6], "NpcTotalHpExp6.txt");
    LOAD_GT(sSpellScalingGameTable, "SpellScaling.txt");
    LOAD_GT(sXpGameTable, "xp.txt");
    LOAD_GT(sHonorLevelGameTable, "HonorLevel.txt");
    LOAD_GT(sArtifactLevelXPGameTable, "ArtifactLevelXP.txt");
    LOAD_GT(sArtifactKnowledgeMultiplierGameTable, "ArtifactKnowledgeMultiplier.txt");
    LOAD_GT(sBattlePetXPTable, "BattlePetXP.txt");
    LOAD_GT(sBattlePetTypeDamageModTable, "BattlePetTypeDamageMod.txt");
    LOAD_GT(sChallengeModeDamageTable, "ChallengeModeDamage.txt");
    LOAD_GT(sChallengeModeHealthTable, "ChallengeModeHealth.txt");

#undef LOAD_GT

    // error checks
    if (!bad_gt_files.empty())
    {
        std::ostringstream str;
        for (std::string const& err  : bad_gt_files)
            str << err << std::endl;

        TC_LOG_ERROR(LOG_FILTER_GENERAL, "\nSome required *.txt GameTable files (" SZFMTD ") not found or not compatible:\n%s", bad_gt_files.size(), str.str().c_str());
        exit(1);
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Initialized %d GameTables in %u ms", gameTableCount, GetMSTimeDiffToNow(oldMSTime));
}
