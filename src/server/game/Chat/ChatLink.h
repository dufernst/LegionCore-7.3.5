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

#ifndef CHATLINK_H
#define CHATLINK_H

#include <sstream>
#include <list>
#include "Common.h"

class SpellInfo;
class Quest;

struct ItemLocale;
struct ItemTemplate;
struct ItemRandomSuffixEntry;
struct ItemRandomPropertiesEntry;
struct AchievementEntry;
struct GlyphPropertiesEntry;

class ChatLink
{
public:
    ChatLink();
    virtual ~ChatLink() = default;

    void SetColor(uint32 color);
    void SetBounds(std::istringstream::pos_type startPos, std::istringstream::pos_type endPos);
    virtual bool Initialize(std::istringstream& iss) = 0;
    virtual bool ValidateName(char* buffer, const char* context) = 0;
    bool ValidatePlayerGUID(std::istringstream& iss, std::string const& context/*, uint32 realmId, uint32 lowGUID*/);
    bool ValidateGuildGUID(std::istringstream& iss, std::string const& context/*, uint32 realmId, uint32 lowGUID*/);

protected:
    uint32 _color;
    std::string _name;
    std::istringstream::pos_type _startPos;
    std::istringstream::pos_type _endPos;
};

class LinkExtractor
{
public:
    explicit LinkExtractor(const char* msg);
    ~LinkExtractor();

    bool IsValidMessage();

private:
    std::list<ChatLink*> _links;
    std::istringstream _iss;
};

//"|c%s|Hitem:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d"
class ItemChatLink : public ChatLink
{
public:
    ItemChatLink();
    ItemChatLink(uint32 id);

    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    std::string FormatName(uint8 index, LocalizedString* suffixStrings) const;
    bool HasValue(std::istringstream& iss) const;

    ItemTemplate const* _item;
    int32 _enchantId;
    int32 _gemItemId[3];
    int32 _randomPropertyId;
    int32 _randomPropertySeed;
    int32 _reporterLevel;
    int32 _reporterSpec;
    int32 _context;
    std::vector<int32> _bonusListIDs;
    std::vector<std::pair<uint32, int32>> _modifiers;
    std::vector<int32> _gemBonusListIDs[3];
    ItemRandomSuffixEntry const* _suffix;
    ItemRandomPropertiesEntry const* _property;
};

//"|c%s|Hquest:%d:%d:%d:%d|h[%s]|h%s"
class QuestChatLink : public ChatLink
{
public:
    QuestChatLink();
    QuestChatLink(uint32 id);

    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    Quest const* _quest;
    int32 _questLevel;
    int32 _minLevel;
    int32 _maxLevel;
};

//"|c%s|Hspell:%d:%d|h[%s]|h%s"
class SpellChatLink : public ChatLink
{
public:
    SpellChatLink();
    SpellChatLink(uint32 id);

    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    SpellInfo const* _spell;
};

//"|c%s|Hachievement:%d:%s:%d:%d:%d:%d:%u:%u:%u:%u|h[%s]|h%s"
class AchievementChatLink : public ChatLink
{
public:
    AchievementChatLink();
    AchievementChatLink(uint32 id);

    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _guid;
    AchievementEntry const* _achievement;
    uint32 _data[8];
};

//"|c%s|Htrade:%s:%d:%d|h[%s]|h|r"
class TradeChatLink : public SpellChatLink
{
public:
    TradeChatLink();
    TradeChatLink(uint32 id);

    bool Initialize(std::istringstream& iss) override;
private:
    int32 _minSkillLevel;
    int32 _maxSkillLevel;
    std::string _base64;
};

//"|c%s|Htalent:%d|h[%s]|h%s"
class TalentChatLink : public SpellChatLink
{
public:
    TalentChatLink();
    TalentChatLink(uint32 id);

    bool Initialize(std::istringstream& iss) override;

private:
    uint32 _talentId;
};

//"|c%s|Hpvptal:%d|h[%s]|h%s"
class PvTalentChatLink : public SpellChatLink
{
public:
    PvTalentChatLink();
    PvTalentChatLink(uint32 id);

    bool Initialize(std::istringstream& iss) override;

private:
    uint32 _talentId;
};

//"|c%s|Henchant:%d|h[%s: %s]|h|r"
//"|c%s|Henchant:%d|h[%s]|h|r"
class EnchantmentChatLink : public SpellChatLink
{
public:
    EnchantmentChatLink() = default;
    EnchantmentChatLink(uint32 id) : SpellChatLink(id) {}

    bool Initialize(std::istringstream& iss) override;
};

//"|c%s|Hcurrency:%d|h[%s]|h%s"
class CurrencyChatLink : public SpellChatLink
{
public:
    CurrencyChatLink();
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

private:
    uint32 _currencyId;
};

//"|c%s|Hbattlepet:%d:%d:%d:%d:%d:%d:%s|h[%s]|h%s"
class BattlePetChatLink : public SpellChatLink
{
public:
    BattlePetChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Hgarrfollowerability:%d|h[%s]|h%s"
class GarrFollowerAbilityChatLink : public SpellChatLink
{
public:
    GarrFollowerAbilityChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Hgarrfollower:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d|h[%s]|h%s"
class GarrFollowerChatLink : public SpellChatLink
{
public:
    GarrFollowerChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[12];
};

//"|c%s|Hgarrmission:%d:%016llx|h[%s]|h%s"
class GarrMissionChatLink : public SpellChatLink
{
public:
    GarrMissionChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Hinstancelock:%s:%d:%u:%d|h[%s]|h%s"
class InstanceLockChatLink : public SpellChatLink
{
public:
    InstanceLockChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Hkeystone:%d:%d:%d:%d:%d|h[%s]|h%s"
class KeystoneChatLink : public SpellChatLink
{
public:
    KeystoneChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Htransmogillusion:%d|h[%s]|h%s"
class TransmogIllusionChatLink : public SpellChatLink
{
public:
    TransmogIllusionChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Htransmogappearance:%d|h[%s]|h%s"
class TransmogAppearanceChatLink : public SpellChatLink
{
public:
    TransmogAppearanceChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Hjournal:%d:%d:%d|h[%s]|h%s"
class JournalChatLink : public SpellChatLink
{
public:
    JournalChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;

protected:
    uint32 _data[5];
};

//"|c%s|Hapower:%d:%d:%d|h[%s]|h%s"
class ArtifactPowerChatLink : public SpellChatLink
{
public:
    ArtifactPowerChatLink() = default;
    bool Initialize(std::istringstream& iss) override;
    bool ValidateName(char* buffer, const char* context) override;
};

#endif // CHATLINK_H
