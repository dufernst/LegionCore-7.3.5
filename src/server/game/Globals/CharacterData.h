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

#ifndef _CHARACTER_DATA_STORE_H
#define _CHARACTER_DATA_STORE_H

struct CharcterTemplateClass
{
    CharcterTemplateClass(uint8 factionGroup, uint8 classID);

    uint8 FactionGroup;
    uint8 ClassID;
};

struct CharacterTemplateItem
{
    uint64 RaceMask;
    uint32 ItemID;
    uint32 Count;
    uint8 ClassID;
    uint8 FactionGroup;
};

struct CharacterTemplateQuest
{
    uint64 RaceMask;
    uint32 QuestID;
    uint8 ClassID;
    uint8 FactionGroup;
};

struct CharacterTemplateSpell
{
    uint64 RaceMask;
    uint32 SpellID;
    uint8 ClassID;
    uint8 FactionGroup;
};

struct CharacterTemplateTitle
{
    uint64 RaceMask;
    uint32 TitleID;
    uint8 ClassID;
    uint8 FactionGroup;
};

struct CharacterTemplate
{
    std::vector<CharcterTemplateClass> Classes;
    std::vector<CharacterTemplateItem> Items;
    std::vector<CharacterTemplateQuest> Quests;
    std::vector<CharacterTemplateSpell> Spells;
    std::vector<CharacterTemplateTitle> Titles;
    uint64 RaceMask;
    uint32 TemplateSetID;
    uint32 Money;
    uint16 MapID;
    uint16 iLevel;
    Position Pos;
    std::string Name;
    std::string Description;
    uint8 Level;
    uint8 fromID;
};

typedef std::unordered_map<uint32, CharacterTemplate> CharacterTemplateContainer;
typedef std::map<uint32, uint32> CharacterConversionMap;

class CharacterDataStoreMgr
{
    CharacterDataStoreMgr();
    ~CharacterDataStoreMgr();

public:
    static CharacterDataStoreMgr* instance();

    void LoadCharacterTemplates();
    CharacterTemplateContainer const& GetCharacterTemplates() const;
    CharacterTemplate const* GetCharacterTemplate(uint32 id) const;

    void LoadFactionChangeAchievements();
    void LoadFactionChangeItems();
    void LoadFactionChangeSpells();
    void LoadFactionChangeReputations();
    void LoadFactionChangeTitles();

    CharacterConversionMap GetFactionChangeAchievements();
    CharacterConversionMap GetFactionChangeItems();
    CharacterConversionMap GetFactionChangeSpells();
    CharacterConversionMap GetFactionChangeReputation();
    CharacterConversionMap GetFactionChangeTitles();

    void LoadReservedPlayersNames();
    bool IsReservedName(std::string const& name) const;

    static ResponseCodes CheckPlayerName(std::string const& name, LocaleConstant locale, bool create = false);
    static PetNameInvalidReason CheckPetName(std::string const& name);
    static bool IsValidCharterName(std::string const& name, LocaleConstant locale = LOCALE_enUS);

    static bool CheckDeclinedNames(std::wstring w_ownname, DeclinedName const& names);
private:
    CharacterConversionMap _factionChangeAchievements;
    CharacterConversionMap _factionChangeItems;
    CharacterConversionMap _factionChangeSpells;
    CharacterConversionMap _factionChangeReputation;
    CharacterConversionMap _factionChangeTitles;
    std::set<std::wstring> _reservedNamesStore;
    CharacterTemplateContainer _characterTemplateStore;
};

#define sCharacterDataStore CharacterDataStoreMgr::instance()

#endif
