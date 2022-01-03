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

#include "AccountMgr.h"
#include "ChatPackets.h"
#include "ScriptMgr.h"
#include "GlobalFunctional.h"
#include "SpellAuraEffects.h"
#include "WordFilterMgr.h"
#include "Chat.h"
#include "GuildMgr.h"
#include "Group.h"
#include "ChannelMgr.h"
#include "CreatureAI.h"

bool WorldSession::processChatmessageFurtherAfterSecurityChecks(std::string& msg, uint32 lang)
{
    if (lang != LANG_ADDON)
    {
        // strip invisible characters for non-addon messages
        if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
            stripLineInvisibleChars(msg);

        if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) && !ChatHandler(this).isValidChatMessage(msg.c_str()))
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Player %s (GUID: %u) sent a chatmessage with an invalid link: %s", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow(), msg.c_str());
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            else
                ChatHandler(this).PSendSysMessage("You can't use bad link in you message");
            return false;
        }
    }

    return true;
}

inline bool isNasty(uint8 c)
{
    if (c == '\t')
        return false;
    if (c <= '\037') // ASCII control block
        return true;
    return false;
}

inline bool ValidateMessage(Player const* player, std::string& msg)
{
    // cut at the first newline or carriage return
    std::string::size_type pos = msg.find_first_of("\n\r");
    if (pos == 0)
        return false;
    else if (pos != std::string::npos)
        msg.erase(pos);

    // abort on any sort of nasty character
    for (uint8 c : msg)
    {
        if (isNasty(c))
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Player %s %s sent a message containing invalid character %u - blocked", player->GetName(), player->GetGUID().ToString().c_str(), uint32(c));
            return false;
        }
    }

    return true;
}

void WorldSession::HandleChatMessageAFK(WorldPackets::Chat::ChatMessageAFK& chatMessageAFK)
{
    Player* sender = GetPlayer();
    if (!sender)
        return;

    if (sender->isInCombat())
        return;

    // do message validity checks
    if (!ValidateMessage(sender, chatMessageAFK.Text))
        return;

    if (!processChatmessageFurtherAfterSecurityChecks(chatMessageAFK.Text, LANG_COMMON))
        return;

    if (sender->HasAura(1852))
    {
        SendNotification(GetTrinityString(LANG_GM_SILENCE), sender->GetName());
        return;
    }

    if (sender->isAFK())
    {
        if (chatMessageAFK.Text.empty())
            sender->ToggleAFK();
        else
            sender->afkMsg = chatMessageAFK.Text;
    }
    else
    {
        sender->afkMsg = chatMessageAFK.Text.empty() ? GetTrinityString(LANG_PLAYER_AFK_DEFAULT) : chatMessageAFK.Text;

        if (sender->isDND())
            sender->ToggleDND();

        sender->ToggleAFK();
    }

    sScriptMgr->OnPlayerChat(sender, CHAT_MSG_AFK, LANG_UNIVERSAL, chatMessageAFK.Text);
}

void WorldSession::HandleChatMessageDND(WorldPackets::Chat::ChatMessageDND& chatMessageDND)
{
    Player* sender = GetPlayer();

    if (sender->isInCombat())
        return;

    // do message validity checks
    if (!ValidateMessage(sender, chatMessageDND.Text))
        return;

    if (!processChatmessageFurtherAfterSecurityChecks(chatMessageDND.Text, LANG_COMMON))
        return;

    if (sender->HasAura(1852))
    {
        SendNotification(GetTrinityString(LANG_GM_SILENCE), sender->GetName());
        return;
    }

    if (sender->isDND())
    {
        if (chatMessageDND.Text.empty())
            sender->ToggleDND();
        else
            sender->dndMsg = chatMessageDND.Text;
    }
    else
    {
        sender->dndMsg = chatMessageDND.Text.empty() ? GetTrinityString(LANG_PLAYER_DND_DEFAULT) : chatMessageDND.Text;

        if (sender->isAFK())
            sender->ToggleAFK();

        sender->ToggleDND();
    }

    sScriptMgr->OnPlayerChat(sender, CHAT_MSG_DND, LANG_UNIVERSAL, chatMessageDND.Text);
}

void WorldSession::HandleChatMessageOpcode(WorldPackets::Chat::ChatMessage& packet)
{
    ChatMsg type;
    OpcodeClient opcode = packet.GetOpcode();

    // additional bot check
    if (opcode == CMSG_CHAT_MESSAGE_GUILD && packet.Language == 0x4321DEAD)
    {
        sLog->outWarden("%s additional bot check: %s", GetPlayerName(false).c_str(), packet.Text.c_str());
        return;
    }

    switch (opcode)
    {
        case CMSG_CHAT_MESSAGE_SAY:
            type = CHAT_MSG_SAY;
            break;
        case CMSG_CHAT_MESSAGE_YELL:
            type = CHAT_MSG_YELL;
            break;
        case CMSG_CHAT_MESSAGE_CHANNEL:
            type = CHAT_MSG_CHANNEL;
            break;
        case CMSG_CHAT_MESSAGE_WHISPER:
            type = CHAT_MSG_WHISPER;
            break;
        case CMSG_CHAT_MESSAGE_GUILD:
            type = CHAT_MSG_GUILD;
            break;
        case CMSG_CHAT_MESSAGE_OFFICER:
            type = CHAT_MSG_OFFICER;
            break;
        case CMSG_CHAT_MESSAGE_DND:
            type = CHAT_MSG_DND;
            break;
        case CMSG_CHAT_MESSAGE_EMOTE:
            type = CHAT_MSG_EMOTE;
            break;
        case CMSG_CHAT_MESSAGE_PARTY:
            type = CHAT_MSG_PARTY;
            break;
        case CMSG_CHAT_MESSAGE_RAID:
            type = CHAT_MSG_RAID;
            break;
        case CMSG_CHAT_MESSAGE_RAID_WARNING:
            type = CHAT_MSG_RAID_WARNING;
            break;
        case CMSG_CHAT_MESSAGE_INSTANCE_CHAT:
            type = CHAT_MSG_INSTANCE_CHAT;
            break;
        default:
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandleMessagechatOpcode : Unknown chat opcode (%u)", packet.GetOpcode());
            return;
    }

    HandleChatMessage(type, packet.Language, packet.Text);
}

void WorldSession::HandleChatMessageWhisperOpcode(WorldPackets::Chat::ChatMessageWhisper& packet)
{
    HandleChatMessage(CHAT_MSG_WHISPER, packet.Language, packet.Text, packet.Target);
}

void WorldSession::HandleChatMessageChannelOpcode(WorldPackets::Chat::ChatMessageChannel& packet)
{
    HandleChatMessage(CHAT_MSG_CHANNEL, packet.Language, packet.Text, packet.Target);
}

void WorldSession::HandleChatMessageEmoteOpcode(WorldPackets::Chat::ChatMessageEmote& packet)
{
    HandleChatMessage(CHAT_MSG_EMOTE, LANG_UNIVERSAL, packet.Text);
}

void WorldSession::HandleChatMessage(ChatMsg type, uint32 lang, std::string msg, std::string target)
{
    if (type >= MAX_CHAT_MSG_TYPE)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "CHAT: Wrong message type received: %u", type);
        return;
    }

    Player* sender = GetPlayer();

    //TC_LOG_DEBUG(LOG_FILTER_GENERAL, "CHAT: packet received. type %u, lang %u", type, lang);

    if (!sender->CanSpeak() && type != CHAT_MSG_DND)
    {
        std::string timeStr = secsToTimeString(m_muteTime - time(nullptr));
        SendNotification(GetTrinityString(LANG_WAIT_BEFORE_SPEAKING), timeStr.c_str());
        return;
    }

    // no language sent with emote packet.
    if (type != CHAT_MSG_EMOTE && type != CHAT_MSG_DND)
    {
        // prevent talking at unknown language (cheating)
        LanguageDesc const* langDesc = GetLanguageDescByID(lang);
        if (!langDesc)
        {
            SendNotification(LANG_UNKNOWN_LANGUAGE);
            return;
        }
        if (!sender->CanSpeakLanguage(lang))
        {
            SendNotification(LANG_NOT_LEARNED_LANGUAGE);
            return;
        }

        if (lang == LANG_ADDON)
        {
            if (sWorld->getBoolConfig(CONFIG_CHATLOG_ADDON))
            {
                if (msg.empty())
                    return;

                sScriptMgr->OnPlayerChat(sender, uint32(CHAT_MSG_ADDON), lang, msg);
            }

            // Disabled addon channel?
            if (!sWorld->getBoolConfig(CONFIG_ADDON_CHANNEL))
                return;
        }
        // LANG_ADDON should not be changed nor be affected by flood control
        else
        {
            // send in universal language if player in .gm on mode (ignore spell effects)
            if (sender->isGameMaster() || sender->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL) > 0)
                lang = LANG_UNIVERSAL;
            else
            {
                // send in universal language in two side iteration allowed mode
                if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT))
                    lang = LANG_UNIVERSAL;
                else
                {
                    switch (type)
                    {
                        case CHAT_MSG_PARTY:
                        case CHAT_MSG_PARTY_LEADER:
                        case CHAT_MSG_RAID:
                        case CHAT_MSG_RAID_LEADER:
                        case CHAT_MSG_RAID_WARNING:
                            // allow two side chat at group channel if two side group allowed
                            if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
                                lang = LANG_UNIVERSAL;
                            if (sender->GetGroup() && sender->GetGroup()->isLFGGroup() && (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFG) || sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFR)))
                                lang = LANG_UNIVERSAL;
                            break;
                        case CHAT_MSG_GUILD:
                        case CHAT_MSG_OFFICER:
                            // allow two side chat at guild channel if two side guild allowed
                            if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
                                lang = LANG_UNIVERSAL;
                            break;
                        default:
                            break;
                    }
                }

                // but overwrite it by SPELL_AURA_MOD_LANGUAGE auras (only single case used)
                if (Unit::AuraEffectList const* ModLangAuras = sender->GetAuraEffectsByType(SPELL_AURA_MOD_LANGUAGE))
                    if (ModLangAuras->begin() != ModLangAuras->end())
                        lang = (*ModLangAuras->begin())->GetMiscValue();
            }
        }
    }
    else
        lang = LANG_UNIVERSAL;

    if (sender->HasAura(1852) && type != CHAT_MSG_WHISPER)
    {
        SendNotification(GetTrinityString(LANG_GM_SILENCE), sender->GetName());
        return;
    }

    if (msg.empty())
        return;

    if (ChatHandler(this).ParseCommands(msg.c_str()) > 0)
        return;

    if (ChatHandler(this).PlayerExtraCommand(msg.c_str()))
        return;

    // do message validity checks
    if (!ValidateMessage(sender, msg))
        return;

    if (!processChatmessageFurtherAfterSecurityChecks(msg, lang))
        return;

    if (msg.empty())
        return;

    bool isSpamm = false;

    /// filtering of bad words
    if(sWorld->getBoolConfig(CONFIG_WORD_FILTER_ENABLE))
    {
        std::string const& m_sentMsgCache(msg);

        // horizontally
        std::string badWord = sWordFilterMgr->FindBadWord(msg);

        // vertically
        if(badWord.empty())
            badWord = sWordFilterMgr->FindBadWord(m_sentMsgCache);

        if (!badWord.empty())
            isSpamm = true;
    }

    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_YELL:
        {
            if (sender->getLevel() < sWorld->getIntConfig(CONFIG_CHAT_SAY_LEVEL_REQ))
            {
                SendNotification(GetTrinityString(LANG_SAY_REQ), sWorld->getIntConfig(CONFIG_CHAT_SAY_LEVEL_REQ));
                return;
            }

            if (type == CHAT_MSG_SAY)
                sender->Say(msg, lang, isSpamm);
            else if (type == CHAT_MSG_EMOTE)
                sender->TextEmote(msg, isSpamm);
            else if (type == CHAT_MSG_YELL)
                sender->Yell(msg, lang, isSpamm);
            break;
        }
        case CHAT_MSG_WHISPER:
        {
            if (sender->getLevel() < sWorld->getIntConfig(CONFIG_CHAT_WHISPER_LEVEL_REQ))
            {
                SendNotification(GetTrinityString(LANG_WHISPER_REQ), sWorld->getIntConfig(CONFIG_CHAT_WHISPER_LEVEL_REQ));
                return;
            }

            if (!normalizePlayerName(target))
            {
                SendPlayerNotFoundNotice(target);
                break;
            }

            Player* receiver = sObjectAccessor->FindPlayerByName(target);
            bool senderIsPlayer = AccountMgr::IsPlayerAccount(GetSecurity());
            bool receiverIsPlayer = AccountMgr::IsPlayerAccount(receiver ? receiver->GetSession()->GetSecurity() : SEC_PLAYER);
            if (!receiver || (senderIsPlayer && !receiverIsPlayer && !receiver->isAcceptWhispers() && !receiver->IsInWhisperWhiteList(sender->GetGUID())))
            {
                SendPlayerNotFoundNotice(target);
                return;
            }

            if (senderIsPlayer && receiverIsPlayer)
            {
                if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT))
                {
                    if (GetPlayer()->GetTeam() != receiver->GetTeam())
                    {
                        SendPlayerNotFoundNotice(target);
                        return;
                    }
                }

                if (receiver->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
                {
                    SendPlayerNotFoundNotice(target);
                    return;
                }

                if (sender->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
                {
                    sender->SendInvisibleStatusMsg(2);
                    return;
                }
            }

            if (GetPlayer()->HasAura(1852) && !receiver->isGameMaster())
            {
                SendNotification(GetTrinityString(LANG_GM_SILENCE), GetPlayer()->GetName());
                return;
            }

            // If player is a Gamemaster and doesn't accept whisper, we auto-whitelist every player that the Gamemaster is talking to
            if (!senderIsPlayer && !sender->isAcceptWhispers() && !sender->IsInWhisperWhiteList(receiver->GetGUID()))
                sender->AddWhisperWhiteList(receiver->GetGUID());

            GetPlayer()->Whisper(msg, lang, receiver->GetGUID(), isSpamm);
            break;
        }
        case CHAT_MSG_PARTY:
        {
            // if player is in battleground, he cannot say to battleground members by /p
            Group* group = GetPlayer()->GetOriginalGroup();
            if (!group)
            {
                group = _player->GetGroup();
                if (!group || group->isBGGroup())
                    return;
            }

            if (group->IsLeader(GetPlayer()->GetGUID()))
                type = CHAT_MSG_PARTY_LEADER;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);
            
            if (msg.empty())
                return;

            WorldPackets::Chat::Chat packet;
            packet.Initialize(ChatMsg(type), Language(lang), sender, nullptr, msg);
            group->BroadcastPacket(packet.Write(), false, group->GetMemberGroup(GetPlayer()->GetGUID()));
            break;
        }
        case CHAT_MSG_GUILD:
        {
            if (GetPlayer()->GetGuildId())
            {
                if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildId()))
                {
                    sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, guild);

                    guild->BroadcastToGuild(this, false, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);
                }
            }
            break;
        }
        case CHAT_MSG_OFFICER:
        {
            if (GetPlayer()->GetGuildId())
            {
                if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildId()))
                {
                    sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, guild);

                    guild->BroadcastToGuild(this, true, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);
                }
            }
            break;
        }
        case CHAT_MSG_RAID:
        {
            // if player is in battleground, he cannot say to battleground members by /ra
            Group* group = GetPlayer()->GetOriginalGroup();
            if (!group)
            {
                group = GetPlayer()->GetGroup();
                if (!group)
                    return;

                if (group->isBGGroup())
                {
                    type = CHAT_MSG_INSTANCE_CHAT;
                    if (group->IsLeader(GetPlayer()->GetGUID()))
                        type = CHAT_MSG_INSTANCE_CHAT_LEADER;

                    sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);
                    
                    if (msg.empty())
                        return;

                    WorldPackets::Chat::Chat packet;
                    packet.Initialize(ChatMsg(type), Language(lang), sender, nullptr, msg);
                    group->BroadcastPacket(packet.Write(), false);
                    break;
                }
            }

            if (group->IsLeader(GetPlayer()->GetGUID()))
                type = CHAT_MSG_RAID_LEADER;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);

            WorldPackets::Chat::Chat packet;
            packet.Initialize(ChatMsg(type), Language(lang), sender, nullptr, msg);
            group->BroadcastPacket(packet.Write(), false);
            break;
        }
        case CHAT_MSG_RAID_WARNING:
        {
            Group* group = GetPlayer()->GetGroup();
            if (!group || !group->isRaidGroup() || !(group->IsLeader(GetPlayer()->GetGUID()) || group->IsAssistant(GetPlayer()->GetGUID()) || group->GetGroupFlags() & GROUP_FLAG_EVERYONE_ASSISTANT) || group->isBGGroup())
                return;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);

            WorldPackets::Chat::Chat packet;
            packet.Initialize(CHAT_MSG_RAID_WARNING, Language(lang), sender, nullptr, msg);
            group->BroadcastPacket(packet.Write(), false);
            break;
        }
        case CHAT_MSG_INSTANCE_CHAT:
        {
            Group* group = GetPlayer()->GetGroup();
            if (!group)
                return;

            if (group->IsLeader(GetPlayer()->GetGUID()))
                type = CHAT_MSG_INSTANCE_CHAT_LEADER;

            sScriptMgr->OnPlayerChat(GetPlayer(), type, lang, msg, group);
            
            if (msg.empty())
                return;

            WorldPackets::Chat::Chat packet;
            packet.Initialize(ChatMsg(type), Language(lang), sender, nullptr, msg);
            group->BroadcastPacket(packet.Write(), false);
            break;
        }
        case CHAT_MSG_CHANNEL:
        {
            if (AccountMgr::IsPlayerAccount(GetSecurity()))
            {
                if (_player->getLevel() < sWorld->getIntConfig(CONFIG_CHAT_CHANNEL_LEVEL_REQ))
                {
                    SendNotification(GetTrinityString(LANG_CHANNEL_REQ), sWorld->getIntConfig(CONFIG_CHAT_CHANNEL_LEVEL_REQ));
                    return;
                }
            }

            if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
            {
                if (Channel* chn = cMgr->GetChannel(target, _player))
                {
                    sScriptMgr->OnPlayerChat(_player, type, lang, msg, chn);
                    chn->Say(_player->GetGUID(), msg.c_str(), lang, isSpamm);
                }
            }
            break;
        }
        case CHAT_MSG_DND:
        {
            if (msg.empty() || !_player->isDND())
            {
                if (!_player->isDND())
                {
                    if (msg.empty())
                        msg = GetTrinityString(LANG_PLAYER_DND_DEFAULT);
                    _player->dndMsg = msg;
                }

                sScriptMgr->OnPlayerChat(_player, type, lang, msg);

                _player->ToggleDND();
                if (_player->isDND() && _player->isAFK())
                    _player->ToggleAFK();
            }
            break;
        }
        default:
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "CHAT: unknown message type %u, lang: %u", type, lang);
            break;
    }
}

void WorldSession::HandleChatAddonMessageOpcode(WorldPackets::Chat::ChatAddonMessage& packet)
{
    ChatMsg type;

    switch (packet.GetOpcode())
    {
        case CMSG_CHAT_ADDON_MESSAGE_GUILD:
            type = CHAT_MSG_GUILD;
            break;
        case CMSG_CHAT_ADDON_MESSAGE_INSTANCE_CHAT:
            type = CHAT_MSG_INSTANCE_CHAT;
            break;
        case CMSG_CHAT_ADDON_MESSAGE_OFFICER:
            type = CHAT_MSG_OFFICER;
            break;
        case CMSG_CHAT_ADDON_MESSAGE_PARTY:
            type = CHAT_MSG_PARTY;
            break;
        case CMSG_CHAT_ADDON_MESSAGE_RAID:
            type = CHAT_MSG_RAID;
            break;
        default:
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandleAddonMessagechatOpcode: Unknown addon chat opcode (%u)", packet.GetOpcode());
            return;
    }

    HandleChatAddonMessage(type, packet.Prefix, packet.Text);
}

void WorldSession::HandleChatAddonMessageWhisper(WorldPackets::Chat::ChatAddonMessageWhisper& packet)
{
    HandleChatAddonMessage(CHAT_MSG_WHISPER, packet.Prefix, packet.Text, packet.Target);
}

void WorldSession::HandleChatAddonMessageChannel(WorldPackets::Chat::ChatAddonMessageChannel& packet)
{
    HandleChatAddonMessage(CHAT_MSG_CHANNEL, packet.Prefix, packet.Text, packet.Target);
}

void WorldSession::HandleChatAddonMessage(ChatMsg type, std::string const& prefix, std::string& message, std::string const& targetName /*= ""*/)
{
    Player* sender = GetPlayer();

    if (prefix.empty() || prefix.length() > 16)
        return;

    // Logging enabled?
    if (sWorld->getBoolConfig(CONFIG_CHATLOG_ADDON))
    {
        if (message.empty())
            return;

        // Weird way to log stuff...
        sScriptMgr->OnPlayerChat(sender, CHAT_MSG_ADDON, LANG_ADDON, message);
    }

    // Disabled addon channel?
    if (!sWorld->getBoolConfig(CONFIG_ADDON_CHANNEL))
        return;

    switch (type)
    {
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        {
            if (sender->GetGuildId())
                if (Guild* guild = sGuildMgr->GetGuildById(sender->GetGuildId()))
                    guild->BroadcastAddonToGuild(this, type == CHAT_MSG_OFFICER, message, prefix);
            break;
        }
        case CHAT_MSG_WHISPER:
        {
            /// @todo implement cross realm whispers (someday)normalizePlayerName
            auto extName = ExtractExtendedPlayerName(targetName);

            if (!normalizePlayerName(extName.Name))
                break;

            Player* receiver = sObjectAccessor->FindPlayerByName(extName.Name);
            if (!receiver)
                break;

            sender->WhisperAddon(message, prefix, receiver);
            break;
        }
        case CHAT_MSG_PARTY:
        case CHAT_MSG_RAID:
        case CHAT_MSG_INSTANCE_CHAT:
        {
            Group* group = nullptr;
            int32 subGroup = -1;
            if (type != CHAT_MSG_INSTANCE_CHAT)
                group = sender->GetOriginalGroup();

            if (!group)
            {
                group = sender->GetGroup();
                if (!group)
                    break;

                if (type == CHAT_MSG_PARTY)
                    subGroup = sender->GetSubGroup();
            }

            WorldPackets::Chat::Chat packet;
            packet.Initialize(type, LANG_ADDON, sender, nullptr, message, 0, "", DEFAULT_LOCALE, prefix);
            group->BroadcastAddonMessagePacket(packet.Write(), prefix, true, subGroup, sender->GetGUID());
            break;
        }
        default:
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "HandleAddonMessagechatOpcode: unknown addon message type %u", type);
            break;
        }
    }
}

void WorldSession::HandleEmote(WorldPackets::Character::EmoteClient& /*packet*/)
{
    if (!GetPlayer()->isAlive() || GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        return;

    sScriptMgr->OnPlayerClearEmote(GetPlayer());
}

void WorldSession::HandleTextEmoteOpcode(WorldPackets::Chat::CTextEmote& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->isAlive())
        return;

    if (player->IsSpectator())
        return;

    if (!player->CanSpeak())
    {
        SendNotification(GetTrinityString(LANG_WAIT_BEFORE_SPEAKING), secsToTimeString(m_muteTime - time(nullptr)).c_str());
        return;
    }

    sScriptMgr->OnPlayerTextEmote(player, packet.SoundIndex, packet.EmoteID, packet.Target);

    EmotesTextEntry const* em = sEmotesTextStore.LookupEntry(packet.EmoteID);
    if (!em)
        return;

    switch (em->EmoteID)
    {
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_ONESHOT_NONE:
            break;
        case EMOTE_STATE_DANCE:
        case EMOTE_STATE_READ:
            player->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, em->EmoteID);
            break;
        default:
            // Only allow text-emotes for "dead" entities (feign death included)
            if (player->HasUnitState(UNIT_STATE_DIED))
                break;

            player->HandleEmoteCommand(em->EmoteID);
            break;
    }

    WorldPackets::Chat::STextEmote textEmote;
    textEmote.SourceGUID = player->GetGUID();
    textEmote.SourceAccountGUID = GetAccountGUID();
    textEmote.TargetGUID = packet.Target;
    textEmote.EmoteID = packet.EmoteID;
    textEmote.SoundIndex = packet.SoundIndex;
    player->SendMessageToSetInRange(textEmote.Write(), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), true);

    Unit* unit = ObjectAccessor::GetUnit(*player, packet.Target);

    player->UpdateAchievementCriteria(CRITERIA_TYPE_DO_EMOTE, packet.EmoteID, 0, 0, unit);

    //Send scripted event call
    if (unit && unit->IsCreature() && dynamic_cast<Creature*>(unit)->AI())
        dynamic_cast<Creature*>(unit)->AI()->ReceiveEmote(player, packet.EmoteID);
}

void WorldSession::HandleChatReportIgnored(WorldPackets::Chat::ChatReportIgnored& chatReportIgnored)
{
    Player* player = ObjectAccessor::FindPlayer(chatReportIgnored.IgnoredGUID);
    if (!player || !player->GetSession())
        return;

    WorldPackets::Chat::Chat packet;
    packet.Initialize(CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player, _player, GetPlayer()->GetName());
    player->SendDirectMessage(packet.Write());
}

void WorldSession::SendPlayerNotFoundNotice(std::string const& name)
{
    SendPacket(WorldPackets::Chat::ChatPlayerNotfound(name).Write());
}

void WorldSession::SendPlayerAmbiguousNotice(std::string const& name)
{
    SendPacket(WorldPackets::Chat::ChatPlayerAmbiguous(name).Write());
}

void WorldSession::SendChatRestrictedNotice(ChatRestrictionType restriction)
{
    WorldPackets::Chat::ChatRestricted packet;
    packet.Reason = restriction;
    SendPacket(packet.Write());
}

void WorldSession::HandleChatRegisterAddonPrefixes(WorldPackets::Chat::ChatRegisterAddonPrefixes& packet)
{
    for (std::string& prefix : packet.Prefixes)
        _registeredAddonPrefixes.insert(prefix);

    if (_registeredAddonPrefixes.size() > WorldPackets::Chat::ChatRegisterAddonPrefixes::MAX_PREFIXES)
    {
        _filterAddonMessages = false;
        return;
    }

    _filterAddonMessages = true;
}

void WorldSession::HandleChatUnregisterAllAddonPrefixes(WorldPackets::Chat::ChatUnregisterAllAddonPrefixes& /*packet*/)
{
    _registeredAddonPrefixes.clear();
}
