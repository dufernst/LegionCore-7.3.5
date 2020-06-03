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

#ifndef CONVERSATIONOBJECT_H
#define CONVERSATIONOBJECT_H

#include "Object.h"
#include "GridObject.h"

class SpellInfo;

struct ConversationSpawnData
{
    explicit ConversationSpawnData();

    std::set<uint32> PhaseID;
    ObjectGuid::LowType guid = 0;
    uint32 id;                                              // entry in creature_template
    uint32 phaseMask = 1;
    uint64 spawnMask = 1;
    float posX;
    float posY;
    float posZ;
    float orientation;
    uint16 mapid;
    uint16 zoneId = 0;
    uint16 areaId = 0;
    bool dbData;
};

#pragma pack(push, 1)
struct ConversationDynamicFieldActors
{
    uint32 actorId;
    uint32 creatureId;
    uint32 displayId;
    uint32 unk1; // This is byte ot shot data need info from client about field
    uint32 unk2; // This is byte ot shot data need info from client about field
    uint32 unk3; // This is byte ot shot data need info from client about field
};

struct ConversationDynamicFieldCreatures
{
    uint64 GuidLow;
    uint64 GuidHight;
    uint32 unk1; // This is byte ot shot data need info from client about field
    uint32 unk2; // This is byte ot shot data need info from client about field
};

struct ConversationDynamicFieldLines
{
    uint32 id;
    uint32 textId;
    uint32 unk1; // This is byte ot shot data need info from client about field
    uint32 unk2; // This is byte ot shot data need info from client about field
};
#pragma pack(pop)

class Conversation : public WorldObject, public GridObject<Conversation>
{
    public:
        Conversation();
        ~Conversation();

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool CanNeverSee2(WorldObject const* seer) const override;

        bool LoadFromDB(ObjectGuid::LowType guid, Map* map);
        bool LoadConversationFromDB(ObjectGuid::LowType guid, Map* map, bool addToMap = true);

        bool CreateConversation(ObjectGuid::LowType guidlow, uint32 triggerEntry, Unit* caster, SpellInfo const* info, Position const& pos);
        void Update(uint32 p_time) override;
        void Remove();
        void SetDuration(int32 newDuration) { _duration = newDuration; }
        int32 GetDuration() const { return _duration; }

        Unit* GetCaster() const { return _caster; }
        void BindToCaster();
        void UnbindFromCaster();
        uint32 GetSpellId() const { return _spellId; }
        ObjectGuid GetCasterGUID() const { return casterGUID; }

        void BuildDynamicValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        bool GenerateDynamicFieldData(uint32 triggerEntry, WorldObject* sercher, uint32& duration);

        DynamicFieldStructuredView<ConversationDynamicFieldActors> GetActors() const;
        ConversationDynamicFieldActors const* GetActor(uint8 index) const;
        void SetActor(ConversationDynamicFieldActors const* actorInfo, uint8 index, bool createIfMissing = false);

        DynamicFieldStructuredView<ConversationDynamicFieldCreatures> GetCreatures() const;
        ConversationDynamicFieldCreatures const* GetCreature(uint8 index) const;
        void SetCreature(ConversationDynamicFieldCreatures const* creatureInfo, uint8 index, bool createIfMissing = false);

        DynamicFieldStructuredView<ConversationDynamicFieldLines> GetLines() const;
        ConversationDynamicFieldLines const* GetLine(uint8 index) const;
        void SetLine(ConversationDynamicFieldLines const* lineInfo, uint8 index, bool createIfMissing = false);

    protected:
        Unit* _caster;
        uint32 _spellId;
        uint32 _duration;
        ObjectGuid casterGUID;

        std::map<ObjectGuid, uint32> playing;
};
#endif
