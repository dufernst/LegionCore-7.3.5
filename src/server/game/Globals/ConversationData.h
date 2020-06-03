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

#ifndef _ConversationDataStoreh_
#define _ConversationDataStoreh_

#include "Conversation.h"

struct ConversationActor
{
    uint32 entry;
    uint32 id;
    uint32 actorId;
    uint32 creatureId;
    uint32 displayId;
    uint32 unk1;
    uint32 unk2;
    uint32 unk3;
    uint32 duration;
};

struct ConversationCreature
{
    uint32 entry;
    uint32 id;
    uint32 creatureId;
    uint32 creatureGuid;
    uint32 unk1;
    uint32 unk2;
    uint32 duration;
};

struct ConversationData
{
    uint32 entry;
    uint32 id;
    uint32 idx;
    uint32 textId;
    uint32 unk1;
    uint32 unk2;
};

typedef std::unordered_map<uint32/*entry*/, std::vector<ConversationData> > ConversationDataMap;
typedef std::unordered_map<uint32/*entry*/, std::vector<ConversationCreature> > ConversationCreatureMap;
typedef std::unordered_map<uint32/*entry*/, std::vector<ConversationActor> > ConversationActorMap;
typedef std::unordered_map<ObjectGuid::LowType, ConversationSpawnData> ConversationDataContainer;

class ConversationDataStoreMgr
{
    ConversationDataStoreMgr();
    ~ConversationDataStoreMgr();

public:
    static ConversationDataStoreMgr* instance();

    void LoadConversations();
    void LoadConversationData();

    ConversationSpawnData const* GetConversationData(ObjectGuid::LowType const& guid) const;
    ConversationSpawnData& NewOrExistConversationData(ObjectGuid::LowType const& guid);

    std::vector<ConversationData> const* GetConversationData(uint32 entry) const;
    std::vector<ConversationCreature> const* GetConversationCreature(uint32 entry) const;
    std::vector<ConversationActor> const* GetConversationActor(uint32 entry) const;
private:
    ConversationDataContainer _conversationDataStore;
    ConversationDataMap _conversationDataList;
    ConversationCreatureMap _conversationCreatureList;
    ConversationActorMap _conversationActorList;
};

#define sConversationDataStore ConversationDataStoreMgr::instance()

#endif
