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

#include "QueryPackets.h"
#include "ObjectMgr.h"
#include "QuestData.h"

void WorldSession::SendNameQueryOpcode(ObjectGuid guid)
{
    WorldPackets::Query::QueryPlayerNameResponse response;
    response.Player = guid;

    if (response.Data.Initialize(guid, ObjectAccessor::FindPlayer(guid)))
        response.Result = RESPONSE_SUCCESS; // name known
    else
        response.Result = RESPONSE_FAILURE; // name unknown

    SendPacket(response.Write());
}


void WorldSession::HandleQueryPlayerName(WorldPackets::Query::QueryPlayerName& packet)
{
    SendNameQueryOpcode(packet.Player);
}

void WorldSession::HandleQueryTime(WorldPackets::Query::QueryTime& /*packet*/)
{
    SendQueryTimeResponse();
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPackets::Query::QueryTimeResponse queryTimeResponse;
    queryTimeResponse.CurrentTime = time(nullptr);
    SendPacket(queryTimeResponse.Write());
}

void WorldSession::HandleCreatureQuery(WorldPackets::Query::QueryCreature& packet)
{
    WorldPackets::Query::QueryCreatureResponse response;
    response.CreatureID = packet.CreatureID;

    if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(packet.CreatureID))
    {
        response.Allow = true;

        WorldPackets::Query::CreatureStats& stats = response.Stats;
        for (uint8 i = 0; i < MAX_CREATURE_NAMES; ++i)
        {
            stats.Name[i] = creatureInfo->Name[i];
            stats.NameAlt[i] = creatureInfo->NameAlt[i];
        }
        stats.Title = creatureInfo->Title;
        stats.TitleAlt = creatureInfo->TitleAlt;

        LocaleConstant localeConstant = GetSessionDbLocaleIndex();
        if (CreatureLocale const* creatureLocale = sObjectMgr->GetCreatureLocale(packet.CreatureID))
        {
            for (uint8 i = 0; i < MAX_CREATURE_NAMES; ++i)
            {
                ObjectMgr::GetLocaleString(creatureLocale->Name[i], localeConstant, stats.Name[i]);
                ObjectMgr::GetLocaleString(creatureLocale->NameAlt[i], localeConstant, stats.NameAlt[i]);
            }
            ObjectMgr::GetLocaleString(creatureLocale->Title, localeConstant, stats.Title);
            ObjectMgr::GetLocaleString(creatureLocale->TitleAlt, localeConstant, stats.TitleAlt);
        }

        stats.CursorName = creatureInfo->CursorName;
        stats.CreatureType = creatureInfo->Type;
        stats.CreatureFamily = creatureInfo->Family;
        stats.Classification = creatureInfo->Classification;
        stats.HpMulti = creatureInfo->HpMulti;
        stats.EnergyMulti = creatureInfo->PowerMulti;
        stats.Leader = creatureInfo->Leader;
        stats.CreatureMovementInfoID = creatureInfo->MovementInfoID;
        stats.RequiredExpansion = creatureInfo->RequiredExpansion;
        stats.VignetteID = creatureInfo->VignetteID;
        for (auto questItem : creatureInfo->QuestItem)
            if (questItem)
                stats.QuestItems.push_back(questItem);
        for (uint8 i = 0; i < MAX_TYPE_FLAGS; ++i)
            stats.Flags[i] = creatureInfo->TypeFlags[i];
        for (uint8 i = 0; i < MAX_KILL_CREDIT; ++i)
            stats.ProxyCreatureID[i] = creatureInfo->KillCredit[i];
        for (uint8 i = 0; i < MAX_CREATURE_MODELS; ++i)
            stats.CreatureDisplayID[i] = sObjectMgr->GetCreatureDisplay(creatureInfo->Modelid[i]);
    }

    SendPacket(response.Write());
}

void WorldSession::HandleQueryGameObject(WorldPackets::Query::QueryGameObject& packet)
{
    WorldPackets::Query::QueryGameObjectResponse response;
    response.GameObjectID = packet.GameObjectID;

    if (GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(packet.GameObjectID))
    {
        response.Allow = true;
        WorldPackets::Query::GameObjectStats& stats = response.Stats;

        stats.DisplayID = gameObjectInfo->displayId;
        stats.IconName = gameObjectInfo->IconName;
        stats.Name[0] = gameObjectInfo->name;
        stats.CastBarCaption = gameObjectInfo->castBarCaption;

        for (auto questItem : gameObjectInfo->QuestItems)
            if (questItem)
                stats.QuestItems.push_back(questItem);

        memcpy(stats.Data, gameObjectInfo->raw.data, MAX_GAMEOBJECT_DATA * sizeof(int32));

        stats.Size = gameObjectInfo->size;
        stats.Type = gameObjectInfo->type;
        stats.UnkString = gameObjectInfo->unk1;
        stats.RequiredLevel = gameObjectInfo->RequiredLevel;

        LocaleConstant localeConstant = GetSessionDbLocaleIndex();
        if (GameObjectLocale const* gameObjectLocale = sObjectMgr->GetGameObjectLocale(packet.GameObjectID))
        {
            ObjectMgr::GetLocaleString(gameObjectLocale->Name, localeConstant, stats.Name[0]);
            ObjectMgr::GetLocaleString(gameObjectLocale->CastBarCaption, localeConstant, stats.CastBarCaption);
            ObjectMgr::GetLocaleString(gameObjectLocale->Unk1, localeConstant, stats.UnkString);
        }
    }

    SendPacket(response.Write());
}

void WorldSession::HandleQueryCorpseLocation(WorldPackets::Query::QueryCorpseLocationFromClient& queryCorpseLocation)
{
    Player* player = ObjectAccessor::FindPlayer(queryCorpseLocation.Player);
    if (!player)
        return;

    WorldPackets::Query::CorpseLocation packet;
    Corpse* corpse = player->GetCorpse();
    if (!corpse)
    {
        packet.CorpseOwnerGUID = queryCorpseLocation.Player;
        SendPacket(packet.Write());
        return;
    }

    uint32 mapID = corpse->GetMapId();
    float x = corpse->GetPositionX();
    float y = corpse->GetPositionY();
    float z = corpse->GetPositionZ();
    uint32 corpsemapid = mapID;

    if (mapID != player->GetMapId())
    {
        if (MapEntry const* corpseMapEntry = sMapStore.LookupEntry(mapID))
        {
            if (corpseMapEntry->IsDungeon() && corpseMapEntry->CorpseMapID >= 0)
            {
                if (Map const* entranceMap = sMapMgr->CreateBaseMap(corpseMapEntry->CorpseMapID))
                {
                    mapID = corpseMapEntry->CorpseMapID;
                    x = corpseMapEntry->CorpsePos.X;
                    y = corpseMapEntry->CorpsePos.Y;
                    z = entranceMap->GetHeight(player->GetPhases(), x, y, MAX_HEIGHT);
                }
            }
        }
    }

    packet.Valid = true;
    packet.CorpseOwnerGUID = player->GetGUID();
    packet.MapID = corpsemapid;
    packet.ActualMapID = mapID;
    packet.position = Position(x, y, z);
    packet.Transport = corpse->GetTransGUID();
    SendPacket(packet.Write());
}

void WorldSession::HandleQueryNPCText(WorldPackets::Query::QueryNPCText& packet)
{
    WorldPackets::Query::QueryNPCTextResponse response;
    response.TextID = packet.TextID;

    if (NpcText const* npcText = sObjectMgr->GetNpcText(packet.TextID))
        for (uint8 i = 0; i < MAX_NPC_TEXT_OPTIONS; ++i)
        {
            response.Probabilities[i] = npcText->Data[i].Probability;
            response.BroadcastTextID[i] = npcText->Data[i].BroadcastTextID;
            if (!response.Allow && npcText->Data[i].BroadcastTextID)
                response.Allow = true;
        }

    SendPacket(response.Write());
}

void WorldSession::HandleQueryPageText(WorldPackets::Query::QueryPageText& packet)
{
    WorldPackets::Query::QueryPageTextResponse response;
    response.PageTextID = packet.PageTextID;

    uint32 pageID = packet.PageTextID;
    while (pageID)
    {
        PageText const* pageText = sObjectMgr->GetPageText(pageID);
        if (!pageText || !sConditionMgr->IsPlayerMeetingCondition(_player, pageText->PlayerConditionID))
            break;

        WorldPackets::Query::QueryPageTextResponse::PageTextInfo page;
        page.ID = pageID;
        page.NextPageID = pageText->NextPageID;
        page.Text = pageText->Text;
        page.PlayerConditionID = pageText->PlayerConditionID;
        page.Flags = pageText->Flags;

        if (PageTextLocale const* pageTextLocale = sObjectMgr->GetPageTextLocale(pageID))
            ObjectMgr::GetLocaleString(pageTextLocale->Text, GetSessionDbLocaleIndex(), page.Text);

        response.Pages.push_back(page);
        pageID = pageText->NextPageID;
    }

    response.Allow = !response.Pages.empty();

    SendPacket(response.Write());
}

void WorldSession::HandleQueryCorpseTransport(WorldPackets::Query::QueryCorpseTransport& packet)
{
    WorldPackets::Query::CorpseTransportQuery response;
    response.Player = packet.Player;
    if (Player* player = ObjectAccessor::FindPlayer(packet.Player))
    {
        Corpse* corpse = player->GetCorpse();
        if (_player->IsInSameRaidWith(player) && corpse && !corpse->GetTransGUID().IsEmpty() && corpse->GetTransGUID() == packet.Transport)
        {
            response.Pos = corpse->GetTransOffset();
            response.Facing = corpse->GetTransOffsetO();
        }
    }

    SendPacket(response.Write());
}

void WorldSession::HandleQuestPOIQuery(WorldPackets::Query::QuestPOIQuery& packet)
{
    if (packet.MissingQuestCount > MAX_QUEST_LOG_SIZE)
        return;

    std::unordered_set<int32> questIds;
    for (int32 i = 0; i < packet.MissingQuestCount; ++i)
        questIds.insert(packet.MissingQuestPOIs[i]);

    WorldPackets::Query::QuestPOIQueryResponse response;

    for (auto itr : questIds)
    {
        int32 QuestID = itr;

        bool questOk = false;

        uint16 questSlot = _player->FindQuestSlot(uint32(QuestID));

        if (questSlot != MAX_QUEST_LOG_SIZE)
            questOk = _player->GetQuestSlotQuestId(questSlot) == uint32(QuestID);

        if (questOk)
        {
            auto poiData = sQuestDataStore->GetQuestPOIVector(QuestID);
            if (!poiData)
                continue;

            WorldPackets::Query::QuestPOIData questPOIData;
            questPOIData.QuestID = QuestID;

            for (auto const& data : *poiData)
            {
                WorldPackets::Query::QuestPOIBlobData questPOIBlobData;

                questPOIBlobData.BlobIndex = data.BlobIndex;
                questPOIBlobData.ObjectiveIndex = data.ObjectiveIndex;
                questPOIBlobData.QuestObjectiveID = data.QuestObjectiveID;
                questPOIBlobData.QuestObjectID = data.QuestObjectID;
                questPOIBlobData.MapID = data.MapID;
                questPOIBlobData.WorldMapAreaID = data.WorldMapAreaID;
                questPOIBlobData.Floor = data.Floor;
                questPOIBlobData.Priority = data.Priority;
                questPOIBlobData.Flags = data.Flags;
                questPOIBlobData.WorldEffectID = data.WorldEffectID;
                questPOIBlobData.PlayerConditionID = data.PlayerConditionID;
                questPOIBlobData.SpawnTrackingID = data.SpawnTrackingID;
                questPOIBlobData.AlwaysAllowMergingBlobs = data.AlwaysAllowMergingBlobs;

                for (auto points : data.points)
                {
                    WorldPackets::Query::QuestPOIBlobPoint questPOIBlobPoint;
                    questPOIBlobPoint.X = points.X;
                    questPOIBlobPoint.Y = points.Y;
                    questPOIBlobData.QuestPOIBlobPointStats.push_back(questPOIBlobPoint);
                }

                questPOIData.QuestPOIBlobDataStats.push_back(questPOIBlobData);
            }

            response.QuestPOIDataStats.push_back(questPOIData);
        }
    }

    SendPacket(response.Write());
}

void WorldSession::HandleItemTextQuery(WorldPackets::Query::ItemTextQuery& packet)
{
    WorldPackets::Query::QueryItemTextResponse response;
    response.Id = packet.Id;

    if (Item* item = _player->GetItemByGuid(packet.Id))
    {
        response.Valid = true;
        response.Item.Text = item->GetText();
    }

    SendPacket(response.Write());
}

void WorldSession::HandleQueryQuestCompletionNPCs(WorldPackets::Query::QueryQuestCompletionNPCs& packet)
{
    WorldPackets::Query::QuestCompletionNPCResponse response;

    for (int32 const& questID : packet.QuestCompletionNPCs)
    {
        if (!sQuestDataStore->GetQuestTemplate(questID))
            continue;

        WorldPackets::Query::QuestCompletionNPC questCompletionNPC;
        questCompletionNPC.QuestID = questID;

        auto creatures = sQuestDataStore->GetCreatureQuestInvolvedRelationBoundsByQuest(questID);
        for (auto it = creatures.first; it != creatures.second; ++it)
            questCompletionNPC.NPCs.push_back(it->second);

        auto gos = sQuestDataStore->GetGOQuestInvolvedRelationBoundsByQuest(questID);
        for (auto it = gos.first; it != gos.second; ++it)
            questCompletionNPC.NPCs.push_back(it->second | 0x80000000); // GO mask

        response.QuestCompletionNPCs.push_back(questCompletionNPC);
    }

    SendPacket(response.Write());
}

void WorldSession::HandleQueryRealmName(WorldPackets::Query::QueryRealmName& packet)
{
    WorldPackets::Query::RealmQueryResponse response;
    response.VirtualRealmAddress = packet.RealmID;
    if (packet.RealmID != realm.Id.Realm && packet.RealmID != GetVirtualRealmAddress())  // Cheater ?
    {
        response.LookupState = 1;
        SendPacket(response.Write());
    }

    response.NameInfo.IsLocal = true;
    response.NameInfo.RealmNameActual = sWorld->GetRealmName();
    response.NameInfo.RealmNameNormalized = sWorld->GetTrimmedRealmName();
    SendPacket(response.Write());
}
