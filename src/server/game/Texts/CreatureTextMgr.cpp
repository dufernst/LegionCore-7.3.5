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

#include "Common.h"
#include "CreatureTextMgrImpl.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"
#include <utility>
#include "MiscPackets.h"
#include "ChatPackets.h"
#include "GlobalFunctional.h"

class CreatureTextBuilder
{
public:
    CreatureTextBuilder(WorldObject const* obj, uint8 gender, ChatMsg msgtype, std::string baseText, uint32 BroadcastTextID, uint32 language, WorldObject const* target) :
        _source(obj), _target(target), _baseText(std::move(baseText)), _msgType(msgtype), _language(language), _BroadcastTextID(BroadcastTextID), _gender(gender) { }

    WorldPackets::Chat::Chat* operator()(LocaleConstant locale) const
    {
        auto chat = new WorldPackets::Chat::Chat();

        auto baseText = _baseText;
        if (auto bct = sBroadcastTextStore.LookupEntry(_BroadcastTextID))
            baseText = DB2Manager::GetBroadcastTextValue(bct, locale, _gender);

        chat->Initialize(_msgType, Language(_language), _source, _target, baseText, 0, "", locale);
        return chat;
    }

private:
    WorldObject const* _source;
    WorldObject const* _target;
    std::string _baseText;
    ChatMsg _msgType;
    uint32 _language;
    uint32 _BroadcastTextID;
    uint8 _gender;
};

class PlayerTextBuilder
{
public:
    PlayerTextBuilder(WorldObject const* speaker, uint8 gender, ChatMsg msgtype, std::string baseText, uint32 BroadcastTextID, uint32 language, WorldObject const* target) :
        _talker(speaker), _target(target), _baseText(std::move(baseText)), _msgType(msgtype), _language(language), _BroadcastTextID(BroadcastTextID), _gender(gender) { }

    WorldPackets::Chat::Chat* operator()(LocaleConstant locale) const
    {
        auto chat = new WorldPackets::Chat::Chat();

        auto baseText = _baseText;
        if (auto bct = sBroadcastTextStore.LookupEntry(_BroadcastTextID))
            baseText = DB2Manager::GetBroadcastTextValue(bct, locale, _gender);

        chat->Initialize(_msgType, Language(_language), _talker, _target, baseText, 0, "", locale);
        return chat;
    }

private:
    WorldObject const* _talker;
    WorldObject const* _target;
    std::string _baseText;
    ChatMsg _msgType;
    uint32 _language;
    uint32 _BroadcastTextID;
    uint8 _gender;
};

CreatureTextId::CreatureTextId(uint32 e, uint32 g, uint32 i) : entry(e), textGroup(g), textId(i) { }

bool CreatureTextId::operator<(CreatureTextId const& right) const
{
    return std::tie(entry, textGroup, textId) < std::tie(right.entry, right.textGroup, right.textId);
}

void CreatureTextMgr::LoadCreatureTexts()
{
    uint32 oldMSTime = getMSTime();

    mTextMap.clear(); // for reload case
    mTextRepeatMap.clear(); //reset all currently used temp texts

                                      //      0      1        2   3     4     5         6            7      8         9      10                 11      12         13
    auto result = WorldDatabase.Query("SELECT Entry, GroupID, ID, Text, Type, Language, Probability, Emote, Duration, Sound, BroadcastTextID, MinTimer, MaxTimer, SpellID FROM creature_text ORDER BY GroupID");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 ceature texts. DB table `creature_text` is empty.");
        return;
    }

    uint32 textCount = 0;
    uint32 creatureCount = 0;

    do
    {
        Field* fields = result->Fetch();

        CreatureTextEntry temp;
        temp.entry = fields[0].GetUInt32();
        temp.group = fields[1].GetUInt8();
        temp.id = fields[2].GetUInt8();
        temp.text = fields[3].GetString();
        temp.type = ChatMsg(fields[4].GetUInt8());
        temp.lang = Language(fields[5].GetUInt8());
        temp.probability = fields[6].GetFloat();
        temp.emote = Emote(fields[7].GetUInt32());
        temp.duration = fields[8].GetUInt32();
        temp.sound = fields[9].GetUInt32();
        temp.BroadcastTextID = fields[10].GetUInt32();
        temp.MinTimer = fields[11].GetUInt32();
        temp.MaxTimer = fields[12].GetUInt32();
        temp.SpellID = fields[13].GetUInt32();

        if (!GetLanguageDescByID(temp.lang))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "CreatureTextMgr:  Entry %u, Group %u in table `creature_texts` using Language %u but Language does not exist.", temp.entry, temp.group, uint32(temp.lang));
            temp.lang = LANG_UNIVERSAL;
        }

        if (temp.type >= MAX_CHAT_MSG_TYPE)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "CreatureTextMgr:  Entry %u, Group %u in table `creature_texts` has Type %u but this Chat Type does not exist.", temp.entry, temp.group, uint32(temp.type));
            temp.type = CHAT_MSG_SAY;
        }

        if (temp.emote && !sEmotesStore.LookupEntry(temp.emote))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "CreatureTextMgr:  Entry %u, Group %u in table `creature_texts` has Emote %u but emote does not exist.", temp.entry, temp.group, uint32(temp.emote));
            temp.emote = EMOTE_ONESHOT_NONE;
        }

        if (temp.BroadcastTextID && !sBroadcastTextStore.LookupEntry(temp.BroadcastTextID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "CreatureTextMgr: Entry %u, Group %u, Id %u in table `creature_text` has non-existing or incompatible BroadcastTextID %u.", temp.entry, temp.group, temp.id, temp.BroadcastTextID);
            temp.BroadcastTextID = 0;
        }

        if (temp.SpellID && !sSpellMgr->GetSpellInfo(temp.SpellID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "CreatureTextMgr: Entry %u, Group %u, Id %u in table `creature_text` has non-existing or incompatible SpellID %u.", temp.entry, temp.group, temp.id, temp.SpellID);
            temp.SpellID = 0;
        }

        if (IsDuplicateText(temp.entry, temp.text))
        {
            WorldDatabase.PExecute("DELETE FROM creature_text WHERE Entry = %u AND GroupID = %u AND ID = %u", temp.entry, temp.group, temp.id);
            continue;
        }

        //entry not yet added, add empty TextHolder (list of groups)
        if (mTextMap.find(temp.entry) == mTextMap.end())
            ++creatureCount;

        if (mTextList.size() <= temp.entry)
            mTextList.resize(temp.entry + 1, nullptr);

        //add the text into our entry's group
        if (!temp.id)
            temp.id = mTextMap[temp.entry][temp.group].size();

        mTextMap[temp.entry][temp.group].push_back(temp);
        mTextList[temp.entry] = &mTextMap[temp.entry];

        ++textCount;
    } while (result->NextRow());

    // sort text by timer
    for (auto& textEntry : mTextMap)
    {
        for (auto& text : textEntry.second)
        {
            if (text.second.size() < 2)
                continue;

            std::sort(text.second.begin(), text.second.end(), [](CreatureTextEntry const& a, CreatureTextEntry const& b) -> bool
            {
                return a.MinTimer < b.MinTimer;
            });
        }
    }
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creature texts for %u creatures in %u ms", textCount, creatureCount, GetMSTimeDiffToNow(oldMSTime));
}

uint32 CreatureTextMgr::SendChat(Creature* source, uint8 textGroup, ObjectGuid whisperGuid /*= 0*/, ChatMsg msgType /*= CHAT_MSG_ADDON*/, Language language /*= LANG_ADDON*/, TextRange range /*= TEXT_RANGE_NORMAL*/, uint32 sound /*= 0*/, Team team /*= TEAM_OTHER*/, bool gmOnly /*= false*/, Player* srcPlr /*= NULL*/, bool ingoreProbality /*= false*/)
{
    if (!source || !source->IsInWorld())
        return 0;

    uint32 entry = source->GetGUID().GetEntry();
    if (mTextList.size() <= entry)
        return 0;

    //! Use entry from guid. as we could overvrite it by new texnology from Blizzzzzzzzzz
    CreatureTextHolder* textHolder = mTextList[entry];
    if (!textHolder)
    {
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "CreatureTextMgr: Could not find Text for Creature(%s) Entry %u in 'creature_text' table. Ignoring.", source->GetName(), source->GetEntry());
        return 0;
    }

    auto itr = textHolder->find(textGroup);
    if (itr == textHolder->end())
    {
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "CreatureTextMgr: Could not find TextGroup %u for Creature(%s) GuidLow %u Entry %u. Ignoring.", uint32(textGroup), source->GetName(), source->GetGUIDLow(), source->GetEntry());
        return 0;
    }

    CreatureTextGroup const& textGroupContainer = itr->second;  //has all texts in the group
    CreatureTextRepeatIds repeatGroup = GetRepeatGroup(source, textGroup);//has all textIDs from the group that were already said
    CreatureTextGroup tempGroup;//will use this to talk after sorting repeatGroup

    for (const auto& giter : textGroupContainer)
        if (std::find(repeatGroup.begin(), repeatGroup.end(), giter.id) == repeatGroup.end())
            tempGroup.push_back(giter);

    if (tempGroup.empty())
    {
        auto mapItr = mTextRepeatMap.find(source->GetGUIDLow());
        if (mapItr != mTextRepeatMap.end())
            mapItr->second.find(textGroup)->second.clear();

        tempGroup = textGroupContainer;
    }

    auto iter = &(ingoreProbality ? Trinity::Containers::SelectRandomContainerElement(tempGroup) : *Trinity::Containers::SelectRandomWeightedContainerElement(tempGroup, [](CreatureTextEntry const& t) -> double
    {
        return t.probability;
    }));

    SendText(source, iter, whisperGuid, msgType, language, range, sound, team, gmOnly, srcPlr);

    SetRepeatId(source, textGroup, iter->id);

    return iter->duration;
}

void CreatureTextMgr::SendText(Creature* source, CreatureTextEntry const* text, ObjectGuid whisperGuid /*= 0*/, ChatMsg msgType /*= CHAT_MSG_ADDON*/, Language language /*= LANG_ADDON*/, TextRange range /*= TEXT_RANGE_NORMAL*/, uint32 sound /*= 0*/, Team team /*= TEAM_OTHER*/, bool gmOnly /*= false*/, Player* srcPlr /*= NULL*/)
{

    Unit* finalSource = source;
    if (srcPlr)
        finalSource = srcPlr;

    auto bct = sBroadcastTextStore.LookupEntry(text->BroadcastTextID);

    auto finalType = msgType == CHAT_MSG_ADDON ? text->type : msgType;

    uint32 fSound = [bct, sound, source, text]() -> uint32
    {
        uint8 gender = GENDER_NONE;
        if (CreatureDisplayInfoEntry const* creatureDisplay = sCreatureDisplayInfoStore.LookupEntry(source->GetDisplayId()))
            gender = creatureDisplay->Gender;
        if (gender == GENDER_NONE)
            gender = source->getGender();

        if (!sound && bct)
            return gender == GENDER_FEMALE ? bct->SoundEntriesID[1] : bct->SoundEntriesID[0];

        if (sound)
            return sound;
        return text->sound;
    }();

    if (fSound)
        SendSound(source, fSound, finalType, whisperGuid, range, team, gmOnly);

    auto data = std::vector<std::pair<uint32, uint32>>();
    if (bct)
        for (auto i = 0; i < std::extent<decltype(bct->EmoteID)>::value; ++i)
            if (auto broadcastTextSoundId = bct->EmoteID[i])
                data.emplace_back(std::make_pair(broadcastTextSoundId, bct->EmoteDelay[i]));

    if (bct)
        SendEmote(finalSource, bct->EmotesID);

    if (!data.empty())
    {
        for (auto v : data)
            if (auto unit = ObjectAccessor::FindUnit(finalSource->GetGUID()))
                unit->AddDelayedEvent(v.second, [this, finalSource, v]() -> void
            {
                SendEmote(finalSource, v.first);
            });
    }
    else
        SendEmote(finalSource, text->emote);

    uint32 finalLang = language == LANG_ADDON ? text->lang : language;
    if (bct)
        finalLang = bct->LanguageID;

    WorldObject const* whisperTarget = ObjectAccessor::GetWorldObject(*source, whisperGuid);
    if (srcPlr)
    {
        PlayerTextBuilder builder(finalSource, finalSource->getGender(), finalType, text->text, text->BroadcastTextID, finalLang, whisperTarget);
        SendChatPacket(finalSource, builder, finalType, whisperTarget, range, team, gmOnly);
    }
    else
    {
        CreatureTextBuilder builder(finalSource, finalSource->getGender(), finalType, text->text, text->BroadcastTextID, finalLang, whisperTarget);
        SendChatPacket(finalSource, builder, finalType, whisperTarget, range, team, gmOnly);
    }
}

float CreatureTextMgr::GetRangeForChatType(ChatMsg msgType)
{
    float dist = sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY);
    switch (msgType)
    {
        case CHAT_MSG_MONSTER_YELL:
            dist = sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_YELL);
            break;
        case CHAT_MSG_MONSTER_EMOTE:
        case CHAT_MSG_RAID_BOSS_EMOTE:
            dist = sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE);
            break;
        default:
            break;
    }

    return dist;
}

void CreatureTextMgr::SendSound(Creature* source, uint32 soundKitID, ChatMsg msgType, ObjectGuid whisperGuid, TextRange range, Team team, bool gmOnly)
{
    if (!soundKitID || !source)
        return;

    SendNonChatPacket(source, WorldPackets::Misc::PlaySound(source->GetGUID(), soundKitID).Write(), msgType, whisperGuid, range, team, gmOnly);
}

void CreatureTextMgr::SendNonChatPacket(WorldObject* source, WorldPacket const* data, ChatMsg msgType, ObjectGuid whisperGuid, TextRange range, Team team, bool gmOnly) const
{
    switch (msgType)
    {
    case CHAT_MSG_MONSTER_PARTY:
    {
        auto player = ObjectAccessor::FindPlayer(whisperGuid);
        if (!player || !player->GetSession())
            return;

        if (Group const* group = player->GetGroup())
            group->BroadcastWorker([data](Player* player) { player->SendDirectMessage(data); });
        return;
    }
    case CHAT_MSG_MONSTER_WHISPER:
    case CHAT_MSG_RAID_BOSS_WHISPER:
        if (range == TEXT_RANGE_NORMAL)//ignores team and gmOnly
        {
            Player* player = ObjectAccessor::FindPlayer(whisperGuid);
            if (!player || !player->GetSession())
                return;
            player->SendDirectMessage(data);
            return;
        }
        break;
    default:
        break;
    }

    switch (range)
    {
        case TEXT_RANGE_AREA:
        {
            auto areaId = source->GetCurrentAreaID();
            source->GetMap()->ApplyOnEveryPlayer([&](Player* player)
            {
                if (player->GetCurrentAreaID() == areaId && (!team || Team(player->GetTeam()) == team) && (!gmOnly || player->isGameMaster()))
                    player->SendDirectMessage(data);
            });
            return;
        }
        case TEXT_RANGE_ZONE:
        {
            auto zoneId = source->GetCurrentZoneID();
            source->GetMap()->ApplyOnEveryPlayer([&](Player* player)
            {
                if (player->GetCurrentZoneID() == zoneId && (!team || Team(player->GetTeam()) == team) && (!gmOnly || player->isGameMaster()))
                    player->SendDirectMessage(data);
            });
            return;
        }
        case TEXT_RANGE_MAP:
        {
            auto playerSrc = ObjectAccessor::FindPlayer(whisperGuid);
            if (playerSrc)
                source = playerSrc;

            source->GetMap()->ApplyOnEveryPlayer([&](Player* player)
            {
                if ((!team || Team(player->GetTeam()) == team) && (!gmOnly || player->isGameMaster()))
                    player->SendDirectMessage(data);
            });
            return;
        }
        case TEXT_RANGE_WORLD:
        {
            auto const& smap = sWorld->GetAllSessions();
            for (const auto& iter : smap)
                if (auto player = iter.second->GetPlayer())
                    if (player->GetSession() && (!team || Team(player->GetTeam()) == team) && (!gmOnly || player->isGameMaster()))
                        player->SendDirectMessage(data);
            return;
        }
        case TEXT_RANGE_NORMAL:
        default:
            break;
    }

    source->SendMessageToSetInRange(data, GetRangeForChatType(msgType), true);
}

void CreatureTextMgr::SendEmote(Unit* source, uint32 emote)
{
    if (!source)
        return;

    source->HandleEmoteCommand(emote);
}

void CreatureTextMgr::SetRepeatId(Creature* source, uint8 textGroup, uint8 id)
{
    if (!source)
        return;

    i_lockTextRepeat.lock();
    CreatureTextRepeatIds& repeats = mTextRepeatMap[source->GetGUIDLow()][textGroup];
    if (std::find(repeats.begin(), repeats.end(), id) == repeats.end())
        repeats.push_back(id);
    else
        TC_LOG_ERROR(LOG_FILTER_UNITS, "CreatureTextMgr: TextGroup %u for Creature(%s) GuidLow %u Entry %u, id %u already added", uint32(textGroup), source->GetName(), source->GetGUIDLow(), source->GetEntry(), uint32(id));
    i_lockTextRepeat.unlock();
}

CreatureTextRepeatIds CreatureTextMgr::GetRepeatGroup(Creature* source, uint8 textGroup)
{
    ASSERT(source);//should never happen
    CreatureTextRepeatIds ids;

    CreatureTextRepeatMap::const_iterator mapItr = mTextRepeatMap.find(source->GetGUIDLow());
    if (mapItr != mTextRepeatMap.end())
    {
        auto groupItr = (*mapItr).second.find(textGroup);
        if (groupItr != mapItr->second.end())
            ids = groupItr->second;
    }
    return ids;
}

bool CreatureTextMgr::TextExist(uint32 sourceEntry, uint8 textGroup)
{
    if (!sourceEntry || mTextList.size() <= sourceEntry)
        return false;

    //! Use entry from guid. as we could overvrite it by new texnology from Blizzzzzzzzzz
    CreatureTextHolder* textHolder = mTextList[sourceEntry];
    if (!textHolder)
    {
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "CreatureTextMgr::TextExist: Could not find Text for Creature (entry %u) in 'creature_text' table.", sourceEntry);
        return false;
    }

    auto itr = textHolder->find(textGroup);
    if (itr == textHolder->end())
    {
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "CreatureTextMgr::TextExist: Could not find TextGroup %u for Creature (entry %u).", uint32(textGroup), sourceEntry);
        return false;
    }

    return true;
}

CreatureTextGroup const* CreatureTextMgr::GetTextGroup(uint32 sourceEntry, uint8 textGroup)
{
    if (!sourceEntry || mTextList.size() <= sourceEntry)
        return nullptr;

    //! Use entry from guid. as we could overvrite it by new texnology from Blizzzzzzzzzz
    CreatureTextHolder* textHolder = mTextList[sourceEntry];
    if (!textHolder)
        return nullptr;

    auto itr = textHolder->find(textGroup);
    if (itr == textHolder->end() || itr->second.empty())
        return nullptr;

    bool timerExist = false;
    for (auto const& iter : itr->second)
    {
        if (iter.MinTimer != 0)
        {
            timerExist = true;
            break;
        }
    }
    if (!timerExist)
        return nullptr;

    return &itr->second;
}

bool CreatureTextMgr::HasBroadCastText(uint32 entry, uint32 BroadcastTextID, uint8& groupId)
{
    if (!entry || mTextList.size() <= entry)
        return false;

    CreatureTextHolder* textHolder = mTextList[entry];
    if (!textHolder)
        return false;

    for (auto const& giter : *textHolder)
    {
        for (auto const& iter : giter.second)
        {
            if (iter.BroadcastTextID == BroadcastTextID)
            {
                groupId = giter.first;
                return true;
            }
        }
    }

    return false;
}

CreatureTextEntry const* CreatureTextMgr::FindSpellInText(uint32 entry, uint32 SpellID)
{
    if (!entry || mTextList.size() <= entry)
        return nullptr;

    CreatureTextHolder* textHolder = mTextList[entry];
    if (!textHolder)
        return nullptr;

    std::string text("spell:" + std::to_string(SpellID) + "|");

    for (auto const& giter : *textHolder)
    {
        for (auto const& iter : giter.second)
        {
            if (iter.SpellID && iter.SpellID == SpellID)
                return &iter;
            if (iter.text.find(text) != std::string::npos)
                return &iter;
        }
    }

    return nullptr;
}

bool CreatureTextMgr::IsDuplicateText(uint32 entry, const std::string& text)
{
    if (!entry || mTextList.size() <= entry)
        return false;

    CreatureTextHolder* textHolder = mTextList[entry];
    if (!textHolder)
        return false;

    for (auto const& giter : *textHolder)
        for (auto const& iter : giter.second)
            if (iter.text.find(text) != std::string::npos)
                return true;

    return false;
}
