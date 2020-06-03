/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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
SDName: Hodir
SDAuthor: PrinceCreed
SD%Complete: 100
SDComments:
EndScriptData */

#include "ulduar.h"
#include "ObjectVisitors.hpp"

enum Spells
{
    // Hodir
    SPELL_FROZEN_BLOWS                          = 62478,
    SPELL_FLASH_FREEZE                          = 61968,
    SPELL_FLASH_FREEZE_VISUAL                   = 62148,
    SPELL_BITING_COLD                           = 62038,
    SPELL_BITING_COLD_TRIGGERED                 = 62039,
    SPELL_FREEZE                                = 62469,
    SPELL_ICICLE                                = 62234,
    SPELL_ICICLE_SNOWDRIFT                      = 62462,
    SPELL_BLOCK_OF_ICE                          = 61969,
    SPELL_BLOCK_OF_ICE_NPC                      = 61990,
    SPELL_FROZEN_KILL                           = 62226,
    SPELL_ICICLE_FALL                           = 69428,
    SPELL_FALL_DAMAGE                           = 62236,
    SPELL_FALL_SNOWDRIFT                        = 62460,
    SPELL_BERSERK                               = 47008,
    
    // Druids
    SPELL_WRATH                                 = 62793,
    SPELL_STARLIGHT                             = 62807,
    // Shamans
    SPELL_LAVA_BURST                            = 61924,
    SPELL_STORM_POWER                           = 63711,
    // Mages
    SPELL_FIREBALL                              = 61909,
    SPELL_CONJURE_FIRE                          = 62823,
    SPELL_MELT_ICE                              = 64528,
    SPELL_SINGED                                = 62821,
    // Priests
    SPELL_SMITE                                 = 61923,
    SPELL_GREATER_HEAL                          = 62809,
    SPELL_DISPEL_MAGIC                          = 63499
};

// Achievements
#define ACHIEVEMENT_STAYING_BUFFED_ALL_WINTER   RAID_MODE(63711, 62821)
#define ACHIEVEMENT_CHEESE_THE_FREEZE           RAID_MODE(2961, 2962)
#define ACHIEVEMENT_COLD_IN_HERE                RAID_MODE(2967, 2968)
#define ACHIEVEMENT_THIS_CACHE_WAS_RARE         RAID_MODE(3182, 3184)
#define ACHIEVEMENT_COOLEST_FRIENDS             RAID_MODE(2963, 2965)

//Spells
#define SPELL_STORM_CLOUD_HELPER                RAID_MODE(65123, 65133)
#define SPELL_STORM_POWER_HELPER                RAID_MODE(63711, 65134)

#define ACTION_FAILED_COOLEST_FRIENDS           1

enum NPCs
{
    NPC_FLASH_FREEZE_PRE                        = 32926,
    NPC_FLASH_FREEZE                            = 32938,
    NPC_ICICLE_TARGET                           = 33174,
    NPC_HODIR                                   = 32845
};

enum Events
{
    EVENT_NONE,
    EVENT_FREEZE,
    EVENT_FLASH_CAST,
    EVENT_FLASH_EFFECT,
    EVENT_ICICLE,
    EVENT_BLOWS,
    EVENT_RARE_CACHE,
    EVENT_BERSERK
};

enum Yells
{
    SAY_AGGRO                                   = -1603210,
    SAY_SLAY_1                                  = -1603211,
    SAY_SLAY_2                                  = -1603212,
    SAY_FLASH_FREEZE                            = -1603213,
    SAY_STALACTITE                              = -1603214,
    SAY_DEATH                                   = -1603215,
    SAY_BERSERK                                 = -1603216,
    SAY_YS_HELP                                 = -1603217,
    SAY_HARD_MODE_MISSED                        = -1603218
};

#define EMOTE_FREEZE                            "Hodir begins to cast Flash Freeze!"
#define EMOTE_BLOWS                             "Hodir gains Frozen Blows!"

enum HodirChests
{
    CACHE_OF_WINTER_10                          = 194307,
    CACHE_OF_WINTER_25                          = 194308
};

struct SummonLocation
{
    float x,y,z,o;
    uint32 entry;
};

SummonLocation addLocations[]=
{
    {1983.75f, -243.36f, 432.767f, 1.57f, 32897}, // Priest 1
    {1999.90f, -230.49f, 432.767f, 1.57f, 33325}, // Druid 1
    {2010.06f, -243.45f, 432.767f, 1.57f, 33328}, // Shaman 1
    {2021.12f, -236.65f, 432.767f, 1.57f, 32893}, // Mage 1
    {2028.10f, -244.66f, 432.767f, 1.57f, 33326}, // Priest 2
    {2014.18f, -232.80f, 432.767f, 1.57f, 32901}, // Druid 2
    {1992.90f, -237.54f, 432.767f, 1.57f, 32900}, // Shaman 2
    {1976.60f, -233.53f, 432.767f, 1.57f, 33327}  // Mage 2
};
#define NORMAL_COUNT 4
#define RAID_COUNT 8


class boss_hodir : public CreatureScript
{
public:
    boss_hodir() : CreatureScript("boss_hodir") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_hodirAI(pCreature);
    }

    struct boss_hodirAI : public BossAI
    {
        boss_hodirAI(Creature *pCreature) : BossAI(pCreature, BOSS_HODIR)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, 65280, true);  // Singed
        }

        uint32 uiCheckIntenseColdTimer;
        bool bMoreThanTwoIntenseCold;
        bool CheeseTheFreeze;
        bool CoolestFriends;
        bool RareCache;

        void Reset() override
        {
            _Reset();
            me->SetReactState(REACT_PASSIVE);
        
            // Spawn NPC Helpers
            for (uint8 i = 0; i < RAID_MODE(NORMAL_COUNT, RAID_COUNT); i++)
            {
                if (Creature* pHelper = me->SummonCreature(addLocations[i].entry,addLocations[i].x,addLocations[i].y,addLocations[i].z,addLocations[i].o))
                    if (Creature *pIceBlock = pHelper->SummonCreature(NPC_FLASH_FREEZE_PRE,addLocations[i].x,addLocations[i].y,addLocations[i].z,addLocations[i].o))
                        pHelper->AddThreat(me, 5000000.0f);
            }
        }
        
        void KilledUnit(Unit * who) override
        {
            if (!(rand()%5))
                DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
        }
        
        void JustDied(Unit * /*victim*/) override
        {
            _JustDied();
            DoScriptText(SAY_DEATH, me);
        
            me->setFaction(35);
        
            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BITING_COLD_TRIGGERED);
                // Kill credit
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, 64899, 0, 0, me);
                // Getting Cold in Here
                if (RareCache)
                    instance->SetData(DATA_HODIR_RARE_CHEST, GO_STATE_READY);
                // Chest spawn
                me->SummonGameObject(RAID_MODE(CACHE_OF_WINTER_10, CACHE_OF_WINTER_25), 1966.43f, -203.906f, 432.687f, -0.91f, 0, 0, 1, 1, 604800);
            }
        }

        void EnterCombat(Unit* /*pWho*/) override
        {
            _EnterCombat();
            DoScriptText(SAY_AGGRO, me);
            me->SetReactState(REACT_AGGRESSIVE);
            DoCast(me, SPELL_BITING_COLD, true);
            events.ScheduleEvent(EVENT_ICICLE, 6000);
            events.ScheduleEvent(EVENT_FREEZE, 25000);
            events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
            events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
            events.ScheduleEvent(EVENT_RARE_CACHE, 180000);
            events.ScheduleEvent(EVENT_BERSERK, 480000);
            uiCheckIntenseColdTimer = 2000;
            bMoreThanTwoIntenseCold = true;
            CheeseTheFreeze = true;
            CoolestFriends = true;
            RareCache = true;
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            if (me->getVictim() && !me->getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
                me->Kill(me->getVictim());

            events.Update(diff);
        
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
             
            
            if (uiCheckIntenseColdTimer <= diff)
            {
                std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
                {
                    Unit *pTarget = Unit::GetUnit(*me, (*itr)->getUnitGuid());
                    if (!pTarget || pTarget->GetTypeId() != TYPEID_PLAYER)
                        continue;
                    
                    /*if (pTarget->GetAura(SPELL_STORM_POWER_HELPER) && pTarget->GetAura(62821) && pTarget->GetAura(62807))
                            pTarget->ToPlayer()->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, ACHIEVEMENT_STAYING_BUFFED_ALL_WINTER);*/
                    
                    if (bMoreThanTwoIntenseCold)
                    {
                        Aura *AuraIntenseCold = pTarget->GetAura(SPELL_BITING_COLD_TRIGGERED);
                        if (AuraIntenseCold && AuraIntenseCold->GetStackAmount() > 2)
                        {
                            me->MonsterTextEmote("Achievement: getting cold in here - Fail", ObjectGuid::Empty, true);
                            bMoreThanTwoIntenseCold = false;
                        }
                    }
                }
                uiCheckIntenseColdTimer = 2000;
            }
            else uiCheckIntenseColdTimer -= diff;
            

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_FREEZE:
                        DoCastAOE(SPELL_FREEZE);
                        events.CancelEvent(EVENT_FREEZE);
                        break;
                    case EVENT_ICICLE:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (pTarget->isAlive())
                                DoCast(pTarget, SPELL_ICICLE);
                        }
                        events.ScheduleEvent(EVENT_ICICLE,urand(6000, 8000));
                        break;
                    case EVENT_FLASH_CAST:
                        DoScriptText(SAY_FLASH_FREEZE, me);
                        me->MonsterTextEmote(EMOTE_FREEZE, ObjectGuid::Empty, true);
                        for (uint8 i = 0; i < RAID_MODE(2,3); ++i)
                        {
                            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                if (pTarget->isAlive())
                                    pTarget->CastSpell(pTarget, SPELL_ICICLE_SNOWDRIFT, true);
                        }
                        DoCast(SPELL_FLASH_FREEZE);
                        events.RescheduleEvent(EVENT_ICICLE, 15000);
                        events.ScheduleEvent(EVENT_FREEZE, urand(30000, 35000));
                        events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
                        events.ScheduleEvent(EVENT_FLASH_EFFECT, 9000);
                        break;
                    case EVENT_FLASH_EFFECT:
                        DoCast(SPELL_FLASH_FREEZE_VISUAL);
                        FlashFreeze();
                        events.CancelEvent(EVENT_FLASH_EFFECT);
                        break;
                    case EVENT_BLOWS:
                        DoScriptText(SAY_STALACTITE, me);
                        me->MonsterTextEmote(EMOTE_BLOWS, ObjectGuid::Empty, true);
                        DoCast(me, SPELL_FROZEN_BLOWS);
                        events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
                        break;
                    case EVENT_RARE_CACHE:
                        DoScriptText(SAY_HARD_MODE_MISSED, me);
                        RareCache = false;
                        events.CancelEvent(EVENT_RARE_CACHE);
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK, true);
                        DoScriptText(SAY_BERSERK, me);
                        events.CancelEvent(EVENT_BERSERK);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

        void FlashFreeze()
        {
            DoZoneInCombat();
            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                if (Unit *pTarget = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                {
                    if (pTarget->HasAura(SPELL_BLOCK_OF_ICE))
                    {
                        DoCast(pTarget, SPELL_FROZEN_KILL);
                        continue;
                    }
                    else
                    {
                        if (GetClosestCreatureWithEntry(pTarget, NPC_ICICLE_TARGET, 5.0f))
                            continue;
                        
                        else if (Creature *pIceBlock = pTarget->SummonCreature(NPC_FLASH_FREEZE,pTarget->GetPositionX(),pTarget->GetPositionY(),pTarget->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,105000))
                        {
                            if (pTarget->GetTypeId() == TYPEID_PLAYER)
                                if (CheeseTheFreeze)
                                    CheeseTheFreeze = false;
                        }
                    }
                }
            }
        }

        bool IsGettingCold()
        {
            return bMoreThanTwoIntenseCold;
        }

        bool GetRareCache()
        {
            return RareCache;
        }

        bool IsCheeseTheFreeze()
        {
            return CheeseTheFreeze;
        }

        bool isCoolestFriends()
        {
            return CoolestFriends;
        }
    
        void DoAction(const int32 action) override
        {
            switch(action)
            {
                case ACTION_FAILED_COOLEST_FRIENDS:
                    if (CoolestFriends)
                    {
                        me->MonsterTextEmote("Achievement:I have the coolest friends - Fail", ObjectGuid::Empty, true);
                        CoolestFriends = false;
                    }
                    break;
            }
        }
    };

};


class npc_icicle : public CreatureScript
{
public:
    npc_icicle() : CreatureScript("npc_icicle") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_icicleAI (pCreature);
    }

    struct npc_icicleAI : public Scripted_NoMovementAI
    {
        npc_icicleAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(28470);
        }

        uint32 IcicleTimer;

        void Reset() override
        {
            IcicleTimer = 2000;
        }

        void UpdateAI(uint32 diff) override
        {
            if (IcicleTimer <= diff)
            {
                DoCast(me, SPELL_FALL_DAMAGE);
                DoCast(me, SPELL_ICICLE_FALL);
                IcicleTimer = 10000;
            }
            else IcicleTimer -= diff;
        }
    };

};


class npc_icicle_snowdrift : public CreatureScript
{
public:
    npc_icicle_snowdrift() : CreatureScript("npc_icicle_snowdrift") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_icicle_snowdriftAI (pCreature);
    }

    struct npc_icicle_snowdriftAI : public Scripted_NoMovementAI
    {
        npc_icicle_snowdriftAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(28470);
        }

        uint32 IcicleTimer;

        void Reset() override
        {
            IcicleTimer = 2000;
        }

        void UpdateAI(uint32 diff) override
        {
            if (IcicleTimer <= diff)
            {
                DoCast(me, SPELL_FALL_SNOWDRIFT);
                DoCast(me, SPELL_ICICLE_FALL);
                IcicleTimer = 10000;
            }
            else IcicleTimer -= diff;
        }
    };

};


class npc_snowpacked_icicle : public CreatureScript
{
public:
    npc_snowpacked_icicle() : CreatureScript("npc_snowpacked_icicle") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_snowpacked_icicleAI (pCreature);
    }

    struct npc_snowpacked_icicleAI : public Scripted_NoMovementAI
    {
        npc_snowpacked_icicleAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(15880);
        }

        uint32 DespawnTimer;

        void Reset() override
        {
            DespawnTimer = 12000;
        }

        void UpdateAI(uint32 diff) override
        {
            if (DespawnTimer <= diff)
            {
                if (GameObject *pSnowdrift = me->FindNearestGameObject(194173, 2))
                    me->RemoveGameObject(pSnowdrift, true);
                me->DespawnOrUnsummon();
            }
            else DespawnTimer -= diff;
        }
    };

};


class npc_hodir_priest : public CreatureScript
{
public:
    npc_hodir_priest() : CreatureScript("npc_hodir_priest") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_hodir_priestAI (pCreature);
    }

    struct npc_hodir_priestAI : public ScriptedAI
    {
        npc_hodir_priestAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* instance;
        uint32 HealTimer;
        uint32 DispelTimer;

        void Reset() override
        {
            HealTimer = urand(4000, 8000);
            DispelTimer = urand(20000, 30000);
        }

        void AttackStart(Unit *who) override
        {
            AttackStartCaster(who, 30);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (instance)
                if (instance->GetBossState(BOSS_HODIR) == DONE)
                    me->DespawnOrUnsummon(1000);

            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                return;

            if (Creature* Hodir = me->FindNearestCreature(NPC_HODIR, 60, true))
                if (!Hodir->isInCombat())
                    return;

           if (HealthBelowPct(35))
                DoCastAOE(SPELL_GREATER_HEAL, true);

            if (HealTimer <= uiDiff)
            {
                DoCastAOE(SPELL_GREATER_HEAL, true);
                HealTimer = urand(12000, 14000);
            }
            else HealTimer -= uiDiff;
        
            if (DispelTimer < uiDiff)
            {
                std::list<Player*> players;
                Trinity::AnyPlayerInObjectRangeCheck checker(me, 30);
                Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                Trinity::VisitNearbyWorldObject(me, 30, searcher);
                if (!players.empty())
                {
                    for (std::list<Player*>::iterator iter = players.begin(); iter != players.end(); ++iter)
                    {
                        if ((*iter)->HasAura(SPELL_FREEZE))
                        {
                            DoCast((*iter),SPELL_DISPEL_MAGIC, true);
                            DispelTimer = urand(25000,30000);
                            return;
                        }
                    }
                }
                DispelTimer = 5000;
            }
            else DispelTimer -= uiDiff;

            DoSpellAttackIfReady(SPELL_SMITE);
        }

        void JustDied(Unit* /*victim*/) override
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };

};


class npc_hodir_shaman : public CreatureScript
{
public:
    npc_hodir_shaman() : CreatureScript("npc_hodir_shaman") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_hodir_shamanAI (pCreature);
    }

    struct npc_hodir_shamanAI : public ScriptedAI
    {
        npc_hodir_shamanAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* instance;
        uint32 StormTimer;
        uint32 LavaBurstTimer;

        void Reset() override
        {
            LavaBurstTimer = 4000;
            StormTimer = 3000;
        }

        void AttackStart(Unit *who) override
        {
            AttackStartCaster(who, 30);
        }

        void UpdateAI(uint32 uiDiff) override
        {
             if (instance)
                if (instance->GetBossState(BOSS_HODIR) == DONE)
                    me->DespawnOrUnsummon(1000);

            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                return;
            
            if (Creature* Hodir = me->FindNearestCreature(NPC_HODIR, 60, true))
                if (!Hodir->isInCombat())
                    return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (StormTimer <= uiDiff)
            {
                std::list<Player*> players;
                Trinity::AnyPlayerInObjectRangeCheck checker(me, 30);
                Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                Trinity::VisitNearbyWorldObject(me, 30, searcher);
                if (!players.empty())
                {
                    std::list<Player*>::iterator iter = players.begin(); 
                    DoCast((*iter), SPELL_STORM_CLOUD_HELPER, true);
                }
                StormTimer = urand(15000, 20000);
            }
            else StormTimer -= uiDiff;

            if (LavaBurstTimer <= uiDiff)
            {
                DoCastVictim(SPELL_LAVA_BURST, true);
                LavaBurstTimer = 4000;
            }
            else LavaBurstTimer -= uiDiff;
        }
        
        void JustDied(Unit* /*victim*/) override
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };

};

class npc_hodir_druid : public CreatureScript
{
public:
    npc_hodir_druid() : CreatureScript("npc_hodir_druid") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_hodir_druidAI (pCreature);
    }

    struct npc_hodir_druidAI : public ScriptedAI
    {
        npc_hodir_druidAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* instance;
        uint32 StarlightTimer;
        uint32 WrathTimer;

        void Reset() override
        {
            WrathTimer = 3000;
            StarlightTimer = 3000;
        }

        void AttackStart(Unit *who) override
        {
            AttackStartCaster(who, 30);
        }
        
        void UpdateAI(uint32 uiDiff) override
        {
             if (instance)
                if (instance->GetBossState(BOSS_HODIR) == DONE)
                    me->DespawnOrUnsummon(1000);

            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                return;
            
            if (Creature* Hodir = me->FindNearestCreature(NPC_HODIR, 60, true))
                if (!Hodir->isInCombat())
                    return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (WrathTimer <= uiDiff)
            {
                DoCastVictim(SPELL_WRATH, true);
                WrathTimer = 4000;
            } 
            else WrathTimer -= uiDiff;


            if (StarlightTimer <= uiDiff)
            {
                DoCast(me, SPELL_STARLIGHT, true);
                StarlightTimer = 25000;
            }
            else StarlightTimer -= uiDiff;
        }
        
        void JustDied(Unit* /*victim*/) override
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };

};


class npc_hodir_mage : public CreatureScript
{
public:
    npc_hodir_mage() : CreatureScript("npc_hodir_mage") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_hodir_mageAI (pCreature);
    }

    struct npc_hodir_mageAI : public ScriptedAI
    {
        npc_hodir_mageAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* instance;
        uint32 FireTimer;
        uint32 MeltIceTimer;
        uint32 FireBallTimer;

        void Reset() override
        {
            FireBallTimer = 4000;
            FireTimer = 3000;
            MeltIceTimer = 5000;
        }

        void AttackStart(Unit *who) override
        {
            AttackStartCaster(who, 30);
        }

        void UpdateAI(uint32 uiDiff) override
        {
             if (instance)
                if (instance->GetBossState(BOSS_HODIR) == DONE)
                    me->DespawnOrUnsummon(1000);

            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                return;
            
            if (Creature* Hodir = me->FindNearestCreature(NPC_HODIR, 60, true))
                if (!Hodir->isInCombat())
                    return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (FireTimer <= uiDiff)
            {
                DoCast(me, SPELL_CONJURE_FIRE, true);
                FireTimer = 30000;
            }
            else FireTimer -= uiDiff;

            if (FireBallTimer <= uiDiff)
            {
                DoCastVictim(SPELL_FIREBALL, true);
                FireBallTimer = 4000;
            }
            else FireBallTimer -= uiDiff;
        
            if (MeltIceTimer < uiDiff)
            {
                if (Creature *pShard = me->FindNearestCreature(NPC_FLASH_FREEZE, 50, true))
                {
                    DoCast(pShard, SPELL_MELT_ICE, true);
                    MeltIceTimer = 10000;
                }
                MeltIceTimer = 5000;
            }
            else MeltIceTimer -= uiDiff;
        }

        void JustDied(Unit* /*victim*/) override
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };

};


class npc_toasty_fire : public CreatureScript
{
public:
    npc_toasty_fire() : CreatureScript("npc_toasty_fire") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_toasty_fireAI (pCreature);
    }

    struct npc_toasty_fireAI : public Scripted_NoMovementAI
    {
        npc_toasty_fireAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(15880);
        }

        void Reset() override
        {
            DoCast(me, SPELL_SINGED, true);
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo *spell) override
        {
            // Toasty fire can be extinguished by falling ice or Flash Freeze
            if (spell->Id == SPELL_BLOCK_OF_ICE || spell->Id == 62457 || spell->Id == 65370)
            {
                if (GameObject *pFire = me->FindNearestGameObject(194300, 4))
                    me->RemoveGameObject(pFire, true);
                me->DespawnOrUnsummon();
            }
        }

        void JustDied(Unit * /*victim*/) override
        {
            if (GameObject *pFire = me->FindNearestGameObject(194300, 4))
                me->RemoveGameObject(pFire, true);
        }
    };

};


class npc_flash_freeze : public CreatureScript
{
public:
    npc_flash_freeze() : CreatureScript("npc_flash_freeze") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_flash_freezeAI (pCreature);
    }

    struct npc_flash_freezeAI : public Scripted_NoMovementAI
    {
        npc_flash_freezeAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED | UNIT_FLAG_PACIFIED);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->SetDisplayId(11686);
            me->setFaction(14);
        }

        uint32 checkTimer = 1000;

        void Reset() override
        {
            if (auto owner = me->GetAnyOwner())
            {
                if (me->GetEntry() == NPC_FLASH_FREEZE_PRE)
                    DoCast(SPELL_BLOCK_OF_ICE_NPC);
                else
                    DoCast(SPELL_BLOCK_OF_ICE);

                owner->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (checkTimer)
            {
                if (checkTimer <= diff)
                    checkTimer = 1000;
                else
                {
                    checkTimer -= diff;
                    return;
                }
            }

            auto owner = me->GetAnyOwner();
            if (!owner || !owner->isAlive())
            {
                me->DespawnOrUnsummon();
                return;
            }

            if (me->GetEntry() == NPC_FLASH_FREEZE_PRE && !owner->HasAura(SPELL_BLOCK_OF_ICE_NPC))
                DoCast(SPELL_BLOCK_OF_ICE_NPC);
        }

        void DamageTaken(Unit* pKiller, uint32 &damage, DamageEffectType dmgType) override
        {
            if (me->GetEntry() == NPC_FLASH_FREEZE_PRE)
            {
                if (pKiller && pKiller->IsPlayer())
                    if (auto pHodir = me->FindNearestCreature(NPC_HODIR, 50.0f, true))
                        if (!pHodir->isInCombat())
                            pHodir->AI()->DoZoneInCombat();
            }
        }

        void JustDied(Unit * /*victim*/) override
        {
            if (auto owner = me->GetAnyOwner())
                owner->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, false);
        }
    };

};

class  achievement_have_the_coolest_friends: public AchievementCriteriaScript
{
    public:
        achievement_have_the_coolest_friends() : AchievementCriteriaScript("achievement_have_the_coolest_friends")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
             
             if (Creature* Hr = target->ToCreature())
                 if (boss_hodir::boss_hodirAI * HrAI = CAST_AI(boss_hodir::boss_hodirAI, Hr->AI()))
                     if (HrAI->isCoolestFriends())
                        return true;
                
                        
             return false;
        }

};

class  achievement_cheese_the_freeze: public AchievementCriteriaScript
{
    public:
        achievement_cheese_the_freeze() : AchievementCriteriaScript("achievement_cheese_the_freeze")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
             
             if (Creature* Hr = target->ToCreature())
                 if (boss_hodir::boss_hodirAI * HrAI = CAST_AI(boss_hodir::boss_hodirAI, Hr->AI()))
                     if (HrAI->IsCheeseTheFreeze())
                        return true;
                
                        
             return false;
        }

};

class  achievement_cache_was_rare: public AchievementCriteriaScript
{
    public:
        achievement_cache_was_rare() : AchievementCriteriaScript("achievement_cache_was_rare")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
             
             if (Creature* Hr = target->ToCreature())
                 if (boss_hodir::boss_hodirAI * HrAI = CAST_AI(boss_hodir::boss_hodirAI, Hr->AI()))
                     if (HrAI->GetRareCache())
                        return true;
                
                        
             return false;
        }

};

class  achievement_getting_cold_in_here: public AchievementCriteriaScript
{
    public:
        achievement_getting_cold_in_here() : AchievementCriteriaScript("achievement_getting_cold_in_here")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
             
             if (Creature* Hr = target->ToCreature())
                 if (boss_hodir::boss_hodirAI * HrAI = CAST_AI(boss_hodir::boss_hodirAI, Hr->AI()))
                     if (HrAI->IsGettingCold())
                        return true;
                
                        
             return false;
        }

};



void AddSC_boss_hodir()
{
    new boss_hodir();
    new npc_icicle();
    new npc_icicle_snowdrift();
    new npc_snowpacked_icicle();
    new npc_hodir_priest();
    new npc_hodir_shaman();
    new npc_hodir_druid();
    new npc_hodir_mage();
    new npc_toasty_fire();
    new npc_flash_freeze();
    new achievement_cache_was_rare();
    new achievement_getting_cold_in_here();
    new achievement_cheese_the_freeze();
    new achievement_have_the_coolest_friends();
}