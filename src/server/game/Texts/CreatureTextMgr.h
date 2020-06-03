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

#ifndef TRINITY_CREATURE_TEXT_MGR_H
#define TRINITY_CREATURE_TEXT_MGR_H

#include "GridNotifiers.h"
#include "SharedDefines.h"
#include "Opcodes.h"

struct CreatureTextEntry
{
    uint32 entry;
    uint32 duration;
    uint32 sound;
    uint32 BroadcastTextID;
    uint32 MinTimer;
    uint32 MaxTimer;
    uint32 SpellID;
    float probability;
    ChatMsg type;
    Language lang;
    Emote emote;
    std::string text;
    uint8 group;
    uint8 id;
};

enum TextRange
{
    TEXT_RANGE_NORMAL   = 0,
    TEXT_RANGE_AREA     = 1,
    TEXT_RANGE_ZONE     = 2,
    TEXT_RANGE_MAP      = 3,
    TEXT_RANGE_WORLD    = 4
};

enum text
{
    TEXT_GENERIC_0                                 = 0,
    TEXT_GENERIC_1                                 = 1,
    TEXT_GENERIC_2                                 = 2,
    TEXT_GENERIC_3                                 = 3,
    TEXT_GENERIC_4                                 = 4,
    TEXT_GENERIC_5                                 = 5,
    TEXT_GENERIC_6                                 = 6,
    TEXT_GENERIC_7                                 = 7,
    TEXT_GENERIC_8                                 = 8,
    TEXT_GENERIC_9                                 = 9,
    TEXT_GENERIC_10                                = 10,
    TEXT_GENERIC_11                                = 11,
    TEXT_GENERIC_12                                = 12,
    TEXT_GENERIC_13                                = 13,
    TEXT_GENERIC_14                                = 14,
    TEXT_GENERIC_15                                = 15,
    TEXT_GENERIC_16                                = 16,
};

enum TextGroup
{
    TEXT_GROUP_DEFAULT         = 0,
    TEXT_GROUP_COMBAT          = 1,
    TEXT_GROUP_DIE             = 2,
    TEXT_GROUP_TIMER           = 3,
    TEXT_GROUP_COMBAT_TIMER    = 4
};

struct CreatureTextId
{
    CreatureTextId(uint32 e, uint32 g, uint32 i);

    bool operator<(CreatureTextId const& right) const;

    uint32 entry;
    uint32 textGroup;
    uint32 textId;
};

typedef std::vector<CreatureTextEntry> CreatureTextGroup;              //texts in a group
typedef std::unordered_map<uint8, CreatureTextGroup> CreatureTextHolder;    //groups for a creature by groupid
typedef std::unordered_map<uint32, CreatureTextHolder> CreatureTextMap;     //all creatures by entry
typedef std::vector<CreatureTextHolder*> CreatureTextList;     //all creatures by entry

//used for handling non-repeatable random texts
typedef std::vector<uint8> CreatureTextRepeatIds;
typedef std::map<uint8, CreatureTextRepeatIds> CreatureTextRepeatGroup;
typedef std::map<ObjectGuid::LowType, CreatureTextRepeatGroup> CreatureTextRepeatMap;//guid based

class CreatureTextMgr
{
    CreatureTextMgr() {};
    ~CreatureTextMgr() {};

    public:
        static CreatureTextMgr* instance()
        {
            static CreatureTextMgr instance;
            return &instance;
        }

        void LoadCreatureTexts();
        CreatureTextMap  const& GetTextMap() const { return mTextMap; }

        void SendSound(Creature* source, uint32 sound, ChatMsg msgType, ObjectGuid whisperGuid, TextRange range, Team team, bool gmOnly);
        void SendEmote(Unit* source, uint32 emote);

        //if sent, returns the 'duration' of the text else 0 if error
        uint32 SendChat(Creature* source, uint8 textGroup, ObjectGuid whisperGuid = ObjectGuid::Empty, ChatMsg msgType = CHAT_MSG_ADDON, Language language = LANG_ADDON, TextRange range = TEXT_RANGE_NORMAL, uint32 sound = 0, Team team = TEAM_OTHER, bool gmOnly = false, Player* srcPlr = nullptr, bool ingoreProbality = false);
        void SendText(Creature* source, CreatureTextEntry const* text, ObjectGuid whisperGuid = ObjectGuid::Empty, ChatMsg msgType = CHAT_MSG_ADDON, Language language = LANG_ADDON, TextRange range = TEXT_RANGE_NORMAL, uint32 sound = 0, Team team = TEAM_OTHER, bool gmOnly = false, Player* srcPlr = nullptr);
        bool TextExist(uint32 sourceEntry, uint8 textGroup);
        CreatureTextGroup const* GetTextGroup(uint32 sourceEntry, uint8 textGroup);

        template<class Builder>
        static void SendChatPacket(WorldObject* source, Builder const& builder, ChatMsg msgType, WorldObject const* whisperTarget = nullptr, TextRange range = TEXT_RANGE_NORMAL, Team team = TEAM_OTHER, bool gmOnly = false);
        void SendNonChatPacket(WorldObject* source, WorldPacket const* data, ChatMsg msgType, ObjectGuid whisperGuid, TextRange range, Team team, bool gmOnly) const;

        bool HasBroadCastText(uint32 entry, uint32 BroadcastTextID, uint8& groupId);
        CreatureTextEntry const* FindSpellInText(uint32 entry, uint32 SpellID);
        bool IsDuplicateText(uint32 entry, const std::string& text);

        static float GetRangeForChatType(ChatMsg msgType);

    private:
        CreatureTextRepeatIds GetRepeatGroup(Creature* source, uint8 textGroup);
        void SetRepeatId(Creature* source, uint8 textGroup, uint8 id);

        CreatureTextMap mTextMap;
        CreatureTextList mTextList;
        CreatureTextRepeatMap mTextRepeatMap;
        sf::contention_free_shared_mutex< > i_lockTextRepeat;
};

#define sCreatureTextMgr CreatureTextMgr::instance()

#endif
