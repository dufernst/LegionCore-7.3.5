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

#ifndef _SpellTargetInfoH
#define _SpellTargetInfoH

#include "SharedDefines.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"

class Unit;
class Player;
class GameObject;
class DynamicObject;
class WorldObject;
class Aura;
class SpellScript;
class ByteBuffer;

namespace WorldPackets
{
    namespace Spells
    {
        struct SpellCastRequest;
        struct SpellTargetData;
    }
}

enum SpellRangeFlag
{
    SPELL_RANGE_DEFAULT             = 0,
    SPELL_RANGE_MELEE               = 1,     //melee
    SPELL_RANGE_RANGED              = 2,     //hunter range and ranged weapon
};

struct SpellDestination
{
    SpellDestination();
    SpellDestination(float x, float y, float z, float orientation = 0.0f, uint32 mapId = MAPID_INVALID);
    SpellDestination(Position const& pos);
    SpellDestination(WorldObject const& wObj);

    WorldLocation _position;
    ObjectGuid _transportGUID;
    Position _transportOffset;
};

enum TargetInfoMask
{
    TARGET_INFO_ALIVE          = 0x00000001,
    TARGET_INFO_CRIT           = 0x00000002,
    TARGET_INFO_IS_JUMP_TARGET = 0x00000004,
};

struct TargetInfo
{
    TargetInfo(ObjectGuid tGUID, uint32  effMask);
    TargetInfo();

    ObjectGuid targetGUID;
    uint64 timeDelay;
    SpellMissInfo missCondition;
    SpellMissInfo reflectResult;
    uint32 effectMask : 32;
    int32 damage;
    int32 damageBeforeHit;
    bool processed : 1;
    bool scaleAura : 1;

    bool HasMask(uint32 Mask) const;
    uint32 GetMask() const;
    void AddMask(uint32 Mask);

private:
    uint32 targetInfoMask;
};

typedef std::shared_ptr<TargetInfo> TargetInfoPtr;

enum WeightType
{
    WEIGHT_FRAGMENT = 1,
    WEIGHT_KEYSTONE = 2,
};

struct ArchaeologyWeight
{
    uint8 type;
    union
    {
        struct
        {
            uint32 currencyId;
            uint32 currencyCount;
        } fragment;
        struct
        {
            uint32 itemId;
            uint32 itemCount;
        } keystone;
        struct
        {
            uint32 id;
            uint32 count;
        } raw;
    };
};

typedef std::vector<ArchaeologyWeight> ArchaeologyWeights;

class SpellCastTargets
{
    friend class Spell;
    friend class WorldSession;
    friend class Player;

    ArchaeologyWeights m_weights;
    WorldObject* m_objectTarget;
    Unit* m_caster;
    Item* m_itemTarget;
    ObjectGuid m_itemTargetGUID;
    ObjectGuid m_objectTargetGUID;
    SpellDestination m_dst;
    SpellDestination m_src;
    uint32 m_itemTargetEntry;
    uint32 m_targetMask;
    float m_pitch;
    float m_speed;
    std::string m_strTarget;
public:
    SpellCastTargets();
    SpellCastTargets(Unit* caster, WorldPackets::Spells::SpellCastRequest const& spellCastRequest);
    ~SpellCastTargets();

    void Write(WorldPackets::Spells::SpellTargetData& data);

    uint32 GetTargetMask() const;
    void SetTargetMask(uint32 newMask);
    void SetTargetFlag(SpellCastTargetFlags flag);

    ObjectGuid GetUnitTargetGUID() const;
    Unit* GetUnitTarget() const;
    void SetUnitTarget(Unit* target);

    ObjectGuid GetGOTargetGUID() const;
    GameObject* GetGOTarget() const;
    void SetGOTarget(GameObject* target);

    ObjectGuid GetCorpseTargetGUID() const;
    Corpse* GetCorpseTarget() const;

    WorldObject* GetObjectTarget() const;
    ObjectGuid GetObjectTargetGUID() const;
    void RemoveObjectTarget();

    ObjectGuid GetItemTargetGUID() const;
    Item* GetItemTarget() const;
    uint32 GetItemTargetEntry() const;
    void SetItemTarget(Item* item);
    void SetTradeItemTarget(Player* caster);
    void UpdateTradeSlotItem();
    SpellDestination const* GetSrc() const;
    Position const* GetSrcPos() const;
    void SetSrc(float x, float y, float z);
    void SetSrc(Position const& pos);
    void SetSrc(WorldObject const& wObj);
    void ModSrc(Position const& pos);
    void RemoveSrc();

    SpellDestination const* GetDst() const;
    WorldLocation const* GetDstPos() const;
    void SetDst(float x, float y, float z, float orientation, uint32 mapId = MAPID_INVALID);
    void SetDst(Position const& pos);
    void SetDst(WorldObject const& wObj);
    void SetDst(SpellCastTargets const& spellTargets);
    void ModDst(Position const& pos);
    void RemoveDst();

    bool HasSrc() const;
    bool HasDst() const;
    bool HasTraj() const;

    float GetPitch() const;
    void SetPitch(float elevation);
    float GetSpeed() const;
    void SetSpeed(float speed);

    float GetDist2d() const;
    float GetSpeedXY() const;
    float GetSpeedZ() const;

    ArchaeologyWeights const& GetWeights();

    void Update(Unit* caster);
    void OutDebug() const;

    void SetCaster(Unit* caster);
};

#endif
