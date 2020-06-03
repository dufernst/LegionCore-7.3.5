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

#include "CharacterData.h"
#include "WordFilterMgr.h"
#include "AuthenticationPackets.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "GlobalFunctional.h"

CharcterTemplateClass::CharcterTemplateClass(uint8 factionGroup, uint8 classID) : FactionGroup(factionGroup), ClassID(classID)
{
}

enum LanguageType
{
    LT_BASIC_LATIN = 0x0000,
    LT_EXTENDEN_LATIN = 0x0001,
    LT_CYRILLIC = 0x0002,
    LT_EAST_ASIA = 0x0004,
    LT_ANY = 0xFFFF
};

CharacterDataStoreMgr::CharacterDataStoreMgr()
{
}

CharacterDataStoreMgr::~CharacterDataStoreMgr()
{
}

CharacterDataStoreMgr* CharacterDataStoreMgr::instance()
{
    static CharacterDataStoreMgr instance;
    return &instance;
}

void CharacterDataStoreMgr::LoadCharacterTemplates()
{
    uint32 oldMSTime = getMSTime();
    _characterTemplateStore.clear();

    auto templates = WorldDatabase.Query(WorldDatabase.GetPreparedStatement(WORLD_SEL_CHARACTER_TEMPLATES));
    if (!templates)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadCharacterTemplates() >> Loaded 0 character templates. DB table `character_template` is empty.");
        return;
    }

    uint32 count[2] = {0, 0};

    do
    {
        auto fields = templates->Fetch();

        uint32 templateSetID = fields[0].GetUInt32();
        uint32 fromID = fields[5].GetUInt8();
        uint32 selectSetID = fromID ? fromID : templateSetID;

        auto stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CHARACTER_TEMPLATE_CLASSES);
        stmt->setUInt32(0, selectSetID);
        if (auto classes = WorldDatabase.Query(stmt))
        {
            CharacterTemplate templ;
            templ.TemplateSetID = templateSetID;
            templ.Name = fields[1].GetString();
            templ.Description = fields[2].GetString();
            templ.Level = fields[3].GetUInt8();
            templ.iLevel = fields[4].GetUInt16();
            templ.RaceMask = fields[5].GetUInt64();
            templ.fromID = fromID;
            templ.Money = 0;
            templ.MapID = 0;

            do
            {
                fields = classes->Fetch();

                uint8 factionGroup = fields[0].GetUInt8();
                if (!((factionGroup & (FACTION_MASK_PLAYER | FACTION_MASK_ALLIANCE)) == (FACTION_MASK_PLAYER | FACTION_MASK_ALLIANCE)) &&
                    !((factionGroup & (FACTION_MASK_PLAYER | FACTION_MASK_HORDE)) == (FACTION_MASK_PLAYER | FACTION_MASK_HORDE)))
                    continue;

                uint8 classID = fields[1].GetUInt8();
                if (!sChrClassesStore.LookupEntry(classID))
                    continue;

                ++count[1];
                templ.Classes.emplace_back(factionGroup, classID);
                templ.Pos = Position(fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), fields[5].GetFloat());
                templ.MapID = fields[6].GetUInt16();
                templ.Money = fields[7].GetUInt32();

                if (auto result = WorldDatabase.PQuery("SELECT `ItemID`, `Count`, `RaceMask` FROM `character_template_item` WHERE (`ClassID` = '%u' OR `ClassID` = 0) AND (`FactionGroup` = '%u' OR `FactionGroup` = 0) AND `TemplateID` = '%u';", classID, factionGroup, selectSetID))
                {
                    do
                    {
                        auto fieldTemp = result->Fetch();
                        CharacterTemplateItem temp;
                        temp.ItemID = fieldTemp[0].GetUInt32();
                        temp.Count = fieldTemp[1].GetUInt32();
                        temp.RaceMask = fieldTemp[2].GetUInt64();
                        temp.ClassID = classID;
                        temp.FactionGroup = factionGroup;
                        templ.Items.push_back(temp);
                    } while (result->NextRow());
                }

                if (auto result = WorldDatabase.PQuery("SELECT `QuestID`, `RaceMask` FROM `character_template_quest` WHERE (`ClassID` = '%u' OR `ClassID` = 0) AND (`FactionGroup` = '%u' OR `FactionGroup` = 0) AND `TemplateID` = '%u';", classID, factionGroup, selectSetID))
                {
                    do
                    {
                        auto fieldTemp = result->Fetch();
                        CharacterTemplateQuest temp;
                        temp.QuestID = fieldTemp[0].GetUInt32();
                        temp.RaceMask = fieldTemp[1].GetUInt64();
                        temp.ClassID = classID;
                        temp.FactionGroup = factionGroup;
                        templ.Quests.push_back(temp);
                    } while (result->NextRow());
                }

                if (auto result = WorldDatabase.PQuery("SELECT `SpellID`, `RaceMask` FROM `character_template_spell` WHERE (`ClassID` = '%u' OR `ClassID` = 0) AND (`FactionGroup` = '%u' OR `FactionGroup` = 0) AND `TemplateID` = '%u';", classID, factionGroup, selectSetID))
                {
                    do
                    {
                        auto fieldTemp = result->Fetch();
                        CharacterTemplateSpell temp;
                        temp.SpellID = fieldTemp[0].GetUInt32();
                        temp.RaceMask = fieldTemp[1].GetUInt64();
                        temp.ClassID = classID;
                        temp.FactionGroup = factionGroup;
                        templ.Spells.push_back(temp);
                    } while (result->NextRow());
                }

                if (auto result = WorldDatabase.PQuery("SELECT `TitleID`, `RaceMask` FROM `character_template_title` WHERE (`ClassID` = '%u' OR `ClassID` = 0) AND (`FactionGroup` = '%u' OR `FactionGroup` = 0) AND `TemplateID` = '%u';", classID, factionGroup, selectSetID))
                {
                    do
                    {
                        auto fieldTemp = result->Fetch();
                        CharacterTemplateTitle temp;
                        temp.TitleID = fieldTemp[0].GetUInt32();
                        temp.RaceMask = fieldTemp[1].GetUInt64();
                        temp.ClassID = classID;
                        temp.FactionGroup = factionGroup;
                        templ.Titles.push_back(temp);
                    } while (result->NextRow());
                }

            } while (classes->NextRow());

            if (!templ.Classes.empty())
            {
                _characterTemplateStore[templateSetID] = templ;
                ++count[0];
            }
        }
    } while (templates->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadCharacterTemplates() >> Loaded %u character templates with character_template for %u classes in %u ms.", count[0], count[1], GetMSTimeDiffToNow(oldMSTime));
}

CharacterTemplateContainer const& CharacterDataStoreMgr::GetCharacterTemplates() const
{
    return _characterTemplateStore;
}

CharacterTemplate const* CharacterDataStoreMgr::GetCharacterTemplate(uint32 id) const
{
    return Trinity::Containers::MapGetValuePtr(_characterTemplateStore, id);
}

void CharacterDataStoreMgr::LoadFactionChangeAchievements()
{
    uint32 oldMSTime = getMSTime();

    auto result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_achievement");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeAchievements() >> Loaded 0 faction change achievement pairs. DB table `player_factionchange_achievement` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        auto alliance = fields[0].GetUInt32();
        auto horde = fields[1].GetUInt32();

        if (!sAchievementStore.LookupEntry(alliance))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Achievement %u referenced in `player_factionchange_achievement` does not exist, pair skipped!", alliance);
        else if (!sAchievementStore.LookupEntry(horde))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Achievement %u referenced in `player_factionchange_achievement` does not exist, pair skipped!", horde);
        else
            _factionChangeAchievements[alliance] = horde;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeAchievements() >> Loaded %u faction change achievement pairs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void CharacterDataStoreMgr::LoadFactionChangeItems()
{
    uint32 oldMSTime = getMSTime();

    auto result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_items");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeItems() >> Loaded 0 faction change item pairs. DB table `player_factionchange_items` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();

        auto alliance = fields[0].GetUInt32();
        auto horde = fields[1].GetUInt32();

        if (!sObjectMgr->GetItemTemplate(alliance))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Item %u referenced in `player_factionchange_items` does not exist, pair skipped!", alliance);
        else if (!sObjectMgr->GetItemTemplate(horde))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Item %u referenced in `player_factionchange_items` does not exist, pair skipped!", horde);
        else
            _factionChangeItems[alliance] = horde;

        ++count;
    } while (result->NextRow());

    for (ItemSparseEntry const* sparse : sItemSparseStore)
    {
        if (_factionChangeItems.find(sparse->ID) != _factionChangeItems.end())
            continue;

        ItemEntry const* db2Data = sItemStore.LookupEntry(sparse->ID);
        if (!db2Data)
            continue;

        if (sparse->ExpansionID != EXPANSION_LEGION)
            continue;

        if (sparse->Flags[1] & ITEM_FLAG2_FACTION_ALLIANCE)
        {
            ItemSparseEntry const* nextItem = sItemSparseStore.LookupEntry(sparse->ID + 1);
            if (!nextItem)
                continue;

            if (!(nextItem->Flags[1] & ITEM_FLAG2_FACTION_HORDE))
                continue;

            if (strlen(sparse->Display->Str[LOCALE_enUS]) > 0 && strcmp(sparse->Display->Str[LOCALE_enUS], nextItem->Display->Str[LOCALE_enUS]) == 0)
            {
                _factionChangeItems[sparse->ID] = sparse->ID + 1;
                ++count;
            }
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeItems() >> Loaded %u faction change item pairs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void CharacterDataStoreMgr::LoadFactionChangeSpells()
{
    uint32 oldMSTime = getMSTime();

    auto result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeSpells() >> Loaded 0 faction change spell pairs. DB table `player_factionchange_spells` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();
        auto alliance = fields[0].GetUInt32();
        auto horde = fields[1].GetUInt32();

        if (!sSpellMgr->GetSpellInfo(alliance))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u referenced in `player_factionchange_spells` does not exist, pair skipped!", alliance);
        else if (!sSpellMgr->GetSpellInfo(horde))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u referenced in `player_factionchange_spells` does not exist, pair skipped!", horde);
        else
            _factionChangeSpells[alliance] = horde;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeSpells() >> Loaded %u faction change spell pairs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void CharacterDataStoreMgr::LoadFactionChangeReputations()
{
    uint32 oldMSTime = getMSTime();

    auto result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_reputations");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeReputations() >> Loaded 0 faction change reputation pairs. DB table `player_factionchange_reputations` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();

        auto alliance = fields[0].GetUInt32();
        auto horde = fields[1].GetUInt32();

        if (!sFactionStore.LookupEntry(alliance))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Reputation %u referenced in `player_factionchange_reputations` does not exist, pair skipped!", alliance);
        else if (!sFactionStore.LookupEntry(horde))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Reputation %u referenced in `player_factionchange_reputations` does not exist, pair skipped!", horde);
        else
            _factionChangeReputation[alliance] = horde;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeReputations() >> Loaded %u faction change reputation pairs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void CharacterDataStoreMgr::LoadFactionChangeTitles()
{
    uint32 oldMSTime = getMSTime();

    auto result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_titles");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeTitles() >> Loaded 0 faction change title pairs. DB table `player_factionchange_title` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();

        auto alliance = fields[0].GetUInt32();
        auto horde = fields[1].GetUInt32();

        if (!sCharTitlesStore.LookupEntry(alliance))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Title %u referenced in `player_factionchange_title` does not exist, pair skipped!", alliance);
        else if (!sCharTitlesStore.LookupEntry(horde))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Title %u referenced in `player_factionchange_title` does not exist, pair skipped!", horde);
        else
            _factionChangeTitles[alliance] = horde;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadFactionChangeTitles() >> Loaded %u faction change title pairs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

CharacterConversionMap CharacterDataStoreMgr::GetFactionChangeAchievements()
{
    return _factionChangeAchievements;
}

CharacterConversionMap CharacterDataStoreMgr::GetFactionChangeItems()
{
    return _factionChangeItems;
}

CharacterConversionMap CharacterDataStoreMgr::GetFactionChangeSpells()
{
    return _factionChangeSpells;
}

CharacterConversionMap CharacterDataStoreMgr::GetFactionChangeReputation()
{
    return _factionChangeReputation;
}

CharacterConversionMap CharacterDataStoreMgr::GetFactionChangeTitles()
{
    return _factionChangeTitles;
}

void CharacterDataStoreMgr::LoadReservedPlayersNames()
{
    uint32 oldMSTime = getMSTime();

    _reservedNamesStore.clear();

    auto result = CharacterDatabase.Query("SELECT name FROM reserved_name");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadReservedPlayersNames() >> Loaded 0 reserved player names. DB table `reserved_name` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        auto fields = result->Fetch();
        std::string name = fields[0].GetString();

        std::wstring wstr;
        if (!Utf8toWStr(name, wstr))
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Table `reserved_name` have invalid name: %s", name.c_str());
            continue;
        }

        wstrToLower(wstr);

        _reservedNamesStore.insert(wstr);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "CharacterDataStoreMgr::LoadReservedPlayersNames() >> Loaded %u reserved player names in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

bool CharacterDataStoreMgr::IsReservedName(std::string const& name) const
{
    std::wstring wstr;
    if (!Utf8toWStr(name, wstr))
        return false;

    wstrToLower(wstr);

    return _reservedNamesStore.find(wstr) != _reservedNamesStore.end();
}

static LanguageType GetRealmLanguageType(bool create)
{
    switch (sWorld->getIntConfig(CONFIG_REALM_ZONE))
    {
        case REALM_ZONE_UNKNOWN:                            // any language
        case REALM_ZONE_DEVELOPMENT:
        case REALM_ZONE_TEST_SERVER:
        case REALM_ZONE_QA_SERVER:
            return LT_ANY;
        case REALM_ZONE_UNITED_STATES:                      // extended-Latin
        case REALM_ZONE_OCEANIC:
        case REALM_ZONE_LATIN_AMERICA:
        case REALM_ZONE_ENGLISH:
        case REALM_ZONE_GERMAN:
        case REALM_ZONE_FRENCH:
        case REALM_ZONE_SPANISH:
            return LT_EXTENDEN_LATIN;
        case REALM_ZONE_KOREA:                              // East-Asian
        case REALM_ZONE_TAIWAN:
        case REALM_ZONE_CHINA:
            return LT_EAST_ASIA;
        case REALM_ZONE_RUSSIAN:                            // Cyrillic
            return LT_CYRILLIC;
        default:
            return create ? LT_BASIC_LATIN : LT_ANY;        // basic-Latin at create, any at login
    }
}

bool isValidString(std::wstring wstr, uint32 strictMask, bool numericOrSpace, bool create = false)
{
    if (strictMask == 0)                                       // any language, ignore realm
    {
        if (isExtendedLatinString(wstr, numericOrSpace))
            return true;

        if (isCyrillicString(wstr, numericOrSpace))
            return true;

        if (isEastAsianString(wstr, numericOrSpace))
            return true;

        return false;
    }

    if (strictMask & 0x2)                                    // realm zone specific
    {
        LanguageType lt = GetRealmLanguageType(create);
        if (lt & LT_EXTENDEN_LATIN)
            if (isExtendedLatinString(wstr, numericOrSpace))
                return true;
        if (lt & LT_CYRILLIC)
            if (isCyrillicString(wstr, numericOrSpace))
                return true;
        if (lt & LT_EAST_ASIA)
            if (isEastAsianString(wstr, numericOrSpace))
                return true;
    }

    if (strictMask & 0x1)                                    // basic Latin
        if (isBasicLatinString(wstr, numericOrSpace))
            return true;

    return false;
}

ResponseCodes CharacterDataStoreMgr::CheckPlayerName(std::string const& name, LocaleConstant locale, bool create /*= false*/)
{
    std::wstring wname;
    if (!Utf8toWStr(name, wname))
        return CHAR_NAME_INVALID_CHARACTER;

    if (wname.size() > MAX_PLAYER_NAME)
        return CHAR_NAME_TOO_LONG;

    uint32 minName = sWorld->getIntConfig(CONFIG_MIN_PLAYER_NAME);
    if (wname.size() < minName)
        return CHAR_NAME_TOO_SHORT;

    uint32 strictMask = sWorld->getIntConfig(CONFIG_STRICT_PLAYER_NAMES);
    if (!isValidString(wname, strictMask, false, create))
        return CHAR_NAME_MIXED_LANGUAGES;

    wstrToLower(wname);
    for (size_t i = 2; i < wname.size(); ++i)
        if (wname[i] == wname[i - 1] && wname[i] == wname[i - 2])
            return CHAR_NAME_THREE_CONSECUTIVE;

    return sDB2Manager.ValidateName(wname, locale);
}

bool CharacterDataStoreMgr::IsValidCharterName(std::string const& name, LocaleConstant locale /*= LOCALE_enUS */ )
{
    std::wstring wname;
    if (!Utf8toWStr(name, wname))
        return false;

    if (wname.size() > MAX_CHARTER_NAME)
        return false;

    uint32 minName = sWorld->getIntConfig(CONFIG_MIN_CHARTER_NAME);
    if (wname.size() < minName)
        return false;

    wstrToLower(wname);
    for (size_t i = 2; i < wname.size(); ++i)
        if (wname[i] == wname[i - 1] && wname[i] == wname[i - 2])
            return false;

    if (!isValidString(wname, sWorld->getIntConfig(CONFIG_STRICT_CHARTER_NAMES), true))
        return false;

    return sDB2Manager.ValidateName(wname, locale);
}

PetNameInvalidReason CharacterDataStoreMgr::CheckPetName(std::string const& name)
{
    std::wstring wname;
    if (!Utf8toWStr(name, wname))
        return PET_NAME_INVALID;

    if (wname.size() > MAX_PET_NAME)
        return PET_NAME_TOO_LONG;

    uint32 minName = sWorld->getIntConfig(CONFIG_MIN_PET_NAME);
    if (wname.size() < minName)
        return PET_NAME_TOO_SHORT;

    uint32 strictMask = sWorld->getIntConfig(CONFIG_STRICT_PET_NAMES);
    if (!isValidString(wname, strictMask, false))
        return PET_NAME_MIXED_LANGUAGES;

    return PET_NAME_SUCCESS;
}

bool CharacterDataStoreMgr::CheckDeclinedNames(std::wstring w_ownname, DeclinedName const& names)
{
    std::wstring mainpart = GetMainPartOfName(w_ownname, 0);

    bool x = true;
    bool y = true;

    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
    {
        std::wstring wname;
        if (!Utf8toWStr(names.name[i], wname))
            return false;

        if (mainpart != GetMainPartOfName(wname, i + 1))
            x = false;

        if (w_ownname != wname)
            y = false;
    }

    return x || y;
}
