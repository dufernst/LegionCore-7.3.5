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

#include "ConversationData.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"

ConversationDataStoreMgr::ConversationDataStoreMgr()
{
}

ConversationDataStoreMgr::~ConversationDataStoreMgr()
{
}

ConversationDataStoreMgr* ConversationDataStoreMgr::instance()
{
    static ConversationDataStoreMgr instance;
    return &instance;
}

void ConversationDataStoreMgr::LoadConversations()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0    1   2      3      4       5           6           7           8            9            10        11  
    QueryResult result = WorldDatabase.Query("SELECT guid, id, map, zoneId, areaId, position_x, position_y, position_z, orientation, spawnMask, phaseMask, PhaseId "
        "FROM conversation ORDER BY `map` ASC, `guid` ASC");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversations() >> Loaded 0 conversation. DB table `conversation` is empty.");
        return;
    }

    // Build single time for check spawnmask
    std::map<uint32, uint64> spawnMasks;
    for (auto& mapDifficultyPair : sDB2Manager.GetAllMapsDifficultyes())
        for (auto& difficultyPair : mapDifficultyPair.second)
            spawnMasks[mapDifficultyPair.first] |= (UI64LIT(1) << difficultyPair.first);

    _conversationDataStore.rehash(result->GetRowCount());
    std::map<uint32, ConversationSpawnData*> lastEntryCreature;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint8 index = 0;

        ObjectGuid::LowType guid = fields[index++].GetUInt64();
        uint32 entry = fields[index++].GetUInt32();

        auto conversationData = GetConversationData(entry);
        auto conversationCreature = GetConversationCreature(entry);
        auto conversationActor = GetConversationActor(entry);

        bool isActor = conversationActor && !conversationActor->empty();
        bool isCreature = conversationCreature && !conversationCreature->empty();
        bool hasData = conversationData && !conversationData->empty();

        if (!hasData || !isActor && !isCreature)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "ConversationDataStoreMgr::LoadConversations() >> Table `conversation` has conversation (GUID: " UI64FMTD ") with non existing conversation data %u, skipped.", guid, entry);
            continue;
        }

        ConversationSpawnData& data = _conversationDataStore[guid];
        data.guid = guid;
        data.id = entry;
        data.mapid = fields[index++].GetUInt16();
        data.zoneId = fields[index++].GetUInt16();
        data.areaId = fields[index++].GetUInt16();
        data.posX = fields[index++].GetFloat();
        data.posY = fields[index++].GetFloat();
        data.posZ = fields[index++].GetFloat();
        data.orientation = fields[index++].GetFloat();
        data.spawnMask = fields[index++].GetUInt64();
        data.phaseMask = fields[index++].GetUInt32();

        Tokenizer phasesToken(fields[index++].GetString(), ' ', 100);
        for (auto itr : phasesToken)
            if (PhaseEntry const* phase = sPhaseStore.LookupEntry(uint32(strtoull(itr, nullptr, 10))))
                data.PhaseID.insert(phase->ID);

        // check near npc with same entry.
        auto lastCreature = lastEntryCreature.find(entry);
        if (lastCreature != lastEntryCreature.end())
        {
            if (data.mapid == lastCreature->second->mapid)
            {
                float dx1 = lastCreature->second->posX - data.posX;
                float dy1 = lastCreature->second->posY - data.posY;
                float dz1 = lastCreature->second->posZ - data.posZ;

                float distsq1 = dx1*dx1 + dy1*dy1 + dz1*dz1;
                if (distsq1 < 0.5f)
                {
                    // split phaseID
                    for (auto phaseID : data.PhaseID)
                        lastCreature->second->PhaseID.insert(phaseID);

                    lastCreature->second->phaseMask |= data.phaseMask;
                    lastCreature->second->spawnMask |= data.spawnMask;
                    WorldDatabase.PExecute("UPDATE conversation SET phaseMask = %u, spawnMask = " UI64FMTD " WHERE guid = %u", lastCreature->second->phaseMask, lastCreature->second->spawnMask, lastCreature->second->guid);
                    WorldDatabase.PExecute("DELETE FROM conversation WHERE guid = %u", guid);
                    TC_LOG_ERROR(LOG_FILTER_SQL, "ConversationDataStoreMgr::LoadConversations() >> Table `conversation` have clone npc %u witch stay too close (dist: %f). original npc guid %u. npc with guid %u will be deleted.", entry, distsq1, lastCreature->second->guid, guid);
                    continue;
                }
            }
            else
                lastEntryCreature[entry] = &data;

        }
        else
            lastEntryCreature[entry] = &data;

        if (!sMapStore.LookupEntry(data.mapid))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadConversations >> Table `conversation` have conversation (GUID: " UI64FMTD ") that spawned at not existed map (Id: %u), skipped.", guid, data.mapid);
            continue;
        }

        if (data.spawnMask & ~spawnMasks[data.mapid])
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadConversations >> Table `conversation` have conversation (GUID: " UI64FMTD ") that have wrong spawn mask " UI64FMTD " including not supported difficulty modes for map (Id: %u) spawnMasks[data.mapid]: %u.", guid, data.spawnMask, data.mapid, spawnMasks[data.mapid]);
            WorldDatabase.PExecute("UPDATE conversation SET spawnMask = " UI64FMTD " WHERE guid = %u", spawnMasks[data.mapid], guid);
            data.spawnMask = spawnMasks[data.mapid];
        }

        if (data.phaseMask == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadConversations >> Table `conversation` have conversation (GUID: " UI64FMTD " Entry: %u) with `phaseMask`=0 (not visible for anyone), set to 1.", guid, data.id);
            data.phaseMask = 1;
        }

        // Add to grid if not managed by the game event or pool system
        sObjectMgr->AddConversationToGrid(guid, &data);

        if (!data.zoneId || !data.areaId)
        {
            uint32 zoneId = 0;
            uint32 areaId = 0;

            sMapMgr->GetZoneAndAreaId(zoneId, areaId, data.mapid, data.posX, data.posY, data.posZ);

            data.zoneId = zoneId;
            data.areaId = areaId;

            WorldDatabase.PExecute("UPDATE conversation SET zoneId = %u, areaId = %u WHERE guid = %u", zoneId, areaId, guid);
        }

        ++count;

    } while (result->NextRow());
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversations() >> Loaded %u conversation in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ConversationDataStoreMgr::LoadConversationData()
{
    _conversationDataList.clear();
    _conversationCreatureList.clear();
    _conversationActorList.clear();

    //                                                  0       1       2       3       4       5
    QueryResult result = WorldDatabase.Query("SELECT `entry`, `id`, `textId`, `unk1`, `unk2`, `idx` FROM `conversation_data` ORDER BY idx");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            ConversationData data;
            data.entry = fields[i++].GetUInt32();
            data.id = fields[i++].GetUInt32();
            data.textId = fields[i++].GetUInt32();
            data.unk1 = fields[i++].GetUInt32();
            data.unk2 = fields[i++].GetUInt32();
            data.idx = fields[i++].GetUInt32();
            _conversationDataList[data.entry].push_back(data);
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversationData() >> Loaded %u conversation data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversationData() >> Loaded 0 conversation data. DB table `conversation_data` is empty.");

    //                                      0      1         2               3          4       5         6
    result = WorldDatabase.Query("SELECT `entry`, `id`, `creatureId`, `creatureGuid`, `unk1`, `unk2`, `duration` FROM `conversation_creature` ORDER BY id");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            ConversationCreature data;
            data.entry = fields[i++].GetUInt32();
            data.id = fields[i++].GetUInt32();
            data.creatureId = fields[i++].GetUInt32();
            data.creatureGuid = fields[i++].GetUInt32();
            data.unk1 = fields[i++].GetUInt32();
            data.unk2 = fields[i++].GetUInt32();
            data.duration = fields[i++].GetUInt32();
            _conversationCreatureList[data.entry].push_back(data);
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversationData() >> Loaded %u conversation creature data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversationData() >> Loaded 0 conversation creature data. DB table `conversation_creature` is empty.");


    //                                      0      1        2           3             4         5       6       7         8
    result = WorldDatabase.Query("SELECT `entry`, `id`, `actorId`, `creatureId`, `displayId`, `unk1`, `unk2`, `unk3`, `duration` FROM `conversation_actor` ORDER BY id");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            ConversationActor data;
            data.entry = fields[i++].GetUInt32();
            data.id = fields[i++].GetUInt32();
            data.actorId = fields[i++].GetUInt32();
            data.creatureId = fields[i++].GetUInt32();
            data.displayId = fields[i++].GetUInt32();
            data.unk1 = fields[i++].GetUInt32();
            data.unk2 = fields[i++].GetUInt32();
            data.unk3 = fields[i++].GetUInt32();
            data.duration = fields[i++].GetUInt32();
            _conversationActorList[data.entry].push_back(data);
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversationData() >> Loaded %u conversation actor data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "ConversationDataStoreMgr::LoadConversationData() >> Loaded 0 conversation actor data. DB table `conversation_actor` is empty.");
}

ConversationSpawnData const* ConversationDataStoreMgr::GetConversationData(ObjectGuid::LowType const& guid) const
{
    return Trinity::Containers::MapGetValuePtr(_conversationDataStore, guid);
}

ConversationSpawnData& ConversationDataStoreMgr::NewOrExistConversationData(ObjectGuid::LowType const& guid)
{
    return _conversationDataStore[guid];
}

const std::vector<ConversationData>* ConversationDataStoreMgr::GetConversationData(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_conversationDataList, entry);
}

const std::vector<ConversationCreature>* ConversationDataStoreMgr::GetConversationCreature(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_conversationCreatureList, entry);
}

const std::vector<ConversationActor>* ConversationDataStoreMgr::GetConversationActor(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_conversationActorList, entry);
}
