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

#include "ChatLink.h"
#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "QuestData.h"

enum ChatLinkColors
{
    CHAT_LINK_COLOR_TRADE = 0xffffd000,   // orange
    CHAT_LINK_COLOR_SPELL = 0xff71d5ff,   // bright blue
    CHAT_LINK_COLOR_ENCHANT = 0xffffd000,   // orange
    CHAT_LINK_COLOR_ACHIEVEMENT = 0xffffff00,
    CHAT_LINK_COLOR_GLYPH = 0xff66bbff,
    CHAT_LINK_COLOR_CURRENCY = 0xff0070dd,
    CHAT_LINK_COLOR_GARRFOLLOWERABILITY = 0xff4e96f7,
    CHAT_LINK_COLOR_INSTANCE_LOCK = 0xffff8000,
    CHAT_LINK_COLOR_TRANSMOG_ILLUSION = 0xffff80ff
};

// 26124
//"|c%s|Hachievement:%d:%s:%d:%d:%d:%d:%u:%u:%u:%u|h[%s]|h%s"           - fixed
//"|c%s|Hapower:%d:%d:%d|h[%s]|h%s"                                     - fixed
//"|c%s|Hbattlepet:%d:%d:%d:%d:%d:%d:%s|h[%s]|h%s"                      - fixed
//"|c%s|Hcurrency:%d|h[%s]|h%s"                                         - fixed
//"|c%s|Hgarrfollowerability:%d|h[%s]|h%s"                              - fixed
//"|c%s|Hgarrfollower:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d|h[%s]|h%s" - fixed
//"|c%s|Hgarrmission:%d:%016llx|h[%s]|h%s"                              - fixed
//"|c%s|Hinstancelock:%s:%d:%u:%d|h[%s]|h%s"                            - fixed
//"|c%s|Hitem:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d"                   - fixed
//"|c%s|Hkeystone:%d:%d:%d:%d:%d|h[%s]|h%s"                             - not found on retail
//"|c%s|Hquest:%d:%d:%d:%d|h[%s]|h%s"                                   - fixed
//"|c%s|Hspell:%d:%d|h[%s]|h%s"                                         - fixed
//"|c%s|Hpvptal:%d|h[%s]|h%s"                                           - fixed
//"|c%s|Htalent:%d|h[%s]|h%s"                                           - fixed
//"|c%s|Htransmogappearance:%d|h[%s]|h%s"                               - not found on retail
//"|c%s|Htransmogillusion:%d|h[%s]|h%s"                                 - fixed
//"|c%s|Hjournal:%d:%d:%d|h[%s]|h%s"                                    - fixed
//"|c%s|Htransmogset:%d|h[%s]|h|r"                                      - not found on retail
//"|c%s|Henchant:%d|h[%s: %s]|h|r"                                      - fixed
//"|c%s|Henchant:%d|h[%s]|h|r"                                          - fixed
//"|c%s|Htrade:%s:%d:%d|h[%s]|h|r"                                      - fixed
//"|T%d:14|t|c%s|HbattlePetAbil:%d:%d:%d:%d|h[%s]|h|r"                  - not found on retail

inline bool ReadUInt32(std::istringstream& iss, uint32& res)
{
    iss >> std::dec >> res;
    return !iss.fail() && !iss.eof();
}

inline bool ReadInt32(std::istringstream& iss, int32& res)
{
    iss >> std::dec >> res;
    return !iss.fail() && !iss.eof();
}

inline bool ReadInt64(std::istringstream& iss, int64& res)
{
    iss >> std::dec >> res;
    return !iss.fail() && !iss.eof();
}

inline bool ReadUInt64(std::istringstream& iss, uint64& res)
{
    iss >> std::dec >> res;
    return !iss.fail() && !iss.eof();
}

inline std::string ReadSkip(std::istringstream& iss, char term)
{
    std::string res;
    char c = iss.peek();
    while (c != term && c != '\0')
    {
        res += c;
        iss.ignore(1);
        c = iss.peek();
    }
    return res;
}

inline bool CheckDelimiter(std::istringstream& iss, char delimiter, char const* context)
{
    char c = iss.peek();
    if (c != delimiter)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): invalid %s link structure ('%c' expected, '%c' found)", iss.str().c_str(), context, delimiter, c);
        return false;
    }
    iss.ignore(1);
    return true;
}

inline bool ReadHex(std::istringstream& iss, uint32& res, uint32 length)
{
    std::istringstream::pos_type pos = iss.tellg();
    iss >> std::hex >> res;
    //uint32 size = uint32(iss.gcount());
    if (length && uint32(iss.tellg() - pos) != length)
        return false;
    return !iss.fail() && !iss.eof();
}

inline bool ReadHex(std::istringstream& iss, uint64& res, uint32 length)
{
    std::istringstream::pos_type pos = iss.tellg();
    iss >> std::hex >> res;
    //uint32 size = uint32(iss.gcount());
    if (length && uint32(iss.tellg() - pos) != length)
        return false;
    return !iss.fail() && !iss.eof();
}

#define DELIMITER ':'
#define PIPE_CHAR '|'

ChatLink::ChatLink(): _color(0), _startPos(0), _endPos(0) { }

void ChatLink::SetColor(uint32 color)
{
    _color = color;
}

void ChatLink::SetBounds(std::istringstream::pos_type startPos, std::istringstream::pos_type endPos)
{
    _startPos = startPos;
    _endPos = endPos;
}

bool ChatLink::ValidateName(char* buffer, char const* /*context*/)
{
    _name = buffer;
    return true;
}

bool ChatLink::ValidatePlayerGUID(std::istringstream& iss, std::string const& context/*, uint32 realmId, uint32 lowGUID*/)
{
    char buffer[6];
    iss.read(buffer, 6);

    if (strcmp(buffer, "Player") != 0)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading %s PlayerGUID string", iss.str().c_str(), context.c_str());
        return false;
    }

    if (!CheckDelimiter(iss, '-', context.c_str()))
        return false;

    uint32 realmId = 0;
    if (!ReadUInt32(iss, realmId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading %s realmID", iss.str().c_str(), context.c_str());
        return false;
    }

    if (!CheckDelimiter(iss, '-', context.c_str()))
        return false;

    uint32 lowGUID = 0;
    if (!ReadHex(iss, lowGUID, 8))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading %s lowGUID", iss.str().c_str(), context.c_str());
        return false;
    }

    return true;
}

bool ChatLink::ValidateGuildGUID(std::istringstream& iss, std::string const& context/*, uint32 realmId, uint32 lowGUID*/)
{
    char buffer[5];
    iss.read(buffer, 5);

    if (strcmp(buffer, "Guild") != 0)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading %s GuildGUID string", iss.str().c_str(), context.c_str());
        return false;
    }

    if (!CheckDelimiter(iss, '-', context.c_str()))
        return false;

    uint32 realmId = 0;
    if (!ReadUInt32(iss, realmId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading %s realmID", iss.str().c_str(), context.c_str());
        return false;
    }

    if (!CheckDelimiter(iss, '-', context.c_str()))
        return false;

    uint64 lowGUID = 0;
    if (!ReadHex(iss, lowGUID, 12))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading %s lowGUID", iss.str().c_str(), context.c_str());
        return false;
    }

    return true;
}

LinkExtractor::LinkExtractor(char const* msg) : _iss(msg) { }

LinkExtractor::~LinkExtractor()
{
    for (auto& itr : _links)
        delete itr;
    _links.clear();
}

bool LinkExtractor::IsValidMessage()
{
    const char validSequence[6] = "cHhhr";
    char const* validSequenceIterator = validSequence;

    char buffer[256];

    std::istringstream::pos_type startPos = 0;
    uint32 color = 0;

    ChatLink* link = nullptr;
    while (!_iss.eof())
    {
        if (validSequence == validSequenceIterator)
        {
            link = nullptr;
            _iss.ignore(255, PIPE_CHAR);
            startPos = _iss.tellg() - std::istringstream::pos_type(1);
        }
        else if (_iss.get() != PIPE_CHAR)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence aborted unexpectedly. Char \"%c\"", _iss.str().c_str(), static_cast<char>(_iss.get()));
            return false;
        }

        // pipe has always to be followed by at least one char
        if (_iss.peek() == '\0')
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): pipe followed by '\\0'", _iss.str().c_str());
            return false;
        }

        // no further pipe commands
        if (_iss.eof())
            break;

        char commandChar;
        _iss.get(commandChar);

        // | in normal messages is escaped by ||
        if (commandChar != PIPE_CHAR)
        {
            if (commandChar == *validSequenceIterator)
            {
                if (validSequenceIterator == validSequence + 4)
                    validSequenceIterator = validSequence;
                else
                    ++validSequenceIterator;
            }
            else
            {
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): invalid sequence, expected '%c' but got '%c'", _iss.str().c_str(), *validSequenceIterator, commandChar);
                return false;
            }
        }
        else if (validSequence != validSequenceIterator)
        {
            // no escaped pipes in sequences
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got escaped pipe in sequence", _iss.str().c_str());
            return false;
        }

        switch (commandChar)
        {
            case 'c':
            {
                if (!ReadHex(_iss, color, 8))
                {
                    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): invalid hexadecimal number while reading color", _iss.str().c_str());
                    return false;
                }
                break;
            }
            case 'H': // read chars up to colon = link type
            {
                _iss.getline(buffer, 256, DELIMITER);
                if (_iss.eof())
                {
                    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly", _iss.str().c_str());
                    return false;
                }

                if (strcmp(buffer, "item") == 0)
                    link = new ItemChatLink();
                else if (strcmp(buffer, "achievement") == 0)
                    link = new AchievementChatLink();
                else if (strcmp(buffer, "spell") == 0)
                    link = new SpellChatLink();
                else if (strcmp(buffer, "apower") == 0)
                    link = new ArtifactPowerChatLink();
                else if (strcmp(buffer, "quest") == 0)
                    link = new QuestChatLink();
                else if (strcmp(buffer, "trade") == 0)
                    link = new TradeChatLink();
                else if (strcmp(buffer, "talent") == 0)
                    link = new TalentChatLink();
                else if (strcmp(buffer, "pvptal") == 0)
                    link = new PvTalentChatLink();
                else if (strcmp(buffer, "enchant") == 0)
                    link = new EnchantmentChatLink();
                else if (strcmp(buffer, "currency") == 0)
                    link = new CurrencyChatLink();
                else if (strcmp(buffer, "instancelock") == 0)
                    link = new InstanceLockChatLink();
                else if (strcmp(buffer, "battlepet") == 0)
                    link = new BattlePetChatLink();
                else if (strcmp(buffer, "garrfollowerability") == 0)
                    link = new GarrFollowerAbilityChatLink();
                else if (strcmp(buffer, "garrfollower") == 0)
                    link = new GarrFollowerChatLink();
                else if (strcmp(buffer, "garrmission") == 0)
                    link = new GarrMissionChatLink();
                else if (strcmp(buffer, "keystone") == 0)
                    link = new KeystoneChatLink();
                else if (strcmp(buffer, "transmogillusion") == 0)
                    link = new TransmogIllusionChatLink();
                else if (strcmp(buffer, "transmogappearance") == 0)
                    link = new TransmogAppearanceChatLink();
                else if (strcmp(buffer, "journal") == 0)
                    link = new JournalChatLink();
                else
                {
                    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): user sent unsupported link type '%s'", _iss.str().c_str(), buffer);
                    return false;
                }

                _links.push_back(link);
                link->SetColor(color);

                if (!link->Initialize(_iss))
                    return false;

                break;
            }
            case 'h': // if h is next element in sequence, this one must contain the linked text :)
            {
                if (*validSequenceIterator == 'h')
                {
                    if (_iss.get() != '[') // links start with '['
                    {
                        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): link caption doesn't start with '['", _iss.str().c_str());
                        return false;
                    }
                    _iss.getline(buffer, 256, ']');
                    if (_iss.eof())
                    {
                        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly", _iss.str().c_str());
                        return false;
                    }

                    if (!link || !link->ValidateName(buffer, _iss.str().c_str()))
                        return false;
                }
                break;
            }
            case 'r':
                if (link)
                    link->SetBounds(startPos, _iss.tellg());
            case '|': // no further payload
                break;
            default:
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid command |%c", _iss.str().c_str(), commandChar);
                return false;
        }
    }

    if (validSequence != validSequenceIterator) // check if every opened sequence was also closed properly
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): EOF in active sequence", _iss.str().c_str());
        return false;
    }

    return true;
}

ItemChatLink::ItemChatLink(): ChatLink(), _item(nullptr), _enchantId(0), _randomPropertyId(0), _randomPropertySeed(0), _reporterLevel(0), _reporterSpec(0), _context(0), _suffix(nullptr), _property(nullptr)
{
    memset(_gemItemId, 0, sizeof(_gemItemId));
}

ItemChatLink::ItemChatLink(uint32 id) : ChatLink(), _item(sObjectMgr->GetItemTemplate(id)), _enchantId(0), _gemItemId{}, _randomPropertyId(0), _randomPropertySeed(0), _reporterLevel(0), _reporterSpec(0), _context(0), _suffix(nullptr), _property(nullptr) { }

// |color|Hitem:item_id:perm_ench_id:gem1:gem2:gem3:0:random_property:property_seed:reporter_level:reporter_spec:modifiers_mask:context:numBonusListIDs:bonusListIDs(%d):mods(%d):gem1numBonusListIDs:gem1bonusListIDs(%d):gem2numBonusListIDs:gem2bonusListIDs(%d):gem3numBonusListIDs:gem3bonusListIDs(%d)|h[name]|h|r
// |cffa335ee|Hitem:124382:0:0:0:0:0:0:0:0:0:0:0:4:42:562:565:567|h[Edict of Argus]|h|r");
bool ItemChatLink::Initialize(std::istringstream& iss)
{
    uint32 itemEntry = 0;
    if (!ReadUInt32(iss, itemEntry))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item entry", iss.str().c_str());
        return false;
    }

    _item = sObjectMgr->GetItemTemplate(itemEntry);
    if (!_item)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid itemEntry %u in |item command", iss.str().c_str(), itemEntry);
        return false;
    }

    bool goodColor = false;
    for (auto itemQualityColor : ItemQualityColors)
    {
        if (_color == itemQualityColor)
        {
            goodColor = true;
            break;
        }
    }

    if (!goodColor)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked item has unknown color %X", iss.str().c_str(), _color);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    // TODO: check valid enchantId
    if (HasValue(iss) && !ReadInt32(iss, _enchantId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item enchantId", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _gemItemId[0]))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item gem id 1", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _gemItemId[1]))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item gem id 2", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _gemItemId[2]))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item gem id 3", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    auto zero = 0;
    if (HasValue(iss) && !ReadInt32(iss, zero))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading zero", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _randomPropertyId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item random property id", iss.str().c_str());
        return false;
    }

    if (_randomPropertyId > 0)
    {
        _property = sItemRandomPropertiesStore.LookupEntry(_randomPropertyId);
        if (!_property)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid item property id %u in |item command", iss.str().c_str(), _randomPropertyId);
            return false;
        }
    }
    else if (_randomPropertyId < 0)
    {
        _suffix = sItemRandomSuffixStore.LookupEntry(-_randomPropertyId);
        if (!_suffix)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid item suffix id %u in |item command", iss.str().c_str(), -_randomPropertyId);
            return false;
        }
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _randomPropertySeed))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item random property seed", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _reporterLevel))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item owner level", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _reporterSpec))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item owner spec", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    auto modifiersMask = 0;
    if (HasValue(iss) && !ReadInt32(iss, modifiersMask))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item modifiers mask", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    if (HasValue(iss) && !ReadInt32(iss, _context))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item context", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "item"))
        return false;

    uint32 numBonusListIDs = 0;
    if (HasValue(iss) && !ReadUInt32(iss, numBonusListIDs))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item bonus lists size", iss.str().c_str());
        return false;
    }

    uint32 const maxBonusListIDs = 16;
    if (numBonusListIDs > maxBonusListIDs)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): too many item bonus list IDs %u in |item command", iss.str().c_str(), numBonusListIDs);
        return false;
    }

    _bonusListIDs.resize(numBonusListIDs);
    for (uint32 index = 0; index < numBonusListIDs; ++index)
    {
        if (!CheckDelimiter(iss, DELIMITER, "item"))
            return false;

        auto id = 0;
        if (!ReadInt32(iss, id))
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item bonus list id (index %u)", iss.str().c_str(), index);
            return false;
        }

        if (!sDB2Manager.GetItemBonusList(id))
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid item bonus list id %d in |item command", iss.str().c_str(), id);
            return false;
        }

        _bonusListIDs[index] = id;
    }

    // TODO: implement reading quality from bonusData and check color quality
    // TODO: implement reading itemNameDescription / ItemNameSuffix from bonusData and check names

    for (uint32 i = 0; i < MAX_ITEM_MODIFIERS; ++i)
    {
        if (modifiersMask & (1 << i))
        {
            if (!CheckDelimiter(iss, DELIMITER, "item"))
                return false;

            auto id = 0;
            if (!ReadInt32(iss, id))
            {
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item modifier id (index %u)", iss.str().c_str(), i);
                return false;
            }

            _modifiers.emplace_back(std::make_pair(i, id));
        }
    }

    for (uint32 i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
    {
        if (!CheckDelimiter(iss, DELIMITER, "item"))
            return false;

        numBonusListIDs = 0;
        if (HasValue(iss) && !ReadUInt32(iss, numBonusListIDs))
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item bonus lists size for gem %u", iss.str().c_str(), i);
            return false;
        }

        if (numBonusListIDs > maxBonusListIDs)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): too many item bonus list IDs %u in |item command for gem %u", iss.str().c_str(), numBonusListIDs, i);
            return false;
        }

        _gemBonusListIDs[i].resize(numBonusListIDs);
        for (uint32 index = 0; index < numBonusListIDs; ++index)
        {
            if (!CheckDelimiter(iss, DELIMITER, "item"))
                return false;

            int32 id = 0;
            if (!ReadInt32(iss, id))
            {
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading item bonus list id (index %u) for gem %u", iss.str().c_str(), index, i);
                return false;
            }

            if (!sDB2Manager.GetItemBonusList(id))
            {
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid item bonus list id %d in |item command for gem %u", iss.str().c_str(), id, i);
                return false;
            }

            _gemBonusListIDs[i][index] = id;
        }
    }

    return true;
}

std::string ItemChatLink::FormatName(uint8 index, LocalizedString* suffixStrings) const
{
    std::stringstream ss;
    ss << _item->GetName()->Str[index];

    if (!(_item->GetFlags3() & ITEM_FLAG3_HIDE_NAME_SUFFIX))
        if (suffixStrings)
            ss << ' ' << suffixStrings->Str[index];
    return ss.str();
}

bool ItemChatLink::HasValue(std::istringstream& iss) const
{
    char next = iss.peek();
    return next != DELIMITER && next != PIPE_CHAR;
}

bool ItemChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    bool result = false;
    for (uint8 index = LOCALE_enUS; index < MAX_LOCALES; ++index)
    {
        if (index == LOCALE_none)
            continue;

        std::string localeItemName = std::string(_item->GetName()->Str[index]);

        if (!localeItemName.empty() && _name.find(localeItemName) != std::string::npos)
        {
            result = true;
            break;
        }
    }

    if (!result)
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked item (id: %u) name wasn't found in any localization", context, _item->GetId());

    return result;
}

QuestChatLink::QuestChatLink(): ChatLink(), _quest(nullptr), _questLevel(0), _minLevel(0), _maxLevel(0) { }

QuestChatLink::QuestChatLink(uint32 id) : ChatLink(), _quest(sQuestDataStore->GetQuestTemplate(id)), _questLevel(0), _minLevel(0), _maxLevel(0) { }

// |color|Hquest:quest_id:quest_level:min_level:max_level|h[name]|h|r
// |cff808080|Hquest:2278:47|h[The Platinum Discs]|h|r
bool QuestChatLink::Initialize(std::istringstream& iss)
{
    uint32 questId = 0;
    if (!ReadUInt32(iss, questId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading quest entry", iss.str().c_str());
        return false;
    }

    _quest = sQuestDataStore->GetQuestTemplate(questId);
    if (!_quest)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): quest template %u not found", iss.str().c_str(), questId);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "quest"))
        return false;

    if (!ReadInt32(iss, _questLevel))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading quest level", iss.str().c_str());
        return false;
    }

    if (_questLevel != _quest->Level)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): quest level %d is broken", iss.str().c_str(), _questLevel);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "quest"))
        return false;

    if (!ReadInt32(iss, _minLevel))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading quest level", iss.str().c_str());
        return false;
    }

    if (_minLevel != _quest->MinLevel)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): _minLevel %d is broken", iss.str().c_str(), _minLevel);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "quest"))
        return false;

    if (!ReadInt32(iss, _maxLevel))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading quest level", iss.str().c_str());
        return false;
    }

    if (_maxLevel != _quest->MaxScalingLevel)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): _maxLevel %d is broken", iss.str().c_str(), _maxLevel);
        return false;
    }
    return true;
}

bool QuestChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    auto res = _quest->LogTitle == buffer;
    if (!res)
        if (auto ql = sQuestDataStore->GetQuestLocale(_quest->GetQuestId()))
            for (const auto& v : ql->LogTitle)
                if (v == buffer)
                {
                    res = true;
                    break;
                }

    if (!res)
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked quest (id: %u) title wasn't found in any localization", context, _quest->GetQuestId());

    return res;
}

SpellChatLink::SpellChatLink(): ChatLink(), _spell(nullptr) { }

SpellChatLink::SpellChatLink(uint32 id) : ChatLink(), _spell(sSpellMgr->GetSpellInfo(id)) { }

// |color|Hspell:spell_id:rank|h[name]|h|r
// |cff71d5ff|Hspell:21563:rank|h[Command]|h|r
bool SpellChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_SPELL)
        return false;

    uint32 spellId = 0;
    if (!ReadUInt32(iss, spellId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading spell entry", iss.str().c_str());
        return false;
    }

    _spell = sSpellMgr->GetSpellInfo(spellId);
    if (!_spell)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid spell id %u in |spell command", iss.str().c_str(), spellId);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "spell"))
        return false;

    uint32 unk = 0;
    if (!ReadUInt32(iss, unk))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading spell unk", iss.str().c_str());
        return false;
    }

    return true;
}

bool SpellChatLink::ValidateName(char* buffer, const char* context)
{
    ChatLink::ValidateName(buffer, context);
    
    // spells with that flag have a prefix of "$PROFESSION: "
    if (_spell->HasAttribute(SPELL_ATTR0_TRADESPELL))
    {
        auto bounds = sSpellMgr->GetSkillLineAbilityMapBounds(_spell->Id);
        if (bounds.first == bounds.second)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): skill line not found for spell %u", context, _spell->Id);
            return false;
        }

        auto skillInfo = bounds.first->second;
        if (!skillInfo)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): skill line ability not found for spell %u", context, _spell->Id);
            return false;
        }

        auto skillLine = sSkillLineStore.LookupEntry(skillInfo->SkillLine);
        if (!skillLine)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): skill line not found for skill %u", context, skillInfo->SkillLine);
            return false;
        }

        auto spellEntry = sSpellStore.LookupEntry(_spell->Id);
        for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
        {
            if (locale == LOCALE_none)
                continue;

            std::stringstream ss;
            ss << skillLine->DisplayName->Str[locale] << ": " << spellEntry->Name->Get(locale);
            if (strcmp(ss.str().c_str(), buffer) == 0)
                return true;
        }
    }

    auto spellEntry = sSpellStore.LookupEntry(_spell->Id);
    for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
        if (locale == LOCALE_none)
            continue;

        if (strcmp(spellEntry->Name->Get(locale), buffer) == 0)
            return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked spell (id: %u) name wasn't found in any localization", context, _spell->Id);
    return false;
}

AchievementChatLink::AchievementChatLink(): ChatLink(), _guid(0), _achievement(nullptr)
{
    memset(_data, 0, sizeof(_data));
}

AchievementChatLink::AchievementChatLink(uint32 id) : ChatLink(), _guid(0),_achievement(sAchievementStore.LookupEntry(id)), _data{} { }

// |color|Hachievement:achievement_id:player_guid:0:0:0:0:0:0:0:0|h[name]|h|r
// |cffffff00|Hachievement:546:0000000000000001:0:0:0:-1:0:0:0:0|h[Safe Deposit]|h|r
bool AchievementChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_ACHIEVEMENT)
        return false;

    uint32 achievementId = 0;
    if (!ReadUInt32(iss, achievementId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading achievement entry", iss.str().c_str());
        return false;
    }

    _achievement = sAchievementStore.LookupEntry(achievementId);
    if (!_achievement)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid achivement id %u in |achievement command", iss.str().c_str(), achievementId);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "achievement"))
        return false;

    if (_achievement->Flags & ACHIEVEMENT_FLAG_GUILD)
    {
        if (!ValidateGuildGUID(iss, "achievement"))
            return false;
    }
    else
    {
        if (!ValidatePlayerGUID(iss, "achievement"))
            return false;
    }

    // temporary skip progress
    const uint8 propsCount = 8;
    for (uint8 index = 0; index < propsCount; ++index)
    {
        if (!CheckDelimiter(iss, DELIMITER, "achievement"))
            return false;

        if (!ReadUInt32(iss, _data[index]))
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading achievement property (%u)", iss.str().c_str(), index);
            return false;
        }
    }

    return true;
}

bool AchievementChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
        if (locale == LOCALE_none)
            continue;

        if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
            return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;
}

TradeChatLink::TradeChatLink(): SpellChatLink(), _minSkillLevel(0), _maxSkillLevel(0) { }

TradeChatLink::TradeChatLink(uint32 id) : SpellChatLink(id), _minSkillLevel(0), _maxSkillLevel(0) { }

// |color|Htrade:spell_id:cur_value:max_value:player_guid:base64_data|h[name]|h|r
// |cffffd000|Htrade:4037:1:150:1:6AAAAAAAAAAAAAAAAAAAAAAOAADAAAAAAAAAAAAAAAAIAAAAAAAAA|h[Engineering]|h|r
bool TradeChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_TRADE)
        return false;

    if (!ValidatePlayerGUID(iss, "trade"))
        return false;

    if (!CheckDelimiter(iss, DELIMITER, "trade"))
        return false;

    uint32 spellId = 0;
    if (!ReadUInt32(iss, spellId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading achievement entry", iss.str().c_str());
        return false;
    }

    _spell = sSpellMgr->GetSpellInfo(spellId);
    if (!_spell)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid spell id %u in |trade command", iss.str().c_str(), spellId);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "trade"))
        return false;

    uint32 skillID = 0;
    if (!ReadUInt32(iss, skillID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading trade skillID", iss.str().c_str());
        return false;
    }

    return true;
}

TalentChatLink::TalentChatLink(): SpellChatLink(), _talentId(0) { }

TalentChatLink::TalentChatLink(uint32 id) : SpellChatLink(), _talentId(0)
{
    if (auto talentInfo = sTalentStore.LookupEntry(id))
        _spell = sSpellMgr->GetSpellInfo(talentInfo->SpellID);
}

// |color|Htalent:talent_id|h[name]|h|r
// |cff71d5ff|Htalent:21901|h[Обстрел Скверны]|h|r
bool TalentChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_SPELL)
        return false;

    if (!ReadUInt32(iss, _talentId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading talent entry", iss.str().c_str());
        return false;
    }

    auto talentInfo = sTalentStore.LookupEntry(_talentId);
    if (!talentInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid talent id %u in |talent command", iss.str().c_str(), _talentId);
        return false;
    }

    _spell = sSpellMgr->GetSpellInfo(talentInfo->SpellID);
    if (!_spell)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid spell id %u in |trade command", iss.str().c_str(), talentInfo->SpellID);
        return false;
    }

    return true;
}

PvTalentChatLink::PvTalentChatLink() : SpellChatLink(), _talentId(0) { }

PvTalentChatLink::PvTalentChatLink(uint32 id) : SpellChatLink(), _talentId(0)
{
    if (auto talentInfo = sPvpTalentStore.LookupEntry(id))
        _spell = sSpellMgr->GetSpellInfo(talentInfo->SpellID);
}

//"|c%s|Hpvptal:%d|h[%s]|h%s"
// |cff71d5ff|Hpvptal:811|h[Удар с небес]|h|r
bool PvTalentChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_SPELL)
        return false;

    if (!ReadUInt32(iss, _talentId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading talent entry", iss.str().c_str());
        return false;
    }

    auto talentInfo = sPvpTalentStore.LookupEntry(_talentId);
    if (!talentInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid talent id %u in |talent command", iss.str().c_str(), _talentId);
        return false;
    }

    _spell = sSpellMgr->GetSpellInfo(talentInfo->SpellID);
    if (!_spell)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid spell id %u in |trade command", iss.str().c_str(), talentInfo->SpellID);
        return false;
    }

    return true;
}


// |color|Henchant:recipe_spell_id|h[prof_name: recipe_name]|h|r
// |cffffd000|Henchant:3919|h[Engineering: Rough Dynamite]|h|r
bool EnchantmentChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_ENCHANT)
        return false;

    uint32 spellId = 0;
    if (!ReadUInt32(iss, spellId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading enchantment spell entry", iss.str().c_str());
        return false;
    }

    _spell = sSpellMgr->GetSpellInfo(spellId);
    if (!_spell)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid spell id %u in |enchant command", iss.str().c_str(), spellId);
        return false;
    }

    return true;
}

CurrencyChatLink::CurrencyChatLink(): SpellChatLink(), _currencyId(0) { }

// |c%s|Hcurrency:%d|h[%s]|h%s"
// |cff0070dd|Hcurrency:1166|h[Искаженный временем знак]|h|r
bool CurrencyChatLink::Initialize(std::istringstream& iss)
{
    if (!ReadUInt32(iss, _currencyId))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading currency entry", iss.str().c_str());
        return false;
    }

    auto currencyInfo = sCurrencyTypesStore.LookupEntry(_currencyId);
    if (!currencyInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid talent id %u in |currency command", iss.str().c_str(), _currencyId);
        return false;
    }

    if (_color != ItemQualityColors[currencyInfo->Quality])
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked currency has color %u, but user claims %u", iss.str().c_str(), ItemQualityColors[currencyInfo->Quality], _color);
        return false;
    }

    return true;
}

bool CurrencyChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool BattlePetChatLink::Initialize(std::istringstream& iss)
{
    // need color quality of player's pet
    //if (_color != CHAT_LINK_COLOR_CURRENCY)
        //return false;

    bool goodColor = false;
    for (auto itemQualityColor : ItemQualityColors)
    {
        if (_color == itemQualityColor)
        {
            goodColor = true;
            break;
        }
    }

    if (!goodColor)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked item has unknown color %X", iss.str().c_str(), _color);
        return false;
    }

    uint32 speciesID = 0;
    if (!ReadUInt32(iss, speciesID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading battlepet species entry", iss.str().c_str());
        return false;
    }

    auto speciesInfo = sBattlePetSpeciesStore.LookupEntry(speciesID);
    if (!speciesInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid species id %u in |battlepet command", iss.str().c_str(), speciesID);
        return false;
    }

    // temporary skip battle pet data - need player pointer!
    const uint8 propsCount = 5;
    for (uint8 index = 0; index < propsCount; ++index)
    {
        if (!CheckDelimiter(iss, DELIMITER, "battlepet"))
            return false;

        if (!ReadUInt32(iss, _data[index]))
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading battlepet property (%u)", iss.str().c_str(), index);
            return false;
        }
    }

    if (!CheckDelimiter(iss, DELIMITER, "battlepet"))
        return false;

    // temp fix
    char c;
    iss.get(c);

    if (c == 'B')
        iss.ignore(23);
    else
        iss.ignore(15);

    // temporary skip "BattlePet" part - need read GUID as Battlepet-unk-unk1
    /*iss.ignore(9);

    if (!CheckDelimiter(iss, '-', "battlepet"))
        return false;

    uint32 unk = 0;
    if (!ReadUInt32(iss, unk))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading battlepet unk", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, '-', "battlepet"))
        return false;

    uint64 unk1 = 0;
    if (!ReadHex(iss, unk1, 12))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading battlepet unk1", iss.str().c_str());
        return false;
    }*/

    return true;
}

bool BattlePetChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
        if (locale == LOCALE_none)
            continue;

        if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
            return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool GarrFollowerChatLink::Initialize(std::istringstream& iss)
{
    // need color quality of garr follower!
    //if (_color != CHAT_LINK_COLOR_CURRENCY)
    //return false;

    bool goodColor = false;
    for (auto itemQualityColor : ItemQualityColors)
    {
        if (_color == itemQualityColor)
        {
            goodColor = true;
            break;
        }
    }

    if (!goodColor)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked item has unknown color %X", iss.str().c_str(), _color);
        return false;
    }

    uint32 followerID = 0;
    if (!ReadUInt32(iss, followerID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading garrfollower entry", iss.str().c_str());
        return false;
    }

    // temporary skip garr follower data - need player pointer!
    const uint8 propsCount = 12;
    for (uint8 index = 0; index < propsCount; ++index)
    {
        if (!CheckDelimiter(iss, DELIMITER, "garrfollower"))
            return false;

        if (!ReadUInt32(iss, _data[index]))
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading garrfollower property (%u)", iss.str().c_str(), index);
            return false;
        }
    }

    return true;
}

bool GarrFollowerChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool GarrFollowerAbilityChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_GARRFOLLOWERABILITY)
        return false;

    uint32 abilityID = 0;
    if (!ReadUInt32(iss, abilityID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading garrfollowerability entry", iss.str().c_str());
        return false;
    }

    auto abilityInfo = sGarrAbilityStore.LookupEntry(abilityID);
    if (!abilityInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid ability id %u in |garrfollowerability command", iss.str().c_str(), abilityID);
        return false;
    }

    return true;
}

bool GarrFollowerAbilityChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool GarrMissionChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_ACHIEVEMENT)
        return false;

    uint32 missionID = 0;
    if (!ReadUInt32(iss, missionID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading garrmission entry", iss.str().c_str());
        return false;
    }

    auto missionInfo = sGarrMissionStore.LookupEntry(missionID);
    if (!missionInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid ability id %u in |garrmission command", iss.str().c_str(), missionID);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "garrmission"))
        return false;

    uint64 DbID = 0;
    if (!ReadUInt64(iss, DbID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading garrmission DbID", iss.str().c_str());
        return false;
    }

    return true;
}

bool GarrMissionChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool InstanceLockChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_INSTANCE_LOCK)
        return false;

    if (!ValidatePlayerGUID(iss, "instancelock"))
        return false;

    if (!CheckDelimiter(iss, DELIMITER, "instancelock"))
        return false;

    uint32 mapID = 0;
    if (!ReadUInt32(iss, mapID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading instancelock mapID", iss.str().c_str());
        return false;
    }

    auto mapEntry = sMapStore.LookupEntry(mapID);
    if (!mapEntry)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid map id %u in |instancelock command", iss.str().c_str(), mapID);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "instancelock"))
        return false;

    uint32 difficultyID = 0;
    if (!ReadUInt32(iss, difficultyID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading instancelock difficultyID", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "instancelock"))
        return false;

    uint32 completedMask = 0;
    if (!ReadUInt32(iss, completedMask))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading instancelock completedMask", iss.str().c_str());
        return false;
    }

    return true;
}

bool InstanceLockChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool KeystoneChatLink::Initialize(std::istringstream& iss)
{
    if (_color != 0xffa335ee)
        return false;

    uint32 challengeID = 0;
    if (!ReadUInt32(iss, challengeID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading keystone challengeID", iss.str().c_str());
        return false;
    }

    auto challengeEntry = sMapChallengeModeStore.LookupEntry(challengeID);
    if (!challengeEntry)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid challengeID %u in |keystone command", iss.str().c_str(), challengeID);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "keystone"))
        return false;

    uint32 challengeLevel = 0;
    if (!ReadUInt32(iss, challengeLevel))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading keystone challengeLevel", iss.str().c_str());
        return false;
    }

    if (challengeLevel > 30)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got too big challengeLevel %u in |keystone command", iss.str().c_str(), challengeLevel);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "keystone"))
        return false;

    uint32 affixes[3] = { 0, 0, 0 };
    if (!ReadUInt32(iss, affixes[0]))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading keystone affix1", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "keystone"))
        return false;

    if (!ReadUInt32(iss, affixes[1]))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading keystone affix2", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "keystone"))
        return false;

    if (!ReadUInt32(iss, affixes[2]))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading keystone affix3", iss.str().c_str());
        return false;
    }

    // validate affixes
    if (affixes[0] > 14 || affixes[1] > 14 || affixes[2] > 14)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got too big affixes ID %u %u %u in |keystone command", iss.str().c_str(), affixes[0], affixes[1], affixes[2]);
        return false;
    }

    if (challengeLevel >= 4)
    {
        // check affix1
        if (affixes[0] != 7 && affixes[0] != 6 && affixes[0] != 8 && affixes[0] != 5 && affixes[0] != 11)
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid affix1 %u in |keystone command", iss.str().c_str(), affixes[0]);
            return false;
        }

        if (challengeLevel >= 7)
        {
            // check affix2
            if (affixes[1] != 4 && affixes[1] != 2 && affixes[1] != 3 && affixes[1] != 13 && affixes[1] != 14 && affixes[1] != 12)
            {
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid affix2 %u in |keystone command", iss.str().c_str(), affixes[1]);
                return false;
            }

            if (challengeLevel >= 10)
            {
                // check affix3
                if (affixes[2] != 9 && affixes[2] != 10)
                {
                    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid affix3 %u in |keystone command", iss.str().c_str(), affixes[2]);
                    return false;
                }
            }
            else
            {
                // check other affixes on 0
                if (affixes[2])
                {
                    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): challenge level %u isn't matched of affixes %u %u %u in |keystone command", iss.str().c_str(), challengeLevel, affixes[0], affixes[1], affixes[2]);
                    return false;
                }
            }
        }
        else
        {
            // check other affixes on 0
            if (affixes[1] || affixes[2])
            {
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): challenge level %u isn't matched of affixes %u %u %u in |keystone command", iss.str().c_str(), challengeLevel, affixes[0], affixes[1], affixes[2]);
                return false;
            }
        }
    }
    else
    {
        // check other affixes on 0
        if (affixes[0] || affixes[1] || affixes[2])
        {
            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): challenge level %u isn't matched of affixes %u %u %u in |keystone command", iss.str().c_str(), challengeLevel, affixes[0], affixes[1], affixes[2]);
            return false;
        }
    }

    return true;
}

bool KeystoneChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool TransmogIllusionChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_TRANSMOG_ILLUSION)
        return false;

    uint32 enchantID = 0;
    if (!ReadUInt32(iss, enchantID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading transmogillusion enchantID", iss.str().c_str());
        return false;
    }

    return true;
}

bool TransmogIllusionChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool TransmogAppearanceChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_TRANSMOG_ILLUSION)
        return false;

    uint32 appearanceID = 0;
    if (!ReadUInt32(iss, appearanceID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading transmogappearance entry", iss.str().c_str());
        return false;
    }

    return true;
}

bool TransmogAppearanceChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool JournalChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_GLYPH)
        return false;

    uint32 typeID = 0;
    if (!ReadUInt32(iss, typeID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading journal typeID", iss.str().c_str());
        return false;
    }

    // Instance ID  - 0
    // Encounter ID - 1
    // Section ID   - 2
    if (typeID > 2)
        return false;

    if (!CheckDelimiter(iss, DELIMITER, "journal"))
        return false;

    // depends on journal typeID
    uint32 entryID = 0;
    if (!ReadUInt32(iss, entryID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading journal entryID", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "journal"))
        return false;

    uint32 difficultyMask = 0;
    if (!ReadUInt32(iss, difficultyMask))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading journal difficultyMask", iss.str().c_str());
        return false;
    }

    return true;
}

bool JournalChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}

bool ArtifactPowerChatLink::Initialize(std::istringstream& iss)
{
    if (_color != CHAT_LINK_COLOR_SPELL)
        return false;

    uint32 powerID = 0;
    if (!ReadUInt32(iss, powerID))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading apower powerID", iss.str().c_str());
        return false;
    }

    auto artifactPowerEntry = sArtifactPowerStore.LookupEntry(powerID);
    if (!artifactPowerEntry)
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): got invalid powerID %u in |apower command", iss.str().c_str(), powerID);
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "apower"))
        return false;

    uint32 unkRankValue = 0;
    if (!ReadUInt32(iss, unkRankValue))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading apower unkRankValue", iss.str().c_str());
        return false;
    }

    if (!CheckDelimiter(iss, DELIMITER, "apower"))
        return false;

    uint32 unkTierValue = 0;
    if (!ReadUInt32(iss, unkTierValue))
    {
        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): sequence finished unexpectedly while reading apower unkTierValue", iss.str().c_str());
        return false;
    }

    return true;
}

bool ArtifactPowerChatLink::ValidateName(char* buffer, char const* context)
{
    ChatLink::ValidateName(buffer, context);

    /*for (uint8 locale = LOCALE_enUS; locale < MAX_LOCALES; ++locale)
    {
    if (locale == LOCALE_none)
    continue;

    if (strcmp(_achievement->Title->Get(locale), buffer) == 0)
    return true;
    }

    TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "ChatHandler::isValidChatMessage('%s'): linked achievement (id: %u) name wasn't found in any localization", context, _achievement->ID);
    return false;*/
    return true;
}
