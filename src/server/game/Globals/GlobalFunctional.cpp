/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "GlobalFunctional.h"
#include "ObjectMgr.h"

bool normalizePlayerName(std::string& name)
{
    if (name.empty())
        return false;

    if (name[0] == -61 && name[1] == -97) // Interdiction d'utiliser ce caractere au debut, il fait planter l'affichage cote client
        return false;

    name = sObjectMgr->GetRealCharName(name);

    wchar_t wstr_buf[MAX_INTERNAL_PLAYER_NAME + 1];
    size_t wstr_len = MAX_INTERNAL_PLAYER_NAME;

    if (!Utf8toWStr(name, &wstr_buf[0], wstr_len))
        return false;

    wstr_buf[0] = wcharToUpper(wstr_buf[0]);
    for (size_t i = 1; i < wstr_len; ++i)
        wstr_buf[i] = wcharToLower(wstr_buf[i]);

    if (!WStrToUtf8(wstr_buf, wstr_len, name))
        return false;

    return true;
}

ExtendedPlayerName ExtractExtendedPlayerName(std::string const& name)
{
    size_t pos = name.find('-');
    if (pos != std::string::npos)
        return ExtendedPlayerName(name.substr(0, pos), name.substr(pos + 1));
    return ExtendedPlayerName(name, "");
}

LanguageDesc lang_description[LANGUAGE_DESC_COUNT] =
{
    { LANG_ADDON,           0, 0 },
    { LANG_UNIVERSAL,       0, 0 },
    { LANG_ORCISH,        669, SKILL_LANG_ORCISH },
    { LANG_DARNASSIAN,    671, SKILL_LANG_DARNASSIAN },
    { LANG_TAURAHE,       670, SKILL_LANG_TAURAHE },
    { LANG_DWARVISH,      672, SKILL_LANG_DWARVEN },
    { LANG_COMMON,        668, SKILL_LANG_COMMON },
    { LANG_DEMONIC,       815, SKILL_LANG_DEMON_TONGUE },
    { LANG_TITAN,         816, SKILL_LANG_TITAN },
    { LANG_THALASSIAN,    813, SKILL_LANG_THALASSIAN },
    { LANG_DRACONIC,      814, SKILL_LANG_DRACONIC },
    { LANG_KALIMAG,       817, SKILL_LANG_OLD_TONGUE },
    { LANG_GNOMISH,      7340, SKILL_LANG_GNOMISH },
    { LANG_TROLL,        7341, SKILL_LANG_TROLL },
    { LANG_GUTTERSPEAK, 17737, SKILL_LANG_GUTTERSPEAK },
    { LANG_DRAENEI,     29932, SKILL_LANG_DRAENEI },
    { LANG_ZOMBIE,          0, 0 },
    { LANG_GNOMISH_BINARY,  0, 0 },
    { LANG_GOBLIN_BINARY,   0, 0 },
    { LANG_WORGEN,      69270, SKILL_LANG_WORGEN },
    { LANG_GOBLIN,      69269, SKILL_LANG_GOBLIN },
    { LANG_PANDAREN_N,  108127,SKILL_LANG_PANDAREN_NEUTRAL },
    { LANG_PANDAREN_H,  108130,SKILL_LANG_PANDAREN_HORDE },
    { LANG_PANDAREN_A,  108131,SKILL_LANG_PANDAREN_ALLIANCE },
};

LanguageDesc const* GetLanguageDescByID(uint32 lang)
{
    for (uint8 i = 0; i < LANGUAGE_DESC_COUNT; ++i)
        if (uint32(lang_description[i].lang_id) == lang)
            return &lang_description[i];

    return nullptr;
}

