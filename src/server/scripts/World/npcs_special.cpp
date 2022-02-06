/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Npcs_Special
SD%Complete: 100
SDComment: To be used for special NPCs that are located globally.
SDCategory: NPCs
EndScriptData
*/

/* ContentData
npc_air_force_bots       80%    support for misc (invisible) guard bots in areas where player allowed to fly. Summon guards after a preset time if tagged by spell
npc_lunaclaw_spirit      80%    support for quests 6001/6002 (Body and Heart)
npc_chicken_cluck       100%    support for quest 3861 (Cluck!)
npc_dancing_flames      100%    midsummer event NPC
npc_guardian            100%    guardianAI used to prevent players from accessing off-limits areas. Not in use by SD2
npc_garments_of_quests   80%    NPC's related to all Garments of-quests 5621, 5624, 5625, 5648, 565
npc_injured_patient     100%    patients for triage-quests (6622 and 6624)
npc_doctor              100%    Gustaf Vanhowzen and Gregory Victor, quest 6622 and 6624 (Triage)
npc_mount_vendor        100%    Regular mount vendors all over the world. Display gossip if player doesn't meet the requirements to buy
npc_rogue_trainer        80%    Scripted trainers, so they are able to offer item 17126 for class quest 6681
npc_sayge               100%    Darkmoon event fortune teller, buff player based on answers given
npc_snake_trap_serpents  80%    AI for snakes that summoned by Snake Trap
npc_locksmith            75%    list of keys needs to be confirmed
npc_firework            100%    NPC's summoned by rockets and rocket clusters, for making them cast visual
EndContentData */

#include "Cell.h"
#include "CellImpl.h"
#include "CharmInfo.h"
#include "CombatAI.h"
#include "CreatureTextMgr.h"
#include "GameEventMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "PassiveAI.h"
#include "PetAI.h"
#include "Player.h"
#include "PlayerDefines.h"
#include "ScriptedCreature.h"
#include "ScriptedFollowerAI.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "Vehicle.h"
#include "World.h"
#include "QuestData.h"

/*########
# npc_air_force_bots
#########*/

enum SpawnType
{
    SPAWNTYPE_TRIPWIRE_ROOFTOP,                             // no warning, summon Creature at smaller range
    SPAWNTYPE_ALARMBOT,                                     // cast guards mark and summon npc - if player shows up with that buff duration < 5 seconds attack
};

struct SpawnAssociation
{
    uint32 thisCreatureEntry;
    uint32 spawnedCreatureEntry;
    SpawnType spawnType;
};

enum eEnums
{
    SPELL_GUARDS_MARK               = 38067,
    AURA_DURATION_TIME_LEFT         = 5000
};

float const RANGE_TRIPWIRE          = 15.0f;
float const RANGE_GUARDS_MARK       = 50.0f;

SpawnAssociation spawnAssociations[] =
{
    {2614,  15241, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Alliance)
    {2615,  15242, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Horde)
    {21974, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Area 52)
    {21993, 15242, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Horde - Bat Rider)
    {21996, 15241, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Alliance - Gryphon)
    {21997, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Goblin - Area 52 - Zeppelin)
    {21999, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Alliance)
    {22001, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Horde)
    {22002, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Horde)
    {22003, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Alliance)
    {22063, 21976, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Goblin - Area 52)
    {22065, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Ethereal - Stormspire)
    {22066, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Scryer - Dragonhawk)
    {22068, 22064, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Ethereal - Stormspire)
    {22069, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Stormspire)
    {22070, 22067, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Scryer)
    {22071, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Scryer)
    {22078, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Aldor)
    {22079, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Aldor - Gryphon)
    {22080, 22077, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Aldor)
    {22086, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Sporeggar)
    {22087, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Sporeggar - Spore Bat)
    {22088, 22085, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Sporeggar)
    {22090, 22089, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Toshley's Station - Flying Machine)
    {22124, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Cenarion)
    {22125, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Cenarion - Stormcrow)
    {22126, 22122, SPAWNTYPE_ALARMBOT}                      //Air Force Trip Wire - Rooftop (Cenarion Expedition)
};

class npc_storm_earth_and_fire : public CreatureScript
{
public:
    npc_storm_earth_and_fire() : CreatureScript("npc_storm_earth_and_fire") { }

    struct npc_storm_earth_and_fireAI : ScriptedAI
    {
        npc_storm_earth_and_fireAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 entryId;
        Unit* owner;
        float oldHast;
		bool jumpToTarget;
		bool jumpToOwner;

        void InitializeAI() override
        {
            owner = NULL; 
            if (TempSummon* tempSum = me->ToTempSummon())
                owner = tempSum->GetAnyOwner();
            entryId = me->GetEntry();
            oldHast = 0;
			jumpToTarget = true;
			jumpToOwner = true;
            me->SetReactState(REACT_HELPER);
        }

        void RecalcDamage()
        {
            if (Player* plr = owner->ToPlayer())
            {
                Item* mainItem = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                Item* offItem = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                bool greenClone = entryId == 69792;

                float finaldmg, offfinaldmg = 0;
                float adddmg, adddmg2 = 0;
                float mainattackspeed, offattackspeed = 2.0f;

                if (mainItem)
                {
                    mainItem->GetTemplate()->GetDamage(mainItem->GetItemLevel(), finaldmg, offfinaldmg);
                    mainattackspeed = mainItem->GetTemplate()->GetDelay() / 1000.0f;
                }
                if (offItem && mainItem)
                {
                    mainItem->GetTemplate()->GetDamage(mainItem->GetItemLevel(), adddmg, adddmg2);
                    offattackspeed = mainItem->GetTemplate()->GetDelay() / 1000.0f;
                }

                finaldmg += urand(adddmg, adddmg2);
                float damagePctDone = me->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, SPELL_SCHOOL_MASK_NORMAL);
                finaldmg -= offfinaldmg;

                offfinaldmg = (offfinaldmg + me->GetTotalAttackPowerValue(BASE_ATTACK) / 14.0f) * (offattackspeed ? offattackspeed: mainattackspeed);
                offfinaldmg *= me->GetModifierValue(UNIT_MOD_DAMAGE_OFFHAND, BASE_PCT);
                offfinaldmg += me->GetModifierValue(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE);
                offfinaldmg *= damagePctDone;
                offfinaldmg = CalculatePct(offfinaldmg, 50);

                if (!greenClone)
                {
                    me->SetStatFloatValue(UNIT_FIELD_MIN_OFF_HAND_DAMAGE, offfinaldmg);
                    me->SetStatFloatValue(UNIT_FIELD_MAX_OFF_HAND_DAMAGE, offfinaldmg);
                }

                finaldmg = (finaldmg + me->GetTotalAttackPowerValue(BASE_ATTACK) / 14.0f) * mainattackspeed;
                finaldmg *= me->GetModifierValue(UNIT_MOD_DAMAGE_MAINHAND, BASE_PCT);
                finaldmg += me->GetModifierValue(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE);
                finaldmg *= damagePctDone;

                if (greenClone)
                    finaldmg += offfinaldmg;

                me->SetStatFloatValue(UNIT_FIELD_MIN_DAMAGE, finaldmg);
                me->SetStatFloatValue(UNIT_FIELD_MAX_DAMAGE, finaldmg);
            }
        }

        void RecalcHast()
        {
            if (Player* plr = owner->ToPlayer())
            {
                float amount = plr->GetRatingBonusValue(CR_HASTE_MELEE);
                amount += me->GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_ATTACK_SPEED);

                if (amount == oldHast)
                    return;

                oldHast = amount;

                Item* mainItem = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                Item* offItem = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                float mainattackspeed = BASE_ATTACK_TIME;
                float offattackspeed = 0;

                if (mainItem)
                    mainattackspeed = mainItem->GetTemplate()->GetDelay();

                if (offItem)
                    offattackspeed = offItem->GetTemplate()->GetDelay();

                ApplyPercentModFloatVar(mainattackspeed, amount, false);
                ApplyPercentModFloatVar(offattackspeed, amount, false);

                if (entryId != 69792)
                    me->SetFloatValue(UNIT_FIELD_ATTACK_ROUND_BASE_TIME+OFF_ATTACK, offattackspeed ? offattackspeed: mainattackspeed);

                me->SetFloatValue(UNIT_FIELD_ATTACK_ROUND_BASE_TIME+BASE_ATTACK, mainattackspeed);
                me->SetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE, owner->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE));
                me->SetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED, owner->GetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED));
            }
        }

        void RecalcStats() override
        {
            if (!owner)
                return;

            if (Aura* aura = me->GetAura(138130))
            {
                if (AuraEffect* eff1 = aura->GetEffect(EFFECT_0))
                    if (Aura* ownerAura = owner->GetAura(137639))
                        if (ownerAura->GetEffect(EFFECT_0)->GetAmount() != eff1->GetAmount())
                        {
                            eff1->SetCanBeRecalculated(true);
                            eff1->RecalculateAmount(me);
                        }

                if (AuraEffect* eff4 = aura->GetEffect(EFFECT_3))
                {
                    if (eff4->GetAmount() != owner->GetTotalAttackPowerValue(BASE_ATTACK))
                    {
                        eff4->SetAmount(owner->GetTotalAttackPowerValue(BASE_ATTACK));
                        me->SetUInt32Value(UNIT_FIELD_ATTACK_POWER, me->GetMaxPositiveAuraModifier(SPELL_AURA_MOD_ATTACK_POWER));
                    }
                }

                RecalcHast();
                RecalcDamage();

                if (Player* plr = owner->ToPlayer())
                {
                    me->countCrit = plr->GetFloatValue(PLAYER_FIELD_CRIT_PERCENTAGE);
                    me->m_modMeleeHitChance = plr->m_modMeleeHitChance;
                }
            }
        }

		void UpdateAI(uint32 diff) override
		{
			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			/*if (Spell* spell = owner->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
				if (SpellInfo const* spellInfo = spell->GetSpellInfo())
					if (spellInfo->Id == 117952)
						if (Unit* target = me->getVictim())
						{
							me->SendMeleeAttackStop(target);
							return;
						} Need more knowledge how to prevent jade lightning from interrupting */

			Unit* target = nullptr;

			if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
			{
				if (Unit* owner = me->GetOwner())
				{
					if (Player* player = owner->ToPlayer())
					{
						target = player->GetSelectedUnit();
						if (!target)
							target = owner->getAttackerForHelper();
						if (!target)
							target = me->GetTargetUnit();
					}
				}
			}
			else
			{
				target = me->GetTargetUnit();

				if (!target)
				{
					if (Unit* owner = me->GetOwner())
					{
						target = owner->getAttackerForHelper();
					}
				}

				if (!target)
					jumpToSummoner(owner);
			}

			if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
			{
				if (!target)
				{
					jumpToSummoner(owner);
					me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
					return;
				}

				if (target->IsFriendlyTo(me))
					target = me->getAttackerForHelper();
				if(target)
					if (target->IsFriendlyTo(me))
					{
						jumpToSummoner(owner);
						me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
						return;
					}

				if (!target)
				{
					jumpToSummoner(owner);
					me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
					return;
				}
			}

			jumpToEnemy(target);

			AttackStart(target);

			DoMeleeAttackIfReady();
		}

		void jumpToSummoner(Unit* target)
		{
			if (me->GetDistance(target) <= 35.0f && me->GetDistance(target) >= 10.0f && jumpToOwner)
			{
				me->CastSpell(target, 138104, true);
				jumpToOwner = false;
			}
			else if (me->GetDistance(target) < 10.0f)
			{
				me->GetMotionMaster()->MoveFollow(target, PET_FOLLOW_DIST, me->GetFollowAngle());
				jumpToOwner = true;
			}
		}

		void jumpToEnemy(Unit* target)
		{
			if (me->GetDistance(target) <= 35.0f && me->GetDistance(target) >= 10.0f && jumpToTarget)
			{
				me->CastSpell(target, 138104, true);
				jumpToTarget = false;
			}
			else if (me->GetDistance(target) < 10.0f)
				jumpToTarget = true;
		}
    };

    ScriptedAI* GetAI(Creature* creature) const override
    {
        return new npc_storm_earth_and_fireAI(creature);
    }
};

class npc_air_force_bots : public CreatureScript
{
    public:
        npc_air_force_bots() : CreatureScript("npc_air_force_bots") { }

        struct npc_air_force_botsAI : public ScriptedAI
        {
            npc_air_force_botsAI(Creature* creature) : ScriptedAI(creature)
            {
                SpawnAssoc = NULL;
                SpawnedGUID.Clear();

                // find the correct spawnhandling
                static uint32 entryCount = sizeof(spawnAssociations) / sizeof(SpawnAssociation);

                for (uint8 i = 0; i < entryCount; ++i)
                {
                    if (spawnAssociations[i].thisCreatureEntry == creature->GetEntry())
                    {
                        SpawnAssoc = &spawnAssociations[i];
                        break;
                    }
                }

                if (!SpawnAssoc)
                    TC_LOG_ERROR(LOG_FILTER_SQL, "TCSR: Creature template entry %u has ScriptName npc_air_force_bots, but it's not handled by that script", creature->GetEntry());
                else
                {
                    CreatureTemplate const* spawnedTemplate = sObjectMgr->GetCreatureTemplate(SpawnAssoc->spawnedCreatureEntry);

                    if (!spawnedTemplate)
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "TCSR: Creature template entry %u does not exist in DB, which is required by npc_air_force_bots", SpawnAssoc->spawnedCreatureEntry);
                        SpawnAssoc = NULL;
                        return;
                    }
                }
            }

            SpawnAssociation* SpawnAssoc;
            ObjectGuid SpawnedGUID;

            void Reset() override {}

            Creature* SummonGuard()
            {
                Creature* summoned = me->SummonCreature(SpawnAssoc->spawnedCreatureEntry, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000);

                if (summoned)
                    SpawnedGUID = summoned->GetGUID();
                else
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "TCSR: npc_air_force_bots: wasn't able to spawn Creature %u", SpawnAssoc->spawnedCreatureEntry);
                    SpawnAssoc = NULL;
                }

                return summoned;
            }

            Creature* GetSummonedGuard()
            {
                Creature* creature = Unit::GetCreature(*me, SpawnedGUID);

                if (creature && creature->isAlive())
                    return creature;

                return NULL;
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (!SpawnAssoc)
                    return;

                if (me->IsValidAttackTarget(who))
                {
                    Player* playerTarget = who->ToPlayer();

                    // airforce guards only spawn for players
                    if (!playerTarget)
                        return;

                    if (!playerTarget->isAlive())
                        return;

                    Creature* lastSpawnedGuard = !SpawnedGUID ? NULL : GetSummonedGuard();

                    // prevent calling Unit::GetUnit at next MoveInLineOfSight call - speedup
                    if (!lastSpawnedGuard)
                        SpawnedGUID.Clear();

                    switch (SpawnAssoc->spawnType)
                    {
                        case SPAWNTYPE_ALARMBOT:
                        {
                            if (!who->IsWithinDistInMap(me, RANGE_GUARDS_MARK))
                                return;

                            Aura* markAura = who->GetAura(SPELL_GUARDS_MARK);
                            if (markAura)
                            {
                                // the target wasn't able to move out of our range within 25 seconds
                                if (!lastSpawnedGuard)
                                {
                                    lastSpawnedGuard = SummonGuard();

                                    if (!lastSpawnedGuard)
                                        return;
                                }

                                if (markAura->GetDuration() < AURA_DURATION_TIME_LEFT)
                                    if (!lastSpawnedGuard->getVictim())
                                        lastSpawnedGuard->AI()->AttackStart(who);
                            }
                            else
                            {
                                if (!lastSpawnedGuard)
                                    lastSpawnedGuard = SummonGuard();

                                if (!lastSpawnedGuard)
                                    return;

                                lastSpawnedGuard->CastSpell(who, SPELL_GUARDS_MARK, true);
                            }
                            break;
                        }
                        case SPAWNTYPE_TRIPWIRE_ROOFTOP:
                        {
                            if (!who->IsWithinDistInMap(me, RANGE_TRIPWIRE))
                                return;

                            if (!lastSpawnedGuard)
                                lastSpawnedGuard = SummonGuard();

                            if (!lastSpawnedGuard)
                                return;

                            // ROOFTOP only triggers if the player is on the ground
                            if (!playerTarget->IsFlying() && !lastSpawnedGuard->getVictim())
                                lastSpawnedGuard->AI()->AttackStart(who);

                            break;
                        }
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_air_force_botsAI(creature);
        }
};

/*######
## npc_lunaclaw_spirit
######*/

enum
{
    QUEST_BODY_HEART_A      = 6001,
    QUEST_BODY_HEART_H      = 6002,

    TEXT_ID_DEFAULT         = 4714,
    TEXT_ID_PROGRESS        = 4715
};

#define GOSSIP_ITEM_GRANT   "You have thought well, spirit. I ask you to grant me the strength of your body and the strength of your heart."

class npc_lunaclaw_spirit : public CreatureScript
{
    public:
        npc_lunaclaw_spirit() : CreatureScript("npc_lunaclaw_spirit") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            if (player->GetQuestStatus(QUEST_BODY_HEART_A) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_BODY_HEART_H) == QUEST_STATUS_INCOMPLETE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GRANT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            player->SEND_GOSSIP_MENU(TEXT_ID_DEFAULT, creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            if (action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                player->SEND_GOSSIP_MENU(TEXT_ID_PROGRESS, creature->GetGUID());
                player->AreaExploredOrEventHappens(player->GetTeam() == ALLIANCE ? QUEST_BODY_HEART_A : QUEST_BODY_HEART_H);
            }
            return true;
        }
};

/*########
# npc_chicken_cluck
#########*/
#define QUEST_CLUCK         3861
#define FACTION_FRIENDLY    35
#define FACTION_CHICKEN     31

class npc_chicken_cluck : public CreatureScript
{
    public:
        npc_chicken_cluck() : CreatureScript("npc_chicken_cluck") { }

        struct npc_chicken_cluckAI : public ScriptedAI
        {
            npc_chicken_cluckAI(Creature* creature) : ScriptedAI(creature) {}

            uint32 ResetFlagTimer;

            void Reset() override
            {
                ResetFlagTimer = 120000;
                me->setFaction(FACTION_CHICKEN);
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }

            void EnterCombat(Unit* /*who*/) override {}

            void UpdateAI(uint32 diff) override
            {
                // Reset flags after a certain time has passed so that the next player has to start the 'event' again
                if (me->HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                {
                    if (ResetFlagTimer <= diff)
                    {
                        EnterEvadeMode();
                        return;
                    }
                    else
                        ResetFlagTimer -= diff;
                }

                if (UpdateVictim())
                    DoMeleeAttackIfReady();
            }

            void ReceiveEmote(Player* player, uint32 emote) override
            {
                switch (emote)
                {
                    case TEXT_EMOTE_CHICKEN:
                        if (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_NONE && rand() % 30 == 1)
                        {
                            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                            me->setFaction(FACTION_FRIENDLY);
                            Talk(1);
                        }
                        break;
                    case TEXT_EMOTE_CHEER:
                        if (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_COMPLETE)
                        {
                            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                            me->setFaction(FACTION_FRIENDLY);
                            //Talk(2);
                        }
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_chicken_cluckAI(creature);
        }

        bool OnQuestAccept(Player* /*player*/, Creature* creature, Quest const* quest) override
        {
            if (quest->GetQuestId() == QUEST_CLUCK)
                CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, creature->AI())->Reset();

            return true;
        }

        bool OnQuestComplete(Player* /*player*/, Creature* creature, Quest const* quest) override
        {
            if (quest->GetQuestId() == QUEST_CLUCK)
                CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, creature->AI())->Reset();

            return true;
        }
};

/*######
## npc_dancing_flames
######*/

#define SPELL_BRAZIER       45423
#define SPELL_SEDUCTION     47057
#define SPELL_FIERY_AURA    45427

class npc_dancing_flames : public CreatureScript
{
    public:
        npc_dancing_flames() : CreatureScript("npc_dancing_flames") { }

        struct npc_dancing_flamesAI : public ScriptedAI
        {
            npc_dancing_flamesAI(Creature* creature) : ScriptedAI(creature) {}

            bool Active;
            uint32 CanIteract;

            void Reset() override
            {
                Active = true;
                CanIteract = 3500;
                DoCast(me, SPELL_BRAZIER, true);
                DoCast(me, SPELL_FIERY_AURA, false);
                float x, y, z;
                me->GetPosition(x, y, z);
                me->Relocate(x, y, z + 0.94f);
                me->SetDisableGravity(true);
                me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!Active)
                {
                    if (CanIteract <= diff)
                    {
                        Active = true;
                        CanIteract = 3500;
                        me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                    }
                    else
                        CanIteract -= diff;
                }
            }

            void EnterCombat(Unit* /*who*/) override {}

            void ReceiveEmote(Player* player, uint32 emote) override
            {
                if (me->IsWithinLOS(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ()) && me->IsWithinDistInMap(player, 30.0f))
                {
                    me->SetFacingTo(player);
                    Active = false;

                    switch (emote)
                    {
                        case TEXT_EMOTE_KISS:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_SHY);
                            break;
                        case TEXT_EMOTE_WAVE:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                            break;
                        case TEXT_EMOTE_BOW:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                            break;
                        case TEXT_EMOTE_JOKE:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                            break;
                        case TEXT_EMOTE_DANCE:
                            if (!player->HasAura(SPELL_SEDUCTION))
                                DoCast(player, SPELL_SEDUCTION, true);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_dancing_flamesAI(creature);
        }
};

/*######
## Triage quest
######*/
#define DOCTOR_ALLIANCE     12939
#define DOCTOR_HORDE        12920
#define ALLIANCE_COORDS     7
#define HORDE_COORDS        6

struct Location
{
    float x, y, z, o;
};

static Location AllianceCoords[]=
{
    {-3757.38f, -4533.05f, 14.16f, 3.62f},                      // Top-far-right bunk as seen from entrance
    {-3754.36f, -4539.13f, 14.16f, 5.13f},                      // Top-far-left bunk
    {-3749.54f, -4540.25f, 14.28f, 3.34f},                      // Far-right bunk
    {-3742.10f, -4536.85f, 14.28f, 3.64f},                      // Right bunk near entrance
    {-3755.89f, -4529.07f, 14.05f, 0.57f},                      // Far-left bunk
    {-3749.51f, -4527.08f, 14.07f, 5.26f},                      // Mid-left bunk
    {-3746.37f, -4525.35f, 14.16f, 5.22f},                      // Left bunk near entrance
};

//alliance run to where
#define A_RUNTOX -3742.96f
#define A_RUNTOY -4531.52f
#define A_RUNTOZ 11.91f

static Location HordeCoords[]=
{
    {-1013.75f, -3492.59f, 62.62f, 4.34f},                      // Left, Behind
    {-1017.72f, -3490.92f, 62.62f, 4.34f},                      // Right, Behind
    {-1015.77f, -3497.15f, 62.82f, 4.34f},                      // Left, Mid
    {-1019.51f, -3495.49f, 62.82f, 4.34f},                      // Right, Mid
    {-1017.25f, -3500.85f, 62.98f, 4.34f},                      // Left, front
    {-1020.95f, -3499.21f, 62.98f, 4.34f}                       // Right, Front
};

//horde run to where
#define H_RUNTOX -1016.44f
#define H_RUNTOY -3508.48f
#define H_RUNTOZ 62.96f

uint32 const AllianceSoldierId[3] =
{
    12938,                                                  // 12938 Injured Alliance Soldier
    12936,                                                  // 12936 Badly injured Alliance Soldier
    12937                                                   // 12937 Critically injured Alliance Soldier
};

uint32 const HordeSoldierId[3] =
{
    12923,                                                  //12923 Injured Soldier
    12924,                                                  //12924 Badly injured Soldier
    12925                                                   //12925 Critically injured Soldier
};

/*######
## npc_doctor (handles both Gustaf Vanhowzen and Gregory Victor)
######*/

class npc_doctor : public CreatureScript
{
    public:
        npc_doctor() : CreatureScript("npc_doctor") {}

        struct npc_doctorAI : public ScriptedAI
        {
            npc_doctorAI(Creature* creature) : ScriptedAI(creature) {}

            ObjectGuid PlayerGUID;

            uint32 SummonPatientTimer;
            uint32 SummonPatientCount;
            uint32 PatientDiedCount;
            uint32 PatientSavedCount;

            bool Event;

            GuidList Patients;
            std::vector<Location*> Coordinates;

            void Reset() override
            {
                PlayerGUID.Clear();

                SummonPatientTimer = 10000;
                SummonPatientCount = 0;
                PatientDiedCount = 0;
                PatientSavedCount = 0;

                Patients.clear();
                Coordinates.clear();

                Event = false;

                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            void BeginEvent(Player* player)
            {
                PlayerGUID = player->GetGUID();

                SummonPatientTimer = 10000;
                SummonPatientCount = 0;
                PatientDiedCount = 0;
                PatientSavedCount = 0;

                switch (me->GetEntry())
                {
                    case DOCTOR_ALLIANCE:
                        for (uint8 i = 0; i < ALLIANCE_COORDS; ++i)
                            Coordinates.push_back(&AllianceCoords[i]);
                        break;
                    case DOCTOR_HORDE:
                        for (uint8 i = 0; i < HORDE_COORDS; ++i)
                            Coordinates.push_back(&HordeCoords[i]);
                        break;
                }

                Event = true;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            void PatientDied(Location* point)
            {
                Player* player = Unit::GetPlayer(*me, PlayerGUID);
                if (player && ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)))
                {
                    ++PatientDiedCount;

                    if (PatientDiedCount > 5 && Event)
                    {
                        if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                            player->FailQuest(6624);
                        else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                            player->FailQuest(6622);

                        Reset();
                        return;
                    }

                    Coordinates.push_back(point);
                }
                else
                    // If no player or player abandon quest in progress
                    Reset();
            }

            void PatientSaved(Creature* /*soldier*/, Player* player, Location* point)
            {
                if (player && PlayerGUID == player->GetGUID())
                {
                    if ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                    {
                        ++PatientSavedCount;

                        if (PatientSavedCount == 15)
                        {
                            if (!Patients.empty())
                            {
                                GuidList::const_iterator itr;
                                for (itr = Patients.begin(); itr != Patients.end(); ++itr)
                                {
                                    if (Creature* patient = Unit::GetCreature((*me), *itr))
                                        patient->setDeathState(JUST_DIED);
                                }
                            }

                            if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                                player->AreaExploredOrEventHappens(6624);
                            else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                                player->AreaExploredOrEventHappens(6622);

                            Reset();
                            return;
                        }

                        Coordinates.push_back(point);
                    }
                }
            }

            void UpdateAI(uint32 diff) override;

            void EnterCombat(Unit* /*who*/) override {}
        };

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
        {
            if ((quest->GetQuestId() == 6624) || (quest->GetQuestId() == 6622))
                CAST_AI(npc_doctor::npc_doctorAI, creature->AI())->BeginEvent(player);

            return true;
        }

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_doctorAI(creature);
        }
};

/*#####
## npc_injured_patient (handles all the patients, no matter Horde or Alliance)
#####*/

class npc_injured_patient : public CreatureScript
{
    public:
        npc_injured_patient() : CreatureScript("npc_injured_patient") { }

        struct npc_injured_patientAI : public ScriptedAI
        {
            npc_injured_patientAI(Creature* creature) : ScriptedAI(creature)
            {
                me->setRegeneratingHealth(false);
            }

            ObjectGuid DoctorGUID;
            Location* Coord;
            uint32 tickTimer;

            void Reset() override
            {
                DoctorGUID.Clear();
                Coord = NULL;
                tickTimer = 200;

                //no select
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                //to make them lay with face down
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);

                uint32 mobId = me->GetEntry();

                switch (mobId)
                {                                                   //lower max health
                    case 12923:
                    case 12938:                                     //Injured Soldier
                        me->SetHealth(me->CountPctFromMaxHealth(75));
                        break;
                    case 12924:
                    case 12936:                                     //Badly injured Soldier
                        me->SetHealth(me->CountPctFromMaxHealth(50));
                        break;
                    case 12925:
                    case 12937:                                     //Critically injured Soldier
                        me->SetHealth(me->CountPctFromMaxHealth(25));
                        break;
                }
            }

            void EnterCombat(Unit* /*who*/) override {}

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                me->DespawnOrUnsummon(500);
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                if (caster->GetTypeId() == TYPEID_PLAYER && me->isAlive() && spell->Id == 20804)
                {
                    if ((CAST_PLR(caster)->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (CAST_PLR(caster)->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                        if (DoctorGUID)
                            if (Creature* doctor = Unit::GetCreature(*me, DoctorGUID))
                                CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientSaved(me, CAST_PLR(caster), Coord);

                    //make not selectable
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    //stand up
                    me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
                    Talk(0);

                    uint32 mobId = me->GetEntry();
                    me->SetWalk(false);

                    switch (mobId)
                    {
                        case 12923:
                        case 12924:
                        case 12925:
                            me->GetMotionMaster()->MovePoint(0, H_RUNTOX, H_RUNTOY, H_RUNTOZ);
                            break;
                        case 12936:
                        case 12937:
                        case 12938:
                            me->GetMotionMaster()->MovePoint(0, A_RUNTOX, A_RUNTOY, A_RUNTOZ);
                            break;
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (tickTimer <= diff)
                    tickTimer = 200;
                else
                {
                    tickTimer -= diff;
                    return;
                }

                //lower HP on every world tick makes it a useful counter, not officlone though
                if (me->isAlive() && me->GetHealth() > 6)
                    me->ModifyHealth(-5);

                if (me->isAlive() && me->GetHealth() <= 6)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->setDeathState(JUST_DIED);

                    if (DoctorGUID)
                        if (Creature* doctor = Unit::GetCreature((*me), DoctorGUID))
                            CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientDied(Coord);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_injured_patientAI(creature);
        }
};

void npc_doctor::npc_doctorAI::UpdateAI(uint32 diff)
{
    if (Event && SummonPatientCount >= 20)
    {
        Reset();
        return;
    }

    if (Event)
    {
        if (SummonPatientTimer <= diff)
        {
            if (Coordinates.empty())
                return;

            std::vector<Location*>::iterator itr = Coordinates.begin() + rand() % Coordinates.size();
            uint32 patientEntry = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE:
                    patientEntry = AllianceSoldierId[rand() % 3];
                    break;
                case DOCTOR_HORDE:
                    patientEntry = HordeSoldierId[rand() % 3];
                    break;
                default:
                    TC_LOG_ERROR(LOG_FILTER_TSCR, "Invalid entry for Triage doctor. Please check your database");
                    return;
            }

            if (Location* point = *itr)
            {
                if (Creature* Patient = me->SummonCreature(patientEntry, point->x, point->y, point->z, point->o, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    //303, this flag appear to be required for client side item->spell to work (TARGET_SINGLE_FRIEND)
                    Patient->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

                    Patients.push_back(Patient->GetGUID());
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->DoctorGUID = me->GetGUID();
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->Coord = point;

                    Coordinates.erase(itr);
                }
            }
            SummonPatientTimer = 10000;
            ++SummonPatientCount;
        }
        else
            SummonPatientTimer -= diff;
    }
}

/*######
## npc_garments_of_quests
######*/

//TODO: get text for each NPC

enum eGarments
{
    SPELL_LESSER_HEAL_R2    = 2052,
    SPELL_FORTITUDE_R1      = 1243,

    QUEST_MOON              = 5621,
    QUEST_LIGHT_1           = 5624,
    QUEST_LIGHT_2           = 5625,
    QUEST_SPIRIT            = 5648,
    QUEST_DARKNESS          = 5650,

    ENTRY_SHAYA             = 12429,
    ENTRY_ROBERTS           = 12423,
    ENTRY_DOLF              = 12427,
    ENTRY_KORJA             = 12430,
    ENTRY_DG_KEL            = 12428,

    //used by 12429, 12423, 12427, 12430, 12428, but signed for 12429
    SAY_COMMON_HEALED       = 0,
    SAY_DG_KEL_THANKS       = 1,
    SAY_DG_KEL_GOODBYE      = 2,
    SAY_ROBERTS_THANKS      = 3,
    SAY_ROBERTS_GOODBYE     = 4,
    SAY_KORJA_THANKS        = 5,
    SAY_KORJA_GOODBYE       = 6,
    SAY_DOLF_THANKS         = 7,
    SAY_DOLF_GOODBYE        = 8,
    SAY_SHAYA_THANKS        = 9,
    SAY_SHAYA_GOODBYE       = 0, //signed for 21469
};

class npc_garments_of_quests : public CreatureScript
{
    public:
        npc_garments_of_quests() : CreatureScript("npc_garments_of_quests") { }

        struct npc_garments_of_questsAI : public npc_escortAI
        {
            npc_garments_of_questsAI(Creature* creature) : npc_escortAI(creature) {Reset();}

            ObjectGuid CasterGUID;

            bool IsHealed;
            bool CanRun;

            uint32 RunAwayTimer;

            void Reset() override
            {
                CasterGUID.Clear();

                IsHealed = false;
                CanRun = false;

                RunAwayTimer = 5000;

                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                //expect database to have RegenHealth=0
                me->SetHealth(me->CountPctFromMaxHealth(70));
            }

            void EnterCombat(Unit* /*who*/) override {}

            void SpellHit(Unit* caster, SpellInfo const* Spell) override
            {
                if (Spell->Id == SPELL_LESSER_HEAL_R2 || Spell->Id == SPELL_FORTITUDE_R1)
                {
                    //not while in combat
                    if (me->isInCombat())
                        return;

                    //nothing to be done now
                    if (IsHealed && CanRun)
                        return;

                    if (Player* player = caster->ToPlayer())
                    {
                        switch (me->GetEntry())
                        {
                            case ENTRY_SHAYA:
                                if (player->GetQuestStatus(QUEST_MOON) == QUEST_STATUS_INCOMPLETE)
                                {
                                    if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                    {
                                        Talk(SAY_SHAYA_THANKS, caster->GetGUID());
                                        CanRun = true;
                                    }
                                    else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                    {
                                        CasterGUID = caster->GetGUID();
                                        me->SetStandState(UNIT_STAND_STATE_STAND);
                                        Talk(SAY_COMMON_HEALED, CasterGUID);
                                        IsHealed = true;
                                    }
                                }
                                break;
                            case ENTRY_ROBERTS:
                                if (player->GetQuestStatus(QUEST_LIGHT_1) == QUEST_STATUS_INCOMPLETE)
                                {
                                    if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                    {
                                        Talk(SAY_ROBERTS_THANKS, caster->GetGUID());
                                        CanRun = true;
                                    }
                                    else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                    {
                                        CasterGUID = caster->GetGUID();
                                        me->SetStandState(UNIT_STAND_STATE_STAND);
                                        Talk(SAY_COMMON_HEALED, CasterGUID);
                                        IsHealed = true;
                                    }
                                }
                                break;
                            case ENTRY_DOLF:
                                if (player->GetQuestStatus(QUEST_LIGHT_2) == QUEST_STATUS_INCOMPLETE)
                                {
                                    if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                    {
                                        Talk(SAY_DOLF_THANKS, caster->GetGUID());
                                        CanRun = true;
                                    }
                                    else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                    {
                                        CasterGUID = caster->GetGUID();
                                        me->SetStandState(UNIT_STAND_STATE_STAND);
                                        Talk(SAY_COMMON_HEALED, CasterGUID);
                                        IsHealed = true;
                                    }
                                }
                                break;
                            case ENTRY_KORJA:
                                if (player->GetQuestStatus(QUEST_SPIRIT) == QUEST_STATUS_INCOMPLETE)
                                {
                                    if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                    {
                                        Talk(SAY_KORJA_THANKS, caster->GetGUID());
                                        CanRun = true;
                                    }
                                    else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                    {
                                        CasterGUID = caster->GetGUID();
                                        me->SetStandState(UNIT_STAND_STATE_STAND);
                                        Talk(SAY_COMMON_HEALED, CasterGUID);
                                        IsHealed = true;
                                    }
                                }
                                break;
                            case ENTRY_DG_KEL:
                                if (player->GetQuestStatus(QUEST_DARKNESS) == QUEST_STATUS_INCOMPLETE)
                                {
                                    if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                    {
                                        Talk(SAY_DG_KEL_THANKS, caster->GetGUID());
                                        CanRun = true;
                                    }
                                    else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                    {
                                        CasterGUID = caster->GetGUID();
                                        me->SetStandState(UNIT_STAND_STATE_STAND);
                                        Talk(SAY_COMMON_HEALED, CasterGUID);
                                        IsHealed = true;
                                    }
                                }
                                break;
                        }

                        //give quest credit, not expect any special quest objectives
                        if (CanRun)
                            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                    }
                }
            }

            void WaypointReached(uint32 /*waypointId*/) override {}

            void UpdateAI(uint32 diff) override
            {
                if (CanRun && !me->isInCombat())
                {
                    if (RunAwayTimer <= diff)
                    {
                        if (Unit* unit = Unit::GetUnit(*me, CasterGUID))
                        {
                            switch (me->GetEntry())
                            {
                                case ENTRY_SHAYA:
                                    Talk(SAY_SHAYA_GOODBYE, CasterGUID);
                                    break;
                                case ENTRY_ROBERTS:
                                    Talk(SAY_ROBERTS_GOODBYE, CasterGUID);
                                    break;
                                case ENTRY_DOLF:
                                    Talk(SAY_DOLF_GOODBYE, CasterGUID);
                                    break;
                                case ENTRY_KORJA:
                                    Talk(SAY_KORJA_GOODBYE, CasterGUID);
                                    break;
                                case ENTRY_DG_KEL:
                                    Talk(SAY_DG_KEL_GOODBYE, CasterGUID);
                                    break;
                            }

                            Start(false, true);
                        }
                        else
                            EnterEvadeMode();                       //something went wrong

                        RunAwayTimer = 30000;
                    }
                    else
                        RunAwayTimer -= diff;
                }

                npc_escortAI::UpdateAI(diff);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_garments_of_questsAI(creature);
        }
};

/*######
## npc_guardian
######*/

#define SPELL_DEATHTOUCH                5

class npc_guardian : public CreatureScript
{
    public:
        npc_guardian() : CreatureScript("npc_guardian") {}

        struct npc_guardianAI : public ScriptedAI
        {
            npc_guardianAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset() override
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void EnterCombat(Unit* /*who*/) override
            {
            }

            void UpdateAI(uint32 /*diff*/) override
            {
                if (!UpdateVictim())
                    return;

                if (me->isAttackReady())
                {
                    DoCast(me->getVictim(), SPELL_DEATHTOUCH, true);
                    me->resetAttackTimer();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_guardianAI(creature);
        }
};

class npc_zombie_explosion : public CreatureScript
{
public:
    npc_zombie_explosion() : CreatureScript("npc_zombie_explosion") {}

    struct npc_zombie_explosionAI : public AnyPetAI
    {
        npc_zombie_explosionAI(Creature* creature) : AnyPetAI(creature) {}

        void InitializeAI() override
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(210128))
            {
                me->SetMaxHealth(spellInfo->Effects[EFFECT_1]->BasePoints);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            AnyPetAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_zombie_explosionAI(creature);
    }
};

class npc_fel_guard : public CreatureScript
{
    public:
    npc_fel_guard() : CreatureScript("npc_fel_guard") {}

    struct npc_fel_guardAI : public AnyPetAI
    {
        npc_fel_guardAI(Creature* creature) : AnyPetAI(creature) {}

        uint32 delay;

        void InitializeAI() override
        {
            delay = 200;
        }

        void UpdateAI(uint32 diff) override
        {
            delay += diff;

            if (delay >= 200)
            {
				Unit* target = nullptr;

				if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
				{
					if (Unit* owner = me->GetOwner())
					{
						if (Player* player = owner->ToPlayer())
						{
							target = player->GetSelectedUnit();
							if (!target)
								target = owner->getAttackerForHelper();
							if (!target)
								target = me->GetTargetUnit();
						}
					}
				}
				else
				{
					target = me->GetTargetUnit();

					if (!target)
					{
						if (Unit* owner = me->GetOwner())
						{
							target = owner->getAttackerForHelper();
						}
					}
				}

                if (target)
                {
					if (!target->IsFriendlyTo(me))
					{
						AttackStart(target);

						if (!me->HasCreatureSpellCooldown(89766))
						{
							me->CastSpell(target, 89766, false);
							me->AddCreatureSpellCooldown(89766);
						}
						if (!me->HasCreatureSpellCooldown(30151))
						{
							me->CastSpell(target, 30151, false);
							me->AddCreatureSpellCooldown(30151);
						}
						if (!me->HasCreatureSpellCooldown(89751))
						{
							int32 ener = me->GetPower(POWER_ENERGY);

							if (ener >= 60 && ener < 120)
							{
								me->CastSpell(target, 89751, false);
								me->AddCreatureSpellCooldown(89751);
							}
						}

						if (!me->HasAuraType(SPELL_AURA_DISABLE_ATTACK_AND_CAST))
							me->CastSpell(target, 30213, false);
					}
					else
					{
						AnyPetAI::UpdateAI(delay);
					}
                }
                else
                {
                    AnyPetAI::UpdateAI(delay);
                }
                delay = 0;
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fel_guardAI(creature);
    }
};

class npc_grimoire_imp : public CreatureScript
{
    public:
    npc_grimoire_imp() : CreatureScript("npc_grimoire_imp") {}

    struct npc_grimoire_impAI : public AnyPetAI
    {
        npc_grimoire_impAI(Creature* creature) : AnyPetAI(creature) {}

        bool dispel;
        int32 delay;

        void InitializeAI() override
        {
            dispel = false;
            delay = 0;
        }

        void Follow(Unit* target)
        {
            me->GetMotionMaster()->MovePoint(0, target->GetPosition());
            delay = 1000;
        }

        void UpdateAI(uint32 diff) override
        {
            delay -= diff;

            if (delay <= 0)
            {
                if (Unit* owner = me->GetOwner())
                {
                    if (!dispel)
                    {
                        if (Unit* owner = me->GetOwner())
                            me->CastSpell(owner, 89808, false);

                        dispel = true;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;

					Unit* target = nullptr;

					if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
					{
						if (Unit* owner = me->GetOwner())
						{
							if (Player* player = owner->ToPlayer())
							{
								target = player->GetSelectedUnit();
								if (!target)
									target = owner->getAttackerForHelper();
								if (!target)
									target = me->GetTargetUnit();
							}
						}
					}
					else
					{
						target = me->GetTargetUnit();

						if (!target)
						{
							if (Unit* owner = me->GetOwner())
							{
								target = owner->getAttackerForHelper();
							}
						}
					}

                    if (!target)
                        return;

					if (target->IsFriendlyTo(me))
						target = me->getAttackerForHelper();

					if(!target)
						return;

                    if (!target->IsWithinLOSInMap(me) || target->GetDistance(me) > 35.f)
                    {
                        Follow(target);
                        return;
                    }

					if (me->GetPower(POWER_ENERGY) < 40)
					{
						me->StopMoving();
						return;
					}

                    me->StopMoving();
                    me->CastSpell(target, 3110, false);
                    delay = 0;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_grimoire_impAI(creature);
    }
};

class npc_grimoire_succubus : public CreatureScript
{
    public:
    npc_grimoire_succubus() : CreatureScript("npc_grimoire_succubus") {}

    struct npc_grimoire_succubusAI : public AnyPetAI
    {
        npc_grimoire_succubusAI(Creature* creature) : AnyPetAI(creature) {}

        bool whiplash;
        bool seduction;
        int32 delay;

        void InitializeAI() override
        {
            whiplash = false;
            seduction = false;
            delay = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            delay -= diff;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

			Unit* target = nullptr;

			if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
			{
				if (Unit* owner = me->GetOwner())
				{
					if (Player* player = owner->ToPlayer())
					{
						target = player->GetSelectedUnit();
						if (!target)
							target = owner->getAttackerForHelper();
						if (!target)
							target = me->GetTargetUnit();
					}
				}
			}
			else
			{
				target = me->GetTargetUnit();

				if (!target)
				{
					if (Unit* owner = me->GetOwner())
					{
						target = owner->getAttackerForHelper();
					}
				}
			}

            if (!target)
                return;

			if (target->IsFriendlyTo(me))
				target = me->getAttackerForHelper();

			if (!target)
				return;

            if (!seduction)
            {
                me->CastSpell(target, 6358, false);
                seduction = true;
                return;
            }

            if (!whiplash)
            {
                me->CastSpell(target, 6360, false);
                whiplash = true;
                delay = 3000;
            }

            AttackStart(target);

            if (delay <= 0)
            {
                me->CastSpell(target, 7814, false);
                delay = 3000;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_grimoire_succubusAI(creature);
    }
};

class npc_grimoire_felhunter : public CreatureScript
{
    public:
    npc_grimoire_felhunter() : CreatureScript("npc_grimoire_felhunter") {}

    struct npc_grimoire_felhunterAI : public AnyPetAI
    {
        npc_grimoire_felhunterAI(Creature* creature) : AnyPetAI(creature) {}

        bool spellLock;
        int32 delay;

        void InitializeAI() override
        {
            spellLock = false;
            delay = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            delay -= diff;

			Unit* target = nullptr;

			if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
			{
				if (Unit* owner = me->GetOwner())
				{
					if (Player* player = owner->ToPlayer())
					{
						target = player->GetSelectedUnit();
						if (!target)
							target = owner->getAttackerForHelper();
						if (!target)
							target = me->GetTargetUnit();
					}
				}
			}
			else
			{
				target = me->GetTargetUnit();

				if (!target)
				{
					if (Unit* owner = me->GetOwner())
					{
						target = owner->getAttackerForHelper();
					}
				}
			}

            if (!target)
                return;

			if (target->IsFriendlyTo(me))
				target = me->getAttackerForHelper();

			if (!target)
				return;

            if (!spellLock)
            {
                me->CastSpell(target, 19647, false);
                delay = 3000;
                spellLock = true;
            }

            AttackStart(target);

            if (delay <= 0)
            {
                me->CastSpell(target, 54049, false);
                delay = 3000;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_grimoire_felhunterAI(creature);
    }
};

class npc_grimoire_voidwalker : public CreatureScript
{
    public:
    npc_grimoire_voidwalker() : CreatureScript("npc_grimoire_voidwalker") {}

    struct npc_grimoire_voidwalkerAI : public AnyPetAI
    {
        npc_grimoire_voidwalkerAI(Creature* creature) : AnyPetAI(creature) {}

        bool suffering;
        int32 delay;

        void InitializeAI() override
        {
            suffering = false;
            delay = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            delay -= diff;

			Unit* target = nullptr;

			if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
			{
				if (Unit* owner = me->GetOwner())
				{
					if (Player* player = owner->ToPlayer())
					{
						target = player->GetSelectedUnit();
						if(!target)
							target = owner->getAttackerForHelper();
						if (!target)
							target = me->GetTargetUnit();
					}
				}
			}
			else
			{
				target = me->GetTargetUnit();

				if (!target)
				{
					if (Unit* owner = me->GetOwner())
					{
						target = owner->getAttackerForHelper();
					}
				}
			}

            if (!target)
                return;

			if (target->IsFriendlyTo(me))
				target = me->getAttackerForHelper();

			if (!target)
				return;

            if (!suffering)
            {
                me->CastSpell(target, 17735, false);
                delay = 3000;
                suffering = true;
            }

            AttackStart(target);

            if (delay <= 0)
            {
                me->CastSpell(target, 3716, false);
                delay = 3000;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_grimoire_voidwalkerAI(creature);
    }
};

// Entry = 94358
class npc_pal_echo_of_the_highlord : public CreatureScript
{
    public:
    npc_pal_echo_of_the_highlord() : CreatureScript("npc_pal_echo_of_the_highlord") {}

    struct npc_pal_echo_of_the_highlord_spiritAI : public ScriptedAI
    {
        npc_pal_echo_of_the_highlord_spiritAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 stage;
        uint32 _diff;
        ObjectGuid targetGUID;

        void InitializeAI() override
        {
            _diff = 0;
            stage = 0;
            me->AddUnitState(UNIT_STATE_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            me->SetUInt32Value(UNIT_FIELD_SHAPESHIFT_FORM, 1);

            if (Unit* owner = me->GetOwner())
            {
                if (Player* plr = owner->ToPlayer())
                {
                    me->SetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS, 120978);
                    me->SetUInt16Value(UNIT_FIELD_VIRTUAL_ITEMS + 1, 0, plr->GetUInt16Value(PLAYER_FIELD_VISIBLE_ITEMS + 31, 0));
                }

                if (Unit* victim = owner->getVictim())
                {
                    me->SetInFront(victim);
                    targetGUID = victim->GetGUID();
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (stage >= 2)
                return;

            _diff += diff;

            if (!stage)
            {
                me->CastSpell(me, 186902, true);
                stage++;
            }

            if (_diff > 500)
            {
                if (Unit* owner = me->GetOwner())
                {
                    if (Unit* target = ObjectAccessor::GetUnit(*me, targetGUID))
                    {
                        me->CastSpell(me, 186802, true);
                        me->CastSpell(target, 224266, true, NULL, NULL, owner->GetGUID());
                    }

                    me->DespawnOrUnsummon(1);
                    stage++;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pal_echo_of_the_highlord_spiritAI(creature);
    }
};

// Entry = 94365
class npc_pal_echo_of_the_highlord1 : public CreatureScript
{
    public:
    npc_pal_echo_of_the_highlord1() : CreatureScript("npc_pal_echo_of_the_highlord1") {}

    struct npc_pal_echo_of_the_highlord1_spiritAI : public ScriptedAI
    {
        npc_pal_echo_of_the_highlord1_spiritAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 stage;
        uint32 _diff;

        void InitializeAI() override
        {
            _diff = 0;
            stage = 0;
            me->AddUnitState(UNIT_STATE_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            me->SetUInt32Value(UNIT_FIELD_SHAPESHIFT_FORM, 1);
        }

        void UpdateAI(uint32 diff) override
        {
            if (stage >= 2)
                return;

            _diff += diff;

            if (!stage)
            {
                if (_diff > 600)
                {
                    if (Unit* owner = me->GetOwner())
                    {
                        me->CastSpell(me, 53385, true, NULL, NULL, owner->GetGUID());
                        me->CastSpell(me, 186902, true);

                        if (Player* plr = owner->ToPlayer())
                        {
                            me->SetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS, 120978);
                            me->SetUInt16Value(UNIT_FIELD_VIRTUAL_ITEMS + 1, 0, plr->GetUInt16Value(PLAYER_FIELD_VISIBLE_ITEMS + 31, 0));
                        }
                    }

                    stage++;
                }
            }
            else
            {
                if (_diff > 1300)
                {
                    me->SetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS, 0);
                    stage = 2;
                    return;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pal_echo_of_the_highlord1_spiritAI(creature);
    }
};

class npc_monk_wind_spirit : public CreatureScript
{
public:
    npc_monk_wind_spirit() : CreatureScript("npc_monk_wind_spirit") {}

    struct npc_monk_wind_spiritAI : public ScriptedAI
    {
        npc_monk_wind_spiritAI(Creature* creature) : ScriptedAI(creature) {}

        bool firstUpdate;
        Unit* owner;
        Unit* target;

        void InitializeAI() override
        {
            firstUpdate = true;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            me->AddUnitState(UNIT_STATE_ROOT);
            target = NULL;
            owner = me->GetOwner();
            if (owner)
            {
                float dist = 40.f;
                ObjectGuid ownerGUID = owner->GetGUID();
                std::list<Unit*> targetList;
                std::vector<uint32> auraId = {123154};

                owner->TargetsWhoHasMyAuras(targetList, auraId);

                for (auto itr : targetList)
                {
                    float curDist = owner->GetExactDist(itr);

                    if (curDist < dist)
                    {
                        dist = curDist;
                        target = itr;
                    }
                }

                if (target)
                {
                    me->AddAura(196051, me);
                    owner->CastSpell(me, 45204, true);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (firstUpdate && owner && owner->IsInWorld())
            {
                if (target && target->IsInWorld())
                {
                    me->ClearUnitState(UNIT_STATE_ROOT);
                    me->SetTargetGUID(target->GetGUID());
                    me->CastSpell(target, 196052, true);
                }
                else
                {
                    me->RemoveAurasDueToSpell(45204);
                    me->RemoveAurasDueToSpell(196051);
                }
                firstUpdate = false;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_monk_wind_spiritAI(creature);
    }
};

/*######
## npc_mount_vendor
######*/

class npc_mount_vendor : public CreatureScript
{
    public:
        npc_mount_vendor() : CreatureScript("npc_mount_vendor") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            if (creature->isQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            bool canBuy = false;
            uint32 vendor = creature->GetEntry();
            uint8 race = player->getRace();

            switch (vendor)
            {
                case 384:                                           //Katie Hunter
                case 1460:                                          //Unger Statforth
                case 2357:                                          //Merideth Carlson
                case 4885:                                          //Gregor MacVince
                    if (player->GetReputationRank(72) != REP_EXALTED && race != RACE_HUMAN)
                        player->SEND_GOSSIP_MENU(5855, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 1261:                                          //Veron Amberstill
                    if (player->GetReputationRank(47) != REP_EXALTED && race != RACE_DWARF)
                        player->SEND_GOSSIP_MENU(5856, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 3362:                                          //Ogunaro Wolfrunner
                    if (player->GetReputationRank(76) != REP_EXALTED && race != RACE_ORC)
                        player->SEND_GOSSIP_MENU(5841, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 3685:                                          //Harb Clawhoof
                    if (player->GetReputationRank(81) != REP_EXALTED && race != RACE_TAUREN)
                        player->SEND_GOSSIP_MENU(5843, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 4730:                                          //Lelanai
                    if (player->GetReputationRank(69) != REP_EXALTED && race != RACE_NIGHTELF)
                        player->SEND_GOSSIP_MENU(5844, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 4731:                                          //Zachariah Post
                    if (player->GetReputationRank(68) != REP_EXALTED && race != RACE_UNDEAD_PLAYER)
                        player->SEND_GOSSIP_MENU(5840, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 7952:                                          //Zjolnir
                    if (player->GetReputationRank(530) != REP_EXALTED && race != RACE_TROLL)
                        player->SEND_GOSSIP_MENU(5842, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 7955:                                          //Milli Featherwhistle
                    if (player->GetReputationRank(54) != REP_EXALTED && race != RACE_GNOME)
                        player->SEND_GOSSIP_MENU(5857, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 16264:                                         //Winaestra
                    if (player->GetReputationRank(911) != REP_EXALTED && race != RACE_BLOODELF)
                        player->SEND_GOSSIP_MENU(10305, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 17584:                                         //Torallius the Pack Handler
                    if (player->GetReputationRank(930) != REP_EXALTED && race != RACE_DRAENEI)
                        player->SEND_GOSSIP_MENU(10239, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 48510:                                         //Kall Worthalon
                    if (player->GetReputationRank(1133) != REP_EXALTED && race != RACE_GOBLIN)
                        player->SEND_GOSSIP_MENU(30002, creature->GetGUID());
                    else canBuy = true;
                    break;
                case 65068:                                         //Old Whitenose
                    canBuy = true;
                    break;
                case 66022:                                         //Turtlemaster Odai
                    canBuy = true;
                    break;
            }

            if (canBuy)
            {
                if (creature->isVendor())
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
                player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            }
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            if (action == GOSSIP_ACTION_TRADE)
                player->GetSession()->SendListInventory(creature->GetGUID());

            return true;
        }
};

/*######
## npc_rogue_trainer
######*/

#define GOSSIP_HELLO_ROGUE1 "I wish to unlearn my talents"
#define GOSSIP_HELLO_ROGUE2 "<Take the letter>"
#define GOSSIP_HELLO_ROGUE3 "Purchase a Dual Talent Specialization."
#define GOSSIP_HELLO_ROGUE4 "I wish to unlearn my specialization"

class npc_rogue_trainer : public CreatureScript
{
    public:
        npc_rogue_trainer() : CreatureScript("npc_rogue_trainer") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            if (creature->isQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if (creature->isTrainer())
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

            if (creature->isCanTrainingAndResetTalentsOf(player))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE1, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_UNLEARNTALENTS);

            if (player->getClass() == CLASS_ROGUE && player->getLevel() >= 24 && !player->HasItemCount(17126) && !player->GetQuestRewardStatus(6681))
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_ROGUE2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(5996, creature->GetGUID());
            } else
                player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, 21100, false);
                    break;
                case GOSSIP_ACTION_TRAIN:
                    player->GetSession()->SendTrainerList(creature->GetGUID());
                    break;
                case GOSSIP_OPTION_UNLEARNTALENTS:
                    player->CLOSE_GOSSIP_MENU();
                    player->SendTalentWipeConfirm(creature->GetGUID(), RESPEC_TYPE_TALENTS);
                    break;
            }
            return true;
        }
};

/*######
## npc_sayge
######*/

#define SPELL_DMG       23768                               //dmg
#define SPELL_RES       23769                               //res
#define SPELL_ARM       23767                               //arm
#define SPELL_SPI       23738                               //spi
#define SPELL_INT       23766                               //int
#define SPELL_STM       23737                               //stm
#define SPELL_STR       23735                               //str
#define SPELL_AGI       23736                               //agi
#define SPELL_FORTUNE   23765                               //faire fortune

#define GOSSIP_HELLO_SAYGE  "Yes"
#define GOSSIP_SENDACTION_SAYGE1    "Slay the Man"
#define GOSSIP_SENDACTION_SAYGE2    "Turn him over to liege"
#define GOSSIP_SENDACTION_SAYGE3    "Confiscate the corn"
#define GOSSIP_SENDACTION_SAYGE4    "Let him go and have the corn"
#define GOSSIP_SENDACTION_SAYGE5    "Execute your friend painfully"
#define GOSSIP_SENDACTION_SAYGE6    "Execute your friend painlessly"
#define GOSSIP_SENDACTION_SAYGE7    "Let your friend go"
#define GOSSIP_SENDACTION_SAYGE8    "Confront the diplomat"
#define GOSSIP_SENDACTION_SAYGE9    "Show not so quiet defiance"
#define GOSSIP_SENDACTION_SAYGE10   "Remain quiet"
#define GOSSIP_SENDACTION_SAYGE11   "Speak against your brother openly"
#define GOSSIP_SENDACTION_SAYGE12   "Help your brother in"
#define GOSSIP_SENDACTION_SAYGE13   "Keep your brother out without letting him know"
#define GOSSIP_SENDACTION_SAYGE14   "Take credit, keep gold"
#define GOSSIP_SENDACTION_SAYGE15   "Take credit, share the gold"
#define GOSSIP_SENDACTION_SAYGE16   "Let the knight take credit"
#define GOSSIP_SENDACTION_SAYGE17   "Thanks"

class npc_sayge : public CreatureScript
{
    public:
        npc_sayge() : CreatureScript("npc_sayge") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            if (creature->isQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if (player->HasSpellCooldown(SPELL_INT) ||
                player->HasSpellCooldown(SPELL_ARM) ||
                player->HasSpellCooldown(SPELL_DMG) ||
                player->HasSpellCooldown(SPELL_RES) ||
                player->HasSpellCooldown(SPELL_STR) ||
                player->HasSpellCooldown(SPELL_AGI) ||
                player->HasSpellCooldown(SPELL_STM) ||
                player->HasSpellCooldown(SPELL_SPI))
                player->SEND_GOSSIP_MENU(7393, creature->GetGUID());
            else
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_SAYGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(7339, creature->GetGUID());
            }

            return true;
        }

        void SendAction(Player* player, Creature* creature, uint32 action)
        {
            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1:
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE1,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE2,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE3,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE4,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                    player->SEND_GOSSIP_MENU(7340, creature->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2:
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE5,            GOSSIP_SENDER_MAIN + 1, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE6,            GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE7,            GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                    player->SEND_GOSSIP_MENU(7341, creature->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 3:
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE8,            GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE9,            GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE10,           GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                    player->SEND_GOSSIP_MENU(7361, creature->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 4:
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE11,           GOSSIP_SENDER_MAIN + 6, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE12,           GOSSIP_SENDER_MAIN + 7, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE13,           GOSSIP_SENDER_MAIN + 8, GOSSIP_ACTION_INFO_DEF);
                    player->SEND_GOSSIP_MENU(7362, creature->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 5:
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE14,           GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE15,           GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE16,           GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                    player->SEND_GOSSIP_MENU(7363, creature->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF:
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE17,           GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                    player->SEND_GOSSIP_MENU(7364, creature->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 6:
                    creature->CastSpell(player, SPELL_FORTUNE, false);
                    player->SEND_GOSSIP_MENU(7365, creature->GetGUID());
                    break;
            }
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            switch (sender)
            {
                case GOSSIP_SENDER_MAIN:
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 1:
                    creature->CastSpell(player, SPELL_DMG, false);
                    player->AddSpellCooldown(SPELL_DMG, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 2:
                    creature->CastSpell(player, SPELL_RES, false);
                    player->AddSpellCooldown(SPELL_RES, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 3:
                    creature->CastSpell(player, SPELL_ARM, false);
                    player->AddSpellCooldown(SPELL_ARM, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 4:
                    creature->CastSpell(player, SPELL_SPI, false);
                    player->AddSpellCooldown(SPELL_SPI, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 5:
                    creature->CastSpell(player, SPELL_INT, false);
                    player->AddSpellCooldown(SPELL_INT, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 6:
                    creature->CastSpell(player, SPELL_STM, false);
                    player->AddSpellCooldown(SPELL_STM, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 7:
                    creature->CastSpell(player, SPELL_STR, false);
                    player->AddSpellCooldown(SPELL_STR, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
                case GOSSIP_SENDER_MAIN + 8:
                    creature->CastSpell(player, SPELL_AGI, false);
                    player->AddSpellCooldown(SPELL_AGI, 0, getPreciseTime() + 7200.0);
                    SendAction(player, creature, action);
                    break;
            }
            return true;
        }
};

class npc_steam_tonk : public CreatureScript
{
    public:
        npc_steam_tonk() : CreatureScript("npc_steam_tonk") { }

        struct npc_steam_tonkAI : public ScriptedAI
        {
            npc_steam_tonkAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset() override {}
            void EnterCombat(Unit* /*who*/) override {}

            void OnPossess(bool apply)
            {
                if (apply)
                {
                    // Initialize the action bar without the melee attack command
                    me->InitCharmInfo();
                    me->GetCharmInfo()->InitEmptyActionBar(false);

                    me->SetReactState(REACT_PASSIVE);
                }
                else
                    me->SetReactState(REACT_AGGRESSIVE);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_steam_tonkAI(creature);
        }
};

#define SPELL_TONK_MINE_DETONATE 25099

class npc_tonk_mine : public CreatureScript
{
    public:
        npc_tonk_mine() : CreatureScript("npc_tonk_mine") { }

        struct npc_tonk_mineAI : public ScriptedAI
        {
            npc_tonk_mineAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            uint32 ExplosionTimer;

            void Reset() override
            {
                ExplosionTimer = 3000;
            }

            void EnterCombat(Unit* /*who*/) override {}
            void AttackStart(Unit* /*who*/) override {}
            void MoveInLineOfSight(Unit* /*who*/) override {}

            void UpdateAI(uint32 diff) override
            {
                if (ExplosionTimer <= diff)
                {
                    DoCast(me, SPELL_TONK_MINE_DETONATE, true);
                    me->setDeathState(DEAD); // unsummon it
                }
                else
                    ExplosionTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_tonk_mineAI(creature);
        }
};

/*####
## npc_brewfest_reveler
####*/

class npc_brewfest_reveler : public CreatureScript
{
    public:
        npc_brewfest_reveler() : CreatureScript("npc_brewfest_reveler") { }

        struct npc_brewfest_revelerAI : public ScriptedAI
        {
            npc_brewfest_revelerAI(Creature* creature) : ScriptedAI(creature) {}
            void ReceiveEmote(Player* player, uint32 emote) override
            {
                if (!IsHolidayActive(HOLIDAY_BREWFEST))
                    return;

                if (emote == TEXT_EMOTE_DANCE)
                    me->CastSpell(player, 41586, false);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_brewfest_revelerAI(creature);
        }
};

/*####
## npc_brewfest_trigger
####*/
enum eBrewfestBarkQuests
{
    BARK_FOR_THE_THUNDERBREWS       = 11294,
    BARK_FOR_TCHALIS_VOODOO_BREWERY = 11408,
    BARK_FOR_THE_BARLEYBREWS        = 11293,
    BARK_FOR_DROHNS_DISTILLERY      = 11407
};

class npc_brewfest_trigger : public CreatureScript
{
public:
    npc_brewfest_trigger() : CreatureScript("npc_brewfest_trigger") { }

    struct npc_brewfest_triggerAI : public ScriptedAI
    {
        npc_brewfest_triggerAI(Creature* c) : ScriptedAI(c) {}
        void MoveInLineOfSight(Unit *who) override
        {
            Player *pPlayer = who->ToPlayer();
            if (!pPlayer)
                return;
            if (pPlayer->GetQuestStatus(BARK_FOR_THE_THUNDERBREWS) == QUEST_STATUS_INCOMPLETE
                || pPlayer->GetQuestStatus(BARK_FOR_TCHALIS_VOODOO_BREWERY) == QUEST_STATUS_INCOMPLETE
                || pPlayer->GetQuestStatus(BARK_FOR_THE_BARLEYBREWS) == QUEST_STATUS_INCOMPLETE
                || pPlayer->GetQuestStatus(BARK_FOR_DROHNS_DISTILLERY) == QUEST_STATUS_INCOMPLETE)
                pPlayer->KilledMonsterCredit(me->GetEntry(), ObjectGuid::Empty);
        }
    };

    CreatureAI *GetAI(Creature *creature) const override
    {
        return new npc_brewfest_triggerAI(creature);
    }
};

/*####
## npc_brewfest_apple_trigger
####*/

#define SPELL_RAM_FATIGUE    43052

class npc_brewfest_apple_trigger : public CreatureScript
{
public:
    npc_brewfest_apple_trigger() : CreatureScript("npc_brewfest_apple_trigger") { }

    struct npc_brewfest_apple_triggerAI : public ScriptedAI
    {
        npc_brewfest_apple_triggerAI(Creature* c) : ScriptedAI(c) {}
        void MoveInLineOfSight(Unit *who) override
        {
            Player *pPlayer = who->ToPlayer();
            if (!pPlayer)
                return;
            if (pPlayer->HasAura(SPELL_RAM_FATIGUE) && me->GetDistance(pPlayer->GetPositionX(),pPlayer->GetPositionY(),pPlayer->GetPositionZ()) <= 7.5f)
                pPlayer->RemoveAura(SPELL_RAM_FATIGUE);
        }
    };

    CreatureAI *GetAI(Creature *creature) const override
    {
        return new npc_brewfest_apple_triggerAI(creature);
    }
};

/*####
## npc_brewfest_keg_thrower
####*/

enum eBrewfestKegThrower
{
    SPELL_BREWFEST_RAM   = 43880,
    SPELL_THROW_KEG      = 43660,
    ITEM_BREWFEST_KEG    = 33797
};

class npc_brewfest_keg_thrower : public CreatureScript
{
public:
    npc_brewfest_keg_thrower() : CreatureScript("npc_brewfest_keg_thrower") { }

    struct npc_brewfest_keg_throwerAI : public ScriptedAI
    {
        npc_brewfest_keg_throwerAI(Creature* c) : ScriptedAI(c) {}
        void MoveInLineOfSight(Unit *who) override
        {
            Player *pPlayer = who->ToPlayer();
            if (!pPlayer)
                return;
            if (pPlayer->HasAura(SPELL_BREWFEST_RAM) 
                && me->GetDistance(pPlayer->GetPositionX(),pPlayer->GetPositionY(),pPlayer->GetPositionZ()) <= 25.0f
                && !pPlayer->HasItemCount(ITEM_BREWFEST_KEG,1))
            {
                me->CastSpell(pPlayer,SPELL_THROW_KEG,false);
                me->CastSpell(pPlayer,42414,false);
            }
        }
    };

    CreatureAI *GetAI(Creature *creature) const override
    {
        return new npc_brewfest_keg_throwerAI(creature);
    }
};

/*####
## npc_brewfest_keg_receiver
####*/

enum eBrewfestKegReceiver
{
    SPELL_CREATE_TICKETS            = 44501,
    QUEST_THERE_AND_BACK_AGAIN_A    = 11122,
    QUEST_THERE_AND_BACK_AGAIN_H    = 11412,
    NPC_BREWFEST_DELIVERY_BUNNY     = 24337   
};

class npc_brewfest_keg_receiver : public CreatureScript
{
public:
    npc_brewfest_keg_receiver() : CreatureScript("npc_brewfest_keg_receiver") { }

    struct npc_brewfest_keg_receiverAI : public ScriptedAI
    {
        npc_brewfest_keg_receiverAI(Creature* c) : ScriptedAI(c) {}
        void MoveInLineOfSight(Unit *who) override
        {
            Player *pPlayer = who->ToPlayer();
            if (!pPlayer)
                return;
            if (pPlayer->HasAura(SPELL_BREWFEST_RAM) 
                && me->GetDistance(pPlayer->GetPositionX(),pPlayer->GetPositionY(),pPlayer->GetPositionZ()) <= 5.0f
                && pPlayer->HasItemCount(ITEM_BREWFEST_KEG,1)) 
            {
                pPlayer->CastSpell(me,SPELL_THROW_KEG,true);
                pPlayer->DestroyItemCount(ITEM_BREWFEST_KEG,1,true);
                pPlayer->GetAura(SPELL_BREWFEST_RAM)->SetDuration(pPlayer->GetAura(SPELL_BREWFEST_RAM)->GetDuration() + 30000);
                if (pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_A) 
                    || pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_H))
                {
                    pPlayer->CastSpell(pPlayer,SPELL_CREATE_TICKETS,true);
                }
                else
                {
                    pPlayer->KilledMonsterCredit(NPC_BREWFEST_DELIVERY_BUNNY, ObjectGuid::Empty);
                    if (pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_A) == QUEST_STATUS_INCOMPLETE) 
                        pPlayer->AreaExploredOrEventHappens(QUEST_THERE_AND_BACK_AGAIN_A);
                    if (pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_H) == QUEST_STATUS_INCOMPLETE) 
                        pPlayer->AreaExploredOrEventHappens(QUEST_THERE_AND_BACK_AGAIN_H);
                    if (pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_A) == QUEST_STATUS_COMPLETE
                        || pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_H) == QUEST_STATUS_COMPLETE)
                        pPlayer->RemoveAura(SPELL_BREWFEST_RAM);
                }
            }
        }
    };

    CreatureAI *GetAI(Creature *creature) const override
    {
        return new npc_brewfest_keg_receiverAI(creature);
    }
};

/*####
## npc_brewfest_ram_master
####*/
#define GOSSIP_ITEM_RAM             "Do you have additional work?"
#define GOSSIP_ITEM_RAM_REINS       "Give me another Ram Racing Reins"
#define SPELL_BREWFEST_SUMMON_RAM   43720

class npc_brewfest_ram_master : public CreatureScript
{
public:
    npc_brewfest_ram_master() : CreatureScript("npc_brewfest_ram_master") { }

    bool OnGossipHello(Player *pPlayer, Creature *pCreature) override
    {
        if (pCreature->isQuestGiver())
            pPlayer->PrepareQuestMenu(pCreature->GetGUID());

            if (pPlayer->HasSpellCooldown(SPELL_BREWFEST_SUMMON_RAM) 
                && !pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_A) 
                && !pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_H)
                && (pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_A) == QUEST_STATUS_INCOMPLETE
                || pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_H) == QUEST_STATUS_INCOMPLETE))
                pPlayer->RemoveSpellCooldown(SPELL_BREWFEST_SUMMON_RAM);

            if (!pPlayer->HasAura(SPELL_BREWFEST_RAM) && ((pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_A) == QUEST_STATUS_INCOMPLETE 
            || pPlayer->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_H) == QUEST_STATUS_INCOMPLETE 
            || (!pPlayer->HasSpellCooldown(SPELL_BREWFEST_SUMMON_RAM) && 
                (pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_A) 
                || pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_H))))))
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_RAM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

            if ((pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_A) 
                || pPlayer->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_H))
                && !pPlayer->HasItemCount(33306,1,true))
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_RAM_REINS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);



        pPlayer->SEND_GOSSIP_MENU(384, pCreature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        pPlayer->PlayerTalkClass->SendCloseGossip();
        if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
        {
            if (pPlayer->HasItemCount(ITEM_BREWFEST_KEG,1)) 
                pPlayer->DestroyItemCount(ITEM_BREWFEST_KEG,1,true);
            pPlayer->CastSpell(pPlayer,SPELL_BREWFEST_SUMMON_RAM,true);
            pPlayer->AddSpellCooldown(SPELL_BREWFEST_SUMMON_RAM,0, getPreciseTime() + 64800.0);
        }
        if (uiAction == GOSSIP_ACTION_INFO_DEF+2)
        {
            pPlayer->CastSpell(pPlayer,44371,false);
        }
        return true;
    }
};

/*####
## npc_winter_reveler
####*/

enum WinterReveler
{
    SPELL_MISTLETOE_DEBUFF       = 26218,
    SPELL_CREATE_MISTLETOE       = 26206,
    SPELL_CREATE_HOLLY           = 26207,
    SPELL_CREATE_SNOWFLAKES      = 45036,
};

class npc_winter_reveler : public CreatureScript
{
    public:
        npc_winter_reveler() : CreatureScript("npc_winter_reveler") { }

        struct npc_winter_revelerAI : public ScriptedAI
        {
            npc_winter_revelerAI(Creature* creature) : ScriptedAI(creature) {}

            void ReceiveEmote(Player* player, uint32 emote) override
            {
                if (player->HasAura(SPELL_MISTLETOE_DEBUFF))
                    return;

                if (!IsHolidayActive(HOLIDAY_FEAST_OF_WINTER_VEIL))
                    return;

                if (emote == TEXT_EMOTE_KISS)
                {
                    uint32 spellId = RAND(SPELL_CREATE_MISTLETOE, SPELL_CREATE_HOLLY, SPELL_CREATE_SNOWFLAKES);
                    me->CastSpell(player, spellId, false);
                    me->CastSpell(player, SPELL_MISTLETOE_DEBUFF, false);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_winter_revelerAI(creature);
        }
};

/*####
## npc_snake_trap_serpents
####*/

#define SPELL_MIND_NUMBING_POISON    25810   //Viper
#define SPELL_DEADLY_POISON          34655   //Venomous Snake
#define SPELL_CRIPPLING_POISON       30981   //Viper

#define VENOMOUS_SNAKE_TIMER 1500
#define VIPER_TIMER 3000

#define C_VIPER 19921

class npc_snake_trap : public CreatureScript
{
    public:
        npc_snake_trap() : CreatureScript("npc_snake_trap_serpents") { }

        struct npc_snake_trap_serpentsAI : public ScriptedAI
        {
            npc_snake_trap_serpentsAI(Creature* creature) : ScriptedAI(creature) {}

            uint32 SpellTimer;
            bool IsViper;

            void EnterCombat(Unit* /*who*/) override {}

            void Reset() override
            {
                SpellTimer = 0;

                CreatureTemplate const* Info = me->GetCreatureTemplate();

                IsViper = Info->Entry == C_VIPER ? true : false;

                me->SetMaxHealth(uint32(107 * (me->getLevel() - 40) * 0.025f));
                //Add delta to make them not all hit the same time
                uint32 delta = (rand() % 7) * 100;
                me->SetStatFloatValue(UNIT_FIELD_ATTACK_ROUND_BASE_TIME, float(Info->baseattacktime + delta));

                // Start attacking attacker of owner on first ai update after spawn - move in line of sight may choose better target
                if (!me->getVictim() && me->isSummon())
                    if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    {
                        if (Owner->getAttackerForHelper())
                            AttackStart(Owner->getAttackerForHelper());
                    }
            }

            //Redefined for random target selection:
            void MoveInLineOfSight(Unit* who) override
            {
                if (!me->getVictim() && me->canCreatureAttack(who))
                {
                    if (me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                        return;

                    float attackRadius = me->GetAttackDistance(who);
                    if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
                    {
                        if (!(rand() % 5))
                        {
                            me->setAttackTimer(BASE_ATTACK, (rand() % 10) * 100);
                            SpellTimer = (rand() % 10) * 100;
                            AttackStart(who);
                        }
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->getVictim()->HasCrowdControlAura(me))
                {
                    me->InterruptNonMeleeSpells(false);
                    return;
                }

                if (SpellTimer <= diff)
                {
                    if (IsViper) //Viper
                    {
                        if (urand(0, 2) == 0) //33% chance to cast
                        {
                            uint32 spell;
                            if (urand(0, 1) == 0)
                                spell = SPELL_MIND_NUMBING_POISON;
                            else
                                spell = SPELL_CRIPPLING_POISON;

                            DoCast(me->getVictim(), spell);
                        }

                        SpellTimer = VIPER_TIMER;
                    }
                    else //Venomous Snake
                    {
                        if (urand(0, 2) == 0) //33% chance to cast
                            DoCast(me->getVictim(), SPELL_DEADLY_POISON);
                        SpellTimer = VENOMOUS_SNAKE_TIMER + (rand() % 5) * 100;
                    }
                }
                else
                    SpellTimer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_snake_trap_serpentsAI(creature);
        }
};

#define SAY_RANDOM_MOJO0    "Now that's what I call froggy-style!"
#define SAY_RANDOM_MOJO1    "Your lily pad or mine?"
#define SAY_RANDOM_MOJO2    "This won't take long, did it?"
#define SAY_RANDOM_MOJO3    "I thought you'd never ask!"
#define SAY_RANDOM_MOJO4    "I promise not to give you warts..."
#define SAY_RANDOM_MOJO5    "Feelin' a little froggy, are ya?"
#define SAY_RANDOM_MOJO6a   "Listen, "
#define SAY_RANDOM_MOJO6b   ", I know of a little swamp not too far from here...."
#define SAY_RANDOM_MOJO7    "There's just never enough Mojo to go around..."

class mob_mojo : public CreatureScript
{
    public:
        mob_mojo() : CreatureScript("mob_mojo") { }

        struct mob_mojoAI : public ScriptedAI
        {
            mob_mojoAI(Creature* creature) : ScriptedAI(creature) {Reset();}
            uint32 hearts;
            ObjectGuid victimGUID;
            void Reset() override
            {
                victimGUID.Clear();
                hearts = 15000;
                if (Unit* own = me->GetOwner())
                    me->GetMotionMaster()->MoveFollow(own, 0, 0);
            }

            void EnterCombat(Unit* /*who*/) override {}

            void UpdateAI(uint32 diff) override
            {
                if (me->HasAura(20372))
                {
                    if (hearts <= diff)
                    {
                        me->RemoveAurasDueToSpell(20372);
                        hearts = 15000;
                    } hearts -= diff;
                }
            }

            void ReceiveEmote(Player* player, uint32 emote) override
            {
                me->HandleEmoteCommand(emote);
                Unit* own = me->GetOwner();
                if (!own || own->GetTypeId() != TYPEID_PLAYER || CAST_PLR(own)->GetTeam() != player->GetTeam())
                    return;
                if (emote == TEXT_EMOTE_KISS)
                {
                    std::string whisp = "";
                    switch (rand() % 8)
                    {
                        case 0:
                            whisp.append(SAY_RANDOM_MOJO0);
                            break;
                        case 1:
                            whisp.append(SAY_RANDOM_MOJO1);
                            break;
                        case 2:
                            whisp.append(SAY_RANDOM_MOJO2);
                            break;
                        case 3:
                            whisp.append(SAY_RANDOM_MOJO3);
                            break;
                        case 4:
                            whisp.append(SAY_RANDOM_MOJO4);
                            break;
                        case 5:
                            whisp.append(SAY_RANDOM_MOJO5);
                            break;
                        case 6:
                            whisp.append(SAY_RANDOM_MOJO6a);
                            whisp.append(player->GetName());
                            whisp.append(SAY_RANDOM_MOJO6b);
                            break;
                        case 7:
                            whisp.append(SAY_RANDOM_MOJO7);
                            break;
                    }

                    me->MonsterWhisper(whisp.c_str(), player->GetGUID());
                    if (victimGUID)
                        if (Player* victim = Unit::GetPlayer(*me, victimGUID))
                            victim->RemoveAura(43906);//remove polymorph frog thing
                    me->AddAura(43906, player);//add polymorph frog thing
                    victimGUID = player->GetGUID();
                    DoCast(me, 20372, true);//tag.hearts
                    me->GetMotionMaster()->MoveFollow(player, 0, 0);
                    hearts = 15000;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_mojoAI(creature);
        }
};

class npc_ebon_gargoyle : public CreatureScript
{
    public:
        npc_ebon_gargoyle() : CreatureScript("npc_ebon_gargoyle") { }

        struct npc_ebon_gargoyleAI : CasterAI
        {
            npc_ebon_gargoyleAI(Creature* creature) : CasterAI(creature) {}

            uint32 despawnTimer;
            bool checkTarget;
            ObjectGuid ownerGuid;
            ObjectGuid mainTargetGUID;
            uint32 targetCheckTime;

            void InitializeAI() override
            {
                checkTarget = false;
                mainTargetGUID.Clear();
                CasterAI::InitializeAI();
                ownerGuid = me->GetOwnerGUID();
                if (!ownerGuid)
                    return;
                // Not needed to be despawned now
                despawnTimer = 0;
            }

            void JustDied(Unit* /*killer*/) override
            {
                // Stop Feeding Gargoyle when it dies
                if (Unit* owner = me->GetOwner())
                    owner->RemoveAurasDueToSpell(50514);
            }

            // Fly away when dismissed
            void SpellHit(Unit* source, SpellInfo const* spell) override
            {
                if (spell->Id != 50515 || !me->isAlive())
                    return;

                Unit* owner = me->GetOwner();
                if (!owner || owner != source)
                    return;

                // Stop Fighting
                me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, true);
                // Sanctuary
                me->CastSpell(me, 54661, true);
                me->SetReactState(REACT_PASSIVE);

                //! HACK: Creature's can't have MOVEMENTFLAG_FLYING
                // Fly Away
                me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY|MOVEMENTFLAG_ASCENDING|MOVEMENTFLAG_FLYING);
                me->SetSpeed(MOVE_FLIGHT, 0.75f, true);
                me->SetSpeed(MOVE_RUN, 0.75f, true);
                float x = me->GetPositionX() + 20 * std::cos(me->GetOrientation());
                float y = me->GetPositionY() + 20 * std::sin(me->GetOrientation());
                float z = me->GetPositionZ() + 40;
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MovePoint(0, x, y, z);

                // Despawn as soon as possible
                despawnTimer = 4 * IN_MILLISECONDS;
            }

            void UpdateAI(uint32 diff) override
            {
                if (despawnTimer > 0)
                {
                    if (despawnTimer > diff)
                        despawnTimer -= diff;
                    else
                        me->DespawnOrUnsummon();
                    return;
                }
                CasterAI::UpdateAI(diff);

                targetCheckTime += diff;
                if (targetCheckTime > 2000)
                {
                    if (mainTargetGUID)
                        if (me->getVictim() && me->getVictim()->GetGUID() != mainTargetGUID)
                            checkTarget = false;

                    targetCheckTime = 0;
                }

                if (!checkTarget)
                {
                    std::list<Unit*> targets;
                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 30);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
                    Trinity::VisitNearbyObject(me, 30, searcher);
                    for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                        if (Unit* unit = (*iter)->ToUnit())
                            if (unit->HasAura(49206, ownerGuid))
                            {
                                if (!mainTargetGUID)
                                    mainTargetGUID = unit->GetGUID();

                                if (me->IsValidAttackTarget(unit))
                                    me->Attack((unit), false);
                                break;
                            }

                    checkTarget = true;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_ebon_gargoyleAI(creature);
        }
};

class npc_mage_mirror_image : public CreatureScript
{
public:
    npc_mage_mirror_image() : CreatureScript("npc_mage_mirror_image") { }

    struct npc_mage_mirror_imageAI : CasterAI
    {
        npc_mage_mirror_imageAI(Creature* creature) : CasterAI(creature) {}

        uint32 targetCheckTime;
        bool firstCheck;

        void InitializeAI() override
        {
            CasterAI::InitializeAI();
            targetCheckTime = 0;
            firstCheck = false;
        }

        void UpdateAI(uint32 diff) override
        {
            targetCheckTime += diff;
            if (targetCheckTime > 2000 || !firstCheck)
            {
                if (Unit* owner = me->GetOwner())
                    if (Unit* victim = owner->getVictim())
                        me->Attack(victim, false);

                firstCheck = true;
                targetCheckTime = 0;
            }

            CasterAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_mage_mirror_imageAI(creature);
    }
};

class npc_lightwell : public CreatureScript
{
    public:
        npc_lightwell() : CreatureScript("npc_lightwell") { }

        struct npc_lightwellAI : public PassiveAI
        {
            npc_lightwellAI(Creature* creature) : PassiveAI(creature)
            {
                if (TempSummon* summon = me->ToTempSummon())
                    if(Unit* owner = summon->GetSummoner())
                        me->SetLevel(owner->getLevel());
                //DoCast(me, 59907, false);
                me->SetAuraStack(59907, me, 17);
            }

            void EnterEvadeMode() override
            {
                if (!me->isAlive())
                    return;

                me->DeleteThreatList();
                me->CombatStop(true);
                me->ResetPlayerDamageReq();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_lightwellAI(creature);
        }
};

class npc_lightwell_mop : public CreatureScript
{
    public:
        npc_lightwell_mop() : CreatureScript("npc_lightwell_mop") { }

        struct npc_lightwell_mopAI : public PassiveAI
        {
            npc_lightwell_mopAI(Creature* creature) : PassiveAI(creature)
            {
                if (TempSummon* summon = me->ToTempSummon())
                    if(Unit* owner = summon->GetSummoner())
                    {
                        me->SetLevel(owner->getLevel());
                        me->SetMaxHealth(CalculatePct(owner->GetMaxHealth(), 50));
                        me->SetFullHealth();
                        me->SetMaxPower(POWER_RAGE, 0);
                        me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 10);
                    }

                DoCast(me, 126138, true);
                me->SetAuraStack(126150, me, 17);
            }

            void EnterEvadeMode() override
            {
                if (!me->isAlive())
                    return;

                me->DeleteThreatList();
                me->CombatStop(true);
                me->ResetPlayerDamageReq();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_lightwell_mopAI(creature);
        }
};

enum eTrainingDummy
{
    NPC_ADVANCED_TARGET_DUMMY                  = 2674,
    NPC_TARGET_DUMMY                           = 2673
};

enum sEvents
{
    EVENT_CHECK_PLAYERS                        = 1,
};

class npc_training_dummy : public CreatureScript
{
public:
    npc_training_dummy() : CreatureScript("npc_training_dummy") { }

    struct npc_training_dummyAI : Scripted_NoMovementAI
    {
        npc_training_dummyAI(Creature* creature) : Scripted_NoMovementAI(creature){}

        void Reset() override
        {
            if (!me->isTrainingDummy())
                me->AddUnitTypeMask(UNIT_MASK_TRAINING_DUMMY);

            if (!me->HasAura(113368))
                me->AddAura(113368, me);

            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);//imune to knock aways like blast wave
            me->SetReactState(REACT_PASSIVE);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
        {
            damage = 0;
        }

        void SpellHit(Unit* source, SpellInfo const* spell) override
        {
            if(source)
            {
                Player* player = source->ToPlayer();
                if(!player)
                    return;
                    
                switch (spell->Id)
                {
                    case 100:
                    case 172:
                    case 589:
                    case 8921:
                    case 20271:
                    case 73899:
                    case 100787:
                    case 118215:
                    {
                        player->KilledMonsterCredit(44175, ObjectGuid::Empty);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            /* std::list<HostileReference*> threatlist = me->getThreatManager().getThreatList();
            if (!threatlist.empty())
            {
                for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); itr++)
                {
                    if (!(*itr))
                        continue;

                    if (Player* pl = me->GetPlayer(*me, (*itr)->getUnitGuid()))
                    {
                        if (!pl->GetCombatTimer())
                        {
                            pl->CombatStop(false);
                            (*itr)->removeReference();
                        }
                    }
                    else
                    {
                        (*itr)->removeReference();
                    }
                }
            } */
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_training_dummyAI(creature);
    }
};

struct npc_anatomical_dummy : ScriptedAI
{
    npc_anatomical_dummy(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        if (!me->isTrainingDummy())
            me->AddUnitTypeMask(UNIT_MASK_TRAINING_DUMMY);

        if (!me->HasAura(113368))
            me->AddAura(113368, me);

        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->SetReactState(REACT_PASSIVE);
    }

    void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType /*dmgType*/) override
    {
        if (rand_chance() < 10.0f)
            Talk(0, attacker->GetGUID());

        damage = 0;
    }
};

/*######
# npc_wormhole
######*/

#define GOSSIP_ENGINEERING1   "Borean Tundra"
#define GOSSIP_ENGINEERING2   "Howling Fjord"
#define GOSSIP_ENGINEERING3   "Sholazar Basin"
#define GOSSIP_ENGINEERING4   "Icecrown"
#define GOSSIP_ENGINEERING5   "Storm Peaks"
#define GOSSIP_ENGINEERING6   "Underground..."

enum WormholeSpells
{
    SPELL_BOREAN_TUNDRA         = 67834,
    SPELL_SHOLAZAR_BASIN        = 67835,
    SPELL_ICECROWN              = 67836,
    SPELL_STORM_PEAKS           = 67837,
    SPELL_HOWLING_FJORD         = 67838,
    SPELL_UNDERGROUND           = 68081,

    TEXT_WORMHOLE               = 907,

    DATA_SHOW_UNDERGROUND       = 1,
};

class npc_wormhole : public CreatureScript
{
    public:
        npc_wormhole() : CreatureScript("npc_wormhole") {}

        struct npc_wormholeAI : public PassiveAI
        {
            npc_wormholeAI(Creature* creature) : PassiveAI(creature) {}

            void InitializeAI() override
            {
                _showUnderground = urand(0, 100) == 0; // Guessed value, it is really rare though
            }

            uint32 GetData(uint32 type) const override
            {
                return (type == DATA_SHOW_UNDERGROUND && _showUnderground) ? 1 : 0;
            }

        private:
            bool _showUnderground;
        };

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            if (creature->isSummon())
            {
                if (player == creature->ToTempSummon()->GetSummoner())
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

                    if (creature->AI()->GetData(DATA_SHOW_UNDERGROUND))
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);

                    player->PlayerTalkClass->SendGossipMenu(TEXT_WORMHOLE, creature->GetGUID());
                }
            }

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();

            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1: // Borean Tundra
                    player->CLOSE_GOSSIP_MENU();
                    creature->CastSpell(player, SPELL_BOREAN_TUNDRA, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2: // Howling Fjord
                    player->CLOSE_GOSSIP_MENU();
                    creature->CastSpell(player, SPELL_HOWLING_FJORD, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 3: // Sholazar Basin
                    player->CLOSE_GOSSIP_MENU();
                    creature->CastSpell(player, SPELL_SHOLAZAR_BASIN, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 4: // Icecrown
                    player->CLOSE_GOSSIP_MENU();
                    creature->CastSpell(player, SPELL_ICECROWN, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 5: // Storm peaks
                    player->CLOSE_GOSSIP_MENU();
                    creature->CastSpell(player, SPELL_STORM_PEAKS, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 6: // Underground
                    player->CLOSE_GOSSIP_MENU();
                    creature->CastSpell(player, SPELL_UNDERGROUND, false);
                    break;
            }

            return true;
        }

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_wormholeAI(creature);
        }
};

/*######
## npc_pet_trainer
######*/

enum ePetTrainer
{
    TEXT_ISHUNTER               = 5838,
    TEXT_NOTHUNTER              = 5839,
    TEXT_PETINFO                = 13474,
    TEXT_CONFIRM                = 7722
};

#define GOSSIP_PET1             "How do I train my pet?"
#define GOSSIP_PET2             "I wish to untrain my pet."
#define GOSSIP_PET_CONFIRM      "Yes, please do."

class npc_pet_trainer : public CreatureScript
{
    public:
        npc_pet_trainer() : CreatureScript("npc_pet_trainer") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            if (creature->isQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if (player->getClass() == CLASS_HUNTER)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                if (player->GetPet() && player->GetPet()->getPetType() == HUNTER_PET)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

                player->PlayerTalkClass->SendGossipMenu(TEXT_ISHUNTER, creature->GetGUID());
                return true;
            }
            player->PlayerTalkClass->SendGossipMenu(TEXT_NOTHUNTER, creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1:
                    player->PlayerTalkClass->SendGossipMenu(TEXT_PETINFO, creature->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2:
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET_CONFIRM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                        player->PlayerTalkClass->SendGossipMenu(TEXT_CONFIRM, creature->GetGUID());
                    }
                    break;
                case GOSSIP_ACTION_INFO_DEF + 3:
                    {
                        player->CLOSE_GOSSIP_MENU();
                    }
                    break;
            }
            return true;
        }
};

/*######
## npc_locksmith
######*/

enum eLockSmith
{
    QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ = 10704,
    QUEST_DARK_IRON_LEGACY                = 3802,
    QUEST_THE_KEY_TO_SCHOLOMANCE_A        = 5505,
    QUEST_THE_KEY_TO_SCHOLOMANCE_H        = 5511,
    QUEST_HOTTER_THAN_HELL_A              = 10758,
    QUEST_HOTTER_THAN_HELL_H              = 10764,
    QUEST_RETURN_TO_KHAGDAR               = 9837,
    QUEST_CONTAINMENT                     = 13159,
    QUEST_ETERNAL_VIGILANCE               = 11011,
    QUEST_KEY_TO_THE_FOCUSING_IRIS        = 13372,
    QUEST_HC_KEY_TO_THE_FOCUSING_IRIS     = 13375,

    ITEM_ARCATRAZ_KEY                     = 31084,
    ITEM_SHADOWFORGE_KEY                  = 11000,
    ITEM_SKELETON_KEY                     = 13704,
    ITEM_SHATTERED_HALLS_KEY              = 28395,
    ITEM_THE_MASTERS_KEY                  = 24490,
    ITEM_VIOLET_HOLD_KEY                  = 42482,
    ITEM_ESSENCE_INFUSED_MOONSTONE        = 32449,
    ITEM_KEY_TO_THE_FOCUSING_IRIS         = 44582,
    ITEM_HC_KEY_TO_THE_FOCUSING_IRIS      = 44581,

    SPELL_ARCATRAZ_KEY                    = 54881,
    SPELL_SHADOWFORGE_KEY                 = 54882,
    SPELL_SKELETON_KEY                    = 54883,
    SPELL_SHATTERED_HALLS_KEY             = 54884,
    SPELL_THE_MASTERS_KEY                 = 54885,
    SPELL_VIOLET_HOLD_KEY                 = 67253,
    SPELL_ESSENCE_INFUSED_MOONSTONE       = 40173,
};

#define GOSSIP_LOST_ARCATRAZ_KEY                "I've lost my key to the Arcatraz."
#define GOSSIP_LOST_SHADOWFORGE_KEY             "I've lost my key to the Blackrock Depths."
#define GOSSIP_LOST_SKELETON_KEY                "I've lost my key to the Scholomance."
#define GOSSIP_LOST_SHATTERED_HALLS_KEY         "I've lost my key to the Shattered Halls."
#define GOSSIP_LOST_THE_MASTERS_KEY             "I've lost my key to the Karazhan."
#define GOSSIP_LOST_VIOLET_HOLD_KEY             "I've lost my key to the Violet Hold."
#define GOSSIP_LOST_ESSENCE_INFUSED_MOONSTONE   "I've lost my Essence-Infused Moonstone."
#define GOSSIP_LOST_KEY_TO_THE_FOCUSING_IRIS    "I've lost my Key to the Focusing Iris."
#define GOSSIP_LOST_HC_KEY_TO_THE_FOCUSING_IRIS "I've lost my Heroic Key to the Focusing Iris."

class npc_locksmith : public CreatureScript
{
    public:
        npc_locksmith() : CreatureScript("npc_locksmith") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            // Arcatraz Key
            if (player->GetQuestRewardStatus(QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ) && !player->HasItemCount(ITEM_ARCATRAZ_KEY, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_ARCATRAZ_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            // Shadowforge Key
            if (player->GetQuestRewardStatus(QUEST_DARK_IRON_LEGACY) && !player->HasItemCount(ITEM_SHADOWFORGE_KEY, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHADOWFORGE_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            // Skeleton Key
            if ((player->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_A) || player->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_H)) &&
                !player->HasItemCount(ITEM_SKELETON_KEY, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SKELETON_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

            // Shatered Halls Key
            if ((player->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_A) || player->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_H)) &&
                !player->HasItemCount(ITEM_SHATTERED_HALLS_KEY, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHATTERED_HALLS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);

            // Master's Key
            if (player->GetQuestRewardStatus(QUEST_RETURN_TO_KHAGDAR) && !player->HasItemCount(ITEM_THE_MASTERS_KEY, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_THE_MASTERS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

            // Violet Hold Key
            if (player->GetQuestRewardStatus(QUEST_CONTAINMENT) && !player->HasItemCount(ITEM_VIOLET_HOLD_KEY, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_VIOLET_HOLD_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);

            // Essence-Infused Moonstone
            if (player->GetQuestRewardStatus(QUEST_ETERNAL_VIGILANCE) && !player->HasItemCount(ITEM_ESSENCE_INFUSED_MOONSTONE, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_ESSENCE_INFUSED_MOONSTONE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);

            // Key to the Focusing Iris
            if (player->GetQuestRewardStatus(QUEST_KEY_TO_THE_FOCUSING_IRIS) && !player->HasItemCount(ITEM_KEY_TO_THE_FOCUSING_IRIS, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_KEY_TO_THE_FOCUSING_IRIS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);

            // Heroic Key to the Focusing Iris
            if (player->GetQuestRewardStatus(QUEST_HC_KEY_TO_THE_FOCUSING_IRIS) && !player->HasItemCount(ITEM_HC_KEY_TO_THE_FOCUSING_IRIS, 1, true))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_HC_KEY_TO_THE_FOCUSING_IRIS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_ARCATRAZ_KEY, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_SHADOWFORGE_KEY, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 3:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_SKELETON_KEY, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 4:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_SHATTERED_HALLS_KEY, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 5:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_THE_MASTERS_KEY, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 6:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_VIOLET_HOLD_KEY, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 7:
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_ESSENCE_INFUSED_MOONSTONE, false);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 8:
                    player->CLOSE_GOSSIP_MENU();
                    player->AddItem(ITEM_KEY_TO_THE_FOCUSING_IRIS,1);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 9:
                    player->CLOSE_GOSSIP_MENU();
                    player->AddItem(ITEM_HC_KEY_TO_THE_FOCUSING_IRIS,1);
                    break;
            }
            return true;
        }
};

/*######
## npc_experience
######*/

#define EXP_COST                100000 //10 00 00 copper (10golds)
#define GOSSIP_TEXT_EXP         14736
#define GOSSIP_XP_OFF           "I no longer wish to gain experience."
#define GOSSIP_XP_ON            "I wish to start gaining experience again."

class npc_experience : public CreatureScript
{
    public:
        npc_experience() : CreatureScript("npc_experience") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_ON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_EXP, creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            bool noXPGain = player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            bool doSwitch = false;

            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1://xp off
                    {
                        if (!noXPGain)//does gain xp
                            doSwitch = true;//switch to don't gain xp
                    }
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2://xp on
                    {
                        if (noXPGain)//doesn't gain xp
                            doSwitch = true;//switch to gain xp
                    }
                    break;
            }
            if (doSwitch)
            {
                if (!player->HasEnoughMoney(uint64(EXP_COST)))
                    player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY);
                else if (noXPGain)
                {
                    player->ModifyMoney(-int64(EXP_COST));
                    player->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
                }
                else if (!noXPGain)
                {
                    player->ModifyMoney(-EXP_COST);
                    player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
                }
            }
            player->PlayerTalkClass->SendCloseGossip();
            return true;
        }
};

enum Fireworks
{
    NPC_OMEN                = 15467,
    NPC_MINION_OF_OMEN      = 15466,
    NPC_FIREWORK_BLUE       = 15879,
    NPC_FIREWORK_GREEN      = 15880,
    NPC_FIREWORK_PURPLE     = 15881,
    NPC_FIREWORK_RED        = 15882,
    NPC_FIREWORK_YELLOW     = 15883,
    NPC_FIREWORK_WHITE      = 15884,
    NPC_FIREWORK_BIG_BLUE   = 15885,
    NPC_FIREWORK_BIG_GREEN  = 15886,
    NPC_FIREWORK_BIG_PURPLE = 15887,
    NPC_FIREWORK_BIG_RED    = 15888,
    NPC_FIREWORK_BIG_YELLOW = 15889,
    NPC_FIREWORK_BIG_WHITE  = 15890,

    NPC_CLUSTER_BLUE        = 15872,
    NPC_CLUSTER_RED         = 15873,
    NPC_CLUSTER_GREEN       = 15874,
    NPC_CLUSTER_PURPLE      = 15875,
    NPC_CLUSTER_WHITE       = 15876,
    NPC_CLUSTER_YELLOW      = 15877,
    NPC_CLUSTER_BIG_BLUE    = 15911,
    NPC_CLUSTER_BIG_GREEN   = 15912,
    NPC_CLUSTER_BIG_PURPLE  = 15913,
    NPC_CLUSTER_BIG_RED     = 15914,
    NPC_CLUSTER_BIG_WHITE   = 15915,
    NPC_CLUSTER_BIG_YELLOW  = 15916,
    NPC_CLUSTER_ELUNE       = 15918,

    GO_FIREWORK_LAUNCHER_1  = 180771,
    GO_FIREWORK_LAUNCHER_2  = 180868,
    GO_FIREWORK_LAUNCHER_3  = 180850,
    GO_CLUSTER_LAUNCHER_1   = 180772,
    GO_CLUSTER_LAUNCHER_2   = 180859,
    GO_CLUSTER_LAUNCHER_3   = 180869,
    GO_CLUSTER_LAUNCHER_4   = 180874,

    SPELL_ROCKET_BLUE       = 26344,
    SPELL_ROCKET_GREEN      = 26345,
    SPELL_ROCKET_PURPLE     = 26346,
    SPELL_ROCKET_RED        = 26347,
    SPELL_ROCKET_WHITE      = 26348,
    SPELL_ROCKET_YELLOW     = 26349,
    SPELL_ROCKET_BIG_BLUE   = 26351,
    SPELL_ROCKET_BIG_GREEN  = 26352,
    SPELL_ROCKET_BIG_PURPLE = 26353,
    SPELL_ROCKET_BIG_RED    = 26354,
    SPELL_ROCKET_BIG_WHITE  = 26355,
    SPELL_ROCKET_BIG_YELLOW = 26356,
    SPELL_LUNAR_FORTUNE     = 26522,

    ANIM_GO_LAUNCH_FIREWORK = 3,
    ZONE_MOONGLADE          = 493,
};

Position omenSummonPos = {7558.993f, -2839.999f, 450.0214f, 4.46f};

class npc_firework : public CreatureScript
{
    public:
        npc_firework() : CreatureScript("npc_firework") { }

        struct npc_fireworkAI : public ScriptedAI
        {
            npc_fireworkAI(Creature* creature) : ScriptedAI(creature) {}

            bool isCluster()
            {
                switch (me->GetEntry())
                {
                    case NPC_FIREWORK_BLUE:
                    case NPC_FIREWORK_GREEN:
                    case NPC_FIREWORK_PURPLE:
                    case NPC_FIREWORK_RED:
                    case NPC_FIREWORK_YELLOW:
                    case NPC_FIREWORK_WHITE:
                    case NPC_FIREWORK_BIG_BLUE:
                    case NPC_FIREWORK_BIG_GREEN:
                    case NPC_FIREWORK_BIG_PURPLE:
                    case NPC_FIREWORK_BIG_RED:
                    case NPC_FIREWORK_BIG_YELLOW:
                    case NPC_FIREWORK_BIG_WHITE:
                        return false;
                    case NPC_CLUSTER_BLUE:
                    case NPC_CLUSTER_GREEN:
                    case NPC_CLUSTER_PURPLE:
                    case NPC_CLUSTER_RED:
                    case NPC_CLUSTER_YELLOW:
                    case NPC_CLUSTER_WHITE:
                    case NPC_CLUSTER_BIG_BLUE:
                    case NPC_CLUSTER_BIG_GREEN:
                    case NPC_CLUSTER_BIG_PURPLE:
                    case NPC_CLUSTER_BIG_RED:
                    case NPC_CLUSTER_BIG_YELLOW:
                    case NPC_CLUSTER_BIG_WHITE:
                    case NPC_CLUSTER_ELUNE:
                    default:
                        return true;
                }
            }

            GameObject* FindNearestLauncher()
            {
                GameObject* launcher = NULL;

                if (isCluster())
                {
                    GameObject* launcher1 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_1, 0.5f);
                    GameObject* launcher2 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_2, 0.5f);
                    GameObject* launcher3 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_3, 0.5f);
                    GameObject* launcher4 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_4, 0.5f);

                    if (launcher1)
                        launcher = launcher1;
                    else if (launcher2)
                        launcher = launcher2;
                    else if (launcher3)
                        launcher = launcher3;
                    else if (launcher4)
                        launcher = launcher4;
                }
                else
                {
                    GameObject* launcher1 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_1, 0.5f);
                    GameObject* launcher2 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_2, 0.5f);
                    GameObject* launcher3 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_3, 0.5f);

                    if (launcher1)
                        launcher = launcher1;
                    else if (launcher2)
                        launcher = launcher2;
                    else if (launcher3)
                        launcher = launcher3;
                }

                return launcher;
            }

            uint32 GetFireworkSpell(uint32 entry)
            {
                switch (entry)
                {
                    case NPC_FIREWORK_BLUE:
                        return SPELL_ROCKET_BLUE;
                    case NPC_FIREWORK_GREEN:
                        return SPELL_ROCKET_GREEN;
                    case NPC_FIREWORK_PURPLE:
                        return SPELL_ROCKET_PURPLE;
                    case NPC_FIREWORK_RED:
                        return SPELL_ROCKET_RED;
                    case NPC_FIREWORK_YELLOW:
                        return SPELL_ROCKET_YELLOW;
                    case NPC_FIREWORK_WHITE:
                        return SPELL_ROCKET_WHITE;
                    case NPC_FIREWORK_BIG_BLUE:
                        return SPELL_ROCKET_BIG_BLUE;
                    case NPC_FIREWORK_BIG_GREEN:
                        return SPELL_ROCKET_BIG_GREEN;
                    case NPC_FIREWORK_BIG_PURPLE:
                        return SPELL_ROCKET_BIG_PURPLE;
                    case NPC_FIREWORK_BIG_RED:
                        return SPELL_ROCKET_BIG_RED;
                    case NPC_FIREWORK_BIG_YELLOW:
                        return SPELL_ROCKET_BIG_YELLOW;
                    case NPC_FIREWORK_BIG_WHITE:
                        return SPELL_ROCKET_BIG_WHITE;
                    default:
                        return 0;
                }
            }

            uint32 GetFireworkGameObjectId()
            {
                uint32 spellId = 0;

                switch (me->GetEntry())
                {
                    case NPC_CLUSTER_BLUE:
                        spellId = GetFireworkSpell(NPC_FIREWORK_BLUE);
                        break;
                    case NPC_CLUSTER_GREEN:
                        spellId = GetFireworkSpell(NPC_FIREWORK_GREEN);
                        break;
                    case NPC_CLUSTER_PURPLE:
                        spellId = GetFireworkSpell(NPC_FIREWORK_PURPLE);
                        break;
                    case NPC_CLUSTER_RED:
                        spellId = GetFireworkSpell(NPC_FIREWORK_RED);
                        break;
                    case NPC_CLUSTER_YELLOW:
                        spellId = GetFireworkSpell(NPC_FIREWORK_YELLOW);
                        break;
                    case NPC_CLUSTER_WHITE:
                        spellId = GetFireworkSpell(NPC_FIREWORK_WHITE);
                        break;
                    case NPC_CLUSTER_BIG_BLUE:
                        spellId = GetFireworkSpell(NPC_FIREWORK_BIG_BLUE);
                        break;
                    case NPC_CLUSTER_BIG_GREEN:
                        spellId = GetFireworkSpell(NPC_FIREWORK_BIG_GREEN);
                        break;
                    case NPC_CLUSTER_BIG_PURPLE:
                        spellId = GetFireworkSpell(NPC_FIREWORK_BIG_PURPLE);
                        break;
                    case NPC_CLUSTER_BIG_RED:
                        spellId = GetFireworkSpell(NPC_FIREWORK_BIG_RED);
                        break;
                    case NPC_CLUSTER_BIG_YELLOW:
                        spellId = GetFireworkSpell(NPC_FIREWORK_BIG_YELLOW);
                        break;
                    case NPC_CLUSTER_BIG_WHITE:
                        spellId = GetFireworkSpell(NPC_FIREWORK_BIG_WHITE);
                        break;
                    case NPC_CLUSTER_ELUNE:
                        spellId = GetFireworkSpell(urand(NPC_FIREWORK_BLUE, NPC_FIREWORK_WHITE));
                        break;
                }

                const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);

                if (spellInfo && spellInfo->Effects[0]->Effect == SPELL_EFFECT_SUMMON_OBJECT_WILD)
                    return spellInfo->Effects[0]->MiscValue;

                return 0;
            }

            void Reset() override
            {
                if (GameObject* launcher = FindNearestLauncher())
                {
                    launcher->SendCustomAnim(ANIM_GO_LAUNCH_FIREWORK);
                    me->SetOrientation(launcher->GetOrientation() + M_PI/2);
                }
                else
                    return;

                if (isCluster())
                {
                    // Check if we are near Elune'ara lake south, if so try to summon Omen or a minion
                    if (me->GetCurrentZoneID() == ZONE_MOONGLADE)
                    {
                        if (!me->FindNearestCreature(NPC_OMEN, 100.0f, false) && me->GetDistance2d(omenSummonPos.GetPositionX(), omenSummonPos.GetPositionY()) <= 100.0f)
                        {
                            switch (urand(0,9))
                            {
                                case 0:
                                case 1:
                                case 2:
                                case 3:
                                    if (Creature* minion = me->SummonCreature(NPC_MINION_OF_OMEN, me->GetPositionX()+frand(-5.0f, 5.0f), me->GetPositionY()+frand(-5.0f, 5.0f), me->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                                        minion->AI()->AttackStart(me->SelectNearestPlayer(20.0f));
                                    break;
                                case 9:
                                    me->SummonCreature(NPC_OMEN, omenSummonPos);
                                    break;
                            }
                        }
                    }
                    if (me->GetEntry() == NPC_CLUSTER_ELUNE)
                        DoCast(SPELL_LUNAR_FORTUNE);

                    float displacement = 0.7f;
                    for (uint8 i = 0; i < 4; i++)
                        me->SummonGameObject(GetFireworkGameObjectId(), me->GetPositionX() + (i%2 == 0 ? displacement : -displacement), me->GetPositionY() + (i > 1 ? displacement : -displacement), me->GetPositionZ() + 4.0f, me->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 1);
                }
                else
                    //me->CastSpell(me, GetFireworkSpell(me->GetEntry()), true);
                    me->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), GetFireworkSpell(me->GetEntry()), true);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_fireworkAI(creature);
        }
};

/*#####
# npc_spring_rabbit
#####*/

enum rabbitSpells
{
    SPELL_SPRING_FLING          = 61875,
    SPELL_SPRING_RABBIT_JUMP    = 61724,
    SPELL_SPRING_RABBIT_WANDER  = 61726,
    SPELL_SUMMON_BABY_BUNNY     = 61727,
    SPELL_SPRING_RABBIT_IN_LOVE = 61728,
    NPC_SPRING_RABBIT           = 32791
};

class npc_spring_rabbit : public CreatureScript
{
    public:
        npc_spring_rabbit() : CreatureScript("npc_spring_rabbit") { }

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_spring_rabbitAI(creature);
        }

        struct npc_spring_rabbitAI : public ScriptedAI
        {
            npc_spring_rabbitAI(Creature* creature) : ScriptedAI(creature) { }

            bool inLove;
            uint32 jumpTimer;
            uint32 bunnyTimer;
            uint32 searchTimer;
            ObjectGuid rabbitGUID;

            void Reset() override
            {
                inLove = false;
                rabbitGUID.Clear();
                jumpTimer = urand(5000, 10000);
                bunnyTimer = urand(10000, 20000);
                searchTimer = urand(5000, 10000);
                if (Unit* owner = me->GetOwner())
                    me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
            }

            void EnterCombat(Unit * /*who*/) override { }

            void DoAction(const int32 /*param*/) override
            {
                inLove = true;
                if (Unit* owner = me->GetOwner())
                    owner->CastSpell(owner, SPELL_SPRING_FLING, true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (inLove)
                {
                    if (jumpTimer <= diff)
                    {
                        if (Unit* rabbit = Unit::GetUnit(*me, rabbitGUID))
                            DoCast(rabbit, SPELL_SPRING_RABBIT_JUMP);
                        jumpTimer = urand(5000, 10000);
                    } else jumpTimer -= diff;

                    if (bunnyTimer <= diff)
                    {
                        DoCast(SPELL_SUMMON_BABY_BUNNY);
                        bunnyTimer = urand(20000, 40000);
                    } else bunnyTimer -= diff;
                }
                else
                {
                    if (searchTimer <= diff)
                    {
                        if (Creature* rabbit = me->FindNearestCreature(NPC_SPRING_RABBIT, 10.0f))
                        {
                            if (rabbit == me || rabbit->HasAura(SPELL_SPRING_RABBIT_IN_LOVE))
                                return;

                            me->AddAura(SPELL_SPRING_RABBIT_IN_LOVE, me);
                            DoAction(1);
                            rabbit->AddAura(SPELL_SPRING_RABBIT_IN_LOVE, rabbit);
                            rabbit->AI()->DoAction(1);
                            rabbit->CastSpell(rabbit, SPELL_SPRING_RABBIT_JUMP, true);
                            rabbitGUID = rabbit->GetGUID();
                        }
                        searchTimer = urand(5000, 10000);
                    } else searchTimer -= diff;
                }
            }
        };
};

/*######
## npc_generic_harpoon_cannon
######*/

class npc_generic_harpoon_cannon : public CreatureScript
{
    public:
        npc_generic_harpoon_cannon() : CreatureScript("npc_generic_harpoon_cannon") { }

        struct npc_generic_harpoon_cannonAI : public ScriptedAI
        {
            npc_generic_harpoon_cannonAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset() override
            {
                me->SetUnitMovementFlags(MOVEMENTFLAG_ROOT);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_generic_harpoon_cannonAI(creature);
        }
};

/*######
## npc_spirit_link_totem
######*/

class npc_spirit_link_totem : public CreatureScript
{
    public:
        npc_spirit_link_totem() : CreatureScript("npc_spirit_link_totem") { }

    struct npc_spirit_link_totemAI : public ScriptedAI
    {
        uint32 CastTimer;

        npc_spirit_link_totemAI(Creature* creature) : ScriptedAI(creature)
        {
            CastTimer = 1000;

            if (creature->GetOwner() && creature->GetOwner()->GetTypeId() == TYPEID_PLAYER)
            {
                if (creature->GetEntry() == 53006)
                {
                    creature->CastSpell(creature, 98007, false);
                    creature->CastSpell(creature, 98017, true);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (CastTimer >= diff)
            {
                if (me->GetOwner() && me->GetOwner()->GetTypeId() == TYPEID_PLAYER)
                {
                    if (me->GetEntry() == 53006)
                    {
                        me->CastSpell(me, 98007, false);
                        me->CastSpell(me, 98017, true);
                    }
                }
            }

            CastTimer = 0;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_spirit_link_totemAI(creature);
    }
};

/*######
# npc_frozen_orb
######*/

class npc_frozen_orb : public CreatureScript
{
    public:
        npc_frozen_orb() : CreatureScript("npc_frozen_orb") { }

        struct npc_frozen_orbAI : public ScriptedAI
        {
            uint32 frozenOrbTimer;
            float x,y,z;

            npc_frozen_orbAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                if (Unit* owner = creature->GetAnyOwner())
                {
                    Position pos;
                    owner->GetFirstCollisionPosition(pos, 40.0f, 0.0f);
                    x = pos.GetPositionX();
                    y = pos.GetPositionY();
                    z = pos.GetPositionZ();
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MovePoint(0, x, y, z, false, me->GetSpeed(MOVE_RUN));
                }

                frozenOrbTimer = 1000;
            }

            void Reset() override
            {
                if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                {
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MovePoint(0, x, y, z, false, me->GetSpeed(MOVE_RUN));
                }
            }

            void EnterEvadeMode() override
            {
                if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                {
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MovePoint(0, x, y, z, false, me->GetSpeed(MOVE_RUN));
                }
            }

            void UpdateAI(uint32 diff) override
            {
                Unit* owner = me->GetAnyOwner();
                if (!owner)
                    return;

                if (frozenOrbTimer <= diff)
                {
                    if (owner && owner->ToPlayer())
                        if (owner->ToPlayer()->HasSpellCooldown(84721))
                            owner->ToPlayer()->RemoveSpellCooldown(84721);

                    owner->CastSpell(me, 84721, true);

                    frozenOrbTimer = 1000;
                }
                else
                    frozenOrbTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_frozen_orbAI(creature);
        }
};

/*######
# npc_guardian_of_ancient_kings
######*/

enum GuardianSpellsAndEntries
{
    NPC_PROTECTION_GUARDIAN         = 46490,
    NPC_HOLY_GUARDIAN               = 46499,
    NPC_RETRI_GUARDIAN              = 46506,
    SPELL_ANCIENT_GUARDIAN_VISUAL   = 86657,
    SPELL_ANCIENT_HEALER            = 86674,
};

class npc_guardian_of_ancient_kings : public CreatureScript
{
    public:
        npc_guardian_of_ancient_kings() : CreatureScript("npc_guardian_of_ancient_kings") { }

        struct npc_guardian_of_ancient_kingsAI : public ScriptedAI
        {
            npc_guardian_of_ancient_kingsAI(Creature *creature) : ScriptedAI(creature) {}

            void Reset() override
            {
                if (me->GetEntry() == NPC_RETRI_GUARDIAN || me->GetEntry() == NPC_HOLY_GUARDIAN)
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
                else if (me->GetEntry() == NPC_PROTECTION_GUARDIAN)
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);

                if (me->GetEntry() == NPC_RETRI_GUARDIAN)
                    me->SetReactState(REACT_DEFENSIVE);
                else
                    me->SetReactState(REACT_PASSIVE);

                if (me->GetEntry() == NPC_PROTECTION_GUARDIAN)
                {
                    if (me->GetOwner())
                        DoCast(me->GetOwner(), SPELL_ANCIENT_GUARDIAN_VISUAL);
                }
                else if (me->GetEntry() == NPC_RETRI_GUARDIAN)
                {
                    if (me->GetOwner())
                    {
                        float mindmg = me->GetOwner()->GetFloatValue(UNIT_FIELD_MIN_DAMAGE);
                        float maxdmg = me->GetOwner()->GetFloatValue(UNIT_FIELD_MAX_DAMAGE);
                        me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, mindmg);
                        me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, maxdmg);

                        if (me->GetOwner()->getVictim())
                            AttackStart(me->GetOwner()->getVictim());

                        DoCast(me, 86703, true);
                    }
                }
                else if (me->GetEntry() == NPC_HOLY_GUARDIAN)
                    if (me->GetOwner())
                        me->GetOwner()->CastSpell(me->GetOwner(), SPELL_ANCIENT_HEALER, true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->getVictim() && me->getVictim()->HasCrowdControlAura(me))
                {
                    me->InterruptNonMeleeSpells(false);
                    return;
                }

                if (Unit* owner = me->GetOwner())
                {
                    if (Unit* newVictim = owner->getAttackerForHelper())
                        if (me->getVictim() != newVictim)
                            AttackStart(newVictim);
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_guardian_of_ancient_kingsAI(creature);
        }
};

/*######
# new npc_demonic_gateway_green
######*/

enum gateway_data
{
    //Add animation to AreaTrigger
    SPELL_DEMONIC_PORTAL_BIRTH_ANIM_DUMMY   = 143251,
    //Summon arrea trigger.
    SPELL_DEMONIC_GATEWAY                   = 113904,

    NPC_PURGE_GATE                          = 59271,
    NPC_GREEN_GATE                          = 59262,
};

uint32 DestEntry[2]= { NPC_PURGE_GATE, NPC_GREEN_GATE };

class npc_demonic_gateway : public CreatureScript
{
    public:
        npc_demonic_gateway() : CreatureScript("npc_demonic_gateway") { }

        struct npc_demonic_gatewayAI : public Scripted_NoMovementAI
        {
            bool gate;
            npc_demonic_gatewayAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void IsSummonedBy(Unit* summoner) override
            {
                gate = me->GetEntry() == NPC_PURGE_GATE;
                me->CastSpell(me, SPELL_DEMONIC_GATEWAY, true);
                me->CastSpell(me, SPELL_DEMONIC_PORTAL_BIRTH_ANIM_DUMMY, false);
            }

            void OnSpellClick(Unit* who) override
            {
                if (!who || !me->ToTempSummon() || who->HasAura(113942))
                    return;
                if (Unit* owner = me->ToTempSummon()->GetSummoner())
                {
                    Player* ownerPlayer = owner->ToPlayer();
                    Player* whoPlayer = who->ToPlayer();
                    if(!ownerPlayer || !whoPlayer || !ownerPlayer->IsGroupVisibleFor(whoPlayer))
                        return;

                    for (int32 i = 0; i < MAX_SUMMON_SLOT; ++i)
                    {
                        if (owner->m_SummonSlot[i].GetEntry() != DestEntry[gate])
                            continue;
                        if(Unit* uGate = ObjectAccessor::GetUnit(*me, owner->m_SummonSlot[i]))
                        {
                            //who->CastSpell(uGate, gate ? 120729 : 113896, true);
                            who->CastCustomSpell(uGate, gate ? 120729 : 113896, NULL, NULL, NULL, false, NULL, NULL, owner->GetGUID());
                            break;
                        }
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_demonic_gatewayAI(creature);
        }
};

/*######
# npc_dire_beast
######*/

class npc_dire_beast : public CreatureScript
{
    public:
        npc_dire_beast() : CreatureScript("npc_dire_beast") { }

        struct npc_dire_beastAI : public ScriptedAI
        {
            npc_dire_beastAI(Creature *creature) : ScriptedAI(creature) {}

            void InitializeAI() override
            {
                if (Unit* owner = me->GetOwner())
                {
                    for (auto itr : owner->m_whoHasMyAuras)
                    {
                        if (owner->TargetHasMyAura(itr.first, 120679))
                        {
                            if (Unit* _target = ObjectAccessor::GetUnit(*me, itr.first))
                            {
                                AttackStart(_target);
                                break;
                            }
                        }
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_dire_beastAI(creature);
        }
};

#define FIREBOLT   104318

/*######
# npc_wild_imp
######*/

class npc_wild_imp : public CreatureScript
{
    public:
        npc_wild_imp() : CreatureScript("npc_wild_imp") { }

        struct npc_wild_impAI : public ScriptedAI
        {
            npc_wild_impAI(Creature *creature) : ScriptedAI(creature)
            {
				initReactState();
            }

            void Reset() override
            {
				initReactState();

                if (me->GetOwner())
                    if (me->GetOwner()->getVictim())
                        AttackStart(me->GetOwner()->getVictim());
            }

            void UpdateAI(uint32 diff) override
            {
				if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
				{
					if (me->GetReactState() != REACT_HELPER)
						me->SetReactState(REACT_HELPER);
				}
				else
				{
					if (me->GetReactState() != REACT_AGGRESSIVE)
						me->SetReactState(REACT_AGGRESSIVE);
				}

                if (!me->GetOwner())
                    return;

                if (!me->GetOwner()->ToPlayer())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

				if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS) && me->getVictim())
                {
					if (!me->getVictim()->IsWithinLOSInMap(me) || me->getVictim()->GetDistance(me) > 35.f)
					{
						Follow(me->getVictim());
						return;
					}
                }

                if (me->getVictim() && me->getVictim()->HasCrowdControlAura(me))
                {
                    me->InterruptNonMeleeSpells(false);
                    return;
                }

				if ((me->getVictim() || me->GetOwner()->getAttackerForHelper()))
				{
					if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
						me->StopMoving();
					me->CastSpell(me->getVictim() ? me->getVictim() : me->GetOwner()->getAttackerForHelper(), FIREBOLT, false);
				}
                else if (Pet* pet = me->GetOwner()->ToPlayer()->GetPet())
                {
					if (pet->getAttackerForHelper())
					{
						if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
							me->StopMoving();
						me->CastSpell(me->getVictim() ? me->getVictim() : pet->getAttackerForHelper(), FIREBOLT, false);
					}
                }
            }

			void Follow(Unit* target)
			{
				me->GetMotionMaster()->MovePoint(0, target->GetPosition());
			}

			void initReactState() 
			{
				if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
					me->SetReactState(REACT_HELPER);
				else
					me->SetReactState(REACT_AGGRESSIVE);
			}
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_wild_impAI(creature);
        }
};

/*######
## npc_windwalk_totem
######*/

#define WINDWALK     114896

class npc_windwalk_totem : public CreatureScript
{
    public:
        npc_windwalk_totem() : CreatureScript("npc_windwalk_totem") { }

    struct npc_windwalk_totemAI : public ScriptedAI
    {
        npc_windwalk_totemAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->CastSpell(creature, WINDWALK, true);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me->HasAura(WINDWALK))
                me->CastSpell(me, WINDWALK, true);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_windwalk_totemAI(creature);
    }
};

/*######
## npc_ring_of_frost
######*/

class npc_ring_of_frost : public CreatureScript
{
    public:
        npc_ring_of_frost() : CreatureScript("npc_ring_of_frost") { }

        struct npc_ring_of_frostAI : public Scripted_NoMovementAI
        {
            npc_ring_of_frostAI(Creature *c) : Scripted_NoMovementAI(c)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new npc_ring_of_frostAI(pCreature);
        }
};

/*######
# npc_wild_mushroom
######*/

#define WILD_MUSHROOM_INVISIBILITY   92661

class npc_wild_mushroom : public CreatureScript
{
    public:
        npc_wild_mushroom() : CreatureScript("npc_wild_mushroom") { }

        struct npc_wild_mushroomAI : public ScriptedAI
        {
            uint32 CastTimer;
            bool stealthed;

            npc_wild_mushroomAI(Creature *creature) : ScriptedAI(creature)
            {
                CastTimer = 6000;
                stealthed = false;
                me->SetReactState(REACT_PASSIVE);
                me->SetMaxHealth(5);
            }

            void UpdateAI(uint32 diff) override
            {
                if (CastTimer <= diff && !stealthed)
                {
                    DoCast(me, WILD_MUSHROOM_INVISIBILITY, true);
                    stealthed = true;
                }
                else
                {
                    CastTimer -= diff;

                    if (!stealthed)
                        me->RemoveAura(WILD_MUSHROOM_INVISIBILITY);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_wild_mushroomAI(creature);
        }
};

/*######
## npc_fungal_growth
######*/

#define FUNGAL_GROWTH_PERIODIC  81282
#define FUNGAL_GROWTH_AREA      94339

class npc_fungal_growth : public CreatureScript
{
    public:
        npc_fungal_growth() : CreatureScript("npc_fungal_growth") { }

        struct npc_fungal_growthAI : public Scripted_NoMovementAI
        {
            npc_fungal_growthAI(Creature *c) : Scripted_NoMovementAI(c)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            void InitializeAI() override
            {
                ScriptedAI::InitializeAI();
                Unit * owner = me->GetOwner();
                if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
                    return;

                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                me->CastSpell(me, FUNGAL_GROWTH_PERIODIC, true);    // Periodic Trigger spell : decrease speed
                me->CastSpell(me, FUNGAL_GROWTH_AREA, true);        // Persistent Area
            }

            void UpdateAI(uint32 diff) override
            {
                if (!me->HasAura(FUNGAL_GROWTH_PERIODIC))
                    me->CastSpell(me, FUNGAL_GROWTH_PERIODIC, true);
            }
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new npc_fungal_growthAI(pCreature);
        }
};

/*######
## npc_spectral_guise -- 59607
######*/

enum spectralGuiseSpells
{
    SPELL_INITIALIZE_IMAGES         = 102284,
    SPELL_CLONE_CASTER              = 102288,
    SPELL_SPECTRAL_GUISE_CLONE      = 119012,
    SPELL_SPECTRAL_GUISE_CHARGES    = 119030,
    SPELL_SPECTRAL_GUISE_STEALTH    = 119032,
    SPELL_ROOT_FOR_EVER             = 31366,
};

class npc_spectral_guise : public CreatureScript
{
    public:
        npc_spectral_guise() : CreatureScript("npc_spectral_guise") { }

        struct npc_spectral_guiseAI : public Scripted_NoMovementAI
        {
            npc_spectral_guiseAI(Creature* c) : Scripted_NoMovementAI(c)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                if (!me->HasAura(SPELL_ROOT_FOR_EVER))
                    me->AddAura(SPELL_ROOT_FOR_EVER, me);
                if (Unit* owner = me->GetOwner())
                {
                    me->SetLevel(owner->getLevel());
                    me->SetMaxHealth(owner->GetMaxHealth() / 2);
                    me->SetHealth(me->GetMaxHealth());

                    owner->CastSpell(me, SPELL_CLONE_CASTER, true);
                    owner->CastSpell(me, SPELL_INITIALIZE_IMAGES, true);
                    me->CastSpell(me, SPELL_SPECTRAL_GUISE_CHARGES, true);
                    owner->CastSpell(me, 41055, true);
                    owner->AddAura(SPELL_SPECTRAL_GUISE_STEALTH, owner);

                    std::list<HostileReference*> threatList = owner->getThreatManager().getThreatList();
                    for (std::list<HostileReference*>::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
                        if (Unit* unit = (*itr)->getTarget())
                            if (unit->GetTypeId() == TYPEID_UNIT)
                                if (Creature* creature = unit->ToCreature())
                                    if (creature->canStartAttack(me, false))
                                        creature->Attack(me, true);

                    // Set no damage
                    me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 0.0f);
                    me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 0.0f);

                    me->AddAura(SPELL_ROOT_FOR_EVER, me);
                    me->DespawnOrUnsummon(6000);
                }
            }

            void UpdateAI(uint32 diff) override {}

            void EnterCombat(Unit* /*who*/) override {}

            void EnterEvadeMode() override {}
        };

        CreatureAI* GetAI(Creature *creature) const override
        {
            return new npc_spectral_guiseAI(creature);
        }
};

/*######
## npc_past_self
######*/

enum PastSelfSpells
{
    SPELL_FADING                    = 107550,
    SPELL_ALTER_TIME                = 110909,
    SPELL_ENCHANTED_REFLECTION      = 102284,
    SPELL_ENCHANTED_REFLECTION_2    = 102288,
};

struct auraData
{
    auraData(uint32 id, int32 duration) : m_id(id), m_duration(duration) {}
    uint32 m_id;
    int32 m_duration;
};

#define ACTION_ALTER_TIME   1

class npc_past_self : public CreatureScript
{
    public:
        npc_past_self() : CreatureScript("npc_past_self") { }

        struct npc_past_selfAI : public Scripted_NoMovementAI
        {
            npc_past_selfAI(Creature* c) : Scripted_NoMovementAI(c)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                me->SetMaxHealth(500);
                me->SetHealth(me->GetMaxHealth());
                mana = 0;
                health = 0;
                auras.clear();
            }

            int32 mana;
            int32 health;
            std::set<auraData*> auras;

            void Reset() override
            {
                if (!me->HasAura(SPELL_FADING))
                    me->AddAura(SPELL_FADING, me);
            }

            void IsSummonedBy(Unit* owner) override
            {
                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                {
                    Unit::AuraApplicationMap const& appliedAuras = owner->GetAppliedAuras();
                    for (Unit::AuraApplicationMap::const_iterator itr = appliedAuras.begin(); itr != appliedAuras.end(); ++itr)
                    {
                        if (Aura* aura = itr->second->GetBase())
                        {
                            SpellInfo const* auraInfo = aura->GetSpellInfo();
                            if (!auraInfo)
                                continue;

                            if (auraInfo->Id == SPELL_ALTER_TIME)
                                continue;

                            if (auraInfo->IsPassive())
                                continue;

                            auras.insert(new auraData(auraInfo->Id, aura->GetDuration()));
                        }
                    }

                    mana = owner->GetPower(POWER_MANA);
                    health = owner->GetHealth();

                    owner->AddAura(SPELL_ENCHANTED_REFLECTION, me);
                    owner->AddAura(SPELL_ENCHANTED_REFLECTION_2, me);
                }
                else
                    me->DespawnOrUnsummon();
            }

            void DoAction(const int32 action) override
            {
                if (action == ACTION_ALTER_TIME)
                {
                    if (TempSummon* pastSelf = me->ToTempSummon())
                    {
                        if (Unit* m_owner = pastSelf->GetSummoner())
                        {
                            if (m_owner->ToPlayer())
                            {
                                if (!m_owner->isAlive())
                                    return;

                                //m_owner->RemoveNonPassivesAuras();

                                for (std::set<auraData*>::iterator itr = auras.begin(); itr != auras.end(); ++itr)
                                {
                                    Aura* aura = !m_owner->HasAura((*itr)->m_id) ? m_owner->AddAura((*itr)->m_id, m_owner) : m_owner->GetAura((*itr)->m_id);
                                    if (aura)
                                    {
                                        aura->SetDuration((*itr)->m_duration);
                                        aura->SetNeedClientUpdateForTargets();
                                    }

                                    delete (*itr);
                                }

                                auras.clear();

                                m_owner->SetPower(POWER_MANA, mana);
                                m_owner->SetHealth(health);

                                m_owner->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), true);
                                me->DespawnOrUnsummon(100);
                            }
                        }
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature *creature) const override
        {
            return new npc_past_selfAI(creature);
        }
};

class npc_guild_battle_standard : public CreatureScript
{
    public:
        npc_guild_battle_standard() : CreatureScript("npc_guild_battle_standard") { }

        struct npc_guild_battle_standardAI : public Scripted_NoMovementAI
        {
            npc_guild_battle_standardAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                creature->SetReactState(REACT_PASSIVE);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_guild_battle_standardAI(creature);
        }
};

enum MiscData
{
    QUEST_ID            = 44765,
    
    DATA_CHECK          = 0,
    
    QuestMaxCount       = 50
};

class npc_riggle_bassbait : public CreatureScript
{
    public:
        npc_riggle_bassbait() : CreatureScript("npc_riggle_bassbait") {}

    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        QuestMenu& questMenu = pPlayer->PlayerTalkClass->GetQuestMenu();

        if (pCreature->AI()->GetData(DATA_CHECK) < QuestMaxCount)
            questMenu.AddMenuItem(QUEST_ID, 4);

        pPlayer->SEND_GOSSIP_MENU(6421, pCreature->GetGUID());

        return true;
    }

    struct npc_riggle_bassbaitAI : public ScriptedAI
    {
        npc_riggle_bassbaitAI(Creature* creature) : ScriptedAI(creature) 
        {
            count = 0;
            StartEvent = true;
        }

        uint32 count;
        bool StartEvent;

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_CHECK)
                return count;
            return 0;
        }

        void Reset() override
        {
            if (StartEvent)
            {
                ZoneTalk(TEXT_GENERIC_0, ObjectGuid::Empty);
                StartEvent = false;
            }
        }

        void OnQuestReward(Player* player, Quest const* quest) override
        {
            ++count;
            // player->CreateConversation(3904);
            // me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        }

        void OnStartQuest(Player* player, Quest const* /*quest*/) override
        {
            player->CreateConversation(3904);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_riggle_bassbaitAI(creature);
    }
};

struct npc_mirror_image : public ScriptedAI
{
    npc_mirror_image(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*who*/) override
    {
        if (auto owner = me->GetAnyOwner())
        {
            me->SetControlled(1, UNIT_STATE_ROOT);
            owner->CastSpell(me, SPELL_CLONE_CASTER, true);
            owner->CastSpell(me, 41055, true);
            owner->CastSpell(me, 45206, true);
            me->SetFacingToObject(owner);
            owner->SetFacingToObject(me);
        }
    }
};

// 111748
struct npc_shadowy_reflection : public ScriptedAI
{
    npc_shadowy_reflection(Creature* creature) : ScriptedAI(creature)
    {
    }

    void IsSummonedBy(Unit* /*who*/) override
    {
        if (Unit* owner = me->GetAnyOwner())
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetControlled(1, UNIT_STATE_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            owner->CastSpell(me, 222485, true);
            me->SetMaxHealth(owner->GetMaxHealth());
            me->SetFullHealth();
        }
    }
};

enum eSpells
{
    SPELL_TRAMPOLINE_BOUNCE_1 = 79040,
    SPELL_TRAMPOLINE_BOUNCE_2 = 79044,
};

class npc_hyjal_soft_target : public CreatureScript
{
    public:
        npc_hyjal_soft_target() : CreatureScript("npc_hyjal_soft_target") { }

        struct npc_hyjal_soft_targetAI : public ScriptedAI
        {
            npc_hyjal_soft_targetAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            uint32 JumpTimer;

            void Reset() override
            {
                JumpTimer = 1000;
            }

            void UpdateAI(uint32 diff) override
            {
                if (JumpTimer <= diff)
                {
                    JumpTimer = 1000;
                    if (Player* pTarget = me->SelectNearestPlayer(1.5f))
                    {
                        uint32 spell;
                        if (urand(0, 1) == 0)
                            spell = SPELL_TRAMPOLINE_BOUNCE_1;
                        else
                            spell = SPELL_TRAMPOLINE_BOUNCE_2;

                        DoCast(pTarget, spell);
                    }
                }
                else
                    JumpTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_hyjal_soft_targetAI(creature);
        }
};

enum SSMBgSpells
{
    BG_SSM_PREVENTION_AURA = 135846,
    BG_SSM_TRACK_SWITCH_OPENED = 120228,
    BG_SSM_TRACK_SWITCH_CLOSED = 120229,
};

class npc_track_switch : public CreatureScript
{
public:
    npc_track_switch() : CreatureScript("npc_track_switch") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->HasAura(BG_SSM_PREVENTION_AURA))
        {
            player->PlayerTalkClass->SendCloseGossip();
            return false;
        }

        if (creature->HasAura(BG_SSM_TRACK_SWITCH_OPENED))
        {
            creature->CastSpell(creature, BG_SSM_TRACK_SWITCH_CLOSED, true);
            creature->RemoveAurasDueToSpell(BG_SSM_TRACK_SWITCH_OPENED);
            creature->CastSpell(creature, BG_SSM_PREVENTION_AURA, true);
        }
        else
        {
            creature->CastSpell(creature, BG_SSM_TRACK_SWITCH_OPENED, true);
            creature->RemoveAurasDueToSpell(BG_SSM_TRACK_SWITCH_CLOSED);
            creature->CastSpell(creature, BG_SSM_PREVENTION_AURA, true);
        }

        return true;
    }
};

// 110472
class npc_king_mrgl : public CreatureScript
{
    public:
        npc_king_mrgl() : CreatureScript("npc_king_mrgl") { }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            uint32 count_cur = player->GetCurrency(1268) / sDB2Manager.GetCurrencyPrecision(1268);
            
            uint32 honors = floor(count_cur / 4);
            if (honors)
            {           
                player->RewardHonor(NULL, 1, honors);
                player->ModifyCurrency(1268, -1*honors*4*sDB2Manager.GetCurrencyPrecision(1268));
            }
            ChatHandler(player).PSendSysMessage("You have received %u %u honors", count_cur, honors);
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
};

//68553
class npc_slg_generic_mop_large_aoi : public CreatureScript
{
public:
    npc_slg_generic_mop_large_aoi() : CreatureScript("npc_slg_generic_mop_large_aoi") {}

    struct npc_slg_generic_mop_large_aoiAI : public ScriptedAI
    {
        npc_slg_generic_mop_large_aoiAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 diesCount = 0;

        void Reset() override
        {
            if (me->GetMapId() == 1279) //The Everbloom
                DoCast(169147); //Visual Teleport To Stormwind
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            if (summon->GetEntry() == 112973) //NightHold: Duskwatch Weaver
            {
                ++diesCount;

                if (diesCount == 9)
                    me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_slg_generic_mop_large_aoiAI (creature);
    }
};

class npc_nightmare_hitching_post : public CreatureScript
{
    public:
        npc_nightmare_hitching_post() : CreatureScript("npc_nightmare_hitching_post") { }

        struct npc_nightmare_hitching_postAI : public AnyPetAI
        {
            bool gate;
            npc_nightmare_hitching_postAI(Creature* creature) : AnyPetAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void IsSummonedBy(Unit* who) override
            {
                me->SetCharmerGUID(ObjectGuid::Empty);
                me->SetCreatorGUID(ObjectGuid::Empty);
                
                me->ClearUnitState(UNIT_STATE_FOLLOW);

                me->StopMoving();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
                
                me->SetControlled(true, UNIT_STATE_ROOT);
            }
            void OnSpellClick(Unit* who) override
            {
                if (!who || !me->ToTempSummon())
                    return;
                
                if (Unit* owner = me->ToTempSummon()->GetSummoner())
                {
                    Player* ownerPlayer = owner->ToPlayer();
                    Player* whoPlayer = who->ToPlayer();
                    if(!ownerPlayer || !whoPlayer || !ownerPlayer->IsGroupVisibleFor(whoPlayer))
                        return;
                    
                    whoPlayer->CastSpell(whoPlayer, 162997);
                    
                    if (whoPlayer->IsMounted())
                    {
                        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(80651);
                        if (!(ci))
                            return;

                        uint32 displayID = sObjectMgr->ChooseDisplayId(0, ci);
                        sObjectMgr->GetCreatureModelRandomGender(&displayID);
                        whoPlayer->Mount(displayID);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_nightmare_hitching_postAI(creature);
        }
};

//90401
class npc_azsuna_allari_q37660 : public CreatureScript
{
public:
    npc_azsuna_allari_q37660() : CreatureScript("npc_azsuna_allari_q37660") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (player->GetQuestObjectiveData(37660, 90403))
            return false;

        player->PlayerTalkClass->SendCloseGossip();
        player->RewardPlayerAndGroupAtEvent(90403, player);
        creature->AI()->Talk(1);

        return true;
    }

    struct npc_azsuna_allari_q37660AI : public ScriptedAI
    {
        npc_azsuna_allari_q37660AI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit* who) override
        {
            who->AddPlayerInPersonnalVisibilityList(who->GetGUID());
            me->GetMotionMaster()->MovePath(90401, false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != WAYPOINT_MOTION_TYPE)
                return;

            if (id == 2)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                me->AddDelayedEvent(1500, [this] {
                    if (Creature* firstdemon = me->FindNearestCreature(90402, 60.0f, true))
                        DoCast(firstdemon, 178939);
                });
                me->AddDelayedEvent(4500, [this] {
                    Talk(2);
                });
                break;
            case 2:
                me->AddDelayedEvent(1500, [this] {
                    if (Creature* seconddemon = me->FindNearestCreature(89276, 60.0f, true))
                        DoCast(seconddemon, 178939);
                });
                me->AddDelayedEvent(4500, [this] {
                    Talk(2);
                });
                me->AddDelayedEvent(10000, [this] {
                    if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                        if (Player* player = Owner->ToPlayer())
                            player->RewardPlayerAndGroupAtEvent(89276, player);
                });
                me->AddDelayedEvent(15000, [this] {
                    Talk(5);
                });
                me->AddDelayedEvent(20000, [this] {
                    Talk(6);
                });
                me->AddDelayedEvent(25000, [this] {
                    Talk(7);
                });
                me->AddDelayedEvent(30000, [this] {
                    me->DespawnOrUnsummon(500);
                });
                break;
            }
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_azsuna_allari_q37660AI(creature);
    }
};

//107995
class npc_azsuna_stellagosa_q37862 : public CreatureScript
{
public:
    npc_azsuna_stellagosa_q37862() : CreatureScript("npc_azsuna_stellagosa_q37862") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (player->GetQuestObjectiveData(37862, 107995) || (player->GetQuestStatus(37862) == QUEST_STATUS_REWARDED
            || player->GetQuestStatus(37862) == QUEST_STATUS_COMPLETE))
            return false;

        player->PlayerTalkClass->SendCloseGossip();
        player->SummonCreature(107995, creature->GetPosition());

        return true;
    }

    struct npc_azsuna_stellagosa_q37862AI : public ScriptedAI
    {
        npc_azsuna_stellagosa_q37862AI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);
        }

        void IsSummonedBy(Unit* who) override
        {
            if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                if (Player* player = Owner->ToPlayer())
                    player->RewardPlayerAndGroupAtEvent(107995, player);
            who->AddPlayerInPersonnalVisibilityList(who->GetGUID());
            who->CastSpell(me, 77901);
            
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->AddDelayedEvent(2000, [this] {
                me->GetMotionMaster()->MovePath(107995, false);;
            });
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != WAYPOINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    if (Player* player = Owner->ToPlayer())
                        player->CastSpell(player, 214402);
                Talk(0);
            }

            if (id == 2)
                Talk(1);

            if (id == 3)
                Talk(2);

            if (id == 4)
                Talk(3);

            if (id == 9)
            {
                if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    if (Player* player = Owner->ToPlayer())
                        player->RemoveAurasDueToSpell(214402);
                me->DespawnOrUnsummon(1000);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_azsuna_stellagosa_q37862AI(creature);
    }
};


Position const revilWP[83] =
{
    { -10340.1f, -1257.12f, 35.3007f },
    { -10322.7f, -1302.15f, 43.0493f },
    { -10314.1f, -1322.48f, 50.1130f },
    { -10314.7f, -1352.39f, 57.6878f },
    { -10323.0f, -1374.87f, 63.3569f },
    { -10329.0f, -1388.85f, 67.6867f },
    { -10338.1f, -1404.26f, 69.4972f },
    { -10343.8f, -1411.67f, 71.2387f },
    { -10350.7f, -1418.14f, 73.7246f },
    { -10357.3f, -1427.69f, 77.9138f },
    { -10358.0f, -1436.61f, 82.3707f },
    { -10361.9f, -1439.45f, 83.3329f },
    { -10369.4f, -1442.81f, 83.7723f },
    { -10374.2f, -1443.17f, 83.8965f },
    { -10390.0f, -1442.06f, 84.1777f },
    { -10406.0f, -1440.68f, 83.9021f },
    { -10415.4f, -1437.34f, 80.5631f },
    { -10420.1f, -1433.61f, 78.2878f },
    { -10425.2f, -1430.41f, 75.7152f },
    { -10430.3f, -1427.62f, 73.1362f },
    { -10439.1f, -1429.63f, 71.4691f },
    { -10448.9f, -1439.41f, 69.0887f },
    { -10453.4f, -1447.40f, 68.9825f },
    { -10455.6f, -1455.19f, 69.8618f },
    { -10453.2f, -1463.69f, 71.0152f },
    { -10451.8f, -1469.66f, 71.9085f }, //26

    { -10450.4f, -1478.22f, 73.086f },
    { -10448.6f, -1493.43f, 74.5244f },
    { -10449.4f, -1512.91f, 74.6119f },
    { -10450.7f, -1532.4f, 74.7503f },
    { -10453.0f, -1552.75f, 73.8262f },
    { -10455.6f, -1570.69f, 73.5308f },
    { -10456.8f, -1579.08f, 73.527f },
    { -10459.8f, -1598.48f, 73.3384f },
    { -10463.5f, -1617.19f, 73.393f },
    { -10464.7f, -1630.09f, 74.1403f },
    { -10464.5f, -1646.19f, 75.9377f },
    { -10465.1f, -1660.54f, 77.2839f },
    { -10466.5f, -1670.03f, 78.6509f },
    { -10465.1f, -1681.93f, 80.0648f },
    { -10460.0f, -1702.89f, 82.1836f },
    { -10458.8f, -1721.87f, 84.333f },
    { -10459.4f, -1737.71f, 87.2548f },
    { -10455.7f, -1748.18f, 88.6723f },
    { -10452.2f, -1757.17f, 90.1557f },
    { -10442.8f, -1772.37f, 93.9909f },
    { -10434.0f, -1788.4f, 96.7405f },
    { -10430.7f, -1804.62f, 98.1107f },
    { -10431.8f, -1812.7f, 99.3514f },
    { -10434.5f, -1823.62f, 100.64f },
    { -10439.0f, -1835.37f, 101.923f },
    { -10442.4f, -1848.84f, 103.478f },
    { -10445.2f, -1862.04f, 104.863f },
    { -10444.4f, -1878.11f, 104.335f },
    { -10438.9f, -1931.75f, 104.616f }, //55

    { -10439.8f, -1953.88f, 103.45f },
    { -10443.1f, -1968.3f, 102.588f },
    { -10441.6f, -1977.59f, 101.44f },
    { -10437.9f, -1986.84f, 100.194f },
    { -10431.9f, -1999.91f, 99.0323f },
    { -10428.3f, -2009.73f, 98.0045f },
    { -10434.3f, -2027.9f, 95.6495f },
    { -10441.3f, -2036.76f, 94.592f },
    { -10458.3f, -2039.62f, 94.8246f },
    { -10477.4f, -2041.67f, 94.5103f },
    { -10505.8f, -2046.45f, 92.4629f },
    { -10511.2f, -2053.68f, 91.9942f },
    { -10516.6f, -2065.14f, 91.9942f },
    { -10525.3f, -2077.04f, 91.6195f },
    { -10533.5f, -2087.24f, 91.5412f },
    { -10543.2f, -2100.32f, 91.7799f },
    { -10536.1f, -2109.99f, 88.8931f },
    { -10526.0f, -2116.87f, 89.4585f },
    { -10511.2f, -2120.04f, 91.1696f },
    { -10508.1f, -2120.42f, 91.2437f },
    { -10497.8f, -2123.38f, 90.794f },
    { -10490.0f, -2129.53f, 90.7914f },
    { -10480.1f, -2137.58f, 90.7798f },
    { -10467.9f, -2137.57f, 90.7902f },
    { -10455.8f, -2138.84f, 90.7796f },
    { -10445.4f, -2141.07f, 90.7797f },
    { -10440.0f, -2143.57f, 90.7797f }, //83
};

//100578
class npc_revil_kost_following_the_curse : public CreatureScript
{
public:
    npc_revil_kost_following_the_curse() : CreatureScript("npc_revil_kost_following_the_curse") {}

    struct npc_revil_kost_following_the_curseAI : public ScriptedAI
    {
        npc_revil_kost_following_the_curseAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_ATTACK_OFF);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED);
            pointid = 0;
        }

        uint32 checktimer = 2000;
        uint32 holyfire = urand(4000,6000);
        uint32 curgel = urand(2000,4000);
        uint32 holynova = urand(8000,10000);
        uint32 pointid;
        uint8 ridersdied = 0;

        void Reset()
        {
            checktimer = 2000;
            holyfire = urand(4000, 6000);
            curgel = urand(2000, 4000);
            holynova = urand(8000, 10000);
        }

        void DoAction(int32 const action) override
        {
            if (action == 1)
            {
                ++ridersdied;
                if (ridersdied == 2)
                {
                    Talk(1);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED);
                    me->SetReactState(REACT_ATTACK_OFF);
                    me->GetMotionMaster()->Clear();
                    me->AddDelayedEvent(2000, [this] {
                        me->AddAura(66090, me);
                    });
                    me->AddDelayedEvent(2000, [this] {
                        me->GetMotionMaster()->MovePoint(1, revilWP[1]);
                    });

                }
            }
            if (action == 2)
            {
                ++ridersdied;
                if (ridersdied == 3)
                {
                    Talk(4);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED);
                    me->SetReactState(REACT_ATTACK_OFF);
                    me->AddDelayedEvent(2000, [this] {
                        me->AddAura(66090, me);
                    });
                    me->AddDelayedEvent(2000, [this] {
                        me->GetMotionMaster()->MovePoint(26, revilWP[26]);
                    });
                }
            }
            if (action == 3)
            {
                ++ridersdied;
                if (ridersdied == 5)
                {
                    Talk(7);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED);
                    me->SetReactState(REACT_ATTACK_OFF);
                    me->AddDelayedEvent(2000, [this] {
                        me->AddAura(66090, me);
                    });
                    me->AddDelayedEvent(2000, [this] {
                        me->GetMotionMaster()->MovePoint(56, revilWP[56]);
                    });
                }
            }

        }

        void IsSummonedBy(Unit* who) override
        {
            who->AddPlayerInPersonnalVisibilityList(who->GetGUID());
            me->AddDelayedEvent(2000, [this] {
                Talk(0);
                me->GetMotionMaster()->MovePoint(0, revilWP[0]);
            });
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                case 0:
                    me->AddDelayedEvent(5000, [this] {
                        if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                            if (Player* player = Owner->ToPlayer())
                            {
                                if (Creature* add1 = player->SummonCreature(99875, -10308.5f, -1275.69f, 39.3f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                                    add1->Attack(me, true);
                                if (Creature* add2 = player->SummonCreature(100346, -10316.5f, -1244.49f, 36.69f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                                    add2->Attack(me, true);

                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED);
                                me->SetReactState(REACT_HELPER);
                            }
                    });
                    break;
                case 1:
                    ++pointid;
                    me->AddDelayedEvent(100, [this] {
                        me->GetMotionMaster()->MovePoint(2, revilWP[2]);
                    });
                    break;
                case 20:
                    Talk(5);
                    break;
                case 25:
                    me->RemoveAurasDueToSpell(66090);
                    me->AddDelayedEvent(1000, [this] {
                        me->StopMoving();
                        Talk(3);
                    });
                    me->AddDelayedEvent(2000, [this] {
                        if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                            if (Player* player = Owner->ToPlayer())
                                if (Creature* add3 = player->SummonCreature(100704, -10476.8f, -1429.74f, 66.1f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                                    add3->Attack(me, true);

                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED);
                        me->SetReactState(REACT_HELPER);
                    });
                    break;
                case 26:
                    pointid = 26;
                    me->AddDelayedEvent(100, [this] {
                        me->GetMotionMaster()->MovePoint(pointid + 1, revilWP[pointid + 1]);
                    });
                    break;
                case 55:
                    Talk(6);
                    me->RemoveAurasDueToSpell(66090);
                    me->AddDelayedEvent(2000, [this] {
                        if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                            if (Player* player = Owner->ToPlayer())
                            {
                                if (Creature* add4 = player->SummonCreature(100707, -10405.5f, -1939.46f, 103.16f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                                    add4->Attack(me, true);
                                if (Creature* add5 = player->SummonCreature(100708, -10405.5f, -1935.29f, 103.30f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                                    add5->Attack(me, true);

                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED);
                                me->SetReactState(REACT_HELPER);
                            }
                    });
                    break;
                case 73:
                    Talk(8);
                    me->AddDelayedEvent(500, [this] {
                        me->StopMoving();
                    });
                    me->AddDelayedEvent(2000, [this] {
                        pointid = 73;
                        me->GetMotionMaster()->MovePoint(pointid + 1, revilWP[pointid + 1]);
                    });
                    break;
                case 82:
                    me->StopMoving();
                    if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                        if (Player* player = Owner->ToPlayer())
                        {
                            player->RewardPlayerAndGroupAtEvent(100716, player);
                            player->RewardPlayerAndGroupAtEvent(100655, player);
                            player->CastSpell(player, 198215, true);
                            me->DespawnOrUnsummon(500);
                        }
                    break;
                default:
                    break;
                }

                if (id >= 2 && id < 25)
                {
                    ++pointid;
                    me->AddDelayedEvent(100, [this] {
                        me->GetMotionMaster()->MovePoint(pointid + 1, revilWP[pointid + 1]);
                    });
                }
                if (id >= 26 && id < 55)
                {
                    ++pointid;
                    me->AddDelayedEvent(100, [this] {
                        me->GetMotionMaster()->MovePoint(pointid + 1, revilWP[pointid + 1]);
                    });
                }
                if (id >= 56 && id < 73)
                {
                    ++pointid;
                    me->AddDelayedEvent(100, [this] {
                        me->GetMotionMaster()->MovePoint(pointid + 1, revilWP[pointid + 1]);
                    });
                }
                if (id >= 74 && id < 82)
                {
                    ++pointid;
                    me->AddDelayedEvent(100, [this] {
                        me->GetMotionMaster()->MovePoint(pointid + 1, revilWP[pointid + 1]);
                    });
                }
            }

        }

        void UpdateAI(uint32 diff)
        {

            if (!UpdateVictim())
                return;

            if (holyfire <= diff)
            {
                DoCast(201642);
                holyfire = urand(8000, 10000);
            }else holyfire -= diff;

            if (curgel <= diff)
            {
                DoCast(201645);
                curgel = urand(4000, 6000);
            }else curgel -= diff;

            if (holynova <= diff)
            {
                DoCast(201877);
                holynova = urand(12000, 16000);
            }else holynova -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_revil_kost_following_the_curseAI(creature);
    }
};

//100346 99875 - first pack, 100704 - second, 100707 100708 - third
class npc_revil_kost_dark_rider : public CreatureScript
{
public:
    npc_revil_kost_dark_rider() : CreatureScript("npc_revil_kost_dark_rider") {}

    struct npc_revil_kost_dark_riderAI : public ScriptedAI
    {
        npc_revil_kost_dark_riderAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        uint32 cursedblade = urand(5000,8000);
        uint32 ridersmark = urand(10000,12000);

        void IsSummonedBy(Unit* who) override
        {
            who->AddPlayerInPersonnalVisibilityList(who->GetGUID());
            Talk(0);
            me->AddDelayedEvent(1500, [this] {
                if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    if (Player* player = Owner->ToPlayer())
                        me->Attack(player, true);
            });
        }

        void JustDied(Unit* killer) override
        {
            Unit* Owner = me->ToTempSummon()->GetSummoner();
            if (!Owner)
                return;
            Player* player = Owner->ToPlayer();
            if (!player)
                return;
            GuidList* revil = player->GetSummonList(100578);
            for (GuidList::const_iterator iter = revil->begin(); iter != revil->end(); ++iter)
            {
                if (!revil)
                    return;
                if (me->GetEntry() == 100346 || me->GetEntry() == 99875)
                    if (Creature* summon = ObjectAccessor::GetCreature(*player, (*iter)))
                        summon->GetAI()->DoAction(1);
                if (me->GetEntry() == 100704)
                    if (Creature* summon = ObjectAccessor::GetCreature(*player, (*iter)))
                        summon->GetAI()->DoAction(2);
                if (me->GetEntry() == 100707 || me->GetEntry() == 100708)
                    if (Creature* summon = ObjectAccessor::GetCreature(*player, (*iter)))
                        summon->GetAI()->DoAction(3);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (cursedblade <= diff)
            {
                DoCast(201830);
                cursedblade = urand(8000, 10000);
            }else cursedblade -= diff;
            
            if (ridersmark <= diff)
            {
                DoCast(201763);
                ridersmark = urand(12000, 15000);
            }else ridersmark -= diff;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_revil_kost_dark_riderAI(creature);
    }
};

//105586
Position const wpoints[4] =
{
    { -784.01f, 4575.47f, 728.09f },
    { -787.05f, 4573.18f, 728.06f },
    { -782.90f, 4563.23f, 727.06f },
    { -778.14f, 4565.29f, 726.91f },
};

class npc_dalaran_defender_barrem : public CreatureScript
{
public:
    npc_dalaran_defender_barrem() : CreatureScript("npc_dalaran_defender_barrem") {}

    struct npc_dalaran_defender_barremAI : public ScriptedAI
    {
        npc_dalaran_defender_barremAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetVisible(true);
            DoCast(209190);
            me->SetStandState(UNIT_STAND_STATE_SLEEP);
            me->SetHealth(me->GetMaxHealth() / 5);
        }

        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
        {
            if (spellId == 209190 && !apply)
            {
                Talk(5);
                if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    if (Player* player = Owner->ToPlayer())
                        if (Creature* cre = me->SummonCreature(105733, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 60000))
                            cre->AddPlayerInPersonnalVisibilityList(player->GetGUID());

                me->DespawnOrUnsummon(60000); //if player do nothing
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            if (summon->GetEntry() == 105733)
            {
                me->AddDelayedEvent(1000, [this] {
                    Talk(3);
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                });
                me->AddDelayedEvent(2000, [this] {
                    me->GetMotionMaster()->MovePoint(0, wpoints[0]);
                });
                me->AddDelayedEvent(7000, [this] {
                    Talk(4);
                });
                me->AddDelayedEvent(9000, [this] {
                    me->GetMotionMaster()->MovePoint(1, wpoints[1]);
                });
            }
        }

        void Complete()
        {
            if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                if (Player* player = Owner->ToPlayer())
                {
                    player->RewardPlayerAndGroupAtEvent(105586, player);
                    me->SetVisible(false);
                }

        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                case 1:
                    me->AddDelayedEvent(50, [this] {
                        me->GetMotionMaster()->MovePoint(2, wpoints[2]);
                    });
                    break;
                case 2:
                    me->AddDelayedEvent(50, [this] {
                        me->GetMotionMaster()->MovePoint(3, wpoints[3]);
                    });
                    break;
                case 3:
                    if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                        if (Player* player = Owner->ToPlayer())
                            if (Creature* boros = me->FindNearestCreature(105602, 8.0f, true))
                            {
                                boros->AI()->Talk(2);
                                me->AddDelayedEvent(5500, [this, player] {
                                    Complete();
                                });
                            }
                    DoCast(41995);
                    break;
                default:
                    break;
                }
            }
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dalaran_defender_barremAI(creature);
    }
};

//91185
class npc_azsuna_daglop_q38237 : public CreatureScript
{
public:
    npc_azsuna_daglop_q38237() : CreatureScript("npc_azsuna_daglop_q38237") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (player->GetQuestObjectiveData(38237, 91185))
            return false;

        player->PlayerTalkClass->SendCloseGossip();
        player->RewardPlayerAndGroupAtEvent(91185, player);
        if (Creature* daglop = player->SummonCreature(91185, creature->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 200000))
            daglop->AddPlayerInPersonnalVisibilityList(player->GetGUID());

        return true;
    }

    struct npc_azsuna_daglop_q38237AI : public ScriptedAI
    {
        npc_azsuna_daglop_q38237AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() {}

        void IsSummonedBy(Unit* who) override
        {
            me->SetVisible(true);
            me->GetMotionMaster()->MovePoint(0, -541.24f, 5648.40f, 3.06f);
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            if (summon->GetEntry() == 91184)
            {
                me->AddDelayedEvent(1000, [this] {
                    Talk(1);
                    me->SetReactState(REACT_PASSIVE);
                });
                me->AddDelayedEvent(2000, [this] {
                    me->GetMotionMaster()->MovePoint(1, -519.28f, 5607.60f, 4.59f);
                });
            }
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == 181029)
            {
                if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    if (Player* player = Owner->ToPlayer())
                        if (Creature* sum = me->SummonCreature(91184, -546.67f, 5658.04f, 2.81f, 5.22f, TEMPSUMMON_TIMED_DESPAWN, 200000))
                        {
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE);
                            me->SetReactState(REACT_AGGRESSIVE);
                            sum->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                            sum->SetVisible(true);
                        }
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                case 0:
                    me->AddDelayedEvent(1500, [this] {
                        DoCast(181029);
                    });
                    break;
                case 1:
                    me->SetVisible(false);
                    me->DespawnOrUnsummon(500);
                    break;
                default:
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoSpellAttackIfReady(36227);
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_azsuna_daglop_q38237AI(creature);
    }
};

//97648
class npc_grasp_of_underking_quest : public CreatureScript
{
public:
    npc_grasp_of_underking_quest() : CreatureScript("npc_grasp_of_underking_quest") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetQuestStatus(39025) == QUEST_STATUS_INCOMPLETE)
        {
            for (uint32 npcs : { 95051, 94255, 94286})
                if (Creature* crea = creature->FindNearestCreature(npcs, 5.0f, true))
                {
                    if (uint32 id = crea->GetEntry())
                        player->RewardPlayerAndGroupAtEvent(id, player);
                    crea->SetDisableGravity(false);
                    crea->ExitVehicle();
                    crea->AddDelayedEvent(500, [crea] {
                        crea->AI()->Talk(0);
                        crea->GetMotionMaster()->MoveRandom(12.0f);
                        crea->DespawnOrUnsummon(6500);
                    });
                    creature->Kill(creature);
                }
            return true;
        }
        player->PlayerTalkClass->SendCloseGossip();
        return true;
    }

    struct npc_grasp_of_underking_questAI : public ScriptedAI
    {
        npc_grasp_of_underking_questAI(Creature* creature) : ScriptedAI(creature) 
        {
            boarded = false;
        }

        bool boarded;

        void Reset() 
        {
            ResetGrasp();
        }

        void JustDied(Unit* /*killer*/) override
        {
            boarded = false;
        }

        void ResetGrasp() 
        {
            if (!boarded)
            {
                boarded = true;
                me->AddDelayedEvent(2000, [this] {
                    if (me->GetPositionX() == 3736.11f && me->GetPositionY() == 4887.47f)
                        if (Creature* oro = me->SummonCreature(95051, me->GetPosition()))
                            oro->EnterVehicle(me, 0);
                    if (me->GetPositionX() == 3649.89f && me->GetPositionY() == 4908.65f)
                        if (Creature* jale = me->SummonCreature(94255, me->GetPosition()))
                            jale->EnterVehicle(me, 0);
                    if (me->GetPositionX() == 3575.87f && me->GetPositionY() == 4839.52f)
                        if (Creature* oakin = me->SummonCreature(94286, me->GetPosition()))
                            oakin->EnterVehicle(me, 0);
                });
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_grasp_of_underking_questAI(creature);
    }
};

// 90317
enum npcs
{
    NPC_DOG         = 90241,
    NPC_SOLDIER     = 101943,
    NPC_INFERNAL    = 93619,
    NPC_GUARD       = 90230
};

Position const dogsdrpos[3] =
{
    { -201.42f, 7005.10f, 4.75f, 5.69f }, //bridge
    { -153.57f, 7091.64f, 0.27f, 5.15f }, //center
    { -181.03f, 7032.41f, -0.83f, 4.97f },
};

Position const infpos = { -154.46f, 7105.88f, 0.0f, 4.9f };

struct npc_azsuna_illidari_outpost_initiator : public ScriptedAI
{
    npc_azsuna_illidari_outpost_initiator(Creature* creature) : ScriptedAI(creature) {}

    uint32 spawnTimer = urand(5000, 9000);
    bool dog = false;
    bool sdr = false;
    bool inf = false;
    bool ncr = false;

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case NPC_DOG:
            dog = false;
            break;
        case NPC_SOLDIER:
            sdr = false;
            break;
        case NPC_INFERNAL:
            inf = false;
            break;
        case NPC_GUARD:
            ncr = false;
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (spawnTimer <= diff)
        {
            if (!dog)
            {
                if (auto _add = me->SummonCreature(NPC_DOG, dogsdrpos[urand(0, 1)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000))
                {
                    _add->SetWalk(false);
                    if (_add->GetPositionX() < -170.0f)
                        _add->GetMotionMaster()->MovePoint(0, -167.0f, 6979.0f, 5.05f);
                    else
                        _add->GetMotionMaster()->MovePoint(0, -126.0f, 7017.0f, 1.15f);
                    dog = true;
                }
            }
            if (!sdr)
            {
                if (auto _add = me->SummonCreature(NPC_SOLDIER, dogsdrpos[urand(0, 1)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000))
                {
                    _add->SetWalk(false);
                    if (_add->GetPositionX() < -170.0f)
                        _add->GetMotionMaster()->MovePoint(0, -167.0f, 6979.0f, 5.05f);
                    else
                        _add->GetMotionMaster()->MovePoint(0, -126.0f, 7017.0f, 1.15f);
                    sdr = true;
                }
            }
            if (!inf)
            {
                if (auto _add = me->SummonCreature(NPC_INFERNAL, infpos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000))
                {
                    _add->GetMotionMaster()->MovePoint(0, -126.0f, 7017.0f, 1.15f);
                    inf = true;
                }
            }
            if (!ncr)
            {
                if (auto _add = me->SummonCreature(NPC_GUARD, dogsdrpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000))
                {
                    _add->SetWalk(false);
                    _add->GetMotionMaster()->MovePoint(0, -126.0f, 7017.0f, 1.15f);
                    ncr = true;
                }
            }
            spawnTimer = urand(5000, 9000);
        }
        else spawnTimer -= diff;
    }
};

class npc_crash_test : public CreatureScript
{
public:
    npc_crash_test() : CreatureScript("npc_crash_test") {}

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        if (!player->isGameMaster())
            return false;

        char printinfo[500];
        sprintf(printinfo, "Current radius: %u", m_radius);
        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, printinfo, GOSSIP_SENDER_MAIN, 1, "", 0, true);
        
        sprintf(printinfo, "Count of npc: %u", m_count);
        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, printinfo, GOSSIP_SENDER_MAIN, 2, "", 0, true);
        
        sprintf(printinfo, "Time of despawn (in ms): %u", m_despawn);
        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, printinfo, GOSSIP_SENDER_MAIN, 3, "", 0, true);

        sprintf(printinfo, "NPC grouped by %u npc", m_grouped);
        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, printinfo, GOSSIP_SENDER_MAIN, 4, "", 0, true);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "KAVABANGAAAA!!!", GOSSIP_SENDER_MAIN, 1);
        player->SEND_GOSSIP_MENU(16777215, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)  override
    {
        player->CLOSE_GOSSIP_MENU();

        uint32 j = 1;
        while(j < m_count)
        {
            float rad = frand(5.0f, m_radius);
            Position pos(creature->GetPosition());
            creature->MovePosition(pos, rad, static_cast<float>(rand_norm()) * static_cast<float>(2 * M_PI));

            creature->GetMap()->LoadGrid(pos.GetPositionX(), pos.GetPositionY());
            for (uint8 i = 0; i < m_grouped; ++i)
            {
                creature->AddDelayedEvent(++j * 5, [creature, pos, this, i]() -> void
                {

                    if (Creature* cre = creature->SummonCreature(i % 2 ? 230018 : 230019, pos.GetPositionX() + frand(-2.0f, 2.0f), pos.GetPositionY() + frand(-2.0f, 2.0f), pos.GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, m_despawn))
                    {
                        cre->setActive(true);
                        cre->SetControlled(true, UNIT_STATE_ROOT);
                    }
                });
            }
        }

        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 /*sender*/, uint32 action, const char* code) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case 1:
            m_radius = atol(code);
            break;
        case 2:
            m_count = atol(code);
            break;
        case 3:
            m_despawn = atol(code);
            break;
        case 4:
            m_grouped = atol(code);
            break;

        }
        return OnGossipHello(player, creature);
    }

private:
    uint32 m_radius = 100;
    uint32 m_count = 100;
    uint32 m_despawn = 20000;
    uint32 m_grouped = 2;

};

// 100997
struct npc_failure_detection_pylon : public ScriptedAI
{
    npc_failure_detection_pylon(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint32 ResTimer = 6000;
    bool discharged = false;

    bool ReadyToRessurect()
    {
        if (auto instance = me->GetInstanceScript())
        {
            if (!instance)
                return false;

            if (instance->IsEncounterInProgress())
                return false;
            else
                return true;
        }

        if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
            return false;

        if (auto owner = me->GetAnyOwner())
        {
            if (owner->isInCombat())
                return false;
            else
                return true;
        }
        return false;
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == 199118)
        {
            discharged = true;
            me->CastSpell(me, 199923);
        }
        if (spell->Id == 199923)
        {
            me->AddDelayedEvent(2500, [this] {
                me->DespawnOrUnsummon(500);
            });
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (ResTimer <= diff)
        {
            if (!discharged)
            {
                if (ReadyToRessurect())
                    me->CastSpell(me, 199117);
            }
        }
        else
            ResTimer -= diff;
    }
};

// 114281
struct npc_flightmasters_whistle_mount : public ScriptedAI
{
    npc_flightmasters_whistle_mount(Creature* creature) : ScriptedAI(creature) 
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        ObjectGuid summonerGUID = summoner->GetGUID();
        me->AddDelayedEvent(1500, [this, summonerGUID]() -> void
        {
            if(Unit* unit = ObjectAccessor::GetUnit(*me, summonerGUID))
                unit->CastSpell(me, 46598, true);
        });
    }

    void PassengerBoarded(Unit* passenger, int8 /*seat*/, bool apply) override
    {
        auto player = passenger->ToPlayer();

        if (!apply || !player)
            return;

        Position pos;
        me->GetNearPosition(pos, 45.0f, 20.0f);
        me->GetMotionMaster()->MovePoint(0, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 5);

        ObjectGuid playerGUID = player->GetGUID();
        me->AddDelayedEvent(3500, [this, playerGUID]() -> void
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, playerGUID))
            {
                player->CastSpell(me, 89092);

                if (auto node = sObjectMgr->GetNearestTaxiNode(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player))
                {
                    TaxiNodesEntry const* to = sTaxiNodesStore.LookupEntry(node);
                    if (!to)
                        return;

                    me->AddDelayedEvent(1500, [playerGUID, to, this]() -> void
                    {
                        if (Player* player = ObjectAccessor::GetPlayer(*me, playerGUID))
                            player->NearTeleportTo(to->Pos.X, to->Pos.Y, to->Pos.Z, 0.0f);
                        me->DespawnOrUnsummon();
                    });
                }
            }
        });
    }

};

// 52552
struct npc_molten_behemoth : public ScriptedAI
{
    npc_molten_behemoth(Creature* creature) : ScriptedAI(creature) {}

    uint32 stonethrow = urand(8000,12000);
    uint32 stomp = urand(15000,18000);
    uint32 checklist = 1000;
    bool notSpellHited = true;

    void JustDied(Unit* killer) override
    {
        if (notSpellHited)
            if (auto player = killer->ToPlayer())
                player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 101167);
    }

    void Reset()
    {
        stomp = urand(15000, 18000);
        stonethrow = urand(8000, 12000);
        checklist = 1000;
        notSpellHited = true;
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == 97243 || spell->Id == 97246)
            notSpellHited = false;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (checklist <= diff)
        {
            if (me->getThreatManager().getThreatList().size() > 1)
                notSpellHited = false;

            checklist = 1000;
        }
        else checklist -= diff;

        if (stomp <= diff)
        {
            DoCast(97243);

            stomp = urand(12000, 15000);
        }
        else stomp -= diff;

        if (stonethrow <= diff)
        {
            if (Unit* player = SelectTarget(SELECT_TARGET_RANDOM, 0))
                DoCast(player, 97247);

            stonethrow = urand(18000, 23000);
        }
        else stonethrow -= diff;

        DoMeleeAttackIfReady();
    }
};

// 53085
struct npc_flamewaker_sentinel : public ScriptedAI
{
    npc_flamewaker_sentinel(Creature* creature) : ScriptedAI(creature) {}

    uint32 slam = urand(8000, 12000);
    uint32 takePlayer = urand(15000, 18000);
    uint32 checklist = 1000;
    bool passengerIn = false;

    void JustDied(Unit* killer) override
    {
        if (passengerIn)
            if (auto player = killer->ToPlayer())
                player->UpdateAchievementCriteria(CRITERIA_TYPE_KILL_CREATURE, 53085);
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == 1)
            return passengerIn;

        return 0;
    }

    void Reset()
    {
        slam = urand(8000, 12000);
        takePlayer = urand(15000, 18000);
        checklist = 1000;
        passengerIn = false;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (checklist <= diff)
        {
            if (me->GetVehicleKit()->GetPassenger(0))
                passengerIn = true;

            checklist = 1000;
        }
        else checklist -= diff;

        if (slam <= diff)
        {
            DoCast(97243);

            slam = urand(12000, 15000);
        }
        else slam -= diff;

        if (takePlayer <= diff)
        {
            if (Unit* player = SelectTarget(SELECT_TARGET_RANDOM, 0))
                player->CastSpell(me, 46598, true);

            takePlayer = urand(18000, 23000);
        }
        else takePlayer -= diff;

        DoMeleeAttackIfReady();
    }
};

// 53085
struct npc_flamewaker_shaman : public ScriptedAI
{
    npc_flamewaker_shaman(Creature* creature) : ScriptedAI(creature) {}

    uint32 flamewave = urand(8000, 12000);
    uint8 damaged = 0;

    void JustDied(Unit* killer) override
    {
        if (damaged >= 3)
            if (auto player = killer->ToPlayer())
                player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 100992);
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (spell->Id == 98185)
            ++damaged;
    }

    void Reset()
    {
        flamewave = urand(8000, 12000);
        damaged = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (flamewave <= diff)
        {
            DoCast(98206);

            flamewave = urand(12000, 15000);
        }
        else flamewave -= diff;

        DoMeleeAttackIfReady();
    }
};

struct npc_psyfiend_pvp : public AnyPetAI
{
    npc_psyfiend_pvp(Creature* creature) : AnyPetAI(creature) {}

    void InitializeAI() override
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(199824))
        {
            me->AddUnitTypeMask(UNIT_MASK_TOTEM);
            me->SetMaxHealth(spellInfo->Effects[EFFECT_0]->BasePoints);
            me->CastSpell(me, 34429, true); // visual
            me->SetReactState(ReactStates::REACT_AGGRESSIVE);
            me->AddUnitState(UnitState::UNIT_STATE_ROOT);
            me->CastSpell(me, 199845, true); // visual
        }
    }
};

// 70197 70199
enum Ids
{
    spell_ritual = 138054,
    stone_lighting = 70197,
    stone_primal = 70199,

    npc_collosus = 69347,
    npc_akilamon = 70080,
    npc_cera = 69396,
    npc_mogu_guard = 69767,
    npc_qinor = 69749,
    npc_jule = 69339,
    npc_kordok = 69633,
    npc_wl_teng = 69471,
    npc_kros = 69341,
};

Position const sumPos[9] =
{
    { 7582.f, 5600.72f, 33.37f, 3.57f },
{ 7087.83f, 4791.36f, 9.0f, 4.3f },
{ 5992.25f, 5280.99f, 7.2f, 4.1f },

{ 6838.29f, 5441.10f, 29.57f, 2.33f },
{ 7440.25f, 5677.70f, 49.62f, 4.63f },
{ 6473.03f, 5808.09f, 28.52f, 5.52f },

{ 6522.24f, 6372.65f, 8.12f, 0.06f },
{ 6382.93f, 6179.49f, -4.29f, 1.61f },
{ 5740.23f, 5370.30f, 3.49f, 1.64f },
};

struct npc_iot_ritual_stone : public ScriptedAI
{
    npc_iot_ritual_stone(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (spell->Id == spell_ritual)
        {
            switch (me->GetEntry())
            {
            case stone_lighting:
                if (me->GetPositionX() == 6851.24f && me->GetPositionY() == 5457.84f)
                    me->SummonCreature(npc_mogu_guard, sumPos[3]);
                if (me->GetPositionX() == 7431.16f && me->GetPositionY() == 5672.9f)
                    me->SummonCreature(npc_qinor, sumPos[4]);
                if (me->GetPositionX() == 6465.76f && me->GetPositionY() == 5815.12f)
                    me->SummonCreature(npc_jule, sumPos[5]);
                if (me->GetPositionX() == 6533.31f && me->GetPositionY() == 6384.15f)
                    me->SummonCreature(npc_kordok, sumPos[6]);
                if (me->GetPositionX() == 6377.94f && me->GetPositionY() == 6180.04f)
                    me->SummonCreature(npc_wl_teng, sumPos[7]);
                if (me->GetPositionX() == 5723.88f && me->GetPositionY() == 5383.63f)
                    me->SummonCreature(npc_kros, sumPos[8]);
                break;
            case stone_primal:
                if (me->GetPositionX() == 7577.93f && me->GetPositionY() == 5588.54f)
                    me->SummonCreature(npc_collosus, sumPos[0]);
                if (me->GetPositionX() == 7080.54f && me->GetPositionY() == 4795.18f)
                    me->SummonCreature(npc_akilamon, sumPos[1]);
                if (me->GetPositionX() == 5964.52f && (me->GetPositionY() <= 5262.f && me->GetPositionY() >= 5261.f))
                    me->SummonCreature(npc_cera, sumPos[2]);
                break;
            }
        }
    }
};

// 69234
struct npc_iot_roach : public ScriptedAI
{
    npc_iot_roach(Creature* creature) : ScriptedAI(creature) {}

    uint16 timer = 1000;

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (auto player = me->FindNearestPlayer(0.5f, true))
            {
                uint8 level = player->getLevel();
                float bp = 80 * level;
                player->CastCustomSpell(me, 136535, &bp, 0, 0, true);
            }

            timer = 1000;
        }
        else timer -= diff;
    }
};

// 69292 69186
enum npc_iot_defence
{
    crystal_a = 69186,
    crystal_h = 69292,
    energize_a = 136436,
    energize_h = 136611,
    shock = 136428,
};

struct npc_iot_defence_crystal : public ScriptedAI
{
    npc_iot_defence_crystal(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint16 timer = 1500;

    void KilledUnit(Unit* unit)
    {
        std::list<Player*> playerList;
        me->GetPlayerListInGrid(playerList, 8.0f);
        if (!playerList.empty())
        {
            for (Player* itr : playerList)
            {
                if (me->GetEntry() == crystal_a)
                {
                    if (itr->GetTeam() == ALLIANCE)
                        itr->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 139326);
                }
                if (me->GetEntry() == crystal_h)
                {
                    if (itr->GetTeam() == HORDE)
                        itr->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 139326);
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->HasAura(energize_a) || me->HasAura(energize_h))
        {
            if (timer <= diff)
            {
                DoCast(shock);
                timer = 1500;
            }
            else timer -= diff;
        }
    }
};

// 111882 105743 111861 113204 105742 105744 111864
enum npc_acquire
{
    sparky          = 111882,
    whipsnap        = 105743,
    slinky          = 111861,
    scaly           = 113204,
    turbax          = 105742,
    blaze           = 105744,
    sticky          = 111864,

    gotta_ketchun   = 222801,

    sparky_credit   = 222823,
    whipsnap_credit = 222845,
    slinky_credit   = 222847,
    scaly_credit    = 225326,
    turbax_credit   = 222832,
    blaze_credit    = 222846,
    sticky_credit   = 222848,
};

struct npc_acquire_achievement_nl : public ScriptedAI
{
    npc_acquire_achievement_nl(Creature* creature) : ScriptedAI(creature) {}

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (spell->Id == gotta_ketchun)
        {
            switch (me->GetEntry())
            {
            case sparky:
                DoCast(me, sparky_credit, true);
                me->Kill(me);
                break;
            case whipsnap:
                DoCast(me, whipsnap_credit, true);
                me->Kill(me);
                break;
            case slinky:
                DoCast(me, slinky_credit, true);
                me->Kill(me);
                break;
            case scaly:
                DoCast(me, scaly_credit, true);
                me->Kill(me);
                break;
            case turbax:
                DoCast(me, turbax_credit, true);
                me->Kill(me);
                break;
            case blaze:
                DoCast(me, blaze_credit, true);
                me->Kill(me);
                break;
            case sticky:
                DoCast(me, sticky_credit, true);
                me->Kill(me);
                break;
            }
        }
    }
};

struct npc_aria_sorrowheart : public ScriptedAI
{
    npc_aria_sorrowheart(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        events.Reset();
    };

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    };

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(0);

        events.ScheduleEvent(EVENT_1, 6000);
        events.ScheduleEvent(EVENT_2, 5000);
    };

    void JustDied(Unit* /*killer*/) override
    {
        Talk(1);

        for (auto const& _guid : *me->GetSaveThreatList())
            if (Player* player = ObjectAccessor::GetPlayer(*me, _guid))
                player->CastSpell(player, 218593, true);
    };

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(218660);
                events.ScheduleEvent(EVENT_1, 17000);
                break;
            case EVENT_2:
                if (auto victim = me->getVictim())
                    DoCast(victim, 218599, true);
                events.ScheduleEvent(EVENT_2, 7000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//99555
struct npc_moonfeather_statue : public ScriptedAI
{
    npc_moonfeather_statue(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        events.Reset();
        events.ScheduleEvent(EVENT_1, 13749);
    };

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(195776);
                events.ScheduleEvent(EVENT_1, 12000);
                break;
            }
        }
    }
};

// 100868
struct npc_chi_ji : public ScriptedAI
{
    npc_chi_ji(Creature* creature) : ScriptedAI(creature) {}

    int32 delay = 1500;

    void UpdateAI(uint32 diff) override
    {
        delay -= diff;
        if (delay <= 0)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING) || me->isMoving())
                return;

            me->CastSpell(me, 198764, false);
            delay += 1500;
        }
    }
};

//110441
struct npc_snowglobe_stalker : public ScriptedAI
{
    npc_snowglobe_stalker(Creature* creature) : ScriptedAI(creature)
    {
        Position pos;
        pos.Relocate(float(me->GetPositionX()), float(me->GetPositionY()), float(me->GetPositionZ()), float(me->GetOrientation()));

        auto areaTrigger = new AreaTrigger;
        if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), 0, me, nullptr, pos, pos, nullptr, ObjectGuid::Empty, 12198))
            delete areaTrigger;

        switch (me->GetMapId())
        {
        case 1220:
            DoCast(253969);
            DoCast(253973);
            break;
        case 0:
            DoCast(246235);
            DoCast(219624);
            areaTrigger->SetSphereScale(0.8f, 100, false);
            break;
        case 1:
            DoCast(246235);
            DoCast(246236);
            areaTrigger->SetSphereScale(0.8f, 100, false);
            break;
        }
    }
};

// 950121
struct npc_areatrigger_debugger : public ScriptedAI
{
    npc_areatrigger_debugger(Creature* creature) : ScriptedAI(creature) {}

    ObjectGuid tempGuid{};

    uint32 timer = 50;

    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        tempGuid = guid;
    }

    void UpdateAI(uint32 diff) override
    {
        if (tempGuid.IsEmpty())
            return;

        if (timer <= diff)
        {
            if (auto at = ObjectAccessor::GetAreaTrigger(*me, tempGuid))
                me->GetMotionMaster()->MovePoint(0, at->GetPosition());
            timer = 50;
        }
        else
            timer -= diff;
        
    }
};

// 98942
struct npc_future_you : public ScriptedAI
{
    npc_future_you(Creature* creature) : ScriptedAI(creature) {}

    uint32 despawn_timer = 0;

    void IsSummonedBy(Unit* owner) override
    {
        owner->AddPlayerInPersonnalVisibilityList(owner->GetGUID());
        Talk(1, owner->GetGUID());
        owner->CastSpell(owner, 194577, false);
        despawn_timer = 119000;
    }

    void ReceiveEmote(Player* player, uint32 textEmote) override
    {
        switch (textEmote)
        {
        case TEXT_EMOTE_GO:
            Talk(2, player->GetGUID());
            break;
        case TEXT_EMOTE_POINT:
            Talk(3, player->GetGUID());
            break;
        case TEXT_EMOTE_RUDE:
            Talk(4, player->GetGUID());
            break;
        case TEXT_EMOTE_DANCE:
            me->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
            break;
        case TEXT_EMOTE_JOKE:
            me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
            break;
        case TEXT_EMOTE_CHEER:
            me->HandleEmoteCommand(EMOTE_ONESHOT_SHY);
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (despawn_timer)
        {
            if (despawn_timer <= diff)
            {
                if (auto owner = me->GetAnyOwner())
                    Talk(0, owner->GetGUID());

                despawn_timer = 0;
            }
            else
                despawn_timer -= diff;
        }
    }
};

// 123794, 123793
struct npc_hearthstation : public ScriptedAI
{
    npc_hearthstation(Creature* creature) : ScriptedAI(creature) {}

    ObjectGuid targetGuid;
    std::string emote;
    uint32 despawn_timer;

    void IsSummonedBy(Unit* owner) override
    {
        me->AddDelayedEvent(100, [=]() -> void
        {
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

            DoCast(owner, 46598, true);
        });
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            me->SetAnimKitId(0);
            //me->PlayOneShotAnimKit();
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            break;
        case ACTION_2:
            me->SetAnimKitId(0);
            //me->PlayOneShotAnimKit();
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            break;
        case ACTION_3:
            auto entry = sBroadcastTextStore.LookupEntry(132809);
            if (!entry)
                return;

            if (auto owner = me->GetAnyOwner())
            {
                emote = DB2Manager::GetBroadcastTextValue(entry, owner->ToPlayer()->GetSession()->GetSessionDbLocaleIndex());
                owner->ToPlayer()->TextEmote(emote);
                me->SetAnimKitId(13441);
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }
            break;
        }
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 246347 || spell->Id == 246346)
        {
            if (auto owner = me->GetAnyOwner())
            {
                me->AddDelayedEvent(1500, [owner]() -> void {
                    owner->RemoveAurasDueToSpell(246289);
                    owner->RemoveAurasDueToSpell(246351);
                });
            }
        }
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (spell->Id == 246308)
        {
            targetGuid = caster->GetGUID();

            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

            switch (urand(0, 1))
            {
            case 0:
                me->CastSpellDelay(me, 246346, false, 1500);
                me->PlayOneShotAnimKit(13443);
                break;
            case 1:
                if (auto target = ObjectAccessor::GetPlayer(*me, targetGuid))
                    me->CastSpellDelay(target, 246347, false, 1500);

                me->PlayOneShotAnimKit(13444);
                break;
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (despawn_timer)
        {
            if (despawn_timer <= diff)
            {
                if (!me->GetAnyOwner())
                    me->DespawnOrUnsummon();

                despawn_timer = 1000;
            }
            else
                despawn_timer -= diff;
        }
    }
};

//40246
struct npc_instant_statue_pedestal : public ScriptedAI
{
    npc_instant_statue_pedestal(Creature* creature) : ScriptedAI(creature) {}

    void IsSummonedBy(Unit* owner) override
    {
        DoCast(owner, 75731, true);
        me->AddDelayedEvent(500, [=]() -> void { me->AddAura(52844, me); });
    }
};

//56194
struct npc_infant_spider : public ScriptedAI
{
    npc_infant_spider(Creature* creature) : ScriptedAI(creature) {}

    uint16 check = 0;

    void IsSummonedBy(Unit* owner) override
    {
        check = 200;
        me->AddDelayedEvent(100, [=]() -> void { DoCast(owner, 46598, true); });
    }

    void UpdateAI(uint32 diff) override
    {
        if (check)
        {
            if (check <= diff)
            {
                if (!me->HasAura(46598))
                    if (auto owner = me->GetAnyOwner())
                        DoCast(owner, 46598, true);

                check = 200;
            }
            else
                check -= diff;
        }
    }
};

enum TrainWrecker
{
    GO_TOY_TRAIN = 193963,
    SPELL_TOY_TRAIN_PULSE = 61551,
    SPELL_WRECK_TRAIN = 62943,
    ACTION_WRECKED = 1,
    EVENT_DO_JUMP = 1,
    EVENT_DO_FACING = 2,
    EVENT_DO_WRECK = 3,
    EVENT_DO_DANCE = 4,
    MOVEID_CHASE = 1,
    MOVEID_JUMP = 2,

    NPC_EXULTING_WIND_UP_TRAIN_WRECKER = 81071
};

struct npc_train_wrecker : public ScriptedAI
{
    npc_train_wrecker(Creature* creature) : ScriptedAI(creature), _isSearching(true), _nextAction(0), _timer(1 * IN_MILLISECONDS) {}

    bool _isSearching;
    uint8 _nextAction;
    uint32 _timer;
    ObjectGuid _target;

    GameObject* VerifyTarget() const
    {
        if (GameObject* target = ObjectAccessor::GetGameObject(*me, _target))
            return target;

        me->HandleEmoteCommand(EMOTE_ONESHOT_RUDE);
        me->DespawnOrUnsummon(3 * IN_MILLISECONDS);
        return nullptr;
    }

    void UpdateAI(uint32 diff) override
    {
        if (_isSearching)
        {
            if (diff < _timer)
                _timer -= diff;
            else
            {
                if (GameObject* target = me->FindNearestGameObject(GO_TOY_TRAIN, 15.0f))
                {
                    _isSearching = false;
                    _target = target->GetGUID();
                    me->SetWalk(true);

                    Position pos;
                    target->GetNearPosition(pos, 0.0f, target->GetAngle(me));

                    me->GetMotionMaster()->MovePoint(MOVEID_CHASE, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                }
                else
                    _timer = 3 * IN_MILLISECONDS;
            }
        }
        else
        {
            switch (_nextAction)
            {
            case EVENT_DO_JUMP:
                if (GameObject* target = VerifyTarget())
                    me->GetMotionMaster()->MoveJump(*target, 5.0, 10.0, MOVEID_JUMP);
                _nextAction = 0;
                break;
            case EVENT_DO_FACING:
                if (GameObject* target = VerifyTarget())
                {
                    me->SetFacingTo(target->GetOrientation());
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    _timer = 1.5 * IN_MILLISECONDS;
                    _nextAction = EVENT_DO_WRECK;
                }
                else
                    _nextAction = 0;
                break;
            case EVENT_DO_WRECK:
                if (diff < _timer)
                {
                    _timer -= diff;
                    break;
                }
                if (GameObject* target = VerifyTarget())
                {
                    me->CastSpell(target, SPELL_WRECK_TRAIN, false);
                    target->Delete();
                    _timer = 2 * IN_MILLISECONDS;
                    _nextAction = EVENT_DO_DANCE;
                }
                else
                    _nextAction = 0;
                break;
            case EVENT_DO_DANCE:
                if (diff < _timer)
                {
                    _timer -= diff;
                    break;
                }
                me->UpdateEntry(NPC_EXULTING_WIND_UP_TRAIN_WRECKER);
                me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                me->DespawnOrUnsummon(5 * IN_MILLISECONDS);
                _nextAction = 0;
                break;
            default:
                break;
            }
        }
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        if (id == MOVEID_CHASE)
            _nextAction = EVENT_DO_JUMP;
        else if (id == MOVEID_JUMP)
            _nextAction = EVENT_DO_FACING;
    }
};

void AddSC_npcs_special()
{
    new npc_storm_earth_and_fire();
    new npc_air_force_bots();
    new npc_lunaclaw_spirit();
    new npc_chicken_cluck();
    new npc_dancing_flames();
    new npc_doctor();
    new npc_injured_patient();
    new npc_garments_of_quests();
    new npc_guardian();
    new npc_mount_vendor();
    new npc_rogue_trainer();
    new npc_sayge();
    new npc_steam_tonk();
    new npc_tonk_mine();
    new npc_winter_reveler();
    new npc_brewfest_reveler();
    new npc_snake_trap();
    new npc_ebon_gargoyle();
    new npc_mage_mirror_image();
    new npc_lightwell();
    new npc_lightwell_mop();
    new mob_mojo();
    new npc_training_dummy();
    new npc_wormhole();
    new npc_pet_trainer();
    new npc_locksmith();
    new npc_experience();
    new npc_firework();
    new npc_spring_rabbit();
    new npc_generic_harpoon_cannon();
    new npc_spirit_link_totem();
    new npc_frozen_orb();
    new npc_guardian_of_ancient_kings();
    new npc_demonic_gateway();
    new npc_dire_beast();
    new npc_wild_imp();
    new npc_windwalk_totem();
    new npc_ring_of_frost();
    new npc_wild_mushroom();
    new npc_fungal_growth();
    new npc_brewfest_trigger;
    new npc_brewfest_apple_trigger;
    new npc_brewfest_keg_thrower;
    new npc_brewfest_keg_receiver;
    new npc_brewfest_ram_master;
    new npc_spectral_guise();
    new npc_past_self();
    new npc_guild_battle_standard();
    new npc_riggle_bassbait();
    new npc_hyjal_soft_target();
    new npc_track_switch();
    new npc_king_mrgl();
    new npc_zombie_explosion();
    new npc_monk_wind_spirit();
    new npc_slg_generic_mop_large_aoi();
    new npc_pal_echo_of_the_highlord();
    new npc_pal_echo_of_the_highlord1();
    new npc_nightmare_hitching_post();
    new npc_fel_guard();
    new npc_grimoire_imp();
    new npc_grimoire_succubus();
    new npc_grimoire_felhunter();
    new npc_grimoire_voidwalker();
    new npc_azsuna_allari_q37660();
    new npc_azsuna_stellagosa_q37862();
    new npc_revil_kost_following_the_curse();
    new npc_revil_kost_dark_rider();
    new npc_dalaran_defender_barrem();
    new npc_azsuna_daglop_q38237();
    new npc_grasp_of_underking_quest();
    new npc_crash_test();
    RegisterCreatureAI(npc_azsuna_illidari_outpost_initiator);
    RegisterCreatureAI(npc_failure_detection_pylon);
    RegisterCreatureAI(npc_mirror_image);
    RegisterCreatureAI(npc_flightmasters_whistle_mount);
    RegisterCreatureAI(npc_molten_behemoth);
    RegisterCreatureAI(npc_flamewaker_sentinel);
    RegisterCreatureAI(npc_flamewaker_shaman);
    RegisterCreatureAI(npc_psyfiend_pvp);
    RegisterCreatureAI(npc_iot_ritual_stone);
    RegisterCreatureAI(npc_iot_roach);
    RegisterCreatureAI(npc_iot_defence_crystal);
    RegisterCreatureAI(npc_acquire_achievement_nl);
    RegisterCreatureAI(npc_shadowy_reflection);
    RegisterCreatureAI(npc_aria_sorrowheart);
    RegisterCreatureAI(npc_moonfeather_statue);
    RegisterCreatureAI(npc_chi_ji);
    RegisterCreatureAI(npc_snowglobe_stalker);
    RegisterCreatureAI(npc_areatrigger_debugger);
    RegisterCreatureAI(npc_anatomical_dummy);
    RegisterCreatureAI(npc_future_you);
    RegisterCreatureAI(npc_hearthstation);
    RegisterCreatureAI(npc_instant_statue_pedestal);
    RegisterCreatureAI(npc_train_wrecker);
    RegisterCreatureAI(npc_infant_spider);
}
