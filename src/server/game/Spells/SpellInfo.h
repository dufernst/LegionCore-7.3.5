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

#ifndef _SPELLINFO_H
#define _SPELLINFO_H

#include "SharedDefines.h"
#include "Util.h"
#include "DB2Structure.h"
#include "Object.h"

class Unit;
class Player;
class Item;
class Spell;
class SpellInfo;
struct SpellChainNode;
struct SpellTargetPosition;
struct SpellDurationEntry;
struct SpellModifier;
struct SpellRangeEntry;
struct SpellRadiusEntry;
struct SpellEntry;
struct SpellCastTimesEntry;
struct Condition;

#define MAX_EFFECT_MASK 4294967295
#define MAX_POWERS_FOR_SPELL 4
#define MAX_CREATURE_SPELL_DATA_SLOT 4

enum SpellCastTargetFlags
{
    TARGET_FLAG_NONE            = 0x00000000,
    TARGET_FLAG_UNUSED_1        = 0x00000001,               // not used
    TARGET_FLAG_UNIT            = 0x00000002,               // pguid
    TARGET_FLAG_UNIT_RAID       = 0x00000004,               // not sent, used to validate target (if raid member)
    TARGET_FLAG_UNIT_PARTY      = 0x00000008,               // not sent, used to validate target (if party member)
    TARGET_FLAG_ITEM            = 0x00000010,               // pguid
    TARGET_FLAG_SOURCE_LOCATION = 0x00000020,               // pguid, 3 float
    TARGET_FLAG_DEST_LOCATION   = 0x00000040,               // pguid, 3 float
    TARGET_FLAG_UNIT_ENEMY      = 0x00000080,               // not sent, used to validate target (if enemy)
    TARGET_FLAG_UNIT_ALLY       = 0x00000100,               // not sent, used to validate target (if ally)
    TARGET_FLAG_CORPSE_ENEMY    = 0x00000200,               // pguid
    TARGET_FLAG_UNIT_DEAD       = 0x00000400,               // not sent, used to validate target (if dead creature)
    TARGET_FLAG_GAMEOBJECT      = 0x00000800,               // pguid, used with TARGET_GAMEOBJECT_TARGET
    TARGET_FLAG_TRADE_ITEM      = 0x00001000,               // pguid
    TARGET_FLAG_STRING          = 0x00002000,               // string
    TARGET_FLAG_GAMEOBJECT_ITEM = 0x00004000,               // not sent, used with TARGET_GAMEOBJECT_ITEM_TARGET
    TARGET_FLAG_CORPSE_ALLY     = 0x00008000,               // pguid
    TARGET_FLAG_UNIT_MINIPET    = 0x00010000,               // pguid, used to validate target (if non combat pet)
    TARGET_FLAG_GLYPH_SLOT      = 0x00020000,               // used in glyph spells
    TARGET_FLAG_DEST_TARGET     = 0x00040000,               // sometimes appears with DEST_TARGET spells (may appear or not for a given spell)
    TARGET_FLAG_EXTRA_TARGETS   = 0x00080000,               // uint32 counter, loop { vec3 - screen position (?), guid }, not used so far
    TARGET_FLAG_FOLLOWER        = 0x00100000,               // select follower
    TARGET_FLAG_ITEM_HEIRLOOM   = 0x00200000,               // item
    TARGET_FLAG_GARRISON_MISSION= 0x00400000,               // garrison mission
    TARGET_FLAG_UNIT_PASSENGER  = 0x10000000,               // guessed, used to validate target (if vehicle passenger) flags is custom

    TARGET_FLAG_UNIT_MASK = TARGET_FLAG_UNIT | TARGET_FLAG_UNIT_RAID | TARGET_FLAG_UNIT_PARTY
        | TARGET_FLAG_UNIT_ENEMY | TARGET_FLAG_UNIT_ALLY | TARGET_FLAG_UNIT_DEAD | TARGET_FLAG_UNIT_MINIPET | TARGET_FLAG_UNIT_PASSENGER,
    TARGET_FLAG_GAMEOBJECT_MASK = TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_GAMEOBJECT_ITEM,
    TARGET_FLAG_CORPSE_MASK = TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_CORPSE_ENEMY,
    TARGET_FLAG_ITEM_MASK = TARGET_FLAG_TRADE_ITEM | TARGET_FLAG_ITEM | TARGET_FLAG_GAMEOBJECT_ITEM,
};

enum SpellTargetSelectionCategories
{
    TARGET_SELECT_CATEGORY_NYI,
    TARGET_SELECT_CATEGORY_DEFAULT,
    TARGET_SELECT_CATEGORY_CHANNEL,
    TARGET_SELECT_CATEGORY_NEARBY,
    TARGET_SELECT_CATEGORY_CONE,
    TARGET_SELECT_CATEGORY_AREA,
    TARGET_SELECT_CATEGORY_BETWEEN,
    TARGET_SELECT_CATEGORY_GOTOMOVE,
    TARGET_SELECT_CATEGORY_THREAD,

    TARGET_SELECT_CATEGORY_MAX
};

enum SpellTargetReferenceTypes
{
    TARGET_REFERENCE_TYPE_NONE,
    TARGET_REFERENCE_TYPE_CASTER,
    TARGET_REFERENCE_TYPE_TARGET,
    TARGET_REFERENCE_TYPE_LAST,
    TARGET_REFERENCE_TYPE_SRC,
    TARGET_REFERENCE_TYPE_DEST,
    TARGET_REFERENCE_TYPE_NEAR_DEST,
};

enum SpellTargetObjectTypes
{
    TARGET_OBJECT_TYPE_NONE = 0,
    TARGET_OBJECT_TYPE_SRC,
    TARGET_OBJECT_TYPE_DEST,
    TARGET_OBJECT_TYPE_UNIT,
    TARGET_OBJECT_TYPE_UNIT_AND_DEST,
    TARGET_OBJECT_TYPE_OBJ_AND_DEST,
    TARGET_OBJECT_TYPE_GOBJ,
    TARGET_OBJECT_TYPE_GOBJ_ITEM,
    TARGET_OBJECT_TYPE_ITEM,
    TARGET_OBJECT_TYPE_CORPSE,
    // only for effect target type
    TARGET_OBJECT_TYPE_CORPSE_ENEMY,
    TARGET_OBJECT_TYPE_CORPSE_ALLY,
    TARGET_OBJECT_TYPE_PLAYER,
};

enum SpellTargetCheckTypes
{
    TARGET_CHECK_DEFAULT,
    TARGET_CHECK_ENTRY,
    TARGET_CHECK_ENEMY,
    TARGET_CHECK_ALLY,
    TARGET_CHECK_PARTY,
    TARGET_CHECK_RAID,
    TARGET_CHECK_RAID_CLASS,
    TARGET_CHECK_PASSENGER,
    TARGET_CHECK_SUMMON,
};

enum SpellTargetDirectionTypes
{
    TARGET_DIR_NONE,
    TARGET_DIR_FRONT,
    TARGET_DIR_BACK,
    TARGET_DIR_RIGHT,
    TARGET_DIR_LEFT,
    TARGET_DIR_FRONT_RIGHT,
    TARGET_DIR_BACK_RIGHT,
    TARGET_DIR_BACK_LEFT,
    TARGET_DIR_FRONT_LEFT,
    TARGET_DIR_RANDOM,
    TARGET_DIR_ENTRY,
    TARGET_DIR_TARGET,
};

enum SpellEffectImplicitTargetTypes
{
    EFFECT_IMPLICIT_TARGET_NONE = 0,
    EFFECT_IMPLICIT_TARGET_EXPLICIT,
    EFFECT_IMPLICIT_TARGET_CASTER,
};

// Spell clasification
enum SpellSpecificType
{
    SPELL_SPECIFIC_NORMAL                        = 0,
    SPELL_SPECIFIC_BLESSING                      = 2,
    SPELL_SPECIFIC_AURA                          = 3,
    SPELL_SPECIFIC_CURSE                         = 5,
    SPELL_SPECIFIC_TRACKER                       = 7,
    SPELL_SPECIFIC_ELEMENTAL_SHIELD              = 10,
    SPELL_SPECIFIC_MAGE_POLYMORPH                = 11,
    SPELL_SPECIFIC_WELL_FED                      = 18,
    SPELL_SPECIFIC_FOOD                          = 19,
    SPELL_SPECIFIC_DRINK                         = 20,
    SPELL_SPECIFIC_FOOD_AND_DRINK                = 21,
    SPELL_SPECIFIC_CHARM                         = 23,
    SPELL_SPECIFIC_SCROLL                        = 24,
    SPELL_SPECIFIC_WARRIOR_ENRAGE                = 26,
    SPELL_SPECIFIC_PRIEST_DIVINE_SPIRIT          = 27,
    SPELL_SPECIFIC_HAND                          = 28,
    SPELL_SPECIFIC_PHASE                         = 29,
    SPELL_SPECIFIC_BANE                          = 30,
};

enum SpellCustomAttributes
{
    SPELL_ATTR0_CU_ENCHANT_PROC                  = 0x00000001,
    SPELL_ATTR0_CU_CONE_BACK                     = 0x00000002,
    SPELL_ATTR0_CU_CONE_LINE                     = 0x00000004,
    SPELL_ATTR0_CU_SHARE_DAMAGE                  = 0x00000008,
    SPELL_ATTR0_CU_NO_INITIAL_THREAT             = 0x00000010,
    SPELL_ATTR0_CU_POSITIVE_FOR_CASTER           = 0x00000020,
    SPELL_ATTR0_CU_AURA_CC                       = 0x00000040,
    SPELL_ATTR0_CU_IS_STEALTH_AURA               = 0x00000080,
    SPELL_ATTR0_CU_DIRECT_DAMAGE                 = 0x00000100,
    SPELL_ATTR0_CU_CHARGE                        = 0x00000200,
    SPELL_ATTR0_CU_PICKPOCKET                    = 0x00000400,
    SPELL_ATTR0_CU_CANT_BE_SAVED_IN_DB           = 0x00000800,
    SPELL_ATTR0_CU_NEGATIVE_EFF0                 = 0x00001000,
    SPELL_ATTR0_CU_NEGATIVE_EFF1                 = 0x00002000,
    SPELL_ATTR0_CU_NEGATIVE_EFF2                 = 0x00004000,
    SPELL_ATTR0_CU_IGNORE_ARMOR                  = 0x00008000,
    SPELL_ATTR0_CU_REQ_TARGET_FACING_CASTER      = 0x00010000,
    SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET      = 0x00020000,
    SPELL_ATTR0_CU_NEGATIVE_EFF3                 = 0x00040000,
    SPELL_ATTR0_CU_NEGATIVE_EFF4                 = 0x00080000,
    SPELL_ATTR0_CU_REQ_CASTER_NOT_FRONT_TARGET   = 0x00100000,
    SPELL_ATTR0_CU_CAN_BE_CASTED_ON_ALLIES       = 0x00200000,
    SPELL_ATTR0_CU_NEED_BE_SAVED_IN_DB           = 0x00400000,
    SPELL_ATTR0_CU_PROC_ONLY_ON_CAST             = 0x00800000,
    SPELL_ATTR0_CU_CANT_BE_LEARNED               = 0x01000000,
    SPELL_ATTR0_CU_NO_COMBAT                     = 0x02000000,
    SPELL_ATTR0_CU_ALWAYS_RESET_TIMER_AND_TICK   = 0x04000000,
    SPELL_ATTR0_CU_EXTRA_PVP_STAT_SPELL          = 0x08000000,
    SPELL_ATTR0_CU_CAST_DIRECTLY                 = 0x10000000,
    SPELL_ATTR0_CU_IGNORE_AVOID_MECHANIC         = 0x20000000, //Ignore Immunity and Parry/Dodge...etc.
    SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE        = 0x40000000,
    SPELL_ATTR0_CU_BONUS_FROM_ORIGINAL_CASTER    = 0x80000000,

    SPELL_ATTR0_CU_NEGATIVE                      = SPELL_ATTR0_CU_NEGATIVE_EFF0 | SPELL_ATTR0_CU_NEGATIVE_EFF1 | SPELL_ATTR0_CU_NEGATIVE_EFF2,
};

enum SpellCustomAttributes1
{
    SPELL_ATTR1_CU_IS_USING_STACKS         = 0x00000001,
    SPELL_ATTR1_CU_CANT_USE_WHEN_ROOTED    = 0x00000002,
    SPELL_ATTR1_CU_USE_PATH_CHECK_FOR_CAST = 0x00000004,
};

enum SpellTypes
{
    SPELL_TYPE_MELEE                       = 0x00000001,
    SPELL_TYPE_RANGE                       = 0x00000002,
    SPELL_TYPE_AOE                         = 0x00000004,
    SPELL_TYPE_CHANELED                    = 0x00000008,
    SPELL_TYPE_AUTOREPEATE                 = 0x00000010,
};

enum SpellAuraDummyType
{
    SPELL_DUMMY_DEFAULT                         = 0,
    SPELL_DUMMY_DAMAGE                          = 1,
    SPELL_DUMMY_CRIT                            = 2,
    SPELL_DUMMY_TIME                            = 3,
    SPELL_DUMMY_PROC                            = 4,
    SPELL_DUMMY_DURATION                        = 5,
};

enum SpellAuraInterruptFlags : uint32
{
    AURA_INTERRUPT_FLAG_HITBYSPELL          = 0x00000001,   // 0    removed when getting hit by a negative spell?
    AURA_INTERRUPT_FLAG_TAKE_DAMAGE         = 0x00000002,   // 1    removed by any damage
    AURA_INTERRUPT_FLAG_CAST                = 0x00000004,   // 2    cast any spells
    AURA_INTERRUPT_FLAG_MOVE                = 0x00000008,   // 3    removed by any movement
    AURA_INTERRUPT_FLAG_TURNING             = 0x00000010,   // 4    removed by any turning
    AURA_INTERRUPT_FLAG_JUMP                = 0x00000020,   // 5    removed by entering combat
    AURA_INTERRUPT_FLAG_NOT_MOUNTED         = 0x00000040,   // 6    removed by dismounting
    AURA_INTERRUPT_FLAG_NOT_ABOVEWATER      = 0x00000080,   // 7    removed by entering water
    AURA_INTERRUPT_FLAG_NOT_UNDERWATER      = 0x00000100,   // 8    removed by leaving water
    AURA_INTERRUPT_FLAG_NOT_SHEATHED        = 0x00000200,   // 9    removed by unsheathing
    AURA_INTERRUPT_FLAG_TALK                = 0x00000400,   // 10   talk to npc / loot? action on creature
    AURA_INTERRUPT_FLAG_USE                 = 0x00000800,   // 11   mine/use/open action on gameobject
    AURA_INTERRUPT_FLAG_MELEE_ATTACK        = 0x00001000,   // 12   removed by attacking
    AURA_INTERRUPT_FLAG_SPELL_ATTACK        = 0x00002000,   // 13   ???
    AURA_INTERRUPT_FLAG_UNK14               = 0x00004000,   // 14
    AURA_INTERRUPT_FLAG_TRANSFORM           = 0x00008000,   // 15   removed by transform?
    AURA_INTERRUPT_FLAG_UNK16               = 0x00010000,   // 16
    AURA_INTERRUPT_FLAG_MOUNT               = 0x00020000,   // 17   misdirect, aspect, swim speed
    AURA_INTERRUPT_FLAG_NOT_SEATED          = 0x00040000,   // 18   removed by standing up (used by food and drink mostly and sleep/Fake Death like)
    AURA_INTERRUPT_FLAG_CHANGE_MAP          = 0x00080000,   // 19   leaving map/getting teleported
    AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION    = 0x00100000,   // 20   removed by auras that make you invulnerable, or make other to lose selection on you
    AURA_INTERRUPT_FLAG_UNK21               = 0x00200000,   // 21
    AURA_INTERRUPT_FLAG_TELEPORTED          = 0x00400000,   // 22
    AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT    = 0x00800000,   // 23   removed by entering pvp combat
    AURA_INTERRUPT_FLAG_DIRECT_DAMAGE       = 0x01000000,   // 24   removed by any direct damage
    AURA_INTERRUPT_FLAG_LANDING             = 0x02000000,   // 25   removed by hitting the ground
    AURA_INTERRUPT_FLAG_UNK26               = 0x04000000,   // 26
    AURA_INTERRUPT_FLAG_TAKE_DAMAGE2        = 0x08000000,   // 27   aura who remove from limit damage by proc (fear, confuse, stun, root, transform)
    AURA_INTERRUPT_FLAG_ENTER_COMBAT        = 0x10000000,   // 28   remove aura on enter combat
    AURA_INTERRUPT_FLAG_UNK29               = 0x20000000,   // 29
    AURA_INTERRUPT_FLAG_UNK30               = 0x40000000,   // 30
    AURA_INTERRUPT_FLAG_LEAVE_COMBAT        = 0x80000000,   // 31
};

static uint32 constexpr AURA_INTERRUPT_FLAG_NOT_VICTIM = AURA_INTERRUPT_FLAG_HITBYSPELL | AURA_INTERRUPT_FLAG_TAKE_DAMAGE | AURA_INTERRUPT_FLAG_DIRECT_DAMAGE;
static uint32 constexpr AURA_INTERRUPT_FLAG_MOVING = AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING | AURA_INTERRUPT_FLAG_JUMP;

enum SpellAuraInterruptFlags2 : uint32
{
    AURA_INTERRUPT_FLAG2_UNK0         = 0x00000001,   // 0
    AURA_INTERRUPT_FLAG2_UNK1         = 0x00000002,   // 1
    AURA_INTERRUPT_FLAG2_UNK2         = 0x00000004,   // 2
    AURA_INTERRUPT_FLAG2_UNK3         = 0x00000008,   // 3    removed by hitting the ground???
    AURA_INTERRUPT_FLAG2_UNK4         = 0x00000010,   // 4
    AURA_INTERRUPT_FLAG2_UNK5         = 0x00000020,   // 5
    AURA_INTERRUPT_FLAG2_UNK6         = 0x00000040,   // 6
    AURA_INTERRUPT_FLAG2_UNK7         = 0x00000080,   // 7
    AURA_INTERRUPT_FLAG2_UNK8         = 0x00000100,   // 8
    AURA_INTERRUPT_FLAG2_UNK9         = 0x00000200,   // 9
    AURA_INTERRUPT_FLAG2_UNK10        = 0x00000400,   // 10
    AURA_INTERRUPT_FLAG2_UNK11        = 0x00000800,   // 11
    AURA_INTERRUPT_FLAG2_UNK12        = 0x00001000,   // 12
    AURA_INTERRUPT_FLAG2_UNK13        = 0x00002000,   // 13
    AURA_INTERRUPT_FLAG2_CHANGE_SPEC  = 0x00004000,   // 14 NYI
    AURA_INTERRUPT_FLAG2_UNK15        = 0x00008000,   // 15
    AURA_INTERRUPT_FLAG2_UNK16        = 0x00010000,   // 16
};

// is they same to SpellAuraInterruptFlags SpellAuraInterruptFlags2 ????
enum SpellChannelInterruptFlags : uint32
{
    CHANNEL_INTERRUPT_FLAG_UNK0       = 0x00000001,   // 0
    CHANNEL_INTERRUPT_FLAG_UNK1       = 0x00000002,   // 1
    CHANNEL_INTERRUPT_FLAG_INTERRUPT  = 0x00000004,   // 2
    CHANNEL_INTERRUPT_FLAG_MOVE       = 0x00000008,   // 3
    CHANNEL_INTERRUPT_FLAG_UNK4       = 0x00000010,   // 4
    CHANNEL_INTERRUPT_FLAG_UNK5       = 0x00000020,   // 5
    CHANNEL_INTERRUPT_FLAG_UNK6       = 0x00000040,   // 6
    CHANNEL_INTERRUPT_FLAG_UNK7       = 0x00000080,   // 7
    CHANNEL_INTERRUPT_FLAG_UNK8       = 0x00000100,   // 8
    CHANNEL_INTERRUPT_FLAG_UNK9       = 0x00000200,   // 9
    CHANNEL_INTERRUPT_FLAG_UNK10      = 0x00000400,   // 10
    CHANNEL_INTERRUPT_FLAG_UNK11      = 0x00000800,   // 11
    CHANNEL_INTERRUPT_FLAG_UNK12      = 0x00001000,   // 12
    CHANNEL_INTERRUPT_FLAG_UNK13      = 0x00002000,   // 13
    CHANNEL_INTERRUPT_FLAG_FLAG_DELAY = 0x00004000,   // 14
    CHANNEL_INTERRUPT_FLAG_UNK15      = 0x00008000,   // 15
    CHANNEL_INTERRUPT_FLAG_UNK16      = 0x00010000,   // 16
    CHANNEL_INTERRUPT_FLAG_UNK17      = 0x00020000,   // 17 
    CHANNEL_INTERRUPT_FLAG_UNK18      = 0x00040000,   // 18
    CHANNEL_INTERRUPT_FLAG_UNK19      = 0x00080000,   // 19
    CHANNEL_INTERRUPT_FLAG_UNK20      = 0x00100000,   // 20
    CHANNEL_INTERRUPT_FLAG_UNK21      = 0x00200000,   // 21
    CHANNEL_INTERRUPT_FLAG_UNK22      = 0x00400000,   // 22
    CHANNEL_INTERRUPT_FLAG_UNK23      = 0x00800000,   // 23 
    CHANNEL_INTERRUPT_FLAG_UNK24      = 0x01000000,   // 24
    CHANNEL_INTERRUPT_FLAG_UNK25      = 0x02000000,   // 25
    CHANNEL_INTERRUPT_FLAG_UNK26      = 0x04000000,   // 26
    CHANNEL_INTERRUPT_FLAG_UNK27      = 0x08000000,   // 27
    CHANNEL_INTERRUPT_FLAG_UNK28      = 0x10000000,   // 28
    CHANNEL_INTERRUPT_FLAG_UNK29      = 0x20000000,   // 29
    CHANNEL_INTERRUPT_FLAG_UNK30      = 0x40000000,   // 30
    CHANNEL_INTERRUPT_FLAG_UNK31      = 0x80000000,   // 31
};

enum SpellInterruptFlags : uint16
{
    SPELL_INTERRUPT_FLAG_MOVEMENT     = 0x01, // why need this for instant?
    SPELL_INTERRUPT_FLAG_PUSH_BACK    = 0x02, // push back
    SPELL_INTERRUPT_FLAG_UNK3         = 0x04, // any info?
    SPELL_INTERRUPT_FLAG_INTERRUPT    = 0x08, // interrupt
    SPELL_INTERRUPT_FLAG_ABORT_ON_DMG = 0x10,               // _complete_ interrupt on direct damage
    //SPELL_INTERRUPT_UNK             = 0x20                // unk, 564 of 727 spells having this spell start with "Glyph"
};

typedef std::vector<SpellPowerEntry const*> SpellPowerData;
typedef std::vector<int32> SpellPowerCost;

uint32 GetTargetFlagMask(SpellTargetObjectTypes objType);

class SpellImplicitTargetInfo
{
public:
    SpellImplicitTargetInfo();
    SpellImplicitTargetInfo(uint32 target);

    bool IsArea() const;
    SpellTargetSelectionCategories GetSelectionCategory() const;
    SpellTargetReferenceTypes GetReferenceType() const;
    SpellTargetObjectTypes GetObjectType() const;
    SpellTargetCheckTypes GetCheckType() const;
    SpellTargetDirectionTypes GetDirectionType() const;
    float CalcDirectionAngle() const;

    Targets GetTarget() const;
    uint32 GetExplicitTargetMask(bool& srcSet, bool& dstSet) const;

private:
    Targets _target;
    struct StaticData
    {
        SpellTargetObjectTypes ObjectType;    // type of object returned by target type
        SpellTargetReferenceTypes ReferenceType; // defines which object is used as a reference when selecting target
        SpellTargetSelectionCategories SelectionCategory;
        SpellTargetCheckTypes SelectionCheckType; // defines selection criteria
        SpellTargetDirectionTypes DirectionType; // direction for cone and dest targets
    };
    static StaticData _data[TOTAL_SPELL_TARGETS];
};

class SpellEffectInfo
{
    SpellInfo const* _spellInfo;
public:
    uint32    Effect;
    uint8     EffectIndex;
    uint32    ApplyAuraName;
    uint32    ApplyAuraPeriod;
    float     DieSides;
    float     RealPointsPerLevel;
    float     BasePoints;
    float     PointsPerResource;
    float     Amplitude;
    float     ChainAmplitude;
    float     BonusCoefficient;
    int32     MiscValue;
    int32     MiscValueB;
    Mechanics Mechanic;
    SpellImplicitTargetInfo TargetA;
    SpellImplicitTargetInfo TargetB;
    SpellRadiusEntry const* RadiusEntry;
    SpellRadiusEntry const* MaxRadiusEntry;
    int32    ChainTargets;
    uint32    ItemType;
    uint32    TriggerSpell;
    flag128   SpellClassMask;
    std::list<Condition*>* ImplicitTargetConditions;
    float     BonusCoefficientFromAP;
    float PvPMultiplier;
    float SpellEffectGroupSizeCoefficient;

    struct ScalingInfo
    {
        float Coefficient = 0.0f;
        float Variance = 0.0f;
        float OtherCoefficient = 0.0f;
    } Scaling;

    SpellEffectInfo();
    SpellEffectInfo(SpellInfo const* spellInfo, uint8 effIndex, SpellEffectEntry const* _effect);

    bool IsEffect() const;
    bool IsEffect(SpellEffects effectName) const;
    bool IsAura() const;
    bool IsAura(AuraType aura) const;
    bool IsTargetingArea() const;
    bool IsAreaAuraEffect() const;
    bool IsFarUnitTargetEffect() const;
    bool IsFarDestTargetEffect() const;
    bool IsUnitOwnedAuraEffect() const;

    float CalcValue(Unit const* caster = nullptr, float const* basePoints = nullptr, Unit const* target = nullptr, Item* m_castitem = nullptr, bool lockBasePoints = false, float* variance = nullptr, int32 comboPoints = 0) const;
    float CalcBaseValue(float value) const;
    float CalcValueMultiplier(Unit* caster, Spell* spell = nullptr) const;
    float CalcDamageMultiplier(Unit* caster, Spell* spell = nullptr) const;

    bool HasRadius() const;
    float CalcRadius(Unit* caster = nullptr, Spell* = nullptr) const;

    uint32 GetProvidedTargetMask() const;
    uint32 GetMissingTargetMask(bool srcSet = false, bool destSet = false, uint32 mask = 0) const;

    SpellEffectImplicitTargetTypes GetImplicitTargetType() const;
    SpellTargetObjectTypes GetUsedTargetObjectType() const;

private:
    struct StaticData
    {
        SpellEffectImplicitTargetTypes ImplicitTargetType; // defines what target can be added to effect target list if there's no valid target type provided for effect
        SpellTargetObjectTypes UsedTargetObjectType; // defines valid target object type for spell effect
    };
    static StaticData _data[TOTAL_SPELL_EFFECTS];
};

struct SpellMiscData
{
    SpellMiscEntry const* miscStore = nullptr;
    SpellDurationEntry const* durationStore = nullptr;
    SpellCastTimesEntry const* _castTimes = nullptr;
    SpellRangeEntry const* RangeEntry = nullptr;

    struct SpellMisc
    {
        uint32 Attributes[MaxAttributes];
        uint32 IconFileDataID;
        uint32 ActiveIconFileDataID;
        float Speed;
        float LaunchDelay;
        uint16 CastingTimeIndex;
        uint16 DurationIndex;
        uint16 RangeIndex;
        uint8 SchoolMask;
    } MiscData;

    struct SpellDuration
    {
        int32 Duration;
        int32 MaxDuration;
        int32 DurationPerLevel;
    } Duration;

    struct SpellCastTimes
    {
        int32 Base;
        int32 Minimum;
        int16 PerLevel;
    } CastTimes;
};

typedef std::unordered_map<uint16, SpellEffectInfo> SpellEffectInfoMap;
typedef std::unordered_map<uint16, SpellTargetRestrictionsEntry const*> SpellTargetRestrictionsMap;
typedef std::unordered_map<uint64, SpellMiscEntry const*> SpellMiscInfoMap;
typedef std::unordered_map<uint8, SpellMiscData> SpellMiscDataMap;

typedef std::vector<SpellXSpellVisualEntry const*> SpellVisualVector;
typedef std::vector<SpellVisualVector> SpellVisualMap;

struct SpellInfoLoadHelper;

template <typename InterruptFlag>
struct AuraInterruptFlagIndex {};

template <>
struct AuraInterruptFlagIndex<SpellAuraInterruptFlags>
{
    static std::size_t constexpr value = 0;
};

template <>
struct AuraInterruptFlagIndex<SpellAuraInterruptFlags2>
{
    static std::size_t constexpr value = 1;
};

class SpellInfo
{
public:
    uint32 Id;
    uint32 AttributesCu[2];
    uint32 InterruptFlags;
    std::array<uint32, MAX_SPELL_AURA_INTERRUPT_FLAGS> AuraInterruptFlags;
    std::array<uint32, MAX_SPELL_AURA_INTERRUPT_FLAGS> ChannelInterruptFlags;
    uint32 MaxLevel;
    uint32 BaseLevel;
    uint32 SpellLevel;
    uint8 MaxUsableLevel;
    int32  EquippedItemClass;
    int32  EquippedItemSubClassMask;
    int32  EquippedItemInventoryTypeMask;
    char const* SpellName;
    uint32 SpellScalingId;
    uint32 SpellPowerId;
    uint32 ResearchProject;
    uint32 MiscMask;
    uint32 EffectMask;
    uint64 EffectDifficultyMask[MAX_SPELL_EFFECTS];
    uint64 MiscDifficultyMask;
    uint64 AuraOptionsDifficultyMask;

    struct SpellTargetRestrictions
    {
        float ConeAngle;
        float Width;
        uint32 Targets;
        uint16 TargetCreatureType;
        uint8 MaxAffectedTargets;
        uint32 MaxTargetLevel;
    } TargetRestrictions;

    struct SpellCooldowns
    {
        uint32 CategoryRecoveryTime;
        uint32 RecoveryTime;
        uint32 StartRecoveryTime;
    } Cooldowns;

    struct SpellClassOptions
    {
        flag128 SpellClassMask;
        uint16 ModalNextSpell;
        uint8 SpellClassSet;
    } ClassOptions;

    struct ScalingInfo
    {
        int32 Class;
        uint32 MinScalingLevel;
        uint32 MaxScalingLevel;
        uint32 ScalesFromItemLevel;
    } Scaling;

    struct SpellAuraOptions
    {
        uint32 ProcCharges;
        uint32 ProcTypeMask;
        uint32 ProcCategoryRecovery;
        uint16 CumulativeAura;
        uint8 ProcChance;
        uint16 SpellProcsPerMinuteID;

        std::vector<SpellProcsPerMinuteModEntry const*> ProcPPMMods;
        float ProcBasePPM;

        bool IsProcAura;
    } AuraOptions;

    struct SpellAuraRescrictions
    {
        uint32 CasterAuraSpell;
        uint32 TargetAuraSpell;
        uint32 ExcludeCasterAuraSpell;
        uint32 ExcludeTargetAuraSpell;
        uint8 CasterAuraState;
        uint8 TargetAuraState;
        uint8 ExcludeCasterAuraState;
        uint8 ExcludeTargetAuraState;
    } AuraRestrictions;

    struct SpellCastingRequirements
    {
        uint16 MinFactionID;
        int16 RequiredAreasID;
        uint16 RequiresSpellFocus;
        uint8 FacingCasterFlags;
        uint8 MinReputation;
        uint8 RequiredAuraVision;
    } CastingReq;

    struct SpellCategories
    {
        uint16 Category;
        uint16 StartRecoveryCategory;
        uint16 ChargeCategory;
        uint8 DefenseType;
        uint8 DispelType;
        uint8 Mechanic;
        uint8 PreventionType;
    } Categories;

    struct SpellCategory
    {
        int32 ChargeRecoveryTime;
        uint8 Flags;
        uint8 UsesPerWeek;
        uint8 MaxCharges;
    } Category;

    struct SpellTotems
    {
        uint32 Totem[MAX_SPELL_TOTEMS];
        uint8 TotemCategory[MAX_SPELL_TOTEMS];
    } Totems;

    struct SpellReagents
    {
        int32 Reagent[MAX_SPELL_REAGENTS];
        uint16 ReagentCount[MAX_SPELL_REAGENTS];
        uint16 CurrencyID;
        uint16 CurrencyCount;
    } Reagents;
    
    struct SpellPowers
    {
        int32 PowerCost;
        float PowerCostPercentage;
        float PowerCostPercentagePerSecond;
        uint32 RequiredAura;
        float HealthCostPercentage;
        uint16 PowerCostPerSecond;
        uint16 ManaCostAdditional;
        uint16 PowerDisplayID;
        uint16 UnitPowerBarID;
        uint8 PowerIndex;
        int8 PowerType;
        uint8 PowerCostPerLevel;
    } Power;

    struct SpellShapeshifts
    {
        uint64 ShapeshiftMask;
        uint64 ShapeshiftExclude;
    } Shapeshift;

    SpellEffectInfo* Effects[MAX_SPELL_EFFECTS];
    SpellEffectInfo NullEffect;
    SpellEffectInfoMap EffectsMap;
    SpellMiscDataMap MiscsMap;
    SpellTargetRestrictionsMap RestrrictionsMap;
    uint32 ExplicitTargetMask;
    SpellChainNode const* ChainEntry;
    SpellPowerEntry const* spellPower[MAX_POWERS_FOR_SPELL];
    SpellMiscData* Miscs[MAX_DIFFICULTY];
    SpellMiscData Misc;
    SpellAuraOptions* _AuraOptionsDiff[MAX_DIFFICULTY];
    SpellAuraOptions _AuraOptions;
    std::bitset<TOTAL_AURAS> HasAuraBit;
    std::bitset<TOTAL_SPELL_EFFECTS> HasEffectBit;
    std::bitset<TOTAL_SPELL_TARGETS> HasTargetABit;
    std::bitset<TOTAL_SPELL_TARGETS> HasTargetBBit;
    std::bitset<TARGET_SELECT_CATEGORY_MAX> HasTargetCategoryBit;
    uint32 EffectMechanicMask;
    bool NeedAuraUpdateTarget;
    bool HasTriggerSpell;

    // TalentInfo
    uint32 talentId;

    SpellInfo(SpellInfoLoadHelper const& data, SpellEntry const* spellEntry, SpellVisualMap* visuals);
    ~SpellInfo();

    SpellEffectInfo const* GetEffect(uint8 effect, uint8 difficulty = 0) const;
    SpellMiscData* GetMisc(uint8 difficulty = 0) const;
    SpellAuraOptions* GetAuraOptions(uint8 difficulty = 0) const;
    bool HasEffect(SpellEffects effect) const;
    bool HasAura(AuraType aura) const;
    bool HasAreaAuraEffect() const;
    bool HasFarUnitTargetEffect() const;
    bool HasDynAuraEffect() const;
    bool IsMountOrCompanions() const;
    bool IsNotProcSpell(bool isVictim) const;

    bool HasAttribute(SpellAttr0 attribute) const { return !!(GetMisc()->MiscData.Attributes[0] & attribute); }
    bool HasAttribute(SpellAttr1 attribute) const { return !!(GetMisc()->MiscData.Attributes[1] & attribute); }
    bool HasAttribute(SpellAttr2 attribute) const { return !!(GetMisc()->MiscData.Attributes[2] & attribute); }
    bool HasAttribute(SpellAttr3 attribute) const { return !!(GetMisc()->MiscData.Attributes[3] & attribute); }
    bool HasAttribute(SpellAttr4 attribute) const { return !!(GetMisc()->MiscData.Attributes[4] & attribute); }
    bool HasAttribute(SpellAttr5 attribute) const { return !!(GetMisc()->MiscData.Attributes[5] & attribute); }
    bool HasAttribute(SpellAttr6 attribute) const { return !!(GetMisc()->MiscData.Attributes[6] & attribute); }
    bool HasAttribute(SpellAttr7 attribute) const { return !!(GetMisc()->MiscData.Attributes[7] & attribute); }
    bool HasAttribute(SpellAttr8 attribute) const { return !!(GetMisc()->MiscData.Attributes[8] & attribute); }
    bool HasAttribute(SpellAttr9 attribute) const { return !!(GetMisc()->MiscData.Attributes[9] & attribute); }
    bool HasAttribute(SpellAttr10 attribute) const { return !!(GetMisc()->MiscData.Attributes[10] & attribute); }
    bool HasAttribute(SpellAttr11 attribute) const { return !!(GetMisc()->MiscData.Attributes[11] & attribute); }
    bool HasAttribute(SpellAttr12 attribute) const { return !!(GetMisc()->MiscData.Attributes[12] & attribute); }
    bool HasAttribute(SpellAttr13 attribute) const { return !!(GetMisc()->MiscData.Attributes[13] & attribute); }
    bool HasAttribute(SpellCustomAttributes customAttribute) const { return !!(AttributesCu[0] & customAttribute); }
    bool HasAttribute(SpellCustomAttributes1 customAttribute) const { return !!(AttributesCu[1] & customAttribute); }

    bool HasAnyAuraInterruptFlag() const;
    bool HasAuraInterruptFlag(SpellAuraInterruptFlags flag) const;
    bool HasAuraInterruptFlag(SpellAuraInterruptFlags2 flag) const;
    bool HasChannelInterruptFlag(SpellChannelInterruptFlags flag) const;
    bool HasChannelInterruptFlag(SpellAuraInterruptFlags2 flag) const;

    bool IsExplicitDiscovery() const;
    bool IsLootCrafting() const;
    bool IsArchaeologyCraftingSpell() const;
    bool IsQuestTame() const;
    bool IsProfession() const;
    bool IsPrimaryProfession() const;
    bool IsPrimaryProfessionFirstRank() const;
    uint16 GetProfessionSkillId() const;

    bool IsAffectingArea() const;
    bool IsTargetingArea() const;
    bool IsTargetingAreaCast(uint8 effect = 0) const;
    bool NeedsExplicitUnitTarget() const;
    bool NeedsToBeTriggeredByCaster(SpellInfo const* triggeringSpell, uint32 difficulty) const;

    bool IsPassive() const;
    bool IsAutocastable() const;
    bool IsStackableWithRanks() const;
    bool IsPassiveStackableWithRanks() const;
    bool IsMultiSlotAura() const;
    bool IsCooldownStartedOnEvent() const;
    bool IsDeathPersistent() const;
    bool IsRequiringDeadTarget() const;
    bool IsAllowingDeadTarget() const;
    bool CanBeUsedInCombat() const;
    bool IsPositive() const;
    bool IsPositiveEffect(uint8 effIndex, bool caster = false) const;
    bool IsChanneled() const;
    bool IsBreakingStealth() const;
    bool IsRangedWeaponSpell() const;
    bool IsRangedSpell() const;
    bool IsAutoRepeatRangedSpell() const;
    bool IsNonNeedDelay() const;
    bool IsNoTargets() const;
    bool IsSelfTargets() const;
    bool IsAffectedBySpellMods() const;
    bool IsAffectedBySpellMod(SpellModifier* mod) const;
    bool CanPierceImmuneAura(SpellInfo const* aura) const;
    bool CanDispelAura(SpellInfo const* aura) const;
    bool IsSingleTarget(Unit const* caster, Unit const* target) const;
    bool IsAuraExclusiveBySpecificWith(SpellInfo const* spellInfo, bool sameCaster = false) const;
    bool IsAuraExclusiveBySpecificPerCasterWith(SpellInfo const* spellInfo) const;
    bool IsAllowsCastWhileMove(Unit* caster) const;

    SpellCastResult CheckShapeshift(uint32 form) const;
    SpellCastResult CheckLocation(uint32 map_id, uint32 zone_id, uint32 area_id, Player const* player = nullptr) const;
    SpellCastResult CheckTarget(Unit const* caster, WorldObject const* target, bool implicit = true) const;
    SpellCastResult CheckExplicitTarget(Unit const* caster, WorldObject const* target, Item const* itemTarget = nullptr) const;
    bool CheckTargetCreatureType(Unit const* target) const;

    SpellSchoolMask GetSchoolMask() const;
    uint32 GetAllEffectsMechanicMask() const;
    uint32 GetEffectMechanicMask(uint8 effIndex) const;
    uint32 GetSpellMechanicMaskByEffectMask(uint32 effectMask) const;
    Mechanics GetEffectMechanic(uint8 effIndex) const;
    bool HasAnyEffectMechanic() const;
    uint32 GetDispelMask() const;
    static uint32 GetDispelMask(DispelType type);
    uint32 GetMechanicMask(uint32 miscVal) const;
    uint32 GetSimilarEffectsMiscValueMask(SpellEffects effectName, Unit* caster = nullptr) const;
    uint32 GetExplicitTargetMask() const;
    uint32 GetSpellTypeMask() const;

    AuraStateType GetAuraState() const;
    SpellSpecificType GetSpellSpecific() const;

    uint32 GetMaxAffectedTargets(uint16 diff) const;
    uint32 GetTargets(uint16 diff) const;
    uint32 GetTargetCreatureType(uint16 diff) const;

    float GetMinRange(bool positive = false) const;
    float GetMaxRange(bool positive = false, Unit* caster = nullptr, Spell* spell = nullptr) const;

    int32 GetDuration(uint8 difficulty = 0) const;
    int32 GetMaxDuration(uint8 difficulty = 0) const;
    bool CanModDuration(uint8 difficulty = 0) const;

    uint32 GetMaxTicks() const;

    uint32 CalcCastTime(Unit* caster = nullptr, Spell* spell = nullptr) const;
    uint32 GetRecoveryTime() const;
    float CalcProcPPM(Unit* caster, int32 itemLevel) const;

    void CalcPowerCost(Unit const* caster, SpellSchoolMask schoolMask, SpellPowerCost& powerCost) const;

    bool IsRanked() const;
    uint8 GetRank() const;
    SpellInfo const* GetFirstRankSpell() const;
    SpellInfo const* GetLastRankSpell() const;
    SpellInfo const* GetNextRankSpell() const;
    SpellInfo const* GetPrevRankSpell() const;
    SpellInfo const* GetAuraRankForLevel(uint8 level) const;
    bool IsRankOf(SpellInfo const* spellInfo) const;
    bool IsDifferentRankOf(SpellInfo const* spellInfo) const;
    bool IsHighRankOf(SpellInfo const* spellInfo) const;
    
    void SetExtraSpellXSpellVisualId(uint32 id);
    uint32 GetSpellXSpellVisualId(Unit* caster, Unit* target = nullptr) const;
    uint32 GetSpellVisual(Unit* caster = nullptr) const;

    bool CanTriggerBladeFlurry() const;

    // loading helpers
    uint32 _GetExplicitTargetMask() const;
    bool _IsPositiveEffect(uint8 effIndex, bool deep) const;
    bool _IsPositiveSpell() const;
    static bool _IsPositiveTarget(uint32 targetA, uint32 targetB);
    void _LoadSpellSpecific();
    void _LoadAuraState();

    void SetRangeIndex(uint32 index);

    void _UnloadImplicitTargetConditionLists();

    bool AddPowerData(SpellPowerEntry const * power);
    bool IsPowerActive(uint8 powerIndex) const;
    SpellPowerEntry const* GetPowerInfo(uint8 powerIndex) const;
    bool GetSpellPowerByCasterPower(Unit const * caster, SpellPowerData& power) const;
    bool HasPower(Powers power) const;
    bool NoPower() const;

    bool IsActiveMitigationDamage() const;
    bool IsStack() const;
    bool CanInterruptChannel(SpellInfo const* spellInfo) const;
    bool IsNotMount() const;
    bool IsRefreshTimers() const;
    bool IsBattleResurrection() const;
    bool CanInterrupt(uint32 spellID, uint32 flags, Unit* target) const;
    bool CanTriggerAbsorb() const;
    bool DontCheckDistance() const;
    bool IsMultiSingleTarget() const;
    uint32 GetMultiSingleTargetCount() const;
    bool CanStartCombat() const;
    bool IsDamageSpell() const;
    bool IsControlSpell() const;
    bool IsHealSpell() const;
    bool IsSafeSpell() const;
    bool CanAutoCast(Unit* m_caster, Unit* target) const;
    bool CanAutoCastAura(Unit* m_caster, Unit* target, uint32 auraId) const;
    bool CanAutoCastEffect(Unit* m_caster, Unit* target, uint32 effectId) const;

private:
    SpellTargetRestrictionsEntry const* GetSpellTargetRestrictions(uint16 diff) const;
    SpellVisualMap _visuals;
    SpellSpecificType _spellSpecific;
    AuraStateType _auraState;
    uint32 _extraSpellXVisualID;
};

#endif // _SPELLINFO_H
