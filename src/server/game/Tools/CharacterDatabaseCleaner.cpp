/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
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

#include "Common.h"
#include "CharacterDatabaseCleaner.h"
#include "World.h"
#include "Database/DatabaseEnv.h"
#include "SpellMgr.h"
#include<string>
#include<vector>

void CharacterDatabaseCleaner::CleanDatabase()
{
    // config to disable
    if (!sWorld->getBoolConfig(CONFIG_CLEAN_CHARACTER_DB))
        return;

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Cleaning character database...");

    uint32 oldMSTime = getMSTime();

    // check flags which clean ups are necessary
    QueryResult result = CharacterDatabase.Query("SELECT value FROM worldstates WHERE entry = 20004");
    if (!result)
        return;

    uint32 flags = (*result)[0].GetUInt32();

    // clean up
    if (flags & CLEANING_FLAG_ACHIEVEMENT_PROGRESS)
        CleanAllAchievementProgress();

    if (flags & CLEANING_FLAG_SKILLS)
        CleanCharacterSkills();

    if (flags & CLEANING_FLAG_SPELLS)
        CleanCharacterSpell();

    if (flags & CLEANING_FLAG_QUESTSTATUS)
        CleanCharacterQuestStatus();

    if (flags & CLEANING_FLAG_PETSLOT)
        CleanPetSlots();

    // NOTE: In order to have persistentFlags be set in worldstates for the next cleanup,
    // you need to define them at least once in worldstates.
    flags &= sWorld->getIntConfig(CONFIG_PERSISTENT_CHARACTER_CLEAN_FLAGS);
    CharacterDatabase.DirectPExecute("UPDATE worldstates SET value = %u WHERE entry = 20004", flags);

    sWorld->SetCleaningFlags(flags);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Cleaned character database in %u ms", GetMSTimeDiffToNow(oldMSTime));

}

void CharacterDatabaseCleaner::CheckUnique(const char* column, const char* table, bool (*check)(uint32))
{
    QueryResult result = CharacterDatabase.PQuery("SELECT DISTINCT %s FROM %s", column, table);
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Table %s is empty.", table);
        return;
    }

    bool found = false;
    std::ostringstream ss;
    do
    {
        Field* fields = result->Fetch();

        uint32 id = fields[0].GetUInt32();

        if (!check(id))
        {
            if (!found)
            {
                ss << "DELETE FROM " << table << " WHERE " << column << " IN (";
                found = true;
            }
            else
                ss << ',';

            ss << id;
        }
    }
    while (result->NextRow());

    if (found)
    {
        ss << ')';
        CharacterDatabase.Execute(ss.str().c_str());
    }
}

bool CharacterDatabaseCleaner::AchievementProgressCheck(uint32 criteria)
{
    return sCriteriaTreeStore.LookupEntry(criteria) != nullptr;
}

void CharacterDatabaseCleaner::CleanCharacterAchievementProgress()
{
    CheckUnique("criteria", "character_achievement_progress", &AchievementProgressCheck);
}

void CharacterDatabaseCleaner::CleanGuildAchievementProgress()
{
    CheckUnique("criteria", "guild_achievement_progress", &AchievementProgressCheck);
}

void CharacterDatabaseCleaner::CleanAccountAchievementProgress()
{
    CheckUnique("criteria", "account_achievement_progress", &AchievementProgressCheck);
}

void CharacterDatabaseCleaner::CleanAllAchievementProgress()
{
    CleanCharacterAchievementProgress();
    CleanAccountAchievementProgress();
    CleanGuildAchievementProgress();
}

bool CharacterDatabaseCleaner::SkillCheck(uint32 skill)
{
    return sSkillLineStore.LookupEntry(skill) != nullptr;
}

void CharacterDatabaseCleaner::CleanCharacterSkills()
{
    CheckUnique("skill", "character_skills", &SkillCheck);
}

bool CharacterDatabaseCleaner::SpellCheck(uint32 spell_id)
{
    return sSpellMgr->GetSpellInfo(spell_id);
}

void CharacterDatabaseCleaner::CleanCharacterSpell()
{
    CheckUnique("spell", "character_spell", &SpellCheck);
}

void CharacterDatabaseCleaner::CleanCharacterQuestStatus()
{
    CharacterDatabase.DirectExecute("DELETE FROM character_queststatus WHERE status = 0");
}

uint8 CheckSlot(PlayerPetSlotList &list, uint8 slot, uint32 id)
{
    uint32 index = 0;
    for(PlayerPetSlotList::iterator itr = list.begin(); itr != list.end(); ++itr, ++index)
    {
        if ((*itr) == id)
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Warning! CheckSlot. Pet Id:%u with Slot: %u already on slot %u", id, slot, index);
            return 1;
        }

        if (slot == index && (*itr) > 0 && (*itr) != id)
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Warning! CheckSlot. Pet Id:%u with Slot: %u has another pet %u", id, slot, *itr);
            return 2;
        }
    }
    return 0;
}

void CharacterDatabaseCleaner::CleanPetSlots()
{
    //QueryResult result = CharacterDatabase.PQuery("SELECT DISTINCT owner FROM character_pet");
    //if (!result)
    //{
    //    TC_LOG_INFO(LOG_FILTER_GENERAL, "Table character_pet is empty.");
    //    return;
    //}
    //
    //uint8 res = 0;
    //do
    //{
    //    Field* fields = result->Fetch();
    //    uint32 ownerID = fields[0].GetUInt32();

    //    QueryResult r2 = CharacterDatabase.PQuery("SELECT id, slot FROM character_pet WHERE owner = %u", ownerID);
    //    if (!r2)
    //    {
    //        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Warning! Problem with table character_pet at cleanup");
    //        continue;
    //    }

    //    PlayerPetSlotList list(PET_SLOT_LAST);
    //    std::set<uint32> lost;
    //    do
    //    {
    //        Field* fields2 = r2->Fetch();
    //        uint8 id = fields2[0].GetUInt32();
    //        uint8 slot = fields2[1].GetUInt8();

    //        res = CheckSlot(list, slot, id);
    //        switch(res)
    //        {
    //            case 1: //double add
    //                break;
    //            case 2: //lost link.
    //                lost.insert(id);
    //                break;
    //            case 0: //good
    //                list.at(slot) = id;
    //                break;
    //        }
    //    }
    //    while (r2->NextRow());

    //    //find slot for lost link
    //    uint32 index = 0;
    //    for(std::set<uint32>::iterator itr = lost.begin(); itr != lost.end(); ++itr)
    //    {
    //        for(; index < PET_SLOT_LAST; ++index)
    //        {
    //            if (list.at(index) > 0)
    //                continue;
    //            list.at(index) = *itr;
    //            break;
    //        }
    //    }

    //    // create save string
    //    std::ostringstream ss;
    //    for (uint32 i = 0; i < PET_SLOT_LAST; ++i)
    //        ss << list[i] << ' ';

    //    CharacterDatabase.PExecute("UPDATE characters SET petSlot = '%s' WHERE guid = %u", ss.str().c_str(), ownerID);
    //}
    //while (result->NextRow());
}
