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

#ifndef __CHATTEXT_BUILDER_H
#define __CHATTEXT_BUILDER_H

#include "Chat.h"
#include "CreatureTextMgr.h"

namespace Trinity
{
class BroadcastTextBuilder
{
public:
    BroadcastTextBuilder(Unit const* obj, ChatMsg msgType, uint32 textId, WorldObject const* target = nullptr, uint32 achievementId = 0);
    WorldPackets::Packet* operator()(LocaleConstant locale) const;

private:
    Unit const* _source;
    WorldObject const* _target;
    ChatMsg _msgType;
    uint32 _textId;
    uint32 _achievementId;
};

class CustomChatTextBuilder
{
public:
    CustomChatTextBuilder(WorldObject const* obj, ChatMsg msgType, std::string const& text, Language language = LANG_UNIVERSAL, WorldObject const* target = nullptr);
    WorldPackets::Packet* operator()(LocaleConstant locale) const;

private:
    WorldObject const* _source;
    WorldObject const* _target;
    ChatMsg _msgType;
    Language _language;
    std::string _text;
};

class TrinityStringChatBuilder
{
public:
    TrinityStringChatBuilder(WorldObject const* obj, ChatMsg msgType, uint32 textId, WorldObject const* target = nullptr, va_list* args = nullptr);
    WorldPackets::Packet* operator()(LocaleConstant locale) const;

private:
    WorldObject const* _source;
    WorldObject const* _target;
    va_list* _args;
    ChatMsg _msgType;
    uint32 _textId;
};
}

#endif // __CHATTEXT_BUILDER_H
