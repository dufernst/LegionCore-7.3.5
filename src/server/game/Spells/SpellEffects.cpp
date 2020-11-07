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

#include "AccountMgr.h"
#include "AreaTrigger.h"
#include "Battleground.h"
#include "BattlegroundDeepwindGorge.h"
#include "BattlegroundMgr.h"
#include "BattlegroundTwinPeaks.h"
#include "BattlegroundWarsongGulch.h"
#include "CellImpl.h"
#include "CollectionMgr.h"
#include "CombatLogPackets.h"
#include "Common.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "DatabaseEnv.h"
#include "DuelPackets.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "Garrison.h"
#include "GossipDef.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "InstanceScript.h"
#include "Language.h"
#include "Log.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "Pet.h"
#include "PetBattle.h"
#include "Player.h"
#include "QuestData.h"
#include "ScriptMgr.h"
#include "ScriptsData.h"
#include "SharedDefines.h"
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "SocialMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "SpellPackets.h"
#include "TalentPackets.h"
#include "TemporarySummon.h"
#include "Totem.h"
#include "Unit.h"
#include "UpdateData.h"
#include "Util.h"
#include "Vehicle.h"
#include "WaypointMovementGenerator.h"
#include "World.h"
#include "WorldPacket.h"
#include <G3D/Quat.h>

pEffect SpellEffects[TOTAL_SPELL_EFFECTS] =
{
    &Spell::EffectNULL,                                     //  0
    &Spell::EffectInstaKill,                                //  1 SPELL_EFFECT_INSTAKILL
    &Spell::EffectSchoolDMG,                                //  2 SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::EffectDummy,                                    //  3 SPELL_EFFECT_DUMMY
    &Spell::EffectUnused,                                   //  4 SPELL_EFFECT_4
    &Spell::EffectNULL,                                     //  5 SPELL_EFFECT_5
    &Spell::EffectApplyAura,                                //  6 SPELL_EFFECT_APPLY_AURA
    &Spell::EffectEnvironmentalDMG,                         //  7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::EffectPowerDrain,                               //  8 SPELL_EFFECT_POWER_DRAIN
    &Spell::EffectHealthLeech,                              //  9 SPELL_EFFECT_HEALTH_LEECH
    &Spell::EffectHeal,                                     // 10 SPELL_EFFECT_HEAL
    &Spell::EffectBind,                                     // 11 SPELL_EFFECT_BIND
    &Spell::EffectNULL,                                     // 12 SPELL_EFFECT_PORTAL
    &Spell::EffectNULL,                                     // 13 SPELL_EFFECT_13
    &Spell::EffectIncreaseCurrencyCap,                      // 14 SPELL_EFFECT_INCREASE_CURRENCY_CAP
    &Spell::EffectTeleportUnits,                            // 15 SPELL_EFFECT_TELEPORT_FROM_PORTAL
    &Spell::EffectQuestComplete,                            // 16 SPELL_EFFECT_QUEST_COMPLETE
    &Spell::EffectWeaponDmg,                                // 17 SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::EffectResurrect,                                // 18 SPELL_EFFECT_RESURRECT
    &Spell::EffectAddExtraAttacks,                          // 19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::EffectNULL,                                     // 20 SPELL_EFFECT_DODGE
    &Spell::EffectNULL,                                     // 21 SPELL_EFFECT_EVADE
    &Spell::EffectParry,                                    // 22 SPELL_EFFECT_PARRY
    &Spell::EffectBlock,                                    // 23 SPELL_EFFECT_BLOCK
    &Spell::EffectCreateItem,                               // 24 SPELL_EFFECT_CREATE_ITEM
    &Spell::EffectNULL,                                     // 25 SPELL_EFFECT_WEAPON
    &Spell::EffectNULL,                                     // 26 SPELL_EFFECT_DEFENSE
    &Spell::EffectPersistentAA,                             // 27 SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::EffectSummonType,                               // 28 SPELL_EFFECT_SUMMON
    &Spell::EffectLeap,                                     // 29 SPELL_EFFECT_LEAP
    &Spell::EffectEnergize,                                 // 30 SPELL_EFFECT_ENERGIZE
    &Spell::EffectWeaponDmg,                                // 31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::EffectTriggerMissileSpell,                      // 32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::EffectOpenLock,                                 // 33 SPELL_EFFECT_OPEN_LOCK
    &Spell::EffectSummonChangeItem,                         // 34 SPELL_EFFECT_SUMMON_CHANGE_ITEM
    &Spell::EffectApplyAreaAura,                            // 35 SPELL_EFFECT_APPLY_AREA_AURA_PARTY
    &Spell::EffectLearnSpell,                               // 36 SPELL_EFFECT_LEARN_SPELL
    &Spell::EffectUnused,                                   // 37 SPELL_EFFECT_SPELL_DEFENSE            one spell: SPELLDEFENSE (DND)
    &Spell::EffectDispel,                                   // 38 SPELL_EFFECT_DISPEL
    &Spell::EffectUnused,                                   // 39 SPELL_EFFECT_LANGUAGE
    &Spell::EffectDualWield,                                // 40 SPELL_EFFECT_DUAL_WIELD
    &Spell::EffectJump,                                     // 41 SPELL_EFFECT_JUMP
    &Spell::EffectJumpDest,                                 // 42 SPELL_EFFECT_JUMP_DEST
    &Spell::EffectTeleUnitsFaceCaster,                      // 43 SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::EffectLearnSkill,                               // 44 SPELL_EFFECT_SKILL_STEP
    &Spell::EffectPlayMovie,                                // 45 SPELL_EFFECT_PLAY_MOVIE
    &Spell::EffectUnused,                                   // 46 SPELL_EFFECT_SPAWN clientside, unit appears as if it was just spawned
    &Spell::EffectTradeSkill,                               // 47 SPELL_EFFECT_TRADE_SKILL
    &Spell::EffectUnused,                                   // 48 SPELL_EFFECT_STEALTH                  one spell: Base Stealth
    &Spell::EffectUnused,                                   // 49 SPELL_EFFECT_DETECT                   one spell: Detect
    &Spell::EffectTransmitted,                              // 50 SPELL_EFFECT_TRANS_DOOR
    &Spell::EffectUnused,                                   // 51 SPELL_EFFECT_51
    &Spell::EffectSetMaxBattlePetCount,                     // 52 SPELL_EFFECT_SET_MAX_BATTLE_PET_COUNT
    &Spell::EffectEnchantItemPerm,                          // 53 SPELL_EFFECT_ENCHANT_ITEM
    &Spell::EffectEnchantItemTmp,                           // 54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::EffectTameCreature,                             // 55 SPELL_EFFECT_TAMECREATURE
    &Spell::EffectSummonPet,                                // 56 SPELL_EFFECT_SUMMON_PET
    &Spell::EffectLearnPetSpell,                            // 57 SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::EffectWeaponDmg,                                // 58 SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::EffectCreateRandomItem,                         // 59 SPELL_EFFECT_CREATE_RANDOM_ITEM       create item base at spell specific loot
    &Spell::EffectProficiency,                              // 60 SPELL_EFFECT_PROFICIENCY
    &Spell::EffectSendEvent,                                // 61 SPELL_EFFECT_SEND_EVENT
    &Spell::EffectPowerBurn,                                // 62 SPELL_EFFECT_POWER_BURN
    &Spell::EffectThreat,                                   // 63 SPELL_EFFECT_THREAT
    &Spell::EffectTriggerSpell,                             // 64 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::EffectApplyAreaAura,                            // 65 SPELL_EFFECT_APPLY_AREA_AURA_RAID
    &Spell::EffectRechargeManaGem,                          // 66 SPELL_EFFECT_CREATE_MANA_GEM          (possibly recharge it, misc - is item ID)
    &Spell::EffectHealMaxHealth,                            // 67 SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::EffectInterruptCast,                            // 68 SPELL_EFFECT_INTERRUPT_CAST
    &Spell::EffectDistract,                                 // 69 SPELL_EFFECT_DISTRACT
    &Spell::EffectNULL,                                     // 70 SPELL_EFFECT_INSTANT_COMPLETE_LOCAL_QUEST
    &Spell::EffectPickPocket,                               // 71 SPELL_EFFECT_PICKPOCKET
    &Spell::EffectAddFarsight,                              // 72 SPELL_EFFECT_ADD_FARSIGHT
    &Spell::EffectUntrainTalents,                           // 73 SPELL_EFFECT_UNTRAIN_TALENTS
    &Spell::EffectApplyGlyph,                               // 74 SPELL_EFFECT_APPLY_GLYPH
    &Spell::EffectHealMechanical,                           // 75 SPELL_EFFECT_HEAL_MECHANICAL          one spell: Mechanical Patch Kit
    &Spell::EffectSummonObjectWild,                         // 76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::EffectScriptEffect,                             // 77 SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::EffectUnused,                                   // 78 SPELL_EFFECT_ATTACK
    &Spell::EffectSanctuary,                                // 79 SPELL_EFFECT_SANCTUARY
    &Spell::EffectModAssistantEquipmentLevel,               // 80 SPELL_EFFECT_MOD_ASSISTANT_EQUIPMENT_LEVEL
    &Spell::EffectNULL,                                     // 81 SPELL_EFFECT_RELOCATE_ACTIVE_ABILITY_TO_THE_BAR
    &Spell::EffectNULL,                                     // 82 SPELL_EFFECT_BIND_SIGHT
    &Spell::EffectDuel,                                     // 83 SPELL_EFFECT_DUEL
    &Spell::EffectStuck,                                    // 84 SPELL_EFFECT_STUCK
    &Spell::EffectSummonPlayer,                             // 85 SPELL_EFFECT_SUMMON_PLAYER
    &Spell::EffectActivateObject,                           // 86 SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::EffectGameObjectDamage,                         // 87 SPELL_EFFECT_GAMEOBJECT_DAMAGE
    &Spell::EffectGameObjectRepair,                         // 88 SPELL_EFFECT_GAMEOBJECT_REPAIR
    &Spell::EffectGameObjectSetDestructionState,            // 89 SPELL_EFFECT_GAMEOBJECT_SET_DESTRUCTION_STATE
    &Spell::EffectKillCreditPersonal,                       // 90 SPELL_EFFECT_KILL_CREDIT              Kill credit but only for single person
    &Spell::EffectUnused,                                   // 91 SPELL_EFFECT_THREAT_ALL               one spell: zzOLDBrainwash
    &Spell::EffectEnchantHeldItem,                          // 92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::EffectForceDeselect,                            // 93 SPELL_EFFECT_FORCE_DESELECT
    &Spell::EffectSelfResurrect,                            // 94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::EffectSkinning,                                 // 95 SPELL_EFFECT_SKINNING
    &Spell::EffectCharge,                                   // 96 SPELL_EFFECT_CHARGE
    &Spell::EffectCastButtons,                              // 97 SPELL_EFFECT_CAST_BUTTON (totem bar since 3.2.2a)
    &Spell::EffectKnockBack,                                // 98 SPELL_EFFECT_KNOCK_BACK
    &Spell::EffectDisEnchant,                               // 99 SPELL_EFFECT_DISENCHANT
    &Spell::EffectInebriate,                                //100 SPELL_EFFECT_INEBRIATE
    &Spell::EffectFeedPet,                                  //101 SPELL_EFFECT_FEED_PET
    &Spell::EffectDismissPet,                               //102 SPELL_EFFECT_DISMISS_PET
    &Spell::EffectReputation,                               //103 SPELL_EFFECT_GIVE_REPUTATION
    &Spell::EffectSummonObject,                             //104 SPELL_EFFECT_SUMMON_OBJECT_SLOT
    &Spell::EffectSurvey,                                   //105 SPELL_EFFECT_SURVEY
    &Spell::EffectSummonRaidMarker,                         //106 SPELL_EFFECT_SUMMON_RAID_MARKER
    &Spell::EffectCorpseLoot,                               //107 SPELL_EFFECT_LOOT_CORPSE
    &Spell::EffectDispelMechanic,                           //108 SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::EffectSummonDeadPet,                            //109 SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::EffectNULL,                                     //110 SPELL_EFFECT_110
    &Spell::EffectDurabilityDamage,                         //111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::EffectNULL,                                     //112 SPELL_EFFECT_112
    &Spell::EffectResurrectNew,                             //113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::EffectTaunt,                                    //114 SPELL_EFFECT_ATTACK_ME
    &Spell::EffectDurabilityDamagePCT,                      //115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::EffectSkinPlayerCorpse,                         //116 SPELL_EFFECT_SKIN_PLAYER_CORPSE       one spell: Remove Insignia, bg usage, required special corpse flags...
    &Spell::EffectSpiritHeal,                               //117 SPELL_EFFECT_SPIRIT_HEAL              one spell: Spirit Heal
    &Spell::EffectSkill,                                    //118 SPELL_EFFECT_SKILL                    professions and more
    &Spell::EffectApplyAreaAura,                            //119 SPELL_EFFECT_APPLY_AREA_AURA_PET
    &Spell::EffectUnused,                                   //120 SPELL_EFFECT_120
    &Spell::EffectWeaponDmg,                                //121 SPELL_EFFECT_NORMALIZED_WEAPON_DMG
    &Spell::EffectNULL,                                     //122 SPELL_EFFECT_DYNAMIC_CHECK_PLAYER_POSITION
    &Spell::EffectSendTaxi,                                 //123 SPELL_EFFECT_SEND_TAXI                taxi/flight related (misc value is taxi path id)
    &Spell::EffectPullTowards,                              //124 SPELL_EFFECT_PULL_TOWARDS
    &Spell::EffectModifyThreatPercent,                      //125 SPELL_EFFECT_MODIFY_THREAT_PERCENT
    &Spell::EffectStealBeneficialBuff,                      //126 SPELL_EFFECT_STEAL_BENEFICIAL_BUFF    spell steal effect?
    &Spell::EffectProspecting,                              //127 SPELL_EFFECT_PROSPECTING              Prospecting spell
    &Spell::EffectApplyAreaAura,                            //128 SPELL_EFFECT_APPLY_AREA_AURA_FRIEND
    &Spell::EffectApplyAreaAura,                            //129 SPELL_EFFECT_APPLY_AREA_AURA_ENEMY
    &Spell::EffectRedirectThreat,                           //130 SPELL_EFFECT_REDIRECT_THREAT
    &Spell::EffectPlaySound,                                //131 SPELL_EFFECT_PLAY_SOUND               sound id in misc value (SoundEntries.dbc)
    &Spell::EffectPlayMusic,                                //132 SPELL_EFFECT_PLAY_MUSIC               sound id in misc value (SoundEntries.dbc)
    &Spell::EffectUnlearnSpecialization,                    //133 SPELL_EFFECT_UNLEARN_SPECIALIZATION   unlearn profession specialization
    &Spell::EffectKillCredit,                               //134 SPELL_EFFECT_KILL_CREDIT              misc value is creature entry
    &Spell::EffectNULL,                                     //135 SPELL_EFFECT_CALL_PET
    &Spell::EffectHealPct,                                  //136 SPELL_EFFECT_HEAL_PCT
    &Spell::EffectEnergizePct,                              //137 SPELL_EFFECT_ENERGIZE_PCT
    &Spell::EffectLeapBack,                                 //138 SPELL_EFFECT_LEAP_BACK                Leap back
    &Spell::EffectQuestClear,                               //139 SPELL_EFFECT_CLEAR_QUEST              Reset quest status (miscValue - quest ID)
    &Spell::EffectForceCast,                                //140 SPELL_EFFECT_FORCE_CAST
    &Spell::EffectForceCast,                                //141 SPELL_EFFECT_FORCE_CAST_WITH_VALUE
    &Spell::EffectTriggerSpell,                             //142 SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE
    &Spell::EffectApplyAreaAura,                            //143 SPELL_EFFECT_APPLY_AREA_AURA_OWNER
    &Spell::EffectKnockBack,                                //144 SPELL_EFFECT_KNOCK_BACK_DEST
    &Spell::EffectPullTowards,                              //145 SPELL_EFFECT_PULL_TOWARDS_DEST                      Black Hole Effect
    &Spell::EffectChangeFollowerVitality,                   //146 SPELL_EFFECT_RESTORE_ASSISTANT_HEALTH 
    &Spell::EffectQuestFail,                                //147 SPELL_EFFECT_QUEST_FAIL               quest fail
    &Spell::EffectTriggerMissileSpell,                      //148 SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE
    &Spell::EffectChargeDest,                               //149 SPELL_EFFECT_CHARGE_DEST
    &Spell::EffectQuestStart,                               //150 SPELL_EFFECT_QUEST_START
    &Spell::EffectTriggerRitualOfSummoning,                 //151 SPELL_EFFECT_TRIGGER_SPELL_2
    &Spell::EffectSummonRaFFriend,                          //152 SPELL_EFFECT_SUMMON_RAF_FRIEND        summon Refer-a-Friend
    &Spell::EffectCreateTamedPet,                           //153 SPELL_EFFECT_CREATE_TAMED_PET         misc value is creature entry
    &Spell::EffectDiscoverTaxi,                             //154 SPELL_EFFECT_DISCOVER_TAXI
    &Spell::EffectTitanGrip,                                //155 SPELL_EFFECT_TITAN_GRIP Allows you to equip two-handed axes, maces and swords in one hand, but you attack $49152s1% slower than normal.
    &Spell::EffectEnchantItemPrismatic,                     //156 SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC
    &Spell::EffectCreateItem2,                              //157 SPELL_EFFECT_CREATE_ITEM_2            create item or create item template and replace by some randon spell loot item
    &Spell::EffectMilling,                                  //158 SPELL_EFFECT_MILLING                  milling
    &Spell::EffectRenamePet,                                //159 SPELL_EFFECT_ALLOW_RENAME_PET         allow rename pet once again
    &Spell::EffectTriggerSpell,                             //160 SPELL_EFFECT_TRIGGER_SPELL_4
    &Spell::EffectSpecCount,                                //161 SPELL_EFFECT_TALENT_SPEC_COUNT        second talent spec (learn/revert)
    &Spell::EffectActivateSpec,                             //162 SPELL_EFFECT_TALENT_SPEC_SELECT       activate primary/secondary spec
    &Spell::EffectObliterateItem,                           //163 SPELL_EFFECT_OBLITERATE_ITEM
    &Spell::EffectRemoveAura,                               //164 SPELL_EFFECT_REMOVE_AURA
    &Spell::EffectDamageFromMaxHealthPCT,                   //165 SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT
    &Spell::EffectGiveCurrency,                             //166 SPELL_EFFECT_GIVE_CURRENCY
    &Spell::EffectUpdatePlayerPhase,                        //167 SPELL_EFFECT_UPDATE_PLAYER_PHASE
    &Spell::EffectNULL,                                     //168 SPELL_EFFECT_CONTROL_PET
    &Spell::EffectDestroyItem,                              //169 SPELL_EFFECT_DESTROY_ITEM
    &Spell::EffectUpdateZoneAurasAndPhases,                 //170 SPELL_EFFECT_UPDATE_ZONE_AURAS_AND_PHASES
    &Spell::EffectSummonObject,                             //171 SPELL_EFFECT_OBJECT_WITH_PERSONAL_VISIBILITY
    &Spell::EffectResurrectWithAura,                        //172 SPELL_EFFECT_RESURRECT_WITH_AURA
    &Spell::EffectBuyGuilkBankTab,                          //173 SPELL_EFFECT_UNLOCK_GUILD_VAULT_TAB
    &Spell::EffectApplyAreaAura,                            //174 SPELL_EFFECT_APPLY_AURA_ON_PET_OR_SELF
    &Spell::EffectNULL,                                     //175 SPELL_EFFECT_MOD_THRET_OF_TARGET_NYI
    &Spell::EffectBecomeUntargetable,                       //176 SPELL_EFFECT_BECOME_UNTARGETABLE
    &Spell::EffectDespawnDynamicObject,                     //177 SPELL_EFFECT_DESPAWN_DYNOBJECT
    &Spell::EffectNULL,                                     //178 SPELL_EFFECT_CANCEL_SCENE_ASSIGNMENT
    &Spell::EffectCreateAreaTrigger,                        //179 SPELL_EFFECT_CREATE_AREATRIGGER
    &Spell::EffectNULL,                                     //180 SPELL_EFFECT_UPDATE_AREATRIGGER
    &Spell::EffectUnlearnTalent,                            //181 SPELL_EFFECT_UNLEARN_TALENT
    &Spell::EffectDespawnAreatrigger,                       //182 SPELL_EFFECT_DESPAWN_AREATRIGGER
    &Spell::EffectNULL,                                     //183 SPELL_EFFECT_ENVEREMENTAL_DAMAGE_2 @Create areatrigger
    &Spell::EffectNULL,                                     //184 SPELL_EFFECT_REPUTATION_REWARD
    &Spell::SendScene,                                      //185 SPELL_EFFECT_ACTIVATE_SCENE4
    &Spell::SendScene,                                      //186 SPELL_EFFECT_ACTIVATE_SCENE5
    &Spell::EffectRandomizeDigsites,                        //187 SPELL_EFFECT_RANDOMIZE_DIGSITES
    &Spell::EffectNULL,                                     //188 SPELL_EFFECT_STAMPEDE
    &Spell::EffectBonusLoot,                                //189 SPELL_EFFECT_LOOT_BONUS
    &Spell::EffectJoinOrLeavePlayerParty,                   //190 SPELL_EFFECT_JOIN_LEAVE_PLAYER_PARTY
    &Spell::EffectTeleportToDigsite,                        //191 SPELL_EFFECT_TELEPORT_TO_DIGSITE
    &Spell::EffectUncageBattlePet,                          //192 SPELL_EFFECT_UNCAGE_BATTLE_PET
    &Spell::EffectNULL,                                     //193 SPELL_EFFECT_LAUNCH_PET_BATTLE
    &Spell::EffectNULL,                                     //194 SPELL_EFFECT_194
    &Spell::SendScene,                                      //195 SPELL_EFFECT_ACTIVATE_SCENE
    &Spell::SendScene,                                      //196 SPELL_EFFECT_ACTIVATE_SCENE2
    &Spell::SendScene,                                      //197 SPELL_EFFECT_ACTIVATE_SCENE6
    &Spell::SendScene,                                      //198 SPELL_EFFECT_ACTIVATE_SCENE3 send package
    &Spell::EffectNULL,                                     //199 SPELL_EFFECT_199
    &Spell::EffectHealBattlePetPct,                         //200 SPELL_EFFECT_HEAL_BATTLEPET_PCT
    &Spell::EffectUnlockPetBattles,                         //201 SPELL_EFFECT_UNLOCK_PET_BATTLES
    &Spell::EffectApplyAreaAura,                            //202 SPELL_EFFECT_APPLY_AURA_WITH_VALUE
    &Spell::EffectRemoveAura,                               //203 SPELL_EFFECT_REMOVE_AURA_2 Based on 144863 -> This spell remove auras. 145052 possible trigger spell.
    &Spell::EffectUpgradeBattlePet,                         //204 SPELL_EFFECT_UPGRADE_BATTLE_PET
    &Spell::EffectLaunchQuestChoice,                        //205 SPELL_EFFECT_LAUNCH_QUEST_CHOICE
    &Spell::EffectCreateItem3,                              //206 SPELL_EFFECT_CREATE_ITEM_3
    &Spell::EffectNULL,                                     //207 SPELL_EFFECT_LAUNCH_QUEST_TASK
    &Spell::EffectModReputation,                            //208 SPELL_EFFECT_MOD_REPUTATION
    &Spell::EffectNULL,                                     //209 SPELL_EFFECT_209
    &Spell::EffectLearnGarrisonBuilding,                    //210 SPELL_EFFECT_LEARN_GARRISON_BUILDING
    &Spell::EffectNULL,                                     //211 SPELL_EFFECT_LEARN_GARRISON_SPECIALIZATION
    &Spell::EffectNULL,                                     //212 SPELL_EFFECT_212
    &Spell::EffectJumpDest,                                 //213 SPELL_EFFECT_JUMP_DEST2
    &Spell::EffectCreateGarrison,                           //214 SPELL_EFFECT_CREATE_GARRISON
    &Spell::EffectNULL,                                     //215 SPELL_EFFECT_UPGRADE_CHARACTER_SPELLS
    &Spell::EffectCreateGarrisonShipment,                   //216 SPELL_EFFECT_CREATE_SHIPMENT
    &Spell::EffectNULL,                                     //217 SPELL_EFFECT_UPGRADE_GARRISON
    &Spell::EffectNULL,                                     //218 SPELL_EFFECT_218
    &Spell::EffectSummonConversation,                       //219 SPELL_EFFECT_SUMMON_CONVERSATION
    &Spell::EffectAddGarrisonFollower,                      //220 SPELL_EFFECT_ADD_GARRISON_FOLLOWER
    &Spell::EffectAddGarrisonMission,                       //221 SPELL_EFFECT_ADD_GARRISON_MISSION
    &Spell::EffectCreateHeirloomItem,                       //222 SPELL_EFFECT_CREATE_HEIRLOOM_ITEM
    &Spell::EffectChangeItemBonuses,                        //223 SPELL_EFFECT_CHANGE_ITEM_BONUSES
    &Spell::EffectActivateGarrisonBuilding,                 //224 SPELL_EFFECT_ACTIVATE_GARRISON_BUILDING
    &Spell::EffectGrantBattlePetLevel,                      //225 SPELL_EFFECT_GRANT_BATTLEPET_LEVEL
    &Spell::EffectPlayerMoveWaypoints,                      //226 SPELL_EFFECT_PLAYER_MOVE_WAYPOINTS
    &Spell::EffectTeleportUnits,                            //227 SPELL_EFFECT_TELEPORT
    &Spell::EffectNULL,                                     //228 SPELL_EFFECT_228
    &Spell::EffectNULL,                                     //229 SPELL_EFFECT_SET_FOLLOWER_QUALITY
    &Spell::EffectNULL,                                     //230 SPELL_EFFECT_230
    &Spell::EffectIncreaseFollowerExperience,               //231 SPELL_EFFECT_INCREASE_FOLLOWER_EXPERIENCE
    &Spell::EffectRemovePhase,                              //232 SPELL_EFFECT_REMOVE_PHASE
    &Spell::EffectReTrainFollower,                          //233 SPELL_EFFECT_RETRAIN_FOLLOWER
    &Spell::EffectNULL,                                     //234 SPELL_EFFECT_234
    &Spell::EffectNULL,                                     //235 SPELL_EFFECT_RANDOM_REWARD_SKILL_POINTS
    &Spell::EffectGieveExperience,                          //236 SPELL_EFFECT_GIVE_EXPERIENCE
    &Spell::EffectNULL,                                     //237 SPELL_EFFECT_GIVE_RESTED_EXPERIENCE_BONUS
    &Spell::EffectIncreaseSkill,                            //238 SPELL_EFFECT_INCREASE_SKILL
    &Spell::EffectNULL,                                     //239 SPELL_EFFECT_END_GARRISON_BUILDING_CONSTRUCTION
    &Spell::EffectGiveArtifactPower,                        //240 SPELL_EFFECT_GIVE_ARTIFACT_POWER
    &Spell::EffectNULL,                                     //241 SPELL_EFFECT_241
    &Spell::EffectGiveArtifactPowerNoBonus,                 //242 SPELL_EFFECT_GIVE_ARTIFACT_POWER_NO_BONUS
    &Spell::EffectApplyEnchantIllusion,                     //243 SPELL_EFFECT_APPLY_ENCHANT_ILLUSION
    &Spell::EffectNULL,                                     //244 SPELL_EFFECT_LEARN_FOLLOWER_ABILITY
    &Spell::EffectUpgradeHeirloom,                          //245 SPELL_EFFECT_UPGRADE_HEIRLOOM
    &Spell::EffectNULL,                                     //246 SPELL_EFFECT_FINISH_GARRISON_MISSION
    &Spell::EffectAddGarrisonMission,                       //247 SPELL_EFFECT_ADD_GARRISON_MISSION2
    &Spell::EffectNULL,                                     //248 SPELL_EFFECT_FINISH_SHIPMENT
    &Spell::EffectForceEquipItem,                           //249 SPELL_EFFECT_FORCE_EQUIP_ITEM
    &Spell::EffectNULL,                                     //250 SPELL_EFFECT_TAKE_SCREENSHOT
    &Spell::EffectNULL,                                     //251 SPELL_EFFECT_SET_GARRISON_CACHE_SIZE
    &Spell::EffectTeleportUnits,                            //252 SPELL_EFFECT_TELEPORT_L
    &Spell::EffectGiveHonorPoints,                          //253 SPELL_EFFECT_GIVE_HONOR_POINTS
    &Spell::EffectJumpDest,                                 //254 SPELL_EFFECT_SMASH_DEST
    &Spell::EffectCollectItemAppearancesSet,                //255 SPELL_EFFECT_COLLECT_ITEM_APPEARANCES_SET
};

void Spell::EffectNULL(SpellEffIndex /*effIndex*/)
{
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "WORLD: Spell Effect DUMMY");
}

void Spell::EffectUnused(SpellEffIndex /*effIndex*/)
{
    // NOT USED BY ANY SPELL OR USELESS OR IMPLEMENTED IN DIFFERENT WAY IN TRINITY
}

void Spell::EffectResurrectNew(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || unitTarget->isAlive())
        return;

    if (!unitTarget->IsPlayer())
        return;

    if (!unitTarget->IsInWorld())
        return;

    Player* target = unitTarget->ToPlayer();
    if (target->IsRessurectRequested())       // already have one active request
        return;

    ExecuteLogEffectResurrect(effIndex, target);
    target->SetResurrectRequestData(m_caster, damage, m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, 0, m_spellInfo);
    SendResurrectRequest(target);
}

void Spell::EffectInstaKill(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    if (m_spellInfo->Id == 108503)
    {
        if (!unitTarget->GetHealth() || !unitTarget->isAlive())
        {
            unitTarget->ToPet()->Remove();
            return;
        }

        if (Pet* pet = unitTarget->ToPet())
            pet->CastPetAuras(false, m_spellInfo->Id);
    }

    if (unitTarget->HasAura(SPELL_BG_PREPARATION) || unitTarget->HasAura(SPELL_ARENA_PREPARATION))
        return;

    if (unitTarget->IsPlayer())
        if (unitTarget->ToPlayer()->GetCommandStatus(CHEAT_GOD))
            return;

    if (m_caster == unitTarget)                              // prevent interrupt message
        finish();

    WorldPackets::CombatLog::SpellInstakillLog data;
    data.Target = unitTarget->GetGUID();
    data.Caster = m_caster->GetGUID();
    data.SpellID = m_spellInfo->Id;
    m_caster->SendMessageToSet(data.Write(), true);

    m_caster->DealDamage(unitTarget, unitTarget->GetHealth(m_caster), nullptr, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);

    if ((m_caster->IsPlayer() || m_caster->isPet()) && unitTarget->ToCreature() && unitTarget->ToCreature()->isWorldBoss())
        sLog->outWarden("Caster Unit %s (GUID: %u) casts SPELL_EFFECT_INSTAKILL(spellId: %u) to target unit %s (Entry: %u Guid: %u).",
            m_caster->GetName(), m_caster->GetGUIDLow(), m_spellInfo->Id, unitTarget->GetName(), unitTarget->GetEntry(), unitTarget->GetGUIDLow());
}

void Spell::EffectEnvironmentalDMG(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive())
        return;

    uint32 absorb = 0;
    uint32 resist = 0;

    m_caster->CalcAbsorbResist(unitTarget, m_spellInfo->GetSchoolMask(), SPELL_DIRECT_DAMAGE, damage, &absorb, &resist, m_spellInfo);

    m_caster->SendSpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage, m_spellInfo->GetSchoolMask(), absorb, resist, false, 0, m_spellInfo->GetSpellXSpellVisualId(m_caster, unitTarget), false);
    if (unitTarget->IsPlayer())
        unitTarget->ToPlayer()->EnvironmentalDamage(DAMAGE_FIRE, damage);
}

void Spell::EffectSchoolDMG(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectSchoolDMG %i, m_diffMode %i, effIndex %i, spellId %u, damage %i", m_damage, m_diffMode, effIndex, m_spellInfo->Id, damage);

    if (unitTarget && unitTarget->isAlive())
    {
        switch (m_spellInfo->ClassOptions.SpellClassSet)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (m_spellInfo->Id)                     // better way to check unknown
                {
                    case 242556: // Umbral Glaive Storm
                    case 242557: // Shattering Umbral Glaives
                    case 225777: // Fel-Crazed Rage
                        if (Player* plr = m_caster->ToPlayer())
                            if (!plr->IsMeleeDamageDealer() || m_caster->HasAura(137049) || m_caster->HasAura(137050) || m_caster->HasAura(162701))
                                damage *= 0.4f;
                        break;
                    case 257244: // Worldforger's Flame
                    case 257430: // Golganneth's Thunderous Wrath
                        if (Player* plr = m_caster->ToPlayer())
                        {
                            if (Item* itm = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
                            {
                                float coeff = itm->GetTemplate()->GetDelay() / 1000.f;
                                damage *= coeff;
                            }
                        }
                        break;
                    case 133234:
                    case 251691:
                    case 250889:
                    case 251700:
                    case 254651:
                        damage /= 10;
                        break;
                    case 215754: // Magma Spit
                        if (Unit* owner = m_caster->GetAnyOwner())
                            if (AuraEffect const* aurEff = owner->GetAuraEffect(215745, EFFECT_0)) // Spawn of Serpentrix
                                damage = aurEff->GetAmount();
                        break;
                    case 105408: // Burning Blood dmg, Madness of Deathwing, Dragon Soul
                        if (m_triggeredByAuraSpell)
                        {
                            if (Aura* const aur = m_caster->GetAura(m_triggeredByAuraSpell->Id))
                                damage *= aur->GetStackAmount();
                        }
                        break;
                    // Resonating Crystal dmg, Morchok, Dragon Soul
                    case 103545:
                        if (!unitTarget)
                            break;

                        if (unitTarget->HasAura(103534))
                            damage *= 1.5f;
                        else if (unitTarget->HasAura(103536))
                            damage *= 0.7f;
                        else if (unitTarget->HasAura(103541))
                            damage *= 0.3f;

                        unitTarget->RemoveAura(103534);
                        unitTarget->RemoveAura(103536);
                        unitTarget->RemoveAura(103541);
                        break;
                    // Stomp, Morchok, Dragon Soul
                    case 103414:
                    {
                        if (!unitTarget)
                            break;

                        if (Creature* pMorchok = m_caster->ToCreature())
                        {
                            if ((pMorchok->GetEntry() == 57773) || pMorchok->AI()->GetData(3))
                                damage /= 2;

                            if ((unitTarget->GetGUID() == pMorchok->AI()->GetGUID(1)) ||
                                (unitTarget->GetGUID() == pMorchok->AI()->GetGUID(2)))
                                damage *= 2;
                        }
                        break;
                    }
                    // percent from health with min
                    case 25599:                             // Thundercrash
                    {
                        damage = unitTarget->GetHealth(m_caster) / 2;
                        if (damage < 200)
                            damage = 200;
                        break;
                    }
                    // arcane charge. must only affect demons (also undead?)
                    case 45072:
                    {
                        if (unitTarget->GetCreatureType() != CREATURE_TYPE_DEMON
                            && unitTarget->GetCreatureType() != CREATURE_TYPE_UNDEAD)
                            return;
                        break;
                    }
                }
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                switch (m_spellInfo->Id)
                {
                    // Claw, Bite, Smack
                    case 49966:
                    case 16827:
                    case 17253:
                    {
                        if (Unit* hunter = m_caster->GetOwner())
                        {
                            damage += int32(hunter->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.333f);

                            // Deals 100% more damage and costs 100% more Focus when your pet has 50 or more Focus.
                            if (m_caster->GetPower(POWER_FOCUS) >= 50)
                            {
                                damage *= 2;
                                m_caster->ModifyPower(POWER_FOCUS, -25, true);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }

                // Gore
                if (m_spellInfo->GetMisc()->MiscData.IconFileDataID == 1578) // whuch spell?
                {
                    if (m_caster->HasAura(57627))           // Charge 6 sec post-affect
                        damage *= 2;
                }
                break;
            }
            case SPELLFAMILY_MAGE:
            {
                switch (m_spellInfo->Id)
                {
                    case 228354: // Flurry
                    {
                        m_damage = damage;
                        return;
                    }
                    case 211076: // Mark of Aluneth
                    {
                        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(224968))
                            damage += CalculatePct(m_caster->GetMaxPower(POWER_MANA), spellInfo->Effects[EFFECT_0]->BasePoints);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
        }

        if (m_originalCaster)
        {
            std::vector<uint32> ExcludeAuraList;
            uint32 bonus = m_originalCaster->SpellDamageBonusDone(unitTarget, m_spellInfo, static_cast<uint32>(damage), SPELL_DIRECT_DAMAGE, ExcludeAuraList, effIndex);
            bonus *= m_originalCaster->GetProcStatsMultiplier(m_spellInfo->Id);
            damage = bonus + uint32(bonus * variance);
        }

        m_damage += damage;

        uint32 combopoints = GetComboPoints();
        switch (m_spellInfo->Id)
        {
            case 152150: // Hack for Death from Above
            {
                m_damage *= (float(combopoints) / 5.0f);
                combopoints = 0;
                break;
            }
            case 22568:  // Ferocious Bite
            {
                for (auto itr : m_loadedScripts)
                {
                    if (uint32 curDamage = itr->CallSpecialFunction(m_damage))
                        m_damage = curDamage;
                }

                combopoints = 0;
                break;
            }
            case 203286: // Greater Pyroblast (Honor Talent)
            case 203728: // Thorns
                m_damage = unitTarget->CountPctFromMaxHealth(damage);
                break;
        }

        if (combopoints)
        {
            if (m_triggeredByAura)
            {
                if (Aura* aura = m_triggeredByAura->GetBase())
                {
                    if (int32 fakeCombo = aura->GetCustomData())
                    {
                        m_damage *= fakeCombo;
                        aura->SetCustomData(0);
                    }
                }
            }
            else
                m_damage *= GetComboPoints();
        }

        // Meteor like spells (divided damage to targets)
        if (m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_SHARE_DAMAGE)
        {
            uint32 count = 0;
            for (auto& ihit : m_UniqueTargetInfo)
                if (ihit->effectMask & (1 << effIndex))
                    ++count;

            m_damage /= count;                    // divide to all targets
        }

        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectSchoolDMG end %i, m_diffMode %i, effIndex %i, spellId %u, damage %i GetComboPoints %i", m_damage, m_diffMode, effIndex, m_spellInfo->Id, damage, GetComboPoints());

        switch (m_spellInfo->Id)
        {
            case 1822:   // Rake
            case 106830: // Thrash
            {
                if (AuraEffect const* aurEff = m_caster->GetAuraEffect(145152, EFFECT_0)) // Bloodtalons
                    AddPct(m_damage, aurEff->GetAmount());
                break;
            }
            case 192660: // Poison Bomb
            {
                if (AuraEffect* const aurEff0 = m_caster->GetAuraEffect(214569, EFFECT_1)) // Zodyck Family Training Shackles
                    if (unitTarget->GetHealthPct() < aurEff0->GetAmount())
                        if (AuraEffect* const aurEff1 = m_caster->GetAuraEffect(214569, EFFECT_0)) // Zodyck Family Training Shackles
                            AddPct(m_damage, aurEff1->GetAmount());
                break;
            }
            case 47666: // Penance
            {
                if (!m_triggeredByAura)
                {
                    if (AuraEffect const* eff = m_caster->GetAuraEffect(242268, EFFECT_0)) // Item - Priest T20 Discipline 2P Bonus
                        AddPct(m_damage, eff->GetAmount());
                }
                break;
            }
            case 27285: // Seed of Corruption (detonate damage)
            {
                if (m_caster->HasAura(234876))
                {
                    SpellInfo const* deathsEmbrace = sSpellMgr->GetSpellInfo(234876);
                    if (unitTarget->GetHealthPct() <= deathsEmbrace->Effects[EFFECT_1]->BasePoints)
                    {
                        int32 bp = CalculatePct((100 - unitTarget->GetHealthPct()), deathsEmbrace->Effects[EFFECT_0]->BasePoints);
                        AddPct(m_damage, bp);
                    }
                }
                break;
            }
            case 11366: // Pyroblast
            {
                if (GetCastTime())
                {
                    if (AuraEffect* auraEff = m_caster->GetAuraEffect(209455, EFFECT_0)) // Kael'thas's Ultimate Ability
                    {
                        AddPct(m_damage, auraEff->GetAmount());
                        m_caster->RemoveAurasDueToSpell(209455);
                    }
                }
                break;
            }
            case 198455: // Spirit Bomb
            case 198480: // Fire Nova
            case 198483: // Snowstorm
            case 198485: // Thunder Bite
            case 224125: // Fiery Jaws
            case 224126: // Frozen Bite
            {
                if (Unit* owner = m_caster->GetAnyOwner())
                    if (AuraEffect const* aurEff = owner->GetAuraEffect(77223, EFFECT_0)) // Mastery: Enhanced
                        m_damage += CalculatePct(m_damage, aurEff->GetAmount());
                break;
            }
            case 51963:  // Gargoyle Strike
            case 198715: // Val'kyr Strike
            case 218321: // Dragged to Helheim 
            {
                if (Unit* owner = m_caster->GetAnyOwner())
                    if (AuraEffect const* aurEff = owner->GetAuraEffect(77515, EFFECT_0)) // Mastery: Dreadblade
                        m_damage += CalculatePct(m_damage, aurEff->GetAmount());
                break;
            }
        }
    }
}

bool Spell::SpellDummyTriggered(SpellEffIndex effIndex)
{
    if (std::vector<SpellDummyTrigger> const* spellTrigger = sSpellMgr->GetSpellDummyTrigger(m_spellInfo->Id))
    {
        bool check = false;
        Unit* triggerTarget = unitTarget;
        Unit* triggerCaster = m_caster;
        Unit* targetAura = m_caster;
        float basepoints0 = damage;
        uint32 cooldown_spell_id = 0;
        std::list<int32> groupList;
        Unit* explTarget = m_targets.GetUnitTarget();

        for (const auto& itr : *spellTrigger)
        {
            #ifdef WIN32
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectDummy %u, 1<<effIndex %u, itr->effectmask %u, option %u, spell_trigger %i, target %u (%u ==> %u) explTarget %u effectHandleMode %u",
            m_spellInfo->Id, 1 << effIndex, itr.effectmask, itr.option, itr.spell_trigger, itr.target, triggerTarget ? triggerTarget->GetGUIDLow() : 0, triggerCaster ? triggerCaster->GetGUIDLow() : 0, explTarget ? explTarget->GetGUIDLow() : 0, effectHandleMode);
            #endif

            if (!(itr.handlemask & (1 << effectHandleMode)))
                continue;

            if (!(itr.effectmask & (1 << effIndex)))
                continue;

            if (itr.target)
                triggerTarget = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(m_caster, unitTarget,
                                                                                                        itr.target, explTarget);

            if (itr.caster)
                triggerCaster = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(m_caster, unitTarget,
                                                                                                        itr.caster, explTarget);

            if (itr.targetaura)
                targetAura = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(m_caster, unitTarget,
                                                                                                     itr.targetaura, explTarget);

            #ifdef WIN32
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectDummy2: %u, 1<<effIndex %u, itr->effectmask %u, option %u, spell_trigger %i, target %u (%u ==> %u)", m_spellInfo->Id, 1 << effIndex,
                           itr.effectmask, itr.option, itr.spell_trigger, itr.target, triggerTarget ? triggerTarget->GetGUIDLow() : 0, triggerCaster ? triggerCaster->GetGUIDLow() : 0);
            #endif

            cooldown_spell_id = abs(itr.spell_trigger);
            if (triggerCaster)
                if (triggerCaster->HasSpellCooldown(cooldown_spell_id) && itr.option != DUMMY_TRIGGER_COOLDOWN)
                    return true;

            float bp0 = itr.bp0;
            float bp1 = itr.bp1;
            float bp2 = itr.bp2;
            int32 spell_trigger = damage;

            if (itr.spell_trigger != 0)
                spell_trigger = abs(itr.spell_trigger);

            if(triggerCaster && !triggerCaster->IsInWorld())
                return false;
            if(triggerTarget && !triggerTarget->IsInWorld())
                return false;

            if(targetAura)
            {
                if (itr.aura > 0)
                {
                    if (!targetAura->HasAura(abs(itr.aura)))
                    {
                        check = true;
                        continue;
                    }
                }
                else if (itr.aura < 0)
                {
                    if (targetAura->HasAura(abs(itr.aura)))
                    {
                        check = true;
                        continue;
                    }
                }
            }

            if(itr.group != 0 && !groupList.empty())
            {
                bool groupFind = false;
                for (std::list<int32>::const_iterator group_itr = groupList.begin(); group_itr != groupList.end(); ++group_itr)
                    if((*group_itr) == itr.group)
                        groupFind = true;
                if(groupFind)
                    continue;
            }

            switch (itr.option)
            {
                case DUMMY_TRIGGER_BP: //0
                {
                    if(!triggerCaster || !triggerTarget)
                        break;
                    if (itr.spell_trigger < 0)
                        basepoints0 *= -1;

                    triggerCaster->CastCustomSpell(triggerTarget, spell_trigger, &basepoints0, &basepoints0, &basepoints0, true, m_CastItem, nullptr, m_originalCasterGUID);
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_BP_CUSTOM: //1
                {
                    if(!triggerCaster || !triggerTarget)
                        break;
                    triggerCaster->CastCustomSpell(triggerTarget, spell_trigger, &bp0, &bp1, &bp2, true, m_CastItem, nullptr, m_originalCasterGUID);
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_COOLDOWN: //2
                {
                    if(!triggerTarget)
                        break;
                    if (Player* player = triggerTarget->ToPlayer())
                    {
                        uint32 spellid = abs(spell_trigger);
                        if (itr.bp0 == 0.0f)
                            player->RemoveSpellCooldown(spellid, true);
                        else
                        {
                            int32 delay = itr.bp0;
                            if (delay > -1 * IN_MILLISECONDS)
                            {
                                if (roll_chance_i(50))
                                    player->ModifySpellCooldown(spellid, -1 * IN_MILLISECONDS);
                            }
                            else
                                player->ModifySpellCooldown(spellid, delay);
                        }
                    }
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_CHECK_PROCK: //3
                {
                    if(!triggerCaster || !triggerTarget)
                        break;
                    if (triggerCaster->HasAura(itr.aura))
                    {
                        if (spell_trigger > 0)
                            triggerCaster->CastSpell(triggerTarget, spell_trigger, true, m_CastItem);
                        else
                            triggerCaster->RemoveAura(spell_trigger);
                    }

                    check = true;
                }
                break;
                case DUMMY_TRIGGER_DUMMY: //4
                {
                    if(!triggerCaster || !triggerTarget)
                        break;
                    triggerCaster->CastSpell(triggerTarget, spell_trigger, false, m_CastItem);
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_CAST_DEST: //5
                {
                    if(!triggerCaster)
                        break;
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_trigger))
                    {
                        SpellCastTargets targets;
                        targets.SetCaster(m_caster);
                        if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT_TARGET)
                            targets.SetUnitTarget(unitTarget);
                        else
                            targets.SetDst(m_targets);
                        CustomSpellValues values;
                        triggerCaster->CastSpell(targets, spellInfo, &values, TRIGGERED_FULL_MASK, nullptr, nullptr, m_originalCasterGUID);
                    }
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_CAST_OR_REMOVE: // 6
                {
                    m_caster->CastSpell(unitTarget, spell_trigger, true, m_CastItem);
                    check = true;
                    break;
                }
                case DUMMY_TRIGGER_DAM_MAXHEALTH: //7
                {
                    if(!triggerCaster)
                        break;

                    int32 percent = basepoints0;
                    if (bp0)
                        percent += bp0;
                    if (bp1)
                        percent /= bp1;
                    if (bp2)
                        percent *= bp2;

                    basepoints0 = CalculatePct(triggerTarget->GetMaxHealth(triggerCaster), percent);

                    triggerCaster->CastCustomSpell(triggerTarget, spell_trigger, &basepoints0, &bp1, &bp2, true, m_CastItem, nullptr, m_originalCasterGUID);
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_COPY_AURA: // 8
                {
                    if (explTarget)
                    {
                        if (Aura* aura = explTarget->GetAura(spell_trigger))
                        {
                            Aura* copyAura = m_caster->AddAura(spell_trigger, triggerTarget);
                            if(!copyAura)
                                break;
                            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                            {
                                AuraEffect* aurEff = aura->GetEffect(i);
                                AuraEffect* copyAurEff = copyAura->GetEffect(i);
                                if (aurEff && copyAurEff)
                                    copyAurEff->SetAmount(aurEff->GetAmount());
                            }
                            copyAura->SetStackAmount(aura->GetStackAmount());
                            copyAura->SetMaxDuration(aura->GetMaxDuration());
                            copyAura->SetDuration(aura->GetDuration());
                            copyAura->SetCharges(aura->GetCharges());
                        }
                    }
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_ADD_POWER_COST: //9
                {
                    if(!triggerCaster)
                        break;

                    int32 rBp1 = RoundingFloatValue(bp1);

                    if (bp0 != 0)
                        basepoints0 = bp0;

                    uint32 power = triggerCaster->GetMaxPower(static_cast<Powers>(rBp1));
                    bp0 = CalculatePct(power, basepoints0);
                    triggerCaster->SetPower(static_cast<Powers>(rBp1), power - RoundingFloatValue(bp0));
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_CAST_DEST2: //10
                {
                    if(!triggerCaster)
                        break;
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_trigger))
                    {
                        SpellCastTargets targets;
                        targets.SetCaster(triggerCaster);
                        if (m_targets.HasDst())
                            targets.SetDst(m_targets);
                        else
                            targets.SetUnitTarget(triggerTarget);
                        CustomSpellValues values;
                        triggerCaster->CastSpell(targets, spellInfo, &values, TRIGGERED_FULL_MASK, nullptr, nullptr, m_originalCasterGUID);
                    }
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_CAST_IGNORE_GCD: // 11
                {
                    if(!triggerCaster || !triggerTarget)
                        break;
                    triggerCaster->CastSpell(triggerTarget, spell_trigger, true, m_CastItem);
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_MOVE_AURA: // 12
                {
                    if (m_triggeredByAura)
                    {
                        if (Aura* aura = m_triggeredByAura->GetBase())
                        {
                            if (triggerTarget->HasAura(spell_trigger)) // Don`t copy to self
                            {
                                aura->SetCustomData(1);
                                break;
                            }

                            Aura* copyAura = m_caster->AddAura(spell_trigger, triggerTarget);
                            if(!copyAura)
                                break;
                            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                            {
                                AuraEffect* aurEff = aura->GetEffect(i);
                                AuraEffect* copyAurEff = copyAura->GetEffect(i);
                                if (aurEff && copyAurEff)
                                    copyAurEff->SetAmount(aurEff->GetAmount());
                            }
                            copyAura->SetStackAmount(aura->GetStackAmount());
                            copyAura->SetMaxDuration(aura->GetMaxDuration());
                            copyAura->SetDuration(aura->GetDuration());
                            copyAura->SetCharges(aura->GetCharges());
                        }
                    }
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_SEND_COOLDOWN: // 13
                {
                    if(!triggerCaster)
                        break;
                    triggerCaster->SendSpellCooldown(m_spellInfo->Id, spell_trigger);
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_ARCHAEOLOGY_LOOT: // 14
                {
                    if(!triggerCaster)
                        break;
                    Player* player = triggerCaster->ToPlayer();
                    if(!player)
                        break;
                    if (player->SolveResearchProject(m_spellInfo->Id, m_targets))
                    {
                        player->AutoStoreLoot(m_spellInfo->Id, LootTemplates_Spell);
                        player->UpdateCraftSkill(m_spellInfo->Id);
                    }
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_TALK_BROADCAST: // 15
                {
                    if(!triggerCaster)
                        break;
                    if (spell_trigger)
                        basepoints0 = spell_trigger;

                    triggerCaster->MonsterYell(basepoints0, 0, triggerTarget ? triggerTarget->GetGUID() : ObjectGuid::Empty);
                    check = true;
                }
                break;
                case DUMMY_TRIGGER_CAST_DELAY: // 16
                {
                    if(!triggerCaster || !triggerTarget)
                        break;
                    uint32 delay = bp0;
                    triggerCaster->CastSpellDelay(triggerTarget, spell_trigger, true, delay, m_CastItem);
                    check = true;
                }
                break;
            }
            if(itr.group != 0 && check)
                groupList.push_back(itr.group);
        }
        if (check)
            return true;
    }
    return false;
}

void Spell::EffectDummy(SpellEffIndex effIndex)
{
    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectDummy end %i, m_diffMode %i, effIndex %i, spellId %u, damage %i, effectHandleMode %u, GetExplicitTargetMask %u", m_damage, m_diffMode, effIndex, m_spellInfo->Id, damage, effectHandleMode, m_spellInfo->GetExplicitTargetMask());
    #endif

    uint32 spell_id = 0;
    int32 bp = 0;
    bool triggered = true;

    if (SpellDummyTriggered(effIndex))
        return;

    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget && !gameObjTarget && !itemTarget)
        return;

    // Fishing dummy
    if (m_spellInfo->Id == 131474)
    {
        if (Item* fishPole = m_caster->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
        {
            if (fishPole->GetTemplate()->GetClass() == ITEM_CLASS_WEAPON && fishPole->GetTemplate()->GetSubClass() == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
                m_caster->CastSpell(m_caster, 131490, false);
            else
                m_caster->CastSpell(m_caster, 131476, false);
        }
    }

    // selection by spell family
    switch (m_spellInfo->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_PRIEST:
        {
            switch (m_spellInfo->Id)
            {
                case 196529: // Renew the Faith
                {
                    if (unitTarget)
                    {
                        if (Aura* _aura = unitTarget->GetAura(41635, m_caster->GetGUID()))
                        {
                            m_caster->CastSpell(unitTarget, 33110, true);

                            if (AuraEffect const* eff = _aura->GetEffect(EFFECT_0))
                                unitTarget->CastSpell(unitTarget, 155793, true, nullptr, eff);

                            _aura->Remove();
                        }
                    }
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_GENERIC:
        {
            switch (m_spellInfo->Id)
            {
                case 45129: // Throw Paper Zeppelin
                {
                    if (!unitTarget->IsPlayer())
                        return;

                    m_caster->CastSpell(unitTarget, damage, true);
                    return;
                }
                case 207510: //Flame of Woe
                    m_caster->CastSpell(m_caster, 207509, true);
                    break;
                case 51858:     //Q: Death Comes From On High ID 12641
                    if (unitTarget &&
                        (unitTarget->GetEntry() == 28543 ||
                        unitTarget->GetEntry() == 28542 ||
                        unitTarget->GetEntry() == 28525 ||
                        unitTarget->GetEntry() == 28544))
                        if (Player *plr = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself())
                            plr->KilledMonsterCredit(unitTarget->GetEntry(), unitTarget->GetGUID());
                    break;
                case 78640: // Deepstone Oil
                {
                    if (m_caster->HasAuraType(SPELL_AURA_MOUNTED))
                        m_caster->CastSpell(m_caster, 78639, true);
                    else
                        m_caster->CastSpell(m_caster, 78627, true);
                    break;
                }
                case 145111: // Moonfang's Curse
                {
                    if (roll_chance_i(30))
                        m_caster->CastSpell(m_caster, 145112, true); // Free Your Mind
                    break;
                }
                case 92679: // Flask of Battle
                {
                    uint32 spellid = 0;
                    uint32 Agi = m_caster->GetStat(STAT_AGILITY);
                    uint32 Str = m_caster->GetStat(STAT_STRENGTH);
                    uint32 Int = m_caster->GetStat(STAT_INTELLECT);

                    if (Player* pPlayer = m_caster->ToPlayer())
                    {
                        if (pPlayer->isInTankSpec())
                            pPlayer->CastSpell(pPlayer, 92729, true);
                        else
                        {
                            if (Agi > Str && Agi > Int) spellid = 92725;
                            else if (Str > Agi && Str > Int) spellid = 92731;
                            else if (Int > Str && Int > Agi) spellid = 92730;
                        }
                        pPlayer->CastCustomSpell(pPlayer, spellid, &damage, nullptr, nullptr, true);
                    }
                    break;
                }
                case 126734: // Synapse Springs (Mod II)
                {
                    uint32 spellid = 0;
                    uint32 Agi = m_caster->GetStat(STAT_AGILITY);
                    uint32 Str = m_caster->GetStat(STAT_STRENGTH);
                    uint32 Int = m_caster->GetStat(STAT_INTELLECT);

                    if (Agi > Str && Agi > Int) spellid = 96228;
                    else if (Str > Agi && Str > Int) spellid = 96229;
                    else if (Int > Str && Int > Agi) spellid = 96230;

                    m_caster->CastCustomSpell(m_caster, spellid, &damage, nullptr, nullptr, true);
                    break;
                }
                case 105617: // Alchemist's Flask
                {
                    if (!m_caster->IsPlayer())
                        return;

                    uint32 Agi = m_caster->GetStat(STAT_AGILITY);
                    uint32 Int = m_caster->GetStat(STAT_INTELLECT);
                    uint32 Str = m_caster->GetStat(STAT_STRENGTH);

                    if (Agi > Int && Agi > Str)
                        m_caster->CastCustomSpell(m_caster, 79639, &damage, nullptr, nullptr, true, m_CastItem);
                    else if (Int > Agi && Int > Str)
                        m_caster->CastCustomSpell(m_caster, 79640, &damage, nullptr, nullptr, true, m_CastItem);
                    else if (Str > Agi && Str > Int)
                        m_caster->CastCustomSpell(m_caster, 79638, &damage, nullptr, nullptr, true, m_CastItem);
                    break;
                }
                case 6203:  // Soulstone
                {
                    if (unitTarget->isAlive())
                        unitTarget->CastSpell(unitTarget, 3026, true); // Self resurrect
                    else
                        m_caster->CastSpell(unitTarget, 95750, true);
                    break;
                }
                case 45206: // Copy Off-hand Weapon
                case 69892:
                {
                    unitTarget->CastSpell(m_caster, damage, true);
                    if (unitTarget->IsPlayer())
                        break;

                    if (m_caster->IsPlayer())
                    {
                        if (Item* offItem = m_caster->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                            unitTarget->SetVirtualItem(1, offItem->GetEntry());
                    }
                    else
                        unitTarget->SetVirtualItem(1, m_caster->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS + 1));
                    break;
                }
                case 41055: // Copy Mainhand Weapon
                case 63416:
                case 69891:
                {
                    if (unitTarget->IsPlayer())
                        break;

                    if (m_caster->IsPlayer())
                    {
                        if (Item* mainItem = m_caster->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
                            unitTarget->SetVirtualItem(0, mainItem->GetEntry());
                    }
                    else
                        unitTarget->SetVirtualItem(0, m_caster->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS));
                    break;
                }
                case 97383: // Capturing
                {
                    m_caster->CastSpell(unitTarget, 97388);
                    break;
                }
                case 97388: // Capturing
                {
                    if (Player* player = m_caster->ToPlayer())
                    {
                        Battleground* bg = player->GetBattleground();
                        if (bg && (bg->GetTypeID(true) == MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeepwindGorge || bg->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeepwindGorge))
                            static_cast<BattlegroundDeepwindGorge*>(bg)->HandlePointCapturing(player, unitTarget->ToCreature());
                    }
                    break;
                }
                case 97372:
                    if (Player* player = m_caster->ToPlayer())
                        if (Battleground* bg = player->GetBattleground())
                            bg->EventPlayerClickedOnFlag(player, unitTarget);
                    break;
                case 126755:    // Wormhole: Pandaria
                {
                    if (!unitTarget)
                        return;

                    Player* player = unitTarget->ToPlayer();
                    if (!player)
                        return;

                    switch (urand(0, 8))
                    {
                        case 0:     // Dread Wastes, Zan'vess
                            player->TeleportTo(870, -1513.342896f, 4575.043945f, 367.025208f, 1.921501f);
                            break;
                        case 1:     // Dread Wastes, Rikkitun Village
                            player->TeleportTo(870, 635.419983f, 4150.577637f, 210.107971f, 3.444401f);
                            break;
                        case 2:     // The Jade Forest, Emperor's Omen
                            player->TeleportTo(870, 2387.586914f, -2107.073486f, 230.486252f, 5.69462f);
                            break;
                        case 3:     // Krasarang Wilds, Narsong Spires
                            player->TeleportTo(870, -1493.026123f, -380.700806f, 119.941902f, 1.411952f);
                            break;
                        case 4:     // Kun-Lai Summit, Firebough Nook
                            player->TeleportTo(870, 2087.523682f, 2113.091309f, 443.190643f, 5.433147f);
                            break;
                        case 5:     // Kun-Lai Summit, Isle of Reckoning
                            player->TeleportTo(870, 5052.794434f, 193.100662f, 2.693901f, 4.899077f);
                            break;
                        case 6:     // Townlong Steppes, Sra'vess
                            player->TeleportTo(870, 3074.227539f, 6119.812500f, 54.291294f, 0.226609f);
                            break;
                        case 7:     // Vale of Eternal Blossoms, Whitepetal Lake
                            player->TeleportTo(870, 1208.797974f, 1376.896851f, 363.663788f, 5.034821f);
                            break;
                        case 8:     // Valley of the Four Winds, The Heartland
                            player->TeleportTo(870, 125.107628f, 1024.812378f, 194.217041f, 3.868679f);
                            break;
                        default:
                            break;
                    }
                    return;
                }
                case 148565:    // Spectral Grog
                {
                    if (!unitTarget)
                        return;

                    Player* player = unitTarget->ToPlayer();
                    if (!player)
                        return;

                    player->CastSpell(player, player->getGender() == GENDER_MALE ? 148564 : 148563, true);
                    return;
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
            switch (m_spellInfo->Id)
            {
                case 20473: // Holy Shock
                {
                    uint32 spellid = unitTarget->IsFriendlyTo(m_caster) ? 25914 : 25912;
                    m_caster->CastSpell(unitTarget, spellid, true);
                    break;
                }
                default:
                    break;
            }
            break;
        case SPELLFAMILY_WARRIOR:
        {
            switch (m_spellInfo->Id)
            {
                case 100: // Charge
                {
                    m_caster->EnergizeBySpell(m_caster, m_spellInfo->Id, damage, POWER_RAGE);
                    break;
                }
                default:
                    break;
            }
            break;
        }
    }

    switch (m_spellInfo->Id)
    {
        case 115921: // Legacy of the Emperor
        {
            spell_id = m_spellInfo->Effects[effIndex]->BasePoints;

            if (unitTarget->IsInPartyWith(m_caster))
                spell_id = 117666;
            break;
        }
        // Lunar Invitation
        case 26373:
            if(m_caster->ToPlayer())
                m_caster->ToPlayer()->TeleportTo(1, 7581.144531f, -2211.47900f, 473.639771f, 0, 0);
        break;
        case 120165: //Conflagrate
        {
            UnitList friends;
            Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(m_caster, m_caster, 5.0f);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(m_caster, friends, u_check);
            Trinity::VisitNearbyObject(m_caster, 5.0f, searcher);

            for (UnitList::const_iterator unit = friends.begin(); unit != friends.end(); ++unit)
            {
                if (m_caster->GetGUID() == (*unit)->GetGUID())
                    continue;
                GetOriginalCaster()->CastSpell(*unit, 120160, true);
                GetOriginalCaster()->CastSpell(*unit, 120201, true);
            }

            break;
        }
        case 107045: //Jade Fire
            m_caster->CastSpell(unitTarget, 107098, false);
            break;
        case 106299: //Summon Living Air
        {
            TempSummon* enne = m_caster->SummonCreature(54631, m_caster->GetPositionX() + rand() % 5, m_caster->GetPositionY() + 2 + rand() % 5, m_caster->GetPositionZ() + 1, 3.3f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
            enne->AddThreat(m_caster, 2000.f);
            break;
        }
        case 120202: // Gate of the Setting Sun | Boss 3 | Bombard
            spell_id = GetSpellInfo()->Effects[0]->BasePoints;
            break;
    }

    //spells triggered by dummy effect should not miss
    if (spell_id)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);

        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "EffectDummy of spell %u: triggering unknown spell id %i\n", m_spellInfo->Id, spell_id);
            return;
        }

        TriggerCastData triggerData;
        triggerData.triggerFlags = triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE;
        triggerData.spellGuid = (m_castGuid[1].GetSubType() == SPELL_CAST_TYPE_NORMAL) ? m_castGuid[1] : m_castGuid[0];
        triggerData.originalCaster = m_originalCasterGUID;
        triggerData.skipCheck = true;
        triggerData.casttime = m_casttime;
        triggerData.powerCost = m_powerCost;

        SpellCastTargets targets;
        targets.SetCaster(m_caster);
        targets.SetUnitTarget(unitTarget);
        auto spell = new Spell(m_caster, spellInfo, triggerData);
        if (bp)
            spell->SetSpellValue(SPELLVALUE_BASE_POINT0, bp);
        spell->prepare(&targets);
    }

    // normal DB scripted effect
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell ScriptStart spellid %u in EffectDummy(%u) caster %s target %s", m_spellInfo->Id, effIndex, m_caster ? m_caster->ToString().c_str() : "", unitTarget ? unitTarget->ToString().c_str() : "");
    m_caster->GetMap()->ScriptsStart(sSpellScripts, uint32(m_spellInfo->Id | (effIndex << 24)), m_caster, unitTarget);

    // Script based implementation. Must be used only for not good for implementation in core spell effects
    // So called only for not proccessed cases
    if (gameObjTarget)
        sScriptMgr->OnDummyEffect(m_caster, m_spellInfo->Id, effIndex, gameObjTarget);
    else if (unitTarget && unitTarget->IsCreature())
        sScriptMgr->OnDummyEffect(m_caster, m_spellInfo->Id, effIndex, unitTarget->ToCreature());
    else if (itemTarget)
        sScriptMgr->OnDummyEffect(m_caster, m_spellInfo->Id, effIndex, itemTarget);
}

void Spell::EffectTriggerSpell(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET
        && effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH)
        return;

    uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;

    // todo: move those to spell scripts
    if (effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
    {
        // special cases
        switch (triggered_spell_id)
        {
            case 153795: //Skyreach: Rukhran - Pierce Armor
            {
                if (unitTarget->HasAura(77535) || unitTarget->HasAura(112048) || unitTarget->HasAura(115308)
                || unitTarget->HasAura(132402) || unitTarget->HasAura(132403))
                {
                    return;
                }
                break;
            }
            // Vanish (not exist)
            case 18461:
            {
                unitTarget->RemoveMovementImpairingEffects();
                unitTarget->CastSpell(unitTarget, 11327, true);
                unitTarget->CombatStop();
                return;
            }
            // Brittle Armor - (need add max stack of 24575 Brittle Armor)
            case 29284:
            {
                // Brittle Armor
                SpellInfo const* spell = sSpellMgr->GetSpellInfo(24575);
                if (!spell)
                    return;

                for (uint32 j = 0; j < spell->GetAuraOptions(m_diffMode)->CumulativeAura; ++j)
                    m_caster->CastSpell(unitTarget, spell->Id, true);
                return;
            }
        }
    }

    switch (triggered_spell_id)
    {
        case 158950: //Level of Craven
        {
            if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH)
                return;

            if (AuraEffect* auraEff = m_caster->GetAuraEffect(triggered_spell_id, EFFECT_0))
            {
                float bp0 = 60.f;
                switch (RoundingFloatValue(auraEff->GetAmount()))
                {
                    case 1: bp0 = 5.f; break;
                    case 5: bp0 = 20.f; break;
                }
                m_caster->RemoveAurasDueToSpell(triggered_spell_id);
                m_caster->CastCustomSpell(m_caster, triggered_spell_id, &bp0, nullptr, nullptr, true);
                return;
            }
            m_caster->CastSpell(m_caster, triggered_spell_id, true);
            return;
        }
        case 73603:  //Kaja'Cola Gives You IDEAS! (TM): Summon Assistant Greely
        case 73607:
        case 204448: //Scorpyron: Exoskeletal Vulnerability
            return;
        case 214606: //Void Empowerment
        {
            if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH)
                return;

            m_caster->AddDelayedEvent(10, [this, triggered_spell_id]() -> void
            {
                if (m_caster)
                    m_caster->CastSpell(m_caster, triggered_spell_id, true);
            });
            return;
        }
        default:
            break;
    }

    // normal case
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(triggered_spell_id);
    if (!spellInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectTriggerSpell spell %u tried to trigger unknown spell %u", m_spellInfo->Id, triggered_spell_id);
        return;
    }

    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectTriggerSpell spell %u tried to trigger spell %u effectHandleMode %u Need %u TargetMask %u", m_spellInfo->Id, triggered_spell_id, effectHandleMode, spellInfo->NeedsToBeTriggeredByCaster(), spellInfo->GetExplicitTargetMask());

    SpellCastTargets targets;
    ObjectGuid TargetGUID = ObjectGuid::Empty;
    targets.SetCaster(m_caster);
    if (effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
    {
        if (!spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, m_caster->GetMap()->GetDifficultyID()))
            return;
        targets.SetUnitTarget(unitTarget);
        if (unitTarget)
            TargetGUID = unitTarget->GetGUID();
    }
    else //if (effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH)
    {
        if (spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, m_caster->GetMap()->GetDifficultyID()) && (m_spellInfo->GetEffect(effIndex, m_diffMode)->GetProvidedTargetMask() & TARGET_FLAG_UNIT_MASK))
            return;

        if (spellInfo->GetExplicitTargetMask() & TARGET_FLAG_DEST_LOCATION)
        {
            if (m_targets.HasDst())
                targets.SetDst(m_targets);
            else
                targets.SetDst(*m_caster);
        }

        if (Unit* target = m_targets.GetUnitTarget())
        {
            targets.SetUnitTarget(target);
            TargetGUID = target->GetGUID();
        }
        else
        {
            targets.SetUnitTarget(m_caster);
            TargetGUID = m_caster->GetGUID();
        }
    }

    CustomSpellValues values;
    // set basepoints for trigger with value effect
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE)
    {
        // maybe need to set value only when basepoints == 0?
        values.AddSpellMod(SPELLVALUE_BASE_POINT0, damage);
        values.AddSpellMod(SPELLVALUE_BASE_POINT1, damage);
        values.AddSpellMod(SPELLVALUE_BASE_POINT2, damage);
    }

    // Remove spell cooldown (not category) if spell triggering spell with cooldown and same category
    if (m_caster->IsPlayer() && m_spellInfo->Cooldowns.CategoryRecoveryTime && m_spellInfo->Categories.Category == spellInfo->Categories.Category)
        m_caster->ToPlayer()->RemoveSpellCooldown(spellInfo->Id);
    
    TriggerCastFlags mask = TRIGGERED_FULL_MASK;

    // Hack. Lana'thel Vampiric Bite
    switch (m_spellInfo->Id)
    {
        case 71726:
        case 70946:
            unitTarget->CastSpell(unitTarget, 70867, false);
            return;
        case 201006: //Odyn: Radiant Tempest
            return;
        case 161121:
        case 167819:
        case 167935:
        case 177380:
        case 226980:
            mask = TRIGGERED_NONE;
            break;
        default:
            break;
    }

    if (spellInfo->CalcCastTime() != 0)
        mask = TRIGGERED_NONE;

    ObjectGuid spellGuid = (m_castGuid[1].GetSubType() == SPELL_CAST_TYPE_NORMAL) ? m_castGuid[1] : m_castGuid[0];
    ObjectGuid originalCaster = m_originalCasterGUID;
    uint32 casttime = m_casttime;
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_TRIGGER_SPELL_4)
        casttime = spellInfo->CalcCastTime();
    uint32 delay = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue ? m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue : 10;
    SpellPowerCost _cost = m_powerCost;
    ObjectGuid itemGUID = m_CastItem ? m_CastItem->GetGUID() : ObjectGuid::Empty;

    Unit* caster = m_caster;

    m_caster->AddDelayedEvent(delay, [caster, spellInfo, values, spellGuid, targets, originalCaster, casttime, TargetGUID, _cost, itemGUID, mask]() -> void
    {
        SpellCastTargets _targets;
        _targets.SetCaster(caster);
        if (targets.HasDst())
            _targets.SetDst(targets);

        TriggerCastData triggerData;
        triggerData.triggerFlags = mask;
        triggerData.spellGuid = spellGuid;
        triggerData.originalCaster = originalCaster;
        triggerData.casttime = casttime;
        triggerData.SubType = SPELL_CAST_TYPE_MISSILE;
        triggerData.powerCost = _cost;
        triggerData.castItem = caster->ToPlayer() ? caster->ToPlayer()->GetItemByGuid(itemGUID) : nullptr;

        if (TargetGUID)
        {
            if (Unit* target = ObjectAccessor::GetUnit(*caster, TargetGUID))
            {
                _targets.SetUnitTarget(target);
                caster->CastSpell(_targets, spellInfo, &values, triggerData);
            }
        }
        else
            caster->CastSpell(_targets, spellInfo, &values, triggerData);
    });
}

void Spell::EffectTriggerMissileSpell(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET
        && effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;

    switch (m_spellInfo->Id)
    {
        case 228596: // Flurry
        {
            if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT_TARGET)
            {
                m_caster->CastCustomSpell(unitTarget, triggered_spell_id, nullptr, &damage, nullptr, true);

                if (AuraEffect const* aurEff = m_caster->GetAuraEffect(231584, EFFECT_0))
                {
                    if (aurEff->GetAmount())
                        m_caster->CastSpell(unitTarget, 228358, true);
                }
                return;
            }
            break;
        }
        case 146364: // OO: Throw Bomb
        {
            if (Aura* aura = m_caster->GetAura(145987))
                aura->ModStackAmount(-1);

            triggered_spell_id = 146365;
            break;
        }
        default:
            break;
    }

    // normal case
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(triggered_spell_id);
    if (!spellInfo)
    {
        SpellDummyTriggered(effIndex);
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectTriggerMissileSpell spell %u tried to trigger unknown spell %u effectHandleMode %u", m_spellInfo->Id, triggered_spell_id, effectHandleMode);
        return;
    }

    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectTriggerMissileSpell spell %u trigger %u effectHandleMode %i unitTarget %i TargetMask %i TargetCount %i",
    //m_spellInfo->Id, triggered_spell_id, effectHandleMode, unitTarget ? unitTarget->GetGUIDLow() : 0, m_spellInfo->GetEffect(effIndex, m_diffMode)->GetProvidedTargetMask(), GetTargetCount());

    SpellCastTargets targets;
    targets.SetCaster(m_caster);
    if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT_TARGET)
    {
        if (!spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, m_caster->GetMap()->GetDifficultyID()))
            return;
        targets.SetUnitTarget(unitTarget);
    }
    else //if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT)
    {
        if (spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo, m_caster->GetMap()->GetDifficultyID()))
            return;

        if (spellInfo->GetExplicitTargetMask() & TARGET_FLAG_DEST_LOCATION)
            targets.SetDst(m_targets);
        else
            targets.SetUnitTarget(m_caster);
    }

    CustomSpellValues values;
    // set basepoints for trigger with value effect
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE)
    {
        // maybe need to set value only when basepoints == 0?
        values.AddSpellMod(SPELLVALUE_BASE_POINT0, damage);
        values.AddSpellMod(SPELLVALUE_BASE_POINT1, damage);
        values.AddSpellMod(SPELLVALUE_BASE_POINT2, damage);
    }

    // Remove spell cooldown (not category) if spell triggering spell with cooldown and same category
    if (m_caster->IsPlayer() && m_spellInfo->Cooldowns.CategoryRecoveryTime && m_spellInfo->Categories.Category == spellInfo->Categories.Category)
        m_caster->ToPlayer()->RemoveSpellCooldown(spellInfo->Id);

    TriggerCastData triggerData;
    triggerData.triggerFlags = TRIGGERED_FULL_MASK;
    triggerData.castItem = m_CastItem;
    triggerData.spellGuid = (m_castGuid[1].GetSubType() == SPELL_CAST_TYPE_NORMAL) ? m_castGuid[1] : m_castGuid[0];
    triggerData.originalCaster = m_originalCasterGUID;
    triggerData.casttime = m_casttime;
    triggerData.SubType = SPELL_CAST_TYPE_MISSILE;
    triggerData.powerCost = m_powerCost;

    // original caster guid only for GO cast
    m_caster->CastSpell(targets, spellInfo, &values, triggerData);
}

void Spell::EffectForceCast(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;

    // normal case
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(triggered_spell_id);

    if (!spellInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::EffectForceCast of spell %u: triggering unknown spell id %i", m_spellInfo->Id, triggered_spell_id);
        return;
    }

    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_FORCE_CAST && damage)
    {
        switch (m_spellInfo->Id)
        {
            case 52588: // Skeletal Gryphon Escape
            case 48598: // Ride Flamebringer Cue
                unitTarget->RemoveAura(damage);
                break;
            case 52463: // Hide In Mine Car
            case 52349: // Overtake
                unitTarget->CastCustomSpell(unitTarget, spellInfo->Id, &damage, nullptr, nullptr, true, nullptr, nullptr, m_originalCasterGUID);
                return;
        }
    }

    CustomSpellValues values;
    // set basepoints for trigger with value effect
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_FORCE_CAST_WITH_VALUE)
    {
        // maybe need to set value only when basepoints == 0?
        values.AddSpellMod(SPELLVALUE_BASE_POINT0, damage);
        values.AddSpellMod(SPELLVALUE_BASE_POINT1, damage);
        values.AddSpellMod(SPELLVALUE_BASE_POINT2, damage);
    }

    SpellCastTargets targets;
    targets.SetCaster(m_caster);
    targets.SetUnitTarget(m_caster);

    unitTarget->CastSpell(targets, spellInfo, &values, TRIGGERED_FULL_MASK);
}

void Spell::EffectTriggerRitualOfSummoning(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(triggered_spell_id);

    if (!spellInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "EffectTriggerRitualOfSummoning of spell %u: triggering unknown spell id %i", m_spellInfo->Id, triggered_spell_id);
        return;
    }

    finish();

    m_caster->CastSpell(static_cast<Unit*>(nullptr), spellInfo, false);
}

void Spell::EffectJump(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (m_caster->isInFlight())
        return;

    if (!unitTarget || m_caster == unitTarget)
        return;

    //Perfome trigger spell at jumping.
    uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;
    DelayCastEvent* delayCast = GetSpellTriggerDelay(effIndex, triggered_spell_id);

    //Fix creature to move back old pos
    if (m_caster->ToCreature())
        m_caster->GetMotionMaster()->MoveIdle();

    Position pos;
    float contact = CONTACT_DISTANCE + unitTarget->GetObjectSize() + m_caster->GetObjectSize();
    if (canHitTargetInLOS && unitTarget->ToCreature() && contact < 200.0f)
        unitTarget->GetNearPoint2D(pos, contact, unitTarget->GetAngle(m_caster));
    else
        unitTarget->GetFirstCollisionPosition(pos, contact, unitTarget->GetAngle(m_caster));
    //float x, y, z;
    //unitTarget->GetContactPoint(m_caster, x, y, z, CONTACT_DISTANCE);

    float speedXY, speedZ;
    float distance = m_caster->GetExactDist(&pos);
    CalculateJumpSpeeds(effIndex, distance, speedXY, speedZ);

    if (m_spellInfo->Id == 196052)
    {
        pos.m_positionX = unitTarget->GetPositionX();
        pos.m_positionY = unitTarget->GetPositionY();
        pos.m_positionZ = unitTarget->GetPositionZ();

        speedXY = 20.f;
        speedZ = 5.f;
    }

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectJump start xyz %f %f %f caster %u target %u damage %i distance %f targetSize %f casterSize %f",
    // pos.m_positionX, pos.m_positionY, pos.m_positionZ, m_caster->GetGUIDLow(), unitTarget->GetGUIDLow(), damage, distance, unitTarget->GetObjectSize(), m_caster->GetObjectSize());

    m_caster->GetMotionMaster()->MoveJump(pos.m_positionX, pos.m_positionY, pos.m_positionZ, speedXY, speedZ, m_spellInfo->Id, 0.0f, delayCast, unitTarget);
}

void Spell::EffectJumpDest(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH)
        return;

    if (m_caster->isInFlight())
        return;

    if (!m_targets.HasDst())
        return;

    //Perfome trigger spell at jumping.
    uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;
    DelayCastEvent* delayCast = GetSpellTriggerDelay(effIndex, triggered_spell_id);

    // Init dest coordinates
    float x, y, z, o = 0.0f;
    destTarget->GetPosition(x, y, z);

    switch (m_spellInfo->Id)
    {
        case 192085: // Jump to Skyhold
            z += 20.0f;
            break;
        case 197619: // Sundering
            o = m_caster->GetOrientation();
            break;
    }

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectJumpDest spell %u triggered_spell_id %u x %f y %f z %f o %f", m_spellInfo->Id, triggered_spell_id, x, y, z, o);

    if (m_spellInfo->Effects[effIndex]->TargetA.GetTarget() == TARGET_DEST_TARGET_BACK)
    {
        Unit* pTarget = nullptr;
        if (m_targets.GetUnitTarget() && m_targets.GetUnitTarget() != m_caster)
            pTarget = m_targets.GetUnitTarget();
        else if (m_caster->getVictim())
            pTarget = m_caster->getVictim();
        else if (m_caster->IsPlayer())
            pTarget = ObjectAccessor::GetUnit(*m_caster, m_caster->ToPlayer()->GetSelection());

        o = pTarget ? Position(x, y).GetRelativeAngle(pTarget) : m_caster->GetOrientation();
    }

    if (Player* plr = m_caster->ToPlayer())
        plr->SetFallInformation(0, z);


    float speedXY, speedZ;
    float distance = m_caster->GetExactDist(x, y, z);
    CalculateJumpSpeeds(effIndex, distance, speedXY, speedZ);
    m_caster->SetSplineTimer(0);

    if (Player* plr = m_caster->ToPlayer())
        plr->SetKnockBackTime(0);


    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectJumpDest start xyz %f %f %f o %f distance %f distance2d %f", x, y, z, o, distance, distance2d);

    // Death Grip and Wild Charge (no form)
    m_caster->GetMotionMaster()->MoveJump(x, y, z, speedXY, speedZ, m_spellInfo->Id, o, delayCast);
}

void Spell::CalculateJumpSpeeds(uint8 i, float dist, float & speedXY, float & speedZ)
{
    if (m_spellInfo->GetEffect(i, m_diffMode)->MiscValue)
        speedZ = float(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue) / 10;
    else if (m_spellInfo->GetEffect(i, m_diffMode)->MiscValueB)
        speedZ = float(m_spellInfo->GetEffect(i, m_diffMode)->MiscValueB) / 10;
    else
        speedZ = 10.0f;

    speedXY = dist * 10.0f / speedZ;

    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "CalculateJumpSpeeds speedZ %f dist %f speedXY %f", speedZ, dist, speedXY);

    switch (m_spellInfo->Id)
    {
        case 49376:  // Wild Charge
        case 102401: // Wild Charge
        case 49575:  // Death Grip
        case 146599: // Gorefiend's Grasp
        case 208674: // Sigil of Chains
        {
            speedXY = 45.f;
            break;
        }
        case 6544:   // Heroic Leap
            speedXY += 20.f;
            break;
        case 252152: // Argus: Edge of Obliteration
        case 258833: // Argus: Edge of Annihilation
            speedXY = m_spellInfo->GetEffect(i, m_diffMode)->MiscValue;
            speedZ = 0.f;
            break;
        default:
            break;
    }
}

void Spell::EffectIncreaseCurrencyCap(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    auto effect = m_spellInfo->GetEffect(effIndex, m_diffMode);

    if (!sCurrencyTypesStore.LookupEntry(effect->MiscValue))
        return;

    if (auto player = unitTarget->ToPlayer())
        player->ModCurrnecyCap(effect->MiscValue, effect->BasePoints);
}

void Spell::EffectPlayerMoveWaypoints(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    auto player = unitTarget->ToPlayer();
    if (!player)
        return;

    auto path = sWaypointMgr->GetPathScript(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
    if (!path)
        return;

    player->GetMotionMaster()->MoveIdle();
    player->GetMotionMaster()->MovePath(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, false);
}

void Spell::EffectTeleportUnits(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || unitTarget->isInFlight())
        return;

    // Pre effects
    uint8 uiMaxSafeLevel = 0;
    switch (m_spellInfo->Id)
    {
        case 48129:  // Scroll of Recall
            uiMaxSafeLevel = 40;
        case 60320:  // Scroll of Recall II
            if (!uiMaxSafeLevel)
                uiMaxSafeLevel = 70;
        case 60321:  // Scroll of Recal III
            if (!uiMaxSafeLevel)
                uiMaxSafeLevel = 80;

            if (unitTarget->getLevel() > uiMaxSafeLevel)
            {
                unitTarget->AddAura(60444, unitTarget); //Apply Lost! Aura

                // ALLIANCE from 60323 to 60330 - HORDE from 60328 to 60335

                uint32 spellId = 60323;
                if (m_caster->ToPlayer()->GetTeam() == HORDE)
                    spellId += 5;
                spellId += urand(0, 7);
                m_caster->CastSpell(m_caster, spellId, true);
                return;
            }
            break;
        case 148705: // Teleport Banishment(Ordos Palace)
        {
            if (Player* caster = m_caster->ToPlayer())
            {
                if (caster->isGameMaster())
                    return;

                if (caster->GetQuestStatus(33104) == QUEST_STATUS_REWARDED || caster->HasAchieved(8325))
                    return;
            }
            break;
        }
    }

    bool checkDest = true;
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetA.GetTarget() == TARGET_UNIT_CASTER && m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == TARGET_NONE)
        if (SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id))
            if (m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_L) || m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT))
            {
                destTarget->Relocate(st->target_X, st->target_Y, st->target_Z, st->target_Orientation);
                destTarget->SetMapId(st->target_mapId);
                checkDest = false;
            }

    // If not exist data for dest location - return
    if (checkDest && !m_targets.HasDst())
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::EffectTeleportUnits - does not have destination for spell ID %u\n", m_spellInfo->Id);
        return;
    }

    // Init dest coordinates
    uint32 mapid = destTarget->GetMapId();
    if (mapid == MAPID_INVALID)
        mapid = unitTarget->GetMapId();
    float x, y, z, orientation;
    destTarget->GetPosition(x, y, z, orientation);
    if (!orientation && m_targets.GetUnitTarget())
        orientation = m_targets.GetUnitTarget()->GetOrientation();
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == TARGET_DEST_TARGET_FRONT) // need change orientation
        orientation += static_cast<float>(M_PI);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectTeleportUnits - teleport unit to %u %f %f %f %f targetGuid %u\n", mapid, x, y, z, orientation, unitTarget->GetGUIDLow());

    if (Player* player = unitTarget->ToPlayer())
    {
        if (uint32 customLoadingScreenId = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue)
            player->SendDirectMessage(WorldPackets::Misc::CustomLoadScreen(m_spellInfo->Id, customLoadingScreenId).Write());

        player->TeleportTo(mapid, x, y, z, orientation, unitTarget == m_caster ? TELE_TO_SPELL | TELE_TO_NOT_LEAVE_COMBAT : 0, m_spellInfo->Id);
    }
    else if (mapid == unitTarget->GetMapId())
        unitTarget->NearTeleportTo(x, y, z, orientation, unitTarget == m_caster);

    // post effects for TARGET_DEST_DB
    switch (m_spellInfo->Id)
    {
        case 66550: // Teleport outside (Isle of Conquest)
        case 66551: // Teleport inside (Isle of Conquest)
        {
            if (Creature* teleportTarget = m_caster->FindNearestCreature((m_spellInfo->Id == 66550 ? 23472 : 22515), 35.0f, true))
            {
                float o;
                teleportTarget->GetPosition(x, y, z, o);

                if (m_caster->IsPlayer())
                    m_caster->ToPlayer()->SafeTeleport(628, x, y, z, o);
            }
            break;
        }
        case 54643: // Telepot inside (Strand of the Ancients)
        {
            if (Creature* teleportTarget = m_caster->FindNearestCreature(23472, 35.0f, true))
            {
                float o;
                teleportTarget->GetPosition(x, y, z, o);

                if (m_caster->IsPlayer())
                    m_caster->ToPlayer()->SafeTeleport(teleportTarget->GetMapId(), x, y, z, o);
            }
            break;
        }
        // Dimensional Ripper - Everlook
        case 23442:
        {
            int32 r = irand(0, 119);
            if (r >= 70)                                  // 7/12 success
            {
                if (r < 100)                              // 4/12 evil twin
                    m_caster->CastSpell(m_caster, 23445, true);
                else                                        // 1/12 fire
                    m_caster->CastSpell(m_caster, 23449, true);
            }
            return;
        }
        // Ultra safe Transporter: Toshley's Station
        case 36941:
        {
            if (roll_chance_i(50))                        // 50% success
            {
                int32 rand_eff = urand(1, 7);
                switch (rand_eff)
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster, 36900, true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster, 36901, true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster, 36895, true);
                        break;
                    case 4:
                        // Decrease the size
                        m_caster->CastSpell(m_caster, 36893, true);
                        break;
                    case 5:
                        // Transform
                    {
                        if (m_caster->ToPlayer()->GetTeam() == ALLIANCE)
                            m_caster->CastSpell(m_caster, 36897, true);
                        else
                            m_caster->CastSpell(m_caster, 36899, true);
                        break;
                    }
                    case 6:
                        // chicken
                        m_caster->CastSpell(m_caster, 36940, true);
                        break;
                    case 7:
                        // evil twin
                        m_caster->CastSpell(m_caster, 23445, true);
                        break;
                }
            }
            return;
        }
        // Dimensional Ripper - Area 52
        case 36890:
        {
            if (roll_chance_i(50))                        // 50% success
            {
                int32 rand_eff = urand(1, 4);
                switch (rand_eff)
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster, 36900, true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster, 36901, true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster, 36895, true);
                        break;
                    case 4:
                        // Transform
                    {
                        if (m_caster->ToPlayer()->GetTeam() == ALLIANCE)
                            m_caster->CastSpell(m_caster, 36897, true);
                        else
                            m_caster->CastSpell(m_caster, 36899, true);
                        break;
                    }
                }
            }
            return;
        }
    }
}

void Spell::EffectApplyAura(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!m_spellAura || !unitTarget)
        return;

    switch (m_spellAura->GetId())
    {
        case 38177:
            if (unitTarget->GetEntry() != 21387)
                return;
            break;
        case 91836:
            m_caster->RemoveAurasDueToSpell(91832);
            break;
        case 42292:
        case 195710: // Honorable Medallion
        case 208683: // Gladiator's Medallion
        {
            if (m_caster->ToPlayer() && m_caster->ToPlayer()->GetTeam())
            {
                int32 visual = m_caster->ToPlayer()->GetTeam() == ALLIANCE ? 97403 : 97404;
                m_caster->CastSpell(m_caster, visual, true);
            }
            break;
        }
    }

    ASSERT(unitTarget == m_spellAura->GetOwner());
    m_spellAura->_ApplyEffectForTargets(effIndex);
    if (_triggeredCastFlags & TRIGGERED_CASTED_BY_AREATRIGGER)
        m_spellAura->SetAuraAttribute(AURA_ATTR_FROM_AREATRIGGER);
}

void Spell::EffectApplyAreaAura(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!m_spellAura || !unitTarget)
        return;
    ASSERT(unitTarget == m_spellAura->GetOwner());
    m_spellAura->_ApplyEffectForTargets(effIndex);
    if (_triggeredCastFlags & TRIGGERED_CASTED_BY_AREATRIGGER)
        m_spellAura->SetAuraAttribute(AURA_ATTR_FROM_AREATRIGGER);
}

void Spell::EffectUnlearnSpecialization(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();
    uint32 spellToUnlearn = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;

    player->removeSpell(spellToUnlearn);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell: Player %u has unlearned spell %u from NpcGUID: %u", player->GetGUIDLow(), spellToUnlearn, m_caster->GetGUIDLow());
}

void Spell::EffectPowerDrain(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue < 0 || m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue >= int8(MAX_POWERS))
        return;

    auto powerType = Powers(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);

    if (!unitTarget || !unitTarget->isAlive() || unitTarget->getPowerType() != powerType || damage < 0)
        return;

    // add spell damage bonus
    std::vector<uint32> ExcludeAuraList;
    uint32 bonus = m_caster->SpellDamageBonusDone(unitTarget, m_spellInfo, static_cast<uint32>(damage), SPELL_DIRECT_DAMAGE, ExcludeAuraList, effIndex);
    damage = bonus + uint32(bonus * variance);

    int32 newDamage = -(unitTarget->ModifyPower(powerType, -damage));

    float gainMultiplier = 0.0f;

    // Don't restore from self drain
    if (m_caster != unitTarget)
    {
        gainMultiplier = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValueMultiplier(m_originalCaster, this);
        m_caster->EnergizeBySpell(m_caster, m_spellInfo->Id, int32(newDamage* gainMultiplier), powerType);
    }
    ExecuteLogEffectTakeTargetPower(effIndex, unitTarget, powerType, newDamage, gainMultiplier);
}

void Spell::EffectSendEvent(SpellEffIndex effIndex)
{
    // we do not handle a flag dropping or clicking on flag in battleground by sendevent system
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET
        && effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    WorldObject* target = nullptr;

    // call events for object target if present
    if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT_TARGET)
    {
        if (unitTarget)
            target = unitTarget;
        else if (gameObjTarget)
            target = gameObjTarget;
    }
    else // if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT)
    {
        // let's prevent executing effect handler twice in case when spell effect is capable of targeting an object
        // this check was requested by scripters, but it has some downsides:
        // now it's impossible to script (using sEventScripts) a cast which misses all targets
        // or to have an ability to script the moment spell hits dest (in a case when there are object targets present)
        if (m_spellInfo->GetEffect(effIndex, m_diffMode)->GetProvidedTargetMask() & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_GAMEOBJECT_MASK))
            return;
        // some spells have no target entries in dbc and they use focus target
        if (focusObject)
            target = focusObject;
        // TODO: there should be a possibility to pass dest target to event script
    }

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell ScriptStart %u for spellid %u in EffectSendEvent ", m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, m_spellInfo->Id);

    if (ZoneScript* zoneScript = m_caster->GetZoneScript())
        zoneScript->ProcessEvent(target, m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
    else if (InstanceScript* instanceScript = m_caster->GetInstanceScript())    // needed in case Player is the caster
        instanceScript->ProcessEvent(target, m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);

    m_caster->GetMap()->ScriptsStart(sEventScripts, m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, m_caster, target);

    if (Player* player = m_caster->ToPlayer())
    {
        player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT, m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, 1);
        player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, 1);
    }
}

void Spell::EffectPowerBurn(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue < 0 || m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue >= int8(MAX_POWERS))
        return;

    auto powerType = Powers(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);

    if (!unitTarget || !unitTarget->isAlive() /*|| unitTarget->getPowerType() != powerType */ || damage < 0)
        return;

    int32 newDamage = -(unitTarget->ModifyPower(powerType, -damage, true));

    // NO - Not a typo - EffectPowerBurn uses effect value multiplier - not effect damage multiplier
    float dmgMultiplier = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValueMultiplier(m_originalCaster, this);

    // add log data before multiplication (need power amount, not damage)
    ExecuteLogEffectTakeTargetPower(effIndex, unitTarget, powerType, newDamage, 0.0f);

    newDamage = int32(newDamage* dmgMultiplier);

    m_damage += newDamage;
}

void Spell::EffectHeal(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit* caster = m_originalCasterGUID ? m_originalCaster : m_caster;

        // Skip if m_originalCaster not available
        if (!caster)
            return;

        if (m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_SHARE_DAMAGE)
        {
            uint32 count = 0;
            for (auto& ihit : m_UniqueTargetInfo)
                if (ihit->effectMask & (1 << effIndex))
                    ++count;

            damage /= count;                    // divide to all targets
        }

        int32 addhealth = damage;

        switch (m_spellInfo->Id)
        {
            case 115464: // Healing Sphere
            {
                unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, [](AuraApplication const* aurApp) -> bool
                {
                    return (aurApp->GetBase()->GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC) != 0;
                });
                unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH, [](AuraApplication const* aurApp) -> bool
                {
                    return (aurApp->GetBase()->GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC) != 0;
                });
                break;
            }
            case 202824: // Greed
            {
                if (AuraEffect const* aurEff = m_caster->GetAuraEffect(202820, EFFECT_0))
                    addhealth = unitTarget->CountPctFromMaxHealth(aurEff->GetAmount(), m_caster);
                break;
            }
            case 115072: // Expel Harm
            case 147489: // Expel Harm
            {
                SpellInfo const* _triggerInfo = sSpellMgr->GetSpellInfo(115129);
                float bp = CalculatePct(addhealth, _triggerInfo->Effects[EFFECT_1]->BasePoints);
                m_caster->CastCustomSpell(unitTarget, 115129, &bp, nullptr, nullptr, true);
                break;
            }
            case 90361: // Spirit Mend
            {
                uint32 ap = m_caster->GetTotalAttackPowerValue(BASE_ATTACK) * 3;
                std::vector<uint32> ExcludeAuraList;
                uint32 spd = m_caster->SpellDamageBonusDone(m_caster, m_spellInfo, damage, SPELL_DIRECT_DAMAGE, ExcludeAuraList, effIndex);
                addhealth += int32(ap * 1.0504f) + spd;
                break;
            }
            case 45064: // Vessel of the Naaru (Vial of the Sunwell trinket)
            {
                int damageAmount = 0; // Amount of heal - depends from stacked Holy Energy
                if (AuraEffect const* aurEff = m_caster->GetAuraEffect(45062, 0))
                {
                    damageAmount += aurEff->GetAmount();
                    m_caster->RemoveAurasDueToSpell(45062);
                }
                addhealth += damageAmount;
                break;
            }
            case 67489: // Runic Healing Injector (heal increased by 25% for engineers - 3.2.0 patch change)
            {
                if (Player* player = m_caster->ToPlayer())
                    if (player->HasSkill(SKILL_ENGINEERING))
                        AddPct(addhealth, 25);
                break;
            }
            default:
                break;
        }
        // Death Pact - return pct of max health to caster
        if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_DEATHKNIGHT && m_spellInfo->ClassOptions.SpellClassMask[0] & 0x00080000)
            addhealth = caster->SpellHealingBonusDone(unitTarget, m_spellInfo, int32(caster->CountPctFromMaxHealth(damage)), HEAL, effIndex);
        else
        {
            addhealth = caster->SpellHealingBonusDone(unitTarget, m_spellInfo, addhealth, HEAL, effIndex);
            // uint32 bonus = caster->SpellHealingBonusDone(unitTarget, m_spellInfo, addhealth, HEAL, effIndex);
            // damage = bonus + uint32(bonus * variance);
        }

        addhealth *= caster->GetProcStatsMultiplier(m_spellInfo->Id);
        addhealth = unitTarget->SpellHealingBonusTaken(caster, m_spellInfo, addhealth, HEAL, effIndex);

        switch (m_spellInfo->Id)
        {
            case 45470:
            {
                if (Aura* aur = m_caster->GetAura(77513)) // Mastery: Blood Shield
                {
                    float bp0 = int32(addhealth * float(aur->GetEffect(EFFECT_0)->GetAmount() / 100.f));
                    if (Aura* aurShield = m_caster->GetAura(77535))
                        bp0 += aurShield->GetEffect(0)->GetAmount();

                    if (bp0 > int32(m_caster->GetMaxHealth()))
                        bp0 = int32(m_caster->GetMaxHealth());

                    if (m_caster->HasAura(192567)) // Unending Thirst
                        m_caster->CastSpell(m_caster, 216019, true);

                    m_caster->CastCustomSpell(m_caster, 77535, &bp0, nullptr, nullptr, true);
                }
                break;
            }
            case 213672: // Touch of the Moon
            {
                if (AuraEffect const* aurEff = m_caster->GetAuraEffect(203018, EFFECT_0))
                    addhealth *= aurEff->GetAmount();
                break;
            }
            case 191580: // Shamanistic Healing
            {
                if (AuraEffect const* aurEff = m_caster->GetAuraEffect(191582, EFFECT_0))
                    addhealth *= aurEff->GetAmount();
                break;
            }
            case 47750: // Penance
            {
                if (!m_triggeredByAura)
                {
                    if (AuraEffect const* eff = m_caster->GetAuraEffect(242268, EFFECT_0)) // Item - Priest T20 Discipline 2P Bonus
                        AddPct(addhealth, eff->GetAmount());
                }
                break;
            }
            case 242400: // Whispers of Shaohao
            {
                addhealth *= damage;
                break;
            }
            case 148009: // Spirit of Chi-Ji
            {
                addhealth /= m_UniqueTargetInfo.size();
                break;
            }
            case 19750: // Selfless Healer - Increases heal of Flash of Light if it heals an other player than you
            {
                if (Aura* selflessHealer = caster->GetAura(114250))
                {
                    int32 perc = 0;

                    Player* player = caster->ToPlayer();
                    if (!player)
                        break;

                    if (player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) != SPEC_PALADIN_HOLY)
                    {
                        if (AuraEffect* eff = selflessHealer->GetEffect(EFFECT_1))
                            perc = eff->GetAmount();
                    }
                    else if (player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_PALADIN_HOLY)
                    {
                        if (AuraEffect* eff = selflessHealer->GetEffect(EFFECT_3))
                            perc = eff->GetAmount();
                    }

                    if (perc && unitTarget->GetGUID() != caster->GetGUID())
                        AddPct(addhealth, perc);
                    else if (unitTarget->GetGUID() == caster->GetGUID())
                    {
                        if (Aura* bastion = caster->GetAura(114637))
                            if (AuraEffect* bastionEff = bastion->GetEffect(EFFECT_2))
                            {
                                AddPct(addhealth, bastionEff->GetAmount());
                                bastion->Remove();
                            }
                    }
                }
                break;
            }
            default:
                break;
        }

        // Remove Grievous bite if fully healed
        if (unitTarget->HasAura(48920) && (unitTarget->GetHealth(caster) + addhealth >= unitTarget->GetMaxHealth(caster)))
            unitTarget->RemoveAura(48920);

        // Mogu'Shan Vault
        if (caster->HasAura(116161) || unitTarget->HasAura(116161)) // SPELL_CROSSED_OVER
        {
            // http://fr.wowhead.com/spell=117549#english-comments
            // uint32 targetSpec = unitTarget->ToPlayer()->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID);

            if (unitTarget == caster)
            {
                float bp1 = addhealth / 2;
                float bp2 = 15;

                caster->CastCustomSpell(unitTarget, 117543, &bp1, &bp2, nullptr, nullptr, nullptr, nullptr, true); // Mana regen bonus
            }
            else
            {
                float bp1 = 10;
                float bp2 = 15;
                float bp3 = 20;
                float bp4 = 25;
                float bp5 = 30;
                float bp6 = 35;

                caster->CastCustomSpell(unitTarget, 117549, &bp1, &bp2, &bp3, &bp4, &bp5, &bp6, true);
            }

            if (unitTarget->GetHealth(caster) + addhealth >= unitTarget->GetMaxHealth(caster))
                unitTarget->CastSpell(unitTarget, 120717, true);  // Revitalized Spirit
        }

        switch (m_caster->getClass())
        {
            case CLASS_DRUID:
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
                    break;

                Aura* aura = m_caster->GetAura(77495); // Mastery: Harmony
                if (!aura)
                    break;

                float modDif = 0.f;
                uint32 modCount = 0;
                uint32 modMaxCount = 9;
                if (AuraEffect* eff = aura->GetEffect(EFFECT_1))
                    modMaxCount = eff->GetAmount();
                if (AuraEffect* eff = aura->GetEffect(EFFECT_0))
                    modDif = eff->GetAmount();
                if (Unit::AuraEffectList const* mPeriodic = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL))
                {
                    for (Unit::AuraEffectList::const_iterator i = mPeriodic->begin(); i != mPeriodic->end(); ++i)
                    {
                        if ((*i)->GetCasterGUID() == m_caster->GetGUID())
                            modCount++;
                        if (modCount >= modMaxCount)
                            break;
                    }
                }
                if (modCount && modDif)
                    addhealth += CalculatePct(addhealth, modDif * modCount);
                break;
            }
            case CLASS_PALADIN:
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
                    break;

                auto aurEff = m_caster->GetAuraEffect(183997, EFFECT_0); // Mastery: Lightbringer
                if (!aurEff)
                    break;

                Unit* DistanceUnit = m_caster;
                if (m_caster->HasAura(197446)) // Beacon of the Lightbringer
                {
                    if (Aura* aura = m_caster->GetAura(53651))
                    {
                        std::list<Unit*> targetList;
                        std::vector<uint32> auraList = { 53563 };

                        m_caster->TargetsWhoHasMyAuras(targetList, auraList);
                        if (!targetList.empty())
                            if (Unit* _target = Trinity::Containers::SelectRandomContainerElement(targetList))
                                DistanceUnit = _target;
                    }
                }

                uint32 _perc = aurEff->GetAmount();
                float _distance = unitTarget->GetDistance(DistanceUnit);
                float _distance2 = unitTarget->GetDistance(m_caster);
                if (_distance > _distance2)
                    _distance = _distance2;

                float maxDist = 30;
                if (m_caster->HasAura(214202)) // Rule of Law
                    maxDist += CalculatePct(maxDist, 50);

                if (_distance <= 10.0f)
                    addhealth += CalculatePct(addhealth, _perc);
                else if (_distance <= maxDist)
                    addhealth += CalculatePct(addhealth, CalculatePct(_perc, (100 - ((_distance - 10) / maxDist) * 100)));
                break;
            }
            default:
                break;
        }

        m_damage -= addhealth;
        //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectHeal m_damage %i addhealth %i damage %i", m_damage, addhealth, damage);
    }
}

void Spell::EffectHealPct(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive() || damage < 0)
        return;

    // Skip if m_originalCaster not available
    if (!m_originalCaster)
        return;

    uint32 divider = 1;

    if (m_spellInfo->HasAttribute(SPELL_ATTR11_UNK4))
    {
        bool resetHeal = true;
        if (Unit::AuraEffectList const* mTotalAuraList = m_caster->GetAuraEffectsByType(SPELL_AURA_DUMMY))
            for (Unit::AuraEffectList::const_iterator i = mTotalAuraList->begin(); i != mTotalAuraList->end(); ++i)
                if ((*i)->GetMiscValue() == 11)
                    if ((*i)->GetAmount() == m_spellInfo->Id)
                    {
                        (*i)->SetAmount(NULL);
                        resetHeal = false;
                    }

        if (resetHeal)
            damage = 0;
    }

    switch (m_spellInfo->Id)
    {
        case 199221: // Sweet Souls
            if (AuraEffect* aurEff = unitTarget->GetAuraEffect(199220, EFFECT_0))
                damage = aurEff->GetAmount();
            break;
        case 63106: // Siphon Life
            divider = 1000;
            break;
        case 118779: // Victory Rush
            if (m_caster->HasAura(138279))
            {
                //damage *= 2;
                m_caster->RemoveAurasDueToSpell(138279);
            }
            break;
        default:
            break;
    }

    uint32 heal = unitTarget->CountPctFromMaxHealth(damage, m_caster);

    switch (m_spellInfo->Id)
    {
        case 197509: // Bloodworm
            if (!unitTarget->IsFullHealth())
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(195679))
                    heal = CalculatePct(unitTarget->GetMaxHealth() - unitTarget->GetHealth(), spellInfo->GetEffect(EFFECT_2, m_diffMode)->CalcValue(unitTarget));
            break;
        default:
            break;
    }

    if (damage)
    {
        heal = m_originalCaster->SpellHealingBonusDone(unitTarget, m_spellInfo, heal, HEAL, effIndex);
        heal = unitTarget->SpellHealingBonusTaken(m_originalCaster, m_spellInfo, heal, HEAL, effIndex);
    }

    heal /= divider;
    m_healing += heal;
}

void Spell::EffectHealMechanical(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive() || damage < 0)
        return;

    // Skip if m_originalCaster not available
    if (!m_originalCaster)
        return;

    uint32 heal = m_originalCaster->SpellHealingBonusDone(unitTarget, m_spellInfo, uint32(damage), HEAL, effIndex);
    heal += uint32(heal * variance);

    m_healing += unitTarget->SpellHealingBonusTaken(m_originalCaster, m_spellInfo, heal, HEAL, effIndex);
}

void Spell::EffectHealthLeech(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive() || damage < 0)
        return;

    std::vector<uint32> ExcludeAuraList;
    uint32 bonus = m_caster->SpellDamageBonusDone(unitTarget, m_spellInfo, static_cast<uint32>(damage), SPELL_DIRECT_DAMAGE, ExcludeAuraList, effIndex);
    bonus *= m_originalCaster->GetProcStatsMultiplier(m_spellInfo->Id);
    bonus = unitTarget->SpellDamageBonusTaken(m_caster, m_spellInfo, bonus); // Not sure, may be not need 

    if (m_spellInfo->Id == 215661) // Justicar's Vengeance
    {
        float pct = m_spellInfo->Effects[EFFECT_1]->BasePoints;

        if (m_caster->CanPvPScalar())
            pct *= 0.5f;

        if (unitTarget->HasAuraWithMechanic((1 << MECHANIC_STUN)))
            AddPct(bonus, pct);
    }

    if (m_spellInfo->Id == 205246) // Phantom Singularity
    {
        if (m_caster->HasAura(234876))
        {
            SpellInfo const* deathsEmbrace = sSpellMgr->GetSpellInfo(234876);
            if (unitTarget->GetHealthPct() <= deathsEmbrace->Effects[EFFECT_1]->BasePoints)
            {
                int32 bp = CalculatePct((100 - unitTarget->GetHealthPct()), deathsEmbrace->Effects[EFFECT_0]->BasePoints);
                AddPct(bonus, bp);
            }
        }
    }

    damage = bonus + uint32(bonus * variance);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "HealthLeech damage %i bonus %u variance %f", damage, bonus, variance);

    m_damage += damage;
    if (m_caster->isAlive())
    {
        float healMultiplier = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValueMultiplier(m_originalCaster, this);

        float critMod = 2.f;
        bool isCrit = IsCritForTarget(unitTarget);

        if ((m_caster->IsPlayer() || m_caster->HasUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER)) &&
            unitTarget->IsPlayer() || unitTarget->HasUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER))
            critMod = 1.5f;

        uint64 healthGain = isCrit ? damage * critMod : damage;
        // healthGain = m_caster->SpellHealingBonusDone(m_caster, m_spellInfo, healthGain, HEAL, effIndex);
        healthGain = m_caster->SpellHealingBonusTaken(m_caster, m_spellInfo, healthGain, HEAL, effIndex);

        // get max possible damage, don't count overkill for heal
        healthGain = uint64(-unitTarget->GetHealthGain(-1 * healthGain) * healMultiplier);

        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "HealthLeech damage %i healthGain %u", damage, healthGain);

        m_caster->HealBySpell(m_caster, m_spellInfo, uint64(healthGain), isCrit);
    }
}

void Spell::DoCreateItem(uint32 /*effIndex*/, uint32 itemtype, std::vector<uint32> const& bonusListIDs)
{
    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();

    uint32 newitemid = itemtype;
    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(newitemid);
    if (!pProto)
    {
        player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    uint32 num_to_add = damage;

    if (num_to_add < 1)
        num_to_add = 1;
    if (num_to_add > pProto->GetMaxStackSize())
        num_to_add = pProto->GetMaxStackSize();

    // init items_count to 1, since 1 item will be created regardless of specialization
    int items_count = 1;
    // the chance to create additional items
    float additionalCreateChance = 0.0f;
    // the maximum number of created additional items
    uint8 additionalMaxNum = 0;
    // get the chance and maximum number for creating extra items
    if (canCreateExtraItems(player, m_spellInfo->Id, additionalCreateChance, additionalMaxNum))
    {
        // roll with this chance till we roll not to create or we create the max num
        while (roll_chance_f(additionalCreateChance) && items_count <= additionalMaxNum)
            ++items_count;
    }

    // really will be created more items
    num_to_add *= items_count;

    // can the player store the new item?
    ItemPosCountVec dest;
    uint32 no_space = 0;
    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, newitemid, num_to_add, &no_space);
    if (msg != EQUIP_ERR_OK)
    {
        // convert to possible store amount
        if (msg == EQUIP_ERR_INV_FULL || msg == EQUIP_ERR_ITEM_MAX_COUNT)
            num_to_add -= no_space;
        else
        {
            // if not created by another reason from full inventory or unique items amount limitation
            player->SendEquipError(msg, nullptr, nullptr, newitemid);
            return;
        }
    }

    if (num_to_add)
    {
        // create the new item and store it
        Item* pItem = player->StoreNewItem(dest, newitemid, true, Item::GenerateItemRandomPropertyId(newitemid, player->GetLootSpecID()), GuidSet(), bonusListIDs);

        if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
        {
            if (pProto->GetQuality() > ITEM_QUALITY_EPIC || (pProto->GetQuality() == ITEM_QUALITY_EPIC && pProto->ItemLevel >= MinNewsItemLevel[CURRENT_EXPANSION]))
                if (!pProto->IsNotAppearInGuildNews())
                    guild->AddGuildNews(GUILD_NEWS_ITEM_CRAFTED, player->GetGUID(), 0, pProto->GetId(), pItem);

            guild->UpdateAchievementCriteria(CRITERIA_TYPE_CRAFT_ITEMS_GUILD, pProto->GetId(), num_to_add, 0, nullptr, player);
        }

        // was it successful? return error if not
        if (!pItem)
        {
            player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
            return;
        }

        // set the "Crafted by ..." property of the item
        if ((pItem->GetTemplate()->GetClass() != ITEM_CLASS_CONSUMABLE && pItem->GetTemplate()->GetClass() != ITEM_CLASS_QUEST && newitemid != 6265 && newitemid != 6948) || newitemid == 5512)
            pItem->SetUInt32Value(ITEM_FIELD_CREATOR, player->GetGUIDLow());

        // send info to the client
        player->SendNewItem(pItem, num_to_add, true, true);

        sScriptMgr->OnCreateItem(player, pItem, num_to_add);

        // we succeeded in creating at least one item, so a level up is possible
        player->UpdateCraftSkill(m_spellInfo->Id);
        player->UpdateAchievementCriteria(CRITERIA_TYPE_CREATE_ITEM, newitemid, num_to_add);
    }
}

void Spell::EffectCreateItem(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (m_caster->IsPlayer() && m_spellInfo->IsArchaeologyCraftingSpell())
        if (!m_caster->ToPlayer()->SolveResearchProject(m_spellInfo->Id, m_targets))
            return;

    uint32 itemID = m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType;
    uint32 itemLevelFromBonus = 0;
    uint32 itemBonusTreeMod = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    if (!itemBonusTreeMod && m_CastItem)
        itemBonusTreeMod = m_CastItem->GetUInt32Value(ITEM_FIELD_CONTEXT);

    if (ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemID))
        if (pProto->GetQuality() == ITEM_QUALITY_LEGENDARY && pProto->GetExpansion() == EXPANSION_LEGION)
            itemBonusTreeMod = 0;

    std::vector<uint32> bonusListIDs = sDB2Manager.GetItemBonusTree(itemID, itemBonusTreeMod, itemLevelFromBonus);

    DoCreateItem(effIndex, itemID, bonusListIDs);
    ExecuteLogEffectCreateItem(effIndex, itemID);
}

void Spell::EffectDestroyItem(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* player = unitTarget->ToPlayer();
    if (!player)
        return;

    player->DestroyItemCount(m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType, 1, true);
    //ExecuteLogEffectTradeSkillItem(effIndex, m_spellInfo->GetEffect(effIndex, m_diffMode).ItemType);
}

void Spell::EffectCreateItem2(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    if (m_caster->IsPlayer() && m_spellInfo->IsArchaeologyCraftingSpell())
        if (!m_caster->ToPlayer()->SolveResearchProject(m_spellInfo->Id, m_targets))
            return;

    Player* player = unitTarget->ToPlayer();

    uint32 item_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType;

    uint32 itemBonusTreeMod = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB;
    if (!itemBonusTreeMod && m_CastItem)
        itemBonusTreeMod = m_CastItem->GetUInt32Value(ITEM_FIELD_CONTEXT);

    if (item_id)
    {
        uint32 itemLevelFromBonus = 0;
        if (ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(item_id))
            if (pProto->GetQuality() == ITEM_QUALITY_LEGENDARY && pProto->GetExpansion() == EXPANSION_LEGION)
                itemBonusTreeMod = 0;

        std::vector<uint32> bonusListIDs = sDB2Manager.GetItemBonusTree(item_id, itemBonusTreeMod, itemLevelFromBonus);
        DoCreateItem(effIndex, item_id, bonusListIDs);
    }

    // special case: fake item replaced by generate using spell_loot_template
    if (m_spellInfo->IsLootCrafting())
    {
        if (item_id)
        {
            if (!player->HasItemCount(item_id))
                return;

            // create some random items
            if(player->AutoStoreLoot(m_spellInfo->Id, LootTemplates_Spell, true, itemBonusTreeMod))
            // remove reagent
                player->DestroyItemCount(item_id, 1, true);
        }
        else
        {
            player->AutoStoreLoot(m_spellInfo->Id, LootTemplates_Spell, true, itemBonusTreeMod);    // create some random items
            player->UpdateCraftSkill(m_spellInfo->Id);
        }
    }

    ExecuteLogEffectCreateItem(effIndex, m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType);
}

void Spell::EffectCreateItem3(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Item* item = m_targets.GetItemTarget();
    if (!item)
        return;

    Player* player = m_caster->ToPlayer();
    if (item->GetEntry() && player)
    {
        if (m_spellInfo->IsLootCrafting())
        {
            if (player->AutoStoreLoot(item->GetTemplate()->Effects[0]->SpellID, LootTemplates_Spell))
                player->DestroyItemCount(item->GetEntry(), 1, true);
        }
    }
}

void Spell::EffectCreateRandomItem(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;
    Player* player = unitTarget->ToPlayer();

    uint32 item_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType;

    if (item_id)
        DoCreateItem(effIndex, item_id);

    // create some random items
    player->AutoStoreLoot(m_spellInfo->Id, LootTemplates_Spell);
    ExecuteLogEffectCreateItem(effIndex, m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType);
}

void Spell::EffectPersistentAA(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_spellAura)
    {
        Unit* caster = m_caster->GetEntry() == WORLD_TRIGGER ? m_originalCaster : m_caster;
        float radius = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(caster);

        // Caster not in world, might be spell triggered from aura removal
        if (!caster->IsInWorld())
            return;

        auto dynObj = new DynamicObject(false);
        if (!dynObj->CreateDynamicObject(sObjectMgr->GetGenerator<HighGuid::DynamicObject>()->Generate(), caster, m_spellInfo->Id, *destTarget, radius, DYNAMIC_OBJECT_AREA_SPELL))
        {
            delete dynObj;
            return;
        }

        SetSpellDynamicObject(dynObj->GetGUID());
        Aura* aura = Aura::TryCreate(m_spellInfo, MAX_EFFECT_MASK, dynObj, caster, &m_spellValue->EffectBasePoints[0]);
        if (aura != nullptr)
        {
            m_spellAura = aura;
            m_spellAura->_RegisterForTargets();
        }
        else
            return;
    }

    ASSERT(m_spellAura->GetDynobjOwner());
    m_spellAura->_ApplyEffectForTargets(effIndex);
    if (_triggeredCastFlags & TRIGGERED_CASTED_BY_AREATRIGGER)
        m_spellAura->SetAuraAttribute(AURA_ATTR_FROM_AREATRIGGER);
}

void Spell::EffectEnergize(SpellEffIndex effIndex)
{
    if (m_spellInfo->GetMisc()->MiscData.Speed)
    {
        if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
            return;
    }
    else if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue < 0 || m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue >= int8(MAX_POWERS))
        return;

    auto power = Powers(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
    if (unitTarget->GetMaxPower(power) == 0)
        return;

    if (float scalar = m_spellInfo->GetEffect(effIndex, m_diffMode)->PvPMultiplier)
        if (unitTarget->CanPvPScalar())
            damage *= scalar;

    // Some level depends spells
    int level_multiplier = 0;
    int level_diff = 0;
    switch (m_spellInfo->Id)
    {
        case 235272:
            damage *= -1;
        break;
        case 225772: // Temporal Shift
        {
            if (AuraEffect const* aurEff = m_caster->GetAuraEffect(225138, EFFECT_1)) // Temporal Shift
                damage += aurEff->GetAmount();
            break;
        }
        case 187890: // Maelstrom Weapon
        {
            if (AuraEffect const* aurEff = m_caster->GetAuraEffect(238106, EFFECT_0)) // Winds of Change
                damage += aurEff->GetAmount();
            break;
        }
        case 1449: // Arcane Explosion
        {
            if (GetTargetCount() < 2)
                damage = 0;
            break;
        }
        case 51637:  // Venomous Vim
        {
            if (AuraEffect const* aurEff = m_caster->GetAuraEffect(152152, EFFECT_0))
                damage += aurEff->GetAmount();
            break;
        }
        case 188070: // Healing Surge
        {
            if(unitTarget->GetPower(POWER_MAELSTROM) >= 20)
                damage = m_spellInfo->Effects[EFFECT_2]->CalcValue(unitTarget) * -1;
            break;
        }
        case 219809: // Tombstone
        {
            if (Aura* aura = unitTarget->GetAura(195181))
            {
                if (aura->GetStackAmount() > 5)
                    damage = m_spellInfo->Effects[EFFECT_2]->CalcValue(unitTarget) * aura->GetStackAmount() * 5;
                else
                    damage = m_spellInfo->Effects[EFFECT_2]->CalcValue(unitTarget) * aura->GetStackAmount() * 10;
            }
            break;
        }
        case 1454: // Life Tap
        {
            damage = CalculatePct(m_caster->GetMaxHealth(), damage);
            break;
        }
        case 24571:                                         // Blood Fury
            level_diff = m_caster->GetEffectiveLevel() - 60;
            level_multiplier = 10;
            break;
        case 24532:                                         // Burst of Energy
            level_diff = m_caster->GetEffectiveLevel() - 60;
            level_multiplier = 4;
            break;
        case 67490:                                         // Runic Mana Injector (mana gain increased by 25% for engineers - 3.2.0 patch change)
        {
            if (Player* player = m_caster->ToPlayer())
                if (player->HasSkill(SKILL_ENGINEERING))
                    AddPct(damage, 25);
            break;
        }
        default:
            break;
    }

    if (level_diff > 0)
        damage -= level_multiplier * level_diff;

    if (unitTarget->HasAura(143594)) //Berserker Stance - General Nazgrim
        damage *= 2;

    if (unitTarget->HasAura(143411)) //Acceleration - Thok Bloodthirsty
    {
        uint8 stack = unitTarget->GetAura(143411)->GetStackAmount();
        switch (stack)
        {
            case 1:
                damage += 1;
                break;
            case 2:
                damage += 4;
                break;
            case 3:
                damage += 11;
                break;
            case 4:
                damage += 14;
                break;
            case 5:
            case 6:
            case 7:
                damage += 21;
                break;
            case 8:
                damage += 46;
                break;
            default:
                damage += 46;
                break;
        }
    }

    m_addptype = power;
    m_addpower = damage;
    m_caster->EnergizeBySpell(unitTarget, m_spellInfo->Id, damage, power);

    if (m_addptype == POWER_RUNES)
    {
        if(Player* _player = m_caster->ToPlayer())
        {
            int32 runesRestor = damage;
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

    // Mad Alchemist's Potion
    if (m_spellInfo->Id == 45051)
    {
        // find elixirs on target
        bool guardianFound = false;
        bool battleFound = false;
        Unit::AuraApplicationMap& Auras = unitTarget->GetAppliedAuras();
        for (auto& Aura : Auras)
        {
            uint32 spell_id = Aura.second->GetBase()->GetId();
            if (!guardianFound)
                if (sSpellMgr->IsSpellMemberOfSpellGroup(spell_id, SPELL_GROUP_ELIXIR_GUARDIAN))
                    guardianFound = true;
            if (!battleFound)
                if (sSpellMgr->IsSpellMemberOfSpellGroup(spell_id, SPELL_GROUP_ELIXIR_BATTLE))
                    battleFound = true;
            if (battleFound && guardianFound)
                break;
        }

        // get all available elixirs by mask and spell level
        std::set<uint32> avalibleElixirs;
        if (!guardianFound)
            sSpellMgr->GetSetOfSpellsInSpellGroup(SPELL_GROUP_ELIXIR_GUARDIAN, avalibleElixirs);
        if (!battleFound)
            sSpellMgr->GetSetOfSpellsInSpellGroup(SPELL_GROUP_ELIXIR_BATTLE, avalibleElixirs);

        for (auto itr = avalibleElixirs.begin(); itr != avalibleElixirs.end();)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(*itr);
            if (spellInfo->SpellLevel < m_spellInfo->SpellLevel || spellInfo->SpellLevel > unitTarget->getLevel())
                avalibleElixirs.erase(itr++);
            else if (sSpellMgr->IsSpellMemberOfSpellGroup(*itr, SPELL_GROUP_ELIXIR_SHATTRATH))
                avalibleElixirs.erase(itr++);
            else if (sSpellMgr->IsSpellMemberOfSpellGroup(*itr, SPELL_GROUP_ELIXIR_UNSTABLE))
                avalibleElixirs.erase(itr++);
            else
                ++itr;
        }

        if (!avalibleElixirs.empty())
        {
            // cast random elixir on target
            m_caster->CastSpell(unitTarget, Trinity::Containers::SelectRandomContainerElement(avalibleElixirs), true, m_CastItem);
        }
    }

    if (m_spellInfo->Id == 144859) //Add CP after use old CP
        m_caster->m_movedPlayer->SaveAddComboPoints(damage);
}

void Spell::EffectEnergizePct(SpellEffIndex effIndex)
{
    if (m_spellInfo->GetMisc()->MiscData.Speed)
    {
        if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
            return;
    }
    else if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive())
        return;

    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue < 0 || m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue >= int8(MAX_POWERS))
        return;

    auto power = Powers(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
    uint32 maxPower = unitTarget->GetMaxPower(power);
    if (maxPower == 0)
        return;

    uint32 gain = CalculatePct(maxPower, damage);

    m_addptype = power;
    m_addpower = gain;
    m_caster->EnergizeBySpell(unitTarget, m_spellInfo->Id, gain, power);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectEnergizePct Id %i damage %i power %i gain %i", m_spellInfo->Id, damage, power, gain);
}

void Spell::SendLoot(ObjectGuid const& guid, LootType loottype)
{
    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    if (gameObjTarget)
    {
        // Players shouldn't be able to loot gameobjects that are currently despawned
        if (!gameObjTarget->isSpawned() && !player->isGameMaster())
        {
            TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Possible hacking attempt: Player %s [guid: %u] tried to loot a gameobject [entry: %u id: %u] which is on respawn time without being in GM mode!",
                           player->GetName(), player->GetGUIDLow(), gameObjTarget->GetEntry(), gameObjTarget->GetGUIDLow());
            return;
        }
        // special case, already has GossipHello inside so return and avoid calling twice
        if (gameObjTarget->GetGoType() == GAMEOBJECT_TYPE_GOOBER)
        {
            gameObjTarget->Use(m_caster);
            return;
        }

        if (sScriptMgr->OnGossipHello(player, gameObjTarget))
            return;

        if (gameObjTarget->AI()->GossipHello(player))
            return;

        switch (gameObjTarget->GetGoType())
        {
            case GAMEOBJECT_TYPE_DOOR:
            case GAMEOBJECT_TYPE_BUTTON:
                gameObjTarget->UseDoorOrButton(0, false, player);
                player->GetMap()->ScriptsStart(sGameObjectScripts, gameObjTarget->GetDBTableGUIDLow(), player, gameObjTarget);
                return;

            case GAMEOBJECT_TYPE_QUESTGIVER:
                player->PrepareGossipMenu(gameObjTarget, gameObjTarget->GetGOInfo()->questgiver.gossipID);
                player->SendPreparedGossip(gameObjTarget);
                return;

            case GAMEOBJECT_TYPE_SPELL_FOCUS:
                // triggering linked GO
                if (uint32 trapEntry = gameObjTarget->GetGOInfo()->spellFocus.linkedTrap)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry, m_caster);
                return;

            case GAMEOBJECT_TYPE_CHEST:
                // TODO: possible must be moved to loot release (in different from linked triggering)
                if (gameObjTarget->GetGOInfo()->chest.triggeredEvent)
                {
                    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Chest ScriptStart id %u for GO %u", gameObjTarget->GetGOInfo()->chest.triggeredEvent, gameObjTarget->GetDBTableGUIDLow());
                    player->GetMap()->ScriptsStart(sEventScripts, gameObjTarget->GetGOInfo()->chest.triggeredEvent, player, gameObjTarget);
                }

                // triggering linked GO
                if (uint32 trapEntry = gameObjTarget->GetGOInfo()->chest.linkedTrap)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry, m_caster);

                // Don't return, let loots been taken
            default:
                break;
        }
    }

    // Send loot
    player->SendLoot(guid, loottype);
}

bool Spell::isDelayableNoMore()
{
    if (m_delayAtDamageCount >= 2)
        return true;

    m_delayAtDamageCount++;
    return false;
}

void Spell::EffectOpenLock(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!m_caster->IsPlayer())
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "WORLD: Open Lock - No Player Caster!");
        return;
    }

    Player* player = m_caster->ToPlayer();

    uint32 lockId = 0;
    ObjectGuid guid;

    if (gameObjTarget)
    {
        if (player->IsPlayerLootCooldown(gameObjTarget->GetEntry(), TYPE_GO, player->GetMap()->GetDifficultyID()) || player->IsPlayerLootCooldown(gameObjTarget->GetGUIDLow(), TYPE_GUID, player->GetMap()->GetDifficultyID()))
        {
            gameObjTarget->DestroyForPlayer(player);
            gameObjTarget->UpdateObjectVisibility();
            return;
        }

        GameObjectTemplate const* goInfo = gameObjTarget->GetGOInfo();
        if ((goInfo->type == GAMEOBJECT_TYPE_BUTTON && goInfo->button.noDamageImmune) || (goInfo->type == GAMEOBJECT_TYPE_GOOBER && goInfo->goober.requireLOS) || (goInfo->type == GAMEOBJECT_TYPE_CAPTURE_POINT))
        {
            if (Battleground* bg = player->GetBattleground())
            {
                bool needDelete = true;
                bg->EventPlayerClickedOnFlag(player, gameObjTarget, needDelete);
                return;
            }
        }
        else if (goInfo->type == GAMEOBJECT_TYPE_FLAGSTAND || goInfo->type == GAMEOBJECT_TYPE_NEW_FLAG)
        {
            if (Battleground* bg = player->GetBattleground())
            {
                switch (bg->GetTypeID(true))
                {
                    case MS::Battlegrounds::BattlegroundTypeId::BrawlEyeOfStorm:
                    case MS::Battlegrounds::BattlegroundTypeId::BattlegroundEyeOfTheStorm:
                    case MS::Battlegrounds::BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm:
                    case MS::Battlegrounds::BattlegroundTypeId::BattlegroundTwinPeaks:
                    case MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch:
                    case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate:
                    case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundKotmoguTemplateSix:
                    case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble:
                    case MS::Battlegrounds::BattlegroundTypeId::BattlegroundKotmoguTemplate:
                    case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongSix:
                        gameObjTarget->Use(player);
                        break;
                    default:
                        break;
                }
                return;
            }
        }
        else if (m_spellInfo->Id == 1842 && gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_TRAP && gameObjTarget->GetOwner())
        {
            gameObjTarget->SetLootState(GO_JUST_DEACTIVATED);
            return;
        }
        // TODO: Add script for spell 41920 - Filling, becouse server it freze when use this spell
        // handle outdoor pvp object opening, return true if go was registered for handling
        // these objects must have been spawned by outdoorpvp!
        else if (goInfo->type == GAMEOBJECT_TYPE_DOOR)
        {
            gameObjTarget->UseDoorOrButton(0, false, player);
            player->GetMap()->ScriptsStart(sGameObjectScripts, gameObjTarget->GetDBTableGUIDLow(), player, gameObjTarget);
        }
        else if (gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_GOOBER && sOutdoorPvPMgr->HandleOpenGo(player, gameObjTarget->GetGUID()) ||
            gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_NEW_FLAG_DROP && sOutdoorPvPMgr->HandleOpenGo(player, gameObjTarget->GetGUID()))
            return;

        lockId = goInfo->GetLockId();
        guid = gameObjTarget->GetGUID();

        if (auto const& goTemplate = sObjectMgr->GetGameObjectTemplate(gameObjTarget->GetEntry()))
            if (goTemplate->type == GAMEOBJECT_TYPE_CHEST)
                if (!sConditionMgr->IsPlayerMeetingCondition(player, goTemplate->chest.conditionID1))
                    return;
    }
    else if (itemTarget)
    {
        lockId = itemTarget->GetTemplate()->GetLockID();
        guid = itemTarget->GetGUID();
    }
    else
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "WORLD: Open Lock - No GameObject/Item Target!");
        return;
    }

    SkillType skillId = SKILL_NONE;
    int32 reqSkillValue = 0;
    int32 skillValue;

    SpellCastResult res = CanOpenLock(effIndex, lockId, skillId, reqSkillValue, skillValue);
    if (res != SPELL_CAST_OK && lockId != 2127)
    {
        SendCastResult(res);
        return;
    }

    if (gameObjTarget)
        SendLoot(guid, LOOT_SKINNING);
    else if (itemTarget)
    {
        itemTarget->SetFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_UNLOCKED);
        itemTarget->SetState(ITEM_CHANGED, itemTarget->GetOwner());
    }

    // not allow use skill grow at item base open
    if (!m_CastItem && skillId != SKILL_NONE)
    {
        // update skill if really known
        if (uint32 pureSkillValue = player->GetPureSkillValue(skillId))
        {
            if (gameObjTarget)
            {
                if (gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_CHEST)
                    reqSkillValue = gameObjTarget->GetGOInfo()->chest.trivialSkillLow - 50;
                else if (gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_GATHERING_NODE)
                    reqSkillValue = gameObjTarget->GetGOInfo()->gatheringNode.trivialSkillLow - 50;

                // Allow one skill-up until respawned
                if (!gameObjTarget->IsInSkillupList(player->GetGUID()) && player->UpdateGatherSkill(skillId, pureSkillValue, reqSkillValue))
                {
                    gameObjTarget->AddToSkillupList(player->GetGUID());
                    if (skillId == SKILL_MINING || skillId == SKILL_HERBALISM || skillId == SKILL_ARCHAEOLOGY)
                        player->GiveGatheringXP();
                }
            }
            else if (itemTarget)
                player->UpdateGatherSkill(skillId, pureSkillValue, reqSkillValue);
        }
    }

    ExecuteLogEffectOpenLock(effIndex, gameObjTarget ? static_cast<Object*>(gameObjTarget) : static_cast<Object*>(itemTarget));
}

void Spell::EffectSummonChangeItem(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster->IsPlayer())
        return;

    Player* player = m_caster->ToPlayer();

    // applied only to using item
    if (!m_CastItem)
        return;

    // ... only to item in own inventory/bank/equip_slot
    if (m_CastItem->GetOwnerGUID() != player->GetGUID())
        return;

    uint32 newitemid = m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType;
    if (!newitemid)
        return;

    uint16 pos = m_CastItem->GetPos();

    Item* pNewItem = Item::CreateItem(newitemid, 1, player);
    if (!pNewItem)
        return;

    for (uint8 j = PERM_ENCHANTMENT_SLOT; j <= TEMP_ENCHANTMENT_SLOT; ++j)
        if (m_CastItem->GetEnchantmentId(EnchantmentSlot(j)))
            pNewItem->SetEnchantment(EnchantmentSlot(j), m_CastItem->GetEnchantmentId(EnchantmentSlot(j)), m_CastItem->GetEnchantmentDuration(EnchantmentSlot(j)), m_CastItem->GetEnchantmentCharges(EnchantmentSlot(j)));

    if (m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) < m_CastItem->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY))
    {
        double lossPercent = 1 - m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) / double(m_CastItem->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY));
        player->DurabilityLoss(pNewItem, lossPercent);
    }

    if (Player::IsInventoryPos(pos))
    {
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true);
        if (msg == EQUIP_ERR_OK)
        {
            m_CastItem->SetInUse(false);
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if (m_CastItem == m_targets.GetItemTarget())
                m_targets.SetItemTarget(nullptr);

            m_CastItem = nullptr;
            m_castItemGUID.Clear();
            m_castItemEntry = 0;

            player->StoreItem(dest, pNewItem, true);
            return;
        }
    }
    else if (Player::IsBankPos(pos))
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanBankItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true);
        if (msg == EQUIP_ERR_OK)
        {
            m_CastItem->SetInUse(false);
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if (m_CastItem == m_targets.GetItemTarget())
                m_targets.SetItemTarget(nullptr);

            m_CastItem = nullptr;
            m_castItemGUID.Clear();
            m_castItemEntry = 0;

            player->BankItem(dest, pNewItem, true);
            return;
        }
    }
    else if (Player::IsEquipmentPos(pos))
    {
        uint16 dest;

        m_CastItem->SetInUse(false);
        player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), true);

        uint8 msg = player->CanEquipItem(m_CastItem->GetSlot(), dest, pNewItem, true);

        if (msg == EQUIP_ERR_OK || msg == EQUIP_ERR_CLIENT_LOCKED_OUT)
        {
            if (msg == EQUIP_ERR_CLIENT_LOCKED_OUT) dest = EQUIPMENT_SLOT_MAINHAND;

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if (m_CastItem == m_targets.GetItemTarget())
                m_targets.SetItemTarget(nullptr);

            m_CastItem = nullptr;
            m_castItemGUID.Clear();
            m_castItemEntry = 0;

            player->EquipItem(dest, pNewItem, true);
            player->AutoUnequipOffhandIfNeed();
            return;
        }
    }

    // fail
    delete pNewItem;
}

void Spell::EffectProficiency(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster->IsPlayer())
        return;
    Player* p_target = m_caster->ToPlayer();

    uint32 subClassMask = m_spellInfo->EquippedItemSubClassMask;
    if (m_spellInfo->EquippedItemClass == ITEM_CLASS_WEAPON && !(p_target->GetWeaponProficiency() & subClassMask))
    {
        p_target->AddWeaponProficiency(subClassMask);
        p_target->SendProficiency(ITEM_CLASS_WEAPON, p_target->GetWeaponProficiency());
    }
    if (m_spellInfo->EquippedItemClass == ITEM_CLASS_ARMOR && !(p_target->GetArmorProficiency() & subClassMask))
    {
        p_target->AddArmorProficiency(subClassMask);
        p_target->SendProficiency(ITEM_CLASS_ARMOR, p_target->GetArmorProficiency());
    }
}

void Spell::EffectSummonType(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 entry = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    if (!entry)
        return;

    SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB);
    if (!properties)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "EffectSummonType: Unhandled summon type %u", m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB);
        return;
    }

    if (!m_originalCaster)
        return;

    int32 duration = m_spellInfo->GetDuration(m_diffMode);
    if (Player* modOwner = m_originalCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);

    TempSummon* summon = nullptr;

    // determine how many units should be summoned
    uint32 numSummons = 1;
    ObjectGuid targetGUID = m_targets.GetUnitTargetGUID();

    // some spells need to summon many units, for those spells number of summons is stored in effect value
    // however so far noone found a generic check to find all of those (there's no related data in summonproperties.dbc
    // and in spell attributes, possibly we need to add a table for those)
    // so here's a list of MiscValueB values, which is currently most generic check
    switch (properties->ID)
    {
        case 64:
        case 61:
        case 1101:
        case 66:
        case 648:
        case 2301:
        case 1061:
        case 1261:
        case 629:
        case 181:
        case 715:
        case 1562:
        case 833:
        case 1161:
        case 3245:
        case 3097:
        case 4057:
            numSummons = (damage > 0) ? damage : 1;
            break;
        case 3210:
            numSummons = damage;
            break;
        case 3238:
            if (destTarget)
                destTarget->m_positionZ += 10.0f;
            break;
        case 3236: // Dire Beast
            m_caster->GetNearPosition(*destTarget, m_caster->GetObjectSize(), m_caster->GetAngle(destTarget->m_positionX, destTarget->m_positionY));
            break;
        case 3655:
            targetGUID = m_caster->GetGuidValue(UNIT_FIELD_TARGET);
            break;
        default:
            numSummons = 1;
            break;
    }

    switch (entry)
    {
        case 55659:
        case 98035:
        case 103673:
        {
            if (AuraEffect const* aurEff = m_caster->GetAuraEffect(214345, EFFECT_0)) // Wilfred's Sigil of Superior Summoning
            {
                if (m_caster->IsPlayer())
                {
                    int32 delay = aurEff->GetAmount() * numSummons;
                    m_caster->ToPlayer()->ModifySpellCooldown(1122, delay);
                    m_caster->ToPlayer()->ModifySpellCooldown(18540, delay);
                }
            }
            break;
        }
        default:
            break;
    }

    switch (properties->Control)
    {
        case SUMMON_CATEGORY_WILD:
        case SUMMON_CATEGORY_ALLY:
        case SUMMON_CATEGORY_UNK:
        {
            switch (properties->Title)
            {
                case SUMMON_TYPE_PET:
                case SUMMON_TYPE_GUARDIAN:
                case SUMMON_TYPE_GUARDIAN2:
                case SUMMON_TYPE_MINION:
                    SummonGuardian(effIndex, entry, properties, numSummons);
                    break;
                // Summons a vehicle, but doesn't force anyone to enter it (see SUMMON_CATEGORY_VEHICLE)
                case SUMMON_TYPE_VEHICLE:
                case SUMMON_TYPE_VEHICLE2:
                case SUMMON_TYPE_GATE:
                case SUMMON_TYPE_STATUE:
                    summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster, targetGUID, m_spellInfo->Id);
                    break;
                case SUMMON_TYPE_TOTEM:
                case SUMMON_TYPE_BANNER:
                {
                    summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster, targetGUID, m_spellInfo->Id);
                    if (!summon || !summon->isTotem())
                        return;

                    switch (m_spellInfo->Id)
                    {
                        case GroundingTotem:
                        {
                            summon->CastSpell(summon, 242900, true);
                            break;
                        }
                        case EarthenWallTotem:
                        {
                            damage = m_caster->CountPctFromMaxHealth(damage);
                            break;
                        }
                        case HealingTideTotem: case LiquidMagmaTotem: case CloudburstTotem: case AncestralProtectionTotem:
                        {
                            damage = m_caster->CountPctFromMaxHealth(10);
                            break;
                        }
                        case VoodooTotem: case WindRushTotem: case EarthbindTotem: case HealingStreamTotem: case CapacitorTotem: case EarthgrabTotem: case SpiritLinkTotem:
                        {
                            damage = m_caster->CountPctFromMaxHealth(2);
                            break;
                        }
                        case 236320: // War Banner (PvP Talent)
                        {
                            damage = m_caster->CountPctFromMaxHealth(5);
                            break;
                        }
                        default:
                            break;
                    }

                    if (damage)                                            // if not spell info, DB values used
                    {
                        summon->SetMaxHealth(damage);
                        summon->SetHealth(damage);
                    }
                    break;
                }
                case SUMMON_TYPE_MINIPET:
                {
                    summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster);
                    if (!summon || !summon->HasUnitTypeMask(UNIT_MASK_MINION))
                        return;

                    summon->SelectLevel(summon->GetCreatureTemplate());       // some summoned creaters have different from 1 DB data for level/hp
                    summon->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, summon->GetCreatureTemplate()->npcflag);
                    summon->SetUInt32Value(UNIT_FIELD_NPC_FLAGS + 1, summon->GetCreatureTemplate()->npcflag2);

                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

                    summon->AI()->EnterEvadeMode();
                    break;
                }
                default:
                {
                    if (properties->Flags & 512 || properties->ID == 2921)
                    {
                        SummonGuardian(effIndex, entry, properties, numSummons);
                        break;
                    }
                    //float radius = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius();

                    TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;

                    for (uint32 count = 0; count < numSummons; ++count)
                    {
                        //Position pos;
                        //if (count == 0)
                        //    pos = static_cast<Position>(*destTarget);
                        //else
                        //    // randomize position for multiple summons
                        //    m_caster->GetRandomPoint(*destTarget, radius, pos);

                        summon = m_originalCaster->SummonCreature(entry, *destTarget, m_originalTargetGUID ? m_originalTargetGUID : targetGUID, summonType, duration, m_spellInfo->Id, properties);
                        if (!summon)
                            continue;

                        switch (properties->ID)
                        {
                            case 3347: // Orphelins
                            {
                                if (int32 slot = properties->Slot)
                                {
                                    if (m_caster->m_SummonSlot[slot] && m_caster->m_SummonSlot[slot] != summon->GetGUID())
                                    {
                                        Creature* oldSummon = m_caster->GetMap()->GetCreature(m_caster->m_SummonSlot[slot]);
                                        if (oldSummon && oldSummon->isSummon())
                                            oldSummon->ToTempSummon()->UnSummon();
                                    }
                                    m_caster->m_SummonSlot[slot] = summon->GetGUID();
                                }
                                break;
                            }
                            default:
                                break;
                        }

                        if (properties->Control == SUMMON_CATEGORY_ALLY)
                        {
                            summon->SetOwnerGUID(m_originalCaster->GetGUID());
                            summon->SetCreatorGUID(m_originalCaster->GetGUID());
                            summon->setFaction(m_originalCaster->getFaction());

                            if (m_originalCaster->IsPlayer())
                                summon->AddUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER);
                        }

                        // Explosive Decoy and Explosive Decoy 2.0
                        if (m_spellInfo->Id == 54359 || m_spellInfo->Id == 62405)
                        {
                            summon->SetMaxHealth(damage);
                            summon->SetHealth(damage);
                            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                        }

                        ExecuteLogEffectSummonObject(effIndex, summon);
                    }
                    return;
                }
            }//switch
            break;
        }
        case SUMMON_CATEGORY_PET:
            SummonGuardian(effIndex, entry, properties, numSummons);
            break;
        case SUMMON_CATEGORY_PUPPET:
            summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster, targetGUID, m_spellInfo->Id);
            break;
        case SUMMON_CATEGORY_VEHICLE:
        {
            // Summoning spells (usually triggered by npc_spellclick) that spawn a vehicle and that cause the clicker
            // to cast a ride vehicle spell on the summoned unit.
            float x, y, z;
            m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);
            summon = m_originalCaster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_caster, targetGUID, m_spellInfo->Id);
            if (!summon || !summon->IsVehicle())
                return;

            // The spell that this effect will trigger. It has SPELL_AURA_CONTROL_VEHICLE
            uint32 spellId = VEHICLE_SPELL_RIDE_HARDCODED;
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValue());
            if (spellInfo && spellInfo->HasAura(SPELL_AURA_CONTROL_VEHICLE))
                spellId = spellInfo->Id;

            // Hard coded enter vehicle spell
            m_originalCaster->CastSpell(summon, spellId, true);

            uint32 faction = properties->Faction;
            if (!faction)
                faction = m_originalCaster->getFaction();

            summon->setFaction(faction);
            break;
        }
    }

    if (summon)
    {
        summon->SetCreatorGUID(m_originalCaster->GetGUID());

        if (m_originalCaster->IsPlayer())
            summon->AddUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER);

        ExecuteLogEffectSummonObject(effIndex, summon);
    }
}

void Spell::EffectLearnSpell(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    if (!unitTarget->IsPlayer())
    {
        if (unitTarget->ToPet())
            EffectLearnPetSpell(effIndex);
        return;
    }

    Player* player = unitTarget->ToPlayer();

    uint32 spellToLearn = (m_spellInfo->Id == 483 || m_spellInfo->Id == 55884) ? damage : m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;
    player->learnSpell(spellToLearn, false);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell: Player %u has learned spell %u from NpcGUID=%u", player->GetGUIDLow(), spellToLearn, m_caster->GetGUIDLow());
}

typedef std::list< std::pair<uint32, ObjectGuid> > DispelList;
void Spell::EffectDispel(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    // Create dispel mask by dispel type
    uint32 dispel_type = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    uint32 dispelMask = SpellInfo::GetDispelMask(DispelType(dispel_type));
    bool LastDispelEff = false;

    // Nature's Cure can dispell all Magic, Curse and poison
    if (m_spellInfo->Id == 88423)
        dispelMask = ((1 << DISPEL_MAGIC) | (1 << DISPEL_CURSE) | (1 << DISPEL_POISON));

    DispelChargesList dispel_list;
    DispelChargesList predicted_dispel_list;
    unitTarget->GetDispellableAuraList(m_caster, dispelMask, dispel_list);

    if (!hasPredictedDispel)
    {
        uint32 allEffectDispelMask = m_spellInfo->GetSimilarEffectsMiscValueMask(SPELL_EFFECT_DISPEL, m_caster);
        unitTarget->GetDispellableAuraList(m_caster, allEffectDispelMask, predicted_dispel_list);

        if (!predicted_dispel_list.empty())
            hasPredictedDispel++;

        hasPredictedDispel++;
    }

    for (uint32 eff = effIndex + 1; eff < MAX_SPELL_EFFECTS; ++eff)
    {
        if (m_spellInfo->EffectMask < uint32(1 << eff))
            break;

        if (m_spellInfo->Effects[eff]->Effect != 0)
        {
            if (m_spellInfo->Effects[eff]->IsEffect(SPELL_EFFECT_DISPEL))
                break;
            continue;
        }
        LastDispelEff = true;
        break;
    }

    if (LastDispelEff && hasPredictedDispel == 1)
        if (Player* player = m_caster->ToPlayer())
            if (m_spellInfo->Cooldowns.RecoveryTime <= 8000 && m_spellInfo->IsPositive())
                player->RemoveSpellCooldown(m_spellInfo->Id, true);

    if (dispel_list.empty())
        return;

    if (Unit::AuraEffectList const* mTotalAuraList = m_caster->GetAuraEffectsByType(SPELL_AURA_DUMMY))
        for (Unit::AuraEffectList::const_iterator i = mTotalAuraList->begin(); i != mTotalAuraList->end(); ++i)
            if ((*i)->GetMiscValue() == 11 && (*i)->GetSpellInfo()->GetMisc()->MiscData.IconFileDataID == m_spellInfo->GetMisc()->MiscData.IconFileDataID)
                (*i)->SetAmount(m_spellInfo->Id);

    // Ok if exist some buffs for dispel try dispel it
    DispelChargesList success_list;

    std::list<uint32> spellFails;
    // dispel N = damage buffs (or while exist buffs for dispel)
    for (int32 count = 0; count < damage && !dispel_list.empty();)
    {
        // Random select buff for dispel
        auto itr = dispel_list.begin();
        std::advance(itr, urand(0, dispel_list.size() - 1));

        int32 chance = itr->first->CalcDispelChance(unitTarget, !unitTarget->IsFriendlyTo(m_caster));
        // 2.4.3 Patch Notes: "Dispel effects will no longer attempt to remove effects that have 100% dispel resistance."
        if (!chance)
        {
            dispel_list.erase(itr);
            continue;
        }
        if (roll_chance_i(chance))
        {
            success_list.push_back(std::make_pair(itr->first, 1));
            --itr->second;
            if (itr->second <= 0)
                dispel_list.erase(itr);
        }
        else
            spellFails.push_back(itr->first->GetId());               // Spell Id
        ++count;
    }

    if (!spellFails.empty())
        m_caster->SendDispelFailed(unitTarget->GetGUID(), m_spellInfo->Id, spellFails);

    if (success_list.empty())
        return;

    std::list<uint32> spellSuccess;

    for (auto& itr : success_list)
    {
        spellSuccess.push_back(itr.first->GetId());
        unitTarget->RemoveAurasDueToSpellByDispel(itr.first->GetId(), m_spellInfo->Id, itr.first->GetCasterGUID(), m_caster,
                                                  itr.second);
    }

    m_count_dispeling += spellSuccess.size();
    m_caster->SendDispelLog(unitTarget->GetGUID(), m_spellInfo->Id, spellSuccess, false, false);

    // On success dispel
    // Devour Magic
    if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_PET_ABILITY && m_spellInfo->Categories.Category == SPELLCATEGORY_DEVOUR_MAGIC)
        m_caster->CastSpell(m_caster, 19658, true);

    CallScriptSuccessfulDispel(effIndex);
}

void Spell::EffectDualWield(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    unitTarget->SetCanDualWield(true);
    if (unitTarget->IsCreature())
        unitTarget->ToCreature()->UpdateDamagePhysical(OFF_ATTACK);
}

void Spell::EffectDistract(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    // Check for possible target
    if (!unitTarget || unitTarget->isInCombat())
        return;

    // target must be OK to do this
    if (unitTarget->HasUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_STUNNED | UNIT_STATE_FLEEING))
        return;

    unitTarget->SetFacingTo(unitTarget->GetAngle(destTarget));
    unitTarget->ClearUnitState(UNIT_STATE_MOVING);

    if (unitTarget->IsCreature())
        unitTarget->GetMotionMaster()->MoveDistract(damage * IN_MILLISECONDS);
}

void Spell::EffectPickPocket(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!m_caster->IsPlayer())
        return;

    // victim must be creature and attackable
    if (!unitTarget || !unitTarget->IsCreature()/*|| m_caster->IsFriendlyTo(unitTarget)*/)
        return;

    // victim have to be alive and humanoid or undead
    if (unitTarget->isAlive() && (unitTarget->GetCreatureTypeMask() &CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) != 0)
        m_caster->ToPlayer()->SendLoot(unitTarget->GetGUID(), LOOT_PICKPOCKETING);
}

void Spell::EffectAddFarsight(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster->IsPlayer())
        return;

    float radius = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius();
    int32 duration = m_spellInfo->GetDuration(m_diffMode);
    // Caster not in world, might be spell triggered from aura removal
    if (!m_caster->IsInWorld())
        return;

    auto dynObj = new DynamicObject(true);
    if (!dynObj->CreateDynamicObject(sObjectMgr->GetGenerator<HighGuid::DynamicObject>()->Generate(), m_caster, m_spellInfo->Id, *destTarget, radius, DYNAMIC_OBJECT_FARSIGHT_FOCUS))
    {
        delete dynObj;
        return;
    }

    SetSpellDynamicObject(dynObj->GetGUID());
    dynObj->SetDuration(duration);
    dynObj->SetCasterViewpoint();
}

void Spell::EffectUntrainTalents(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || m_caster->IsPlayer())
        return;

    ObjectGuid guid = m_caster->GetGUID();
    if (!guid.IsEmpty()) // the trainer is the caster
        unitTarget->ToPlayer()->SendTalentWipeConfirm(guid, RESPEC_TYPE_TALENTS);
}

void Spell::EffectTeleUnitsFaceCaster(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    if (unitTarget->isInFlight())
        return;

    //float fx, fy, fz;
    //m_caster->GetClosePoint(fx, fy, fz, unitTarget->GetObjectSize(), m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster));
    Position pos;
    m_caster->GetNearPosition(pos, m_caster->GetObjectSize(), m_caster->GetAngle(unitTarget));

    // Earthen Vortex, Morchok, Dragon Soul
    // Prevent dropping into textures
    switch (m_spellInfo->Id)
    {
        case 103821:
            pos.m_positionX += 8.0f;
            break;
        default:
            break;
    }

    unitTarget->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), -m_caster->GetOrientation(), unitTarget == m_caster);
}

void Spell::EffectLearnSkill(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget->IsPlayer())
        return;

    if (damage < 0)
        return;

    uint32 skillid = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    SkillRaceClassInfoEntry const* rcEntry = sDB2Manager.GetSkillRaceClassInfo(skillid, unitTarget->getRace(), unitTarget->getClass());
    if (!rcEntry)
        return;

    SkillTiersEntry const* tier = sObjectMgr->GetSkillTier(rcEntry->SkillTierID);
    if (!tier)
        return;

    uint16 skillval = unitTarget->ToPlayer()->GetPureSkillValue(skillid);
    uint16 skillmaxval = unitTarget->ToPlayer()->GetPureMaxSkillValue(skillid);
    uint16 tiermaxval = tier->Value[RoundingFloatValue(damage) - 1];
    uint16 skillStep = unitTarget->ToPlayer()->GetSkillStep(skillid);
    uint16 learnSkillStep = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValue();

    if (skillmaxval > tiermaxval) // Don`t down grade max skill
        tiermaxval = skillmaxval;

    if (skillStep > learnSkillStep)
        learnSkillStep = skillStep;

    unitTarget->ToPlayer()->SetSkill(skillid, learnSkillStep, std::max<uint16>(skillval, 1), tiermaxval);

    // Archaeology
    if (skillid == SKILL_ARCHAEOLOGY && !skillval && sWorld->getBoolConfig(CONFIG_ARCHAEOLOGY_ENABLED))
    {
        unitTarget->ToPlayer()->GenerateResearchSites();
        unitTarget->ToPlayer()->GenerateResearchProjects();
    }
}

void Spell::EffectIncreaseSkill(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget->IsPlayer())
        return;

    if (damage < 0)
        return;

    uint32 skillid = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    uint32 limitValue = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB;
    uint32 skillValue = unitTarget->ToPlayer()->GetPureSkillValue(skillid) + m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValue();
    uint32 maxSkillValue = unitTarget->ToPlayer()->GetMaxSkillValue(skillid);

    if (skillValue > maxSkillValue)
        return;

    if (skillValue > limitValue)
        return;

    SkillRaceClassInfoEntry const* rcEntry = sDB2Manager.GetSkillRaceClassInfo(skillid, unitTarget->getRace(), unitTarget->getClass());
    if (!rcEntry)
        return;

    unitTarget->ToPlayer()->SetSkill(skillid, unitTarget->ToPlayer()->GetSkillStep(skillid), skillValue, maxSkillValue);
}

void Spell::EffectPlayMovie(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget->IsPlayer())
        return;

    uint32 movieId = GetSpellInfo()->GetEffect(effIndex, m_diffMode)->MiscValue;
    if (!sMovieStore.LookupEntry(movieId))
        return;

    unitTarget->ToPlayer()->SendMovieStart(movieId);
}

void Spell::EffectTradeSkill(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster->IsPlayer())
        return;
    // uint32 skillid =  m_spellInfo->GetEffect(i, m_diffMode).MiscValue;
    // uint16 skillmax = unitTarget->ToPlayer()->(skillid);
    // m_caster->ToPlayer()->SetSkill(skillid, skillval?skillval:1, skillmax+75);
}

void Spell::EffectEnchantItemPerm(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    auto caster = m_caster->ToPlayer();
    if (!caster)
        return;

    if (!itemTarget)
        return;

    // Handle vellums
    if (itemTarget->IsVellum())
    {
        // destroy one vellum from stack
        uint32 count = 1;
        caster->DestroyItemCount(itemTarget, count, true);
        unitTarget = caster;

        uint32 itemID = m_spellInfo->GetEffect(effIndex, m_diffMode)->ItemType;
        uint32 itemLevelFromBonus = 0;
        uint32 itemBonusTreeMod = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB;
        if (!itemBonusTreeMod && m_CastItem)
            itemBonusTreeMod = m_CastItem->GetUInt32Value(ITEM_FIELD_CONTEXT);

        if (ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemID))
            if (pProto->GetQuality() == ITEM_QUALITY_LEGENDARY && pProto->GetExpansion() == EXPANSION_LEGION)
                itemBonusTreeMod = 0;

        std::vector<uint32> bonusListIDs = sDB2Manager.GetItemBonusTree(itemID, itemBonusTreeMod, itemLevelFromBonus);

        // and add a scroll
        DoCreateItem(effIndex, itemID, bonusListIDs);
        itemTarget = nullptr;
        m_targets.SetItemTarget(nullptr);
    }
    else
    {
        // do not increase skill if vellum used
        if (!(m_CastItem && m_CastItem->GetTemplate()->GetFlags() & ITEM_FLAG_NO_REAGENT_COST))
            caster->UpdateCraftSkill(m_spellInfo->Id);

        uint32 enchant_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
        if (!enchant_id)
            return;

        SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!pEnchant)
            return;

        // item can be in trade slot and have owner diff. from caster
        Player* item_owner = itemTarget->GetOwner();
        if (!item_owner)
            return;

        if (item_owner != caster && !AccountMgr::IsPlayerAccount(caster->GetSession()->GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
        {
            sLog->outCommand(caster->GetSession()->GetAccountId(), "GM %s (Account: %u) enchanting(perm): %s (Entry: %d) for player: %s (Account: %u)",
                             caster->GetName(), caster->GetSession()->GetAccountId(),
                             itemTarget->GetTemplate()->GetName()->Str[caster->GetSession()->GetSessionDbLocaleIndex()], itemTarget->GetEntry(),
                             item_owner->GetName(), item_owner->GetSession()->GetAccountId());
        }

        EnchantmentSlot slot = pEnchant->RequiredSkillID == SKILL_ENGINEERING ? USE_ENCHANTMENT_SLOT : PERM_ENCHANTMENT_SLOT;

        // remove old enchanting before applying new if equipped
        item_owner->ApplyEnchantment(itemTarget, slot, false);

        itemTarget->SetEnchantment(slot, enchant_id, 0, 0, m_caster->GetGUID());

        // add new enchanting if equipped
        item_owner->ApplyEnchantment(itemTarget, slot, true);

        item_owner->RemoveTradeableItem(itemTarget);
        itemTarget->ClearSoulboundTradeable(item_owner);
    }
}

void Spell::EffectEnchantItemPrismatic(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    auto caster = m_caster->ToPlayer();
    if (!caster)
        return;

    if (!itemTarget)
        return;

    uint32 enchant_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    if (!enchant_id)
        return;

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if (!pEnchant)
        return;

    bool add_socket = false;
    for (auto i : pEnchant->Effect)
    {
        if (i == ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET)
        {
            add_socket = true;
            break;
        }
    }

    if (!add_socket)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::EffectEnchantItemPrismatic: attempt apply enchant spell %u with SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC (%u) but without ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET (%u), not suppoted yet.", m_spellInfo->Id, SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC, ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET);
        return;
    }

    // item can be in trade slot and have owner diff. from caster
    Player* item_owner = itemTarget->GetOwner();
    if (!item_owner)
        return;

    if (item_owner != caster && !AccountMgr::IsPlayerAccount(caster->GetSession()->GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
    {
        sLog->outCommand(caster->GetSession()->GetAccountId(), "GM %s (Account: %u) enchanting(perm): %s (Entry: %d) for player: %s (Account: %u)",
                        caster->GetName(), caster->GetSession()->GetAccountId(),
                         itemTarget->GetTemplate()->GetName()->Str[caster->GetSession()->GetSessionDbLocaleIndex()], itemTarget->GetEntry(),
                         item_owner->GetName(), item_owner->GetSession()->GetAccountId());
    }

    // remove old enchanting before applying new if equipped
    item_owner->ApplyEnchantment(itemTarget, PRISMATIC_ENCHANTMENT_SLOT, false);

    itemTarget->SetEnchantment(PRISMATIC_ENCHANTMENT_SLOT, enchant_id, 0, 0, m_caster->GetGUID());

    // add new enchanting if equipped
    item_owner->ApplyEnchantment(itemTarget, PRISMATIC_ENCHANTMENT_SLOT, true);

    item_owner->RemoveTradeableItem(itemTarget);
    itemTarget->ClearSoulboundTradeable(item_owner);
}

void Spell::EffectEnchantItemTmp(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    auto caster = m_caster->ToPlayer();
    if (!caster)
        return;

    if (!itemTarget)
        return;

    uint32 enchant_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    if (!enchant_id)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have 0 as enchanting id", m_spellInfo->Id, effIndex);
        return;
    }

    if (!sSpellItemEnchantmentStore.LookupEntry(enchant_id))
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have not existed enchanting id %u ", m_spellInfo->Id, effIndex, enchant_id);
        return;
    }

    // select enchantment duration
    uint32 duration;

    // other cases with this SpellVisual already selected
    if (m_spellInfo->GetSpellVisual() == 215)
        duration = 1800;                                    // 30 mins
    // some fishing pole bonuses except Glow Worm which lasts full hour
    else if (m_spellInfo->GetSpellVisual() == 563 && m_spellInfo->Id != 64401)
        duration = 600;                                     // 10 mins
    else if (m_spellInfo->Id == 29702)
        duration = 300;                                     // 5 mins
    else if (m_spellInfo->Id == 37360)
        duration = 300;                                     // 5 mins
    // default case
    else
        duration = 3600;                                    // 1 hour

    // item can be in trade slot and have owner diff. from caster
    Player* item_owner = itemTarget->GetOwner();
    if (!item_owner)
        return;

    if (item_owner != caster && !AccountMgr::IsPlayerAccount(caster->GetSession()->GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
    {
        sLog->outCommand(caster->GetSession()->GetAccountId(), "GM %s (Account: %u) enchanting(temp): %s (Entry: %d) for player: %s (Account: %u)",
                         caster->GetName(), caster->GetSession()->GetAccountId(),
                         itemTarget->GetTemplate()->GetName()->Str[caster->GetSession()->GetSessionDbLocaleIndex()], itemTarget->GetEntry(),
                         item_owner->GetName(), item_owner->GetSession()->GetAccountId());
    }

    // remove old enchanting before applying new if equipped
    item_owner->ApplyEnchantment(itemTarget, TEMP_ENCHANTMENT_SLOT, false);

    itemTarget->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, duration * 1000, 0, m_caster->GetGUID());

    // add new enchanting if equipped
    item_owner->ApplyEnchantment(itemTarget, TEMP_ENCHANTMENT_SLOT, true);
}

void Spell::EffectTameCreature(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (m_caster->GetPetGUID())
        return;

    Player* player = m_caster->ToPlayer();
    if (!unitTarget || !player)
        return;

    if (!unitTarget->IsCreature())
        return;

    Creature* creatureTarget = unitTarget->ToCreature();

    if (creatureTarget->isPet())
        return;

    if (m_caster->getClass() != CLASS_HUNTER)
        return;

    // cast finish successfully
    //SendChannelUpdate(0);
    finish();

    Pet* pet = m_caster->CreateTamedPetFrom(creatureTarget, m_spellInfo->Id);
    if (!pet)                                               // in very specific state like near world end/etc.
        return;

    // "kill" original creature
    creatureTarget->DespawnOrUnsummon();

    uint8 level = m_caster->getLevel();

    // prepare visual effect for levelup
    pet->SetLevel(level - 1);

    // add to world
    pet->GetMap()->AddToMap(pet->ToCreature());

    // visual effect for levelup
    pet->SetLevel(level);
    pet->SetEffectiveLevel(m_caster->GetEffectiveLevel());

    // caster have pet now
    m_caster->SetMinion(pet, true);

    pet->SavePetToDB();
    player->PetSpellInitialize();
    player->GetSession()->SendStablePet();
}

void Spell::EffectSummonPet(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* owner = nullptr;
    if (m_originalCaster)
    {
        owner = m_originalCaster->ToPlayer();
        if (!owner && m_originalCaster->ToCreature()->isTotem())
            owner = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself();
    }

    uint32 petentry = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    if (!owner)
    {
        if (SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(67))
            SummonGuardian(effIndex, petentry, properties, 1);
        return;
    }

    Pet* OldSummon = owner->GetPet();

    // if pet requested type already exist
    if (OldSummon)
    {
        if (petentry == 0)
        {
            // pet in corpse state can't be summoned
            if (OldSummon->isDead())
                return;

            ASSERT(OldSummon->GetMap() == owner->GetMap());

            float px, py, pz;
            owner->GetClosePoint(px, py, pz, OldSummon->GetObjectSize());
            OldSummon->NearTeleportTo(px, py, pz, OldSummon->GetOrientation());

            if (owner->IsPlayer() && OldSummon->isControlled())
                owner->ToPlayer()->PetSpellInitialize();

            return;
        }

        if (owner->IsPlayer())
            owner->ToPlayer()->RemovePet(OldSummon);
        else
            return;
    }

    auto slot = PetSlot(RoundingFloatValue(m_spellInfo->GetEffect(effIndex, m_diffMode)->BasePoints));
    owner->m_currentSummonedSlot = slot;

    Position pos;
    if (canHitTargetInLOS && owner->ToCreature())
        owner->GetNearPoint2D(pos, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    else
        owner->GetFirstCollisionPosition(pos, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

    Pet* pet = owner->SummonPet(petentry, pos.m_positionX, pos.m_positionY, pos.m_positionZ, owner->GetOrientation(), SUMMON_PET, 0, m_spellInfo->Id);
    if (!pet)
        return;

    if (m_caster->IsCreature())
    {
        if (m_caster->ToCreature()->isTotem())
            pet->SetReactState(REACT_AGGRESSIVE);
        else
            pet->SetReactState(REACT_DEFENSIVE);
    }

    // generate new name for summon pet
    if (petentry)
    {
        std::string new_name = sObjectMgr->GeneratePetName(petentry);
        if (!new_name.empty())
            pet->SetName(new_name);
    }
    pet->SetSlot(slot);

    ExecuteLogEffectSummonObject(effIndex, pet);
}

void Spell::EffectLearnPetSpell(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    if (unitTarget->ToPlayer())
    {
        EffectLearnSpell(effIndex);
        return;
    }
    Pet* pet = unitTarget->ToPet();
    if (!pet)
        return;

    SpellInfo const* learn_spellproto = sSpellMgr->GetSpellInfo(m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell);
    if (!learn_spellproto)
        return;

    pet->learnSpell(learn_spellproto->Id);
    pet->SavePetToDB();
    if (Player* player = pet->GetOwner()->ToPlayer())
        player->PetSpellInitialize();
}

void Spell::EffectTaunt(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    // this effect use before aura Taunt apply for prevent taunt already attacking target
    // for spell as marked "non effective at already attacking target"
    if (!unitTarget || !unitTarget->CanHaveThreatList() || unitTarget->getVictim() == m_caster)
    {
        SendCastResult(SPELL_FAILED_DONT_REPORT);
        return;
    }

    // Also use this effect to set the taunter's threat to the taunted creature's highest value
    if (unitTarget->getThreatManager().getCurrentVictim())
    {
        float myThreat = unitTarget->getThreatManager().getThreat(m_caster);
        float itsThreat = unitTarget->getThreatManager().getCurrentVictim()->getThreat();
        if (itsThreat > myThreat)
            unitTarget->getThreatManager().addThreat(m_caster, itsThreat - myThreat);
    }

    //Set aggro victim to caster
    if (!unitTarget->getThreatManager().getOnlineContainer().empty())
        if (HostileReference* forcedVictim = unitTarget->getThreatManager().getOnlineContainer().getReferenceByTarget(m_caster))
            unitTarget->getThreatManager().setCurrentVictim(forcedVictim);

    if (unitTarget->ToCreature()->IsAIEnabled && !unitTarget->ToCreature()->HasReactState(REACT_PASSIVE))
        unitTarget->ToCreature()->AI()->AttackStart(m_caster);
}

void Spell::EffectWeaponDmg(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive())
        return;

    Unit* _caster = m_caster;

    if (m_originalCaster && m_spellInfo->HasAttribute(SPELL_ATTR0_CU_BONUS_FROM_ORIGINAL_CASTER))
        _caster = m_originalCaster;

    // multiple weapon dmg effect workaround
    // execute only the last weapon damage
    // and handle all effects at once
    for (uint32 j = effIndex + 1; j < MAX_SPELL_EFFECTS; ++j)
    {
        if (m_spellInfo->EffectMask < uint32(1 << j))
            break;

        switch (m_spellInfo->Effects[j]->Effect)
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                if ((m_spellInfo->Effects[effIndex]->TargetA.GetTarget() == m_spellInfo->Effects[j]->TargetA.GetTarget() || m_spellInfo->Effects[effIndex]->TargetA.GetTarget() == TARGET_NONE || m_spellInfo->Effects[j]->TargetA.GetTarget() == TARGET_NONE)
                    && (m_spellInfo->Effects[effIndex]->TargetB.GetTarget() == m_spellInfo->Effects[j]->TargetB.GetTarget() || m_spellInfo->Effects[effIndex]->TargetB.GetTarget() == TARGET_NONE || m_spellInfo->Effects[j]->TargetB.GetTarget() == TARGET_NONE))
                    return;     // we must calculate only at last weapon effect
                break;
        }
    }

    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell EffectWeaponDmg spellid %u in Effect(%u) m_damage %i, damage %i", m_spellInfo->Id, effIndex, m_damage, damage);

    // some spell specific modifiers
    float totalDamagePercentMod = 1.0f;                    // applied to final bonus+weapon damage
    int32 fixed_bonus = 0;
    bool calcAllEffects = true;

    switch (m_spellInfo->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (m_spellInfo->Id)
            {
                case 69055:     // Saber Lash
                {
                    uint32 count = 0;
                    for (auto& ihit : m_UniqueTargetInfo)
                        if (ihit->effectMask & (1 << effIndex))
                            ++count;

                    totalDamagePercentMod /= count;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Skyshatter Harness item set bonus
            // Stormstrike
            if (AuraEffect* aurEff = _caster->IsScriptOverriden(m_spellInfo, 5634))
                _caster->CastSpell(_caster, 38430, true, nullptr, aurEff);
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            switch (m_spellInfo->Id)
            {
                case 198670: // Piercing Shot
                {
                    calcAllEffects = false;
                    break;
                }
                default:
                    break;
            }
            break;
        }
    }

    bool normalized = false;
    float weaponDamagePercentMod = 0.0f;
    for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        if (m_spellInfo->EffectMask < uint32(1 << j))
            break;

        switch (m_spellInfo->Effects[j]->Effect)
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                fixed_bonus += CalculateDamage(j, unitTarget);
                break;
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                fixed_bonus += CalculateDamage(j, unitTarget);
                normalized = true;
                break;
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                if (!calcAllEffects && effIndex == j + 1 || calcAllEffects)
                    weaponDamagePercentMod += m_spellInfo->GetEffect(j, m_diffMode)->CalcValue(_caster, nullptr, unitTarget, m_CastItem, false, nullptr, GetComboPoints()) / 100.0f;
                    //weaponDamagePercentMod += CalculateDamage(j, unitTarget) / 100.0f;
                break;
            default:
                break;                                      // not weapon damage effect, just skip
        }
    }

    // apply to non-weapon bonus weapon total pct effect, weapon total flat effect included in weapon damage
    if (fixed_bonus)
    {
        UnitMods unitMod;
        switch (m_attackType)
        {
            default:
            case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
            case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
            case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
        }

        float weapon_total_pct = 1.0f;
        if (m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask & SPELL_SCHOOL_MASK_NORMAL)
            weapon_total_pct = _caster->GetModifierValue(unitMod, TOTAL_PCT);

        if (fixed_bonus)
            fixed_bonus = int32(fixed_bonus * weapon_total_pct);
    }

    int32 weaponDamage = _caster->CalculateDamage(m_attackType, normalized, m_spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL, unitTarget);
    bool calculateWPD = true;

    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell EffectWeaponDmg spellid %u in Effect(%u) fixed_bonus %i, weaponDamagePercentMod %f weaponDamage %i", m_spellInfo->Id, effIndex, fixed_bonus, weaponDamagePercentMod, weaponDamage);
    #endif

    // Sequence is important
    for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        if (m_spellInfo->EffectMask < uint32(1 << j))
            break;

        // We assume that a spell have at most one fixed_bonus
        // and at most one weaponDamagePercentMod
        switch (m_spellInfo->Effects[j]->Effect)
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                weaponDamage += fixed_bonus;
                break;
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                if (calculateWPD)
                {
                    weaponDamage = int32(weaponDamage * weaponDamagePercentMod);
                    calculateWPD = false;
                }
            default:
                break;                                      // not weapon damage effect, just skip
        }
    }

    weaponDamage = int32(weaponDamage * totalDamagePercentMod);

    // prevent negative damage
    uint32 eff_damage(std::max(weaponDamage, 0));

    // Add melee damage bonuses (also check for negative)
    uint32 _damage = _caster->MeleeDamageBonusDone(unitTarget, eff_damage, m_attackType, m_spellInfo, effIndex);

    m_damage += _damage;

    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell EffectWeaponDmg spellid %u in Effect(%u) weaponDamage %i, _damage %i m_damage %i effectHandleMode %i ComboPoints %i", m_spellInfo->Id, effIndex, weaponDamage, _damage, m_damage, effectHandleMode, GetComboPoints());
    #endif

    switch (m_spellInfo->Id)
    {
        case 32175:  // Stormstrike
        case 32176:  // Stormstrike
        case 115357: // Windstrike
        case 115360: // Windstrike
        {
            if (m_triggeredByAura && m_triggeredByAura->GetId() == 198367)
                m_damage = CalculatePct(m_damage, m_triggeredByAura->GetAmount());
            break;
        }
        case 224266: // Templar's Verdict
        case 224239: // Divine Storm
        {
            if (Unit* owner = m_caster->GetOwner())
            {
                if (AuraEffect* eff = owner->GetAuraEffect(186788, EFFECT_0))
                    m_damage = CalculatePct(m_damage, eff->GetAmount());
            }
            break;
        }
        case 187708: // Carve
        case 212436: // Butchery
        {
            if (AuraEffect const* aurEff = _caster->GetAuraEffect(203673, EFFECT_0)) // Hellcarver
                if (GetTargetCount() > 1)
                    AddPct(m_damage, aurEff->GetAmount() * (GetTargetCount() - 1));
            break;
        }
        case 66196: // Frost Strike
        case 222026: // Frost Strike
        {
            if (AuraEffect const* aurEff = _caster->GetAuraEffect(204132, EFFECT_0)) // Tundra Stalker (Honor Talent)
                if (unitTarget->HasAuraWithMechanic(1 << MECHANIC_SNARE))
                    AddPct(m_damage, aurEff->GetAmount());
            break;
        }
        case 49998: // Death Strike
        {
            float pctDamage = m_caster->CanPvPScalar() ? m_spellInfo->GetEffect(EFFECT_2, m_diffMode)->CalcValue(_caster) / 2 : m_spellInfo->GetEffect(EFFECT_2, m_diffMode)->CalcValue(_caster);
            int32 lastTime = m_spellInfo->GetEffect(EFFECT_3, m_diffMode)->CalcValue(_caster);
            int32 pctHeal = m_spellInfo->GetEffect(EFFECT_4, m_diffMode)->CalcValue(_caster);
            float bp = _caster->CountPctFromMaxHealth(pctHeal) + CalculatePct(_caster->GetDamageTakenInPastSecs(lastTime, true, true), pctDamage);

            _caster->CastCustomSpell(_caster, 45470, &bp, nullptr, nullptr, false);
            break;
        }
        case 53: // Backstab
        {
            if (!unitTarget->HasInArc(static_cast<float>(M_PI/2), _caster))
                AddPct(m_damage, m_spellInfo->Effects[EFFECT_3]->CalcValue(_caster));
            break;
        }
        case 33917: // Mangle
        {
            if (m_caster->HasAura(231064) && unitTarget->HasAuraWithMechanic((1 << MECHANIC_BLEED)))
                AddPct(m_damage, m_spellInfo->GetEffect(EFFECT_2, m_diffMode)->BasePoints);
            break;
        }
        case 60103: // Lava Lash
        {
            if (!_caster->IsPlayer())
                break;

            //Bonus 4P shaman
            if (_caster->HasAura(131554))
            {
                if (Item* offItem = _caster->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                {
                    // Flametongue
                    if (offItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 2)
                        AddPct(m_damage, 40);
                }
            }
            break;
        }
    }
}

void Spell::EffectThreat(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive() || !m_caster->isAlive())
        return;

    if (!unitTarget->CanHaveThreatList())
        return;

    unitTarget->AddThreat(m_caster, float(damage));
}

void Spell::EffectHealMaxHealth(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive())
        return;

    int32 addhealth = 0;

    // damage == 0 - heal for caster max health
    if (damage == 0)
        addhealth = m_caster->GetMaxHealth(unitTarget);
    else
        addhealth = unitTarget->GetMaxHealth(m_caster) - unitTarget->GetHealth(m_caster);

    addhealth = m_caster->SpellHealingBonusDone(m_caster, m_spellInfo, addhealth, HEAL, effIndex);
    addhealth = unitTarget->SpellHealingBonusTaken(m_caster, m_spellInfo, addhealth, HEAL, effIndex);

    m_healing += addhealth;
}

void Spell::EffectInterruptCast(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive())
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectInterruptCast Id %u", m_spellInfo->Id);

    // TODO: not all spells that used this effect apply cooldown at school spells
    // also exist case: apply cooldown to interrupted cast only and to all spells
    // there is no CURRENT_AUTOREPEAT_SPELL spells that can be interrupted
    for (uint32 i = CURRENT_GENERIC_SPELL; i <= CURRENT_CHANNELED_SPELL; ++i)
    {
        if (Spell* spell = unitTarget->GetCurrentSpell(CurrentSpellTypes(i)))
        {
            SpellInfo const* curSpellInfo = spell->m_spellInfo;
            // check if we can interrupt spell
            if ((spell->getState() == SPELL_STATE_CASTING || (spell->getState() == SPELL_STATE_PREPARING && spell->GetCastTime() > 0.0f))
                && (curSpellInfo->Categories.PreventionType == SPELL_PREVENTION_TYPE_SILENCE || curSpellInfo->Categories.PreventionType == SPELL_PREVENTION_TYPE_UNK3 || curSpellInfo->Categories.PreventionType == SPELL_PREVENTION_TYPE_UNK5)
                /* && (curSpellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT || curSpellInfo->HasChannelInterruptFlag(CHANNEL_INTERRUPT_FLAG_INTERRUPT)) */)
            {
                // Prevent interrupt spell if need
                bool prevent = false;
                if (Unit::AuraEffectList const* mPreventInterrupt = unitTarget->GetAuraEffectsByType(SPELL_AURA_PREVENT_INTERRUPT))
                    for (Unit::AuraEffectList::const_iterator impi = mPreventInterrupt->begin(); impi != mPreventInterrupt->end(); ++impi)
                        if ((*impi)->IsAffectingSpell(curSpellInfo))
                            prevent = true;

                if (prevent)
                    break;

                if (m_originalCaster)
                    unitTarget->ProhibitSpellSchool(curSpellInfo->GetSchoolMask(), unitTarget->ModSpellDuration(m_spellInfo, unitTarget, m_spellInfo->GetDuration(m_diffMode), false, 1 << effIndex, m_originalCaster));

                if (auto creature = unitTarget->ToCreature())
                    if (creature->IsAIEnabled)
                        creature->AI()->OnInterruptCast(m_caster, m_spellInfo->Id, curSpellInfo->Id, curSpellInfo->GetSchoolMask());

                ExecuteLogEffectInterruptCast(effIndex, unitTarget, curSpellInfo->Id);
                unitTarget->InterruptSpell(CurrentSpellTypes(i), false);
                unitTarget->SendLossOfControl(m_caster, m_spellInfo->Id, m_spellInfo->GetDuration(m_diffMode), m_spellInfo->GetDuration(m_diffMode), m_spellInfo->GetEffectMechanic(effIndex), curSpellInfo->GetSchoolMask(), LOC_SCHOOL_INTERRUPT, true);

                DamageInfo dmgInfoProc = DamageInfo(m_caster, unitTarget, 0, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, 0);
                m_caster->ProcDamageAndSpell(unitTarget, PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, (PROC_EX_INTERRUPT | PROC_EX_NORMAL_HIT), &dmgInfoProc, BASE_ATTACK, m_spellInfo, nullptr, nullptr, this);
            }
        }
    }
}

void Spell::EffectSummonObjectWild(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 gameobject_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    GameObject* pGameObj = sObjectMgr->IsStaticTransport(gameobject_id) ? new StaticTransport : new GameObject;

    WorldObject* target = focusObject;
    if (!target)
        target = m_caster;

    float x, y, z;
    if (m_targets.HasDst())
        destTarget->GetPosition(x, y, z);
    else
        m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);

    Map* map = target->GetMap();

    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(target->GetOrientation(), 0.f, 0.f));
    if (!pGameObj->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), gameobject_id, map, m_caster->GetPhaseMask(), Position(x, y, z, target->GetOrientation()), quat, 100, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetTratsport(m_caster->GetTransport());

    int32 duration = m_spellInfo->GetDuration(m_diffMode);

    pGameObj->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    ExecuteLogEffectSummonObject(effIndex, pGameObj);

    // Wild object not have owner and check clickable by players
    map->AddToMap(pGameObj);

    if (pGameObj->GetGoType() == GAMEOBJECT_TYPE_NEW_FLAG_DROP && m_caster->IsPlayer())
    {
        auto player = m_caster->ToPlayer();
        auto bg = player->GetBattleground();

        switch (pGameObj->GetMapId())
        {
            case 489:                                       //WS
                if (bg && (bg->GetTypeID(true) == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble || bg->GetTypeID(true) == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongSix || bg->GetTypeID(true) == MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch) && bg->GetStatus() == STATUS_IN_PROGRESS)
                {
                    uint32 team = TEAM_ALLIANCE;
                    if (player->GetBGTeamId() == team)
                        team = TEAM_HORDE;

                    static_cast<BattlegroundWarsongGulch*>(bg)->SetDroppedFlagGUID(pGameObj->GetGUID(), team, player->GetGUID());
                }
                break;
            case 726:                                       //TP
                if (bg && bg->GetTypeID(true) == MS::Battlegrounds::BattlegroundTypeId::BattlegroundTwinPeaks && bg->GetStatus() == STATUS_IN_PROGRESS)
                {
                    uint32 team = TEAM_ALLIANCE;
                    if (player->GetBGTeamId() == team)
                        team = TEAM_HORDE;

                    static_cast<BattlegroundTwinPeaks*>(bg)->SetDroppedFlagGUID(pGameObj->GetGUID(), team);
                }
                break;
            default:
                break;
        }
    }

    if (uint32 linkedEntry = pGameObj->GetGOInfo()->GetLinkedGameObjectEntry())
    {
        GameObject* linkedGO = sObjectMgr->IsStaticTransport(linkedEntry) ? new StaticTransport : new GameObject;
        if (linkedGO->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), linkedEntry, map, m_caster->GetPhaseMask(), Position(x, y, z, target->GetOrientation()), quat, 100, GO_STATE_READY))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
            linkedGO->SetSpellId(m_spellInfo->Id);
            linkedGO->SetTratsport(m_caster->GetTransport());

            ExecuteLogEffectSummonObject(effIndex, linkedGO);

            // Wild object not have owner and check clickable by players
            map->AddToMap(linkedGO);
        }
        else
        {
            delete linkedGO;
            linkedGO = nullptr;
        }
    }
}

void Spell::EffectScriptEffect(SpellEffIndex effIndex)
{
    if (SpellDummyTriggered(effIndex))
        return;

    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    switch (m_spellInfo->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_GENERIC:
        {
            // process discovery spells
            if (m_spellInfo->Categories.Mechanic == MECHANIC_DISCOVERY || m_spellInfo->IsExplicitDiscovery())
            {
                if (Player* player = m_caster->ToPlayer())
                {
                    // learn random explicit discovery recipe (if any)
                    if (uint32 discoveredSpell = GetExplicitDiscoverySpell(m_spellInfo->Id, player))
                    {
                        player->learnSpell(discoveredSpell, false);
                        return;
                    }
                }
            }

            switch (m_spellInfo->Id)
            {
                case 241856: // Slayer's Felbroken Shrieker
                {
                    if (Player* player = m_caster->ToPlayer())
                        player->learnSpell(229417, false);
                    return;
                }
                case 241851: // Netherlord's Chaotic Wrathsteed
                {
                    if (Player* player = m_caster->ToPlayer())
                        player->learnSpell(232412, false);
                    return;
                }
                case 241854: // Archmage's Prismatic Disc
                {
                    if (Player* player = m_caster->ToPlayer())
                        player->learnSpell(229376, false);
                    return;
                }
                case 240981: // High Priest's Lightsworn Seeker
                {
                    if (Player* player = m_caster->ToPlayer())
                        player->learnSpell(229377, false);
                    return;
                }
                case 143483: //Paragons Purpose heal, Paragons of the Klaxxi[SO]
                {
                    if (!unitTarget)
                        return;

                    unitTarget->CastSpell(unitTarget, 143482, true); //Paragons Purpose buff dmg
                    return;
                }
                // Seafood Magnifique Feast
                case 87806:
                {
                    if (!unitTarget)
                        return;

                    if (unitTarget->IsPlayer())
                    {
                        float stat = 0.0f;
                        uint32 spellId = 0;

                        if (unitTarget->GetStat(STAT_STRENGTH) > stat) { spellId = 87584; stat = unitTarget->GetStat(STAT_STRENGTH); }
                        if (unitTarget->GetStat(STAT_AGILITY)  > stat) { spellId = 87586; stat = unitTarget->GetStat(STAT_AGILITY); }
                        if (unitTarget->GetStat(STAT_INTELLECT)  > stat) { spellId = 87587; stat = unitTarget->GetStat(STAT_INTELLECT); }
                        
                        if (spellId)
                            unitTarget->CastSpell(unitTarget, spellId, true);
                    }
                    return;
                }
                // Banquet of the Grill
                case 126532:
                {
                    if (!unitTarget)
                        return;

                    if (unitTarget->IsPlayer())
                    {
                        float stat = 0.0f;
                        uint32 spellId = 0;

                        if (unitTarget->GetStat(STAT_STRENGTH) > stat) { spellId = 104263; stat = unitTarget->GetStat(STAT_STRENGTH); }
                        if (unitTarget->GetStat(STAT_AGILITY)  > stat) { spellId = 104286; stat = unitTarget->GetStat(STAT_AGILITY); }
                        if (unitTarget->GetStat(STAT_INTELLECT)  > stat) { spellId = 104266; stat = unitTarget->GetStat(STAT_INTELLECT); }
                        
                        if (spellId)
                            unitTarget->CastSpell(unitTarget, spellId, true);
                    }
                    return;
                }
                // Great Pandaren Banquet
                case 104924:
                {
                    if (!unitTarget)
                        return;

                    if (unitTarget->IsPlayer())
                    {
                        float stat = 0.0f;
                        uint32 spellId = 0;

                        if (unitTarget->GetStat(STAT_STRENGTH) > stat) { spellId = 104284; stat = unitTarget->GetStat(STAT_STRENGTH); }
                        if (unitTarget->GetStat(STAT_AGILITY)  > stat) { spellId = 104287; stat = unitTarget->GetStat(STAT_AGILITY); }
                        if (unitTarget->GetStat(STAT_INTELLECT)  > stat) { spellId = 104290; stat = unitTarget->GetStat(STAT_INTELLECT); }
                        
                        if (spellId)
                            unitTarget->CastSpell(unitTarget, spellId, true);
                    }
                    return;
                }
                case 104126:    //Mop.Quest.Monkey Wisdom
                {
                    if (Player *player = unitTarget->ToPlayer())
                    {
                        //const uint32 text[6] = {2000009994, 2000009995, 2000009996, 2000009997, 2000009998, 2000009999};
                        //std::string texta(sObjectMgr->GetTrinityString(text[urand(0, 5)], player->GetSession()->GetSessionDbLocaleIndex()));
                        //WorldPacket data(SMSG_CHAT, 200);
                        //player->BuildPlayerChat(&data, CHAT_MSG_RAID_BOSS_WHISPER, texta, LANG_UNIVERSAL);
                        //player->SendDirectMessage(&data);
                    }
                    return;
                }
                case 45204: // Clone Me!
                    unitTarget->CastSpell(m_caster, damage, true);
                    break;
                case 55693:                                 // Remove Collapsing Cave Aura
                    if (!unitTarget)
                        return;
                    unitTarget->RemoveAurasDueToSpell(m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValue());
                    break;
                // PX-238 Winter Wondervolt TRAP
                case 26275:
                {
                    uint32 spells[4] = {26272, 26157, 26273, 26274};

                    // check presence
                    for (auto spell : spells)
                        if (unitTarget->HasAuraEffect(spell, 0))
                            return;

                    // select spell
                    uint32 iTmpSpellId = spells[urand(0, 3)];

                    // cast
                    unitTarget->CastSpell(unitTarget, iTmpSpellId, true);
                    return;
                }
                // Bending Shinbone
                case 8856:
                {
                    if (!itemTarget && !m_caster->IsPlayer())
                        return;

                    uint32 spell_id = roll_chance_i(20) ? 8854 : 8855;

                    m_caster->CastSpell(m_caster, spell_id, true, nullptr);
                    return;
                }
                // Brittle Armor - need remove one 24575 Brittle Armor aura
                case 24590:
                    unitTarget->RemoveAuraFromStack(24575);
                    return;
                // Mercurial Shield - need remove one 26464 Mercurial Shield aura
                case 26465:
                    unitTarget->RemoveAuraFromStack(26464);
                    return;
                // Shadow Flame (All script effects, not just end ones to prevent player from dodging the last triggered spell)
                case 22539:
                case 22972:
                case 22975:
                case 22976:
                case 22977:
                case 22978:
                case 22979:
                case 22980:
                case 22981:
                case 22982:
                case 22983:
                case 22984:
                case 22985:
                {
                    if (!unitTarget || !unitTarget->isAlive())
                        return;

                    // Onyxia Scale Cloak
                    if (unitTarget->HasAura(22683))
                        return;

                    // Shadow Flame
                    m_caster->CastSpell(unitTarget, 22682, true);
                    return;
                }
                case 17512: // Piccolo of the Flaming Fire
                case 51508: // Party G.R.E.N.A.D.E.
                {
                    if (!unitTarget || !unitTarget->IsPlayer())
                        return;
                    unitTarget->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                    return;
                }
                // Decimate
                case 28374:
                case 54426:
                    if (unitTarget)
                    {
                        int32 damage = int32(unitTarget->GetHealth(m_caster)) - int32(unitTarget->CountPctFromMaxHealth(5, m_caster));
                        if (damage > 0)
                            m_caster->CastCustomSpell(28375, SPELLVALUE_BASE_POINT0, damage, unitTarget);
                    }
                    return;
                // Mirren's Drinking Hat
                case 29830:
                {
                    uint32 item = 0;
                    switch (urand(1, 6))
                    {
                        case 1:
                        case 2:
                        case 3:
                            item = 23584; break;            // Loch Modan Lager
                        case 4:
                        case 5:
                            item = 23585; break;            // Stouthammer Lite
                        case 6:
                            item = 23586; break;            // Aerie Peak Pale Ale
                    }
                    if (item)
                        DoCreateItem(effIndex, item);
                    break;
                }
                // Plant Warmaul Ogre Banner
                case 32307:
                    if (Player* caster = m_caster->ToPlayer())
                    {
                        caster->RewardPlayerAndGroupAtEvent(18388, unitTarget);
                        if (Creature* target = unitTarget->ToCreature())
                        {
                            target->setDeathState(CORPSE);
                            target->RemoveCorpse();
                        }
                    }
                    break;
                // Mug Transformation
                case 41931:
                {
                    if (!m_caster->IsPlayer())
                        return;

                    uint8 bag = 19;
                    uint8 slot = 0;
                    Item* item = nullptr;

                    while (bag) // 256 = 0 due to var type
                    {
                        item = m_caster->ToPlayer()->GetItemByPos(bag, slot);
                        if (item && item->GetEntry() == 38587)
                            break;

                        ++slot;
                        if (slot == 39)
                        {
                            slot = 0;
                            ++bag;
                        }
                    }
                    if (bag)
                    {
                        if (m_caster->ToPlayer()->GetItemByPos(bag, slot)->GetCount() == 1) m_caster->ToPlayer()->RemoveItem(bag, slot, true);
                        else m_caster->ToPlayer()->GetItemByPos(bag, slot)->SetCount(m_caster->ToPlayer()->GetItemByPos(bag, slot)->GetCount() - 1);
                        // Spell 42518 (Braufest - Gratisprobe des Braufest herstellen)
                        m_caster->CastSpell(m_caster, 42518, true);
                        return;
                    }
                    break;
                }
                // Brutallus - Burn
                case 45141:
                case 45151:
                {
                    //Workaround for Range ... should be global for every ScriptEffect
                    float radius = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius();
                    if (unitTarget && unitTarget->IsPlayer() && unitTarget->GetDistance(m_caster) >= radius && !unitTarget->HasAura(46394) && unitTarget != m_caster)
                        unitTarget->CastSpell(unitTarget, 46394, true);

                    break;
                }
                // Goblin Weather Machine
                case 46203:
                {
                    if (!unitTarget)
                        return;

                    uint32 spellId = 0;
                    switch (rand() % 4)
                    {
                        case 0: spellId = 46740; break;
                        case 1: spellId = 46739; break;
                        case 2: spellId = 46738; break;
                        case 3: spellId = 46736; break;
                    }
                    unitTarget->CastSpell(unitTarget, spellId, true);
                    break;
                }
                // 5, 000 Gold
                case 46642:
                {
                    if (!unitTarget || !unitTarget->IsPlayer())
                        return;

                    unitTarget->ToPlayer()->ModifyMoney(5000 * GOLD);

                    break;
                }
                // Roll Dice - Decahedral Dwarven Dice
                case 47770:
                {
                    char buf[128];
                    const char *gender = "his";
                    if (m_caster->getGender() > 0)
                        gender = "her";
                    sprintf(buf, "%s rubs %s [Decahedral Dwarven Dice] between %s hands and rolls. One %u and one %u.", m_caster->GetName(), gender, gender, urand(1, 10), urand(1, 10));
                    m_caster->MonsterTextEmote(buf, ObjectGuid::Empty);
                    break;
                }
                // Roll 'dem Bones - Worn Troll Dice
                case 47776:
                {
                    char buf[128];
                    const char *gender = "his";
                    if (m_caster->getGender() > 0)
                        gender = "her";
                    sprintf(buf, "%s causually tosses %s [Worn Troll Dice]. One %u and one %u.", m_caster->GetName(), gender, urand(1, 6), urand(1, 6));
                    m_caster->MonsterTextEmote(buf, ObjectGuid::Empty);
                    break;
                }
                // Death Knight Initiate Visual
                case 51519:
                {
                    if (!unitTarget || !unitTarget->IsCreature())
                        return;

                    uint32 iTmpSpellId = 0;
                    switch (unitTarget->GetDisplayId())
                    {
                        case 25369: iTmpSpellId = 51552; break; // bloodelf female
                        case 25373: iTmpSpellId = 51551; break; // bloodelf male
                        case 25363: iTmpSpellId = 51542; break; // draenei female
                        case 25357: iTmpSpellId = 51541; break; // draenei male
                        case 25361: iTmpSpellId = 51537; break; // dwarf female
                        case 25356: iTmpSpellId = 51538; break; // dwarf male
                        case 25372: iTmpSpellId = 51550; break; // forsaken female
                        case 25367: iTmpSpellId = 51549; break; // forsaken male
                        case 25362: iTmpSpellId = 51540; break; // gnome female
                        case 25359: iTmpSpellId = 51539; break; // gnome male
                        case 25355: iTmpSpellId = 51534; break; // human female
                        case 25354: iTmpSpellId = 51520; break; // human male
                        case 25360: iTmpSpellId = 51536; break; // nightelf female
                        case 25358: iTmpSpellId = 51535; break; // nightelf male
                        case 25368: iTmpSpellId = 51544; break; // orc female
                        case 25364: iTmpSpellId = 51543; break; // orc male
                        case 25371: iTmpSpellId = 51548; break; // tauren female
                        case 25366: iTmpSpellId = 51547; break; // tauren male
                        case 25370: iTmpSpellId = 51545; break; // troll female
                        case 25365: iTmpSpellId = 51546; break; // troll male
                        default: return;
                    }

                    unitTarget->CastSpell(unitTarget, iTmpSpellId, true);
                    Creature* npc = unitTarget->ToCreature();
                    npc->LoadEquipment();
                    return;
                }
                // Emblazon Runeblade
                case 51770:
                {
                    if (!m_originalCaster)
                        return;

                    m_originalCaster->CastSpell(m_originalCaster, damage, false);
                    break;
                }
                // Deathbolt from Thalgran Blightbringer
                // reflected by Freya's Ward
                // Retribution by Sevenfold Retribution
                case 51854:
                {
                    if (!unitTarget)
                        return;
                    if (unitTarget->HasAura(51845))
                        unitTarget->CastSpell(m_caster, 51856, true);
                    else
                        m_caster->CastSpell(unitTarget, 51855, true);
                    break;
                }
                // Summon Ghouls On Scarlet Crusade
                case 51904:
                {
                    if (!m_targets.HasDst())
                        return;

                    float x, y, z;
                    float radius = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius();
                    for (uint8 i = 0; i < 15; ++i)
                    {
                        m_caster->GetRandomPoint(*destTarget, radius, x, y, z);
                        m_caster->CastSpell(x, y, z, 54522, true);
                    }
                    break;
                }
                case 52173: // Coyote Spirit Despawn
                case 60243: // Blood Parrot Despawn
                    if (unitTarget->IsCreature() && unitTarget->ToCreature()->isSummon())
                        unitTarget->ToTempSummon()->UnSummon();
                    return;
                case 52479: // Gift of the Harvester
                    if (unitTarget && m_originalCaster)
                        m_originalCaster->CastSpell(unitTarget, urand(0, 1) ? damage : 52505, true);
                    return;
                case 53110: // Devour Humanoid
                    if (unitTarget)
                        unitTarget->CastSpell(m_caster, damage, true);
                    return;
                case 57347: // Retrieving (Wintergrasp RP-GG pickup spell)
                {
                    if (!unitTarget || !unitTarget->IsCreature() || !m_caster->IsPlayer())
                        return;

                    unitTarget->ToCreature()->DespawnOrUnsummon();

                    return;
                }
                case 57349: // Drop RP-GG (Wintergrasp RP-GG at death drop spell)
                {
                    if (!m_caster->IsPlayer())
                        return;

                    // Delete item from inventory at death
                    m_caster->ToPlayer()->DestroyItemCount(damage, 5, true);

                    return;
                }
                case 58418:                                 // Portal to Orgrimmar
                case 58420:                                 // Portal to Stormwind
                {
                    if (!unitTarget || !unitTarget->IsPlayer() || effIndex != 0)
                        return;

                    uint32 spellID = m_spellInfo->GetEffect(EFFECT_0, m_diffMode)->CalcValue();
                    uint32 questID = m_spellInfo->GetEffect(EFFECT_1, m_diffMode)->CalcValue();

                    if (unitTarget->ToPlayer()->GetQuestStatus(questID) == QUEST_STATUS_COMPLETE)
                        unitTarget->CastSpell(unitTarget, spellID, true);

                    return;
                }
                case 58941:                                 // Rock Shards
                    if (unitTarget && m_originalCaster)
                    {
                        for (uint32 i = 0; i < 3; ++i)
                        {
                            m_originalCaster->CastSpell(unitTarget, 58689, true);
                            m_originalCaster->CastSpell(unitTarget, 58692, true);
                        }
                        if (dynamic_cast<InstanceMap*>(m_originalCaster->GetMap())->GetDifficultyID() == DIFFICULTY_NORMAL)
                        {
                            m_originalCaster->CastSpell(unitTarget, 58695, true);
                            m_originalCaster->CastSpell(unitTarget, 58696, true);
                        }
                        else
                        {
                            m_originalCaster->CastSpell(unitTarget, 60883, true);
                            m_originalCaster->CastSpell(unitTarget, 60884, true);
                        }
                    }
                    return;
                case 58983: // Big Blizzard Bear
                {
                    if (!unitTarget || !unitTarget->IsPlayer())
                        return;

                    // Prevent stacking of mounts and client crashes upon dismounting
                    unitTarget->RemoveAurasByType(SPELL_AURA_MOUNTED);

                    // Triggered spell id dependent on riding skill
                    if (uint16 skillval = unitTarget->ToPlayer()->GetSkillValue(SKILL_RIDING))
                    {
                        if (skillval >= 150)
                            unitTarget->CastSpell(unitTarget, 58999, true);
                        else
                            unitTarget->CastSpell(unitTarget, 58997, true);
                    }
                    return;
                }
                case 63845: // Create Lance
                {
                    if (!m_caster->IsPlayer())
                        return;

                    if (m_caster->ToPlayer()->GetTeam() == ALLIANCE)
                        m_caster->CastSpell(m_caster, 63914, true);
                    else
                        m_caster->CastSpell(m_caster, 63919, true);
                    return;
                }
                case 59317:                                 // Teleporting
                    if (!unitTarget || !unitTarget->IsPlayer())
                        return;

                    // return from top
                    if (unitTarget->ToPlayer()->GetCurrentAreaID() == 4637)
                        unitTarget->CastSpell(unitTarget, 59316, true);
                    // teleport atop
                    else
                        unitTarget->CastSpell(unitTarget, 59314, true);

                    return;
                case 143626:                                // Celestial Cloth and Its Uses
                case 143644:                                // Hardened Magnificent Hide and Its Uses
                case 143646:                                // Balanced Trillium Ingot and Its Uses
                {
                    Player* player = m_caster->ToPlayer();
                    if (!player)
                        return;

                    uint32 spellToRecipe[][3] = {
                        {143626, 143011, 146925},
                        {143644, 142976, 146923},
                        {143646, 143255, 146921},
                        {0, 0, 0}
                    };

                    uint32 learnSpell = 0;
                    uint32 acceleratedSpell = 0;
                    for (uint32 i = 0; spellToRecipe[i][0] != 0; ++i)
                        if (spellToRecipe[i][0] == m_spellInfo->Id)
                        {
                            learnSpell = spellToRecipe[i][1];
                            acceleratedSpell = spellToRecipe[i][2];
                            break;
                        }

                    if (!learnSpell || player->HasSpell(learnSpell))
                        return;

                    player->learnSpell(learnSpell, false);
                    player->learnSpell(acceleratedSpell, false);

                    // learn random explicit discovery recipe (if any)
                    if (uint32 discoveredSpell = GetExplicitDiscoverySpell(learnSpell, player))
                        player->learnSpell(discoveredSpell, false);
                    return;
                }
                case 62482: // Grab Crate
                {
                    if (unitTarget)
                    {
                        if (Unit* seat = m_caster->GetVehicleBase())
                        {
                            if (Unit* parent = seat->GetVehicleBase())
                            {
                                // TODO: a hack, range = 11, should after some time cast, otherwise too far
                                m_caster->CastSpell(parent, 62496, true);
                                unitTarget->CastSpell(parent, m_spellInfo->GetEffect(EFFECT_0, m_diffMode)->CalcValue());
                            }
                        }
                    }
                    return;
                }
                case 66545: //Summon Memory
                {
                    uint8 uiRandom = urand(0, 25);
                    uint32 uiSpells[26] = {66704, 66705, 66706, 66707, 66709, 66710, 66711, 66712, 66713, 66714, 66715, 66708, 66708, 66691, 66692, 66694, 66695, 66696, 66697, 66698, 66699, 66700, 66701, 66702, 66703, 66543};

                    m_caster->CastSpell(m_caster, uiSpells[uiRandom], true);
                    break;
                }
                case 45668:                                 // Ultra-Advanced Proto-Typical Shortening Blaster
                {
                    if (!unitTarget || !unitTarget->IsCreature())
                        return;

                    if (roll_chance_i(50))                  // chance unknown, using 50
                        return;

                    static uint32 const spellPlayer[5] =
                    {
                        45674,                            // Bigger!
                        45675,                            // Shrunk
                        45678,                            // Yellow
                        45682,                            // Ghost
                        45684                             // Polymorph
                    };

                    static uint32 const spellTarget[5] =
                    {
                        45673,                            // Bigger!
                        45672,                            // Shrunk
                        45677,                            // Yellow
                        45681,                            // Ghost
                        45683                             // Polymorph
                    };

                    m_caster->CastSpell(m_caster, spellPlayer[urand(0, 4)], true);
                    unitTarget->CastSpell(unitTarget, spellTarget[urand(0, 4)], true);
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_POTION:
        {
            switch (m_spellInfo->Id)
            {
                // Netherbloom
                case 28702:
                {
                    if (!unitTarget)
                        return;
                    // 25% chance of casting a random buff
                    if (roll_chance_i(75))
                        return;

                    // triggered spells are 28703 to 28707
                    // Note: some sources say, that there was the possibility of
                    //       receiving a debuff. However, this seems to be removed by a patch.
                    const uint32 spellid = 28703;

                    // don't overwrite an existing aura
                    for (uint8 i = 0; i < 5; ++i)
                        if (unitTarget->HasAura(spellid + i))
                            return;
                    unitTarget->CastSpell(unitTarget, spellid + urand(0, 4), true);
                    break;
                }

                // Nightmare Vine
                case 28720:
                {
                    if (!unitTarget)
                        return;
                    // 25% chance of casting Nightmare Pollen
                    if (roll_chance_i(75))
                        return;
                    unitTarget->CastSpell(unitTarget, 28721, true);
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Shattering Throw
            if (m_spellInfo->ClassOptions.SpellClassMask[1] & 0x00400000)
            {
                if (!unitTarget)
                    return;
                // remove shields, will still display immune to damage part
                unitTarget->RemoveAurasWithMechanic(1 << MECHANIC_MAGICAL_IMMUNITY, AURA_REMOVE_BY_ENEMY_SPELL);
                return;
            }
            break;
        }
    }

    // normal DB scripted effect
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell ScriptStart spellid %u in EffectScriptEffect(%u)", m_spellInfo->Id, effIndex);
    m_caster->GetMap()->ScriptsStart(sSpellScripts, uint32(m_spellInfo->Id | (effIndex << 24)), m_caster, unitTarget);
}

void Spell::EffectSanctuary(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    unitTarget->getHostileRefManager().UpdateVisibility();

    UnitSet* attackers = unitTarget->getAttackers();
    for (UnitSet::iterator itr = attackers->begin(); itr != attackers->end(); ++itr)
    {
        if (!(*itr)->canSeeOrDetect(unitTarget))
            (*itr)->AttackStop();
    }

    //unitTarget->m_lastSanctuaryTime = getMSTime();
}

void Spell::EffectDuel(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    Player* caster = m_caster->ToPlayer();
    Player* target = unitTarget->ToPlayer();

    // caster or target already have requested duel
    if (caster->duel || target->duel)
        return;

    // check cheat
    if (caster->GetGUID() == target->GetGUID())
        return;

    // check ignore - similar to client FriendList::IsIgnored(FriendList *)g_friendList, duelerGUID, 1)
    if (!target->GetSocial() || target->GetSocial()->HasIgnore(caster->GetGUID()))
        return;

    uint32 objEntry = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    // create arbiter gameobject (duel flag)
    GameObject* pGameObj = sObjectMgr->IsStaticTransport(objEntry) ? new StaticTransport : new GameObject;

    Map* map = m_caster->GetMap();
    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(m_caster->GetOrientation(), 0.f, 0.f));
    if (!pGameObj->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), objEntry, map, m_caster->GetPhaseMask(), Position(m_caster->GetPositionX() + (unitTarget->GetPositionX() - m_caster->GetPositionX()) / 2, m_caster->GetPositionY() + (unitTarget->GetPositionY() - m_caster->GetPositionY()) / 2, m_caster->GetPositionZ(), m_caster->GetOrientation()), quat, 0, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetTratsport(m_caster->GetTransport());
    pGameObj->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, m_caster->getFaction());
    pGameObj->SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, m_caster->GetEffectiveLevel());
    int32 duration = m_spellInfo->GetDuration(m_diffMode);
    pGameObj->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);
    // correct GAMEOBJECT_BYTES_1
    pGameObj->SetGoState(GO_STATE_READY);
    pGameObj->SetGoArtKit(0);
    pGameObj->SetGoAnimProgress(0xFF);

    ExecuteLogEffectSummonObject(effIndex, pGameObj);

    m_caster->AddGameObject(pGameObj);
    map->AddToMap(pGameObj);

    // create duel-info
    auto duel = new DuelInfo;
    duel->initiator = caster->GetGUID();
    duel->opponent = target->GetGUID();
    duel->arbiter = pGameObj->GetGUID();
    duel->isMounted = (GetSpellInfo()->Id == 62875); // Mounted Duel
    caster->duel = duel;

    auto duel2 = new DuelInfo;
    duel2->initiator = caster->GetGUID();
    duel2->opponent = caster->GetGUID();
    duel2->arbiter = pGameObj->GetGUID();
    duel2->isMounted = (GetSpellInfo()->Id == 62875); // Mounted Duel
    target->duel = duel2;

    WorldPackets::Duel::DuelRequested duelRequested;
    duelRequested.ArbiterGUID = pGameObj->GetGUID();
    duelRequested.RequestedByGUID = caster->GetGUID();
    duelRequested.RequestedByWowAccount = caster->GetSession()->GetBattlenetAccountGUID();
    caster->SendDirectMessage(duelRequested.Write());
    target->SendDirectMessage(duelRequested.Write());

    caster->SetGuidValue(PLAYER_FIELD_DUEL_ARBITER, pGameObj->GetGUID());
    target->SetGuidValue(PLAYER_FIELD_DUEL_ARBITER, pGameObj->GetGUID());

    sScriptMgr->OnPlayerDuelRequest(target, caster);
}

void Spell::EffectStuck(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster)
        return;

    auto player = m_caster->ToPlayer();
    if (!player)
        return;

    if (!sWorld->getBoolConfig(CONFIG_CAST_UNSTUCK))
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell Effect: Stuck");
    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "Player %s (guid %u) used auto-unstuck future at map %u (%f, %f, %f)", player->GetName(), player->GetGUIDLow(), m_caster->GetMapId(), m_caster->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

    if (player->isInFlight())
        return;

    // if player is dead without death timer is teleported to graveyard, otherwise not apply the effect
    if (player->isDead())
    {
        if (!player->GetDeathTimer())
            player->RepopAtGraveyard();

        return;
    }

    // the player dies if hearthstone is in cooldown, else the player is teleported to home
    if (player->HasSpellCooldown(8690))
    {
        player->Kill(player);
        return;
    }

    player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation(), TELE_TO_SPELL);

    // Stuck spell trigger Hearthstone cooldown
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(8690);
    if (!spellInfo)
        return;

    TriggerCastData triggerData;

    Spell spell(player, spellInfo, triggerData);
    spell.SendSpellCooldown();
}

void Spell::EffectSummonPlayer(SpellEffIndex /*effIndex*/)
{
    // workaround - this effect should not use target map
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    // Evil Twin (ignore player summon, but hide this for summoner)
    if (unitTarget->HasAura(23445))
        return;

    float x, y, z;
    m_caster->GetPosition(x, y, z);

    unitTarget->ToPlayer()->SetSummonPoint(m_caster->GetMapId(), x, y, z);

    WorldPackets::Misc::SummonRequest request;
    request.SummonerGUID = m_caster->GetGUID();
    request.SummonerVirtualRealmAddress = GetVirtualRealmAddress();
    request.AreaID = m_caster->GetCurrentZoneID();
    request.SkipStartingArea = false; //@TODO
    request.Reason = WorldPackets::Misc::SummonRequest::SummonReason::SPELL;
    unitTarget->ToPlayer()->SendDirectMessage(request.Write());
}

enum class GameObjectActions : uint32
{                                   // Name from client executable      // Comments
    None,                           // -NONE-
    AnimateCustom0,                 // Animate Custom0
    AnimateCustom1,                 // Animate Custom1
    AnimateCustom2,                 // Animate Custom2
    AnimateCustom3,                 // Animate Custom3
    Disturb,                        // Disturb                          // Triggers trap
    Unlock,                         // Unlock                           // Resets GO_FLAG_LOCKED
    Lock,                           // Lock                             // Sets GO_FLAG_LOCKED
    Open,                           // Open                             // Sets GO_STATE_ACTIVE
    OpenAndUnlock,                  // Open + Unlock                    // Sets GO_STATE_ACTIVE and resets GO_FLAG_LOCKED
    Close,                          // Close                            // Sets GO_STATE_READY
    ToggleOpen,                     // Toggle Open
    Destroy,                        // Destroy                          // Sets GO_STATE_DESTROYED
    Rebuild,                        // Rebuild                          // Resets from GO_STATE_DESTROYED
    Creation,                       // Creation
    Despawn,                        // Despawn
    MakeInert,                      // Make Inert                       // Disables interactions
    MakeActive,                     // Make Active                      // Enables interactions
    CloseAndLock,                   // Close + Lock                     // Sets GO_STATE_READY and sets GO_FLAG_LOCKED
    UseArtKit0,                     // Use ArtKit0                      // 46904: 121
    UseArtKit1,                     // Use ArtKit1                      // 36639: 81, 46903: 122
    UseArtKit2,                     // Use ArtKit2
    UseArtKit3,                     // Use ArtKit3
    SetTapList,                     // Set Tap List
    GoTo1stFloor,                   // Go to 1st floor
    GoTo2ndFloor,                   // Go to 2nd floor
    GoTo3rdFloor,                   // Go to 3rd floor
    GoTo4thFloor,                   // Go to 4th floor
    GoTo5thFloor,                   // Go to 5th floor
    GoTo6thFloor,                   // Go to 6th floor
    GoTo7thFloor,                   // Go to 7th floor
    GoTo8thFloor,                   // Go to 8th floor
    GoTo9thFloor,                   // Go to 9th floor
    GoTo10thFloor,                  // Go to 10th floor
    UseArtKit4,                     // Use ArtKit4
    PlayAnimKit,                    // Play Anim Kit "%s"               // MiscValueB -> Anim Kit ID
    OpenAndPlayAnimKit,             // Open + Play Anim Kit "%s"        // MiscValueB -> Anim Kit ID
    CloseAndPlayAnimKit,            // Close + Play Anim Kit "%s"       // MiscValueB -> Anim Kit ID
    PlayOneshotAnimKit,             // Play One-shot Anim Kit "%s"      // MiscValueB -> Anim Kit ID
    StopAnimKit,                    // Stop Anim Kit
    OpenAndStopAnimKit,             // Open + Stop Anim Kit
    CloseAndStopAnimKit,            // Close + Stop Anim Kit
    PlaySpellVisual,                // Play Spell Visual "%s"           // MiscValueB -> Spell Visual ID
    StopSpellVisual,                // Stop Spell Visual
    SetTappedToChallengePlayers,    // Set Tapped to Challenge Players
};

void Spell::EffectActivateObject(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!gameObjTarget || m_spellInfo->Id == gameObjTarget->GetGOInfo()->GetSpell())
        return;

    switch (GameObjectActions(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue))
    {
    case GameObjectActions::None: // -NONE-
        break;
    case GameObjectActions::AnimateCustom0: // Animate Custom0
    case GameObjectActions::AnimateCustom1: // Animate Custom1
    case GameObjectActions::AnimateCustom2: // Animate Custom2
    case GameObjectActions::AnimateCustom3: // Animate Custom3
        //gameObjTarget->SendCustomAnim();
        break;
    case GameObjectActions::Disturb: // Disturb // Triggers trap
        gameObjTarget->Use(m_caster);
        break;
    case GameObjectActions::Unlock: // Unlock // Resets GO_FLAG_LOCKED
        gameObjTarget->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
        break;
    case GameObjectActions::Lock: // Lock // Sets GO_FLAG_LOCKED
        gameObjTarget->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
        break;
    case GameObjectActions::Open: // Open // Sets GO_STATE_ACTIVE
        gameObjTarget->Use(m_caster);
        break;
    case GameObjectActions::OpenAndUnlock: // Open + Unlock // Sets GO_STATE_ACTIVE and resets GO_FLAG_LOCKED
        gameObjTarget->UseDoorOrButton(0, false, m_caster);
        gameObjTarget->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
        break;
    case GameObjectActions::Close: // Close // Sets GO_STATE_READY
        gameObjTarget->ResetDoorOrButton();
        break;
    case GameObjectActions::ToggleOpen: // Toggle Open
        break;
    case GameObjectActions::Destroy: // Destroy // Sets GO_STATE_DESTROYED
        gameObjTarget->UseDoorOrButton(0, true, m_caster);
        break;
    case GameObjectActions::Rebuild: // Rebuild // Resets from GO_STATE_DESTROYED
        gameObjTarget->ResetDoorOrButton();
        break;
    case GameObjectActions::Creation: // Creation
    case GameObjectActions::Despawn: // Despawn
        break;
    case GameObjectActions::MakeInert: // Make Inert // Disables interactions
        gameObjTarget->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
        break;
    case GameObjectActions::MakeActive: // Make Active // Enables interactions
        gameObjTarget->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
        break;
    case GameObjectActions::CloseAndLock: // Close + Lock // Sets GO_STATE_READY and sets GO_FLAG_LOCKED
        gameObjTarget->ResetDoorOrButton();
        gameObjTarget->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
        break;
    case GameObjectActions::UseArtKit0: // Use ArtKit0 // 46904:121
    case GameObjectActions::UseArtKit1: // Use ArtKit1 // 36639:81, 46903:122
    case GameObjectActions::UseArtKit2: // Use ArtKit2
    case GameObjectActions::UseArtKit3: // Use ArtKit3
    case GameObjectActions::UseArtKit4: // Use ArtKit4
    case GameObjectActions::SetTapList: // Set Tap List
    case GameObjectActions::GoTo1stFloor: // Go to 1st floor
    case GameObjectActions::GoTo2ndFloor: // Go to 2nd floor
    case GameObjectActions::GoTo3rdFloor: // Go to 3rd floor
    case GameObjectActions::GoTo4thFloor: // Go to 4th floor
    case GameObjectActions::GoTo5thFloor: // Go to 5th floor
    case GameObjectActions::GoTo6thFloor: // Go to 6th floor
    case GameObjectActions::GoTo7thFloor: // Go to 7th floor
    case GameObjectActions::GoTo8thFloor: // Go to 8th floor
    case GameObjectActions::GoTo9thFloor: // Go to 9th floor
    case GameObjectActions::GoTo10thFloor: // Go to 10th floor
        break;
    case GameObjectActions::PlayAnimKit: // Play Anim Kit "%s"  // MiscValueB -> Anim Kit ID
    case GameObjectActions::OpenAndPlayAnimKit: // Open + Play Anim Kit "%s"// MiscValueB -> Anim Kit ID
        gameObjTarget->SetAnimKitId(m_spellInfo->Effects[0]->MiscValueB, false);
        break;
    case GameObjectActions::CloseAndPlayAnimKit: // Close + Play Anim Kit "%s"  // MiscValueB -> Anim Kit ID
    case GameObjectActions::PlayOneshotAnimKit: // Play One-shot Anim Kit "%s" // MiscValueB -> Anim Kit ID
        break;
    case GameObjectActions::StopAnimKit: // Stop Anim Kit
    case GameObjectActions::OpenAndStopAnimKit: // Open + Stop Anim Kit
    case GameObjectActions::CloseAndStopAnimKit: // Close + Stop Anim Kit
    case GameObjectActions::PlaySpellVisual: // Play Spell Visual "%s"// MiscValueB -> Spell Visual ID
    case GameObjectActions::StopSpellVisual: // Stop Spell Visual
    case GameObjectActions::SetTappedToChallengePlayers: // Set Tapped to Challenge Players
        break;
    default:
        break;
    }

    switch (m_spellInfo->Id)
    {
        case 144229: //spoils_of_pandaria open box
            return;
        default:
            break;
    }

    ScriptInfo activateCommand;
    activateCommand.command = SCRIPT_COMMAND_ACTIVATE_OBJECT;
    gameObjTarget->GetMap()->ScriptCommandStart(activateCommand, 0, m_caster, gameObjTarget);
}

void Spell::EffectApplyGlyph(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    std::vector<uint32>& glyphs = player->GetGlyphs(player->GetActiveTalentGroup());
    std::size_t replacedGlyph = glyphs.size();
    for (std::size_t i = 0; i < glyphs.size(); ++i)
    {
        if (std::vector<uint32> const* activeGlyphBindableSpells = sDB2Manager.GetGlyphBindableSpells(glyphs[i]))
        {
            if (std::find(activeGlyphBindableSpells->begin(), activeGlyphBindableSpells->end(), m_miscData[0]) != activeGlyphBindableSpells->end())
            {
                replacedGlyph = i;
                player->RemoveAurasDueToSpell(sGlyphPropertiesStore.AssertEntry(glyphs[i])->SpellID);
                break;
            }
        }
    }

    uint32 glyphId = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    if (replacedGlyph < glyphs.size())
    {
        if (glyphId)
            glyphs[replacedGlyph] = glyphId;
        else
            glyphs.erase(glyphs.begin() + replacedGlyph);
    }
    else if (glyphId)
        glyphs.push_back(glyphId);

    if (GlyphPropertiesEntry const* glyphProperties = sGlyphPropertiesStore.LookupEntry(glyphId))
        player->CastSpell(player, glyphProperties->SpellID, true);

    WorldPackets::Talent::ActiveGlyphs activeGlyphs;
    activeGlyphs.Glyphs.emplace_back(m_miscData[0], uint16(glyphId));
    activeGlyphs.IsFullUpdate = false;
    player->SendDirectMessage(activeGlyphs.Write());
}

void Spell::EffectEnchantHeldItem(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    // this is only item spell effect applied to main-hand weapon of target player (players in area)
    if (!unitTarget)
        return;

    auto player = unitTarget->ToPlayer();
    if (!player)
        return;

    Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!item)
        return;

    // must be equipped
    if (!item->IsEquipped())
        return;

    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue)
    {
        uint32 enchant_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
        int32 duration = m_spellInfo->GetDuration(m_diffMode);          //Try duration index first ..
        if (!duration)
            duration = damage;//+1;            //Base points after ..
        if (!duration)
            duration = 10;                                  //10 seconds for enchants which don't have listed duration

        if (!sSpellItemEnchantmentStore.LookupEntry(enchant_id))
            return;

        // Always go to temp enchantment slot
        EnchantmentSlot slot = TEMP_ENCHANTMENT_SLOT;

        // Enchantment will not be applied if a different one already exists
        if (item->GetEnchantmentId(slot) && item->GetEnchantmentId(slot) != enchant_id)
            return;

        // Apply the temporary enchantment
        item->SetEnchantment(slot, enchant_id, duration*IN_MILLISECONDS, 0, m_caster->GetGUID());
        player->ApplyEnchantment(item, slot, true);
    }
}

void Spell::EffectDisEnchant(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;
    
    if (!itemTarget || itemTarget->GetDonateItem())
        return;

    if (Player* caster = m_caster->ToPlayer())
    {
        caster->UpdateCraftSkill(m_spellInfo->Id);
        caster->SendLoot(itemTarget->GetGUID(), LOOT_DISENCHANTING);
    }    
    // item will be removed at disenchanting end
}

void Spell::EffectInebriate(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();
    int8 currentDrunk = player->GetDrunkValue();
    int8 drunkMod = damage;
    if (currentDrunk + drunkMod > 100)
    {
        currentDrunk = 100;
        if (rand_chance() < 25.0f)
            player->CastSpell(player, 67468, false);    // Drunken Vomit
    }
    else
        currentDrunk += drunkMod;

    if (currentDrunk < 0)
        currentDrunk = 0;

    player->SetDrunkValue(currentDrunk, m_CastItem ? m_CastItem->GetEntry() : 0);
}

void Spell::EffectFeedPet(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    Item* foodItem = itemTarget;
    if (!foodItem)
        return;

    Pet* pet = player->GetPet();
    if (!pet)
        return;

    if (!pet->isAlive())
        return;

    ExecuteLogEffectDestroyItem(effIndex, foodItem->GetEntry());

    uint32 count = 1;
    player->DestroyItemCount(foodItem, count, true);
    // TODO: fix crash when a spell has two effects, both pointed at the same item target

    m_caster->CastSpell(pet, m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell, true);
}

void Spell::EffectDismissPet(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isPet())
        return;

    Pet* pet = unitTarget->ToPet();

    ExecuteLogEffectUnsummonObject(effIndex, pet);
    if (Player* player = pet->GetOwner()->ToPlayer())
    {
        player->RemovePet(pet);
        player->m_currentPetNumber = 0;
    }
}

void Spell::EffectSummonObject(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster)
        return;

    uint8 slot = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB;
    if (slot >= MAX_GAMEOBJECT_SLOT)
        return;

    uint32 go_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    float x, y, z, o;

    ObjectGuid guid = m_caster->m_ObjectSlot[slot];
    if (!guid.IsEmpty())
    {
        if (GameObject* obj = m_caster->GetMap()->GetGameObject(guid))
        {
            // Recast case - null spell id to make auras not be removed on object remove from world
            if (m_spellInfo->Id == obj->GetSpellId())
                obj->SetSpellId(0);
            m_caster->RemoveGameObject(obj, true);
        }
        m_caster->m_ObjectSlot[slot].Clear();
    }

    GameObject* pGameObj = sObjectMgr->IsStaticTransport(go_id) ? new StaticTransport : new GameObject;

    // If dest location if present
    if (m_targets.HasDst())
        destTarget->GetPosition(x, y, z, o);
    // Summon in random point all other units if location present
    else
    {
        m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);
        o = m_caster->GetOrientation();
    }

    Map* map = m_caster->GetMap();
    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(m_caster->GetOrientation(), 0.f, 0.f));
    if (!pGameObj->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), go_id, map, m_caster->GetPhaseMask(), Position(x, y, z, o), quat, 0, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetTratsport(m_caster->GetTransport());
    //pGameObj->SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, m_caster->GetEffectiveLevel());
    int32 duration = m_spellInfo->GetDuration(m_diffMode);
    pGameObj->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);
    m_caster->AddGameObject(pGameObj);

    // object only for SPELL_EFFECT_OBJECT_WITH_PERSONAL_VISIBILITY
    if (m_currentExecutedEffect == SPELL_EFFECT_OBJECT_WITH_PERSONAL_VISIBILITY && m_caster->IsPlayer())
        pGameObj->AddPlayerInPersonnalVisibilityList(m_caster->GetGUID());

    ExecuteLogEffectSummonObject(effIndex, pGameObj);

    map->AddToMap(pGameObj);

    m_caster->m_ObjectSlot[slot] = pGameObj->GetGUID();
}

void Spell::EffectSurvey(SpellEffIndex /*effIndex*/)
{
    if (!sWorld->getBoolConfig(CONFIG_ARCHAEOLOGY_ENABLED))
        return;

    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    uint8 slot = 4;
    uint32 go_id;
    float x = m_caster->GetPositionX();
    float y = m_caster->GetPositionY();
    float z = m_caster->GetPositionZ();
    float o = m_caster->GetOrientation();

    int32 duration;
    if (!player->OnSurvey(go_id, x, y, z, o))
        duration = 10000;
    else
        duration = 60000;

    if (!go_id)
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::EffectSurvey: no go id for x: %f y: %f z: %f map: %u", m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetMapId());
        return;
    }

    ObjectGuid const& guid = m_caster->m_ObjectSlot[slot];
    if (!guid.IsEmpty())
    {
        if (GameObject* obj = m_caster->GetMap()->GetGameObject(guid))
        {
            // Recast case - null spell id to make auras not be removed on object remove from world
            if (m_spellInfo->Id == obj->GetSpellId())
                obj->SetSpellId(0);
            m_caster->RemoveGameObject(obj, true);
        }
        m_caster->m_ObjectSlot[slot].Clear();
    }

    GameObject* pGameObj = sObjectMgr->IsStaticTransport(go_id) ? new StaticTransport : new GameObject;

    Map* map = m_caster->GetMap();
    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(m_caster->GetOrientation(), 0.f, 0.f));
    if (!pGameObj->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), go_id, map, m_caster->GetPhaseMask(), Position(x, y, z, o), quat, 0, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetTratsport(m_caster->GetTransport());
    pGameObj->AddPlayerInPersonnalVisibilityList(player->GetGUID());

    pGameObj->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);
    m_caster->AddGameObject(pGameObj);

    map->AddToMap(pGameObj);

    m_caster->m_ObjectSlot[slot] = pGameObj->GetGUID();
}

void Spell::EffectResurrect(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;
    if (!unitTarget->IsPlayer())
        return;

    if (unitTarget->isAlive())
        return;
    if (!unitTarget->IsInWorld())
        return;

    switch (m_spellInfo->Id)
    {
        // Defibrillate (Goblin Jumper Cables) have 33% chance on success
        case 8342:
            if (roll_chance_i(67))
            {
                m_caster->CastSpell(m_caster, 8338, true, m_CastItem);
                return;
            }
            break;
        // Defibrillate (Goblin Jumper Cables XL) have 50% chance on success
        case 22999:
            if (roll_chance_i(50))
            {
                m_caster->CastSpell(m_caster, 23055, true, m_CastItem);
                return;
            }
            break;
        // Defibrillate (Gnomish Army Knife) have 67% chance on success_list
        case 54732:
            if (roll_chance_i(33))
            {
                return;
            }
            break;
        default:
            break;
    }

    Player* target = unitTarget->ToPlayer();

    if (target->IsRessurectRequested())       // already have one active request
        return;

    //if (m_spellInfo->HasAttribute(SPELL_ATTR8_BATTLE_RESURRECTION) && GetCaster()->isInCombat())
    //    if (InstanceScript* instance = target->GetInstanceScript())
    //        if (instance->CanUseCombatResurrection())
    //            instance->ConsumeCombatResurrectionCharge();

    int32 hpPerc = m_spellInfo->Effects[EFFECT_1]->CalcValue(m_caster);
    if (!hpPerc)
        hpPerc = damage;

    ExecuteLogEffectResurrect(effIndex, target);

    target->SetResurrectRequestData(m_caster, target->CountPctFromMaxHealth(hpPerc), CalculatePct(target->GetMaxPower(POWER_MANA), damage), 0, m_spellInfo);
    SendResurrectRequest(target);

    m_caster->CastSpell(unitTarget, 160029, true);
}

void Spell::EffectAddExtraAttacks(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->isAlive() || !unitTarget->getVictim())
        return;

    if (unitTarget->m_extraAttacks)
        return;

    unitTarget->m_extraAttacks = damage;

    ExecuteLogEffectExtraAttacks(effIndex, unitTarget, damage);
}

void Spell::EffectParry(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetCanParry(true);
}

void Spell::EffectBlock(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetCanBlock(true);
}

void Spell::EffectLeap(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || unitTarget->isInFlight())
        return;

    if (!m_targets.HasDst())
        return;

    Position pos = static_cast<Position>(*m_targets.GetDstPos());
    unitTarget->AddUnitState(UNIT_STATE_JUMPING);

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectLeap If %i, X %f, Y %f, Z %f", m_spellInfo->Id, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());

    unitTarget->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), unitTarget == m_caster, false);
}

void Spell::EffectReputation(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();

    int32 rep_change = damage;
    uint32 faction_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);
    if (!factionEntry)
        return;

    if (RepRewardRate const* repData = sObjectMgr->GetRepRewardRate(faction_id))
        rep_change = int32(static_cast<float>(rep_change) * repData->spell_rate);

    // Bonus from spells that increase reputation gain
    float repMod = player->GetTotalAuraModifierByMiscValues(SPELL_AURA_MOD_REPUTATION_GAIN, 0, 0) + player->GetTotalAuraModifierByMiscValues(SPELL_AURA_MOD_REPUTATION_GAIN, faction_id, 0);
    float bonus = rep_change * repMod / 100.0f; // 10%
    rep_change += static_cast<int32>(bonus);

    player->GetReputationMgr().ModifyReputation(factionEntry, rep_change);

    if (factionEntry->ParagonFactionID)
        player->GetReputationMgr().ModifyParagonReputation(factionEntry, rep_change);

    // log Army of the Light and Argussian Reach reputation
    if (factionEntry->ID == 2165 || factionEntry->ID == 2170)
        sLog->outWarden("Player %s (GUID: %u) adds a %s reputation value %d (%u) for use spell %u", player->GetName(), player->GetGUIDLow(), player->GetReputationMgr().GetRank(factionEntry) == REP_EXALTED ? "paragon" : "", rep_change, factionEntry->ID, m_spellInfo->Id);
}

void Spell::EffectQuestComplete(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;
    Player* player = unitTarget->ToPlayer();

    uint32 questId = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    if (questId)
    {
        Quest const* quest = sQuestDataStore->GetQuestTemplate(questId);
        if (!quest)
            return;

        uint16 logSlot = player->FindQuestSlot(questId);
        if (logSlot < MAX_QUEST_LOG_SIZE)
            player->AreaExploredOrEventHappens(questId);
        else if (player->CanTakeQuest(quest, false))    // never rewarded before
            player->CompleteQuest(questId);             // quest not in log - for internal use
    }
}

void Spell::EffectForceDeselect(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    GuidUnorderedSet ignorList;

    if (Player* plr = m_caster->ToPlayer())
    {
        if (Group* group = plr->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                if (Player* plrInGrour = itr->getSource())
                    ignorList.insert(plrInGrour->GetGUID());
            }
        }
    }

    WorldPackets::Spells::ClearTarget clearTarget;
    clearTarget.Guid = m_caster->GetGUID();
    m_caster->SendMessageToSet(clearTarget.Write(), true, ignorList);
}

void Spell::EffectSelfResurrect(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster || m_caster->isAlive())
        return;
    if (!m_caster->IsPlayer())
        return;
    if (!m_caster->IsInWorld())
        return;

    uint64 health = 0;
    uint32 mana = 0;

    int32 hpPerc = m_spellInfo->Effects[EFFECT_1]->CalcValue(m_caster);
    if (!hpPerc)
        hpPerc = damage;

    // flat case
    if (damage < 0)
    {
        health = uint32(-damage);
        mana = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    }
    // percent case
    else
    {
        health = m_caster->CountPctFromMaxHealth(hpPerc);
        if (m_caster->GetMaxPower(POWER_MANA) > 0)
            mana = CalculatePct(m_caster->GetMaxPower(POWER_MANA), damage);
    }

    Player* player = m_caster->ToPlayer();
    player->ResurrectPlayer(0.0f);

    player->SetHealth(health);
    player->SetPower(POWER_MANA, mana);
    player->SetPower(POWER_RAGE, 0);
    player->SetPower(POWER_ENERGY, player->GetMaxPower(POWER_ENERGY));
    player->SetPower(POWER_FOCUS, 0);

    player->SpawnCorpseBones();

    //if (m_spellInfo->IsBattleResurrection())
    //    if (InstanceScript* instanceScript = player->GetInstanceScript())
    //        instanceScript->ConsumeCombatResurrectionCharge();
}

void Spell::EffectSkinning(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget->IsCreature())
        return;
    if (!m_caster->IsPlayer())
        return;

    Creature* creature = unitTarget->ToCreature();
    int32 targetLevel = creature->getLevelForTarget(m_caster);

    uint32 skill = creature->GetCreatureTemplate()->GetRequiredLootSkill();

    m_caster->ToPlayer()->SendLoot(creature->GetGUID(), LOOT_SKINNING);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

    int32 reqValue = targetLevel < 10 ? 0 : targetLevel < 20 ? (targetLevel - 10) * 10 : targetLevel * 5;
    if (targetLevel > 80)
        reqValue = targetLevel * 6;

    int32 skillValue = m_caster->ToPlayer()->GetPureSkillValue(skill);

    // Double chances for elites
    m_caster->ToPlayer()->UpdateGatherSkill(skill, skillValue, reqValue, creature->isElite() ? 2 : 1);
}

void Spell::EffectCharge(SpellEffIndex effIndex)
{
    if (effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
    {
        if (!unitTarget)
            return;

        uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;
        float angle = unitTarget->GetRelativeAngle(m_caster);
        Position pos;

        unitTarget->GetContactPoint(m_caster, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
        if (!canHitTargetInLOS)
            unitTarget->GetFirstCollisionPosition(pos, unitTarget->GetObjectSize(), angle);

        m_caster->SetSplineTimer(0);

        if (Player* plr = m_caster->ToPlayer())
            plr->SetKnockBackTime(0);

        if (!m_caster->GetMotionMaster()->SpellMoveCharge(pos.m_positionX, pos.m_positionY, pos.m_positionZ + unitTarget->GetObjectSize(), 27.f, EVENT_CHARGE, triggered_spell_id))
            return;
    }

    if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT_TARGET)
    {
        if (!unitTarget)
            return;

        // not all charge effects used in negative spells
        if (!m_spellInfo->IsPositive() && m_caster->IsPlayer())
            m_caster->Attack(unitTarget, true);
    }
}

void Spell::EffectChargeDest(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH)
        return;

    if (m_targets.HasDst())
    {
        Position pos;
        destTarget->GetPosition(&pos);
        float angle = m_caster->GetRelativeAngle(pos.GetPositionX(), pos.GetPositionY());
        float dist = m_caster->GetDistance(pos);
        if (canHitTargetInLOS && m_caster->ToCreature() && dist < 200.0f)
            m_caster->GetNearPoint2D(pos, dist, angle);
        else
            m_caster->GetFirstCollisionPosition(pos, dist, angle);

        // Racer Slam Hit Destination
        if (m_spellInfo->Id == 49302)
        {
            if (urand(0, 100) < 80)
            {
                m_caster->CastSpell(m_caster, 49336, false);
                m_caster->CastSpell(static_cast<Unit*>(nullptr), 49444, false); // achievement counter
            }
        }

        uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;

        if (!m_caster->GetMotionMaster()->SpellMoveCharge(pos.m_positionX, pos.m_positionY, pos.m_positionZ, SPEED_CHARGE, EVENT_CHARGE, triggered_spell_id))
            return;
    }
}

void Spell::EffectKnockBack(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    if (auto creature = unitTarget->ToCreature())
        if (creature->isWorldBoss() || creature->IsDungeonBoss() || creature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_IMMUNITY_KNOCKBACK)
            return;

    // Spells with SPELL_EFFECT_KNOCK_BACK(like Thunderstorm) can't knoback target if target has ROOT/STUN
    if (unitTarget->HasUnitState(UNIT_STATE_ROOT))
        return;

    if (unitTarget->IsPlayer())
        if (unitTarget->ToPlayer()->GetKnockBackTime())
            return;

    // Hack. Instantly interrupt non melee spells being casted
    if (Spell* spell = unitTarget->GetCurrentSpell(CURRENT_GENERIC_SPELL))
    {
        if (unitTarget->IsNonMeleeSpellCast(true) && (spell->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT) && !unitTarget->HasAuraType(SPELL_AURA_CAST_WHILE_WALKING2) && !unitTarget->HasAuraType(SPELL_AURA_CAST_WHILE_WALKING))
            unitTarget->InterruptNonMeleeSpells(true);
    }

    float ratio = 0.1f;
    float speedxy = float(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue) * ratio;
    float speedz = float(damage) * ratio;

    if (m_spellInfo->Id == 196353)
    {
        float speed = unitTarget->GetSpeedRate(MOVE_FLIGHT);
        speedxy *= speed;
        speedz  *= speed;
    }
    /*if (fabs(speedxy < 0.1f) && speedz < 0.1f)
        return;*/

    float x, y;
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_KNOCK_BACK_DEST)
    {
        if (m_targets.HasDst())
            destTarget->GetPosition(x, y);
        else
            return;
    }
    else //if (m_spellInfo->GetEffect(i, m_diffMode).Effect == SPELL_EFFECT_KNOCK_BACK)
    {
        m_caster->GetPosition(x, y);
    }

    if (unitTarget->IsPlayer() && unitTarget->HasUnitState(UNIT_STATE_CHARGING | UNIT_STATE_LONG_JUMP))
    {
        unitTarget->GetMotionMaster()->Clear(false);
        unitTarget->StopMoving();
    }

    bool bgAuras = unitTarget->HasAura(156621) || unitTarget->HasAura(156618) || unitTarget->HasAura(34976) || unitTarget->HasAura(140876) || unitTarget->HasAura(141210);

    if (bgAuras)
        speedz = speedz / 5;

    if (speedxy < 0)
        unitTarget->JumpTo(-speedxy, speedz);
    else
        unitTarget->KnockbackFrom(x, y, speedxy, speedz);

    if (unitTarget->IsPlayer())
        unitTarget->ToPlayer()->SetKnockBackTime(getMSTime());
}

void Spell::EffectLeapBack(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (!unitTarget)
        return;

    float angle = m_spellInfo->Effects[effIndex]->TargetB.CalcDirectionAngle();

    switch (m_spellInfo->GetMisc()->MiscData.IconFileDataID)
    {
        case 236163: // Wild Charge (Talent)
            angle = M_PI;
            break;
        case 132572: // Disengage
        {
            if (Unit* victim = unitTarget->getVictim())
                angle = victim->GetAngle(unitTarget);
            else
                angle = M_PI;
            break;
        }
        case 134065: // Trampoline Bounce
        {
            angle = float(rand_norm())*static_cast<float>(2*M_PI);
            break;
        }
        case 196287:
            angle = unitTarget->GetRelativeAngle(m_caster) - M_PI;
            break;
        default:
            break;
    }

    unitTarget->JumpTo(float(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue) / 10, float(damage / 10), angle);
}

void Spell::EffectQuestClear(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;
    Player* player = unitTarget->ToPlayer();

    uint32 quest_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    Quest const* quest = sQuestDataStore->GetQuestTemplate(quest_id);

    if (!quest)
        return;

    // Player has never done this quest
    if (player->GetQuestStatus(quest_id) == QUEST_STATUS_NONE)
        return;

    // remove all quest entries for 'entry' from quest log
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 logQuest = player->GetQuestSlotQuestId(slot);
        if (logQuest == quest_id)
        {
            player->SetQuestSlot(slot, 0);

            // we ignore unequippable quest items in this case, it's still be equipped
            player->TakeQuestSourceItem(logQuest, false);
        }
    }

    player->RemoveActiveQuest(quest_id);
    player->RemoveRewardedQuest(quest_id);
}

void Spell::EffectSendTaxi(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->ActivateTaxiPathTo(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, m_spellInfo->Id);
}

void Spell::EffectPullTowards(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    auto speedZ = static_cast<float>(m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValue() / 10);
    auto speedXY = static_cast<float>(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue / 10);
    Position pos;
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_PULL_TOWARDS_DEST)
    {
        if (m_targets.HasDst())
            pos.Relocate(*destTarget);
        else
            return;
    }
    else //if (m_spellInfo->GetEffect(i, m_diffMode).Effect == SPELL_EFFECT_PULL_TOWARDS)
    {
        pos.Relocate(m_caster);
    }

    unitTarget->GetMotionMaster()->MoveJump(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), speedXY, speedZ);
}

void Spell::EffectDispelMechanic(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    uint32 mechanic = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    uint32 allEffectDispelMask = m_spellInfo->GetSimilarEffectsMiscValueMask(SPELL_EFFECT_DISPEL_MECHANIC, m_caster);

    std::queue < std::pair < uint32, ObjectGuid > > dispel_list;

    Unit::AuraMap const& auras = unitTarget->GetOwnedAuras();
    for (const auto& itr : auras)
    {
        Aura* aura = itr.second;
        if (!aura->GetApplicationOfTarget(unitTarget->GetGUID()))
            continue;

        if (roll_chance_i(aura->CalcDispelChance(unitTarget, !unitTarget->IsFriendlyTo(m_caster))))
        {
            if (SpellInfo const* auraSpellInfo = aura->GetSpellInfo())
                if (uint32 mechanicMask = auraSpellInfo->GetAllEffectsMechanicMask())
                {
                    if ((mechanicMask & allEffectDispelMask) && hasPredictedDispel != 2)
                        hasPredictedDispel = 1;

                    if (mechanicMask & (1 << mechanic))
                        dispel_list.push(std::make_pair(aura->GetId(), aura->GetCasterGUID()));
                }
        }
    }

    for (; !dispel_list.empty(); dispel_list.pop())
    {
        unitTarget->RemoveAura(dispel_list.front().first, dispel_list.front().second, 0, AURA_REMOVE_BY_ENEMY_SPELL);
    }

    if (hasPredictedDispel == 1)
    {
        if (Unit::AuraEffectList const* mTotalAuraList = m_caster->GetAuraEffectsByType(SPELL_AURA_DUMMY))
            for (Unit::AuraEffectList::const_iterator i = mTotalAuraList->begin(); i != mTotalAuraList->end(); ++i)
                if ((*i)->GetMiscValue() == 11 && (*i)->GetSpellInfo()->GetMisc()->MiscData.IconFileDataID == m_spellInfo->GetMisc()->MiscData.IconFileDataID)
                    (*i)->SetAmount(m_spellInfo->Id);

        hasPredictedDispel++; // 2 is lock
    }
}

void Spell::EffectSummonDeadPet(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    Pet* pet = player->GetPet();
    if (!pet)
        return;

    if (pet->isAlive())
        return;

    if (damage < 0)
        return;

    float x, y, z;
    player->GetPosition(x, y, z);

    player->GetMap()->CreatureRelocation(pet, x, y, z, player->GetOrientation());

    pet->SetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
    pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    pet->setDeathState(ALIVE);
    pet->ClearUnitState(uint32(UNIT_STATE_ALL_STATE));
    pet->SetHealth(pet->CountPctFromMaxHealth(damage));

    //pet->AIM_Initialize();
    //player->PetSpellInitialize();
    pet->SavePetToDB();
}

void Spell::EffectDurabilityDamage(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    int32 slot = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        unitTarget->ToPlayer()->DurabilityPointsLossAll(damage, (slot < -1));
        ExecuteLogEffectDurabilityDamage(effIndex, unitTarget, -1, -1);
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (Item* item = unitTarget->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
    {
        unitTarget->ToPlayer()->DurabilityPointsLoss(item, damage);
        ExecuteLogEffectDurabilityDamage(effIndex, unitTarget, item->GetEntry(), slot);
    }
}

void Spell::EffectDurabilityDamagePCT(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    int32 slot = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        unitTarget->ToPlayer()->DurabilityLossAll(float(damage) / 100.0f, (slot < -1), false);
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (damage <= 0)
        return;

    if (Item* item = unitTarget->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        unitTarget->ToPlayer()->DurabilityLoss(item, float(damage) / 100.0f);
}

void Spell::EffectModifyThreatPercent(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    unitTarget->getThreatManager().modifyThreatPercent(m_caster, damage);
}

void Spell::EffectTransmitted(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 objEntry = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

    if (auto const* overrideSummonedGameObjects = m_caster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_SUMMONED_OBJECT))
        for (auto aurEff : *overrideSummonedGameObjects)
            if (uint32(aurEff->GetMiscValue()) == objEntry)
            {
                objEntry = uint32(aurEff->GetMiscValueB());
                break;
            }

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(objEntry);
    if (!goinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (Entry: %u) not exist and not created at spell (ID: %u) cast", objEntry, m_spellInfo->Id);
        return;
    }

    float fx, fy, fz;

    if (m_targets.HasDst())
        destTarget->GetPosition(fx, fy, fz);
    //FIXME: this can be better check for most objects but still hack
    else if (m_spellInfo->GetEffect(effIndex, m_diffMode)->HasRadius() && m_spellInfo->GetMisc(m_diffMode)->MiscData.Speed == 0)
        m_caster->GetClosePoint(fx, fy, fz, DEFAULT_WORLD_OBJECT_SIZE, m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_originalCaster));
    else
    {
        //GO is always friendly to it's creator, get range for friends
        float min_dis = m_spellInfo->GetMinRange(true);
        float max_dis = m_spellInfo->GetMaxRange(true);
        auto dis = static_cast<float>(rand_norm()) * (max_dis - min_dis) + min_dis;

        m_caster->GetClosePoint(fx, fy, fz, DEFAULT_WORLD_OBJECT_SIZE, dis);
    }

    Map* cMap = m_caster->GetMap();
    // if gameobject is summoning object, it should be spawned right on caster's position
    if (goinfo->type == GAMEOBJECT_TYPE_RITUAL)
        m_caster->GetPosition(fx, fy, fz);

    GameObject* pGameObj = sObjectMgr->IsStaticTransport(objEntry) ? new StaticTransport : new GameObject;
    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(m_caster->GetOrientation(), 0.f, 0.f));
    if (!pGameObj->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), objEntry, cMap, m_caster->GetPhaseMask(), Position(fx, fy, fz, m_caster->GetOrientation()), quat, 100, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetTratsport(m_caster->GetTransport());
    int32 duration = m_spellInfo->GetDuration(m_diffMode);

    switch (goinfo->type)
    {
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            // pGameObj->SetFaction(m_caster->getFaction());
            ObjectGuid bobberGuid = pGameObj->GetGUID();
            // client requires fishing bobber guid in channel object slot 0 to be usable
            m_caster->ClearDynamicValue(UNIT_DYNAMIC_FIELD_CHANNEL_OBJECTS);
            m_caster->SetDynamicStructuredValue(UNIT_DYNAMIC_FIELD_CHANNEL_OBJECTS, 0, &bobberGuid);
            m_caster->AddGameObject(pGameObj);              // will removed at spell cancel
            m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_SPELL, m_spellInfo->Id);
            m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_SPELL_XSPELL_VISUAL, m_SpellVisual);

            if (m_caster->IsPlayer())
                pGameObj->SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, m_caster->ToPlayer()->GetSkillTempBonusValue(SKILL_FISHING));

            pGameObj->SetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION+2, 0.0f);
            pGameObj->SetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION+3, 1.0f);

            // end time of range when possible catch fish (FISHING_BOBBER_READY_TIME..GetDuration(m_spellInfo))
            // start time == fish-FISHING_BOBBER_READY_TIME (0..GetDuration(m_spellInfo)-FISHING_BOBBER_READY_TIME)
            duration = duration - urand(1, 15)*IN_MILLISECONDS + FISHING_BOBBER_READY_TIME*IN_MILLISECONDS;
            break;
        }
        case GAMEOBJECT_TYPE_RITUAL:
        {
            if (m_caster->IsPlayer())
            {
                pGameObj->AddUniqueUse(m_caster->ToPlayer());
                m_caster->AddGameObject(pGameObj);      // will be removed at spell cancel
            }
            break;
        }
        case GAMEOBJECT_TYPE_DUEL_ARBITER: // 52991
            m_caster->AddGameObject(pGameObj);
            break;
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        case GAMEOBJECT_TYPE_CHEST:
        default:
            break;
    }

    pGameObj->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);

    //pGameObj->SetOwnerGUID(m_caster->GetGUID());

    //pGameObj->SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, m_caster->GetEffectiveLevel());
    pGameObj->SetSpellId(m_spellInfo->Id);

    ExecuteLogEffectSummonObject(effIndex, pGameObj);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AddObject at SpellEfects.cpp EffectTransmitted");
    m_caster->AddGameObject(pGameObj);
    //m_ObjToDel.push_back(pGameObj);

    cMap->AddToMap(pGameObj);

    if (Creature* creature = m_caster->ToCreature())
        if (creature->AI())
            creature->AI()->JustSummonedGO(pGameObj);

    // Glyph of Soulwell, Create Soulwell - 29893
    if (m_spellInfo->Id == 29893 && m_caster->HasAura(58094))
        m_caster->CastSpell(fx, fy, fz, 34145, true);

    if (uint32 linkedEntry = pGameObj->GetGOInfo()->GetLinkedGameObjectEntry())
    {
        GameObject* linkedGO = sObjectMgr->IsStaticTransport(linkedEntry) ? new StaticTransport : new GameObject;
        G3D::Quat quat2(G3D::Matrix3::fromEulerAnglesZYX(m_caster->GetOrientation(), 0.f, 0.f));
        if (linkedGO->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), linkedEntry, cMap, m_caster->GetPhaseMask(), Position(fx, fy, fz, m_caster->GetOrientation()), quat2, 100, GO_STATE_READY))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
            //linkedGO->SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, m_caster->getLevel());
            linkedGO->SetSpellId(m_spellInfo->Id);
            linkedGO->SetOwnerGUID(m_caster->GetGUID());

            linkedGO->SetTratsport(m_caster->GetTransport());
            ExecuteLogEffectSummonObject(effIndex, linkedGO);

            linkedGO->GetMap()->AddToMap(linkedGO);
        }
        else
        {
            delete linkedGO;
            linkedGO = nullptr;
        }
    }
}

void Spell::EffectProspecting(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    auto player = m_caster->ToPlayer();
    if (!player)
        return;

    if (!itemTarget || !(itemTarget->GetTemplate()->GetFlags() & ITEM_FLAG_IS_PROSPECTABLE))
        return;

    if (itemTarget->GetCount() < 5)
        return;

    if (sWorld->getBoolConfig(CONFIG_SKILL_PROSPECTING))
        player->UpdateGatherSkill(SKILL_JEWELCRAFTING, player->GetPureSkillValue(SKILL_JEWELCRAFTING), itemTarget->GetTemplate()->GetRequiredSkillRank());

    m_caster->ToPlayer()->SendLoot(itemTarget->GetGUID(), LOOT_PROSPECTING);
}

void Spell::EffectMilling(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    auto caster = m_caster->ToPlayer();
    if (!caster)
        return;

    if (!itemTarget || !(itemTarget->GetTemplate()->GetFlags() & ITEM_FLAG_IS_MILLABLE))
        return;

    if (itemTarget->GetCount() < 5)
        return;

    if (sWorld->getBoolConfig(CONFIG_SKILL_MILLING))
        caster->UpdateGatherSkill(SKILL_INSCRIPTION, caster->GetPureSkillValue(SKILL_INSCRIPTION), itemTarget->GetTemplate()->GetRequiredSkillRank());

    m_caster->ToPlayer()->SendLoot(itemTarget->GetGUID(), LOOT_MILLING);
}

void Spell::EffectSkill(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "WORLD: SkillEFFECT");
}

/* There is currently no need for this effect. We handle it in Battleground.cpp
   If we would handle the resurrection here, the spiritguide would instantly disappear as the
   player revives, and so we wouldn't see the spirit heal visual effect on the npc.
   This is why we use a half sec delay between the visual effect and the resurrection itself */
void Spell::EffectSpiritHeal(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    /*
    if (!unitTarget->IsPlayer())
        return;
    if (!unitTarget->IsInWorld())
        return;

    //m_spellInfo->GetEffect(i, m_diffMode).BasePoints; == 99 (percent?)
    //unitTarget->ToPlayer()->setResurrect(m_caster->GetGUID(), unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetMaxHealth(), unitTarget->GetMaxPower(POWER_MANA));
    unitTarget->ToPlayer()->ResurrectPlayer(1.0f);
    unitTarget->ToPlayer()->SpawnCorpseBones();
    */
}

// remove insignia spell effect
void Spell::EffectSkinPlayerCorpse(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Effect: SkinPlayerCorpse");
    if ((!m_caster->IsPlayer()) || (!unitTarget->IsPlayer()) || (unitTarget->isAlive()))
        return;

    unitTarget->ToPlayer()->RemovedInsignia(m_caster->ToPlayer());
}

void Spell::EffectStealBeneficialBuff(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Effect: StealBeneficialBuff");

    if (!unitTarget || unitTarget == m_caster)                 // can't steal from self
        return;

    DispelChargesList steal_list;

    // Create dispel mask by dispel type
    uint32 dispelMask = SpellInfo::GetDispelMask(DispelType(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue));
    Unit::AuraMap const& auras = unitTarget->GetOwnedAuras();
    for (const auto& itr : auras)
    {
        Aura* aura = itr.second;
        AuraApplication * aurApp = aura->GetApplicationOfTarget(unitTarget->GetGUID());
        if (!aurApp)
            continue;

        if ((aura->GetSpellInfo()->GetDispelMask()) & dispelMask)
        {
            // Need check for passive? this
            if (!aurApp->IsPositive() || aura->IsPassive() || aura->GetSpellInfo()->HasAttribute(SPELL_ATTR4_NOT_STEALABLE))
                continue;

            // The charges / stack amounts don't count towards the total number of auras that can be dispelled.
            // Ie: A dispel on a target with 5 stacks of Winters Chill and a Polymorph has 1 / (1 + 1) -> 50% chance to dispell
            // Polymorph instead of 1 / (5 + 1) -> 16%.
            bool dispel_charges = (aura->GetSpellInfo()->HasAttribute(SPELL_ATTR7_DISPEL_CHARGES)) != 0;
            uint8 charges = dispel_charges ? aura->GetCharges() : aura->GetStackAmount();
            if (charges > 0)
                steal_list.push_back(std::make_pair(aura, charges));
        }
    }

    if (steal_list.empty())
        return;

    // Ok if exist some buffs for dispel try dispel it
    DispelList success_list;

    std::list<uint32> spellFails;
    // dispel N = damage buffs (or while exist buffs for dispel)
    for (int32 count = 0; count < damage && !steal_list.empty();)
    {
        // Random select buff for dispel
        auto itr = steal_list.begin();
        std::advance(itr, urand(0, steal_list.size() - 1));

        int32 chance = itr->first->CalcDispelChance(unitTarget, !unitTarget->IsFriendlyTo(m_caster));
        // 2.4.3 Patch Notes: "Dispel effects will no longer attempt to remove effects that have 100% dispel resistance."
        if (!chance)
        {
            steal_list.erase(itr);
            continue;
        }
        if (roll_chance_i(chance))
        {
            success_list.push_back(std::make_pair(itr->first->GetId(), itr->first->GetCasterGUID()));
            --itr->second;
            if (itr->second <= 0)
                steal_list.erase(itr);
        }
        else
            spellFails.push_back(itr->first->GetId());              // Spell Id
        ++count;
    }

    if (!spellFails.empty())
        m_caster->SendDispelFailed(unitTarget->GetGUID(), m_spellInfo->Id, spellFails);

    if (success_list.empty())
        return;

    // On success dispel
    // Devour Magic
    if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_PET_ABILITY && m_spellInfo->Categories.Category == SPELLCATEGORY_DEVOUR_MAGIC)
        m_caster->CastSpell(m_caster, 19658, true);

    std::list<uint32> spellSuccess;
    for (auto& itr : success_list)
    {
        spellSuccess.push_back(itr.first);          // Spell Id
        unitTarget->RemoveAurasDueToSpellBySteal(itr.first, itr.second, m_caster);
    }

    m_caster->SendDispelLog(unitTarget->GetGUID(), m_spellInfo->Id, spellSuccess, false, true);
}

void Spell::EffectKillCreditPersonal(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->KilledMonsterCredit(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectKillCredit(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    if (int32 creatureEntry = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue)
        unitTarget->ToPlayer()->RewardPlayerAndGroupAtEvent(creatureEntry, unitTarget);
}

void Spell::EffectQuestFail(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->FailQuest(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectQuestStart(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();
    if (Quest const* qInfo = sQuestDataStore->GetQuestTemplate(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue))
    {
        if (player->CanTakeQuest(qInfo, false) && player->CanAddQuest(qInfo, false))
        {
            player->AddQuest(qInfo, nullptr);
        }
    }
}

void Spell::EffectCreateTamedPet(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player || unitTarget->GetPetGUID() || unitTarget->getClass() != CLASS_HUNTER)
        return;

    uint32 creatureEntry = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    Pet* pet = unitTarget->CreateTamedPetFrom(creatureEntry, m_spellInfo->Id);
    if (!pet)
        return;

    // add to world
    pet->GetMap()->AddToMap(pet->ToCreature());

    unitTarget->SetMinion(pet, true);

    pet->SavePetToDB();
    player->PetSpellInitialize();
    player->GetSession()->SendStablePet();
}

void Spell::EffectDiscoverTaxi(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;
    uint32 nodeid = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    if (sTaxiNodesStore.LookupEntry(nodeid))
        unitTarget->ToPlayer()->GetSession()->SendDiscoverNewTaxiNode(nodeid);
}

void Spell::EffectTitanGrip(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetCanTitanGrip(true);
}

void Spell::EffectRedirectThreat(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (unitTarget)
        m_caster->SetReducedThreatPercent(uint32(damage), unitTarget->GetGUID());
}

void Spell::EffectGameObjectDamage(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!gameObjTarget)
        return;

    Unit* caster = m_originalCaster;
    if (!caster)
        return;

    FactionTemplateEntry const* casterFaction = caster->getFactionTemplateEntry();
    FactionTemplateEntry const* targetFaction = sFactionTemplateStore.LookupEntry(gameObjTarget->GetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE));
    // Do not allow to damage GO's of friendly factions (ie: Wintergrasp Walls/Ulduar Storm Beacons)
    if ((casterFaction && targetFaction && !casterFaction->IsFriendlyTo(*targetFaction)) || !targetFaction)
        gameObjTarget->ModifyHealth(-damage, caster, GetSpellInfo()->Id, m_caster->GetGUID());
}

void Spell::EffectGameObjectRepair(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!gameObjTarget)
        return;

    gameObjTarget->ModifyHealth(damage, m_caster, 0, m_caster->GetGUID());
}

void Spell::EffectGameObjectSetDestructionState(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!gameObjTarget || !m_originalCaster)
        return;

    Player* player = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself();
    gameObjTarget->SetDestructibleState(GameObjectDestructibleState(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue), player, true);
}

void Spell::SummonGuardian(uint32 i, uint32 entry, SummonPropertiesEntry const* properties, uint32 numGuardians)
{
    Unit* caster = m_originalCaster;
    if (!caster)
        return;

    if (caster->isTotem())
        caster = caster->ToTotem()->GetOwner();

    float radius = 5.0f;
    int32 duration = m_spellInfo->GetDuration(m_diffMode);
    if (!numGuardians)
        numGuardians = 1;

    if (Player* modOwner = m_originalCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);

    //TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Map* map = caster->GetMap();
    ObjectGuid targetGUID = m_targets.GetUnitTargetGUID();
    switch (properties->ID)
    {
        case 3655:
            targetGUID = m_caster->GetGuidValue(UNIT_FIELD_TARGET);
            break;
    }

    for (uint32 count = 0; count < numGuardians; ++count)
    {
        Position pos;
        if (count == 0)
            pos = static_cast<Position>(*destTarget);
        else
            // randomize position for multiple summons
            m_caster->GetRandomPoint(*destTarget, radius, pos);

        TempSummon* summon = map->SummonCreature(entry, pos, properties, duration, caster, targetGUID, m_spellInfo->Id);
        if (!summon)
            return;
        if (summon->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
        {
            for (uint8 idx = 0; idx < summon->GetPetCastSpellSize(); ++idx)
            {
                if (SpellInfo const* sInfo = sSpellMgr->GetSpellInfo(summon->GetPetCastSpellOnPos(idx)))
                {
                    if (sInfo->GetMaxRange(false) >= 30.0f && sInfo->GetMaxRange(false) > summon->GetAttackDist() && (sInfo->AttributesCu[0] & SPELL_ATTR0_CU_DIRECT_DAMAGE) && !sInfo->IsTargetingAreaCast())
                    {
                        PetStats const* pStats = sObjectMgr->GetPetStats(entry);
                        if (!pStats)
                            summon->SetCasterPet(true);
                        if (!sInfo->IsPositive())
                            summon->SetAttackDist(sInfo->GetMaxRange(false));
                    }
                }
            }
        }

        if (properties && properties->Control == SUMMON_CATEGORY_ALLY)
            summon->setFaction(caster->getFaction());

        if (summon->GetEntry() == 27893 && m_caster->IsPlayer())
        {
            if (uint32 weapon = m_caster->GetUInt32Value(PLAYER_FIELD_VISIBLE_ITEMS + EQUIPMENT_SLOT_MAINHAND * 2))
            {
                summon->SetDisplayId(11686);
                summon->SetVirtualItem(0, weapon);
            }
            else
                summon->SetDisplayId(1126);
        }

        if (summon->GetEntry() == 100820 && m_caster->IsPlayer())
        {
            switch (summon->GetDisplayId())
            {
                case 66843:
                    summon->addSpell(224125);
                    break;
                case 66844:
                    summon->addSpell(224126);
                    break;
                case 66845:
                    summon->addSpell(224127);
                    break;
            }
        }

        if (Player* plr = caster->ToPlayer())
        {
            uint8 spellClassSet = m_spellInfo->ClassOptions.SpellClassSet;

            if ((spellClassSet == SPELLFAMILY_PET_ABILITY || spellClassSet == plr->GetClassFamily()) && !summon->isTotem())
                summon->AddUnitTypeMask(UNIT_MASK_CREATED_BY_CLASS_SPELL);

            switch (summon->GetEntry())
            {
                case 98035: // Call Dreadstalkers
                {
                    Unit* _target = plr->getVictim();
                    if (!_target)
                        _target = plr->GetSelectedUnit();
                    summon->Attack(_target, true);
                    if (_target)
                        summon->CastSpell(_target, 194247, true);
                    break;
                }
            }
        }

        // guardians should follow owner.
        if (caster && !summon->HasUnitState(UNIT_STATE_FOLLOW)) // no charm info and no victim
            if (summon->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE) // Prevent rewrite movement from scripts
                summon->GetMotionMaster()->MoveFollow(caster, summon->GetFollowDistance(), summon->GetFollowAngle());

        if (!m_caster->IsPlayer())
            summon->SelectLevel(summon->GetCreatureTemplate());

        ExecuteLogEffectSummonObject(i, summon);
    }
}

void Spell::EffectRenamePet(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsCreature() || !unitTarget->ToCreature()->isPet() || unitTarget->ToPet()->getPetType() != HUNTER_PET)
        return;

    unitTarget->SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_CAN_BE_RENAMED);
}

void Spell::EffectPlayMusic(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->SendMusic(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectSpecCount(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    //unitTarget->ToPlayer()->UpdateSpecCount(damage);
}

void Spell::EffectActivateSpec(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();
    if (!player)
        return;

    switch (m_miscData[1])
    {
        case 0:
        {
            if (player->getClass() == CLASS_DEMON_HUNTER)
                if (!player->HasCompletedQuest(40051) && player->IsLoXpMap(player->GetMapId()))
                    return;

            if (ChrSpecializationEntry const* specialization = sChrSpecializationStore.LookupEntry(m_miscData[0]))
            {
                if (player->IsCanChangeSpecToAnotherRole() || (specialization->Role == player->GetSpecializationRole()))
                    player->ActivateTalentGroup(specialization);
                else
                {
                    WorldPackets::Misc::DisplayGameError display;
                    display.Error = UIErrors::ERR_LFG_ROLE_CHECK_FAILED;
                    player->SendDirectMessage(display.Write()); 
                    return;
                }
            }

            player->SendOperationsAfterDelay(OAD_RECALC_ITEMS);

            if (AuraEffect* eff = player->GetAuraEffect(115043, EFFECT_0)) // Player Damage Reduction
            {
                eff->SetCanBeRecalculated(true);
                eff->RecalculateAmount();
            }
            break;
        }
        case 1:
        {
            if (player->getClass() != CLASS_HUNTER)
                return;

            Pet* pet = player->GetPet();
            if (!pet)
                return;

            pet->UnlearnSpecializationSpell();
            pet->SetSpecialization(m_miscData[0]);
            pet->LearnSpecializationSpell();
            player->AddPetInfo(pet);
            player->SendTalentsInfoData(true);
            break;
        }
        default:
            break;
    }

    if (player->HasPvpStatsScalingEnabled())
    {
        if (Aura* statTemplate = player->GetAura(SPELL_PVP_STATS_TEMPLATE))
            statTemplate->RecalculateAmountOfEffects(true);
    }
}

void Spell::EffectPlaySound(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    switch (m_spellInfo->Id)
    {
        case 91604: // Restricted Flight Area
        case 58600: // Restricted Flight Area
            unitTarget->ToPlayer()->GetSession()->SendNotification(LANG_ZONE_NOFLYZONE);
            unitTarget->PlayDirectSound(9417); // Fel Reaver sound
            break;
    }

    unitTarget->ToPlayer()->SendSound(m_spellInfo->Effects[effIndex]->MiscValue, ObjectGuid::Empty);
}

void Spell::EffectRemoveAura(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectRemoveAura unitTarget %s", unitTarget->GetGUID().ToString().c_str());

    // there may be need of specifying casterguid of removed auras
    // Deley operations from 227615
    uint32 triggered_spell_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell;
    Unit* target = unitTarget;
    unitTarget->AddDelayedEvent(10, [target, triggered_spell_id]() -> void
    {
        if (target)
            target->RemoveAurasDueToSpell(triggered_spell_id);
    });
}

void Spell::EffectDamageFromMaxHealthPCT(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    switch (m_spellInfo->Id)
    {
        case 125372:
        case 246310:
            damage = damage / 100;
            break;
        default:
            break;
    }

    std::vector<uint32> ExcludeAuraList;
    uint32 tempDmg = unitTarget->CountPctFromMaxHealth(damage, m_caster);

    if (!m_spellInfo->HasAttribute(SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS))
    {
        tempDmg = m_caster->SpellDamageBonusDone(unitTarget, m_spellInfo, tempDmg, SPELL_DIRECT_DAMAGE, ExcludeAuraList, effIndex);
        tempDmg = unitTarget->SpellDamageBonusTaken(m_caster, m_spellInfo, tempDmg);
    }
    m_damage += tempDmg;
}

void Spell::EffectGiveCurrency(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    uint32 currencyID = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    float amount = damage * sDB2Manager.GetCurrencyPrecision(currencyID);

    unitTarget->ToPlayer()->ModifyCurrency(currencyID, amount, true);

    // log Veiled Argunite and Wakening Essence currency
    if (currencyID == 1508 || currencyID == 1533)
        sLog->outWarden("Player %s (GUID: %u) adds a currency value %u (%u) from spell %u", unitTarget->ToPlayer()->GetName(), unitTarget->ToPlayer()->GetGUIDLow(), amount, currencyID, m_spellInfo->Id);
}

void Spell::EffectCastButtons(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster->IsPlayer())
        return;

    Player* p_caster = m_caster->ToPlayer();
    uint32 button_id = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue + 132;
    uint32 n_buttons = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB;

    for (; n_buttons; --n_buttons, ++button_id)
    {
        ActionButton const* ab = p_caster->GetActionButton(button_id);
        if (!ab || ab->GetType() != ACTION_BUTTON_SPELL)
            continue;

        //! Action button data is unverified when it's set so it can be "hacked"
        //! to contain invalid spells, so filter here.
        uint32 spell_id = ab->GetAction();
        if (!spell_id)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);
        if (!spellInfo)
            continue;

        if (!p_caster->HasSpell(spell_id) || p_caster->HasSpellCooldown(spell_id))
            continue;

        if (!(spellInfo->HasAttribute(SPELL_ATTR7_SUMMON_TOTEM)))
            continue;

        SpellPowerCost cost;
        spellInfo->CalcPowerCost(m_caster, spellInfo->GetSchoolMask(), cost);
        if (m_caster->GetPower(POWER_MANA) < cost[POWER_MANA])
            continue;

        m_caster->CastSpell(m_caster, spell_id, TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_CAST_DIRECTLY));
    }
}

void Spell::EffectRechargeManaGem(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    uint32 item_id = m_spellInfo->GetEffect(EFFECT_0, m_diffMode)->ItemType;

    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(item_id);
    if (!pProto)
    {
        player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (Item* pItem = player->GetItemByEntry(item_id))
    {
        for (auto const& v : pProto->Effects)
            pItem->SetSpellCharges(v->LegacySlotIndex, v->Charges);

        pItem->SetState(ITEM_CHANGED, player);
    }
}

void Spell::EffectBind(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();

    uint32 area_id;
    WorldLocation loc;
    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetA.GetTarget() == TARGET_DEST_DB || m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == TARGET_DEST_DB)
    {
        SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id);
        if (!st)
        {
            TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::EffectBind - unknown teleport coordinates for spell ID %u", m_spellInfo->Id);
            return;
        }

        loc.m_mapId = st->target_mapId;
        loc.m_positionX = st->target_X;
        loc.m_positionY = st->target_Y;
        loc.m_positionZ = st->target_Z;
        loc.SetOrientation(st->target_Orientation);
        area_id = player->GetCurrentAreaID();
    }
    else
    {
        player->GetPosition(&loc);
        loc.m_mapId = player->GetMapId();
        area_id = player->GetCurrentAreaID();
    }

    player->SetHomebind(loc, area_id);
    player->SendBindPointUpdate();

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "New homebind X      : %f", loc.m_positionX);
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "New homebind Y      : %f", loc.m_positionY);
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "New homebind Z      : %f", loc.m_positionZ);
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "New homebind MapId  : %u", loc.m_mapId);
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "New homebind AreaId : %u", area_id);

    player->SendDirectMessage(WorldPackets::Misc::PlayerBound(player->GetGUID(), area_id).Write());
}

void Spell::EffectSummonRaFFriend(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!m_caster->IsPlayer() || !unitTarget || !unitTarget->IsPlayer())
        return;

    m_caster->CastSpell(unitTarget, m_spellInfo->GetEffect(effIndex, m_diffMode)->TriggerSpell, true);
}

void Spell::EffectUnlearnTalent(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    TalentEntry const* talent = sTalentStore.LookupEntry(m_miscData[0]);
    if (!talent)
        return;

    Player* player = m_caster ? m_caster->ToPlayer() : nullptr;
    if (!player)
        return;

    player->RemoveTalent(talent);
    player->SendTalentsInfoData(false);
}

void Spell::EffectDespawnAreatrigger(SpellEffIndex /*effIndex*/)
{
    if (!m_caster)
        return;

    m_caster->RemoveAllAreaObjects();
}

void Spell::EffectDespawnDynamicObject(SpellEffIndex /*effIndex*/)
{
    if (!m_caster)
        return;

    m_caster->RemoveAllDynObjects();
}

void Spell::EffectCreateAreaTrigger(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Unit* caster = m_originalCaster ? m_originalCaster : m_caster;

    Position pos;
    if (!m_targets.HasDst())
        caster->GetPosition(&pos);
    else
        destTarget->GetPosition(&pos);

    if (Unit* unitTarget = m_targets.GetUnitTarget())
        destAtTarget = static_cast<WorldLocation>(*unitTarget);

    Position posMove;
    destAtTarget.GetPosition(&posMove);

    // trigger entry/miscvalue relation is currently unknown, for now use MiscValue as trigger entry
    uint32 triggerEntry = GetSpellInfo()->GetEffect(effIndex, caster->GetSpawnMode())->MiscValue;
    if (!triggerEntry)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectCreateAreaTrigger pos (%f %f %f) posMove(%f %f %f) HasDst %i", pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), posMove.GetPositionX(), posMove.GetPositionY(), posMove.GetPositionZ(), m_targets.HasDst());

    auto areaTrigger = new AreaTrigger;
    if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), triggerEntry, caster, GetSpellInfo(), pos, posMove, this, ObjectGuid::Empty, 0, m_castGuid[0]))
        delete areaTrigger;
}

void Spell::EffectBuyGuilkBankTab(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!m_caster->IsPlayer())
        return;

    Guild* guild = sGuildMgr->GetGuildById(m_caster->ToPlayer()->GetGuildId());
    if (!guild)
        return;

    if (guild->GetLeaderGUID() != m_caster->GetGUID())
        return;

    guild->HandleSpellEffectBuyBankTab(m_caster->ToPlayer()->GetSession(), damage - 1);
}

void Spell::EffectResurrectWithAura(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsInWorld())
        return;

    Player* target = unitTarget->ToPlayer();
    if (!target)
        return;

    if (unitTarget->isAlive())
        return;

    if (target->IsRessurectRequested())       // already have one active request
        return;

    ExecuteLogEffectResurrect(effIndex, target);
    target->SetResurrectRequestData(m_caster, target->CountPctFromMaxHealth(damage), CalculatePct(target->GetMaxPower(POWER_MANA), damage), 0, m_spellInfo);
    SendResurrectRequest(target);
}

void Spell::EffectSummonRaidMarker(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player || !m_targets.HasDst())
        return;

    Group* group = player->GetGroup();
    if (!group || (group->isRaidGroup() && !group->IsLeader(player->GetGUID()) && !group->IsAssistant(player->GetGUID())))
        return;

    float x, y, z;
    destTarget->GetPosition(x, y, z);

    group->AddRaidMarker(damage, player->GetMapId(), x, y, z);
}

void Spell::EffectCorpseLoot(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget->IsCreature())
        return;
    if (!m_caster->IsPlayer())
        return;

    Creature* creature = unitTarget->ToCreature();
    if (!creature)
        return;

    m_caster->ToPlayer()->SendLoot(creature->GetGUID(), LOOT_CORPSE);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
}

void Spell::EffectRandomizeDigsites(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!sWorld->getBoolConfig(CONFIG_ARCHAEOLOGY_ENABLED))
        return;

    Player* player = m_caster->ToPlayer();
    if (!player || !player->GetSkillValue(SKILL_ARCHAEOLOGY))
        return;

    player->RandomizeSitesInMap(m_spellInfo->GetEffect(effIndex)->MiscValue, damage);
}

void Spell::EffectTeleportToDigsite(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!sWorld->getBoolConfig(CONFIG_ARCHAEOLOGY_ENABLED))
        return;

    Player* player = m_caster->ToPlayer();
    if (!player || !player->GetSkillValue(SKILL_ARCHAEOLOGY))
        return;

    player->TeleportToDigsiteInMap(player->GetMapId());
}

void Spell::EffectUncageBattlePet(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!m_CastItem || m_CastItem->GetEntry() != BATTLE_PET_CAGE_ITEM_ID)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    uint32 speciesID = m_CastItem->GetModifier(ITEM_MODIFIER_BATTLE_PET_SPECIES_ID);

    auto const& speciesEntry = sBattlePetSpeciesStore.LookupEntry(speciesID);
    if (!speciesEntry)
        return;

    uint8 speciesCount = player->GetBattlePetCountForSpecies(speciesID);
    if ((sDB2Manager.HasBattlePetSpeciesFlag(speciesID, BATTLEPET_SPECIES_FLAG_UNIQUE) && speciesCount >= 1) || speciesCount >= MAX_BATTLE_PETS_PER_SPECIES)
    {
        player->GetSession()->SendBattlePetError(BattlePetError::ERR_CANT_HAVE_MORE_PETS_OF_THAT_TYPE, speciesEntry->CreatureID);
        SendCastResult(SPELL_FAILED_CANT_ADD_BATTLE_PET);
        return;
    }

    //if (static_cast<uint16>(battlePetMgr.BattlePets.size()) >= battlePetMgr.GetMaxBattlePets())
    //{
    //    player->GetSession()->SendBattlePetError(BattlePetError::ERR_CANT_HAVE_MORE_PETS, speciesEntry->CreatureID);
    //    SendCastResult(SPELL_FAILED_CANT_ADD_BATTLE_PET);
    //    return;
    //}

    uint32 tempData = m_CastItem->GetModifier(ITEM_MODIFIER_BATTLE_PET_BREED_DATA);

    auto BattlePetPtr = std::make_shared<BattlePet>();
    BattlePetPtr->Slot = PETBATTLE_NULL_SLOT;
    BattlePetPtr->NameTimeStamp = 0;
    BattlePetPtr->Species = speciesID;
    BattlePetPtr->DisplayModelID = m_CastItem->GetModifier(ITEM_MODIFIER_BATTLE_PET_DISPLAY_ID);
    BattlePetPtr->Flags = 0;
    BattlePetPtr->Level = m_CastItem->GetModifier(ITEM_MODIFIER_BATTLE_PET_LEVEL);
    BattlePetPtr->XP = 0;
    BattlePetPtr->Breed = tempData & 0xFFFFFF;
    BattlePetPtr->Quality = (tempData >> 24) & 0xFF;
    BattlePetPtr->UpdateStats();
    BattlePetPtr->Health = BattlePetPtr->InfoMaxHealth;
    BattlePetPtr->AddToPlayer(player);

    player->_battlePets.emplace(BattlePetPtr->JournalID, BattlePetPtr);
    player->GetSession()->SendBattlePetUpdates();

    m_CastItem->SetInUse(false);
    player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), true);
    m_CastItem = nullptr;
}

void Spell::EffectSetMaxBattlePetCount(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    //if (Player* player = m_caster->ToPlayer())
    //    player->GetBattlePetMgr().SetMaxBattlePets(m_spellInfo->GetEffect(effIndex, m_diffMode)->BasePoints);
}

void Spell::EffectGrantBattlePetLevel(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (!m_CastItem)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    auto const& battlePet = player->GetBattlePet(player->GetGuidValue(PLAYER_FIELD_SUMMONED_BATTLE_PET_GUID));
    if (!battlePet)
        return;

    auto const& familyMask = 1 << sBattlePetSpeciesStore.AssertEntry(battlePet->Species)->PetTypeEnum;
    if ((familyMask & m_spellInfo->GetEffect(effIndex)->MiscValue) == 0)
    {
        SendCastResult(SPELL_FAILED_WRONG_BATTLE_PET_TYPE);
        return;
    }
    
    if (battlePet->Level == BATTLEPET_MAX_LEVEL)
    {
        SendCastResult(SPELL_FAILED_CANT_UPGRADE_BATTLE_PET);
        return;
    }

    battlePet->Level += m_spellInfo->GetEffect(effIndex)->BasePoints;
    battlePet->needSave = true;

    if (battlePet->Level > BATTLEPET_MAX_LEVEL)
        battlePet->Level = BATTLEPET_MAX_LEVEL;   

    player->GetSession()->SendBattlePetUpdates(nullptr, true);

    BattlePetSpeciesEntry const* speciesInfo = sBattlePetSpeciesStore.LookupEntry(battlePet->Species);
    if (speciesInfo)
    {
        player->UpdateAchievementCriteria(CRITERIA_TYPE_BATTLEPET_LEVEL_UP, battlePet->Level, speciesInfo->PetTypeEnum, battlePet->Species);
        player->UpdateAchievementCriteria(CRITERIA_TYPE_LEVEL_BATTLE_PET_CREDIT, speciesInfo->ID, battlePet->Level, battlePet->Species);
    }

    m_CastItem->SetInUse(false);
    player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), true);
    m_CastItem = nullptr;
}

void Spell::EffectUpgradeBattlePet(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (!m_CastItem)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    auto const& battlePet = player->GetBattlePet(player->GetGuidValue(PLAYER_FIELD_SUMMONED_BATTLE_PET_GUID));
    if (!battlePet)
        return;

    auto const& familyMask = 1 << sBattlePetSpeciesStore.AssertEntry(battlePet->Species)->PetTypeEnum;
    if ((familyMask & m_spellInfo->GetEffect(effIndex)->MiscValue) == 0)
    {
        SendCastResult(SPELL_FAILED_WRONG_BATTLE_PET_TYPE);
        return;
    }

    uint8 quality = 1;
    switch (RoundingFloatValue(m_spellInfo->GetEffect(effIndex)->BasePoints))
    {
        case 75: // Uncommon
            quality = BATTLEPET_QUALITY_UNCOMMON;
            break;
        case 85: // Rare
            quality = BATTLEPET_QUALITY_RARE;
            break;
        case 98: // Epic
        case 99: // Epic
            quality = BATTLEPET_QUALITY_EPIC;
            break;
        default:
            break;
    }

    if (quality >= BATTLEPET_QUALITY_EPIC)
    {
        SendCastResult(SPELL_FAILED_CANT_UPGRADE_BATTLE_PET);
        return;
    }

    if (battlePet->Quality >= quality)
    {
        SendCastResult(SPELL_FAILED_CANT_UPGRADE_BATTLE_PET);
        return;
    }

    battlePet->Quality = quality;
    battlePet->needSave = true;

    if (battlePet->Level >= 15)
    {
        battlePet->Level = battlePet->Level - 2;
        m_targets.GetUnitTarget()->SetUInt32Value(UNIT_FIELD_WILD_BATTLE_PET_LEVEL, battlePet->Level);
    }

    m_CastItem->SetInUse(false);
    player->GetSession()->SendBattlePetUpdates(nullptr, true);
    player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), true);
    player->SetUInt32Value(PLAYER_FIELD_CURRENT_BATTLE_PET_BREED_QUALITY, battlePet->Quality);
    m_CastItem = nullptr;
}

void Spell::EffectUnlockPetBattles(SpellEffIndex /*effIndex*/)
{
    if (!unitTarget)
        return;

    if (Player* player = unitTarget->ToPlayer())
        player->GetSession()->SendPetBattleSlotUpdates();
}

void Spell::EffectHealBattlePetPct(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (Player* player = m_caster->ToPlayer())
    {
        BattlePetMap* pets = player->GetBattlePets();
        for (auto& pet : *pets)
        {
            pet.second->UpdateStats();
            if (pet.second->Health != pet.second->InfoMaxHealth)
                pet.second->needSave = true;
            pet.second->Health = pet.second->InfoMaxHealth;
        }

        player->GetSession()->SendBattlePetsHealed();
        player->GetSession()->SendBattlePetUpdates();
    }
}

//! Based on SPELL_EFFECT_ACTIVATE_SCENE3 spell 117790
void Spell::SendScene(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET || !m_caster->IsPlayer())
        return;

    Position pos;
    if (m_targets.HasDst())
        pos = static_cast<Position>(*m_targets.GetDstPos());
    else
        m_caster->GetPosition(&pos);

    m_caster->ToPlayer()->SendSpellScene(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue, m_spellInfo, true, &pos);
}

void Spell::EffectBonusLoot(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();

    uint32 lootId = m_spellInfo->Id;
    uint8 lootType = TYPE_CREATURE;

    uint32 spellCooldownId = m_spellInfo->Id;
    PersonalLootData const* plData = sObjectMgr->GetPersonalLootDataBySpell(m_spellInfo->Id);
    if (plData)
    {
        if (m_spellInfo->Id != plData->lootspellId)
            spellCooldownId = plData->bonusspellId;
        if (plData->cooldownid)
        {
            lootId = plData->cooldownid;
            lootType = plData->cooldowntype;
        }
        else
            lootId = plData->entry;
    }

    TC_LOG_DEBUG(LOG_FILTER_LOOT, "Spell::EffectBonusLoot spellCooldownId %i Id %u IsTriggered %u CurrencyID %i", spellCooldownId, m_spellInfo->Id, IsTriggered(), m_spellInfo->Reagents.CurrencyID);

    uint32 difficulty = 0;
    if (unitTarget == m_caster)
    {
        //Loot for boos
        if (plData)
        {
            if (Aura* aura = unitTarget->GetAura(spellCooldownId))
            {
                if (AuraEffect* eff = aura->GetEffect(EFFECT_0))
                    difficulty = eff->GetAmount();
                else
                {
                    aura->Remove();
                    return;
                }
                aura->Remove();
            }
            else
                return;

            if (player->IsPlayerLootCooldown(spellCooldownId, TYPE_SPELL, difficulty))
                return;

            if (m_spellInfo->Id == plData->lootspellId)
            {
                player->AddPlayerLootCooldown(plData->lootspellId, 0, TYPE_SPELL, true, difficulty);
                return;
            }
            // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Spell::EffectBonusLoot lootId %i lootType %i", lootId, lootType);

            Loot* loot = &player->personalLoot[player->GetGUID()];
            loot->clear();
            loot->personal = true;
            loot->isBoss = true;
            loot->_DifficultyID = difficulty;
            loot->bonusLoot = unitTarget == m_caster;
            if (plData)
                loot->chance = plData->chance + 5;
            switch (lootType)
            {
                case TYPE_GO:
                    loot->FillLoot(lootId, LootTemplates_Gameobject, player, true);
                    break;
                case TYPE_CREATURE:
                    loot->FillLoot(lootId, LootTemplates_Creature, player, true);
                    break;
            }
            loot->AutoStoreItems();
            if (loot->gold)
            {
                // TC_LOG_DEBUG(LOG_FILTER_LOOT, "Spell::EffectBonusLoot gold %i bonusLoot %i", loot->gold, loot->bonusLoot);
                player->ModifyMoney(loot->gold);
                player->SendDisplayToast(0, ToastType::MONEY, loot->bonusLoot, loot->gold, DisplayToastMethod::DISPLAY_TOAST_DEFAULT);
                player->UpdateAchievementCriteria(CRITERIA_TYPE_LOOT_MONEY, loot->gold, 0, 0, player);
                loot->gold = 0;
            }
            // if (IsTriggered())
                // TakeReagents();
            player->AddPlayerLootCooldown(spellCooldownId, 0, TYPE_SPELL, true, difficulty);
            return;
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_LOOT, "Spell::EffectBonusLoot difficulty %i spellCooldownId %u", difficulty, spellCooldownId);

    unitTarget->RemoveAurasDueToSpell(sDB2Manager.GetSpellByTrigger(m_spellInfo->Id));

    Loot* loot = &player->personalLoot[player->GetGUID()];
    loot->clear();

    switch (lootId)
    {
        case 240041:
            if (roll_chance_i(15))
            {
                player->PvpRatedQuestReward(44891);
                return;
            }
            break;
        case 240047:
            if (roll_chance_i(15))
            {
                player->PvpRatedQuestReward(44908);
                return;
            }
            break;
        case 240051:
            if (roll_chance_i(15))
            {
                player->PvpRatedQuestReward(44909);
                return;
            }
            break;
    }

    if (m_CastItem)
        if (ItemTemplate const* proto = m_CastItem->GetTemplate())
            if ((proto->AllowableClass && proto->AllowableClass != -1 && proto->AllowableClass <= CLASSMASK_ALL_PLAYABLE) || (proto->AllowableRace && proto->AllowableRace != -1 && proto->AllowableRace <= RACEMASK_ALL_PLAYABLE)) // Don`t check speck if exist check by class or race
                loot->_specCheck = false;

    switch (m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB)
    {
        case 416: // Salvage for Sack of Salvaged Goods
        case 417: // Salvage for Salvage Crate
        case 418: // Salvage for Large Crate of Salvage
        case 435:
            break;
        case 436: // Scale loot item by source item level
            loot->_isTokenLoot = true;
            loot->_itemContext = m_caster->GetMap()->GetDifficultyLootItemContext(false, m_caster->getLevel() == MAX_LEVEL, false);
            if (lootId >= 254773 && lootId <= 254794) // For token 910, have other TreeMod
                loot->_itemContext = 43;
            if (lootId >= 242842 && lootId <= 242864 || lootId == 243074) // For token 880, have other TreeMod
            {
                loot->_itemContext = 43;
                loot->_needLevel = 880;
            }

            if (lootId >= 240485 && lootId <= 240518 || lootId == 262946) // For legendary token
                loot->_isLegendaryLoot = true;
            break;
        case 438: // Bonus loot from boss
            break;
    }

    loot->personal = true;
    if (!loot->FillLoot(lootId, LootTemplates_Spell, player, true) && m_CastItem) // If loot not in spell, get loot from item
        loot->FillLoot(m_CastItem->GetEntry(), LootTemplates_Item, player, true);

    loot->AutoStoreItems();
    uint32 count = 1;

    TC_LOG_DEBUG(LOG_FILTER_LOOT, "Spell::EffectBonusLoot Other loot for item and any lootId %i", lootId);
    if (m_CastItem && !m_spellInfo->Reagents.Reagent[0] && m_CastItem->GetEntry() != 141860) // If exist reagent don1t take item
        player->DestroyItemCount(m_CastItem, count, true);
}

void Spell::EffectUpdatePlayerPhase(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    Player* player = unitTarget->ToPlayer();
    player->AddDelayedEvent(100, [player]() -> void
    {
        player->GetPhaseMgr().RemoveUpdateFlag(PHASE_UPDATE_FLAG_ZONE_UPDATE);
    });
}

void Spell::EffectJoinOrLeavePlayerParty(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !m_caster || !unitTarget->IsPlayer())
       return;

    Player* player = unitTarget->ToPlayer();

    Group* group = player->GetGroup();
    Creature* creature = m_caster->ToCreature();
    if (!creature)
       return;

    if (!group)
    {
        group = new Group();
        group->Create(player);
        // group->ConvertToLFG(dungeon);
        group->SetDungeonDifficultyID(Difficulty(m_diffMode));
        sGroupMgr->AddGroup(group);
    }
    else if (group->IsMember(creature->GetGUID()))
       return;

    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue == 1)
       group->AddCreatureMember(creature);
    else
       group->RemoveCreatureMember(creature->GetGUID());
}

void Spell::EffectSummonConversation(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Position pos;
    if (!m_targets.HasDst())
        GetCaster()->GetPosition(&pos);
    else
        destTarget->GetPosition(&pos);

    // trigger entry/miscvalue relation is currently unknown, for now use MiscValue as trigger entry
    uint32 triggerEntry = GetSpellInfo()->Effects[effIndex]->MiscValue;
    if (!triggerEntry)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::EffectSummonConversation pos (%f %f %f) HasDst %i", pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), m_targets.HasDst());

    auto conversation = new Conversation;
    if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), triggerEntry, GetCaster(), GetSpellInfo(), pos))
        delete conversation;
}

void Spell::EffectLearnGarrisonBuilding(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    Player* player = unitTarget->ToPlayer();
    if (!player)
        return;

    if (Garrison* garrison = player->GetGarrison())
        garrison->LearnBlueprint(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectCreateGarrison(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->CreateGarrison(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectCreateGarrisonShipment(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    if (Garrison *garr = unitTarget->ToPlayer()->GetGarrison())
        garr->CreateGarrisonShipment(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectAddGarrisonFollower(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    Player* player = unitTarget->ToPlayer();
    if (!player)
        return;

    if (Garrison* garrison = player->GetGarrison())
        garrison->AddFollower(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectAddGarrisonMission(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    Player* player = unitTarget->ToPlayer();
    if (!player)
        return;

    if (Garrison* garrison = player->GetGarrison())
        garrison->AddMission(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectActivateGarrisonBuilding(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    if (Garrison* garrison = unitTarget->ToPlayer()->GetGarrison())
        garrison->ActivateBuilding(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectCreateHeirloomItem(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!m_caster->IsPlayer())
        return;

    CollectionMgr* collectionMgr = m_caster->ToPlayer()->GetCollectionMgr();
    if (!collectionMgr)
        return;

    uint32 itemID = m_miscData[0];
    if (!collectionMgr->HasHeirloom(itemID))
        return;

    std::vector<uint32> bonusList;
    bonusList.push_back(collectionMgr->GetHeirloomBonus(itemID));

    DoCreateItem(effIndex, itemID, bonusList);
    ExecuteLogEffectCreateItem(effIndex, itemID);
}

void Spell::EffectChangeItemBonuses(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    Item* item = m_targets.GetItemTarget();
    if (!item || !item->IsSoulBound())
        return;

    uint32 OldItemBonusTree = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
    uint32 NewItemBonusTree = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB;

    if (m_spellInfo->Id == 183484 || m_spellInfo->Id == 249954) // Obliterum
    {
        uint32 minItemLevel = m_spellInfo->GetEffect(EFFECT_3, m_diffMode)->BasePoints;
        uint32 maxItemLevel = m_spellInfo->GetEffect(EFFECT_1, m_diffMode)->BasePoints;
        uint32 itemLevel = item->GetItemLevel(m_caster->getLevel());
        uint32 itemBaseLevel = item->GetTemplate()->GetBaseItemLevel();
        if (minItemLevel < 100)
            minItemLevel = 885; // Check for 249954

        if (sWorld->getBoolConfig(CONFIG_OBLITERUM_LEVEL_ENABLE))
        {
            minItemLevel = sWorld->getIntConfig(CONFIG_OBLITERUM_LEVEL_MIN);
            maxItemLevel = sWorld->getIntConfig(CONFIG_OBLITERUM_LEVEL_MAX);
        }

        if (itemLevel < minItemLevel || itemLevel >= maxItemLevel)
            return;

        uint32 baseItemBonus = m_spellInfo->Id == 183484 ? 666 : 3598;
        uint32 offset_ = (itemLevel - itemBaseLevel) / 5;
        uint32 OldItemBonus = baseItemBonus + offset_;
        uint32 NewItemBonus = baseItemBonus + 1 + offset_;

        std::vector<uint32> bonuses = item->GetDynamicValues(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);

        bool _found = false;
        std::vector<uint32> bonusesNew;

        for (auto const bonus : bonuses)
        {
            bool bonusDel = false;
            if (bonus == OldItemBonus)
            {
                bonusDel = true;
                _found = true;
                break;
            }
            if (!bonusDel)
                bonusesNew.emplace_back(bonus);
        }

        if (!_found)
            return;

        item->ClearDynamicValue(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);
        item->_bonusData.Initialize(item->GetTemplate());

        bonusesNew.emplace_back(NewItemBonus);

        for (uint32 bonusId : bonusesNew)
            item->AddBonuses(bonusId);

        item->SetState(ITEM_CHANGED, player);

        return;
    }

    if (OldItemBonusTree == NewItemBonusTree) // Not release
        return;

    std::set<ItemBonusTreeNodeEntry const*> const* OldBonusTree = sDB2Manager.GetItemBonusSet(OldItemBonusTree);
    std::set<ItemBonusTreeNodeEntry const*> const* NewBonusTre = sDB2Manager.GetItemBonusSet(NewItemBonusTree);

    if (OldBonusTree == nullptr || NewBonusTre == nullptr)
        return;

    std::vector<uint32> bonuses = item->GetDynamicValues(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);

    bool _found = false;
    uint32 _treeMod = 0;
    for (auto const bonus : bonuses)
    {
        for (auto const oldBonus : *OldBonusTree)
        {
            if (bonus == oldBonus->ChildItemBonusListID)
            {
                _found = true;
                _treeMod = oldBonus->ItemContext;
                break;
            }
        }
    }

    if (!_found)
        return;

    std::vector<uint32> bonusesNew;

    for (auto const bonus : bonuses)
    {
        bool bonusDel = false;
        for (auto const oldBonus : *OldBonusTree)
        {
            if (bonus == oldBonus->ChildItemBonusListID && _treeMod == oldBonus->ItemContext)
            {
                bonusDel = true;
                break;
            }
        }
        if (!bonusDel)
            bonusesNew.emplace_back(bonus);
    }

    item->ClearDynamicValue(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);
    item->_bonusData.Initialize(item->GetTemplate());

    for (auto newBonus : *NewBonusTre)
        if (_treeMod == newBonus->ItemContext)
            bonusesNew.emplace_back(newBonus->ChildItemBonusListID);

    for (uint32 bonusId : bonusesNew)
        item->AddBonuses(bonusId);

    item->SetState(ITEM_CHANGED, player);
}

void Spell::EffectUpgradeHeirloom(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    CollectionMgr* collectionMgr = m_caster->ToPlayer()->GetCollectionMgr();
    if (collectionMgr)
        collectionMgr->UpgradeHeirloom(m_miscData[0], m_castItemEntry);
}

void Spell::EffectGieveExperience(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    auto playerTarget = unitTarget->ToPlayer();
    if (!playerTarget)
        return;

    auto questXp = sQuestXPStore.LookupEntry(unitTarget->getLevel());
    if (!questXp || m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB >= 10)
        return;

    float questXpRate = playerTarget->GetPersonnalXpRate() ? playerTarget->GetPersonnalXpRate() : sWorld->getRate(RATE_XP_QUEST);
    if (playerTarget->getLevel() < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        playerTarget->GiveXP(questXp->Difficulty[m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValueB] * questXpRate * playerTarget->GetTotalAuraMultiplier(SPELL_AURA_MOD_XP_QUEST_PCT), nullptr);
}

void Spell::EffectRemovePhase(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->RemovePhase(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectModAssistantEquipmentLevel(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    auto player = m_caster->ToPlayer();
    if (!player)
        return;

    if (auto garrison = player->GetGarrison())
        if (auto follower = garrison->GetFollowerByID(m_miscData[0]))
            follower->ModAssistant(m_spellInfo, player);
}

void Spell::EffectIncreaseFollowerExperience(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    Garrison* garrison = player->GetGarrison();
    if (!garrison)
        return;

    if (auto const& follower = garrison->GetFollowerByID(m_miscData[0]))
        follower->IncreaseFollowerExperience(m_spellInfo, player);
}

void Spell::EffectReTrainFollower(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    if (Garrison* garrison = player->GetGarrison())
        garrison->ReTrainFollower(m_spellInfo, m_miscData[0]);
}

void Spell::EffectChangeFollowerVitality(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (auto player = m_caster->ToPlayer())
        if (auto garrison = player->GetGarrison())
            garrison->ChangeFollowerVitality(m_spellInfo, m_miscData[0]);
}

void Spell::EffectForceEquipItem(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    auto const& item = player->GetItemByEntry(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
    if (!item)
        return;

    return;

    uint16 dest;
    if (player->CanEquipItem(NULL_SLOT, dest, item, true) == EQUIP_ERR_OK)
    {
        player->EquipItem(dest, item, true);
        player->AutoUnequipOffhandIfNeed();
    }
}

void Spell::EffectGiveArtifactPower(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectGiveArtifactPower Id %u", m_spellInfo->Id);

    if (Item* artifact = player->GetArtifactWeapon())
    {
        auto artifactCategoryID = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;
        if (ItemTemplate const* proto = artifact->GetTemplate())
            if (uint32 artifactID = proto->GetArtifactID())
                if (ArtifactEntry const* entry = sArtifactStore.LookupEntry(artifactID))
                    if (entry->ArtifactCategoryID != artifactCategoryID)
                        return;

        artifact->GiveArtifactXp(damage, m_CastItem, uint32(artifactCategoryID));
    }
}

void Spell::EffectGiveArtifactPowerNoBonus(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (!unitTarget)
        return;

    Player* player = unitTarget->ToPlayer();
    if (!player)
        return;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "EffectGiveArtifactPowerNoBonus Id %u", m_spellInfo->Id);

    if (Item* artifact = player->GetArtifactWeapon())
    {
        auto artifactCategotyID = m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue;

        if (ItemTemplate const* proto = artifact->GetTemplate())
            if (uint32 artifactID = proto->GetArtifactID())
                if (ArtifactEntry const* entry = sArtifactStore.LookupEntry(artifactID))
                    if (entry->ArtifactCategoryID != artifactCategotyID)
                        return;

        artifact->GiveArtifactXp(damage, m_CastItem, artifactCategotyID);
    }
}

void Spell::EffectLaunchQuestChoice(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->SendDisplayPlayerChoice(GetCaster()->GetGUID(), m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

void Spell::EffectUpdateZoneAurasAndPhases(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->UpdateAreaDependentAuras(unitTarget->GetCurrentAreaID());
}

void Spell::EffectApplyEnchantIllusion(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!itemTarget)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player || player->GetGUID() != itemTarget->GetOwnerGUID())
        return;

    itemTarget->SetState(ITEM_CHANGED, player);
    itemTarget->SetModifier(ITEM_MODIFIER_ENCHANT_ILLUSION_ALL_SPECS, m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
    if (itemTarget->IsEquipped())
        player->SetUInt16Value(PLAYER_FIELD_VISIBLE_ITEMS + VISIBLE_ITEM_ENCHANTMENT_OFFSET + (itemTarget->GetSlot() * 2), 1, itemTarget->GetVisibleItemVisual(player));

    player->RemoveTradeableItem(itemTarget);
    itemTarget->ClearSoulboundTradeable(player);
}

void Spell::EffectCollectItemAppearancesSet(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    unitTarget->ToPlayer()->GetCollectionMgr()->AddTransmogSet(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue);
}

DelayCastEvent* Spell::GetSpellTriggerDelay(SpellEffIndex effIndex, uint32 triggered_spell_id)
{
    ObjectGuid TargetGUID = unitTarget ? unitTarget->GetGUID() : ObjectGuid::Empty;
    ObjectGuid CasterGUID;
    if (std::vector<SpellTriggerDelay> const* spellTrigger = sSpellMgr->GetSpellTriggerDelay(m_spellInfo->Id))
    {
        Unit* casterTarget = nullptr;
        Unit* triggerTarget = nullptr;
        Unit* _targetaura = m_caster;

        for (const auto& itr : *spellTrigger)
        {
            if (!(itr.effectMask & (1 << effIndex)))
                continue;

            if(itr.targetaura == 1) //get target for aura owner
                _targetaura = (m_originalCaster ? m_originalCaster : m_caster)->GetAnyOwner();
            if (!_targetaura)
                _targetaura = m_caster;

            if(itr.aura > 0 && !_targetaura->HasAura(itr.aura))
                continue;
            if(itr.aura < 0 && _targetaura->HasAura(abs(itr.aura)))
                continue;

            triggered_spell_id = itr.spell_trigger;
            if (itr.target)
                triggerTarget = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(m_caster, unitTarget,
                                                                                                        itr.target, m_targets.GetUnitTarget());
            if (triggerTarget)
                TargetGUID = triggerTarget->GetGUID();

            if (itr.caster)
                casterTarget = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(m_caster, unitTarget,
                                                                                                       itr.caster, m_targets.GetUnitTarget());
            if (casterTarget)
                CasterGUID = casterTarget->GetGUID();
            break;
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "GetSpellTriggerDelay spell %u triggered_spell_id %u TargetGUID %u CasterGUID %u", m_spellInfo->Id, triggered_spell_id, TargetGUID.GetGUIDLow(), CasterGUID.GetGUIDLow());

    if (triggered_spell_id)
        return new DelayCastEvent(*m_caster, TargetGUID, triggered_spell_id, true, CasterGUID);

    return nullptr;
}

void Spell::EffectGiveHonorPoints(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (Player* player = m_caster->ToPlayer())
        player->RewardHonor(nullptr, 1, m_spellInfo->GetEffect(effIndex, m_diffMode)->BasePoints);    
}

void Spell::EffectObliterateItem(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    Item* item = m_targets.GetItemTarget();
    if (!item)
        return;

    ItemTemplate const* proto = item->GetTemplate();
    if (!proto || !(proto->GetFlags3() & ITEM_FLAG3_OBLITERATABLE))
        return;

    uint32 itemLevel = item->GetItemLevel(m_caster->getLevel());
    uint32 currencyId = 0;
    uint32 itemId = 0;
    uint32 addCount = 0;

        if (sWorld->getBoolConfig(CONFIG_PVP_LEVEL_ENABLE))
        {
            uint8 activeSeason = sWorld->getIntConfig(CONFIG_PVP_ACTIVE_SEASON);
            if ((legionPvpItem[activeSeason][0] && proto->GetItemNameDescriptionID() == legionPvpItem[activeSeason][0])
                || (legionPvpItem[activeSeason][1] && proto->GetItemNameDescriptionID() == legionPvpItem[activeSeason][1])) // Legion Season
            {
                switch (proto->GetItemNameDescriptionID())
                {
                    case 13226: // Legion Season 1
                    {
                        currencyId = 1356;
                        if (proto->ItemLevel == 810) // Combatant
                            addCount = 1;
                        else if (itemLevel < 865) // Gladiator
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13227: // Legion Season 1 Elite
                    {
                        currencyId = 1357;
                        if (itemLevel < 875)
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13287: // Legion Season 2
                    {
                        currencyId = 1356;
                        if (proto->ItemLevel == 810) // Combatant
                            addCount = 1;
                        else if (itemLevel < 880) // Gladiator
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13288: // Legion Season 2 Elite
                    {
                        currencyId = 1357;
                        if (itemLevel < 890)
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13295: // Legion Season 3
                    {
                        currencyId = 1356;
                        if (proto->ItemLevel == 810) // Combatant
                            addCount = 1;
                        else if (itemLevel < 895) // Gladiator
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13296: // Legion Season 3 Elite
                    {
                        currencyId = 1357;
                        if (itemLevel < 905)
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13297: // Legion Season 4
                    {
                        currencyId = 1356;
                        if (proto->ItemLevel == 810) // Combatant
                            addCount = 1;
                        else if (itemLevel < 910) // Gladiator
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13298: // Legion Season 4 Elite
                    {
                        currencyId = 1357;
                        if (itemLevel < 920)
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13299: // Legion Season 5
                    {
                        currencyId = 1356;
                        if (proto->ItemLevel == 810) // Combatant
                            addCount = 1;
                        else if (itemLevel < 925) // Gladiator
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13300: // Legion Season 5 Elite
                    {
                        currencyId = 1357;
                        if (itemLevel < 935)
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13311: // Legion Season 6
                    {
                        currencyId = 1356;
                        if (proto->ItemLevel == 810) // Combatant
                            addCount = 1;
                        else if (itemLevel < 940) // Gladiator
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13313: // Legion Season 6 Elite
                    {
                        currencyId = 1357;
                        if (itemLevel < 950)
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13312: // Legion Season 7
                    {
                        currencyId = 1356;
                        if (proto->ItemLevel == 810) // Combatant
                            addCount = 1;
                        else if (itemLevel < 955) // Gladiator
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    case 13314: // Legion Season 7 Elite
                    {
                        currencyId = 1357;
                        if (itemLevel < 965)
                            addCount = 1;
                        else
                            addCount = 15;
                        break;
                    }
                    default:
                        break;
                }

            }
            else
            {
                itemId = 136342;
                if (itemLevel <= 680)
                    addCount = urand(1, 3);
                else
                    addCount = urand((itemLevel - 680) / 5, (itemLevel - 680) / 5 * 3);
            }
        }
        else
        {
            switch (proto->GetItemNameDescriptionID())
            {
                case 13311: // Legion Season 6
                {
                    currencyId = 1356;
                    if (proto->ItemLevel == 810) // Combatant
                        addCount = 1;
                    else if (itemLevel < 930) // Gladiator
                        addCount = 1;
                    else
                        addCount = 15;
                    break;
                }
                case 13313: // Legion Season 6 Elite
                {
                    currencyId = 1357;
                    if (itemLevel < 940)
                        addCount = 1;
                    else
                        addCount = 15;
                    break;
                }
                case 13312: // Legion Season 7
                {
                    currencyId = 1356;
                    if (proto->ItemLevel == 810) // Combatant
                        addCount = 1;
                    else if (itemLevel < 945) // Gladiator
                        addCount = 1;
                    else
                        addCount = 15;
                    break;
                }
                case 13314: // Legion Season 7 Elite
                {
                    currencyId = 1357;
                    if (itemLevel < 955)
                        addCount = 1;
                    else
                        addCount = 15;
                    break;
                }
                default:
                {
                    itemId = 136342;
                    if (itemLevel <= 680)
                        addCount = urand(1, 3);
                    else
                        addCount = urand((itemLevel - 680) / 5, (itemLevel - 680) / 5 * 3);
                }
            }
        }
    

    // Quest replace case
    switch (proto->GetId())
    {
        case 146975: // For quest 46810, 46946
        case 146976:
        case 147417:
            itemId = 146978;
            addCount = 1;
            break;
        case 136352: // For quest 41778
            itemId = 136351;
            addCount = 1;
            break;
    }

    if (!currencyId && !itemId)
        return;

    uint32 count = 1;
    player->DestroyItemCount(item, count, true);

    if (currencyId)
        player->ModifyCurrency(currencyId, addCount, true);

    else if (itemId)
    {
        ItemPosCountVec dest;
        if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, addCount) == EQUIP_ERR_OK)
        {
            if (item = player->StoreNewItem(dest, itemId, true))
            {
                player->SendNewItem(item, addCount, true, false, true);
                //player->SendDisplayToast(itemId, ToastType::ITEM, false, addCount, DisplayToastMethod::ITEM, 0, item);
            }
        }
    }
}

void Spell::EffectBecomeUntargetable(SpellEffIndex /*effIndex*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!unitTarget)
        return;

    std::list<Unit*> members;
    m_caster->GetRaidMembers(members);

    auto const& ownerGUID = m_caster->GetGUID();
    GuidUnorderedSet ignoredPlayers;
    for (auto const& member : members)
    {
        auto memberGUID = member->GetGUID();
        if (memberGUID != ownerGUID && member->IsPlayer())
            ignoredPlayers.insert(memberGUID);
    }

    WorldPackets::Spells::ClearTarget clearTarget;
    clearTarget.Guid = m_caster->GetGUID();
    m_caster->SendMessageToSet(clearTarget.Write(), true, ignoredPlayers);

    unitTarget->getHostileRefManager().UpdateVisibility();

    UnitSet* attackers = unitTarget->getAttackers();
    for (UnitSet::iterator itr = attackers->begin(); itr != attackers->end(); ++itr)
    {
        if (!(*itr)->canSeeOrDetect(unitTarget))
            (*itr)->AttackStop();
    }
}

void Spell::EffectModReputation(SpellEffIndex effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    auto effect = m_spellInfo->GetEffect(effIndex, m_diffMode);

    if (auto player = m_caster->ToPlayer())
        player->GetReputationMgr().ModifyReputation(sFactionStore.LookupEntry(effect->MiscValue), effect->BasePoints);
}
