/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "ChatPackets.h"
#include "Group.h"
#include "Player.h"
#include "World.h"
#include "Creature.h"
#include "Channel.h"
#include "CreatureTextMgr.h"

void WorldPackets::Chat::ChatMessage::Read()
{
    _worldPacket >> Language;
    Text = _worldPacket.ReadString(_worldPacket.ReadBits(9));
}

void WorldPackets::Chat::ChatMessageWhisper::Read()
{
    _worldPacket >> Language;
    uint32 targetLen = _worldPacket.ReadBits(9);
    uint32 textLen = _worldPacket.ReadBits(9);
    Target = _worldPacket.ReadString(targetLen);
    Text = _worldPacket.ReadString(textLen);
}

void WorldPackets::Chat::ChatMessageChannel::Read()
{
    _worldPacket >> Language;
    uint32 targetLen = _worldPacket.ReadBits(9);
    uint32 textLen = _worldPacket.ReadBits(9);
    Target = _worldPacket.ReadString(targetLen);
    Text = _worldPacket.ReadString(textLen);
}

void WorldPackets::Chat::ChatAddonMessage::Read()
{
    uint32 prefixLen = _worldPacket.ReadBits(5);
    uint32 textLen = _worldPacket.ReadBits(9);
    Prefix = _worldPacket.ReadString(prefixLen);
    Text = _worldPacket.ReadString(textLen, false);
}

void WorldPackets::Chat::ChatAddonMessageWhisper::Read()
{
    uint32 targetLen = _worldPacket.ReadBits(9);
    uint32 prefixLen = _worldPacket.ReadBits(5);
    uint32 textLen = _worldPacket.ReadBits(9);
    Target = _worldPacket.ReadString(targetLen);
    Prefix = _worldPacket.ReadString(prefixLen);
    Text = _worldPacket.ReadString(textLen, false);
}

void WorldPackets::Chat::ChatAddonMessageChannel::Read()
{
    uint32 targetLen = _worldPacket.ReadBits(9);
    uint32 prefixLen = _worldPacket.ReadBits(5);
    uint32 textLen = _worldPacket.ReadBits(9);
    Target = _worldPacket.ReadString(targetLen);
    Prefix = _worldPacket.ReadString(prefixLen);
    Text = _worldPacket.ReadString(textLen, false);
}

void WorldPackets::Chat::ChatMessageDND::Read()
{
    Text = _worldPacket.ReadString(_worldPacket.ReadBits(9));
}

void WorldPackets::Chat::ChatMessageAFK::Read()
{
    Text = _worldPacket.ReadString(_worldPacket.ReadBits(9));
}

void WorldPackets::Chat::ChatMessageEmote::Read()
{
    Text = _worldPacket.ReadString(_worldPacket.ReadBits(9));
}

WorldPackets::Chat::Chat::Chat(Chat const& chat) : ServerPacket(SMSG_CHAT, chat._worldPacket.size()),
SlashCmd(chat.SlashCmd), _Language(chat._Language), SenderGUID(chat.SenderGUID),
SenderGuildGUID(chat.SenderGuildGUID), SenderAccountGUID(chat.SenderAccountGUID), TargetGUID(chat.TargetGUID), PartyGUID(chat.PartyGUID),
SenderVirtualAddress(chat.SenderVirtualAddress), TargetVirtualAddress(chat.TargetVirtualAddress), SenderName(chat.SenderName), TargetName(chat.TargetName),
Prefix(chat.Prefix), _Channel(chat._Channel), ChatText(chat.ChatText), AchievementID(chat.AchievementID), _ChatFlags(chat._ChatFlags),
DisplayTime(chat.DisplayTime), HideChatLog(chat.HideChatLog), FakeSenderName(chat.FakeSenderName)
{
}

void ModifyMessageForPlayer(WorldObject const* sender, std::string& message, ChatMsg chatType)
{
    if (chatType == CHAT_MSG_SAY  || chatType == CHAT_MSG_PARTY ||
        chatType == CHAT_MSG_RAID || chatType == CHAT_MSG_GUILD ||
        chatType == CHAT_MSG_YELL || chatType == CHAT_MSG_WHISPER || chatType == CHAT_MSG_CHANNEL)
        {
            if (Player const* playerSender = sender->ToPlayer())
            {
                std::string const& logo = playerSender->getSelectedChatLogo();
                if (logo.empty())
                    return;
                
                message = logo + message;
            }
        }
}

void WorldPackets::Chat::Chat::Initialize(ChatMsg chatType, Language language, WorldObject const* sender, WorldObject const* receiver, std::string message,
    uint32 achievementId /*= 0*/, std::string channelName /*= ""*/, LocaleConstant locale /*= DEFAULT_LOCALE*/, std::string addonPrefix /*= ""*/)
{
    // Clear everything because same packet can be used multiple times
    Clear();

    SenderGUID.Clear();
    SenderAccountGUID.Clear();
    SenderGuildGUID.Clear();
    PartyGUID.Clear();
    TargetGUID.Clear();
    SenderName.clear();
    TargetName.clear();
    _ChatFlags = CHAT_TAG_NONE;

    SlashCmd = chatType;
    _Language = language;

    if (sender)
        SetSender(sender, locale);

    if (receiver)
        SetReceiver(receiver, locale);

    SenderVirtualAddress = GetVirtualRealmAddress();
    TargetVirtualAddress = GetVirtualRealmAddress();
    AchievementID = achievementId;
    _Channel = std::move(channelName);
    Prefix = std::move(addonPrefix);
    
    if (sender)
        ModifyMessageForPlayer(sender, message, chatType); //custom
    
    ChatText = std::move(message);
    // ChatTextCode = CodeChatMessage(ChatText, _Language);
}

uint32 GetWordWeight(std::string const& word)
{
    uint32 weight = 0;
    for (size_t i = 0; i < word.size(); ++i)
        weight += static_cast<uint8>(word[i]);
    return weight;
}

bool isCaps(std::wstring wstr)
{
    if (wstr.empty())
        return false;

    uint32 upperCount = 0;
    for (size_t i = 0; i < wstr.size(); ++i)
        if (std::iswupper(wstr[i]))
            ++upperCount;

    return upperCount * 2 >= wstr.size();
}

std::string WorldPackets::Chat::Chat::CodeChatMessage(std::string text, uint32 lang_id)
{
    auto const* wordMap = sDB2Manager.GetLanguageWordMap(lang_id);
    if (!wordMap)
        return "";

    std::string convertedMessage;

    Tokenizer t(text, ' ');
    for (size_t i = 0; i < t.size(); ++i)
    {
        std::string word = t[i];
        std::wstring wword;
        if (!Utf8toWStr(word, wword))
            continue;

        if (wword.empty())
            continue;

        if (StringVector const* wordVector = sDB2Manager.GetLanguageWordsBySize(lang_id, wword.size()))
        {
            std::string replacer = (*wordVector)[GetWordWeight(t[i]) % wordVector->size()];
            if (isCaps(wword))
                std::transform(replacer.begin(), replacer.end(), replacer.begin(), toupper);

            convertedMessage += replacer + " ";
        }
    }

    return convertedMessage;
}

void WorldPackets::Chat::Chat::SetSender(WorldObject const* sender, LocaleConstant locale)
{
    SenderGUID = sender->GetGUID();

    if (Creature const* creatureSender = sender->ToCreature())
        SenderName = creatureSender->GetNameForLocaleIdx(locale);

    if (Player const* playerSender = sender->ToPlayer())
    {
        SenderAccountGUID = playerSender->GetSession()->GetAccountGUID();
        _ChatFlags = playerSender->GetChatTag();

        SenderGuildGUID = playerSender->GetGuildId() ? ObjectGuid::Create<HighGuid::Guild>(playerSender->GetGuildId()) : ObjectGuid::Empty;

        if (Group const* group = playerSender->GetGroup())
            PartyGUID = group->GetGUID();
    }
}

void WorldPackets::Chat::Chat::SetReceiver(WorldObject const* receiver, LocaleConstant locale)
{
    if (!receiver)
        return;

    TargetGUID = receiver->GetGUID();
    if (Creature const* creatureReceiver = receiver->ToCreature())
        TargetName = creatureReceiver->GetNameForLocaleIdx(locale);
}

WorldPacket const* WorldPackets::Chat::Chat::Write()
{
    _worldPacket << SlashCmd;
    _worldPacket << _Language;
    _worldPacket << SenderGUID;
    _worldPacket << SenderGuildGUID;
    _worldPacket << SenderAccountGUID;
    _worldPacket << TargetGUID;
    _worldPacket << TargetVirtualAddress;
    _worldPacket << SenderVirtualAddress;
    _worldPacket << PartyGUID;
    _worldPacket << AchievementID;
    _worldPacket << DisplayTime;
    _worldPacket.WriteBits(SenderName.length(), 11);
    _worldPacket.WriteBits(TargetName.length(), 11);
    _worldPacket.WriteBits(Prefix.length(), 5);
    _worldPacket.WriteBits(_Channel.length(), 7);
    _worldPacket.WriteBits(ChatText.length(), 12);
    _worldPacket.WriteBits(_ChatFlags, 11);
    _worldPacket.WriteBit(HideChatLog);
    _worldPacket.WriteBit(FakeSenderName);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(SenderName);
    _worldPacket.WriteString(TargetName);
    _worldPacket.WriteString(Prefix);
    _worldPacket.WriteString(_Channel);
    _worldPacket.WriteString(ChatText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::Emote::Write()
{
    _worldPacket << Guid;
    _worldPacket << EmoteID;

    return &_worldPacket;
}

void WorldPackets::Chat::CTextEmote::Read()
{
    _worldPacket >> Target;
    _worldPacket >> EmoteID;
    _worldPacket >> SoundIndex;
}

WorldPacket const* WorldPackets::Chat::STextEmote::Write()
{
    _worldPacket << SourceGUID;
    _worldPacket << SourceAccountGUID;
    _worldPacket << EmoteID;
    _worldPacket << SoundIndex;
    _worldPacket << TargetGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::PrintNotification::Write()
{
    _worldPacket.WriteBits(NotifyText.size(), 12);
    _worldPacket.WriteString(NotifyText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::ChatPlayerNotfound::Write()
{
    _worldPacket.WriteBits(Name.length(), 9);
    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::ChatServerMessage::Write()
{
    _worldPacket << MessageID;

    _worldPacket.WriteBits(StringParam.length(), 11);
    _worldPacket.WriteString(StringParam);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::DefenseMessage::Write()
{
    _worldPacket << int32(ZoneID);
    _worldPacket.WriteBits(MessageText.length(), 12);
    _worldPacket.WriteString(MessageText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::WorldText::Write()
{
    _worldPacket << Guid;
    _worldPacket << Arg1;
    _worldPacket << Arg2;
    _worldPacket.WriteBits(Text.length(), 12);
    _worldPacket.WriteString(Text);

    return &_worldPacket;
}

void WorldPackets::Chat::ChatRegisterAddonPrefixes::Read()
{
    Prefixes.resize(_worldPacket.read<uint32>());
    for (std::string& prefix : Prefixes)
        prefix.assign(_worldPacket.ReadString(_worldPacket.ReadBits(5)));
}

void WorldPackets::Chat::ChatReportIgnored::Read()
{
    _worldPacket >> IgnoredGUID;
    _worldPacket >> Reason;
}

WorldPacket const* WorldPackets::Chat::ChatPlayerAmbiguous::Write()
{
    _worldPacket.WriteString(Name, 9);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::ChatRestricted::Write()
{
    _worldPacket << Reason;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Chat::ChatNotInParty::Write()
{
    _worldPacket << SlashCmd;

    return &_worldPacket;
}
