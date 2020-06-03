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

#ifndef _SPELLMGR_H
#define _SPELLMGR_H

// For static or at-server-startup loaded spell data

#include "Common.h"
#include "SharedDefines.h"
#include "Unit.h"

class SpellInfo;
class Player;
class Unit;
class ProcEventInfo;
struct SkillLineAbilityEntry;

// only used in code
enum SpellCategories
{
    SPELLCATEGORY_HEALTH_MANA_POTIONS = 4,
    SPELLCATEGORY_DEVOUR_MAGIC        = 12,
    SPELLCATEGORY_JUDGEMENT           = 1210,               // Judgement (seal trigger)
    SPELLCATEGORY_FOOD             = 11,
    SPELLCATEGORY_DRINK            = 59,
};

//ClassOptions.SpellClassMask
enum SpellFamilyFlag
{
    // SPELLFAMILYFLAG  = ClassOptions.SpellClassMask[0]
    // SPELLFAMILYFLAG1 = ClassOptions.SpellClassMask[1]
    // SPELLFAMILYFLAG2 = ClassOptions.SpellClassMask[2]
    // SPELLFAMILYFLAG3 = ClassOptions.SpellClassMask[3]

    // Rogue
    SPELLFAMILYFLAG_ROGUE_VANISH            = 0x00000800,
    SPELLFAMILYFLAG_ROGUE_VAN_EVAS_SPRINT   = 0x00000860,    // Vanish, Evasion, Sprint
    SPELLFAMILYFLAG1_ROGUE_COLDB_SHADOWSTEP = 0x00000240,    // Cold Blood, Shadowstep
    SPELLFAMILYFLAG_ROGUE_KICK              = 0x00000010,   // Kick
    SPELLFAMILYFLAG1_ROGUE_DISMANTLE        = 0x00100000,   // Dismantle
    SPELLFAMILYFLAG_ROGUE_BLADE_FLURRY      = 0x40000000,   // Blade Flurry
    SPELLFAMILYFLAG1_ROGUE_BLADE_FLURRY     = 0x00000800,   // Blade Flurry

    // Warrior
    SPELLFAMILYFLAG_WARRIOR_CHARGE          = 0x00000001,
    SPELLFAMILYFLAG_WARRIOR_SLAM            = 0x00200000,
    SPELLFAMILYFLAG_WARRIOR_EXECUTE         = 0x20000000,
    SPELLFAMILYFLAG_WARRIOR_CONCUSSION_BLOW = 0x04000000,

    // Warlock
    SPELLFAMILYFLAG_WARLOCK_LIFETAP         = 0x00040000,

    // Druid
    SPELLFAMILYFLAG2_DRUID_STARFALL         = 0x00000100,

    // Paladin
    SPELLFAMILYFLAG1_PALADIN_DIVINESTORM    = 0x00020000,

    // Shaman
    SPELLFAMILYFLAG_SHAMAN_FROST_SHOCK      = 0x80000000,
    SPELLFAMILYFLAG_SHAMAN_HEALING_STREAM   = 0x00002000,
    SPELLFAMILYFLAG_SHAMAN_MANA_SPRING      = 0x00004000,
    SPELLFAMILYFLAG2_SHAMAN_LAVA_LASH       = 0x00000004,
    SPELLFAMILYFLAG_SHAMAN_FIRE_NOVA        = 0x28000000,

    // Deathknight
    SPELLFAMILYFLAG_DK_DEATH_STRIKE         = 0x00000010,
    SPELLFAMILYFLAG_DK_DEATH_COIL           = 0x00002000,

    // TODO: Figure out a more accurate name for the following familyflag(s)
    SPELLFAMILYFLAG_SHAMAN_TOTEM_EFFECTS    = 0x04000000,  // Seems to be linked to most totems and some totem effects
};

enum SpellLinkedType
{
    SPELL_LINK_CAST             = 0, // +: cast; -: remove
    SPELL_LINK_REMOVE           = 0,
    SPELL_LINK_ON_HIT           = 1,
    SPELL_LINK_AURA             = 2, // +: aura; -: immune
    SPELL_LINK_BEFORE_HIT       = 3,
    SPELL_LINK_AURA_HIT         = 4,
    SPELL_LINK_BEFORE_CAST      = 5,
    SPELL_LINK_PREPARE_CAST     = 6,
    SPELL_LINK_BEFORE_CHECK     = 7,
    SPELL_LINK_FINISH_CAST      = 8,
    SPELL_LINK_ON_ADD_TARGET    = 9,
    SPELL_LINK_FAILED_CAST      = 10,
    SPELL_LINK_MAX,
};

enum SpellLinkedUnitType
{
    LINK_UNIT_TYPE_DEFAULT         = 0,
    LINK_UNIT_TYPE_PET             = 1,
    LINK_UNIT_TYPE_OWNER           = 2,
    LINK_UNIT_TYPE_CASTER          = 3,
    LINK_UNIT_TYPE_SELECTED        = 4,
    LINK_UNIT_TYPE_TARGET          = 5,
    LINK_UNIT_TYPE_VICTIM          = 6,
    LINK_UNIT_TYPE_ATTACKER        = 7,
    LINK_UNIT_TYPE_NEARBY          = 8,
    LINK_UNIT_TYPE_NEARBY_ALLY     = 9,
    LINK_UNIT_TYPE_ORIGINALCASTER  = 10,
    LINK_UNIT_TYPE_EXPL_TARGET     = 11,
    LINK_UNIT_TYPE_LAST_TARGET     = 12,
    LINK_UNIT_TYPE_UNIT_TARGET     = 13,
    LINK_UNIT_TYPE_TOP_AGGRO       = 14, //Only Creature
};

enum SpellLinkedHasType
{
    LINK_HAS_AURA_ON_CASTER       = 0,
    LINK_HAS_AURA_ON_TARGET       = 1,
    LINK_HAS_SPELL_ON_CASTER      = 2,
    LINK_HAS_AURA_ON_OWNER        = 3,
    LINK_HAS_AURATYPE             = 4,
    LINK_HAS_MY_AURA_ON_CASTER    = 5,
    LINK_HAS_MY_AURA_ON_TARGET    = 6,
    LINK_HAS_AURA_STATE           = 7,
    LINK_HAS_SPECID               = 8,
    LINK_HAS_OBJECT_TYPE          = 9,
    LINK_HAS_FRIEND               = 10,
    LINK_HAS_ATTACKABLE           = 11,
    LINK_HAS_DISTANCE             = 12,
    LINK_HAS_AURA_STACK_ON_CASTER = 13,
    LINK_HAS_AURA_STACK_ON_TARGET = 14,
    LINK_HAS_CASTER_FULL_HP       = 15,
    LINK_HAS_TARGET_FULL_HP       = 16,
    LINK_IN_COMBAT                = 17,
    LINK_HAS_CASTER_CHECK_HP      = 18,
    LINK_HAS_TARGET_CHECK_HP      = 19,
    LINK_HAS_CASTER_CLASS         = 20,
    LINK_HAS_TARGET_CLASS         = 21,
    LINK_HAS_SPELL_COOLDOWN       = 22,
    LINK_HAS_CASTER_AURA_MECHANIC = 23,
    LINK_HAS_TARGET_AURA_MECHANIC = 24,
    LINK_HAS_CASTER_IS_MOVING     = 25,
    LINK_HAS_TARGET_IS_MOVING     = 26,
};

enum SpellLinkedActionType
{
    LINK_ACTION_DEFAULT             = 0,
    LINK_ACTION_LEARN               = 1,
    LINK_ACTION_AURATYPE            = 2,
    LINK_ACTION_SEND_COOLDOWN       = 3,
    LINK_ACTION_CAST_NO_TRIGGER     = 4,
    LINK_ACTION_ADD_AURA            = 5,
    LINK_ACTION_CASTINAURA          = 6,
    LINK_ACTION_CHANGE_STACK        = 7,
    LINK_ACTION_REMOVE_COOLDOWN     = 8,
    LINK_ACTION_REMOVE_MOVEMENT     = 9, // RemoveMovementImpairingEffects
    LINK_ACTION_CHANGE_DURATION     = 10, // Mod Duration
    LINK_ACTION_CAST_DEST           = 11,
    LINK_ACTION_CHANGE_CHARGES      = 12,
    LINK_ACTION_MODIFY_COOLDOWN     = 13,
    LINK_ACTION_CATEGORY_COOLDOWN   = 14,
    LINK_ACTION_CATEGORY_CHARGES    = 15,
    LINK_ACTION_RECALCULATE_AMOUNT  = 16,
    LINK_ACTION_CAST_COUNT          = 17,
    LINK_ACTION_FULL_TRIGGER        = 18,
    LINK_ACTION_REMOVE_CATEGORY_CD  = 19,
    LINK_ACTION_SEND_COOLDOWN_SELF  = 20,
    LINK_ACTION_CAST_DURATION       = 21,
    LINK_ACTION_CAST_ON_SUMMON      = 22,
    LINK_ACTION_MOD_DURATION        = 23,
};

enum SpellLinkedTargetType
{
    LINK_TARGET_DEFAULT          = 0,
    LINK_TARGET_FROM_EFFECT      = 1,
    LINK_TARGET_NULL             = 2,
    LINK_TARGET_NOT_NULL         = 3,
};

enum SpellTriggeredType
{
    SPELL_TRIGGER_BP                            = 0,            // set basepoint to spell from amount
    SPELL_TRIGGER_BP_CUSTOM                     = 1,            // set basepoint to spell custom from BD
    SPELL_TRIGGER_MANA_COST                     = 2,            // set basepoint to spell mana cost
    SPELL_TRIGGER_DAM_HEALTH                    = 3,            // set basepoint to spell damage or heal percent
    SPELL_TRIGGER_COOLDOWN                      = 4,            // Set cooldown for trigger spell
    SPELL_TRIGGER_UPDATE_DUR                    = 5,            // Update duration for select spell
    SPELL_TRIGGER_GET_DUR_AURA                  = 6,            // Get duration from select aura to cast bp
    SPELL_TRIGGER_UPDATE_DUR_TO_MAX             = 8,            // Update duration for select spell to max duration
    SPELL_TRIGGER_PERC_FROM_DAMGE               = 9,            // Percent from damage
    SPELL_TRIGGER_PERC_MAX_MANA                 = 10,           // Percent from max mana
    SPELL_TRIGGER_PERC_BASE_MANA                = 11,           // Percent from base mana
    SPELL_TRIGGER_PERC_CUR_MANA                 = 12,           // Percent from curent mana
    SPELL_TRIGGER_CHECK_PROCK                   = 13,           // Check proc from spell to trigger
    SPELL_TRIGGER_DUMMY                         = 14,           // spell to trigger without option for bp
    SPELL_TRIGGER_CAST_DEST                     = 15,           // spell to trigger without option for bp
    SPELL_TRIGGER_CHECK_DAMAGE                  = 16,           // spell to trigger if damage > amount
    SPELL_TRIGGER_ADD_STACK                     = 17,           // add spell stack
    SPELL_TRIGGER_ADD_CHARGES                   = 18,           // add spell charges
    SPELL_TRIGGER_ADD_CHARGES_STACK             = 19,           // add spell charges and stack
    SPELL_TRIGGER_CAST_OR_REMOVE                = 20,           // cast spell without option
    SPELL_TRIGGER_UPDATE_DUR_TO_IGNORE_MAX      = 21,           // Update duration for select spell to ignore max duration
    SPELL_TRIGGER_ADD_DURATION                  = 22,           // Add duration for select spell
    SPELL_TRIGGER_MODIFY_COOLDOWN               = 23,           // Modify cooldown for trigger spell
    SPELL_TRIGGER_ADD_DURATION_OR_CAST_DURATION = 25,           // Add duration for select spell or cast his
    SPELL_TRIGGER_REMOVE_CD_RUNE                = 26,           // Add duration for select spell or cast his
    SPELL_TRIGGER_BP_SPELLID                    = 27,           // set basepoint to spellId from proc
    SPELL_TRIGGER_BP_SPD_AP                     = 28,           // set basepoint to spellId from SPD or AP
    SPELL_TRIGGER_COMBOPOINT_BP                 = 29,           // set basepoint to bp * combopoints
    SPELL_TRIGGER_DAM_PERC_FROM_MAX_HP          = 30,           // set basepoint to (damage / max hp) * 100
    SPELL_TRIGGER_SUMM_DAMAGE_PROC              = 31,           // summ damage in amount, proc if damage > bp0(1,2) * SPD(SPDH,AP)
    SPELL_TRIGGER_ADDPOWER_PCT                  = 32,           // set basepoint to spell add power percent from aura amount
    SPELL_TRIGGER_ADD_ABSORB_PCT                = 33,           // set basepoint from absorb percent
    SPELL_TRIGGER_ADD_BLOCK_PCT                 = 34,           // set basepoint from block percent
    SPELL_TRIGGER_NEED_COMBOPOINTS              = 35,           // Proc from spell that need compopoiunts
    SPELL_TRIGGER_HOLYPOWER_BONUS               = 36,           // Holypower bonus
    SPELL_TRIGGER_CAST_AFTER_MAX_STACK          = 37,           // Cast after max stack
    SPELL_TRIGGER_DAM_MAXHEALTH                 = 38,           // set basepoint to spell damage or max heal percent
    SPELL_TRIGGER_STACK_AMOUNT                  = 39,           // stack damage in amount
    SPELL_TRIGGER_BP_DURATION                   = 40,           // damage is duration
    SPELL_TRIGGER_ADD_STACK_AND_CAST            = 41,           // change stack and set bp = stack
    SPELL_TRIGGER_ADD_SPELL_CHARGES             = 42,           // add spell charges
    SPELL_TRIGGER_DAM_PERC_AT_TARGET            = 43,           // Trigger spell for target in at
    SPELL_TRIGGER_FROM_ENERGY                   = 44,           // set basepoint to spell from energy percent
    SPELL_TRIGGER_CAST_DELAY                    = 45,           // cast spell with delay
    SPELL_TRIGGER_MOFIFY_CHARGES_COLLDOWN       = 46,           // spell charges cooldown
    SPELL_TRIGGER_TO_AT_SRC                     = 47,           // cast spell to src AT
    SPELL_TRIGGER_CAST_DURACTION                = 48,           // cast spell with spell duraction
};

enum DummyTriggerType
{
    DUMMY_TRIGGER_BP                            = 0,            // set basepoint to spell from amount
    DUMMY_TRIGGER_BP_CUSTOM                     = 1,            // set basepoint to spell custom from BD
    DUMMY_TRIGGER_COOLDOWN                      = 2,            // Set cooldown for trigger spell
    DUMMY_TRIGGER_CHECK_PROCK                   = 3,            // Check proc from spell to trigger
    DUMMY_TRIGGER_DUMMY                         = 4,            // spell to trigger without option for bp
    DUMMY_TRIGGER_CAST_DEST                     = 5,            // spell to trigger without option for bp
    DUMMY_TRIGGER_CAST_OR_REMOVE                = 6,            // cast spell without option
    DUMMY_TRIGGER_DAM_MAXHEALTH                 = 7,            // set basepoint to spell damage or max heal percent
    DUMMY_TRIGGER_COPY_AURA                     = 8,            // Copy aura
    DUMMY_TRIGGER_ADD_POWER_COST                = 9,            // Add power cost to spell
    DUMMY_TRIGGER_CAST_DEST2                    = 10,           // Cast spell on dest
    DUMMY_TRIGGER_CAST_IGNORE_GCD               = 11,           // cast ignore GCD
    DUMMY_TRIGGER_MOVE_AURA                     = 12,           // Move aura
    DUMMY_TRIGGER_SEND_COOLDOWN                 = 13,           // Send cooldown for spell
    DUMMY_TRIGGER_ARCHAEOLOGY_LOOT              = 14,           // For archaeology spell who not have item create effect
    DUMMY_TRIGGER_TALK_BROADCAST                = 15,           // Talk broadcast
    DUMMY_TRIGGER_CAST_DELAY                    = 16,           // Cast spell delay
};

enum AuraTriggerType
{
    AURA_TRIGGER                                = 0,            // cast spell
    AURA_TRIGGER_BP                             = 1,            // set basepoint to spell custom from amount
    AURA_TRIGGER_BP_CUSTOM                      = 2,            // set basepoint to spell custom from BD
    AURA_TRIGGER_CHECK_COMBAT                   = 3,            // cast spell in check combat
    AURA_TRIGGER_DEST                           = 4,            // cast spell on dest
    AURA_TRIGGER_DYNOBJECT                      = 5,            // cast spell on dest DynObject
    AURA_TRIGGER_FROM_SUMMON_SLOT               = 6,            // cast spell from summon slot(totem or any)
    AURA_TRIGGER_AREATRIGGER                    = 7,            // cast spell on dest AreaTrigger
    AURA_TRIGGER_FROM_SUMMON_SLOT_DEST          = 8,            // cast spell from summon slot(totem or any) to dest loc
    AURA_TRIGGER_FROM_SUMMON_DEST               = 9,            // cast spell to summon dest loc
    AURA_TRIGGER_AREATRIGGER_CAST               = 10,           // cast AreaTrigger
    AURA_TRIGGER_TARGETCASTER                   = 11,           // cast spell from the target
    AURA_TRIGGER_IF_ENERGE                      = 12,           // triggered spell if percent energy
    AURA_TRIGGER_CAST_OR_STACK                  = 13,           // triggered spell or stack it
    AURA_TRIGGER_AMOUNT                         = 14,           // triggered spell with amount and amount unset
    AURA_TRIGGER_2                              = 15,           // TriggerCastFlags - false
    AURA_TRIGGER_SEND_SPELL_VISUAL              = 16,           // Send spell visual
    AURA_TRIGGER_REMOVE_AURA                    = 17,           // remove aura
    AURA_TRIGGER_IN_MOVING                      = 18            // triggered only moving target
};

enum AuraTriggerOption
{
    AURA_REMOVE_ON_PROC                         = 1,            // remove aura
};


enum SpellAuraDummyOption
{
    SPELL_DUMMY_ENABLE                          = 0,            // enable or disable aura(set amount to 0)
    SPELL_DUMMY_ADD_PERC                        = 1,            // add percent to amount
    SPELL_DUMMY_ADD_VALUE                       = 2,            // add value to amount
    SPELL_DUMMY_ADD_ATTRIBUTE                   = 3,            // add attribute to spell value
    SPELL_DUMMY_MOD_EFFECT_MASK                 = 4,            // Modify effect mask for add aura
    SPELL_DUMMY_CRIT_RESET                      = 5,            // reset or not crit chance
    SPELL_DUMMY_CRIT_ADD_PERC                   = 6,            // add percent to crit
    SPELL_DUMMY_CRIT_ADD_VALUE                  = 7,            // add value to crit
    SPELL_DUMMY_ADD_PERC_BP                     = 8,            // add percent(bp / 100) to amount
    SPELL_DUMMY_DAMAGE_ADD_PERC                 = 9,            // add percent to damage
    SPELL_DUMMY_DAMAGE_ADD_VALUE                = 10,           // add value to damage
    SPELL_DUMMY_DURATION_ADD_PERC               = 11,           // add percent to duration
    SPELL_DUMMY_DURATION_ADD_VALUE              = 12,           // add value to duration
    SPELL_DUMMY_CASTTIME_ADD_PERC               = 13,           // add percent to castTime
    SPELL_DUMMY_CASTTIME_ADD_VALUE              = 14,           // add value to castTime
};

enum SpellTargetFilterType
{
    SPELL_FILTER_SORT_BY_HEALT                  = 0,            // Sort target by healh
    SPELL_FILTER_BY_AURA                        = 1,            // Remove target by aura
    SPELL_FILTER_BY_DISTANCE                    = 2,            // Check distance
    SPELL_FILTER_TARGET_TYPE                    = 3,            // Check target rype
    SPELL_FILTER_SORT_BY_DISTANCE               = 4,            // Sort by distance
    SPELL_FILTER_TARGET_FRIENDLY                = 5,            // Check Friendly
    SPELL_FILTER_TARGET_IN_RAID                 = 6,            // Check Raid
    SPELL_FILTER_TARGET_IN_PARTY                = 7,            // Check Party
    SPELL_FILTER_TARGET_EXPL_TARGET             = 8,            // Select explicit target
    SPELL_FILTER_TARGET_EXPL_TARGET_REMOVE      = 9,            // Select explicit target remove
    SPELL_FILTER_TARGET_IN_LOS                  = 10,           // Select target in los
    SPELL_FILTER_TARGET_IS_IN_BETWEEN           = 11,           // Select target is in between
    SPELL_FILTER_TARGET_IS_IN_BETWEEN_SHIFT     = 12,           // Select target is in between and shift
    SPELL_FILTER_BY_AURA_OR                     = 13,           // Remove target by any aura
    SPELL_FILTER_BY_ENTRY                       = 14,           // Remove target by any entry
    SPELL_FILTER_TARGET_ATTACKABLE              = 15,           // Check Attackable
    SPELL_FILTER_BY_DISTANCE_TARGET             = 16,           // Filter by distance target
    SPELL_FILTER_OWNER_TARGET_REMOVE            = 17,           // Owner target remove
    SPELL_FILTER_SORT_BY_DISTANCE_FROM_TARGET   = 18,           // Sort by distance from target
    SPELL_FILTER_BY_DISTANCE_DEST               = 19,           // Filter by distance dest
    SPELL_FILTER_BY_DISTANCE_PET                = 20,           // Filter by distance pet
    SPELL_FILTER_BY_OWNER                       = 21,           // Filter by owner
    SPELL_FILTER_ONLY_RANGED_SPEC               = 22,           // Select Ranged damager and Healer
    SPELL_FILTER_ONLY_MELEE_SPEC                = 23,           // Select Melee damager and Tank
    SPELL_FILTER_ONLY_TANK_SPEC_OR_NOT          = 24,           // Tank selection or exception
    SPELL_FILTER_BY_AURA_CASTER                 = 25,           // Remove target by aura caster
    SPELL_FILTER_PLAYER_IS_HEALER_SPEC          = 26,           // Select Healer
    SPELL_FILTER_RANGED_SPEC_PRIORITY           = 27,           // Select Ranged damager and Healer
    SPELL_FILTER_MELEE_SPEC_PRIORITY            = 28            // Select Melee damager and Tank
};

enum SpellCheckCastType
{
    SPELL_CHECK_CAST_DEFAULT                    = 0,            // Not check type, check only dataType and dataType2
    SPELL_CHECK_CAST_HEALTH                     = 1,            // Check healh percent
};

enum SpellConcatenateAuraOption
{
    CONCATENATE_NONE                     = 0x000,            //
    CONCATENATE_CHANGE_AMOUNT            = 0x001,            // change amount
    CONCATENATE_RECALCULATE_AURA         = 0x002,            // Recalculate amount on aura
    CONCATENATE_RECALCULATE_SPELL        = 0x004,            // Recalculate amount on spell
};

enum SpellConcatenateAuraType
{
    CONCATENATE_ON_UPDATE_AMOUNT  = 0,            //
    CONCATENATE_ON_APPLY_AURA     = 1,            // Recalculate amount on aura
    CONCATENATE_ON_REMOVE_AURA    = 2,            // Recalculate amount on aura
};

// Spell proc event related declarations (accessed using SpellMgr functions)
enum ProcFlags
{
    PROC_FLAG_NONE                            = 0x00000000,

    PROC_FLAG_KILLED                          = 0x00000001,    // 00 Killed by agressor - not sure about this flag
    PROC_FLAG_KILL                            = 0x00000002,    // 01 Kill target (in most cases need XP/Honor reward)

    PROC_FLAG_DONE_MELEE_AUTO_ATTACK          = 0x00000004,    // 02 Done melee auto attack(on hit)
    PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK         = 0x00000008,    // 03 Taken melee auto attack(on hit)

    PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS      = 0x00000010,    // 04 Done attack by Spell that has dmg class melee(on hit)
    PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS     = 0x00000020,    // 05 Taken attack by Spell that has dmg class melee(on hit)

    PROC_FLAG_DONE_RANGED_AUTO_ATTACK         = 0x00000040,    // 06 Done ranged auto attack(on hit)
    PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK        = 0x00000080,    // 07 Taken ranged auto attack(on hit)

    PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS     = 0x00000100,    // 08 Done attack by Spell that has dmg class ranged(after cast if charges and hit of not)
    PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS    = 0x00000200,    // 09 Taken attack by Spell that has dmg class ranged(on hit)

    PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS   = 0x00000400,    // 10 Done positive spell that has dmg class none(after cast)
    PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS  = 0x00000800,    // 11 Taken positive spell that has dmg class none(on hit)

    PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG   = 0x00001000,    // 12 Done negative spell that has dmg class none(after cast)
    PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG  = 0x00002000,    // 13 Taken negative spell that has dmg class none

    PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS  = 0x00004000,    // 14 Done positive spell that has dmg class magic(after cast if charges and hit of not)
    PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS = 0x00008000,    // 15 Taken positive spell that has dmg class magic

    PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG  = 0x00010000,    // 16 Done negative spell that has dmg class magic(after cast if charges and hit of not)
    PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG = 0x00020000,    // 17 Taken negative spell that has dmg class magic

    PROC_FLAG_DONE_PERIODIC                   = 0x00040000,    // 18 Successful do periodic (damage / healing)
    PROC_FLAG_TAKEN_PERIODIC                  = 0x00080000,    // 19 Taken spell periodic (damage / healing)

    PROC_FLAG_TAKEN_DAMAGE                    = 0x00100000,    // 20 Taken any damage
    PROC_FLAG_DONE_TRAP_ACTIVATION            = 0x00200000,    // 21 On trap activation (possibly needs name change to ON_GAMEOBJECT_CAST or USE)

    PROC_FLAG_DONE_MAINHAND_ATTACK            = 0x00400000,    // 22 Done main-hand melee attacks (spell and autoattack)
    PROC_FLAG_DONE_OFFHAND_ATTACK             = 0x00800000,    // 23 Done off-hand melee attacks (spell and autoattack)

    PROC_FLAG_DEATH                           = 0x01000000,    // 24 Died in any way

    PROC_FLAG_ON_JUMP                         = 0x02000000,    // 25 When jump
    PROC_FLAG_TEST_PROC                       = 0x04000000,    // 26 Test proc

    PROC_FLAG_ENTER_COMBAT                    = 0x08000000,    // 27 Entered combat
    PROC_FLAG_ENCOUNTER_START                 = 0x10000000,    // 28 Encounter started

    PROC_FLAG_DONE_SPELL_MAGIC_DMG_POS_NEG    = 0x20000000,    // 29 Alway take charges(or stack) and not amount modify by stack, proc flags 10|14|16

    PROC_FLAG_TEST_PROC4                      = 0x40000000,    // 30 Test proc
    PROC_FLAG_TEST_PROC5                      = 0x80000000,    // 31 Test proc

    // flag masks
    AUTO_ATTACK_PROC_FLAG_MASK                = PROC_FLAG_DONE_MELEE_AUTO_ATTACK | PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK
                                                | PROC_FLAG_DONE_RANGED_AUTO_ATTACK | PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK,

    MELEE_PROC_FLAG_MASK                      = PROC_FLAG_DONE_MELEE_AUTO_ATTACK | PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK
                                                | PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS | PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS
                                                | PROC_FLAG_DONE_MAINHAND_ATTACK | PROC_FLAG_DONE_OFFHAND_ATTACK,

    RANGED_PROC_FLAG_MASK                     = PROC_FLAG_DONE_RANGED_AUTO_ATTACK | PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK
                                                | PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS | PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS,

    SPELL_PROC_FLAG_MASK                      = PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS | PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS
                                                | PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS | PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS
                                                | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS
                                                | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG
                                                | PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS
                                                | PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG | PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG
                                                | PROC_FLAG_DONE_SPELL_MAGIC_DMG_POS_NEG,

    SPELL_CAST_PROC_FLAG_MASK                  = SPELL_PROC_FLAG_MASK | PROC_FLAG_DONE_TRAP_ACTIVATION | RANGED_PROC_FLAG_MASK,

    PERIODIC_PROC_FLAG_MASK                    = PROC_FLAG_DONE_PERIODIC | PROC_FLAG_TAKEN_PERIODIC,

    DONE_HIT_PROC_FLAG_MASK                    = PROC_FLAG_DONE_MELEE_AUTO_ATTACK | PROC_FLAG_DONE_RANGED_AUTO_ATTACK
                                                 | PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS | PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS
                                                 | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG
                                                 | PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG
                                                 | PROC_FLAG_DONE_PERIODIC | PROC_FLAG_DONE_MAINHAND_ATTACK | PROC_FLAG_DONE_OFFHAND_ATTACK
                                                 | PROC_FLAG_DONE_SPELL_MAGIC_DMG_POS_NEG,

    TAKEN_HIT_PROC_FLAG_MASK                   = PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK | PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK
                                                 | PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS | PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS
                                                 | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG
                                                 | PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG
                                                 | PROC_FLAG_TAKEN_PERIODIC | PROC_FLAG_TAKEN_DAMAGE,

    REQ_SPELL_PHASE_PROC_FLAG_MASK             = SPELL_PROC_FLAG_MASK & DONE_HIT_PROC_FLAG_MASK,

    SPELL_PROC_FROM_CAST_MASK                  = PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG | PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS 
                                                 | PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS
                                                 | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG
                                                 | PROC_FLAG_DONE_SPELL_MAGIC_DMG_POS_NEG,
};

constexpr auto MELEE_BASED_TRIGGER_MASK = (PROC_FLAG_DONE_MELEE_AUTO_ATTACK |
	PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK |
	PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS |
	PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS |
	PROC_FLAG_DONE_RANGED_AUTO_ATTACK |
	PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK |
	PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS |
	PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS);

enum ProcFlagsExLegacy
{
    PROC_EX_NONE                = 0x0000000,                 // If none can tigger on Hit/Crit only (passive spells MUST defined by SpellFamily flag)
    PROC_EX_NORMAL_HIT          = 0x0000001,                 // If set only from normal hit (only damage spells)
    PROC_EX_CRITICAL_HIT        = 0x0000002,
    PROC_EX_MISS                = 0x0000004,
    PROC_EX_RESIST              = 0x0000008,
    PROC_EX_DODGE               = 0x0000010,
    PROC_EX_PARRY               = 0x0000020,
    PROC_EX_BLOCK               = 0x0000040,
    PROC_EX_EVADE               = 0x0000080,
    PROC_EX_IMMUNE              = 0x0000100,
    PROC_EX_DEFLECT             = 0x0000200,
    PROC_EX_ABSORB              = 0x0000400,
    PROC_EX_REFLECT             = 0x0000800,
    PROC_EX_INTERRUPT           = 0x0001000,                 // Melee hit result can be Interrupt (not used)
    PROC_EX_FULL_BLOCK          = 0x0002000,                 // block al attack damage
    PROC_EX_ON_CAST             = 0x0004000,
    PROC_EX_NOT_ACTIVE_SPELL    = 0x0008000,                 // Spell mustn't do damage/heal to proc
    PROC_EX_EX_TRIGGER_ALWAYS   = 0x0010000,                 // If set trigger always no matter of hit result
    PROC_EX_EX_ONE_TIME_TRIGGER = 0x0020000,                 // If set trigger always but only one time
    PROC_EX_ONLY_ACTIVE_SPELL   = 0x0040000,                 // Spell has to do damage/heal to proc
    PROC_EX_DISPEL              = 0x0080000,

    // Flags for internal use - do not use these in db!
    PROC_EX_INTERNAL_CANT_PROC  = 0x0800000,
    PROC_EX_INTERNAL_DOT        = 0x1000000,
    PROC_EX_INTERNAL_HOT        = 0x2000000,
    PROC_EX_INTERNAL_TRIGGERED  = 0x4000000,
    PROC_EX_INTERNAL_REQ_FAMILY = 0x8000000
};

constexpr auto AURA_SPELL_PROC_EX_MASK =
	(PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT | PROC_EX_MISS |
	PROC_EX_RESIST | PROC_EX_DODGE | PROC_EX_PARRY | PROC_EX_BLOCK |
	PROC_EX_EVADE | PROC_EX_IMMUNE | PROC_EX_DEFLECT |
	PROC_EX_ABSORB | PROC_EX_REFLECT | PROC_EX_INTERRUPT | PROC_EX_DISPEL);

enum ProcFlagsSpellType
{
    PROC_SPELL_TYPE_NONE              = 0x0000000,
    PROC_SPELL_TYPE_DAMAGE            = 0x0000001, // damage type of spell
    PROC_SPELL_TYPE_HEAL              = 0x0000002, // heal type of spell
    PROC_SPELL_TYPE_NO_DMG_HEAL       = 0x0000004, // other spells
    PROC_SPELL_TYPE_MASK_ALL          = PROC_SPELL_TYPE_DAMAGE | PROC_SPELL_TYPE_HEAL | PROC_SPELL_TYPE_NO_DMG_HEAL
};

enum ProcFlagsSpellPhase
{
    PROC_SPELL_PHASE_NONE             = 0x0000000,
    PROC_SPELL_PHASE_CAST             = 0x0000001,
    PROC_SPELL_PHASE_HIT              = 0x0000002,
    PROC_SPELL_PHASE_FINISH           = 0x0000004,
    PROC_SPELL_PHASE_MASK_ALL         = PROC_SPELL_PHASE_CAST | PROC_SPELL_PHASE_HIT | PROC_SPELL_PHASE_FINISH
};

enum ProcFlagsHit
{
    PROC_HIT_NONE                = 0x0000000, // no value - PROC_HIT_NORMAL | PROC_HIT_CRITICAL for TAKEN proc type, PROC_HIT_NORMAL | PROC_HIT_CRITICAL | PROC_HIT_ABSORB for DONE
    PROC_HIT_NORMAL              = 0x0000001, // non-critical hits
    PROC_HIT_CRITICAL            = 0x0000002,
    PROC_HIT_MISS                = 0x0000004,
    PROC_HIT_FULL_RESIST         = 0x0000008,
    PROC_HIT_DODGE               = 0x0000010,
    PROC_HIT_PARRY               = 0x0000020,
    PROC_HIT_BLOCK               = 0x0000040, // partial or full block
    PROC_HIT_EVADE               = 0x0000080,
    PROC_HIT_IMMUNE              = 0x0000100,
    PROC_HIT_DEFLECT             = 0x0000200,
    PROC_HIT_ABSORB              = 0x0000400, // partial or full absorb
    PROC_HIT_REFLECT             = 0x0000800,
    PROC_HIT_INTERRUPT           = 0x0001000, // (not used atm)
    PROC_HIT_FULL_BLOCK          = 0x0002000,
    PROC_HIT_MASK_ALL = 0x2FFF,
};

enum ProcAttributes
{
    PROC_ATTR_REQ_EXP_OR_HONOR   = 0x0000010,
};

struct SpellProcEventEntry
{
    uint32      schoolMask;                                 // if nonzero - bit mask for matching proc condition based on spell candidate's school: Fire=2, Mask=1<<(2-1)=2
    uint32      spellFamilyName;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyNamer value
    flag128     spellFamilyMask;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyFlags  (like auras 107 and 108 do)
    uint32      procFlags;                                  // bitmask for matching proc event
    uint32      procEx;                                     // proc Extend info (see ProcFlagsEx)
    float       ppmRate;                                    // for melee (ranged?) damage spells - proc rate per minute. if zero, falls back to flat chance from Spell.dbc
    float       customChance;                               // Owerride chance (in most cases for debug only)
    double      cooldown;                                   // hidden cooldown used for some spell proc events, applied to _triggered_spell_
    uint32      effectMask;                                 // Effect Mask for aply to effect
};

typedef std::vector<std::vector<SpellProcEventEntry> > SpellProcEventMap;

struct SpellProcEntry
{
    uint32      schoolMask;                                 // if nonzero - bitmask for matching proc condition based on spell's school
    uint32      spellFamilyName;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyName
    flag128     spellFamilyMask;                            // if nonzero - bitmask for matching proc condition based on candidate spell's SpellFamilyFlags
    uint32      typeMask;                                   // if nonzero - owerwrite procFlags field for given Spell.dbc entry, bitmask for matching proc condition, see enum ProcFlags
    uint32      spellTypeMask;                              // if nonzero - bitmask for matching proc condition based on candidate spell's damage/heal effects, see enum ProcFlagsSpellType
    uint32      spellPhaseMask;                             // if nonzero - bitmask for matching phase of a spellcast on which proc occurs, see enum ProcFlagsSpellPhase
    uint32      hitMask;                                    // if nonzero - bitmask for matching proc condition based on hit result, see enum ProcFlagsHit
    uint32      attributesMask;                             // bitmask, see ProcAttributes
    float       ratePerMinute;                              // if nonzero - chance to proc is equal to value * aura caster's weapon speed / 60
    float       chance;                                     // if nonzero - owerwrite procChance field for given Spell.dbc entry, defines chance of proc to occur, not used if perMinuteRate set
    float       cooldown;                                   // if nonzero - cooldown in secs for aura proc, applied to aura
    uint32      charges;                                    // if nonzero - owerwrite procCharges field for given Spell.dbc entry, defines how many times proc can occur before aura remove, 0 - infinite
    uint32      modcharges;                                 // if nonzero - procCharges field for given Spell.dbc entry, defines how many times proc can occur before aura remove, 0 - infinite
};

typedef std::unordered_map<uint32, SpellProcEntry> SpellProcMap;

struct SpellEnchantProcEntry
{
    uint32      customChance;
    float       PPMChance;
    uint32      procEx;
};

typedef std::unordered_map<uint32, SpellEnchantProcEntry> SpellEnchantProcEventMap;

struct SpellBonusEntry
{
    float  direct_damage;
    float  dot_damage;
    float  ap_bonus;
    float  ap_dot_bonus;
    float  damage_bonus;
    float  heal_bonus;
};

typedef std::unordered_map<uint32, SpellBonusEntry> SpellBonusMap;
typedef std::vector<SpellBonusEntry*> SpellBonusVector;

enum SpellGroup
{
    SPELL_GROUP_NONE = 0,
    SPELL_GROUP_ELIXIR_BATTLE = 1,
    SPELL_GROUP_ELIXIR_GUARDIAN = 2,
    SPELL_GROUP_ELIXIR_UNSTABLE = 3,
    SPELL_GROUP_ELIXIR_SHATTRATH = 4,
    SPELL_GROUP_CORE_RANGE_MAX = 5,
};

const uint16 SPELL_GROUP_DB_RANGE_MIN = 1000;

//                  spell_id, group_id
typedef std::multimap<uint32, SpellGroup > SpellSpellGroupMap;
typedef std::pair<SpellSpellGroupMap::const_iterator, SpellSpellGroupMap::const_iterator> SpellSpellGroupMapBounds;

//                      group_id, spell_id
typedef std::multimap<SpellGroup, int32> SpellGroupSpellMap;
typedef std::pair<SpellGroupSpellMap::const_iterator, SpellGroupSpellMap::const_iterator> SpellGroupSpellMapBounds;

enum SpellGroupStackRule
{
    SPELL_GROUP_STACK_RULE_DEFAULT = 0,
    SPELL_GROUP_STACK_RULE_EXCLUSIVE = 1,
    SPELL_GROUP_STACK_RULE_EXCLUSIVE_FROM_SAME_CASTER = 2,
    SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT = 3,
    SPELL_GROUP_STACK_RULE_EXCLUSIVE_HIGHEST,
    SPELL_GROUP_STACK_RULE_MAX
};

typedef std::map<SpellGroup, SpellGroupStackRule> SpellGroupStackMap;

struct SpellThreatEntry
{
    int32       flatMod;                                    // flat threat-value for this Spell  - default: 0
    float       pctMod;                                     // threat-multiplier for this Spell  - default: 1.0f
    float       apPctMod;                                   // Pct of AP that is added as Threat - default: 0.0f
};

typedef std::unordered_map<uint32, SpellThreatEntry> SpellThreatMap;

// coordinates for spells (accessed using SpellMgr functions)
struct SpellTargetPosition
{
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
};

// Enum with EffectRadiusIndex and their actual radius
enum EffectRadiusIndex
{
    EFFECT_RADIUS_2_YARDS       = 7,
    EFFECT_RADIUS_5_YARDS       = 8,
    EFFECT_RADIUS_20_YARDS      = 9,
    EFFECT_RADIUS_30_YARDS      = 10,
    EFFECT_RADIUS_45_YARDS      = 11,
    EFFECT_RADIUS_100_YARDS     = 12,
    EFFECT_RADIUS_10_YARDS      = 13,
    EFFECT_RADIUS_8_YARDS       = 14,
    EFFECT_RADIUS_3_YARDS       = 15,
    EFFECT_RADIUS_1_YARD        = 16,
    EFFECT_RADIUS_13_YARDS      = 17,
    EFFECT_RADIUS_15_YARDS      = 18,
    EFFECT_RADIUS_18_YARDS      = 19,
    EFFECT_RADIUS_25_YARDS      = 20,
    EFFECT_RADIUS_35_YARDS      = 21,
    EFFECT_RADIUS_200_YARDS     = 22,
    EFFECT_RADIUS_40_YARDS      = 23,
    EFFECT_RADIUS_65_YARDS      = 24,
    EFFECT_RADIUS_70_YARDS      = 25,
    EFFECT_RADIUS_4_YARDS       = 26,
    EFFECT_RADIUS_50_YARDS      = 27,
    EFFECT_RADIUS_50000_YARDS   = 28,
    EFFECT_RADIUS_6_YARDS       = 29,
    EFFECT_RADIUS_500_YARDS     = 30,
    EFFECT_RADIUS_80_YARDS      = 31,
    EFFECT_RADIUS_12_YARDS      = 32,
    EFFECT_RADIUS_99_YARDS      = 33,
    EFFECT_RADIUS_55_YARDS      = 35,
    EFFECT_RADIUS_0_YARDS       = 36,
    EFFECT_RADIUS_7_YARDS       = 37,
    EFFECT_RADIUS_21_YARDS      = 38,
    EFFECT_RADIUS_34_YARDS      = 39,
    EFFECT_RADIUS_9_YARDS       = 40,
    EFFECT_RADIUS_150_YARDS     = 41,
    EFFECT_RADIUS_11_YARDS      = 42,
    EFFECT_RADIUS_16_YARDS      = 43,
    EFFECT_RADIUS_0_5_YARDS     = 44,   // 0.5 yards
    EFFECT_RADIUS_10_YARDS_2    = 45,
    EFFECT_RADIUS_5_YARDS_2     = 46,
    EFFECT_RADIUS_15_YARDS_2    = 47,
    EFFECT_RADIUS_60_YARDS      = 48,
    EFFECT_RADIUS_90_YARDS      = 49,
    EFFECT_RADIUS_15_YARDS_3    = 50,
    EFFECT_RADIUS_60_YARDS_2    = 51,
    EFFECT_RADIUS_5_YARDS_3     = 52,
    EFFECT_RADIUS_60_YARDS_3    = 53,
    EFFECT_RADIUS_50000_YARDS_2 = 54,
    EFFECT_RADIUS_130_YARDS     = 55,
    EFFECT_RADIUS_38_YARDS      = 56,
    EFFECT_RADIUS_45_YARDS_2    = 57,
    EFFECT_RADIUS_32_YARDS      = 59,
    EFFECT_RADIUS_44_YARDS      = 60,
    EFFECT_RADIUS_14_YARDS      = 61,
    EFFECT_RADIUS_47_YARDS      = 62,
    EFFECT_RADIUS_23_YARDS      = 63,
    EFFECT_RADIUS_3_5_YARDS     = 64,   // 3.5 yards
    EFFECT_RADIUS_80_YARDS_2    = 65
};

typedef std::unordered_map<uint32, SpellTargetPosition> SpellTargetPositionMap;

// Spell pet auras
struct PetAura
{
    int32 petEntry;
    int32 spellId;
    int32 target;
    int32 targetaura;
    int32 option;
    float bp0;
    float bp1;
    float bp2;
    int32 aura;
    int32 casteraura;
    int32 createdspell;
    int32 fromspell;
};

typedef std::unordered_map<int32, std::vector<PetAura> > SpellPetAuraMap;

struct SpellArea
{
    uint32 spellId;
    int32  areaId;                                          // zone/subzone/or 0 is not limited to zone, if - than mapid
    uint32 questStart;                                      // quest start (quest must be active or rewarded for spell apply)
    uint32 questEnd;                                        // quest end (quest must not be rewarded for spell apply)
    int32  auraSpell;                                       // spell aura must be applied for spell apply)if possitive) and it must not be applied in other case
    uint32 raceMask;                                        // can be applied only to races
    Gender gender;                                          // can be applied only to gender
    uint32 questStartStatus;                                // QuestStatus that quest_start must have in order to keep the spell
    uint32 questEndStatus;                                  // QuestStatus that the quest_end must have in order to keep the spell (if the quest_end's status is different than this, the spell will be dropped)
    bool autocast;                                          // if true then auto applied at area enter, in other case just allowed to cast
    int32 classmask;                                        // can be applied only to class
    uint32 active_event;                                    // can be applied only if current event active
    
    SpellInfo const* spellInfo;
    uint32 RequiredAreasID;
    std::vector<uint32> areaGroupMembers;

    // helpers
    bool IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const;
};

typedef std::multimap<uint32, SpellArea> SpellAreaMap;
typedef std::multimap<uint32, SpellArea const*> SpellAreaForQuestMap;
typedef std::multimap<uint32, SpellArea const*> SpellAreaForAuraMap;
typedef std::multimap<uint32, SpellArea const*> SpellAreaForAreaMap;
typedef std::pair<SpellAreaMap::const_iterator, SpellAreaMap::const_iterator> SpellAreaMapBounds;
typedef std::pair<SpellAreaForQuestMap::const_iterator, SpellAreaForQuestMap::const_iterator> SpellAreaForQuestMapBounds;
typedef std::pair<SpellAreaForAuraMap::const_iterator, SpellAreaForAuraMap::const_iterator>  SpellAreaForAuraMapBounds;
typedef std::pair<SpellAreaForAreaMap::const_iterator, SpellAreaForAreaMap::const_iterator>  SpellAreaForAreaMapBounds;

// Spell rank chain  (accessed using SpellMgr functions)
struct SpellChainNode
{
    SpellInfo const* prev;
    SpellInfo const* next;
    SpellInfo const* first;
    SpellInfo const* last;
    uint8  rank;
};

typedef std::unordered_map<uint32, SpellChainNode> SpellChainMap;
typedef std::multimap<uint32 /*spell_id*/, uint32 /*req_spell*/> SpellRequiredMap;
typedef std::multimap<uint32 /*spell_id*/, uint32 /*req_spell*/> SpellsRequiringSpellMap;

// Spell learning properties (accessed using SpellMgr functions)
struct SpellLearnSkillNode
{
    uint16 skill;
    uint16 step;
    uint16 value;                                           // 0  - max skill value for player level
    uint16 maxvalue;                                        // 0  - max skill value for player level
};

typedef std::unordered_map<uint32, SpellLearnSkillNode> SpellLearnSkillMap;

struct SpellLearnSpellNode
{
    uint32 spell;
    uint32 overridesSpell;
    bool active;                    // show in spellbook or not
    bool autoLearned;               // This marks the spell as automatically learned from another source that - will only be used for unlearning
};

typedef std::multimap<uint32, SpellLearnSpellNode> SpellLearnSpellMap;
typedef std::pair<SpellLearnSpellMap::const_iterator, SpellLearnSpellMap::const_iterator> SpellLearnSpellMapBounds;

typedef std::multimap<uint32, SkillLineAbilityEntry const*> SkillLineAbilityMap;
typedef std::pair<SkillLineAbilityMap::const_iterator, SkillLineAbilityMap::const_iterator> SkillLineAbilityMapBounds;

typedef std::multimap<uint32, uint32> PetLevelupSpellSet;
typedef std::unordered_map<uint32, PetLevelupSpellSet> PetLevelupSpellMap;

struct PetDefaultSpellsEntry
{
    uint32 spellid[MAX_CREATURE_SPELL_DATA_SLOT];
};

// < 0 for petspelldata id, > 0 for creature_id
typedef std::unordered_map<int32, PetDefaultSpellsEntry> PetDefaultSpellsMap;

typedef std::vector<uint32> SpellCustomAttribute;
typedef std::vector<bool> EnchantCustomAttribute;

typedef std::vector<SpellInfo*> SpellInfoMap;

struct SpellLinked
{
    int32 effect;
    int32 hastalent;
    uint8 hastype;
    int32 hasparam;
    int32 hastalent2;
    uint8 hastype2;
    int32 hasparam2;
    int32 chance;
    int32 caster;
    int32 target;
    int32 cooldown;
    uint32 hitmask;
    uint32 effectMask;
    int32 removeMask;
    uint8 actiontype;
    int8 targetCount;
    int8 targetCountType;
    int8 group;
    int32 duration;
    float param;
    std::list<int32> randList;
};

struct SpellTalentLinked
{
    int32 talent;
    int32 triger;
    int32 type;
    int32 caster;
    int32 target;
};

struct SpellPrcoCheck
{
    int32 checkspell;
    int32 hastalent;
    int32 chance;
    int32 target;
    int32 powertype;
    int32 dmgclass;
    int32 effectmask;
    int32 specId;
    int32 spellAttr0;
    int32 targetTypeMask;
    uint32 mechanicMask;
    int32 fromlevel;
    int32 perchp;
    int32 spelltypeMask;
    int32 combopoints;
    int32 deathstateMask;
    int32 hasDuration;
};

struct SpellTriggered
{
    int32 spell_id;
    int32 spell_trigger;
    int32 spell_cooldown;
    int32 target;
    int32 caster;
    int32 targetaura;
    int32 targetaura2;
    int32 option;
    float bp0;
    float bp1;
    float bp2;
    int32 effectmask;
    int32 aura;
    int32 aura2;
    int32 chance;
    int32 group;
    int32 procFlags;
    int32 procEx;
    int32 check_spell_id;
    int32 addptype;
    int32 schoolMask;
    int32 dummyId;
    int32 dummyEffect;
    int32 CreatureType;
    int32 slot;
    std::list<int32> randList;
};

struct SpellDummyTrigger
{
    int32 spell_id;
    int32 spell_trigger;
    int32 option;
    int32 target;
    int32 caster;
    int32 targetaura;
    int32 aura; // Chaeck on caster
    float bp0;
    float bp1;
    float bp2;
    int32 effectmask;
    int32 handlemask;
    int8  group;
};

struct AuraPeriodickTrigger
{
    int32 spell_id;
    int32 spell_trigger;
    int32 target;
    int32 caster;
    int32 option;
    int32 option2;
    float bp0;
    float bp1;
    float bp2;
    uint32 effectmask;
    int32 chance;
    int32 slot;
    uint8 hastype;
    int32 hastalent;
    int32 hasparam;
    uint8 hastype2;
    int32 hastalent2;
    int32 hasparam2;
};

enum SpellVisualType
{
    SPELL_VISUAL_TYPE_ON_CAST = 0,
    SPELL_VISUAL_TYPE_CUSTOM  = 1,
};

struct SpellVisual
{
    int32 spellId;
    int32 SpellVisualID;
    int16 MissReason;
    int16 ReflectStatus;
    float TravelSpeed;
    bool SpeedAsTime;
    bool HasPosition;
    int16 type;
};

struct SpellVisualPlayOrphan
{
    int32 spellId;
    int32 SpellVisualID;
    float TravelSpeed;
    bool SpeedAsTime;
    float UnkFloat;
    Position SourceOrientation;
    int8 type;
};

struct SpellVisualKit
{
    int32 spellId;
    int32 KitType;
    int32 KitRecID;
    int32 Duration;
};

struct SpellPendingCast
{
    int32 spell_id;
    int32 pending_id;
    int8  option;
    int32 check;
};

struct SpellAuraDummy
{
    int32 spellId;
    int32 spellDummyId;
    int32 type;
    int32 target;
    int32 caster;
    int32 targetaura;
    int32 option;
    int32 effectmask;
    int32 effectDummy;
    int32 aura;
    int32 removeAura;
    int32 attr;
    int32 attrValue;
    float custombp;
    int32 hastalent;
    uint8 hastype;
    int32 hasparam;
    int32 hastalent2;
    uint8 hastype2;
    int32 hasparam2;
};

struct SpellTargetFilter
{
    int32 spellId;
    int32 targetId;
    int32 option;
    int32 param1;
    int32 param2;
    int32 param3;
    int32 aura;
    int32 chance;
    int32 effectMask;
    int32 resizeType;
    int32 count;
    int32 maxcount;
    int32 addcount;
    int32 addcaster;
};

struct SpellScene
{
    int32 SceneScriptPackageID;
    int32 MiscValue;
    int32 PlaybackFlags;
    uint32 CustomDuration;
    uint32 scriptID;
};

struct SceneTriggerEvent
{
    std::string Event;
    int32 MiscValue;
    uint32 trigerSpell;
    uint32 MonsterCredit;
};

struct SpellConcatenateAura
{
    int32 spellid;
    int32 effectSpell;
    int32 auraId;
    int32 effectAura;
    uint32 option;
    int8 caster;
    int8 target;
};

struct SpellCheckCast
{
    int32 spellId;
    int32 type;
    int32 errorId;
    int32 customErrorId;
    int32 caster;
    int32 target;
    int32 checkType;
    int32 dataType;
    int32 checkType2;
    int32 dataType2;
    int32 param1;
    int32 param2;
    int32 param3;
};

struct SpellTriggerDelay
{
    int32 spell_id;
    int32 spell_trigger;
    int32 option;
    uint32 effectMask;
    int32 target;
    int32 caster;
    int32 targetaura;
    int32 aura;
};

typedef std::unordered_map<int32, std::vector<SpellTriggered> > SpellTriggeredMap;
typedef std::vector<std::vector<SpellTriggered>* > SpellTriggeredVector;
typedef std::unordered_map<int32, std::vector<SpellDummyTrigger> > SpellDummyTriggerMap;
typedef std::vector<std::vector<SpellDummyTrigger>* > SpellDummyTriggerVector;
typedef std::unordered_map<int32, std::vector<AuraPeriodickTrigger> > SpellAuraTriggerMap;
typedef std::vector<std::vector<AuraPeriodickTrigger>* > SpellAuraTriggerVector;
typedef std::unordered_map<int32, std::vector<SpellAuraDummy> > SpellAuraDummyMap;
typedef std::vector<std::vector<SpellAuraDummy>* > SpellAuraDummyVector;
typedef std::unordered_map<int32, std::vector<SpellTargetFilter> > SpellTargetFilterMap;
typedef std::vector<std::vector<SpellTargetFilter>* > SpellTargetFilterVector;
typedef std::unordered_map<int32, std::vector<SpellCheckCast> > SpellCheckCastMap;
typedef std::vector<std::vector<SpellCheckCast>* > SpellCheckCastVector;
typedef std::unordered_map<int32, std::vector<SpellLinked> > SpellLinkedMap[SPELL_LINK_MAX];
typedef std::unordered_map<int32, std::vector<SpellTalentLinked> > SpellTalentLinkedMap;
typedef std::vector<std::vector<SpellPrcoCheck> > SpellPrcoCheckMap;
typedef std::unordered_map<int32, std::vector<SpellVisual> > PlaySpellVisualMap;
typedef std::unordered_map<int32, std::vector<SpellVisualPlayOrphan> > SpellVisualPlayOrphanMap;
typedef std::unordered_map<int32, std::vector<SpellVisualKit> > SpellVisualKitMap;
typedef std::unordered_map<int32, std::vector<SpellPendingCast> > SpellPendingCastMap;
typedef std::map<int32, SpellScene > SpellSceneMap;
typedef std::map<int32, std::vector<SceneTriggerEvent> > SceneTriggerEventMap;
typedef std::unordered_map<int32, std::vector<SpellConcatenateAura> > SpellConcatenateApplyMap;
typedef std::unordered_map<int32, std::vector<SpellConcatenateAura> > SpellConcatenateUpdateMap;
typedef std::unordered_map<int32, std::vector<SpellTriggerDelay> > SpellTriggerDelayMap;
typedef std::vector<std::vector<SpellTriggerDelay>* > SpellTriggerDelayVector;

bool IsWeaponSkill(uint32 skill);

bool IsPrimaryProfessionSkill(uint32 skill);

inline bool IsProfessionSkill(uint32 skill)
{
    return  IsPrimaryProfessionSkill(skill) || skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID || skill == SKILL_ARCHAEOLOGY ||
            skill == SKILL_WAY_OF_THE_GRILL || skill == SKILL_WAY_OF_THE_WOK || skill == SKILL_WAY_OF_THE_POT || skill == SKILL_WAY_OF_THE_STEAMER ||
            skill == SKILL_WAY_OF_THE_OVEN || skill == SKILL_WAY_OF_THE_BREW;
}

inline bool IsProfessionOrRidingSkill(uint32 skill)
{
    return  IsProfessionSkill(skill) || skill == SKILL_RIDING;
}

bool IsPartOfSkillLine(uint32 skillId, uint32 spellId);

// spell diminishing returns
DiminishingGroup GetDiminishingReturnsGroupForSpell(SpellInfo const* spellproto, bool triggered);
DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group);
DiminishingLevels GetDiminishingReturnsMaxLevel(DiminishingGroup group);
int32 GetDiminishingReturnsLimitDuration(DiminishingGroup group, SpellInfo const* spellproto);
bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group);

typedef std::unordered_map<uint32, std::list<uint32> > SpellOverrideInfo;
typedef std::set<uint32> TalentSpellSet;

struct SpellInfoLoadHelper
{
    SpellEntry const* Entry = nullptr;

    std::set<SpellAuraOptionsEntry const*> AuraOptions;
    SpellAuraRestrictionsEntry const* AuraRestrictions = nullptr;
    SpellCastingRequirementsEntry const* CastingRequirements = nullptr;
    SpellCategoriesEntry const* Categories = nullptr;
    SpellClassOptionsEntry const* ClassOptions = nullptr;
    SpellCooldownsEntry const* Cooldowns = nullptr;
    SpellEquippedItemsEntry const* EquippedItems = nullptr;
    SpellInterruptsEntry const* Interrupts = nullptr;
    SpellLevelsEntry const* Levels = nullptr;
    std::set<SpellMiscEntry const*> Misc;
    SpellReagentsEntry const* Reagents = nullptr;
    SpellScalingEntry const* Scaling = nullptr;
    SpellShapeshiftEntry const* Shapeshift = nullptr;
    SpellReagentsCurrencyEntry const* ReagentsCurrency = nullptr;
    SpellTotemsEntry const* Totems = nullptr;
};

class SpellMgr
{
    // Constructors
        SpellMgr();
        ~SpellMgr();

    // Accessors (const or static functions)
    public:
        static SpellMgr* instance()
        {
            static SpellMgr instance;
            return &instance;
        }

        // Spell correctness for client using
        static bool IsSpellValid(SpellInfo const* spellInfo, Player* player = nullptr, bool msg = true);
        bool IsSpellForbidden(uint32 spellid);

        // Spell Ranks table
        SpellChainNode const* GetSpellChainNode(uint32 spell_id) const;
        uint32 GetFirstSpellInChain(uint32 spell_id) const;
        uint32 GetLastSpellInChain(uint32 spell_id) const;
        uint32 GetNextSpellInChain(uint32 spell_id) const;
        uint32 GetPrevSpellInChain(uint32 spell_id) const;
        uint8 GetSpellRank(uint32 spell_id) const;
        // not strict check returns provided spell if rank not avalible
        uint32 GetSpellWithRank(uint32 spell_id, uint32 rank, bool strict = false) const;

        // Spell Required table
        Trinity::IteratorPair<SpellRequiredMap::const_iterator> GetSpellsRequiredForSpellBounds(uint32 spell_id) const;
        Trinity::IteratorPair<SpellsRequiringSpellMap::const_iterator> GetSpellsRequiringSpellBounds(uint32 spell_id) const;
        bool IsSpellRequiringSpell(uint32 spellid, uint32 req_spellid) const;
    SpellsRequiringSpellMap GetSpellsRequiringSpell();
        uint32 GetSpellRequired(uint32 spell_id) const;

        // Spell learning
        SpellLearnSkillNode const* GetSpellLearnSkill(uint32 spell_id) const;
        SpellLearnSpellMapBounds GetSpellLearnSpellMapBounds(uint32 spell_id) const;
        bool IsSpellLearnSpell(uint32 spell_id) const;
        bool IsSpellLearnToSpell(uint32 spell_id1, uint32 spell_id2) const;

        // Spell target coordinates
        SpellTargetPosition const* GetSpellTargetPosition(uint32 spell_id) const;

        // Spell Groups table
        SpellSpellGroupMapBounds GetSpellSpellGroupMapBounds(uint32 spell_id) const;
        uint32 IsSpellMemberOfSpellGroup(uint32 spellid, SpellGroup groupid) const;

        SpellGroupSpellMapBounds GetSpellGroupSpellMapBounds(SpellGroup group_id) const;
        void GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells) const;
        void GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells, std::set<SpellGroup>& usedGroups) const;

        // Spell Group Stack Rules table
        bool AddSameEffectStackRuleSpellGroups(SpellInfo const* spellInfo, int32 amount, std::map<SpellGroup, int32>& groups) const;
        bool AddSameEffectStackRuleSpellGroups(SpellInfo const* spellInfo, AuraEffect* eff, std::multimap<SpellGroup, AuraEffect*>& groups) const;
        SpellGroupStackRule CheckSpellGroupStackRules(SpellInfo const* spellInfo1, SpellInfo const* spellInfo2) const;
        SpellGroupStackRule GetSpellGroupStackRule(SpellGroup groupid) const;

        // Spell proc event table
        const std::vector<SpellProcEventEntry>* GetSpellProcEvent(uint32 spellId) const;
        bool IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const* spellProcEvent, uint32 EventProcFlag, SpellInfo const* procSpell, uint32 procFlags, uint32 procExtra, bool active);

        // Spell proc table
        SpellProcEntry const* GetSpellProcEntry(uint32 spellId) const;
        bool CanSpellTriggerProcOnEvent(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo);

        // Spell bonus data table
        SpellBonusEntry const* GetSpellBonusData(uint32 spellId) const;

        // Spell threat table
        SpellThreatEntry const* GetSpellThreatEntry(uint32 spellID) const;

        SkillLineAbilityMapBounds GetSkillLineAbilityMapBounds(uint32 spell_id) const;

        const std::vector<PetAura>* GetPetAura(int32 entry) const;

        SpellEnchantProcEntry const* GetSpellEnchantProcEvent(uint32 enchId) const;
        bool IsArenaAllowedEnchancment(uint32 ench_id) const;

        const std::vector<SpellLinked>* GetSpellLinked(int8 type, int32 spell_id) const;
        const std::vector<SpellTalentLinked>* GetSpelltalentLinked(int32 spell_id) const;
        const std::vector<SpellConcatenateAura>* GetSpellConcatenateApply(int32 spell_id) const;
        const std::vector<SpellConcatenateAura>* GetSpellConcatenateUpdate(int32 spell_id) const;
        const std::vector<SpellPrcoCheck>* GetSpellPrcoCheck(int32 spell_id) const;
        const std::vector<SpellTriggered>* GetSpellTriggered(int32 spell_id) const;
        const std::vector<SpellDummyTrigger>* GetSpellDummyTrigger(int32 spell_id) const;
        const std::vector<AuraPeriodickTrigger>* GetSpellAuraTrigger(int32 spell_id) const;
        const std::vector<SpellAuraDummy>* GetSpellAuraDummy(int32 spell_id) const;
        const std::vector<SpellTargetFilter>* GetSpellTargetFilter(int32 spell_id) const;
        const std::vector<SpellCheckCast>* GetSpellCheckCast(int32 spell_id) const;
        const std::vector<SpellVisual>* GetPlaySpellVisualData(int32 spell_id) const;
        const std::vector<SpellVisualPlayOrphan>* GetSpellVisualPlayOrphan(int32 spell_id) const;
        const std::vector<SpellVisualKit>* GetSpellVisualKit(int32 spell_id) const;
        const std::vector<SpellPendingCast>* GetSpellPendingCast(int32 spell_id) const;
        const SpellScene* GetSpellScene(int32 miscValue) const;
        const std::vector<SceneTriggerEvent>* GetSceneTriggerEvent(int32 miscValue) const;
        const std::vector<SpellTriggerDelay>* GetSpellTriggerDelay(int32 spell_id) const;

        PetLevelupSpellSet const* GetPetLevelupSpellList(uint32 petFamily) const;
        PetDefaultSpellsEntry const* GetPetDefaultSpellsEntry(int32 id) const;

        // Spell area
        SpellAreaMapBounds GetSpellAreaMapBounds(uint32 spell_id) const;
        SpellAreaForQuestMapBounds GetSpellAreaForQuestMapBounds(uint32 quest_id) const;
        SpellAreaForQuestMapBounds GetSpellAreaForQuestEndMapBounds(uint32 quest_id) const;
        SpellAreaForAuraMapBounds GetSpellAreaForAuraMapBounds(uint32 spell_id) const;
        SpellAreaForAreaMapBounds GetSpellAreaForAreaMapBounds(uint32 area_id) const;

        // SpellInfo object management
        SpellInfo const* GetSpellInfo(uint32 spellId) const;
        uint32 GetSpellInfoStoreSize() const;
        std::list<uint32> const* GetSpellOverrideInfo(uint32 spellId);
        SpellInfo const* AssertSpellInfo(uint32 spellId) const;

        bool IsTalent(uint32 spellId);

    std::list<SkillLineAbilityEntry const*> const& GetTradeSpellFromSkill(uint32 skillLineId);


        // Loading data at server startup
        void LoadSpellRanks();
        void LoadSpellRequired();
        void LoadSpellLearnSkills();
        void LoadSpellLearnSpells();
        void LoadSpellTargetPositions();
        void LoadSpellGroups();
        void LoadSpellGroupStackRules();
        void LoadSpellProcEvents();
        void LoadSpellProcs();
        void LoadSpellBonusess();
        void LoadSpellThreats();
        void LoadSkillLineAbilityMap();
        void LoadSpellPetAuras();
        void LoadEnchantCustomAttr();
        void LoadSpellEnchantProcData();
        void LoadSpellLinked();
        void LoadTalentSpellLinked();
        void LoadSpellConcatenateAura();
        void LoadSpellPrcoCheck();
        void LoadSpellTriggered();
        void LoadPetLevelupSpellMap();
        void LoadPetDefaultSpells();
        void LoadSpellAreas();
        void LoadSpellInfoStore();
        void UnloadSpellInfoStore();
        void UnloadSpellInfoImplicitTargetConditionLists();
        void LoadSpellCustomAttr();
        void LoadTalentSpellInfo();
        void LoadForbiddenSpells();
        void LoadSpellVisual();
        void LoadSpellPendingCast();
        void LoadSpellScene();
        void LoadSpellInfoSpellSpecificAndAuraState();

        std::vector<uint32>        mSpellCreateItemList;

    private:
        SpellChainMap              mSpellChains;
        SpellsRequiringSpellMap    mSpellsReqSpell;
        SpellRequiredMap           mSpellReq;
        SpellLearnSkillMap         mSpellLearnSkills;
        SpellLearnSpellMap         mSpellLearnSpells;
        SpellTargetPositionMap     mSpellTargetPositions;
        SpellSpellGroupMap         mSpellSpellGroup;
        SpellGroupSpellMap         mSpellGroupSpell;
        SpellGroupStackMap         mSpellGroupStack;
        SpellProcEventMap          mSpellProcEventMap;
        SpellProcMap               mSpellProcMap;
        SpellBonusMap              mSpellBonusMap;
        SpellBonusVector           mSpellBonusVector;
        SpellThreatMap             mSpellThreatMap;
        SpellPetAuraMap            mSpellPetAuraMap;
        SpellLinkedMap             mSpellLinkedMap;
        SpellTalentLinkedMap       mSpellTalentLinkedMap;
        SpellPrcoCheckMap          mSpellPrcoCheckMap;
        SpellTriggeredMap          mSpellTriggeredMap;
        SpellTriggeredVector       mSpellTriggeredVector;
        SpellDummyTriggerMap       mSpellDummyTriggerMap;
        SpellDummyTriggerVector    mSpellDummyTriggerVector;
        SpellAuraTriggerMap        mSpellAuraTriggerMap;
        SpellAuraTriggerVector     mSpellAuraTriggerVector;
        SpellAuraDummyMap          mSpellAuraDummyMap;
        SpellAuraDummyVector       mSpellAuraDummyVector;
        SpellTargetFilterMap       mSpellTargetFilterMap;
        SpellTargetFilterVector    mSpellTargetFilterVector;
        SpellCheckCastMap          mSpellCheckCastMap;
        SpellCheckCastVector       mSpellCheckCastVector;
        SpellEnchantProcEventMap   mSpellEnchantProcEventMap;
        EnchantCustomAttribute     mEnchantCustomAttr;
        SpellAreaMap               mSpellAreaMap;
        SpellAreaForQuestMap       mSpellAreaForQuestMap;
        SpellAreaForQuestMap       mSpellAreaForActiveQuestMap;
        SpellAreaForQuestMap       mSpellAreaForQuestEndMap;
        SpellAreaForAuraMap        mSpellAreaForAuraMap;
        SpellAreaForAreaMap        mSpellAreaForAreaMap;
        SkillLineAbilityMap        mSkillLineAbilityMap;
        PetLevelupSpellMap         mPetLevelupSpellMap;
        PetDefaultSpellsMap        mPetDefaultSpellsMap;           // only spells not listed in related mPetLevelupSpellMap entry
        SpellInfoMap               mSpellInfoMap;
        SpellOverrideInfo          mSpellOverrideInfo;
        TalentSpellSet             mTalentSpellInfo;
        PlaySpellVisualMap         mSpellVisualMap;
        SpellVisualPlayOrphanMap   mSpellVisualPlayOrphanMap;
        SpellVisualKitMap          mSpellVisualKitMap;
        SpellPendingCastMap        mSpellPendingCastMap;
        std::list<uint32>          mForbiddenSpells;
        SpellSceneMap              mSpellSceneMap;
        SceneTriggerEventMap       mSceneTriggerEventMap;
        SpellConcatenateApplyMap   mSpellConcatenateApplyMap;
        SpellConcatenateUpdateMap  mSpellConcatenateUpdateMap;
        SpellTriggerDelayMap       mSpellTriggerDelayMap;
        SpellTriggerDelayVector    mSpellTriggerDelayVector;
        std::map<uint32, std::list<SkillLineAbilityEntry const*>> _skillTradeSpells;
};

#define sSpellMgr SpellMgr::instance()

#endif
