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

#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "DataContainers.h"
#include "EventProcessor.h"
#include "FollowerReference.h"
#include "FollowerRefManager.h"
#include "FunctionProcessor.h"
#include "HostileRefManager.h"
#include "MotionMaster.h"
#include "Object.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "UnitDefines.h"
#include "UpdateFields.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <safe_ptr.h>
#include "../DynamicObject/DynamicObject.h"
#include <cds/container/iterable_list_hp.h>
#include <boost/container/flat_set.hpp>

struct CharmInfo;

#define WORLD_TRIGGER   12999
#define ARTIFACTS_ALL_WEAPONS_GENERAL_WEAPON_EQUIPPED_PASSIVE 197886
#define SPELL_DH_DOUBLE_JUMP 196055

/// Key for storing temp summon data in TempSummonDataContainer
struct TempSummonGroupKey
{
    TempSummonGroupKey(uint32 summonerEntry, SummonerType summonerType, uint8 group)
        : _summonerEntry(summonerEntry), _summonerType(summonerType), _summonGroup(group)
    {
    }

    bool operator<(TempSummonGroupKey const& rhs) const
    {
        return std::tie(_summonerEntry, _summonerType, _summonGroup) <
            std::tie(rhs._summonerEntry, rhs._summonerType, rhs._summonGroup);
    }

private:
    uint32 _summonerEntry;      ///< Summoner's entry
    SummonerType _summonerType; ///< Summoner's type, see SummonerType for available types
    uint8 _summonGroup;         ///< Summon's group id
};

typedef std::map<TempSummonGroupKey, GuidList> TempSummonGroupMap;
typedef std::map<uint32, GuidList> TempSummonMap;

#define UPDATE_STATS_TIME   200

enum UpdateStatsMask
{
    USM_STRENGTH        = 0x00000001,
    USM_AGILITY         = 0x00000002,
    USM_STAMINA         = 0x00000004,
    USM_INTELLECT       = 0x00000008,
    USM_MELEE_HAST      = 0x00000010,
    USM_RANGE_HAST      = 0x00000020,
    USM_CAST_HAST       = 0x00000040,
    USM_HAST            = 0x00000080,
    USM_ARMOR           = 0x00000100,
    USM_MANA_REGEN      = 0x00000200,
    USM_MELEE_AP        = 0x00000400,
    USM_RANGE_AP        = 0x00000800,

    USM_ALL             = 0xFFFFFFFF,
};

enum OperationsAfterDelay
{
    OAD_RECALC_ITEMS       = 0x00000001,
    OAD_LOAD_PET           = 0x00000002,
    OAD_UPDATE_RUNES_REGEN = 0x00000004,
    OAD_RECALC_PVP_BP      = 0x00000008,
    OAD_RESET_SPELL_QUEUE  = 0x00000010,
    OAD_RECALC_CHAR_STAT   = 0x00000020,
    OAD_RECALC_AURAS       = 0x00000040,
    OAD_RECALC_ITEM_LVL    = 0x00000080,
    OAD_ARENA_DESERTER     = 0x00000100,
};

enum SpellModOp
{
    SPELLMOD_DAMAGE                 = 0,
    SPELLMOD_DURATION               = 1,
    SPELLMOD_THREAT                 = 2,
    SPELLMOD_EFFECT1                = 3,
    SPELLMOD_CHARGES                = 4,
    SPELLMOD_RANGE                  = 5,
    SPELLMOD_RADIUS                 = 6,
    SPELLMOD_CRITICAL_CHANCE        = 7,
    SPELLMOD_ALL_EFFECTS            = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME  = 9,
    SPELLMOD_CASTING_TIME           = 10,
    SPELLMOD_COOLDOWN               = 11,
    SPELLMOD_EFFECT2                = 12,
    SPELLMOD_IGNORE_ARMOR           = 13,
    SPELLMOD_COST                   = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS      = 15,
    SPELLMOD_RESIST_MISS_CHANCE     = 16,
    SPELLMOD_JUMP_TARGETS           = 17,
    SPELLMOD_CHANCE_OF_SUCCESS      = 18,
    SPELLMOD_ACTIVATION_TIME        = 19,
    SPELLMOD_DAMAGE_MULTIPLIER      = 20,
    SPELLMOD_GLOBAL_COOLDOWN        = 21,
    SPELLMOD_DOT                    = 22,
    SPELLMOD_EFFECT3                = 23,
    SPELLMOD_BONUS_MULTIPLIER       = 24,
    // spellmod 25
    SPELLMOD_PROC_PER_MINUTE        = 26,
    SPELLMOD_VALUE_MULTIPLIER       = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE   = 28,
    SPELLMOD_CRIT_DAMAGE_BONUS_2    = 29, //one not used spell
    SPELLMOD_SPELL_COST_REFUND_ON_FAIL = 30,
    SPELLMOD_STACK_AMOUNT           = 31,
    SPELLMOD_EFFECT4                = 32,
    SPELLMOD_EFFECT5                = 33,
    SPELLMOD_SPELL_COST2            = 34,
    SPELLMOD_JUMP_DISTANCE          = 35,
    SPELLMOD_UNK_2                  = 36,
    SPELLMOD_STACK_AMOUNT2          = 37,
    SPELLMOD_UNK                    = 38,

    MAX_SPELLMOD
};

enum class SpellCooldownFlags : uint8
{
    NONE                    = 0x0,
    INCLUDE_GCD             = 0x1,  ///< Starts GCD in addition to normal cooldown specified in the packet
    INCLUDE_EVENT_COOLDOWNS = 0x2   ///< Starts GCD for spells that should start their cooldown on events, requires SPELL_COOLDOWN_FLAG_INCLUDE_GCD set
};

enum PetSpellModOp // aura SPELL_AURA_MOD_PET_STATS_MODIFIER
{
    PETSPELLMOD_MAX_HP                 = 1,
    PETSPELLMOD_DAMAGE                 = 2,
    PETSPELLMOD_DAMAGE_UNK             = 3,
    PETSPELLMOD_ARMOR                  = 13,
    PETSPELLMOD_DOT                    = 24, //may be DOT?
};

enum SpellValueMod
{
    SPELLVALUE_BASE_POINT0,
    SPELLVALUE_BASE_POINT1,
    SPELLVALUE_BASE_POINT2,
    SPELLVALUE_BASE_POINT3,
    SPELLVALUE_BASE_POINT4,
    SPELLVALUE_BASE_POINT5,
    SPELLVALUE_RADIUS_MOD,
    SPELLVALUE_MAX_TARGETS,
    SPELLVALUE_AURA_STACK,
};

typedef std::pair<SpellValueMod, float>     CustomSpellValueMod;
class CustomSpellValues : public std::vector<CustomSpellValueMod>
{
    public:
        void AddSpellMod(SpellValueMod mod, float value)
        {
            push_back(std::make_pair(mod, value));
        }
};

enum SpellFacingFlags
{
    SPELL_FACING_FLAG_INFRONT = 0x0001
};

// high byte (3 from 0..3) of UNIT_FIELD_BYTES_2
enum ShapeshiftForm
{
    FORM_NONE               = 0x00,
    FORM_CAT                = 0x01,
    FORM_TREE               = 0x02,
    FORM_TRAVEL             = 0x03,
    FORM_AQUA               = 0x04,
    FORM_BEAR               = 0x05,
    FORM_AMBIENT            = 0x06,
    FORM_GHOUL              = 0x07,
    FORM_DIREBEAR           = 0x08, // Removed in 4.0.1
    FORM_SPIRITED_CRANE     = 0x09,
    FORM_THARONJA_SKELETON  = 0x0A,
    FORM_TEST_OF_STRENGTH   = 0x0B,
    FORM_BLB_PLAYER         = 0x0C,
    FORM_SHADOW_DANCE       = 0x0D,
    FORM_CREATUREBEAR       = 0x0E,
    FORM_CREATURECAT        = 0x0F,
    FORM_GHOSTWOLF          = 0x10,
    FORM_BATTLESTANCE       = 0x11,
    FORM_DEFENSIVESTANCE    = 0x12,
    FORM_BERSERKERSTANCE    = 0x13,
    FORM_WISE_SERPENT       = 0x14,
    FORM_ZOMBIE             = 0x15,
    FORM_METAMORPHOSIS      = 0x16,
    FORM_STURDY_OX          = 0x17,
    FORM_FIERCE_TIGER       = 0x18,
    FORM_UNDEAD             = 0x19,
    FORM_MASTER_ANGLER      = 0x1A,
    FORM_FLIGHT_EPIC        = 0x1B,
    FORM_SHADOW             = 0x1C,
    FORM_FLIGHT             = 0x1D,
    FORM_STEALTH            = 0x1E,
    FORM_MOONKIN            = 0x1F,
    FORM_SPIRITOFREDEMPTION = 0x20,
    FORM_GLADIATORSTANCE    = 0x21,
    FORM_METAMORPHOSIS2     = 0x22,
    FORM_MOONKIN2           = 0x23,
    FORM_TREANT_FORM        = 0x24,
    FORM_SPIRIT_OWL_FORM    = 0x25,
    FORM_SPIRIT_OWL_FORM_2  = 0x26,
    FORM_WISP_FORM          = 0x27,
    FORM_WISP_FORM_2        = 0x28,
};

#define MAX_SPELL_CHARM         4
#define MAX_SPELL_VEHICLE       6
#define MAX_SPELL_POSSESS       8
#define MAX_SPELL_CONTROL_BAR   10

enum Swing
{
    NOSWING                    = 0,
    SINGLEHANDEDSWING          = 1,
    TWOHANDEDSWING             = 2
};

enum VictimState
{
    VICTIMSTATE_INTACT         = 0, // set when attacker misses
    VICTIMSTATE_HIT            = 1, // victim got clear/blocked hit
    VICTIMSTATE_DODGE          = 2,
    VICTIMSTATE_PARRY          = 3,
    VICTIMSTATE_INTERRUPT      = 4,
    VICTIMSTATE_BLOCKS         = 5, // unused? not set when blocked, even on full block
    VICTIMSTATE_EVADES         = 6,
    VICTIMSTATE_IS_IMMUNE      = 7,
    VICTIMSTATE_DEFLECTS       = 8
};

//i would like to remove this: (it is defined in item.h
enum InventorySlot
{
    NULL_BAG                   = 0,
    NULL_SLOT                  = 255
};

struct FactionTemplateEntry;
struct SpellValue;

class AuraApplication;
class Aura;
class UnitAura;
class AuraEffect;
class Creature;
class Spell;
class SpellInfo;
class DynamicObject;
class GameObject;
class Item;
class Pet;
class Minion;
class Guardian;
class UnitAI;
class Totem;
class Transport;
class Vehicle;
class VehicleJoinEvent;
class TransportBase;
class SpellCastTargets;

typedef std::list<Unit*> UnitList;
typedef std::list< std::pair<Aura*, uint8> > DispelChargesList;
typedef std::set<Aura*> UsedSpellMods;

typedef std::shared_ptr<AuraApplication> AuraApplicationPtr;

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

struct ComparatorSpellImmune
{
    int operator ()(SpellImmune v1, SpellImmune v2) const
    {
        if (v1.spellId == v2.spellId)
            return v1.type - v2.type;
        return v1.spellId - v2.spellId;
    }
};

struct ComparatorAuraEffect
{
    int operator ()(AuraEffect* v1, AuraEffect* v2) const
    {
        std::less<AuraEffect*> comp;
        if (comp(v1, v2))
            return -1;
        return comp(v2, v1) ? 1 : 0;
    }
};

struct MirrorImageData
{
    ObjectGuid GuildGUID;
    std::vector<uint32> ItemDisplayID;
    bool isPlayer;
    uint32 DisplayID;
    uint8 RaceID;
    uint8 Gender;
    uint8 ClassID;
    uint8 SkinColor;
    uint8 FaceVariation;
    uint8 HairVariation;
    uint8 HairColor;
    uint8 BeardVariation;
    std::array<uint8, 3> CustomDisplay;
};

// typedef std::list<SpellImmune> SpellImmuneList;
typedef cds::container::IterableList< cds::gc::HP, SpellImmune,
    cds::container::iterable_list::make_traits<cds::container::opt::compare< ComparatorSpellImmune >     // item comparator option
    >::type> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT_EXCLUDE_CREATE = 1,    // percent modifier affecting all stat values from auras and gear but not player base for level
    BASE_PCT = 2,
    TOTAL_VALUE = 3,
    TOTAL_PCT = 4,
    MODIFIER_TYPE_END = 5
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum AuraRemoveMode
{
    AURA_REMOVE_NONE                = 0,
    AURA_REMOVE_BY_DEFAULT          = 1, // scripted remove, remove by stack with aura with different ids and sc aura remove
    AURA_REMOVE_BY_CANCEL           = 2,
    AURA_REMOVE_BY_ENEMY_SPELL      = 3, // dispel and absorb aura destroy
    AURA_REMOVE_BY_EXPIRE           = 4, // aura duration has ended
    AURA_REMOVE_BY_DEATH            = 5,
    AURA_REMOVE_BY_MECHANIC         = 6,
    AURA_REMOVE_BY_DROP_CHARGERS    = 7, // aura remove by drop charges
    AURA_REMOVE_BY_INTERRUPT        = 8,
};

enum TriggerCastFlags
{
    TRIGGERED_NONE                                  = 0x00000000,   //! Not triggered
    TRIGGERED_IGNORE_GCD                            = 0x00000001,   //! Will ignore GCD
    TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD          = 0x00000002,   //! Will ignore Spell and Category cooldowns
    TRIGGERED_IGNORE_POWER_AND_REAGENT_COST         = 0x00000004,   //! Will ignore power and reagent cost
    TRIGGERED_IGNORE_CAST_ITEM                      = 0x00000008,   //! Will not take away cast item or update related achievement criteria
    TRIGGERED_IGNORE_AURA_SCALING                   = 0x00000010,   //! Will ignore aura scaling
    TRIGGERED_IGNORE_CAST_IN_PROGRESS               = 0x00000020,   //! Will not check if a current cast is in progress
    TRIGGERED_IGNORE_COMBO_POINTS                   = 0x00000040,   //! Will ignore combo point requirement
    TRIGGERED_CAST_DIRECTLY                         = 0x00000080,   //! In Spell::prepare, will be cast directly without setting containers for executed spell
    TRIGGERED_IGNORE_AURA_INTERRUPT_FLAGS           = 0x00000100,   //! Will ignore interruptible aura's at cast
    TRIGGERED_IGNORE_SET_FACING                     = 0x00000200,   //! Will not adjust facing to target (if any)
    TRIGGERED_IGNORE_SHAPESHIFT                     = 0x00000400,   //! Will ignore shapeshift checks
    TRIGGERED_IGNORE_CASTER_AURASTATE               = 0x00000800,   //! Will ignore caster aura states including combat requirements and death state
    TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE   = 0x00002000,   //! Will ignore mounted/on vehicle restrictions
    TRIGGERED_IGNORE_CASTER_AURAS                   = 0x00010000,   //! Will ignore caster aura restrictions or requirements
    TRIGGERED_DISALLOW_PROC_EVENTS                  = 0x00020000,   //! Disallows proc events from triggered spell (default)
    TRIGGERED_DONT_REPORT_CAST_ERROR                = 0x00040000,   //! Will return SPELL_FAILED_DONT_REPORT in CheckCast functions
    TRIGGERED_CASTED_BY_AREATRIGGER                 = 0x00080000,   //! Special case: spell dependent on areatrigger
    TRIGGERED_IGNORE_LOS                            = 0x00100000,   //! Can target not in los
    TRIGGERED_FULL_MASK                             = 0xFFF7FFFF,   //! all flags except TRIGGERED_CASTED_BY_AREATRIGGER
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_INTELLECT must be in existed order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_RUNIC_POWER must be in existed order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_COMBO_POINTS,
    UNIT_MOD_RUNE,
    UNIT_MOD_RUNIC_POWER,
    UNIT_MOD_SOUL_SHARDS,
    UNIT_MOD_LUNAR,
    UNIT_MOD_HOLY_POWER,
    UNIT_MOD_ALTERNATIVE,
    UNIT_MOD_MAELSTROME,
    UNIT_MOD_CHI,
    UNIT_MOD_INSANITY,
    UNIT_MOD_OBSOLETE,
    UNIT_MOD_OBSOLETE2,
    UNIT_MOD_ARCANE_CHARGES,
    UNIT_MOD_FURY,
    UNIT_MOD_PAIN,
    UNIT_MOD_MAX_POWERS,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existed order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,

    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_MAX_POWERS
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,

    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD,

    MOD_END
};

enum DeathState
{
    ALIVE       = 0,
    JUST_DIED   = 1,
    CORPSE      = 2,
    DEAD        = 3,
    JUST_RESPAWNED = 4,
};

enum UnitState
{
    UNIT_STATE_DIED            = 0x00000001,                     // player has fake death aura
    UNIT_STATE_MELEE_ATTACKING = 0x00000002,                     // player is melee attacking someone
    //UNIT_STATE_MELEE_ATTACK_BY = 0x00000004,                     // player is melee attack by someone
    UNIT_STATE_STUNNED         = 0x00000008,
    UNIT_STATE_ROAMING         = 0x00000010,
    UNIT_STATE_CHASE           = 0x00000020,
    //UNIT_STATE_SEARCHING       = 0x00000040,
    UNIT_STATE_FLEEING         = 0x00000080,
    UNIT_STATE_IN_FLIGHT       = 0x00000100,                     // player is in flight mode
    UNIT_STATE_FOLLOW          = 0x00000200,
    UNIT_STATE_ROOT            = 0x00000400,
    UNIT_STATE_CONFUSED        = 0x00000800,
    UNIT_STATE_DISTRACTED      = 0x00001000,
    UNIT_STATE_ISOLATED        = 0x00002000,                     // area auras do not affect other players
    UNIT_STATE_ATTACK_PLAYER   = 0x00004000,
    UNIT_STATE_CASTING         = 0x00008000,
    UNIT_STATE_POSSESSED       = 0x00010000,
    UNIT_STATE_CHARGING        = 0x00020000,
    UNIT_STATE_JUMPING         = 0x00040000,
    UNIT_STATE_ONVEHICLE       = 0x00080000,
    UNIT_STATE_MOVE            = 0x00100000,
    UNIT_STATE_ROTATING        = 0x00200000,
    UNIT_STATE_EVADE           = 0x00400000,
    UNIT_STATE_ROAMING_MOVE    = 0x00800000,
    UNIT_STATE_CONFUSED_MOVE   = 0x01000000,
    UNIT_STATE_FLEEING_MOVE    = 0x02000000,
    UNIT_STATE_CHASE_MOVE      = 0x04000000,
    UNIT_STATE_FOLLOW_MOVE     = 0x08000000,
    UNIT_STATE_IGNORE_PATHFINDING = 0x10000000,                 // do not use pathfinding in any MovementGenerator
    UNIT_STATE_MOVE_IN_CASTING = 0x20000000,
    UNIT_STATE_LONG_JUMP       = 0x40000000,
    UNIT_STATE_UNATTACKABLE    = UNIT_STATE_IN_FLIGHT | UNIT_STATE_ONVEHICLE,
    // for real move using movegen check and stop (except unstoppable flight)
    UNIT_STATE_MOVING          = UNIT_STATE_ROAMING_MOVE | UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE | UNIT_STATE_CHASE_MOVE | UNIT_STATE_FOLLOW_MOVE ,
    UNIT_STATE_CONTROLLED      = UNIT_STATE_CONFUSED | UNIT_STATE_STUNNED | UNIT_STATE_FLEEING,
    UNIT_STATE_LOST_CONTROL    = UNIT_STATE_CONTROLLED | UNIT_STATE_JUMPING | UNIT_STATE_CHARGING,
    UNIT_STATE_SIGHTLESS       = UNIT_STATE_LOST_CONTROL | UNIT_STATE_EVADE,
    UNIT_STATE_CANNOT_AUTOATTACK     = UNIT_STATE_LOST_CONTROL | UNIT_STATE_CASTING,
    UNIT_STATE_CANNOT_TURN     = UNIT_STATE_LOST_CONTROL | UNIT_STATE_ROTATING,
    // stay by different reasons
    UNIT_STATE_NOT_MOVE        = UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DIED | UNIT_STATE_DISTRACTED,
    UNIT_STATE_ALL_STATE       = 0xffffffff                      //(UNIT_STATE_STOPPED | UNIT_STATE_MOVING | UNIT_STATE_IN_COMBAT | UNIT_STATE_IN_FLIGHT)
};

enum UnitMoveType
{
    MOVE_WALK           = 0,
    MOVE_RUN            = 1,
    MOVE_RUN_BACK       = 2,
    MOVE_SWIM           = 3,
    MOVE_SWIM_BACK      = 4,
    MOVE_TURN_RATE      = 5,
    MOVE_FLIGHT         = 6,
    MOVE_FLIGHT_BACK    = 7,
    MOVE_PITCH_RATE     = 8,

    MAX_MOVE_TYPE
};

extern float baseMoveSpeed[MAX_MOVE_TYPE];
extern float playerBaseMoveSpeed[MAX_MOVE_TYPE];

enum WeaponAttackType
{
    BASE_ATTACK   = 0,
    OFF_ATTACK    = 1,
    RANGED_ATTACK = 2,
    MAX_ATTACK
};

// Last check : 5.0.5
enum CombatRating
{
    CR_AMPLIFY                          = 0,
    CR_DEFENSE_SKILL                    = 1,
    CR_DODGE                            = 2,
    CR_PARRY                            = 3,
    CR_BLOCK                            = 4,
    CR_HIT_MELEE                        = 5,
    CR_HIT_RANGED                       = 6,
    CR_HIT_SPELL                        = 7,
    CR_CRIT_MELEE                       = 8,
    CR_CRIT_RANGED                      = 9,
    CR_CRIT_SPELL                       = 10,
    CR_MULTISTRIKE                      = 11,
    CR_READINESS                        = 12,
    CR_SPEED                            = 13,
    CR_RESILIENCE_CRIT_TAKEN            = 14,
    CR_RESILIENCE_PLAYER_DAMAGE         = 15,
    CR_LIFESTEAL                        = 16,
    CR_HASTE_MELEE                      = 17,
    CR_HASTE_RANGED                     = 18,
    CR_HASTE_SPELL                      = 19,
    CR_AVOIDANCE                        = 20,
    CR_STURDINESS                       = 21,
    CR_UNUSED_7                         = 22,
    CR_EXPERTISE                        = 23,
    CR_ARMOR_PENETRATION                = 24,
    CR_MASTERY                          = 25,
    CR_PVP_POWER                        = 26,
    CR_CLEAVE                           = 27,
    CR_VERSATILITY_DAMAGE_DONE          = 28,
    CR_VERSATILITY_HEALING_DONE         = 29,
    CR_VERSATILITY_DAMAGE_TAKEN         = 30,
    CR_UNUSED_12                        = 31,

    MAX_COMBAT_RATING                   = 31,

    CR_PVP_UPDATE_MASK = 1 << CR_CRIT_SPELL | 1 << CR_CRIT_MELEE | 1 << CR_CRIT_RANGED | 1 << CR_HASTE_SPELL | 1 << CR_HASTE_MELEE |
                         1 << CR_HASTE_RANGED | 1 << CR_MASTERY | 1 << CR_VERSATILITY_DAMAGE_DONE | 1 << CR_VERSATILITY_HEALING_DONE | 1 << CR_VERSATILITY_DAMAGE_TAKEN
};

enum DamageEffectType
{
    DIRECT_DAMAGE           = 0,                            // used for normal weapon damage (not for class abilities or spells)
    SPELL_DIRECT_DAMAGE     = 1,                            // spell/class abilities damage
    DOT                     = 2,
    HEAL                    = 3,
    NODAMAGE                = 4,                            // used also in case when damage applied to health but not applied to spell channelInterruptFlags/etc
    SELF_DAMAGE             = 5
};

enum UnitTypeMask
{
    UNIT_MASK_NONE                   = 0x00000000,
    UNIT_MASK_SUMMON                 = 0x00000001,
    UNIT_MASK_MINION                 = 0x00000002,
    UNIT_MASK_GUARDIAN               = 0x00000004,
    UNIT_MASK_TOTEM                  = 0x00000008,
    UNIT_MASK_PET                    = 0x00000010,
    UNIT_MASK_VEHICLE                = 0x00000020,
    UNIT_MASK_PUPPET                 = 0x00000040,
    UNIT_MASK_HUNTER_PET             = 0x00000080,
    UNIT_MASK_CONTROLABLE_GUARDIAN   = 0x00000100,
    UNIT_MASK_ACCESSORY              = 0x00000200,
    UNIT_MASK_UNK                    = 0x00000400,
    UNIT_MASK_UNK2                   = 0x00000800,
    UNIT_MASK_UNK3                   = 0x00001000,
    UNIT_MASK_TRAINING_DUMMY         = 0x00002000,
    UNIT_MASK_CREATED_BY_CLASS_SPELL = 0x00004000,
    UNIT_MASK_CREATED_BY_PLAYER      = 0x00008000,
};

namespace Movement { class MoveSpline; }
namespace WorldPackets { namespace CombatLog { class CombatLogServerPacket; } }

struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count)
        : DRGroup(group), stack(0), hitTime(t), hitCount(count)
    {}

    DiminishingGroup        DRGroup:16;
    uint16                  stack:16;
    uint32                  hitTime;
    uint32                  hitCount;
};

enum MeleeHitOutcome
{
    MELEE_HIT_EVADE, MELEE_HIT_MISS, MELEE_HIT_DODGE, MELEE_HIT_BLOCK, MELEE_HIT_PARRY,
    MELEE_HIT_GLANCING, MELEE_HIT_CRIT, MELEE_HIT_CRUSHING, MELEE_HIT_NORMAL
};

struct HealDone
{
    HealDone(uint32 heal, uint32 time)
    : s_heal(heal), s_timestamp(time) {}

    uint32 s_heal;
    uint32 s_timestamp;
};

struct HealTaken
{
    HealTaken(uint32 heal, uint32 time)
    : s_heal(heal), s_timestamp(time) {}

    uint32 s_heal;
    uint32 s_timestamp;
};

struct DamageDone
{
    DamageDone(uint32 dmg, uint32 time)
    : s_damage(dmg), s_timestamp(time) {}

    uint32 s_damage;
    uint32 s_timestamp;
};

struct DamageTaken
{
    DamageTaken(uint32 dmg, uint32 time)
    : s_damage(dmg), s_timestamp(time) {}

    uint32 s_damage;
    uint32 s_timestamp;
};

class DispelInfo
{
private:
    Unit* const m_dispeller;
    uint32 const m_dispellerSpellId;
    uint8 m_chargesRemoved;
public:
    explicit DispelInfo(Unit* _dispeller, uint32 _dispellerSpellId, uint8 _chargesRemoved) :
    m_dispeller(_dispeller), m_dispellerSpellId(_dispellerSpellId), m_chargesRemoved(_chargesRemoved) {}

    Unit* GetDispeller() { return m_dispeller; }
    uint32 GetDispellerSpellId() const { return m_dispellerSpellId; }
    uint8 GetRemovedCharges() const { return m_chargesRemoved; }
    void SetRemovedCharges(uint8 amount)
    {
        m_chargesRemoved = amount;
    }
};

struct CleanDamage
{
    CleanDamage(uint32 mitigated, uint32 absorbed, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome, bool _isAoe = false) :
    absorbed_damage(absorbed), mitigated_damage(mitigated), attackType(_attackType), hitOutCome(_hitOutCome), isAoe(_isAoe) {}

    uint32 absorbed_damage;
    uint32 mitigated_damage;

    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome;

    bool isAoe;
};

enum CastType
{
    SPELL_CAST_TYPE_NULL      = 0,
    SPELL_CAST_TYPE_CLIENT    = 2,
    SPELL_CAST_TYPE_NORMAL    = 3,
    SPELL_CAST_TYPE_ITEM      = 4, //Spell 200452, 193456, 192000
    SPELL_CAST_TYPE_PASSIVE   = 7, //Spell 194461
    SPELL_CAST_TYPE_PET       = 9,
    SPELL_CAST_TYPE_FROM_AURA = 13,
    SPELL_CAST_TYPE_UNK2      = 14, //Spell 3286, 17251
    SPELL_CAST_TYPE_MISSILE   = 16
};

struct TriggerCastData
{
    TriggerCastFlags triggerFlags = TRIGGERED_NONE;
    Item* castItem = nullptr;
    AuraEffect const* triggeredByAura = nullptr;
    ObjectGuid originalCaster = ObjectGuid::Empty;
    int32 casttime = 0;
    ObjectGuid spellGuid = ObjectGuid::Empty;
    bool skipCheck = false;
    bool replaced = false;
    uint32 miscData0 = 0;
    uint32 miscData1 = 0;
    CastType SubType = SPELL_CAST_TYPE_NORMAL;
    uint32 delay = 0;
    uint32 parentTargetCount = 0;
    GuidList parentTargets;
    SpellPowerCost powerCost;
    uint32 auraDuration = 0;
    uint8 setSTack = 0;
};

struct CalcDamageInfo;
struct SpellNonMeleeDamage;

class DamageInfo
{
private:
    Unit* const m_attacker;
    Unit* const m_victim;
    uint32 m_damage;
    uint32 m_overHeal;
    SpellInfo const* const m_spellInfo;
    SpellSchoolMask const m_schoolMask;
    DamageEffectType const m_damageType;
    WeaponAttackType m_attackType;
    uint32 m_absorb;
    uint32 m_resist;
    uint32 m_block;
    uint32 m_cleanDamage;
    uint32 m_damageBeforeHit;
    int32 m_addpower;
    int32 m_addptype;
    uint32 m_startTime = 0;
    uint32 m_castTime = 0;
    ObjectGuid m_magnetGuid;
    uint32 targetInfoMask;
    bool isAoe = false;

public:
    explicit DamageInfo(Unit* _attacker, Unit* _victim, uint32 _damage, SpellInfo const* _spellInfo, SpellSchoolMask _schoolMask, DamageEffectType _damageType, uint32 m_damageBeforeHit);
    explicit DamageInfo(CalcDamageInfo& dmgInfo);
    explicit DamageInfo(SpellNonMeleeDamage& dmgInfo, SpellInfo const* _spellInfo);

    void ModifyDamage(int32 amount);
    void AbsorbDamage(int32 amount);
    void ResistDamage(uint32 amount);
    void BlockDamage(uint32 amount);
    void SetAddPower(int32 amount) { m_addpower = amount; }
    void SetAddPType(int32 amount) { m_addptype = amount; }
    void SetStartCast(int32 amount) { m_startTime = amount; }
    void SetCastTime(int32 amount) { m_castTime = amount; }
    void SetOverHeal(uint32 amount) { m_overHeal = amount; }
    void SetMagnet(ObjectGuid magnet) { m_magnetGuid = magnet; }
    void SetTargetInfoMask(uint32 Mask);

    Unit* GetAttacker() const { return m_attacker; };
    Unit* GetVictim() const { return m_victim; };
    SpellInfo const* GetSpellInfo() const { return m_spellInfo; };
    SpellSchoolMask GetSchoolMask() const { return m_schoolMask; };
    DamageEffectType GetDamageType() const { return m_damageType; };
    WeaponAttackType GetAttackType() const { return m_attackType; };
    uint32 GetDamage() const { return m_damage; };
    uint32 GetAbsorb() const { return m_absorb; };
    uint32 GetResist() const { return m_resist; };
    uint32 GetBlock() const { return m_block; };
    uint32 GetCleanDamage() const { return m_cleanDamage; };
    uint32 GetDamageBeforeHit() const { return m_damageBeforeHit; }
    int32 GetAddPower() const { return m_addpower; };
    int32 GetAddPType() const { return m_addptype; };
    int32 GetStartCast() const { return m_startTime; };
    int32 GetCastTime() const { return m_castTime; };
    uint32 GetOverHeal() const { return m_overHeal; };
    ObjectGuid GetMagnet() const { return m_magnetGuid; };
    bool HasTargetInfoMask(uint32 Mask);
};

class HealInfo
{
    Unit* const m_healer;
    Unit* const m_target;
    uint32 m_heal;
    uint32 m_absorb;
    SpellInfo const* const m_spellInfo;
    SpellSchoolMask const m_schoolMask;
public:
    explicit HealInfo(Unit* _healer, Unit* _target, uint32 _heal, SpellInfo const* _spellInfo, SpellSchoolMask _schoolMask)
        : m_healer(_healer), m_target(_target), m_heal(_heal), m_spellInfo(_spellInfo), m_schoolMask(_schoolMask)
    {
        m_absorb = 0;
    }
    void AbsorbHeal(uint32 amount)
    {
        amount = std::min(amount, GetHeal());
        m_absorb += amount;
        m_heal -= amount;
    }

    uint32 GetHeal() const { return m_heal; };
};

class ProcEventInfo
{
private:
    Unit* const _actor;
    Unit* const _actionTarget;
    Unit* const _procTarget;
    uint32 _typeMask;
    uint32 _spellTypeMask;
    uint32 _spellPhaseMask;
    uint32 _hitMask;
    Spell* _spell;
    DamageInfo* _damageInfo;
    HealInfo* _healInfo;
public:
    explicit ProcEventInfo(Unit* actor, Unit* actionTarget, Unit* procTarget, uint32 typeMask, uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask, Spell* spell, DamageInfo* damageInfo, HealInfo* healInfo);
    Unit* GetActor() { return _actor; };
    Unit* GetActionTarget() const { return _actionTarget; }
    Unit* GetProcTarget() const { return _procTarget; }
    uint32 GetTypeMask() const { return _typeMask; }
    uint32 GetSpellTypeMask() const { return _spellTypeMask; }
    uint32 GetSpellPhaseMask() const { return _spellPhaseMask; }
    uint32 GetHitMask() const { return _hitMask; }
    Spell* GetSpell() const { return _spell; }
    SpellInfo const* GetSpellInfo() const { return nullptr; }
    SpellSchoolMask GetSchoolMask() const { return SPELL_SCHOOL_MASK_NONE; }
    DamageInfo* GetDamageInfo() const { return _damageInfo; }
    HealInfo* GetHealInfo() const { return _healInfo; }
};

// Struct for use in Unit::CalculateMeleeDamage
// Need create structure like in SMSG_ATTACKER_STATE_UPDATE opcode
struct CalcDamageInfo
{
    Unit  *attacker;             // Attacker
    Unit  *target;               // Target for damage
    uint32 damageSchoolMask;
    uint32 damage;
    uint32 absorb;
    uint32 resist;
    uint32 blocked_amount;
    uint32 HitInfo;
    uint32 TargetState;
// Helper
    WeaponAttackType attackType; //
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx;
    uint32 cleanDamage;          // Used only for rage calculation
    uint32 damageBeforeHit;
    MeleeHitOutcome hitOutCome;  // TODO: remove this field (need use TargetState)
    bool isAoe = false;
};

// Spell damage info structure based on structure sending in SMSG_SPELL_NON_MELEE_DAMAGE_LOG opcode
struct SpellNonMeleeDamage
{
    SpellNonMeleeDamage(Unit* _attacker, Unit* _target, uint32 spellID, uint32 spellXSpellVisualID, uint32 _schoolMask, ObjectGuid castGuid = ObjectGuid::Empty);

    Unit* target;
    Unit* attacker;
    ObjectGuid CastGuid;
    uint32 SpellID;
    uint32 SpellXSpellVisualID;
    uint32 damage;
    uint32 schoolMask;
    uint32 absorb;
    uint32 resist;
    uint32 blocked;
    uint32 HitInfo;
    uint32 cleanDamage;
    uint32 damageBeforeHit;
    uint32 preHitHealth;
    bool periodicLog;
    bool isAoe = false;
};

struct SpellPeriodicAuraLogInfo
{
    SpellPeriodicAuraLogInfo(AuraEffect const* _auraEff, uint32 _damage, uint32 _overDamage, uint32 _absorb, uint32 _resist, float _multiplier, bool _critical)
        : auraEff(_auraEff), damage(_damage), overDamage(_overDamage), absorb(_absorb), resist(_resist), multiplier(_multiplier), critical(_critical){}

    AuraEffect const* auraEff;
    uint32 damage;
    uint32 overDamage;                                      // overkill/overheal
    uint32 absorb;
    uint32 resist;
    float  multiplier;
    bool   critical;
};

uint32 createProcExtendMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition);

enum CurrentSpellTypes
{
    CURRENT_MELEE_SPELL             = 0,
    CURRENT_GENERIC_SPELL           = 1,
    CURRENT_CHANNELED_SPELL         = 2,
    CURRENT_AUTOREPEAT_SPELL        = 3,

    CURRENT_MAX_SPELL
};

#define CURRENT_FIRST_NON_MELEE_SPELL 1

struct GlobalCooldown
{
    explicit GlobalCooldown(uint32 _dur = 0, uint32 _time = 0) : duration(_dur), cast_time(_time) {}

    uint32 duration;
    uint32 cast_time;
};

typedef std::unordered_map<uint32 /*category*/, GlobalCooldown> GlobalCooldownList;

class GlobalCooldownMgr                                     // Shared by Player and CharmInfo
{
public:
    GlobalCooldownMgr() {}

public:
    uint32 GetGlobalCooldownEnd(SpellInfo const* spellInfo);
    bool HasGlobalCooldown(SpellInfo const* spellInfo);
    void AddGlobalCooldown(SpellInfo const* spellInfo, uint32 gcd, uint32 time = 0);
    void CancelGlobalCooldown(SpellInfo const* spellInfo);

private:
    GlobalCooldownList m_GlobalCooldowns;
};

enum ActiveStates
{
    ACT_PASSIVE  = 0x01,                                    // 0x01 - passive
    ACT_DISABLED = 0x81,                                    // 0x80 - castable
    ACT_ENABLED  = 0xC1,                                    // 0x40 | 0x80 - auto cast + castable
    ACT_COMMAND  = 0x07,                                    // 0x01 | 0x02 | 0x04
    ACT_REACTION = 0x06,                                    // 0x02 | 0x04
    ACT_DECIDE   = 0x00                                     // custom
};

enum ReactStates
{
    REACT_PASSIVE    = 0,
    REACT_DEFENSIVE  = 1,
    REACT_AGGRESSIVE = 2,
    REACT_HELPER     = 3,
    REACT_ATTACK_OFF = 4
};

enum CommandStates
{
    COMMAND_STAY    = 0,
    COMMAND_FOLLOW  = 1,
    COMMAND_ATTACK  = 2,
    COMMAND_ABANDON = 3,
    COMMAND_MOVE_TO = 4
};

typedef std::list<Player*> SharedVisionList;

enum CharmType
{
    CHARM_TYPE_CHARM,
    CHARM_TYPE_POSSESS,
    CHARM_TYPE_VEHICLE,
    CHARM_TYPE_CONVERT,
};

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE      = 0,
    REACTIVE_HUNTER_PARRY = 1,
};

#define MAX_REACTIVE 3
#define AUTO_SUMMON_SLOT   (-1)
#define SUMMON_SLOT_PET     0
#define SUMMON_SLOT_TOTEM   1
#define MAX_TOTEM_SLOT      5
#define SUMMON_SLOT_MINIPET 5
#define SUMMON_SLOT_QUEST   6
#define MAX_SUMMON_SLOT     18

#define MAX_GAMEOBJECT_SLOT 5

enum Stagger
{
    LIGHT_STAGGER       = 124275,
    MODERATE_STAGGER    = 124274,
    HEAVY_STAGGER       = 124273
};

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200
#define MAX_PLAYER_STEALTH_DETECT_RANGE 30.0f               // max distance for detection targets by player

struct SpellProcEventEntry;                                 // used only privately

#define MAX_DAMAGE_LOG_SECS 120

enum
{
    DAMAGE_DONE_COUNTER     = 0,
    DAMAGE_TAKEN_COUNTER    = 1,
    HEALING_DONE_COUNTER    = 2,
    MAX_DAMAGE_COUNTERS     = 3,
};

enum class ToastType : uint8
{
    ITEM       = 0,
    CURRENCY   = 1,
    MONEY      = 2,
    HONOR      = 3,
};

enum class DisplayToastMethod : uint8
{
    // entry_* - used for all ToastType
    DISPLAY_TOAST_NONE                   = 0,
    DISPLAY_TOAST_DEFAULT                = 1,  // LootToast-Default
    DISPLAY_TOAST_PET_BATTLE_LOOT        = 2,  // triggers PET_BATTLE_LOOT_RECEIVED
    DISPLAY_TOAST_ENTRY_LOOT_PERSONAL_1  = 3,  // in LUA events isPersonal = true
    DISPLAY_TOAST_GARRISON_BONUS_ROLL    = 4,
    DISPLAY_TOAST_ITEM_UPGRADE_1         = 5,  // old draenor quest upgrade
    DISPLAY_TOAST_ITEM_UPGRADE_2         = 6,  // old draenor quest upgrade
    DISPLAY_TOAST_ENTRY_LOOT_PERSONAL_2  = 7,  // in LUA events isPersonal = true
    DISPLAY_TOAST_GARRISON_BONUS_ROLL_2  = 8,
    DISPLAY_TOAST_ENTRY_PVP_FACTION      = 9,  // PvP background (Alliance/Horde) in toast frame
    DISPLAY_TOAST_ENTRY_LOOT_PERSONAL_3  = 10, // in LUA events isPersonal = true
    DISPLAY_TOAST_UNK_LESS_AWESOME       = 11, // in LUA events lessAwesome = true, LootToast-LessAwesome, not glowing and less bright color
    DISPLAY_TOAST_ITEM_WARFORGE          = 12, // LootToast-MoreAwesome
    DISPLAY_TOAST_ITEM_LEGENDARY         = 13, // triggers SHOW_LOOT_TOAST_LEGENDARY_LOOTED
    DISPLAY_TOAST_ENTRY_RATED_PVP_REWARD = 17, // more awesome PvP background (Alliance/Horde) in toast frame

    // Internal Toasts as bool trigger
    DISPLAY_TOAST_UNK                    = 14,
    DISPLAY_TOAST_UNK_1                  = 15,
    DISPLAY_TOAST_SPECIAL_UNK            = 16, // honor points frame displays only with this, but not display other frames, used on retail for quests, internal?
};

typedef cds::container::FeldmanHashSet< cds::gc::HP, Unit*, UnitHashAccessor > UnitSet;
typedef std::set<AuraEffect*> AuraEffectSet;

class Unit : public WorldObject
{
    enum DamageTrackingInfo
    {
        DAMAGE_TRACKING_PERIOD = 120,
        DAMAGE_TRACKING_UPDATE_INTERVAL = 1 * IN_MILLISECONDS
    };

    public:
        typedef std::set<ObjectGuid> ControlList;
        typedef std::multimap<uint32, Aura*> AuraMap;
        typedef std::multimap<uint32, AuraApplicationPtr> AuraApplicationMap;
        typedef std::multimap<AuraStateType,  AuraApplication*> AuraStateAurasMap;
        typedef cds::container::IterableList< cds::gc::HP, AuraEffect*,
                                              cds::container::iterable_list::make_traits<
                cds::container::opt::compare< ComparatorAuraEffect >     // item comparator option
            >::type
         > AuraEffectList;
        typedef std::map<uint32, AuraEffectList*> AuraEffectListMap;
        typedef std::list<Aura*> AuraList;
        typedef std::map<uint32, AuraList> AuraMyMap;
        typedef std::list<AuraApplicationPtr> AuraApplicationList;
        typedef std::list<DiminishingReturn> Diminishing;

        typedef std::vector<AuraApplication*> VisibleAuraVector;
        struct VisibleAuraSlotCompare { bool operator()(AuraApplication* left, AuraApplication* right) const; };
        typedef std::set<AuraApplication*, VisibleAuraSlotCompare> VisibleAuraContainer;

        virtual ~Unit();

        void Clear();
        uint32 GetSize();

        UnitAI* GetAI() { return i_AI; }
        void SetAI(UnitAI* newAI) { i_AI = newAI; }
        void SetDisabledCurrentAI() { i_disabledAI = i_AI; i_AI = nullptr; }

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        void CleanupBeforeRemoveFromMap(bool finalCleanup);
        void CleanupsBeforeDelete(bool finalCleanup = true) override;                        // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)
        void CleanupBeforeTeleport();

        DiminishingLevels GetDiminishing(DiminishingGroup  group);
        void IncrDiminishing(DiminishingGroup group);
        float ApplyDiminishingToDuration(DiminishingGroup  group, int32 &duration, Unit* caster, DiminishingLevels Level, int32 limitduration);
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        void ClearDiminishings() { m_Diminishing.clear(); }
        uint32 DiminishingDuration() const;

        void SendCombatLogMessage(WorldPackets::CombatLog::CombatLogServerPacket* combatLog) const;

        // target dependent range checks
        float GetSpellMaxRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;
        float GetSpellMinRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;

        virtual void Update(uint32 time);

        void setAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        uint32 getAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const { return m_attackTimer[type] == 0; }
        void SetAttackTimerFraction(WeaponAttackType type, uint32 time);
        uint32 GetAttackTimerFraction(WeaponAttackType type) const;
        bool haveOffhandWeapon() const;
        bool CanDualWield() const { return m_canDualWield; }
        void SetCanDualWield(bool value) { m_canDualWield = value; }
        float GetModelSize(uint32 displayId = 0) const;
        float GetBoundingRadius() const { return m_floatValues[UNIT_FIELD_BOUNDING_RADIUS]; }
        float GetCombatReach() const { return m_floatValues[UNIT_FIELD_COMBAT_REACH]; }
        float GetMeleeReach() const { float reach = m_floatValues[UNIT_FIELD_COMBAT_REACH]; return reach > MIN_MELEE_REACH ? reach : MIN_MELEE_REACH; }
        bool IsWithinCombatRange(const Unit* obj, float dist2compare) const;
        bool IsWithinMeleeRange(const Unit* obj, float dist = MELEE_RANGE) const;
        uint32 m_extraAttacks;
        bool m_canDualWield;
        float countCrit;

        void _addAttacker(Unit* pAttacker);                  // must be called only from Unit::Attack(Unit*)
        void _removeAttacker(Unit* pAttacker);               // must be called only from Unit::AttackStop()
        Unit* getAttackerForHelper() const;                 // If someone wants to help, who to give them
        bool hasAttacker(Unit* pAttacker);

        bool Attack(Unit* victim, bool meleeAttack);
        void CastStop(uint32 except_spellid = 0, bool interruptStun = false);
        bool AttackStop();
        void RemoveAllAttackers();
        UnitSet* getAttackers() { return &m_attackers; }
        bool isAttackingPlayer() const;
        Unit* getVictim() const { return m_attacking; }
        void UpdateVictim(Unit* victim);

        void CombatStop(bool includingCast = false);
        void CombatStopWithPets(bool includingCast = false);
        void StopAttackFaction(uint32 faction_id);
        Unit* SelectNearbyTarget(Unit* exclude = nullptr, float dist = NOMINAL_MELEE_RANGE) const;
        Unit* SelectNearbyAlly(Unit* exclude = nullptr, float dist = NOMINAL_MELEE_RANGE, bool checkValidAssist = false) const;
        Unit* GetNearbyVictim(Unit* exclude = nullptr, float dist = NOMINAL_MELEE_RANGE, bool IsInFront = false, bool IsNeutral = false) const;
        void SendMeleeAttackStop(Unit* victim = nullptr);
        void SendMeleeAttackStart(Unit* victim);
        bool IsVisionObscured(Unit* victim);

        void AddUnitState(uint32 f) { m_state |= f; }
        bool HasUnitState(const uint32 f) const { return (m_state & f) != 0; }
        void ClearUnitState(uint32 f) { m_state &= ~f; }
        void ClearAllUnitState() { m_state = 0; }
        uint32 GetUnitState() const { return m_state; }
        bool CanFreeMove() const;

        void SetSplineTimer(uint32 interval);

        uint32 HasUnitTypeMask(uint32 mask) const { return (mask & m_unitTypeMask) != 0; }
        void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }
        bool isSummon() const   { return (m_unitTypeMask & UNIT_MASK_SUMMON) != 0; }
        bool isGuardian() const { return (m_unitTypeMask & UNIT_MASK_GUARDIAN) != 0; }
        bool isPet() const      { return (m_unitTypeMask & UNIT_MASK_PET) != 0; }
        bool isHunterPet() const{ return (m_unitTypeMask & UNIT_MASK_HUNTER_PET) != 0; }
        bool isTotem() const    { return (m_unitTypeMask & UNIT_MASK_TOTEM) != 0; }
        bool IsVehicle() const  { return (m_unitTypeMask & UNIT_MASK_VEHICLE) != 0; }
        bool isMinion() const   { return (m_unitTypeMask & UNIT_MASK_MINION) != 0; }
        bool isTrainingDummy() const { return m_unitTypeMask & UNIT_MASK_TRAINING_DUMMY; }
        bool isAnySummons() const;
        bool CanVehicleAI() const;

        uint8 getLevel() const;
        uint8 GetEffectiveLevel() const;
        uint8 getLevelForTarget(WorldObject const* target) const override;
        float getScaleForTarget(int32 delta) const;
        uint32 GetDamageFromLevelScale(Unit* target, uint32 damage);
        void SetLevel(uint8 lvl);
        void SetEffectiveLevel(uint8 lvl);
        void SetMaxItemLevel(uint16 ilvl);
        void SetMinItemLevel(uint16 ilvl);
        uint8 getRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE); }
        uint64 getRaceMask() const { return UI64LIT(1) << (getRace() - 1); }
        uint8 getClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS); }
        uint32 getClassMask() const { return 1 << (getClass()-1); }
        uint8 getGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER); }
        void SetGender(uint8 gender) { SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, gender); }
        void SetRace(uint8 race) { SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE, race); }
        void SetClass(uint8 newClass) { SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, newClass); }
        
        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STATS + stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STATS + stat, val); }
        uint32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL); }
        uint32 GetArmor(Unit* victim) const;
        void SetArmor(int32 val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }
        void ApplyDiminishingStat(float& pct, uint8 whichClass);

        uint32 GetResistance(SpellSchools school) const { return GetUInt32Value(UNIT_FIELD_RESISTANCES+school); }
        uint32 GetResistance(SpellSchoolMask mask) const;
        void SetResistance(SpellSchools school, int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES+school, val); }

        uint64 GetHealth(Unit* victim) const;
        uint64 GetHealth() const { return GetUInt64Value(UNIT_FIELD_HEALTH); }
        uint64 GetMaxHealth(Unit* victim) const;
        uint64 GetMaxHealth() const { return GetUInt64Value(UNIT_FIELD_MAX_HEALTH); }

        bool IsFullHealth() const { return GetHealth() == GetMaxHealth(); }
        bool HealthBelowPct(int32 pct) const { return GetHealth() < CountPctFromMaxHealth(pct); }
        bool HealthBelowPctDamaged(int32 pct, uint32 damage) const { return int64(GetHealth()) - int64(damage) < int64(CountPctFromMaxHealth(pct)); }
        bool HealthAbovePct(int32 pct) const { return GetHealth() > CountPctFromMaxHealth(pct); }
        bool HealthAbovePctHealed(int32 pct, uint32 heal) const { return uint64(GetHealth()) + uint64(heal) > CountPctFromMaxHealth(pct); }
        float GetHealthPct() const { return GetMaxHealth() ? 100.f * GetHealth() / GetMaxHealth() : 0.0f; }
        float GetPowerPct(Powers power) const { return GetMaxPower(power) ? 100.f * GetPower(power) / GetMaxPower(power) : 0.0f; }
        uint64 CountPctFromMaxHealth(int32 pct, Unit* victim = nullptr) const { return CalculatePct(GetMaxHealth(victim), pct); }
        uint64 CountPctFromCurHealth(int32 pct, Unit* victim = nullptr) const { return CalculatePct(GetHealth(victim), pct); }
        uint32 CountPctFromMaxMana(int32 pct) const { return CalculatePct(GetMaxPower(POWER_MANA), pct); }
        uint32 CountPctFromCurMana(int32 pct) const { return CalculatePct(GetPower(POWER_MANA), pct); }
        uint32 CountPctFromMaxPower(int32 pct, Powers power) const { return CalculatePct(GetMaxPower(power), pct); }
        uint32 CountPctFromCurPower(int32 pct, Powers power) const { return CalculatePct(GetPower(power), pct); }

        void SetHealth(uint64 val, uint32 spellId = 0);
        void SetHealthScal(uint64 val, Unit* victim = nullptr, uint32 spellId = 0);
        void SetMaxHealth(uint64 val);

        void SetFullHealth() { SetHealth(GetMaxHealth()); }
        int64 ModifyHealth(int64 val, Unit* victim = nullptr, uint32 spellId = 0);
        int64 GetHealthGain(int64 dVal);

        Powers getPowerType() const { return Powers(GetUInt32Value(UNIT_FIELD_DISPLAY_POWER)); }
        void SetFieldPowerType(uint32 powerType) { SetUInt32Value(UNIT_FIELD_DISPLAY_POWER, powerType); }
        void setPowerType(Powers power);
        int32 GetPower(Powers power) const;
        int32 GetMaxPower(Powers power) const;
        void SetPower(Powers power, int32 val, bool send = true);
        void InitialPowers(bool maxpower = false);
        void ResetPowers(float perc = 0.0f, bool duel = false);
        void SetMaxPower(Powers power, int32 val);
        int32 GetPowerForReset(Powers power, uint16 powerDisplayID = 0) const;
        void VisualForPower(Powers power, int32 curentVal, int32 modVal = 0, int32 maxPower = 0, bool generate = false, SpellInfo const* spellInfo = nullptr);
        void UpdatePowerState(bool inCombat = true);
        Powers GetPowerTypeBySpecId(uint32 specID);

        void SaveAddComboPoints(int8 count) { m_comboSavePoints += count; }
        uint8 GetSaveComboPoints() const { return m_comboSavePoints; }

        uint8 GetComboPoints() const;
        void AddComboPoints(int8 count);
        void SetPowerCost(SpellPowerCost power);
        void ResetPowerCost();
        int32 GetPowerCost(int8 power);
        int32 ModifyPower(Powers power, int32 val, bool set = false, SpellInfo const* spellInfo = nullptr);
        int32 ModifyPowerPct(Powers power, float pct, bool apply = true);
        uint32 GetPowerIndex(uint32 powerType) const;

        uint32 GetAttackTime(WeaponAttackType att) const;
        void SetAttackTime(WeaponAttackType att, uint32 val);
        void CalcAttackTimePercentMod(WeaponAttackType att, float val);

        SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE)); }
        virtual void SetSheath(SheathState sheathed) { SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE, sheathed); }

        // faction template id
        uint32 getFaction() const { return GetUInt32Value(UNIT_FIELD_FACTION_TEMPLATE); }
        void setFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTION_TEMPLATE, faction); }
        FactionTemplateEntry const* getFactionTemplateEntry() const;

        ReputationRank GetReactionTo(Unit const* target) const;
        ReputationRank static GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, Unit const* target);

        bool IsHostileTo(Unit const* unit) const;
        bool IsHostileToPlayers() const;
        bool IsFriendlyTo(Unit const* unit) const;
        bool IsNeutralToAll() const;
        bool IsInPartyWith(Unit const* unit) const;
        bool IsInRaidWith(Unit const* unit) const;
        void GetPartyMembers(std::list<Unit*> &units);
        void GetRaidMembers(std::list<Unit*> &tagUnitMap);
        bool IsContestedGuard() const;

        bool IsPvP() const;
        bool IsFFAPvP() const;
        virtual void SetPvP(bool state);
        uint32 GetCreatureType() const;
        uint32 GetCreatureTypeMask() const;

        bool IsSitState() const;
        bool IsStandState() const;
        void SetStandState(uint8 state, uint32 animKitID = 0);

        void setStandStateFlag(uint8 state) { SetByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE, state); }
        void setStandStateValue(uint8 value) { SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE, value); } 
        uint8 getStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE); }
        void  RemoveStandStateFlags(uint8 flags) { RemoveByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE, flags); }

        void  SetStandVisFlags(uint8 flags) { SetByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }
        void  SetStandVisValue(uint8 value) { SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, value); }
        uint8 GetStandVisValue() const { return GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_ANIM_TIER); }
        void  RemoveStandVisFlags(uint8 flags) { RemoveByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }

        void  SetMiscStandFlags(uint8 flags) { SetByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_ANIM_TIER, flags); }
        void  SetMiscStandValue(uint8 value) { SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_ANIM_TIER, value); }
        uint8 GetMiscStandValue() const { return GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_ANIM_TIER); }
        void  RemoveMiscStandFlags(uint8 flags) { RemoveByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_ANIM_TIER, flags); }

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID); }
        void Mount(uint32 mount, uint32 vehicleId = 0, uint32 creatureEntry = 0);
        void Dismount();
        void UpdateMount();
        MountCapabilityEntry const* GetMountCapability(uint32 mountType) const;
        bool NeedDismount();

        void SendDurabilityLoss(Player* receiver, uint32 percent);

        void SetAnimTier(uint32 tier);
        void PlayOneShotAnimKit(uint16 animKitID);
        void SetAnimKitId(uint16 animKitID);
        uint16 GetAIAnimKitId() const override { return _aiAnimKitId; }
        void SetMovementAnimKitId(uint16 animKitID);
        uint16 GetMovementAnimKitId() const override { return _movementAnimKitId; }
        void SetMeleeAnimKitId(uint16 animKitID);
        uint16 GetMeleeAnimKitId() const override { return _meleeAnimKitId; }

        uint16 GetMaxSkillValueForLevel(Unit const* target = nullptr) const { return (target ? getLevelForTarget(target) : getLevel()) * 5; }
        void DealDamageMods(Unit* victim, uint32 &damage, uint32* absorb, SpellInfo const* spellProto = nullptr, bool addMythicMod = false);
        uint32 DealDamage(Unit* victim, uint32 damage, CleanDamage const* cleanDamage = nullptr, DamageEffectType damagetype = DIRECT_DAMAGE, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const* spellProto = nullptr, bool durabilityLoss = true);
        uint32 CalcStaggerDamage(uint32 damage, SpellSchoolMask damageSchoolMask, SpellInfo const* spellInfo = nullptr, bool full = false);
        void Kill(Unit* victim, bool durabilityLoss = true, SpellInfo const* spellProto = nullptr);
        int32 DealHeal(Unit* victim, uint32 addhealth, SpellInfo const* spellProto = nullptr);
        int32 InterceptionOfDamage(Unit* victim, uint32 damage, SpellSchoolMask damageSchoolMask);

        void ProcDamageAndSpell(Unit* victim, uint32 procAttacker, uint32 procVictim, uint32 procEx, DamageInfo* dmgInfoProc, WeaponAttackType attType = BASE_ATTACK, SpellInfo const* procSpell = nullptr, SpellInfo const* procAura = nullptr, Uint32Set* AppliedProcMods = nullptr, Spell* spell = nullptr);
        void ProcDamageAndSpellFor(bool isVictim, Unit* target, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellInfo const* procSpell, DamageInfo* dmgInfoProc, SpellInfo const* procAura = nullptr, Uint32Set* AppliedProcMods = nullptr, Spell* spell = nullptr);

        void GetProcAurasTriggeredOnEvent(std::list<AuraApplication*>& aurasTriggeringProc, std::list<AuraApplication*>* procAuras, ProcEventInfo eventInfo);
        void TriggerAurasProcOnEvent(CalcDamageInfo& damageInfo);
        void TriggerAurasProcOnEvent(std::list<AuraApplication*>* myProcAuras, std::list<AuraApplication*>* targetProcAuras, Unit* actionTarget, uint32 typeMaskActor, uint32 typeMaskActionTarget, uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask, Spell* spell, DamageInfo* damageInfo, HealInfo* healInfo);
        void TriggerAurasProcOnEvent(ProcEventInfo& eventInfo, std::list<AuraApplication*>& procAuras);

        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate(Unit* victim, WeaponAttackType attType = BASE_ATTACK, bool extra = false, uint32 replacementAttackTrigger = 0, uint32 replacementAttackAura = 0, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);

        void CalculateMeleeDamage(Unit* victim, uint32 damage, CalcDamageInfo* damageInfo, WeaponAttackType attackType = BASE_ATTACK, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DealMeleeDamage(CalcDamageInfo* damageInfo, bool durabilityLoss);
        void HandleProcExtraAttackFor(Unit* victim);
        int32 CalculateMonkSpellDamage(float coeff);

        void CalculateSpellDamageTaken(SpellNonMeleeDamage* damageInfo, float damage, SpellInfo const* spellInfo, uint32 effectMask, WeaponAttackType attackType = BASE_ATTACK, float critMod = 0.f, bool crit = false);
        void DealSpellDamage(SpellNonMeleeDamage* damageInfo, bool durabilityLoss);

        // player or player's pet resilience (-1%)
        uint32 GetDamageReduction(uint32 damage) const { return GetCombatRatingDamageReduction(CR_RESILIENCE_PLAYER_DAMAGE, 100.0f, damage); }

        void ApplyResilience(const Unit* victim, float * damage) const;

        float MeleeSpellMissChance(Unit const* victim, WeaponAttackType attType, uint32 spellId) const;
        SpellMissInfo MeleeSpellHitResult(Unit* victim, SpellInfo const* spellInfo) const;
        SpellMissInfo MagicSpellHitResult(Unit* victim, SpellInfo const* spellInfo) const;
        SpellMissInfo SpellHitResult(Unit* victim, SpellInfo const* spellInfo, bool canReflect = false, uint32 effectMask = 0);

        float GetUnitMissChance(Unit const* attacker, uint32 spellId = 0) const;
        float GetUnitDodgeChance(WeaponAttackType attType, Unit const* victim) const;
        float GetUnitParryChance(WeaponAttackType attType, Unit const* victim) const;
        float GetUnitBlockChance(WeaponAttackType attType, Unit const* victim) const;
        float GetUnitCriticalChance(WeaponAttackType attackType, Unit const* victim) const;
        int32 GetMechanicResistChance(SpellInfo const* spellInfo) const;
        bool CanUseAttackType(uint8 attacktype) const;

        virtual uint32 GetBlockPercent() { return 30; }

        Trinity::AnyDataContainer& GetAnyDataContainer();

        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM, const SpellInfo* spellProto) const;

        MeleeHitOutcome RollMeleeOutcomeAgainst(Unit const* victim, WeaponAttackType attType) const;

        bool isVendor() const;
        bool isTrainer() const;
        bool isQuestGiver() const;
        bool isGossip() const;
        bool isTaxi() const;
        bool isGuildMaster() const;
        bool isBattleMaster() const;
        bool isBanker() const;
        bool isInnkeeper() const;
        bool isSpiritHealer() const;
        bool isSpiritGuide() const;
        bool isTabardDesigner()const;
        bool isAuctioner() const;
        bool isArmorer() const;
        bool isWildBattlePet() const;
        bool isServiceProvider() const;
        bool isSpiritService() const;
        bool isInFlight() const;
        bool isInCombat() const;

        void CombatStart(Unit* target, bool initialAggro = true);
        void SetInCombatState(Unit* enemy = nullptr, bool PvP = false);
        void SetInCombatWith(Unit* enemy);
        void ClearInCombat();
        uint32 GetCombatTimer() const { return m_CombatTimer; }
        void SetCombatTimer(uint32 amount) { m_CombatTimer = amount; }

        bool HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName, uint32 familyFlags) const;
        bool virtual HasSpell(uint32 /*spellID*/) { return false; }
        bool HasCrowdControlAuraType(AuraType type, uint32 excludeAura = 0) const;
        bool HasCrowdControlAura(Unit* excludeCasterChannel = nullptr) const;

        bool HasStealthAura()      const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
        bool HasInvisibilityAura() const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
        bool isFeared()  const { return (HasAuraType(SPELL_AURA_MOD_FEAR) || HasAuraType(SPELL_AURA_MOD_FEAR_2)); }
        bool isInRoots() const { return HasAuraType(SPELL_AURA_MOD_ROOT) || HasAuraType(SPELL_AURA_MOD_ROOTED); }
        bool IsPolymorphed() const;

        bool isFrozen() const;

        bool isTargetableForAttack(bool checkFakeDeath = true) const;

        bool IsValidAttackTarget(Unit const* target) const;
        bool _IsValidAttackTarget(Unit const* target, SpellInfo const* bySpell, WorldObject const* obj = nullptr) const;

        bool IsValidAssistTarget(Unit const* target) const;
        bool _IsValidAssistTarget(Unit const* target, SpellInfo const* bySpell) const;

        bool IsInWater() const override;
        bool IsUnderWater() const override;
        virtual void UpdateUnderwaterState(Map* m, float x, float y, float z);
        bool isInAccessiblePlaceFor(Creature const* c) const;

        void SendHealSpellLog(Unit* victim, uint32 SpellID, uint32 Damage, uint32 OverHeal, uint32 Absorb, bool critical = false);
        int32 HealBySpell(Unit* victim, SpellInfo const* spellInfo, uint32 addHealth, bool critical = false);
        void SendEnergizeSpellLog(Unit* victim, uint32 spellID, int32 damage, int32 overEnergize, Powers powertype);
        void EnergizeBySpell(Unit* victim, uint32 SpellID, int32 Damage, Powers powertype);

        void CastSpellDuration(Unit* victim, uint32 spellId, bool triggered, uint32 duration = 0, uint32 stack = 0, uint32 timeCast = 0, AuraEffect const* triggeredByAura = nullptr);
        void CastSpellTime(Unit* victim, uint32 spellId, bool triggered, uint32 castTime = 0);
        void CastSpellTime(float x, float y, float z, uint32 spellId, bool triggered, uint32 castTime = 0);
        void CastSpell(SpellCastTargets const& targets, SpellInfo const* spellInfo, CustomSpellValues const* value, TriggerCastData& triggerData);
        void CastSpell(SpellCastTargets const& targets, SpellInfo const* spellInfo, CustomSpellValues const* value, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(Unit* victim, uint32 spellId, bool triggered, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(Unit* victim, uint32 spellId, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(Unit* victim, SpellInfo const* spellInfo, bool triggered, Item* castItem= nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(Unit* victim, SpellInfo const* spellInfo, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem= nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(G3D::Vector3 pos, uint32 spellId, bool triggered, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(WorldLocation const* loc, uint32 spellId, bool triggered, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(Position pos, uint32 spellId, bool triggered, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(float x, float y, float z, uint32 spellId, TriggerCastFlags triggeredCastFlags = TRIGGERED_NONE, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastCustomSpell(Unit* Victim, uint32 spellId, float const* bp0, float const* bp1, float const* bp2, bool triggered, Item* castItem= nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastCustomSpell(Unit* Victim, uint32 spellId, float const* bp0, float const* bp1, float const* bp2, TriggerCastFlags triggeredCastFlags = TRIGGERED_NONE, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastCustomSpell(float x, float y, float z, uint32 spellId, float const * bp0, float const * bp1, float const * bp2, TriggerCastFlags triggeredCastFlags = TRIGGERED_NONE, Item * castItem = nullptr, AuraEffect const * triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastCustomSpell(Unit* Victim, uint32 spellId, float const* bp0, float const* bp1, float const* bp2, float const* bp3, float const* bp4, float const* bp5, bool triggered, Item* castItem= nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastCustomSpell(uint32 spellId, SpellValueMod mod, int32 value, Unit* Victim = nullptr, bool triggered = true, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastCustomSpell(uint32 spellId, CustomSpellValues const &value, Unit* Victim = nullptr, bool triggered = true, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpell(GameObject* go, uint32 spellId, bool triggered, Item* castItem = nullptr, AuraEffect* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpellOnAttackablTarget(uint32 spellId, float dist, Unit* exclude, uint32 targetCount);
        void CastSpellDelay(Unit* victim, uint32 spellId, bool triggered, uint32 delay, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        void CastSpellDelay(Position pos, uint32 spellId, bool triggered, uint32 delay, Item* castItem = nullptr, AuraEffect const* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid::Empty);
        Aura* ToggleAura(uint32 spellId, Unit* target);
        Aura* AddAura(uint32 spellId, Unit* target, Item* castItem = nullptr, uint16 stackAmount = 0, int32 Duration = 0, int32 MaxDuration = 0);
        Aura* AddAura(SpellInfo const* spellInfo, uint32 effMask, Unit* target, Item* castItem = nullptr, uint16 stackAmount = 0, int32 Duration = 0, int32 MaxDuration = 0);
        void SetAuraStack(uint32 spellId, Unit* target, uint32 stack);
        void SendPlaySpellVisualKit(uint32 id, uint32 kitType, uint32 duration = 0);

        void DeMorph();

        void SendAttackStateUpdate(CalcDamageInfo* damageInfo);
        void SendAttackStateUpdate(uint32 HitInfo, Unit* target, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(SpellNonMeleeDamage* log);
        void SendSpellNonMeleeDamageLog(Unit* target, uint32 SpellID, uint32 Damage, SpellSchoolMask damageSchoolMask, uint32 AbsorbedDamage, uint32 Resist, bool PhysicalDamage, uint32 Blocked, uint32 spellXSpellVisualID, bool CriticalHit = false);
        void SendPeriodicAuraLog(SpellPeriodicAuraLogInfo* pInfo);
        void SendSpellMiss(Unit* target, uint32 spellID, SpellMissInfo missInfo);
        void SendSpellDamageResist(Unit* target, uint32 spellId);
        void SendSpellDamageImmune(Unit* target, uint32 spellId, bool isPeriodic);

        void NearTeleportTo(const Position &pos)
        { NearTeleportTo(pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation); }
        void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false, bool stopMove = true);
        virtual bool UpdatePosition(float x, float y, float z, float ang, bool teleport = false, bool stop = false);
        // returns true if unit's position really changed
        virtual bool UpdatePosition(const Position &pos, bool teleport = false) { return UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport); }
        void UpdateOrientation(float orientation);
        void UpdateHeight(float newZ);
        void SendTeleportPacket(Position &oldPos);

        void SendMoveKnockBack(Player* player, float speedXY, float speedZ, float vcos, float vsin);
        void KnockbackFrom(float x, float y, float speedXY, float speedZ, Movement::SpellEffectExtraData const* spellEffectExtraData = nullptr);
        void JumpTo(float speedXY, float speedZ, float angle = 0.0f);
        void JumpTo(WorldObject* obj, float speedZ);

        void MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath = false, bool forceDestination = false);
        void MonsterSmothMoveWithSpeed(Position pos, float speed, bool generatePath = false, bool forceDestination = false);

        //void SetFacing(float ori, WorldObject* obj = NULL);
        //void SendMonsterMove(float NewPosX, float NewPosY, float NewPosZ, uint8 type, uint32 MovementFlags, uint32 Time, Player* player = NULL);
        void SendMovementFlagUpdate(bool self = false);

        /*! These methods send the same packet to the client in apply and unapply case.
            The client-side interpretation of this packet depends on the presence of relevant movementflags
            which are sent with movementinfo. Furthermore, these packets are broadcast to nearby players as well
            as the current unit.
        */

        bool IsLevitating() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);}
        bool IsWalking() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING);}
        bool SetWalk(bool enable);
        bool SetDisableGravity(bool disable, bool isPlayer = false);
        bool SetFall(bool enable, bool isPlayer = false);
        bool SetSwim(bool enable);
        bool SetCanFly(bool apply);
        bool SetWaterWalking(bool enable);
        bool SetFeatherFall(bool enable);
        bool SetHover(bool enable);
        bool SetCollision(bool disable);

        void SetInFront(Unit const* target);
        void SetFacingTo(float ori);
        void SetFacingTo(Unit const* target);
        void SetFacingToObject(WorldObject* object);

        void SendHighestThreatUpdate(HostileReference* pHostileReference);
        void SendThreatClear();
        void SendThreatRemove(HostileReference* pHostileReference);
        void SendThreatUpdate();

        void SendBreakTarget(Unit* victim);

        bool isAlive() const { return m_deathState == ALIVE; };
        bool isDying() const { return m_deathState == JUST_DIED; };
        bool isDead(bool withFeign = true) const;
        DeathState getDeathState() { return m_deathState; };
        virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        ObjectGuid GetSummonedByGUID() const { return GetGuidValue(UNIT_FIELD_SUMMONED_BY); }
        ObjectGuid GetDemonCreatorGUID() const { return GetGuidValue(UNIT_FIELD_DEMON_CREATOR); }
        void SetOwnerGUID(ObjectGuid owner);
        ObjectGuid GetOwnerGUID() const { return GetGuidValue(UNIT_FIELD_CREATED_BY); }
        ObjectGuid GetCreatorGUID() const { return GetGuidValue(UNIT_FIELD_CREATED_BY); }
        void SetCreatorGUID(ObjectGuid creator) { SetGuidValue(UNIT_FIELD_CREATED_BY, creator); }
        ObjectGuid GetMinionGUID() const { return GetGuidValue(UNIT_FIELD_SUMMON); }
        void SetMinionGUID(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_SUMMON, guid); }
        ObjectGuid GetCharmerGUID() const { return GetGuidValue(UNIT_FIELD_CHARMED_BY); }
        void SetCharmerGUID(ObjectGuid owner) { SetGuidValue(UNIT_FIELD_CHARMED_BY, owner); }
        ObjectGuid GetCharmGUID() const { return  GetGuidValue(UNIT_FIELD_CHARM); }
        void SetPetGUID(ObjectGuid guid) { m_SummonSlot[SUMMON_SLOT_PET] = guid; }
        ObjectGuid GetPetGUID() const { return m_SummonSlot[SUMMON_SLOT_PET]; }
        void SetCritterGUID(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_CRITTER, guid); }
        ObjectGuid GetCritterGUID() const { return GetGuidValue(UNIT_FIELD_CRITTER); }

        bool IsControlledByPlayer() const { return m_ControlledByPlayer; }
        ObjectGuid GetCharmerOrOwnerGUID() const { return !GetCharmerGUID().IsEmpty() ? GetCharmerGUID() : GetOwnerGUID(); }
        ObjectGuid GetCharmerOrOwnerOrOwnGUID() const
        {
            ObjectGuid guid = GetCharmerOrOwnerGUID();
            if (!guid.IsEmpty())
                return guid;

            return GetGUID();
        }
        bool isCharmedOwnedByPlayerOrPlayer() const { return GetCharmerOrOwnerOrOwnGUID().IsPlayer(); }

        Player* GetSpellModOwner() const;

        bool IsOwnerOrSelf(Unit* owner) const;
        Unit* GetOwner() const;
        Unit* GetAnyOwner() const;
        Guardian *GetGuardianPet() const;
        Minion *GetFirstMinion() const;
        Unit* GetCharmer() const;
        Unit* GetCharm() const;
        Unit* GetCharmerOrOwner() const;
        Unit* GetCharmerOrOwnerOrSelf() const;
        Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;
        Player* GetAffectingPlayer() const;

        void SetMinion(Minion *minion, bool apply);
        Creature* GetMinionByEntry(uint32 entry);
        void GetAllMinionsByEntry(std::list<Creature*>& Minions, uint32 entry);
        void RemoveAllMinionsByFilter(uint32 entry, uint8 filter = 0);
        void SetCharm(Unit* target, bool apply);
        Unit* GetNextRandomRaidMemberOrPet(float radius);
        bool SetCharmedBy(Unit* charmer, CharmType type, AuraApplication const* aurApp = nullptr);
        void RemoveCharmedBy(Unit* charmer);
        void RestoreFaction();

        ControlList m_Controlled;

        Unit* GetFirstControlled() const;
        void RemoveAllControlled();
        void TeleportAllControlled();
        void RestoreAllControlled();

        bool isCharmed() const;
        bool isPossessed() const;
        bool isPossessedByPlayer() const;
        bool isPossessing() const;
        bool isPossessing(Unit* u) const;

        CharmInfo* GetCharmInfo() { return m_charmInfo; }
        CharmInfo* InitCharmInfo();
        void DeleteCharmInfo();
        void UpdateCharmAI();
        Unit* GetMover() const;
        Player* GetPlayerMover() const;
        Player* m_movedPlayer;
        SharedVisionList const& GetSharedVisionList() { return m_sharedVision; }
        void AddPlayerToVision(Player* player);
        void RemovePlayerFromVision(Player* player);
        bool HasSharedVision() const { return !m_sharedVision.empty(); }
        void RemoveBindSightAuras();
        void RemoveCharmAuras();

        Pet* CreateTamedPetFrom(Creature* creatureTarget, uint32 spell_id = 0);
        Pet* CreateTamedPetFrom(uint32 creatureEntry, uint32 spell_id = 0);
        bool InitTamedPet(Pet* pet, uint32 spell_id);

        // aura apply/remove helpers - you should better not use these
        Aura* _TryStackingOrRefreshingExistingAura(SpellInfo const* newAura, uint32 effMask, Unit* caster, float* baseAmount = nullptr, Item* castItem = nullptr, ObjectGuid casterGUID = ObjectGuid::Empty);
        void _AddAura(UnitAura* aura, Unit* caster);
        AuraApplication * _CreateAuraApplication(Aura* aura, uint32 effMask);
        void _ApplyAuraEffect(Aura* aura, uint32 effIndex);
        void _ApplyAura(AuraApplication * aurApp, uint32 effMask);
        void _UnapplyAura(AuraApplicationMap::iterator &i, AuraRemoveMode removeMode);
        void _UnapplyAura(AuraApplication * aurApp, AuraRemoveMode removeMode);
        void _RemoveNoStackAurasDueToAura(Aura* aura);
        void _RegisterAuraEffect(AuraEffect* aurEff, bool apply);

        // m_ownedAuras container management
        AuraMap      & GetOwnedAuras()       { return m_ownedAuras; }
        AuraMap const& GetOwnedAuras() const { return m_ownedAuras; }

        void RemoveOwnedAura(AuraMap::iterator &i, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveOwnedAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveOwnedAura(Aura* aura, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveOwnedAuraAll();

        Aura* GetOwnedAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, Aura* except = nullptr) const;

        // m_appliedAuras container management
        AuraApplicationMap      & GetAppliedAuras()       { return m_appliedAuras; }
        AuraApplicationMap const& GetAppliedAuras() const { return m_appliedAuras; }

        void RemoveAura(AuraApplicationMap::iterator &i, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(AuraApplication * aurApp, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(Aura* aur, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);

        void RemoveOwnedAuras(std::function<bool(Aura const*)> const& check);
        void RemoveOwnedAuras(uint32 spellId, std::function<bool(Aura const*)> const& check);

        void RemoveAppliedAuras(std::function<bool(AuraApplicationPtr)> const& check);
        void RemoveAppliedAuras(uint32 spellId, std::function<bool(AuraApplicationPtr)> const& check, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

        void RemoveAurasDueToSpell(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAuraFromStack(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT, int32 num = 1);
        void RemoveAurasDueToSpellByDispel(uint32 spellId, uint32 dispellerSpellId, ObjectGuid casterGUID, Unit* dispeller, uint8 chargesRemoved = 1);
        void RemoveAurasDueToSpellBySteal(uint32 spellId, ObjectGuid casterGUID, Unit* stealer);
        void RemoveAurasDueToItemSpell(Item* castItem, uint32 spellId);
        void RemoveAurasByType(AuraType auraType, ObjectGuid casterGUID = ObjectGuid::Empty, Aura* except = nullptr, bool negative = true, bool positive = true);
        void RemoveAurasByType(AuraType auraType, std::function<bool(AuraApplication const*)> const& check);
        void RemoveNotOwnSingleTargetAuras(uint32 newPhase = 0x0);

        template <typename InterruptFlags>
        uint32 RemoveAurasWithInterruptFlags(InterruptFlags flag, uint32 spellID = 0, uint32 except = 0);

        void RemoveAurasWithAttribute(uint32 flags);
        void RemoveAurasWithFamily(SpellFamilyNames family, uint32 familyFlag1, uint32 familyFlag2, uint32 familyFlag3, ObjectGuid casterGUID);
        void RemoveAurasWithMechanic(uint32 mechanic_mask, AuraRemoveMode removemode = AURA_REMOVE_BY_MECHANIC, uint32 except=0);
        bool HasAurasWithMechanic(uint32 mechanic_mask, ObjectGuid caster = ObjectGuid::Empty);
        void RemoveMovementImpairingEffects();

        void RemoveAreaAurasDueToLeaveWorld();
        void RemoveAllAuras();
        void RemoveNonPassivesAuras();
        void RemoveArenaAuras();
        void RemoveAllAurasOnDeath();
        void RemoveAllAurasRequiringDeadTarget();
        void RemoveAllAurasExceptType(AuraType type);
        void RemoveAllAurasByType(AuraType type);
        void DelayOwnedAuras(uint32 spellId, ObjectGuid caster, int32 delaytime);
        void RecalcArenaAuras(bool hasPvPScaling);
        void RemoveAurasAllDots();
        void RemoveAurasAllNotOwned(ObjectGuid caster);

        void _RemoveAllAuraStatMods();
        void _ApplyAllAuraStatMods();

        AuraEffectList const* GetAuraEffectsByType(AuraType type) const { return m_modAuras[type]; }
        void GetAuraEffectsByMechanic(uint32 mechanic_mask, AuraList& auraList, ObjectGuid casterGUID = ObjectGuid::Empty);
        void GetTotalNotStuckAuraEffectByType(AuraType auratype, AuraEffectList& EffectList, std::vector<uint32>& ExcludeAuraList);
        AuraEffectList* GetAuraEffectsByType(AuraType type) { return m_modAuras[type]; }
        void GetAuraEffectsByListType(std::list<AuraType> *auratypelist, AuraEffectList& EffectList);

        AuraList& GetSingleCastAuras() { return m_scAuras; }
        AuraList const& GetSingleCastAuras() const { return m_scAuras; }
        AuraMyMap& GetMyCastAuras() { return m_my_Auras; }
        AuraList& GetMultiSingleTargetAuras() { return m_gbAuras; }
        void AddMyCastAuras(Aura* aura);
        void RemoveMyCastAuras(uint32 auraId, Aura* aura);
        void RemoveMultiSingleTargetAuras(uint32 newPhase = 0);
        void RemoveMyAurasDueToSpell(uint32 spellId);

        AuraEffect* GetAuraEffect(uint32 spellId, uint8 effIndex, ObjectGuid casterGUID = ObjectGuid::Empty) const;
        AuraEffect* GetAuraEffectOfRankedSpell(uint32 spellId, uint8 effIndex, ObjectGuid casterGUID = ObjectGuid::Empty) const;
        AuraEffect* GetAuraEffect(AuraType type, SpellFamilyNames family, uint32 familyFlag1, uint32 familyFlag2, uint32 familyFlag3, ObjectGuid casterGUID = ObjectGuid::Empty);

        AuraApplication* GetAuraApplication(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, AuraApplication * except = nullptr) const;
        Aura* GetAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0) const;

        AuraApplication* GetAuraApplicationOfRankedSpell(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, AuraApplication * except = nullptr) const;
        Aura* GetAuraOfRankedSpell(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0) const;

        void GetDispellableAuraList(Unit* caster, uint32 dispelMask, DispelChargesList& dispelList);

        bool HasAuraEffect(uint32 spellId, uint8 effIndex, ObjectGuid caster = ObjectGuid::Empty) const;
        uint32 GetAuraCount(uint32 spellId) const;
        bool HasAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0) const;
        bool HasAuraType(AuraType auraType) const;
        uint32 GetAuraTypeCount(AuraType auraType) const;
        bool HasAuraTypeWithCaster(AuraType auratype, ObjectGuid caster) const;
        bool HasAuraTypeWithMiscvalue(AuraType auratype, int32 miscvalue) const;
        bool HasAuraTypeWithAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        bool HasAuraTypeWithValue(AuraType auratype, int32 value) const;

        template <typename InterruptFlags>
        bool HasNegativeAuraWithInterruptFlag(InterruptFlags flag, ObjectGuid guid = ObjectGuid::Empty) const;

        bool HasNegativeAuraWithAttribute(uint32 flag, ObjectGuid guid = ObjectGuid::Empty);
        bool HasAuraWithAttribute(uint32 Attributes, uint32 flag) const;
        bool HasAuraWithMechanic(uint32 mechanicMask);
        bool HasAuraCastWhileWalking(SpellInfo const* spellInfo);
        bool HasAuraWithSchoolMask(AuraType auratype, SpellSchoolMask schoolMask);
        void GetAuraSet(std::set<uint32>& auraList);

        bool IsActiveMitigation() const;

        AuraEffect* IsScriptOverriden(SpellInfo const* spell, int32 script) const;
        uint32 GetDiseasesByCaster(ObjectGuid casterGUID, bool remove = false);
        uint32 GetDoTsByCaster(ObjectGuid casterGUID) const;

        int32 GetTotalAuraModifier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate, bool raid = false) const;
        float GetTotalAuraMultiplier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate) const;

        int32 GetTotalAuraModifier(AuraType auratype, bool raid = false) const;
        int32 GetTotalForAurasModifier(std::list<AuraType> *auratypelist) const;
        float GetTotalForAurasMultiplier(std::list<AuraType> *auratypelist) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;
        int32 GetTotalAuraDurationByType(AuraType auratype, bool firstAuraInList = false) const;

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMaskB(AuraType auratype, uint32 misc_maskB) const;
        float GetTotalPositiveAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask, AuraEffect const* except = nullptr) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetTotalAuraModifierByMiscValues(AuraType auratype, int32 misc_value, int32 miscValueB) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value, bool mainStats = false) const;
        float GetTotalAuraMultiplierByMiscValueB(AuraType auratype, int32 misc_valueB) const;
        float GetTotalAuraMultiplierByMiscValues(AuraType auratype, int32 miscValue, int32 miscValueB) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        int32 GetTotalAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        float GetTotalAuraMultiplierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        int32 GetMaxPositiveAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        int32 GetMaxNegativeAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;

        float GetResistanceBuffMods(SpellSchools school, bool positive) const { return GetFloatValue(positive ? UNIT_FIELD_RESISTANCE_BUFF_MODS_POSITIVE+school : UNIT_FIELD_RESISTANCE_BUFF_MODS_NEGATIVE+school); }
        void SetResistanceBuffMods(SpellSchools school, bool positive, float val) { SetFloatValue(positive ? UNIT_FIELD_RESISTANCE_BUFF_MODS_POSITIVE+school : UNIT_FIELD_RESISTANCE_BUFF_MODS_NEGATIVE+school, val); }
        void ApplyResistanceBuffModsMod(SpellSchools school, bool positive, float val, bool apply) { ApplyModSignedFloatValue(positive ? UNIT_FIELD_RESISTANCE_BUFF_MODS_POSITIVE+school : UNIT_FIELD_RESISTANCE_BUFF_MODS_NEGATIVE+school, val, apply); }
        void ApplyResistanceBuffModsPercentMod(SpellSchools school, bool positive, float val, bool apply) { ApplyPercentModFloatValue(positive ? UNIT_FIELD_RESISTANCE_BUFF_MODS_POSITIVE+school : UNIT_FIELD_RESISTANCE_BUFF_MODS_NEGATIVE+school, val, apply); }
        void InitStatBuffMods()
        {
            for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
                SetFloatValue(UNIT_FIELD_STAT_POS_BUFF + i, 0);
            for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
                SetFloatValue(UNIT_FIELD_STAT_NEG_BUFF + i, 0);
        }
        void ApplyStatBuffMod(Stats stat, float val, bool apply)
        {
            ApplyModSignedFloatValue((val > 0 ? UNIT_FIELD_STAT_POS_BUFF + stat : UNIT_FIELD_STAT_NEG_BUFF + stat), val, apply);
        }

        void ApplyStatPercentBuffMod(Stats stat, float val, bool apply)
        {
            ApplyPercentModFloatValue(UNIT_FIELD_STAT_POS_BUFF + stat, val, apply);
            ApplyPercentModFloatValue(UNIT_FIELD_STAT_NEG_BUFF + stat, val, apply);
        }

        float GetRatingMultiplier(CombatRating cr) const;

        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val); }
        uint32 GetCreateHealth() const { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }
        void SetCreateMana(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_MANA, val); }
        uint32 GetCreateMana() const { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }
        int32 GetCreatePowers(Powers power) const;
        float GetPosStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_STAT_POS_BUFF + stat); }
        float GetNegStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_STAT_NEG_BUFF + stat); }
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }

        uint32 GetChannelSpellId() const { return GetUInt32Value(UNIT_FIELD_CHANNEL_SPELL); }
        void SetChannelSpellId(uint32 channelSpellId) { SetUInt32Value(UNIT_FIELD_CHANNEL_SPELL, channelSpellId); }
        uint32 GetChannelSpellXSpellVisualId() const { return GetUInt32Value(UNIT_FIELD_CHANNEL_SPELL_XSPELL_VISUAL); }
        void SetChannelSpellXSpellVisualId(uint32 channelSpellXSpellVisualId) { SetUInt32Value(UNIT_FIELD_CHANNEL_SPELL_XSPELL_VISUAL, channelSpellXSpellVisualId); }

        DynamicFieldStructuredView<ObjectGuid> GetChannelObjects() const { return GetDynamicStructuredValues<ObjectGuid>(UNIT_DYNAMIC_FIELD_CHANNEL_OBJECTS); }
        void AddChannelObject(ObjectGuid guid) { AddDynamicStructuredValue(UNIT_DYNAMIC_FIELD_CHANNEL_OBJECTS, &guid); }
  
        // Update Mod Hast
        void UpdateCastHastMods();
        void UpdateMeleeHastMod();
        void UpdateRangeHastMod();
        void UpdateHastMod();
        void UpdateManaRegen();
        void UpdatePowerRegen(uint32 power);
        int32 CalculateHastPct(uint16 index) { return int32(1.0f / GetFloatValue(index) * 100); }

        void SetCurrentCastedSpell(Spell* pSpell);
        virtual void ProhibitSpellSchool(SpellSchoolMask /*idSchoolMask*/, uint32 /*unTimeMs*/) { }
        void InterruptSpell(CurrentSpellTypes spellType, bool withDelayed = true, bool withInstant = true, uint32 spellID = 0);
        void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

        virtual bool HasSpellCooldown(uint32 /*spell_id*/) { return false; }
        virtual double GetSpellCooldownDelay(uint32 /*spell_id*/) { return 0.0; }
        virtual void AddSpellCooldown(uint32 /*spell_id*/, uint32 /*itemid*/, double /*end_time*/) { }

        GlobalCooldownMgr & GetGlobalCooldownMgr();

        // set withDelayed to true to account delayed spells as casted
        // delayed+channeled spells are always accounted as casted
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false, bool isAutoshoot = false, bool skipInstant = true) const;

        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0, bool withInstant = true);

        Spell* GetCurrentSpell(CurrentSpellTypes spellType) const { return m_currentSpells[spellType]; }
        Spell* GetCurrentSpell(uint32 spellType) const { return m_currentSpells[spellType]; }
        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;
        int32 GetCurrentSpellCastTime(uint32 spell_id) const;
        SpellInfo const* GetCastSpellInfo(SpellInfo const* spellInfo) const;
        void SendSpellCreateVisual(SpellInfo const* spellInfo, Position const* position = nullptr, Unit* target = nullptr, uint32 type = 0, uint32 visualId = 0);
        void CancelSpellVisualKit(int32 spellVisualKitID);
        void SendSpellPlayOrphanVisual(SpellInfo const* spellInfo, bool apply, Position const* position = nullptr, Unit* target = nullptr);
        void PlaySpellVisual(Position target, int32 visual, float travelSpeed, ObjectGuid targetGuid = ObjectGuid::Empty, bool speedAsTime = false);
        void PlayOrphanSpellVisual(Position source, Position orientation, Position target, int32 visual, float travelSpeed, ObjectGuid targetGuid = ObjectGuid::Empty, bool speedAsTime = false);
        void CancelOrphanSpellVisual(int32 spellVisualID);
        void SendSpellVisualKit(SpellInfo const* spellInfo);
        void SendMissileCancel(uint32 spellId, bool reverse = true);
        void SendLossOfControl(Unit* caster, uint32 spellId, uint32 duraction, uint32 rmDuraction, Mechanics mechanic, SpellSchoolMask schoolMask, LossOfControlType type, bool apply);
        void SendDisplayToast(uint32 entry, ToastType displayToastMethod, bool isBonusRoll, uint32 count, DisplayToastMethod type, uint32 questID = 0, Item* item = nullptr);
        void GeneratePersonalLoot(Creature* creature, Player* anyLooter);
        void GenerateLoot(Creature* creature, Player* anyLooter);

        bool CheckAndIncreaseCastCounter();
        bool RequiresCurrentSpellsToHolyPower(SpellInfo const* spellProto);
        uint8 HandleHolyPowerCost(uint8 cost, SpellPowerEntry const* power);
        uint8 GetModForHolyPowerSpell() {return m_modForHolyPowerSpell;}
        void DecreaseCastCounter() { if (m_castCounter) --m_castCounter; }

        ObjectGuid m_SummonSlot[MAX_SUMMON_SLOT];
        ObjectGuid m_ObjectSlot[MAX_GAMEOBJECT_SLOT];

        ShapeshiftForm GetShapeshiftForm() const { return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM)); }
        void SetShapeshiftForm(ShapeshiftForm form)
        {
            SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM, form);
        }

        bool IsInFeralForm() const
        {
            ShapeshiftForm form = GetShapeshiftForm();
            return form == FORM_CAT || form == FORM_BEAR;
        }

        bool IsInDisallowedMountForm() const;

        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        float m_expertise;
        int32 m_baseSpellCritChance;
        LiquidData liquid_status;
        ZLiquidStatus Zliquid_status;
        std::vector<int32> m_everyPower;
        std::vector<int32> m_addPower;
        std::vector<int32> m_powerRegenTimer; // max = health
        std::vector<int32> m_powerCombatTimer; // max = health
        std::vector<float> m_powerFraction;

        void SetAddPower(Powers power, int32 val) { m_addPower[power] = val; }
        void ModAddPower(Powers power, int32 val) { m_addPower[power] += val; }
        uint32 GetAddPower(Powers power) const { return m_addPower[power]; }
        void setPowerCombatTimer(uint8 power, uint32 time) {m_powerCombatTimer[power] = time;}

        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];

        SpellPowerCost m_powerCost;
        SpellPowerCost m_powerCostSave;
        int8 m_comboSavePoints;

        // Event handler
        EventProcessor m_Events;

        void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
        {
            m_Functions.AddDelayedEvent(timeOffset, std::move(function));
        }

        void KillAllDelayedEvents()
        {
            m_Functions.KillAllFunctions();
        }

        void AddDelayedCombat(uint64 timeOffset, std::function<void()>&& function)
        {
            m_CombatFunctions.AddDelayedEvent(timeOffset, std::move(function));
        }

        void KillAllDelayedCombats()
        {
            m_CombatFunctions.KillAllFunctions();
        }

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value) { m_auraModifiersGroup[unitMod][modifierType] = value; }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const { return m_canModifyStats; }
        void SetCanModifyStats(bool modifyStats) { m_canModifyStats = modifyStats; }
        virtual float GetTotalStatValue(Stats stat) = 0;
        virtual bool UpdateStats(Stats stat) = 0;
        virtual bool UpdateAllStats() = 0;
        virtual void UpdateResistances(uint32 school) = 0;
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType) = 0;
        float GetTotalAttackPowerValue(WeaponAttackType attType) const;
        float GetWeaponDamageRange(WeaponAttackType attType, WeaponDamageRange type) const;
        void SetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, float value) { m_weaponDamage[attType][damageRange] = value; }

        bool isInFrontInMap(Unit const* target, float distance, float arc = M_PI) const;
        bool isInBackInMap(Unit const* target, float distance, float arc = M_PI) const;

        // Visibility system
        bool IsVisible() const { return (m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM) > SEC_PLAYER) ? false : true; }

        // common function for visibility checks for player/creatures with detection code
        void SetPhaseMask(uint32 newPhaseMask, bool update) override;// overwrite WorldObject::SetPhaseMask
        void SetPhaseId(std::set<uint32> const& newPhase, bool update) override;// overwrite WorldObject::SetPhaseId
        void UpdateObjectVisibility(bool forced = true) override;

        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];
        uint32 m_lastSanctuaryTime;
        SpellImmuneList const& GetSpellImmune(uint32 op) { return m_spellImmune[op]; }

        // Threat related methods
        bool CanHaveThreatList() const;
        void AddThreat(Unit* victim, float fThreat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const* threatSpell = nullptr);
        float ApplyTotalThreatModifier(float fThreat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DeleteThreatList();
        void DeleteFromThreatList(Unit* victim);
        void TauntApply(Unit* victim);
        void TauntFadeOut(Unit* taunter);
        ThreatManager& getThreatManager() { return m_ThreatManager; }
        void addHatedBy(HostileReference* pHostileReference) { m_HostileRefManager.insertFirst(pHostileReference); };
        void removeHatedBy(HostileReference* /*pHostileReference*/) { /* nothing to do yet */ }
        HostileRefManager& getHostileRefManager() { return m_HostileRefManager; }

        GuidSet* GetSaveThreatList() { return &m_savethreatlist; }
        void ClearSaveThreatTarget() { m_savethreatlist.clear(); }
        uint32 GetSizeSaveThreat() { return m_savethreatlist.size(); }
        void AddThreatTarget(ObjectGuid const& targetGuid) { m_savethreatlist.insert(targetGuid); }
        void RemoveThreatTarget(ObjectGuid const& targetGuid) { m_savethreatlist.erase(targetGuid); }
        bool GetThreatTarget(ObjectGuid const& targetGuid);

        VisibleAuraContainer const& GetVisibleAuras() const;
        bool HasVisibleAura(AuraApplication* aurApp) const;
        void SetVisibleAura(AuraApplication* aurApp);
        void SetVisibleAuraUpdate(AuraApplication* aurApp) { m_visibleAurasToUpdate.insert(aurApp); }
        void RemoveVisibleAura(AuraApplication* aurApp);

        void UpdateInterruptMask();
        void AddInterruptMask(std::array<uint32, 2> const& mask);

        uint32 GetDisplayId() const { return GetUInt32Value(UNIT_FIELD_DISPLAY_ID); }
        void SetDisplayId(uint32 modelId, bool resize = false);
        MirrorImageData GetMirrorImageInfo() const;
        uint32 GetNativeDisplayId() const { return GetUInt32Value(UNIT_FIELD_NATIVE_DISPLAY_ID); }
        void InitMirrorImageData(Unit * target);
        void ClearMirrorImageData();
        void RestoreDisplayId();
        void SetNativeDisplayId(uint32 modelId) { SetUInt32Value(UNIT_FIELD_NATIVE_DISPLAY_ID, modelId); }
        void setTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 getTransForm() const { return m_transform;}

        // DynamicObject management
        void _RegisterDynObject(DynamicObject* dynObj);
        void _UnregisterDynObject(DynamicObject* dynObj);
        DynamicObject* GetDynObject(uint32 spellId);
        int32 CountDynObject(uint32 spellId);
        void GetDynObjectList(std::list<DynamicObject*> &list, uint32 spellId);
        GuidList* GetSummonList(uint32 entry) { return &tempSummonList[entry]; }
        void RemoveDynObject(uint32 spellId);
        void RemoveAllDynObjects();

        // AreaTriger management
        void _RegisterAreaObject(AreaTrigger* dynObj);
        void _UnregisterAreaObject(AreaTrigger* dynObj);
        AreaTrigger* GetAreaObject(uint32 spellId);
        int32 CountAreaObject(uint32 spellId);
        void GetAreaObjectList(std::list<AreaTrigger*> &list, uint32 spellId);
        void GetAreaObjectList(std::list<AreaTrigger*> &list, std::vector<uint32>& spellIdList);
        void RemoveAreaObject(uint32 spellId, uint32 entry = 0);
        void RemoveAllAreaObjects();
        void ReCreateAreaTriggerObjects();

        // Conversation management
        void _RegisterConversationObject(Conversation* dynObj);
        void _UnregisterConversationObject(Conversation* dynObj);
        Conversation* GetConversationObject(uint32 spellId);
        void GetConversationObjectList(std::list<Conversation*> &list, uint32 spellId);
        void RemoveConversationObject(uint32 spellId, uint32 entry = 0);
        void RemoveAllConversationObjects();
        
        GameObject* GetGameObject(uint32 spellId) const;
        GameObject* GetGameObjectbyId(uint32 entry) const;
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();

        void SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list = nullptr);
        void SummonCreatureGroupDespawn(uint8 group, std::list<TempSummon*>* list = nullptr);

        TempSummonGroupMap tempSummonGroupList;
        TempSummonMap tempSummonList;

        uint32 CalculateDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, Unit* victim = nullptr);
        float GetAPMultiplier(WeaponAttackType attType, bool normalized);
        void ModifyAuraState(AuraStateType flag, bool apply);
        uint32 BuildAuraStateUpdateForTarget(Unit* target) const;
        bool HasAuraState(AuraStateType flag, SpellInfo const* spellProto = nullptr, Unit const* Caster = nullptr) const;
        void UnsummonAllTotems();
        Unit* GetMagicHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo);
        Unit* GetMeleeHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo = nullptr);

        int32 GetSpellPowerHealing();
        int32 GetSpellPowerDamage(SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_MAGIC);
        int32 SpellBaseDamageBonusDone(SpellSchoolMask schoolMask) const;
        int32 SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask);
        uint32 SpellDamageBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 pdamage, DamageEffectType damagetype, std::vector<uint32>& ExcludeAuraList, SpellEffIndex effIndex = EFFECT_0, uint32 stack = 1);
        float SpellPctDone(Unit* victim, SpellInfo const* spellProto, SpellEffIndex effIndex = EFFECT_0);
        void SpellDotPctDone(Unit* victim, SpellInfo const* spellProto, uint32 effIndex, float& DoneTotalMod, float& DoneTotalAdd, bool onapply = true);
        uint32 SpellDamageBonusTaken(Unit* caster, SpellInfo const* spellProto, uint32 pdamage);
        int32 SpellBaseHealingBonusDone(SpellSchoolMask schoolMask, bool calcVers = true);
        int32 SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask);
        uint32 SpellHealingBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, SpellEffIndex effIndex = EFFECT_0, uint32 stack = 1);
        uint32 SpellHealingBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, SpellEffIndex effIndex = EFFECT_0, uint32 stack = 1);
        bool CanPvPScalar();
        float GetProcStatsMultiplier(uint32 spellId) const;

        uint32 MeleeDamageBonusDone(Unit *victim, uint32 damage, WeaponAttackType attType, SpellInfo const *spellProto = nullptr, uint32 effIndex = 0);
        uint32 MeleeDamageBonusTaken(Unit* attacker, uint32 pdamage,WeaponAttackType attType, SpellInfo const *spellProto = nullptr);

        void SetOverrideAutoattack(uint32 spellId, uint8 attackType);
        uint32 GetAutoattackSpellId(uint8 attackType) const;

        void SetDelayIterruptFlag(uint32 flag) { _delayInterruptFlag = flag; }
        uint32 GetDelayIterruptFlag() { return _delayInterruptFlag; }

        bool isSpellBlocked(Unit* victim, SpellInfo const* spellProto, WeaponAttackType attType = BASE_ATTACK);
        bool isBlockCritical();
        bool isSpellCrit(Unit* victim, SpellInfo const* spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType, float &critChance, Spell* spell = nullptr) const;
        uint32 SpellCriticalHealingBonus(SpellInfo const* spellProto, uint32 damage);
        float SpellCriticalDamageBonus(SpellInfo const* spellProto, Unit* victim);

        void SetContestedPvP(Player* attackedPlayer = nullptr, bool forceByAura = false);
        OutdoorPvP* GetOutdoorPvP() const;

        uint32 GetCastingTimeForBonus(SpellInfo const* spellProto, DamageEffectType damagetype, uint32 CastingTime) const;
        float CalculateDefaultCoefficient(SpellInfo const *spellInfo, DamageEffectType damagetype) const;

        float GetRemainingPeriodicAmount(ObjectGuid caster, uint32 spellId, AuraType auraType, uint8 effectIndex, float oldAmount) const;

        void ApplyUberImmune(uint32 spellid, bool apply);
        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        void ApplySpellDispelImmunity(const SpellInfo* spellProto, DispelType type, bool apply);
        virtual bool IsImmunedToSpell(SpellInfo const* spellInfo);

        uint32 GetSchoolImmunityMask() const;
        uint32 GetMechanicImmunityMask() const;

        bool IsImmunedToDamage(SpellSchoolMask meleeSchoolMask);
        bool IsImmunedToDamage(SpellInfo const* spellInfo);
        virtual bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const;
                                                            // redefined in Creature
        static bool IsDamageReducedByArmor(SpellSchoolMask damageSchoolMask, SpellInfo const* spellInfo = nullptr, uint32 effectMask = 0);
        uint32 CalcArmorReducedDamage(Unit* attacker, Unit* victim, uint32 const damage, SpellInfo const* spellInfo);
        void CalcAbsorbResist(Unit* victim, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32 *absorb, uint32 *resist, SpellInfo const* spellInfo = nullptr);
        void CalcHealAbsorb(Unit* victim, const SpellInfo* spellProto, uint32 &healAmount, uint32 &absorb);
        float CalcAbsorb(Unit* victim, SpellInfo const* spellProto, float amount);

        void  UpdateSpeed(UnitMoveType mtype, bool forced);
        float GetSpeed(UnitMoveType mtype) const;
        float GetSpeedRate(UnitMoveType mtype) const { return m_speed_rate[mtype]; }
        void SetSpeed(UnitMoveType mtype, float rate, bool forced = false);
        void SetSpeedRate(UnitMoveType mtype, float rate) { m_speed_rate[mtype] = rate; }

        void SendAdjustSplineDuration(float scale);
        void SendFlightSplineSync(float splineDist);

        bool isHover() const { return HasAuraType(SPELL_AURA_HOVER); }

        float ApplyEffectModifiers(SpellInfo const* spellProto, uint8 effect_index, float value) const;
        float CalculateSpellDamage(Unit const* target, SpellInfo const* spellProto, uint8 effect_index, float const* basePoints = nullptr, Item* m_castitem = nullptr, bool lockBasePoints = false, float* variance = nullptr, int32 comboPoints = 0) const;
        int32 CalcSpellDuration(SpellInfo const* spellProto, int8 comboPoints = 0);
        int32 ModSpellDuration(SpellInfo const* spellProto, Unit const* target, int32 duration, bool positive, uint32 effectMask, Unit* caster = nullptr);
        void  ModSpellCastTime(SpellInfo const* spellProto, int32 & castTime, Spell* spell= nullptr);

        void addFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void removeFollower(FollowerReference* /*pRef*/) { /* nothing to do yet */ }
        static Unit* GetUnit(WorldObject& object, ObjectGuid guid);
        static Player* GetPlayer(WorldObject& object, ObjectGuid guid);
        static Creature* GetCreature(WorldObject& object, ObjectGuid guid);
        static GameObject* GetGameObjectOnMap(WorldObject& object, ObjectGuid guid);

        MotionMaster* GetMotionMaster() { return &i_motionMaster; }
        const MotionMaster* GetMotionMaster() const { return &i_motionMaster; }

        bool IsStopped() const { return !(HasUnitState(UNIT_STATE_MOVING)); }
        void StopMoving();

        void AddUnitMovementFlag(uint32 f) { m_movementInfo.AddMovementFlag(f); }
        void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.RemoveMovementFlag(f); }
        bool HasUnitMovementFlag(uint32 f) const { return m_movementInfo.HasMovementFlag(f); }
        uint32 GetUnitMovementFlags() const { return m_movementInfo.GetMovementFlags(); }
        void SetUnitMovementFlags(uint32 f) { m_movementInfo.SetMovementFlags(f); }

        void AddExtraUnitMovementFlag(uint32 f) { m_movementInfo.AddExtraMovementFlag(f); }
        void RemoveExtraUnitMovementFlag(uint32 f) { m_movementInfo.RemoveExtraMovementFlag(f); }
        bool HasExtraUnitMovementFlag(uint32 f) const { return m_movementInfo.HasExtraMovementFlag(f); }
        uint32 GetExtraUnitMovementFlags() const { return m_movementInfo.GetExtraMovementFlags(); }
        void SetExtraUnitMovementFlags(uint32 f) { m_movementInfo.SetExtraMovementFlags(f); }
        bool IsSplineEnabled() const;
        bool IsSplineFinished() const;

        bool SetCanTransitionBetweenSwimAndFly(bool enable);
        bool SetCanTurnWhileFalling(bool enable);
        bool SetCanDoubleJump(bool enable);
        void SendSetVehicleRecId(uint32 vehicleID);

        void SetControlled(bool apply, UnitState state);

        ///----------Pet responses methods-----------------
        void SendPetActionFeedback (uint32 spellID, uint8 msg);
        void SendPetTalk (uint32 pettalk);
        void SendPetAIReaction(ObjectGuid guid);
        ///----------End of Pet responses methods----------

        void propagateSpeedChange() { GetMotionMaster()->propagateSpeedChange(); }

        // reactive attacks
        void ClearAllReactives();
        void StartReactiveTimer(ReactiveType reactive) { m_reactiveTimer[reactive] = REACTIVE_TIMER_START;}
        void UpdateReactives(uint32 p_time);

        // group updates
        void UpdateAuraForGroup();

        // proc trigger system
        bool CanProc(){return !m_procDeep;}
        void SetCantProc(bool apply)
        {
            if (apply)
                ++m_procDeep;
            else
            {
                if(m_procDeep)
                    --m_procDeep;
            }
        }

        uint32 GetModelForForm(ShapeshiftForm form);
        uint32 GetModelForTotem(uint32 spellId) const;

        void SetReducedThreatPercent(uint32 pct, ObjectGuid guid)
        {
            m_reducedThreatPercent = pct;
            m_misdirectionTargetGUID = guid;
        }
        uint32 GetReducedThreatPercent() { return m_reducedThreatPercent; }
        Unit* GetMisdirectionTarget() { return !m_misdirectionTargetGUID.IsEmpty() ? GetUnit(*this, m_misdirectionTargetGUID) : nullptr; }
        ObjectGuid GetMisdirectionTargetGuid() { return m_misdirectionTargetGUID; }

        friend class VehicleJoinEvent;
        bool IsAIEnabled, NeedChangeAI;
        ObjectGuid LastCharmerGUID;
        bool CreateVehicleKit(uint32 id, uint32 creatureEntry, uint32 RecAura = 0, bool loading = false);
        void RemoveVehicleKit(bool onRemoveFromWorld = false);
        Vehicle* GetVehicleKit() const { return m_vehicleKit; }
        Vehicle* GetVehicle() const { return m_vehicle; }
        bool IsOnVehicle() const { return m_vehicle != nullptr; }
        bool IsOnVehicle(const Unit* vehicle) const { return m_vehicle && m_vehicle == vehicle->GetVehicleKit(); }
        Unit* GetVehicleBase()  const;
        Creature* GetVehicleCreatureBase() const;
        ObjectGuid GetTransGUID()   const override;
        // Returns the transport this unit is on directly (if on vehicle and transport, return vehicle)
        TransportBase* GetDirectTransport() const;

        bool m_ControlledByPlayer;

        bool HandleSpellClick(Unit* clicker, int8 seatId = -1);
        void EnterVehicle(Unit* base, int8 seatId = -1, bool fullTriggered = false);
        void ExitVehicle(Position const* exitPosition = nullptr);
        void ChangeSeat(int8 seatId, bool next = true);

        // Should only be called by AuraEffect::HandleAuraControlVehicle(AuraApplication const* auraApp, uint8 mode, bool apply) const;
        void _ExitVehicle(Position const* exitPosition = nullptr);
        void _EnterVehicle(Vehicle* vehicle, int8 seatId, AuraApplication const* aurApp = nullptr);

        bool isMoving() const   { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING); }
        bool isTurning() const  { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_TURNING); }
        virtual bool CanSwim() const;
        virtual bool CanFly() const = 0;
        bool IsFlying() const   { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY); }
        void SetTimeForSpline(uint32 time) { m_timeForSpline = time; }
        uint32 GetTimeForSpline() { return m_timeForSpline; }

        void RewardRage(float baseRage, bool attacker);

        virtual float GetFollowAngle() const { return static_cast<float>(M_PI/2); }
        
        virtual float GetFollowDistance() const { return 1.0f; }

        void OutDebugInfo() const;
        virtual bool isBeingLoaded() const { return false;}
        bool IsDuringRemoveFromWorld() const {return m_duringRemoveFromWorld;}

        Pet* ToPet()
        {
            if (isPet()) return reinterpret_cast<Pet*>(this);
            return nullptr;
        }
        Pet const* ToPet() const
        {
            if (isPet()) return reinterpret_cast<Pet const*>(this);
            return nullptr;
        }

        Totem* ToTotem()
        {
            if (isTotem()) return reinterpret_cast<Totem*>(this);
            return nullptr;
        }
        Totem const* ToTotem() const
        {
            if (isTotem()) return reinterpret_cast<Totem const*>(this);
            return nullptr;
        }

        TempSummon* ToTempSummon()
        {
            if (isSummon()) return reinterpret_cast<TempSummon*>(this);
            return nullptr;
        }
        TempSummon const* ToTempSummon() const
        {
            if (isSummon()) return reinterpret_cast<TempSummon const*>(this);
            return nullptr;
        }

        void SetTarget(ObjectGuid const& guid)
        {
            if (!_focusSpell)
                SetGuidValue(UNIT_FIELD_TARGET, guid);
            if (guid)
                m_lastTargetGUID = guid;
        }
        void SetTargetGUID(ObjectGuid const& guid)
        {
            m_curTargetGUID = guid;
        }
        ObjectGuid GetTargetGUID()
        {
            return m_curTargetGUID;
        }
        ObjectGuid GetLastTargetGUID()
        {
            return m_lastTargetGUID;
        }
        void SetForceGUID(ObjectGuid const& guid)
        {
            m_curForceGUID = guid;
        }
        ObjectGuid GetForceGUID()
        {
            return m_curForceGUID;
        }
        Unit* GetTargetUnit() const;
        Unit* GetLastTargetUnit() const;

        ObjectGuid m_SpecialTarget;

        float  m_baseRHastRatingPct;
        float  m_baseMHastRatingPct;
        float  m_baseHastRatingPct;
        //Combat rating
        int16 m_baseRatingValue[MAX_COMBAT_RATING];

        bool isAINotifyScheduled() const { return m_AINotifyScheduled; }
        void setAINotifyScheduled(bool val) { m_AINotifyScheduled = val; }

        // Handling caster facing during spell cast
        void FocusTarget(Spell const* focusSpell, ObjectGuid target);
        void ReleaseFocus(Spell const* focusSpell);

        uint32 GetNpcDamageTakenInPastSecs(uint32 secs) const;
        uint32 GetPlayerDamageTakenInPastSecs(uint32 secs) const;
        uint32 GetDamageTakenInPastSecs(uint32 secs, bool fromNpc = true, bool fromPlayer = true) const;
        uint32 GetTotalDamageTakenFromPlayer(ObjectGuid guid) { return m_playerTotalDamage[guid]; }

        int32 GetHighestExclusiveSameEffectSpellGroupValue(AuraEffect const* aurEff, AuraType auraType, bool checkMiscValue = false, int32 miscValue = 0) const;

        // Movement info
        Movement::MoveSpline* movespline;
        void UpdateSplineSpeed();

        void OnRelocated();

        std::map<ObjectGuid, std::vector<uint32>> m_whoHasMyAuras;
        std::recursive_mutex who_aura_lock;
        std::recursive_mutex i_auraEff_lock;

        void TargetsWhoHasMyAuras(std::list<Unit*>& targetList, std::vector<uint32>& auraList);
        void TargetsWhoHasMyAuras(std::list<ObjectGuid>& targetList, std::vector<uint32>& auraList);
        uint32 TargetsWhoHasMyAuras(std::vector<uint32>& auraList);
        bool TargetHasMyAura(ObjectGuid const& targetGUID, uint32 auraId);
        void RemoveIdInWHMAList(ObjectGuid const& guid, uint32 auraId); // m_whoHasMyAuras
        void RemovePetAndOwnerAura(uint32 spellId, Unit* owner = nullptr);
        bool HasMyAura(uint32 spellId);
        uint32 GetCountMyAura(uint32 spellId);
        AuraList* GetMyAura(uint32 spellId);

        Unit* GetUnitForLinkedSpell(Unit* caster, Unit* target, uint8 type, Unit* explTarget = nullptr);
        bool HasAuraLinkedSpell(Unit* caster, Unit* target, uint8 type, int32 hastalent, int32 param = 0);

        void SendDispelFailed(ObjectGuid const& targetGuid, uint32 spellId, std::list<uint32>& spellList);
        void SendDispelLog(ObjectGuid const& targetGuid, uint32 spellId, std::list<uint32>& spellList, bool broke, bool stolen);

        void SetCasterPet(bool isCaster) { isCasterPet = isCaster; }
        bool GetCasterPet() { return isCasterPet; }
        void SetAttackDist(float dist) { m_attackDist = dist; }
        float GetAttackDist() { return m_attackDist; }
        
        uint32 GetCustomDisplayId() const { return _customDisplayId; }
        void SetCustomDisplayId(uint32 const &modelId) { _customDisplayId = modelId; }
        void ResetCustomDisplayId() { if (GetDisplayId() == _customDisplayId) SetDisplayId(GetNativeDisplayId(), true); _customDisplayId = 0; }

        void SendSpellCooldown(int32 spellID, int32 spelCooldown, int32 cooldown = 0, SpellCooldownFlags flags = SpellCooldownFlags::NONE);
        void SetDynamicPassiveSpells(uint32 spellId, uint32 slot);
        uint32 GetDynamicPassiveSpells(uint32 slot);

        void SetDynamicWorldEffects(uint32 effect, uint32 slot);

        void DestroyForPlayer(Player* target) const override;

        uint32 GetVirtualItemId(uint32 slot) const;
        uint16 GetVirtualItemAppearanceMod(uint32 slot) const;
        void SetVirtualItem(uint32 slot, uint32 itemID, uint16 appearanceModID = 0, uint16 itemVisual = 0);
        
        void CreateConversation(uint32 id);
        void CreateAreaTrigger(uint32 id, float x = 0, float y = 0, float z = 0, float orientation = 0, uint32 customEntry = 0);

        ObjectGuid _petBattleId;

        bool waitOnSeat;
        std::atomic<bool> m_cleanupDone; // lock made to not add stuff after cleanup before delete

        GuidSetInMap m_spell_targets;
        void AddSpellTargets(uint32 spellId, ObjectGuid target);
        void RemoveSpellTargets(uint32 spellId, ObjectGuid target);
        void ClearSpellTargets(uint32 spellId);
        bool ExistSpellTarget(uint32 spellId, ObjectGuid target);

        BrawlersGuild* GetBrawlerGuild();

        Unit* GetHati();

        bool IsValidDesolateHostTarget(Unit* target, SpellInfo const* spellInfo) const;
        bool IsUnitMeetCondition(uint32 visualId, SpellInfo const* spellInfo = nullptr) const;

        bool needUpdateDynamicFlags;
        bool m_VisibilityUpdateScheduled;

        uint32 _targetCount;
        uint32 _castCount;
        uint32 _eventCount;
        uint32 _functionCount;

    protected:
        explicit Unit (bool isWorldObject);
        UnitAI* i_AI, *i_disabledAI;

        void _UpdateSpells(uint32 time);
        void _DeleteRemovedAuras();

        void _UpdateAutoRepeatSpell();

        bool m_AutoRepeatFirstCast;

        uint32 m_attackTimer[MAX_ATTACK];
        uint32 m_attackTimerFraction[MAX_ATTACK];

        float m_createStats[MAX_STATS];

        UnitSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        MirrorImageData m_mirrorImageData;

        GlobalCooldownMgr m_GlobalCooldownMgr;

        int32 m_procDeep;
        int32 m_timeForSpline;

        typedef std::list<DynamicObject*> DynObjectList;
        DynObjectList m_dynObj;

        typedef std::list<AreaTrigger*> AreaObjectList;
        AreaObjectList m_AreaObj;

        typedef std::list<Conversation*> ConversationObjectList;
        ConversationObjectList m_ConversationObj;

        typedef std::list<GameObject*> GameObjectList;
        GameObjectList m_gameObj;
        bool m_isSorted;
        uint32 m_transform;

        AuraEffectSet m_triggeredEffect;

        Spell* m_currentSpells[CURRENT_MAX_SPELL];
        uint32 m_castCounter;                               // count casts chain of triggered spells for prevent infinity cast crashes

        AuraMap m_ownedAuras;
        AuraApplicationMap m_appliedAuras;
        AuraApplicationMap m_procAuras;
        AuraList m_removedAuras;
        AuraMap::iterator m_auraUpdateIterator;
        uint32 m_removedAurasCount;

        AuraEffectList* m_modAuras[TOTAL_AURAS];
        AuraEffectListMap m_modMapAuras;
        uint8 m_auraTypeCount[TOTAL_AURAS];
        AuraList m_scAuras;                        // casted singlecast auras
        AuraList m_gbAuras;                        // casted singlecast auras
        AuraApplicationList m_interruptableAuras;             // auras which have interrupt mask applied on unit
        AuraStateAurasMap m_auraStateAuras;        // Used for improve performance of aura state checks on aura apply/remove
        std::array<uint32, 2> m_interruptMask;

        AuraMyMap m_my_Auras;                      // casted auras
        std::recursive_mutex my_aura_lock;
        sf::contention_free_shared_mutex< > m_aura_lock;
        std::atomic<bool> m_aura_is_lock;
        std::recursive_mutex i_proc_lock;

        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];
        float m_weaponDamage[MAX_ATTACK][2];
        bool m_canModifyStats;

        float m_base_value = 0.0f;
        float m_base_pct = 0.0f;
        float m_total_value = 0.0f;
        float m_total_pct = 0.0f;
        float m_dmg_multiplier = 1.0f;

        float m_speed_rate[MAX_MOVE_TYPE];

        uint32 m_overrideAutoattack[2];

        VisibleAuraContainer m_visibleAuras;
        boost::container::flat_set<AuraApplication*, VisibleAuraSlotCompare> m_visibleAurasToUpdate;

        CharmInfo* m_charmInfo;
        SharedVisionList m_sharedVision;

        virtual SpellSchoolMask GetMeleeDamageSchoolMask() const;

        MotionMaster i_motionMaster;

        uint32 m_reactiveTimer[MAX_REACTIVE];
        uint32 m_regenTimer;
        bool isCasterPet;
        float m_attackDist;

        GuidSet m_savethreatlist;
        ThreatManager m_ThreatManager;

        Vehicle* m_vehicle;
        Vehicle* m_vehicleKit;

        uint32 m_unitTypeMask;
        LiquidTypeEntry const* _lastLiquid;

        Position m_lastUnderWatterPos;

        bool m_zoneUpdateAllow;
        Position m_lastZoneUpdPos;
        uint32 m_zoneUpdateTimer;

        bool m_IsInKillingProcess;

        bool IsAlwaysVisibleFor(WorldObject const* seer) const override;
        bool IsAlwaysDetectableFor(WorldObject const* seer) const override;

        void DisableSpline(bool clearFlags = true);
    private:
        bool SpellProcCheck(Unit* victim, SpellInfo const* spellProto, SpellInfo const* procSpell, uint8 effect, AuraEffect* triggeredByAura);
        bool SpellProcTriggered(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        void CalculateFromDummy(Unit* victim, float &amount, SpellInfo const* spellProto = nullptr, uint32 mask = 131071, SpellAuraDummyType type = SPELL_DUMMY_DAMAGE) const; //mask for all 16 effect
        void CalculateCastTimeFromDummy(int32& castTime, SpellInfo const* spellProto);
        bool IsTriggeredAtSpellProcEvent(Unit* victim, SpellInfo const* spellProto, SpellInfo const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active, SpellProcEventEntry const* & spellProcEvent, uint8 effect);
        bool HandleDummyAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown, Spell* spell = nullptr);
        bool HandleHasteAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleProcTriggerSpellCopy(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, double cooldown);
        bool HandleSpellCritChanceAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleObsModEnergyAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleModDamagePctTakenAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleAuraProc(Unit* victim, DamageInfo* dmgInfoProc, Aura* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown, bool * handled);
        bool HandleProcTriggerSpell(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleOverrideClassScriptAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, double cooldown);
        bool RollProcResult(Unit* victim, Aura* aura, WeaponAttackType attType, bool isVictim, SpellProcEventEntry const* spellProcEvent, uint32 procFlag, uint32 procExtra, DamageInfo* dmgInfoProc, SpellInfo const* procSpell, ObjectGuid castItemGUID);
        bool RollProcEffectResult(AuraEffect* aurEff, SpellProcEventEntry const* spellProcEvent, uint32 procFlag, uint32 procExtra, uint8 effect);
        bool HandleAuraRaidProcFromChargeWithValue(AuraEffect* triggeredByAura);
        bool HandleCastWhileWalkingAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleSpellModAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleIgnoreAurastateAuraProc(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);
        bool HandleProcMelleTriggerSpell(Unit* victim, DamageInfo* dmgInfoProc, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, double cooldown);

        void UpdateSplineMovement(uint32 t_diff);
        void UpdateSplinePosition(bool stop = false);

        // player or player's pet
        float GetCombatRatingReduction(CombatRating cr) const;
        uint32 GetCombatRatingDamageReduction(CombatRating cr, float cap, uint32 damage) const;

    protected:
        void SetFeared(bool apply);
        void SetConfused(bool apply);
        void SetStunned(bool apply);
        void SetRooted(bool apply);

        uint32 m_sequenceIndex;
    private:

        Position m_lastVisibilityUpdPos;
        uint32 m_rootTimes;
        uint8 m_modForHolyPowerSpell;
        bool m_AINotifyScheduled;

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_CombatTimer;
        TimeTrackerSmall m_movesplineTimer;
        TimeTrackerSmall _flightSplineSyncTimer;
        MountCapabilityEntry const* _mount;

        Diminishing m_Diminishing;
        // Manage all Units that are threatened by us
        HostileRefManager m_HostileRefManager;

        FollowerRefManager m_FollowingRefManager;
        Trinity::AnyDataContainer m_anyDataContainer;

        uint32 m_reducedThreatPercent;
        ObjectGuid m_misdirectionTargetGUID;
        ObjectGuid m_curTargetGUID;
        ObjectGuid m_lastTargetGUID;
        ObjectGuid m_curForceGUID;

        bool m_duringRemoveFromWorld; // lock made to not add stuff after beginning removing from world

        Spell const* _focusSpell;   ///> Locks the target during spell cast for proper facing       
        bool _isWalkingBeforeCharm; // Are we walking before we were charmed? 

        uint16 _aiAnimKitId;
        uint16 _movementAnimKitId;
        uint16 _meleeAnimKitId;
        
        uint32 _customDisplayId;

        // ccd system
        uint32 _delayInterruptFlag;

        time_t _lastDamagedTime;

        int32 damageTrackingTimer_;

        std::array<uint32, DAMAGE_TRACKING_PERIOD> playerDamageTaken_;
        std::array<uint32, DAMAGE_TRACKING_PERIOD> npcDamageTaken_;

        // used to track total damage each player has made to the unit
        std::map<ObjectGuid, uint32> m_playerTotalDamage;
        FunctionProcessor m_Functions;
        FunctionProcessor m_CombatFunctions;
};

class DelayCastEvent : public BasicEvent
{
    friend class EffectMovementGenerator;
    friend class Spell;
    public:
        DelayCastEvent(Unit& owner, ObjectGuid t, uint32 s, bool _triggered, ObjectGuid c = ObjectGuid::Empty) : BasicEvent(), m_owner(owner), TargetGUID(t), CasterGUID(c), Spell(s), triggered(_triggered) {}
        bool Execute(uint64, uint32) override;
        void Execute(Unit* caster); // used at cast after jump for example.

    private:
        Unit& m_owner;
        ObjectGuid TargetGUID;
        ObjectGuid CasterGUID;
        uint32 Spell;
        bool triggered;
};

namespace Trinity
{
    // Binary predicate for sorting Units based on percent value of a power
    class PowerPctOrderPred
    {
        public:
            PowerPctOrderPred(Powers power, bool ascending = true) : m_power(power), m_ascending(ascending) {}
            bool operator() (const Unit* a, const Unit* b) const
            {
                float rA = a->GetMaxPower(m_power) ? float(a->GetPower(m_power)) / float(a->GetMaxPower(m_power)) : 0.0f;
                float rB = b->GetMaxPower(m_power) ? float(b->GetPower(m_power)) / float(b->GetMaxPower(m_power)) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }
        private:
            const Powers m_power;
            const bool m_ascending;
    };

    // Binary predicate for sorting Units based on percent value of health
    class HealthPctOrderPred
    {
        public:
            HealthPctOrderPred(bool ascending = true) : m_ascending(ascending) {}
            bool operator() (const Unit* a, const Unit* b) const
            {
                float rA = a->GetMaxHealth() ? float(a->GetHealth()) / float(a->GetMaxHealth()) : 0.0f;
                float rB = b->GetMaxHealth() ? float(b->GetHealth()) / float(b->GetMaxHealth()) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }
        private:
            const bool m_ascending;
    };

    // Binary predicate for sorting DynamicObjects based on value of duration
    class DurationPctOrderPred
    {
        public:
            DurationPctOrderPred(bool ascending = true) : m_ascending(ascending) {}
            bool operator() (const DynamicObject* a, const DynamicObject* b) const
            {
                int32 rA = a->GetDuration() ? float(a->GetDuration()) : 0;
                int32 rB = b->GetDuration() ? float(b->GetDuration()) : 0;
                return m_ascending ? rA < rB : rA > rB;
            }
        private:
            const bool m_ascending;
    };

    // Binary predicate for sorting Units based on value of distance of an GameObject
    class DistanceCompareOrderPred
    {
        public:
            DistanceCompareOrderPred(const DynamicObject* object, bool ascending = true) : m_object(object), m_ascending(ascending) {}
            bool operator() (const Unit* a, const Unit* b) const
            {
                return m_ascending ? a->GetDistance(m_object) < b->GetDistance(m_object) :
                                     a->GetDistance(m_object) > b->GetDistance(m_object);
            }
        private:
            const DynamicObject* m_object;
            const bool m_ascending;
    };

    class DistanceCompareOrderPred2
    {
    public:
        DistanceCompareOrderPred2(const WorldObject* object, bool ascending = true) : m_object(object), m_ascending(ascending) {}
        bool operator() (const WorldObject* a, const WorldObject* b) const
        {
            return m_ascending ? a->GetDistance(m_object) < b->GetDistance(m_object) :
                a->GetDistance(m_object) > b->GetDistance(m_object);
        }
    private:
        const WorldObject* m_object;
        const bool m_ascending;
    };
}
#endif
