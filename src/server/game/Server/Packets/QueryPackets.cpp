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

#include "QueryPackets.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"

void WorldPackets::Query::QueryCreature::Read()
{
    _worldPacket >> CreatureID;
}

WorldPacket const* WorldPackets::Query::QueryCreatureResponse::Write()
{
    _worldPacket << CreatureID;
    _worldPacket.WriteBit(Allow);
    _worldPacket.FlushBits();

    if (Allow)
    {
        _worldPacket.WriteBits(Stats.Title.length() + 1, 11);
        _worldPacket.WriteBits(Stats.TitleAlt.length() + 1, 11);
        _worldPacket.WriteBits(Stats.CursorName.length() + 1, 6);
        _worldPacket.WriteBit(Stats.Leader);

        for (uint32 i = 0; i < MAX_CREATURE_NAMES; ++i)
        {
            _worldPacket.WriteBits(Stats.Name[i].length() + 1, 11);
            _worldPacket.WriteBits(Stats.NameAlt[i].length() + 1, 11);
        }

        for (uint32 i = 0; i < MAX_CREATURE_NAMES; ++i)
        {
            if (!Stats.Name[i].empty())
                _worldPacket << Stats.Name[i];

            if (!Stats.NameAlt[i].empty())
                _worldPacket << Stats.NameAlt[i];
        }

        for (auto flag : Stats.Flags)
            _worldPacket << flag;

        _worldPacket << Stats.CreatureType;
        _worldPacket << Stats.CreatureFamily;
        _worldPacket << Stats.Classification;

        for (auto proxyCreatureID : Stats.ProxyCreatureID)
            _worldPacket << proxyCreatureID;

        for (auto creatureDisplayID : Stats.CreatureDisplayID)
            _worldPacket << creatureDisplayID;

        _worldPacket << Stats.HpMulti;
        _worldPacket << Stats.EnergyMulti;

        _worldPacket << static_cast<int32>(Stats.QuestItems.size());
        _worldPacket << Stats.CreatureMovementInfoID;
        _worldPacket << Stats.RequiredExpansion;
        _worldPacket << Stats.FlagQuest;
        _worldPacket << Stats.VignetteID;

        if (!Stats.Title.empty())
            _worldPacket << Stats.Title;

        if (!Stats.TitleAlt.empty())
            _worldPacket << Stats.TitleAlt;

        if (!Stats.CursorName.empty())
            _worldPacket << Stats.CursorName;

        for (int32 questItem : Stats.QuestItems)
            _worldPacket << questItem;
    }

    return &_worldPacket;
}

void WorldPackets::Query::QueryPlayerName::Read()
{
    _worldPacket >> Player;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Query::PlayerGuidLookupHint const& lookupHint)
{
    data.WriteBit(lookupHint.VirtualRealmAddress.is_initialized());
    data.WriteBit(lookupHint.NativeRealmAddress.is_initialized());
    data.FlushBits();

    if (lookupHint.VirtualRealmAddress)
        data << *lookupHint.VirtualRealmAddress;

    if (lookupHint.NativeRealmAddress)
        data << *lookupHint.NativeRealmAddress;

    return data;
}

bool WorldPackets::Query::PlayerGuidLookupData::Initialize(ObjectGuid const& guid, Player const* player /*= nullptr*/)
{
    if (player)
    {
        if(player->GetGUID() != guid)
            return false;

        AccountID = player->GetSession()->GetAccountGUID();
        BnetAccountID = player->GetSession()->GetBattlenetAccountGUID();
        Name = player->GetName();
        Race = player->getRace();
        Sex = player->getGender();
        ClassID = player->getClass();
        Level = player->getLevel();

        if (DeclinedName const* names = player->GetDeclinedNames())
            DeclinedNames = *names;

        IsDeleted = false;
    }
    else
    {
        CharacterInfo const* characterInfo = sWorld->GetCharacterInfo(guid);
        if (!characterInfo)
            return false;

        AccountID = ObjectGuid::Create<HighGuid::WowAccount>(characterInfo->AccountId);
        BnetAccountID = ObjectGuid::Create<HighGuid::BNetAccount>(characterInfo->BnetAccountId);
        Name = characterInfo->Name;
        Race = characterInfo->Race;
        Sex = characterInfo->Sex;
        ClassID = characterInfo->Class;
        Level = characterInfo->Level;

        IsDeleted = characterInfo->IsDeleted;
    }

    GuidActual = guid;
    VirtualRealmAddress = GetVirtualRealmAddress();

    return true;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Query::PlayerGuidLookupData const& lookupData)
{
    data.WriteBit(lookupData.IsDeleted);
    data.WriteBits(lookupData.Name.length(), 6);

    for (auto const& name : lookupData.DeclinedNames.name)
    {
        if (name.length())
            data.WriteBits(name.length(), 7);
        else
            data.WriteBits(lookupData.Name.length(), 7);
    }

    for (auto const& name : lookupData.DeclinedNames.name)
    {
        if (name.length())
            data.WriteString(name);
        else
            data.WriteString(lookupData.Name);
    }

    data << lookupData.AccountID;
    data << lookupData.BnetAccountID;
    data << lookupData.GuidActual;
    data << uint32(lookupData.VirtualRealmAddress);
    data << uint8(lookupData.Race);
    data << uint8(lookupData.Sex);
    data << uint8(lookupData.ClassID);
    data << uint8(lookupData.Level);
    data.WriteString(lookupData.Name);

    return data;
}

WorldPacket const* WorldPackets::Query::QueryPlayerNameResponse::Write()
{
    _worldPacket << Result;
    _worldPacket << Player;

    if (Result == RESPONSE_SUCCESS)
        _worldPacket << Data;

    return &_worldPacket;
}

void WorldPackets::Query::QueryPageText::Read()
{
    _worldPacket >> PageTextID;
    _worldPacket >> ItemGUID;
}


ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Query::QueryPageTextResponse::PageTextInfo const& page)
{
    data << uint32(page.ID);
    data << uint32(page.NextPageID);
    data << int32(page.PlayerConditionID);
    data << uint8(page.Flags);
    data.WriteBits(page.Text.length(), 12);
    data.FlushBits();

    data.WriteString(page.Text);

    return data;
}

WorldPacket const* WorldPackets::Query::QueryPageTextResponse::Write()
{
    _worldPacket << PageTextID;
    _worldPacket.WriteBit(Allow);

    _worldPacket.FlushBits();

    if (Allow)
    {
        _worldPacket << uint32(Pages.size());
        for (PageTextInfo const& pageText : Pages)
            _worldPacket << pageText;
    }

    return &_worldPacket;
}

void WorldPackets::Query::QueryNPCText::Read()
{
    _worldPacket >> TextID;
    _worldPacket >> Guid;
}

WorldPacket const* WorldPackets::Query::QueryNPCTextResponse::Write()
{
    _worldPacket << TextID;
    _worldPacket.WriteBit(Allow);

    _worldPacket.FlushBits();

    _worldPacket << int32(Allow ? (MAX_NPC_TEXT_OPTIONS * (sizeof(float) + sizeof(uint32))) : 0);

    if (Allow)
    {
        for (auto Probabilitie : Probabilities)
            _worldPacket << Probabilitie;
        for (auto broadcastTextID : BroadcastTextID)
            _worldPacket << broadcastTextID;
    }

    return &_worldPacket;
}

void WorldPackets::Query::QueryGameObject::Read()
{
    _worldPacket >> GameObjectID;
    _worldPacket >> Guid;
}

WorldPacket const* WorldPackets::Query::QueryGameObjectResponse::Write()
{
    _worldPacket << GameObjectID;
    _worldPacket.WriteBit(Allow);
    _worldPacket.FlushBits();

    ByteBuffer statsData;
    if (Allow)
    {
        statsData << int32(Stats.Type);
        statsData << int32(Stats.DisplayID);
        for (auto const& name : Stats.Name)
            statsData << name;

        statsData << Stats.IconName;
        statsData << Stats.CastBarCaption;
        statsData << Stats.UnkString;

        for (auto const& data : Stats.Data)
            statsData << int32(data);

        statsData << float(Stats.Size);
        statsData << uint8(Stats.QuestItems.size());
        for (auto questItem : Stats.QuestItems)
            statsData << int32(questItem);

        statsData << int32(Stats.RequiredLevel);
    }

    _worldPacket << uint32(statsData.size());
    if (!statsData.empty())
        _worldPacket.append(statsData);

    return &_worldPacket;
}

void WorldPackets::Query::QueryCorpseLocationFromClient::Read()
{
    _worldPacket >> Player;
}

WorldPacket const* WorldPackets::Query::CorpseLocation::Write()
{
    _worldPacket.WriteBit(Valid);
    _worldPacket.FlushBits();

    _worldPacket << CorpseOwnerGUID;
    _worldPacket << ActualMapID;
    _worldPacket << position;
    _worldPacket << MapID;
    _worldPacket << Transport;

    return &_worldPacket;
}

void WorldPackets::Query::QueryCorpseTransport::Read()
{
    _worldPacket >> Player;
    _worldPacket >> Transport;
}

WorldPacket const* WorldPackets::Query::CorpseTransportQuery::Write()
{
    _worldPacket << Player;
    _worldPacket << Pos;
    _worldPacket << Facing;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Query::QueryTimeResponse::Write()
{
    _worldPacket << uint32(CurrentTime);

    return &_worldPacket;
}

void WorldPackets::Query::QuestPOIQuery::Read()
{
    _worldPacket >> MissingQuestCount;

    for (auto& missingQuestPOI : MissingQuestPOIs)
        _worldPacket >> missingQuestPOI;
}

WorldPacket const* WorldPackets::Query::QuestPOIQueryResponse::Write()
{
    _worldPacket << static_cast<int32>(QuestPOIDataStats.size());
    _worldPacket << static_cast<int32>(QuestPOIDataStats.size());

    for (auto const& questPOIData : QuestPOIDataStats)
    {
        _worldPacket << int32(questPOIData.QuestID);

        _worldPacket << static_cast<int32>(questPOIData.QuestPOIBlobDataStats.size());

        for (auto const& questPOIBlobData : questPOIData.QuestPOIBlobDataStats)
        {
            _worldPacket << int32(questPOIBlobData.BlobIndex);
            _worldPacket << int32(questPOIBlobData.ObjectiveIndex);
            _worldPacket << int32(questPOIBlobData.QuestObjectiveID);
            _worldPacket << int32(questPOIBlobData.QuestObjectID);
            _worldPacket << int32(questPOIBlobData.MapID);
            _worldPacket << int32(questPOIBlobData.WorldMapAreaID);
            _worldPacket << int32(questPOIBlobData.Floor);
            _worldPacket << int32(questPOIBlobData.Priority);
            _worldPacket << int32(questPOIBlobData.Flags);
            _worldPacket << int32(questPOIBlobData.WorldEffectID);
            _worldPacket << int32(questPOIBlobData.PlayerConditionID);
            _worldPacket << int32(questPOIBlobData.SpawnTrackingID);
            _worldPacket << static_cast<int32>(questPOIBlobData.QuestPOIBlobPointStats.size());

            for (auto const& questPOIBlobPoint : questPOIBlobData.QuestPOIBlobPointStats)
            {
                _worldPacket << int32(questPOIBlobPoint.X);
                _worldPacket << int32(questPOIBlobPoint.Y);
            }

            _worldPacket.WriteBit(questPOIBlobData.AlwaysAllowMergingBlobs);
            _worldPacket.FlushBits();
        }
    }

    return &_worldPacket;
}

void WorldPackets::Query::QueryQuestCompletionNPCs::Read()
{
    uint32 questCount = 0;

    _worldPacket >> questCount;
    QuestCompletionNPCs.resize(questCount);

    for (int32& QuestID : QuestCompletionNPCs)
        _worldPacket >> QuestID;
}

WorldPacket const* WorldPackets::Query::QuestCompletionNPCResponse::Write()
{
    _worldPacket << static_cast<uint32>(QuestCompletionNPCs.size());
    for (auto const& quest : QuestCompletionNPCs)
    {
        _worldPacket << int32(quest.QuestID);

        _worldPacket << static_cast<uint32>(quest.NPCs.size());
        for (int32 const& npc : quest.NPCs)
            _worldPacket << int32(npc);
    }

    return &_worldPacket;
}

void WorldPackets::Query::QueryPetName::Read()
{
    _worldPacket >> UnitGUID;
}

WorldPacket const* WorldPackets::Query::QueryPetNameResponse::Write()
{
    _worldPacket << UnitGUID;
    _worldPacket.WriteBit(Allow);

    if (Allow)
    {
        _worldPacket.WriteBits(Name.length(), 8);
        _worldPacket.WriteBit(HasDeclined);

        for (auto const& name : DeclinedNames.name)
            _worldPacket.WriteBits(name.length(), 7);

        for (auto const& name : DeclinedNames.name)
            _worldPacket.WriteString(name);

        _worldPacket << Timestamp;
        _worldPacket.WriteString(Name);
    }

    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Query::ItemTextQuery::Read()
{
    _worldPacket >> Id;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Query::ItemTextCache const& itemTextCache)
{
    data.WriteBits(itemTextCache.Text.length(), 13);
    data.WriteString(itemTextCache.Text);

    return data;
}

WorldPacket const* WorldPackets::Query::QueryItemTextResponse::Write()
{
    _worldPacket.WriteBit(Valid);
    _worldPacket.FlushBits();
    _worldPacket << Item;
    _worldPacket << Id;

    return &_worldPacket;
}

void WorldPackets::Query::QueryRealmName::Read()
{
    _worldPacket >> RealmID;
}

WorldPacket const* WorldPackets::Query::RealmQueryResponse::Write()
{
    _worldPacket << VirtualRealmAddress;
    _worldPacket << LookupState;
    if (!LookupState)
    {
        _worldPacket.WriteBit(NameInfo.IsLocal);
        _worldPacket.WriteBit(NameInfo.UnkBit);
        _worldPacket.WriteBits(NameInfo.RealmNameActual.length(), 8);
        _worldPacket.WriteBits(NameInfo.RealmNameNormalized.length(), 8);
        _worldPacket.WriteString(NameInfo.RealmNameActual);
        _worldPacket.WriteString(NameInfo.RealmNameNormalized);
    }

    return &_worldPacket;
}
