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

#ifndef ZONE_SCRIPT_H_
#define ZONE_SCRIPT_H_

#include "Define.h"
#include "ObjectGuid.h"

class Creature;
class GameObject;
class InstanceScript;
class Player;
class Unit;
class WorldObject;

struct CreatureData;

enum ZoneType
{
    ZONE_TYPE_MAP       = 0,
    ZONE_TYPE_INSTANCE  = 1,
};

class ZoneScript
{
        uint8 m_type;
    public:
        ZoneScript();
        virtual ~ZoneScript() = default;

        void SetType(uint8 type);
        virtual uint32 GetCreatureEntry(uint32 /*guidlow*/, CreatureData const* data);
        virtual uint32 GetGameObjectEntry(uint32 /*guidlow*/, uint32 entry) { return entry; }

        virtual void OnCreatureCreate(Creature* /*creature*/) {}
        virtual void OnCreatureRemove(Creature* /*creature*/) {}
        virtual void CreatureDies(Creature* /*creature*/, Unit* /*killer*/) {}
        virtual void OnGameObjectCreate(GameObject* /*go*/) {}
        virtual void OnGameObjectRemove(GameObject* /*go*/) {}

        virtual void OnCreatureCreateForScript(Creature* /*creature*/) {}
        virtual void OnCreatureRemoveForScript(Creature* /*creature*/) {}
        virtual void OnCreatureUpdateDifficulty(Creature* /*creature*/) {}
        virtual void EnterCombatForScript(Creature* /*creature*/, Unit* /*enemy*/) {}
        virtual void CreatureDiesForScript(Creature* /*creature*/, Unit* /*killer*/) {}
        virtual void OnGameObjectCreateForScript(GameObject* /*go*/) {}
        virtual void OnGameObjectRemoveForScript(GameObject* /*go*/) {}

        virtual void OnUnitDeath(Unit* /*unit*/) {}
        virtual void OnUnitCharmed(Unit* /*unit*/, Unit* /*charmer*/) {}
        virtual void OnUnitRemoveCharmed(Unit* /*unit*/, Unit* /*charmer*/) {}

        //All-purpose data storage 64 bit
        virtual ObjectGuid GetGuidData(uint32 /*DataId*/) const { return ObjectGuid::Empty; }
        virtual void SetGuidData(uint32 /*DataId*/, ObjectGuid /*Value*/) { }

        virtual uint64 GetData64(uint32 /*DataId*/) { return 0; }
        virtual void SetData64(uint32 /*DataId*/, uint64 /*Value*/) {}

        //All-purpose data storage 32 bit
        virtual uint32 GetData(uint32 /*DataId*/) const { return 0; }
        virtual void SetData(uint32 /*DataId*/, uint32 /*Value*/) {}

        virtual void ProcessEvent(WorldObject* /*obj*/, uint32 /*eventId*/) {}

        InstanceScript* ToInstanceScript();
};

#endif
