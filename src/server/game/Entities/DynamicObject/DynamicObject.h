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

#ifndef DYNAMICOBJECT_H
#define DYNAMICOBJECT_H

#include "Object.h"
#include "GridObject.h"
#include "MapObject.h"

class Unit;
class Aura;
class SpellInfo;

enum DynamicObjectType
{
    DYNAMIC_OBJECT_PORTAL           = 0x0,      // unused
    DYNAMIC_OBJECT_AREA_SPELL       = 0x1,
    DYNAMIC_OBJECT_FARSIGHT_FOCUS   = 0x2,
};

class DynamicObject : public WorldObject, public GridObject<DynamicObject>, public MapObject
{
    public:
        DynamicObject(bool isWorldObject);
        ~DynamicObject();

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool CreateDynamicObject(ObjectGuid::LowType guidlow, Unit* caster, uint32 spellId, Position const& pos, float radius, DynamicObjectType type);
        void Update(uint32 p_time) override;
        void Remove();
        void SetDuration(int32 newDuration);
        int32 GetDuration() const;
        void Delay(int32 delaytime);
        void SetAura(Aura* aura);
        void RemoveAura();
        void SetCasterViewpoint();
        void RemoveCasterViewpoint();
        Unit* GetCaster() const { return _caster; }
        void BindToCaster();
        void UnbindFromCaster();
        uint32 GetSpellId() const {  return GetUInt32Value(DYNAMICOBJECT_FIELD_SPELL_ID); }
        ObjectGuid GetCasterGUID() const { return GetGuidValue(DYNAMICOBJECT_FIELD_CASTER); }
        float GetRadius() const { return GetFloatValue(DYNAMICOBJECT_FIELD_RADIUS); }
        DynamicObjectType GetType() const { return DynamicObjectType(GetUInt32Value(DYNAMICOBJECT_FIELD_TYPE)); }
        uint32 GetVisualId() const { return GetUInt32Value(DYNAMICOBJECT_FIELD_SPELL_XSPELL_VISUAL_ID); }

        void Say(int32 textId, uint32 language, ObjectGuid targetGuid) { MonsterSay(textId, language, targetGuid); }
        void Yell(int32 textId, uint32 language, ObjectGuid targetGuid) { MonsterYell(textId, language, targetGuid); }
        void TextEmote(int32 textId, ObjectGuid targetGuid) { MonsterTextEmote(textId, targetGuid); }
        void Whisper(int32 textId, ObjectGuid receiver) { MonsterWhisper(textId, receiver); }
        void YellToZone(int32 textId, uint32 language, ObjectGuid targetGuid) { MonsterYellToZone(textId, language, targetGuid); }

    protected:
        Aura* _aura;
        Aura* _removedAura;
        Unit* _caster;
        int32 _duration; // for non-aura dynobjects
        bool _isViewpoint;
};
#endif
