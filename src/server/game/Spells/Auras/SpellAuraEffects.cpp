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

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "Battleground.h"
#include "CellImpl.h"
#include "CharacterPackets.h"
#include "Common.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Log.h"
#include "MiscPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "PlayerDefines.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellPackets.h"
#include "Unit.h"
#include "Util.h"
#include "Vehicle.h"
#include "WorldPacket.h"

class Aura;
//
// EFFECT HANDLER NOTES
//
// in aura handler there should be check for modes:
// AURA_EFFECT_HANDLE_REAL set
// AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK set
// AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK set - aura is recalculated or is just applied/removed - need to redo all things related to m_amount
// AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK - logical or of above conditions
// AURA_EFFECT_HANDLE_STAT - set when stats are reapplied
// such checks will speedup trinity change amount/send for client operations
// because for change amount operation packets will not be send
// aura effect handlers shouldn't contain any AuraEffect or Aura object modifications

pAuraEffectHandler AuraEffectHandler[TOTAL_AURAS]=
{
    &AuraEffect::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &AuraEffect::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &AuraEffect::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &AuraEffect::HandleNoImmediateEffect,                         //  3 SPELL_AURA_PERIODIC_DAMAGE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &AuraEffect::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &AuraEffect::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &AuraEffect::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &AuraEffect::HandleNoImmediateEffect,                         //  8 SPELL_AURA_PERIODIC_HEAL implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &AuraEffect::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &AuraEffect::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &AuraEffect::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &AuraEffect::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &AuraEffect::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         // 15 SPELL_AURA_DAMAGE_SHIELD    implemented in Unit::DoAttackDamage
    &AuraEffect::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &AuraEffect::HandleModStealthDetect,                          // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &AuraEffect::HandleModInvisibility,                           // 18 SPELL_AURA_MOD_INVISIBILITY
    &AuraEffect::HandleModInvisibilityDetect,                     // 19 SPELL_AURA_MOD_INVISIBILITY_DETECT
    &AuraEffect::HandleNoImmediateEffect,                         // 20 SPELL_AURA_OBS_MOD_HEALTH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         // 21 SPELL_AURA_OBS_MOD_POWER implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &AuraEffect::HandleNoImmediateEffect,                         // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         // 24 SPELL_AURA_PERIODIC_ENERGIZE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &AuraEffect::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &AuraEffect::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &AuraEffect::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS        implement in Unit::SpellHitResult
    &AuraEffect::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &AuraEffect::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &AuraEffect::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &AuraEffect::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &AuraEffect::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &AuraEffect::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &AuraEffect::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &AuraEffect::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &AuraEffect::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &AuraEffect::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &AuraEffect::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &AuraEffect::HandleNoImmediateEffect,                         // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in Unit::ProcDamageAndSpellFor and Unit::HandleProcTriggerSpell
    &AuraEffect::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in Unit::ProcDamageAndSpellFor
    &AuraEffect::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &AuraEffect::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &AuraEffect::HandleNULL,                                      // 46 SPELL_AURA_46
    &AuraEffect::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &AuraEffect::HandleNULL,                                      // 48 SPELL_AURA_PERIODIC_TRIGGER_SPELL_BY_CLIENT
    &AuraEffect::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         // 50 SPELL_AURA_MOD_CRITICAL_HEALING_AMOUNT implemented in Unit::SpellCriticalHealingBonus
    &AuraEffect::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &AuraEffect::HandleAuraModWeaponCritPercent,                  // 52 SPELL_AURA_MOD_WEAPON_CRIT_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         // 53 SPELL_AURA_PERIODIC_LEECH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &AuraEffect::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &AuraEffect::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &AuraEffect::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &AuraEffect::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &AuraEffect::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &AuraEffect::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &AuraEffect::HandleNoImmediateEffect,                         // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         // 63 SPELL_AURA_MOD_ADDITIONAL_POWER_COST SpellInfo::CalcPowerCost
    &AuraEffect::HandleNoImmediateEffect,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleModCastingSpeed,                           // 65 SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    &AuraEffect::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &AuraEffect::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &AuraEffect::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &AuraEffect::HandleNoImmediateEffect,                         // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleNoImmediateEffect,                         // 70 SPELL_AURA_PERIODIC_WEAPON_PERCENT_DAMAGE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNULL,                                      // 71 SPELL_AURA_71
    &AuraEffect::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &AuraEffect::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &AuraEffect::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL  implemented in Unit::SpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE
    &AuraEffect::HandleNoImmediateEffect,                         // 76 SPELL_AURA_FAR_SIGHT
    &AuraEffect::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &AuraEffect::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &AuraEffect::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &AuraEffect::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &AuraEffect::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &AuraEffect::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &AuraEffect::HandleNoImmediateEffect,                         // 84 SPELL_AURA_MOD_REGEN implemented in Player::RegenerateHealth
    &AuraEffect::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN implemented in Player::Regenerate
    &AuraEffect::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &AuraEffect::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT implemented in Player::RegenerateHealth
    &AuraEffect::HandleNoImmediateEffect,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &AuraEffect::HandleEnablePvpStatsScaling,                     // 90 SPELL_AURA_ENABLE_PVP_STAT_SCALING
    &AuraEffect::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_DETECT_RANGE implemented in Creature::GetAttackDistance
    &AuraEffect::HandlePreventFleeing,                            // 92 SPELL_AURA_PREVENTS_FLEEING
    &AuraEffect::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &AuraEffect::HandleNoImmediateEffect,                         // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::Regenerate
    &AuraEffect::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &AuraEffect::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Unit::SelectMagnetTarget
    &AuraEffect::HandleNoImmediateEffect,                         // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &AuraEffect::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &AuraEffect::HandleUnused,                                    //100 SPELL_AURA_AURAS_VISIBLE obsolete? all player can see all auras now, but still have spells including GM-spell
    &AuraEffect::HandleModResistancePercent,                      //101 SPELL_AURA_MOD_RESISTANCE_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModTotalThreat,                        //103 SPELL_AURA_MOD_TOTAL_THREAT
    &AuraEffect::HandleAuraWaterWalk,                             //104 SPELL_AURA_WATER_WALK
    &AuraEffect::HandleAuraFeatherFall,                           //105 SPELL_AURA_FEATHER_FALL
    &AuraEffect::HandleAuraHover,                                 //106 SPELL_AURA_HOVER
    &AuraEffect::HandleNoImmediateEffect,                         //107 SPELL_AURA_ADD_FLAT_MODIFIER implemented in AuraEffect::CalculateSpellMod()
    &AuraEffect::HandleNoImmediateEffect,                         //108 SPELL_AURA_ADD_PCT_MODIFIER implemented in AuraEffect::CalculateSpellMod()
    &AuraEffect::HandleNoImmediateEffect,                         //109 SPELL_AURA_ADD_TARGET_TRIGGER
    &AuraEffect::HandleModPowerRegenPCT,                          //110 SPELL_AURA_MOD_POWER_REGEN_PERCENT implemented in Player::Regenerate, Creature::Regenerate
    &AuraEffect::HandleNoImmediateEffect,                         //111 SPELL_AURA_INTERCEPT_MELEE_RANGED_ATTACKS
    &AuraEffect::HandleNoImmediateEffect,                         //112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &AuraEffect::HandleNoImmediateEffect,                         //113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //115 SPELL_AURA_MOD_HEALING                 implemented in Unit::SpellBaseHealingBonusTaken
    &AuraEffect::HandleNoImmediateEffect,                         //116 SPELL_AURA_MOD_REGEN_DURING_COMBAT
    &AuraEffect::HandleNoImmediateEffect,                         //117 SPELL_AURA_MOD_MECHANIC_RESISTANCE     implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //118 SPELL_AURA_MOD_HEALING_PCT             implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleAuraPvpTalents,                            //119 SPELL_AURA_PVP_TALENTS
    &AuraEffect::HandleAuraUntrackable,                           //120 SPELL_AURA_UNTRACKABLE
    &AuraEffect::HandleAuraEmpathy,                               //121 SPELL_AURA_EMPATHY
    &AuraEffect::HandleModOffhandDamagePercent,                   //122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &AuraEffect::HandleModTargetResistance,                       //123 SPELL_AURA_MOD_TARGET_RESISTANCE
    &AuraEffect::HandleAuraModRangedAttackPower,                  //124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &AuraEffect::HandleNoImmediateEffect,                         //125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleFixate,                                    //128 SPELL_AURA_FIXATE
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //129 SPELL_AURA_MOD_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &AuraEffect::HandleNoImmediateEffect,                         //131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModIncreaseEnergyPercent,              //132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &AuraEffect::HandleAuraModIncreaseHealthPercent,              //133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &AuraEffect::HandleAuraModRegenInterrupt,                     //134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &AuraEffect::HandleModHealingDone,                            //135 SPELL_AURA_MOD_HEALING_DONE
    &AuraEffect::HandleNoImmediateEffect,                         //136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleModTotalPercentStat,                       //137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &AuraEffect::HandleModMeleeSpeedPct,                          //138 SPELL_AURA_MOD_MELEE_HASTE
    &AuraEffect::HandleForceReaction,                             //139 SPELL_AURA_FORCE_REACTION
    &AuraEffect::HandleAuraModRangedHaste,                        //140 SPELL_AURA_MOD_RANGED_HASTE
    &AuraEffect::HandleNULL,                                      //141 SPELL_AURA_141
    &AuraEffect::HandleAuraModBaseResistancePCT,                  //142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &AuraEffect::HandleAuraModCooldownSpeedRate,                  //143 SPELL_AURA_MOD_COOLDOWN_SPEED_RATE
    &AuraEffect::HandleNoImmediateEffect,                         //144 SPELL_AURA_SAFE_FALL                         implemented in WorldSession::HandleMovementOpcodes
    &AuraEffect::HandleNULL,                                      //145 SPELL_AURA_MOD_MAX_HEALTH_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //146 SPELL_AURA_ALLOW_TAME_PET_TYPE
    &AuraEffect::HandleModStateImmunityMask,                      //147 SPELL_AURA_MECHANIC_IMMUNITY_MASK
    &AuraEffect::HandleNULL,                                      //148 SPELL_AURA_148
    &AuraEffect::HandleNoImmediateEffect,                         //149 SPELL_AURA_REDUCE_PUSHBACK
    &AuraEffect::HandleShieldBlockValue,                          //150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &AuraEffect::HandleAuraTrackStealthed,                        //151 SPELL_AURA_TRACK_STEALTHED
    &AuraEffect::HandleNoImmediateEffect,                         //152 SPELL_AURA_MOD_DETECTED_RANGE implemented in Creature::GetAttackDistance
    &AuraEffect::HandleNULL,                                      //153 SPELL_AURA_MOD_AUTO_ATTACK_RANGE
    &AuraEffect::HandleModStealthLevel,                           //154 SPELL_AURA_MOD_STEALTH_LEVEL
    &AuraEffect::HandleNoImmediateEffect,                         //155 SPELL_AURA_MOD_WATER_BREATHING
    &AuraEffect::HandleNoImmediateEffect,                         //156 SPELL_AURA_MOD_REPUTATION_GAIN
    &AuraEffect::HandleNoImmediateEffect,                         //157 SPELL_AURA_PET_DAMAGE_MULTI
    &AuraEffect::HandleAllowTalentSwapping,                       //158 SPELL_AURA_ALLOW_TALENT_SWAPPING
    &AuraEffect::HandleNoImmediateEffect,                         //159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &AuraEffect::HandleUnused,                                    //160 SPELL_AURA_160
    &AuraEffect::HandleNoImmediateEffect,                         //161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &AuraEffect::HandleNoImmediateEffect,                         //162 SPELL_AURA_POWER_BURN implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS
    &AuraEffect::HandleNULL,                                      //164 SPELL_AURA_CANT_BREATH
    &AuraEffect::HandleNoImmediateEffect,                         //165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModAttackPowerPercent,                 //166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &AuraEffect::HandleAuraModRangedAttackPowerPercent,           //167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS            implemented in Unit::SpellDamageBonus, Unit::MeleeDamageBonus
    &AuraEffect::HandleNULL,                                      //169 SPELL_AURA_SET_FFA_PVP_STATE
    &AuraEffect::HandleNULL,                                      //170 SPELL_AURA_DETECT_AMORE       various spells that change visual of units for aura target (clientside?)
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &AuraEffect::HandleAuraModCooldownPct,                        //173 SPELL_AURA_MOD_COOLDOWN_PCT
    &AuraEffect::HandleModSpellDamagePercentFromStat,             //174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonus
    &AuraEffect::HandleModSpellHealingPercentFromStat,            //175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonus
    &AuraEffect::HandleSpiritOfRedemption,                        //176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &AuraEffect::HandleCharmConvert,                              //177 SPELL_AURA_AOE_CHARM
    &AuraEffect::HandleAuraModMaxManaPercent,                     //178 SPELL_AURA_MOD_MAX_MANA
    &AuraEffect::HandleNoImmediateEffect,                         //179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalBonus
    &AuraEffect::HandleNoImmediateEffect,                         //180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS   implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleNULL,                                      //181 SPELL_AURA_MOD_SHIPMENT_RESOURCE_COST
    &AuraEffect::HandleNULL,                                      //182 SPELL_AURA_DISABLE_LEGENDARY_ITEMS_PASSIVE_EFFECTS
    &AuraEffect::HandleNoImmediateEffect,                         //183 SPELL_AURA_MOD_CRITICAL_DAMAGE
    &AuraEffect::HandleNoImmediateEffect,                         //184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemented in Unit::GetUnitCriticalChance
    &AuraEffect::HandleNoImmediateEffect,                         //188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::GetUnitCriticalChance
    &AuraEffect::HandleModRating,                                 //189 SPELL_AURA_MOD_RATING
    &AuraEffect::HandleNoImmediateEffect,                         //190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN     implemented in Player::CalculateReputationGain
    &AuraEffect::HandleAuraModUseNormalSpeed,                     //191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
    &AuraEffect::HandleModMeleeRangedSpeedPct,                    //192 SPELL_AURA_MOD_MELEE_RANGED_HASTE
    &AuraEffect::HandleModCombatSpeedPct,                         //193 SPELL_AURA_MELEE_SLOW (in fact combat (any type attack) speed pct)
    &AuraEffect::HandleNoImmediateEffect,                         //194 SPELL_AURA_MOD_TARGET_ABSORB_SCHOOL implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleNULL,                                      //195 SPELL_AURA_195
    &AuraEffect::HandleNoImmediateEffect,                         //196 SPELL_AURA_MOD_COOLDOWN - flat mod of spell cooldowns
    &AuraEffect::HandleNoImmediateEffect,                         //197 SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE implemented in Unit::SpellCriticalBonus Unit::GetUnitCriticalChance
    &AuraEffect::HandleNULL,                                      //198 SPELL_AURA_MOD_DODGE_CHANCE_FROM_CRIT_CHANCE_BY_MASK
    &AuraEffect::HandleNULL,                                      //199 SPELL_AURA_199
    &AuraEffect::HandleNoImmediateEffect,                         //200 SPELL_AURA_MOD_XP_PCT implemented in Player::RewardPlayerAndGroupAtKill
    &AuraEffect::HandleAuraAllowFlight,                           //201 SPELL_AURA_FLY                             this aura enable flight mode...
    &AuraEffect::HandleNULL,                                      //202 SPELL_AURA_202
    &AuraEffect::HandleNULL,                                      //203 SPELL_AURA_PREVENT_INTERRUPT
    &AuraEffect::HandleNULL,                                      //204 SPELL_AURA_PREVENT_CORPSE_RELEASE
    &AuraEffect::HandleNULL,                                      //205 SPELL_AURA_MOD_ABILITY_COOLDOWN
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //206 SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //207 SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //208 SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //209 SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //210 SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //211 SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK
    &AuraEffect::HandleNoImmediateEffect,                         //212 SPELL_AURA_MOD_HONOR_POINTS_GAIN_BY_TYPE_MASK
    &AuraEffect::HandleNoImmediateEffect,                         //213 SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT implemented in Player::RewardRage
    &AuraEffect::HandleNULL,                                      //214 Tamed Pet Passive
    &AuraEffect::HandleArenaPreparation,                          //215 SPELL_AURA_ARENA_PREPARATION
    &AuraEffect::HandleModCastingSpeed,                           //216 SPELL_AURA_HASTE_SPELLS
    &AuraEffect::HandleModMeleeSpeedPct,                          //217 SPELL_AURA_MOD_MELEE_HASTE_2
    &AuraEffect::HandleNULL,                                      //218 SPELL_AURA_MOD_ITEM_STATS_AND_PROCK_BY_PCT_WITH_MASK
    &AuraEffect::HandleNULL,                                      //219 SPELL_AURA_219
    &AuraEffect::HandleNULL,                                      //220 SPELL_AURA_220
    &AuraEffect::HandleNULL,                                      //221 SPELL_AURA_MOD_DETAUNT
    &AuraEffect::HandleNULL,                                      //222 SPELL_AURA_222
    &AuraEffect::HandleNULL,                                      //223 SPELL_AURA_223
    &AuraEffect::HandleEnableExtraTalent,                         //224 SPELL_AURA_ENABLE_EXTRA_TALENT
    &AuraEffect::HandleNoImmediateEffect,                         //225 SPELL_AURA_MOD_VISIBILITY_RANGE
    &AuraEffect::HandleNoImmediateEffect,                         //226 SPELL_AURA_PERIODIC_DUMMY implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //227 SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //228 SPELL_AURA_DETECT_STEALTH stealth detection
    &AuraEffect::HandleNoImmediateEffect,                         //229 SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE
    &AuraEffect::HandleAuraModIncreaseHealth,                     //230 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &AuraEffect::HandleNoImmediateEffect,                         //231 SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE
    &AuraEffect::HandleNoImmediateEffect,                         //232 SPELL_AURA_MECHANIC_DURATION_MOD           implement in Unit::CalculateSpellDuration
    &AuraEffect::HandleUnused,                                    //233 set model id to the one of the creature with id GetMiscValue() - clientside
    &AuraEffect::HandleNoImmediateEffect,                         //234 SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK implement in Unit::CalculateSpellDuration
    &AuraEffect::HandleUnused,                                    //235 SPELL_AURA_235
    &AuraEffect::HandleAuraControlVehicle,                        //236 SPELL_AURA_CONTROL_VEHICLE
    &AuraEffect::HandleUnused,                                    //237 SPELL_AURA_237
    &AuraEffect::HandleUnused,                                    //238 SPELL_AURA_238
    &AuraEffect::HandleAuraModScale,                              //239 SPELL_AURA_MOD_SCALE_2 only in Noggenfogger Elixir (16595) before 2.3.0 aura 61
    &AuraEffect::HandleAuraModExpertise,                          //240 SPELL_AURA_MOD_EXPERTISE
    &AuraEffect::HandleForceMoveForward,                          //241 SPELL_AURA_FORCE_MOVE_FORWARD Forces the caster to move forward
    &AuraEffect::HandleNULL,                                      //242 SPELL_AURA_242
    &AuraEffect::HandleAuraModFaction,                            //243 SPELL_AURA_MOD_FACTION
    &AuraEffect::HandleComprehendLanguage,                        //244 SPELL_AURA_COMPREHEND_LANGUAGE
    &AuraEffect::HandleNoImmediateEffect,                         //245 SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL
    &AuraEffect::HandleUnused,                                    //246 SPELL_AURA_246
    &AuraEffect::HandleAuraCloneCaster,                           //247 SPELL_AURA_CLONE_CASTER
    &AuraEffect::HandleUnused,                                    //248 SPELL_AURA_248
    &AuraEffect::HandleNoImmediateEffect,                         //249 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE_BY_TARGET_AURA_MECHANIC implemented in Unit::SpellDamageBonus 
    &AuraEffect::HandleAuraModIncreaseHealthOrPercent,            //250 SPELL_AURA_MOD_INCREASE_HEALTH_OR_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //251 SPELL_AURA_MOD_ENEMY_DODGE
    &AuraEffect::HandleModCombatSpeedPct,                         //252 SPELL_AURA_252 Is there any difference between this and SPELL_AURA_MELEE_SLOW ? maybe not stacking mod?
    &AuraEffect::HandleNoImmediateEffect,                         //253 SPELL_AURA_MOD_BLOCK_CRIT_CHANCE  implemented in Unit::isBlockCritical
    &AuraEffect::HandleAuraModDisarm,                             //254 SPELL_AURA_MOD_DISARM_OFFHAND
    &AuraEffect::HandleNoImmediateEffect,                         //255 SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT    implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleNULL,                                      //256 SPELL_AURA_256
    &AuraEffect::HandleUnused,                                    //257 SPELL_AURA_257
    &AuraEffect::HandleNoImmediateEffect,                         //258 SPELL_AURA_OVERRIDE_SUMMONED_OBJECT
    &AuraEffect::HandleNoImmediateEffect,                         //259 SPELL_AURA_MOD_HOT_PCT implemented in Unit::SpellHealingBonusTaken
    &AuraEffect::HandleNoImmediateEffect,                         //260 SPELL_AURA_SCREEN_EFFECT (miscvalue = id in ScreenEffect.dbc) not required any code
    &AuraEffect::HandlePhase,                                     //261 SPELL_AURA_PHASE
    &AuraEffect::HandleNoImmediateEffect,                         //262 SPELL_AURA_ABILITY_IGNORE_AURASTATE implemented in spell::cancast
    &AuraEffect::HandleAuraAllowOnlyAbility,                      //263 SPELL_AURA_ALLOW_ONLY_ABILITY player can use only abilities set in SpellClassMask
    &AuraEffect::HandleNoImmediateEffect,                         //264 SPELL_AURA_DISABLE_AUTO_ATTACK
    &AuraEffect::HandleUnused,                                    //265 unused
    &AuraEffect::HandleUnused,                                    //266 unused
    &AuraEffect::HandleNoImmediateEffect,                         //267 SPELL_AURA_MOD_IMMUNE_AURA_APPLY_SCHOOL         implemented in Unit::IsImmunedToSpellEffect
    &AuraEffect::HandleUnused,                                    //268 SPELL_AURA_268
    &AuraEffect::HandleNoImmediateEffect,                         //269 SPELL_AURA_MOD_DAMAGE_TAKEN_FROM_CASTER implemented in Unit::SpellDamageBonusTaken
    &AuraEffect::HandleNULL,                                      //270 SPELL_AURA_INCREASE_SCHOOL_DAMAGE_TAKEN
    &AuraEffect::HandleNoImmediateEffect,                         //271 SPELL_AURA_MOD_DAMAGE_FROM_CASTER    implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //272 SPELL_AURA_272
    &AuraEffect::HandleUnused,                                    //273 SPELL_AURA_X_RAY
    &AuraEffect::HandleUnused,                                    //274 SPELL_AURA_274
    &AuraEffect::HandleNoImmediateEffect,                         //275 SPELL_AURA_MOD_IGNORE_SHAPESHIFT Use SpellClassMask for spell select
    &AuraEffect::HandleNoImmediateEffect,                         //276 SPELL_AURA_MOD_DAMAGE_DONE_FOR_MECHANIC
    &AuraEffect::HandleUnused,                                    //277 SPELL_AURA_277
    &AuraEffect::HandleAuraModDisarm,                             //278 SPELL_AURA_MOD_DISARM_RANGED disarm ranged weapon
    &AuraEffect::HandleAuraInitializeImages,                      //279 SPELL_AURA_INITIALIZE_IMAGES
    &AuraEffect::HandleNoImmediateEffect,                         //280 SPELL_AURA_MOD_ARMOR_PENETRATION_PCT implemented in Unit::CalcArmorReducedDamage
    &AuraEffect::HandleNoImmediateEffect,                         //281 SPELL_AURA_MOD_REPUTATION_GAIN_PCT implemented in Player::RewardHonor
    &AuraEffect::HandleNULL,                                      //282 SPELL_AURA_282
    &AuraEffect::HandleNoImmediateEffect,                         //283 SPELL_AURA_MOD_HEALING_RECEIVED       implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleAuraLinked,                                //284 SPELL_AURA_LINKED
    &AuraEffect::HandleUnused,                                    //285 SPELL_AURA_285
    &AuraEffect::HandleNULL,                                      //286 SPELL_AURA_286
    &AuraEffect::HandleNoImmediateEffect,                         //287 SPELL_AURA_DEFLECT_SPELLS             implemented in Unit::MagicSpellHitResult and Unit::MeleeSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //288 SPELL_AURA_IGNORE_HIT_DIRECTION  implemented in Unit::MagicSpellHitResult and Unit::MeleeSpellHitResult Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //289 SPELL_AURA_PREVENT_DURABILITY_LOSS
    &AuraEffect::HandleAuraModCritPct,                            //290 SPELL_AURA_MOD_CRIT_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //291 SPELL_AURA_MOD_XP_QUEST_PCT  implemented in Player::RewardQuest
    &AuraEffect::HandleNULL,                                      //292 SPELL_AURA_292
    &AuraEffect::HandleAuraOverrideSpells,                        //293 SPELL_AURA_OVERRIDE_SPELLS
    &AuraEffect::HandleNoImmediateEffect,                         //294 SPELL_AURA_PREVENT_REGENERATE_POWER implemented in Player::Regenerate(Powers power)
    &AuraEffect::HandleUnused,                                    //295 SPELL_AURA_295
    &AuraEffect::HandleAuraSetVehicle,                            //296 SPELL_AURA_SET_VEHICLE_ID sets vehicle on target
    &AuraEffect::HandleNoImmediateEffect,                         //297 SPELL_AURA_BLOCK_SPELL_FAMILY
    &AuraEffect::HandleAuraStrangulate,                           //298 SPELL_AURA_STRANGULATE 70569 - Strangulating, maybe prevents talk or cast
    &AuraEffect::HandleUnused,                                    //299 SPELL_AURA_299
    &AuraEffect::HandleNoImmediateEffect,                         //300 SPELL_AURA_SHARE_DAMAGE_PCT implemented in Unit::DealDamage
    &AuraEffect::HandleNoImmediateEffect,                         //301 SPELL_AURA_SCHOOL_HEAL_ABSORB implemented in Unit::CalcHealAbsorb
    &AuraEffect::HandleUnused,                                    //302 SPELL_AURA_302
    &AuraEffect::HandleNoImmediateEffect,                         //303 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE implemented in Unit::SpellDamageBonus, Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModFakeInebriation,                    //304 SPELL_AURA_MOD_DRUNK
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //305 SPELL_AURA_MOD_MINIMUM_SPEED
    &AuraEffect::HandleUnused,                                    //306 SPELL_AURA_306
    &AuraEffect::HandleUnused,                                    //307 SPELL_AURA_HEAL_ABSORB_TEST
    &AuraEffect::HandleNoImmediateEffect,                         //308 SPELL_AURA_MOD_CRIT_CHANCE_FOR_CASTER
    &AuraEffect::HandleAuraModResiliencePct,                      //309 SPELL_AURA_MOD_RESILIENCE_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //310 SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE implemented in Spell::CalculateDamageDone
    &AuraEffect::HandleNULL,                                      //311 SPELL_AURA_311
    &AuraEffect::HandleAnimReplacementSet,                        //312 SPELL_AURA_ANIM_REPLACEMENT_SET
    &AuraEffect::HandleNULL,                                      //313 SPELL_AURA_313
    &AuraEffect::HandlePreventResurrection,                       //314 SPELL_AURA_PREVENT_RESURRECTION
    &AuraEffect::HandleNULL,                                      //315 SPELL_AURA_UNDERWATER_WALKING
    &AuraEffect::HandleUnused,                                    //316 SPELL_AURA_316
    &AuraEffect::HandleAuraModSpellPowerPercent,                  //317 SPELL_AURA_MOD_SPELL_POWER_PCT
    &AuraEffect::HandleAuraMastery,                               //318 SPELL_AURA_MASTERY
    &AuraEffect::HandleModMeleeSpeedPct,                          //319 SPELL_AURA_MOD_MELEE_ATTACK_SPEED
    &AuraEffect::HandleAuraModRangedHaste,                        //320 SPELL_AURA_MOD_RANGED_HASTE_3
    &AuraEffect::HandleAuraModNoActions,                          //321 SPELL_AURA_MOD_NO_ACTIONS
    &AuraEffect::HandleNoImmediateEffect,                         //322 SPELL_AURA_INTERFERE_TARGETTING
    &AuraEffect::HandleNULL,                                      //323 SPELL_AURA_MOD_CRITICAL_DAMAGE_FROM_MEELE_SPELLS
    &AuraEffect::HandleUnused,                                    //324 SPELL_AURA_324
    &AuraEffect::HandleUnused,                                    //325 SPELL_AURA_325
    &AuraEffect::HandlePhase,                                     //326 SPELL_AURA_PHASE_2
    &AuraEffect::HandleNULL,                                      //327 SPELL_AURA_FORCE_TO_SEE_ALL_PHASES
    &AuraEffect::HandleNoImmediateEffect,                         //328 SPELL_AURA_PROC_ON_POWER_PCT implemented in Unit::VisualForPower
    &AuraEffect::HandleNoImmediateEffect,                         //329 SPELL_AURA_MOD_POWER_GENERATE_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //330 SPELL_AURA_CAST_WHILE_WALKING
    &AuraEffect::HandleAuraForceWeather,                          //331 SPELL_AURA_FORCE_WEATHER
    &AuraEffect::HandleOverrideActionbarSpells,                   //332 SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS implemented in WorldSession::HandleCastSpellOpcode
    &AuraEffect::HandleOverrideActionbarSpells,                   //333 SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS_2 implemented in WorldSession::HandleCastSpellOpcode
    &AuraEffect::HandleNULL,                                      //334 SPELL_AURA_MOD_BLIND
    &AuraEffect::HandleAuraSeeWhileInvisible,                     //335 SPELL_AURA_SEE_WHILE_INVISIBLE
    &AuraEffect::HandleNoImmediateEffect,                         //336 SPELL_AURA_MOD_FLYING_RESTRICTIONS
    &AuraEffect::HandleNoImmediateEffect,                         //337 SPELL_AURA_MOD_VENDOR_ITEMS_PRICES
    &AuraEffect::HandleNoImmediateEffect,                         //338 SPELL_AURA_MOD_DURABILITY_LOSS
    &AuraEffect::HandleNoImmediateEffect,                         //339 SPELL_AURA_INCREASE_SKILL_GAIN_CHANCE
    &AuraEffect::HandleNoImmediateEffect,                         //340 SPELL_AURA_MOD_RESURRECTED_HEALTH_BY_GUILD_MEMBER
    &AuraEffect::HandleAuraModCategoryCooldown,                   //341 SPELL_AURA_MOD_SPELL_CATEGORY_COOLDOWN
    &AuraEffect::HandleModMeleeRangedSpeedPct,                    //342 SPELL_AURA_MOD_MELEE_AND_RANGED_ATTACK_SPEED_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //343 SPELL_AURA_MOD_AUTOATTACK_DAMAGE_TARGET
    &AuraEffect::HandleNoImmediateEffect,                         //344 SPELL_AURA_MOD_AUTOATTACK_DAMAGE
    &AuraEffect::HandleNoImmediateEffect,                         //345 SPELL_AURA_BYPASS_ARMOR_FOR_CASTER
    &AuraEffect::HandleProgressBar,                               //346 SPELL_AURA_ENABLE_ALT_POWER
    &AuraEffect::HandleNoImmediateEffect,                         //347 SPELL_AURA_MOD_SPELL_COOLDOWN_BY_HASTE
    &AuraEffect::HandleNULL,                                      //348 SPELL_AURA_348
    &AuraEffect::HandleNoImmediateEffect,                         //349 SPELL_AURA_MOD_CURRENCY_GAIN implemented in Player::ModifyCurrency (TODO?)
    &AuraEffect::HandleNULL,                                      //350 SPELL_AURA_350
    &AuraEffect::HandleNoImmediateEffect,                         //351 SPELL_AURA_MOD_CURRENCY_LOOT
    &AuraEffect::HandleNULL,                                      //352 SPELL_AURA_ALLOW_WORGEN_TRANSFORM
    &AuraEffect::HandleNULL,                                      //353 SPELL_AURA_353
    &AuraEffect::HandleNoImmediateEffect,                         //354 SPELL_AURA_MOD_HEALING_DONE_FROM_PCT_HEALTH
    &AuraEffect::HandleModCastingSpeed,                           //355 SPELL_AURA_MOD_CASTING_SPEED
    &AuraEffect::HandleNULL,                                      //356 SPELL_AURA_356
    &AuraEffect::HandleNULL,                                      //357 SPELL_AURA_ENABLE_BOSS1_UNIT_FRAME
    &AuraEffect::HandleNULL,                                      //358 SPELL_AURA_WORGEN_ALTERED_FORM
    &AuraEffect::HandleNoImmediateEffect,                         //359 SPELL_AURA_MOD_HEALING_DONE2 implemented in Unit::SpellBaseHealingBonusDone
    &AuraEffect::HandleNoImmediateEffect,                         //360 SPELL_AURA_PROC_TRIGGER_SPELL_COPY
    &AuraEffect::HandleNoImmediateEffect,                         //361 SPELL_AURA_PROC_MELEE_TRIGGER_SPELL
    &AuraEffect::HandleUnused,                                    //362 SPELL_AURA_DUMMY2
    &AuraEffect::HandleModNextSpell,                              //363 SPELL_AURA_MOD_NEXT_SPELL
    &AuraEffect::HandleUnused,                                    //364 SPELL_AURA_364
    &AuraEffect::HandleNULL,                                      //365 SPELL_AURA_MAX_FAR_CLIP_PLANE
    &AuraEffect::HandleOverrideSpellPowerByAttackPower,           //366 SPELL_AURA_OVERRIDE_SPELL_POWER_BY_AP_PCT  implemented in Unit::SpellBaseDamageBonus
    &AuraEffect::HandleOverrideAutoattack,                        //367 SPELL_AURA_OVERRIDE_AUTOATTACK
    &AuraEffect::HandleUnused,                                    //368 SPELL_AURA_MOD_DISARM2
    &AuraEffect::HandleNULL,                                      //369 SPELL_AURA_ENABLE_POWER_BAR_TIMER
    &AuraEffect::HandleNULL,                                      //370 SPELL_AURA_370
    &AuraEffect::HandleNoImmediateEffect,                         //371 SPELL_AURA_DISABLE_ATTACK_AND_CAST
    &AuraEffect::HandleAuraMounted,                               //372 SPELL_AURA_MOUNTED_VISUAL
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //373 SPELL_AURA_MOD_SPEED_NO_CONTROL
    &AuraEffect::HandleNoImmediateEffect,                         //374 SPELL_AURA_MOD_FALL_DAMAGE
    &AuraEffect::HandleUnused,                                    //375 SPELL_AURA_375
    &AuraEffect::HandleNULL,                                      //376 SPELL_AURA_376
    &AuraEffect::HandleNoImmediateEffect,                         //377 SPELL_AURA_CAST_WHILE_WALKING2
    &AuraEffect::HandleUnused,                                    //378 SPELL_AURA_378
    &AuraEffect::HandleModManaRegen,                              //379 SPELL_AURA_MOD_MANA_REGEN_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //380 SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_HASTE
    &AuraEffect::HandleNULL,                                      //381 SPELL_AURA_381
    &AuraEffect::HandleAuraModPetStatsModifier,                   //382 SPELL_AURA_MOD_PET_STATS_MODIFIER
    &AuraEffect::HandleNoImmediateEffect,                         //383 SPELL_AURA_IGNORE_CD implemented in Spell::CheckCast
    &AuraEffect::HandleUnused,                                    //384 SPELL_AURA_384
    &AuraEffect::HandleNULL,                                      //385 SPELL_AURA_385
    &AuraEffect::HandleNULL,                                      //386 SPELL_AURA_386
    &AuraEffect::HandleNULL,                                      //387 SPELL_AURA_387
    &AuraEffect::HandleNoImmediateEffect,                         //388 SPELL_AURA_MOD_FLY_PATH_SPEED
    &AuraEffect::HandleNULL,                                      //389 SPELL_AURA_389
    &AuraEffect::HandleUnused,                                    //390 SPELL_AURA_390
    &AuraEffect::HandleUnused,                                    //391 SPELL_AURA_391
    &AuraEffect::HandleUnused,                                    //392 SPELL_AURA_392
    &AuraEffect::HandleNoImmediateEffect,                         //393 SPELL_AURA_MOD_DEFLECT_SPELLS_FROM_FRONT
    &AuraEffect::HandleShowConfirmationPrompt,                    //394 SPELL_AURA_SHOW_CONFIRMATION_PROMPT
    &AuraEffect::HandleCreateAreaTrigger,                         //395 SPELL_AURA_CREATE_AREATRIGGER
    &AuraEffect::HandleNoImmediateEffect,                         //396 SPELL_AURA_PROC_ON_POWER_AMOUNT in Unit::VisualForPower
    &AuraEffect::HandleBattlegroundFlag,                          //397 SPELL_AURA_BATTLEGROUND_FLAG
    &AuraEffect::HandleBattlegroundFlag,                          //398 SPELL_AURA_BATTLEGROUND_FLAG_2
    &AuraEffect::HandleModTimeRate,                               //399 SPELL_AURA_MOD_TIME_RATE
    &AuraEffect::HandleAuraModSkill,                              //400 SPELL_AURA_MOD_SKILL_2
    &AuraEffect::HandleNULL,                                      //401 SPELL_AURA_CART_AURA
    &AuraEffect::HandleAuraeEablePowerType,                       //402 SPELL_AURA_ENABLE_POWER_TYPE
    &AuraEffect::HandleNoImmediateEffect,                         //403 SPELL_AURA_OVERRIDE_SPELL_VISUAL
    &AuraEffect::HandleOverrideAttackPowerBySpellPower,           //404 SPELL_AURA_OVERRIDE_AP_BY_SPELL_POWER_PCT
    &AuraEffect::HandleIncreaseModRatingPct,                      //405 SPELL_AURA_MOD_RATING_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //406 SPELL_AURA_OVERRIDE_CLIENT_CONTROLS
    &AuraEffect::HandleModFear,                                   //407 SPELL_AURA_MOD_FEAR_2
    &AuraEffect::HandleNULL,                                      //408 SPELL_AURA_PROC_SPELL_CHARGE removed by spell defined in EffectTriggerSpell
    &AuraEffect::HandleAuraCanTurnWhileFalling,                   //409 SPELL_AURA_CAN_TURN_WHILE_FALLING
    &AuraEffect::HandleNULL,                                      //410 SPELL_AURA_MOD_EXTRA_PET_DAMAGE
    &AuraEffect::HandleAuraModCharges,                            //411 SPELL_AURA_MOD_MAX_CHARGES
    &AuraEffect::HandleModPowerRegen,                             //412 SPELL_AURA_HASTE_AFFECTS_BASE_MANA_REGEN
    &AuraEffect::HandleAuraModParryPercent,                       //413 SPELL_AURA_MOD_PARRY_PERCENT2
    &AuraEffect::HandleNULL,                                      //414 SPELL_AURA_MOD_DEFLECT_RANGED_ATTACKS
    &AuraEffect::HandleNULL,                                      //415 SPELL_AURA_415
    &AuraEffect::HandleNoImmediateEffect,                         //416 SPELL_AURA_MOD_COOLDOWN_BY_HASTE_REGEN
    &AuraEffect::HandleNoImmediateEffect,                         //417 SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_HASTE_REGEN
    &AuraEffect::HandleAuraModMaxPower,                           //418 SPELL_AURA_MOD_MAX_POWER
    &AuraEffect::HandleAuraModAddEnergyPercent,                   //419 SPELL_AURA_MOD_ADD_ENERGY_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //420 SPELL_AURA_MOD_BATTLE_PET_XP_GAIN
    &AuraEffect::HandleNoImmediateEffect,                         //421 SPELL_AURA_MOD_ABSORB_AMOUNT
    &AuraEffect::HandleNoImmediateEffect,                         //422 SPELL_AURA_MOD_ABSORBTION_PERCENT
    &AuraEffect::HandleNULL,                                      //423 SPELL_AURA_423
    &AuraEffect::HandleNULL,                                      //424 SPELL_AURA_424
    &AuraEffect::HandleUnused,                                    //425 SPELL_AURA_425
    &AuraEffect::HandleNULL,                                      //426 SPELL_AURA_426
    &AuraEffect::HandleNULL,                                      //427 SPELL_AURA_ITEMS_SCALING_AURA
    &AuraEffect::HandleSummonController,                          //428 SPELL_AURA_SUMMON_CONTROLLER
    &AuraEffect::HandleNoImmediateEffect,                         //429 SPELL_AURA_PET_DAMAGE_DONE_PCT implemented in Unit::SpellBaseDamageBonus Unit::MeleeDamageBonusDone
    &AuraEffect::HandleAuraActivateScene,                         //430 SPELL_AURA_ACTIVATE_SCENE
    &AuraEffect::HandleAuraContestedPvP,                          //431 SPELL_AURA_CONTESTED_PVP
    &AuraEffect::HandleNULL,                                      //432 SPELL_AURA_432
    &AuraEffect::HandleNULL,                                      //433 SPELL_AURA_433
    &AuraEffect::HandleNULL,                                      //434 SPELL_AURA_434
    &AuraEffect::HandleUnused,                                    //435 SPELL_AURA_435
    &AuraEffect::HandleNoImmediateEffect,                         //436 SPELL_AURA_MOD_ENVIRONMENTAL_DAMAGE_TAKEN
    &AuraEffect::HandleAuraModMinimumSpeedRate,                   //437 SPELL_AURA_MOD_MINIMUM_SPEED_RATE
    &AuraEffect::HandleNULL,                                      //438 SPELL_AURA_PRELOAD_PHASE
    &AuraEffect::HandleUnused,                                    //439
    &AuraEffect::HandleNULL,                                      //440 SPELL_AURA_440
    &AuraEffect::HandleNULL,                                      //441 SPELL_AURA_441
    &AuraEffect::HandleNULL,                                      //442 SPELL_AURA_442
    &AuraEffect::HandleModLifeStealPct,                           //443 SPELL_AURA_MOD_LIFE_STEAL_PCT
    &AuraEffect::HandleUnused,                                    //444
    &AuraEffect::HandleUnused,                                    //445
    &AuraEffect::HandleNULL,                                      //446
    &AuraEffect::HandleNoImmediateEffect,                         //447 SPELL_AURA_MOD_XP_FROM_CREATURE_TYPE
    &AuraEffect::HandleNoImmediateEffect,                         //448 SPELL_AURA_MOD_CRITICAL_HEAl_PCT
    &AuraEffect::HandleUnused,                                    //449 SPELL_AURA_449
    &AuraEffect::HandleNULL,                                      //450 SPELL_AURA_UPGRADE_CHARACTER_SPELL_TIER
    &AuraEffect::HandleOverridePetSpecs,                          //451 SPELL_AURA_OVERRIDE_PET_SPECS
    &AuraEffect::HandleUnused,                                    //452 SPELL_AURA_452
    &AuraEffect::HandleAuraModChargeRecoveryMod,                  //453 SPELL_AURA_CHARGE_RECOVERY_MOD
    &AuraEffect::HandleAuraModChargeRecoveryMod,                  //454 SPELL_AURA_CHARGE_RECOVERY_MULTIPLIER
    &AuraEffect::HandleAuraModRoot,                               //455 SPELL_AURA_MOD_ROOTED
    &AuraEffect::HandleNULL,                                      //456 SPELL_AURA_CHARGE_RECOVERY_AFFECTED_BY_HASTE
    &AuraEffect::HandleNoImmediateEffect,                         //457 SPELL_AURA_CHARGE_RECOVERY_AFFECTED_BY_HASTE_REGEN
    &AuraEffect::HandleNoImmediateEffect,                         //458 SPELL_AURA_IGNORE_DUAL_WIELD_HIT_PENALTY
    &AuraEffect::HandleDisableMovementForce,                      //459 SPELL_AURA_DISABLE_MOVEMENT_FORCE
    &AuraEffect::HandleNoImmediateEffect,                         //460 SPELL_AURA_RESET_COOLDOWNS_AT_DUEL_START
    &AuraEffect::HandleUnused,                                    //461 SPELL_AURA_461
    &AuraEffect::HandleNULL,                                      //462 SPELL_AURA_MOD_HEALING_AND_ABSORB_FROM_CASTER
    &AuraEffect::HandleConverCritRatingPctToParryRating,          //463 SPELL_AURA_CONVERT_CRIT_RATING_PCT_TO_PARRY_RATING
    &AuraEffect::HandleUnused,                                    //464 SPELL_AURA_464
    &AuraEffect::HandleAuraBonusArmor,                            //465 SPELL_AURA_MOD_BONUS_ARMOR
    &AuraEffect::HandleAuraBonusArmor,                            //466 SPELL_AURA_MOD_BONUS_ARMOR_PCT
    &AuraEffect::HandleModStatBonusPercent,                       //467 SPELL_AURA_MOD_STAT_BONUS_PCT
    &AuraEffect::HandleAuraProcOnHpBelow,                         //468 SPELL_AURA_PROC_ON_HP_BELOW implemented in Unit::ModifyHealth
    &AuraEffect::HandleShowConfirmationPrompt,                    //469 SPELL_AURA_SHOW_CONFIRMATION_PROMPT_WITH_DIFFICULTY
    &AuraEffect::HandleExpedite,                                  //470 SPELL_AURA_EXPEDITE
    &AuraEffect::HandleModVersalityPct,                           //471 SPELL_AURA_MOD_VERSALITY_PCT
    &AuraEffect::HandleFixate,                                    //472 SPELL_AURA_FIXATE2
    &AuraEffect::HandleNoImmediateEffect,                         //473 SPELL_AURA_PREVENT_DURABILITY_LOSS_FROM_COMBAT
    &AuraEffect::HandleNULL,                                      //474 SPELL_AURA_MOD_DROP_ITEM_BONUSES_CHANCE
    &AuraEffect::HandleAllowUsingGameobjectsWhileMounted,         //475 SPELL_AURA_ALLOW_USING_GAMEOBJECTS_WHILE_MOUNTED
    &AuraEffect::HandleNULL,                                      //476 SPELL_AURA_MOD_CURRENCY_GAIN_LOOTED
    &AuraEffect::HandleNULL,                                      //477 SPELL_AURA_DRAENOR_SPELLS_AND_ITEMS_SCALING
    &AuraEffect::HandleNULL,                                      //478 SPELL_AURA_478
    &AuraEffect::HandleNULL,                                      //479 SPELL_AURA_479
    &AuraEffect::HandleUnused,                                    //480 SPELL_AURA_480
    &AuraEffect::HandleNULL,                                      //481 SPELL_AURA_CONVERT_CONSUMED_RUNE
    &AuraEffect::HandleNULL,                                      //482 SPELL_AURA_CAMERA_SELFIE
    &AuraEffect::HandleNULL,                                      //483 SPELL_AURA_TRANSFORM_SUPRESSION_IN_PVP
    &AuraEffect::HandleNULL,                                      //484 SPELL_AURA_ALLOW_KICK_SPECIFIC_SPELL_WITH_ID
    &AuraEffect::HandleNULL,                                      //485 SPELL_AURA_MOD_GRAVITY_SPEED
    &AuraEffect::HandleNULL,                                      //486 SPELL_AURA_INTERFERE_TARGETTING_2 
    &AuraEffect::HandleNULL,                                      //487 SPELL_AURA_487
    &AuraEffect::HandleDisableGravity,                            //488 SPELL_AURA_DISABLE_GRAVITY
    &AuraEffect::HandleNULL,                                      //489 SPELL_AURA_FORGET_LANGUAGE
    &AuraEffect::HandleNULL,                                      //490 SPELL_AURA_SWITCH_TEAM
    &AuraEffect::HandleNoImmediateEffect,                         //491 SPELL_AURA_SPELL_AURA_MOD_HONOR_POINTS_GAIN
    &AuraEffect::HandleNULL,                                      //492 SPELL_AURA_492
};

AuraEffect::AuraEffect(Aura* base, uint8 effIndex, float *baseAmount, Unit* caster, uint8 diffMode) : m_base(base), m_spellInfo(base->GetSpellInfo()), _effectInfo(base->GetSpellInfo()->GetEffect(effIndex, (caster ? (caster->GetMap() ? caster->GetMap()->GetDifficultyID() : 0) : 0))), m_baseAmount(baseAmount ? *baseAmount : m_spellInfo->GetEffect(effIndex, diffMode)->BasePoints), m_amount(0.f), m_calc_amount{0.f},
m_amount_add(0.f), m_amount_mod(1.0f), m_crit_mod(0.0f), m_oldbaseAmount(0.f), saveTarget(nullptr), m_spellmod(nullptr), m_periodicTimer(0), m_period(0), m_tickNumber(0), m_period_mod(0.0f), m_activation_time(0.f), m_nowInTick(false),
m_effIndex(effIndex), m_canBeRecalculated(true), m_isPeriodic(false), m_diffMode(diffMode)
{
    m_send_baseAmount = m_baseAmount;
}

AuraEffect::~AuraEffect()
{
    delete m_spellmod;
}

void AuraEffect::GetTargetList(std::list<Unit*> & targetList) const
{
    Aura::ApplicationMap const& targetMap = GetBase()->GetApplicationMap();
    // remove all targets which were not added to new list - they no longer deserve area aura
    for (Aura::ApplicationMap::const_iterator appIter = targetMap.begin(); appIter != targetMap.end(); ++appIter)
    {
        if (AuraApplicationPtr aurApp = appIter->second)
            if (aurApp->HasEffect(GetEffIndex()))
                targetList.push_back(aurApp->GetTarget());
    }
}

void AuraEffect::GetApplicationList(std::list<AuraApplication*> & applicationList) const
{
    Aura::ApplicationMap const& targetMap = GetBase()->GetApplicationMap();
    for (Aura::ApplicationMap::const_iterator appIter = targetMap.begin(); appIter != targetMap.end(); ++appIter)
    {
        if (AuraApplicationPtr aurApp = appIter->second)
            if (aurApp->HasEffect(GetEffIndex()))
                applicationList.push_back(aurApp.get());
    }
}

float AuraEffect::CalculateAmount(Unit* caster)
{
    Item* castItem = nullptr;
    Aura* thisAura = GetBase();
    // Clear saved mod on reapply aura
    m_amount_add = 0.f;
    m_amount_mod = 1.0f;

    if(caster)
        if (ObjectGuid itemGUID = thisAura->GetCastItemGUID())
            if (Player* playerCaster = caster->ToPlayer())
                castItem = playerCaster->GetItemByGuid(itemGUID);

    Unit* target = thisAura->GetUnitOwner();

    // default amount calculation
    m_send_baseAmount = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->CalcValue(caster, &m_baseAmount, thisAura->GetOwner()->ToUnit(), castItem);
    m_crit_mod = (caster && caster->IsPlayer() && target && target != caster && target->IsPlayer()) ?  1.5f : 2.0f;

    // check item enchant aura cast
    if (!m_send_baseAmount && caster && castItem)
    {
        if (castItem->GetItemSuffixFactor())
        {
            if (ItemRandomSuffixEntry const* item_rand_suffix = sItemRandomSuffixStore.LookupEntry(abs(castItem->GetItemRandomPropertyId())))
            {
                for (uint8 k = 0; k < MAX_ITEM_ENCHANTS; k++)
                {
                    if (SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(item_rand_suffix->Enchantment[k]))
                        for (auto t : pEnchant->EffectArg)
                            if (t == m_spellInfo->Id)
                            {
                                m_send_baseAmount = uint32((item_rand_suffix->AllocationPct[k] * castItem->GetItemSuffixFactor()) / 10000);
                                break;
                            }

                    if (m_send_baseAmount)
                        break;
                }
            }
        }
    }

    bool CalcStack = bool(m_spellInfo->GetAuraOptions(m_diffMode)->CumulativeAura) && m_spellInfo->IsStack();

    if (caster && caster->IsPlayer() && (m_spellInfo->HasAttribute(SPELL_ATTR8_MASTERY_SPECIALIZATION)))
    {
        m_canBeRecalculated = false;
        m_send_baseAmount += caster->GetFloatValue(PLAYER_FIELD_MASTERY) * m_spellInfo->GetEffect(m_effIndex, m_diffMode)->BonusCoefficient;
    }

    thisAura->CallScriptEffectBeforeCalcAmountHandlers(const_cast<AuraEffect const*>(this), m_send_baseAmount, m_canBeRecalculated);

    float amount = m_send_baseAmount;

    if (thisAura->HasAuraAttribute(AURA_ATTR_DRINK_ARENA_DELAY))
    {
        amount = 0;
        m_calc_amount = amount;
        return amount;
    }

    // custom amount calculations go here
    switch (GetAuraType())
    {
        case SPELL_AURA_MOD_FEAR:
        case SPELL_AURA_MOD_FEAR_2:
        case SPELL_AURA_MOD_ROOT:
        case SPELL_AURA_MOD_ROOTED:
        case SPELL_AURA_TRANSFORM:
            m_canBeRecalculated = false;
            if (!m_spellInfo->GetAuraOptions(m_diffMode)->ProcTypeMask)
                break;
            amount = int32(target->CountPctFromMaxHealth(5, caster));
            break;
        case SPELL_AURA_MOD_SPEED_ALWAYS:
        {
            switch (m_spellInfo->Id)
            {
                case 11327:  // Vanish
                case 1784:   // Stealth
                case 5215:   // Prowl
                {
                    if (caster)
                        if (Aura* aura = caster->GetAura(21009)) // Elusiveness
                            if (AuraEffect* eff = aura->GetEffect(EFFECT_0))
                                amount += eff->GetAmount();
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
        {
            if (m_spellInfo->HasAttribute(SPELL_ATTR0_CU_EXTRA_PVP_STAT_SPELL) && (GetMiscValueB() & (1 << STAT_STAMINA)))
            {
                if (Player* plr = caster->ToPlayer())
                {
                    if (plr->HasPvpStatsScalingEnabled())
                        amount = 0;
                }
            }
            break;
        }
        case SPELL_AURA_MOD_RESILIENCE_PCT:
        {
            if (!caster)
                break;

            if (Player* plr = caster->ToPlayer())
            {
                switch (plr->GetSpecializationId())
                {
                    case SPEC_DRUID_BEAR:
                    case SPEC_MONK_BREWMASTER:
                    case SPEC_WARRIOR_PROTECTION:
                    case SPEC_DK_BLOOD:
                    case SPEC_PALADIN_PROTECTION:
                    case SPEC_DEMON_HUNER_VENGEANCE:
                    {
                        amount = 0;
                        break;
                    }
                }
            }
            break;
        }
        case SPELL_AURA_SCHOOL_ABSORB:
        {
            m_canBeRecalculated = false;

            if (!caster)
                break;

            switch (m_spellInfo->Id)
            {
                case 108416: // Sacrificial Pact
                {
                    Unit* spelltarget = caster->GetGuardianPet() ? caster->GetGuardianPet(): caster;
                    int32 sacrifiedHealth = spelltarget->CountPctFromCurHealth(m_spellInfo->Effects[EFFECT_1]->BasePoints);
                    spelltarget->ModifyHealth(-sacrifiedHealth, nullptr, GetId());
                    amount = CalculatePct(sacrifiedHealth, m_spellInfo->Effects[EFFECT_0]->BasePoints);
                    break;
                }
                default:
                    break;
            }
            amount = caster->CalcAbsorb(target, m_spellInfo, amount);
            break;
        }
        case SPELL_AURA_PERIODIC_DUMMY:
        {
            switch (m_spellInfo->Id)
            {
                case 146198: // Essence of Yu'lon
                {
                    if (SpellInfo const* _info = sSpellMgr->GetSpellInfo(148008))
                        amount = caster->GetSpellPowerDamage(_info->GetSchoolMask()) * _info->Effects[EFFECT_0]->BonusCoefficient / GetTotalTicks(); 
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_ADD_PCT_MODIFIER:
        {
            if (!caster)
                break;

            switch (m_spellInfo->Id)
            {
                case 188923: // Cleave
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(209462, EFFECT_0)) // One Against Many
                        amount += aurEff->GetAmount();
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED:
        case SPELL_AURA_MOD_MINIMUM_SPEED_RATE:
        {
            if (!caster)
                break;

            switch (m_spellInfo->Id)
            {
                case 197922: // Fel Rush
                case 197923: // Fel Rush
                {
                    if (caster->HasAuraType(SPELL_AURA_BATTLEGROUND_FLAG) || caster->HasAuraType(SPELL_AURA_BATTLEGROUND_FLAG_2))
                        amount = 0;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_RATING:
        {
            if (!caster)
                break;

            amount *= caster->GetProcStatsMultiplier(m_spellInfo->Id);
            break;
        }
        case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
        {
            if (!caster)
                break;

            switch (m_spellInfo->Id)
            {
                case SPELL_BG_HORDE_FLAG:
                case SPELL_BG_ALLIANCE_FLAG:
                {
                    if (Player* plr = caster->ToPlayer())
                    {
                        if (plr->isInTankSpec())
                            amount = (m_spellInfo->Effects[EFFECT_1]->BasePoints + 30);
                    }
                    break;
                }
                case 194384: // Atonement
                {
                    amount /= 100;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_CHARGE_RECOVERY_AFFECTED_BY_HASTE_REGEN:
        {
            if (Player* player = caster->ToPlayer())
            {
                amount = -100;
                amount += player->GetFloatValue(UNIT_FIELD_MOD_HASTE_REGEN) * 100.0f;
            }
            if (amount > 0)
                amount = 0;
            break;
        }
        case SPELL_AURA_PERIODIC_LEECH:
        {
            if (!caster || !target)
                break;

            std::vector<uint32> ExcludeAuraList;
            amount = caster->SpellDamageBonusDone(target, m_spellInfo, amount, DOT, ExcludeAuraList, static_cast<SpellEffIndex>(GetEffIndex()));

            caster->SpellDotPctDone(target, m_spellInfo, GetEffIndex(), m_amount_mod, m_amount_add);
            break;
        }
        case SPELL_AURA_PERIODIC_DAMAGE:
        {
            if (!caster || !target)
                break;

            if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_ROGUE) // Nightstalker
            {
                if (AuraEffect* aurEff = caster->GetAuraEffect(130493, EFFECT_1))
                    if (aurEff->IsAffectingSpell(m_spellInfo))
                        AddPct(m_amount_mod, aurEff->GetAmount());
            }

            if (m_spellInfo->HasPower(POWER_COMBO_POINTS) && !m_spellInfo->CanModDuration(m_diffMode))
            {
                int8 comboPoints = thisAura->GetComboPoints();
                comboPoints = comboPoints ? comboPoints : caster->GetMaxPower(POWER_COMBO_POINTS);
                m_amount_mod *= comboPoints;
            }

            bool calcMod = true;

            switch (GetId())
            {
                case 13812: // Explosive Trap
                {
                    if (AuraEffect const* eff = caster->GetAuraEffect(199543, EFFECT_0)) // Expert Trapper
                    {
                        Trinity::AnyDataContainer& cont = caster->GetAnyDataContainer();

                        if (target->GetGUID() == cont.GetValue<ObjectGuid>("13813Expert", ObjectGuid::Empty))
                            AddPct(m_amount_mod, eff->GetAmount());
                    }
                    break;
                }
                case 195452: // Nightblade
                {
                    if (AuraEffect const* auraEff = caster->GetAuraEffect(197498, EFFECT_0)) // Finality: Nightblade
                        AddPct(m_amount_mod, auraEff->GetAmount());
                    break;
                }
                case 162487: // Steel Trap
                {
                    if (!target->isInCombat())
                    {
                        if (AuraEffect* aurEff = caster->GetAuraEffect(234955, EFFECT_2)) // Waylay
                        {
                            std::list<AreaTrigger*> aTList;
                            caster->GetAreaTriggersWithEntryInRange(aTList, 6966, caster->GetGUID(), 100.f);

                            for (auto itr : aTList)
                            {
                                if (AreaTriggerAI* ATAI = itr->AI())
                                {
                                    if (ATAI->CallSpecialFunction() > 2000)
                                        AddPct(m_amount_mod, aurEff->GetAmount());

                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                case 980: // Agony
                {
                    CalcStack = false;
                    break;
                }
                case 210705: // Ashamane's Rip
                {
                    if (AuraEffect* auraEff = target->GetAuraEffect(1079, EFFECT_0, caster->GetGUID()))
                    {
                        m_amount_mod = auraEff->m_amount_mod;
                        m_periodicTimer = auraEff->m_periodicTimer;
                    }
                    break;
                }
                case 155722: // Rake (DOT)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(1822);

                    if (spellInfo)
                    {
                        int32 bp = spellInfo->Effects[EFFECT_3]->CalcValue(caster);
                        if (caster->CanPvPScalar()) // hack, why doesn't it work PvPMultiplier?!
                            bp *= spellInfo->Effects[EFFECT_3]->PvPMultiplier;

                        if (caster->HasAuraType(SPELL_AURA_MOD_STEALTH) || caster->HasAura(102543))
                            AddPct(m_amount_mod, bp);
                    }
                }
                case 1079:   // Rip
                case 106830: // Thrash
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(145152, EFFECT_0)) // Bloodtalons
                        AddPct(m_amount_mod, aurEff->GetAmount());
                }
                case 155625: // Moonfire (Cat form)
                {
                    if (AuraEffect* auraEff = caster->GetAuraEffect(5217, EFFECT_0)) // Tiger's Fury
                        AddPct(m_amount_mod, auraEff->GetAmount());
                    break;
                }
                case 233490: // Unstable Affliction
                case 233496:
                case 233497:
                case 233498:
                case 233499:
                {
                    std::vector<uint32> ExcludeAuraList;
                    amount = caster->SpellDamageBonusDone(target, m_spellInfo, amount, DOT, ExcludeAuraList, static_cast<SpellEffIndex>(GetEffIndex()));
                    float mod = 1.f;
                    caster->SpellDotPctDone(target, m_spellInfo, GetEffIndex(), mod, m_amount_add, false);
                    amount *= mod;
                    calcMod = false;
                    break;
                }
                case 603: // Doom
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(214225, EFFECT_0)) // Kazaaks Final Curse
                    {
                        uint32 count = 0;
                        for (auto& guid : caster->m_Controlled)
                            if(Unit* unit = ObjectAccessor::GetUnit(*caster, guid))
                                if (unit->HasUnitTypeMask(UNIT_MASK_CREATED_BY_CLASS_SPELL) || unit->isPet())
                                    count++;

                        AddPct(m_amount_mod, aurEff->GetAmount() * count);
                    }
                    break;
                }
            }

            if (calcMod)
            {
                std::vector<uint32> ExcludeAuraList;
                amount = caster->SpellDamageBonusDone(target, m_spellInfo, amount, DOT, ExcludeAuraList, static_cast<SpellEffIndex>(GetEffIndex()));
                m_amount_add *= caster->GetProcStatsMultiplier(m_spellInfo->Id);
                caster->SpellDotPctDone(target, m_spellInfo, GetEffIndex(), m_amount_mod, m_amount_add);
            }

            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        {
            if (!caster || !target)
                break;

            amount = caster->SpellHealingBonusDone(target, m_spellInfo, amount, DOT, static_cast<SpellEffIndex>(GetEffIndex()));
            m_amount_add *= caster->GetProcStatsMultiplier(m_spellInfo->Id);
            caster->SpellDotPctDone(target, m_spellInfo, GetEffIndex(), m_amount_mod, m_amount_add);
            break;
        }
        case SPELL_AURA_MOD_THREAT:
        {
            uint8 level_diff = 0;
            float multiplier = 0.0f;
            switch (GetId())
            {
                // Arcane Shroud
                case 26400:
                    level_diff = target->getLevelForTarget(caster) - 60;
                    multiplier = 2;
                    break;
                // The Eye of Diminution
                case 28862:
                    level_diff = target->getLevelForTarget(caster) - 60;
                    multiplier = 1;
                    break;
            }
            if (level_diff > 0)
                amount += int32(multiplier * level_diff);
            break;
        }
        case SPELL_AURA_HASTE_SPELLS:
        {
            switch (m_spellInfo->Id)
            {                
                case 25810:
                {
                    if (target && target->IsPlayer())
                        amount /= 3;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_STAT:
        {
            switch (m_spellInfo->Id)
            {
                case 142530: // Bloody Dancing Steel
                case 120032: // Dancing Steel
                {
                    int32 str = caster->GetStat(STAT_STRENGTH);
                    int32 agi = caster->GetStat(STAT_AGILITY);
                    
                    switch (m_effIndex)
                    {
                        case EFFECT_0: if (str > agi) amount = 0; break;
                        case EFFECT_1: if (str < agi) amount = 0; break;
                    }
                    break;
                }
                case 108300: // Killer Instinct
                {
                    if (caster)
                        amount = caster->GetStat(STAT_INTELLECT);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_INCREASE_HEALTH_2:
        case SPELL_AURA_MOD_INCREASE_HEALTH_OR_PCT:
        {
            switch (m_spellInfo->Id)
            {
                case 132413: // Shadow Bulwark
                {
                    amount = CalculatePct(caster->GetMaxHealth(), amount);
                    break;
                }
                case 137211: // Tremendous Fortitude
                {
                    amount = CalculatePct(caster->GetMaxHealth(), amount);
                    if (amount > 18000)
                        amount = 18000;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_INCREASE_SPEED:
            // Dash - do not set speed if not in cat form
            if (GetSpellInfo()->ClassOptions.SpellClassSet == SPELLFAMILY_DRUID && GetSpellInfo()->ClassOptions.SpellClassMask[2] & 0x00000008)
                amount = target->GetShapeshiftForm() == FORM_CAT ? amount : 0;
            break;
        case SPELL_AURA_STRANGULATE:
        {
            // Hack. OO Galakras: Anti-Air Cannon
            if (Aura* aura = target->GetAura(147514))
                if (aura->GetStackAmount() < 2)
                    aura->SetStackAmount(aura->GetStackAmount() + 1);
            break;
        }
        case SPELL_AURA_SHOW_CONFIRMATION_PROMPT_WITH_DIFFICULTY:
            if (caster)
                amount = caster->GetMap()->GetDifficultyID();
            m_canBeRecalculated = false;
            break;
        case SPELL_AURA_MOUNTED:
        {
            auto mountType = uint32(GetMiscValueB());
            if (MountEntry const* mountEntry = sDB2Manager.GetMount(GetId()))
                mountType = mountEntry->MountTypeID;

            if (MountCapabilityEntry const* mountCapability = thisAura->GetUnitOwner()->GetMountCapability(mountType))
            {
                amount = mountCapability->ID;
                m_canBeRecalculated = false;
            }
            break;
        }
        case SPELL_AURA_MELEE_SLOW:
        {
            switch (m_spellInfo->Id)
            {
                case 195181: // Bone Shield
                {
                    CalcStack = false; // This effect not stack
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    // Mixology - Effect value mod
    if (caster && caster->IsPlayer())
    {
        if (sSpellMgr->IsSpellMemberOfSpellGroup(GetId(), SPELL_GROUP_ELIXIR_BATTLE) || sSpellMgr->IsSpellMemberOfSpellGroup(GetId(), SPELL_GROUP_ELIXIR_GUARDIAN))
        {
            if (caster->HasAura(53042) && caster->HasSpell(GetSpellInfo()->Effects[0]->TriggerSpell))
            {
                switch (GetId())
                {
                    case 53749: // Guru's Elixir
                        amount += 8;
                        break;
                    case 11328: // Elixir of Agility
                        amount += 10;
                        break;
                    case 28497: // Elixir of Mighty Agility
                    case 53747: // Elixir of Spirit
                    case 54212: // Flask of Pure Mojo
                    case 60340: // Elixir of Accuracy
                    case 60341: // Elixir of Deadly Strikes
                    case 60343: // Elixir of Mighty Defense
                    case 60344: // Elixir of Expertise
                    case 60345: // Elixir of Armor Piercing
                    case 60346: // Elixir of Lightning Speed
                    case 60347: // Elixir of Mighty Thoughts
                        amount += 20;
                        break;
                    case 53752: // Lesser Flask of Toughness
                    case 62380: // Lesser Flask of Resistance
                        amount += 40;
                        break;
                    case 53755: // Flask of the Frost Wyrm
                        amount += 47;
                        break;
                    case 53760: // Flask of Endless Rage
                        amount += 64;
                        break;
                    case 53751: // Elixir of Mighty Fortitude
                        amount += 200;
                        break;
                    case 53763: // Elixir of Protection
                        amount += 280;
                        break;
                    case 53758: // Flask of Stoneblood
                        amount += 320;
                        break;
                    // Cataclysm
                    case 79469: // Flask of Steelskin
                    case 79470: // Flask of the Draconic Mind
                    case 79471: // Flask of the Winds
                    case 79472: // Flask of Titanic Strength
                    case 94160: // Flask of Flowing Water
                        amount += 80;
                        break;
                    case 79635: // Flask of the Master
                    case 79632: // Elixir of Mighty Speed
                    case 79481: // Elixir of Impossible Accuracy
                    case 79477: // Elixir of the Cobra
                    case 79474: // Elixir of the Naga
                    case 79468: // Ghost Elixir
                        amount += 40;
                        break;
                    case 79480: // Elixir of Deep Earth
                        amount += 345;
                        break;
                    case 79631: // Prismatic Elixir
                        amount += 45;
                        break;
                    // Pandaria
                    case 105689: // Flask of Spring Blossoms
                    case 105691: // Flask of the Warm Sun
                    case 105696: // Flask of Winter's Bite
                        amount += 320;
                        break;
                    case 105694: // Flask of the Earth
                    case 105693: // Flask of Falling Leaves
                        amount += 480;
                        break;
                }
            }
        }
    }

    thisAura->CallScriptEffectCalcAmountHandlers(const_cast<AuraEffect const*>(this), amount, m_canBeRecalculated);

    CalculateFromDummyAmount(caster, target, amount);

    if (CalcStack)
        m_amount_mod *= thisAura->GetStackAmount();

    switch (m_spellInfo->Id)
    {
        case 197496: // Finality: Eviscerate
        case 197498: // Finality: Nightblade
        {
            amount = amount / 5 * caster->GetComboPoints();
            break;
        }
        case 231895: // Crusade
        case 209316: // Cord of Infinity
        case 207995: // Eye of the Twisting Nether
        case 207998: // Eye of the Twisting Nether
        case 207999: // Eye of the Twisting Nether
        {
            amount /= 10.f;
            break;
        }
        case 202942: // Star Power
        { 
            if (caster->HasAura(102560))
                amount /= 3.f;
            break;
        }
        case 205766: // Bone Chilling
        {
            amount /= 2.f; 
            break;
        }
        case 187301: // Everywhere At Once
        {
            if (m_effIndex == 0)
                amount *= 1000.f;
            if (m_effIndex == 1)
                amount *= -2000.f;
            break;
        }
    }

    //Disable same auras on arena or BG(RBG)
    if(caster)
    {
        if (Player* plr = caster->ToPlayer())
        {
            if (plr->HasPvpStatsScalingEnabled() && m_spellInfo->HasAttribute(SPELL_ATTR8_NOT_IN_BG_OR_ARENA))
                m_amount_mod = 0.0f;
            if ((plr->InRBG() || plr->InArena()) && m_spellInfo->HasAttribute(SPELL_ATTR4_NOT_USABLE_IN_ARENA_OR_RATED_BG))
                m_amount_mod = 0.0f;
        }
    }

    switch (GetAuraType())
    {
        case SPELL_AURA_MOD_ABSORBTION_PERCENT:
        {
            if (caster)
            {
                if (auto scalar = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->PvPMultiplier)
                    if (caster && caster->CanPvPScalar())
                        m_amount_mod *= scalar;

                amount = amount * m_amount_mod;
                amount += m_amount_add;

                if (amount < -100.0f)
                    amount = -100.0f;
            }
            break;
        }
        case SPELL_AURA_SCHOOL_ABSORB:
        {
            if (caster)
            {
                if (auto scalar = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->PvPMultiplier)
                    if (caster && caster->CanPvPScalar())
                        m_amount_mod *= scalar;

                amount = amount * m_amount_mod * caster->GetProcStatsMultiplier(m_spellInfo->Id);
                amount += m_amount_add;

                if (!(GetSpellInfo()->HasAttribute(SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS)))
                {
                    if (Player* modOwner = caster->GetSpellModOwner())
                        amount += CalculatePct(amount, modOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY) + modOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));

                    if (Unit::AuraEffectList const* mAbsorbAmount = caster->GetAuraEffectsByType(SPELL_AURA_MOD_ABSORB_AMOUNT))
                        for (Unit::AuraEffectList::const_iterator i = mAbsorbAmount->begin(); i != mAbsorbAmount->end(); ++i)
                            AddPct(amount, (*i)->GetAmount());

                    if (target)
                    {
                        if (Unit::AuraEffectList const* mAbsorbtionPercent = target->GetAuraEffectsByType(SPELL_AURA_MOD_ABSORBTION_PERCENT))
                            for (Unit::AuraEffectList::const_iterator i = mAbsorbtionPercent->begin(); i != mAbsorbtionPercent->end(); ++i)
                                AddPct(amount, (*i)->GetAmount());
                    }
                }

                amount = RoundingFloatValue(amount);
            }
            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_PERIODIC_DAMAGE:
        {
            if (caster)
                m_amount_mod *= caster->GetProcStatsMultiplier(m_spellInfo->Id);
            amount = amount * m_amount_mod;
            amount += m_amount_add;
            if (m_spellInfo->Id == 210723) // Hack for Ashamane's Frenzy
                amount += m_oldbaseAmount;
            else if (m_spellInfo->HasAttribute(SPELL_ATTR10_STACK_DAMAGE_OR_HEAL))
                if(thisAura->GetMaxDuration() != -1 && caster && GetTotalTicks())
                    amount = target->GetRemainingPeriodicAmount(caster->GetGUID(), m_spellInfo->Id, GetAuraType(), m_effIndex, amount);
            break;
        }
        default:
        {
            if (auto scalar = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->PvPMultiplier)
                if (caster && caster->CanPvPScalar())
                    m_amount_mod *= scalar;

            amount = amount * m_amount_mod;
            amount += m_amount_add;
            break;
        }
    }

    // Epicurean 107072
    if (m_spellInfo->HasAttribute(SPELL_ATTR2_FOOD_BUFF))
        if (target && target->IsPlayer() && (target->getRace() == RACE_PANDAREN_ALLIANCE || target->getRace() == RACE_PANDAREN_HORDE || target->getRace() == RACE_PANDAREN_NEUTRAL))
            amount *= 2;

    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraApplication::CalculateAmount GetId %i GetAuraType %u amount %i effIndex %i m_amount_mod %f m_amount_add %i GetStackAmount %u", GetId(), GetAuraType(), amount, m_effIndex, m_amount_mod, m_amount_add, GetBase()->GetStackAmount());
    #endif

    m_calc_amount = amount;
    return amount;
}

void AuraEffect::CalculateFromDummyAmount(Unit* caster, Unit* target, float &amount)
{
    if(!caster)
        return;

    auto spellAuraDummy = sSpellMgr->GetSpellAuraDummy(GetId());
    if (!spellAuraDummy)
        return;

    for (const auto& itr : *spellAuraDummy)
    {
        // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::CalculateFromDummyAmount start GetId %i, amount %i, mask %i type %u option %u caster %u GetCaster %u",
        // GetId(), amount, itr->effectmask, itr->type, itr->option, caster->GetGUIDLow(), GetCaster()->GetGUIDLow());

        if (itr.type != SPELL_DUMMY_DEFAULT)
            continue;

        if (!(itr.effectmask & (1 << m_effIndex)))
            continue;

        Unit* _caster = caster;
        Unit* _targetAura = caster;
        bool check = false;

        if (itr.caster == 1 && target) //get caster as target
            _caster = target;
        if (itr.caster == 2 && _caster->ToPlayer()) //get target pet
        {
            if (Pet* pet = _caster->ToPlayer()->GetPet())
                _caster = pet->ToUnit();
        }
        if (itr.caster == 3) //get target owner
        {
            if (Unit* owner = _caster->GetOwner())
                _caster = owner;
        }

        if (itr.targetaura == 1 && GetCaster()) //get caster aura
            _targetAura = GetCaster();
        if (itr.targetaura == 2 && target) //get target aura
            _targetAura = target;

        if (!_targetAura)
            _targetAura = caster;

        if (itr.hastalent)
            if (_caster->HasAuraLinkedSpell(_caster, target, itr.hastype, itr.hastalent, itr.hasparam))
                continue;

        if (itr.hastalent2)
            if (_caster->HasAuraLinkedSpell(_caster, target, itr.hastype2, itr.hastalent2, itr.hasparam2))
                continue;

        Aura* _aura = _caster->GetAura(abs(itr.spellDummyId), caster->GetGUID());
        Player* player = _caster->ToPlayer();

        switch (itr.option)
        {
        case SPELL_DUMMY_ENABLE: //0
        {
            if (itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                continue;
            if (itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                continue;

            if (itr.spellDummyId > 0 && !_aura)
            {
                m_amount_mod = 0.0f;
                check = true;
            }
            if (itr.spellDummyId < 0 && _aura)
            {
                m_amount_mod = 0.0f;
                check = true;
            }
            break;
        }
        case SPELL_DUMMY_ADD_PERC: //1
        {
            if (itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                continue;
            if (itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                continue;

            if (itr.spellDummyId > 0)
            {
                AuraEffect const* dummyEff = nullptr;
                if (_aura)
                    dummyEff = _aura->GetEffect(itr.effectDummy);
                if (dummyEff)
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyEff->GetAmount();
                    AddPct(m_amount_mod, bp);
                    check = true;
                }
                else if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(itr.spellDummyId)) // for spell without aura for this affect
                {
                    if (!player || !player->HasSpell(itr.spellDummyId))
                        continue;

                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    AddPct(m_amount_mod, bp);
                    check = true;
                }
            }
            if (itr.spellDummyId < 0)
            {
                AuraEffect const* dummyEff = nullptr;
                if (_aura)
                    dummyEff = _aura->GetEffect(itr.effectDummy);
                if (dummyEff)
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyEff->GetAmount();
                    AddPct(m_amount_mod, -bp);
                    check = true;
                }
                else if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(abs(itr.spellDummyId))) // for spell without aura for this affect
                {
                    if (!player || !player->HasSpell(abs(itr.spellDummyId)))
                        continue;

                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    AddPct(m_amount_mod, -bp);
                    check = true;
                }
            }
            break;
        }
        case SPELL_DUMMY_ADD_VALUE: //2
        {
            if (itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                continue;
            if (itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                continue;

            if (itr.spellDummyId > 0)
            {
                AuraEffect const* dummyEff = nullptr;
                if (_aura)
                    dummyEff = _aura->GetEffect(itr.effectDummy);
                if (dummyEff)
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyEff->GetAmount();
                    m_amount_add += bp;
                    check = true;
                }
                else if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(itr.spellDummyId)) // for spell without aura for this affect
                {
                    if (!player || !player->HasSpell(itr.spellDummyId))
                        continue;

                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    m_amount_add += bp;
                    check = true;
                }
            }
            if (itr.spellDummyId < 0)
            {
                AuraEffect const* dummyEff = nullptr;
                if (_aura)
                    dummyEff = _aura->GetEffect(itr.effectDummy);
                if (dummyEff)
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyEff->GetAmount();
                    m_amount_add -= bp;
                    check = true;
                }
                else if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(abs(itr.spellDummyId))) // for spell without aura for this affect
                {
                    if (!player || !player->HasSpell(abs(itr.spellDummyId)))
                        continue;

                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    m_amount_add -= bp;
                    check = true;
                }
            }
            break;
        }
        case SPELL_DUMMY_ADD_PERC_BP: //8
        {
            if (itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                continue;
            if (itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                continue;

            if (itr.spellDummyId > 0)
            {
                AuraEffect const* dummyEff = nullptr;
                if (_aura)
                    dummyEff = _aura->GetEffect(itr.effectDummy);
                if (dummyEff)
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyEff->GetAmount();

                    bp /= 100.0f;
                    AddPct(m_amount_mod, bp);
                    check = true;
                }
                else if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(itr.spellDummyId)) // for spell without aura for this affect
                {
                    if (!player || !player->HasSpell(itr.spellDummyId))
                        continue;

                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;

                    bp /= 100.0f;
                    AddPct(m_amount_mod, bp);
                    check = true;
                }
            }
            if (itr.spellDummyId < 0)
            {
                AuraEffect const* dummyEff = nullptr;
                if (_aura)
                    dummyEff = _aura->GetEffect(itr.effectDummy);
                if (dummyEff)
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyEff->GetAmount();

                    bp /= 100.0f;
                    AddPct(m_amount_mod, -bp);
                    check = true;
                }
                else if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(abs(itr.spellDummyId))) // for spell without aura for this affect
                {
                    if (!player || !player->HasSpell(abs(itr.spellDummyId)))
                        continue;

                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;

                    bp /= 100.0f;
                    AddPct(m_amount_mod, -bp);
                    check = true;
                }
            }
            break;
        }
        }
        if (check && itr.removeAura)
            _caster->RemoveAurasDueToSpell(itr.removeAura);
    }
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::CalculateFromDummyAmount end GetId %i, amount %i m_effIndex %u m_amount_mod %u", GetId(), amount, m_effIndex, m_amount_mod);
}

void AuraEffect::CalculatePeriodic(Unit* caster, bool resetPeriodicTimer /*= true*/, bool load /*= false*/)
{
    m_period = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->ApplyAuraPeriod;
    m_period_mod = 0.0f;
    m_activation_time = 0.0f;

    // prepare periodics
    switch (GetAuraType())
    {
        case SPELL_AURA_OBS_MOD_POWER:
            // 3 spells have no amplitude set
            if (!m_period)
                m_period = 1 * IN_MILLISECONDS;
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        case SPELL_AURA_PERIODIC_ENERGIZE:
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        case SPELL_AURA_POWER_BURN:
        case SPELL_AURA_PERIODIC_DUMMY:
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
        case SPELL_AURA_PERIODIC_WEAPON_PERCENT_DAMAGE:
            if(GetBase()->GetMaxDuration())
                m_isPeriodic = true;
            else
                m_isPeriodic = false;
            break;
        default:
            break;
    }

    GetBase()->CallScriptEffectCalcPeriodicHandlers(const_cast<AuraEffect const*>(this), m_isPeriodic, m_period);

    if (!m_isPeriodic)
        return;

    Player* modOwner = caster ? caster->GetSpellModOwner() : nullptr;

    if (m_spellInfo->HasAttribute(SPELL_ATTR0_CU_ALWAYS_RESET_TIMER_AND_TICK))
        resetPeriodicTimer = true;

    // Apply casting time mods
    if (m_period)
    {
        // Apply periodic time mod
        if (modOwner)
            modOwner->ApplySpellMod(GetId(), SPELLMOD_ACTIVATION_TIME, m_activation_time);

        if (caster)
        {
            // Haste modifies periodic time of channeled spells
            if (m_spellInfo->IsChanneled())
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME))
                    caster->ModSpellCastTime(m_spellInfo, m_period);
            }
            else
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME))
                    m_period = int32(m_period * caster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE));

                if (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION_RECOVERY))
                {
                    int32 _duration = GetBase()->GetMaxDuration() * caster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE);
                    GetBase()->SetMaxDuration(_duration);
                    GetBase()->SetDuration(_duration);
                }
                if (Unit* target = GetBase()->GetUnitOwner())
                {
                    if (uint32 totalMod = target->GetTotalAuraModifier(SPELL_AURA_EXPEDITE))
                    {
                        float perc = 1.0f;
                        AddPct(perc, totalMod);
                        m_period_mod -= m_period - (m_period / perc);
                        int32 _duration = GetBase()->GetDuration() / perc;
                        if (_duration > GetBase()->GetMaxDuration())
                            GetBase()->SetMaxDuration(_duration);
                        GetBase()->SetDuration(_duration);
                    }
                }
            }
            if (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION))
                m_period = int32(m_period * caster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE));
        }

        m_period += m_period_mod;
        m_period += m_activation_time;

        //! If duration nod defined should we change duration? this remove aura.
        if (!resetPeriodicTimer && !load && !(m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME)) && 
            !(m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION_RECOVERY)) && !(m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION)) && 
            m_spellInfo->Misc.Duration.Duration != -1 /* && GetBase()->GetMaxDuration() >= 0*/)
        {
            int32 dotduration = GetBase()->GetMaxDuration() + m_periodicTimer;
            GetBase()->SetMaxDuration(dotduration);
            GetBase()->SetDuration(dotduration);
        }
    }

    if (load) // aura loaded from db
    {
        m_tickNumber = m_period ? GetBase()->GetDuration() / m_period : 0;
        if (m_spellInfo->HasAttribute(SPELL_ATTR5_START_PERIODIC_AT_APPLY))
        {
            m_periodicTimer = m_period ? GetBase()->GetDuration() % m_period : 0;
            ++m_tickNumber;
        }
    }
    else // aura just created or reapplied
    {
        m_tickNumber = 0;
        // reset periodic timer on aura create or on reapply when aura isn't dot
        // possibly we should not reset periodic timers only when aura is triggered by proc
        // or maybe there's a spell attribute somewhere
        if (resetPeriodicTimer && !m_nowInTick) // If now in tick do not increment, prevent freeze
        {
            m_periodicTimer = 0;
            // Start periodic on next tick or at aura apply
            if (m_period && (m_spellInfo->HasAttribute(SPELL_ATTR5_START_PERIODIC_AT_APPLY)))
                m_periodicTimer += m_period;
        }
    }
}

void AuraEffect::CalculateSpellMod()
{
    switch (GetAuraType())
    {
        case SPELL_AURA_ADD_FLAT_MODIFIER:
        case SPELL_AURA_ADD_PCT_MODIFIER:
        {
            if (!m_spellmod)
            {
                m_spellmod = new SpellModifier(GetBase());
                m_spellmod->op = SpellModOp(GetMiscValue());
                ASSERT(m_spellmod->op < MAX_SPELLMOD);

                m_spellmod->type = SpellModType(GetAuraType());    // SpellModType value == spell aura types
                m_spellmod->spellId = GetId();
                m_spellmod->mask = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->SpellClassMask;
                m_spellmod->charges = GetBase()->GetCharges();
            }
            m_spellmod->value = GetAmount();
            break;
        }
        default:
            break;
    }
    GetBase()->CallScriptEffectCalcSpellModHandlers(const_cast<AuraEffect const*>(this), m_spellmod);
}

void AuraEffect::ChangeAmount(float newAmount, bool mark, bool onStackOrReapply)
{
    // Reapply if amount change
    uint8 handleMask = 0;
    if (newAmount != GetAmount())
        handleMask |= AURA_EFFECT_HANDLE_CHANGE_AMOUNT;
    if (onStackOrReapply)
        handleMask |= AURA_EFFECT_HANDLE_REAPPLY;

    if (!handleMask)
        return;

    std::list<AuraApplication*> effectApplications;
    GetApplicationList(effectApplications);

    for (std::list<AuraApplication*>::const_iterator apptItr = effectApplications.begin(); apptItr != effectApplications.end(); ++apptItr)
        if ((*apptItr)->HasEffect(GetEffIndex()))
            HandleEffect(*apptItr, handleMask, false);

    if (handleMask & AURA_EFFECT_HANDLE_CHANGE_AMOUNT)
    {
        if (!mark)
        {
            m_amount = newAmount;
            GetBase()->UpdateConcatenateAura(GetCaster(), newAmount, m_effIndex);
        }
        else
            SetAmount(newAmount);
    }

    if (handleMask & (AURA_EFFECT_HANDLE_REAPPLY | AURA_EFFECT_HANDLE_CHANGE_AMOUNT))
        CalculateSpellMod();

    for (std::list<AuraApplication*>::const_iterator apptItr = effectApplications.begin(); apptItr != effectApplications.end(); ++apptItr)
        if ((*apptItr)->HasEffect(GetEffIndex()))
            HandleEffect(*apptItr, handleMask, true);
}

void AuraEffect::HandleEffect(AuraApplication * aurApp, uint8 mode, bool apply)
{
    if(!GetBase())
        return;

    // check if call is correct, we really don't want using bitmasks here (with 1 exception)
    ASSERT(mode == AURA_EFFECT_HANDLE_REAL
        || mode == AURA_EFFECT_HANDLE_SEND_FOR_CLIENT
        || mode == AURA_EFFECT_HANDLE_CHANGE_AMOUNT
        || mode == AURA_EFFECT_HANDLE_STAT
        || mode == AURA_EFFECT_HANDLE_SKILL
        || mode == AURA_EFFECT_HANDLE_REAPPLY
        || mode == (AURA_EFFECT_HANDLE_CHANGE_AMOUNT | AURA_EFFECT_HANDLE_REAPPLY));

    // register/unregister effect in lists in case of real AuraEffect apply/remove
    // registration/unregistration is done always before real effect handling (some effect handlers code is depending on this)
    if (mode & AURA_EFFECT_HANDLE_REAL)
        aurApp->GetTarget()->_RegisterAuraEffect(this, apply);

    // real aura apply/remove, handle modifier
    if (mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_REAPPLY))
        ApplySpellMod(aurApp->GetTarget(), apply);

    // call scripts helping/replacing effect handlers
    bool prevented = false;
    if (apply)
        prevented = GetBase()->CallScriptEffectApplyHandlers(const_cast<AuraEffect const*>(this), const_cast<AuraApplication const*>(aurApp), static_cast<AuraEffectHandleModes>(mode));
    else
        prevented = GetBase()->CallScriptEffectRemoveHandlers(const_cast<AuraEffect const*>(this), const_cast<AuraApplication const*>(aurApp), static_cast<AuraEffectHandleModes>(mode));

    // check if script events have removed the aura or if default effect prevention was requested
    if ((apply && aurApp->GetRemoveMode()) || prevented)
        return;

    if (GetAuraType() >= TOTAL_AURAS)
        return;

    (*this.*AuraEffectHandler [GetAuraType()])(const_cast<AuraApplication const*>(aurApp), mode, apply);

    // check if script events have removed the aura or if default effect prevention was requested
    if (apply && aurApp->GetRemoveMode())
        return;

    // call scripts triggering additional events after apply/remove
    if (apply)
        GetBase()->CallScriptAfterEffectApplyHandlers(const_cast<AuraEffect const*>(this), const_cast<AuraApplication const*>(aurApp), static_cast<AuraEffectHandleModes>(mode));
    else
        GetBase()->CallScriptAfterEffectRemoveHandlers(const_cast<AuraEffect const*>(this), const_cast<AuraApplication const*>(aurApp), static_cast<AuraEffectHandleModes>(mode));
}

void AuraEffect::HandleEffect(Unit* target, uint8 mode, bool apply)
{
    AuraApplication* aurApp = GetBase()->GetApplicationOfTarget(target->GetGUID());
    ASSERT(aurApp);
    HandleEffect(aurApp, mode, apply);
}

void AuraEffect::ApplySpellMod(Unit* target, bool apply)
{
    if (!m_spellmod || !target->IsPlayer())
        return;

    target->ToPlayer()->AddSpellMod(m_spellmod, apply);

    // reapply some passive spells after add/remove related spellmods
    // Warning: it is a dead loop if 2 auras each other amount-shouldn't happen
    switch (GetMiscValue())
    {
        case SPELLMOD_ALL_EFFECTS:
        case SPELLMOD_EFFECT1:
        case SPELLMOD_EFFECT2:
        case SPELLMOD_EFFECT3:
        case SPELLMOD_EFFECT4:
        case SPELLMOD_EFFECT5:
        {
            ObjectGuid guid = target->GetGUID();
            // Auras with charges do not mod amount of passive auras
            if (GetBase()->HasAuraAttribute(AURA_ATTR_IS_USING_CHARGES))
                return;

            Unit::AuraApplicationMap & auras = target->GetAppliedAuras();
            for (auto& iter : auras)
            {
                AuraApplicationPtr auraApp = iter.second;
                if (!auraApp || !auraApp->GetBase())
                    continue;

                Aura* aura = auraApp->GetBase();
                // only passive and permament auras-active auras should have amount set on spellcast and not be affected
                // if aura is casted by others, it will not be affected
                if ((aura->IsPassive() || aura->IsPermanent()) && aura->GetCasterGUID() == guid && aura->GetSpellInfo()->IsAffectedBySpellMod(m_spellmod))
                {
                    if (GetMiscValue() == SPELLMOD_ALL_EFFECTS)
                    {
                        for (uint8 i = 0; i<MAX_SPELL_EFFECTS; ++i)
                        {
                            if (AuraEffect* aurEff = aura->GetEffect(i))
                            {
                                aurEff->SetCanBeRecalculated(true);
                                aurEff->RecalculateAmount();
                            }
                        }
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT1)
                    {
                        if (AuraEffect* aurEff = aura->GetEffect(0))
                        {
                            aurEff->SetCanBeRecalculated(true);
                            aurEff->RecalculateAmount();
                        }
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT2)
                    {
                        if (AuraEffect* aurEff = aura->GetEffect(1))
                        {
                            aurEff->SetCanBeRecalculated(true);
                            aurEff->RecalculateAmount();
                        }
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT3)
                    {
                        if (AuraEffect* aurEff = aura->GetEffect(2))
                        {
                            aurEff->SetCanBeRecalculated(true);
                            aurEff->RecalculateAmount();
                        }
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT4)
                    {
                        if (AuraEffect* aurEff = aura->GetEffect(3))
                        {
                            aurEff->SetCanBeRecalculated(true);
                            aurEff->RecalculateAmount();
                        }
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT5)
                    {
                        if (AuraEffect* aurEff = aura->GetEffect(4))
                        {
                            aurEff->SetCanBeRecalculated(true);
                            aurEff->RecalculateAmount();
                        }
                    }
                }
            }
        }
        case SPELLMOD_CASTING_TIME:
        {
            if (apply)
                if (GetAmount() == -100)
                    if (Player* plr = target->ToPlayer())
                    {
                        for (PlayerSpellMap::const_iterator itr = plr->GetSpellMapConst().begin(); itr != plr->GetSpellMapConst().end(); ++itr)
                        {
                            if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled)
                                continue;

                            uint32 spellId = itr->first;
                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

                            if (!spellInfo)
                                continue;

                            if (spellInfo->IsAffectedBySpellMod(m_spellmod) && spellInfo->GetRecoveryTime() <= 300000 && spellInfo->Category.MaxCharges >= 1)
                                plr->RemoveSpellCooldown(spellInfo->Id, true);
                        }
                    }
            break;
        }
        default:
            break;
    }
}

void AuraEffect::Update(uint32 diff, Unit* caster)
{
    if(!GetBase() || GetBase()->IsRemoved())
        return;

    GetBase()->CallScriptEffectUpdateHandlers(diff, this);

    if (m_isPeriodic && (GetBase()->GetDuration() >=0 || GetBase()->IsPassive() || GetBase()->IsPermanent()))
    {
        if (GetBase()->IsPassive() || GetBase()->IsPermanent())
            m_periodicTimer += diff;
        else
            m_periodicTimer += int32(diff) > GetBase()->GetDuration() ? GetBase()->GetDuration() : int32(diff);

        while (m_periodicTimer >= m_period && m_period > 0 && m_periodicTimer)
        {
            m_nowInTick = true;
            m_periodicTimer -= m_period;
            ++m_tickNumber;

            // update before tick (aura can be removed in TriggerSpell or PeriodicTick calls)
            UpdatePeriodic(caster);

            std::list<AuraApplication*> effectApplications;
            GetApplicationList(effectApplications);
            // tick on targets of effects
            for (std::list<AuraApplication*>::const_iterator apptItr = effectApplications.begin(); apptItr != effectApplications.end(); ++apptItr)
                if ((*apptItr)->HasEffect(GetEffIndex()))
                    PeriodicTick(*apptItr, caster, static_cast<SpellEffIndex>(GetEffIndex()));

            RecalculateTickPeriod(caster); // Test
            m_nowInTick = false;
        }
    }
}

void AuraEffect::UpdatePeriodic(Unit* caster)
{
    switch (GetAuraType())
    {
        case SPELL_AURA_PERIODIC_HEAL:
        {
            if (GetSpellInfo()->IsChanneled())
            {
                if (auto target = GetBase()->GetUnitOwner())
                {
                    if (target->IsHostileTo(caster))
                        caster->InterruptSpell(CURRENT_CHANNELED_SPELL, false);
                }
            }
            break;
        }
        case SPELL_AURA_PERIODIC_DUMMY:
            switch (GetSpellInfo()->ClassOptions.SpellClassSet)
            {
                case SPELLFAMILY_GENERIC:
                    switch (GetId())
                    {
                        case 58549: // Tenacity
                        case 59911: // Tenacity (vehicle)
                           GetBase()->RefreshDuration();
                           break;
                        case 66823: case 67618: case 67619: case 67620: // Paralytic Toxin
                            // Get 0 effect aura
                            if (AuraEffect* slow = GetBase()->GetEffect(0))
                            {
                                int32 newAmount = slow->GetAmount() - 10;
                                if (newAmount < -100)
                                    newAmount = -100;
                                slow->ChangeAmount(newAmount);
                            }
                            break;
                        case 167421:    //Khadgar's Watch
                            if (!caster->HasAura(167410))
                                caster->CastSpell(caster, 167410, true, nullptr, this);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
           }
       default:
           break;
    }
    GetBase()->CallScriptEffectUpdatePeriodicHandlers(this);
}

uint32 AuraEffect::GetTickNumber() const
{
    return m_tickNumber;
}

void AuraEffect::SetTickNumber(uint32 tick)
{
    m_tickNumber = tick;
}

uint32 AuraEffect::GetTotalTicks() const
{
    return m_period ? (GetBase()->GetMaxDuration() / m_period) : 1;
}

void AuraEffect::ResetPeriodic(bool resetPeriodicTimer)
{
    if (resetPeriodicTimer && !m_nowInTick) // If now in tick do not increment, prevent freeze
        m_periodicTimer = m_period;
    m_tickNumber = 0;
}

float AuraEffect::GetPeriodMod()
{
    return m_period_mod;
}

void AuraEffect::SetPeriodMod(float mod)
{
    m_period_mod = mod;
}

bool AuraEffect::IsPeriodic() const
{
    return m_isPeriodic;
}

void AuraEffect::SetPeriodic(bool isPeriodic)
{
    m_isPeriodic = isPeriodic;
}

bool AuraEffect::IsAffectingSpell(SpellInfo const* spell) const
{
    if (!spell)
        return false;

    // Check family name
    if (spell->ClassOptions.SpellClassSet != m_spellInfo->ClassOptions.SpellClassSet)
        return false;

    // Check EffectClassMask
    return m_spellInfo->GetEffect(m_effIndex, m_diffMode)->SpellClassMask & spell->ClassOptions.SpellClassMask;
}

bool AuraEffect::HasSpellClassMask() const
{
    if (m_spellInfo)
        return m_spellInfo->GetEffect(m_effIndex, m_diffMode)->SpellClassMask;
    return false;
}

flag128 AuraEffect::GetSpellClassMask() const
{
    if (m_spellInfo)
        return m_spellInfo->GetEffect(m_effIndex, m_diffMode)->SpellClassMask;
    return 0;
}

void AuraEffect::SendTickImmune(Unit* target, Unit* caster) const
{
    if (caster)
        caster->SendSpellDamageImmune(target, m_spellInfo->Id, true);
}

void AuraEffect::PeriodicTick(AuraApplication * aurApp, Unit* caster, SpellEffIndex effIndex) const
{
    bool prevented = GetBase()->CallScriptEffectPeriodicHandlers(this, aurApp);
    if (prevented)
        return;

    Unit* target = aurApp->GetTarget();

    if (!AuraCostPower(caster))
        return;

    AuraSpellTrigger(target, caster, effIndex);

    switch (GetAuraType())
    {
        case SPELL_AURA_PERIODIC_DUMMY:
            HandlePeriodicDummyAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
            HandlePeriodicTriggerSpellAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
            HandlePeriodicTriggerSpellWithValueAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            HandlePeriodicDamageAurasTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_LEECH:
            HandlePeriodicHealthLeechAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            HandlePeriodicHealthFunnelAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
            HandlePeriodicHealAurasTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_MANA_LEECH:
            HandlePeriodicManaLeechAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_OBS_MOD_POWER:
            HandleObsModPowerAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_ENERGIZE:
            HandlePeriodicEnergizeAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_POWER_BURN:
            HandlePeriodicPowerBurnAuraTick(target, caster, effIndex);
            break;
        case SPELL_AURA_PERIODIC_WEAPON_PERCENT_DAMAGE:
            HandlePeriodicWeaponPercentDamageAurasTick(target, caster, effIndex);
            break;
        default:
            break;
    }
}

void AuraEffect::RecalculateTickPeriod(Unit* caster)
{
    if (m_period)
    {
        m_period = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->ApplyAuraPeriod;
        GetBase()->CallScriptEffectCalcPeriodicHandlers(this, m_isPeriodic, m_period);

        if (caster)
        {
            // Recalc periodic time
            if (Player* modOwner = caster->GetSpellModOwner())
                modOwner->ApplySpellMod(GetId(), SPELLMOD_ACTIVATION_TIME, m_period);

            // Haste modifies periodic time of channeled spells
            if (m_spellInfo->IsChanneled())
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME))
                    caster->ModSpellCastTime(m_spellInfo, m_period);
            }
            else
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME))
                    m_period = int32(m_period * caster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE));
            }
            if (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION))
                m_period = int32(m_period * caster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE));
        }

        m_period += m_period_mod;
        m_period += m_activation_time;
    }
}

void AuraEffect::HandleProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex effIndex)
{
    bool prevented = GetBase()->CallScriptEffectProcHandlers(this, const_cast<AuraApplication const*>(aurApp), eventInfo);
    if (prevented)
        return;

    switch (GetAuraType())
    {
        case SPELL_AURA_PROC_TRIGGER_SPELL:
            HandleProcTriggerSpellAuraProc(aurApp, eventInfo, effIndex);
            break;
        case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
            HandleProcTriggerSpellWithValueAuraProc(aurApp, eventInfo, effIndex);
            break;
        case SPELL_AURA_PROC_TRIGGER_DAMAGE:
            HandleProcTriggerDamageAuraProc(aurApp, eventInfo, effIndex);
            break;
        default:
            break;
    }
}

void AuraEffect::CleanupTriggeredSpells(Unit* target)
{
    uint32 tSpellId = m_spellInfo->GetEffect(GetEffIndex(), m_diffMode)->TriggerSpell;
    if (!tSpellId)
        return;

    SpellInfo const* tProto = sSpellMgr->GetSpellInfo(tSpellId);
    if (!tProto)
        return;

    if (tProto->GetDuration(m_diffMode) != -1)
        return;

    // needed for spell 43680, maybe others
    // TODO: is there a spell flag, which can solve this in a more sophisticated way?
    if (m_spellInfo->GetEffect(GetEffIndex(), m_diffMode)->ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
    {
        if (uint32(m_spellInfo->GetDuration(m_diffMode)) == m_spellInfo->GetEffect(GetEffIndex(), m_diffMode)->ApplyAuraPeriod)
            return;

        if (tProto->GetAuraOptions(m_diffMode)->CumulativeAura)
            if (GetTotalTicks() < static_cast<int32>(tProto->GetAuraOptions(m_diffMode)->CumulativeAura))
                return;
    }

    target->RemoveAurasDueToSpell(tSpellId, GetCasterGUID());
}

void AuraEffect::HandleShapeshiftBoosts(Unit* target, bool apply) const
{
    std::vector<uint32> spellId;

    switch (GetMiscValue())
    {
        case FORM_CAT:
            spellId.push_back(3025);     // Wild Charge
            spellId.push_back(48629);    // Swipe, Mangle, Thrash
            spellId.push_back(106840);   // Skull Bash, Stampeding Roar, Berserk
            if (!apply || target->HasAura(108299))
                spellId.push_back(108300);
            break;
        case FORM_TRAVEL:
            spellId.push_back(5419);
            break;
        case FORM_AQUA:
            spellId.push_back(5421);
            break;
        case FORM_BEAR:
            spellId.push_back(1178);
            spellId.push_back(21178);  // Swipe, Wild Charge
            spellId.push_back(106829); // Mangle, Thrash, Skull Bash
            spellId.push_back(106899); // Stampeding Roar, Berserk
            if (!apply || target->HasAura(108299))
                spellId.push_back(108300);
            if (!apply || target->HasAura(236019)) // Tooth and Claw (PvP Talent)
                spellId.push_back(236440);
            if (!apply || target->HasAura(233754)) // Celestial Guardian (PvP Talent)
                spellId.push_back(234081);
            break;
        case FORM_MOONKIN:
            // Glyph of the Start
            if (!apply || target->HasAura(114301))
                spellId.push_back(114302);  // Astral Form
            break;
        case FORM_FLIGHT:
            spellId.push_back(33948);
            spellId.push_back(34764);
            break;
        case FORM_FLIGHT_EPIC:
            spellId.push_back(40122);
            spellId.push_back(40121);
            break;
        case FORM_SPIRITOFREDEMPTION:
        {
            if (GetSpellInfo()->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD))
            {
                spellId.push_back(27792);
                spellId.push_back(27795);                               // must be second, this important at aura remove to prevent to early iterator invalidation.
            }
            break;
        }
        default:
            break;
    }

    if (apply)
    {
        if (Player* plrTarget = target->ToPlayer())
        {
            // Remove cooldown of spells triggered on stance change - they may share cooldown with stance spell
            for (auto& itr : spellId)
            {
                plrTarget->RemoveSpellCooldown(itr);
                plrTarget->CastSpell(target, itr, true, nullptr, this);
            }

            PlayerSpellMap const& sp_list = plrTarget->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled)
                    continue;

                bool hasAnalogy = false;

                for (auto& i : spellId)
                    if (itr->first == i)
                    {
                        hasAnalogy = true;
                        break;
                    }

                if (hasAnalogy)
                    continue;

                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
                if (!spellInfo || !(spellInfo->GetMisc(m_diffMode)->MiscData.Attributes[0] & (SPELL_ATTR0_PASSIVE | SPELL_ATTR0_HIDDEN_CLIENTSIDE)))
                    continue;

                if (spellInfo->Shapeshift.ShapeshiftMask & (UI64LIT(1) << (GetMiscValue() - 1)))
                    target->CastSpell(target, itr->first, true, nullptr, this);
            }

            switch (GetMiscValue())
            {
                case FORM_CAT:
                    // Savage Roar
                    if (target->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_DRUID, 0, 0x10000000, 0))
                        target->CastSpell(target, 62071, true);
                    break;
            }
        }
    }
    else
    {
        for (auto& itr : spellId)
            target->RemoveAurasDueToSpell(itr);

        AuraEffect* newAura = nullptr;
        // Iterate through all the shapeshift auras that the target has, if there is another aura with SPELL_AURA_MOD_SHAPESHIFT, then this aura is being removed due to that one being applied
        if (Unit::AuraEffectList* shapeshifts = target->GetAuraEffectsByType(SPELL_AURA_MOD_SHAPESHIFT))
        {
            for (Unit::AuraEffectList::iterator itr = shapeshifts->begin(); itr != shapeshifts->end(); ++itr)
            {
                if ((*itr) != this)
                {
                    newAura = *itr;
                    break;
                }
            }
        }
        Unit::AuraApplicationMap& tAuras = target->GetAppliedAuras();
        for (auto itr = tAuras.begin(); itr != tAuras.end();)
        {
            // Use the new aura to see on what stance the target will be
            uint64 newStance = newAura ? (UI64LIT(1) << (newAura->GetMiscValue() - 1)) : 0;

            // If the stances are not compatible with the spell, remove it
            if (itr->second->GetBase()->IsRemovedOnShapeLost(target) && !(itr->second->GetBase()->GetSpellInfo()->Shapeshift.ShapeshiftMask & newStance) && itr->second->GetBase()->GetSpellInfo()->_IsPositiveSpell())
                target->RemoveAura(itr);
            else
                ++itr;
        }

        switch (GetMiscValue())
        {
            case FORM_BEAR:
                target->SetPower(POWER_RAGE, 0);
                break;
        }
    }
}

void AuraEffect::HandleModInvisibilityDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    auto type = InvisibilityType(GetMiscValue());

    if (apply)
    {
        target->m_invisibilityDetect.AddFlag(type);
        target->m_invisibilityDetect.AddValue(type, GetAmount());
    }
    else
    {
        if (!target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY_DETECT))
            target->m_invisibilityDetect.DelFlag(type);

        target->m_invisibilityDetect.AddValue(type, -GetAmount());
        //then exit from visibility try to link posible off-line thread state for some atackers npc.
        target->getHostileRefManager().UpdateVisibility();
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModInvisibility(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    auto type = InvisibilityType(GetMiscValue());

    if (apply)
    {
        // apply glow vision
        //! spell 164042 used for Q34445. Is glow apply on alll or not?
        if (target->IsPlayer() && m_spellInfo->Id != 164042 && m_spellInfo->Id  != 165052)
           target->SetByteFlag(PLAYER_FIELD_BYTES_7, PLAYER_BYTES_5_AURA_VISION, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

        target->m_invisibility.AddFlag(type);
        target->m_invisibility.AddValue(type, GetAmount());
    }
    else
    {
        if (!target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
        {
            // if not have different invisibility auras.
            // remove glow vision
            if (target->IsPlayer())
                target->RemoveByteFlag(PLAYER_FIELD_BYTES_7, PLAYER_BYTES_5_AURA_VISION, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            target->m_invisibility.DelFlag(type);
        }
        else
        {
            bool found = false;
            if (Unit::AuraEffectList const* invisAuras = target->GetAuraEffectsByType(SPELL_AURA_MOD_INVISIBILITY))
            {
                for (Unit::AuraEffectList::const_iterator i = invisAuras->begin(); i != invisAuras->end(); ++i)
                {
                    if (GetMiscValue() == (*i)->GetMiscValue())
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
                target->m_invisibility.DelFlag(type);
        }

        target->m_invisibility.AddValue(type, -GetAmount());
    }

    // call functions which may have additional effects after chainging state of unit
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        // drop flag at invisibiliy in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION, GetId());
    }
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModStealthDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    auto type = StealthType(GetMiscValue());

    if (apply)
    {
        target->m_stealthDetect.AddFlag(type);
        target->m_stealthDetect.AddValue(type, GetAmount());
    }
    else
    {
        if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH_DETECT))
            target->m_stealthDetect.DelFlag(type);

        target->m_stealthDetect.AddValue(type, -GetAmount());
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModStealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    auto type = StealthType(GetMiscValue());

    if (apply)
    {
        target->m_stealth.AddFlag(type);
        target->m_stealth.AddValue(type, GetAmount());

        target->SetStandVisFlags(UNIT_STAND_FLAGS_CREEP);
        if (target->IsPlayer())
            target->SetByteFlag(PLAYER_FIELD_BYTES_7, PLAYER_BYTES_5_AURA_VISION, PLAYER_FIELD_BYTE2_STEALTH);
    }
    else
    {
        target->m_stealth.AddValue(type, -GetAmount());

        if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH)) // if last SPELL_AURA_MOD_STEALTH
        {
            target->m_stealth.DelFlag(type);

            target->RemoveStandVisFlags(UNIT_STAND_FLAGS_CREEP);
            if (target->IsPlayer())
                target->RemoveByteFlag(PLAYER_FIELD_BYTES_7, PLAYER_BYTES_5_AURA_VISION, PLAYER_FIELD_BYTE2_STEALTH);
        }
    }

    // call functions which may have additional effects after chainging state of unit
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        // drop flag at stealth in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION, GetId());
    }
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModStealthLevel(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    auto type = StealthType(GetMiscValue());

    if (apply)
        target->m_stealth.AddValue(type, GetAmount());
    else
        target->m_stealth.AddValue(type, -GetAmount());

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleAllowTalentSwapping(AuraApplication const * aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ALLOW_CHANGING_TALENTS);
    }
    else
    {
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ALLOW_CHANGING_TALENTS);
    }
}

void AuraEffect::HandleSpiritOfRedemption(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    bool isSpell = false;

    if (SpellInfo const* spellInfo = aurApp->GetBase()->GetSpellInfo())
        isSpell = spellInfo->Categories.StartRecoveryCategory;

    if (!target->IsPlayer())
        return;

    // prepare spirit state
    if (apply)
    {
        if (target->IsPlayer())
        {
            // disable breath/etc timers
            target->ToPlayer()->StopMirrorTimers();

            // set stand state (expected in this form)
            if (!target->IsStandState())
                target->SetStandState(UNIT_STAND_STATE_STAND);
        }

        if (!isSpell)
            target->SetHealth(1);
    }
    // die at aura end
    else if (target->isAlive())
    {
        if(AuraEffect const* aurEff = target->GetAuraEffect(211336, EFFECT_0)) // Need calculate percete for ressurect
        {
            target->ToPlayer()->ResurrectPlayer(1.0f);
            aurEff->GetBase()->Remove();
            target->CastSpell(target, 211319, true);
        }
        else if (!isSpell)
            // call functions which may have additional effects after chainging state of unit
            target->setDeathState(JUST_DIED);
    }

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    else
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
}

void AuraEffect::HandleAuraGhost(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        if (target->IsPlayer())
            target->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST);

        target->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
        target->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
    }
    else
    {
        if (target->HasAuraType(SPELL_AURA_GHOST))
            return;

        if (target->IsPlayer())
            target->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST);

        target->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
        target->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
    }
    target->UpdateObjectVisibility();
}

void AuraEffect::HandlePhase(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();


    if (Player* player = target->ToPlayer())
    {
        if (apply)    
            player->GetPhaseMgr().RegisterPhasingAuraEffect(this);    
        else    
            player->GetPhaseMgr().UnRegisterPhasingAuraEffect(this);
    }
    else
    {
        uint32 newPhase = 0;
        if (Unit::AuraEffectList const* phases = target->GetAuraEffectsByType(SPELL_AURA_PHASE))
            for (Unit::AuraEffectList::const_iterator itr = phases->begin(); itr != phases->end(); ++itr)    
                newPhase |= (*itr)->GetMiscValue();

        if (Unit::AuraEffectList const* phases2 = target->GetAuraEffectsByType(SPELL_AURA_PHASE_2))
            for (Unit::AuraEffectList::const_iterator itr = phases2->begin(); itr != phases2->end(); ++itr)    
                newPhase |= (*itr)->GetMiscValue();

        if (!newPhase)
        {
            newPhase = PHASEMASK_NORMAL;
            if (Creature* creature = target->ToCreature())
                if (CreatureData const* data = sObjectMgr->GetCreatureData(creature->GetDBTableGUIDLow()))
                    newPhase = data->phaseMask;
        }

        target->SetPhaseMask(newPhase, false);
    }

    // call functions which may have additional effects after chainging state of unit
    // phase auras normally not expected at BG but anyway better check
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        // drop flag at invisibiliy in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION, GetId());
    }

    // need triggering visibility update base at phase update of not GM invisible (other GMs anyway see in any phases)
    if (target->IsVisible())
        target->UpdateObjectVisibility();
}

/**********************/
/***   UNIT MODEL   ***/
/**********************/

void AuraEffect::HandleAuraModShapeshift(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->RemoveAurasByType(SPELL_AURA_CLONE_CASTER);

    uint32 modelid = 0;
    Powers PowerType = POWER_MANA;
    auto form = ShapeshiftForm(GetMiscValue());

    switch (form)
    {
        case FORM_FIERCE_TIGER:
        case FORM_STURDY_OX:
        case FORM_CAT:                                      // 0x01
        case FORM_GHOUL:                                    // 0x07
            PowerType = POWER_ENERGY;
            break;
        case FORM_BEAR:                                     // 0x05
        case FORM_BATTLESTANCE:                             // 0x11
        case FORM_DEFENSIVESTANCE:                          // 0x12
        case FORM_BERSERKERSTANCE:                          // 0x13
        case FORM_GLADIATORSTANCE:                          // 0x21
            PowerType = POWER_RAGE;
            break;
        case FORM_TREE:                                     // 0x02
        case FORM_TRAVEL:                                   // 0x03
        case FORM_AQUA:                                     // 0x04
        case FORM_AMBIENT:                                  // 0x06
        case FORM_THARONJA_SKELETON:                        // 0x0A
        case FORM_TEST_OF_STRENGTH:                         // 0x0B
        case FORM_BLB_PLAYER:                               // 0x0C
        case FORM_SHADOW_DANCE:                             // 0x0D
        case FORM_CREATUREBEAR:                             // 0x0E
        case FORM_CREATURECAT:                              // 0x0F
        case FORM_GHOSTWOLF:                                // 0x10
            break;
        case FORM_SPIRITED_CRANE:                           // 0x09
        case FORM_WISE_SERPENT:                             // 0x14
            PowerType = POWER_MANA;
            break;
        case FORM_ZOMBIE:                                   // 0x15
        case FORM_METAMORPHOSIS:                            // 0x16
        case FORM_UNDEAD:                                   // 0x19
        case FORM_MASTER_ANGLER:                            // 0x1A
        case FORM_FLIGHT_EPIC:                              // 0x1B
        case FORM_SHADOW:                                   // 0x1C
        case FORM_FLIGHT:                                   // 0x1D
        case FORM_STEALTH:                                  // 0x1E
        case FORM_MOONKIN:                                  // 0x1F
        case FORM_SPIRITOFREDEMPTION:                       // 0x20
        case FORM_WISP_FORM:                                // 0x27
            break;
        default:
            TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Auras: Unknown Shapeshift Type: %u", GetMiscValue());
    }

    modelid = target->GetModelForForm(form);

    if (apply)
    {
        // remove polymorph before changing display id to keep new display id
        switch (form)
        {
            case FORM_CAT:
            case FORM_TREE:
            case FORM_TRAVEL:
            case FORM_AQUA:
            case FORM_BEAR:
            case FORM_FLIGHT_EPIC:
            case FORM_FLIGHT:
            case FORM_MOONKIN:
            case FORM_MOONKIN2:
            {
                // remove movement affects
                uint32 mechanicMask = (1 << MECHANIC_SNARE | 1 << MECHANIC_ROOT);
                target->RemoveAurasWithMechanic(mechanicMask);

                // and polymorphic affects
                if (target->IsPolymorphed())
                    target->RemoveAurasDueToSpell(target->getTransForm());
                break;
            }
            default:
               break;
        }

        // remove other shapeshift before applying a new one
        target->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT, ObjectGuid::Empty, GetBase());

        // stop handling the effect if it was removed by linked event
        if (aurApp->GetRemoveMode())
            return;

        if (modelid > 0)
            target->SetDisplayId(modelid);

        if (PowerType != POWER_MANA)
        {
            //int32 oldPower = target->GetPower(PowerType);
            // reset power to default values only at power change
            if (target->getPowerType() != PowerType)
                target->SetFieldPowerType(PowerType);
        }

        // stop handling the effect if it was removed by linked event
        if (aurApp->GetRemoveMode())
            return;

        target->SetShapeshiftForm(form);
    }
    else
    {
        // reset model id if no other auras present
        // may happen when aura is applied on linked event on aura removal
        if (!target->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        {
            target->SetShapeshiftForm(FORM_NONE);
            if (target->getClass() == CLASS_DRUID)
            {
                target->setPowerType(POWER_MANA);
                // Remove movement impairing effects also when shifting out
                target->RemoveMovementImpairingEffects();
            }
        }

        if (modelid > 0)
            target->RestoreDisplayId();

        switch (form)
        {
            case FORM_BEAR:
            {
                target->AddSpellCooldown(5487, 0, getPreciseTime() + 1.5);
                target->SendSpellCooldown(5487, 0, 1500);
                break;
            }
            case FORM_CAT:
            {
                target->AddSpellCooldown(768, 0, getPreciseTime() + 1.5);
                target->SendSpellCooldown(768, 0, 1500);
                break;
            }
            case FORM_MOONKIN:
            {
                target->AddSpellCooldown(24858, 0, getPreciseTime() + 1.5);
                target->SendSpellCooldown(24858, 0, 1500);
                break;
            }
            case FORM_MOONKIN2:
            {
                target->AddSpellCooldown(197625, 0, getPreciseTime() + 1.5);
                target->SendSpellCooldown(197625, 0, 1500);
                break;
            }
            case FORM_TREE:
            {
                target->AddSpellCooldown(114282, 0, getPreciseTime() + 1.5);
                target->SendSpellCooldown(114282, 0, 1500);
                break;
            }
            case FORM_TRAVEL:
            case FORM_AQUA:
            case FORM_FLIGHT_EPIC:
            case FORM_FLIGHT:
            {
                target->AddSpellCooldown(783, 0, getPreciseTime() + 1.5);
                target->AddSpellCooldown(210053, 0, getPreciseTime() + 1.5);
                target->SendSpellCooldown(783, 0, 1500);
                target->SendSpellCooldown(210053, 0, 1500);
                break;
            }
            default:
                break;
        }
    }

    // adding/removing linked auras
    // add/remove the shapeshift aura's boosts
    HandleShapeshiftBoosts(target, apply);

    if (target->IsPlayer())
        target->ToPlayer()->InitDataForForm();

    if (target->getClass() == CLASS_DRUID)
    {
        // Disarm handling
        // If druid shifts while being disarmed we need to deal with that since forms aren't affected by disarm
        // and also HandleAuraModDisarm is not triggered
        if (!target->CanUseAttackType(BASE_ATTACK))
            if (Item* pItem = target->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
                target->ToPlayer()->_ApplyWeaponDamage(EQUIPMENT_SLOT_MAINHAND, pItem, apply);
    }

    // stop handling the effect if it was removed by linked event
    if (apply && aurApp->GetRemoveMode())
        return;

    if (target->IsPlayer())
    {
        SpellShapeshiftFormEntry const* shapeInfo = sSpellShapeshiftFormStore.LookupEntry(form);
        // Learn spells for shapeshift form - no need to send action bars or add spells to spellbook
        for (auto parentSpell : shapeInfo->PresetSpellID)
        {
            if (!parentSpell)
                continue;

            if (apply)
                target->ToPlayer()->AddTemporarySpell(parentSpell);
            else
                target->ToPlayer()->RemoveTemporarySpell(parentSpell);
        }
    }
}

void AuraEffect::HandleAuraTransform(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // Remove aura from invisible model
    target->RemoveAurasByType(SPELL_AURA_CLONE_CASTER);

    if (apply)
    {
        // update active transform spell only when transform or shapeshift not set or not overwriting negative by positive case
        SpellInfo const* transformSpellInfo = sSpellMgr->GetSpellInfo(target->getTransForm());
        if (!transformSpellInfo || !GetSpellInfo()->IsPositive() || transformSpellInfo->IsPositive())
        //if (!target->GetModelForForm(target->GetShapeshiftForm()) || !GetSpellInfo()->IsPositive())
        {
            target->setTransForm(GetId());

            // Prevent visual bug with mirrors image
            if (target->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE))
                target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);

            // special case (spell specific functionality)
            if (GetMiscValue() == 0)
            {
                switch (GetId())
                {
                    // Orb of Deception
                    case 16739:
                    {
                        if (!target->IsPlayer())
                            return;

                        switch (target->getRace())
                        {
                            // Blood Elf
                            case RACE_BLOODELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 17829 : 17830);
                                break;
                            // Orc
                            case RACE_ORC:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10139 : 10140);
                                break;
                            // Troll
                            case RACE_TROLL:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10135 : 10134);
                                break;
                            // Tauren
                            case RACE_TAUREN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10136 : 10147);
                                break;
                            // Undead
                            case RACE_UNDEAD_PLAYER:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10146 : 10145);
                                break;
                            // Draenei
                            case RACE_DRAENEI:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 17827 : 17828);
                                break;
                            // Dwarf
                            case RACE_DWARF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10141 : 10142);
                                break;
                            // Gnome
                            case RACE_GNOME:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10148 : 10149);
                                break;
                            // Human
                            case RACE_HUMAN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10137 : 10138);
                                break;
                            // Night Elf
                            case RACE_NIGHTELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10143 : 10144);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    // Murloc costume
                    case 42365:
                        target->SetDisplayId(21723);
                        break;
                    // Dread Corsair
                    case 50517:
                    // Blood Elf Illusion
                    case 160331:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 20578 : 20579);
                        break;
                    // Jewel of Hellfire
                    case 187174:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 63135 : 63137);
                        break;
                    // Corsair Costume
                    case 51926:
                    {
                        if (!target->IsPlayer())
                            return;

                        switch (target->getRace())
                        {
                            // Blood Elf
                            case RACE_BLOODELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25032 : 25043);
                                break;
                            // Orc
                            case RACE_ORC:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25039 : 25050);
                                break;
                            // Troll
                            case RACE_TROLL:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25041 : 25052);
                                break;
                            // Tauren
                            case RACE_TAUREN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25040 : 25051);
                                break;
                            // Undead
                            case RACE_UNDEAD_PLAYER:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25042 : 25053);
                                break;
                            // Draenei
                            case RACE_DRAENEI:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25033 : 25044);
                                break;
                            // Dwarf
                            case RACE_DWARF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25034 : 25045);
                                break;
                            // Gnome
                            case RACE_GNOME:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25035 : 25046);
                                break;
                            // Human
                            case RACE_HUMAN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25037 : 25048);
                                break;
                            // Night Elf
                            case RACE_NIGHTELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25038 : 25049);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    // Pygmy Oil
                    case 53806:
                        target->SetDisplayId(14973);
                        break;
                    // Nightborne Disguise
                    case 202477:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 69518 : 69517);
                        break;
                    // Honor the Dead
                    case 65386:
                    case 65495:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 29203 : 29204);
                        break;
                    // Darkspear Pride
                    case 75532:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 31737 : 31738);
                        break;
                    // Gnomeregan Pride
                    case 75531:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 31654 : 31655);
                        break;
                    //Coin of Many Faces
                    case 192225:
                    {
                          uint32 rand_morph[58] = {
                          58282,61404,58262,58786,61515,
                          25807,60237,61228,60591,61309,
                          60462,60472,61505,61333,58677,
                          36699,60484,61372,58425,60480,
                          60522,60606,61395,60409,60031,
                          60557,27417,57249,58430,60494,
                          60597,56513,57276,60036,57246,
                          61508,60540,57191,60488,61390,
                          60405,60602,57242,59934,58325,
                          57172,58368,61347,58432,61393,
                          61368,61313,57421,60077,39715,
                          60495,57077,61390 };

                        target->SetDisplayId(rand_morph[urand(0, 57)]);
                        break;
                    }
                    default:
                        break;
                }
            }
            else
            {
                CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(GetMiscValue());
                if (!ci)
                {
                    target->SetDisplayId(16358);              // pig pink ^_^
                    TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Auras: unknown creature id = %d (only need its modelid) From Spell Aura Transform in Spell ID = %d", GetMiscValue(), GetId());
                }
                else
                {
                    uint32 model_id = 0;

                    if (uint32 modelid = sObjectMgr->GetCreatureDisplay(ci->GetRandomValidModelId()))
                        model_id = modelid;                     // Will use the default model here

                    target->SetDisplayId(model_id);

                    // Dragonmaw Illusion (set mount model also)
                    if (GetId() == 42016 && target->GetMountID() && target->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                        target->SetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID, 16314);
                }
            }
        }

        // polymorph case
        if ((mode & AURA_EFFECT_HANDLE_REAL) && target->IsPlayer() && target->IsPolymorphed())
        {
            // for players, start regeneration after 1s (in polymorph fast regeneration case)
            // only if caster is Player (after patch 2.4.2)
            if (GetCasterGUID().IsPlayer())
                target->ToPlayer()->setPowerCombatTimer(POWER_RAGE, 1*IN_MILLISECONDS);

            //dismount polymorphed target (after patch 2.4.2)
            if (target->IsMounted())
                target->RemoveAurasByType(SPELL_AURA_MOUNTED);
            //remove fly aura Zen Flight
            if (target->HasAura(125883))
                target->RemoveAurasDueToSpell(125883, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
        }
    }
    else
    {
        // HandleEffect(this, AURA_EFFECT_HANDLE_SEND_FOR_CLIENT, true) will reapply it if need
        if (target->getTransForm() == GetId())
            target->setTransForm(0);

        target->RestoreDisplayId();

        // Dragonmaw Illusion (restore mount model)
        if (GetId() == 42016 && target->GetMountID() == 16314)
        {
            Unit::AuraEffectList const* mountAuras = target->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
            if (mountAuras && mountAuras->begin() != mountAuras->end())
            {
                uint32 cr_id = (*mountAuras->begin())->GetMiscValue();
                if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(cr_id))
                {
                    uint32 team = 0;
                    if (target->IsPlayer())
                        team = target->ToPlayer()->GetTeam();

                    uint32 displayID = sObjectMgr->GetCreatureDisplay(sObjectMgr->ChooseDisplayId(team, ci));
                    sObjectMgr->GetCreatureModelRandomGender(&displayID);

                    target->SetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID, displayID);
                }
            }
        }
    }
}

void AuraEffect::HandleAuraModScale(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if(GetAmount() > 0)
        target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE, GetAmount(), apply);
    else
        target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE, -GetAmount(), !apply);
}

void AuraEffect::HandleAuraCloneCaster(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        Unit* caster = GetCaster();
        if (!caster || caster == target)
            return;
        
        // this need for donate morph
        if (caster->IsPlayer() && caster->GetCustomDisplayId() && (caster->GetDisplayId() == caster->GetCustomDisplayId()))
        {
            if (target->IsPlayer())
                return;
            target->SetObjectScale(caster->GetFloatValue(OBJECT_FIELD_SCALE));
        }

        target->InitMirrorImageData(caster);

        // What must be cloned? at least display and scale
        target->SetDisplayId(caster->GetDisplayId());
        //target->SetObjectScale(caster->GetFloatValue(OBJECT_FIELD_SCALE)); // we need retail info about how scaling is handled (aura maybe?)
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
    else
    {
        target->ClearMirrorImageData();
        target->RestoreDisplayId();
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
}

void AuraEffect::HandleAuraInitializeImages(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        Unit* caster = GetCaster();
        if (!caster || caster == target)
            return;

        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
    else
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
}

void AuraEffect::HandleAuraModExpertise(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->IsPlayer())
        target->ToPlayer()->UpdateExpertise();
}

/************************/
/***      FIGHT       ***/
/************************/

void AuraEffect::HandleFeignDeath(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        UnitList targets;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(target, target, target->GetMap()->GetVisibilityRange());
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(target, targets, u_check);
        Trinity::VisitNearbyObject(target, target->GetMap()->GetVisibilityRange(), searcher);
        for (auto& iter : targets)
        {
            if (!iter->HasUnitState(UNIT_STATE_CASTING))
                continue;

            for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
                if (iter->GetCurrentSpell(i) && iter->GetCurrentSpell(i)->m_targets.GetUnitTargetGUID() == target->GetGUID())
                    iter->InterruptSpell(CurrentSpellTypes(i), false);
        }

        target->CombatStop();
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION, GetId());

        // prevent interrupt message
        if (GetCasterGUID() == target->GetGUID() && target->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            target->FinishSpell(CURRENT_GENERIC_SPELL, false);
        target->InterruptNonMeleeSpells(true);
        target->getHostileRefManager().deleteReferences();

        // stop handling the effect if it was removed by linked event
        if (aurApp->GetRemoveMode())
            return;

        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_EMOTES);
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        target->SetFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_FEIGN_DEATH);
        target->AddUnitState(UNIT_STATE_DIED);

        if (auto creature = target->ToCreature())
        {
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            creature->StopAttack();
            creature->RemoveAurasAllDots();
        }
    }
    else
    {
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_EMOTES);
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        target->RemoveFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_FEIGN_DEATH);
        target->ClearUnitState(UNIT_STATE_DIED);

        if (auto creature = target->ToCreature())
        {
            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            creature->InitializeReactState();
        }

        Powers power = target->getPowerType();
        target->SetPower(power, target->GetPower(power));
    }
}

void AuraEffect::HandleModUnattackable(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
    if (!apply && target->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
        return;

    target->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, apply);

    // call functions which may have additional effects after chainging state of unit
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        target->CombatStop();
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION, GetId());
    }
}

void AuraEffect::HandleAuraModDisarm(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    AuraType type = GetAuraType();

    //Prevent handling aura twice
    if ((apply) ? target->GetAuraTypeCount(type) > 1 : target->HasAuraType(type))
        return;

    uint32 field, flag, slot;
    WeaponAttackType attType;
    switch (type)
    {
    case SPELL_AURA_MOD_DISARM:
        field=UNIT_FIELD_FLAGS;
        flag=UNIT_FLAG_DISARMED;
        slot=EQUIPMENT_SLOT_MAINHAND;
        attType=BASE_ATTACK;
        break;
    case SPELL_AURA_MOD_DISARM_OFFHAND:
        field=UNIT_FIELD_FLAGS_2;
        flag=UNIT_FLAG2_DISARM_OFFHAND;
        slot=EQUIPMENT_SLOT_OFFHAND;
        attType=OFF_ATTACK;
        break;
    case SPELL_AURA_MOD_DISARM_RANGED:
        /*field=UNIT_FIELD_FLAGS_2;
        flag=UNIT_FLAG2_DISARM_RANGED;
        slot=EQUIPMENT_SLOT_MAINHAND;
        attType=RANGED_ATTACK;
        break*/
    default:
        return;
    }

    // if disarm aura is to be removed, remove the flag first to reapply damage/aura mods
    if (!apply)
        target->RemoveFlag(field, flag);

    // Handle damage modification, shapeshifted druids are not affected
    if (target->IsPlayer() && !target->IsInFeralForm())
        if (Item* pItem = target->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            if (Player::GetAttackBySlot(slot) < MAX_ATTACK)
                target->ToPlayer()->_ApplyWeaponDamage(slot, pItem, !apply);

    // if disarm effects should be applied, wait to set flag until damage mods are unapplied
    if (apply)
        target->SetFlag(field, flag);

    if (target->IsCreature() && target->ToCreature()->GetCurrentEquipmentId())
        target->UpdateDamagePhysical(attType);
}

void AuraEffect::HandleAuraModSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);

        // call functions which may have additional effects after chainging state of unit
        // Stop cast only spells vs PreventionType & SPELL_PREVENTION_TYPE_SILENCE
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
            if (Spell* spell = target->GetCurrentSpell(CurrentSpellTypes(i)))
                if (spell->m_spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE)
                    // Stop spells on prepare or casting state
                    target->InterruptSpell(CurrentSpellTypes(i), false);
    }
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(SPELL_AURA_MOD_SILENCE) || target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            return;

        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void AuraEffect::HandleAuraModPacify(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(SPELL_AURA_MOD_PACIFY) || target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    }
}

void AuraEffect::HandleAuraModNoActions(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_NO_ACTIONS);

        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
            if (Spell* spell = target->GetCurrentSpell(CurrentSpellTypes(i)))
                if (spell->m_spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_NO_ACTIONS)
                    target->InterruptSpell(CurrentSpellTypes(i), false);
    }
    else if (!target->HasAuraType(SPELL_AURA_MOD_NO_ACTIONS))
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_NO_ACTIONS);
}

void AuraEffect::HandleAuraModPacifyAndSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // Vengeance of the Blue Flight (TODO: REMOVE THIS!)
    if (m_spellInfo->Id == 45839)
    {
        if (apply)
            target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        else
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            return;
    }

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        target->CastStop();
    }
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(SPELL_AURA_MOD_PACIFY) || target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED|UNIT_FLAG_SILENCED);
    }
}

void AuraEffect::HandleAuraAllowOnlyAbility(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->IsPlayer())
    {
        if (apply)
            target->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
        else
        {
            // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
            if (target->HasAuraType(SPELL_AURA_ALLOW_ONLY_ABILITY))
                return;
            target->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
        }
    }
}

void AuraEffect::HandleAuraTrackCreatures(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    uint32 mask = 1 << (GetMiscValue() - 1);
    target->ApplyModFlag(PLAYER_FIELD_TRACK_CREATURE_MASK, mask, apply);
}

void AuraEffect::HandleAuraTrackResources(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    target->SetUInt32Value(PLAYER_FIELD_TRACK_RESOURCE_MASK, apply ? static_cast<uint32>(1)<<(GetMiscValue()-1): 0);
}

void AuraEffect::HandleAuraTrackStealthed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    if (!(apply))
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    target->ApplyModFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_TRACK_STEALTHED, apply);
}

void AuraEffect::HandleAuraModStalked(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // used by spells: Hunter's Mark, Mind Vision, Syndicate Tracker (MURP) DND
    if (apply)
        target->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (!target->HasAuraType(GetAuraType()))
            target->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleAuraUntrackable(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetStandVisFlags(UNIT_STAND_FLAGS_UNTRACKABLE);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
        target->RemoveStandVisFlags(UNIT_STAND_FLAGS_UNTRACKABLE);
    }
}
void AuraEffect::HandleAuraModSkill(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_SKILL)))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    uint32 prot = GetMiscValue();
    if (prot == SKILL_DEFENSE)
        return;

    int32 points = GetAmount();
    target->ModifySkillBonus(prot, (apply ? points : -points), GetAuraType() == SPELL_AURA_MOD_SKILL_TALENT);
}

/****************************/
/***       MOVEMENT       ***/
/****************************/

void AuraEffect::HandleAuraMounted(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        uint32 creatureEntry = GetMiscValue();
        uint32 displayId = 0;
        uint32 vehicleId = 0;

        if (auto mountEntry = sDB2Manager.GetMount(GetId()))
        {
            if (auto mountDisplays = sDB2Manager.GetMountDisplays(mountEntry->ID))
            {
                DB2Manager::MountXDisplayContainer usableDisplays;
                std::copy_if(mountDisplays->begin(), mountDisplays->end(), std::back_inserter(usableDisplays), [target](MountXDisplayEntry const* mountDisplay)
                {
                    return sConditionMgr->IsPlayerMeetingCondition(target, mountDisplay->PlayerConditionID);
                });

                if (!usableDisplays.empty())
                    displayId = Trinity::Containers::SelectRandomContainerElement(usableDisplays)->CreatureDisplayInfoID;
            }

            // TODO: CREATE TABLE mount_vehicle (mountId, vehicleCreatureId) for future mounts that are vehicles (new mounts no longer have proper data in MiscValue)
            //if (MountVehicle const* mountVehicle = sObjectMgr->GetMountVehicle(mountEntry->Id))
            //    creatureEntry = mountVehicle->VehicleCreatureId;
        }

        // Festive Holiday Mount
        if (target->HasAura(62061))
        {
            if (GetBase()->HasEffectType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                creatureEntry = 24906;
            else
                creatureEntry = 15665;
        }
        
        // Nightmarish Reins
        if (target->HasAura(162997))
        {
            creatureEntry = 80651;
            displayId = 55896;
        }

        // Glyph of the Luminous Charger
        if (target->HasAura(89401) && GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->MiscValueB == 230)
        {
            target->CastSpell(target, 126666, true);
        }

        if (auto ci = sObjectMgr->GetCreatureTemplate(creatureEntry))
        {
            vehicleId = ci->VehicleId;

            uint32 team = 0;
            if (target->IsPlayer())
                team = target->ToPlayer()->GetTeam();

            if (!displayId/* || vehicleId*/)
            {
                displayId = sObjectMgr->GetCreatureDisplay(ObjectMgr::ChooseDisplayId(team, ci));
                sObjectMgr->GetCreatureModelRandomGender(&displayId);
            }

            //some spell has one aura of mount and one of vehicle
            for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (GetSpellInfo()->EffectMask < uint32(1 << i))
                    break;

                if (GetSpellInfo()->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_SUMMON && GetSpellInfo()->GetEffect(i, m_diffMode)->MiscValue == GetMiscValue())
                    displayId = 0;
            }
        }

        if (GetSpellInfo()->IsNotMount())
            target->SetFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_NOT_CHECK_MOUNT);

        target->Mount(displayId, vehicleId, GetMiscValue());
    }
    else
    {
        target->Dismount();

        if (mode & AURA_EFFECT_HANDLE_REAL)
            target->RemoveAurasByType(SPELL_AURA_MOUNTED);

        if (GetSpellInfo()->IsNotMount())
            target->RemoveFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_NOT_CHECK_MOUNT);
    }

    target->UpdateMount();
}

void AuraEffect::HandleAuraAllowFlight(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()) || target->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
            return;

        //! Not entirely sure if this should be sent for creatures as well, but I don't think so.
        target->m_movementInfo.fall.SetFallTime(0);
        target->RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING_FLY);
        target->AddUnitMovementFlag(MOVEMENTFLAG_FALLING);
    }
    else
        target->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING);

    target->SetCanTransitionBetweenSwimAndFly(apply);
    target->SetCanFly(apply);

    //! We still need to initiate a server-side MoveFall here,
    //! which requires MSG_MOVE_FALL_LAND on landing.
    if (target->IsCreature())
        target->GetMotionMaster()->MoveFall();
}

void AuraEffect::HandleAuraWaterWalk(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    target->SetWaterWalking(apply);
}

void AuraEffect::HandleAuraFeatherFall(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    target->SetFeatherFall(apply);

    // start fall from current height
    if (!apply && target->IsPlayer())
        target->ToPlayer()->SetFallInformation(0, target->GetPositionZ());
}

void AuraEffect::HandleAuraCanTurnWhileFalling(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    if (!apply && target->HasAuraType(GetAuraType())) // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        return;

    target->SetCanTurnWhileFalling(apply);
}

void AuraEffect::HandleAuraHover(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    target->SetHover(apply);    //! Sets movementflags
}

void AuraEffect::HandleWaterBreathing(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // update timers in client
    if (target->IsPlayer())
        target->ToPlayer()->UpdateMirrorTimers();
}

void AuraEffect::HandleForceMoveForward(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVEMENT);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVEMENT);
    }
}

void AuraEffect::HandleModThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    for (int8 i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1 << i))
            ApplyPercentModFloatVar(target->m_threatModifier[i], GetAmount(), apply);
}

void AuraEffect::HandleAuraModTotalThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->isAlive() || !target->IsPlayer())
        return;

    Unit* caster = GetCaster();
    if (caster && caster->isAlive())
        target->getHostileRefManager().addTempThreat(GetAmount(), apply);
}

void AuraEffect::HandleModTaunt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->isAlive() || !target->CanHaveThreatList())
        return;

    Unit* caster = GetCaster();
    if (!caster || !caster->isAlive())
        return;

    if (apply)
        target->TauntApply(caster);
    else
    {
        // When taunt aura fades out, mob will switch to previous target if current has less than 1.1 * secondthreat
        target->TauntFadeOut(caster);
    }
}

/*****************************/
/***        CONTROL        ***/
/*****************************/

void AuraEffect::HandleModConfuse(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if(apply)
        target->SetTimeForSpline(GetBase()->GetDuration());
    else
        target->SetTimeForSpline(0);

    target->SetControlled(apply, UNIT_STATE_CONFUSED);
}

void AuraEffect::HandleModFear(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetTimeForSpline(GetBase()->GetDuration());
    else
        target->SetTimeForSpline(0);

    target->SetControlled(apply, UNIT_STATE_FLEEING);
}

void AuraEffect::HandleAuraModStun(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->SetControlled(apply, UNIT_STATE_STUNNED);
}

void AuraEffect::HandleAuraModRoot(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    // Earthgrab totem - Immunity
    if (apply && target->HasAura(116946))
        return;

    target->SetControlled(apply, UNIT_STATE_ROOT);
}

void AuraEffect::HandlePreventFleeing(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->HasAuraType(SPELL_AURA_MOD_FEAR))
        target->SetControlled(!(apply), UNIT_STATE_FLEEING);
}

void AuraEffect::HandleModPossess(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    // no support for posession AI yet
    if (caster && caster->IsCreature())
    {
        HandleModCharm(aurApp, mode, apply);
        return;
    }

    if (apply)
        target->SetCharmedBy(caster, CHARM_TYPE_POSSESS, aurApp);
    else
        target->RemoveCharmedBy(caster);
}

void AuraEffect::HandleModCharm(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    if (apply)
        target->SetCharmedBy(caster, CHARM_TYPE_CHARM, aurApp);
    else
        target->RemoveCharmedBy(caster);
}

void AuraEffect::HandleCharmConvert(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    if (apply)
        target->SetCharmedBy(caster, CHARM_TYPE_CONVERT, aurApp);
    else
        target->RemoveCharmedBy(caster);
}

/**
 * Such auras are applied from a caster(=player) to a vehicle.
 * This has been verified using spell #49256
 */
void AuraEffect::HandleAuraControlVehicle(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsVehicle())
        return;

    Unit* caster = GetCaster();

    if (!caster || caster == target)
        return;

    if (apply)
    {
        // Currently spells that have base points  0 and DieSides 0 = "0/0" exception are pushed to -1,
        // however the idea of 0/0 is to ingore flag VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT and -1 checks for it,
        // so this break such spells or most of them.
        // Current formula about m_amount: effect base points + dieside - 1
        // TO DO: Reasearch more about 0/0 and fix it.
        caster->_EnterVehicle(target->GetVehicleKit(), int8(m_amount) - 1, aurApp);
        if (Player* player = target->ToPlayer())
        {
            player->UnsummonPetTemporaryIfAny();
            player->UnsummonCurrentBattlePetIfAny(true);
        }
    }
    else
    {
        // Remove pending passengers before exiting vehicle - might cause an Uninstall
        target->GetVehicleKit()->RemovePendingEventsForPassenger(caster);

        if (GetId() == 53111) // Devour Humanoid
        {
            target->Kill(caster);
            if (caster->IsCreature())
                caster->ToCreature()->RemoveCorpse();
        }

        if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT))
            caster->_ExitVehicle();
        else
            target->GetVehicleKit()->RemovePassenger(caster);  // Only remove passenger from vehicle without launching exit movement or despawning the vehicle

        // some SPELL_AURA_CONTROL_VEHICLE auras have a dummy effect on the player - remove them
        caster->RemoveAurasDueToSpell(GetId());
        if (Player* player = target->ToPlayer())
        {
            if (!player->m_Teleports)
            {
                player->ResummonPetTemporaryUnSummonedIfAny();
                player->SummonLastSummonedBattlePet();
            }
        }
    }
}

void AuraEffect::HandleAuraModIncreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (GetAuraType() == SPELL_AURA_MOD_SPEED_NO_CONTROL)
    {
        target->UpdateSpeed(MOVE_SWIM, true);
        return;
    }

    target->UpdateSpeed(MOVE_RUN, true);

    if (GetAuraType() == SPELL_AURA_MOD_MINIMUM_SPEED)
    {
        target->UpdateSpeed(MOVE_RUN_BACK, true);
        target->UpdateSpeed(MOVE_FLIGHT, true);
        target->UpdateSpeed(MOVE_WALK, true);
    }

    // if (Player *player = target->ToPlayer())
        // player->SendMovementSetCollisionHeight(player->GetCollisionHeight(true));
}

void AuraEffect::HandleAuraModIncreaseMountedSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleAuraModIncreaseSpeed(aurApp, mode, apply);
}

void AuraEffect::HandleAuraModIncreaseFlightSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    if (mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK)
        target->UpdateSpeed(MOVE_FLIGHT, true);

    //! Update ability to fly
    if (GetAuraType() == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK && (apply || (!target->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !target->HasAuraType(SPELL_AURA_FLY))))
        {
            target->SetCanTransitionBetweenSwimAndFly(apply);
            target->SetCanFly(apply);

            if (!apply && !target->IsLevitating() && target->IsCreature())
                target->GetMotionMaster()->MoveFall();
        }

        //! Someone should clean up these hacks and remove it from this function. It doesn't even belong here.
        if (mode & AURA_EFFECT_HANDLE_REAL)
        {
            //Players on flying mounts must be immune to polymorph
            if (target->IsPlayer())
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);

            // Dragonmaw Illusion (overwrite mount model, mounted aura already applied)
            if (apply && target->HasAuraEffect(42016, 0) && target->GetMountID())
                target->SetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID, 16314);
        }
    }
}

void AuraEffect::HandleAuraModIncreaseSwimSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    aurApp->GetTarget()->UpdateSpeed(MOVE_SWIM, true);
}

void AuraEffect::HandleAuraModDecreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateSpeed(MOVE_SWIM, true);
    target->UpdateSpeed(MOVE_FLIGHT, true);
    target->UpdateSpeed(MOVE_RUN_BACK, true);
    target->UpdateSpeed(MOVE_SWIM_BACK, true);
    target->UpdateSpeed(MOVE_FLIGHT_BACK, true);
}

void AuraEffect::HandleAuraModUseNormalSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->UpdateSpeed(MOVE_RUN,  true);
    target->UpdateSpeed(MOVE_SWIM, true);
    target->UpdateSpeed(MOVE_FLIGHT,  true);
}

void AuraEffect::HandleModStateImmunityMask(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    std::list <AuraType> aura_immunity_list;
    uint32 mechanic_immunity_list = 0;
    int32 miscVal = GetMiscValue();

    switch (miscVal)
    {
        case 27:
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SILENCE, apply);
            aura_immunity_list.push_back(SPELL_AURA_MOD_SILENCE);
            break;
        case 1991:
        {
            mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
            aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
            aura_immunity_list.push_back(SPELL_AURA_MOD_ROOTED);
            break;
        }
        case 96:
        case 1615:
        case 1984:
        {
            if (GetAmount())
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
            }
            break;
        }
        case 1701:
        {
            if (GetAmount())
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_GRIP)
                    | (1 << MECHANIC_DISARM) | (1 << MECHANIC_SLEEP);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_GRIP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);

                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DISARM);
            }
            break;
        }
        case 679:
        case 1921:
        case 2071:
        {
            if (GetId() == 57742 || GetId() == 115018 || GetId() == 200851)
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
            }
            break;
        }
        case 1557:
        {
            if (GetId() == 64187)
            {
                mechanic_immunity_list = (1 << MECHANIC_STUN);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
            }
            else
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
            }
            break;
        }
        case 1614:
        case 1694:
        {
            target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, apply);
            aura_immunity_list.push_back(SPELL_AURA_MOD_TAUNT);
            break;
        }
        case 1630:
        {
            if (!GetAmount())
            {
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_TAUNT);
            }
            else
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
            }
            break;
        }
        case 477:
        case 1733:
        {
            if (!GetAmount())
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN)
                    | (1 << MECHANIC_DISARM) | (1 << MECHANIC_BANISH);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_BANISH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DISARM);
            }
            break;
        }
        case 878:
        {
            if (GetAmount() == 1)
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_DISORIENTED) | (1 << MECHANIC_FREEZE);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
            }
            break;
        }
        case 1887:
        {
            // Frost Pillar
            if (GetId() == 51271)
            {
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, apply);
                miscVal = 0;
            }

            break;
        }
        case 1664:
        {
            mechanic_immunity_list = (1 << MECHANIC_SNARE);

            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);

            aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
            break;
        }
        case 1808:
        {
            mechanic_immunity_list = (1 << MECHANIC_STUN);

            target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, apply);

            aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
            break;
        }
        case 1994:
        {
            mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
            aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
            aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
            aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
            aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
            aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
            break;
        }
        default:
            break;
    }

    if (aura_immunity_list.empty())
    {
        if (miscVal & (1<<0))
        {
            aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
            aura_immunity_list.push_back(SPELL_AURA_MOD_ROOTED);
        }

        if (miscVal & (1<<1))
            aura_immunity_list.push_back(SPELL_AURA_TRANSFORM);

        if (miscVal & (1<<2))
            aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);

        // These flag can be recognized wrong:
        if (miscVal & (1<<6))
            aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);

        if (miscVal & (1<<7))
            aura_immunity_list.push_back(SPELL_AURA_MOD_DISARM);

        if (miscVal & (1<<9))
        {
            aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
            aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
        }

        if (miscVal & (1<<10))
            aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
    }

    // apply immunities
    for (auto& iter : aura_immunity_list)
        target->ApplySpellImmune(GetId(), IMMUNITY_STATE, iter, apply);

    if (apply && GetSpellInfo()->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY))
    {
        target->RemoveAurasWithMechanic(mechanic_immunity_list, AURA_REMOVE_BY_DEFAULT, GetId());
        for (auto& iter : aura_immunity_list)
            target->RemoveAurasByType(iter, ObjectGuid::Empty, GetBase());
    }
}

void AuraEffect::HandleModMechanicImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    uint32 mechanic;

    switch (GetId())
    {
        case 42292: // PvP trinket
        case 195710: // Honorable Medallion
        case 208683: // Gladiator's Medallion
            mechanic = IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK;
            // Actually we should apply immunities here, too, but the aura has only 100 ms duration, so there is practically no point
            break;
        default:
            if (GetMiscValue() < 1)
                return;
            mechanic = 1 << GetMiscValue();
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, GetMiscValue(), apply);
            break;
    }

    if (apply && GetSpellInfo()->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY))
        target->RemoveAurasWithMechanic(mechanic, AURA_REMOVE_BY_DEFAULT, GetId());
}

void AuraEffect::HandleAuraModEffectImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

   target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, GetMiscValue(), apply);
}

void AuraEffect::HandleAuraModStateImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellImmune(GetId(), IMMUNITY_STATE, GetMiscValue(), apply);

    if (apply && GetSpellInfo()->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY))
        target->RemoveAurasByType(AuraType(GetMiscValue()), ObjectGuid::Empty, GetBase());
}

void AuraEffect::HandleAuraModSchoolImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellImmune(GetId(), IMMUNITY_SCHOOL, GetMiscValue(), (apply));

    if (GetSpellInfo()->Categories.Mechanic == MECHANIC_BANISH)
    {
        if (apply)
            target->AddUnitState(UNIT_STATE_ISOLATED);
        else
        {
            bool banishFound = false;
            if (Unit::AuraEffectList const* banishAuras = target->GetAuraEffectsByType(GetAuraType()))
                for (Unit::AuraEffectList::const_iterator i = banishAuras->begin(); i != banishAuras->end(); ++i)
                    if ((*i)->GetSpellInfo()->Categories.Mechanic == MECHANIC_BANISH)
                    {
                        banishFound = true;
                        break;
                    }
            if (!banishFound)
                target->ClearUnitState(UNIT_STATE_ISOLATED);
        }
    }

    uint32 count = 0;
    if (apply && GetMiscValue() == SPELL_SCHOOL_MASK_NORMAL)
        count += target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION, GetId());

    // remove all flag auras (they are positive, but they must be removed when you are immune)
    if (GetSpellInfo()->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY) && GetSpellInfo()->HasAttribute(SPELL_ATTR2_DAMAGE_REDUCED_SHIELD))
        count += target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION, GetId());

    // TODO: optimalize this cycle - use RemoveAurasWithInterruptFlags call or something else
    if ((apply) && GetSpellInfo()->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY) && GetSpellInfo()->IsPositive())                       //Only positive immunity removes auras
    {
        uint32 school_mask = GetMiscValue();
        Unit::AuraApplicationMap& Auras = target->GetAppliedAuras();
        for (Unit::AuraApplicationMap::iterator iter = Auras.begin(); iter != Auras.end();)
        {
            AuraApplicationPtr auraApp = iter->second;
            if (!auraApp || !auraApp->GetBase())
            {
                ++iter;
                continue;
            }
            SpellInfo const* spell = auraApp->GetBase()->GetSpellInfo();
            if ((spell->GetSchoolMask() & school_mask) && GetSpellInfo()->CanDispelAura(spell) && !auraApp->IsPositive() && spell->Id != GetId())
            {
                target->RemoveAura(iter);
                count++;
            }
            else
                ++iter;
        }
    }

    if(Spell* modSpell = target->FindCurrentSpellBySpellId(GetId()))
        modSpell->m_count_dispeling += count;
}

void AuraEffect::HandleAuraModDmgImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellImmune(GetId(), IMMUNITY_DAMAGE, GetMiscValue(), apply);
}

void AuraEffect::HandleAuraModDispelImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellDispelImmunity(m_spellInfo, DispelType(GetMiscValue()), (apply));
}

void AuraEffect::HandleAuraModResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
    {
        if (GetMiscValue() & int32(1<<x))
        {
            target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, GetAmount(), apply);
            if (target->IsPlayer() || target->ToCreature()->isPet())
                target->ApplyResistanceBuffModsMod(SpellSchools(x), GetAmount() > 0, GetAmount(), apply);
        }
    }
}

void AuraEffect::HandleAuraModBaseResistancePCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    int32 miscVal = GetMiscValue();

    // only players have base stats
    if (Player* plr = target->ToPlayer())
    {
        for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
        {
            if (miscVal & int32(1<<x))
            {
                switch (miscVal)
                {
                    case 1:
                        plr->SendUpdateStat(USM_ARMOR);
                        break;
                    default:
                        target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, GetAmount(), apply);
                        break;
                }
            }
        }
    }
    else
    {
        //pets only have base armor
        if (target->ToCreature()->isPet() && (miscVal & SPELL_SCHOOL_MASK_NORMAL))
            target->HandleStatModifier(UNIT_MOD_ARMOR, BASE_PCT, GetAmount(), apply);
    }
}

void AuraEffect::HandleModResistancePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    for (int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if (GetMiscValue() & int32(1<<i))
        {
            switch (GetMiscValue())
            {
                case 1:
                {
                    if (Player* plr = target->ToPlayer())
                        plr->SendUpdateStat(USM_ARMOR);
                    else
                        target->UpdateArmor();
                    break;
                }
                default:
                {
                    target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT, GetAmount(), apply);
                    if (target->IsPlayer() || target->ToCreature()->isPet())
                    {
                        target->ApplyResistanceBuffModsPercentMod(SpellSchools(i), true, GetAmount(), apply);
                        target->ApplyResistanceBuffModsPercentMod(SpellSchools(i), false, GetAmount(), apply);
                    }
                    break;
                }
            }
        }
    }
}

void AuraEffect::HandleModBaseResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // only players have base stats
    if (!target->IsPlayer())
    {
        //only pets have base stats
        if (target->ToCreature()->isPet() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            target->HandleStatModifier(UNIT_MOD_ARMOR, TOTAL_VALUE, GetAmount(), apply);
    }
    else
    {
        for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
            if (GetMiscValue() & (1<<i))
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, GetAmount(), apply);
    }
}

void AuraEffect::HandleModTargetResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // applied to damage as HandleNoImmediateEffect in Unit::CalcAbsorbResist and Unit::CalcArmorReducedDamage

    // show armor penetration
    if (target->IsPlayer() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
        target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE, GetAmount(), apply);

    // show as spell penetration only full spell penetration bonuses (all resistances except armor and holy
    if (target->IsPlayer() && (GetMiscValue() & SPELL_SCHOOL_MASK_SPELL) == SPELL_SCHOOL_MASK_SPELL)
        target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE, GetAmount(), apply);
}

void AuraEffect::HandleAuraModStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    if (GetMiscValue() < -2 || GetMiscValue() > 4)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "WARNING: Spell %u effect %u has an unsupported misc value (%i) for SPELL_AURA_MOD_STAT ", GetId(), GetEffIndex(), GetMiscValue());
        return;
    }

    Unit* target = aurApp->GetTarget();
    int32 miscValue = GetMiscValue();

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        // -1 or -2 is all stats (misc < -2 checked in function beginning)
        if (miscValue < 0 || miscValue == i)
        {
            target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, GetAmount(), apply);
        }
    }
}

void AuraEffect::HandleModPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (GetMiscValue() < -1 || GetMiscValue() > 4)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    // only players have base stats

    if (Player* plr = target->ToPlayer())
    {
        for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        {
            if (GetMiscValue() == i || GetMiscValue() == -1)
                plr->SendUpdateStat(1 << i);
        }
    }
}

void AuraEffect::HandleModSpellDamagePercentFromStat(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromStat(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleAuraModSpellPowerPercent(AuraApplication const * aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit *target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModHealingDone(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;
    // implemented in Unit::SpellHealingBonus
    // this information is for client side only
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModTotalPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    int32 miscValueB = GetMiscValueB();

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        if (miscValueB & (1 << i) || !miscValueB) // 0 is also used for all stats
        {
            if (Player * plr = target->ToPlayer())
            {
                plr->SendUpdateStat(1 << i);
            }
            else if (target->isPet())
            {
                target->UpdateStats(Stats(i));
            }
        }
    }
}

void AuraEffect::HandleModPowerRegen(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    switch (GetMiscValue())
    {
        case POWER_MANA:
        {
            if (Player* plr = target->ToPlayer())
            {
                plr->Regenerate(Powers(POWER_MANA), plr->m_powerRegenTimer[POWER_MANA]);
                plr->m_powerRegenTimer[POWER_MANA] = 0;
                plr->SendUpdateStat(USM_MANA_REGEN);
                break;
            }

            target->UpdateManaRegen();
            break;
        }
        case POWER_RUNES:
            if (target->IsPlayer())
                target->ToPlayer()->SendOperationsAfterDelay(OAD_UPDATE_RUNES_REGEN);
            break;
        default:
            target->UpdatePowerRegen(GetMiscValue());
            break;
    }
}

void AuraEffect::HandleModPowerRegenPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    Player* plr = target->ToPlayer();

    switch (GetMiscValue())
    {
        case POWER_MANA:
        {
            plr ? plr->SendUpdateStat(USM_MANA_REGEN) : target->UpdateManaRegen();
            break;
        }
        case POWER_RUNES:
            if (plr)
                plr->SendOperationsAfterDelay(OAD_UPDATE_RUNES_REGEN);
            break;
        default:
            target->UpdatePowerRegen(GetMiscValue());
            break;
    }

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandleModPowerRegenPCT mode %i, apply %i, GetAmount %i", mode, apply, GetAmount());
}

void AuraEffect::HandleModManaRegen(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (Player* plr = target->ToPlayer())
    {
        plr->SendUpdateStat(USM_MANA_REGEN);
        //Note: an increase in regen does NOT cause threat.
        plr->SendOperationsAfterDelay(OAD_UPDATE_RUNES_REGEN);
    }
    else
        target->UpdateManaRegen();
}

void AuraEffect::HandleEnableExtraTalent(AuraApplication const * aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (Unit* target = aurApp->GetTarget())
    {
        if (Player* plr = target->ToPlayer())
        {
            if (TalentEntry const* talentEntry = sTalentStore.LookupEntry(GetMiscValue()))
            {
                for (uint32 t = 0; t < MAX_TALENT_COLUMNS; ++t)
                    if (plr->HasTalent(talentEntry->ID, plr->GetActiveTalentGroup()))
                        return;

                bool hasItem = false;
                uint32 itemId = GetMiscValueB();

                for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
                {
                    if (Item* pItem = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    {
                        if (pItem->GetEntry() == itemId)
                        {
                            hasItem = true;
                            break;
                        }
                    }
                }

                if (!hasItem || !apply)
                {
                    plr->removeSpell(talentEntry->SpellID);
                    return;
                }

                plr->learnSpell(talentEntry->SpellID, false);
            }
        }
    }
}

void AuraEffect::HandleAuraModIncreaseHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    int32 amount = GetAmount();

    if(target->isAnySummons())
    {
        if(target != GetCaster())
            return;
    }

    if (apply)
    {
        target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(amount), apply);
        target->ModifyHealth(amount, nullptr, GetId());
    }
    else
    {
        if (int32(target->GetHealth()) > GetAmount())
            target->ModifyHealth(-GetAmount(), nullptr, GetId());
        else
            target->SetHealth(1, GetId());
        target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(amount), apply);
    }
}

void AuraEffect::HandleAuraModIncreaseMaxHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    auto target = aurApp->GetTarget();
    if (target->isAnySummons() && target != GetCaster())
        return;

    auto oldhealth = target->GetHealth();
    auto healthPercentage = static_cast<double>(oldhealth) / static_cast<double>(target->GetMaxHealth());

    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, GetAmount(), apply);

    // refresh percentage
    if (oldhealth > 0)
    {
        auto newhealth = uint64(ceil(static_cast<double>(target->GetMaxHealth()) * healthPercentage));
        if (newhealth == 0)
            newhealth = 1;

        target->SetHealth(newhealth);
    }
}

void AuraEffect::HandleAuraModIncreaseEnergy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    // do not check power type, we can always modify the maximum
    // as the client will not see any difference
    // also, placing conditions that may change during the aura duration
    // inside effect handlers is not a good idea
    //if (int32(powerType) != GetMiscValue())
    //    return;

    target->UpdateMaxPower(Powers(GetMiscValue()));
}

void AuraEffect::HandleAuraModIncreaseEnergyPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    auto powerType = Powers(GetMiscValue());
    // do not check power type, we can always modify the maximum
    // as the client will not see any difference
    // also, placing conditions that may change during the aura duration
    // inside effect handlers is not a good idea
    //if (int32(powerType) != GetMiscValue())
    //    return;

    auto amount = GetAmount();

    if (apply)
    {
        target->UpdateMaxPower(powerType);
        target->ModifyPowerPct(powerType, amount, apply);
    }
    else
    {
        target->ModifyPowerPct(powerType, amount, apply);
        target->UpdateMaxPower(powerType);
    }
}

void AuraEffect::HandleAuraModMaxPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    uint32 miscValue = Powers(GetMiscValue());

    if (Unit* target = aurApp->GetTarget())
    {
        target->UpdateMaxPower(Powers(miscValue));
        if (apply && miscValue == POWER_RUNES)
            target->ModifyPower(POWER_RUNES, GetAmount());
    }
}

void AuraEffect::HandleAuraModMaxManaPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    if (Unit* target = aurApp->GetTarget())
        target->UpdateMaxPower(Powers(GetMiscValue()));
}

void AuraEffect::HandleAuraModAddEnergyPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    if (Unit* target = aurApp->GetTarget())
        target->UpdateMaxPower(Powers(GetMiscValue()));
}

void AuraEffect::HandleAuraModIncreaseHealthOrPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if(target->isAnySummons())
    {
        if(target != GetCaster())
            return;
    }
    // Unit will keep hp% after MaxHealth being modified if unit is alive.
    int32 amount = GetAmount();

    if (amount > 100)
    {
        if (apply)
        {
            target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(amount), apply);
            target->ModifyHealth(amount, nullptr, GetId());
        }
        else
        {
            if (target->GetHealth() > amount)
                target->ModifyHealth(-amount, nullptr, GetId());
            else
                target->SetHealth(1, GetId());

            target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(amount), apply);
        }
    }
    else
    {
        uint32 percentHeal = GetAmount() / GetBase()->GetStackAmount(); // Heal only per stack, not all stack every time
        uint32 heal = target->CountPctFromMaxHealth(percentHeal);
        float percent = target->GetHealthPct();
        target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, float(amount), apply);

        bool otherLogick = false;
        switch (GetId())
        {
            case 55233: // Vampiric Blood
            case 205725: // Anti-Magic Barrier
                otherLogick = true;
                break;
        }

        if (target->isAlive())
        {
            if (apply && otherLogick)
                target->ModifyHealth(heal, nullptr, GetId());
            else
                target->SetHealth(target->CountPctFromMaxHealth(int32(ceil(percent))), GetId());
        }
    }
}

void AuraEffect::HandleAuraModIncreaseHealthPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->isAnySummons())
    {
        if (target != GetCaster() && GetCaster() != target->GetAnyOwner())
            return;
    }

    // Unit will keep hp% after MaxHealth being modified if unit is alive.
    float percent = target->GetHealthPct();
    uint32 percentHeal = GetAmount() / GetBase()->GetStackAmount(); // Heal only per stack, not all stack every time
    uint32 heal = target->CountPctFromMaxHealth(percentHeal);

    bool otherLogick = false;
    switch (GetId())
    {
        case 12975: // Last Stand
        case 187827: // Last Stand
            otherLogick = true;
            break;
    }

    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, GetAmount(), apply);

    if (target->isAlive())
    {
        if (apply && otherLogick)
            target->ModifyHealth(heal, nullptr, GetId());
        else
            target->SetHealth(target->CountPctFromMaxHealth(int32(ceil(percent))), GetId());
    }
}

void AuraEffect::HandleAuraModPetStatsModifier(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    if(Unit* target = aurApp->GetTarget())
    {
        Guardian* pet = nullptr;
        if (Player* _player = target->ToPlayer())
            pet = _player->GetGuardianPet();

        if(!pet)
            pet = target->ToPet();

        if (pet)
        {
            if(GetMiscValue() == PETSPELLMOD_ARMOR)
                pet->UpdateArmor();
            if(GetMiscValue() == PETSPELLMOD_MAX_HP)
            {
                if(apply)
                    pet->ModifyHealth(GetAmount(), nullptr, GetId());
                pet->UpdateMaxHealth();
            }
        }
    }
}

void AuraEffect::HandleAuraModParryPercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    target->ToPlayer()->UpdateParryPercentage();
}

void AuraEffect::HandleAuraModDodgePercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    target->ToPlayer()->UpdateDodgePercentage();
}

void AuraEffect::HandleAuraModBlockPercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    target->ToPlayer()->UpdateBlockPercentage();
}

void AuraEffect::HandleAuraModRegenInterrupt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleModManaRegen(aurApp, mode, apply);
}

void AuraEffect::HandleAuraModWeaponCritPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target->IsPlayer())
        return;

    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    if (GetSpellInfo()->EquippedItemClass == -1)
    {
        target->ToPlayer()->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, GetAmount(), apply);
        target->ToPlayer()->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, GetAmount(), apply);
        target->ToPlayer()->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, GetAmount(), apply);
    }
}

void AuraEffect::HandleModHitChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->IsPlayer())
    {
        target->ToPlayer()->UpdateMeleeHitChances();
        target->ToPlayer()->UpdateRangedHitChances();
    }
    else
    {
        target->m_modMeleeHitChance += (apply) ? GetAmount() : (-GetAmount());
        target->m_modRangedHitChance += (apply) ? GetAmount() : (-GetAmount());
    }
}

void AuraEffect::HandleModSpellHitChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->IsPlayer())
        target->ToPlayer()->UpdateSpellHitChances();
    else
        target->m_modSpellHitChance += (apply) ? GetAmount(): (-GetAmount());
}

void AuraEffect::HandleModSpellCritChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->IsPlayer())
        target->ToPlayer()->UpdateSpellCritChance();
    else
        target->m_baseSpellCritChance += (apply) ? GetAmount():-GetAmount();
}

void AuraEffect::HandleAuraModCritPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target->IsPlayer())
    {
        target->m_baseSpellCritChance += apply ? GetAmount() : -GetAmount();
        return;
    }

    target->ToPlayer()->HandleBaseModValue(CRIT_PERCENTAGE, FLAT_MOD, GetAmount(), apply);
    target->ToPlayer()->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, GetAmount(), apply);
    target->ToPlayer()->HandleBaseModValue(RANGED_CRIT_PERCENTAGE, FLAT_MOD, GetAmount(), apply);
    target->ToPlayer()->UpdateSpellCritChance();
}

void AuraEffect::HandleAuraModResiliencePct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    Player* _player = target->ToPlayer();

    float baseValue = _player->GetFloatValue(PLAYER_FIELD_MOD_RESILIENCE_PERCENT);

    if (apply)
        baseValue += GetAmount() / 100.f;
    else
        baseValue -= GetAmount() / 100.f;

    _player->SetFloatValue(PLAYER_FIELD_MOD_RESILIENCE_PERCENT, std::max(0.f, baseValue));
}

void AuraEffect::HandleModCastingSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (Player* plr = target->ToPlayer())
    {
        plr->SendUpdateStat(USM_HAST | USM_CAST_HAST);
    }
    else
    {
        target->UpdateHastMod();
        target->UpdateCastHastMods();
    }
}

void AuraEffect::HandleModMeleeRangedSpeedPct(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    //! ToDo: Haste auras with the same handler _CAN'T_ stack together
    Unit* target = aurApp->GetTarget();

    if (Player* plr = target->ToPlayer())
    {
        plr->SendUpdateStat(USM_MELEE_HAST | USM_RANGE_HAST);
    }
    else
    {
        target->UpdateMeleeHastMod();
        target->UpdateRangeHastMod();
        target->UpdateCastHastMods();
    }
}

void AuraEffect::HandleModCombatSpeedPct(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (Player* plr = target->ToPlayer())
    {
        plr->SendUpdateStat(USM_HAST | USM_MELEE_HAST | USM_RANGE_HAST | USM_CAST_HAST);
    }
    else
    {
        target->UpdateHastMod();
        target->UpdateMeleeHastMod();
        target->UpdateRangeHastMod();
        target->UpdateCastHastMods();
    }
}

void AuraEffect::HandleModAttackSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    float hastMod = target->GetFloatValue(UNIT_FIELD_MOD_HASTE);
    ApplyPercentModFloatVar(hastMod, GetAmount(), !apply);

    target->CalcAttackTimePercentMod(BASE_ATTACK, hastMod);
    target->UpdateDamagePhysical(BASE_ATTACK);
}

void AuraEffect::HandleModMeleeSpeedPct(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    //! ToDo: Haste auras with the same handler _CAN'T_ stack together
    Unit* target = aurApp->GetTarget();

    if (Player* plr = target->ToPlayer())
        plr->SendUpdateStat(USM_MELEE_HAST);
    else
        target->UpdateMeleeHastMod();
}

void AuraEffect::HandleAuraModRangedHaste(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    //! ToDo: Haste auras with the same handler _CAN'T_ stack together
    Unit* target = aurApp->GetTarget();

    if (Player* plr = target->ToPlayer())
        plr->SendUpdateStat(USM_RANGE_HAST);
    else
        target->UpdateRangeHastMod();
}

void AuraEffect::HandleModRating(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (Player* plr = target->ToPlayer())
        plr->SendUpdateCR(GetMiscValue());
}

void AuraEffect::HandleConverCritRatingPctToParryRating(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    // Just recalculate ratings
    target->ToPlayer()->UpdateParryPercentage();
}

void AuraEffect::HandleAuraModAttackPower(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    if (Unit* target = aurApp->GetTarget())
    {
        if (Player* plr = target->ToPlayer())
        {
            plr->SendUpdateStat(USM_MELEE_AP);
        }
        else
        {
            target->UpdateAttackPowerAndDamage();
        }
    }
}

void AuraEffect::HandleAuraModRangedAttackPower(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    if (Unit* target = aurApp->GetTarget())
    {
        if (Player* plr = target->ToPlayer())
        {
            plr->SendUpdateStat(USM_RANGE_AP);
        }
        else
        {
            target->UpdateAttackPowerAndDamage(true);
        }
    }
}

void AuraEffect::HandleAuraModAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->ToPlayer())
        target->ToPlayer()->SendUpdateStat(USM_MELEE_AP);
    else if (Pet* pet = target->ToPet())
        pet->UpdateAttackPowerAndDamage(false);
}

void AuraEffect::HandleAuraModRangedAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->ToPlayer())
        target->ToPlayer()->SendUpdateStat(USM_RANGE_AP);
}

void AuraEffect::HandleOverrideAttackPowerBySpellPower(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // Recalculate bonus
    if (target->IsPlayer())
        target->ToPlayer()->SendUpdateStat(USM_MELEE_AP);
}

void AuraEffect::HandleOverrideSpellPowerByAttackPower(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

   target->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleOverrideAutoattack(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK)))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    if (apply)
    {
        target->SetOverrideAutoattack(GetTriggerSpell(), BASE_ATTACK);
        target->SetOverrideAutoattack(GetMiscValue(), OFF_ATTACK);
    }
    else
    {
        target->SetOverrideAutoattack(0, BASE_ATTACK);
        target->SetOverrideAutoattack(0, OFF_ATTACK);
    }
}

void AuraEffect::HandleIncreaseModRatingPct(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    if (Player* player = target->ToPlayer())
        player->SendUpdateCR(GetMiscValue());
}

void AuraEffect::HandleModStatBonusPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    int32 miscValue = GetMiscValue();

    if (miscValue < -1 || miscValue > 4)
        return;

    if (Player* plr = target->ToPlayer())
    {
        for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        {
            if (miscValue == i || miscValue == -1)
            {
                plr->SendUpdateStat(1 << i);
            }
        }
    }
}

void AuraEffect::HandleAuraBonusArmor(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Player* player = aurApp->GetTarget()->ToPlayer();
    if (!player)
        return;

    if (GetAuraType() == SPELL_AURA_MOD_BONUS_ARMOR)
        player->HandleStatModifier(UNIT_MOD_ARMOR, TOTAL_VALUE, GetAmount(), apply);

    player->SendUpdateStat(USM_ARMOR);
}

void AuraEffect::HandleModDamageDone(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // GetMiscValue() is bitmask of spell schools
    // 1 (0-bit) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wands
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    if ((GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellInfo()->EquippedItemClass == -1 || !target->IsPlayer())
        {
            target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, GetAmount(), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, GetAmount(), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, GetAmount(), apply);

            if (target->IsPlayer())
            {
                if (GetAmount() > 0)
                    target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS, GetAmount(), apply);
                else
                    target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG, GetAmount(), apply);
            }
        }
    }

    // Skip non magic case for speedup
    if ((GetMiscValue() & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if (GetSpellInfo()->EquippedItemClass != -1 || (GetSpellInfo()->EquippedItemInventoryTypeMask != 0 && GetSpellInfo()->EquippedItemInventoryTypeMask != -1))
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    if (target->IsPlayer())
    {
        target->ToPlayer()->ApplySpellPowerBonus(GetAmount(), apply);

        if (Guardian* pet = target->ToPlayer()->GetGuardianPet())
            pet->UpdateAttackPowerAndDamage();
    }
}

void AuraEffect::HandleModDamagePercentDone(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    if (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL)
    {
        target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, GetAmount(), apply);
        target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, GetAmount(), apply);
        target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_PCT, GetAmount(), apply);

        if (target->IsPlayer())
            target->ToPlayer()->ApplyPercentModFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PERCENT, GetAmount(), apply);
    }
}

void AuraEffect::HandleModOffhandDamagePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, GetAmount(), apply);
}

void AuraEffect::HandleShieldBlockValue(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    if (auto player = aurApp->GetTarget()->ToPlayer())
        player->HandleBaseModValue(SHIELD_BLOCK_VALUE, FLAT_MOD, GetAmount(), apply);
}

void AuraEffect::HandleModPowerCostPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    float amount = CalculatePct(1.0f, GetAmount());
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1 << i))
            target->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + i, amount, apply);

    // Preparation
    // This allows changind spec while in battleground
    if (GetId() == SPELL_BG_PREPARATION || GetId() == SPELL_ARENA_PREPARATION)
        target->ModifyAuraState(AURA_STATE_PVP_RAID_PREPARE, apply);
}

void AuraEffect::HandleModPowerCost(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1<<i))
            target->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+i, GetAmount(), apply);
}

void AuraEffect::HandleArenaPreparation(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
    }
}

void AuraEffect::HandleAuraDummy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_REAPPLY)))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    if (mode & (AURA_EFFECT_HANDLE_REAL | AURA_EFFECT_HANDLE_REAPPLY))
    {
        // AT APPLY
        if (apply)
        {
            switch (GetId())
            {
                case 154010:
                    caster->CastSpell(caster, GetAmount(), true);
                    break;
                case 1515:                                      // Tame beast
                    // FIX_ME: this is 2.0.12 threat effect replaced in 2.1.x by dummy aura, must be checked for correctness
                    if (caster && target->CanHaveThreatList())
                        target->AddThreat(caster, 10.0f);
                    break;
                case 13139:                                     // net-o-matic
                    // root to self part of (root_target->charge->root_self sequence
                    if (caster)
                        caster->CastSpell(caster, 13138, true, nullptr, this);
                    break;
                case 37096:                                     // Blood Elf Illusion
                {
                    if (caster)
                    {
                        switch (caster->getGender())
                        {
                            case GENDER_FEMALE:
                                caster->CastSpell(target, 37095, true, nullptr, this); // Blood Elf Disguise
                                break;
                            case GENDER_MALE:
                                caster->CastSpell(target, 37093, true, nullptr, this);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                }
                case 39850:                                     // Rocket Blast
                    if (roll_chance_i(20))                       // backfire stun
                        target->CastSpell(target, 51581, true, nullptr, this);
                    break;
                case 43873:                                     // Headless Horseman Laugh
                    target->PlayDistanceSound(11965);
                    break;
                case 46361:                                     // Reinforced Net
                    if (caster)
                        target->GetMotionMaster()->MoveFall();
                    break;
                case 71563:
                    {
                        Aura* newAura = target->AddAura(71564, target);
                        if (newAura != nullptr)
                            newAura->SetStackAmount(newAura->GetSpellInfo()->GetAuraOptions(m_diffMode)->CumulativeAura);
                    }
                    break;
                case 59628: // Tricks of the Trade
                    if (caster && caster->GetMisdirectionTarget())
                        target->SetReducedThreatPercent(100, caster->GetMisdirectionTarget()->GetGUID());
                    break;
            }
        }
        // AT REMOVE
        else
        {
            if ((GetSpellInfo()->IsQuestTame()) && caster && caster->isAlive() && target->isAlive())
            {
                uint32 finalSpelId = 0;
                switch (GetId())
                {
                    case 19548: finalSpelId = 19597; break;
                    case 19674: finalSpelId = 19677; break;
                    case 19687: finalSpelId = 19676; break;
                    case 19688: finalSpelId = 19678; break;
                    case 19689: finalSpelId = 19679; break;
                    case 19692: finalSpelId = 19680; break;
                    case 19693: finalSpelId = 19684; break;
                    case 19694: finalSpelId = 19681; break;
                    case 19696: finalSpelId = 19682; break;
                    case 19697: finalSpelId = 19683; break;
                    case 19699: finalSpelId = 19685; break;
                    case 19700: finalSpelId = 19686; break;
                    case 30646: finalSpelId = 30647; break;
                    case 30653: finalSpelId = 30648; break;
                    case 30654: finalSpelId = 30652; break;
                    case 30099: finalSpelId = 30100; break;
                    case 30102: finalSpelId = 30103; break;
                    case 30105: finalSpelId = 30104; break;
                }

                if (finalSpelId)
                    caster->CastSpell(target, finalSpelId, true, nullptr, this);
            }

            switch (m_spellInfo->ClassOptions.SpellClassSet)
            {
                case SPELLFAMILY_GENERIC:
                    switch (GetId())
                    {
                        case 161883: //Incinerating Breath
                            if (Creature* caster2 = target->ToCreature())
                                caster2->SetReactState(REACT_AGGRESSIVE);
                            break;
                        case SPELL_WAITING_FOR_RESURRECT:
                            // Waiting to resurrect spell cancel, we must remove player from resurrect queue
                            if (target->IsPlayer())
                            {
                                if (Battleground* bg = target->ToPlayer()->GetBattleground())
                                    bg->RemovePlayerFromResurrectQueue(target->GetGUID());
                                if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(target->GetCurrentZoneID()))
                                    bf->RemovePlayerFromResurrectQueue(target->GetGUID());
                            }
                            break;
                        case 36730:                                     // Flame Strike
                        {
                            target->CastSpell(target, 36731, true, nullptr, this);
                            break;
                        }
                        case 44191:                                     // Flame Strike
                        {
                            if (target->GetMap()->IsDungeon())
                            {
                                uint32 spellId = target->GetMap()->IsHeroic() ? 46163 : 44190;

                                target->CastSpell(target, spellId, true, nullptr, this);
                            }
                            break;
                        }
                        case 42783: // Wrath of the Astromancer
                            target->CastSpell(target, GetAmount(), true, nullptr, this);
                            break;
                        case 46308: // Burning Winds casted only at creatures at spawn
                            target->CastSpell(target, 47287, true, nullptr, this);
                            break;
                        case 52172:  // Coyote Spirit Despawn Aura
                        case 60244:  // Blood Parrot Despawn Aura
                            target->CastSpell(static_cast<Unit*>(nullptr), GetAmount(), true, nullptr, this);
                            break;
                        case 58600: // Restricted Flight Area
                        case 83100: // Restricted Flight Area
                        case 91604: // Restricted Flight Area
                            if (aurApp->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                                target->CastSpell(target, 58601, true);
                            break;
                    }
                    break;
                case SPELLFAMILY_DEATHKNIGHT:
                    // Summon Gargoyle (Dismiss Gargoyle at remove)
                    if (GetId() == 61777)
                        target->CastSpell(target, GetAmount(), true);
                    break;
                case SPELLFAMILY_ROGUE:
                    //  Tricks of the trade
                    switch (GetId())
                    {
                        case 59628: //Tricks of the trade buff on rogue (6sec duration)
                            target->SetReducedThreatPercent(0, ObjectGuid::Empty);
                            break;
                        case 57934: //Tricks of the trade buff on rogue (30sec duration)
                        {
                            if (caster && target && aurApp)
                            {
                                if (aurApp->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE || !caster->GetMisdirectionTarget())
                                    target->SetReducedThreatPercent(0, ObjectGuid::Empty);
                                else
                                    target->SetReducedThreatPercent(0,caster->GetMisdirectionTarget()->GetGUID());
                            }
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // AT APPLY & REMOVE

    switch (m_spellInfo->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_GENERIC:
        {
            if (!(mode & AURA_EFFECT_HANDLE_REAL))
                break;
            switch (GetId())
            {
                // Permanent Feign Death
                case 96733: //Permanent Feign Death (Stun)
                // Use it every time when we have port spawn
                //DELETE FROM creature_template_addon WHERE entry in(SELECT id FROM `creature` WHERE guid in(SELECT guid FROM `creature_addon` WHERE `auras` LIKE '%29266%'));
                //UPDATE creature_template SET `unit_flags` = `unit_flags` & ~(256 | 512 | 262144 | 536870912) where entry in(SELECT id FROM `creature` WHERE guid in(SELECT guid FROM `creature_addon` WHERE `auras` LIKE '%29266%'));
                case 29266:
                    if (!target)
                        break;
                    
                    if (apply)
                    {
                        target->SetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, 0x64);
                        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_EMOTES | UNIT_FLAG_UNK_15 | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    }
                    else
                    {
                        target->SetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, 0);
                        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_EMOTES | UNIT_FLAG_UNK_15 | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    }
                    break;
                // Recently Bandaged
                case 11196:
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, GetMiscValue(), apply);
                    break;
                // Unstable Power
                case 24658:
                {
                    uint32 spellId = 24659;
                    if (apply && caster)
                    {
                        SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId);

                        for (uint32 i = 0; i < spell->GetAuraOptions(m_diffMode)->CumulativeAura; ++i)
                            caster->CastSpell(target, spell->Id, true, nullptr, nullptr, GetCasterGUID());
                        break;
                    }
                    target->RemoveAurasDueToSpell(spellId);
                    break;
                }
                // Restless Strength
                case 24661:
                {
                    uint32 spellId = 24662;
                    if (apply && caster)
                    {
                        SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId);
                        for (uint32 i = 0; i < spell->GetAuraOptions(m_diffMode)->CumulativeAura; ++i)
                            caster->CastSpell(target, spell->Id, true, nullptr, nullptr, GetCasterGUID());
                        break;
                    }
                    target->RemoveAurasDueToSpell(spellId);
                    break;
                }
                // Tag Murloc
                case 30877:
                {
                    // Tag/untag Blacksilt Scout
                    target->SetEntry(apply ? 17654 : 17326);
                    break;
                }
                case 57819: // Argent Champion
                case 57820: // Ebon Champion
                case 57821: // Champion of the Kirin Tor
                case 57822: // Wyrmrest Champion
                case 93337: // Champion of Ramkahen
                case 93339: // Champion of the Earthen Ring
                case 93341: // Champion of the Guardians of Hyjal
                case 93347: // Champion of Therazane
                case 93368: // Champion of the Wildhammer Clan
                case 94158: // Champion of the Dragonmaw Clan
                case 93795: // Stormwind Champion
                case 93805: // Ironforge Champion
                case 93806: // Darnassus Champion
                case 93811: // Exodar Champion
                case 93816: // Gilneas Champion
                case 93821: // Gnomeregan Champion
                case 93825: // Orgrimmar Champion
                case 93827: // Darkspear Champion
                case 93828: // Silvermoon Champion
                case 93830: // Bilgewater Champion
                case 94462: // Undercity Champion
                case 94463: // Thunder Bluff Champion
                case 126434: // Tushui
                case 126436: // Huojin
                {
                    if (!caster || !caster->IsPlayer())
                        break;

                    uint32 FactionID = 0;
                    uint32 DungeonLevel = 0;

                    if (apply)
                    {
                        switch (m_spellInfo->Id)
                        {
                            case 57819: FactionID = 1106; break; // Argent Crusade
                            case 57820: FactionID = 1098; break; // Knights of the Ebon Blade
                            case 57821: FactionID = 1090; break; // Kirin Tor
                            case 57822: FactionID = 1091; break; // The Wyrmrest Accord
                            // Alliance factions
                            case 93795: FactionID = 72;   DungeonLevel = 0;  break; // Stormwind
                            case 93805: FactionID = 47;   DungeonLevel = 0;  break; // Ironforge
                            case 93806: FactionID = 69;   DungeonLevel = 0;  break; // Darnassus
                            case 93811: FactionID = 930;  DungeonLevel = 0;  break; // Exodar
                            case 93816: FactionID = 1134; DungeonLevel = 0;  break; // Gilneas
                            case 93821: FactionID = 54;   DungeonLevel = 0;  break; // Gnomeregan
                            case 126434: FactionID = 1353; DungeonLevel = 0; break; // Tushui Pandaren
                            // Horde factions
                            case 93825: FactionID = 76;   DungeonLevel = 0;  break; // Orgrimmar
                            case 93827: FactionID = 530;  DungeonLevel = 0;  break; // Darkspear Trolls
                            case 93828: FactionID = 911;  DungeonLevel = 0;  break; // Silvermoon
                            case 93830: FactionID = 1133; DungeonLevel = 0;  break; // Bilgewater Cartel
                            case 94462: FactionID = 68;   DungeonLevel = 0;  break; // Undercity
                            case 94463: FactionID = 81;   DungeonLevel = 0;  break; // Thunder Bluff
                            case 126436: FactionID = 1352; DungeonLevel = 0; break; // Huojin Pandaren
                            // Cataclysm factions
                            case 93337: FactionID = 1173; DungeonLevel = 83; break; // Ramkahen
                            case 93339: FactionID = 1135; DungeonLevel = 83; break; // The Earthen Ring
                            case 93341: FactionID = 1158; DungeonLevel = 83; break; // Guardians of Hyjal
                            case 93347: FactionID = 1171; DungeonLevel = 83; break; // Therazane
                            case 93368: FactionID = 1174; DungeonLevel = 83; break; // Wildhammer Clan
                            case 94158: FactionID = 1172; DungeonLevel = 83; break; // Dragonmaw Clan
                        }
                    }
                    caster->ToPlayer()->SetChampioningFaction(FactionID, DungeonLevel);
                    break;
                }
                // LK Intro VO (1)
                case 58204:
                    if (target->IsPlayer())
                    {
                        // Play part 1
                        if (apply)
                            target->PlayDirectSound(14970, target->ToPlayer());
                        // continue in 58205
                        else
                            target->CastSpell(target, 58205, true);
                    }
                    break;
                // LK Intro VO (2)
                case 58205:
                    if (target->IsPlayer())
                    {
                        // Play part 2
                        if (apply)
                            target->PlayDirectSound(14971, target->ToPlayer());
                        // Play part 3
                        else
                            target->PlayDirectSound(14972, target->ToPlayer());
                    }
                    break;
                case 62061: // Festive Holiday Mount
                    if (target->HasAuraType(SPELL_AURA_MOUNTED))
                    {
                        uint32 creatureEntry = 0;
                        if (apply)
                        {
                            if (target->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                                creatureEntry = 24906;
                            else
                                creatureEntry = 15665;
                        }
                        else
                        {
                            if (Unit::AuraEffectList const* mountAuras = target->GetAuraEffectsByType(SPELL_AURA_MOUNTED))
                                if (mountAuras->begin() != mountAuras->end())
                                    creatureEntry = (*mountAuras->begin())->GetMiscValue();
                        }

                        if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creatureEntry))
                        {
                            uint32 team = 0;
                            if (target->IsPlayer())
                                team = target->ToPlayer()->GetTeam();

                            uint32 displayID = sObjectMgr->ChooseDisplayId(team, creatureInfo);
                            sObjectMgr->GetCreatureModelRandomGender(&displayID);

                            target->SetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID, displayID);
                        }
                    }
                    break;
            }

            break;
        }
        case SPELLFAMILY_DRUID:
        {
            if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
                break;

            switch (GetId())
            {
                case 52610:                                 // Savage Roar
                case 127538:                                // Savage Roar
                {
                    if (apply)
                    {
                        target->RemoveAurasDueToSpell(GetId() == 52610 ? 127538 : 52610);
                        if (target->GetShapeshiftForm() == FORM_CAT)
                            target->CastSpell(target, 62071, true, nullptr, nullptr, GetCasterGUID());
                    }
                    else
                        target->RemoveAurasDueToSpell(62071);
                    break;
                }
                case 61336:                                 // Survival Instincts
                {
                    if (!(mode & AURA_EFFECT_HANDLE_REAL))
                        break;

                    if (apply)
                    {
                        if (!target->IsInFeralForm())
                            break;

                        target->CastSpell(target, 50322, true);
                    }
                    else
                        target->RemoveAurasDueToSpell(50322);
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            if (!(mode & AURA_EFFECT_HANDLE_REAL))
                break;

            switch (GetId())
            {
                // Smoke Bomb
                case 76577:
                {
                    if (apply)
                    {
                        if (Aura* aur = caster->AddAura(88611, target))
                        {
                            aur->SetMaxDuration(GetBase()->GetDuration());
                            aur->SetDuration(GetBase()->GetDuration());
                        }
                    }
                    else
                        target->RemoveAura(88611);
                    break;
                }
            }

            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            switch (GetId())
            {
                // Demonic Pact
                case 47236:
                    caster->CastSpell(caster, 53646, true);
                    break;
            }
            break;
        }
        default:
            break;
        }
}

void AuraEffect::HandleChannelDeathItem(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (apply || aurApp->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
        return;

    Unit* caster = GetCaster();

    if (!caster || !caster->IsPlayer())
        return;

    Player* plCaster = caster->ToPlayer();

    // Item amount
    if (GetAmount() <= 0)
        return;

    if (GetSpellInfo()->GetEffect(m_effIndex, m_diffMode)->ItemType == 0)
        return;

    //Adding items
    uint32 noSpaceForCount = 0;
    uint32 count = m_amount;

    ItemPosCountVec dest;
    InventoryResult msg = plCaster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, GetSpellInfo()->GetEffect(m_effIndex, m_diffMode)->ItemType, count, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK)
    {
        count-=noSpaceForCount;
        plCaster->SendEquipError(msg, nullptr, nullptr, GetSpellInfo()->GetEffect(m_effIndex, m_diffMode)->ItemType);
        if (count == 0)
            return;
    }

    Item* newitem = plCaster->StoreNewItem(dest, GetSpellInfo()->GetEffect(m_effIndex, m_diffMode)->ItemType, true);
    if (!newitem)
    {
        plCaster->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }
    plCaster->SendNewItem(newitem, count, true, true);
}

void AuraEffect::HandleBindSight(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* caster = GetCaster();
    if (!caster || !caster->IsPlayer())
        return;

    caster->ToPlayer()->SetViewpoint(aurApp->GetTarget(), apply);
}

void AuraEffect::HandleForceReaction(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsPlayer())
        return;

    auto player = target->ToPlayer();

    uint32 faction_id = GetMiscValue();
    auto faction_rank = ReputationRank(RoundingFloatValue(m_amount));

    player->GetReputationMgr().ApplyForceReaction(faction_id, faction_rank, apply);
    player->GetReputationMgr().SendForceReactions();

    // stop fighting if at apply forced rank friendly or at remove real rank friendly
    if ((apply && faction_rank >= REP_FRIENDLY) || (!apply && player->GetReputationRank(faction_id) >= REP_FRIENDLY))
        player->StopAttackFaction(faction_id);
}

void AuraEffect::HandleAuraEmpathy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsCreature())
        return;

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(target->GetEntry());
    if (ci && ci->Type == CREATURE_TYPE_BEAST)
        target->ApplyModUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
}

void AuraEffect::HandleAuraModFaction(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->setFaction(GetMiscValue());
        if (target->IsPlayer())
        {
            //target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
            target->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
            target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_IGNORE_REPUTATION);
        }
    }
    else
    {
        target->RestoreFaction();
        if (target->IsPlayer())
        {
            //target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
            target->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
            target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_IGNORE_REPUTATION);
        }
    }
}

void AuraEffect::HandleComprehendLanguage(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    else
    {
        if (target->HasAuraType(GetAuraType()))
            return;

        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    }
}

void AuraEffect::HandleAuraLinked(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    Unit* target = aurApp->GetTarget();

    uint32 triggeredSpellId = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->TriggerSpell;
    SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggeredSpellId);
    if (!triggeredSpellInfo)
        return;

    if (mode & AURA_EFFECT_HANDLE_REAL)
    {
        if (apply)
        {
            Unit* caster = triggeredSpellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, target->GetMap()->GetDifficultyID()) ? GetCaster() : target;

            if (!caster)
                return;
            // If amount avalible cast with basepoints (Crypt Fever for example)
            if (GetAmount())
                caster->CastCustomSpell(target, triggeredSpellId, &m_amount, nullptr, nullptr, true, nullptr, this);
            else
                caster->CastSpell(target, triggeredSpellId, true, nullptr, this);
        }
        else
        {
            ObjectGuid casterGUID = triggeredSpellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, target->GetMap()->GetDifficultyID()) ? GetCasterGUID() : target->GetGUID();
            target->RemoveAura(triggeredSpellId, casterGUID, 9, aurApp->GetRemoveMode());
        }
    }
    else if (mode & AURA_EFFECT_HANDLE_REAPPLY && apply)
    {
        ObjectGuid casterGUID = triggeredSpellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, target->GetMap()->GetDifficultyID()) ? GetCasterGUID() : target->GetGUID();
        // change the stack amount to be equal to stack amount of our aura
        Aura* triggeredAura = target->GetAura(triggeredSpellId, casterGUID);
        if (triggeredAura != nullptr)
            triggeredAura->ModStackAmount(GetBase()->GetStackAmount() - triggeredAura->GetStackAmount());
    }
}

void AuraEffect::HandleAuraModFakeInebriation(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->m_invisibilityDetect.AddFlag(INVISIBILITY_DRUNK);
        target->m_invisibilityDetect.AddValue(INVISIBILITY_DRUNK, GetAmount());

        if (target->IsPlayer())
        {
            int32 oldval = target->ToPlayer()->GetInt32Value(PLAYER_FIELD_FAKE_INEBRIATION);
            target->ToPlayer()->SetInt32Value(PLAYER_FIELD_FAKE_INEBRIATION, oldval + GetAmount());
        }
    }
    else
    {
        bool removeDetect = !target->HasAuraType(SPELL_AURA_MOD_FAKE_INEBRIATE);

        target->m_invisibilityDetect.AddValue(INVISIBILITY_DRUNK, -GetAmount());

        if (target->IsPlayer())
        {
            int32 oldval = target->ToPlayer()->GetInt32Value(PLAYER_FIELD_FAKE_INEBRIATION);
            target->ToPlayer()->SetInt32Value(PLAYER_FIELD_FAKE_INEBRIATION, oldval - GetAmount());

            if (removeDetect)
                removeDetect = !target->ToPlayer()->GetDrunkValue();
        }

        if (removeDetect)
            target->m_invisibilityDetect.DelFlag(INVISIBILITY_DRUNK);
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleAuraOverrideSpells(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();

    if (!target || !target->IsInWorld())
        return;

    auto overrideId = uint32(GetMiscValue());

    if (apply)
    {
        if (auto const* overrideList = target->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_SPELLS))
        {
            for (auto const& over : *overrideList)
            {
                if (over->GetId() != GetId())
                    over->GetBase()->Remove();
            }
        }
        target->SetUInt16Value(PLAYER_FIELD_BYTES_5, PLAYER_BYTES_2_OVERRIDE_SPELLS_UINT16_OFFSET, overrideId);
        if (OverrideSpellDataEntry const* overrideSpells = sOverrideSpellDataStore.LookupEntry(overrideId))
            for (uint32 spellId : overrideSpells->Spells)
                if (spellId)
                    target->AddTemporarySpell(spellId);
    }
    else
    {
        target->SetUInt16Value(PLAYER_FIELD_BYTES_5, PLAYER_BYTES_2_OVERRIDE_SPELLS_UINT16_OFFSET, 0);
        if (OverrideSpellDataEntry const* overrideSpells = sOverrideSpellDataStore.LookupEntry(overrideId))
            for (uint32 spellId : overrideSpells->Spells)
                if (spellId)
                    target->RemoveTemporarySpell(spellId);
    }
}

void AuraEffect::HandleAuraSetVehicle(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    uint32 vehicleId = GetMiscValue();
    if (apply)
    {
        if (!target->CreateVehicleKit(vehicleId, 0, GetId()))
            return;
    }
    else if (target->GetVehicleKit())
        target->RemoveVehicleKit();

    if (!target->IsPlayer())
        return;

    if (apply)
    {
        target->SendSetVehicleRecId(vehicleId);

        if (Vehicle * veh = target->GetVehicleKit())
            veh->Reset();
        else 
            return;

        target->ToPlayer()->SendOnCancelExpectedVehicleRideAura();
    }
}

void AuraEffect::HandlePreventResurrection(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (!aurApp->GetTarget()->IsPlayer())
        return;

    if (apply)
        aurApp->GetTarget()->RemoveFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_RELEASE_TIMER);
    else if (!aurApp->GetTarget()->GetMap()->Instanceable())
        aurApp->GetTarget()->SetFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_RELEASE_TIMER);
}

void AuraEffect::AuraSpellTrigger(Unit* target, Unit* caster, SpellEffIndex effIndex) const
{
    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::AuraSpellTrigger: Spell %u in Effect %d", GetId(), effIndex);

    if (std::vector<AuraPeriodickTrigger> const* spellTrigger = sSpellMgr->GetSpellAuraTrigger(m_spellInfo->Id))
    {
        bool check = false;
        Unit* triggerTarget = GetBase()->GetUnitOwner();
        Unit* triggerCaster = caster;
        float basepoints0 = m_amount;
        uint8 baseOption = 0; // One option for aura, proc only if this option execute

        Item* castItem = GetBase()->GetCastItemGUID() && caster && caster->IsPlayer() ? caster->ToPlayer()->GetItemByGuid(GetBase()->GetCastItemGUID()) : nullptr;

        for (const auto& itr : *spellTrigger)
        {
            if (!(itr.effectmask & (1 << effIndex)))
                continue;

            //if(target)
            {
                if (itr.target)
                    triggerTarget = (target ? target : caster)->GetUnitForLinkedSpell(caster, target, itr.target);

                if(!triggerTarget)
                {
                    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::AuraSpellTrigger: not exist triggerTarget");
                    check = true;
                    continue;
                }
            }

            if (caster && itr.caster)
                triggerCaster = caster->GetUnitForLinkedSpell(caster, target, itr.caster);

            if(!triggerCaster)
            {
                //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::AuraSpellTrigger: not exist triggerCaster");
                check = true;
                continue;
            }

            if(itr.chance != 0)
            {
                if(!roll_chance_i(itr.chance))
                    continue;
            }

            if (itr.hastalent)
                if (triggerCaster->HasAuraLinkedSpell(caster, triggerTarget, itr.hastype, itr.hastalent, itr.hasparam))
                    continue;

            if (itr.hastalent2)
                if (triggerCaster->HasAuraLinkedSpell(caster, triggerTarget, itr.hastype2, itr.hastalent2, itr.hasparam2))
                    continue;

            float bp0 = itr.bp0;
            float bp1 = itr.bp1;
            float bp2 = itr.bp2;
            int32 spell_trigger = m_amount;

            if (itr.spell_trigger != 0)
                spell_trigger = abs(itr.spell_trigger);

            switch (itr.option)
            {
                case AURA_TRIGGER: //0
                {
                    triggerCaster->CastSpell(triggerTarget, spell_trigger, true);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_BP: //1
                {
                    if (itr.spell_trigger < 0)
                        basepoints0 *= -1;

                    if(bp0)
                        basepoints0 += bp0;
                    if(bp1)
                        basepoints0 /= bp1;
                    if(bp2)
                        basepoints0 *= bp2;

                    triggerCaster->CastCustomSpell(triggerTarget, spell_trigger, &basepoints0, &basepoints0, &basepoints0, true, castItem, this);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_BP_CUSTOM: //2
                {
                    triggerCaster->CastCustomSpell(triggerTarget, spell_trigger, &bp0, &bp1, &bp2, true, castItem, this);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_CHECK_COMBAT: //3
                {
                    if (itr.spell_trigger < 0 && !caster->isInCombat())
                        triggerCaster->CastSpell(triggerTarget, spell_trigger, true);
                    else if (itr.spell_trigger > 0 && caster->isInCombat())
                        triggerCaster->CastSpell(triggerTarget, spell_trigger, true);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_DEST: //4
                {
                    triggerCaster->CastSpell(triggerTarget->GetPositionX(), triggerTarget->GetPositionY(), triggerTarget->GetPositionZ(), spell_trigger, true, castItem, this);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_DYNOBJECT: //5
                {
                    std::list<DynamicObject*> list;
                    triggerCaster->GetDynObjectList(list, GetId());
                    if(!list.empty())
                    {
                        Unit* owner = triggerCaster->GetAnyOwner();
                        for (auto dynObj : list)
                        {
                            if(dynObj)
                                triggerCaster->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), spell_trigger, true, castItem, this, owner ? owner->GetGUID() : ObjectGuid::Empty);
                        }
                    }
                    check = true;
                    break;
                }
                case AURA_TRIGGER_FROM_SUMMON_SLOT: //6
                {
                    if(itr.slot < MAX_SUMMON_SLOT)
                        if(triggerCaster->m_SummonSlot[itr.slot])
                            if(Creature* summon = triggerCaster->GetMap()->GetCreature(triggerCaster->m_SummonSlot[itr.
                                slot]))
                                summon->CastSpell(summon, spell_trigger, true, castItem, this, triggerCaster->GetGUID());
                    check = true;
                    break;
                }
                case AURA_TRIGGER_AREATRIGGER: //7
                {
                    std::list<AreaTrigger*> list;
                    triggerCaster->GetAreaObjectList(list, GetId());
                    if(!list.empty())
                    {
                        Unit* owner = triggerCaster->GetAnyOwner();
                        for (auto areaObj : list)
                        {
                            if(areaObj)
                                triggerCaster->CastSpell(areaObj->GetPositionX(), areaObj->GetPositionY(), areaObj->GetPositionZ(), spell_trigger, true, castItem, this, owner ? owner->GetGUID() : ObjectGuid::Empty);
                        }
                    }
                    check = true;
                    break;
                }
                case AURA_TRIGGER_FROM_SUMMON_SLOT_DEST: //8
                {
                    if(itr.slot < MAX_SUMMON_SLOT)
                        if(triggerCaster->m_SummonSlot[itr.slot])
                            if(Creature* summon = triggerCaster->GetMap()->GetCreature(triggerCaster->m_SummonSlot[itr.
                                slot]))
                                summon->CastSpell(summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ(), spell_trigger, true, castItem, this, triggerCaster->GetGUID());
                    check = true;
                    break;
                }
                case AURA_TRIGGER_FROM_SUMMON_DEST: //9
                {
                    GuidList* summonList = triggerCaster->GetSummonList(itr.slot);
                    for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                    {
                        if(Creature* summon = ObjectAccessor::GetCreature(*triggerCaster, (*iter)))
                            summon->CastSpell(summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ(), spell_trigger, true, castItem, this, triggerCaster->GetGUID());
                    }
                    check = true;
                    break;
                }
                case AURA_TRIGGER_AREATRIGGER_CAST: //10
                {
                    std::list<AreaTrigger*> list;
                    triggerCaster->GetAreaObjectList(list, GetId());
                    if(!list.empty())
                    {
                        for (auto areaObj : list)
                        {
                            if(areaObj)
                                areaObj->CastAction();
                        }
                    }
                    check = true;
                    break;
                }
                case AURA_TRIGGER_TARGETCASTER: //11
                {
                    triggerTarget->CastSpell(triggerTarget, spell_trigger, true, castItem, nullptr, triggerCaster->GetGUID());
                    check = true;
                    break;
                }
                case AURA_TRIGGER_IF_ENERGE: //12
                {
                    int32 rbp0 = RoundingFloatValue(bp0);
                    int32 rbp1 = RoundingFloatValue(bp1);

                    if (bp1)
                    {
                        if (triggerTarget->GetPowerPct(Powers(rbp0)) >= rbp1)
                            if (!triggerTarget->HasAura(spell_trigger))
                                triggerCaster->CastSpell(triggerTarget, spell_trigger, true);
                    }
                    else
                    {
                        if (triggerTarget->GetPowerPct(Powers(rbp0)) <= rbp1)
                            if (!triggerTarget->HasAura(spell_trigger))
                                triggerCaster->CastSpell(triggerTarget, spell_trigger, true);
                    }
                    check = true;
                    break;
                }
                case AURA_TRIGGER_CAST_OR_STACK: //13
                {
                    if (Aura* aura = triggerTarget->GetAura(spell_trigger))
                        aura->ModStackAmount(itr.slot ? int32(itr.slot) : 1);
                    else
                        triggerTarget->CastSpell(triggerTarget, spell_trigger, true, castItem, nullptr, triggerCaster->GetGUID());
                    check = true;
                    break;
                }
                case AURA_TRIGGER_AMOUNT: // 14
                {
                    if (!basepoints0)
                        break;

                    const_cast<AuraEffect*>(this)->SetAmount(0);
                    if (itr.spell_trigger < 0)
                        basepoints0 *= -1;

                    triggerCaster->CastCustomSpell(triggerTarget, spell_trigger, &basepoints0, &basepoints0, &basepoints0, true, castItem, this);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_2: //15
                {
                    triggerCaster->CastSpell(triggerTarget, spell_trigger, false);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_SEND_SPELL_VISUAL: //16
                {
                    triggerCaster->SendSpellCreateVisual(m_spellInfo, triggerTarget, triggerTarget);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_REMOVE_AURA: //17
                {
                    if (itr.spell_trigger < 0)
                        triggerTarget->RemoveAurasDueToSpell(spell_trigger, triggerCaster->GetGUID());
                    else
                        triggerTarget->RemoveAurasDueToSpell(spell_trigger);
                    check = true;
                    break;
                }
                case AURA_TRIGGER_IN_MOVING: //18
                {
                    if (triggerTarget->isMoving())
                        triggerCaster->CastSpell(triggerTarget, spell_trigger, true);
                    check = true;
                    break;
                }
            }
            if (check && !baseOption && itr.option2)
                baseOption = itr.option2;
        }
        if (check)
        {
            switch (baseOption)
            {
                case AURA_REMOVE_ON_PROC: //1
                    GetBase()->Remove();
                break;
            }
        }
    }
}

bool AuraEffect::AuraCostPower(Unit* caster) const
{
    if (!caster)
        return true;

    if (GetSpellInfo()->NoPower() || GetBase()->powerData.empty())
        return true;

    for (SpellPowerEntry const* power : GetBase()->powerData)
    {
        if (power->ManaPerSecond || power->PowerPctPerSecond)
        {
            float percCostOnTick = m_period * 100.0f / 1000.0f;

            auto powertype = Powers(power->PowerType);
            if (powertype == POWER_HEALTH)
            {
                uint32 reqHealth = power->ManaPerSecond;
                if (power->PowerPctPerSecond)
                    reqHealth += CalculatePct(caster->GetMaxHealth(), power->PowerPctPerSecond);

                reqHealth = CalculatePct(reqHealth, percCostOnTick);

                if (reqHealth < caster->GetHealth())
                    caster->ModifyHealth(-1 * reqHealth, nullptr, GetId());
                else
                {
                    GetBase()->Remove();
                    return false;
                }
            }
            else
            {
                int32 reqPower = power->ManaPerSecond;
                if (power->PowerPctPerSecond)
                    reqPower += CalculatePct(caster->GetMaxPower(powertype), power->PowerPctPerSecond);

                if (Player* modOwner = caster->GetSpellModOwner())
                {
                    if (power->OrderIndex == POWER_INDEX_MAIN)
                        modOwner->ApplySpellMod(GetId(), SPELLMOD_COST, reqPower);
                    if (power->OrderIndex == POWER_INDEX_SECOND)
                        modOwner->ApplySpellMod(GetId(), SPELLMOD_SPELL_COST2, reqPower);
                }

                reqPower = CalculatePct(reqPower, percCostOnTick);

                if (reqPower <= caster->GetPower(powertype))
                    caster->ModifyPower(powertype, -1 * reqPower, true, GetSpellInfo());
                else
                {
                    GetBase()->Remove();
                    return false;
                }
            }
        }
    }
    return true;
}

void AuraEffect::HandlePeriodicDummyAuraTick(Unit* target, Unit* caster, SpellEffIndex /*effIndex*/) const
{
    if (GetId() == 102522)
    {
        switch(rand()%4)
        {
            case 1:
                caster->CastSpell(caster, 109090);
                break;
            case 2:
                caster->CastSpell(caster, 109105);
                break;
        }
        
        if(GetBase()->GetDuration() < 1000)
            caster->CastSpell(caster, 109107);
    }

    uint32 trigger_spell_id = m_spellInfo->GetEffect(m_effIndex, m_diffMode)->TriggerSpell;

    switch (GetSpellInfo()->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_GENERIC:
            switch (GetId())
            {
                case 200050: // Shade of Xavius - Apocolyptic Nightmare
                {
                    Position const centrPos = {2713.65f, 1327.26f, 128.36f};
                    Position pos;
                    centrPos.SimplePosXYRelocationByAngle(pos, float(urand(0, 45)), frand(0.0f, 6.28f));
                    caster->CastSpell(pos, 200067, true);
                    break;
                }
                case 146310: // Restless Agility
                case 146317: // Restless Spirit
                {
                    if (Aura* aur = GetBase())
                    {
                        if (aur->GetStackAmount() > 1)
                            aur->SetStackAmount(aur->GetStackAmount() - 1);
                    }
                    break;
                }
                case 215632: // Focused Lightning
                {
                    if (Aura* aur = GetBase())
                    {
                        if (!caster->HasAura(215631))
                            if (aur->GetStackAmount() > 1)
                                aur->SetStackAmount(aur->GetStackAmount() - 1);
                    }
                    break;
                }
                case 224346: // Solemn Night
                {
                    if (Aura* aur = GetBase())
                    {
                        if (Aura* aur2 = caster->GetAura(224347))
                            if (aur2->GetStackAmount() > 1)
                                aur2->SetStackAmount(aur2->GetStackAmount() - 1);
                    }
                    break;
                }
                case 25281: // Turkey Marker
                {
                    if (Aura* aur = GetBase())
                    {
                        if (aur->GetStackAmount() >= 15)
                        {
                            target->CastSpell(target, 25285, true);
                            target->RemoveAura(25281);
                        }
                    }
                    break;
                }
                case 66149: // Bullet Controller Periodic - 10 Man
                {
                    if (!caster)
                        break;

                    caster->CastCustomSpell(66152, SPELLVALUE_MAX_TARGETS, urand(1, 6), target, true);
                    caster->CastCustomSpell(66153, SPELLVALUE_MAX_TARGETS, urand(1, 6), target, true);
                    break;
                }
                case 62399: // Overload Circuit
                    if (target->GetMap()->IsDungeon() && int(target->GetAppliedAuras().count(62399)) >= (target->GetMap()->IsHeroic() ? 4 : 2))
                    {
                         target->CastSpell(target, 62475, true); // System Shutdown
                         if (Unit* veh = target->GetVehicleBase())
                             veh->CastSpell(target, 62475, true);
                    }
                    break;
                case 64821: // Fuse Armor (Razorscale)
                    if (GetBase()->GetStackAmount() == GetSpellInfo()->GetAuraOptions(m_diffMode)->CumulativeAura)
                    {
                        target->CastSpell(target, 64774, true, nullptr, nullptr, GetCasterGUID());
                        target->RemoveAura(64821);
                    }
                    break;
            }
            break;
        case SPELLFAMILY_DRUID:
        {
            switch (GetSpellInfo()->Id)
            {
                case 186370: // Mark of Shifting
                {
                    float heal = CalculatePct(caster->GetMaxHealth(), m_amount); // hack
                    if (Aura* aura = caster->GetAura(77495)) // Mastery: Harmony
                    {
                        float modDif = 0.f;
                        uint32 modCount = 0;
                        uint32 modMaxCount = 9;
                        if (AuraEffect* eff = aura->GetEffect(EFFECT_0))
                            modDif = eff->GetAmount();
                        if (Unit::AuraEffectList const* mPeriodic = target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL))
                            for (Unit::AuraEffectList::const_iterator i = mPeriodic->begin(); i != mPeriodic->end(); ++i)
                                if ((*i)->GetCasterGUID() == caster->GetGUID())
                                    modCount++;

                        if (modCount >= modMaxCount)
                            modCount = modMaxCount;

                        if (modCount && modDif)
                            AddPct(heal, modDif * modCount);
                    }
                    caster->CastCustomSpell(caster, 224392, &heal, nullptr, nullptr, true);
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            switch (GetSpellInfo()->Id)
            {
                // Camouflage
                case 80326:
                {
                    if (!caster || (caster->isMoving() && !caster->HasAura(119449) && !caster->isPet()) || caster->HasAura(80325))
                        return;

                    if (caster->HasAura(119449) || (caster->GetOwner() && caster->GetOwner()->HasAura(119449)))
                        caster->CastSpell(caster, 119450, true);
                    else
                        caster->CastSpell(caster, 80325, true);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_MONK:
        {
            if (trigger_spell_id == 120086)
            {
                if (GetTickNumber() > 1)
                    return;
            }
            break;
        }
        default:
            break;
    }

    if(caster && trigger_spell_id)
    {
        std::list<DynamicObject*> list;
        caster->GetDynObjectList(list, GetId());
        if(!list.empty())
        {
            Unit* owner = caster->GetAnyOwner();
            for (auto dynObj : list)
            {
                if(dynObj)
                    caster->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), trigger_spell_id, true, nullptr, this, owner ? owner->GetGUID() : ObjectGuid::Empty);
            }
        }
        else if(target)
        {
            SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(trigger_spell_id);
            if(!triggeredSpellInfo)
                return;

            SpellCastResult checkResult = triggeredSpellInfo->CheckExplicitTarget(caster, target);
            if (checkResult == SPELL_CAST_OK)
                caster->CastSpell(target, trigger_spell_id, true, nullptr, this);
            else if (Unit* _target = (caster->ToPlayer() ? caster->ToPlayer()->GetSelectedUnit() : caster->getVictim()))
            {
                checkResult = triggeredSpellInfo->CheckExplicitTarget(caster, _target);
                if (checkResult == SPELL_CAST_OK)
                    caster->CastSpell(_target, trigger_spell_id, true, nullptr, this);
                else
                    caster->CastSpell(caster, trigger_spell_id, true, nullptr, this);
            }
        }
    }
}

void AuraEffect::HandlePeriodicTriggerSpellAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const
{
    // generic casting code with custom spells and target/caster customs
    uint32 triggerSpellId = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->TriggerSpell;

    SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId);
    SpellInfo const* auraSpellInfo = GetSpellInfo();
    uint32 auraId = auraSpellInfo->Id;
    Item* castItem = nullptr;

    if (Aura* aura = GetBase())
    {
        if (ObjectGuid itemGUID = aura->GetCastItemGUID())
        {
            if (caster)
                if (Player* plr = caster->ToPlayer())
                    castItem = plr->GetItemByGuid(itemGUID);
        }
    }

    // specific code for cases with no trigger spell provided in field
    if (triggeredSpellInfo == nullptr)
    {
        switch (auraSpellInfo->ClassOptions.SpellClassSet)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (auraId)
                {
                    // Cannon Barrage, Skulloc
                    case 168129:
                    {
                        float x = frand(6752, 6857);
                        float y = frand(-1015, -953);

                        caster->CastSpell(x, y, caster->GetPositionZ(), 168384, true);
                        caster->CastSpell(x, y, caster->GetPositionZ(), 168385, true);
                        return;
                    }
                    // Earth's Vengeance, Morchok, Dragon Soul
                    case 103176:
                    {
                        for (uint8 i = 0; i < 2; ++i)
                        {
                            Position pos;
                            caster->GetNearPosition(pos, 40.0f, frand(0.0f, 2 * M_PI));
                            if (caster->GetDistance(pos) >= 20.0f)
                                caster->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 103177, true);
                        }
                        return;
                    }
                    // Cluster Cluck: Remote Control Fireworks Visual
                    case 74177:
                        target->PlayDistanceSound(6820);
                        return;
                    // resistible Pool Pony: Naga Hatchling Proximity Control - Master
                    case 71917:
                        target->PlayDistanceSound(3437);
                        return;
                    // Thaumaturgy Channel
                    case 9712:
                        triggerSpellId = 21029;
                        break;
                    // Brood Affliction: Bronze
                    case 23170:
                        triggerSpellId = 23171;
                        break;
                    // Party G.R.E.N.A.D.E
                    case 51510:
                        triggerSpellId = 51508;
                        break;
                    // Restoration
                    case 24379:
                    case 23493:
                    {
                        if (caster)
                        {
                            int32 heal = caster->CountPctFromMaxHealth(10);
                            caster->HealBySpell(target, auraSpellInfo, heal);

                            if (int32 mana = caster->GetMaxPower(POWER_MANA))
                            {
                                mana /= 10;
                                caster->EnergizeBySpell(caster, 23493, mana, POWER_MANA);
                            }
                        }
                        return;
                    }
                    // Nitrous Boost
                    case 27746:
                        if (caster && target->GetPower(POWER_MANA) >= 10)
                        {
                            target->ModifyPower(POWER_MANA, -10, true);
                            target->SendEnergizeSpellLog(caster, 27746, 10, 0, POWER_MANA);
                        }
                        else
                            target->RemoveAurasDueToSpell(27746);
                        return;
                    // Frost Blast
                    case 27808:
                        if (caster)
                            caster->CastCustomSpell(29879, SPELLVALUE_BASE_POINT0, int32(target->CountPctFromMaxHealth(21, caster)), target, true, nullptr, this);
                        return;
                    // Inoculate Nestlewood Owlkin
                    case 29528:
                        if (!target->IsCreature()) // prevent error reports in case ignored player target
                            return;
                        break;
                    // Feed Captured Animal
                    case 29917:
                        triggerSpellId = 29916;
                        break;
                    // Extract Gas
                    case 30427:
                    {
                        // move loot to player inventory and despawn target
                        if (caster && caster->IsPlayer() && target->IsCreature() && target->ToCreature()->GetCreatureTemplate()->Type == CREATURE_TYPE_GAS_CLOUD)
                        {
                            Player* player = caster->ToPlayer();
                            Creature* creature = target->ToCreature();
                            // missing lootid has been reported on startup - just return
                            if (!creature->GetCreatureTemplate()->SkinLootId)
                                return;

                            player->AutoStoreLoot(creature->GetCreatureTemplate()->SkinLootId, LootTemplates_Skinning, true);

                            creature->DespawnOrUnsummon();
                        }
                        return;
                    }
                    // Quake
                    case 30576:
                        triggerSpellId = 30571;
                        break;
                    // Doom
                    // TODO: effect trigger spell may be independant on spell targets, and executed in spell finish phase
                    // so instakill will be naturally done before trigger spell
                    case 31347:
                    {
                        target->CastSpell(target, 31350, true, nullptr, this);
                        target->Kill(target);
                        return;
                    }
                    // Spellcloth
                    case 31373:
                    {
                        // Summon Elemental after create item
                        target->SummonCreature(17870, 0, 0, 0, target->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                        return;
                    }
                    // Flame Quills
                    case 34229:
                    {
                        // cast 24 spells 34269-34289, 34314-34316
                        for (uint32 spell_id = 34269; spell_id != 34290; ++spell_id)
                            target->CastSpell(target, spell_id, true, nullptr, this);
                        for (uint32 spell_id = 34314; spell_id != 34317; ++spell_id)
                            target->CastSpell(target, spell_id, true, nullptr, this);
                        return;
                    }
                    // Remote Toy
                    case 37027:
                        triggerSpellId = 37029;
                        break;
                    // Eye of Grillok
                    case 38495:
                        triggerSpellId = 38530;
                        break;
                    // Absorb Eye of Grillok (Zezzak's Shard)
                    case 38554:
                    {
                        if (!caster || !target->IsCreature())
                            return;

                        caster->CastSpell(caster, 38495, true, nullptr, this);

                        Creature* creatureTarget = target->ToCreature();

                        creatureTarget->DespawnOrUnsummon();
                        return;
                    }
                    // Tear of Azzinoth Summon Channel - it's not really supposed to do anything, and this only prevents the console spam
                    case 39857:
                        triggerSpellId = 39856;
                        break;
                    // Personalized Weather
                    case 46736:
                        triggerSpellId = 46737;
                        break;
                    //Sha pool (Immerseus)
                    case 143462: 
                        {
                            if (caster && caster->ToCreature())
                            {
                                switch (caster->GetEntry())
                                {
                                case 71544: //npc_sha_pool
                                    triggerSpellId = 143297; //Sha splash
                                    break;
                                case 71543: //boss_immerseus
                                    triggerSpellId = 143460; //Sha pool dmg
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                        break;
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
        // Spell exist but require custom code
        switch (auraId)
        {
            case 211927:
            {
                if (caster)
                    caster->CastSpell(caster, triggerSpellId, true);
                return;
            }
            case 229610: //Demonic Portal
            case 229241: //Command: Fel Beam
            case 218809: //Call of Night
            case 248789: //Spear of Doom
            {
                if (caster && target)
                    target->CastSpell(target, triggerSpellId, true, nullptr, nullptr, caster->GetGUID());
                return;
            }
            case 229244: //Command: Fel Beam
            {
                if (caster)
                    if (Unit* owner = caster->GetAnyOwner())
                        target->CastSpell(target, triggerSpellId, true, nullptr, nullptr, owner->GetGUID());
                return;
            }
            case 229287: //Command: Bombardment
            {
                if (caster)
                    if (Unit* owner = caster->GetAnyOwner())
                        caster->CastSpell(caster, triggerSpellId, true, nullptr, nullptr, owner->GetGUID());
                return;
            }
            // Frailty
            case 247456:
            {
                target->CastCustomSpell(target, triggerSpellId, &m_amount, nullptr, nullptr, true, nullptr, this);
                if (AuraEffect* aurEff = GetBase()->GetEffect(EFFECT_1))
                    aurEff->SetAmount(0);
                return;
            }
            case 193440: // Demonwrath
            {
                if (!caster)
                    return;

                target = nullptr;
                if(Player* player = caster->ToPlayer())
                    target = player->GetPet();

                if (!target || !target->IsInWorld())
                    return;

                caster->CastSpell(target, triggerSpellId, true);
                return;
            }
            case 196070:
                if (caster)
                    caster->CastSpell(caster, triggerSpellId, true);
                return;
            case 210312:
                if (caster)
                {
                    if (Unit* owner = caster->GetAnyOwner())
                        caster->CastSpell(caster, triggerSpellId, true, nullptr, nullptr, owner->GetGUID());
                }
                return;
            case 202977: //Infested Breath
                if (caster)
                    caster->CastSpell(caster, triggerSpellId, true);
                return;
            case 191855: //Serpentrix: Toxic Wound
                target->CastSpell(target, 191856, true, nullptr, nullptr, GetCasterGUID());
                return;
            case 19483: // Immolation
            {
                if (!caster->isInCombat())
                    return;
                break;
            }
            case 51769: // Emblazon Runeblade
                if (caster)
                    caster->CastSpell(caster, triggerSpellId, false);
                return;
            // Hour of Twilight, Ultraxion, Dragon Soul
            case 106371:
                if (caster)
                    if (Creature* pUltraxion = caster->ToCreature())
                        if (Unit* pTarget = pUltraxion->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0.0f, true))
                            pUltraxion->CastSpell(pTarget, 105925, true);
                        return;
            case 113656: // Fists of Fury
            {
                if (!caster->IsPlayer())
                    return;
                break;
            }
            case 146194: // Flurry of Xuen
            {
                switch (effIndex)
                {
                    case EFFECT_0:
                    {
                        if (caster->getClass() == CLASS_HUNTER) 
                            return;
                        break;
                    }
                    case EFFECT_1:
                    {
                        if (caster->getClass() != CLASS_HUNTER) 
                            return;

                        if (Unit* _target = caster->getVictim())
                            if (caster->HasInArc(static_cast<float>(M_PI), _target))
                                caster->CastSpell(_target, triggerSpellId, true);
                        return;
                    }
                    default:
                        break;
                }
                break;
            }
            case 107851: // Focused Assault, Hagara, Dragon Soul
            {    
                if (caster)
                    if (Creature* pHagara = caster->ToCreature())
                        if (Unit* pTarget = pHagara->getVictim())
                        {
                            if (!pHagara->GetMap()->IsHeroic())
                                if (!pHagara->isInFront(pTarget) || !pHagara->IsWithinMeleeRange(pTarget))
                                {
                                    GetBase()->Remove();
                                    return;
                                }

                            pHagara->CastSpell(pHagara->getVictim(), 107850, true);
                        }
                return;
            }
            case 105285: // Target, Hagara, Dragon Soul
            {    
                if (caster)
                    caster->CastSpell(target, triggerSpellId, true);
                return;
            }
            // The Biggest Egg Ever: Mechachicken's Rocket Barrage Aura Effect
            case 71416:
                // prock 71419 with this action.
                target->PlayDistanceSound(23829);
                target->HandleEmoteCommand(403);
                break;
            case 114889: // Stone Bulwark Totem
            {
                if (Unit * shaman = caster->GetOwner())
                {
                    int32 SPD    = shaman->GetSpellPowerDamage();
                    float amount = SPD * 0.875f * (GetTickNumber() != 1 ? 1: 4);
                    caster->CastCustomSpell(shaman, triggerSpellId, &amount, nullptr, nullptr, true);
                }
                return;
            }
            // Pursuing Spikes (Anub'arak)
            case 65920:
            case 65922:
            case 65923:
            {
                Unit* permafrostCaster = nullptr;
                Aura* permafrostAura = target->GetAura(66193);
                if (!permafrostAura)
                    permafrostAura = target->GetAura(67855);
                if (!permafrostAura)
                    permafrostAura = target->GetAura(67856);
                if (!permafrostAura)
                    permafrostAura = target->GetAura(67857);

                if (permafrostAura)
                    permafrostCaster = permafrostAura->GetCaster();

                if (permafrostCaster)
                {
                    if (Creature* permafrostCasterCreature = permafrostCaster->ToCreature())
                        permafrostCasterCreature->DespawnOrUnsummon(3000);

                    target->CastSpell(target, 66181, false);
                    target->RemoveAllAuras();
                    if (Creature* targetCreature = target->ToCreature())
                        targetCreature->DisappearAndDie();
                }
                break;
            }
            // Negative Energy Periodic
            case 46284:
                target->CastCustomSpell(triggerSpellId, SPELLVALUE_MAX_TARGETS, m_tickNumber / 10 + 1, nullptr, true, nullptr, this);
                return;
            // Poison (Grobbulus)
            case 28158:
            case 54362:
            // Slime Pool (Dreadscale & Acidmaw)
            case 66882:
                target->CastCustomSpell(triggerSpellId, SPELLVALUE_RADIUS_MOD, (int32)((((float)m_tickNumber / 60) * 0.9f + 0.1f) * 10000 * 2 / 3), nullptr, true, nullptr, this);
                return;
            // Slime Spray - temporary here until preventing default effect works again
            // added on 9.10.2010
            case 69508:
            {
                if (caster)
                    caster->CastSpell(target, triggerSpellId, true, nullptr, nullptr, caster->GetGUID());
                return;
            }
            case 24745: // Summon Templar, Trigger
            case 24747: // Summon Templar Fire, Trigger
            case 24757: // Summon Templar Air, Trigger
            case 24759: // Summon Templar Earth, Trigger
            case 24761: // Summon Templar Water, Trigger
            case 24762: // Summon Duke, Trigger
            case 24766: // Summon Duke Fire, Trigger
            case 24769: // Summon Duke Air, Trigger
            case 24771: // Summon Duke Earth, Trigger
            case 24773: // Summon Duke Water, Trigger
            case 24785: // Summon Royal, Trigger
            case 24787: // Summon Royal Fire, Trigger
            case 24791: // Summon Royal Air, Trigger
            case 24792: // Summon Royal Earth, Trigger
            case 24793: // Summon Royal Water, Trigger
            case 68281: // Goblin quest: KTC Snapflash
            {
                // All this spells trigger a spell that requires reagents; if the
                // triggered spell is cast as "triggered", reagents are not consumed
                if (caster)
                    caster->CastSpell(target, triggerSpellId, false);
                return;
            }
            case 106768:
                return;
        }
    }

    // Reget trigger spell proto
    triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId);

    if (triggeredSpellInfo)
    {
        if (Unit* triggerCaster = triggeredSpellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, target->GetMap()->GetDifficultyID()) ? caster : target)
        {
            std::list<DynamicObject*> list;
            triggerCaster->GetDynObjectList(list, GetId());
            if (!list.empty())
            {
                Unit* owner = caster->GetAnyOwner();
                for (auto dynObj : list)
                    if (dynObj)
                        caster->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), triggerSpellId, true, nullptr, this, owner ? owner->GetGUID() : ObjectGuid::Empty);
            }
            else if (target)
            {
                SpellCastResult checkResult = triggeredSpellInfo->CheckExplicitTarget(triggerCaster, target);
                if (checkResult == SPELL_CAST_OK)
                    triggerCaster->CastSpell(target, triggeredSpellInfo, true, castItem, this);
                else if (Unit* _target = (triggerCaster->ToPlayer() ? triggerCaster->ToPlayer()->GetSelectedUnit() : triggerCaster->getVictim()))
                {
                    checkResult = triggeredSpellInfo->CheckExplicitTarget(triggerCaster, _target);
                    if (checkResult == SPELL_CAST_OK)
                        triggerCaster->CastSpell(_target, triggeredSpellInfo, true, castItem, this);
                    else
                        triggerCaster->CastSpell(triggerCaster, triggeredSpellInfo, true, castItem, this);
                }
            }
        }
    }
    else
    {
        Creature* c = target->ToCreature();
        if (!c || !caster || !sScriptMgr->OnDummyEffect(caster, GetId(), SpellEffIndex(GetEffIndex()), target->ToCreature()) || !c->AI()->sOnDummyEffect(caster, GetId(), SpellEffIndex(GetEffIndex())))
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandlePeriodicTriggerSpellAuraTick: Spell %u has non-existent spell %u in EffectTriggered[%d] and is therefor not triggered.", GetId(), triggerSpellId, GetEffIndex());
    }
}

void AuraEffect::HandlePeriodicTriggerSpellWithValueAuraTick(Unit* target, Unit* caster, SpellEffIndex /*effIndex*/) const
{
    uint32 triggerSpellId = GetSpellInfo()->GetEffect(m_effIndex, m_diffMode)->TriggerSpell;
    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
    {
        if (Unit* triggerCaster = triggeredSpellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, target->GetMap()->GetDifficultyID()) ? caster : target)
        {
            float basepoints0 = GetAmount();
            GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), basepoints0, target);
            triggerCaster->CastCustomSpell(target, triggerSpellId, &basepoints0, nullptr, nullptr, true, nullptr, this);
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandlePeriodicTriggerSpellWithValueAuraTick: Spell %u Trigger %u", GetId(), triggeredSpellInfo->Id);
        }
    }
    else
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS,"AuraEffect::HandlePeriodicTriggerSpellWithValueAuraTick: Spell %u has non-existent spell %u in EffectTriggered[%d] and is therefor not triggered.", GetId(), triggerSpellId, GetEffIndex());
}

void AuraEffect::HandlePeriodicDamageAurasTick(Unit* target, Unit* caster, SpellEffIndex effIndex, bool lastTick) const
{
    if (!caster || !target->isAlive())
        return;

    if (lastTick && !GetBase()->HasAuraAttribute(AURA_ATTR_REAPPLIED_AURA) &&
        (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION_RECOVERY) || !m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME)))
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    // Consecrate ticks can miss and will not show up in the combat log
    if (GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(target, GetSpellInfo(), false, 1 << GetEffIndex()) != SPELL_MISS_NONE)
        return;

    // some auras remove at specific health level or more
    if (GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE && !lastTick)
    {
        switch (GetSpellInfo()->Id)
        {
            case 43093: case 31956: case 38801:  // Grievous Wound
            case 35321: case 38363: case 39215:  // Gushing Wound
            {
                if (target->IsFullHealth())
                {
                    target->RemoveAurasDueToSpell(GetSpellInfo()->Id);
                    return;
                }
                break;
            }
            case 38772: // Grievous Wound
            {
                uint32 percent = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster);
                if (!target->HealthBelowPct(percent))
                {
                    target->RemoveAurasDueToSpell(GetSpellInfo()->Id);
                    return;
                }
                break;
            }
        }
    }

    uint32 absorb = 0;
    uint32 resist = 0;

    // ignore non positive values (can be result apply spellmods to aura damage
    uint32 damage = std::max(GetAmount(), 0.f);
    uint32 damageBeforeHit = 0;

    float crit_chance = 0.0f;
    bool crit = caster->isSpellCrit(target, m_spellInfo, m_spellInfo->GetSchoolMask(), BASE_ATTACK, crit_chance);

    if (GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
    {
        switch (GetId())
        {
            case 124255: // Stagger
                damage = GetAmount();
                break;
            default:
            {
                if (!m_spellInfo->HasAttribute(SPELL_ATTR10_STACK_DAMAGE_OR_HEAL))
                {
                    std::vector<uint32> ExcludeAuraList;

                    switch (m_spellInfo->Id)
                    {
                        case 155722: // Rake
                        case 1079:   // Rip
                        case 106830: // Thrash
                        case 155625: // Moonfire (Cat form)
                        case 210705: // Ashamane's Rip
                        {
                            ExcludeAuraList.push_back(5217); // Tiger's Fury
                            break;
                        }
                    }

                    damage = std::max(GetBaseSendAmount(), 0.f);
                    damage = caster->SpellDamageBonusDone(target, m_spellInfo, damage, DOT, ExcludeAuraList, effIndex);
                    float damage_mod = 1.0f;
                    float damage_add = 0.f;
                    caster->SpellDotPctDone(target, m_spellInfo, GetEffIndex(), damage_mod, damage_add, false);
                    damage *= damage_mod;
                    damage *= m_amount_mod;
                    damage += m_amount_add;
                    damage += damage_add;
                    const_cast<AuraEffect*>(this)->m_amount = damage;
                }

                if (!(GetSpellInfo()->HasAttribute(SPELL_ATTR9_UNK28)))
                    damageBeforeHit = damage;
                break;
            }
        }

        if (caster->getClass() == CLASS_ROGUE && ((GetSpellInfo()->GetAllEffectsMechanicMask() & (1 << MECHANIC_BLEED)) || (GetSpellInfo()->GetDispelMask() & (1 << DISPEL_POISON)) || GetSpellInfo()->Id == 192759))
            if (AuraEffect* aurEff0 = caster->GetAuraEffect(214569, EFFECT_1)) // Zodyck Family Training Shackles
                if (target->GetHealthPct() < aurEff0->GetAmount())
                    if (AuraEffect* aurEff1 = caster->GetAuraEffect(214569, EFFECT_0)) // Zodyck Family Training Shackles
                        damage += CalculatePct(damage, aurEff1->GetAmount());

        switch (GetId())
        {
            case 199845: // Psyflay (Honor Talent)
                damage = target->CountPctFromMaxHealth(GetAmount());
                break;
            case 164812: // Moonfire
            case 164815: // Sunfire
            case 202347: // Stellar Flare
            {
                if (AuraEffect const* aurEff = target->GetAuraEffect(197637, EFFECT_0, caster->GetGUID())) // Stellar Empowerment
                    AddPct(damage, aurEff->GetAmount());
                break;
            }
            case 980: // Agony
                if (Aura* agony = GetBase())
                    damage = agony->CalcAgonyTickDamage(damage);
                break;
            default:
                break;
        }

        if (crit)
            damage *= caster->SpellCriticalDamageBonus(m_spellInfo, target);

        damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), damage);
    }
    else if (GetSpellInfo()->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA)
    {
        damage = uint32(target->CountPctFromMaxHealth(damage, caster));
        if (crit)
            damage *= caster->SpellCriticalDamageBonus(m_spellInfo, target);
    }
    else
        damage = uint32(target->CountPctFromMaxHealth(static_cast<int32>(damage)));

    if (!(GetSpellInfo()->HasAttribute(SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS)) && GetAuraType() != SPELL_AURA_PERIODIC_DAMAGE)
    {
        if (Player* spellModOwner = caster->GetSpellModOwner())
            damage += CalculatePct(damage, spellModOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY) + spellModOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));
    }

    if (caster->getClass() == CLASS_WARLOCK && m_spellInfo->ClassOptions.SpellClassMask.HasFlag(1026, 256)) // Agony, Corruption, Unstable Affliction
    {
        if (caster->HasAura(234876))
        {
            SpellInfo const* deathsEmbrace = sSpellMgr->GetSpellInfo(234876);

            if (target->GetHealthPct() <= deathsEmbrace->Effects[EFFECT_1]->BasePoints)
            {
                int32 bp = CalculatePct((100 - target->GetHealthPct()), deathsEmbrace->Effects[EFFECT_0]->BasePoints);
                AddPct(damage, bp);
            }
        }
    }

    float dmg = damage;

    if (lastTick && !m_spellInfo->HasAttribute(SPELL_ATTR10_STACK_DAMAGE_OR_HEAL))
    {
        float pct = std::max(m_periodicTimer, 100);
        pct /= m_period / 100.f;

        dmg = CalculatePct(dmg, pct);
    }

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), dmg, target);

    if (!(GetSpellInfo()->HasAttribute(SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS)))
        caster->ApplyResilience(target, &dmg);

    if (target->getClass() == CLASS_MONK)
        dmg = target->CalcStaggerDamage(dmg, m_spellInfo->GetSchoolMask(), m_spellInfo);

    caster->CalcAbsorbResist(target, GetSpellInfo()->GetSchoolMask(), DOT, dmg, &absorb, &resist, GetSpellInfo());

    damage = caster->InterceptionOfDamage(target, dmg, m_spellInfo->GetSchoolMask());

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: Caster %u attacked %u dmg %u damage %i GetId %u absorb %u", GetCasterGUID().GetGUIDLow(), target->GetGUIDLow(), dmg, damage, GetId(), absorb);

    caster->DealDamageMods(target, damage, &absorb, GetSpellInfo(), GetBase()->GetTriggeredCastFlags() & TRIGGERED_CASTED_BY_AREATRIGGER);

    DamageInfo dmgInfoProc = DamageInfo(caster, target, damage, GetSpellInfo(), SpellSchoolMask(GetSpellInfo()->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, damageBeforeHit);
    dmgInfoProc.AbsorbDamage(absorb);
    dmgInfoProc.ResistDamage(resist);
    CleanDamage cleanDamage = CleanDamage(0, absorb, BASE_ATTACK, MELEE_HIT_NORMAL);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx = (crit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT) | PROC_EX_INTERNAL_DOT;
    damage = (damage <= absorb+resist) ? 0 : (damage-absorb-resist);
    if (damage)
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;

    if (absorb && !damage)
        procEx |= PROC_EX_ABSORB;

    if (damage > 0)
    {
        if (GetSpellInfo()->Effects[m_effIndex]->IsTargetingArea() || GetSpellInfo()->Effects[m_effIndex]->Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA || GetBase()->HasAuraAttribute(AURA_ATTR_FROM_AREATRIGGER))
        {
            resist += damage;
            damage = int32(float(damage) * target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE, GetSpellInfo()->GetSchoolMask()));

            if (GetCaster() && GetCaster()->IsCreature())
                damage = int32(float(damage) * target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE, GetSpellInfo()->GetSchoolMask()));

            if (Player* player = target->ToPlayer())
                damage -= CalculatePct(damage, player->GetFloatValue(PLAYER_FIELD_AVOIDANCE));

            resist -= damage;
        }
    }

    int32 overkill = damage - target->GetHealth(GetCaster());
    if (overkill < 0)
        overkill = 0;

    SpellPeriodicAuraLogInfo pInfo(this, damage, overkill, absorb, resist, 0.0f, crit);

    caster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, &dmgInfoProc, BASE_ATTACK, GetSpellInfo());

    target->SendPeriodicAuraLog(&pInfo);

    caster->DealDamage(target, damage, &cleanDamage, DOT, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), true);
}

void AuraEffect::HandlePeriodicWeaponPercentDamageAurasTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const
{
    if (!caster || !target->isAlive())
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    uint32 absorb = 0;
    uint32 resist = 0;
    Item* castItem = nullptr;

    if (ObjectGuid itemGUID = GetBase()->GetCastItemGUID())
        if (Player* playerCaster = caster->ToPlayer())
            castItem = playerCaster->GetItemByGuid(itemGUID);

    // ignore non positive values (can be result apply spellmods to aura damage
    int32 weaponDamage = caster->CalculateDamage(BASE_ATTACK, false, m_spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL, target);
    float weaponDamagePercentMod = m_spellInfo->GetEffect(GetEffIndex(), m_diffMode)->CalcValue(caster, nullptr, target, castItem, false, nullptr, GetBase()->GetComboPoints()) / 100.0f;
    auto damage = uint32(weaponDamage * weaponDamagePercentMod);
    uint32 damageBeforeHit = 0;

    float crit_chance = 0.0f;
    bool crit = caster->isSpellCrit(target, m_spellInfo, m_spellInfo->GetSchoolMask(), BASE_ATTACK, crit_chance);
    if (crit)
        damage *= m_crit_mod;

    damage = caster->MeleeDamageBonusDone(target, damage, BASE_ATTACK, GetSpellInfo(), effIndex);

    if (!m_spellInfo->HasAttribute(SPELL_ATTR10_STACK_DAMAGE_OR_HEAL))
    {
        damage *= m_amount_mod;
        damage += m_amount_add;
    }

    damageBeforeHit = damage;
    damage = target->MeleeDamageBonusTaken(caster, damage, BASE_ATTACK);

    if (!(GetSpellInfo()->HasAttribute(SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS)))
        if (Player* spellModOwner = caster->GetSpellModOwner())
            damage += CalculatePct(damage, spellModOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY) + spellModOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));

    float dmg = damage;

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), dmg, target);

    if (!(GetSpellInfo()->HasAttribute(SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS)))
        caster->ApplyResilience(target, &dmg);

    if (target->getClass() == CLASS_MONK)
        dmg = target->CalcStaggerDamage(dmg, m_spellInfo->GetSchoolMask(), m_spellInfo);

    caster->CalcAbsorbResist(target, GetSpellInfo()->GetSchoolMask(), DOT, dmg, &absorb, &resist, GetSpellInfo());

    damage = caster->InterceptionOfDamage(target, dmg, m_spellInfo->GetSchoolMask());

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: Caster %u attacked %u damage %u GetId %u absorb %u", GetCasterGUID().GetGUIDLow(), target->GetGUIDLow(), damage, GetId(), absorb);

    caster->DealDamageMods(target, damage, &absorb, GetSpellInfo(), GetBase()->GetTriggeredCastFlags() & TRIGGERED_CASTED_BY_AREATRIGGER);

    DamageInfo dmgInfoProc = DamageInfo(caster, target, damage, GetSpellInfo(), SpellSchoolMask(GetSpellInfo()->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, damageBeforeHit);
    dmgInfoProc.AbsorbDamage(absorb);
    dmgInfoProc.ResistDamage(resist);
    CleanDamage cleanDamage = CleanDamage(0, absorb, BASE_ATTACK, MELEE_HIT_NORMAL);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx = (crit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT) | PROC_EX_INTERNAL_DOT;
    damage = (damage <= absorb+resist) ? 0 : (damage-absorb-resist);
    if (damage)
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;

    if (absorb && !damage)
        procEx |= PROC_EX_ABSORB;

    int32 overkill = damage - target->GetHealth(GetCaster());
    if (overkill < 0)
        overkill = 0;

    SpellPeriodicAuraLogInfo pInfo(this, damage, overkill, absorb, resist, 0.0f, crit);

    caster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, &dmgInfoProc, BASE_ATTACK, GetSpellInfo());

    target->SendPeriodicAuraLog(&pInfo);

    caster->DealDamage(target, damage, &cleanDamage, DOT, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), true);
}

void AuraEffect::HandlePeriodicHealthLeechAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const
{
    if (!caster || !target->isAlive())
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    if (GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(target, GetSpellInfo(), false, 1 << GetEffIndex()) != SPELL_MISS_NONE)
        return;

    uint32 absorb = 0;
    uint32 resist = 0;

    float damage = std::max(GetBaseSendAmount(), 0.f);
    std::vector<uint32> ExcludeAuraList;
    damage = caster->SpellDamageBonusDone(target, m_spellInfo, damage, DOT, ExcludeAuraList, effIndex);
    float damage_mod = 1.0f; 
    float damage_add = 0.f;
    caster->SpellDotPctDone(target, m_spellInfo, GetEffIndex(), damage_mod, damage_add, false);
    damage *= damage_mod;
    damage *= m_amount_mod;
    damage += m_amount_add;
    damage += damage_add;
    uint32 damageBeforeHit = damage;

    float crit_chance = 0.0f;
    bool crit = caster->isSpellCrit(target, m_spellInfo, m_spellInfo->GetSchoolMask(), BASE_ATTACK, crit_chance);
    if (crit)
        damage *= caster->SpellCriticalDamageBonus(m_spellInfo, target);

    damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), damage);

    if (caster->getClass() == CLASS_WARLOCK && m_spellInfo->ClassOptions.SpellClassMask.HasFlag(8388608, 0, 0, 0)) // Drain Soul
    {
        if (caster->HasAura(234876))
        {
            SpellInfo const* deathsEmbrace = sSpellMgr->GetSpellInfo(234876);

            if (target->GetHealthPct() <= deathsEmbrace->Effects[EFFECT_1]->BasePoints)
            {
                int32 bp = CalculatePct((100 - target->GetHealthPct()), deathsEmbrace->Effects[EFFECT_0]->BasePoints);
                AddPct(damage, bp);
            }
        }
    }

    float dmg = damage;

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), dmg, target);

    caster->ApplyResilience(target, &dmg);

    if (target->getClass() == CLASS_MONK)
        dmg = target->CalcStaggerDamage(dmg, m_spellInfo->GetSchoolMask(), m_spellInfo);

    damage = caster->InterceptionOfDamage(target, dmg, m_spellInfo->GetSchoolMask());
    caster->CalcAbsorbResist(target, GetSpellInfo()->GetSchoolMask(), DOT, damage, &absorb, &resist, m_spellInfo);

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: %s health leech of %s for %u dmg inflicted by %u abs is %u",
        GetCasterGUID().ToString(), target->GetGUID().ToString(), damage, GetId(), absorb);

    uint32 tempDmg = damage;
    caster->DealDamageMods(target, tempDmg, &absorb, GetSpellInfo(), GetBase()->GetTriggeredCastFlags() & TRIGGERED_CASTED_BY_AREATRIGGER);
    damage = tempDmg;

    DamageInfo dmgInfoProc = DamageInfo(caster, target, damage, GetSpellInfo(), SpellSchoolMask(GetSpellInfo()->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, damageBeforeHit);
    dmgInfoProc.AbsorbDamage(absorb);
    dmgInfoProc.ResistDamage(resist);
    CleanDamage cleanDamage = CleanDamage(0, absorb, BASE_ATTACK, MELEE_HIT_NORMAL);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx = (crit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT) | PROC_EX_INTERNAL_DOT;
    damage = (damage <= absorb+resist) ? 0 : (damage-absorb-resist);

    if (damage)
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;

    if (caster->isAlive())
        caster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, &dmgInfoProc, BASE_ATTACK, GetSpellInfo());

    caster->SendSpellNonMeleeDamageLog(target, GetId(), damage, GetSpellInfo()->GetSchoolMask(), absorb, resist, false, 0, m_spellInfo->GetSpellXSpellVisualId(caster, target), crit);
    caster->DealDamage(target, damage, &cleanDamage, DOT, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), false);

    if (caster->isAlive())
    {
        float gainMultiplier = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->CalcValueMultiplier(caster);

        auto heal = uint32(damage * gainMultiplier);
        heal = uint32(caster->SpellHealingBonusTaken(caster, GetSpellInfo(), heal, DOT, effIndex, GetBase()->GetStackAmount()));

        int32 gain = caster->HealBySpell(caster, GetSpellInfo(), heal);
        caster->getHostileRefManager().threatAssist(caster, gain * 0.5f, GetSpellInfo());
    }
}

void AuraEffect::HandlePeriodicHealthFunnelAuraTick(Unit* target, Unit* caster, SpellEffIndex /*effIndex*/) const
{
    if (!caster || !caster->isAlive() || !target->isAlive())
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    float damage = std::max(GetAmount(), 0.f);
    // do not kill health donator
    if (static_cast<int32>(caster->GetHealth()) < damage)
        damage = caster->GetHealth() - 1;
    if (!damage)
        return;

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), damage, target);

    caster->ModifyHealth(-static_cast<int32>(damage), nullptr, GetId());
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: donator %u target %u damage %u.", caster->GetEntry(), target->GetEntry(), damage);

    float gainMultiplier = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->CalcValueMultiplier(caster);

    damage = int32(damage * gainMultiplier);

    caster->HealBySpell(target, GetSpellInfo(), damage);
}

void AuraEffect::HandlePeriodicHealAurasTick(Unit* target, Unit* caster, SpellEffIndex effIndex, bool lastTick) const
{
    if (!caster || !target->isAlive())
        return;

    if (lastTick && !GetBase()->HasAuraAttribute(AURA_ATTR_REAPPLIED_AURA) && 
        (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION_RECOVERY) || !m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME)))
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // heal for caster damage (must be alive)
    if (target != caster && GetSpellInfo()->HasAttribute(SPELL_ATTR2_HEALTH_FUNNEL) && !caster->isAlive())
        return;

    // don't regen when permanent aura target has full power
    if (GetBase()->IsPermanent() && target->IsFullHealth())
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    float damage = std::max(m_amount, 0.f);

    float crit_chance = 0.0f;
    bool crit = caster->isSpellCrit(target, m_spellInfo, m_spellInfo->GetSchoolMask(), BASE_ATTACK, crit_chance);

    if (GetAuraType() == SPELL_AURA_OBS_MOD_HEALTH)
    {
        // Taken mods
        float TakenTotalMod = 1.0f;

        // Tenacity increase healing % taken
       if (AuraEffect const* Tenacity = target->GetAuraEffect(58549, 0))
            AddPct(TakenTotalMod, Tenacity->GetAmount());

        // Healing taken percent
       float ModHealingPct = target->GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALING_PCT);
       if (ModHealingPct != 1.0f)
           TakenTotalMod *= ModHealingPct;

       if (!(m_spellInfo->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS)) && !(m_spellInfo->HasAttribute(SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS)))
       {
           if (Player* spellModOwner = caster->GetSpellModOwner())
               damage += CalculatePct(damage, spellModOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY) + spellModOwner->GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));
       }

        TakenTotalMod = std::max(TakenTotalMod, 0.0f);

        damage = target->CountPctFromMaxHealth(damage, caster);
        damage = damage * TakenTotalMod;
        
        switch (GetSpellInfo()->Id)
        {
            case 136:
            {
                if(target->isPet())
                {
                    for (auto itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
                    {
                        Unit* unit = ObjectAccessor::GetUnit(*caster, *itr);
                        if (!unit)
                            continue;
                        if (Creature* creature = unit->ToCreature())
                        {
                            switch (creature->GetEntry())
                            {
                                case 100324:
                                case 106548:
                                case 106549:
                                case 106550:
                                case 106551:
                                {
                                    if (creature->isAlive())
                                    {
                                        uint32 _heal = damage;
                                        uint32 _absorb = 0;
                                        caster->CalcHealAbsorb(unit, GetSpellInfo(), _heal, _absorb);
                                        int32 _gain = caster->DealHeal(unit, _heal, GetSpellInfo());
                                        SpellPeriodicAuraLogInfo _pInfo(this, _heal, _heal - _gain, _absorb, 0, 0.0f, crit);
                                        unit->SendPeriodicAuraLog(&_pInfo);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
        if (!m_spellInfo->HasAttribute(SPELL_ATTR10_STACK_DAMAGE_OR_HEAL))
        {
            damage = std::max(GetBaseSendAmount(), 0.f);
            damage = caster->SpellHealingBonusDone(target, GetSpellInfo(), damage, DOT, effIndex);
            float damage_mod = 1.0f; 
            caster->SpellDotPctDone(target, m_spellInfo, GetEffIndex(), damage_mod, damage, false);
            damage *= damage_mod;
            damage *= m_amount_mod;
            damage += m_amount_add;
            const_cast<AuraEffect*>(this)->m_amount = damage;
        }

        // Wild Growth = amount + (6 - 2*doneTicks) * ticks* amount / 100
        if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_DRUID)
        {
            if (m_spellInfo->GetMisc(m_diffMode)->MiscData.IconFileDataID == 236153)
            {
                float totalTicks = GetTotalTicks();
                uint32 tickNumber = GetTickNumber();
                int32 pct = (100 + totalTicks - 1) - (2 * (tickNumber - 1));
                damage = CalculatePct(damage, pct);
            }

            if (Aura* aura = caster->GetAura(77495)) // Mastery: Harmony
            {
                float modDif = 0.f;
                uint32 modCount = 0;
                uint32 modMaxCount = 9;
                if (AuraEffect* eff = aura->GetEffect(EFFECT_0))
                    modDif = eff->GetAmount();
                if (Unit::AuraEffectList const* mPeriodic = target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL))
                    for (Unit::AuraEffectList::const_iterator i = mPeriodic->begin(); i != mPeriodic->end(); ++i)
                        if ((*i)->GetCasterGUID() == caster->GetGUID())
                            modCount++;

                if (modCount >= modMaxCount)
                    modCount = modMaxCount;
                if (modCount && modDif)
                    damage += CalculatePct(damage, modDif * modCount);
            }
        }

        switch (m_spellInfo->Id)
        {
            case 194025: // Thrive in the Shadows
            {
                if (AuraEffect* aurEff = caster->GetAuraEffect(194024, EFFECT_0))
                    damage = CalculatePct(target->GetMaxHealth(), aurEff->GetAmount()) / 7;
                break;
            }
            case 196356: // Trust in the Light
            {
                damage = CalculatePct(target->GetMaxHealth(), GetAmount()) / 3;
                break;
            }
            case 119611: // Renewing Mist
            {
                if (AuraEffect* aurEff = caster->GetAuraEffect(202428, EFFECT_0)) // Counteract Magic (Honor Talent)
                    if (target->HasAuraWithSchoolMask(SPELL_AURA_PERIODIC_DAMAGE, SPELL_SCHOOL_MASK_MAGIC))
                        AddPct(damage, aurEff->GetAmount());
                break;
            }
            case 194316: // Cauterizing Blink
            {
                if (GetTotalTicks())
                    if (AuraEffect* aurEff = caster->GetAuraEffect(194318, EFFECT_0)) // Cauterizing Blink
                        damage = CalculatePct(caster->GetMaxHealth(), aurEff->GetAmount());
                break;
            }
            case 774: // Rejuvenation
            case 155777: // Rejuvenation
            case 8936: // Regrowth
            case 48438: // Wild Growth
            {
                if (GetTickNumber() > 1)
                    if (AuraEffect* aurEff = caster->GetAuraEffect(238122, EFFECT_0)) // Deep Rooted
                        if (target->HealthBelowPct(aurEff->GetAmount()))
                            GetBase()->RefreshTimers();
                break;
            }
            case 28880:  // Warrior     - Gift of the Naaru
            case 59542:  // Paladin     - Gift of the Naaru
            case 59543:  // Hunter      - Gift of the Naaru
            case 59544:  // Priest      - Gift of the Naaru
            case 59545:  // Deathknight - Gift of the Naaru
            case 59547:  // Shaman      - Gift of the Naaru
            case 59548:  // Mage        - Gift of the Naaru
            case 121093: // Monk        - Gift of the Naaru
            {
                if(GetTotalTicks())
                    damage = CalculatePct(caster->GetMaxHealth(), m_spellInfo->Effects[1]->BasePoints) / GetTotalTicks();
                break;
            }
            // Spirit Mend
            case 90361:
                damage += int32((caster->GetTotalAttackPowerValue(BASE_ATTACK) * 2) * 1.0504f) / 5;
                break;
            default:
                break;
        }

        damage = target->SpellHealingBonusTaken(caster, GetSpellInfo(), damage, DOT, effIndex, GetBase()->GetStackAmount());
    }

    if (lastTick && !m_spellInfo->HasAttribute(SPELL_ATTR10_STACK_DAMAGE_OR_HEAL))
    {
        float pct = std::max(m_periodicTimer, 100);
        pct /= m_period / 100.f;

        damage = CalculatePct(damage, pct);
    }

    if (crit)
        damage = caster->SpellCriticalHealingBonus(m_spellInfo, damage);

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: %s heal of %s for %u health inflicted by %u",
        GetCasterGUID().ToString(), target->GetGUID().ToString(), damage, GetId());

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), damage, target);

    uint32 absorb = 0;
    auto heal = uint32(damage);
    if (!heal)
        return;

    caster->CalcHealAbsorb(target, GetSpellInfo(), heal, absorb);
    int32 gain = caster->DealHeal(target, heal, GetSpellInfo());

    DamageInfo dmgInfoProc = DamageInfo(caster, target, damage, GetSpellInfo(), GetSpellInfo() ? SpellSchoolMask(GetSpellInfo()->GetMisc(m_diffMode)->MiscData.SchoolMask) : SPELL_SCHOOL_MASK_NORMAL, SPELL_DIRECT_DAMAGE, damage);
    dmgInfoProc.AbsorbDamage(absorb);
    dmgInfoProc.SetOverHeal(heal - gain);

    SpellPeriodicAuraLogInfo pInfo(this, heal, heal - gain, absorb, 0, 0.0f, crit);
    target->SendPeriodicAuraLog(&pInfo);

    target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f, GetSpellInfo());

    bool haveCastItem = GetBase()->GetCastItemGUID();

    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx = (crit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT) | PROC_EX_INTERNAL_HOT;
    // ignore item heals
    if (!haveCastItem)
        caster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, &dmgInfoProc, BASE_ATTACK, GetSpellInfo());
}

void AuraEffect::HandlePeriodicManaLeechAuraTick(Unit* target, Unit* caster, SpellEffIndex /*effIndex*/) const
{
    auto powerType = Powers(GetMiscValue());

    if (!caster || !caster->isAlive() || !target->isAlive() || target->getPowerType() != powerType)
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    if (GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(target, GetSpellInfo(), false, 1 << GetEffIndex()) != SPELL_MISS_NONE)
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    float drainAmount = std::max(m_amount, 0.f);

    if (!GetSpellInfo()->NoPower())
    {
        SpellPowerData powerData;
        if (!GetSpellInfo()->GetSpellPowerByCasterPower(GetCaster(), powerData))
            if (SpellPowerEntry const* power = GetSpellInfo()->GetPowerInfo(0))
                powerData.push_back(power);

        // Special case: draining x% of mana (up to a maximum of 2*x% of the caster's maximum mana)
        // It's mana percent cost spells, m_amount is percent drain from target
        for (SpellPowerEntry const* power : powerData)
        {
            if (power->PowerCostPct)
            {
                // max value
                int32 maxmana = CalculatePct(caster->GetMaxPower(powerType), drainAmount * 2.0f);
                ApplyPct(drainAmount, target->GetMaxPower(powerType));
                if (drainAmount > maxmana)
                    drainAmount = maxmana;
            }
        }
    }

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), drainAmount, target);

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: %s power leech of %s for %u dmg inflicted by %u",
        GetCasterGUID().ToString(), target->GetGUID().ToString(), drainAmount, GetId());

    int32 drainedAmount = -target->ModifyPower(powerType, -drainAmount, true);

    float gainMultiplier = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->CalcValueMultiplier(caster);

    SpellPeriodicAuraLogInfo pInfo(this, drainedAmount, 0, 0, 0, gainMultiplier, false);
    target->SendPeriodicAuraLog(&pInfo);

    auto gainAmount = int32(drainedAmount * gainMultiplier);
    int32 gainedAmount = 0;
    if (gainAmount)
    {
        gainedAmount = caster->ModifyPower(powerType, gainAmount);
        target->AddThreat(caster, float(gainedAmount) * 0.5f, GetSpellInfo()->GetSchoolMask(), GetSpellInfo());
    }

    // Drain Mana
    if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_WARLOCK && m_spellInfo->ClassOptions.SpellClassMask[0] & 0x00000010)
    {
        int32 manaFeedVal = 0;
        if (AuraEffect const* aurEff = GetBase()->GetEffect(1))
            manaFeedVal = aurEff->GetAmount();
        // Mana Feed - Drain Mana
        if (manaFeedVal > 0)
        {
            float feedAmount = CalculatePct(gainedAmount, manaFeedVal);
            caster->CastCustomSpell(caster, 32554, &feedAmount, nullptr, nullptr, true, nullptr, this);
        }
    }
}

void AuraEffect::HandleObsModPowerAuraTick(Unit* target, Unit* caster, SpellEffIndex /*effIndex*/) const
{
    Powers powerType;
    if (GetMiscValue() == POWER_ALL)
        powerType = target->getPowerType();
    else
        powerType = Powers(GetMiscValue());

    if (!target->isAlive() || !target->GetMaxPower(powerType))
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // don't regen when permanent aura target has full power
    if (GetBase()->IsPermanent() && target->GetPower(powerType) == target->GetMaxPower(powerType))
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    float amount = std::max(m_amount, 0.f) * target->GetMaxPower(powerType) / 100.f;

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), amount, target);
    
    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: %s energize of %s for %u dmg inflicted by %u",
        GetCasterGUID().ToString(), target->GetGUID().ToString(), amount, GetId());

    SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0, 0.0f, false);
    target->SendPeriodicAuraLog(&pInfo);

    int32 gain = target->ModifyPower(powerType, amount);

    if (caster)
        target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f, GetSpellInfo());

    switch (GetId())
    {
        case 12051: // Evocation
            if (caster)
                if (AuraEffect* aurEff = caster->GetAuraEffect(210847, EFFECT_0)) // Aegwynn's Ascendance
                    aurEff->SetAmount(aurEff->GetAmount() + gain);
            break;
        default:
            break;
    }
}

void AuraEffect::HandlePeriodicEnergizeAuraTick(Unit* target, Unit* caster, SpellEffIndex /*effIndex*/) const
{
    auto powerType = Powers(GetMiscValue());
    if (!target->isAlive() || !target->GetMaxPower(powerType))
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // don't regen when permanent aura target has full power
    if (GetBase()->IsPermanent() && target->GetPower(powerType) == target->GetMaxPower(powerType))
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    float amount = powerType == POWER_ALTERNATE ? m_amount : std::max(m_amount, 0.f);

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), amount, target);

    SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0, 0.0f, false);
    target->SendPeriodicAuraLog(&pInfo);

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "PeriodicTick: %s energize of %s for %u dmg inflicted by %u",
        GetCasterGUID().ToString(), target->GetGUID().ToString(), amount, GetId());

    int32 gain = target->ModifyPower(powerType, amount, true);

    if (caster)
        target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f, GetSpellInfo());

    if (powerType == POWER_RUNES)
    {
        if(Player* _player = target->ToPlayer())
        {
            float runesRestor = amount;
            // First regene rune with full CD
            for (int i = 0; i < _player->GetMaxPower(POWER_RUNES) ; i++)
            {
                if (_player->GetRuneCooldown(i) == _player->GetRuneBaseCooldown() && runesRestor)
                {
                    runesRestor--;
                    _player->SetRuneCooldown(i, 0);
                    _player->AddRunePower(i);
                }
            }
            for (int i = 0; i < _player->GetMaxPower(POWER_RUNES) ; i++)
            {
                if (_player->GetRuneCooldown(i) && runesRestor)
                {
                    runesRestor--;
                    _player->SetRuneCooldown(i, 0);
                    _player->AddRunePower(i);
                }
            }
        }
    }
}

void AuraEffect::HandlePeriodicPowerBurnAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const
{
    auto powerType = Powers(GetMiscValue());

    if (!caster || !target->isAlive() || target->getPowerType() != powerType)
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    // ignore negative values (can be result apply spellmods to aura damage
    float damage = std::max(m_amount, 0.f);

    GetBase()->CallScriptEffectChangeTickDamageHandlers(const_cast<AuraEffect const*>(this), damage, target);

    auto gain = uint32(-target->ModifyPower(powerType, -damage));

    float dmgMultiplier = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->CalcValueMultiplier(caster);

    SpellInfo const* spellInfo = GetSpellInfo();
    SpellNonMeleeDamage damageInfo(caster, target, spellInfo->Id, spellInfo->GetSpellXSpellVisualId(caster, target), spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask);
    // no SpellDamageBonus for burn mana
    caster->CalculateSpellDamageTaken(&damageInfo, int32(gain * dmgMultiplier), spellInfo, (1 << effIndex));

    caster->DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb, spellInfo, GetBase()->GetTriggeredCastFlags() & TRIGGERED_CASTED_BY_AREATRIGGER);

    DamageInfo dmgInfoProc = DamageInfo(damageInfo, spellInfo);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx       = createProcExtendMask(&damageInfo, SPELL_MISS_NONE) | PROC_EX_INTERNAL_DOT;
    if (damageInfo.damage)
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;

    caster->ProcDamageAndSpell(damageInfo.target, procAttacker, procVictim, procEx, &dmgInfoProc, BASE_ATTACK, spellInfo);

    caster->SendSpellNonMeleeDamageLog(&damageInfo);
    caster->DealSpellDamage(&damageInfo, true);
}

void AuraEffect::HandleProcTriggerSpellAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex /*effIndex*/)
{
    Unit* triggerCaster = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();

    uint32 triggerSpellId = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->TriggerSpell;
    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandleProcTriggerSpellAuraProc: Triggering spell %u from aura %u proc", triggeredSpellInfo->Id, GetId());
        triggerCaster->CastSpell(triggerTarget, triggeredSpellInfo, true, nullptr, this);
    }
    else
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS,"AuraEffect::HandleProcTriggerSpellAuraProc: Could not trigger spell %u from aura %u proc, because the spell does not have an entry in Spell.dbc.", triggerSpellId, GetId());
}

void AuraEffect::HandleProcTriggerSpellWithValueAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex /*effIndex*/)
{
    Unit* triggerCaster = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();

    uint32 triggerSpellId = GetSpellInfo()->GetEffect(m_effIndex, m_diffMode)->TriggerSpell;
    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
    {
        float basepoints0 = GetAmount();
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandleProcTriggerSpellWithValueAuraProc: Triggering spell %u with value %d from aura %u proc", triggeredSpellInfo->Id, basepoints0, GetId());
        triggerCaster->CastCustomSpell(triggerTarget, triggerSpellId, &basepoints0, nullptr, nullptr, true, nullptr, this);
    }
    else
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS,"AuraEffect::HandleProcTriggerSpellWithValueAuraProc: Could not trigger spell %u from aura %u proc, because the spell does not have an entry in Spell.dbc.", triggerSpellId, GetId());
}

void AuraEffect::HandleProcTriggerDamageAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex effIndex)
{
    Unit* target = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();
    auto const& spellInfo = GetSpellInfo();

    SpellNonMeleeDamage damageInfo(target, triggerTarget, GetId(), spellInfo->GetSpellXSpellVisualId(target), spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask);
    std::vector<uint32> ExcludeAuraList;
    uint32 damage = target->SpellDamageBonusDone(triggerTarget, spellInfo, GetAmount(), SPELL_DIRECT_DAMAGE, ExcludeAuraList, effIndex);
    damage = triggerTarget->SpellDamageBonusTaken(target, spellInfo, damage);
    target->CalculateSpellDamageTaken(&damageInfo, damage, spellInfo, (1 << effIndex));
    target->DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb, spellInfo, GetBase()->GetTriggeredCastFlags() & TRIGGERED_CASTED_BY_AREATRIGGER);
    target->SendSpellNonMeleeDamageLog(&damageInfo);
    target->DealSpellDamage(&damageInfo, true);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandleProcTriggerDamageAuraProc: Triggering %u spell damage from aura %u proc", damage, GetId());
}

void AuraEffect::HandleAuraForceWeather(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    if (apply)
        target->SendDirectMessage(WorldPackets::Misc::Weather(WeatherState(GetMiscValue()), 1.0f).Write());
    else
        target->GetMap()->SendZoneWeather(target->GetCurrentZoneID(), target);
}

void AuraEffect::HandleProgressBar(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target)
        return;

    if (!apply)
    {
        target->SetMaxPower(POWER_ALTERNATE, 0);
        target->SetPower(POWER_ALTERNATE, 0);
        return;
    }

    UnitPowerBarEntry const * entry = sUnitPowerBarStore.LookupEntry(GetMiscValue());
    if (!entry)
        return;

    target->SetMaxPower(POWER_ALTERNATE, entry->MaxPower);
    target->SetPower(POWER_ALTERNATE, entry->StartPower);
}

void AuraEffect::HandleAuraStrangulate(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target)
        return;

    switch (m_spellInfo->Id)
    {
        case 108194: //Asphyxiate
        case 221562: //Asphyxiate
            target->SetControlled(apply, UNIT_STATE_STUNNED);
            target->UpdateHeight(apply ? 1.0f : 0.0f);
            break;
        case 121448: //Encased in Resin
        case 208944: //Elisande: Time Stop
        case 249241: //Felhounds: Molten Touch
            target->SetControlled(apply, UNIT_STATE_STUNNED);
            break;
        case 258373: //Argus: Grasp of the Unmaker
            target->SetControlled(apply, UNIT_STATE_STUNNED);
            target->SetDisableGravity(apply);
            break;
        default:
            break;
    }
}

void AuraEffect::HandleOverrideActionbarSpells(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target->IsPlayer())
        return;

    if (!apply)
    {
        std::list<AuraType> auratypelist;
        auratypelist.push_back(SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
        auratypelist.push_back(SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS_2);
        Unit::AuraEffectList swaps;
        target->GetAuraEffectsByListType(&auratypelist, swaps);

        SpellInfo const* thisInfo = GetSpellInfo();
        if (!thisInfo)
            return;

        // update auras that replace same spells that unapplying aura do
        for (Unit::AuraEffectList::iterator itr = swaps.begin(); itr != swaps.end(); ++itr)
        {
            SpellInfo const* otherInfo = (*itr)->GetSpellInfo();
            if (!otherInfo)
                continue;

            if (thisInfo->ClassOptions.SpellClassSet == otherInfo->ClassOptions.SpellClassSet)
            {
                bool updated = false;
                for (int i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if (thisInfo->EffectMask < uint32(1 << i))
                        break;

                    for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
                    {
                        if (otherInfo->EffectMask < uint32(1 << j))
                            break;

                        if (thisInfo->Effects[i]->SpellClassMask & otherInfo->Effects[j]->SpellClassMask)
                        {
                            updated = true;
                            (*itr)->GetBase()->SetNeedClientUpdateForTargets();
                            break;
                        }
                    }
                    if (updated)
                        break;
                }
            }
        }
    }
}

void AuraEffect::HandleAuraModCategoryCooldown(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target || target->GetSession()->PlayerLoading())
        return;

    target->SendCategoryCooldownMods();
}

void AuraEffect::HandleAuraSeeWhileInvisible(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    target->UpdateObjectVisibility();
}

void AuraEffect::HandleAuraMastery(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    target->UpdateMasteryAuras();
}

void AuraEffect::HandleAuraModCharges(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    target->RecalculateSpellCategoryCharges(GetMiscValue());
}

void AuraEffect::HandleAuraModChargeRecoveryMod(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    if (apply)
    {
        target->RecalculateSpellCategoryRegenTime(GetMiscValue());
        target->SendSpellChargeData();
    }
}

void AuraEffect::HandleBattlegroundFlag(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();

    // when removing flag aura, handle flag drop
    if (!apply && target)
    {
        if (target->InBattleground())
        {
            if (Battleground* bg = target->GetBattleground())
                bg->EventPlayerDroppedFlag(target);
        }
        else
            sOutdoorPvPMgr->HandleDropFlag(target, GetSpellInfo()->Id);
    }
}

void AuraEffect::HandleCreateAreaTrigger(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    //Use custom summon at spell_norushen_residual_corruption id 145074
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    uint32 triggerEntry = GetMiscValue();
    if (!triggerEntry || !target || !GetCaster() || GetCaster()->GetMap() != target->GetMap())
        return;

    // when removing flag aura, handle flag drop
    if (apply)
    {
        Position pos;
        target->GetPosition(&pos);

        auto areaTrigger = new AreaTrigger;
        if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), triggerEntry, GetCaster(), GetSpellInfo(), pos, pos, nullptr, target->GetGUID()))
        {
            delete areaTrigger;
            return;
        }
        areaTrigger->m_aura = GetBase();

        GetBase()->SetSpellAreaTrigger(areaTrigger->GetGUID());
    }
    else if (!apply)
    {
        ObjectGuid areaTriggerGuid = GetBase()->GetSpellAreaTrigger();
        if (!areaTriggerGuid.IsEmpty())
            if (auto areaTrigger = ObjectAccessor::GetAreaTrigger(*GetCaster(), areaTriggerGuid))
            {
                areaTrigger->m_aura = nullptr;
                areaTrigger->SetDuration(0);
            }
    }
}

void AuraEffect::HandleAuraActivateScene(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target || !target->IsPlayer())
        return;

    Position pos;
    target->GetPosition(&pos);

    target->ToPlayer()->SendSpellScene(GetMiscValue(), m_spellInfo, apply, &pos);

    if (apply)
    {
        SpellScene const* spellScene = sSpellMgr->GetSpellScene(GetMiscValue());
        if (!spellScene)
            return;

        if (uint32 duration = spellScene->CustomDuration)
            GetBase()->SetDuration(duration);
    }
}

void AuraEffect::HandleAuraeEablePowerType(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    PowerDisplayEntry const* powerDisplay = sPowerDisplayStore.LookupEntry(GetMiscValue());
    if (!powerDisplay)
        return;

    Unit* target = aurApp->GetTarget();
    if (target->GetPowerIndex(powerDisplay->ActualType) == MAX_POWERS)
        return;

    if (apply)
    {
        if (!target->HasAuraTypeWithMiscvalue(GetAuraType(), GetMiscValue()))
            target->RemoveAurasByType(GetAuraType(), ObjectGuid::Empty, GetBase());

        target->SetUInt32Value(UNIT_FIELD_OVERRIDE_DISPLAY_POWER_ID, powerDisplay->ID);
    }
    else
        target->SetUInt32Value(UNIT_FIELD_OVERRIDE_DISPLAY_POWER_ID, 0);
}

void AuraEffect::HandleShowConfirmationPrompt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* player = aurApp->GetTarget()->ToPlayer();
    if (!player)
        return;

    if (apply)
        player->AddTemporarySpell(GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->TriggerSpell);
    else
        player->RemoveTemporarySpell(GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->TriggerSpell);
}

void AuraEffect::HandleSummonController(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    uint32 summonSpellID = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->TriggerSpell;

    if (apply)
        target->CastSpell(target, summonSpellID, true);
    else
    {
        target->RemoveAllMinionsByFilter(summonSpellID, 1); // remove by spellID
        target->RemoveGameObject(summonSpellID, true);
    }
}

void AuraEffect::HandleModNextSpell(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target->IsPlayer())
        return;

    Player* player = target->ToPlayer();
    if (!player)
        return;

    uint32 triggeredSpellId = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->TriggerSpell;
    if (apply)
        player->AddTemporarySpell(triggeredSpellId);
    else
        player->RemoveTemporarySpell(triggeredSpellId);
}

void AuraEffect::HandleModLifeStealPct(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    target->UpdateLifesteal();
}

void AuraEffect::HandleModVersalityPct(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    target->UpdateVersality();
}

void AuraEffect::HandleDisableMovementForce(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* player = nullptr;
    if (Unit* target = aurApp->GetTarget())
        if (target->ToPlayer())
            player = target->ToPlayer();

    if (!player)
        return;

    if (ObjectGuid forceGuid = player->GetForceGUID())
        if (AreaTrigger const* at = ObjectAccessor::GetAreaTrigger(*player, forceGuid))
            player->SendMovementForce(at);
}

void AuraEffect::HandleAnimReplacementSet(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    uint16 animid = GetSpellInfo()->GetEffect(GetEffIndex(), m_diffMode)->MiscValue;

    if (animid != 61)
        target->SetAnimKitId(animid);
}

void AuraEffect::HandleAllowUsingGameobjectsWhileMounted(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (!aurApp->GetTarget()->IsPlayer())
        return;

    if (apply)
        aurApp->GetTarget()->SetFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_CAN_USE_OBJECTS_MOUNTED);
    else if (!aurApp->GetTarget()->HasAuraType(SPELL_AURA_ALLOW_USING_GAMEOBJECTS_WHILE_MOUNTED))
        aurApp->GetTarget()->RemoveFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_CAN_USE_OBJECTS_MOUNTED);
}

void AuraEffect::HandleOverridePetSpecs(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    Unit* owner = target->GetOwner();
    if (!owner || owner->getClass() != CLASS_HUNTER)
        return;

    if (Pet* pet = target->ToPet())
        pet->CheckSpecialization();
}

void AuraEffect::HandleAuraModMinimumSpeedRate(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    aurApp->GetTarget()->UpdateSpeed(MOVE_RUN, true);
}

void AuraEffect::HandleDisableGravity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (Unit* target = aurApp->GetTarget())
    {
        if (apply)
            target->SetDisableGravity(true);
    }
}

void AuraEffect::HandleFixate(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* caster = GetCaster();
    if (!caster || !caster->CanHaveThreatList())
        return;

    Unit* target = aurApp->GetTarget();
    if (!target || target == caster)
        return;

    //if (apply)
    //{
    //    caster->AddThreat(target, std::numeric_limits<float>::max());
    //    caster->TauntApply(target);
    //}
    //else
    //    caster->TauntFadeOut(target);
}

void AuraEffect::HandleModTimeRate(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    target->ApplyPercentModFloatValue(UNIT_FIELD_MOD_TIME_RATE, GetAmount(), !apply);

    target->UpdateMeleeHastMod();

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateSpeed(MOVE_WALK, true);
    target->UpdateSpeed(MOVE_SWIM, true);
    target->UpdateSpeed(MOVE_FLIGHT, true);
    target->UpdateSpeed(MOVE_RUN_BACK, true);
    target->UpdateSpeed(MOVE_SWIM_BACK, true);
    target->UpdateSpeed(MOVE_FLIGHT_BACK, true);

    if (target->IsCreature() && target->IsSplineEnabled())
    {
        target->SendAdjustSplineDuration(target->GetFloatValue(UNIT_FIELD_MOD_TIME_RATE));
        target->UpdateSplineSpeed();
    }

    auto player = target->ToPlayer();
    if (!player)
        return;

    float Multiplier = 1.0f;
    ApplyPercentModFloatVar(Multiplier, GetAmount(), !apply);
    SpellCooldowns* cd_list = player->GetSpellCooldowns();
    for (auto& itr : *cd_list)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr.first);
        if (!spellInfo)
            continue;

        if (spellInfo->Category.ChargeRecoveryTime)
            continue;

        //Example: 210339
        if (spellInfo->Id == GetId())
            continue;

        double t = getPreciseTime();
        double cooldown = double(itr.second.end > t ? itr.second.end - t : 0.0) * IN_MILLISECONDS;
        cooldown *= Multiplier;
        itr.second.end = getPreciseTime() + cooldown / IN_MILLISECONDS;

        WorldPackets::Spells::ModifyCooldownRecoverySpeed package;
        package.SpellID = spellInfo->Id;
        package.Multiplier = Multiplier;
        package.Multiplier2 = apply ? Multiplier : 1.0f;
        player->SendDirectMessage(package.Write());
    }

    SpellChargeDataMap* charge_list = player->GetSpellChargeDatas();
    for (auto& itr : *charge_list)
    {
        SpellChargeData& data = itr.second;
        if (!data.chargeRegenTime || !data.spellInfo)
            continue;

        if (data.spellInfo->Id == GetId())
            continue;

        data.chargeRegenTime *= Multiplier;
        WorldPackets::Spells::ModifyChargeRecoverySpeed package;
        package.SpellID = data.categoryEntry->ID;
        package.Multiplier = Multiplier;
        package.Multiplier2 = apply ? Multiplier : 1.0f;
        player->SendDirectMessage(package.Write());
    }

    for (auto& appliedAura : target->GetAppliedAuras())
    {
        auto aura = appliedAura.second->GetBase();
        if (!aura)
            continue;

        if (aura->IsPassive() || aura->GetDuration() == -1 || aura->GetSpellInfo()->HasAttribute(SPELL_ATTR5_HIDE_DURATION))
            continue;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetSpellInfo()->EffectMask < uint32(1 << i))
                break;

            if (AuraEffect* aurEff = aura->GetEffect(i))
            {
                if (!aurEff->GetPeriod())
                    continue;

                if (apply)
                {
                    int32 period = aurEff->GetPeriod() * Multiplier;
                    aurEff->SetPeriodMod(aurEff->GetPeriodMod() - (aurEff->GetPeriod() - period));
                    aurEff->SetAmplitude(period);
                    aurEff->SetPeriodicTimer(aurEff->GetPeriodicTimer() * Multiplier);
                }
                else
                {
                    aurEff->SetPeriodMod(0.0f);
                    aurEff->RecalculateTickPeriod(aura->GetCaster());
                }
            }
        }

        aura->SetMaxDuration(aura->GetMaxDuration() * Multiplier);
        aura->SetDuration(aura->GetDuration() * Multiplier);
        aura->TimeMod = apply ? Multiplier : 1.0f;
        aura->SetNeedClientUpdateForTargets();
    }
}

void AuraEffect::HandleAuraPvpTalents(AuraApplication const* auraApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = auraApp->GetTarget();
    if (!target)
        return;

    if (Player* plr = target->ToPlayer())
    {
        if (apply)
        {
            plr->TogglePvpTalents(true);
        }
        else if (!target->HasAuraType(SPELL_AURA_PVP_TALENTS))
        {
            plr->TogglePvpTalents(false);
        }

        plr->SendOperationsAfterDelay(OAD_RECALC_PVP_BP);
        plr->SetPvpRulesTimer(false);
    }
}

void AuraEffect::HandleEnablePvpStatsScaling(AuraApplication const* auraApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = auraApp->GetTarget();
    if (!target)
        return;

    if (Player* plr = target->ToPlayer())
    {
        plr->TogglePvpStatsScaling(apply);
        plr->SendUpdateStat(USM_ALL);
    }
}

void AuraEffect::HandleAuraContestedPvP(AuraApplication const* auraApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = auraApp->GetTarget();
    if (!target || !target->IsPlayer())
        return;

    if (apply)
        target->ToPlayer()->SetContestedPvP(nullptr, true);
    else
        target->ToPlayer()->ResetContestedPvP();
}

void AuraEffect::HandleAuraModCooldownSpeedRate(AuraApplication const* auraApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL) && !(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT))
        return;

    auto target = auraApp->GetTarget();
    if (!target || !target->IsPlayer())
        return;

    Player* player = target->ToPlayer();
    SpellCooldowns* cd_list = player->GetSpellCooldowns();
    for (auto& itr : *cd_list)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr.first);
        if (!spellInfo)
            continue;

        if (spellInfo->Category.ChargeRecoveryTime)
            continue;

        float amount = 0;
        switch (GetId())
        {
            case 209376: // Forbearant Faithful
                if (spellInfo->Id == 1022 || spellInfo->Id == 204018 || spellInfo->Id == 642 || spellInfo->Id == 633)
                    amount = GetAmount();
                break;
            case 214170: // Spiritual Journey
                if (spellInfo->Id == 51533)
                    amount = GetAmount();
                break;
            case 202776: // Blurred Time
                amount = GetAmount();
                break;
            case 216331: // Avenging Crusader (Honor Talent)
                if (spellInfo->Id == 35395 || spellInfo->Id == 20271)
                    amount = GetAmount();
                break;
            case 204366: // Thundercharge (Honor Talent)
                if (m_effIndex != EFFECT_0)
                    break;
                amount = GetAmount();
                break;
            case 233397: // Delirium
                if (spellInfo->HasAura(SPELL_AURA_MOD_INCREASE_SPEED) || spellInfo->HasAura(SPELL_AURA_MOD_SPEED_NO_CONTROL))
                    amount = GetAmount();
                break;
            case 214975: // Heartstop Aura (Honor Talent)
                amount = GetAmount();
                break;
            case 152173: // Serenity
                if (spellInfo->Id == 107428 || spellInfo->Id == 113656 || spellInfo->Id == 205320 || spellInfo->Id == 116847)
                    amount = GetAmount();
                break;
            default:
                break;
        }

        if (!amount)
            continue;

        float Multiplier = 1.0f;
        ApplyPercentModFloatVar(Multiplier, amount, !apply);

        double t = getPreciseTime();
        double cooldown = double(itr.second.end > t ? itr.second.end - t : 0.0) * IN_MILLISECONDS;
        cooldown *= Multiplier;
        itr.second.end = getPreciseTime() + cooldown / IN_MILLISECONDS;

        WorldPackets::Spells::ModifyCooldownRecoverySpeed package;
        package.SpellID = spellInfo->Id;
        package.Multiplier = Multiplier;
        package.Multiplier2 = apply ? Multiplier : 1.0f;
        player->SendDirectMessage(package.Write());
    }

    SpellChargeDataMap* charge_list = player->GetSpellChargeDatas();
    for (auto& itr : *charge_list)
    {
        SpellChargeData& data = itr.second;
        if (!data.chargeRegenTime || !data.spellInfo)
            continue;

        float amount = 0;
        switch (GetId())
        {
            case 209376: // Forbearant Faithful
                if (data.spellInfo->Id == 1022 || data.spellInfo->Id == 204018 || data.spellInfo->Id == 642 || data.spellInfo->Id == 633)
                    amount = GetAmount();
                break;
            case 214170: // Spiritual Journey
                if (data.spellInfo->Id == 51533)
                    amount = GetAmount();
                break;
            case 202776: // Blurred Time
                amount = GetAmount();
                break;
            case 216331: // Avenging Crusader (Honor Talent)
                if (data.spellInfo->Id == 35395 || data.spellInfo->Id == 20271)
                    amount = GetAmount();
                break;
            case 204366: // Thundercharge (Honor Talent)
                if (m_effIndex != EFFECT_0)
                    break;
                amount = GetAmount();
                break;
            case 233397: // Delirium
                if (data.spellInfo->HasAura(SPELL_AURA_MOD_INCREASE_SPEED) || data.spellInfo->HasAura(SPELL_AURA_MOD_SPEED_NO_CONTROL))
                    amount = GetAmount();
                break;
            case 214975: // Heartstop Aura (Honor Talent)
                amount = GetAmount();
                break;
            case 152173: // Serenity
                if (data.spellInfo->Id == 107428 || data.spellInfo->Id == 113656 || data.spellInfo->Id == 205320 || data.spellInfo->Id == 116847)
                    amount = GetAmount();
                break;
            default:
                break;
        }

        if (!amount)
            continue;

        float Multiplier = 1.0f;
        ApplyPercentModFloatVar(Multiplier, amount, !apply);

        data.chargeRegenTime *= Multiplier;
        data.timer *= Multiplier;

        WorldPackets::Spells::ModifyChargeRecoverySpeed package;
        package.SpellID = data.categoryEntry->ID;
        package.Multiplier = Multiplier;
        package.Multiplier2 = apply ? Multiplier : 1.0f;
        player->SendDirectMessage(package.Write());
    }
}

void AuraEffect::HandleAuraModCooldownPct(AuraApplication const* auraApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = auraApp->GetTarget();
    if (!target || !target->IsPlayer())
        return;

    return; // Disable, need more info how is work

    Player* player = target->ToPlayer();
    SpellCooldowns* cd_list = player->GetSpellCooldowns();
    for (auto& itr : *cd_list)
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr.first))
        {
            float modCooldownPct = 1.0f;
            if (apply)
            {
                switch (GetId())
                {
                    case 214975: // Heartstop Aura (Honor Talent)
                        AddPct(modCooldownPct, -GetAmount());
                        break;
                    default:
                        break;
                }
            }

            double t = getPreciseTime();
            double cooldown = double(itr.second.end > t ? itr.second.end - t : 0.0) * IN_MILLISECONDS;
            cooldown = CalculatePct(cooldown, (100 - (itr.second.rate * 100)) + 100); // Normalize CD
            cooldown *= modCooldownPct;
            itr.second.end = getPreciseTime() + cooldown / IN_MILLISECONDS;
            itr.second.rate = modCooldownPct;

            WorldPackets::Character::CooldownCheat package;
            package.CheatCode = spellInfo->Id;
            package.Value = modCooldownPct;
            player->SendDirectMessage(package.Write());
        }
    }
}

void AuraEffect::HandleAuraProcOnHpBelow(AuraApplication const* auraApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = auraApp->GetTarget();
    Unit* caster = GetCaster();
    if (!target || !caster)
        return;

    uint32 curCount = target->CountPctFromMaxHealth(GetAmount());
    uint32 triggered_spell_id = GetTriggerSpell();
    SpellInfo const* triggerEntry = sSpellMgr->GetSpellInfo(triggered_spell_id);
    if(!triggerEntry)
        return;

    uint32 curHealth = target->GetHealth();

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "HandleAuraProcOnHpBelow GetId %u triggered_spell_id %u curCount %u curHealth %u", GetId(), triggered_spell_id, curCount, curHealth);

    if(GetMiscValue() == 1) // Below
    {
        if (curCount > curHealth)
        {
            if (GetId() == 214622 && caster->HasAura(214648)) // Warlord's Fortitude, prevent proc
                return;
            if (GetId() == 210532 && caster->HasAura(205069)) // Seraphim's Blessing (Honor Talent)
                return;

            caster->CastSpell(target, triggered_spell_id, true, nullptr, this);
        }
    }
    if (GetMiscValue() == 0) // Above
        if (curCount < curHealth)
            caster->CastSpell(target, triggered_spell_id, true, nullptr, this);
}

void AuraEffect::HandleExpedite(AuraApplication const* auraApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = auraApp->GetTarget();
    Unit* caster = GetCaster();
    if (!target || !caster)
        return;

    float perc = 1.0f;
    AddPct(perc, GetAmount());

    // TODO Need more research
    AuraType type = SPELL_AURA_NONE;
    if (GetMiscValue() == 174)
        type = SPELL_AURA_PERIODIC_DAMAGE;
    if (GetMiscValue() == 182)
        type = SPELL_AURA_PERIODIC_DAMAGE_PERCENT;

    if (type != SPELL_AURA_NONE)
    {
        std::set<Aura*> auraList;
        if (Unit::AuraEffectList* mTotalAuraList = target->GetAuraEffectsByType(type))
        {
            for (Unit::AuraEffectList::iterator i = mTotalAuraList->begin(); i != mTotalAuraList->end(); ++i)
            {
                AuraEffect* eff = (*i);
                if (eff->GetCasterGUID() == GetCasterGUID())
                {
                    auraList.insert(eff->GetBase());
                    if (apply)
                    {
                        int32 period = eff->GetPeriod() / perc;
                        eff->SetPeriodMod(eff->GetPeriodMod() - (eff->GetPeriod() - period));
                        eff->SetAmplitude(period);
                        eff->SetPeriodicTimer(eff->GetPeriodicTimer() / perc);
                    }
                    else
                    {
                        eff->SetPeriodMod(0.0f);
                        eff->RecalculateTickPeriod(caster);
                    }
                }
            }
        }

        for (auto aura : auraList)
        {
            if (aura)
            {
                if (apply)
                {
                    int32 _duration = aura->GetDuration() / perc;
                    if (_duration > aura->GetMaxDuration())
                        aura->SetMaxDuration(_duration);
                    aura->SetDuration(_duration);
                }
                else
                {
                    int32 _duration = aura->GetDuration() * perc;
                    if (_duration > aura->GetMaxDuration())
                        aura->SetMaxDuration(_duration);
                    aura->SetDuration(_duration);
                }
            }
        }
    }
}

void AuraEffect::HandleModVisibilityRange(AuraApplication const* auraApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    auto target = auraApp->GetTarget();
    if (!target)
        return;

    if (apply)
        target->SetRWVisibilityRange(GetAmount());
    else
        target->SetRWVisibilityRange(0.0f);
}
